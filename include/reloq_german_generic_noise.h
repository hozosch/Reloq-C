#ifndef RELOQ_GERMAN_GENERIC_NOISE_H
#define RELOQ_GERMAN_GENERIC_NOISE_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

#include <stdint.h>

/* Reusable German 11,025-Hz consonant source and constriction classes.
 * The caller starts with its neutral profile; these functions replace the
 * indicated tract and source fields, but do not author a word or duration.
 * The same place/manner coefficients apply across word boundaries. */
enum reloq_german_generic_noise_phase {
    RELOQ_DE_GENERIC_BURST = 0,
    RELOQ_DE_GENERIC_ASPIRATION,
    RELOQ_DE_GENERIC_FRICATION
};

int reloq_german_generic_fricative_noise(
    const struct reloq_german_phone_node *node,
    int32_t targets[RELOQ_TARGET_STREAM_COUNT]);
int reloq_german_s_nasal_source(const struct reloq_german_phone_node *node);

int reloq_german_generic_stop_noise(
    const struct reloq_german_phone_node *node, int syllable_onset,
    enum reloq_german_generic_noise_phase phase,
    int32_t targets[RELOQ_TARGET_STREAM_COUNT]);

/* The source envelope of a voiced initial stop is different from a
 * voiceless onset: its weak voicing belongs to the closure, before a late
 * burst. Relative times are interpreted by the caller's time model. */
int reloq_german_generic_voiced_stop_closure(
    enum reloq_german_phone phone,
    int32_t targets[RELOQ_TARGET_STREAM_COUNT]);

#endif
