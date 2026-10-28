#ifndef RELOQ_GERMAN_VOWEL_NUCLEUS_H
#define RELOQ_GERMAN_VOWEL_NUCLEUS_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* An authored, deliberately narrow German voiced-nucleus gesture for the
 * default Apple-reference voice at the nominal 1 ms target timebase. The
 * supported isolated, primary-stressed, one-syllable SPR word is
 * [h] V [p|t|k], where V is A/E/I/O/U/a/e/i/o/u. The caller supplies the
 * absolute onset tick of V and must preserve the surrounding punctuation,
 * controls and word boundaries separately: phone nodes omit them.
 *
 * Appends three first-formant targets at onset, plus AV and AH source targets
 * through the vowel's release or following stop closure. If terminal_period
 * is 1, also appends the four measured period F0 contour targets (only the
 * last three after an h, whose precursor F0 belongs to the h gesture).
 * A caller must establish actual punctuation before setting this flag.
 * It does not render the h, stop, other formants, consonant noise, terminal
 * fade, or audio; it is not a complete duration/prosody model. The returned
 * closure offset
 * is the second AV target relative to onset. In a coda this is also the
 * abrupt AV-off target. For a bare vowel the AV-off target comes 60 ticks
 * later, so closure_offset is not a phoneme boundary in that case.
 *
 * Returns 0 on success, 1 when shape/stress/context is uncovered, and -1
 * on invalid arguments or builder error. The builder and *closure_offset
 * remain unchanged on either failure. */
int reloq_german_vowel_nucleus_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t onset_tick, int terminal_period,
    struct reloq_target_builder *builder, int64_t *closure_offset);

#endif
