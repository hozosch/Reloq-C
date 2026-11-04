/* Test the periodic/noise separation through actual session PCM. */
#include "reloq_session.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
struct capture {int16_t samples[5000];size_t count;};
static int take(void *context,const int16_t *samples,size_t count)
{
    struct capture *out=context;
    assert(count<=5000-out->count);
    memcpy(out->samples+out->count,samples,count*sizeof *samples);
    out->count+=count;return 0;
}
static void render(float rate,unsigned profile,int voiced,int aspiration,int broad,
    struct capture *out)
{
    struct reloq_session_config config={.sample_rate=rate,.duration_scale=1,
        .output_gain=1,.initial_seed=8,.max_frame_samples=5000,
        .glottal_reference_rate=11025,.reference_formants=5,.frication_reference_rate=16000};
    struct reloq_session *session=reloq_session_create(&config);assert(session);
    assert(!reloq_session_high_rate_profile(session,profile));assert(!reloq_session_reset(session));
    struct reloq_core_voiced_frame frame={.pitch_tenths=1153,.open_percent=50,
        .av_level=voiced?55:0,.glottal_profile=4};
    frame.noise.duration_ms=100;
    frame.noise.ah_level=aspiration?50:0;
    frame.noise.af_level=!aspiration&&!voiced?55:0;
    frame.noise.bypass_level=broad?50:0;
    frame.noise.s_frication=!broad;
    frame.noise.tract.highest_formant=5;
    frame.noise.tract.nasal_pole=frame.noise.tract.nasal_zero=(struct reloq_core_formant){200,90};
    frame.noise.tract.tracheal_pole=frame.noise.tract.tracheal_zero=(struct reloq_core_formant){250,90};
    for(unsigned i=0;i<8;++i) {
        frame.noise.tract.oral[i]=(struct reloq_core_formant){500+700*(float)i,100+20*(float)i};
        frame.noise.af_bandwidths[i]=500;
    }
    if(!broad){frame.noise.af_levels[3]=50;frame.noise.af_levels[4]=55;}
    assert(!reloq_session_render_frames(session,&frame,1,take,out,NULL));
    int audible=0;for(size_t i=0;i<out->count;++i)audible|=out->samples[i]!=0;
    assert(out->count&&audible);reloq_session_destroy(session);
}
int main(void)
{
    const float rates[]={8000,11025,16000,22050,32000,44100,48000};unsigned checks=0;
    for(unsigned r=0;r<7;++r) {
        for(int aspiration=0;aspiration<2;++aspiration) {
            struct capture b={0},c={0};
            render(rates[r],1,0,aspiration,0,&b);render(rates[r],3,0,aspiration,0,&c);
            assert(b.count==c.count&&!memcmp(b.samples,c.samples,b.count*sizeof *b.samples));++checks;
        }
        struct capture b={0},c={0};render(rates[r],1,0,0,1,&b);render(rates[r],3,0,0,1,&c);
        assert(b.count==c.count&&!memcmp(b.samples,c.samples,b.count*sizeof *b.samples));++checks;
        memset(&b,0,sizeof b);memset(&c,0,sizeof c);
        render(rates[r],1,1,0,0,&b);render(rates[r],3,1,0,0,&c);
        assert(b.count==c.count);
        assert((memcmp(b.samples,c.samples,b.count*sizeof *b.samples)!=0)==(rates[r]>16000));++checks;
    }
    printf("Gentle richness separation: %u PCM comparisons, AF/AH unchanged and 16 kHz preserved\n",checks);
    return 0;
}
