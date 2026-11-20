#include "reloq_trace_clock.h"
#include <assert.h>
#include <limits.h>

int main(void)
{
    int32_t start,end,last=-1;int64_t origin=0;
    assert(!reloq_trace_clock(0,300000,0,&start,&end,&last,&origin,1));
    assert(start==0 && end==300000);
    /* A following phone can start during the preceding phone, and a short
       sub-phone must not move the latest clock backwards. */
    assert(!reloq_trace_clock(250000,500000,0,&start,&end,&last,&origin,1));
    assert(start==250000 && end==500000 && last==500000);
    assert(!reloq_trace_clock(480000,490000,0,&start,&end,&last,&origin,1));
    assert(last==500000);
    assert(reloq_trace_clock(490000,600000,0,&start,&end,&last,&origin,0)==-1);
    assert(!reloq_trace_clock(500000,900000,0,&start,&end,&last,&origin,1));
    /* After a real clock restart, retain the exact native elapsed time. */
    assert(!reloq_trace_clock(0,100000,90,&start,&end,&last,&origin,1));
    assert(start==900000 && end==1000000 && last==100000);
    last=-1;
    assert(reloq_trace_clock(1,2,INT32_MAX,&start,&end,&last,&origin,1)==-1);
    return 0;
}
