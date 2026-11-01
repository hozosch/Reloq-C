/* Only the original tract's usable band is a reference for pole matching.
 * Its reconstruction tail is a colour envelope, not another copy of the
 * low-frequency tract to invert near 11.025 kHz. That image used to make
 * the finite kernel nearly cancel at DC during ordinary vowel changes. */
static void classic_tract_shelf(double c[5],double rate,unsigned extension)
{
    const double a=extension?3.1622776601683795:10;
    const double w=2*pi*4000/rate,co=cos(w),z=sin(w)*sqrt(2*a);
    const double den=(a+1)-(a-1)*co+z;
    c[0]=a*((a+1)+(a-1)*co+z)/den;
    c[1]=-2*a*((a-1)+(a+1)*co)/den;
    c[2]=a*((a+1)+(a-1)*co-z)/den;
    c[3]=2*((a-1)-(a+1)*co)/den;
    c[4]=((a+1)-(a-1)*co-z)/den;
}
static double classic_upper_weight(double hz)
{
    return hz<=5512.5?0:hz>=6500?1:.5-.5*cos(pi*(hz-5512.5)/987.5);
}
static double classic_rate_extension(double rate)
{
    return rate>=48000?1:rate>=44100?.8:rate>=32000?.5:rate>=22050?.25:0;
}
struct classic_tract_grid {
    atomic_int ready;
    double complex before[FFT_SIZE/2+1],after[FFT_SIZE/2+1];
    double colour[FFT_SIZE/2+1],upper[FFT_SIZE/2+1];
    double factored[2][FFT_SIZE/2+1];
};
static struct classic_tract_grid classic_tract_grids[5];
static const struct classic_tract_grid *classic_tract_grid(double rate,double reference)
{
    const double rates[]={16000,22050,32000,44100,48000};
    unsigned i=0;while(i<5&&rates[i]!=rate)++i;
    if(i==5||reference!=11025)return NULL;
    struct classic_tract_grid *g=classic_tract_grids+i;
    if(atomic_load_explicit(&g->ready,memory_order_acquire)!=2) {
        int expected=0;
        if(atomic_compare_exchange_strong(&g->ready,&expected,1)) {
            double c[2][5];
            classic_tract_shelf(c[0],rate,0);classic_tract_shelf(c[1],rate,1);
            for(unsigned k=0;k<=FFT_SIZE/2;++k) {
                double hz=(double)k*rate/FFT_SIZE,at=fmin(hz,reference*.5);
                g->before[k]=cexp(-I*2*pi*at/reference);
                g->after[k]=cexp(-I*2*pi*at/rate);
                g->colour[k]=hz<=11025?classic_colour_power(hz):0;
                g->upper[k]=classic_upper_weight(hz);
                double complex z=cexp(-I*2*pi*hz/rate);
                for(unsigned n=0;n<2;++n)
                    g->factored[n][k]=cabs((c[n][0]+c[n][1]*z+c[n][2]*z*z)/
                        (1+c[n][3]*z+c[n][4]*z*z));
            }
            atomic_store_explicit(&g->ready,2,memory_order_release);
        } else while(atomic_load_explicit(&g->ready,memory_order_acquire)!=2) {}
    }
    return g;
}
static RATE_NOINLINE void fit_classic_tract(struct reloq_rate_transfer *s,
    const struct pole before[12],const struct pole after[12],unsigned old_count,
    long count,double rate,double reference)
{
    const struct classic_tract_grid *g=classic_tract_grid(rate,reference);
    double spectrum[FFT_SIZE]={0},extension=classic_rate_extension(rate);
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE,base=0;
        if(hz<=11025) {
            double at=fmin(hz,reference*.5);
            double complex z0=g?g->before[k]:cexp(-I*2*pi*at/reference);
            double complex z1=g?g->after[k]:cexp(-I*2*pi*at/rate);
            double a=1,b=1;
            for(unsigned j=0;j<old_count;++j)a*=power(before[j],z0);
            for(long j=0;j<count;++j)b*=power(after[j],z1);
            for(unsigned j=8;j<12;j+=2) {
                a*=power(before[j],z0)/fmax(1e-30,power(before[j+1],z0));
                b*=power(after[j],z1)/fmax(1e-30,power(after[j+1],z1));
            }
            base=a/fmax(1e-30,b)*(g?g->colour[k]:classic_colour_power(hz));
        }
        double upper=g?g->upper[k]:classic_upper_weight(hz),factored;
        if(g)factored=g->factored[!!s->colour_extension][k];
        else {
            double complex z=cexp(-I*2*pi*hz/rate);const double *c=s->presence;
            factored=cabs((c[0]+c[1]*z+c[2]*z*z)/(1+c[3]*z+c[4]*z*z));
        }
        spectrum[k]=fmax(.000001,sqrt(base+extension*upper)/factored);
    }
    causal_kernel(s,spectrum,1);
    for(unsigned i=0;i<RELOQ_RATE_TAPS;++i)s->target[i]*=.8f;
    prepare_ramp(s,(float)rate);
}
