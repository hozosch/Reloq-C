#ifndef RELOQ_GERMAN_REDUCED_TRACT_H
#define RELOQ_GERMAN_REDUCED_TRACT_H

#include "reloq_german_reduced_landmarks.h"
#include "reloq_target_builder.h"

/* Appends only the tract of the reduced second syllable in an explicitly
 * primary-stressed German CV(C) word. After first CVC[p/k], the follower
 * can be onsetless /@/, /@n/ or /t@/, /t@n/. After open first CV, its
 * onset can be /p,t,k/ before /@/ or /@n/. Tested first onsets p/t/k and
 * ten simple first vowels. The first stage owns its opening gesture and,
 * for CVC+t, its coda release; this stage begins after that release.
 * For open CV it begins at first_closure (the second onset constriction),
 * and for CVC+onsetless it begins at first_coda_release. In the latter two
 * cases the first stage must end before that tick to avoid duplicate points.
 * Timing and first-syllable gestures are caller-owned.
 * Streams F1..F5, B1..B5, FNP/FNZ, FTP/FTZ, FL, TL, OQ, DI are emitted;
 * F0 and the phonation/noise sources are independent modules. The terminal
 * period must be checked by the orchestrator because phone nodes omit it.
 *
 * This is a reusable Schwa/nasal context model, not word data. Return 0
 * modeled; 1 for a valid but uncovered shape; -1 invalid timing/storage.
 * Builder stays unchanged on nonzero return. */
int reloq_german_reduced_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *timing,
    struct reloq_target_builder *builder);

#endif
