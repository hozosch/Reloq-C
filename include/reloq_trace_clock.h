/* Convert the provider's per-sentence phone or syllable clock to absolute
 * 1/10000 ms units. Phone intervals may overlap during coarticulation. */
#ifndef RELOQ_TRACE_CLOCK_H
#define RELOQ_TRACE_CLOCK_H
#include <stdint.h>
#include <limits.h>

static inline int reloq_trace_clock(int32_t from,int32_t to,int64_t ticks,
    int32_t *start,int32_t *end,int32_t *previous_to,int64_t *origin,int allow_overlap)
{
    if(!start || !end || !previous_to || !origin || from<0 || to<=from ||
       ticks<0 || *previous_to< -1 || *origin<0 || *origin>INT32_MAX)return -1;
    int reset=*previous_to<0 || (from<*previous_to && (int64_t)*previous_to-from>500000);
    if(reset) {
        if(ticks>INT32_MAX/10000)return -1;
        int64_t next_origin=ticks*10000;
        if(*previous_to>=0 && next_origin+from<*origin+*previous_to)return -1;
        *origin=next_origin;
    } else if(from<*previous_to && !allow_overlap)return -1;
    if(*origin+to>INT32_MAX)return -1;
    if(reset || !allow_overlap || to>*previous_to)*previous_to=to;
    *start=(int32_t)(*origin+from);*end=(int32_t)(*origin+to);
    return 0;
}
#endif
