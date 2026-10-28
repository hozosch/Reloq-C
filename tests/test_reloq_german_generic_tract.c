#include "reloq_german_generic_tract.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct reloq_german_generic_tract vowel(
    enum reloq_german_phone phone, enum reloq_german_phone left,
    enum reloq_german_phone right, int left_same, int right_same,
    int f1, int f2, int f3, int av)
{
    struct reloq_german_phone_node node = {
        .phone = phone, .left_phone = left, .right_phone = right,
        .same_syllable_left = left_same,
        .same_syllable_right = right_same,
        .same_word_right = right_same
    };
    struct reloq_german_generic_tract result = {0};
    assert(reloq_german_generic_vowel_tract(
               &node, f1, f2, f3, av, &result) == 0);
    return result;
}

static struct reloq_german_generic_tract sonorant(
    enum reloq_german_phone phone, enum reloq_german_phone left,
    enum reloq_german_phone right, int left_same, int right_same)
{
    struct reloq_german_phone_node node = {
        .phone = phone, .left_phone = left, .right_phone = right,
        .same_syllable_left = left_same,
        .same_syllable_right = right_same
    };
    struct reloq_german_generic_tract result = {0};
    assert(reloq_german_generic_sonorant_tract(&node, &result) == 0);
    return result;
}

static void test_vowel_context(void)
{
    struct reloq_german_generic_tract base = vowel(
        RELOQ_DE_PHONE_A, 0, 0, 0, 0, 700, 1160, 2500, 51);
    assert(base.f1 == 700 && base.f2 == 1160 && base.f3 == 2500 &&
           base.b1 == 120 && base.b2 == 100 &&
           base.fnp == 200 && base.fnz == 200 &&
           base.av == 51 && base.ah == 34);
    struct reloq_german_generic_tract coda = vowel(
        RELOQ_DE_PHONE_A, 0, RELOQ_DE_PHONE_t, 0, 1,
        700, 1160, 2500, 51);
    assert(coda.f1 == 780 && coda.f2 == 1300);
    struct reloq_german_generic_tract combined = vowel(
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_t, 1, 1,
        700, 1160, 2500, 51);
    assert(combined.f1 == 780 && combined.f2 == 1150);
    struct reloq_german_generic_tract across_boundary = vowel(
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_t, 0, 0,
        700, 1160, 2500, 51);
    assert(across_boundary.f1 == 700 && across_boundary.f2 == 1160);
    const enum reloq_german_phone adjacent[] = {
        RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_t, RELOQ_DE_PHONE_k,
        RELOQ_DE_PHONE_m, RELOQ_DE_PHONE_n, RELOQ_DE_PHONE_G,
        RELOQ_DE_PHONE_l, RELOQ_DE_PHONE_r, RELOQ_DE_PHONE_s,
        RELOQ_DE_PHONE_f, RELOQ_DE_PHONE_S, RELOQ_DE_PHONE_X,
        RELOQ_DE_PHONE_w, RELOQ_DE_PHONE_x,
        RELOQ_DE_PHONE_b, RELOQ_DE_PHONE_d, RELOQ_DE_PHONE_g,
        RELOQ_DE_PHONE_v, RELOQ_DE_PHONE_z, RELOQ_DE_PHONE_Z
    };
    for (size_t i = 0; i < sizeof adjacent / sizeof adjacent[0]; ++i) {
        struct reloq_german_phone_node short_a = {
            .phone = RELOQ_DE_PHONE_A, .right_phone = adjacent[i],
            .same_word_right = 1,
            /* Different syllables still share a lexical articulatory
             * context; a different word must not receive the same F1. */
            .same_syllable_right = (i & 1) != 0
        };
        struct reloq_german_generic_tract got;
        assert(reloq_german_generic_vowel_tract(
                   &short_a, 700, 1160, 2500, 51, &got) == 0);
        assert(got.f1 == (adjacent[i] == RELOQ_DE_PHONE_x ? 800 : 780));
        short_a.same_word_right = 0;
        assert(reloq_german_generic_vowel_tract(
                   &short_a, 700, 1160, 2500, 51, &got) == 0);
        assert(got.f1 == 700);
    }
    struct reloq_german_generic_tract before_vowel = vowel(
        RELOQ_DE_PHONE_A, 0, RELOQ_DE_PHONE_E, 0, 1,
        700, 1160, 2500, 51);
    assert(before_vowel.f1 == 780);
    struct reloq_german_generic_tract before_h = vowel(
        RELOQ_DE_PHONE_A, 0, RELOQ_DE_PHONE_h, 0, 1,
        700, 1160, 2500, 51);
    assert(before_h.f1 == 700);
    struct reloq_german_generic_tract before_j = vowel(
        RELOQ_DE_PHONE_A, 0, RELOQ_DE_PHONE_j, 0, 1,
        700, 1160, 2500, 51);
    assert(before_j.f1 == 820);
    struct reloq_german_generic_tract schwa = vowel(
        RELOQ_DE_PHONE_AT, RELOQ_DE_PHONE_m, 0, 1, 0,
        490, 1500, 2500, 49);
    assert(schwa.f1 == 420 && schwa.f2 == 1270 && schwa.av == 53);
    struct reloq_german_generic_tract nasal = vowel(
        RELOQ_DE_PHONE_Q_a_TILDE, 0, 0, 0, 0,
        730, 1150, 2450, 49);
    assert(nasal.f1 == 624 && nasal.f2 == 960 && nasal.f3 == 2880 &&
           nasal.b1 == 180 && nasal.fnp == 280 && nasal.fnz == 350 &&
           nasal.av == 52);
    struct reloq_german_generic_tract rounded = vowel(
        RELOQ_DE_PHONE_Y, 0, 0, 0, 0,
        410, 1700, 2600, 52);
    assert(rounded.f1 == 400 && rounded.f2 == 1480 &&
           rounded.f3 == 2100 && rounded.av == 57);
    struct reloq_german_generic_tract long_ae = vowel(
        RELOQ_DE_PHONE_Q_E_COLON, 0, 0, 0, 0,
        500, 2005, 2600, 51);
    assert(long_ae.f1 == 500 && long_ae.f2 == 2005 &&
           long_ae.f3 == 2600 && long_ae.av == 51 &&
           long_ae.ah == 34);
    struct reloq_german_generic_tract free = vowel(
        RELOQ_DE_PHONE_Q_aj, RELOQ_DE_PHONE_r, 0, 1, 0,
        820, 1220, 2500, 50);
    assert(free.f2 == 1200 && free.f3 == 2300);
    struct reloq_german_generic_tract out = vowel(
        RELOQ_DE_PHONE_Q_aw, RELOQ_DE_PHONE_t, 0, 1, 0,
        780, 1160, 2500, 50);
    assert(out.f2 == 1300 && out.f3 == 2500);
    struct reloq_german_generic_tract home = vowel(
        RELOQ_DE_PHONE_Q_oj, RELOQ_DE_PHONE_t, 0, 1, 0,
        550, 900, 2500, 53);
    assert(home.f1 == 550 && home.f2 == 900 && home.f3 == 2500);
}

static void test_sonorant_context(void)
{
    struct reloq_german_generic_tract m = sonorant(
        RELOQ_DE_PHONE_m, 0, RELOQ_DE_PHONE_E, 0, 1);
    assert(m.f1 == 300 && m.f2 == 1300 && m.f3 == 2500 &&
           m.b1 == 120 && m.b2 == 150 && m.fnp == 200 &&
           m.fnz == 350 && m.av == 51 && m.ah == 0);
    struct reloq_german_generic_tract n = sonorant(
        RELOQ_DE_PHONE_n, 0, RELOQ_DE_PHONE_I, 0, 1);
    assert(n.f1 == 200 && n.f2 == 1850 && n.f3 == 2800 &&
           n.b2 == 200 && n.av == 48);
    struct reloq_german_generic_tract ng = sonorant(
        RELOQ_DE_PHONE_G, 0, RELOQ_DE_PHONE_U, 0, 1);
    assert(ng.f1 == 300 && ng.f2 == 1600 && ng.f3 == 2600 &&
           ng.b1 == 108 && ng.fnz == 400 && ng.av == 49);
    struct reloq_german_generic_tract l = sonorant(
        RELOQ_DE_PHONE_l, 0, RELOQ_DE_PHONE_I, 0, 1);
    assert(l.f1 == 300 && l.f2 == 1550 && l.f3 == 2900 &&
           l.av == 46 && l.ah == 34);
    struct reloq_german_generic_tract r = sonorant(
        RELOQ_DE_PHONE_r, 0, RELOQ_DE_PHONE_E, 0, 1);
    assert(r.f1 == 400 && r.f2 == 1450 && r.f3 == 2200 &&
           r.av == 0 && r.ah == 27);
    struct reloq_german_generic_tract j = sonorant(
        RELOQ_DE_PHONE_j, 0, RELOQ_DE_PHONE_U, 0, 1);
    assert(j.f1 == 300 && j.f2 == 1900 && j.f3 == 2500 &&
           j.b2 == 250 && j.av == 52);
    struct reloq_german_generic_tract final_m = sonorant(
        RELOQ_DE_PHONE_m, RELOQ_DE_PHONE_A, 0, 1, 0);
    assert(final_m.f1 == 200 && final_m.f2 == 750 && final_m.av == 42);
    struct reloq_german_generic_tract boundary_m = sonorant(
        RELOQ_DE_PHONE_m, RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_E, 1, 0);
    assert(boundary_m.f1 == 200 && boundary_m.f2 == 750);
}

static void test_invalid(void)
{
    struct reloq_german_phone_node node = {.phone = RELOQ_DE_PHONE_s};
    struct reloq_german_generic_tract out = {.f1 = 123};
    assert(reloq_german_generic_sonorant_tract(&node, &out) == -1);
    assert(out.f1 == 123);
    assert(reloq_german_generic_vowel_tract(&node, 700, 1160, 2500, 51,
                                             &out) == -1);
    assert(out.f1 == 123);
    assert(reloq_german_generic_sonorant_tract(NULL, &out) == -1);
    assert(reloq_german_generic_vowel_tract(&node, 700, 1160, 2500, 51,
                                             NULL) == -1);
}

int main(void)
{
    test_vowel_context();
    test_sonorant_context();
    test_invalid();
    puts("generic tract context: vowel, sonorant and boundary checks passed");
    return 0;
}
