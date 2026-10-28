/* Independent complex-library oracle for the bounded two-lane coefficient
 * calculation, including phase-quadrant boundaries and fallback inputs. */
#include "../src/reloq_rate_transfer.c"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#ifdef RATE_PAIRS
static uint32_t rng=144;
static uint32_t next(void)
{rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
#endif
int main(void)
{
#ifdef RATE_PAIRS
    double largest=0;unsigned cases=0;
    for(unsigned row=0;row<1000000;++row) {
        double original[2],phase[2],r[2],im[2];
        for(unsigned i=0;i<2;++i) {
            original[i]=r[i]=((double)next()/UINT32_MAX*1400)-700;
            phase[i]=((double)next()/UINT32_MAX*128)-64;
            if(row<512)phase[i]=((int)(row/4)-64)*(pi/2)+
                ((int)(row%4)-2)*0x1p-47;
            if(row==512)phase[i]=i?-0.0:0.0;
            im[i]=-phase[i];
        }
        for(unsigned retained=0;retained<2;++retained) {
            double magnitude[2];
            for(unsigned i=0;i<2;++i) {
                r[i]=original[i];im[i]=-phase[i];magnitude[i]=exp(original[i]);
            }
            exponentiate_pair(r,im,retained?magnitude:NULL);
            for(unsigned i=0;i<2;++i) {
                double complex expected=cexp(complex_pair(original[i],phase[i]));
                double error=hypot(r[i]-creal(expected),im[i]-cimag(expected))/exp(original[i]);
                assert(error<=1e-15);if(error>largest)largest=error;++cases;
                if(phase[i]==0)assert(signbit(im[i])==signbit(phase[i]));
            }
        }
    }
    const double extremes[]={-1024,-709,-701,701,709,1024,NAN,INFINITY,-INFINITY};
    for(unsigned i=0;i<sizeof extremes/sizeof *extremes;++i) {
        double r[2]={extremes[i],0},im[2]={0,extremes[i]};
        double complex expected[2]={cexp(complex_pair(r[0],-im[0])),cexp(complex_pair(r[1],-im[1]))};
        exponentiate_pair(r,im,NULL);
        for(unsigned j=0;j<2;++j) {
            assert((isnan(r[j])&&isnan(creal(expected[j])))||r[j]==creal(expected[j]));
            assert((isnan(im[j])&&isnan(cimag(expected[j])))||im[j]==cimag(expected[j]));
        }
    }
    printf("%u coefficient inputs checked against cexp; max relative complex error %.17g\n",cases,largest);
#endif
    return 0;
}
