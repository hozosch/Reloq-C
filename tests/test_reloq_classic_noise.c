/* Check the production AF bank, source profile and finite FIR together. */
#include <stdio.h>
#include <assert.h>
#include "../src/reloq_rate_transfer.c"
struct bands {double main,reference,upper,total,shape;};
static struct bands measure(struct reloq_rate_transfer *s,
 const struct reloq_core_frication_bank *ref,const struct reloq_core_frication_bank *cur,
 const struct reloq_noise_profile *p,float rate)
{
 struct bands b={0};double residual[1000];unsigned n=0;
 for(unsigned h=0;h<=(unsigned)(rate/2);h+=25){
  double complex z=cexp(-I*2*pi*h/rate),fir=0,power=1;
  for(unsigned i=0;i<s->taps;++i){fir+=s->target[i]*power;power*=z;}
  double actual=bank_power(cur,z)*profile_power(p,h)*.7943282347242815*pow(cabs(fir),2);
  double target=bank_power(ref,cexp(-I*2*pi*h/11025))*classic_reference_power(s,cexp(-I*2*pi*h/11025))*(rate/11025.);
  if(h<=5000){b.main+=actual;b.reference+=target;}
  if(h>=6500)b.upper+=actual;
  b.total+=actual;
  if(h>=1000&&h<=5000)residual[n++]=10*log10(actual/target);
 }
 double relative=10*log10(b.main/b.reference);
 for(unsigned i=0;i<n;++i){double err=fabs(residual[i]-relative);if(err>b.shape)b.shape=err;}
 return b;
}
int main(void) {
 const float rates[]={16000,22050,32000,44100,48000};
 const long all_levels[4][8]={{0,0,0,0,50,0,0,0},{0,52,60,59,0,0,0,0},{0,45,0,0,0,0,0,0},{0}};
 unsigned checks=0;double worst=0,largest_reduction=0,total_error=0;
 for(int small=0;small<2;++small)for(int kind=0;kind<4;++kind)for(unsigned r=0;r<5;++r){
  float centers[8]={300,1500,2600,3400,4400,5500,6500,7500};
  float widths[8]={100,200,350,500,500,1000,1500,1500},f[8],b[8];long a[8],base[8];
  memcpy(base,all_levels[kind],sizeof base);
  if(small){for(unsigned i=0;i<8;++i)centers[i]*=1.17f;if(kind==0){base[2]=base[4];base[4]=0;}}
  memcpy(f,centers,sizeof f);memcpy(b,widths,sizeof b);memcpy(a,base,sizeof a);long count=small?4:5;
  struct reloq_core_clock rc=reloq_core_clock_for_rate(11025),c=reloq_core_clock_for_rate(rates[r]);
  c.noise_reference_rate=16000;c.noise_reference_formants=small?4:5;
  struct reloq_core_frication source={0};assert(!reloq_core_frication_configure(&source,1,65,kind==3?60:0,-10,0));
  struct reloq_core_frication_bank ref={0},cur={.shaped_source=1};struct reloq_noise_profile p={0};
  assert(!reloq_core_frication_bank_configure(&ref,&rc,centers,widths,base,small?4:5,65,-10,0,source.bypass_gain));
  reloq_noise_profile_prepare(&p,rates[r],0,65,kind==3?60:0,f,b,a,&count);
  assert(!reloq_core_frication_bank_configure(&cur,&c,f,b,a,count,65,-10,0,source.bypass_gain));
  struct reloq_rate_transfer s={.trial_profile=1};assert(reloq_rate_transfer_prepare_classic(&s,&ref,&cur,&p,rates[r]));
  float normal[RELOQ_RATE_TAPS];memcpy(normal,s.target,sizeof normal);
  struct bands full=measure(&s,&ref,&cur,&p,rates[r]);
  assert(reloq_rate_transfer_prepare_classic_bandwidth(&s,&ref,&cur,&p,rates[r],0));
  struct bands anchor=measure(&s,&ref,&cur,&p,rates[r]);
  assert(fabs(10*log10(anchor.main/anchor.reference))<.1);
  double error=fabs(10*log10(full.total/anchor.total));
  if(error>total_error)total_error=error;
  assert(error<.12);
  if(full.shape>worst)worst=full.shape;
  assert(full.shape<.85);
  double reduction=-10*log10(full.main/anchor.main);
  if(reduction>largest_reduction)largest_reduction=reduction;
  if(!r){assert(!memcmp(normal,s.target,sizeof normal));assert(full.upper/full.main<.01);}
  else {assert(full.upper>anchor.upper);assert(reduction>-.05);}
  assert(reloq_rate_transfer_prepare_classic(&s,&ref,&cur,&p,rates[r]));
  assert(!memcmp(normal,s.target,sizeof normal));
  if(kind==0&&r){
   for(unsigned rolled=0;rolled<2;++rolled) {
    s.sibilant_extension=rolled;
    assert(reloq_rate_transfer_prepare_classic_bandwidth(&s,&ref,&cur,&p,rates[r],.25));
    struct bands old_cast=measure(&s,&ref,&cur,&p,rates[r]);
    assert(reloq_rate_transfer_prepare_classic_bandwidth(&s,&ref,&cur,&p,rates[r],.18));
    struct bands cast=measure(&s,&ref,&cur,&p,rates[r]);
    assert(cast.upper<old_cast.upper&&cast.upper<full.upper);
    assert(fabs(10*log10(cast.total/anchor.total))<.12);
   }
  }
  ++checks;
 }
 printf("Classic noise: %u cases; main-band shape error <=%.3f dB; total power error <=%.3f dB; largest main-band compensation %.3f dB\n",checks,worst,total_error,largest_reduction);
 return 0;
}
