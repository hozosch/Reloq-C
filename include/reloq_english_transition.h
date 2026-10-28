#ifndef RELOQ_ENGLISH_TRANSITION_H
#define RELOQ_ENGLISH_TRANSITION_H

/* Nominal US nasal transition clock, before speaking-rate scaling.
 * Symbols use the public phone alphabet. A zero following phone is a
 * phrase boundary. Following N/M are syllabic n/m. Returns -1 outside
 * this family (the left phone must be an ordinary m/n/ng).
 * word_boundary distinguishes a new lexical word from a syllable edge.
 * No text, word identity, native ABI or learned model is involved. */
int reloq_english_nasal_transition_ticks(unsigned char left,
    unsigned char following,int word_boundary);

#endif
