/* Linux/glibc development probe, never linked into a shipped engine.
 * Count requested live heap bytes during a scoped public-ECI transaction.
 * Real allocator overhead, stacks, mmap arenas and RSS are not this metric. */
#include <stdatomic.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
extern void *__libc_malloc(size_t);
extern void *__libc_calloc(size_t,size_t);
extern void *__libc_realloc(void *,size_t);
extern void __libc_free(void *);
#define SLOTS 131072u
struct record {void *address;size_t size;};
static struct record records[SLOTS];
struct heap_stats {size_t peak,live,allocations,reallocations,total_requested;int overflow;};
static struct heap_stats stats;
static atomic_flag guard=ATOMIC_FLAG_INIT;
static _Thread_local int nested __attribute__((tls_model("initial-exec")));
static int active;
static void lock(void){while(atomic_flag_test_and_set_explicit(&guard,memory_order_acquire)){};}
static void unlock(void){atomic_flag_clear_explicit(&guard,memory_order_release);}
static size_t slot(void *address,int insert)
{
    size_t at=(((uintptr_t)address>>4)*UINT64_C(11400714819323198485))&(SLOTS-1),empty=SLOTS;
    for(size_t n=0;n<SLOTS;++n,at=(at+1)&(SLOTS-1)) {
        if(records[at].address==address)return at;
        if(records[at].address==(void *)(uintptr_t)1&&empty==SLOTS)empty=at;
        if(!records[at].address)return insert?(empty<SLOTS?empty:at):SLOTS;
    }
    return insert?empty:SLOTS;
}
static void drop(void *address)
{
    if(!address)return;
    size_t at=slot(address,0);
    if(at<SLOTS){stats.live-=records[at].size;records[at]=(struct record){(void *)(uintptr_t)1,0};}
}
static void add(void *address,size_t size)
{
    if(!address)return;
    size_t at=slot(address,1);
    if(at==SLOTS||size>SIZE_MAX-stats.live||size>SIZE_MAX-stats.total_requested){stats.overflow=1;return;}
    records[at]=(struct record){address,size};stats.live+=size;stats.total_requested+=size;
    if(stats.live>stats.peak)stats.peak=stats.live;
    ++stats.allocations;
}
void heap_watch_begin(void)
{
    lock();memset(records,0,sizeof records);memset(&stats,0,sizeof stats);active=1;unlock();
}
void heap_watch_end(struct heap_stats *out)
{
    lock();active=0;if(out)*out=stats;unlock();
}
void *malloc(size_t size)
{
    if(nested)return __libc_malloc(size);
    nested=1;lock();void *p=__libc_malloc(size);if(active)add(p,size);unlock();nested=0;return p;
}
void *calloc(size_t count,size_t size)
{
    if(nested)return __libc_calloc(count,size);
    nested=1;lock();void *p=__libc_calloc(count,size);
    if(active&&p){if(size&&count>SIZE_MAX/size)stats.overflow=1;else add(p,count*size);}
    unlock();nested=0;return p;
}
void *realloc(void *address,size_t size)
{
    if(nested)return __libc_realloc(address,size);
    nested=1;lock();void *p=__libc_realloc(address,size);
    if(active&&(p||!size)){drop(address);add(p,size);++stats.reallocations;}
    unlock();nested=0;return p;
}
void free(void *address)
{
    if(nested){__libc_free(address);return;}
    nested=1;lock();if(active)drop(address);__libc_free(address);unlock();nested=0;
}
