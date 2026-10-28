#ifndef RELOQ_GERMAN_STOP_PHASE_H
#define RELOQ_GERMAN_STOP_PHASE_H
#include "reloq_german_onset_cluster.h"

/* A plain stop onset has a closure, a place burst and a transition into
 * its vowel. Voiceless stops own the aspirated transition; voiced stops
 * leave it to the vowel's incoming interval. A word edge is not silence.
 * The caller supplies fresh only at an utterance edge or an actual pause.
 * vowel_coda is the vowel's following same-syllable phone, otherwise NONE.
 * Keep lexical coda r distinct from the independent reduced vowel R.
 * Return 1 when covered, 0 for other contexts, -1 for invalid arguments. */
struct reloq_german_stop_phase {
    struct reloq_german_onset_cluster gesture;
    int incoming_ticks;
};
int reloq_german_stop_phase_plan(const struct reloq_german_phone_node *node,
    enum reloq_german_phone vowel_coda, int fresh,
    struct reloq_german_stop_phase *out);
/* The preceding coda keeps its own release, including the 20/30-tick
 * aspirated k-t/t-k transitions. The next onset never absorbs that time. */
int reloq_german_stop_pair_phase(const struct reloq_german_phone_node *node,
    unsigned next_stress, struct reloq_german_stop_phase *out);
/* A syllable-final stop releases into the following syllable's nasal/l. */
int reloq_german_stop_sonorant_phase(const struct reloq_german_phone_node *node,
    unsigned next_stress, struct reloq_german_stop_phase *out);
/* Postvocalic stop before the next syllable's fricative: the contact
 * interpolates from burst targets to the following fricative targets. */
int reloq_german_stop_fricative_phase(const struct reloq_german_phone_node *node,
    unsigned next_stress, struct reloq_german_stop_phase *out);
/* A word-final p/t/k keeps its own closure and burst before any measured
 * following phone. Phrase-final release is owned by the pause composer;
 * a following vowel owns the CV contact, other contacts remain here. */
int reloq_german_stop_word_edge(const struct reloq_german_phone_node *node,
    unsigned next_stress, int phrase_final, struct reloq_german_stop_phase *out);
int reloq_german_stop_coda_phase(const struct reloq_german_phone_node *n,
    unsigned next_stress,int next_final,enum reloq_german_phone after_next,
    struct reloq_german_stop_phase *out);
/* A syllable's coda releases into the following vowel's syllable.
 * Closure, burst and contact use independent stress/place contexts;
 * an active glottal source gesture occupies the same contact interval. */
int reloq_german_stop_vowel_boundary(const struct reloq_german_phone_node *n,
    unsigned next_stress,enum reloq_german_phone vowel_coda,
    struct reloq_german_stop_phase *out);

#endif
