/* Exercise the production PCM converter across real frame boundaries.
 * The input ramps into a near-full-scale periodic onset, as in short vowel
 * initial announcements. No audio device or synthesized result is cached. */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/reloq_session.c"

static double onset(struct reloq_session *session,int hz,int16_t *output)
{
    const int block=hz/200,total=hz/20;
    double greatest_step=0,previous_gain=0;
    int previous_valid=0,peak=0;
    for(int start=0;start<total;start+=block) {
        int count=total-start<block?total-start:block;
        for(int i=0;i<count;++i) {
            double t=1000.0*(start+i)/hz;
            double level=t<7?6000+28000*t/7:34000;
            session->buffers.output[i]=(float)(level*sin(2*3.141592653589793*160*(start+i)/hz));
        }
        assert(output_pcm(session,(size_t)count)==RELOQ_SESSION_OK);
        for(int i=0;i<count;++i) {
            double input=session->buffers.output[i];
            int sample=session->buffers.pcm[i];output[start+i]=(int16_t)sample;
            if(abs(sample)>peak)peak=abs(sample);
            int valid=fabs(input)>12000;
            double gain=valid?sample/input:0;
            if(valid&&previous_valid&&fabs(gain-previous_gain)>greatest_step)
                greatest_step=fabs(gain-previous_gain);
            previous_gain=gain;previous_valid=valid;
        }
    }
    assert(peak>22000&&peak<28001);
    return greatest_step;
}

int main(void)
{
    const int rates[]={22050,32000,44100,48000};
    for(size_t r=0;r<sizeof rates/sizeof *rates;++r) {
        int hz=rates[r],block=hz/200,total=hz/20;
        struct reloq_session_config config={.sample_rate=(float)hz,.duration_scale=1,
            .output_gain=2066.394f/2768.533f,.max_frame_samples=(size_t)block};
        struct reloq_session *session=reloq_session_create(&config);assert(session);
        assert(reloq_session_text_loudness(session,1)==RELOQ_SESSION_OK);
        assert(reserve_buffers(&session->buffers,(size_t)block)==RELOQ_SESSION_OK);
        int16_t *first=malloc((size_t)total*sizeof *first),*after=malloc((size_t)total*sizeof *after);
        assert(first&&after);
        double step=onset(session,hz,first);
        assert(step<.012);
        for(int left=hz*30/1000;left>0;) {
            int n=left<block?left:block;
            memset(session->buffers.output,0,(size_t)n*sizeof *session->buffers.output);
            assert(output_pcm(session,(size_t)n)==RELOQ_SESSION_OK);left-=n;
        }
        assert(onset(session,hz,after)<.012);
        assert(!memcmp(first,after,(size_t)total*sizeof *first));
        printf("%d Hz: peak stays bounded and onset gain moves smoothly (max %.5f)\n",hz,step);
        free(first);free(after);reloq_session_destroy(session);
    }
    return 0;
}
