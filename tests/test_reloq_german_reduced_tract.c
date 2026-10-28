#include "reloq_german_reduced_tract.h"

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

static void setup(struct fixture *x, const enum reloq_german_phone *first,
                  size_t first_count, const enum reloq_german_phone *second,
                  size_t second_count)
{
    memset(x,0,sizeof *x);
    x->count = first_count + second_count;
    const int onset = second[0] != RELOQ_DE_PHONE_AT;
    const int nasal = second[second_count - 1] == RELOQ_DE_PHONE_n;
    for (size_t i = 0; i < x->count; ++i) {
        x->phones[i].phone = i < first_count ? first[i] : second[i-first_count];
        x->phones[i].syllable_index = i >= first_count;
        x->phones[i].syllable_in_word = i >= first_count;
        x->phones[i].stress = i < first_count;
        x->phones[i].stress_explicit = 1;
    }
    x->syllables[0].phone_count = first_count;
    x->syllables[0].onset_count = 1;
    x->syllables[0].nucleus_count = 1;
    x->syllables[0].coda_count = first_count - 2;
    x->syllables[0].nucleus_contiguous = 1;
    x->syllables[0].stress = 1;
    x->syllables[0].stress_explicit = 1;
    x->syllables[0].boundary_after = RELOQ_DE_BOUNDARY_SYLLABLE;
    x->syllables[1].first_phone = first_count;
    x->syllables[1].phone_count = second_count;
    x->syllables[1].syllable_in_word = 1;
    x->syllables[1].onset_count = onset;
    x->syllables[1].nucleus_count = 1;
    x->syllables[1].coda_count = nasal;
    x->syllables[1].nucleus_contiguous = 1;
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
    const enum reloq_german_phone first_cvc[] = {
        RELOQ_DE_PHONE_p,RELOQ_DE_PHONE_A,RELOQ_DE_PHONE_p};
    const enum reloq_german_phone first_cv[] = {
        RELOQ_DE_PHONE_p,RELOQ_DE_PHONE_A};
    const enum reloq_german_phone t_at[] = {
        RELOQ_DE_PHONE_t,RELOQ_DE_PHONE_AT};
    const enum reloq_german_phone p_at[] = {
        RELOQ_DE_PHONE_p,RELOQ_DE_PHONE_AT};
    const enum reloq_german_phone at_n[] = {
        RELOQ_DE_PHONE_AT,RELOQ_DE_PHONE_n};

    setup(&x,first_cvc,3,t_at,2);
    assert(reloq_german_reduced_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == 0);
    point(&x,RELOQ_TARGET_F1,0,x.timing.second_onset_origin,300);
    point(&x,RELOQ_TARGET_F2,2,x.timing.second_schwa_target,1540);
    point(&x,RELOQ_TARGET_TL,1,x.timing.second_burst_start,0);

    setup(&x,first_cv,2,p_at,2);
    assert(reloq_german_reduced_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == 0);
    point(&x,RELOQ_TARGET_F2,0,x.timing.first_closure,800);
    point(&x,RELOQ_TARGET_F2,1,x.timing.second_burst_end,1000);
    point(&x,RELOQ_TARGET_F2,2,x.timing.second_schwa_target,1270);
    point(&x,RELOQ_TARGET_TL,1,x.timing.first_closure,0);

    setup(&x,first_cvc,3,at_n,2);
    assert(reloq_german_reduced_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == 0);
    point(&x,RELOQ_TARGET_F2,0,x.timing.first_coda_release,1000);
    point(&x,RELOQ_TARGET_F2,1,x.timing.second_schwa_target,1540);
    point(&x,RELOQ_TARGET_F2,3,x.timing.second_nasal_onset,1700);
    point(&x,RELOQ_TARGET_FNZ,3,x.timing.second_closure - 55,350);

    const size_t unchanged = x.builder.stream[RELOQ_TARGET_F1].count;
    x.phones[0].stress_explicit = 0;
    assert(reloq_german_reduced_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == 1);
    assert(x.builder.stream[RELOQ_TARGET_F1].count == unchanged);
    x.phones[0].stress_explicit = 1;
    x.timing.second_schwa_target = x.timing.second_vowel_onset;
    assert(reloq_german_reduced_tract_append(
        x.phones,x.count,x.syllables,2,&x.timing,&x.builder) == -1);
    assert(x.builder.stream[RELOQ_TARGET_F1].count == unchanged);
    puts("reduced-syllable tract tests passed");
    return 0;
}
