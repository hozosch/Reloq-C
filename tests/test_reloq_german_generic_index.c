#include "reloq_german_generic_index.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct capture {
    size_t samples;
    int calls;
    size_t index_samples[8];
    size_t index_ordinals[8];
    size_t index_count;
    size_t refuse_ordinal;
    int16_t pcm[512];
};

static int send_pcm(void *context, const int16_t *pcm, size_t count)
{
    struct capture *c = context;
    if (count > 512 - c->samples) return -1;
    memcpy(c->pcm + c->samples, pcm, count * sizeof *pcm);
    c->samples += count;
    ++c->calls;
    return 0;
}

static int send_index(void *context, size_t ordinal)
{
    struct capture *c = context;
    if (c->index_count >= 8) return -1;
    c->index_samples[c->index_count] = c->samples;
    c->index_ordinals[c->index_count++] = ordinal;
    return ordinal == c->refuse_ordinal ? -1 : 0;
}

int main(void)
{
    static const unsigned char annotation[] = "`[.1a] `[.1b].";
    const struct reloq_german_generic_index marks[] = {
        {0}, {6}, {7}, {8}, {sizeof annotation - 1}
    };
    const int64_t word_ticks[] = {0, 20};
    size_t positions[5];
    assert(reloq_german_generic_index_positions(
        annotation, sizeof annotation - 1, marks, 5,
        word_ticks, 2, 11, positions) == 0);
    assert(positions[0] == 0 && positions[1] == 220 &&
           positions[2] == 220 && positions[3] == SIZE_MAX &&
           positions[4] == SIZE_MAX);

    struct capture c = {.refuse_ordinal = SIZE_MAX};
    struct reloq_german_generic_index_stream stream;
    assert(reloq_german_generic_index_stream_begin(
        &stream, positions, 5, send_pcm, send_index, &c) == 0);
    assert(c.samples == 0 && c.index_count == 1 &&
           c.index_ordinals[0] == 0);
    int16_t pcm[300];
    for (size_t i = 0; i < 300; ++i) pcm[i] = (int16_t)i;
    assert(reloq_german_generic_index_stream_pcm(&stream, pcm, 300) == 0);
    assert(c.samples == 300 && c.index_count == 3 &&
           c.index_samples[1] == 220 && c.index_samples[2] == 220 &&
           !memcmp(c.pcm, pcm, sizeof pcm));
    assert(reloq_german_generic_index_stream_end(&stream) == 0);
    assert(c.index_count == 5 && c.index_samples[3] == 300 &&
           c.index_samples[4] == 300);

    /* Refusing a marker stops PCM at the exact accepted boundary; no later
     * callback or bytes can escape the aborted transaction. */
    memset(&c, 0, sizeof c);
    c.refuse_ordinal = 1;
    assert(reloq_german_generic_index_stream_begin(
        &stream, positions, 5, send_pcm, send_index, &c) == 0);
    assert(reloq_german_generic_index_stream_pcm(&stream, pcm, 300) == -1);
    assert(c.samples == 220 && c.index_count == 2 &&
           !memcmp(c.pcm, pcm, 220 * sizeof *pcm));
    puts("Generic ECI index boundaries and PCM serialization: OK");
    return 0;
}
