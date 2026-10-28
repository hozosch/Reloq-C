#ifndef RELOQ_GERMAN_ONSET_CLUSTER_H
#define RELOQ_GERMAN_ONSET_CLUSTER_H
#include "reloq_german_obstruent_tract.h"
#include "reloq_german_source_envelope.h"

/* A consonant gesture ends before its release into the next consonant.
 * The vowel owns only its incoming interval. All coordinates remain native. */
struct reloq_german_onset_cluster {
    int body_ticks, release_ticks, burst_ticks;
    int closure_av, av, ah, release_ah, midpoint_av, di;
    int af, noise[6], tl, tl_step;
    int source_lead_ticks, source_lead_tl, nasal_peak, omit_av;
    struct reloq_german_obstruent_tract tract;
};
int reloq_german_onset_cluster_plan(const struct reloq_german_phone_node *p,
    size_t count, size_t index, struct reloq_german_onset_cluster *out);
int reloq_german_onset_cluster_emit(const struct reloq_german_onset_cluster *p,
    int64_t begin, size_t index, reloq_german_source_emit emit, void *context);
#endif
