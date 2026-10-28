/* Shared, read-only questions about input and syllable projections. */
#ifndef RELOQ61_RECORD_READERS_H
#define RELOQ61_RECORD_READERS_H
#include <stdint.h>
int32_t reloq61_record_in_syllable(void *,int32_t,int32_t,int32_t);
int32_t reloq61_record_unstressed(void *,int32_t,int32_t,unsigned);
int32_t reloq61_record_last_accent(void *);
#endif
