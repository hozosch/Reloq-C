/* Persian source-pattern normalization, GPL-3.0-or-later. */
#ifndef RELOQ_DIRECT_FA_NORMALIZE_H
#define RELOQ_DIRECT_FA_NORMALIZE_H
#include <stddef.h>

/* Allocates UTF-8 output and a source-byte to output-byte map. Returns 0 on
 * success, -1 for invalid arguments/UTF-8, -2 for allocation failure. The
 * caller frees both arrays with reloq_fa_direct_free(). No eSpeak runtime. */
int reloq_fa_direct_normalize(const char *text,size_t length,char **output,
                              size_t **source_map,size_t *error_offset);
#endif
