/* Standalone Linux check for the scoped R source-queue rewrite.
 * Build without PIE so the private 32-bit test reference can address the
 * static language object. See the actual acoustic tests for Windows/Android. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../vendor/openevv61/src/eci/bridge/eci_stmarray.c"
static DeltaLang language;
static delta_state state;
static StreamArray stream;
static StreamArrayList list;
int main(void) {
    assert((uintptr_t)&language <= INT32_MAX);
    state.dlang=(int32_t)(uintptr_t)&language;
    list.streams=&stream; list.count=1; list.named=1; language.list=&list;
    tvq_ctor(&stream.queue,8); assert(stream.queue.buf);
    assert(sa_appendValue(&stream,0,0));
    assert(sa_appendValue(&stream,100,10));
    assert(sa_appendValue(&stream,200,40));
    assert(sa_appendValue(&stream,250,45));
    assert(sa_appendValue(&stream,300,50));
    assert(sa_appendValue(&stream,400,0));
    assert(sa_appendValue(&stream,1000,60));
    assert(sa_appendValue(&stream,71000,5));
    assert(reloq61_array_bank_zero_segment(&state,0,210,300));
    const uint32_t times[]={0,100,200,210,300,400,1000,71000};
    const int32_t vals[]={0,10,40,0,0,0,60,5};
    for(unsigned i=0;i<sizeof times/sizeof *times;i++) {
        uint32_t t=0;int32_t v=-1;assert(sa_fetchNext(&stream,&t,&v));
        assert(t==times[i] && v==vals[i]);
    }
    assert(!reloq61_array_bank_zero_segment(&state,0,210,300));
    tvq_dtor(&stream.queue);
    stream=(StreamArray){0}; tvq_ctor(&stream.queue,8); assert(stream.queue.buf);
    for (uint32_t t=1;t<=6;t++) assert(sa_appendValue(&stream,t,(int16_t)t));
    for (int i=0;i<3;i++) {uint32_t t;int32_t v;assert(sa_fetchNext(&stream,&t,&v));}
    for (uint32_t t=7;t<=9;t++) assert(sa_appendValue(&stream,t,(int16_t)t));
    assert(stream.queue.head > stream.queue.tail);
    assert(reloq61_array_bank_zero_segment(&state,0,5,7));
    const uint32_t wrap_times[]={4,5,7,8,9};
    const int32_t wrap_values[]={4,0,0,8,9};
    for(unsigned i=0;i<sizeof wrap_times/sizeof *wrap_times;i++) {
        uint32_t t=0;int32_t v=-1;assert(sa_fetchNext(&stream,&t,&v));
        assert(t==wrap_times[i] && v==wrap_values[i]);
    }
    tvq_dtor(&stream.queue);
    puts("Passed queue rewrite, preservation, escaped time and stale-clock checks.");
}
