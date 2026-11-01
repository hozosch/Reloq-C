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
      (.6+.4*classic_rate_extension(rates[r])))<1e-7);
  assert(fabs(response(s.voice_colour[0],0,rates[r])-1)<1e-9);
  assert(20*log10(response(s.voice_colour[0],1500,rates[r]))<.1);
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
  assert(!memcmp(whole,y,sizeof whole));

 }
 puts("Full Classic Presence target: AV/AH share the reference contour; upper extension begins at 22.05 kHz");
}
int main(void) {
 classic_response();
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
