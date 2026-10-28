/* Measure the requested extra peak through the production streaming path. */
#include "reloq_core.h"
#include "reloq_rate_transfer.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void prepare(struct reloq_rate_transfer *s,int rate)
{
    struct reloq_core_voiced_formants f={0};
    f.highest_formant=5;
    for(unsigned i=0;i<8;++i) {
        f.oral[i].frequency_hz=500+600*i;
        f.oral[i].bandwidth_hz=80+30*i;
    }
    f.nasal_pole=f.nasal_zero=f.tracheal_pole=f.tracheal_zero=f.oral[0];
    reloq_rate_transfer_prepare(s,&f,(float)rate,11025,5);
}

static double lift(int rate,int frequency)
{
    struct reloq_rate_transfer current={0};prepare(&current,rate);
    struct reloq_rate_transfer previous=current;
    memset(previous.voice_colour[3],0,sizeof previous.voice_colour[3]);
    previous.voice_colour[3][0]=1;
    previous.voice_band[0]=0;
    double old_energy=0,new_energy=0;
    for(int i=0;i<rate/2;++i) {
        float x=(float)(.01*sin(6.283185307179586*frequency*i/rate)),a=x,b=x;
        reloq_rate_transfer_process_voice(&previous,&a,1);
        reloq_rate_transfer_process_voice(&current,&b,1);
        assert(isfinite(a)&&isfinite(b));
        if(i>rate/4) {old_energy+=(double)a*a;new_energy+=(double)b*b;}
    }
    return 10*log10(new_energy/old_energy);
}

int main(void)
{
    const int rates[]={16000,22050,32000,44100,48000};
    for(unsigned k=0;k<sizeof rates/sizeof *rates;++k) {
        int rate=rates[k];
        double least=10,most=0;
        for(int frequency=3500;frequency<=4000;frequency+=25) {
            double gain=lift(rate,frequency);
            assert(gain>2.41 && gain<2.502);
            if(gain<least)least=gain;
            if(gain>most)most=gain;
        }
        assert(most-least<.12);
        assert(fabs(lift(rate,3750)-2.5)<.002);
        assert(lift(rate,1000)<.002);
        assert(lift(rate,2000)<.04);
        assert(lift(rate,3000)<1.56);
        struct reloq_rate_transfer stable={0};prepare(&stable,rate);
        const double *sections[]={stable.voice_colour[3],stable.voice_band};
        for(unsigned i=0;i<2;++i) {
            const double *c=sections[i];
            assert(1+c[3]+c[4]>0 && 1-c[3]+c[4]>0 && 1-c[4]>0);
        }
        struct reloq_rate_transfer a={0};prepare(&a,rate);
        struct reloq_rate_transfer b=a;
        float full[4096],split[4096];
        for(unsigned i=0;i<4096;++i)
            full[i]=split[i]=(float)(.01*sin(i*.743)+.003*cos(i*.177));
        reloq_rate_transfer_process_voice(&a,full,4096);
        for(size_t i=0;i<4096;) {
            size_t n=1+(i*31)%127;if(n>4096-i)n=4096-i;
            reloq_rate_transfer_process_voice(&b,split+i,n);i+=n;
        }
        assert(!memcmp(full,split,sizeof full));
        assert(!memcmp(a.voice_colour_delay,b.voice_colour_delay,sizeof a.voice_colour_delay));
        assert(!memcmp(a.voice_band_delay,b.voice_band_delay,sizeof a.voice_band_delay));
        /* The new section cannot change aspiration's existing smoothing. */
        a.aspiration_voicing=b.aspiration_voicing=0;
        for(unsigned i=0;i<4096;++i)full[i]=split[i]=.01f;
        b.voice_colour[3][0]=0;
        reloq_rate_transfer_process_aspiration(&a,full,4096,1);
        reloq_rate_transfer_process_aspiration(&b,split,4096,1);
        assert(!memcmp(full,split,sizeof full));
        memset(&a,0,sizeof a);prepare(&a,rate);
        memset(full,0,sizeof full);reloq_rate_transfer_process_voice(&a,full,4096);
        for(unsigned i=0;i<4096;++i)assert(full[i]==0);
    }
    for(int rate=8000;rate<16000;rate+=3025) {
        struct reloq_rate_transfer a={0};prepare(&a,rate);
        float samples[]={.1f,-.2f,.3f},before[3];memcpy(before,samples,sizeof before);
        reloq_rate_transfer_process_voice(&a,samples,3);
        assert(!memcmp(samples,before,sizeof before));
    }
    puts("Presence: flat 3.5..4 kHz band at about 2.44..2.5 dB, stable causal sections, all five extended clocks; exact block boundaries, unchanged aspiration and legacy bypass passed");
    return 0;
}
