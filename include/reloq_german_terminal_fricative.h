#ifndef RELOQ_GERMAN_TERMINAL_FRICATIVE_H
#define RELOQ_GERMAN_TERMINAL_FRICATIVE_H

#include "reloq_german_phone_sequence.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_target_builder.h"

/* Coda tract, frication and active fade for one explicitly stressed,
 * isolated [h|p|t|k] V [s|f] German SPR group at nominal 11,025 Hz.
 * The caller supplies the absolute vowel closure (its second positive AV
 * target), including any initial-consonant offset. The coda starts at that
 * same tick. The source fades for 30 ticks after release_tick; a separate
 * phrase model provides the following quiet terminal send.
 *
 * The preceding vowel rule must supply its positive AV and AH=34 targets at
 * closure. This gesture then supplies the same-tick zero targets and all
 * coda controls through the fade. The phone nodes omit punctuation, so the
 * caller must separately validate the isolated terminal context.
 * Returns 0 on success, 1 if outside this scope, -1 on invalid input or
 * builder failure. On failure, builder and *release_tick stay unchanged. */
int reloq_german_terminal_fricative_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t closure_tick,
    struct reloq_target_builder *builder, int64_t *release_tick);

/* The same coda gesture with a measured phrase-final punctuation window.
 * The coda targets and release_tick are independent of the mark. The end
 * tick is release_tick + 430 for implicit/period/question/exclamation/colon,
 * +280 for semicolon, and +180 for comma/ellipsis. Each window starts with
 * the 30-tick fade authored above; the rest has zero source amplitude.
 * The original append API keeps its period-compatible meaning and callers.
 * On nonzero result all outputs and builder targets remain unchanged. */
int reloq_german_terminal_fricative_append_marked(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t closure_tick,
    enum reloq_german_terminal_mark mark,
    struct reloq_target_builder *builder, int64_t *release_tick,
    int64_t *end_tick);

#endif
