#ifndef RELOQ_GERMAN_LEXICON_H
#define RELOQ_GERMAN_LEXICON_H

#include "reloq_lexicon.h"

/* Suitable validator for binding German SPR overrides. It accepts exactly
 * one complete `[...] group in IBM's published German SPR notation, with
 * the control prefix required by the complete annotation scanner. */
int reloq_german_lexicon_validate_spr(const unsigned char *spr,
                                       size_t length, void *context);

/* One Windows-1252 word, using the same input/output contract as
 * reloq_german_g2p_word: 1 on pronunciation, 0 uncovered, -1 invalid,
 * -2 insufficient output capacity. On -2 *byte_length is the required
 * length without NUL; output remains unchanged. On 0/-1, both remain
 * unchanged. A NULL output with capacity 0 returns -2 and the required
 * length when a pronunciation exists, as in reloq_german_g2p_word.
 *
 * Explicit, validated caller-owned SPR overrides take precedence over the
 * finite, independently captured reference CVC lexicon, measured dictionary
 * component roles, and then German G2P.
 * Action references in the view are NEVER executed or treated as a
 * pronunciation. Without a matching pronunciation, bounded G2P decides
 * whether to return a candidate or UNCOVERED. NULL view uses the reference
 * CVC lexicon before G2P.
 * This helper can replace the G2P call in the text adapter's one_group;
 * The built-in finite CVC records are generated from public SPR behavior;
 * they do not implement German pronunciation outside that isolated class. */
int reloq_german_lexicon_word(const struct reloq_lexicon *view,
                              const unsigned char *word, size_t word_length,
                              unsigned char *output, size_t capacity,
                              size_t *byte_length);

#endif
