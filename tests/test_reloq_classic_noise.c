/* Exercise the production AF bank, source profile and finite FIR together.
 * Independent response observations are checked by test_reloq_rate_dispatch. */
#include <stdio.h>
#include <assert.h>
#include "../src/reloq_rate_transfer.c"
int main(void) {
 float rates[]={16000,22050,32000,44100,48000};
 long all_levels[4][8]={{0,0,0,0,50,0,0,0},{0,52,60,59,0,0,0,0},{0,45,0,0,0,0,0,0},{0}};
 unsigned checks=0;double worst=0;
 for(int small=0;small<2;++small)for(int kind=0;kind<4;++kind)for(unsigned r=0;r<5;++r){
 float centers[8]={300,1500,2600,3400,4400,5500,6500,7500};
 float widths[8]={100,200,350,500,500,1000,1500,1500},f[8],b[8];long a[8];
 long base[8];memcpy(base,all_levels[kind],sizeof base);
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
 double sum=0,want=0,upper=0,maxerr=0;
 for(unsigned h=0;h<=(unsigned)(rates[r]/2);h+=25){
 double complex z=cexp(-I*2*pi*h/rates[r]),fir=0,power=1;
 for(unsigned i=0;i<s.taps;++i){fir+=s.target[i]*power;power*=z;}
 double actual=bank_power(&cur,z)*profile_power(&p,h)*.7943282347242815*pow(cabs(fir),2);
 double target=bank_power(&ref,cexp(-I*2*pi*h/11025))*classic_reference_power(&s,cexp(-I*2*pi*h/11025))*(rates[r]/11025.);
 if(h<=5000){sum+=actual;want+=target;}
 if(h>=6500)upper+=actual;
 if(h>=1000&&h<=5000){double err=fabs(10*log10(actual/target));if(err>maxerr)maxerr=err;}
 }
 assert(fabs(10*log10(sum/want))<.06);
 assert(maxerr<.75);if(maxerr>worst)worst=maxerr;
 if(!r)assert(upper/sum<.01);
 else assert(upper/sum>0);
 /* The bandwidth control changes only the optional extra band. Its
  * complete Presence main band and the normal cache entry must survive. */
 float normal[RELOQ_RATE_TAPS];memcpy(normal,s.target,sizeof normal);
 assert(reloq_rate_transfer_prepare_classic_bandwidth(&s,&ref,&cur,&p,rates[r],0));
 if(!r)assert(!memcmp(normal,s.target,sizeof normal));
 double limited_low=0,limited_upper=0;
 for(unsigned h=0;h<=(unsigned)(rates[r]/2);h+=25){
  double complex z=cexp(-I*2*pi*h/rates[r]),fir=0,power=1;
  for(unsigned i=0;i<s.taps;++i){fir+=s.target[i]*power;power*=z;}
  double actual=bank_power(&cur,z)*profile_power(&p,h)*.7943282347242815*pow(cabs(fir),2);
  if(h<=5000)limited_low+=actual;
  if(h>=6500)limited_upper+=actual;
 }
 assert(fabs(10*log10(limited_low/want))<.1);
 if(r)assert(limited_upper<upper);
 assert(reloq_rate_transfer_prepare_classic_bandwidth(&s,&ref,&cur,&p,rates[r],.25));
 double quarter_low=0,quarter_upper=0;
 for(unsigned h=0;h<=(unsigned)(rates[r]/2);h+=25){
  double complex z=cexp(-I*2*pi*h/rates[r]),fir=0,power=1;
  for(unsigned i=0;i<s.taps;++i){fir+=s.target[i]*power;power*=z;}
  double actual=bank_power(&cur,z)*profile_power(&p,h)*.7943282347242815*pow(cabs(fir),2);
  if(h<=5000)quarter_low+=actual;
  if(h>=6500)quarter_upper+=actual;
 }
 assert(fabs(10*log10(quarter_low/want))<.1);
 if(r){assert(quarter_upper>limited_upper);assert(quarter_upper<upper);}
 else assert(!memcmp(normal,s.target,sizeof normal));
 assert(reloq_rate_transfer_prepare_classic(&s,&ref,&cur,&p,rates[r]));
 assert(!memcmp(normal,s.target,sizeof normal));
 ++checks;
 }
 printf("Classic noise: %u full-path and %u limited/quarter-band/cache checks; worst 1–5 kHz residual %.3f dB\n",checks,checks,worst);
 return 0;
}
