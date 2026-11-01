/* Independently designed IIR contours fitted to Classic Presence's physical
 * passband. The 11.025-kHz target is a 5-dB shelf at 2.5 kHz and a 0.4-dB
 * broad peak at 1.4 kHz. No interpolated PCM or mirrored band is synthesized. */
static void classic_shelf(double out[5],double rate,double center,double gain,double slope)
{
    double a=pow(10,gain/40),angle=2*pi*center/rate,co=cos(angle);
    double alpha=sin(angle)*sqrt((a+1/a)*(1/slope-1)+2)/2;
    double root=2*sqrt(a)*alpha,den=(a+1)-(a-1)*co+root;
    out[0]=a*((a+1)+(a-1)*co+root)/den;
    out[1]=-2*a*((a-1)+(a+1)*co)/den;
    out[2]=a*((a+1)+(a-1)*co-root)/den;
    out[3]=2*((a-1)-(a+1)*co)/den;
    out[4]=((a+1)-(a-1)*co-root)/den;
}
static void classic_body(double out[5],double rate)
{
    double a=pow(10,.4/40),angle=2*pi*1400/rate;
    double alpha=sin(angle)/1.6,den=1+alpha/a,co=-2*cos(angle);
    out[0]=(1+alpha*a)/den;out[1]=co/den;out[2]=(1-alpha*a)/den;
    out[3]=co/den;out[4]=(1-alpha/a)/den;
}
static void prepare_classic_colour(struct reloq_rate_transfer *s,float rate)
{
    /* Match the original physical-frequency balance through 4.6 kHz to
     * within 0.15 dB at every supported native clock. These are fitted
     * design parameters, rather than another module's digital coefficients. */
    static const double fit[5][3]={
        {2510.72745404,5.09271688,1.14483805},
        {2522.10171093,5.16012718,1.19808952},
        {2531.81637567,5.21178194,1.22604020},
        {2536.80797287,5.23717547,1.23724102},
        {2537.72860162,5.24179383,1.23913521}
    };
    unsigned slot=rate>=48000?4:rate>=44100?3:rate>=32000?2:rate>=22050?1:0;
    /* Section zero still cancels the shared old peak for periodic voicing.
     * Replace the former narrow bands; keep their existing bass gain. */
    classic_shelf(s->voice_colour[1],rate,fit[slot][0],fit[slot][1],fit[slot][2]);
    classic_body(s->voice_colour[2],rate);
    const double bass_gain=0.8912509381337456;
    for(unsigned j=0;j<3;++j)s->voice_colour[1][j]*=bass_gain;
    memset(s->voice_colour[3],0,sizeof s->voice_colour[3]);s->voice_colour[3][0]=1;
    memset(s->voice_band,0,sizeof s->voice_band);
}
static double classic_reference_power(struct reloq_rate_transfer *s,double complex z)
{
    if(s->classic_reference[0][0]==0) {
        classic_shelf(s->classic_reference[0],11025,2500,5,1);
        classic_body(s->classic_reference[1],11025);
    }
    double power=1;
    for(unsigned j=0;j<2;++j) {
        const double *c=s->classic_reference[j];
        double complex h=(c[0]+c[1]*z+c[2]*z*z)/(1+c[3]*z+c[4]*z*z);
        power*=creal(h)*creal(h)+cimag(h)*cimag(h);
    }
    return power;
}
