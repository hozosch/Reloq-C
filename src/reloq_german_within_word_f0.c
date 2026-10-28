#include "reloq_german_within_word_f0.h"

#include <limits.h>

static int vowel(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_A || phone == RELOQ_DE_PHONE_E ||
        phone == RELOQ_DE_PHONE_I || phone == RELOQ_DE_PHONE_O ||
        phone == RELOQ_DE_PHONE_U || phone == RELOQ_DE_PHONE_a ||
        phone == RELOQ_DE_PHONE_e || phone == RELOQ_DE_PHONE_i ||
        phone == RELOQ_DE_PHONE_o || phone == RELOQ_DE_PHONE_u;
}

static int front(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_E || phone == RELOQ_DE_PHONE_I ||
           phone == RELOQ_DE_PHONE_e || phone == RELOQ_DE_PHONE_i;
}

int reloq_german_within_word_f0_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    int64_t first_coda_send_end, int64_t second_vowel_onset,
    int64_t second_vowel_closure, struct reloq_target_builder *builder)
{
    if (!phones || !syllables || !builder || first_coda_send_end < 0 ||
        second_vowel_onset < 0 ||
        second_vowel_closure <= second_vowel_onset ||
        second_vowel_closure > INT64_MAX - 460) return -1;
    if (phone_count != 6 || syllable_count != 2 ||
        syllables[0].first_phone != 0 || syllables[1].first_phone != 3 ||
        syllables[0].phone_count != 3 ||
        syllables[1].phone_count != 3 ||
        syllables[0].word_index != syllables[1].word_index ||
        syllables[0].boundary_after != RELOQ_DE_BOUNDARY_SYLLABLE ||
        syllables[1].boundary_after != RELOQ_DE_BOUNDARY_UTTERANCE ||
        syllables[0].stress != 0 || syllables[1].stress != 1 ||
        !syllables[0].stress_explicit ||
        !syllables[1].stress_explicit ||
        syllables[0].onset_count != 1 ||
        syllables[0].nucleus_count != 1 ||
        syllables[0].coda_count != 1 ||
        syllables[1].onset_count != 1 ||
        syllables[1].nucleus_count != 1 ||
        syllables[1].coda_count != 1 ||
        (phones[0].phone != RELOQ_DE_PHONE_p &&
         phones[0].phone != RELOQ_DE_PHONE_t &&
         phones[0].phone != RELOQ_DE_PHONE_k) ||
        !vowel(phones[1].phone) ||
        (phones[2].phone != RELOQ_DE_PHONE_p &&
         phones[2].phone != RELOQ_DE_PHONE_k) ||
        phones[3].phone != RELOQ_DE_PHONE_t ||
        !vowel(phones[4].phone) ||
        (phones[5].phone != RELOQ_DE_PHONE_p &&
         phones[5].phone != RELOQ_DE_PHONE_t &&
         phones[5].phone != RELOQ_DE_PHONE_k)) return 1;
    const int64_t span = second_vowel_closure - second_vowel_onset;
    if (span > INT64_MAX / 99) return -1;
    const int lead = (phones[2].phone == RELOQ_DE_PHONE_p ? 7 : 28) +
        front(phones[4].phone);
    if (first_coda_send_end > INT64_MAX - lead ||
        first_coda_send_end + lead >= second_vowel_onset + span * 35 / 100)
        return -1;
    const struct reloq_target_event events[] = {
        {RELOQ_TARGET_F0, first_coda_send_end + lead, 1043},
        {RELOQ_TARGET_F0, second_vowel_onset + span * 35 / 100, 1293},
        {RELOQ_TARGET_F0, second_vowel_onset + span * 70 / 100, 793},
        {RELOQ_TARGET_F0, second_vowel_onset + span * 99 / 100, 740}
    };
    return reloq_target_builder_append(builder, 0, events,
                                        sizeof events / sizeof events[0]);
}
