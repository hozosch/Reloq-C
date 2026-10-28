#ifndef RELOQ_GERMAN_PHRASE_F0_H
#define RELOQ_GERMAN_PHRASE_F0_H

#include "reloq_german_phone_sequence.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_target_builder.h"

/* Phrase-final F0 for one isolated, explicitly primary-stressed German
 * syllable [h|p|t|k] V [p|t|k|s|f], or bare V [p|t|k|s|f], with a
 * measured terminal mark in the default 11,025-Hz voice. V is one of
 * A/E/I/O/U/a/e/i/o/u. All times are 1-ms model ticks from an isolated
 * word's origin, which is zero. vowel_onset_tick is the first vowel-formant
 * target (zero without initial consonant, 80 with h, independently
 * predicted from the initial-stop rule after p/t/k); vowel_closure_tick
 * is the second
 * positive vowel AV target. release_start_tick is the independently
 * determined terminal source-release origin (possibly after a coda).
 * For an initial consonant, first_window_ticks is the independently
 * predicted first numeric send's END, used to locate the precursor F0 at
 * floor(first_window_ticks / 10). Without an initial consonant, pass -1.
 *
 * Four F0 targets are appended; the last three scale with the vowel source
 * interval and values depend on the phrase-final mark. This must replace
 * the older period F0 options in initial_h/vowel_nucleus, or
 * those targets would be duplicated. The terminal window comes from the
 * separate timing rule; total_ticks is release_start_tick + window_ticks.
 * With a final s/f, release_start_tick is the fricative hold's end. The
 * final window is 430, 280 or 180 ticks for full, semicolon or comma-like
 * terminals: 30 ticks of source fade then 400, 250 or 150 zero-source
 * ticks. The stop window has 61 release frames then its quiet span.
 * This is not a general prosody model, phone-duration rule, PCM voice or
 * punctuation classifier. The caller must independently identify the final
 * mark and ensure its other source/formant rules cover this same context.
 *
 * Return 0 on success, 1 on an uncovered phone/stress shape, -1 on invalid
 * time, mark or builder error. On failure the builder and *total_ticks are
 * unchanged. */
int reloq_german_phrase_f0_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    enum reloq_german_terminal_mark mark, int64_t first_window_ticks,
    int64_t vowel_onset_tick, int64_t vowel_closure_tick,
    int64_t release_start_tick, struct reloq_target_builder *builder,
    int64_t *total_ticks);

#endif
