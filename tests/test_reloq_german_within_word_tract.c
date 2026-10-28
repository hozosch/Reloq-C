#include "reloq_german_within_word_tract.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct fixture {
    struct reloq_german_phone_node phones[6];
    struct reloq_german_syllable syllables[2];
    struct reloq_german_within_word_landmarks timing;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][32];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_target_builder builder;
};

static void setup(struct fixture *x,
                  enum reloq_german_phone onset,
                  enum reloq_german_phone first_vowel,
                  enum reloq_german_phone first_coda,
                  enum reloq_german_phone second_vowel,
                  enum reloq_german_phone second_coda)
{
    memset(x, 0, sizeof *x);
    const enum reloq_german_phone phone[] = {
        onset, first_vowel, first_coda,
        RELOQ_DE_PHONE_t, second_vowel, second_coda};
    for (size_t i = 0; i < 6; ++i) {
        x->phones[i].phone = phone[i];
        x->phones[i].syllable_index = i / 3;
        x->phones[i].syllable_in_word = i / 3;
        x->phones[i].stress = i / 3;
        x->phones[i].stress_explicit = 1;
    }
    for (size_t i = 0; i < 2; ++i) {
        struct reloq_german_syllable *s = &x->syllables[i];
        s->first_phone = 3 * i;
        s->phone_count = 3;
        s->syllable_in_word = i;
        s->onset_count = s->nucleus_count = s->coda_count = 1;
        s->nucleus_contiguous = 1;
        s->stress = i;
        s->stress_explicit = 1;
        s->boundary_after = i ? RELOQ_DE_BOUNDARY_UTTERANCE :
            RELOQ_DE_BOUNDARY_SYLLABLE;
    }
    assert(reloq_german_within_word_landmarks_plan(
        x->phones, 6, x->syllables, 2, &x->timing) == 0);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        x->buffers[i].points = x->points[i];
        x->buffers[i].capacity = 32;
    }
    assert(reloq_target_builder_begin(&x->builder, x->buffers) == 0);
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
    setup(&x, RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_p,
          RELOQ_DE_PHONE_I, RELOQ_DE_PHONE_p);
    assert(reloq_german_within_word_tract_append(
        x.phones, 6, x.syllables, 2, &x.timing, &x.builder) == 0);
    assert(x.builder.stream[RELOQ_TARGET_F1].count == 12);
    assert(x.builder.stream[RELOQ_TARGET_F4].count == 14);
    assert(x.builder.stream[RELOQ_TARGET_TL].count == 22);
    assert(x.builder.stream[RELOQ_TARGET_FL].count == 4);
    point(&x, RELOQ_TARGET_F1, 2, 34, 780);
    point(&x, RELOQ_TARGET_F1, 4, 101, 500);
    point(&x, RELOQ_TARGET_F2, 5, 201, 1300);
    point(&x, RELOQ_TARGET_F3, 8, 344, 2800);
    point(&x, RELOQ_TARGET_F4, 6, 202, 3500);
    point(&x, RELOQ_TARGET_FL, 3, 557, 0);

    /* Contextually reduced rounded k-closure in the first weak syllable,
     * independent of the next vowel's full stressed closure/release. */
    setup(&x, RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_O, RELOQ_DE_PHONE_k,
          RELOQ_DE_PHONE_O, RELOQ_DE_PHONE_k);
    assert(reloq_german_within_word_tract_append(
        x.phones, 6, x.syllables, 2, &x.timing, &x.builder) == 0);
    point(&x, RELOQ_TARGET_F1, 4, x.timing.first_closure, 300);
    point(&x, RELOQ_TARGET_F1, 10, x.timing.second_closure, 500);
    point(&x, RELOQ_TARGET_F2, 11, x.timing.second_coda_release, 1500);

    const size_t unchanged = x.builder.stream[RELOQ_TARGET_F1].count;
    x.phones[1].stress_explicit = 0;
    assert(reloq_german_within_word_tract_append(
        x.phones, 6, x.syllables, 2, &x.timing, &x.builder) == 1);
    assert(x.builder.stream[RELOQ_TARGET_F1].count == unchanged);
    x.phones[1].stress_explicit = 1;
    x.timing.first_vowel_hold = x.timing.first_closure + 1;
    assert(reloq_german_within_word_tract_append(
        x.phones, 6, x.syllables, 2, &x.timing, &x.builder) == -1);
    assert(x.builder.stream[RELOQ_TARGET_F1].count == unchanged);
    puts("within-word tract tests passed");
    return 0;
}
