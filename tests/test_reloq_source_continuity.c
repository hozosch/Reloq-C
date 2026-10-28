/* Constant excitation and a fixed tract must not depend on frame partition. */
#include "reloq_session.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
struct capture {int16_t samples[5000];size_t count;};
static int take(void *context,const int16_t *samples,size_t count)
{
    struct capture *out=context;
    if(count>5000-out->count)return 1;
    memcpy(out->samples+out->count,samples,count*sizeof *samples);
    out->count+=count;return 0;
}
static int render(float rate,float pitch,long tilt,int step,struct capture *out)
{
    struct reloq_session_config config={.sample_rate=rate,.duration_scale=1,
        .output_gain=1,.initial_seed=8,.max_frame_samples=512};
    struct reloq_session *session=reloq_session_create(&config);
    if(!session)return 1;
    struct reloq_core_voiced_frame frame={.pitch_tenths=pitch,.open_percent=85,
        .av_level=55,.glottal_profile=tilt};
    frame.noise.duration_ms=(float)step;
    frame.noise.tract.highest_formant=4;
    frame.noise.tract.nasal_pole=(struct reloq_core_formant){200,90};
    frame.noise.tract.nasal_zero=(struct reloq_core_formant){200,90};
    frame.noise.tract.tracheal_pole=(struct reloq_core_formant){250,90};
    frame.noise.tract.tracheal_zero=(struct reloq_core_formant){250,90};
    for(unsigned i=0;i<8;++i){
        frame.noise.tract.oral[i]=(struct reloq_core_formant){500+700*(float)i,100+20*(float)i};
        frame.noise.af_bandwidths[i]=200;
    }
    int error=0;
    for(int t=0;t<100&&!error;t+=step)
        error=reloq_session_render_frames(session,&frame,1,take,out,NULL);
    reloq_session_destroy(session);return error;
}
int main(void)
{
    const float rates[]={8000,11025,48000},pitches[]={1153,3000,4500};
    const long tilts[]={0,1,19,35};unsigned cases=0;
    for(unsigned r=0;r<3;++r)for(unsigned p=0;p<3;++p)for(unsigned t=0;t<4;++t){
        struct capture short_frames={0},long_frames={0};
        if(render(rates[r],pitches[p],tilts[t],1,&short_frames)||
           render(rates[r],pitches[p],tilts[t],5,&long_frames))return 1;
        if(!short_frames.count||short_frames.count!=long_frames.count||
           memcmp(short_frames.samples,long_frames.samples,short_frames.count*sizeof(int16_t))){
            fprintf(stderr,"source frame partition differs: rate %.0f, pitch %.0f, tilt %ld\n",
                (double)rates[r],(double)pitches[p],tilts[t]);return 2;
        }
        ++cases;
    }
    printf("Source continuity: %u frame-partition comparisons passed\n",cases);return 0;
}
