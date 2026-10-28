#ifndef RELOQ_GERMAN_NUMBERS_H
#define RELOQ_GERMAN_NUMBERS_H
#include <stddef.h>

/* A callback receives one complete word's SPR body (without `[...]).
 * weak marks the independently observed feminine article before units.
 * A nonzero callback result propagates cancellation/allocation failure. */
typedef int (*reloq_german_number_emit)(void *,const char *,int weak);
/* Complete CP1252 word; 1 means no productive number compound matched.
 * On a match return the emission status, without partially consuming a word. */
int reloq_german_number_word(const unsigned char *word,size_t length,
    reloq_german_number_emit emit,void *context);
int reloq_german_number_read(const unsigned char *text,size_t length,
    size_t *consumed,reloq_german_number_emit emit,void *context);
/* Context supplied by the text frontend. day_suffix selects the ending of
 * a day name in a numeric date: 0 -er (default), 1 -en, 2 -e. */
struct reloq_german_number_context {
    int day_suffix;
};
int reloq_german_number_read_ctx(const unsigned char *text,size_t length,
    size_t *consumed,reloq_german_number_emit emit,void *context,
    const struct reloq_german_number_context *number_context);

/* Speak value as an ordinal; dative selects the -en ending, otherwise -e.
 * Groups are emitted with the same callback as cardinals. */
#include <stdint.h>
int reloq_german_ordinal_emit(uint64_t value,int dative,
    reloq_german_number_emit emit,void *context);
#endif
