#ifndef RELOQ_ENGLISH_WORD_EDGES_H
#define RELOQ_ENGLISH_WORD_EDGES_H
#include <stdbool.h>
#include <stddef.h>

/* Consecutive consonant spellings, in reading order. Categories belong to
 * the caller: y, for example, may already be a vowel in this context.
 * Longer runs are rejected; five letters therefore suffice for the view. */
bool reloq_english_onset(const unsigned char *spelling,size_t length);
bool reloq_english_coda(const unsigned char *spelling,size_t length);
bool reloq_english_syllable_onset(const unsigned char *spelling,size_t length);
#endif
