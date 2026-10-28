#ifndef RELOQ_GERMAN_GENERIC_INDEX_H
#define RELOQ_GERMAN_GENERIC_INDEX_H

#include "reloq_german_generic_voice.h"

#include <stdint.h>

/* Resolve ordered annotation offsets against the single utterance's word
 * starts. word_ticks are the actual already rate-scaled planner ticks; one
 * planner tick renders one frame of samples_per_tick PCM samples. SIZE_MAX
 * marks a trailing index after the complete utterance. */
int reloq_german_generic_index_positions(
    const unsigned char *annotation, size_t annotation_length,
    const struct reloq_german_generic_index *indices, size_t index_count,
    const int64_t *word_ticks, size_t word_count, size_t samples_per_tick,
    size_t *positions);

struct reloq_german_generic_index_stream {
    const size_t *positions;
    size_t count, next, samples;
    reloq_session_pcm_sink pcm_sink;
    reloq_german_generic_index_sink index_sink;
    void *context;
};

/* Start only after a complete frontend and acoustic plan has succeeded.
 * PCM and indices share one serialized stream; index callbacks occur after
 * accepting every sample before their measured word boundary. */
int reloq_german_generic_index_stream_begin(
    struct reloq_german_generic_index_stream *stream,
    const size_t *positions, size_t count,
    reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink, void *context);
int reloq_german_generic_index_stream_pcm(void *context,
                                          const int16_t *pcm, size_t samples);
int reloq_german_generic_index_stream_end(
    struct reloq_german_generic_index_stream *stream);

#endif
