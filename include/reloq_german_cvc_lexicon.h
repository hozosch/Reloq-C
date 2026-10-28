#ifndef RELOQ_GERMAN_CVC_LEXICON_H
#define RELOQ_GERMAN_CVC_LEXICON_H

#include "reloq_lexicon.h"

/* Generated, finite German orthographic CVC class. These are individual
 * public-API SPR observations over a complete controlled input domain, not
 * a productive rule for arbitrary words. Caller-owned overrides take
 * precedence in reloq_german_lexicon_word. */
int reloq_german_cvc_reference_lookup(
    const unsigned char *word, size_t length,
    const struct reloq_lexicon_override **found);
size_t reloq_german_cvc_reference_count(void);

#endif
