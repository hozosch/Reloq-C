#ifndef RELOQ_GERMAN_AFFRICATE_H
#define RELOQ_GERMAN_AFFRICATE_H
#include "reloq_german_obstruent_tract.h"

/* Preserve lexical/source coordinates while expanding the four SPR
 * affricates into independently timed closure and frication gestures. */
int reloq_german_affricate_expand(struct reloq_german_phone_node *phones,
    size_t *count, size_t capacity);
int reloq_german_affricate_ticks(const struct reloq_german_phone_node *phones,
    size_t count, size_t index, int phrase_final, int32_t *ticks);
int reloq_german_affricate_context_ticks(const struct reloq_german_phone_node *phones,
    size_t count, size_t index, int phrase_final, int fresh, int following_final,
    int32_t *ticks);
int reloq_german_affricate_tract(const struct reloq_german_phone_node *phone,
    int initial, int final, enum reloq_german_phone following,
    struct reloq_german_obstruent_tract *out);
#endif
