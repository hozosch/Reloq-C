#ifndef RELOQ_GERMAN_GENERIC_NASAL_CODA_H
#define RELOQ_GERMAN_GENERIC_NASAL_CODA_H

#include "reloq_german_phone_sequence.h"

#include <stdint.h>

/* Context of a nasal immediately following a vowel in its syllable.  These
 * target levels precede the voice's shared source trim.  A zero F1 override
 * leaves the existing tract value alone.  An absent taper uses the onset AV
 * until end_av at the phone boundary. */
struct reloq_german_generic_nasal_coda {
    int32_t duration_ticks;
    int32_t f1_override;
    int32_t start_av;
    int32_t taper_av;
    int32_t end_av;
    int32_t taper_percent;
};

/* Return 1 for an explicitly measured context, 0 to retain the ordinary
 * consonant plan, and -1 for invalid arguments.  `next_vowel` means the
 * immediate next phone in the SAME word is a vowel after a syllable edge;
 * `phrase_final` means punctuation or the end of the utterance follows this
 * word.  `previous_av` is the pretrim AV of the preceding vowel.
 * No spelling or lexical information enters this rule. */
int reloq_german_generic_nasal_coda_plan(
    const struct reloq_german_phone_node *node,
    int coda_after_vowel, int phrase_final, int next_vowel,
    int32_t previous_av, int32_t source_trim,
    struct reloq_german_generic_nasal_coda *out);

#endif
