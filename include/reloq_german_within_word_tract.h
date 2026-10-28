#ifndef RELOQ_GERMAN_WITHIN_WORD_TRACT_H
#define RELOQ_GERMAN_WITHIN_WORD_TRACT_H

#include "reloq_german_phone_sequence.h"
#include "reloq_german_syllable_plan.h"
#include "reloq_german_within_word_landmarks.h"
#include "reloq_target_builder.h"

/* German default-voice oral-tract gestures for one explicitly annotated
 * two-syllable word, .0[p|t|k]V[p|k].1tW[p|t|k]. V and W are drawn
 * independently from A/E/I/O/U/a/e/i/o/u.
 * The first weak vowel's formants, its place-dependent closure and the
 * second stressed vowel are separate contextual gestures; all timing is
 * supplied by the independently planned landmarks. No word or recording
 * lookup is performed here. The output includes F1..F5, B1..B5,
 * FNP/FNZ, FTP/FTZ, FL, TL, OQ and DI. Source/noise and F0 belong to
 * other components. The terminal period is an orchestration precondition.
 *
 * 0 appended, 1 uncovered phonetic shape, -1 invalid arguments/timing or
 * builder error. Append is transactional on failure. */
int reloq_german_within_word_tract_append(
    const struct reloq_german_phone_node *phones, size_t count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_within_word_landmarks *landmarks,
    struct reloq_target_builder *builder);

#endif
