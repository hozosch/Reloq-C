#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../src/reloq_rate_transfer.c"
static uint32_t random_state=917;
static float value(void) {random_state=random_state*1664525u+1013904223u;return ((int32_t)(random_state>>8)-8388608)*0x1p-25f;}
static double response(const double c[5],double hz,double rate)
{
 double complex z=cexp(-I*2*pi*hz/rate);
 return cabs((c[0]+c[1]*z+c[2]*z*z)/(1+c[3]*z+c[4]*z*z));
}
static void noise_grid_reuse(void)
{
 const float rates[]={16000,22050,32000,44100,48000};unsigned checks=0;
 for(unsigned r=0;r<5;++r)for(int kind=0;kind<4;++kind) {
  struct reloq_noise_profile p={0};p.kind=kind;p.rate=rates[r];p.upper_target=kind*.125;
  for(unsigned j=0;j<6;++j) {
   p.target[j][0]=.8+(double)j*.01;
   p.target[j][1]=-.12;p.target[j][2]=.04;p.target[j][3]=-.15;p.target[j][4]=.07;
  }
  const struct noise_clock_grid *clock=get_noise_clock(r,rates[r]);
  const struct noise_grid *grid=get_noise_grid(&p,r,rates[r],clock);assert(grid);
  for(unsigned k=0;k<=FFT_SIZE/2;++k) {
   double hz=(double)k*rates[r]/FFT_SIZE;
   double direct=profile_power(&p,hz);
   double complex current=cexp(-I*2*pi*hz/rates[r]);
   double complex reference=cexp(-I*2*pi*hz/11025);
   double classic=hz<=11025?classic_colour_power(hz):0;
   double extension=hz<=5512.5?0:hz>=6500?1:
       .5-.5*cos(pi*(hz-5512.5)/987.5);
   double ratio=hz/8500,quadratic=ratio*ratio;
   double retracted=1/(1+quadratic*quadratic);
   assert(!memcmp(&direct,grid->profile+k,sizeof direct));
   assert(!memcmp(&current,clock->z[0]+k,sizeof current));
   assert(!memcmp(&reference,clock->z[1]+k,sizeof reference));
   assert(!memcmp(&classic,clock->classic+k,sizeof classic));
   assert(!memcmp(&extension,clock->extension+k,sizeof extension));
   assert(!memcmp(&retracted,clock->retracted+k,sizeof retracted));++checks;
  }
  p.target[0][0]+=.01;
  assert(!get_noise_grid(&p,r,rates[r],clock));
  p.target[0][0]=.8;p.rate=rates[r]/2;
  assert(!get_noise_grid(&p,r,rates[r],clock));
 }
 printf("Noise transfer: %u cached/direct frequency and response checks bit-identical; custom profile falls back\n",checks);
}
static void classic_response(void)
{
 /* The shared Classic contour and AF reference stay fixed. The small
  * upper-bass restoration belongs only to periodic excitation. */
 const double hz[]={0,1000,2000,3000,4000,4250,4500,5000,5250,5512.5};
 const double db[]={-1.93819978,-1.57232249,-.47737361,1.92730905,3.07599515,3.12060225,2.95364833,1.46825031,-.13629875,-2.67036249};
 const float rates[]={16000,22050,32000,44100,48000};
 for(unsigned r=0;r<5;++r) {
  struct reloq_rate_transfer s={.trial_profile=1},a={0};
  prepare_colour(&s,rates[r]);prepare_colour(&a,rates[r]);
  assert(fabs(response(s.presence,0,rates[r])-1)<1e-9);
  assert(response(a.presence,4000,rates[r])>1.3);
  assert(fabs(20*log10(response(s.voice_colour[0],300,rates[r]))-
      .6)<1e-7);
  assert(fabs(response(s.voice_colour[0],0,rates[r])-1)<1e-9);
  assert(20*log10(response(s.voice_colour[0],1500,rates[r]))<.1);
  /* A fixed broad bell concentrates periodic presence at 3.5–4 kHz;
   * unlike the old shelf it does not raise all the way to Nyquist. */
  assert(fabs(20*log10(response(s.voice_colour[1],0,rates[r])))<1e-7);
  assert(fabs(20*log10(response(s.voice_colour[1],3750,rates[r]))-1.3)<1e-7);
  assert(response(s.voice_colour[1],3750,rates[r])>response(s.voice_colour[1],5000,rates[r]));
  assert(20*log10(response(s.voice_colour[1],300,rates[r]))<.05);
  for(unsigned f=0;f<10;++f) {
   double complex z=cexp(-I*2*pi*hz[f]/11025);
   double gain=10*log10(classic_reference_power(&s,z));
   assert(fabs(gain-db[f])<.0001);
  }
  float x[1024],y[1024],whole[1024];
  for(unsigned j=0;j<1024;++j)x[j]=y[j]=value();
  struct reloq_rate_transfer split=s;
  reloq_rate_transfer_process_voice(&s,y,1024);
  reloq_rate_transfer_process_voice(&split,x,19);
  reloq_rate_transfer_process_voice(&split,x+19,1005);
  assert(!memcmp(x,y,sizeof x));
  memcpy(whole,y,sizeof whole);
  reloq_rate_transfer_process_aspiration(&s,y,1024,1);
  assert(memcmp(whole,y,sizeof whole));
  for(unsigned j=0;j<1024;++j)assert(fabs(y[j])<=fabs(whole[j]));

 }
 puts("Full Classic Presence target: AV/AH share the reference contour; upper extension begins at 22.05 kHz");
}
static void gradual_response(void)
{
 const float rates[]={22050,32000,44100,48000};unsigned checks=0;
 /* Check the actual production amounts, including the 16-kHz foundation.
  * Gain is gradual in physical Hz and never lifts the bass wholesale. */
 const float all_rates[]={16000,22050,32000,44100,48000};
 const float amounts[]={0,.12f,.24f,.32f,.36f};double previous=0;
 for(unsigned r=0;r<5;++r) {
  struct reloq_core_voiced_formants f={0};
  struct reloq_rate_transfer s={.trial_profile=1,.body_amount=amounts[r]};
  prepare_colour(&s,all_rates[r]);reloq_rate_transfer_prepare_body(&s,&f,all_rates[r]);
  double middle=response(s.voice_colour[1],3750,all_rates[r]);
  double bass=response(s.voice_colour[1],300,all_rates[r]);
  if(s.body_ready) {middle*=response(s.body_target[0],3750,all_rates[r]);bass*=response(s.body_target[0],300,all_rates[r]);}
  double expected=1.3+3*amounts[r],db=20*log10(middle);
  assert(fabs(db-expected)<.00001);assert(db>previous);previous=db;
  assert(fabs(20*log10(bass))<.05);
 }
 for(unsigned r=0;r<4;++r) {
  struct reloq_core_voiced_formants f={.highest_formant=8};
  for(unsigned j=5;j<8;++j)f.oral[j]=(struct reloq_core_formant){5000+1300*(j-5),1000+250*(j-5)};
  struct reloq_rate_transfer a={.trial_profile=1,.body_amount=.46f};
  prepare_colour(&a,rates[r]);reloq_rate_transfer_prepare_body(&a,&f,rates[r]);
  struct reloq_rate_transfer b=a;
  double fixed[3][5];memcpy(fixed,a.body_target,sizeof fixed);
  assert(fabs(20*log10(response(fixed[0],3750,rates[r]))-1.38)<.00001);
  assert(20*log10(response(fixed[0],200,rates[r]))<.1);
  for(unsigned j=1;j<3;++j)assert(fixed[j][0]==1&&fixed[j][1]==0&&fixed[j][2]==0&&fixed[j][3]==0&&fixed[j][4]==0);
  float whole[1037],split[1037];
  for(unsigned change=0;change<12;++change) {
   if(change) {
    f.oral[5].frequency_hz=4500+137*change;
    f.oral[6].frequency_hz=6000+91*change;
    reloq_rate_transfer_prepare_body(&a,&f,rates[r]);
    reloq_rate_transfer_prepare_body(&b,&f,rates[r]);
    assert(!memcmp(fixed,a.body_target,sizeof fixed));
   }
   for(unsigned j=0;j<1037;++j)whole[j]=split[j]=value();
   reloq_rate_transfer_process_voice(&a,whole,1037);
   const unsigned boundaries[]={0,1,17,211,599,1037};
   for(unsigned k=1;k<6;++k)reloq_rate_transfer_process_voice(&b,
       split+boundaries[k-1],boundaries[k]-boundaries[k-1]);
   assert(!memcmp(whole,split,sizeof whole));
   for(unsigned j=0;j<1037;++j)assert(isfinite(whole[j])&&fabs(whole[j])<16);
   ++checks;
  }
  for(unsigned j=0;j<3;++j)assert(fabs(response(a.body_target[j],0,rates[r])-1)<1e-9);
 }
 printf("Broad periodic contour: %u changing-formant/chunk/state comparisons identical; no moving upper resonances\n",checks);
}
static void gentle_a_response(void)
{
 const float rates[]={16000,22050,32000,44100,48000};
 const float amounts[]={0,.15f,.26666667f,.36666667f,.41666667f};
 const double expected[]={0,.45,.8,1.1,1.25};
 for(unsigned r=0;r<5;++r) {
  struct reloq_core_voiced_formants f={0};
  struct reloq_rate_transfer original={0},c={.body_amount=amounts[r]};
  prepare_colour(&original,rates[r]);prepare_colour(&c,rates[r]);
  reloq_rate_transfer_prepare_body(&c,&f,rates[r]);
  if(r) {
   assert(c.body_ready);
   assert(fabs(20*log10(response(c.body_target[0],3750,rates[r]))-expected[r])<.00001);
   assert(fabs(20*log10(response(c.body_target[0],300,rates[r])))<.01);
  } else assert(!c.body_ready);
  struct reloq_rate_transfer split=c;
  float base[1037],whole[1037],partial[1037];
  for(unsigned i=0;i<1037;++i)base[i]=whole[i]=partial[i]=value();
  reloq_rate_transfer_process_voice(&original,base,1037);
  reloq_rate_transfer_process_voice(&c,whole,1037);
  const unsigned points[]={0,1,17,211,599,1037};
  for(unsigned i=1;i<6;++i)reloq_rate_transfer_process_voice(&split,partial+points[i-1],points[i]-points[i-1]);
  assert(!memcmp(whole,partial,sizeof whole));
  assert((memcmp(base,whole,sizeof base)!=0)==(r!=0));
  for(unsigned i=0;i<1037;++i)assert(isfinite(whole[i]));
 }
 puts("Gentle A-voice contour: fixed 0/0.45/0.8/1.1/1.25 dB; bass and chunk boundaries checked");
}
int main(void) {
 classic_response();
 gradual_response();
 gentle_a_response();
 noise_grid_reuse();
 unsigned tests=0;
#ifdef RATE_AVX2
 int avx2_available=avx2_enabled();
#else
 int avx2_available=0;
#endif
 for(unsigned taps=65;taps<=129;taps+=64)for(unsigned r=0;r<9;++r)for(unsigned cursor=0;cursor<129;cursor+=7)for(unsigned colour=0;colour<2;++colour) {
  struct reloq_rate_transfer a={0},b;unsigned remaining[]={0,1,2,3,4,5,16,48,49};
  a.ready=1;a.taps=taps;a.remaining=remaining[r];a.cursor=cursor;a.key[0]=r==0&&cursor==0?-1:48000;
  for(unsigned i=0;i<RELOQ_RATE_TAPS;++i){a.kernel[i]=value();a.step[i]=value()*.01f;a.target[i]=value();}
  for(unsigned i=0;i<2*RELOQ_RATE_TAPS;++i)a.history[i]=value();
  if(colour){a.presence_rate=48000;a.presence[0]=.75;a.presence[1]=.125;a.presence[2]=.0625;a.presence[3]=-.125;a.presence[4]=.0625;}
  b=a;float x[1021],y[1021];for(unsigned i=0;i<1021;++i)x[i]=i%17?value():-0.0f;memcpy(y,x,sizeof x);
  for(size_t at=0;at<1021;){size_t n=1+(at*13+cursor*19)%71;if(n>1021-at)n=1021-at;
   process_baseline(&a,x+at,n);reloq_rate_transfer_process(&b,y+at,n);at+=n;
   assert(!memcmp(&a,&b,sizeof a));assert(!memcmp(x,y,at*sizeof *x));++tests;
  }
 }
#ifdef RATE_AVX2
 if(avx2_available)for(unsigned test=0;test<20000;++test) {
  double ar[4],ai[4],br[4],bi[4],m[4];
  for(unsigned i=0;i<4;++i) {ar[i]=0;ai[i]=(double)value()*256;m[i]=fabs((double)value())+1e-12;}
  if(test%37==0)ai[test%4]=70;
  if(test%41==0)ai[test%4]=-0.0;
  memcpy(br,ar,sizeof ar);memcpy(bi,ai,sizeof ai);
  exponentiate_pair(ar,ai,m);exponentiate_pair(ar+2,ai+2,m+2);
  exponentiate_four(br,bi,m);
  assert(!memcmp(ar,br,sizeof ar)&&!memcmp(ai,bi,sizeof ai));
 }
 if(avx2_available)puts("Spectral phase: 20000 four-lane/reference comparisons bit-identical");
#endif
 printf("Filter dispatch: %u ramp/wrap/chunk/state comparisons bit-identical; AVX2=%d\n",tests,avx2_available);return 0;
}
