/* Source anti-aliasing must preserve phase, masks and chunk boundaries.
 * Glottal tilt must open harmonics without moving the DC reference. */
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
        assert(!source_profile(&a,&clock,profile,11025));
        assert(!classic_source_profile(&b,&clock,profile,11025,rates[r]));
        assert(fabs(gain(&a,0,rates[r])-gain(&b,0,rates[r]))<2e-5);
        assert(b.previous_gain>-1&&b.previous_gain<=0);
        if(r==0)assert(!memcmp(&a,&b,sizeof a));
        else if(profile>4)assert(gain(&b,3500,rates[r])>gain(&a,3500,rates[r]));
        ++tilt;
    }
    free(whole);free(split);free(legacy);free(mask);free(other);
    printf("Classic source: %u phase/mask/chunk cases, %u DC/tilt/stability cases\n",partition,tilt);
    return 0;
}
