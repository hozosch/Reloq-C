#ifndef RELOQ_GERMAN_NASAL_VOWEL_H
#define RELOQ_GERMAN_NASAL_VOWEL_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Vowel continuation after the independent initial /m,n/ gesture in one
 * isolated, explicitly primary-stressed German m/n V [p|t|k] syllable.
 * V is A/E/I/O/U/a/e/i/o/u; the voice is the default 11,025-Hz German
 * reference. origin_tick is the word's absolute onset, normally zero.
 * The caller must establish a terminal period separately: phone nodes do
 * not retain punctuation, and other terminal marks are not measured here.
 *
 * Append the 48-target second numeric send: vowel tract and source targets
 * including the return of nasal FNZ to 200 and their hold endpoints. The
 * same-tick AV=0 and AH=0 targets then close the source at the start of
 * the next numeric send, after the preceding positive AV/AH targets.
 * The initial nasal targets,
 * phrase F0, terminal coda and FL/FTP/FTZ release are separate gestures.
 * All three outputs are offsets from origin_tick. vowel_target_offset is
 * the first F1/F2/F3/FNZ=200 target; hold_end_offset is the second numeric
 * send end; closure_offset is the AV-off/coda transition tick. These are
 * measured acoustic scheduler landmarks, not phoneme durations.
 *
 * Returns 0 for this context, 1 for an uncovered valid phone shape, -1
 * for invalid input, overflow or builder error. On failure the builder and
 * all three outputs remain unchanged. */
int reloq_german_nasal_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t origin_tick, struct reloq_target_builder *builder,
    int64_t *vowel_target_offset, int64_t *hold_end_offset,
    int64_t *closure_offset);

#endif
