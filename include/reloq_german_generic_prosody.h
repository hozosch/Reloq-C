#ifndef RELOQ_GERMAN_GENERIC_PROSODY_H
#define RELOQ_GERMAN_GENERIC_PROSODY_H

#include "reloq_german_terminal_timing.h"

#include <stddef.h>
#include <stdint.h>

struct reloq_german_phone_node;

/* Original-observed lexical verb class, without productive suffix guesses.
 * Shared with morphology to distinguish a complete infinitive constituent
 * from a nominal ending which merely has the same final letters. */
int reloq_german_observed_verb(const unsigned char *word,size_t length);

/* Retain the punctuation *between* SPR groups. A phone sequence alone loses
 * that distinction and must not put the same fixed silence after every word.
 * The caller has already validated the complete annotation. Exactly one
 * mark per word, including the final word, is written on success. Controls
 * such as `2 connect two words; punctuation marks terminate a clause or
 * sentence. A NUL-delimited callback boundary is a full sentence boundary.
 * The final word's mark is also suitable for future phrase F0 planning.
 * Returns zero on success, -1 for invalid data or insufficient capacity;
 * the destination stays untouched on failure. */
int reloq_german_generic_prosody_marks(
    const unsigned char *annotation, size_t length, size_t word_count,
    enum reloq_german_terminal_mark *marks, size_t capacity);

/* A connected word transition has no automatically inserted silence.
 * Measured Apple German punctuation windows have these 11,025-Hz 1-ms
 * durations. They apply only where annotation explicitly marks a clause
 * boundary; the generic C voice still approximates phone releases. */
int reloq_german_generic_prosody_boundary_ticks(
    enum reloq_german_terminal_mark mark, int32_t *ticks);

/* Weak `00 prominence in the original annotation compresses a vowel much
 * more than its written .0 syllable stress alone. Return 1 with an interval
 * for a simple vowel in that class; 0 leaves the ordinary planner intact;
 * -1 means invalid arguments. base_ticks is the vowel's lexical duration.
 * The caller supplies a verified monosyllabic simple-CV/CVC context;
 * uncertain onset and compound shapes keep their existing durations. This
 * rule uses phone classes and immediate coda context, not word names. */
int reloq_german_generic_prosody_weak_vowel_ticks(
    const struct reloq_german_phone_node *node, int32_t base_ticks,
    int simple_monosyllable, int32_t *ticks);

/* Native phrase accent assignment for independently annotated natural text.
 * Observed lexical/contextual classes distinguish weak words, deaccented
 * lexical stresses, ordinary accents and phrase nuclei. Multi-syllable weak
 * words retain their lexical primary stress; monosyllables follow their
 * selected prominence. Phones, secondary stress and punctuation remain.
 * When words and SPR groups cannot be aligned one-to-one (e.g.
 * numerals or abbreviations), the output is an exact copy instead of a
 * speculative reduction. Existing numeric controls and spelled letter
 * sequences are preserved. Explicit user SPR must bypass this operation.
 *
 * The input spelling is Windows-1252, already encoded by the German text
 * frontend. On success, *out is a newly allocated NUL-terminated annotation
 * which the caller frees. On cancellation return 1; on invalid inputs or
 * memory failure return -1. Output pointers stay untouched on error. */
int reloq_german_generic_prosody_reaccent(
    const unsigned char *cp1252, size_t cp1252_length,
    const unsigned char *annotation, size_t annotation_length,
    unsigned char **out, size_t *out_length,
    int (*cancelled)(void *), void *cancel_context);

/* One spoken SPR group together with the written token it came from. The
 * frontend supplies this explicit alignment when numerals, abbreviations or
 * symbols expand into several groups, so phrase prosody still applies.
 * text is Windows-1252 (or the expanded spelling of the group); force_weak
 * marks groups that are unconditionally unstressed function words.
 * fixed_phrase distinguishes supplied SPR, original symbols and explicit
 * character runs. Original characters retain source identity for the native
 * context pass; supplied SPR and Unicode extension names retain their phones. */
enum reloq_german_fixed_source {
    RELOQ_DE_SOURCE_WORD=0, RELOQ_DE_SOURCE_SPR=1,
    RELOQ_DE_SOURCE_SYMBOL=2, RELOQ_DE_SOURCE_CHARACTER=3,
    RELOQ_DE_SOURCE_NATIVE_TOKEN=4, RELOQ_DE_SOURCE_DICTIONARY=5
};
struct reloq_german_prosody_unit {
    const unsigned char *text;
    size_t length;
    int force_weak;
    int fixed_phrase;
    int force_deaccent; /* No phrase accent; preserve full lexical duration. */
    int preserve; /* Explicit SPR keeps its supplied accent and phones. */
    int unresolved; /* Alignment only; never an authored pronunciation. */
    int source_emphasis; /* Caller control: 1=00, 2=0, ... 6=4; 0=automatic. */
};
int reloq_german_generic_prosody_reaccent_units(
    const struct reloq_german_prosody_unit *units, size_t count,
    const unsigned char *annotation, size_t annotation_length,
    unsigned char **out, size_t *out_length,
    int (*cancelled)(void *), void *cancel_context);

#endif
