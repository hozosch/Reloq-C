/* Independent fits of the two-pole sampled-source correction
 * (sinc(f/Fs) / sinc(f/11025))^2, over 250..5500 Hz. These are filter
 * coefficients, not voice samples. The 16-kHz fit is within 0.001 dB;
 * all other supported clocks remain within 0.32 dB in that band. */
static void classic_source_colour(double c[5],float rate)
{
    static const float rates[]={16000,22050,32000,44100,48000};
    static const double rows[][5]={
        {1.4302950851143468,.60557534209895758,.0077974041701231213,.87847311060212496,.16519472078130254},
        {2.470597295037428,-.34807344286969832,.070722999913842913,.95215373914973789,.24109311293183489},
        {3.592017069794724,-2.872797382154483,.7753440047971647,.38527392338138444,.10928976905602183},
        {4.5709060275422191,-5.243320304362622,1.711274317039712,-.056845012337572921,.095705052556882927},
        {4.8262676123645347,-5.8804660440533478,1.9915226878466257,-.16625068913431965,.10357494529213174}
    };
    unsigned i=0;while(i<5&&rates[i]!=rate)++i;
    if(i<5)memcpy(c,rows[i],sizeof rows[i]);
    else {memset(c,0,5*sizeof *c);c[0]=1;}
}
