#ifndef RELOQ_GERMAN_TERMINAL_STOP_TRACT_H
#define RELOQ_GERMAN_TERMINAL_STOP_TRACT_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"
#include "reloq_german_obstruent_tract.h"

/* Context form of the measured final closure, for the connected planner.
 * The caller establishes a phrase boundary; no spelling or stress filter. */
int reloq_german_terminal_stop_tract_plan(
    const struct reloq_german_phone_node *node,
    struct reloq_german_obstruent_tract *out);

/* Oral-tract closure and release targets for an isolated, explicitly
 * primary-stressed German [h|p|t|k]V[p|t|k] syllable. The ten covered vowel
 * symbols are A/E/I/O/U/a/e/i/o/u. The caller supplies the vowel's absolute
 * second positive AV target as closure_tick, from its own timing model.
 *
 * This rule appends F1-F5, B1-B5, FNP/FNZ, FTP/FTZ, OQ, DI and FL targets
 * from closure through the stop's release. It deliberately does not append
 * AV, AH, AF, noise resonators, F0, or the terminal quiet tail; those belong
 * to other source, prosody and scheduling rules. The builder's same-stream
 * events must be ordered through closure before calling this function.
 *
 * The rule is measured for one isolated German word at the default 11,025-Hz
 * reference voice and explicit primary stress. The phone nodes omit controls
 * and punctuation, so the caller must separately establish this context.
 * No other voice, speech rate, connected context, or sampling rate is
 * asserted. Return 0 on success, 1 for uncovered context, -1 for invalid
 * arguments or a builder/tick error. Failure leaves the builder and
 * *release_tick unchanged. */
int reloq_german_terminal_stop_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t closure_tick,
    struct reloq_target_builder *builder, int64_t *release_tick);

#endif
