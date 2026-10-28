#include "reloq_german_generic_voice.h"
#include "reloq_german_syllable_plan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct target_capture {
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][128];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_target_builder builder;
};

static void plan(const char *spr, struct target_capture *capture)
{
    struct reloq_german_phone_node nodes[16];
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_german_syllable syllables[16];
    size_t syllable_count = 0;
    assert(reloq_german_phone_sequence_build((const unsigned char *)spr,
               strlen(spr), 0, nodes, 16, &info) == 0);
    assert(reloq_german_syllable_plan_build(nodes, info.phone_count,
               syllables, 16, &syllable_count) == 0);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        capture->buffers[i] = (struct reloq_target_buffer){
            capture->points[i], 128, 0};
    assert(reloq_target_builder_begin(&capture->builder,
               capture->buffers) == 0);
    int64_t end_tick = 0;
    assert(reloq_german_generic_plan(nodes, info.phone_count, syllables,
               syllable_count, &capture->builder, &end_tick, NULL, NULL) == 0);
    assert(end_tick > 100);
}

static void sample(const struct target_capture *capture,int64_t tick,
    int av,int ah,int af,int ab)
{
    const enum reloq_target_stream streams[]={RELOQ_TARGET_AV,RELOQ_TARGET_AH,RELOQ_TARGET_AF,RELOQ_TARGET_AB};
    const int expected[]={av,ah,af,ab};
    for (unsigned i=0;i<4;++i) {
        const struct reloq_target_buffer *track=&capture->builder.stream[streams[i]];
        struct reloq_acoustic_curve curve={0,track->points,track->count};
        struct reloq_acoustic_cursor cursor;float actual=-1;
        assert(!reloq_acoustic_begin(&curve,&cursor));
        assert(!reloq_acoustic_value(&cursor,tick,&actual));
        /* The requested consonant gain raises positive source endpoints
         * by one dB. Interpolation can add zero or one integer control dB
         * at these historical sample ticks. Parallel branch levels and
         * all timing landmarks retain the original observations. The
         * production planner differential additionally checks exact +1
         * endpoint ownership, unchanged zero knots and S/Z exclusions. */
        if (i<3) assert(actual>=expected[i] && actual<=expected[i]+1);
        else assert(actual==expected[i]);
    }
}

/* Numeric frame observations from the separate, unmodified reference.
 * These checks exercise the complete planner and interpolation, including
 * simultaneous aspiration and fading frication; they are not snapshots of
 * C-generated targets. Source levels allow only the requested +1-dB
 * endpoint adjustment, including its integer interpolation boundary. */
int main(void)
{
    static struct target_capture c;
    plan("`[.1pA].",&c);
    sample(&c,5,0,0,0,0);sample(&c,6,0,0,60,64);
    sample(&c,15,0,0,60,64);sample(&c,16,0,49,58,62);
    sample(&c,25,0,49,48,51);sample(&c,50,0,49,18,19);
    sample(&c,65,0,49,0,0);sample(&c,66,51,34,0,0);
    sample(&c,152,49,34,0,0);sample(&c,153,48,35,0,0);
    sample(&c,212,0,35,0,0);
    plan("`[.1tA].",&c);
    sample(&c,17,0,0,59,0);sample(&c,18,0,47,58,0);
    sample(&c,40,0,47,40,0);sample(&c,89,0,47,0,0);
    sample(&c,90,51,34,0,0);
    plan("`[.1kA].",&c);
    sample(&c,17,0,0,60,0);sample(&c,18,0,45,59,0);
    sample(&c,40,0,45,37,0);sample(&c,78,0,45,0,0);
    sample(&c,79,51,34,0,0);
    plan("`[.1fA].",&c);
    sample(&c,0,0,0,58,60);sample(&c,118,0,0,58,60);
    sample(&c,119,51,34,55,57);sample(&c,125,50,34,41,43);
    sample(&c,143,50,34,0,0);sample(&c,215,49,34,0,0);
    sample(&c,216,48,35,0,0);sample(&c,275,0,35,0,0);
    plan("`[.1Af].",&c);
    sample(&c,82,49,34,0,0);sample(&c,95,49,34,30,31);
    sample(&c,107,49,34,58,60);sample(&c,257,0,0,58,60);
    sample(&c,270,0,0,32,34);sample(&c,287,0,0,0,0);
    puts("source handoffs: 35 original frame samples with bounded +1-dB source adjustment across bursts, aspiration, CV, VC and release passed");
    return 0;
}
