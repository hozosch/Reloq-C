#ifndef RELOQ_GERMAN_MORPHOLOGY_H
#define RELOQ_GERMAN_MORPHOLOGY_H
#include "reloq_lexicon.h"
/* The prefix boundary must leave a legal consonant onset. */
int reloq_german_morphology_prefix_onset(const unsigned char *,size_t,size_t);
/* Dictionary-backed productive components. Same output/capacity contract
 * as reloq_german_g2p_word. Whole-word overrides must be checked first.
 * Only measured lexical roles may participate; ambiguous decompositions
 * return uncovered. No recursive G2P guesses become lexical roots. */
int reloq_german_morphology_word(const struct reloq_lexicon *lexicon,
    const unsigned char *word, size_t length, unsigned char *output,
    size_t capacity, size_t *byte_length);
/* Verify the existing measured pronunciation fingerprint. No derivational
 * eligibility is implied by this read-only lookup. */
int reloq_german_morphology_observed_root(const struct reloq_lexicon *,
 const unsigned char *,size_t,const struct reloq_lexicon_override **);
/* Read-only lexical boundary evidence for the spelling fallback. This
 * supplies no pronunciation and does not promote an estimate to verified. */
int reloq_german_morphology_is_root_boundary(const unsigned char *,size_t,size_t);
/* Measured initial/final constituent eligibility; no pronunciation. */
unsigned reloq_german_morphology_constituent_class(const unsigned char *,size_t);
/* Shared stem operation; it preserves the provenance of the supplied
 * pronunciation. A caller using an estimated stem must keep that status. */
size_t reloq_german_adjective_base(const unsigned char *,size_t);
int reloq_german_adjective_form(const unsigned char *,size_t,
    const unsigned char *,size_t,unsigned char *,size_t,size_t *);
/* Explicit user-requested departure: productive nominal prefix stress.
 * Capitalized single-root nouns only; recognized verb forms keep reference
 * stress. This changes accents, never phonemes, quantity or boundaries. */
void reloq_german_morphology_nominal_stress(const struct reloq_lexicon *,
    const unsigned char *,size_t,unsigned char *,size_t);
/* Requested correction of the productive konzentrier- stem. Keep the
 * surrounding morphology and the accent level of a prefixed constituent. */
void reloq_german_morphology_repair_stem(const unsigned char *,size_t,
    unsigned char *,size_t);
/* Requested intact ch constituent before a vowel-initial compound head.
 * Uses the same measured component classes as the ordinary morphology. */
int reloq_german_morphology_user_compound(const struct reloq_lexicon *,
    const unsigned char *,size_t,unsigned char *,size_t,size_t *);
#endif
