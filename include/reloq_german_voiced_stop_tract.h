#ifndef RELOQ_GERMAN_VOICED_STOP_TRACT_H
#define RELOQ_GERMAN_VOICED_STOP_TRACT_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Complete the oral tract and source controls after the first send of an
 * isolated, explicitly primary-stressed [b|d|g] V [p|t|k] syllable.
 * V is AEIOUaeiou in the default German 11,025-Hz voice. All ticks are
 * absolute in the utterance target timeline: onset is the first send end,
 * formant_target starts the vowel formants, hold ends the vowel's numeric
 * send, and closure is the following stop's AV-off target.
 *
 * The initial-stop envelope owns the first-send values at origin and onset.
 * The vowel gesture owns F1-F3, AV and AH after onset. This helper appends
 * exactly 38 later targets: F4/F5, B1-B5, FNP/FNZ, A1F-AB, AF, OQ and DI
 * at formant_target and hold, and TL at onset+1 and closure. In particular,
 * the initial F5=4000 relaxes to 3900 at the vowel target. The final stop
 * and terminal source own the later constriction, burst and fade.
 *
 * Returns 0 on success, 1 for an uncovered context, and -1 for an invalid
 * argument, timeline or builder failure. The builder remains unchanged on
 * every failure. */
int reloq_german_voiced_stop_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t onset_tick, int64_t formant_target_tick, int64_t hold_tick,
    int64_t closure_tick, struct reloq_target_builder *builder);

#endif
