#ifndef RELOQ_GERMAN_GLOTTAL_H
#define RELOQ_GERMAN_GLOTTAL_H

#include "reloq_german_word_features.h"
#include "reloq_german_source_envelope.h"

/* Semantic onset metadata is kept outside the public SPR phone ABI. */
struct reloq_german_glottal_segment {
    int64_t begin, onset, end, release;
    unsigned active, fresh, strength, release_stop;
};

int reloq_german_glottal_onset(const struct reloq_german_phone_node *phone,
    const struct reloq_german_word_features *features);
int32_t reloq_german_glottal_transition(
    const struct reloq_german_phone_node *left,
    const struct reloq_german_phone_node *right);
unsigned reloq_german_glottal_strength(
    const struct reloq_german_phone_node *phone);

/* Emits independent DI/OQ contours, including their ordinary plateaus.
 * Times use native milliseconds; the caller scales once on output. */
int reloq_german_glottal_append(
    const struct reloq_german_glottal_segment *segments,size_t count,
    reloq_german_source_emit emit,void *context);
#endif
