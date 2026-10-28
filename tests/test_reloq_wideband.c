/* Exercise added noise filters through the real frame adapter and PCM core. */
#include "reloq_german_voice_defaults.h"
#include "reloq_german_voice_model.h"
#include "reloq_frame_adapter.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
struct output {int16_t data[5000];size_t count;};
static int receive(void *p,const int16_t *samples,size_t n)
{
 struct output *o=p;assert(o->count+n<=5000);
 memcpy(o->data+o->count,samples,n*sizeof *samples);o->count+=n;return 0;
}
/* Measure energy by processing an impulse through the production filter.
 * This test does not repeat the calibration formula. */
static double impulse_energy(struct reloq_core_resonator filter,int rate)
{
 double energy=0;
 for(int at=0;at<rate/2;++at) {
  float x=at==0?1:0;
  assert(!reloq_core_resonator_process(&filter,&x,1));
  assert(isfinite(x));energy+=(double)x*x;
 }
 return energy;
}
static void noise_calibration(void)
{
 const int rates[]={16000,22050,32000,44100,48000};
 const float frequencies[]={350,1500,2500,3600,3900,5000,6300,7500};
 const float widths[]={80,200,350,500,500,1000,1500,1500};
 for(unsigned band=0;band<8;++band)for(unsigned k=0;k<5;++k) {
  long levels[8]={0};levels[band<5?band:4]=50;
  struct reloq_core_frication_bank reference={0},current={0};
  struct reloq_core_clock base=reloq_core_clock_for_rate(16000);
  assert(!reloq_core_frication_bank_configure(&reference,&base,
   frequencies,widths,levels,5,60,4,0,0));
  double wanted=impulse_energy(reference.stages[band<5?band:4],16000);
  memset(levels,0,sizeof levels);levels[band]=50;
  struct reloq_core_clock clock=reloq_core_clock_for_rate((float)rates[k]);
  clock.noise_reference_rate=16000;
  assert(!reloq_core_frication_bank_configure(&current,&clock,
   frequencies,widths,levels,8,60,4,0,0));
  double actual=impulse_energy(current.stages[band],rates[k]);
  assert(wanted>0 && fabs(actual/wanted-1)<0.002);
 }
}
/* AB is unfiltered at the native 16-kHz noise clock. At extended clocks,
 * measure the complete production branch (including its filter history)
 * to check both retained noise energy and suppressed ultrasonic spread. */
static void bypass_bandwidth(void)
{
 const int rates[]={16000,22050,32000,44100,48000};
 const float f[8]={500,1500,2500,3600,3900,5000,6300,7500};
 const float bw[8]={80,200,350,500,500,1000,1500,1500};
 const long levels[8]={0};
 for(unsigned k=0;k<5;++k) {
  struct reloq_core_clock clock=reloq_core_clock_for_rate((float)rates[k]);
  clock.noise_reference_rate=16000;
  struct reloq_core_frication_bank bank={0};
  int active=rates[k]>=44100?8:rates[k]>=32000?7:6;
  assert(!reloq_core_frication_bank_configure(&bank,&clock,f,bw,levels,active,60,4,0,1));
  float source[2048]={1},result[2048]={0},scratch[2048];double energy=0;
  assert(!reloq_core_frication_bank_mix(&bank,source,result,scratch,2048));
  double pass_r=0,pass_i=0,stop_r=0,stop_i=0,upper_r=0,upper_i=0;
  for(unsigned i=0;i<2048;++i) {
   energy+=(double)result[i]*result[i];
   double a=6.283185307179586*2000*i/rates[k];
   double b=6.283185307179586*(rates[k]*.45)*i/rates[k];
   pass_r+=result[i]*cos(a);pass_i-=result[i]*sin(a);
   stop_r+=result[i]*cos(b);stop_i-=result[i]*sin(b);
   double c=6.283185307179586*f[active-1]*i/rates[k];
   upper_r+=result[i]*cos(c);upper_i-=result[i]*sin(c);
  }
  assert(fabs(energy-1)<0.0001);
  /* The 7.5-kHz edge remains above half power inside the native 8-kHz
   * band; a fixed legacy 5.5-kHz cap fails this check. */
  assert((upper_r*upper_r+upper_i*upper_i)/(pass_r*pass_r+pass_i*pass_i)>.50);
  if(rates[k]>16000)assert((stop_r*stop_r+stop_i*stop_i)/(pass_r*pass_r+pass_i*pass_i)<0.025);
  else assert(fabs(stop_r-1)<1e-6 && fabs(stop_i)<1e-6);
 }
}
int main(void)
{
 noise_calibration();
 bypass_bandwidth();
 const int rates[]={8000,11025,16000,22050,32000,44100,48000};
 const unsigned expected[]={4,5,6,6,7,8,8};
 for(unsigned k=0;k<7;++k) {
  struct reloq_german_voice_model model={50,0,0,0,(float)rates[k]};
  unsigned active=reloq_german_voice_model_formants(&model);assert(active==expected[k]);
  float c[RELOQ_PARAMETER_TRACKS];uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
  struct reloq_session_config config;
  assert(!reloq_german_voice_defaults(c,fields,&config));
  config.sample_rate=(float)rates[k];config.max_frame_samples=5000;
  if(rates[k]>=16000){config.glottal_reference_rate=11025;config.frication_reference_rate=16000;config.reference_formants=5;}
  c[0]=100;c[2]=0;c[7]=0;c[8]=60;c[39]=50;
  reloq_german_voice_model_frame(&model,c);
  struct reloq_session *s=reloq_session_create(&config);assert(s);
  struct reloq_core_voiced_frame f;
  assert(!reloq_frame_adapter_map(c,active,&f));
  struct output full={0},muted={0},quiet={0};size_t delivered=0;
  assert(!reloq_session_render_frames(s,&f,1,receive,&full,&delivered));
  assert(delivered==1 && full.count==(size_t)(rates[k]/10));
  if(active>5) {
   /* The product AF profile owns its upper poles. The unprofiled numeric
    * core must still accept explicitly supplied upper channels. */
   for(unsigned i=5;i<active;++i)assert(f.noise.af_levels[i]==0);
   struct reloq_session_config raw=config;raw.frication_reference_rate=0;
   struct reloq_session *numeric=reloq_session_create(&raw);assert(numeric);
   struct output raw_full={0};
   assert(!reloq_session_render_frames(numeric,&f,1,receive,&raw_full,&delivered));
   for(unsigned i=5;i<active;++i)f.noise.af_levels[i]=40;
   assert(!reloq_session_reset(numeric));
   assert(!reloq_session_render_frames(numeric,&f,1,receive,&muted,&delivered));
   assert(raw_full.count==muted.count && memcmp(raw_full.data,muted.data,raw_full.count*sizeof(int16_t)));
   reloq_session_destroy(numeric);
   for(unsigned i=5;i<active;++i)f.noise.af_levels[i]=0;
  }
  f.noise.af_level=0;
  assert(!reloq_session_reset(s));
  assert(!reloq_session_render_frames(s,&f,1,receive,&quiet,&delivered));
  for(size_t i=0;i<quiet.count;++i)assert(!quiet.data[i]);
  reloq_session_destroy(s);
 }
 puts("Wideband: active oral/noise bank counts, audible upper channels, unchanged source silence and native sample clocks passed");
 return 0;
}
