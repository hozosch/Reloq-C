#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "../src/reloq_rate_transfer.c"
static uint32_t random_state=917;
static float value(void) {random_state=random_state*1664525u+1013904223u;return ((int32_t)(random_state>>8)-8388608)*0x1p-25f;}
int main(void) {
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
