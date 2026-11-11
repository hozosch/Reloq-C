#ifndef RELOQ_NVDA_SPEECH_H
#define RELOQ_NVDA_SPEECH_H
#include <stddef.h>

struct reloq_nvda_speech;
struct reloq_nvda_speech *reloq_nvda_speech_new(void);
void reloq_nvda_speech_free(struct reloq_nvda_speech *dictionary);
/* Validate and append all entries from one UTF-8 file atomically. */
int reloq_nvda_speech_add(struct reloq_nvda_speech *dictionary,const unsigned char *data,size_t length);
/* Apply entries in order. The result and source-offset map are caller-owned.
 * map[i] is the corresponding UTF-8 byte boundary in the result. */
int reloq_nvda_speech_apply(const struct reloq_nvda_speech *dictionary,const char *text,size_t length,
    char **result,size_t *result_length,size_t **map,int *matched);
#endif
