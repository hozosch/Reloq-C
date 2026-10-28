#ifndef RELOQ_GERMAN_OBSTRUENT_TRACT_H
#define RELOQ_GERMAN_OBSTRUENT_TRACT_H

#include "reloq_german_source_envelope.h"

/* Two independently selected tract endpoints, with sparse formant tracks.
 * A negative value deliberately contributes no target at that endpoint.
 * The verified context is a plain CV, VC or V.CV inside a word or at a
 * phrase edge. Clusters and unpunctuated word edges return zero. */
struct reloq_german_obstruent_tract {
    int32_t entering[5], departing[5], bandwidth[5];
};

int reloq_german_obstruent_tract_plan(
    const struct reloq_german_phone_node *node, int phrase_initial,
    int phrase_final, struct reloq_german_obstruent_tract *out);

int reloq_german_obstruent_tract_append(
    const struct reloq_german_obstruent_tract *plan,
    int64_t begin, int64_t end, size_t index,
    reloq_german_source_emit emit, void *context);

#endif
