#ifndef RELOQ_GERMAN_WORD_CLASS_H
#define RELOQ_GERMAN_WORD_CLASS_H
#include <stddef.h>

/* CP1252 input. -1 leaves mixed/lowercase tokens with normal word analysis;
 * 0 selects a word, 1 selects character names. Explicit caller dictionary
 * entries take precedence over this default classification. */
int reloq_german_word_class(const unsigned char *word,size_t length);
/* Lowercase root coda, before optional inflectional s/t. */
int reloq_german_root_coda_legal(const unsigned char *cluster,size_t length);
#endif
