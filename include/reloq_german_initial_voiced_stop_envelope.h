#ifndef RELOQ_GERMAN_INITIAL_VOICED_STOP_ENVELOPE_H
#define RELOQ_GERMAN_INITIAL_VOICED_STOP_ENVELOPE_H

#include "reloq_german_phone_sequence.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_target_builder.h"

/* The 52 first-send targets not owned by initial_voiced_stop_append().
 * Together the two gestures predict the complete 78 accepted first-send
 * target insertions in one primary-stressed [b|d|g]V[p|t|k]. period case,
 * V=AEIOUaeiou, at the default German 11,025-Hz voice/rate. CV probe forms
 * are also accepted with release_tick == closure_tick. The caller separately
 * validates a single, isolated SPR group and keeps a continuous Klatt state.
 *
 * burst_tick/onset_tick/closure_tick/release_tick are absolute on the same
 * timeline as origin_tick; closure is the vowel's second positive AV target.
 * The initial three FL/FTP/FTZ streams carry future endpoints at release;
 * F0 needs the complete vowel interval before the first send is rendered.
 * F4/F5, B1–B5, FNP/FNZ, OQ/DI and all five *inactive* noise resonators
 * complete the onset. The active one belongs to the companion helper.
 *
 * This helper includes all four F0 points, so a caller must not also append
 * reloq_german_phrase_f0_append() for this isolated period scope. The voice
 * duration after release is 460 ticks in the measured period condition.
 * Other prosody marks, second syllables or word boundaries return 1 and
 * must be modeled separately. On any failure builder storage is unchanged. */
int reloq_german_initial_voiced_stop_envelope_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    enum reloq_german_terminal_mark mark,
    int64_t origin_tick, int64_t burst_tick, int64_t onset_tick,
    int64_t closure_tick, int64_t release_tick,
    struct reloq_target_builder *builder);

#endif
