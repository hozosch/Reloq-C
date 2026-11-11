#ifndef RELOQ_SAPI_DICTIONARY_MERGE_H
#define RELOQ_SAPI_DICTIONARY_MERGE_H
#include <stddef.h>

struct sapi_dictionary_input {const unsigned char *data;size_t size;};
/* Merge one native dictionary volume. Later sources and later entries win;
 * source buffers remain caller-owned. On success, caller frees *output. */
int sapi_dictionary_merge(const struct sapi_dictionary_input *inputs,size_t count,
                          int fold_root,unsigned char **output,size_t *size);
#endif
