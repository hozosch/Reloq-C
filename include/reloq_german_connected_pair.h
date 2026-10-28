#ifndef RELOQ_GERMAN_CONNECTED_PAIR_H
#define RELOQ_GERMAN_CONNECTED_PAIR_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

#include <stdint.h>

/* One explicitly primary-stressed, simple-syllable pair with the nonfinal
 * boundary `2 between the words: [h|p|t|k]V[p|t|k] `2
 * [h|p|t|k]V[p|t|k]. This is the default 11,025 Hz German voice. A NONE
 * first initial denotes a vowel-initial word. V is A/E/I/O/U/a/e/i/o/u.
 * The caller validates text, SPR, stress, punctuation and the second word's
 * independently planned final-vowel interval. These functions neither
 * concatenate isolated words nor insert a generic silence at the boundary.
 * Return 0 on covered success, 1 on an uncovered phone shape, -1 on invalid
 * input/overflow/builder failure. Output is unchanged on failure. */
struct reloq_german_connected_pair {
    enum reloq_german_phone first_initial;
    enum reloq_german_phone first_vowel;
    enum reloq_german_phone first_coda;
    enum reloq_german_phone second_initial;
};

struct reloq_german_connected_landmarks {
    int64_t first_vowel_ticks;
    int64_t first_closure_tick;
    int64_t first_coda_send_end;
    int64_t second_early_f0_tick;
    int merge_identical_stops;
};

/* Query first, before its first numeric send endpoint is known. In h-first
 * words that endpoint itself depends on the new nonfinal vowel duration. */
int reloq_german_connected_first_duration(
    const struct reloq_german_connected_pair *pair,
    int64_t *first_vowel_ticks);

/* Plan the nonfinal first vowel/coda before the second vowel anchor is
 * known. This breaks the acoustic planning dependency: call this, then
 * kt_onset_landmarks, then plan the second vowel's final release. */
int reloq_german_connected_first_closure(
    const struct reloq_german_connected_pair *pair,
    int64_t first_vowel_onset, int64_t first_send_end,
    int64_t *first_closure_tick, int64_t *first_coda_send_end);

/* The onset and first-send end are the caller's already planned first
 * numeric gesture. For p/t/k, this ends before the vowel anchor. For h,
 * the first send includes the h plus vowel and ends between vowel anchor
 * and AV closure. For a bare first vowel, first_send_end is -1 and onset
 * is zero. The second vowel onset is the caller's independently planned
 * connected-onset anchor; its closure is not needed for this boundary plan.
 * All quantities are absolute 1 ms model ticks from the phrase origin. */
int reloq_german_connected_pair_landmarks(
    const struct reloq_german_connected_pair *pair,
    int64_t first_vowel_onset, int64_t first_send_end,
    int64_t second_vowel_onset,
    struct reloq_german_connected_landmarks *out);

/* Append the two nonfinal and four final F0 targets for the same pair.
 * second_vowel_closure must be planned independently with the second word's
 * final vowel rule. This operation is transactional in the target builder.
 * The first F0 values and percentages differ from a final isolated word. */
int reloq_german_connected_pair_f0_append(
    const struct reloq_german_connected_pair *pair,
    int64_t first_vowel_onset, int64_t first_send_end,
    int64_t second_vowel_onset, int64_t second_vowel_closure,
    struct reloq_target_builder *builder,
    struct reloq_german_connected_landmarks *out);

/* First nonfinal /k/ followed by /t/: independently measured coda-closure,
 * place-release and short aspiration targets. The closure is derived from
 * the first vowel's place class; the release depends on the following /t/.
 * Values and timing are absolute phrase ticks. The caller must supply the
 * first vowel's positive AV/AH closure targets and its preceding nucleus,
 * then continue with a separate onset gesture at *second_onset_origin.
 * Unlike the preceding landmark/F0 functions, this acoustic gesture is
 * intentionally narrow: other consonant pairs return uncovered. */
int reloq_german_connected_kt_coda_append(
    const struct reloq_german_connected_pair *pair,
    int64_t first_closure_tick, struct reloq_target_builder *builder,
    int64_t *second_onset_origin);

/* The next /t/ onset after that /k/ coda. The four initial gesture
 * landmarks are calculated from the preceding release and the following
 * vowel class; the burst/aspiration is not an isolated-word onset pasted
 * after a silent gap. The second vowel's own formants/source and final
 * coda remain separate gestures. Return the initial send endpoint and
 * vowel acoustic anchor in absolute phrase ticks. */
int reloq_german_connected_kt_onset_landmarks(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick,
    int64_t *first_send_end, int64_t *second_vowel_onset);

/* second_final_release_tick comes from the independently planned second
 * vowel and final coda. Query the preceding landmarks function first, then
 * calculate that final release and append this initial gesture. */
int reloq_german_connected_kt_onset_append(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick, int64_t second_final_release_tick,
    struct reloq_target_builder *builder,
    int64_t *first_send_end, int64_t *second_vowel_onset);

#endif
