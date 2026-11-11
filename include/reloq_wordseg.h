#ifndef RELOQ_WORDSEG_H
#define RELOQ_WORDSEG_H
#include <stddef.h>
#include <stdint.h>

enum reloq_wordseg_category {
    RELOQ_WORDSEG_LOWER = 1, RELOQ_WORDSEG_UPPER = 2,
    RELOQ_WORDSEG_MARK = 4, RELOQ_WORDSEG_WORD = 8
};

/* Locale-independent Unicode classifications matching the NVDA adapter. */
unsigned reloq_wordseg_category(uint32_t codepoint);
/* Write the full Unicode case-folding of one scalar to the 12-byte buffer. */
size_t reloq_wordseg_casefold(uint32_t codepoint, unsigned char output[12]);

/* Segment a bounded ASCII English word. Returns 1 when changed, 0 when
 * retained, and -1 when output is too small or arguments are invalid.
 * The result is NUL terminated, with spaces between recognized pieces. */
int reloq_wordseg_english(const unsigned char *word, size_t length, int uk,
    unsigned char *output, size_t capacity);

/* Scoped to the serialized ECI worker, with no cross-utterance results. */
struct reloq_wordseg_options {
    int english, camel; void *dictionary; int abbreviations_disabled;
    const unsigned char *protected_text; size_t protected_length;
};
struct reloq_wordseg_options reloq_wordseg_scope(struct reloq_wordseg_options);
struct reloq_wordseg_options reloq_wordseg_current(void);

#endif
