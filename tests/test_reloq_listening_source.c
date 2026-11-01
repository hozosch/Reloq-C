/* Check the listening filter's clock bypass and streaming state without a
 * language provider. Real-speech independence is checked by its generator. */
#include "reloq_rate_transfer.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int reloq_source_listening_set(unsigned,double);
void __wrap_reloq_rate_transfer_process_voice(struct reloq_rate_transfer *,float *,size_t);
void __real_reloq_rate_transfer_process_voice(struct reloq_rate_transfer *s,float *x,size_t n)
{(void)s;(void)x;(void)n;}

static void filter(float *x,size_t n,float rate,unsigned profile,size_t block)
{
    struct reloq_rate_transfer state={0};state.presence_rate=rate;
    assert(!reloq_source_listening_set(profile,1));
    for(size_t at=0;at<n;at+=block) {
        size_t count=n-at<block?n-at:block;
        __wrap_reloq_rate_transfer_process_voice(&state,x+at,count);
    }
}
static double tone_gain(float rate,unsigned profile,double hz)
{
    float x[12000];double in=0,out=0;
    for(size_t j=0;j<12000;++j)x[j]=(float)(1000*sin(6.283185307179586*hz*j/rate));
    for(size_t j=2000;j<12000;++j)in+=(double)x[j]*x[j];
    filter(x,12000,rate,profile,137);
    for(size_t j=2000;j<12000;++j)out+=(double)x[j]*x[j];
    return 10*log10(out/in);
}
int main(void)
{
    assert(reloq_source_listening_set(81,1)==-1);
    assert(reloq_source_listening_set(1,-1)==-1);
    assert(reloq_source_listening_set(1,NAN)==-1);
    const float rates[]={11025,16000,22050,32000,44100,48000};
    const unsigned profiles[]={0,40,60};
    for(unsigned r=0;r<6;++r)for(unsigned p=0;p<3;++p) {
        float a[1024],b[1024],original[1024];
        for(size_t j=0;j<1024;++j)original[j]=(float)(sin(j*.237)+cos(j*.131));
        memcpy(a,original,sizeof a);memcpy(b,original,sizeof b);
        filter(a,1024,rates[r],profiles[p],1024);
        filter(b,1024,rates[r],profiles[p],17);
        assert(!memcmp(a,b,sizeof a));
        if(!p||!r)assert(!memcmp(a,original,sizeof a));
    }
    for(unsigned p=1;p<3;++p) {
        double low=tone_gain(48000,profiles[p],100);
        double high=tone_gain(48000,profiles[p],4000);
        assert(fabs(low)<.01);
        assert(high>(p==1?6:10));
        assert(high<(p==1?8.01:12.01));
        printf("profile %u: 100 Hz %.4f dB, 4 kHz %.4f dB\n",profiles[p],low,high);
    }
    puts("Source filter: 18 exact block/reset comparisons and clock bypasses passed");
    return 0;
}
