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
    if(profile>15 || !isfinite(gain) || gain<0 || gain>4)return -1;
    memset(&listening,0,sizeof listening);
    listening.profile=profile;listening.gain=gain;return 0;
}

unsigned reloq_listening_profile(void) { return listening.profile; }

static void prepare(float rate)
{
    const double pi=3.14159265358979323846;
    double body_db=rate>=48000?4:rate>=44100?3.5:rate>=32000?3:
        rate>=22050?2:1.5;
    double clarity_db=rate>=48000?2.6:rate>=44100?2.3:rate>=32000?1.8:
        rate>=22050?1.3:.8;
    /* A finite lower-mid band changes harmonic balance. The rejected
     * 900-Hz low shelf raised almost the entire dominant voice band and
     * was cancelled by the subsequent energy match. */
    double a=pow(10,body_db/40),w=2*pi*500/rate;
    double z=sin(w)/(2*.8),co=cos(w);
    double den=1+z/a;
    double *c=listening.body.c;
    c[0]=(1+z*a)/den;c[1]=-2*co/den;c[2]=(1-z*a)/den;
    c[3]=-2*co/den;c[4]=(1-z/a)/den;
    /* Work where voiced formants still carry useful energy, separately
     * from frication. The rejected shelf at 4.9 kHz mostly preceded the
     * serial tract's steep upper attenuation. */
    a=pow(10,clarity_db/40);w=2*pi*2800/rate;
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
