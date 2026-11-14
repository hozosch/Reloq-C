/* Experimental Persian frontend, GPL-3.0-or-later. */
#ifndef RELOQ_PERSIAN_PINGLISH_CANDIDATE_H
#define RELOQ_PERSIAN_PINGLISH_CANDIDATE_H

#include <stddef.h>

enum fa_latin_mode { FA_LATIN_ENGLISH, FA_LATIN_PINGLISH };
enum fa_latin_result {
    FA_LATIN_EN_US,          /* Give the original word to the US frontend. */
    FA_LATIN_CANDIDATE,      /* Unstressed candidate; consult dictionary first. */
    FA_LATIN_NEEDS_CONTEXT,  /* This word requires rules not implemented here. */
    FA_LATIN_INVALID,
    FA_LATIN_OVERFLOW
};

/* One ASCII word only. In Pinglish mode the output uses eSpeak mnemonic
 * phonemes without stress. This is a reference-stage candidate, not a
 * synthesized phoneme sequence or a complete language frontend. On every
 * non-candidate result, out is empty if capacity is nonzero. */
enum fa_latin_result fa_latin_candidate(enum fa_latin_mode mode,
                                         const char *word, char *out,
                                         size_t capacity);

/* For the two eSpeak Persian A vowel mnemonics: short a -> Finnish A (ä),
 * long/back A -> Finnish a. Returns 0 for all other input phones. */
char fa_reference_a_to_finnish(char espeak_vowel);
#endif
