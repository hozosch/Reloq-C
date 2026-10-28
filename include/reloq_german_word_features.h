#ifndef RELOQ_GERMAN_WORD_FEATURES_H
#define RELOQ_GERMAN_WORD_FEATURES_H
#include <stddef.h>
#include <stdint.h>

/* Linguistic facts which the public SPR spelling cannot carry. The word
 * number survives pronunciation rewriting and native index remapping. */
struct reloq_german_word_features {
    size_t word_index;
    uint64_t glottal_syllables;
    unsigned syllable_count;
    uint64_t full_a_syllables;
    uint64_t full_quantity_syllables;
};

/* Only accept an observed entry whose complete SPR still matches. */
int reloq_german_word_features_lookup(const unsigned char *word,size_t length,
    const unsigned char *spr,size_t spr_length,
    struct reloq_german_word_features *out);
/* Conservative word-internal full-vowel hiatus default. Stress alone does
 * not establish a new root; exact lexical facts and witnessed compound
 * heads are resolved separately and retain precedence over this default. */
int reloq_german_word_features_root_hiatus(const unsigned char *spr,size_t length,
    struct reloq_german_word_features *out);
/* User-selected earlier full-a quality for unstressed written final a.
 * The scheduler selects intermediate duration; known glottal metadata and
 * the linguistic short-A spelling are retained. */
int reloq_german_word_features_final_a(const unsigned char *spr,size_t length,
    struct reloq_german_word_features *inout);
/* Original lexical quantity, which is distinct from stress and is absent
 * from the public SPR alphabet. Merge only with a matching reading. */
int reloq_german_word_features_quantity(const unsigned char *word,size_t length,
    const unsigned char *spr,size_t spr_length,
    struct reloq_german_word_features *inout);

/* Resolve independently observed phonetic readings without written words.
 * Explicit lossless headers take precedence; ambiguous readings are not
 * guessed. Text and authored phonetics share the same final-a policy.
 * Returns 2 for an explicit header, 1 for resolved defaults, -1 invalid. */
int reloq_german_word_features_from_spr(const unsigned char *spr,size_t length,
    struct reloq_german_word_features *out);

/* Merge native onset metadata only for an identical syllable layout.
 * Explicit phonetics and the requested numeric onset policy take precedence. */
int reloq_german_word_features_native_onsets(const unsigned char *,size_t,
    uint64_t,unsigned,struct reloq_german_word_features *);

/* Complete an annotation's sparse feature array. Provided text facts win
 * over inferred defaults; an authored header wins over both. Caller owns
 * the returned array. 0 success, 1 cancelled, -1 invalid, -2 allocation. */
int reloq_german_word_features_resolve(const unsigned char *annotation,size_t length,
    const struct reloq_german_word_features *known,size_t known_count,
    struct reloq_german_word_features **out,size_t *count,
    int (*cancelled)(void *),void *context);

/* Serialize a full plan for lossless text -> phonetics -> PCM round trips.
 * Plain original SPR stays accepted; this explicit optional extension
 * carries distinctions which that alphabet does not express. */
int reloq_german_word_features_annotate(const unsigned char *annotation,size_t length,
    const struct reloq_german_word_features *features,size_t feature_count,
    unsigned char **out,size_t *written);
#endif
