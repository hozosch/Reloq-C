#ifndef RELOQ_PERSIAN_US_PROXY_H
#define RELOQ_PERSIAN_US_PROXY_H

#include <stddef.h>

/* The caller owns a dedicated US pronunciation worker. The probe emits its
 * complete phoneme display through the sink and returns zero on success. */
typedef int (*reloq_fa_proxy_sink)(void *context,const char *bytes,size_t length);
typedef int (*reloq_fa_proxy_probe)(void *context,const char *text,size_t length,
    reloq_fa_proxy_sink sink,void *sink_context);

struct reloq_fa_us_proxy {
    char *text;
    size_t length;
    /* One boundary before every word after the first; byte offsets into text.
     * When a boundary precedes punctuation, text includes the space required
     * by the native index callback after that boundary. */
    size_t *word_offsets;
    size_t word_count;
};

/* The input is the Persian acoustic projection. The output is the silent US
 * pilot, never Persian pronunciation. Return 0 on success, 1 for invalid
 * input, -1 if the pronunciation worker fails, or -2 if memory is exhausted.
 * On failure result is empty. No process, NVDA component or global state is
 * required. */
int reloq_fa_us_proxy_make(const char *input,size_t length,
    reloq_fa_proxy_probe probe,void *context,struct reloq_fa_us_proxy *result);
/* For the dedicated, deterministic ECI worker only: reuse an isolated
 * candidate check within this request. Nothing survives the request. */
int reloq_fa_us_proxy_make_reusing(const char *input,size_t length,
    reloq_fa_proxy_probe probe,void *context,struct reloq_fa_us_proxy *result);
void reloq_fa_us_proxy_clear(struct reloq_fa_us_proxy *result);

#endif
