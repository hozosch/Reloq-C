#include <stdio.h>
#include <assert.h>
#include "../src/reloq_rate_transfer.c"

static void main_band(void)
{
    const float rates[]={16000,22050,32000,44100,48000};
    unsigned checks=0;double maximum=0;
    for(unsigned count=4;count<=5;++count) {
        struct reloq_core_voiced_formants f={.highest_formant=(long)count,
            .nasal_pole={280,90},.nasal_zero={280,90},
            .tracheal_pole={250,90},.tracheal_zero={250,90},
            .oral={{500,80},{1500,90},{2500,120},{3500,200},{4500,300}}};
        for(unsigned r=0;r<5;++r) {
            struct reloq_rate_transfer s={.trial_profile=1};
            reloq_rate_transfer_prepare(&s,&f,rates[r],11025,count);
            double sum=0,worst=0;
            for(unsigned k=0;k<RELOQ_RATE_TAPS;++k)sum+=s.target[k];
            assert(fabs(sum-.8)<1e-5);
            for(unsigned hz=500;hz<=5000;hz+=25) {
                double complex z=cexp(-I*2*pi*hz/rates[r]);
                double complex z0=cexp(-I*2*pi*hz/11025),fir=0,w=1;
                for(unsigned k=0;k<RELOQ_RATE_TAPS;++k){fir+=s.target[k]*w;w*=z;}
                const double *c=s.presence;
                fir*=(c[0]+c[1]*z+c[2]*z*z)/(1+c[3]*z+c[4]*z*z);
                double before=1,after=1;
                for(unsigned j=0;j<count;++j) {
                    before*=power(pole_at(f.oral+j,11025),z0);
                    after*=power(pole_at(f.oral+j,rates[r]),z);
                }
                double error=fabs(10*log10(after/before*pow(cabs(fir),2)/
                    classic_reference_power(&s,z0)));
                worst=fmax(worst,error);
            }
            assert(worst<(r?.5:.05));maximum=fmax(maximum,worst);++checks;
        }
    }
    printf("Classic AV/AH: %u main-band/level cases; worst 0.5–5 kHz residual %.4f dB\n",checks,maximum);
}

static void image_regression(void)
{
    /* This ordinary /w/-vowel transition in our English frontend made
     * dev6's 48-kHz kernel L1 norm exceed 600,000. Scale it through both
     * tract sizes and clocks; an image at 11.025 kHz must not be inverted. */
    const float rates[]={16000,22050,32000,44100,48000};
    unsigned checks=0;double maximum=0;
    for(unsigned count=4;count<=5;++count)for(unsigned r=0;r<5;++r)
        for(unsigned step=0;step<41;++step) {
            struct reloq_core_voiced_formants f={.highest_formant=(long)count,
                .nasal_pole={200,90},.nasal_zero={200,90},
                .tracheal_pole={250,90},.tracheal_zero={250,90},
                .oral={{302,152},{624,99},{2248,486},{3348,322},{3648,252}}};
            for(unsigned j=0;j<count;++j)f.oral[j].frequency_hz*=.8f+.01f*step;
            struct reloq_rate_transfer s={.trial_profile=1};
            reloq_rate_transfer_prepare(&s,&f,rates[r],11025,count);
            double dc=0,l1=0;
            for(unsigned k=0;k<RELOQ_RATE_TAPS;++k) {
                assert(isfinite(s.target[k]));dc+=s.target[k];l1+=fabs(s.target[k]);
            }
            assert(fabs(dc-.8)<1e-5);assert(l1<32);
            maximum=fmax(maximum,l1);++checks;
        }
    printf("Classic image regression: %u clock/tract/transition cases; maximum kernel L1 %.4f\n",checks,maximum);
}

int main(void)
{
    main_band();image_regression();return 0;
}
