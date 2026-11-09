/* Round-trip time gaps around the 16-bit sign and escape boundaries. */
#include "eci_tvqueue.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

typedef struct StreamArray {
    TimeValueQueue queue;
    char *name;
    uint32_t written,read;
} StreamArray;

extern void *tvq_ctor(TimeValueQueue *,uint16_t);
extern void tvq_dtor(TimeValueQueue *);
extern int32_t sa_appendValue(StreamArray *,uint32_t,int16_t);
extern int32_t sa_fetchNext(StreamArray *,uint32_t *,int32_t *);

int main(void)
{
    const uint32_t times[]={0,32767,65535,131070,196606,0x90000000u,0x90010000u};
    const int16_t values[]={0,123,-12,32767,-32768,42,-91};
    StreamArray stream={0};
    assert(tvq_ctor(&stream.queue,4));
    for(unsigned i=0;i<sizeof times/sizeof *times;++i)
        assert(sa_appendValue(&stream,times[i],values[i]));
    for(unsigned i=0;i<sizeof times/sizeof *times;++i) {
        uint32_t time;int32_t value;
        assert(sa_fetchNext(&stream,&time,&value));
        assert(time==times[i]&&value==values[i]);
    }
    uint32_t time;int32_t value;
    assert(!sa_fetchNext(&stream,&time,&value));
    tvq_dtor(&stream.queue);
    puts("ECI stream times: signed boundary and escaped timestamps round-trip");
    return 0;
}
