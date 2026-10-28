#ifndef RELOQ_GERMAN_TERMINAL_SOURCE_H
#define RELOQ_GERMAN_TERMINAL_SOURCE_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Source/noise release for one explicitly primary-stressed, one-syllable
 * German SPR group [h|p|t|k] V [p|t|k], where V is A/E/I/O/U/a/e/i/o/u.
 * `vowel_closure_tick` is the absolute second positive AV target of V,
 * supplied by a preceding timing model. This function calculates the final
 * active release origin after any coda and appends AV/AH/AF/A1F..A5F/AB
 * points at the origin and origin+60. Both same-time AH targets matter.
 *
 * The source release occupies the inclusive 61 ticks [origin, origin+60].
 * Silence after it and F0/other controls are independent. Observed final
 * array sends are longer than this active release and are not phone lengths.
 * A caller must isolate the SPR group and validate punctuation/controls,
 * because the phone nodes omit them. The rule is verified for a period,
 * question, exclamation and comma only in that controlled setup.
 *
 * Returns 0 on success, 1 for unsupported context, -1 on invalid input or
 * builder error. The builder and *release_start_tick remain unchanged on
 * either failure. A bare-vowel planner must avoid duplicating its own
 * terminal AV/AH points when integrating this release model. */
int reloq_german_terminal_source_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t vowel_closure_tick,
    struct reloq_target_builder *builder, int64_t *release_start_tick);

#endif
