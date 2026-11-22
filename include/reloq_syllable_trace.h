/* Optional request-local observation of the provider's syllable clock. */
#ifndef RELOQ_SYLLABLE_TRACE_H
#define RELOQ_SYLLABLE_TRACE_H
#include <stdint.h>
typedef void (*reloq_syllable_sink)(void *context, int32_t from, int32_t to);
void reloq61_syllable_trace_set(reloq_syllable_sink sink, void *context);
void reloq61_syllable_trace_emit(void *state,int32_t from, int32_t to);
/* Optional editor mode: record individual native phone intervals instead of
 * syllables. The same request-local sink and native time units are used. */
void reloq61_syllable_trace_phone_mode(int enabled);
void reloq61_phone_trace_emit(void *state,int32_t from,int32_t to);
/* Optional early diagnostic: the candidate nucleus clock after duration
 * assignment and before intonation. Like phone mode, this selects what the
 * current request-local timing sink receives. */
void reloq61_syllable_trace_nucleus_mode(int enabled);
int reloq61_nucleus_trace_enabled(void);
void reloq61_nucleus_trace_emit(void *state,int32_t from,int32_t to);
void reloq61_nucleus_identity_trace_emit(uint32_t identity);
/* Optional identity trace for the same phone events. The value is a private
 * provider phone slot, not a Persian linguistic phone ID. */
typedef void (*reloq_phone_identity_sink)(void *context,uint32_t identity);
void reloq61_phone_identity_trace_set(reloq_phone_identity_sink sink,void *context);
void reloq61_phone_identity_trace_emit(uint32_t identity);
#endif
