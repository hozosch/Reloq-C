#include "reloq_german_reduced_closed_first_tract.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct fixture {
    struct reloq_german_phone_node phones[6];
    struct reloq_german_syllable syllables[2];
    struct reloq_german_reduced_landmarks timing;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][24];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_target_builder builder;
    size_t count;
};

static void setup(struct fixture *x, enum reloq_german_phone vowel,
                  enum reloq_german_phone coda, int has_onset, int nasal)
{
    memset(x,0,sizeof *x);
    const enum reloq_german_phone seq[] = {
        RELOQ_DE_PHONE_p,vowel,coda,
        has_onset ? RELOQ_DE_PHONE_t : RELOQ_DE_PHONE_AT,
        has_onset ? RELOQ_DE_PHONE_AT : RELOQ_DE_PHONE_n,
        RELOQ_DE_PHONE_n};
    x->count = 3U + 1U + (size_t)has_onset + (size_t)nasal;
    for (size_t i = 0; i < x->count; ++i) {
        x->phones[i].phone = seq[i];
        x->phones[i].syllable_index = i >= 3;
        x->phones[i].syllable_in_word = i >= 3;
        x->phones[i].stress = i < 3;
        x->phones[i].stress_explicit = 1;
    }
    x->syllables[0].phone_count = 3;
    x->syllables[0].onset_count = x->syllables[0].nucleus_count =
        x->syllables[0].coda_count = 1;
    x->syllables[0].nucleus_contiguous = 1;
    x->syllables[0].stress = 1;
    x->syllables[0].stress_explicit = 1;
    x->syllables[0].boundary_after = RELOQ_DE_BOUNDARY_SYLLABLE;
    x->syllables[1].first_phone = 3;
    x->syllables[1].phone_count = x->count - 3;
    x->syllables[1].onset_count = has_onset;
    x->syllables[1].nucleus_count = 1;
    x->syllables[1].coda_count = nasal;
    x->syllables[1].nucleus_contiguous = 1;
    x->syllables[1].syllable_in_word = 1;
    x->syllables[1].stress_explicit = 1;
    x->syllables[1].boundary_after = RELOQ_DE_BOUNDARY_UTTERANCE;
    assert(reloq_german_reduced_landmarks_plan(
        x->phones,x->count,x->syllables,2,&x->timing) == 0);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        x->buffers[i].points = x->points[i];
        x->buffers[i].capacity = 24;
    }
    assert(reloq_target_builder_begin(&x->builder,x->buffers) == 0);
}

static void point(const struct fixture *x, enum reloq_target_stream stream,
                  size_t index, int64_t tick, int32_t value)
{
    const struct reloq_target_buffer *b = &x->builder.stream[stream];
    assert(index < b->count);
    assert(b->points[index].tick == tick);
    assert(b->points[index].value == value);
}

int main(void)
{
    struct fixture x;
    setup(&x,RELOQ_DE_PHONE_A,RELOQ_DE_PHONE_p,1,0);
    assert(reloq_german_reduced_closed_first_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == 0);
    point(&x,RELOQ_TARGET_F1,4,x.timing.first_closure,500);
    point(&x,RELOQ_TARGET_F2,5,x.timing.first_coda_release,1300);
    point(&x,RELOQ_TARGET_TL,9,x.timing.first_coda_release,0);

    setup(&x,RELOQ_DE_PHONE_I,RELOQ_DE_PHONE_k,0,1);
    assert(reloq_german_reduced_closed_first_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == 0);
    point(&x,RELOQ_TARGET_F3,4,x.timing.first_closure,2300);
    assert(x.builder.stream[RELOQ_TARGET_F1].count == 5);
    assert(x.builder.stream[RELOQ_TARGET_TL].count == 9);
    const size_t unchanged = x.builder.stream[RELOQ_TARGET_F1].count;
    x.phones[0].stress_explicit = 0;
    assert(reloq_german_reduced_closed_first_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == 1);
    assert(x.builder.stream[RELOQ_TARGET_F1].count == unchanged);
    x.phones[0].stress_explicit = 1;
    x.timing.first_coda_release += 1;
    assert(reloq_german_reduced_closed_first_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == 1);
    assert(x.builder.stream[RELOQ_TARGET_F1].count == unchanged);
    puts("reduced closed-first tract tests passed");
    return 0;
}
