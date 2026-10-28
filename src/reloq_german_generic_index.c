#include "reloq_german_generic_index.h"

#include "reloq_german_annotation.h"

#include <stdint.h>

int reloq_german_generic_index_positions(
    const unsigned char *annotation, size_t annotation_length,
    const struct reloq_german_generic_index *indices, size_t index_count,
    const int64_t *word_ticks, size_t word_count, size_t samples_per_tick,
    size_t *positions)
{
    if (!annotation || (index_count && (!indices || !positions)) ||
        (word_count && !word_ticks) || !samples_per_tick) return -1;
    for (size_t i = 0; i < index_count; ++i)
        if (indices[i].annotation_offset > annotation_length ||
            (i && indices[i].annotation_offset <
                      indices[i - 1].annotation_offset)) return -1;
    for (size_t i = 0; i < word_count; ++i)
        if (word_ticks[i] < 0 ||
            (i && word_ticks[i] < word_ticks[i - 1]) ||
            (uintmax_t)word_ticks[i] >
                (SIZE_MAX - 1) / samples_per_tick) return -1;

    size_t next = 0, word = 0;
    struct reloq_german_annotation_cursor cursor = {0};
    struct reloq_german_annotation_event event;
    int state;
    while ((state = reloq_german_annotation_next(
                annotation, annotation_length, &cursor, 0, &event)) == 1) {
        if (event.kind != RELOQ_DE_ANNOTATION_SPR ||
            event.spr.kind != RELOQ_DE_SPR_WORD_OPEN) continue;
        if (word >= word_count) return -1;
        while (next < index_count &&
               indices[next].annotation_offset <= event.offset)
            positions[next++] = (size_t)word_ticks[word] * samples_per_tick;
        ++word;
    }
    if (state != 0 || word != word_count) return -1;
    while (next < index_count) positions[next++] = SIZE_MAX;
    return 0;
}

static int due(struct reloq_german_generic_index_stream *stream)
{
    while (stream->next < stream->count &&
           stream->positions[stream->next] != SIZE_MAX &&
           stream->positions[stream->next] <= stream->samples) {
        if (stream->index_sink(stream->context, stream->next)) return -1;
        ++stream->next;
    }
    return 0;
}

int reloq_german_generic_index_stream_begin(
    struct reloq_german_generic_index_stream *stream,
    const size_t *positions, size_t count,
    reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink, void *context)
{
    if (!stream || !pcm_sink || (count && (!positions || !index_sink)))
        return -1;
    *stream = (struct reloq_german_generic_index_stream){
        .positions = positions, .count = count,
        .pcm_sink = pcm_sink, .index_sink = index_sink, .context = context
    };
    return due(stream);
}

int reloq_german_generic_index_stream_pcm(void *context,
                                          const int16_t *pcm, size_t samples)
{
    struct reloq_german_generic_index_stream *stream = context;
    if (!stream || (samples && !pcm) || samples > SIZE_MAX - stream->samples)
        return -1;
    if (due(stream)) return -1;
    while (samples) {
        size_t count = samples;
        if (stream->next < stream->count &&
            stream->positions[stream->next] != SIZE_MAX) {
            size_t next = stream->positions[stream->next];
            if (next <= stream->samples) return -1;
            if (next - stream->samples < count)
                count = next - stream->samples;
        }
        if (stream->pcm_sink(stream->context, pcm, count)) return -1;
        stream->samples += count;
        pcm += count;
        samples -= count;
        if (due(stream)) return -1;
    }
    return 0;
}

int reloq_german_generic_index_stream_end(
    struct reloq_german_generic_index_stream *stream)
{
    if (!stream || due(stream)) return -1;
    while (stream->next < stream->count) {
        if (stream->positions[stream->next] != SIZE_MAX ||
            stream->index_sink(stream->context, stream->next)) return -1;
        ++stream->next;
    }
    return 0;
}
