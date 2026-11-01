#include <stdio.h>
#include <assert.h>
#include "../src/reloq_rate_transfer.c"
int main(void){float rates[]={16000,22050,32000,44100,48000};
unsigned checks=0;double maximum=0;
for(unsigned count=4;count<=5;++count){unsigned top=count;
struct reloq_core_voiced_formants f={.highest_formant=(long)top,.nasal_pole={280,90},.nasal_zero={280,90},.tracheal_pole={250,90},.tracheal_zero={250,90},.oral={{500,80},{1500,90},{2500,120},{3500,200},{4500,300},{5000,1000},{6300,1250},{7500,1500}}};
for(unsigned r=0;r<5;++r){struct reloq_rate_transfer s={.trial_profile=1};reloq_rate_transfer_prepare(&s,&f,rates[r],11025,count);double sum=0;for(unsigned k=0;k<RELOQ_RATE_TAPS;++k)sum+=s.target[k];assert(fabs(sum-.8)<1e-5);double worst=0;
for(unsigned hz=500;hz<=5000;hz+=25){double complex z=cexp(-I*2*pi*hz/rates[r]),z0=cexp(-I*2*pi*hz/11025),fir=0,w=1;for(unsigned i=0;i<RELOQ_RATE_TAPS;++i){fir+=s.target[i]*w;w*=z;}const double *c=s.presence;fir*=(c[0]+c[1]*z+c[2]*z*z)/(1+c[3]*z+c[4]*z*z);double before=1,after=1;for(unsigned j=0;j<count;++j)before*=power(pole_at(f.oral+j,11025),z0);for(unsigned j=0;j<top;++j)after*=power(pole_at(f.oral+j,rates[r]),z);double error=fabs(10*log10(after/before*pow(cabs(fir),2)/classic_reference_power(&s,z0)));if(error>worst)worst=error;}
assert(worst<(r?.5:.05));if(worst>maximum)maximum=worst;++checks;}
}
printf("Classic AV/AH: %u main-band/level cases; worst 0.5–5 kHz residual %.4f dB\n",checks,maximum);return 0;}
