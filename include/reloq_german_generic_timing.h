#ifndef RELOQ_GERMAN_GENERIC_TIMING_H
#define RELOQ_GERMAN_GENERIC_TIMING_H

#include "reloq_german_phone_sequence.h"

#include <stdint.h>

/* One p/t/k onset whose immediately preceding phone, if any, is a vowel.
 * Ticks are measured from the beginning of this phone. For an initial onset,
 * the noise burst begins after five ticks of initial closure. For an onset
 * after a vowel in the same word, closure begins at the preceding vowel's
 * release. The next vowel begins at total_ticks. No word spelling is used.
 *
 * Return 1 for a supported plain CV/V.CV context, zero for another context,
 * and -1 for invalid arguments. The output is unchanged unless 1 is returned.
 */
struct reloq_german_generic_stop_timing {
    int32_t total_ticks;
    int32_t burst_at_ticks;
    int32_t aspiration_at_ticks;
};

int reloq_german_generic_stop_onset_timing(
    const struct reloq_german_phone_node *node,
    struct reloq_german_generic_stop_timing *out);

/* A same-word V.CCV transition with p/t/k at both sides of the syllable
 * boundary. The two closures are planned together, including the first
 * stop's release and the second stop's burst/aspiration. This narrow class
 * covers an explicit .0/.1 first-syllable A/E/U and a following .0 A or
 * .0/.1 I; other vowels, secondary stress and word boundaries are untouched.
 * `coda_index` identifies the first stop. Return 1 if covered, 0 if not,
 * -1 for invalid arguments. The output is unchanged unless 1 is returned. */
struct reloq_german_generic_internal_stop_pair {
    int32_t coda_ticks;
    int32_t coda_burst_at_ticks;
    uint8_t merge_coda_with_onset;
    struct reloq_german_generic_stop_timing onset;
};

int reloq_german_generic_internal_stop_pair(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t coda_index,
    struct reloq_german_generic_internal_stop_pair *out);

/* In an internal V.CV sequence the first vowel is compressed before the
 * stop closure. The caller must confirm the following p/t/k has a vowel in
 * its own syllable; the current node alone has no two-phones-ahead field.
 * This helper then checks the direct next-syllable stop in the same word.
 * Return 1 for a supported phone, zero if inapplicable, -1 for invalid args.
 */
int reloq_german_generic_pre_stop_vowel_ticks(
    const struct reloq_german_phone_node *node, int32_t *out);

/* Word-initial or intervocalic b/d/g release toward a vowel. The low voiced closure
 * remains level until burst_at_ticks, then the brief place burst ends at
 * total_ticks. Unlike p/t/k, weak b/d/g onsets contract by place as well as
 * by vowel context. Same-word intervocalic onsets use their own closure
 * lengths; rounded, reduced and nasal vowels retain distinct classes.
 * Return 1 for the calibrated class, 0 for another context, -1 on bad args. */
struct reloq_german_generic_voiced_onset_timing {
    int32_t total_ticks;
    int32_t burst_at_ticks;
};

int reloq_german_generic_voiced_onset_timing(
    const struct reloq_german_phone_node *node,
    struct reloq_german_generic_voiced_onset_timing *out);

#endif
