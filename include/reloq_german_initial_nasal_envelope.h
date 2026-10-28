#ifndef RELOQ_GERMAN_INITIAL_NASAL_ENVELOPE_H
#define RELOQ_GERMAN_INITIAL_NASAL_ENVELOPE_H

#include "reloq_german_phone_sequence.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_target_builder.h"

/* Ten phrase-wide targets accompanying the initial-nasal gesture for one
 * isolated, explicitly primary-stressed German m/n V [p|t|k]. utterance,
 * V=AEIOUaeiou, at the default 11,025-Hz voice and rate. The four F0
 * anchors span the vowel from nasal onset_tick to its second positive AV
 * target at closure_tick. FL, FTP and FTZ have constant target pairs from
 * origin_tick to the final-stop release_tick. These future knots must be
 * present before evaluating the first nasal array window.
 *
 * All ticks are absolute. onset_tick must be the first-send end returned
 * by initial_nasal_append (85 ticks after m, 70 after n). The vowel and
 * terminal-stop gestures own their own formants, widths and source targets;
 * the caller must merge those gestures with this one for complete controls.
 * The measured period ending schedules 460 quiet-tail ticks after release.
 *
 * Returns 0 on success, 1 for an uncovered phone/stress/terminal shape,
 * or -1 for invalid timing/storage. Failure leaves the builder unchanged. */
int reloq_german_initial_nasal_envelope_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    enum reloq_german_terminal_mark mark,
    int64_t origin_tick, int64_t onset_tick, int64_t closure_tick,
    int64_t release_tick, struct reloq_target_builder *builder);

#endif
