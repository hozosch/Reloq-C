/* Optional request-local observation of the provider's syllable clock. */
#ifndef RELOQ_SYLLABLE_TRACE_H
#define RELOQ_SYLLABLE_TRACE_H
#include <stdint.h>
typedef void (*reloq_syllable_sink)(void *context, int32_t from, int32_t to);
void reloq61_syllable_trace_set(reloq_syllable_sink sink, void *context);
void reloq61_syllable_trace_emit(int32_t from, int32_t to);
#endif
