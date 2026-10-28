#ifndef RELOQ_GERMAN_TRANSITION_DURATION_H
#define RELOQ_GERMAN_TRANSITION_DURATION_H

#include "reloq_german_phone_sequence.h"

/* Vowel contact, including word edges. The caller owns its time budget;
 * glottal excitation does not replace the vowel-pair duration decision. */
int reloq_german_transition_vv_ticks(
    const struct reloq_german_phone_node *left,
    const struct reloq_german_phone_node *right, int32_t *out);

/* Outgoing oral contact between a fricative/nasal and a consonant.
 * This interval belongs after the first consonant's body; it is neither
 * an extra hold of that body nor the following vowel's CV transition. */
int reloq_german_transition_cc_ticks(
    const struct reloq_german_phone_node *left,
    const struct reloq_german_phone_node *right, int32_t *out);

/* Independently authored behavior rules for intraword CV and VC intervals.
 * Return 1 and nominal millisecond ticks for a supported context, 0 with
 * unchanged output for an unsupported context, or -1 for invalid pointers.
 * Stops' pre-vowel aspiration remains with the separate release planner.
 * No original phone codes, stream topology or rule-machine ABI are used. */
int reloq_german_transition_cv_ticks(
    const struct reloq_german_phone_node *consonant, int32_t *out);

int reloq_german_transition_vc_ticks(
    const struct reloq_german_phone_node *vowel,
    const struct reloq_german_phone_node *consonant, int32_t *out);

#endif
