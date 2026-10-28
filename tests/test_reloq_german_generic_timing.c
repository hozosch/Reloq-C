#include "reloq_german_generic_timing.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct reloq_german_phone_node onset(
    enum reloq_german_phone left, enum reloq_german_phone stop,
    enum reloq_german_phone right, unsigned stress)
{
    struct reloq_german_phone_node node = {0};
    node.phone = stop;
    node.left_phone = left;
    node.right_phone = right;
    node.same_word_left = left != RELOQ_DE_PHONE_NONE;
    node.same_syllable_left = 0;
    node.same_syllable_right = 1;
    node.stress = (uint8_t)stress;
    return node;
}

static void expect_stop(enum reloq_german_phone left,
                        enum reloq_german_phone stop,
                        enum reloq_german_phone right, unsigned stress,
                        int total, int burst, int aspiration)
{
    const struct reloq_german_phone_node node = onset(
        left, stop, right, stress);
    struct reloq_german_generic_stop_timing t = {0};
    assert(reloq_german_generic_stop_onset_timing(&node, &t) == 1);
    assert(t.total_ticks == total);
    assert(t.burst_at_ticks == burst);
    assert(t.aspiration_at_ticks == aspiration);
    assert(t.total_ticks - t.aspiration_at_ticks > 0);
}

static void expect_vowel(enum reloq_german_phone phone,
                         unsigned stress, int ticks)
{
    struct reloq_german_phone_node node = {0};
    node.phone = phone;
    node.right_phone = RELOQ_DE_PHONE_p;
    node.same_word_right = 1;
    node.stress = (uint8_t)stress;
    int32_t value = -1;
    assert(reloq_german_generic_pre_stop_vowel_ticks(&node, &value) == 1);
    assert(value == ticks);
}

static void test_boundaries(void)
{
    struct reloq_german_generic_stop_timing timing = {12, 34, 56};
    struct reloq_german_phone_node node = onset(
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_A, 1);
    node.same_syllable_left = 1;
    assert(reloq_german_generic_stop_onset_timing(&node, &timing) == 0);
    node.same_syllable_left = 0;
    node.right_phone = RELOQ_DE_PHONE_s;
    assert(reloq_german_generic_stop_onset_timing(&node, &timing) == 0);
    node.right_phone = RELOQ_DE_PHONE_A;
    node.stress = 2;
    assert(reloq_german_generic_stop_onset_timing(&node, &timing) == 0);
    node.stress = 1;
    node.left_phone = RELOQ_DE_PHONE_s;
    assert(reloq_german_generic_stop_onset_timing(&node, &timing) == 0);
    assert(timing.total_ticks == 12 && timing.burst_at_ticks == 34 &&
           timing.aspiration_at_ticks == 56);
    assert(reloq_german_generic_stop_onset_timing(NULL, &timing) == -1);
    assert(reloq_german_generic_stop_onset_timing(&node, NULL) == -1);
    int32_t value = 9;
    assert(reloq_german_generic_pre_stop_vowel_ticks(NULL, &value) == -1);
    assert(reloq_german_generic_pre_stop_vowel_ticks(&node, &value) == 0);
    assert(value == 9);
}

static void test_voiced_onset_matrix(void)
{
    static const enum reloq_german_phone vowels[] = {
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_E, RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O, RELOQ_DE_PHONE_U, RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_e, RELOQ_DE_PHONE_i, RELOQ_DE_PHONE_o,
        RELOQ_DE_PHONE_u
    };
    static const int b_strong[] = {81, 93, 106, 93, 106,
                                   81, 93, 106, 93, 106};
    static const int d_strong[] = {100, 100, 100, 100, 100,
                                   100, 105, 105, 100, 100};
    for (unsigned stress = 0; stress <= 1; ++stress) {
        for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i) {
            const enum reloq_german_phone stops[] = {
                RELOQ_DE_PHONE_b, RELOQ_DE_PHONE_d, RELOQ_DE_PHONE_g};
            const int span[] = {
                stress ? b_strong[i] : 71,
                stress ? d_strong[i] : 50,
                stress ? 90 : 80
            };
            const int burst[] = {
                span[0] - 11, stress ? 90 : 40, 70
            };
            for (size_t j = 0; j < sizeof stops / sizeof stops[0]; ++j) {
                struct reloq_german_phone_node node = onset(
                    RELOQ_DE_PHONE_NONE, stops[j], vowels[i], stress);
                struct reloq_german_generic_voiced_onset_timing got = {0};
                assert(reloq_german_generic_voiced_onset_timing(
                    &node, &got) == 1);
                assert(got.total_ticks == span[j]);
                assert(got.burst_at_ticks == burst[j]);
                assert(got.burst_at_ticks < got.total_ticks);
            }
        }
    }
    struct reloq_german_phone_node node = onset(
        RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_d, RELOQ_DE_PHONE_A, 1);
    struct reloq_german_generic_voiced_onset_timing unchanged = {123, 45};
    node.same_word_left = 1;
    assert(reloq_german_generic_voiced_onset_timing(&node, &unchanged) == 0);
    node.same_word_left = 0;
    node.stress = 2;
    assert(reloq_german_generic_voiced_onset_timing(&node, &unchanged) == 0);
    node.stress = 1;
    node.phone = RELOQ_DE_PHONE_t;
    assert(reloq_german_generic_voiced_onset_timing(&node, &unchanged) == 0);
    node.phone = RELOQ_DE_PHONE_d;
    node.right_phone = RELOQ_DE_PHONE_s;
    assert(reloq_german_generic_voiced_onset_timing(&node, &unchanged) == 0);
    assert(unchanged.total_ticks == 123 && unchanged.burst_at_ticks == 45);
    assert(reloq_german_generic_voiced_onset_timing(NULL, &unchanged) == -1);
    assert(reloq_german_generic_voiced_onset_timing(&node, NULL) == -1);
}

static void test_internal_stop_pair(void)
{
    struct {
        const char *spr;
        size_t coda_index;
        int coda, release, merged;
        int onset, burst, aspiration;
    } positive[] = {
        {"`[.1Ap.0tA].", 1, 100, 90, 0, 93, 53, 65},
        {"`[.0Ap.1tI].", 1, 100, 90, 0, 143, 53, 71},
        {"`[.1At.0kA].", 1, 42, 34, 0, 162, 100, 112},
        {"`[.1Uk.0pI].", 1, 60, 48, 0, 105, 76, 86},
        {"`[.0Ek.1kI].", 1, 65, 53, 1, 129, 56, 68},
        {"`[.0pAp.1tIp].", 2, 100, 90, 0, 143, 53, 71},
    };
    for (size_t i = 0; i < sizeof positive / sizeof positive[0]; ++i) {
        struct reloq_german_phone_node phones[12] = {{0}};
        struct reloq_german_phone_sequence_info info = {0};
        const char *spr = positive[i].spr;
        assert(reloq_german_phone_sequence_build(
            (const unsigned char *)spr, strlen(spr), 10000,
            phones, 12, &info) == 0);
        struct reloq_german_generic_internal_stop_pair pair = {0};
        assert(reloq_german_generic_internal_stop_pair(
            phones, info.phone_count, positive[i].coda_index, &pair) == 1);
        assert(pair.coda_ticks == positive[i].coda);
        assert(pair.coda_burst_at_ticks == positive[i].release);
        assert(pair.merge_coda_with_onset == positive[i].merged);
        assert(pair.onset.total_ticks == positive[i].onset);
        assert(pair.onset.burst_at_ticks == positive[i].burst);
        assert(pair.onset.aspiration_at_ticks == positive[i].aspiration);
        assert(pair.coda_burst_at_ticks < pair.coda_ticks);
        assert(pair.onset.burst_at_ticks < pair.onset.aspiration_at_ticks);
        assert(pair.onset.aspiration_at_ticks < pair.onset.total_ticks);
    }
    const char *negative[] = {
        "`[.1Ap] `2 `[.1tA].", /* the connected-pair planner owns this */
        "`[.1Ap]. `2 `[.1tA].", /* sentence boundary */
        "`[.1Ap.2tI].",        /* secondary stress */
        "`[.1Op.0tA].",        /* first vowel outside validated class */
        "`[.1Ap.0tU].",       /* next vowel outside validated class */
        "`[.0Ap.1tA].",       /* stressed A not measured */
        "`[.1As.0tA].",       /* fricative coda */
        "`[.1Ap.0mA].",       /* sonorant onset */
    };
    for (size_t i = 0; i < sizeof negative / sizeof negative[0]; ++i) {
        struct reloq_german_phone_node phones[12] = {{0}};
        struct reloq_german_phone_sequence_info info = {0};
        assert(reloq_german_phone_sequence_build(
            (const unsigned char *)negative[i], strlen(negative[i]),
            10000, phones, 12, &info) == 0);
        struct reloq_german_generic_internal_stop_pair unchanged = {
            123, 45, 1, {67, 8, 9}};
        assert(reloq_german_generic_internal_stop_pair(
            phones, info.phone_count, 1, &unchanged) == 0);
        assert(unchanged.coda_ticks == 123 &&
               unchanged.onset.total_ticks == 67);
    }
    struct reloq_german_phone_node dummy = {0};
    struct reloq_german_generic_internal_stop_pair output = {0};
    assert(reloq_german_generic_internal_stop_pair(
        NULL, 1, 1, &output) == -1);
    assert(reloq_german_generic_internal_stop_pair(
        &dummy, 1, 1, NULL) == -1);
    assert(reloq_german_generic_internal_stop_pair(
        &dummy, 1, 0, &output) == 0);
}

static enum reloq_german_phone phone(const char *symbol)
{
    static const struct {
        char letter;
        enum reloq_german_phone phone;
    } phones[] = {
        {'-', RELOQ_DE_PHONE_NONE}, {'A', RELOQ_DE_PHONE_A},
        {'E', RELOQ_DE_PHONE_E}, {'I', RELOQ_DE_PHONE_I},
        {'O', RELOQ_DE_PHONE_O}, {'U', RELOQ_DE_PHONE_U},
        {'a', RELOQ_DE_PHONE_a}, {'e', RELOQ_DE_PHONE_e},
        {'i', RELOQ_DE_PHONE_i}, {'o', RELOQ_DE_PHONE_o},
        {'u', RELOQ_DE_PHONE_u}, {'Y', RELOQ_DE_PHONE_Y},
        {'y', RELOQ_DE_PHONE_y}, {'p', RELOQ_DE_PHONE_p},
        {'t', RELOQ_DE_PHONE_t}, {'k', RELOQ_DE_PHONE_k}
    };
    if (strlen(symbol) != 1) return RELOQ_DE_PHONE_NONE;
    for (size_t i = 0; i < sizeof phones / sizeof phones[0]; ++i)
        if (*symbol == phones[i].letter) return phones[i].phone;
    return RELOQ_DE_PHONE_NONE;
}

int main(int argc, char **argv)
{
    if (argc == 6 && !strcmp(argv[1], "--query")) {
        const enum reloq_german_phone first = phone(argv[2]);
        const enum reloq_german_phone stop = phone(argv[3]);
        const enum reloq_german_phone next = phone(argv[4]);
        const int stress = atoi(argv[5]);
        if (stop != RELOQ_DE_PHONE_p && stop != RELOQ_DE_PHONE_t &&
            stop != RELOQ_DE_PHONE_k) return 2;
        const struct reloq_german_phone_node node = onset(
            first, stop, next, (unsigned)stress);
        struct reloq_german_generic_stop_timing timing;
        int32_t first_ticks = -1;
        if (first) {
            struct reloq_german_phone_node vowel = {0};
            vowel.phone = first;
            vowel.right_phone = stop;
            vowel.stress = stress == 0 ? 1 : 0;
            vowel.same_word_right = 1;
            if (reloq_german_generic_pre_stop_vowel_ticks(
                    &vowel, &first_ticks) != 1) return 2;
        }
        if (reloq_german_generic_stop_onset_timing(&node, &timing) != 1)
            return 2;
        printf("%d %d %d %d\n", (int)first_ticks,
               (int)timing.total_ticks, (int)timing.burst_at_ticks,
               (int)timing.aspiration_at_ticks);
        return 0;
    }
    /* Reference values were measured in target insertions via the external
     * public ECI, not generated from our tables. Internal burst and vowel
     * start are distinct time landmarks. */
    expect_stop(RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_p,
                RELOQ_DE_PHONE_A, 0, 34, 5, 15);
    expect_stop(RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_p,
                RELOQ_DE_PHONE_A, 1, 65, 5, 15);
    expect_stop(RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_t,
                RELOQ_DE_PHONE_E, 1, 95, 5, 23);
    expect_stop(RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_k,
                RELOQ_DE_PHONE_e, 1, 93, 5, 17);
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_p,
                RELOQ_DE_PHONE_A, 0, 119, 90, 100);
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_p,
                RELOQ_DE_PHONE_A, 1, 150, 90, 100);
    expect_stop(RELOQ_DE_PHONE_e, RELOQ_DE_PHONE_p,
                RELOQ_DE_PHONE_A, 0, 104, 75, 85);
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_t,
                RELOQ_DE_PHONE_A, 0, 97, 57, 69);
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_t,
                RELOQ_DE_PHONE_e, 1, 165, 57, 75);
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_k,
                RELOQ_DE_PHONE_A, 0, 132, 70, 82);
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_k,
                RELOQ_DE_PHONE_e, 1, 163, 75, 87);
    /* The reference core ends before aspiration; schwa shortens t closure. */
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_p,
                RELOQ_DE_PHONE_AT, 0, 119, 90, 100);
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_t,
                RELOQ_DE_PHONE_AT, 0, 74, 34, 46);
    expect_stop(RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_k,
                RELOQ_DE_PHONE_AT, 0, 132, 70, 82);
    expect_vowel(RELOQ_DE_PHONE_A, 1, 81);
    expect_vowel(RELOQ_DE_PHONE_A, 0, 74);
    expect_vowel(RELOQ_DE_PHONE_e, 1, 142);
    expect_vowel(RELOQ_DE_PHONE_e, 0, 89);
    expect_vowel(RELOQ_DE_PHONE_I, 1, 83);
    expect_vowel(RELOQ_DE_PHONE_I, 0, 69);
    test_boundaries();
    test_voiced_onset_matrix();
    test_internal_stop_pair();
    puts("generic stop timing: context and stress landmarks passed");
    return 0;
}
