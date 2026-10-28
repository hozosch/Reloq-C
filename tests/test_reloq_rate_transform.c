/* Compare the folded transform to the unchanged full real transform and
 * an independent long-double definition of the cosine transform. */
#include "../src/reloq_rate_transfer.c"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
static uint32_t rng=145;
static double random_value(void)
{rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return (int32_t)rng/1073741824.0;}
int main(void)
{
    double worst=0,reference_worst=0;unsigned cases=0;
    for(unsigned test=0;test<256;++test) {
        double input[FFT_SIZE]={0},folded[FFT_SIZE],full[FFT_SIZE],im[FFT_SIZE]={0};
        for(unsigned j=0;j<=FFT_SIZE/2;++j) {
            double x=test==0?0:test==1?1:test==2?(j==0):test==3?(j==FFT_SIZE/2):
                test==4?(j==FFT_SIZE/4):test<16?log(1+20*exp(-pow(((double)j-test*10)/10,2))):random_value()*15;
            input[j]=x;if(j&&j<FFT_SIZE/2)input[FFT_SIZE-j]=x;
        }
        memcpy(folded,input,sizeof input);memcpy(full,input,sizeof input);
        even_transform(folded);real_transform(full,im);
        double norm=1;for(unsigned j=0;j<FFT_SIZE;++j)norm+=fabs(input[j]);
        for(unsigned k=0;k<FFT_SIZE;++k) {
            double error=fabs(folded[k]-full[k])/norm;
            if(error>worst)worst=error;
            assert(error<2e-13);
            if(test<32 && k<=FFT_SIZE/2) {
                long double expected=input[0]+(k&1?-input[FFT_SIZE/2]:input[FFT_SIZE/2]);
                for(unsigned j=1;j<FFT_SIZE/2;++j)
                    expected+=2*(long double)input[j]*cosl(2*3.141592653589793238462643383279502884L*j*k/FFT_SIZE);
                double e=(double)fabsl((long double)folded[k]-expected)/norm;
                if(e>reference_worst)reference_worst=e;
                assert(e<2e-13);
            }
            ++cases;
        }
    }
    for(unsigned test=0;test<256;++test) {
        double a[256],b[256],c[256],d[256];
        for(unsigned j=0;j<256;++j){a[j]=c[j]=random_value();b[j]=d[j]=random_value();}
        inverse_half(a,b,256,256);inverse_half(c,d,256,65);
        assert(!memcmp(a,c,65*sizeof *a));assert(!memcmp(b,d,65*sizeof *b));
    }
    printf("%u transform bins; normalized full-transform difference %.17g, long-double error %.17g; 256 complex prefixes bit-exact\n",cases,worst,reference_worst);
    return 0;
}
