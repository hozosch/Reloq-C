#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern void *evv_low_alloc(size_t);
extern void evv_low_free(void *);

enum { THREADS = 8, ROUNDS = 1000, BYTES = 64 };
static pthread_barrier_t barrier;
static atomic_int failures;

static void *worker(void *argument)
{
    unsigned char value=(unsigned char)(uintptr_t)argument;
    for(int round=0;round<ROUNDS;++round) {
        pthread_barrier_wait(&barrier);
        unsigned char *memory=evv_low_alloc(BYTES);
        if(memory)memset(memory,value,BYTES);
        else atomic_fetch_add(&failures,1);
        pthread_barrier_wait(&barrier);
        if(memory)for(size_t i=0;i<BYTES;++i)if(memory[i]!=value) {
            atomic_fetch_add(&failures,1);break;
        }
        pthread_barrier_wait(&barrier);
        evv_low_free(memory);
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[THREADS];
    if(pthread_barrier_init(&barrier,NULL,THREADS))return 1;
    for(uintptr_t i=0;i<THREADS;++i)
        if(pthread_create(&threads[i],NULL,worker,(void *)(i+1)))return 1;
    for(int i=0;i<THREADS;++i)pthread_join(threads[i],NULL);
    pthread_barrier_destroy(&barrier);
    int errors=atomic_load(&failures);
    if(errors){fprintf(stderr,"Low arena concurrent allocation errors: %d\n",errors);return 1;}
    printf("Passed %d concurrent low-arena allocations.\n",THREADS*ROUNDS);
    return 0;
}
