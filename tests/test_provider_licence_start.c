/* Hold the first licence constructor open while a second engine starts.
 * Neither caller may read or use the object until construction completes. */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef struct RequestLicense { const void *vt; int32_t granted; } RequestLicense;
extern int getObject(int32_t kind, void **out);
const uint32_t ew_bytes = 1;
static atomic_int constructing, release_constructor, second_started, second_done, early_read;

RequestLicense *rl_ctor(RequestLicense *r)
{
    atomic_store(&constructing,1);
    while (!atomic_load(&release_constructor)) sched_yield();
    r->vt=NULL; r->granted=1;
    return r;
}
int32_t rl_licenseGranted(RequestLicense *r)
{
    if (!r->granted) atomic_store(&early_read,1);
    return 0; /* Do not allocate an engine; only the ordering is under test. */
}
void *cpp_new(uint32_t n) { (void)n; return NULL; }
void *ew_ctor(void *p) { return p; }

static void *first(void *unused)
{
    (void)unused; void *out=NULL; (void)getObject(1,&out); return NULL;
}
static void *second(void *unused)
{
    (void)unused; void *out=NULL; atomic_store(&second_started,1);
    (void)getObject(1,&out); atomic_store(&second_done,1); return NULL;
}

int main(void)
{
    pthread_t a,b;
    assert(!pthread_create(&a,NULL,first,NULL));
    while (!atomic_load(&constructing)) sched_yield();
    assert(!pthread_create(&b,NULL,second,NULL));
    while (!atomic_load(&second_started)) sched_yield();
    struct timespec pause={0,30000000};
    nanosleep(&pause,NULL);
    int finished_early=atomic_load(&second_done);
    atomic_store(&release_constructor,1);
    assert(!pthread_join(a,NULL)); assert(!pthread_join(b,NULL));
    assert(!finished_early && !atomic_load(&early_read));
    puts("Passed concurrent provider initialization ordering.");
    return 0;
}
