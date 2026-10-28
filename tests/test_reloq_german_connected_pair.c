#include "reloq_german_connected_pair.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

static void boundary_matrix(void)
{
    const enum reloq_german_phone codas[] = {
        RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_t, RELOQ_DE_PHONE_k};
    const enum reloq_german_phone onsets[] = {
        RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_t,
        RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_h};
    const int end[3][4] = {
        {40,100,100,100}, {58,30,58,65}, {52,52,65,87}};
    const int lead[3][4] = {
        {11,9,7,13}, {11,9,38,13}, {11,28,7,13}};
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 4; ++j) {
            struct reloq_german_connected_pair pair = {
                RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_A,
                codas[i], onsets[j]};
            struct reloq_german_connected_landmarks actual = {0};
            assert(!reloq_german_connected_pair_landmarks(
                &pair, 0, -1, 400, &actual));
            assert(actual.first_vowel_ticks == 90);
            assert(actual.first_closure_tick == 90);
            assert(actual.first_coda_send_end == 90 + end[i][j]);
            assert(actual.second_early_f0_tick ==
                   90 + end[i][j] + lead[i][j]);
            assert(actual.merge_identical_stops ==
                   (j < 3 && i == j));
        }
}

static void vowel_matrix(void)
{
    const enum reloq_german_phone vowels[] = {
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_E, RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O, RELOQ_DE_PHONE_U, RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_e, RELOQ_DE_PHONE_i, RELOQ_DE_PHONE_o,
        RELOQ_DE_PHONE_u};
    const int base[] = {90,90,84,84,72,140,130,120,130,120};
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i) {
        struct reloq_german_connected_pair pair = {
            RELOQ_DE_PHONE_h, vowels[i], RELOQ_DE_PHONE_k,
            RELOQ_DE_PHONE_t};
        struct reloq_german_connected_landmarks actual = {0};
        assert(!reloq_german_connected_pair_landmarks(
            &pair, 80, 100, 500, &actual));
        assert(actual.first_vowel_ticks == base[i]);
        pair.first_initial = RELOQ_DE_PHONE_p;
        assert(!reloq_german_connected_pair_landmarks(
            &pair, 65, 15, 500, &actual));
        assert(actual.first_vowel_ticks == base[i] * 9 / 10);
    }
}

static void f0_and_transaction(void)
{
    struct reloq_target_builder builder;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        storage[i] = (struct reloq_target_buffer){points[i], 8, 0};
    assert(!reloq_target_builder_begin(&builder, storage));
    struct reloq_german_connected_pair pair = {
        RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_A,
        RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_t};
    struct reloq_german_connected_landmarks actual = {0};
    int64_t first_closure = -1, first_release = -1;
    int64_t first_duration = -1;
    assert(!reloq_german_connected_first_duration(&pair, &first_duration));
    assert(first_duration == 81);
    assert(!reloq_german_connected_first_closure(
        &pair, 65, 15, &first_closure, &first_release));
    assert(first_closure == 146 && first_release == 198);
    pair.first_initial = RELOQ_DE_PHONE_h;
    assert(!reloq_german_connected_first_duration(&pair, &first_duration));
    assert(first_duration == 90);
    assert(!reloq_german_connected_first_closure(
        &pair, 80, 130, &first_closure, &first_release));
    assert(first_closure == 170 && first_release == 222);
    assert(reloq_german_connected_first_closure(
        &pair, 80, 75, &first_closure, &first_release) == -1);
    pair.first_initial = RELOQ_DE_PHONE_p;
    assert(!reloq_german_connected_pair_f0_append(
        &pair, 65, 15, 347, 435, &builder, &actual));
    assert(actual.first_vowel_ticks == 81);
    assert(actual.first_closure_tick == 146);
    assert(actual.first_coda_send_end == 198);
    assert(actual.second_early_f0_tick == 226);
    assert(!actual.merge_identical_stops);
    const int ticks[] = {0, 125, 226, 360, 408, 434};
    const int values[] = {1153,1224,1043,1293,793,740};
    const struct reloq_target_buffer *f0 =
        &builder.stream[RELOQ_TARGET_F0];
    assert(f0->count == 6);
    for (size_t i = 0; i < 6; ++i) {
        assert(f0->points[i].tick == ticks[i]);
        assert(f0->points[i].value == values[i]);
    }

    /* An invalid after-the-fact closure must not append any targets or
     * replace the successful landmark result. */
    struct reloq_german_connected_landmarks old = actual;
    assert(reloq_german_connected_pair_f0_append(
        &pair, 65, 15, 347, 347, &builder, &actual) == -1);
    assert(f0->count == 6 &&
           actual.first_coda_send_end == old.first_coda_send_end);
    pair.first_coda = RELOQ_DE_PHONE_s;
    assert(reloq_german_connected_pair_landmarks(
        &pair, 65, 15, 347, &actual) == 1);
    assert(reloq_german_connected_pair_landmarks(
        NULL, 65, 15, 347, &actual) == -1);
    pair.first_coda = RELOQ_DE_PHONE_k;
    assert(reloq_german_connected_pair_landmarks(
        &pair, INT64_MAX, 15, INT64_MAX, &actual) == -1);
}

static void kt_coda_gesture(void)
{
    struct reloq_target_builder builder;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][12];
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        storage[i] = (struct reloq_target_buffer){points[i], 12, 0};
    assert(!reloq_target_builder_begin(&builder, storage));
    struct reloq_german_connected_pair pair = {
        RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_A,
        RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_t};
    int64_t origin = -1;
    assert(!reloq_german_connected_kt_coda_append(
        &pair, 146, &builder, &origin));
    assert(origin == 218);
    size_t target_count = 0;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        target_count += builder.stream[i].count;
    assert(target_count == 72);
    const struct reloq_target_buffer *f2 =
        &builder.stream[RELOQ_TARGET_F2];
    assert(f2->count == 2 && f2->points[0].tick == 146 &&
           f2->points[0].value == 1750 &&
           f2->points[1].tick == 198 && f2->points[1].value == 1800);
    const struct reloq_target_buffer *a2f =
        &builder.stream[RELOQ_TARGET_A2F];
    const int tick[] = {146,186,186,198};
    const int gain[] = {0,0,60,60};
    assert(a2f->count == 4);
    for (size_t i = 0; i < 4; ++i) {
        assert(a2f->points[i].tick == tick[i]);
        assert(a2f->points[i].value == gain[i]);
    }
    const struct reloq_target_buffer *ah =
        &builder.stream[RELOQ_TARGET_AH];
    assert(ah->count == 6 && ah->points[4].tick == 198 &&
           ah->points[4].value == 50 && ah->points[5].tick == 218);
    size_t original = f2->count;
    pair.first_coda = RELOQ_DE_PHONE_p;
    assert(reloq_german_connected_kt_coda_append(
        &pair, 300, &builder, &origin) == 1);
    assert(f2->count == original && origin == 218);
    pair.first_coda = RELOQ_DE_PHONE_k;
    assert(reloq_german_connected_kt_coda_append(
        &pair, INT64_MAX, &builder, &origin) == -1);
    assert(f2->count == original && origin == 218);
}

static void kt_onset_gesture(void)
{
    struct reloq_target_builder builder;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][12];
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        storage[i] = (struct reloq_target_buffer){points[i], 12, 0};
    assert(!reloq_target_builder_begin(&builder, storage));
    struct reloq_german_connected_pair pair = {
        RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_A,
        RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_t};
    int64_t end = -1, vowel = -1;
    assert(!reloq_german_connected_kt_onset_landmarks(
        &pair, RELOQ_DE_PHONE_I, 198, &end, &vowel));
    assert(end == 304 && vowel == 347);
    assert(!reloq_german_connected_kt_onset_append(
        &pair, RELOQ_DE_PHONE_I, 198, 560,
        &builder, &end, &vowel));
    assert(end == 304 && vowel == 347);
    size_t target_count = 0;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        target_count += builder.stream[i].count;
    assert(target_count == 78); /* Four F0 targets live in the pair stage. */
    const struct reloq_target_buffer *f2 =
        &builder.stream[RELOQ_TARGET_F2];
    assert(f2->count == 2 && f2->points[0].tick == 218 &&
           f2->points[0].value == 1600 &&
           f2->points[1].tick == 304 && f2->points[1].value == 1700);
    const struct reloq_target_buffer *af =
        &builder.stream[RELOQ_TARGET_AF];
    assert(af->count == 4 && af->points[1].tick == 286 &&
           af->points[1].value == 0 && af->points[2].tick == 286 &&
           af->points[2].value == 59 && af->points[3].tick == 304);
    const struct reloq_target_buffer *ah =
        &builder.stream[RELOQ_TARGET_AH];
    assert(ah->count == 6 && ah->points[4].tick == 304 &&
           ah->points[4].value == 47 && ah->points[5].tick == 347);
    const struct reloq_target_buffer *f0 =
        &builder.stream[RELOQ_TARGET_F0];
    assert(f0->count == 0); /* Supplied by the paired F0 function. */
    assert(builder.stream[RELOQ_TARGET_FL].points[1].tick == 560);
    size_t old = af->count;
    assert(reloq_german_connected_kt_onset_append(
        &pair, RELOQ_DE_PHONE_I, 198, 300,
        &builder, &end, &vowel) == -1);
    assert(af->count == old && end == 304 && vowel == 347);

    /* Lower/longer vowel qualities change the initial burst window and
     * acoustic anchor, not the shared release-relative ramp origin. */
    struct reloq_target_builder lower;
    struct reloq_target_buffer empty[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_acoustic_point lower_points[RELOQ_TARGET_STREAM_COUNT][8];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        empty[i] = (struct reloq_target_buffer){lower_points[i], 8, 0};
    assert(!reloq_target_builder_begin(&lower, empty));
    assert(!reloq_german_connected_kt_onset_append(
        &pair, RELOQ_DE_PHONE_a, 198, 560,
        &lower, &end, &vowel));
    assert(end == 298 && vowel == 341);
    assert(lower.stream[RELOQ_TARGET_AF].points[1].tick == 286);

    const enum reloq_german_phone quality[] = {
        RELOQ_DE_PHONE_A,RELOQ_DE_PHONE_E,RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O,RELOQ_DE_PHONE_U,RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_e,RELOQ_DE_PHONE_i,RELOQ_DE_PHONE_o,
        RELOQ_DE_PHONE_u};
    const int expected_end[] = {298,304,304,298,298,298,304,304,298,298};
    const int expected_anchor[] = {341,347,347,341,328,341,
                                    358,358,352,335};
    for (size_t i = 0; i < sizeof quality / sizeof quality[0]; ++i) {
        assert(!reloq_german_connected_kt_onset_landmarks(
            &pair, quality[i], 198, &end, &vowel));
        assert(end == expected_end[i] && vowel == expected_anchor[i]);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            empty[s].count = 0;
        assert(!reloq_target_builder_begin(&lower, empty));
        assert(!reloq_german_connected_kt_onset_append(
            &pair, quality[i], 198, 560, &lower, &end, &vowel));
        assert(end == expected_end[i] && vowel == expected_anchor[i]);
    }
}

int main(void)
{
    boundary_matrix();
    vowel_matrix();
    f0_and_transaction();
    kt_coda_gesture();
    kt_onset_gesture();
    puts("connected pair timing/F0 passed");
    return 0;
}
