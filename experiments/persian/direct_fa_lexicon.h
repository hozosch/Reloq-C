/* Experimental exact-word Persian lexicon, GPL-3.0-or-later. */
#ifndef RELOQ_DIRECT_FA_LEXICON_H
#define RELOQ_DIRECT_FA_LEXICON_H
#include <stddef.h>

/* 1 = found, 0 = absent, -1 = invalid arguments, -2 = insufficient output.
 * The input is one complete UTF-8 word, not a NUL-terminated requirement.
 * Output is a NUL-terminated Finnish-based Persian listening-probe tag.
 * Unknown words are not guessed. This is not yet an NVDA language module. */
int reloq_fa_direct_word(const char *word, size_t bytes, char *output, size_t capacity);
/* As above, with bounded reference function-word stress and the admitted
 * non-joiner + ها suffix. follows_word is true only when another word
 * follows before a boundary. Unknown stems and other suffixes are rejected. */
int reloq_fa_direct_context_word(const char *word, size_t bytes, int follows_word,
                                 char *output, size_t capacity);
size_t reloq_fa_direct_word_count(void);
#endif
