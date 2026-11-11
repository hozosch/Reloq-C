#ifndef RELOQ_NVDA_DICTIONARY_H
#define RELOQ_NVDA_DICTIONARY_H

#include <stddef.h>

/* One UTF-8 NVDA speech-dictionary entry. Strings are valid only during the
 * callback; the caller must copy them to keep them. "kind" uses NVDA's
 * numeric EntryType values, not the Eloquence dictionary volume numbers. */
struct reloq_nvda_entry {
    const char *pattern, *replacement;
    size_t pattern_length, replacement_length;
    unsigned case_sensitive, kind;
};
typedef int (*reloq_nvda_entry_fn)(void *context, const struct reloq_nvda_entry *entry);

enum reloq_nvda_parse_result {
    RELOQ_NVDA_VALID = 0,
    RELOQ_NVDA_INVALID = -1,
    RELOQ_NVDA_MEMORY = -2,
    RELOQ_NVDA_CALLBACK = -3
};

/* Parse the complete file. Invalid UTF-8, NULs, oversized files, bad option
 * fields and mixed native/NVDA files are errors. A caller that publishes a
 * dictionary must also validate every pattern using its regexp engine. */
int reloq_nvda_parse(const unsigned char *data, size_t length,
                     reloq_nvda_entry_fn callback, void *context, size_t *count);

#endif
