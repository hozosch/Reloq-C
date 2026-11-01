/* Listening-only link wrapper. It is never built into release packages.
 * Run one serialized ECI engine per process. Select/reset a profile only
 * between completed utterances. The original source, tract and noise paths
 * remain those of 1.0; these sections see periodic excitation alone. */
#include "reloq_rate_transfer.h"
#include "reloq_noise_profile.h"
#include "reloq_core.h"
#include <math.h>
#include <string.h>

struct colour_section { double c[5],d[2]; };
static struct {
    unsigned profile;
    double gain;
    float rate;
    struct colour_section body,clarity;
} listening={.gain=1};

__attribute__((visibility("default")))
int reloq_listening_set_profile(unsigned profile,double gain)
{
    if(profile>7 || !isfinite(gain) || gain<0 || gain>4)return -1;
    memset(&listening,0,sizeof listening);
    listening.profile=profile;listening.gain=gain;return 0;
}

static void prepare(float rate)
{
    const double pi=3.14159265358979323846;
    double body_db=rate>=48000?3:rate>=44100?2.5:rate>=32000?2:
        rate>=22050?1.5:1;
    double clarity_db=rate>=48000?3.5:rate>=44100?3:rate>=32000?2.5:
        rate>=22050?1.5:1;
    double a=pow(10,body_db/40),w=2*pi*900/rate;
    double z=sin(w)*sqrt(2*a),co=cos(w);
    double den=(a+1)+(a-1)*co+z;
    double *c=listening.body.c;
    c[0]=a*((a+1)-(a-1)*co+z)/den;
    c[1]=2*a*((a-1)-(a+1)*co)/den;
    c[2]=a*((a+1)-(a-1)*co-z)/den;
    c[3]=-2*((a-1)+(a+1)*co)/den;c[4]=((a+1)+(a-1)*co-z)/den;
    a=pow(10,clarity_db/40);w=2*pi*4900/rate;
    z=sin(w)*sqrt(2*a);co=cos(w);
    den=(a+1)-(a-1)*co+z;c=listening.clarity.c;
    c[0]=a*((a+1)+(a-1)*co+z)/den;
    c[1]=-2*a*((a-1)+(a+1)*co)/den;
    c[2]=a*((a+1)+(a-1)*co-z)/den;
    c[3]=2*((a-1)-(a+1)*co)/den;c[4]=((a+1)-(a-1)*co-z)/den;
    listening.rate=rate;
}
static double section(struct colour_section *s,double x)
{
    double y=s->c[0]*x+s->d[0];
    s->d[0]=s->c[1]*x-s->c[3]*y+s->d[1];
    s->d[1]=s->c[2]*x-s->c[4]*y;return y;
}
void __real_reloq_rate_transfer_process_voice(struct reloq_rate_transfer *,float *,size_t);
void __wrap_reloq_rate_transfer_process_voice(struct reloq_rate_transfer *s,
    float *samples,size_t count)
{
    __real_reloq_rate_transfer_process_voice(s,samples,count);
    const int active=s->presence_rate>=16000 && (listening.profile&3);
    if(!active && listening.gain==1)return;
    if(active && listening.rate!=s->presence_rate)prepare(s->presence_rate);
    for(size_t j=0;j<count;++j) {
        double y=samples[j];
        if(active && (listening.profile&1))y=section(&listening.body,y);
        if(active && (listening.profile&2))y=section(&listening.clarity,y);
        samples[j]=(float)(y*listening.gain);
    }
}

void __real_reloq_rate_transfer_prepare_noise_mix_profile(struct reloq_rate_transfer *,
    const struct reloq_core_frication_bank *,const struct reloq_core_frication_bank *,
    const struct reloq_core_frication_bank *,const struct reloq_noise_profile *,float,float,int);
void __wrap_reloq_rate_transfer_prepare_noise_mix_profile(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *low,const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,const struct reloq_noise_profile *profile,
    float rate,float mix,int historic)
{
    if((listening.profile&4) && rate>=16000 && mix>0 && profile->kind==1) {
        /* S-only trial: fit the classic 11.025-kHz lower bank directly.
         * Suppress the companion's noise and bypass the extra source tilt.
         * The existing bounded causal fit retains integrated AF energy;
         * this neither resamples audio nor recolours other noise classes. */
        struct reloq_core_frication_bank empty={0};
        __real_reloq_rate_transfer_prepare_noise_mix_profile(s,low,&empty,current,
            profile,rate,1,1);
    } else __real_reloq_rate_transfer_prepare_noise_mix_profile(s,low,high,current,
        profile,rate,mix,historic);
}
