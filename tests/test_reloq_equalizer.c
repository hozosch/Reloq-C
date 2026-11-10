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
static double peak_tone(int rate,double frequency,int enabled)
{
    struct reloq_equalizer_peak peaks[RELOQ_EQ_FILTER_COUNT];
    for(int i=0;i<RELOQ_EQ_FILTER_COUNT;++i)peaks[i]=(struct reloq_equalizer_peak){0,1000,0,100};
    peaks[0]=(struct reloq_equalizer_peak){enabled,1000,6,100};
    struct reloq_equalizer eq;int16_t pcm[N];int n=rate/10;
    assert(!reloq_equalizer_init_extended(&eq,rate,0,0,0,peaks));
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
        /* Boosting one band must not lower the rest of ordinary speech. */
        double bass=tone_ratio(rate,100,6,0,0)/tone_ratio(rate,3500,6,0,0);
        double middle=tone_ratio(rate,1200,0,6,0)/tone_ratio(rate,100,0,6,0);
        double treble=tone_ratio(rate,3500,0,0,6)/tone_ratio(rate,100,0,0,6);
        assert(bass>1.4 && middle>1.4 && treble>1.3);
        double unchanged_high=tone_ratio(rate,3500,6,0,0)/tone_ratio(rate,3500,0,0,0);
        double unchanged_low=tone_ratio(rate,100,0,0,6)/tone_ratio(rate,100,0,0,0);
        assert(unchanged_high>0.97 && unchanged_high<1.03);
        assert(unchanged_low>0.97 && unchanged_low<1.03);
        double at_peak=peak_tone(rate,1000,1)/peak_tone(rate,1000,0);
        double away=peak_tone(rate,150,1)/peak_tone(rate,150,0);
        assert(at_peak>1.9&&at_peak<2.1&&away>0.98&&away<1.08);
        double lower_edge=peak_tone(rate,707.1068,1)/peak_tone(rate,707.1068,0);
        double upper_edge=peak_tone(rate,1414.2136,1)/peak_tone(rate,1414.2136,0);
        assert(lower_edge>1.30&&lower_edge<1.55&&upper_edge>1.30&&upper_edge<1.55);
        struct reloq_equalizer_peak peaks[RELOQ_EQ_FILTER_COUNT];
        for(int i=0;i<RELOQ_EQ_FILTER_COUNT;++i)peaks[i]=(struct reloq_equalizer_peak){0,1000,0,100};
        peaks[0]=(struct reloq_equalizer_peak){1,(rate+1)/2,12,400};
        assert(!reloq_equalizer_init_extended(&flat,rate,0,0,0,peaks)&&!flat.active);
        memcpy(a,input,sizeof a);reloq_equalizer_process(&flat,a,N);assert(!memcmp(a,input,sizeof a));
        for(int i=0;i<RELOQ_EQ_FILTER_COUNT;++i)
            peaks[i]=(struct reloq_equalizer_peak){1,20+i*(rate/2-21)/4,i%2? -12:12,i%2?10:400};
        assert(!reloq_equalizer_init_extended(&one,rate,0,0,0,peaks));
        assert(!reloq_equalizer_init_extended(&chunks,rate,0,0,0,peaks));
        memcpy(a,input,sizeof a);memcpy(b,input,sizeof b);
        reloq_equalizer_process(&one,a,N);
        for(int i=0;i<N;i+=37)reloq_equalizer_process(&chunks,b+i,(size_t)(N-i<37?N-i:37));
        assert(!memcmp(a,b,sizeof a));
        for(int i=0;i<N;++i)assert(a[i]>=-32761&&a[i]<=32761);
        for(int i=0;i<one.band_count;++i)assert(isfinite(one.bands[i].z1)&&isfinite(one.bands[i].z2));
        /* The widest, highest boosted filter immediately below Nyquist must
         * remain finite on long input, even at the smallest sample rate. */
        peaks[0]=(struct reloq_equalizer_peak){1,rate/2-1,12,400};
        assert(!reloq_equalizer_init_extended(&one,rate,0,0,0,peaks));
        for(int block=0;block<100;++block){
            for(int i=0;i<N;++i)a[i]=(int16_t)(((i+block*211u)*7927u)%65536u-32768);
            reloq_equalizer_process(&one,a,N);
            for(int i=0;i<N;++i)assert(a[i]>=-32761&&a[i]<=32761);
            for(int i=0;i<one.band_count;++i)assert(isfinite(one.bands[i].z1)&&isfinite(one.bands[i].z2));
        }
        /* An actual boosted peak is limited without relying on PCM clipping;
         * the same state recovers when the signal stops. */
        assert(!reloq_equalizer_init(&one,rate,0,12,0));
        int n=rate/10;
        for(int i=0;i<n;++i)a[i]=(int16_t)(30000*sin(2*PI*1200*i/rate));
        reloq_equalizer_process(&one,a,(size_t)n);
        assert(one.limiter_gain>0 && one.limiter_gain<0.8);
        for(int i=0;i<n;++i)assert(a[i]>=-32761 && a[i]<=32761);
        for(int i=0;i<10;++i){memset(a,0,(size_t)n*sizeof *a);reloq_equalizer_process(&one,a,(size_t)n);}
        assert(one.limiter_gain>0.99);
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
    puts("Equalizer: neutral bypass, selectable filters, Nyquist, limiter and chunk continuity passed at seven rates");
    return 0;
}
