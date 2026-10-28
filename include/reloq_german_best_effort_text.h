#ifndef RELOQ_GERMAN_BEST_EFFORT_TEXT_H
#define RELOQ_GERMAN_BEST_EFFORT_TEXT_H

#include "reloq_lexicon.h"
#include "reloq_german_word_features.h"

#include <stddef.h>

/* Experimental opt-in adapter. Build a complete ECI SPR annotation from
 * UTF-8 German text. A validated lexicon and strict rules take precedence;
 * only missing words use the fallible best-effort G2P. Productive numerical
 * readings, initialisms and operator names are likewise approximate; syntax
 * punctuation remains literal in the annotation. The approximation count
 * includes generated number/symbol groups and need not equal input words.
 * Return 0 success, 1 uncovered word, 2 cancellation, -1 invalid text,
 * -2 allocation error.
 * Caller frees *annotation on success. Other outputs remain unchanged on
 * error except encoded_error_offset (Windows-1252 byte offset) when supplied.
 * The caller should report approximate_words prominently to listeners. */
int reloq_german_best_effort_annotate(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_lexicon *lexicon,
    unsigned char **annotation, size_t *annotation_length,
    size_t *approximate_words, size_t *encoded_error_offset);

/* Same transaction, with cooperative cancellation across UTF-8 encoding,
 * text scanning and pronunciation lookup.  Output pointers and lengths are
 * untouched when cancellation is reported. */
int reloq_german_best_effort_annotate_checked(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_lexicon *lexicon,
    unsigned char **annotation, size_t *annotation_length,
    size_t *approximate_words, size_t *encoded_error_offset,
    int (*cancelled)(void *), void *cancel_context);

/* Optional native ECI index mapping during the same frontend pass. The
 * caller supplies ascending Windows-1252 character offsets (each original
 * character is one encoded byte) and receives offsets in the completed SPR
 * annotation. An index inside a source token follows that token's acoustic
 * word; a mark at its beginning precedes it. Duplicate offsets retain their
 * order. The mapped annotation offsets are defined only on success. */
struct reloq_german_best_effort_boundary {
    size_t encoded_offset;
    size_t annotation_offset;
};

int reloq_german_best_effort_annotate_indexed_checked(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_lexicon *lexicon,
    unsigned char **annotation, size_t *annotation_length,
    size_t *approximate_words, size_t *encoded_error_offset,
    struct reloq_german_best_effort_boundary *boundaries,
    size_t boundary_count,
    int (*cancelled)(void *), void *cancel_context);

/* Full text plan: keep hidden lexical features alongside the public SPR.
 * Both returned allocations belong to the caller. No outputs are published
 * on failure; existing annotation-only callers keep their original API. */
int reloq_german_best_effort_plan_indexed_checked(
    const unsigned char *utf8,size_t utf8_length,
    const struct reloq_lexicon *lexicon,
    unsigned char **annotation,size_t *annotation_length,
    size_t *approximate_words,size_t *encoded_error_offset,
    struct reloq_german_best_effort_boundary *boundaries,size_t boundary_count,
    struct reloq_german_word_features **features,size_t *feature_count,
    int (*cancelled)(void *),void *cancel_context);

/* 1 enables public inline controls. Internal mode 2 admits only validated
 * SPR/prominence from Unicode character runs, including mixed prose. */
int reloq_german_best_effort_plan_tags_checked(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_lexicon *lexicon, int process_tags,
    unsigned char **annotation, size_t *annotation_length,
    size_t *approximate_words, size_t *encoded_error_offset,
    struct reloq_german_best_effort_boundary *boundaries,
    size_t boundary_count,
    struct reloq_german_word_features **features,size_t *feature_count,
    int (*cancelled)(void *), void *cancel_context);

#endif
