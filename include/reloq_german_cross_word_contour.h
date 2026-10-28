#ifndef RELOQ_GERMAN_CROSS_WORD_CONTOUR_H
#define RELOQ_GERMAN_CROSS_WORD_CONTOUR_H

#include "reloq_german_cross_word_landmarks.h"
#include "reloq_target_builder.h"

/* Word-role pitch and voiced source amplitude in one connected German SPR
 * sentence: two through twenty explicitly primary-stressed p/t/k + vowel
 * words, each optionally ending in p/t/k, separated by verified `2 controls.
 * Times come from the independent phrase landmark planner on a single clock.
 * The word array must correspond to the already validated phone sequence;
 * this component cannot inspect the missing SPR control or infer stress.
 *
 * F0, AV and AH are the only target streams authored here. AF, resonator
 * source gains, phoneme duration, send windows and final source release are
 * separate gestures. 0 modeled, 1 unmeasured class/mark, -1 invalid input or
 * target storage; the builder is unchanged for either nonzero status. */
int reloq_german_cross_word_contour_append(
    const struct reloq_german_cross_word_word *words, size_t word_count,
    enum reloq_german_terminal_mark mark, int64_t planned_ticks,
    struct reloq_target_builder *builder);

#endif
