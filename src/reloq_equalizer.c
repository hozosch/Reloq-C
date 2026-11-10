#include "reloq_equalizer.h"

#include <math.h>
#include <string.h>

/* RBJ/W3C Audio EQ Cookbook: low and high shelves, with a broad mid bell.
 * Coefficients use the actual output clock, including the 8-kHz voice. */
#define RELOQ_PI 3.14159265358979323846
static void design(struct reloq_equalizer_band *f, int type, int gain,
                   double frequency, double rate)
{
    if(!gain){f->b0=1;return;}
    double a=pow(10.0,gain/40.0), w=2*RELOQ_PI*frequency/rate;
    double c=cos(w), s=sin(w), b0,b1,b2,a0,a1,a2;
    if(type==1) {
        double alpha=s/(2*0.8);
        b0=1+alpha*a;b1=-2*c;b2=1-alpha*a;
        a0=1+alpha/a;a1=-2*c;a2=1-alpha/a;
    } else {
        double root=2*sqrt(a)*s/sqrt(2.0);
        if(type==0) {
            b0=a*((a+1)-(a-1)*c+root);
            b1=2*a*((a-1)-(a+1)*c);
            b2=a*((a+1)-(a-1)*c-root);
            a0=(a+1)+(a-1)*c+root;
            a1=-2*((a-1)+(a+1)*c);
            a2=(a+1)+(a-1)*c-root;
        } else {
            b0=a*((a+1)+(a-1)*c+root);
            b1=-2*a*((a-1)+(a+1)*c);
            b2=a*((a+1)+(a-1)*c-root);
            a0=(a+1)-(a-1)*c+root;
            a1=2*((a-1)-(a+1)*c);
            a2=(a+1)-(a-1)*c-root;
        }
    }
    f->b0=b0/a0;f->b1=b1/a0;f->b2=b2/a0;
    f->a1=a1/a0;f->a2=a2/a0;
}

static double gain_squared(const struct reloq_equalizer_band *f, double w)
{
    double c=cos(w),s=sin(w),c2=cos(2*w),s2=sin(2*w);
    double nr=f->b0+f->b1*c+f->b2*c2, ni=f->b1*s+f->b2*s2;
    double dr=1+f->a1*c+f->a2*c2, di=f->a1*s+f->a2*s2;
    return (nr*nr+ni*ni)/(dr*dr+di*di);
}

int reloq_equalizer_init(struct reloq_equalizer *e, int rate,
                         int low, int mid, int high)
{
    if(!e||rate<8000||rate>48000||low< -12||low>12||mid< -12||mid>12||high< -12||high>12)
        return -1;
    memset(e,0,sizeof *e);e->preamp=1;
    if(!low&&!mid&&!high)return 0;
    e->active=1;
    design(&e->bands[0],0,low,250,rate);
    design(&e->bands[1],1,mid,1200,rate);
    design(&e->bands[2],2,high,3000,rate);
    /* Preserve headroom for the largest combined boost. The small extra
     * reserve reduces transient clipping; the last clamp protects PCM16. */
    double peak=1;
    for(int i=0;i<=256;++i) {
        double power=1,w=RELOQ_PI*i/256.0;
        for(int j=0;j<3;++j)power*=gain_squared(&e->bands[j],w);
        if(power>peak)peak=power;
    }
    if(peak>1)e->preamp=0.95/sqrt(peak);
    return 0;
}

void reloq_equalizer_process(struct reloq_equalizer *e, int16_t *pcm, size_t n)
{
    if(!e||!e->active||!pcm)return;
    for(size_t i=0;i<n;++i) {
        double x=pcm[i]*e->preamp;
        for(unsigned j=0;j<3;++j) {
            struct reloq_equalizer_band *f=&e->bands[j];
            double y=f->b0*x+f->z1;
            f->z1=f->b1*x-f->a1*y+f->z2;
            f->z2=f->b2*x-f->a2*y;
            x=y;
        }
        if(x>32767)x=32767;else if(x< -32768)x=-32768;
        pcm[i]=(int16_t)lrint(x);
    }
}
