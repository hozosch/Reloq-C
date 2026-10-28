#ifndef RELOQ_GERMAN_FRICATIVE_VOWEL_H
#define RELOQ_GERMAN_FRICATIVE_VOWEL_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Vowel onset and voice closure for one isolated, explicitly stressed
 * German V[s|f] or hV[s|f] group at nominal 11,025 Hz. `onset_tick` is 0
 * without h or 80 with h. `first_send_end_tick` is a scheduler landmark,
 * not a phone duration; the caller authors its other steady-state controls.
 * Its positive AV and AH=34 targets at closure precede the coda gesture's
 * same-tick zeros. The coda gesture owns F1-F5 and noise from closure onward. This helper owns
 * only initial F1-F3, AV and AH, and does not author F0 or initial h.
 *
 * Returns 0 on success, 1 if outside this controlled context, -1 on invalid
 * input or builder failure. Outputs and builder stay unchanged on failure. */
int reloq_german_fricative_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t onset_tick,
    struct reloq_target_builder *builder, int64_t *closure_tick,
    int64_t *first_send_end_tick);

#endif
