/* Source anti-aliasing preserves phase, masks and chunk boundaries.
 * Moving poles must not create energy in an unforced resonator. */
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
static unsigned coupled_cases(void)
{
    unsigned checked=0;
    for(unsigned rate=16000;rate<=48000;rate+=16000) {
        struct reloq_core_clock clock=reloq_core_clock_for_rate((float)rate);
        for(unsigned frequency=100;frequency<=7000;frequency+=2300) {
            struct reloq_core_resonator a={.stable=1},b;
            assert(!reloq_core_resonator_set_from_hz(&a,&clock,(float)frequency,80,0));
            a.enabled=1;b=a;
            float whole[4096]={1},split[4096]={1};
            assert(!reloq_core_resonator_process(&a,whole,4096));
            for(long at=0;at<4096;) {
                long n=1+at%71;if(n>4096-at)n=4096-at;
                assert(!reloq_core_resonator_process(&b,split+at,n));at+=n;
            }
            assert(!memcmp(whole,split,sizeof whole));
            double previous=0,latest=0;
            for(unsigned i=0;i<4096;++i) {
                double y=(i?0:a.input_gain)+a.latest_gain*latest+a.previous_gain*previous;
                assert(fabs(whole[i]-y)<5e-7*(1+fabs(y)));
                previous=latest;latest=y;
            }
            ++checked;
        }
        struct reloq_core_resonator moving={.stable=1,.enabled=1,
            .coupled_real=1,.coupled_imaginary=.5};
        for(unsigned i=0;i<1000;++i) {
            double before=moving.coupled_real*moving.coupled_real+
                moving.coupled_imaginary*moving.coupled_imaginary;
            assert(!reloq_core_resonator_set_from_hz(&moving,&clock,
                i%2?100:7000,80,i%3==0));
            float sample=0;
            assert(!reloq_core_resonator_process(&moving,&sample,1));
            double after=moving.coupled_real*moving.coupled_real+
                moving.coupled_imaginary*moving.coupled_imaginary;
            assert(isfinite(sample)&&after<=before*(1+1e-12));++checked;
        }
    }
    return checked;
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
        assert(!source_profile(&b,&clock,profile,11025,0));
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
        struct reloq_core_resonator wide={0};
        assert(!source_profile(&wide,&clock,profile,11025,1));
        assert(fabs(gain(&wide,0,rates[r])/dc-1)<2e-4);
        assert(wide.previous_gain>-1&&wide.previous_gain<0);
        if(!r)assert(fabs(gain(&wide,1000,rates[r])/gain(&b,1000,rates[r])-1)<1e-5);
        else {
            double rw=sqrt(-wide.previous_gain);
            double cw=fmax(-1,fmin(1,wide.latest_gain/(2*rw)));
            assert(acos(cw)*rates[r]>=acos(ca)*11025-3.2);
            assert(log(rw)*rates[r]<=decay_a+.05);
            if(r==1) {
                struct reloq_core_resonator eight={0};
                struct reloq_core_clock clock8=reloq_core_clock_for_rate(8000);
                assert(!reloq_core_glottal_prefilter_configure(&eight,&clock8,profile));
                double r8=sqrt(-eight.previous_gain);
                double c8=fmax(-1,fmin(1,eight.latest_gain/(2*r8)));
                /* 16 -> 22.05 has exactly the 8 -> 11.025 clock ratio:
                 * it must repeat that measured physical pole expansion. */
                double f8=acos(c8)*8000,f11=acos(ca)*11025,fw=acos(cw)*rates[r];
                assert(fabs(fw*f8/(f11*f11)-1)<.005);
                assert(fabs((log(rw)*rates[r])*(log(r8)*8000)/(decay_a*decay_a)-1)<.0001);
            }
        }
        ++tilt;
    }
    free(whole);free(split);free(legacy);free(mask);free(other);
    printf("Classic source: %u phase/mask/chunk cases, %u DC/pole cases, %u coupled-state equivalence/decay cases\n",partition,tilt,coupled_cases());
    return 0;
}
