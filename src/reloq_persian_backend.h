#ifndef RELOQ_PERSIAN_BACKEND_H
#define RELOQ_PERSIAN_BACKEND_H

#include <stddef.h>
#include <stdint.h>
#include "reloq_persian_plan.h"

/* Transitional bridge from language-owned events to the already accepted
 * acoustic provider. Source-language analysis stays in the common core.
 * This is not yet the complete sentence/contour renderer. */
struct reloq_fa_backend_plan {
    char *text;
    size_t length;
    /* Byte position at each event boundary, including the terminal one. */
    size_t *offsets;
    /* Native word ordinals and packed word/affricate ordinals. */
    uint32_t *onsets,*geminates;
    size_t onset_count,geminate_count;
};

int reloq_fa_backend_prepare(const char *source,size_t source_bytes,
    const struct reloq_fa_plan_event *events,size_t count,
    struct reloq_fa_backend_plan *result,size_t *error_event);
/* The US-contour donor needs the original phone sequence, including both
 * phones of an affricate and an explicit glottal boundary. */
int reloq_fa_backend_prepare_donor(const char *source,size_t source_bytes,
    const struct reloq_fa_plan_event *events,size_t count,
    struct reloq_fa_backend_plan *result,size_t *error_event);
void reloq_fa_backend_clear(struct reloq_fa_backend_plan *plan);

#endif
