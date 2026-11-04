#ifndef RELOQ_SAPI_DICTIONARY_DIFF_H
#define RELOQ_SAPI_DICTIONARY_DIFF_H
#include <stddef.h>

/* Compare valid, distinct native lookup keys within one dictionary volume.
 * Existing value edits and removals do not alter the added-key count. */
int sapi_dictionary_diff(const unsigned char *before,size_t before_size,
    const unsigned char *after,size_t after_size,int fold_root,
    size_t *added,size_t *total);
#endif
