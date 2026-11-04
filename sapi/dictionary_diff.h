#ifndef RELOQ_SAPI_DICTIONARY_DIFF_H
#define RELOQ_SAPI_DICTIONARY_DIFF_H
#include <stddef.h>
#include <stdint.h>

/* Compare valid, distinct native lookup keys within one dictionary volume.
 * Existing value edits and removals do not alter the added-key count. */
int sapi_dictionary_diff(const unsigned char *before,size_t before_size,
    const unsigned char *after,size_t after_size,int fold_root,
    size_t *added,size_t *total);
/* A Windows message carries both counts without allocating a result that can
 * outlive the settings window. Zero represents an unavailable comparison. */
uint32_t sapi_dictionary_pack_counts(size_t entries,size_t files,int compared);
int sapi_dictionary_unpack_counts(uint32_t value,size_t *entries,size_t *files,int *compared);
#endif
