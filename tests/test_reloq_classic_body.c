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
    if(((profile==4||profile==5)&&rate>=16000)||(profile==3&&rate>=22050)) {
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
static void production_paths(void)
{
    const float rates[]={16000,22050,32000,44100,48000};
    unsigned cases=0,noise_differences=0;
    for(unsigned r=0;r<5;++r)for(unsigned reference=4;reference<=5;++reference) {
        float controls[62];uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
        struct reloq_session_config c;
        assert(!reloq_german_voice_defaults(controls,fields,&c));
        c.sample_rate=rates[r];c.glottal_reference_rate=11025;
        c.reference_formants=reference;c.frication_reference_rate=16000;
        struct reloq_session *a=reloq_session_create(&c),*b=reloq_session_create(&c);assert(a&&b);
        assert(!reloq_session_high_rate_profile(b,1));assert(!reloq_session_reset(a));assert(!reloq_session_reset(b));
        assert(!a->frame_state.rate_transfer.trial_profile&&!b->frame_state.rate_transfer.trial_profile);
        assert(!b->frame_state.rate_transfer.body_amount&&b->frame_state.noise_transfer.trial_profile==1);
        long highest=rates[r]>=44100?8:rates[r]>=32000?7:6;
        struct reloq_core_voiced_frame frame;assert(!reloq_frame_adapter_map(controls,highest,&frame));
        frame.noise.tract.nasal_pole=frame.noise.tract.nasal_zero=(struct reloq_core_formant){200,90};
        frame.noise.tract.tracheal_pole=frame.noise.tract.tracheal_zero=(struct reloq_core_formant){250,90};
        for(unsigned j=0;j<8;++j) {
            frame.noise.tract.oral[j]=(struct reloq_core_formant){500+700*j,100+20*j};
            frame.noise.af_bandwidths[j]=500;frame.noise.af_levels[j]=j>=2&&j<=4?55:0;
        }
        frame.noise.duration_ms=2;
        int different=0;
        for(unsigned i=0;i<30;++i) {
            frame.pitch_tenths=1000+37*i;frame.open_percent=50+(i%4)*5;
            frame.av_level=i%7?55:0;frame.noise.ah_level=i%3?25:10;
            frame.noise.af_level=i%4?55:0;frame.noise.s_frication=i%2;
            frame.glottal_profile=4+(long)(i%3);
            frame.noise.tract.oral[1].frequency_hz=1200+13*i;
            float oa[256],ob[256],ra[256],rb[256],sa[256],sb[256],ba[256],bb[256];
            uint8_t ma[256],mb[256];size_t na=0,nb=0;
            assert(!reloq_core_voiced_frame_render_body(&a->frame_state,&frame,oa,ra,sa,ma,ba,256,&na,0));
            assert(!reloq_core_voiced_frame_render_body(&b->frame_state,&frame,ob,rb,sb,mb,bb,256,&nb,0));
            assert(na==nb&&!memcmp(ba,bb,na*sizeof *ba));
            if(frame.noise.af_level) {
                float trim=rates[r]>=32000?(frame.noise.s_frication?1.35f:
                    rates[r]==32000?.25f:0):0;
                assert(b->frame_state.noise_transfer.frication_trim_db==trim);
            }
            if(memcmp(oa,ob,na*sizeof *oa))different=1;
            ++cases;
        }
        assert(different);++noise_differences;
        reloq_session_destroy(a);reloq_session_destroy(b);
    }
    printf("A voice / Classic noise: %u evolving frame bodies bit-identical; %u independent noise paths differ\n",cases,noise_differences);
}
int main(void)
{
    production_paths();
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
    double difference=0,production_hnr=1000;
    for(unsigned vowel=0;vowel<3;++vowel)for(unsigned pitch=0;pitch<3;++pitch) {
        double f0=113+64*pitch;
        for(unsigned r=0;r<5;++r) {
            struct observation o=body(rates[r],5,vowel,f0);
            struct observation a=body(rates[r],4,vowel,f0);
            double bands[]={fabs(10*log10(o.bass/a.bass)),fabs(10*log10(o.middle/a.middle)),
                fabs(10*log10(o.upper/a.upper))};
            for(unsigned b=0;b<3;++b){assert(bands[b]<.0001);if(bands[b]>difference)difference=bands[b];}
            if(o.hnr<production_hnr)production_hnr=o.hnr;
        }
    }
    printf("Production A voice: 45 vowel/pitch/rate cases; A/B band difference <=%.6f dB; harmonic/noise >=%.2f dB\n",difference,production_hnr);
    return 0;
}
