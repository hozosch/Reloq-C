#ifndef RELOQ_GERMAN_NUCLEUS_ACOUSTICS_H
#define RELOQ_GERMAN_NUCLEUS_ACOUSTICS_H

#include "reloq_german_phone_sequence.h"
#include <stdint.h>

/* A phonological nucleus has independent entering and departing resonances.
 * end_mask distinguishes a real departing target from a sustained resonance;
 * there are no reference object layouts, instruction addresses or word keys.
 * Source levels describe the unscaled reference voice. av and av_end are the entering/departing voicing amplitudes. Aspiration
 * has its own source curve; neither member is an aspiration target. */
struct reloq_german_nucleus_acoustics {
    int32_t entering[5];
    int32_t departing[5];
    int32_t bandwidth[5];
    int32_t av;
    int32_t av_end;
    uint8_t end_mask;
    int32_t fnp, fnz;
};

/* Returns 1 for a supported nucleus, 0 for another phone, -1 for bad input.
 * Neighbors are the independently chosen acoustic neighbors. The caller can
 * keep a lexical boundary or provide a cross-word acoustic connection.
 * weak_final identifies an unaccented phrase-final nucleus; it is not a
 * spelling rule. Output is unchanged on 0 or -1. */
int reloq_german_nucleus_acoustics_plan(
    enum reloq_german_phone phone, enum reloq_german_phone left,
    enum reloq_german_phone right, uint8_t stress, int weak_final,
    struct reloq_german_nucleus_acoustics *out);

#endif
