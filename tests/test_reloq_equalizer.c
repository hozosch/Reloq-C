#include "reloq_equalizer.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PI 3.14159265358979323846
#define N 4800

static double tone_ratio(int rate, double frequency, int low, int mid, int high)
{
    struct reloq_equalizer eq;
    int16_t pcm[N];int n=rate/10;
    assert(n<=N && !reloq_equalizer_init(&eq,rate,low,mid,high));
    for(int i=0;i<n;++i)pcm[i]=(int16_t)(4000*sin(2*PI*frequency*i/rate));
    reloq_equalizer_process(&eq,pcm,(size_t)n);
    double sum=0;for(int i=n/2;i<n;++i)sum+=(double)pcm[i]*pcm[i];
    return sqrt(sum/(n-n/2));
}

int main(void)
{
    const int rates[]={8000,11025,16000,22050,32000,44100,48000};
    for(unsigned k=0;k<sizeof rates/sizeof *rates;++k) {
        int rate=rates[k];int16_t input[N],a[N],b[N];
        for(int i=0;i<N;++i)input[i]=(int16_t)((i*7927u)%65536u-32768);
        struct reloq_equalizer flat,one,chunks;
        assert(!reloq_equalizer_init(&flat,rate,0,0,0));
        memcpy(a,input,sizeof a);reloq_equalizer_process(&flat,a,N);
        assert(!flat.active && !memcmp(a,input,sizeof a));
        assert(!reloq_equalizer_init(&one,rate,6,-4,3));
        assert(!reloq_equalizer_init(&chunks,rate,6,-4,3));
        memcpy(a,input,sizeof a);memcpy(b,input,sizeof b);
        reloq_equalizer_process(&one,a,N);
        for(int i=0;i<N;i+=37)reloq_equalizer_process(&chunks,b+i,(size_t)(N-i<37?N-i:37));
        assert(!memcmp(a,b,sizeof a) && memcmp(a,input,sizeof a));
        for(int band=0;band<3;++band)
            assert(isfinite(one.bands[band].z1)&&isfinite(one.bands[band].z2));
        /* A boost must shape its intended band relative to the other bands,
         * independently of the overall headroom applied for clipping. */
        double bass=tone_ratio(rate,100,6,0,0)/tone_ratio(rate,3500,6,0,0);
        double middle=tone_ratio(rate,1200,0,6,0)/tone_ratio(rate,100,0,6,0);
        double treble=tone_ratio(rate,3500,0,0,6)/tone_ratio(rate,100,0,0,6);
        assert(bass>1.4 && middle>1.4 && treble>1.3);
        for(int gain=-12;gain<=12;gain+=24) {
            assert(!reloq_equalizer_init(&one,rate,gain,gain,gain));
            memcpy(a,input,sizeof a);reloq_equalizer_process(&one,a,N);
            for(int band=0;band<3;++band)
                assert(isfinite(one.bands[band].z1)&&isfinite(one.bands[band].z2));
        }
    }
    struct reloq_equalizer invalid;
    assert(reloq_equalizer_init(&invalid,8000,-13,0,0)<0);
    assert(reloq_equalizer_init(&invalid,48000,0,13,0)<0);
    puts("Equalizer: neutral bypass, response, chunk continuity and bounds passed at seven rates");
    return 0;
}
