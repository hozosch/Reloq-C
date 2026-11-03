/* Observe periodic power and aliases through the complete source/tract. */
#include "../src/reloq_core_internal.h"
#include "reloq_frame_adapter.h"
#include "reloq_german_voice_defaults.h"
#include "../src/reloq_session.c"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
static const double pi=3.14159265358979323846;
struct observation {double bass,middle,upper,hnr;};
static struct observation observe(const float *x,size_t n,double rate,double f0)
{
    double total=0,mean=0,harmonics=0;struct observation o={0};
    for(size_t i=0;i<n;++i){total+=(double)x[i]*x[i];mean+=x[i];}
    total=total/n-(mean/n)*(mean/n);
    for(unsigned h=1;h*f0<rate*.5;++h) {
        double hz=h*f0,angle=2*pi*hz/rate,c=cos(angle),s=sin(angle);
        double re=0,im=0,wr=1,wi=0;
        for(size_t i=0;i<n;++i) {
            re+=x[i]*wr;im+=x[i]*wi;
            double next=wr*c-wi*s;wi=wr*s+wi*c;wr=next;
        }
        double power=2*(re*re+im*im)/(n*n);harmonics+=power;
        if(hz>=100&&hz<800)o.bass+=power;
        if(hz>=800&&hz<2500)o.middle+=power;
        if(hz>=3500&&hz<5000)o.upper+=power;
    }
    o.hnr=10*log10(harmonics/fmax(1e-20,total-harmonics));return o;
}
static struct observation body(float rate,unsigned profile,unsigned vowel,double f0)
{
    float controls[62];uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    assert(!reloq_german_voice_defaults(controls,fields,&config));
    controls[0]=2000;controls[1]=(float)(10*f0);controls[2]=50;
    struct reloq_core_voiced_frame frame;
    assert(!reloq_frame_adapter_map(controls,5,&frame));
    frame.glottal_profile=7*vowel;frame.noise.ah_level=0;
    frame.noise.af_level=0;frame.noise.bypass_level=0;
    const float formants[3][5]={{700,1200,2600,3500,4500},
        {300,2200,3000,3800,4500},{300,800,2500,3500,4500}};
    for(unsigned i=0;i<5;++i) {
        frame.noise.tract.oral[i].frequency_hz=formants[vowel][i];
        frame.noise.tract.oral[i].bandwidth_hz=80+20*i;
    }
    struct reloq_core_voiced_frame_state *s=calloc(1,sizeof *s);assert(s);
    assert(!reloq_core_voiced_frame_initialize(s,rate,8,4,6,0));s->av_trim=-5;
    s->glottal_reference_rate=11025;s->reference_formants=5;s->frication_reference_rate=16000;
    s->rate_transfer.trial_profile=s->noise_transfer.trial_profile=profile==1;
    s->rate_transfer.smooth_updates=profile==2;
    if(profile==5) {
        config.sample_rate=rate;
        config.glottal_reference_rate=11025;config.reference_formants=5;
        config.frication_reference_rate=16000;
        struct reloq_session *session=reloq_session_create(&config);assert(session);
        assert(!reloq_session_high_rate_profile(session,1));
        assert(!reloq_session_reset(session));
        *s=session->frame_state;
        reloq_session_destroy(session);
    }
    if(profile==3) {
        s->rate_transfer.trial_profile=rate==16000;
        s->rate_transfer.smooth_updates=rate>=22050;
    }
    if((profile==3||profile==4||profile==5)&&rate>=22050) {
        frame.noise.tract.highest_formant=rate>=44100?8:rate>=32000?7:6;
        for(unsigned i=5;i<8;++i)frame.noise.tract.oral[i]=(struct reloq_core_formant){
            5000+1300*(i-5),1000+250*(i-5)};
    }
    size_t capacity=(size_t)(2*rate)+16,count=0;
    float *work=calloc(3*capacity,sizeof *work);uint8_t *mask=calloc(capacity,1);assert(work&&mask);
    assert(!reloq_core_voiced_frame_render_owned(s,&frame,work,work+capacity,
        work+2*capacity,mask,NULL,capacity,&count));
    struct observation o=observe(work+count-(size_t)rate,(size_t)rate,rate,f0);
    free(work);free(mask);free(s);return o;
}
int main(void)
{
    const float rates[]={16000,22050,32000,44100,48000};
    unsigned checks=0;double bass_error=0,minimum_hnr=1000;
    for(unsigned vowel=0;vowel<3;++vowel) {
        struct observation ref=body(11025,0,vowel,171),base={0};
        for(unsigned r=0;r<5;++r) {
            struct observation o=body(rates[r],1,vowel,171);
            double bass=fabs(10*log10(o.bass/ref.bass));
            if(bass>bass_error)bass_error=bass;
            assert(bass<1);
            if(!r) {
                base=o;double upper=10*log10(o.upper/ref.upper);
                assert(upper> -1.5&&upper<4);
            } else assert(o.upper>=.85*base.upper);
            assert(o.hnr>38);if(o.hnr<minimum_hnr)minimum_hnr=o.hnr;
            ++checks;
            /* A's auxiliary periodic body used to miss source anti-aliasing. */
            o=body(rates[r],2,vowel,187);
            assert(o.hnr>38);if(o.hnr<minimum_hnr)minimum_hnr=o.hnr;
            ++checks;
        }
    }
    printf("Classic full source: %u vowel/rate/alias cases; bass error <=%.3f dB; harmonic/noise >=%.2f dB\n",checks,bass_error,minimum_hnr);
    double a_bass=0,a_hnr=1000,a_match=0;
    for(unsigned vowel=0;vowel<3;++vowel)for(unsigned pitch=0;pitch<3;++pitch) {
        double f0=113+64*pitch;
        struct observation base=body(16000,1,vowel,f0);
        for(unsigned r=1;r<5;++r) {
            struct observation o=body(rates[r],3,vowel,f0);
            struct observation a=body(rates[r],4,vowel,f0);
            double bass=fabs(10*log10(o.bass/base.bass));
            if(bass>a_bass)a_bass=bass;
            if(o.hnr<a_hnr)a_hnr=o.hnr;
            assert(bass<.75);assert(o.hnr>50);
            assert(o.upper>.7*base.upper);
            /* Compare the actual A body at this clock, not protected PCM
             * peaks. Stable state/closure retain its vowel spectrum. */
            double bands[]={fabs(10*log10(o.bass/a.bass)),
                fabs(10*log10(o.middle/a.middle)),fabs(10*log10(o.upper/a.upper))};
            for(unsigned b=0;b<3;++b) {
                assert(bands[b]<1);
                if(bands[b]>a_match)a_match=bands[b];
            }
        }
    }
    printf("A periodic body: 36 vowel/pitch/rate cases; bass vs 16 kHz <=%.3f dB; A spectral difference <=%.3f dB; harmonic/noise >=%.2f dB\n",a_bass,a_match,a_hnr);
    double staged_bass=0,staged_hnr=1000,smallest_step=1000,max_upper=0;
    for(unsigned vowel=0;vowel<3;++vowel)for(unsigned pitch=0;pitch<3;++pitch) {
        double f0=113+64*pitch;
        struct observation base=body(16000,5,vowel,f0),previous=base;
        for(unsigned r=1;r<5;++r) {
            struct observation o=body(rates[r],5,vowel,f0);
            double bass=fabs(10*log10(o.bass/base.bass));
            double step=10*log10(o.upper/previous.upper),upper=10*log10(o.upper/base.upper);
            if(bass>staged_bass)staged_bass=bass;
            if(o.hnr<staged_hnr)staged_hnr=o.hnr;
            if(step<smallest_step)smallest_step=step;
            if(upper>max_upper)max_upper=upper;
            if(bass>=1||o.hnr<=48||step<=-.15||upper>=6)
                fprintf(stderr,"gradual vowel=%u f0=%.0f rate=%.0f bass=%.3f hnr=%.3f step=%.3f upper=%.3f\n",vowel,f0,rates[r],bass,o.hnr,step,upper);
            assert(bass<1);assert(o.hnr>48);assert(step>-.15);assert(upper<6);
            previous=o;
        }
    }
    printf("Gradual production body: 36 cases; bass <=%.3f dB; smallest upper-band step %.3f dB; upper gain <=%.3f dB; harmonic/noise >=%.2f dB\n",staged_bass,smallest_step,max_upper,staged_hnr);
    return 0;
}
