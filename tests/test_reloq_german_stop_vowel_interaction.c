#include "reloq_german_stop_vowel_interaction.h"

#include <assert.h>
#include <limits.h>

/* Values below are independently captured, *held-out* Apple ECI vowel
 * onset/window/Formant measurements. The calibration used pAt and pa{p};
 * these tests use other onset/coda pairs. */
static void check(enum reloq_german_phone onset,
                  enum reloq_german_phone vowel,
                  enum reloq_german_phone coda,
                  struct reloq_german_vowel_onset_prediction prediction,
                  struct reloq_german_vowel_onset_prediction expected)
{
    struct reloq_german_phone_node phones[3] = {0};
    for (size_t i = 0; i < 3; ++i) {
        phones[i].stress = 1;
        phones[i].stress_explicit = 1;
        phones[i].same_syllable_left = i != 0;
        phones[i].same_syllable_right = i != 2;
    }
    phones[0].phone = onset;
    phones[1].phone = vowel;
    phones[2].phone = coda;
    struct reloq_german_vowel_onset_prediction actual = {0};
    assert(reloq_german_stop_vowel_interaction(
               phones, 3, 1, &prediction, &actual) == 0);
    assert(actual.anchor_tick == expected.anchor_tick);
    assert(actual.first_send_end == expected.first_send_end);
    assert(actual.f1 == expected.f1 && actual.f2 == expected.f2 &&
           actual.f3 == expected.f3);
}

int main(void)
{
    check(RELOQ_DE_PHONE_t, RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_t,
          (struct reloq_german_vowel_onset_prediction)
              {89,136,780,1440,2500},
          (struct reloq_german_vowel_onset_prediction)
              {89,136,780,1300,2500});
    check(RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_t,
          (struct reloq_german_vowel_onset_prediction)
              {78,125,780,1390,2400},
          (struct reloq_german_vowel_onset_prediction)
              {78,125,780,1250,2400});
    check(RELOQ_DE_PHONE_t, RELOQ_DE_PHONE_e, RELOQ_DE_PHONE_k,
          (struct reloq_german_vowel_onset_prediction)
              {113,249,370,2150,2700},
          (struct reloq_german_vowel_onset_prediction)
              {113,251,370,2150,2700});
    check(RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_i, RELOQ_DE_PHONE_t,
          (struct reloq_german_vowel_onset_prediction)
              {93,216,265,2150,2900},
          (struct reloq_german_vowel_onset_prediction)
              {93,219,265,2150,2900});
    struct reloq_german_phone_node weak[3] = {0};
    weak[0].phone = RELOQ_DE_PHONE_p;
    weak[1].phone = RELOQ_DE_PHONE_a;
    weak[2].phone = RELOQ_DE_PHONE_t;
    for (size_t i = 0; i < 3; ++i) {
        weak[i].stress_explicit = 1;
        weak[i].same_syllable_left = i != 0;
        weak[i].same_syllable_right = i != 2;
    }
    const struct reloq_german_vowel_onset_prediction predicted =
        {65, INT64_MAX, 800, 1000, 2500};
    struct reloq_german_vowel_onset_prediction result = {0};
    assert(reloq_german_stop_vowel_interaction(
               weak, 3, 1, &predicted, &result) == 1);
    weak[0].stress = weak[1].stress = weak[2].stress = 1;
    assert(reloq_german_stop_vowel_interaction(
               weak, 3, 1, &predicted, &result) == -1);
    assert(result.first_send_end == 0);
    assert(reloq_german_stop_vowel_interaction(
               weak, 3, 1, NULL, &result) == -1);
    return 0;
}
