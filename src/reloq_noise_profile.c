#include "reloq_noise_profile.h"
#include <math.h>
#include <string.h>

static const double pi=3.14159265358979323846;
enum {BROAD,ALVEOLAR,POSTALVEOLAR,VELAR};
static long level(long x) {return x<1?1:x>52?52:x;}
static void lowpass(double c[5],double rate,double hz,double q)
{
    double k=tan(pi*fmin(hz,rate*.475)/rate),n=1/(1+k/q+k*k);
    c[0]=k*k*n;c[1]=2*c[0];c[2]=c[0];
    c[3]=2*(k*k-1)*n;c[4]=(1-k/q+k*k)*n;
}
static void highpass(double c[5],double rate,double hz,double q)
{
    double k=tan(pi*fmin(hz,rate*.475)/rate),n=1/(1+k/q+k*k);
    c[0]=n;c[1]=-2*n;c[2]=n;
    c[3]=2*(k*k-1)*n;c[4]=(1-k/q+k*k)*n;
}
static void shelf(double c[5],double rate,double hz,double gain)
{
    double a=sqrt(gain),w=2*pi*hz/rate,s=cos(w),z=sin(w)*sqrt(2*a);
    double d=(a+1)-(a-1)*s+z;
    c[0]=a*((a+1)+(a-1)*s+z)/d;
    c[1]=-2*a*((a-1)+(a+1)*s)/d;
    c[2]=a*((a+1)+(a-1)*s-z)/d;
    c[3]=2*((a-1)-(a+1)*s)/d;
    c[4]=((a+1)-(a-1)*s-z)/d;
}
void reloq_noise_profile_prepare(struct reloq_noise_profile *s,float rate,
    long av,long af,long ab,float f[8],float b[8],long a[8],long *count)
{
    if(!s || rate<16000 || !count)return;
    int kind=a[4]&&!a[2]?ALVEOLAR:a[2]||a[3]?
        (a[2]&&!a[3]?VELAR:POSTALVEOLAR):ab?BROAD:VELAR;
    /* During the reference release keep the preceding source filter and
     * upper poles. Zero input still advances its delay state. */
    if(!af && s->ready)kind=s->kind;
    double cut=6500,center=0,gain=1;
    if(rate>=22050) {
        /* These are physical frequencies of the accepted 22.05 kHz
         * profile, not fractions of Nyquist. Only the filter coefficients
         * depend on the output rate. Moving the poles with Nyquist changed
         * both the consonant colour and its voiced/unvoiced balance. */
        cut=kind==VELAR?5800:kind==ALVEOLAR?9000:7600;
        /* Requested greater presence in the broad, f-like component.
         * Relax its upper shelf; leave the common source gain
         * and the separate s/sh/velar banks unchanged. This also applies
         * to that component when present in a bilabial release. */
        if(kind==BROAD){center=4500;gain=.22;}
        else if(kind==ALVEOLAR){center=6500;gain=pow(10,-5.0/20);}
        /* OpenEVV's selected 79e upper bank. No optional K stages, P/K
         * source gain, burst shortening or voiced-cascade EQ is imported. */
        if(kind==ALVEOLAR || kind==POSTALVEOLAR) {
            *count=7;
            const float inherited_f5=f[4];
            if(kind==ALVEOLAR) {
                int voiced=av!=0 || a[3]!=0;
                double lift=!a[3] && av<30?(30-av)/30.0:0;
                f[5]=(float)(6900+round(lift*300));
                f[6]=(float)(8900+round(lift*250));
                b[5]=1500;b[6]=2300;
                a[5]=af?level(a[4]-16-(voiced?3:0))+(long)round(lift*2):0;
                a[6]=af?level(a[4]-26-(voiced?6:0))+(long)round(lift*2):0;
                f[4]+=(float)(round(lift*180)-round(lift*200));
                if(a[3]){f[5]-=400;f[6]-=600;}
            } else {
                f[5]=6100;f[6]=8100;
                b[5]=1650;b[6]=2300;
                a[5]=af?level(a[3]-24):0;a[6]=af?level(a[3]-34):0;
            }
            a[7]=0;
            if(rate>=32000) {
                /* Keep the accepted lower bank fixed and add a broad,
                 * quieter upper band in the newly available passband.
                 * The source contour still determines the main balance. */
                *count=8;
                f[7]=rate*.375f;b[7]=rate*.22f;
                a[7]=af?level((kind==ALVEOLAR?a[4]-18:a[3]-28)-
                    ((av!=0 || (kind==ALVEOLAR && a[3]!=0))?6:0)):0;
            }
            const unsigned upper=rate>=32000?3:2;
            if(af)for(unsigned j=0;j<upper;++j) {
                s->upper_frequency[j]=f[j+5];s->upper_width[j]=b[j+5];
            }
            else if(s->ready) {
                f[4]=inherited_f5;
                for(unsigned j=0;j<upper;++j) {
                    f[j+5]=s->upper_frequency[j];b[j+5]=s->upper_width[j];
                }
            }
        }
    } else {
        if(kind==BROAD){center=4000;gain=.30;}
        else if(kind==ALVEOLAR){center=4800;gain=pow(10,-4.0/20);}
    }
    if(s->ready && s->rate==rate && s->kind==kind)return;
    lowpass(s->target[0],rate,cut,.541196100146197);
    lowpass(s->target[1],rate,cut,1.306562964876377);
    memset(s->target[2],0,sizeof s->target[2]);s->target[2][0]=1;
    if(center)shelf(s->target[2],rate,center,gain);
    /* A separate high band extends the noise excitation without moving
     * the lower contour or its first seven resonance targets. It is zero
     * below 32 kHz and has no DC contribution. */
    highpass(s->target[3],rate,11025,.541196100146197);
    highpass(s->target[4],rate,11025,1.306562964876377);
    lowpass(s->target[5],rate,rate*.46,.707106781186548);
    s->upper_target=rate>=32000?(kind==ALVEOLAR?1.0:kind==POSTALVEOLAR?.25:0):0;
    if(!s->ready) {
        memcpy(s->coefficient,s->target,sizeof s->target);
        s->upper_gain=s->upper_target;
    }
    s->remaining=(unsigned)(rate*.001);if(!s->remaining)s->remaining=1;
    s->upper_step=(s->upper_target-s->upper_gain)/s->remaining;
    for(unsigned i=0;i<6;++i)for(unsigned j=0;j<5;++j)
        s->step[i][j]=(s->target[i][j]-s->coefficient[i][j])/s->remaining;
    s->ready=1;s->rate=rate;s->kind=kind;
}
void reloq_noise_profile_process(struct reloq_noise_profile *s,float *samples,size_t n)
{
    if(!s->ready)return;
    for(size_t k=0;k<n;++k) {
        if(s->remaining) {
            for(unsigned i=0;i<6;++i)for(unsigned j=0;j<5;++j)s->coefficient[i][j]+=s->step[i][j];
            s->upper_gain+=s->upper_step;
            if(!--s->remaining) {
                memcpy(s->coefficient,s->target,sizeof s->target);
                s->upper_gain=s->upper_target;
            }
        }
        double x=samples[k],upper=x;
        for(unsigned i=0;i<3;++i) {
            const double *c=s->coefficient[i];double *d=s->delay[i],y=c[0]*x+d[0];
            d[0]=c[1]*x-c[3]*y+d[1];d[1]=c[2]*x-c[4]*y;x=y;
        }
        for(unsigned i=3;s->rate>=32000 && i<6;++i) {
            const double *c=s->coefficient[i];double *d=s->delay[i],y=c[0]*upper+d[0];
            d[0]=c[1]*upper-c[3]*y+d[1];d[1]=c[2]*upper-c[4]*y;upper=y;
        }
        x+=s->upper_gain*upper;
        /* Restore the accepted frication level without the additional
         * 0.2.138 lift. Aspiration,
         * breathiness and voiced excitation do not pass through here. */
        samples[k]=(float)(x*1.6788040181225603); /* +4.5 dB; compensate restored 1-dB master for unchanged S/Z, >=16 kHz */
    }
}
