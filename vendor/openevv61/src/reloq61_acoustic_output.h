#ifndef RELOQ61_ACOUSTIC_OUTPUT_H
#define RELOQ61_ACOUSTIC_OUTPUT_H
#include <stdint.h>
typedef int16_t (*reloq61_endpoint_filter)(void *,int16_t,int16_t);
int32_t reloq61_append_filtered_interval(void *,int32_t,int32_t,int32_t,int32_t,reloq61_endpoint_filter);
void reloq61_submit_interval(void *,int16_t,int16_t,int32_t,int32_t);
#endif
