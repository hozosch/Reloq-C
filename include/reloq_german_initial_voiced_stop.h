#ifndef RELOQ_GERMAN_INITIAL_VOICED_STOP_H
#define RELOQ_GERMAN_INITIAL_VOICED_STOP_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Initial /b,d,g/ closure and burst in the default German 11,025-Hz voice.
 * The narrow SPR shape is one explicitly primary-stressed syllable
 * [b|d|g] V [p|t|k], or the CV probe form without a coda, where V is one
 * of AEIOUaeiou. Natural spelling, punctuation, and prosody are gated by
 * the caller. An intervening syllable or word is not covered.
 *
 * Appends 26 ordered first-send points: initial F1–F3 at origin/onset;
 * AV=20 and TL=35 through the closure; same-tick off/on burst transitions
 * in AV/TL/AH/AF and exactly one noise resonator (AB/A5F/A2F/A3F).
 * The caller supplies origin; both returned offsets are relative to it.
 * The onset is the first send end and
 * vowel formant anchor; positive vowel AV follows one tick later. The
 * first send begins at origin and ends at origin + onset_offset.
 *
 * The surrounding voice owns F0, FL/FTP/FTZ through final release,
 * F4/F5, bandwidths, nasal poles, inactive noise resonators, OQ/DI,
 * the vowel and coda. These are needed for all 78 first-send Apple targets.
 * Do not append the initial F1–F3 a second time from another gesture.
 *
 * Return 0 on covered success, 1 on uncovered context, -1 on invalid input
 * or storage/tick failure. Both offsets and all builder storage are
 * unchanged on failure. */
int reloq_german_initial_voiced_stop_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t origin_tick, struct reloq_target_builder *builder,
    int64_t *vowel_onset_offset, int64_t *burst_offset);

#endif
