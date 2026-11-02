/* Observe periodic power and aliases through the complete source/tract. */
#include "../src/reloq_core_internal.h"
#include "reloq_frame_adapter.h"
#include "reloq_german_voice_defaults.h"
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
    return 0;
}
