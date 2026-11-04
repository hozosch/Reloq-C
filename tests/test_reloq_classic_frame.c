/* The explicit Classic voice mode retains the reference tract. */
#include "reloq_core.h"
#include "../src/reloq_core_internal.h"
#include "reloq_frame_adapter.h"
#include "reloq_german_voice_defaults.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t render(float rate,unsigned reference,unsigned profile,long highest,int16_t pcm[4096])
{
    float controls[62];uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    assert(!reloq_german_voice_defaults(controls,fields,&config));
    controls[0]=40;controls[1]=1000;controls[2]=60;controls[7]=25;
    struct reloq_core_voiced_frame frame;
    assert(!reloq_frame_adapter_map(controls,highest,&frame));
    struct reloq_core_voiced_frame_state *state=calloc(1,sizeof *state);assert(state);
    assert(!reloq_core_voiced_frame_initialize(state,rate,8,4,6,0));
    state->glottal_reference_rate=11025;state->frication_reference_rate=16000;
    state->reference_formants=reference;state->rate_transfer.trial_profile=profile;
    state->noise_transfer.trial_profile=profile;
    float *work=calloc(3*4096,sizeof *work);uint8_t *mask=calloc(4096,1);assert(work&&mask);
    size_t written=0;
    assert(!reloq_core_voiced_frame_render_owned(state,&frame,work,work+4096,
        work+8192,mask,pcm,4096,&written));assert(written);
    free(work);free(mask);free(state);return written;
}
int main(void)
{
    const float rates[]={16000,22050,32000,44100,48000};unsigned checks=0;
    for(unsigned r=0;r<5;++r)for(unsigned reference=4;reference<=5;++reference)
        for(unsigned profile=0;profile<2;++profile) {
            int16_t extended[4096],base[4096];
            size_t a=render(rates[r],reference,profile,8,extended);
            size_t b=render(rates[r],reference,profile,(long)reference,base);
            assert(a==b);
            if(profile)assert(!memcmp(extended,base,a*sizeof *base));
            else assert(memcmp(extended,base,a*sizeof *base));
            ++checks;
        }
    printf("Classic renderer: %u AV/AH tract and A-control PCM comparisons\n",checks);
    return 0;
}
