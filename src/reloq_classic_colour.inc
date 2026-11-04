/* Independently derived magnitude observations of 11.025-kHz Classic
 * Presence, including the reconstruction transition and image rejection.
 * This is a native spectral target, not PCM or a resampler. Reference:
 * eloquence-contours 24b9cb1 Classic Presence. */
static double classic_colour_power(double hz)
{
    static const float db[]={
    -1.93819978f,-1.93373975f,-1.91994113f,-1.89561811f,-1.85905610f,-1.80849577f,
    -1.74293316f,-1.66313427f,-1.57232249f,-1.47552771f,-1.37697663f,-1.27670124f,
    -1.16900905f,-1.04415005f,-0.89178307f,-0.70406029f,-0.47737361f,-0.21291772f,
    0.08355549f,0.40258076f,0.73230047f,1.06009105f,1.37423542f,1.66534825f,
    1.92730905f,2.15756664f,2.35680566f,2.52807522f,2.67555304f,2.80315480f,
    2.91320592f,3.00537229f,3.07599515f,3.11789366f,3.12060225f,3.07092570f,
    2.95364833f,2.75223351f,2.44938558f,2.02739720f,1.46825031f,0.75346571f,
    -0.13629875f,-1.22191898f,-2.52680407f,-2.67036249f,-4.07836026f,-5.91019599f,
    -8.06562469f,-10.60363663f,-13.61002226f,-17.22058410f,-21.67777406f,-27.50627382f,
    -36.36441315f,-63.57807444f,-41.87815804f,-40.32713890f,-42.15827619f,-46.86180840f,
    -57.94400671f,-58.63092430f,-52.21967689f,-51.56956705f,-53.73660060f,-59.13041578f,
    -76.30382176f,-65.20962450f,-60.97989582f,-60.92369771f,-63.51893940f,-69.70675802f,
    -112.43100574f,-72.34731259f,-68.99725550f,-69.16017767f,-71.89201813f,-78.52057100f,
    -96.99685263f,-79.14412997f,-76.24063811f,-76.61782928f,-79.48289041f,-85.87145899f,
    -120.09131156f,-90.02358687f,-87.14220429f,-87.67162598f,-89.61790065f,-91.09604933f,
    -91.14727049f,
    };
    hz=fmax(0,fmin(11025,fabs(hz)));
    unsigned i;double start,span;
    if(hz<5500){i=(unsigned)(hz/125);start=125*i;span=125;}
    else if(hz<5512.5){i=44;start=5500;span=12.5;}
    else if(hz<5625){i=45;start=5512.5;span=112.5;}
    else if(hz<11000){i=(unsigned)(hz/125)+1;start=125*(i-1);span=125;}
    else {i=89;start=11000;span=25;}
    double t=fmax(0,fmin(1,(hz-start)/span));
    return pow(10,(db[i]+t*(db[i+1]-db[i]))/10);
}
static inline double classic_reference_power(struct reloq_rate_transfer *s,double complex z)
{
    (void)s;return classic_colour_power(fabs(carg(z))*11025/(2*pi));
}
