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

static void design_peak(struct reloq_equalizer_band *f,int gain,int frequency,
                        int width_percent,double rate)
{
    /* The cookbook bandwidth formula compensates for frequency warping.
     * Cap the sinh argument near Nyquist to keep coefficients finite. */
    double a=pow(10.0,gain/40.0),w=2*RELOQ_PI*frequency/rate;
    double c=cos(w),s=sin(w),argument=log(2.0)*0.5*(width_percent/100.0)*w/s;
    if(argument>20)argument=20;
    double alpha=s*sinh(argument);
    double a0=1+alpha/a;
    f->b0=(1+alpha*a)/a0;f->b1=-2*c/a0;f->b2=(1-alpha*a)/a0;
    f->a1=-2*c/a0;f->a2=(1-alpha/a)/a0;
}

int reloq_equalizer_init_extended(struct reloq_equalizer *e,int rate,
    int low,int mid,int high,const struct reloq_equalizer_peak *peaks)
{
    if(!e||rate<8000||rate>48000||low< -12||low>12||mid< -12||mid>12||high< -12||high>12)
        return -1;
    memset(e,0,sizeof *e);e->limiter_gain=1;
    if(peaks)for(int i=0;i<RELOQ_EQ_FILTER_COUNT;++i){
        const struct reloq_equalizer_peak *p=peaks+i;
        if((p->enabled!=0&&p->enabled!=1)||p->frequency<1||p->frequency>24000||
           p->gain< -12||p->gain>12||p->width_percent<10||p->width_percent>400)return -1;
    }
    e->active=low||mid||high;
    if(peaks)for(int i=0;i<RELOQ_EQ_FILTER_COUNT;++i)
        if(peaks[i].enabled&&peaks[i].gain&&peaks[i].frequency<rate/2.0)e->active=1;
    if(!e->active)return 0;
    e->band_count=3;
    design(&e->bands[0],0,low,250,rate);
    design(&e->bands[1],1,mid,1200,rate);
    design(&e->bands[2],2,high,3000,rate);
    if(peaks)for(int i=0;i<RELOQ_EQ_FILTER_COUNT;++i)
        if(peaks[i].enabled&&peaks[i].gain&&peaks[i].frequency<rate/2.0)
            design_peak(&e->bands[e->band_count++],peaks[i].gain,peaks[i].frequency,peaks[i].width_percent,rate);
    /* Do not attenuate every band merely because one band is boosted.
     * Recover gradually after a peak actually needs limiting. */
    e->limiter_release=1-exp(-1.0/(0.075*rate));
    return 0;
}

int reloq_equalizer_init(struct reloq_equalizer *e,int rate,int low,int mid,int high)
{return reloq_equalizer_init_extended(e,rate,low,mid,high,NULL);}

void reloq_equalizer_process(struct reloq_equalizer *e, int16_t *pcm, size_t n)
{
    if(!e||!e->active||!pcm)return;
    for(size_t i=0;i<n;++i) {
        double x=pcm[i];
        for(int j=0;j<e->band_count;++j) {
            struct reloq_equalizer_band *f=&e->bands[j];
            double y=f->b0*x+f->z1;
            f->z1=f->b1*x-f->a1*y+f->z2;
            f->z2=f->b2*x-f->a2*y;
            x=y;
        }
        /* Instantaneous attack keeps every output sample within PCM16;
         * the 75-ms release avoids gain following individual oscillations.
         * Ordinary non-peaking speech is not attenuated at all. */
        double absolute=fabs(x), allowed=absolute>32760?32760/absolute:1;
        double recovered=e->limiter_gain+(1-e->limiter_gain)*e->limiter_release;
        e->limiter_gain=recovered<allowed?recovered:allowed;
        x*=e->limiter_gain;
        if(x>32767)x=32767;else if(x< -32768)x=-32768;
        pcm[i]=(int16_t)lrint(x);
    }
}
