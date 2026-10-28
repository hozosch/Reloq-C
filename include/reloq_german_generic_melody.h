#ifndef RELOQ_GERMAN_GENERIC_MELODY_H
#define RELOQ_GERMAN_GENERIC_MELODY_H

#include "reloq_german_phone_sequence.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_target_builder.h"

#include <stddef.h>
#include <stdint.h>

/* Melody-only flag; the phone timing and tract keep their normal controls. */
enum { RELOQ_DE_MELODY_TAIL = 128 };

/* A syllable's estimated gesture positions in the generic C planner.  The
 * nucleus times describe the source-bearing phone, not its seven-tick
 * target-ramp inset.  All ticks are on the unscaled native time axis. */
struct reloq_german_generic_melody_syllable {
    int64_t start_tick;
    int64_t nucleus_tick;
    int64_t closure_tick;
    int64_t end_tick;
    size_t word_index;
    enum reloq_german_phone first_phone;
    uint8_t stress;
    uint8_t prosody_control; /* enum reloq_german_word_prosody_control */
    int64_t onset_end_tick; /* End of the consonant body, before CV contact. */
    int64_t vowel_core_end_tick; /* First vowel plateau, before glide/contact. */
    int64_t first_body_end_tick;
    int64_t coda_begin_tick, coda_body_end_tick;
    int64_t onset_begin_tick;
    int64_t return_begin_tick, return_end_tick;
    uint8_t preceding_stress;
    uint8_t inherited_vowel; /* Onset range belongs to a preceding nucleus. */
};

/* Place phrase-level F0 targets into an otherwise empty F0 builder stream.
 * The other acoustic streams are untouched.  Clause punctuation segments the
 * input; zero-stress syllables inherit their phrase contour.  This is a
 * realization of the observed accent gestures, independent of spelling and
 * recorded words. The rime includes voiced sonorant codas.
 *
 * `inflection` is the native ECI 0..100 control (30 is Apple's initial
 * value).  At 0 the numeric F0 is constant at the measured 1189 tenths of
 * Hz.  `pitch_scale` applies the separate native pitch control afterward;
 * `time_scale` maps planner ticks to the scheduler's time axis.
 * Returns zero on success and -1 on invalid input, overflow or builder error.
 * An error may leave the builder partially filled; the caller discards it. */
int reloq_german_generic_melody_append(
    const struct reloq_german_generic_melody_syllable *syllables,
    size_t syllable_count,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    int inflection, double pitch_scale, double time_scale,
    struct reloq_target_builder *builder);

struct reloq_inline_word;
int reloq_german_generic_melody_append_controls(
    const struct reloq_german_generic_melody_syllable *syllables,
    size_t syllable_count,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    int inflection, double pitch_scale, double time_scale,
    const struct reloq_inline_word *controls,
    struct reloq_target_builder *builder);

#endif
