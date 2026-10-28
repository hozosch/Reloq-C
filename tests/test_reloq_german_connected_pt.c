#include "reloq_german_connected_pt.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>

static struct reloq_target_builder builder;
static struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][12];
static struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];

static void begin(void)
{
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        storage[s] = (struct reloq_target_buffer){points[s],12,0};
    assert(!reloq_target_builder_begin(&builder,storage));
}

static size_t count(void)
{
    size_t total = 0;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        total += builder.stream[s].count;
    return total;
}

static void check(enum reloq_target_stream s, size_t i,
                  int64_t tick, int32_t value)
{
    const struct reloq_target_buffer *stream = &builder.stream[s];
    assert(i < stream->count);
    assert(stream->points[i].tick == tick);
    assert(stream->points[i].value == value);
}

static void papp_tipp(void)
{
    /* The separate Apple reference reports C=146, R=246, B=23, V=95:
     * /t/ send 246..333 and following vowel anchor 405. */
    const struct reloq_german_connected_pair pair = {
        RELOQ_DE_PHONE_p,RELOQ_DE_PHONE_A,
        RELOQ_DE_PHONE_p,RELOQ_DE_PHONE_t};
    int64_t origin = -1, end = -1, vowel = -1;
    begin();
    assert(!reloq_german_connected_pt_coda_append(
        &pair,146,&builder,&origin));
    assert(origin == 247 && count() == 68);
    check(RELOQ_TARGET_F1,0,146,500);
    check(RELOQ_TARGET_F1,1,246,300);
    check(RELOQ_TARGET_F2,0,146,900);
    check(RELOQ_TARGET_F2,1,246,1300);
    check(RELOQ_TARGET_F3,0,146,2500);
    check(RELOQ_TARGET_F3,1,246,2500);
    check(RELOQ_TARGET_AB,0,146,0);
    check(RELOQ_TARGET_AB,1,236,0);
    check(RELOQ_TARGET_AB,2,236,60);
    check(RELOQ_TARGET_AB,3,246,60);
    check(RELOQ_TARGET_AF,2,236,60);
    check(RELOQ_TARGET_AV,3,246,0);
    check(RELOQ_TARGET_AH,3,246,0);
    assert(builder.stream[RELOQ_TARGET_AV].count == 4);
    assert(builder.stream[RELOQ_TARGET_AH].count == 4);

    assert(!reloq_german_connected_pt_onset_landmarks(
        &pair,RELOQ_DE_PHONE_I,246,&end,&vowel));
    assert(end == 333 && vowel == 405);
    assert(!reloq_german_connected_pt_onset_append(
        &pair,RELOQ_DE_PHONE_I,246,618,&builder,&end,&vowel));
    assert(end == 333 && vowel == 405 && count() == 146);
    check(RELOQ_TARGET_F1,2,247,300);
    check(RELOQ_TARGET_F1,3,333,300);
    check(RELOQ_TARGET_F2,2,247,1600);
    check(RELOQ_TARGET_F2,3,333,1700);
    check(RELOQ_TARGET_F3,3,333,2700);
    check(RELOQ_TARGET_A5F,4,247,0);
    check(RELOQ_TARGET_A5F,5,315,0);
    check(RELOQ_TARGET_A5F,6,315,56);
    check(RELOQ_TARGET_A5F,7,333,56);
    check(RELOQ_TARGET_AF,5,315,0);
    check(RELOQ_TARGET_AF,6,315,59);
    check(RELOQ_TARGET_AH,7,333,0);
    check(RELOQ_TARGET_AH,8,333,47);
    check(RELOQ_TARGET_AH,9,405,47);
    check(RELOQ_TARGET_FL,1,618,0);
    check(RELOQ_TARGET_FTP,1,618,250);
    assert(builder.stream[RELOQ_TARGET_F0].count == 0);
}

static void vowel_matrix(void)
{
    const enum reloq_german_phone vowels[] = {
        RELOQ_DE_PHONE_A,RELOQ_DE_PHONE_E,RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O,RELOQ_DE_PHONE_U,RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_e,RELOQ_DE_PHONE_i,RELOQ_DE_PHONE_o,
        RELOQ_DE_PHONE_u};
    const int coda_f1[] = {500,300,300,300,300,500,300,300,300,300};
    const int coda_f2[] = {900,1000,1000,700,600,900,1500,1500,600,600};
    const int coda_f3[] = {2500,2200,2200,2500,2100,
                            2500,2500,2500,2100,2100};
    const int isolated_t_onset[] = {89,95,95,89,67,89,113,113,107,80};
    const int isolated_t_burst[] = {17,23,23,17,17,17,23,23,17,17};
    for (size_t i = 0; i < 10; ++i)
        for (size_t j = 0; j < 10; ++j) {
            struct reloq_german_connected_pair pair = {
                RELOQ_DE_PHONE_NONE,vowels[i],
                RELOQ_DE_PHONE_p,RELOQ_DE_PHONE_t};
            int64_t origin = -1, end = -1, anchor = -1;
            begin();
            assert(!reloq_german_connected_pt_coda_append(
                &pair,150,&builder,&origin));
            assert(origin == 251 && count() == 68);
            check(RELOQ_TARGET_F1,0,150,coda_f1[i]);
            check(RELOQ_TARGET_F2,0,150,coda_f2[i]);
            check(RELOQ_TARGET_F3,0,150,coda_f3[i]);
            assert(!reloq_german_connected_pt_onset_append(
                &pair,vowels[j],250,600,&builder,&end,&anchor));
            assert(end == 314 + isolated_t_burst[j]);
            assert(anchor == 314 + isolated_t_onset[j]);
            assert(count() == 146);
            check(RELOQ_TARGET_F1,3,end,j == 9 ? 280 : 300);
            check(RELOQ_TARGET_F2,3,end,
                  isolated_t_burst[j] == 23 ? 1700 : 1600);
            check(RELOQ_TARGET_A5F,6,319,56);
            check(RELOQ_TARGET_AH,9,anchor,47);
        }
}

static void invalid_and_transaction(void)
{
    struct reloq_german_connected_pair pair = {
        RELOQ_DE_PHONE_p,RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_p,RELOQ_DE_PHONE_t};
    int64_t origin = -1, end = -1, anchor = -1;
    begin();
    pair.first_coda = RELOQ_DE_PHONE_k;
    assert(reloq_german_connected_pt_coda_append(
        &pair,100,&builder,&origin) == 1);
    assert(reloq_german_connected_pt_onset_landmarks(
        &pair,RELOQ_DE_PHONE_A,200,&end,&anchor) == 1);
    assert(count() == 0 && origin == -1 && end == -1 && anchor == -1);
    pair.first_coda = RELOQ_DE_PHONE_p;
    assert(reloq_german_connected_pt_coda_append(
        &pair,INT64_MAX,&builder,&origin) == -1);
    assert(reloq_german_connected_pt_onset_landmarks(
        &pair,RELOQ_DE_PHONE_A,INT64_MAX,&end,&anchor) == -1);
    assert(reloq_german_connected_pt_onset_landmarks(
        &pair,RELOQ_DE_PHONE_h,200,&end,&anchor) == 1);
    assert(reloq_german_connected_pt_onset_append(
        &pair,RELOQ_DE_PHONE_A,200,353,&builder,&end,&anchor) == -1);
    assert(count() == 0 && origin == -1 && end == -1 && anchor == -1);

    /* Give one stream no room: an all-stream gesture must append nothing. */
    storage[RELOQ_TARGET_F2].capacity = 1;
    assert(!reloq_target_builder_begin(&builder,storage));
    assert(reloq_german_connected_pt_coda_append(
        &pair,100,&builder,&origin) == -1);
    assert(count() == 0 && origin == -1);
    begin();
    assert(!reloq_german_connected_pt_coda_append(
        &pair,100,&builder,&origin));
    assert(origin == 201 && count() == 68);
    /* The initial /t/ append is equally transactional after a valid coda. */
    storage[RELOQ_TARGET_A5F] = builder.stream[RELOQ_TARGET_A5F];
    storage[RELOQ_TARGET_A5F].capacity = 4;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        if (s != RELOQ_TARGET_A5F) storage[s] = builder.stream[s];
    assert(!reloq_target_builder_begin(&builder,storage));
    assert(reloq_german_connected_pt_onset_append(
        &pair,RELOQ_DE_PHONE_I,200,500,&builder,&end,&anchor) == -1);
    assert(count() == 68 && end == -1 && anchor == -1);
}

int main(void)
{
    papp_tipp();
    vowel_matrix();
    invalid_and_transaction();
    puts("connected p to t targets passed");
    return 0;
}
