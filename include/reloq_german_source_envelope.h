#ifndef RELOQ_GERMAN_SOURCE_ENVELOPE_H
#define RELOQ_GERMAN_SOURCE_ENVELOPE_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Ownership is per excitation stream, independent of tract interpolation.
 * NONE leaves an unsupported context with the caller's previous planner. */
enum reloq_german_source_kind {
    RELOQ_SOURCE_NONE, RELOQ_SOURCE_NUCLEUS, RELOQ_SOURCE_CARRY,
    RELOQ_SOURCE_NASAL_ONSET, RELOQ_SOURCE_NASAL_BRIDGE,
    RELOQ_SOURCE_NASAL_FINAL, RELOQ_SOURCE_LIQUID_ONSET,
    RELOQ_SOURCE_PARTIAL_VOICE, RELOQ_SOURCE_VOICED_NOISE,
    RELOQ_SOURCE_R_INITIAL, RELOQ_SOURCE_R_MEDIAL,
    RELOQ_SOURCE_NASAL_CLUSTER, RELOQ_SOURCE_UNVOICED
};

struct reloq_german_source_segment {
    int64_t begin, end;
    int32_t av, av_end;
    int32_t incoming_ticks, outgoing_ticks;
    enum reloq_german_source_kind kind;
    uint8_t phrase_final;
    uint8_t continuing_comma;
};

typedef int (*reloq_german_source_emit)(void *, enum reloq_target_stream,
                                      int64_t, int32_t, size_t);

/* Sparse FNZ events at acoustic core landmarks. Nasals contribute only
 * their midpoint; neighbors own their own knots even across word edges. */
int reloq_german_resonance_append(enum reloq_german_phone phone,
    int64_t begin,int64_t end,int32_t oral_fnz,size_t index,
    reloq_german_source_emit emit,void *context);

enum reloq_german_source_kind reloq_german_source_classify(
    const struct reloq_german_phone_node *phones, size_t count, size_t index,
    int nucleus, int phrase_final);

/* Measured common consonant intervals. Zero retains the caller's duration. */
int32_t reloq_german_source_consonant_ticks(
    const struct reloq_german_phone_node *phones, size_t count, size_t index);

int reloq_german_source_release_ticks(enum reloq_german_phone phone,
    enum reloq_german_source_kind kind);

int reloq_german_source_owns_tilt(enum reloq_german_source_kind kind);
int reloq_german_source_tilt_append(
    const struct reloq_german_phone_node *phones,
    const struct reloq_german_source_segment *segments, size_t count,
    reloq_german_source_emit emit, void *context);

/* Sparse AV/AH targets: omitted points deliberately preserve interpolation
 * between adjacent nuclei. A coda liquid extends its nucleus envelope.
 * User volume and output speed are applied by emit, exactly once. */
int reloq_german_source_envelope_append(
    const struct reloq_german_phone_node *phones,
    const struct reloq_german_source_segment *segments, size_t count,
    reloq_german_source_emit emit, void *context);

#endif
