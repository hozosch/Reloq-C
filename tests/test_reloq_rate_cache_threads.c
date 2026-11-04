/* Exercise simultaneous cold publication without the separate ECI frontend. */
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include "../src/reloq_rate_transfer.c"

enum {THREADS=12};
static atomic_int ready,go;
static void *run(void *arg)
{
    int index=*(const int *)arg,kind=index%4;float rate=48000;
    struct reloq_noise_profile p={0};p.kind=kind;p.rate=rate;p.upper_target=kind*.125;
    for(unsigned j=0;j<6;++j) {
        p.target[j][0]=.8+j*.01;p.target[j][1]=-.12;p.target[j][2]=.04;
        p.target[j][3]=-.15;p.target[j][4]=.07;
    }
    atomic_fetch_add_explicit(&ready,1,memory_order_release);
    while(!atomic_load_explicit(&go,memory_order_acquire)){}
    const struct noise_clock_grid *clock=get_noise_clock(4,rate);
    const struct noise_grid *grid=get_noise_grid(&p,4,rate,clock);assert(grid);
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE;
        double direct=profile_power(&p,hz);
        double classic=hz<=11025?classic_colour_power(hz):0;
        double extension=hz<=5512.5?0:hz>=6500?1:
            .5-.5*cos(pi*(hz-5512.5)/987.5);
        double ratio=hz/8500,quadratic=ratio*ratio;
        double retracted=1/(1+quadratic*quadratic);
        assert(!memcmp(&direct,grid->profile+k,sizeof direct));
        assert(!memcmp(&classic,clock->classic+k,sizeof classic));
        assert(!memcmp(&extension,clock->extension+k,sizeof extension));
        assert(!memcmp(&retracted,clock->retracted+k,sizeof retracted));
    }
    return NULL;
}
int main(void)
{
    pthread_t threads[THREADS];int indices[THREADS];
    for(int i=0;i<THREADS;++i){indices[i]=i;assert(!pthread_create(threads+i,NULL,run,indices+i));}
    while(atomic_load_explicit(&ready,memory_order_acquire)<THREADS){}
    atomic_store_explicit(&go,1,memory_order_release);
    for(int i=0;i<THREADS;++i)assert(!pthread_join(threads[i],NULL));
    printf("Noise filter cache: %d concurrent cold initializations match direct responses\n",THREADS);
    return 0;
}
