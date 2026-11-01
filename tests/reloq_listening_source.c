/* Listening-only upper-harmonic source colour. Never a release target.
 * Serialized engine; select/reset only between completed utterances.
 * Period, pulse shape, tract and noise masks remain native. Only periodic
 * excitation enters this one-section shelf, before AH/AF are mixed in. */
#include "reloq_rate_transfer.h"
#include <math.h>
#include <string.h>

static struct {
    unsigned profile;
    double gain,c[5],d[2];
    float rate;
} listening={.gain=1};

__attribute__((visibility("default")))
int reloq_source_listening_set(unsigned profile,double gain)
{
    if(profile>80 || !isfinite(gain) || gain<0 || gain>4)return -1;
    memset(&listening,0,sizeof listening);
    listening.profile=profile;listening.gain=gain;return 0;
}

static void prepare(float rate)
{
    const double pi=3.14159265358979323846;
    double weight=rate>=48000?1:rate>=44100?.8:rate>=32000?.6:rate>=22050?.4:.2;
    double a=pow(10,listening.profile*.2*weight/40);
    double w=2*pi*1800/rate,co=cos(w),z=sin(w)*sqrt(2*a);
    double den=(a+1)-(a-1)*co+z,*c=listening.c;
    c[0]=a*((a+1)+(a-1)*co+z)/den;
    c[1]=-2*a*((a-1)+(a+1)*co)/den;
    c[2]=a*((a+1)+(a-1)*co-z)/den;
    c[3]=2*((a-1)-(a+1)*co)/den;
    c[4]=((a+1)-(a-1)*co-z)/den;
    listening.rate=rate;
}

void __real_reloq_rate_transfer_process_voice(struct reloq_rate_transfer *,float *,size_t);
void __wrap_reloq_rate_transfer_process_voice(struct reloq_rate_transfer *state,
    float *samples,size_t count)
{
    __real_reloq_rate_transfer_process_voice(state,samples,count);
    int active=state->presence_rate>=16000 && listening.profile;
    if(!active && listening.gain==1)return;
    if(active && listening.rate!=state->presence_rate)prepare(state->presence_rate);
    for(size_t j=0;j<count;++j) {
        double y=samples[j];
        if(active) {
            const double *c=listening.c;
            double x=y;y=c[0]*x+listening.d[0];
            listening.d[0]=c[1]*x-c[3]*y+listening.d[1];
            listening.d[1]=c[2]*x-c[4]*y;
        }
        samples[j]=(float)(y*listening.gain);
    }
}
