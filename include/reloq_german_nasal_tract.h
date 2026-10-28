#ifndef RELOQ_GERMAN_NASAL_TRACT_H
#define RELOQ_GERMAN_NASAL_TRACT_H
#include "reloq_german_phone_sequence.h"
#include "reloq_german_source_envelope.h"

/* A negative departure is absent, not a zero-frequency target. */
struct reloq_german_nasal_tract {
    int32_t entering[5], departing[5], bandwidth[5];
};

/* Audited n/ng contexts: phrase onset, or inside one word.
 * Phrase flags describe acoustic pauses, independently of word boundaries. */
int reloq_german_nasal_tract_plan(const struct reloq_german_phone_node *node,
    int phrase_initial,int phrase_final,struct reloq_german_nasal_tract *out);
int reloq_german_nasal_tract_append(const struct reloq_german_nasal_tract *plan,
    int64_t begin,int64_t end,size_t index,reloq_german_source_emit emit,void *context);
#endif
