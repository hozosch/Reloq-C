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
/* General orthographic fallback after lexical and morphology rules fail.
 * Unwritten short vowels are inferred and may be ambiguous. */
int reloq_fa_direct_guess_word(const char *word,size_t bytes,char *output,size_t capacity);
/* Use a classified finite verb and an independently known stem after a
 * confirmed verbal prefix; 0 means no safe morphological analysis. */
int reloq_fa_direct_guess_prefixed_verb(const char *word,size_t bytes,char *output,size_t capacity);
size_t reloq_fa_direct_word_count(void);
/* Apply conservative stress rules to a single generated reference tag.
 * The spelling must already be in the native normalized form. */
int reloq_fa_direct_stress(const char *word,size_t bytes,char *tag,size_t capacity);
/* Reconcile automatic reference lengths after all word-stress and compound
 * decisions. This never processes user-supplied phonetic controls. */
void reloq_fa_direct_normalize_durations(char *tag);
int reloq_fa_direct_is_verb(const char *word,size_t bytes);
/* Coarse lexical (1), verbal (2), and other/function (4) evidence; bits may
 * coexist for a homograph. Zero means the spelling is not classified. */
unsigned reloq_fa_direct_word_classes(const char *word,size_t bytes);
/* Apply a bounded noun + derived-adjective stress hint to an existing tag. */
void reloq_fa_direct_stress_after_word(const char *previous,size_t previous_bytes,
                                       const char *word,size_t bytes,char *tag);
/* Unambiguous common proclitics before a following word in the same phrase. */
int reloq_fa_direct_is_proclitic(const char *word,size_t bytes);
#endif
