#ifndef RELOQ_GERMAN_G2P_H
#define RELOQ_GERMAN_G2P_H

#include <stddef.h>

/* Root-local v decision, before syllable-final devoicing. */
int reloq_german_g2p_v_voiced(const unsigned char *root,size_t length,int one_nucleus);

/* A bounded, rule-based German word-to-SPR transducer. Input is one
 * Windows-1252 word, as produced by reloq_german_input_encode after word
 * segmentation. Output is a NUL-terminated bracketed SPR group, e.g.
 * `[.1br'aw'.0z@]. The output excludes outer text controls and pauses.
 *
 * This intentionally covers transparent one-nucleus stems: explicit vowel
 * length/diphthongs, simple -e/-en/-em/-es/-et/-er unstressed endings,
 * a guarded productive -en -> -ung nominal derivation, guarded -heit/-keit
 * nouns, -ungen/-heiten/-keiten plurals, and -chen/-lein diminutives from
 * already covered stems. A productive negative un- prefix reassigns the
 * primary accent to the prefix; productive -ig derives syllabification from
 * the covered stem's -en form. Unstressed be-/ge-/ver-/er- before an already
 * covered -en infinitive preserve the base's primary stress and weak ending.
 * Regular -te/-ten from a covered one-stress -en stem with a verified doubled
 * coda class moves its consonant into the stressed syllable. A covered
 * one-stress -en stem with final /b,d,g/ supports the productive
 * -t/-st/-te/-ten devoicing pattern; orthographically ambiguous -dt forms
 * require an explicit vowel-length mark. Guarded -lich/-liche/-lichen/
 * -lichem/-liches/-licher attaches to already covered, stable stem codas.
 * Ambiguous spellings and more complex morphology return 0 instead of guessing.
 * Return 1 on generated pronunciation, 0 on uncovered but valid spelling,
 * -1 on invalid arguments or malformed input, -2 if output capacity is too
 * small. On 0 or -1 the output and *byte_length are unchanged. On -2 the
 * output is unchanged and *byte_length is the required length without NUL.
 * This is a language-rule component, not a complete German G2P system.
 */
int reloq_german_g2p_word(const unsigned char *word, size_t word_length,
                          unsigned char *output, size_t capacity,
                          size_t *byte_length);

/* Research-only opt-in approximation. First calls the conservative API above;
 * for its otherwise uncovered alphabetic words, estimates a syllable split,
 * vowel length, stress and schwa from spelling. Successful fallback output is
 * structurally usable SPR, not an Apple-matched pronunciation. Its return
 * path may retain a secondary accent from two independently composable
 * components when their full phone stream has one unambiguous split;
 * a complete, independently covered -en verb can also be composed with
 * a verified separable prefix, including selected regular finite forms.
 * A multisyllabic, conservatively covered compound head may retain its
 * secondary accent when its phones and boundary match uniquely. Common
 * weak suffixes and ambiguous single-syllable heads remain guarded. Its return
 * codes, bounded input, buffer and transactional output behavior match the
 * conservative API. If approximation_used is non-NULL, set it to 0 for a
 * covered conservative result, 1 for a fallback result; it is unchanged on
 * error. No production frontend calls this function implicitly. */
int reloq_german_g2p_word_best_effort(
    const unsigned char *word, size_t word_length,
    unsigned char *output, size_t capacity, size_t *byte_length,
    int *approximation_used);

/* A stripped stem followed by a vocalic ending is not a complete phonetic
 * word. In particular, its terminal consonants do not acquire an epenthetic
 * schwa before the ending is restored. Same status contract as above. */
int reloq_german_g2p_open_stem_best_effort(
    const unsigned char *word,size_t word_length,
    unsigned char *output,size_t capacity,size_t *byte_length,
    int *approximation_used);

/* Quantity is chosen before syllabification. -1 leaves an unobserved
 * intrastem consonant span to the caller, 0 is short and 1 is long. */
int reloq_german_g2p_cluster_quantity(const unsigned char *span,size_t length);

#endif
