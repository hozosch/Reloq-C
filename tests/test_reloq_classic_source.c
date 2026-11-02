/* Source anti-aliasing must preserve phase, masks and chunk boundaries.
 * Source-filter expansion must preserve the physical poles and DC level;
 * richness now belongs to the independently tested A-body blend. */
#include "../src/reloq_core.c"
#include <assert.h>
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static double gain(const struct reloq_core_resonator *s,double hz,double rate)
{
    double complex z=cexp(-I*6.283185307179586*hz/rate);
    return cabs(s->input_gain/(1-s->latest_gain*z-s->previous_gain*z*z));
}
int main(void)
{
    struct reloq_core_voiced_state dirty;
    memset(&dirty,0xa5,sizeof dirty);
    assert(!reloq_core_voiced_configure(&dirty,48000,1370,85,55));
    assert(!dirty.bandlimited_closure&&!dirty.closure_pre&&
        !dirty.closure_tail[0]&&!dirty.closure_tail[1]&&!dirty.closure_cursor);
    const float rates[]={16000,22050,32000,44100,48000};
    const float pitches[]={1370,2330,4300,8800};
    const float open[]={0,45,70,85,100};unsigned partition=0,tilt=0;
    enum {N=10003};
    float *whole=malloc(N*sizeof *whole),*split=malloc(N*sizeof *split),*legacy=malloc(N*sizeof *legacy);
    uint8_t *mask=malloc(N),*other=malloc(N);assert(whole&&split&&legacy&&mask&&other);
    for(unsigned r=0;r<5;++r)for(unsigned p=0;p<4;++p)for(unsigned o=0;o<5;++o) {
        struct reloq_core_voiced_state a={0};
        assert(!reloq_core_voiced_configure(&a,rates[r],pitches[p],open[o],55));
        struct reloq_core_voiced_state old=a;a.bandlimited_closure=1;
        struct reloq_core_voiced_state b=a;
        assert(!reloq_core_voiced_fill_with_mask(&old,legacy,other,N));
        assert(!reloq_core_voiced_fill_with_mask(&a,whole,mask,N));
        assert(!memcmp(mask,other,N));
        assert(a.next_phase_ms==old.next_phase_ms&&a.open_left==old.open_left&&a.closed_left==old.closed_left);
        for(size_t at=0;at<N;) {
            size_t n=1+(at*13)%199;if(n>N-at)n=N-at;
            assert(!reloq_core_voiced_fill_with_mask(&b,split+at,other+at,n));at+=n;
        }
        assert(!memcmp(whole,split,N*sizeof *whole)&&!memcmp(mask,other,N));
        for(size_t i=0;i<N;++i)assert(isfinite(whole[i]));
        ++partition;
    }
    for(unsigned r=0;r<5;++r)for(long profile=1;profile<=35;++profile) {
        struct reloq_core_clock clock=reloq_core_clock_for_rate(rates[r]);
        struct reloq_core_resonator a={0},b={0};
        struct reloq_core_clock reference=reloq_core_clock_for_rate(11025);
        assert(!reloq_core_glottal_prefilter_configure(&a,&reference,profile));
        assert(!source_profile(&b,&clock,profile,11025));
        double dc=gain(&a,0,11025);
        assert(fabs(gain(&b,0,rates[r])/dc-1)<2e-4);
        double radius_a=sqrt(-a.previous_gain),radius_b=sqrt(-b.previous_gain);
        double decay_a=log(radius_a)*11025,decay_b=log(radius_b)*rates[r];
        assert(fabs(decay_a-decay_b)<.05);
        double ca=fmax(-1,fmin(1,a.latest_gain/(2*radius_a)));
        double cb=fmax(-1,fmin(1,b.latest_gain/(2*radius_b)));
        assert(fabs(acos(ca)*11025-acos(cb)*rates[r])<3.2);
        assert(b.previous_gain>-1&&b.previous_gain<=0);
        ++tilt;
    }
    free(whole);free(split);free(legacy);free(mask);free(other);
    printf("Classic source: %u phase/mask/chunk cases, %u DC/physical-pole/stability cases\n",partition,tilt);
    return 0;
}
