/* Native anatomy endpoints use the common bounded C arithmetic. */
#include "reloq61_scalar_state.h"
#include "reloq_voice_filter.h"
/* Shared Chinese rule; data and identity belong to the active locale. */
extern int32_t reloq_zhcn61_run_filters(void *state,int32_t p0,int32_t p1,int32_t p2);
int32_t reloq_zhtw61_run_filters(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    return reloq_zhcn61_run_filters(state,p0,p1,p2);
}

/* Word tone percentages act on integer targets. Keeping the percentage as
 * one ratio avoids expanding a high female target past signed 32 bits. */
/* Shared Chinese rule; data and identity belong to the active locale. */
extern int32_t reloq_zhcn61_chi_word_stress_tone(void *state,int32_t p0,int32_t p1);
int32_t reloq_zhtw61_chi_word_stress_tone(void *state,int32_t p0,int32_t p1)
{
    return reloq_zhcn61_chi_word_stress_tone(state,p0,p1);
}
