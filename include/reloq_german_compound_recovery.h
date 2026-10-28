#ifndef RELOQ_GERMAN_COMPOUND_RECOVERY_H
#define RELOQ_GERMAN_COMPOUND_RECOVERY_H

#include <stddef.h>

struct reloq_lexicon;

/* Conservative last-resort composition of a German noun compound that the
 * whole-word text frontend has not covered. Both spelling components must
 * independently produce complete German SPR words through the existing
 * lexicon/grammar path. Only an unambiguous one-syllable first component and
 * a two- or three-syllable head with a plausible orthographic boundary
 * qualify. The productive -en+s linker is admitted after a separately
 * covered two-syllable weak /@n/ word before a consonant-initial head.
 * The result is ONE SPR group, retaining the first component's primary
 * stress and demoting the head's primary stress to secondary. It contains no
 * observed pronunciation or word-specific rewrite.
 *
 * Return 1 on success, 0 if no safe composition exists, -1 on bad input,
 * -2 if capacity is insufficient. On -2, *byte_length is the required
 * length excluding NUL, and output is unchanged. On 0/-1 all outputs remain
 * unchanged. Input is exactly one CP1252 spelling word; no punctuation. */
int reloq_german_compound_recover(
    const struct reloq_lexicon *lexicon,
    const unsigned char *word, size_t word_length,
    unsigned char *output, size_t capacity, size_t *byte_length);

#endif
