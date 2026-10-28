#ifndef RELOQ_GERMAN_WITHIN_WORD_F0_H
#define RELOQ_GERMAN_WITHIN_WORD_F0_H

#include "reloq_german_syllable_plan.h"
#include "reloq_target_builder.h"

/* Two-syllable weak+primary F0 gesture for CVC.CVC within one German word.
 * The voiced nucleus of the second syllable spans [vowel_onset, closure).
 * The first syllable has no separate F0 targets; its coda creates a short
 * lead into the primary syllable. Only p/k -> t with ten simple vowel
 * qualities and p/t/k final stop is validated. This is one acoustic track,
 * not a complete CVC.CVC synthesizer. 0 success, 1 unmodeled, -1 invalid. */
int reloq_german_within_word_f0_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    int64_t first_coda_send_end, int64_t second_vowel_onset,
    int64_t second_vowel_closure, struct reloq_target_builder *builder);

#endif
