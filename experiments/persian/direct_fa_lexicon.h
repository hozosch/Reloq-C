/* Experimental exact-word Persian lexicon, GPL-3.0-or-later. */
#ifndef RELOQ_DIRECT_FA_LEXICON_H
#define RELOQ_DIRECT_FA_LEXICON_H
#include <stddef.h>

/* 1 = found, 0 = absent, -1 = invalid arguments, -2 = insufficient output.
 * The input is one complete UTF-8 word, not a NUL-terminated requirement.
 * Output is a NUL-terminated Finnish-based Persian listening-probe tag.
 * Unknown words are not guessed. This is not yet an NVDA language module. */
int reloq_fa_direct_word(const char *word, size_t bytes, char *output, size_t capacity);
/* As above, with bounded reference function-word stress, admitted plural
 * forms and other verified suffixes. follows_word is true only when another
 * word follows before a boundary. Unknown stems and unadmitted forms fail. */
int reloq_fa_direct_context_word(const char *word, size_t bytes, int follows_word,
                                 char *output, size_t capacity);
size_t reloq_fa_direct_word_count(void);
/* Apply conservative stress rules to a single generated reference tag.
 * The spelling must already be in the native normalized form. */
int reloq_fa_direct_stress(const char *word,size_t bytes,char *tag,size_t capacity);
int reloq_fa_direct_is_verb(const char *word,size_t bytes);
#endif
