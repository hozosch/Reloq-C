/* Bounded native Persian phrase probe, GPL-3.0-or-later. */
#ifndef RELOQ_DIRECT_FA_TEXT_H
#define RELOQ_DIRECT_FA_TEXT_H
#include <stddef.h>

/* Normalization followed by whole-phrase conversion succeeds only if every
 * word is in the experimental
 * direct lexicon and punctuation is supported. There is no partial output on
 * failure. The caller frees both output arrays with reloq_fa_direct_free().
 * source_map maps each source UTF-8 byte boundary to the output byte offset;
 * within a word or number it only identifies the beginning, not an exact phoneme.
 * Returns 0 on success, 1 for uncovered text, 2 for malformed or unsupported
 * numeric syntax, -1 for invalid input, -2 for allocation failure.
 * error_offset is a source UTF-8 byte offset. */
int reloq_fa_direct_text(const char *text, size_t length, char **output,
                         size_t **source_map, size_t *error_offset);

/* Convert a complete Persian phrase and project source-byte event positions
 * to phoneme-tag boundaries in the same request. Non-index events inside a
 * word are rejected; indices inside a word map to its beginning. Offsets are
 * written only on success. The caller releases output with direct_free.
 * Returns the direct_text status, or 3 for an unsafe event position. */
int reloq_fa_direct_project(const char *text,size_t length,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
void reloq_fa_direct_free(void *pointer);
#endif
