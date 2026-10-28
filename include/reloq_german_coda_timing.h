#ifndef RELOQ_GERMAN_CODA_TIMING_H
#define RELOQ_GERMAN_CODA_TIMING_H
#include "reloq_german_phone_sequence.h"

struct reloq_german_coda_timing {
    int32_t body_ticks, outgoing_ticks;
};
/* Context-dependent postvocalic nasal/fricative body and CC contact.
 * The caller supplies whether the next phone immediately precedes a pause.
 * 1 = supported, 0 = retain another planner, -1 = invalid arguments. */
int reloq_german_coda_timing_plan(
    const struct reloq_german_phone_node *phones,size_t count,size_t index,
    int pause_after_next,struct reloq_german_coda_timing *out);
#endif
