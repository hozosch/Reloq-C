#ifndef RELOQ_ENGLISH_TRACT_H
#define RELOQ_ENGLISH_TRACT_H
#include <stddef.h>
#include <stdint.h>

/* Positive values own a formant target. -1 retains the other endpoint;
 * two absent endpoints leave the preceding/following gestures connected. */
struct reloq_english_tract_span {int64_t begin,end;int value;};
int reloq_english_tract_targets(int left,int right,int64_t contact,int64_t end,
    struct reloq_english_tract_span out[2],size_t *count);
int reloq_english_tract_nuclear(unsigned char phone);
#endif
