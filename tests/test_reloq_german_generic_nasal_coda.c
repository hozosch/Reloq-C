#include "reloq_german_generic_nasal_coda.h"

#include <assert.h>
#include <stdio.h>

static struct reloq_german_phone_node coda(
    enum reloq_german_phone vowel, enum reloq_german_phone nasal,
    enum reloq_german_phone next, int same_word)
{
    struct reloq_german_phone_node node = {0};
    node.phone = nasal;
    node.left_phone = vowel;
    node.right_phone = next;
    node.same_syllable_left = 1;
    node.same_word_left = 1;
    node.same_word_right = (uint8_t)same_word;
    return node;
}

static void vowel_contexts(void)
{
    const enum reloq_german_phone vowels[] = {
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_E, RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O, RELOQ_DE_PHONE_U
    };
    const int32_t previous_av[] = {51, 52, 53, 53, 54};
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        for (int nasal = 0; nasal < 3; ++nasal) {
            const enum reloq_german_phone p = nasal == 0 ?
                RELOQ_DE_PHONE_m : nasal == 1 ? RELOQ_DE_PHONE_n :
                RELOQ_DE_PHONE_G;
            struct reloq_german_generic_nasal_coda result = {0};
            struct reloq_german_phone_node node = coda(
                vowels[i], p, RELOQ_DE_PHONE_NONE, 0);
            assert(reloq_german_generic_nasal_coda_plan(
                &node, 1, 1, 0, previous_av[i], 2, &result) == 1);
            /* This fixture is unstressed: final n has a 110-tick body. */
            assert(result.duration_ticks == (nasal == 2 ? 140 : nasal == 1 ? 110 : 120));
            if (nasal == 0) {
                int base = i == 0 ? 42 : 45;
                assert(result.start_av == base + 2);
                assert(result.end_av == base);
                assert(result.taper_percent == 0);
            } else {
                assert(result.start_av == 50 &&
                       result.taper_av == 44 &&
                       result.taper_percent == 80 &&
                       result.end_av == (nasal == 2 ? 0 : 42));
            }
            node = coda(vowels[i], p, RELOQ_DE_PHONE_A, 1);
            assert(reloq_german_generic_nasal_coda_plan(
                &node, 1, 0, 1, previous_av[i], 2, &result) == 1);
            assert(result.duration_ticks == (nasal == 1 ? 30 : 60));
            assert(result.f1_override == (nasal == 0 ? 300 : 0));
            assert(result.start_av == (nasal == 0 ? previous_av[i] : 50));
            assert(result.end_av == 50);
            node = coda(vowels[i], p, RELOQ_DE_PHONE_h, 0);
            assert(reloq_german_generic_nasal_coda_plan(
                &node, 1, 0, 0, previous_av[i], 2, &result) == 1);
            assert(result.duration_ticks == (nasal == 2 ? 81 : 70));
            assert(result.f1_override == (nasal == 0 ? 300 : 0));
            assert(result.start_av == (nasal == 0 ? 53 :
                                       nasal == 2 ? 51 : 50));
            assert(result.end_av == result.start_av);
        }
}

static void exclusions(void)
{
    struct reloq_german_generic_nasal_coda untouched = {
        987, 543, 123, 456, 789, 20
    };
    struct reloq_german_phone_node node = coda(
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_n, RELOQ_DE_PHONE_t, 1);
    assert(reloq_german_generic_nasal_coda_plan(
        &node, 1, 0, 0, 51, 2, &untouched) == 0);
    assert(untouched.duration_ticks == 987);
    node.phone = RELOQ_DE_PHONE_l;
    assert(reloq_german_generic_nasal_coda_plan(
        &node, 1, 1, 0, 51, 2, &untouched) == 0);
    node.phone = RELOQ_DE_PHONE_n;
    assert(reloq_german_generic_nasal_coda_plan(
        &node, 0, 1, 0, 51, 2, &untouched) == 0);
    assert(reloq_german_generic_nasal_coda_plan(
        &node, 1, 1, 0, 51, -1, &untouched) == -1);
    assert(reloq_german_generic_nasal_coda_plan(
        NULL, 1, 1, 0, 51, 2, &untouched) == -1);
}

int main(void)
{
    vowel_contexts();
    exclusions();
    puts("generic nasal coda: five vowel classes x m/n/G x three boundary classes");
    return 0;
}
