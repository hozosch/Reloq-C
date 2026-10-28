#ifndef RELOQ_GERMAN_BOUNDARY_PROFILE_H
#define RELOQ_GERMAN_BOUNDARY_PROFILE_H

#include "reloq_german_connected_pair.h"

/* One measured acoustic transition between two primary-stressed syllables.
 * The common phrase planner owns vowel duration, F0 and the two isolated
 * vowel/coda gestures. Each profile supplies the context-dependent coda
 * and following onset on the same absolute target clock. A NULL lookup
 * means that the phoneme boundary is not modeled and must emit no PCM. */
struct reloq_german_boundary_profile {
    enum reloq_german_phone first_coda;
    enum reloq_german_phone second_initial;
    int64_t next_origin_offset;
    enum reloq_target_stream onset_fade_stream;
    int (*onset_landmarks)(
        const struct reloq_german_connected_pair *, enum reloq_german_phone,
        int64_t, int64_t *, int64_t *);
    int (*coda_append)(
        const struct reloq_german_connected_pair *, int64_t,
        struct reloq_target_builder *, int64_t *);
    int (*onset_append)(
        const struct reloq_german_connected_pair *, enum reloq_german_phone,
        int64_t, int64_t, struct reloq_target_builder *,
        int64_t *, int64_t *);
};

const struct reloq_german_boundary_profile *
reloq_german_boundary_profile_find(enum reloq_german_phone first_coda,
                                   enum reloq_german_phone second_initial);

#endif
