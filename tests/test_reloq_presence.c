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
    previous.mellow_mode=1;previous.presence_rate=0;prepare(&previous,rate);
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

static void check_voiced_aspiration(int rate)
{
    struct reloq_rate_transfer normal={0},mellow={0};
    mellow.mellow_mode=1;
    prepare(&normal,rate);prepare(&mellow,rate);
    struct reloq_rate_transfer split=mellow;
    float a[3840],b[3840],c[3840];
    const size_t count=(size_t)(rate*.08);
    assert(count<=sizeof a/sizeof *a);
    for(size_t i=0;i<count;++i)a[i]=b[i]=c[i]=1;
    /* An initially unvoiced breath, including h, is identical. */
    reloq_rate_transfer_process_aspiration(&normal,a,count,0);
    reloq_rate_transfer_process_aspiration(&mellow,b,count,0);
    for(size_t i=0;i<count;++i)assert(a[i]==1 && b[i]==1);
    /* After the 5-ms transition, mellow is 4.5 dB below normal.
     * Splitting the same samples across blocks cannot change the output. */
    reloq_rate_transfer_process_aspiration(&normal,a,count,1);
    reloq_rate_transfer_process_aspiration(&mellow,b,count,1);
    for(size_t i=0;i<count;) {
        size_t n=1+(i*31)%127;if(n>count-i)n=count-i;
        reloq_rate_transfer_process_aspiration(&split,c+i,n,1);i+=n;
    }
    assert(!memcmp(b,c,count*sizeof *b));
    assert(fabs(a[count-1]-0.7943282347242815)<1e-6);
    assert(fabs(b[count-1]-0.47315125896148047)<1e-6);
    float last=b[count-1];
    for(size_t i=0;i<count;++i)a[i]=b[i]=1;
    reloq_rate_transfer_process_aspiration(&normal,a,count,0);
    reloq_rate_transfer_process_aspiration(&mellow,b,count,0);
    assert(b[0]>last && b[0]-last<.01);
    assert(a[count-1]>.99999 && b[count-1]>.99999);
}

int main(void)
{
    const int rates[]={16000,22050,32000,44100,48000};
    for(unsigned k=0;k<sizeof rates/sizeof *rates;++k) {
        int rate=rates[k];
        check_voiced_aspiration(rate);
        double least=10,most=0;
        for(int frequency=3500;frequency<=4000;frequency+=25) {
            double gain=lift(rate,frequency);
            assert(gain>5.69 && gain<6.002);
            if(gain<least)least=gain;
            if(gain>most)most=gain;
        }
        assert(most-least<.31);
        assert(lift(rate,3750)>5.95);
        assert(lift(rate,1000)<.02);
        assert(lift(rate,2000)<.35);
        assert(lift(rate,2500)>1.3 && lift(rate,2500)<1.6);
        assert(lift(rate,3000)>3.7 && lift(rate,3000)<4.1);
        assert(lift(rate,5000)<1.1);
        for(unsigned mode=0;mode<2;++mode) {
            struct reloq_rate_transfer stable={0};stable.mellow_mode=mode;prepare(&stable,rate);
            const double *sections[]={stable.voice_colour[3],stable.voice_band};
            for(unsigned i=0;i<2;++i) {
                const double *c=sections[i];
                assert(1+c[3]+c[4]>0 && 1-c[3]+c[4]>0 && 1-c[4]>0);
            }
            struct reloq_rate_transfer a=stable,b=a;
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
        }
        struct reloq_rate_transfer a={0};prepare(&a,rate);
        struct reloq_rate_transfer b=a;
        float full[4096],split[4096];
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
    puts("Mellow mode: wider voice reduction and 4.5 dB less voiced aspiration than normal across all five extended clocks; unvoiced breath, stability and block boundaries passed");
    return 0;
}
