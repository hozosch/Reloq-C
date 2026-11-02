/* Observe the complete periodic path, not only the tract's fitted FIR. */
#include "../src/reloq_rate_transfer.c"
#include "../src/reloq_core_internal.h"
#include "reloq_frame_adapter.h"
#include "reloq_german_voice_defaults.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static double harmonic(const float *x,size_t count,float rate,double hz)
{
    size_t first=count/4;double real=0,imaginary=0,weight=0;
    for(size_t i=first;i<count;++i) {
        double window=.5-.5*cos(2*pi*(i-first)/(count-first-1));
        double angle=2*pi*hz*(i-first)/rate;
        real+=window*x[i]*cos(angle);imaginary+=window*x[i]*sin(angle);weight+=window;
    }
    return hypot(real,imaginary)/weight;
}
static void body(float rate,unsigned profile,long tilt,double amplitude[4])
{
    float controls[62];uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    assert(!reloq_german_voice_defaults(controls,fields,&config));
    controls[0]=1000;controls[1]=1750;controls[2]=50;
    struct reloq_core_voiced_frame frame;
    assert(!reloq_frame_adapter_map(controls,5,&frame));
    frame.glottal_profile=tilt;frame.noise.ah_level=0;
    frame.noise.af_level=0;frame.noise.bypass_level=0;
    struct reloq_core_voiced_frame_state *s=calloc(1,sizeof *s);assert(s);
    assert(!reloq_core_voiced_frame_initialize(s,rate,8,4,6,0));s->av_trim=-5;
    s->glottal_reference_rate=11025;s->reference_formants=5;s->frication_reference_rate=16000;
    s->rate_transfer.trial_profile=s->noise_transfer.trial_profile=profile;
    size_t capacity=(size_t)rate+16,count=0;
    float *work=calloc(3*capacity,sizeof *work);uint8_t *mask=calloc(capacity,1);assert(work&&mask);
    assert(!reloq_core_voiced_frame_render_owned(s,&frame,work,work+capacity,
        work+2*capacity,mask,NULL,capacity,&count));
    const double hz[]={3500,4025,4375,4900};
    for(unsigned i=0;i<4;++i)amplitude[i]=harmonic(work,count,rate,hz[i]);
    free(work);free(mask);free(s);
}
int main(void)
{
    const float rates[]={16000,22050,32000,44100,48000};
    unsigned checks=0;double worst=0;
    for(unsigned r=0;r<5;++r) {
        struct reloq_rate_transfer s={.trial_profile=1};prepare_colour(&s,rates[r]);
        for(double hz=250;hz<=5500;hz+=125) {
            double complex z=cexp(-I*2*pi*hz/rates[r]);const double *c=s.voice_colour[2];
            double actual=cabs((c[0]+c[1]*z+c[2]*z*z)/(1+c[3]*z+c[4]*z*z));
            double a=sin(pi*hz/rates[r])/(pi*hz/rates[r]);
            double b=sin(pi*hz/11025)/(pi*hz/11025);
            assert(fabs(20*log10(actual/((a/b)*(a/b))))<.33);++checks;
        }
        float breath[2048],unvoiced[2048];
        for(unsigned i=0;i<2048;++i)breath[i]=unvoiced[i]=1;
        reloq_rate_transfer_process_aspiration(&s,breath,2048,1);
        assert(breath[2047]<.8f&&breath[2047]>.794f);
        s.aspiration_voicing=0;
        reloq_rate_transfer_process_aspiration(&s,unvoiced,2048,0);
        for(unsigned i=0;i<2048;++i)assert(unvoiced[i]==1);
        ++checks;
    }
    for(long tilt=0;tilt<=14;tilt+=7) {
        double original[4],current[4];body(11025,0,tilt,original);body(16000,1,tilt,current);
        const double hz[]={3500,4025,4375,4900};
        for(unsigned i=0;i<4;++i) {
            assert(original[i]>0&&current[i]>0);
            /* Periodic-only output keeps 1 dB of extra peak headroom. */
            double residual=fabs(20*log10(current[i]/original[i])+1-10*log10(classic_colour_power(hz[i])));
            if(residual>worst)worst=residual;
            assert(residual<1.6);++checks;
        }
    }
    printf("Classic complete body: %u source/aspiration/reference checks; worst 3.5--4.9 kHz residual %.3f dB\n",checks,worst);
    return 0;
}
