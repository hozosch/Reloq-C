#include "reloq_target_stage_router.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fixture {
    struct reloq_target_builder builder;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][16];
};

static void begin(struct fixture *f)
{
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], 16, 0};
    assert(reloq_target_builder_begin(&f->builder, buffers) == 0);
}

static void match(const struct fixture *left, const struct fixture *right)
{
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        const struct reloq_target_buffer *a = &left->builder.stream[s];
        const struct reloq_target_buffer *b = &right->builder.stream[s];
        assert(a->count == b->count);
        for(size_t i=0;i<a->count;++i)
            assert(a->points[i].tick==b->points[i].tick&&a->points[i].value==b->points[i].value);
    }
}

static void growing_storage(void)
{
    struct reloq_target_builder direct={0},destination={0};
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT]={{0}};
    struct reloq_acoustic_point *memory=calloc(RELOQ_TARGET_STREAM_COUNT*512,sizeof *memory);
    assert(memory);
    for(size_t s=1;s<RELOQ_TARGET_STREAM_COUNT;++s)
        buffers[s]=(struct reloq_target_buffer){memory+s*512,512,0};
    assert(!reloq_target_builder_begin(&direct,buffers));
    struct reloq_target_stage_router router={0};
    assert(!reloq_target_stage_router_begin_growing(&router,&destination));
    for(size_t i=0;i<384;++i) {
        struct reloq_target_event events[RELOQ_TARGET_STREAM_COUNT-1];
        for(size_t s=1;s<RELOQ_TARGET_STREAM_COUNT;++s)
            events[s-1]=(struct reloq_target_event){(enum reloq_target_stream)s,0,(int)(s+i%9)};
        assert(!reloq_target_builder_append(&direct,(int64_t)i/2,events,RELOQ_TARGET_STREAM_COUNT-1));
        assert(!reloq_target_stage_router_append(&router,(int64_t)i/2,events,RELOQ_TARGET_STREAM_COUNT-1));
    }
    /* Reject an entire invalid batch before allocating or changing counts. */
    struct reloq_target_stage_router before=router;
    struct reloq_target_event invalid[]={{RELOQ_TARGET_F1,300,1},{RELOQ_TARGET_AV,-1,2}};
    assert(reloq_target_stage_router_append(&router,0,invalid,2)==-1);
    assert(!memcmp(&before,&router,sizeof router));
    assert(!reloq_target_stage_router_merge(&router,&destination));
    reloq_target_stage_router_dispose(&router);
    for(size_t s=1;s<RELOQ_TARGET_STREAM_COUNT;++s) {
        assert(destination.stream[s].count==direct.stream[s].count);
        for(size_t i=0;i<direct.stream[s].count;++i)
            assert(destination.stream[s].points[i].tick==direct.stream[s].points[i].tick&&
                destination.stream[s].points[i].value==direct.stream[s].points[i].value);
        free(destination.stream[s].points);destination.stream[s]=(struct reloq_target_buffer){0};
    }
    /* An abandoned plan keeps ownership private until disposal. */
    assert(!reloq_target_stage_router_begin_growing(&router,&destination));
    assert(!reloq_target_stage_router_append(&router,300,invalid,1));
    reloq_target_stage_router_dispose(&router);
    assert(!destination.stream[RELOQ_TARGET_F1].points);
    free(memory);
}

int main(void)
{
    growing_storage();
    struct fixture direct, composed;
    begin(&direct);
    begin(&composed);
    struct reloq_target_stage_router router = {0};
    assert(reloq_target_stage_router_begin(&router,
                                           &composed.builder) == 0);
    const struct reloq_target_event at_zero[] = {
        {RELOQ_TARGET_F1, 0, 500},
        {RELOQ_TARGET_F2, 0, 1500},
        {RELOQ_TARGET_AV, 0, 0},
        {RELOQ_TARGET_AH, 0, 0},
        {RELOQ_TARGET_AF, 0, 0},
        {RELOQ_TARGET_A2F, 0, 0},
        {RELOQ_TARGET_OQ, 0, 56}
    };
    assert(reloq_target_builder_append(&direct.builder, 0, at_zero,
                                       sizeof at_zero / sizeof at_zero[0]) == 0);
    assert(reloq_target_stage_router_append(&router, 0, at_zero,
                                            sizeof at_zero / sizeof at_zero[0]) == 0);

    /* Different gestures can overlap without forcing every source and
     * formant to acquire the other's landmarks. The stage append order is
     * globally nonchronological, but each owner's streams remain ordered. */
    const struct reloq_target_event frication[] = {
        {RELOQ_TARGET_AF, 0, 0},
        {RELOQ_TARGET_AF, 0, 62},
        {RELOQ_TARGET_A2F, 0, 54},
        {RELOQ_TARGET_AF, 10, 62},
        {RELOQ_TARGET_AF, 10, 0}
    };
    const struct reloq_target_event tract[] = {
        {RELOQ_TARGET_F1, 0, 530},
        {RELOQ_TARGET_F2, 0, 1600},
        {RELOQ_TARGET_F1, 19, 680},
        {RELOQ_TARGET_F2, 19, 1120}
    };
    const struct reloq_target_event voice[] = {
        {RELOQ_TARGET_AV, 0, 52},
        {RELOQ_TARGET_AH, 3, 49},
        {RELOQ_TARGET_AV, 20, 50},
        {RELOQ_TARGET_AH, 20, 0}
    };
    assert(reloq_target_stage_router_append(&router, 12, frication,
            sizeof frication / sizeof frication[0]) == 0);
    assert(reloq_target_stage_router_append(&router, 8, tract,
            sizeof tract / sizeof tract[0]) == 0);
    assert(reloq_target_stage_router_append(&router, 10, voice,
            sizeof voice / sizeof voice[0]) == 0);
    assert(reloq_target_builder_append(&direct.builder, 12, frication,
            sizeof frication / sizeof frication[0]) == 0);
    assert(reloq_target_builder_append(&direct.builder, 8, tract,
            sizeof tract / sizeof tract[0]) == 0);
    assert(reloq_target_builder_append(&direct.builder, 10, voice,
            sizeof voice / sizeof voice[0]) == 0);

    assert(composed.builder.stream[RELOQ_TARGET_F1].count == 0);
    assert(router.stage[RELOQ_STAGE_TRACT]
                 .stream[RELOQ_TARGET_F1].count == 3);
    assert(router.stage[RELOQ_STAGE_TRACT]
                 .stream[RELOQ_TARGET_AF].count == 0);
    assert(router.stage[RELOQ_STAGE_VOICED]
                 .stream[RELOQ_TARGET_AV].count == 3);
    assert(router.stage[RELOQ_STAGE_NOISE]
                 .stream[RELOQ_TARGET_AF].count == 5);
    assert(reloq_target_stage_router_merge(&router, &composed.builder) == 0);
    match(&direct, &composed);
    assert(reloq_target_stage_router_merge(&router, &composed.builder) == -1);
    assert(reloq_target_stage_router_append(&router, 30, voice, 1) == -1);

    const struct reloq_target_event melody = {RELOQ_TARGET_F0, 0, 1100};
    assert(reloq_target_builder_append(&direct.builder, 0,
                                       &melody, 1) == 0);
    assert(reloq_target_builder_append(&composed.builder, 0,
                                       &melody, 1) == 0);
    match(&direct, &composed);
    reloq_target_stage_router_dispose(&router);

    /* An invalid F0 event and an overflowing private stage do not affect
     * the destination. The phrase planner can discard its private state. */
    begin(&composed);
    assert(reloq_target_stage_router_begin(&router,
                                           &composed.builder) == 0);
    assert(reloq_target_stage_router_append(&router, 0,
                                            &melody, 1) == -1);
    struct reloq_target_event too_many[17];
    for (size_t i = 0; i < 17; ++i)
        too_many[i] = (struct reloq_target_event){
            RELOQ_TARGET_AV, (int64_t)i, 45};
    assert(reloq_target_stage_router_append(&router, 0,
                                            too_many, 17) == -1);
    assert(composed.builder.stream[RELOQ_TARGET_AV].count == 0);
    reloq_target_stage_router_dispose(&router);

    /* A single gesture may schedule independent tract, voice and noise
     * windows. Compare the merged result to ordinary per-stream appends.
     * Equal-time AF steps must retain their insertion order. */
    begin(&direct);
    begin(&composed);
    assert(reloq_target_stage_router_begin(&router,
                                           &composed.builder) == 0);
    const int64_t stage_origins[RELOQ_STAGE_ROLE_COUNT] = {
        5, 9, 14, 3};
    const struct reloq_target_event one_transition[] = {
        {RELOQ_TARGET_F1, 0, 400},
        {RELOQ_TARGET_AV, 0, 45},
        {RELOQ_TARGET_AF, 0, 0},
        {RELOQ_TARGET_AF, 0, 58},
        {RELOQ_TARGET_OQ, 0, 18},
        {RELOQ_TARGET_F1, 30, 710},
        {RELOQ_TARGET_AV, 25, 52},
        {RELOQ_TARGET_AF, 10, 58},
        {RELOQ_TARGET_AF, 10, 0},
        {RELOQ_TARGET_OQ, 25, 56}
    };
    assert(reloq_target_stage_router_append_at(
        &router, stage_origins, one_transition,
        sizeof one_transition / sizeof one_transition[0]) == 0);
    for (size_t i = 0; i < sizeof one_transition / sizeof one_transition[0];
         ++i) {
        enum reloq_target_stream stream = one_transition[i].stream;
        size_t role = stream == RELOQ_TARGET_F1 ? RELOQ_STAGE_TRACT :
                      stream == RELOQ_TARGET_AV ? RELOQ_STAGE_VOICED :
                      stream == RELOQ_TARGET_AF ? RELOQ_STAGE_NOISE :
                      RELOQ_STAGE_REMAINDER;
        assert(reloq_target_builder_append(&direct.builder,
                   stage_origins[role], &one_transition[i], 1) == 0);
    }
    assert(reloq_target_stage_router_merge(&router, &composed.builder) == 0);
    match(&direct, &composed);
    assert(composed.builder.stream[RELOQ_TARGET_AF].points[1].tick == 14 &&
           composed.builder.stream[RELOQ_TARGET_AF].points[1].value == 58 &&
           composed.builder.stream[RELOQ_TARGET_AV].points[0].tick == 9);
    reloq_target_stage_router_dispose(&router);

    /* With all origins equal, append_at and the legacy append produce
     * precisely the same points; negative offsets remain valid. */
    begin(&direct);
    begin(&composed);
    assert(reloq_target_stage_router_begin(&router,
                                           &composed.builder) == 0);
    const int64_t equal_origins[RELOQ_STAGE_ROLE_COUNT] = {
        20, 20, 20, 20};
    const struct reloq_target_event anticipatory[] = {
        {RELOQ_TARGET_F2, -4, 1750},
        {RELOQ_TARGET_AV, -2, 0},
        {RELOQ_TARGET_AF, 0, 50},
        {RELOQ_TARGET_OQ, 1, 56}
    };
    assert(reloq_target_stage_router_append_at(
        &router, equal_origins, anticipatory,
        sizeof anticipatory / sizeof anticipatory[0]) == 0);
    assert(reloq_target_builder_append(
        &direct.builder, 20, anticipatory,
        sizeof anticipatory / sizeof anticipatory[0]) == 0);
    assert(reloq_target_stage_router_merge(&router, &composed.builder) == 0);
    match(&direct, &composed);
    reloq_target_stage_router_dispose(&router);

    /* Errors are atomic across distinct stage origins, including a later
     * event that overflows while an earlier stage has enough capacity. */
    begin(&composed);
    assert(reloq_target_stage_router_begin(&router,
                                           &composed.builder) == 0);
    const int64_t overflowing[RELOQ_STAGE_ROLE_COUNT] = {
        10, 10, INT64_MAX, 10};
    const struct reloq_target_event invalid[] = {
        {RELOQ_TARGET_F1, 0, 500},
        {RELOQ_TARGET_AV, 0, 50},
        {RELOQ_TARGET_AF, 1, 65}
    };
    assert(reloq_target_stage_router_append_at(
        &router, overflowing, invalid,
        sizeof invalid / sizeof invalid[0]) == -1);
    assert(router.stage[RELOQ_STAGE_TRACT]
                 .stream[RELOQ_TARGET_F1].count == 0 &&
           router.stage[RELOQ_STAGE_VOICED]
                 .stream[RELOQ_TARGET_AV].count == 0 &&
           router.stage[RELOQ_STAGE_NOISE]
                 .stream[RELOQ_TARGET_AF].count == 0);
    assert(reloq_target_stage_router_append_at(
        &router, NULL, invalid, 1) == -1);
    assert(reloq_target_stage_router_append_at(
        &router, equal_origins, &melody, 1) == -1);
    reloq_target_stage_router_dispose(&router);
    puts("independent target stages: OK");
    return 0;
}
