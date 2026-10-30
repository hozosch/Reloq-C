#include "reloq_core.h"
#include "reloq_rate_transfer.h"
#include <complex.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdatomic.h>

#if defined(__SSE2__) && !defined(RELOQ_RATE_TRANSFER_SCALAR)
#include <emmintrin.h>
#define RATE_PAIRS 1
#endif
#if defined(RATE_PAIRS) && defined(__GNUC__) && !defined(RELOQ_RATE_TRANSFER_NO_AVX2)
#include <immintrin.h>
#define RATE_AVX2 1
#ifdef _WIN32
#include <windows.h>
#endif
#endif

#define FFT_SIZE 512
#if defined(__GNUC__) || defined(__clang__)
#define RATE_NOINLINE __attribute__((noinline))
#else
#define RATE_NOINLINE
#endif
static const double pi=3.14159265358979323846;

/* All transform inputs here are finite. Express the four products directly
 * to avoid the general complex multiply's NaN/Inf recovery path. Preserve
 * its real/imaginary operation order; no fast-math or relaxed sums. */
static double complex multiply(double complex a,double complex b)
{
    double r=creal(a)*creal(b)-cimag(a)*cimag(b);
    double i=creal(a)*cimag(b)+cimag(a)*creal(b);
    return r+I*i;
}

/* C complex numbers have the representation of their two real components.
 * Construct them without extra floating-point additions or sign changes. */
static double complex complex_pair(double real,double imag)
{
    double parts[2]={real,imag};double complex value;
    memcpy(&value,parts,sizeof value);return value;
}

/* Clock-independent Fourier rotations are shared by all render threads.
 * Compute them with exactly the original recurrence, once:
 * this changes neither arithmetic order in a butterfly nor its rounding.
 * Publication is atomic; completed tables are immutable for all engines. */
struct transform_constants {
    atomic_int ready;
    unsigned swaps;
    unsigned short swap_from[FFT_SIZE/4],swap_to[FFT_SIZE/4];
    unsigned small_swaps;
    unsigned short small_from[FFT_SIZE/8],small_to[FFT_SIZE/8];
    double rotation_real[FFT_SIZE-1],rotation_imag[FFT_SIZE-1];
    double fold_sine[FFT_SIZE/4],fold_cosine[FFT_SIZE/4];
    double window[RELOQ_RATE_TAPS],noise_window[RELOQ_RATE_TAPS];
};
static struct transform_constants transform;
static void prepare_transform(void)
{
    if(atomic_load_explicit(&transform.ready,memory_order_acquire)==2)return;
    int expected=0;
    if(!atomic_compare_exchange_strong(&transform.ready,&expected,1)) {
        while(atomic_load_explicit(&transform.ready,memory_order_acquire)!=2) {}
        return;
    }
    unsigned at=0;
    /* The same permutation precedes every transform. Store only its
     * disjoint swaps, in the scalar loop's original order. */
    for(unsigned i=1,j=0;i<FFT_SIZE/2;++i) {
        unsigned bit=FFT_SIZE/4;
        while(j&bit){j^=bit;bit>>=1;}j^=bit;
        if(i<j) {
            transform.swap_from[transform.swaps]=(unsigned short)i;
            transform.swap_to[transform.swaps++]=(unsigned short)j;
        }
    }
    for(unsigned i=1,j=0;i<FFT_SIZE/4;++i) {
        unsigned bit=FFT_SIZE/8;
        while(j&bit){j^=bit;bit>>=1;}j^=bit;
        if(i<j) {
            transform.small_from[transform.small_swaps]=(unsigned short)i;
            transform.small_to[transform.small_swaps++]=(unsigned short)j;
        }
    }
    for(unsigned n=2;n<=FFT_SIZE;n*=2) {
        double angle=2*pi/n;
        double complex step=cos(angle)+I*sin(angle),w=1;
        for(unsigned j=0;j<n/2;++j,w=multiply(w,step)) {
            transform.rotation_real[at]=creal(w);transform.rotation_imag[at++]=cimag(w);
        }
    }
    const unsigned fade=RELOQ_RATE_TAPS*3/4;
    for(unsigned i=0;i<RELOQ_RATE_TAPS;++i)
        transform.window[i]=i<=fade?1:.5*(1+cos(pi*(i-fade)/(RELOQ_RATE_TAPS-1-fade)));
    for(unsigned i=0;i<RELOQ_RATE_TAPS;++i)
        transform.noise_window[i]=i<48?1:i<65?.5*(1+cos(pi*(i-48)/16)):0;
    for(unsigned i=0;i<FFT_SIZE/4;++i) {
        transform.fold_sine[i]=sin(2*pi*i/FFT_SIZE);
        transform.fold_cosine[i]=cos(2*pi*i/FFT_SIZE);
    }
    atomic_store_explicit(&transform.ready,2,memory_order_release);
}
/* Inverse radix-two transform; only a smooth magnitude residual is fitted,
 * not the sharply resonant tract itself. This keeps the FIR short. */
static void inverse_half(double *real,double *imag,unsigned count,unsigned prefix)
{
    prepare_transform();
    const unsigned short *from=count==FFT_SIZE/2?transform.swap_from:transform.small_from;
    const unsigned short *to=count==FFT_SIZE/2?transform.swap_to:transform.small_to;
    unsigned swaps=count==FFT_SIZE/2?transform.swaps:transform.small_swaps;
    for(unsigned k=0;k<swaps;++k) {
        unsigned i=from[k],j=to[k];
        double r=real[i],im=imag[i];
        real[i]=real[j];imag[i]=imag[j];real[j]=r;imag[j]=im;
    }
    unsigned at=0;
#ifdef RATE_PAIRS
    /* The first two stages are independent four-value tiles. Keep their
     * intermediate values in double SIMD registers instead of storing and
     * loading both complete arrays between the stages. The zero/one
     * rotations still execute the same products, sums and subtractions:
     * even signed-zero behaviour is retained. */
    __m128d wr2=_mm_set1_pd(transform.rotation_real[0]);
    __m128d wi2=_mm_set1_pd(transform.rotation_imag[0]);
    __m128d wr4=_mm_loadu_pd(transform.rotation_real+1);
    __m128d wi4=_mm_loadu_pd(transform.rotation_imag+1);
    for(unsigned i=0;i<count;i+=4) {
        __m128d r0=_mm_loadu_pd(real+i),r2=_mm_loadu_pd(real+i+2);
        __m128d i0=_mm_loadu_pd(imag+i),i2=_mm_loadu_pd(imag+i+2);
        __m128d ur=_mm_unpacklo_pd(r0,r2),ui=_mm_unpacklo_pd(i0,i2);
        __m128d xr=_mm_unpackhi_pd(r0,r2),xi=_mm_unpackhi_pd(i0,i2);
        __m128d vr=_mm_sub_pd(_mm_mul_pd(wr2,xr),_mm_mul_pd(wi2,xi));
        __m128d vi=_mm_add_pd(_mm_mul_pd(wr2,xi),_mm_mul_pd(wi2,xr));
        __m128d sr=_mm_add_pd(ur,vr),dr=_mm_sub_pd(ur,vr);
        __m128d si=_mm_add_pd(ui,vi),di=_mm_sub_pd(ui,vi);
        r0=_mm_unpacklo_pd(sr,dr);r2=_mm_unpackhi_pd(sr,dr);
        i0=_mm_unpacklo_pd(si,di);i2=_mm_unpackhi_pd(si,di);
        vr=_mm_sub_pd(_mm_mul_pd(wr4,r2),_mm_mul_pd(wi4,i2));
        vi=_mm_add_pd(_mm_mul_pd(wr4,i2),_mm_mul_pd(wi4,r2));
        _mm_storeu_pd(real+i,_mm_add_pd(r0,vr));
        _mm_storeu_pd(imag+i,_mm_add_pd(i0,vi));
        _mm_storeu_pd(real+i+2,_mm_sub_pd(r0,vr));
        _mm_storeu_pd(imag+i+2,_mm_sub_pd(i0,vi));
    }
    at=3;
    /* Fuse each following radix-two stage pair in four-way tiles.
     * Every butterfly retains its original operations and rotations;
     * independent tiles merely finish both stages while values are live.
     * This removes four full-array store/reload passes per transform. */
    unsigned n=8;
    for(;2*n<=count;n*=4) {
        /* The causal FIR consumes only 129 real samples, or the first
         * 65 complex outputs here. At the last pair of stages compute
         * those same butterflies and omit outputs that are never read.
         * Each retained output keeps its complete original arithmetic. */
        if(n==128 && prefix==65) {
            for(unsigned j=0;j<n/2;j+=2) {
                __m128d wr=_mm_loadu_pd(transform.rotation_real+at+j);
                __m128d wi=_mm_loadu_pd(transform.rotation_imag+at+j);
                __m128d ar=_mm_loadu_pd(real+j),ai=_mm_loadu_pd(imag+j);
                __m128d br=_mm_loadu_pd(real+j+64),bi=_mm_loadu_pd(imag+j+64);
                __m128d cr=_mm_loadu_pd(real+j+128),ci=_mm_loadu_pd(imag+j+128);
                __m128d dr=_mm_loadu_pd(real+j+192),di=_mm_loadu_pd(imag+j+192);
                __m128d vr=_mm_sub_pd(_mm_mul_pd(wr,br),_mm_mul_pd(wi,bi));
                __m128d vi=_mm_add_pd(_mm_mul_pd(wr,bi),_mm_mul_pd(wi,br));
                __m128d pr=_mm_add_pd(ar,vr),pi=_mm_add_pd(ai,vi);
                __m128d mr=_mm_sub_pd(ar,vr),mi=_mm_sub_pd(ai,vi);
                vr=_mm_sub_pd(_mm_mul_pd(wr,dr),_mm_mul_pd(wi,di));
                vi=_mm_add_pd(_mm_mul_pd(wr,di),_mm_mul_pd(wi,dr));
                __m128d qr=_mm_add_pd(cr,vr),qi=_mm_add_pd(ci,vi);
                __m128d nr=_mm_sub_pd(cr,vr),ni=_mm_sub_pd(ci,vi);
                __m128d wr0=_mm_loadu_pd(transform.rotation_real+at+64+j);
                __m128d wi0=_mm_loadu_pd(transform.rotation_imag+at+64+j);
                vr=_mm_sub_pd(_mm_mul_pd(wr0,qr),_mm_mul_pd(wi0,qi));
                vi=_mm_add_pd(_mm_mul_pd(wr0,qi),_mm_mul_pd(wi0,qr));
                _mm_storeu_pd(real+j,_mm_add_pd(pr,vr));
                _mm_storeu_pd(imag+j,_mm_add_pd(pi,vi));
                if(j==0) {
                    __m128d wr1=_mm_loadu_pd(transform.rotation_real+at+128);
                    __m128d wi1=_mm_loadu_pd(transform.rotation_imag+at+128);
                    vr=_mm_sub_pd(_mm_mul_pd(wr1,nr),_mm_mul_pd(wi1,ni));
                    vi=_mm_add_pd(_mm_mul_pd(wr1,ni),_mm_mul_pd(wi1,nr));
                    _mm_storeu_pd(real+64,_mm_add_pd(mr,vr));
                    _mm_storeu_pd(imag+64,_mm_add_pd(mi,vi));
                }
            }
            return;
        }
        for(unsigned j=0;j<n/2;j+=2) {
            __m128d wr=_mm_loadu_pd(transform.rotation_real+at+j);
            __m128d wi=_mm_loadu_pd(transform.rotation_imag+at+j);
            for(unsigned i=0;i<count;i+=2*n) {
                unsigned a=i+j,b=a+n/2,c=a+n,d=c+n/2;
                __m128d ar=_mm_loadu_pd(real+a),ai=_mm_loadu_pd(imag+a);
                __m128d br=_mm_loadu_pd(real+b),bi=_mm_loadu_pd(imag+b);
                __m128d cr=_mm_loadu_pd(real+c),ci=_mm_loadu_pd(imag+c);
                __m128d dr=_mm_loadu_pd(real+d),di=_mm_loadu_pd(imag+d);
                __m128d vr=_mm_sub_pd(_mm_mul_pd(wr,br),_mm_mul_pd(wi,bi));
                __m128d vi=_mm_add_pd(_mm_mul_pd(wr,bi),_mm_mul_pd(wi,br));
                __m128d pr=_mm_add_pd(ar,vr),pi=_mm_add_pd(ai,vi);
                __m128d mr=_mm_sub_pd(ar,vr),mi=_mm_sub_pd(ai,vi);
                vr=_mm_sub_pd(_mm_mul_pd(wr,dr),_mm_mul_pd(wi,di));
                vi=_mm_add_pd(_mm_mul_pd(wr,di),_mm_mul_pd(wi,dr));
                __m128d qr=_mm_add_pd(cr,vr),qi=_mm_add_pd(ci,vi);
                __m128d nr=_mm_sub_pd(cr,vr),ni=_mm_sub_pd(ci,vi);
                __m128d wr0=_mm_loadu_pd(transform.rotation_real+at+n/2+j);
                __m128d wi0=_mm_loadu_pd(transform.rotation_imag+at+n/2+j);
                vr=_mm_sub_pd(_mm_mul_pd(wr0,qr),_mm_mul_pd(wi0,qi));
                vi=_mm_add_pd(_mm_mul_pd(wr0,qi),_mm_mul_pd(wi0,qr));
                _mm_storeu_pd(real+a,_mm_add_pd(pr,vr));
                _mm_storeu_pd(imag+a,_mm_add_pd(pi,vi));
                _mm_storeu_pd(real+c,_mm_sub_pd(pr,vr));
                _mm_storeu_pd(imag+c,_mm_sub_pd(pi,vi));
                __m128d wr1=_mm_loadu_pd(transform.rotation_real+at+n+j);
                __m128d wi1=_mm_loadu_pd(transform.rotation_imag+at+n+j);
                vr=_mm_sub_pd(_mm_mul_pd(wr1,nr),_mm_mul_pd(wi1,ni));
                vi=_mm_add_pd(_mm_mul_pd(wr1,ni),_mm_mul_pd(wi1,nr));
                _mm_storeu_pd(real+b,_mm_add_pd(mr,vr));
                _mm_storeu_pd(imag+b,_mm_add_pd(mi,vi));
                _mm_storeu_pd(real+d,_mm_sub_pd(mr,vr));
                _mm_storeu_pd(imag+d,_mm_sub_pd(mi,vi));
            }
        }
        at+=3*n/2;
    }
    /* A 128-point transform has one final, unpaired radix-two stage. */
    if(n<=count) {
        for(unsigned j=0;j<n/2;j+=2) {
            __m128d wr=_mm_loadu_pd(transform.rotation_real+at+j);
            __m128d wi=_mm_loadu_pd(transform.rotation_imag+at+j);
            __m128d ar=_mm_loadu_pd(real+j),ai=_mm_loadu_pd(imag+j);
            __m128d br=_mm_loadu_pd(real+j+n/2),bi=_mm_loadu_pd(imag+j+n/2);
            __m128d vr=_mm_sub_pd(_mm_mul_pd(wr,br),_mm_mul_pd(wi,bi));
            __m128d vi=_mm_add_pd(_mm_mul_pd(wr,bi),_mm_mul_pd(wi,br));
            _mm_storeu_pd(real+j,_mm_add_pd(ar,vr));
            _mm_storeu_pd(imag+j,_mm_add_pd(ai,vi));
            _mm_storeu_pd(real+j+n/2,_mm_sub_pd(ar,vr));
            _mm_storeu_pd(imag+j+n/2,_mm_sub_pd(ai,vi));
        }
    }
#else
    (void)prefix;
    for(unsigned n=2;n<=count;n*=2) {
        for(unsigned i=0;i<count;i+=n)for(unsigned j=0;j<n/2;++j) {
            double wr=transform.rotation_real[at+j],wi=transform.rotation_imag[at+j];
            double xr=real[i+j+n/2],xi=imag[i+j+n/2];
            double vr=wr*xr-wi*xi,vi=wr*xi+wi*xr;
            double ur=real[i+j],ui=imag[i+j];
            real[i+j]=ur+vr;imag[i+j]=ui+vi;
            real[i+j+n/2]=ur-vr;imag[i+j+n/2]=ui-vi;
        }
        at+=n/2;
    }
#endif
}

/* The frequency grid depends on the two clocks, not on moving formants.
 * Keep it outside the copied per-frame filter state. Exact clock keys and
 * the same cexp expression avoid any quantization of tract parameters. */
struct frequency_grid {
    atomic_int ready;
    double hz[FFT_SIZE/2+1];
    double before_real[FFT_SIZE/2+1],before_imag[FFT_SIZE/2+1];
    double after_real[FFT_SIZE/2+1],after_imag[FFT_SIZE/2+1];
};
static struct frequency_grid grids[5][2];
static const struct frequency_grid *prepare_grid(double rate,double reference)
{
    static const double clocks[5]={16000,22050,32000,44100,48000};
    unsigned clock=0;
    while(clock<5 && clocks[clock]!=rate)++clock;
    /* Arbitrary development clocks retain the direct expression. The
     * supported output/reference clock pairs use bounded immutable caches. */
    if(clock==5 || (reference!=11025 && reference!=16000))return NULL;
    struct frequency_grid *g=&grids[clock][reference==16000];
    if(atomic_load_explicit(&g->ready,memory_order_acquire)==2)return g;
    int expected=0;
    if(!atomic_compare_exchange_strong(&g->ready,&expected,1)) {
        while(atomic_load_explicit(&g->ready,memory_order_acquire)!=2) {}
        return g;
    }
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        g->hz[k]=(double)k*rate/FFT_SIZE;
        double at=fmin(g->hz[k],reference*.5);
        double complex before=cexp(-I*2*pi*at/reference),after=cexp(-I*2*pi*at/rate);
        g->before_real[k]=creal(before);g->before_imag[k]=cimag(before);
        g->after_real[k]=creal(after);g->after_imag[k]=cimag(after);
    }
    atomic_store_explicit(&g->ready,2,memory_order_release);
    return g;
}

/* A real 512-point transform needs a 256-point complex transform:
 * even and odd input samples occupy its two independent components.
 * The 512-point rotations then unpack the original frequency grid. */
static void real_transform(double real[FFT_SIZE],double imag[FFT_SIZE])
{
    double r[FFT_SIZE/2],im[FFT_SIZE/2];
    for(unsigned k=0;k<FFT_SIZE/2;++k){r[k]=real[2*k];im[k]=real[2*k+1];}
    inverse_half(r,im,FFT_SIZE/2,FFT_SIZE/2);
    for(unsigned k=0;k<FFT_SIZE/2;++k) {
        unsigned q=k?FFT_SIZE/2-k:0;
        double ar=(r[k]+r[q])*.5,ai=(im[k]-im[q])*.5;
        double dr=r[k]-r[q],di=im[k]+im[q];
        double wr=transform.rotation_real[FFT_SIZE/2-1+k];
        double wi=transform.rotation_imag[FFT_SIZE/2-1+k];
        double vr=(wi*dr+wr*di)*.5,vi=(wi*di-wr*dr)*.5;
        real[k]=ar+vr;imag[k]=ai+vi;
        real[k+FFT_SIZE/2]=ar-vr;imag[k+FFT_SIZE/2]=ai-vi;
    }
}
/* The log-magnitude input is exactly even. Its real 512-point transform
 * is a cosine transform of the 257 independent values. Fold these into
 * 256 real values, transform with 128 complex points, then recover odd
 * output bins from consecutive imaginary components. No frequency bins
 * or causal taps are dropped. The packed intermediate stores Re(U[k])
 * at k and Im(U[k]) at 256-k; DC and Nyquist are purely real. */
static void even_transform(double values[FFT_SIZE])
{
    enum { M=FFT_SIZE/2,H=M/2 };
    prepare_transform();
    double packed[M],r[H],im[H];
    double odd=values[0]-values[M];
    packed[0]=(values[0]+values[M])*.5;
    packed[H]=values[H];
    for(unsigned j=1;j<H;++j) {
        double difference=values[j]-values[M-j];
        double middle=(values[j]+values[M-j])*.5;
        double side=difference*transform.fold_sine[j];
        packed[j]=middle-side;packed[M-j]=middle+side;
        odd+=2*difference*transform.fold_cosine[j];
    }
    for(unsigned k=0;k<H;++k){r[k]=packed[2*k];im[k]=packed[2*k+1];}
    inverse_half(r,im,H,H);
    for(unsigned k=0;k<H;++k) {
        unsigned q=k?H-k:0;
        double ar=(r[k]+r[q])*.5,ai=(im[k]-im[q])*.5;
        double dr=r[k]-r[q],di=im[k]+im[q];
        double wr=transform.rotation_real[H-1+k],wi=transform.rotation_imag[H-1+k];
        double vr=(wi*dr+wr*di)*.5,vi=(wi*di-wr*dr)*.5;
        packed[k]=ar+vr;
        if(k)packed[M-k]=ai+vi;
        else packed[H]=ar-vr;
    }
    for(unsigned k=0;k<=H;++k)values[2*k]=2*packed[k];
    values[1]=odd;
    for(unsigned k=1;k<H;++k){odd+=2*packed[M-k];values[2*k+1]=odd;}
    for(unsigned k=1;k<M;++k)values[FFT_SIZE-k]=values[k];
}
/* A Hermitian spectrum has a real inverse. Fold its two halves into one
 * 256-point complex transform, whose real and imaginary components are
 * respectively the even and odd output samples. Only the causal filter's
 * first 129 samples are needed; subsequent array elements are left alone. */
static void hermitian_transform(double real[FFT_SIZE],double imag[FFT_SIZE])
{
    double r[FFT_SIZE/2],im[FFT_SIZE/2];
    for(unsigned k=0;k<FFT_SIZE/2;++k) {
        unsigned q=FFT_SIZE/2-k;
        double sr=real[k]+real[q],si=imag[k]-imag[q];
        double dr=real[k]-real[q],di=imag[k]+imag[q];
        double wr=transform.rotation_real[FFT_SIZE/2-1+k];
        double wi=transform.rotation_imag[FFT_SIZE/2-1+k];
        r[k]=sr-(wr*di+wi*dr);im[k]=si+(wr*dr-wi*di);
    }
    inverse_half(r,im,FFT_SIZE/2,RELOQ_RATE_TAPS/2+1);
    for(unsigned k=0;k<=RELOQ_RATE_TAPS/2;++k){real[2*k]=r[k];real[2*k+1]=im[k];}
}

/* The reference tract responds causally to a source onset. A symmetric
 * magnitude-fit kernel adds a separate half-kernel delay and a precursor
 * around short releases. Spectral factorization gives the causal minimum-
 * phase response for the same positive magnitude residual. No phone gain
 * or source timing is changed. This is part of our wideband extension;
 * the public language interface supplies only 8/11.025 kHz. Its numeric
 * core additionally accepts 16 kHz, used as the independent AF reference. */
#ifdef RATE_PAIRS
/* Spectral factorization supplies finite log magnitudes and usually small
 * phases. Reduce two phases to [-pi/4,pi/4], then evaluate sine through
 * degree 17 and cosine through degree 16 in independent double lanes.
 * Splitting pi/2 preserves the reduction's low bits for |phase| <= 64.
 * The Taylor remainders on this interval are below 4e-18. Larger values,
 * exceptional inputs and exponent extremes keep the general library path.
 * This is coefficient arithmetic, with no stored speech or phase table. */
static void exponentiate_pair(double real[2],double imag[2],const double *magnitude)
{
    double x0=-imag[0],x1=-imag[1];
    if(!(fabs(x0)<=64 && fabs(x1)<=64 &&
         (magnitude || (real[0]>=-700 && real[0]<=700 && real[1]>=-700 && real[1]<=700)))) {
        for(unsigned i=0;i<2;++i) {
            double complex z=magnitude?
                complex_pair(magnitude[i]*cos(-imag[i]),magnitude[i]*sin(-imag[i])):
                cexp(complex_pair(real[i],-imag[i]));
            real[i]=creal(z);imag[i]=cimag(z);
        }
        return;
    }
    __m128d x=_mm_set_pd(x1,x0);
    __m128d quadrant=_mm_mul_pd(x,_mm_set1_pd(0.63661977236758134308));
    __m128d half=_mm_or_pd(_mm_set1_pd(.5),_mm_and_pd(x,_mm_set1_pd(-0.0)));
    __m128i q=_mm_cvttpd_epi32(_mm_add_pd(quadrant,half));
    __m128d n=_mm_cvtepi32_pd(q);
    __m128d r=_mm_sub_pd(x,_mm_mul_pd(n,_mm_set1_pd(0x1.921fb54400000p0)));
    r=_mm_sub_pd(r,_mm_mul_pd(n,_mm_set1_pd(0x1.0b4611a626331p-34)));
    __m128d z=_mm_mul_pd(r,r);
    __m128d sp=_mm_set1_pd(1.0/355687428096000.0);
    sp=_mm_add_pd(_mm_set1_pd(-1.0/1307674368000.0),_mm_mul_pd(z,sp));
    sp=_mm_add_pd(_mm_set1_pd(1.0/6227020800.0),_mm_mul_pd(z,sp));
    sp=_mm_add_pd(_mm_set1_pd(-1.0/39916800.0),_mm_mul_pd(z,sp));
    sp=_mm_add_pd(_mm_set1_pd(1.0/362880.0),_mm_mul_pd(z,sp));
    sp=_mm_add_pd(_mm_set1_pd(-1.0/5040.0),_mm_mul_pd(z,sp));
    sp=_mm_add_pd(_mm_set1_pd(1.0/120.0),_mm_mul_pd(z,sp));
    sp=_mm_add_pd(_mm_set1_pd(-1.0/6.0),_mm_mul_pd(z,sp));
    sp=_mm_add_pd(r,_mm_mul_pd(_mm_mul_pd(r,z),sp));
    __m128d cp=_mm_set1_pd(1.0/20922789888000.0);
    cp=_mm_add_pd(_mm_set1_pd(-1.0/87178291200.0),_mm_mul_pd(z,cp));
    cp=_mm_add_pd(_mm_set1_pd(1.0/479001600.0),_mm_mul_pd(z,cp));
    cp=_mm_add_pd(_mm_set1_pd(-1.0/3628800.0),_mm_mul_pd(z,cp));
    cp=_mm_add_pd(_mm_set1_pd(1.0/40320.0),_mm_mul_pd(z,cp));
    cp=_mm_add_pd(_mm_set1_pd(-1.0/720.0),_mm_mul_pd(z,cp));
    cp=_mm_add_pd(_mm_set1_pd(1.0/24.0),_mm_mul_pd(z,cp));
    cp=_mm_add_pd(_mm_set1_pd(-.5),_mm_mul_pd(z,cp));
    cp=_mm_add_pd(_mm_set1_pd(1),_mm_mul_pd(z,cp));
    /* Duplicate each 32-bit quadrant across its double lane. Selecting
     * and signing whole lanes avoids per-bin stores and scalar branches. */
    q=_mm_shuffle_epi32(q,_MM_SHUFFLE(1,1,0,0));
    const __m128i one=_mm_set1_epi32(1),two=_mm_set1_epi32(2);
    __m128d odd=_mm_castsi128_pd(_mm_cmpeq_epi32(_mm_and_si128(q,one),one));
    __m128d sine=_mm_or_pd(_mm_and_pd(odd,cp),_mm_andnot_pd(odd,sp));
    __m128d cosine=_mm_or_pd(_mm_and_pd(odd,sp),_mm_andnot_pd(odd,cp));
    sine=_mm_xor_pd(sine,_mm_castsi128_pd(_mm_slli_epi64(_mm_and_si128(q,two),62)));
    cosine=_mm_xor_pd(cosine,_mm_castsi128_pd(_mm_slli_epi64(
        _mm_and_si128(_mm_add_epi32(q,one),two),62)));
    __m128d iszero=_mm_cmpeq_pd(x,_mm_setzero_pd());
    sine=_mm_or_pd(_mm_and_pd(iszero,x),_mm_andnot_pd(iszero,sine));
    __m128d amplitude=magnitude?_mm_loadu_pd(magnitude):_mm_set_pd(exp(real[1]),exp(real[0]));
    _mm_storeu_pd(real,_mm_mul_pd(amplitude,cosine));
    _mm_storeu_pd(imag,_mm_mul_pd(amplitude,sine));
}
#ifdef RATE_AVX2
static int avx2_enabled(void);
/* The same phase reduction and Horner operations in four independent lanes.
 * Exceptional lanes retain the original pair-wise fallback decisions. */
__attribute__((target("avx2")))
static void exponentiate_four(double real[4],double imag[4],const double magnitude[4])
{
    double x0=-imag[0],x1=-imag[1],x2=-imag[2],x3=-imag[3];
    if(!(fabs(x0)<=64&&fabs(x1)<=64&&fabs(x2)<=64&&fabs(x3)<=64)) {
        exponentiate_pair(real,imag,magnitude);
        exponentiate_pair(real+2,imag+2,magnitude+2);return;
    }
    __m256d x=_mm256_set_pd(x3,x2,x1,x0);
    __m256d quadrant=_mm256_mul_pd(x,_mm256_set1_pd(0.63661977236758134308));
    __m256d half=_mm256_or_pd(_mm256_set1_pd(.5),_mm256_and_pd(x,_mm256_set1_pd(-0.0)));
    __m128i q32=_mm256_cvttpd_epi32(_mm256_add_pd(quadrant,half));
    __m256d n=_mm256_cvtepi32_pd(q32);
    __m256d r=_mm256_sub_pd(x,_mm256_mul_pd(n,_mm256_set1_pd(0x1.921fb54400000p0)));
    r=_mm256_sub_pd(r,_mm256_mul_pd(n,_mm256_set1_pd(0x1.0b4611a626331p-34)));
    __m256d z=_mm256_mul_pd(r,r);
    __m256d sp=_mm256_set1_pd(1.0/355687428096000.0);
    sp=_mm256_add_pd(_mm256_set1_pd(-1.0/1307674368000.0),_mm256_mul_pd(z,sp));
    sp=_mm256_add_pd(_mm256_set1_pd(1.0/6227020800.0),_mm256_mul_pd(z,sp));
    sp=_mm256_add_pd(_mm256_set1_pd(-1.0/39916800.0),_mm256_mul_pd(z,sp));
    sp=_mm256_add_pd(_mm256_set1_pd(1.0/362880.0),_mm256_mul_pd(z,sp));
    sp=_mm256_add_pd(_mm256_set1_pd(-1.0/5040.0),_mm256_mul_pd(z,sp));
    sp=_mm256_add_pd(_mm256_set1_pd(1.0/120.0),_mm256_mul_pd(z,sp));
    sp=_mm256_add_pd(_mm256_set1_pd(-1.0/6.0),_mm256_mul_pd(z,sp));
    sp=_mm256_add_pd(r,_mm256_mul_pd(_mm256_mul_pd(r,z),sp));
    __m256d cp=_mm256_set1_pd(1.0/20922789888000.0);
    cp=_mm256_add_pd(_mm256_set1_pd(-1.0/87178291200.0),_mm256_mul_pd(z,cp));
    cp=_mm256_add_pd(_mm256_set1_pd(1.0/479001600.0),_mm256_mul_pd(z,cp));
    cp=_mm256_add_pd(_mm256_set1_pd(-1.0/3628800.0),_mm256_mul_pd(z,cp));
    cp=_mm256_add_pd(_mm256_set1_pd(1.0/40320.0),_mm256_mul_pd(z,cp));
    cp=_mm256_add_pd(_mm256_set1_pd(-1.0/720.0),_mm256_mul_pd(z,cp));
    cp=_mm256_add_pd(_mm256_set1_pd(1.0/24.0),_mm256_mul_pd(z,cp));
    cp=_mm256_add_pd(_mm256_set1_pd(-.5),_mm256_mul_pd(z,cp));
    cp=_mm256_add_pd(_mm256_set1_pd(1),_mm256_mul_pd(z,cp));
    /* One signed quadrant per double lane; only its low two bits select
     * the same sine/cosine swap and signs as the two-lane reference. */
    __m256i q=_mm256_cvtepi32_epi64(q32);
    const __m256i one=_mm256_set1_epi64x(1),two=_mm256_set1_epi64x(2);
    __m256d odd=_mm256_castsi256_pd(_mm256_cmpeq_epi64(_mm256_and_si256(q,one),one));
    __m256d sine=_mm256_or_pd(_mm256_and_pd(odd,cp),_mm256_andnot_pd(odd,sp));
    __m256d cosine=_mm256_or_pd(_mm256_and_pd(odd,sp),_mm256_andnot_pd(odd,cp));
    sine=_mm256_xor_pd(sine,_mm256_castsi256_pd(_mm256_slli_epi64(_mm256_and_si256(q,two),62)));
    cosine=_mm256_xor_pd(cosine,_mm256_castsi256_pd(_mm256_slli_epi64(
        _mm256_and_si256(_mm256_add_epi64(q,one),two),62)));
    __m256d iszero=_mm256_cmp_pd(x,_mm256_setzero_pd(),_CMP_EQ_OQ);
    sine=_mm256_or_pd(_mm256_and_pd(iszero,x),_mm256_andnot_pd(iszero,sine));
    __m256d amplitude=_mm256_loadu_pd(magnitude);
    _mm256_storeu_pd(real,_mm256_mul_pd(amplitude,cosine));
    _mm256_storeu_pd(imag,_mm256_mul_pd(amplitude,sine));
}
#endif

#endif
static void causal_kernel(struct reloq_rate_transfer *s,
    double spectrum[FFT_SIZE],int unit_dc)
{
    double imag[FFT_SIZE],magnitude[FFT_SIZE/2+1];
    for(unsigned i=0;i<=FFT_SIZE/2;++i) {
        /* The fixed positive floor also covers NaN. A comparison has the
         * same value as fmax here and needs no out-of-line math call. */
        magnitude[i]=spectrum[i]>1e-12?spectrum[i]:1e-12;
        spectrum[i]=magnitude[i]==1?0:log(magnitude[i]);
        if(i && i<FFT_SIZE/2)spectrum[FFT_SIZE-i]=spectrum[i];
    }
    even_transform(spectrum);
    for(unsigned i=0;i<FFT_SIZE;++i) {
        double cepstrum=spectrum[i]/FFT_SIZE;
        spectrum[i]=i==0||i==FFT_SIZE/2?cepstrum:
            i<FFT_SIZE/2?2*cepstrum:0;
    }
    /* The causal cepstrum changes only phase. Its real transform is the
     * original log magnitude, so retain that magnitude directly instead
     * of recovering it with 257 exponentials after two transforms. This
     * identity preserves the complete grid and spectral factorization. */
    real_transform(spectrum,imag);
    unsigned at=0;
#ifdef RATE_PAIRS
#ifdef RATE_AVX2
    if(avx2_enabled())
        for(;at<FFT_SIZE/2;at+=4)exponentiate_four(spectrum+at,imag+at,magnitude+at);
#endif
    for(;at<FFT_SIZE/2;at+=2)exponentiate_pair(spectrum+at,imag+at,magnitude+at);
#endif
    for(unsigned i=at;i<=FFT_SIZE/2;++i) {
        double phase=-imag[i];
        spectrum[i]=magnitude[i]*cos(phase);imag[i]=magnitude[i]*sin(phase);
    }
    hermitian_transform(spectrum,imag);
    double dc=0;
    unsigned taps=s->taps?s->taps:RELOQ_RATE_TAPS;
    for(unsigned i=0;i<RELOQ_RATE_TAPS;++i) {
        double window=taps==RELOQ_RATE_TAPS?transform.window[i]:taps==65?transform.noise_window[i]:
            i<taps*3/4?1:i<taps?.5*(1+cos(pi*(i-taps*3/4)/(taps-1-taps*3/4))):0;
        s->target[i]=(float)(spectrum[i]/FFT_SIZE*window);
        dc+=s->target[i];
    }
    if(unit_dc && fabs(dc)>1e-12)
        for(unsigned i=0;i<RELOQ_RATE_TAPS;++i)s->target[i]/=(float)dc;
}
struct pole {double a,b,c;};
static struct pole pole_at(const struct reloq_core_formant *f,double rate)
{
    double r=exp(-pi*f->bandwidth_hz/rate),b=2*r*cos(2*pi*f->frequency_hz/rate),c=-r*r;
    return (struct pole){1-b-c,b,c};
}
static double power(struct pole p,double complex z)
{
    double complex d=1-p.b*z-multiply(p.c*z,z);
    double denominator=creal(d)*creal(d)+cimag(d)*cimag(d);
    return p.a*p.a/(denominator>1e-30?denominator:1e-30);
}
#ifdef RATE_PAIRS
/* Two independent frequency points follow the scalar pole equation in
 * separate double lanes. Products across poles still accumulate in the
 * original order. No reciprocal approximation or reassociation is used.
 * The grid and pole coefficients are finite; a NaN denominator retains
 * fmax's constant-floor result as well. */
static inline __m128d power_pair(struct pole p,__m128d zr,__m128d zi)
{
    __m128d c=_mm_set1_pd(p.c),b=_mm_set1_pd(p.b),a=_mm_set1_pd(p.a);
    __m128d cr=_mm_mul_pd(c,zr),ci=_mm_mul_pd(c,zi);
    __m128d mr=_mm_sub_pd(_mm_mul_pd(cr,zr),_mm_mul_pd(ci,zi));
    __m128d mi=_mm_add_pd(_mm_mul_pd(cr,zi),_mm_mul_pd(ci,zr));
    /* multiply() constructs mr + I*mi, including its zero real term. */
    mr=_mm_add_pd(mr,_mm_mul_pd(_mm_setzero_pd(),mi));
    __m128d dr=_mm_sub_pd(_mm_sub_pd(_mm_set1_pd(1),_mm_mul_pd(b,zr)),mr);
    __m128d di=_mm_sub_pd(_mm_xor_pd(_mm_mul_pd(b,zi),_mm_set1_pd(-0.0)),mi);
    __m128d den=_mm_add_pd(_mm_mul_pd(dr,dr),_mm_mul_pd(di,di));
    return _mm_div_pd(_mm_mul_pd(a,a),_mm_max_pd(den,_mm_set1_pd(1e-30)));
}
static void spectrum_pairs(double spectrum[FFT_SIZE],const struct frequency_grid *g,
    const struct pole before[12],const struct pole after[12],unsigned old_count,
    long count,double low,double high,const int equal_special[2])
{
    for(unsigned k=0;k<=FFT_SIZE/2;k+=2) {
        if(g->hz[k]>=high) {
            spectrum[k]=1;if(k<FFT_SIZE/2)spectrum[k+1]=1;
            continue;
        }
        /* A half-spectrum has 257 bins; its final bin is handled below
         * without reading a second element beyond the immutable grid. */
        unsigned second=k<FFT_SIZE/2?k+1:k;
        __m128d r0=_mm_set_pd(g->before_real[second],g->before_real[k]);
        __m128d i0=_mm_set_pd(g->before_imag[second],g->before_imag[k]);
        __m128d r1=_mm_set_pd(g->after_real[second],g->after_real[k]);
        __m128d i1=_mm_set_pd(g->after_imag[second],g->after_imag[k]);
        __m128d a=_mm_set1_pd(1),b=a,floor=_mm_set1_pd(1e-30);
        for(unsigned j=0;j<old_count;++j)a=_mm_mul_pd(a,power_pair(before[j],r0,i0));
        for(long j=0;j<count;++j)b=_mm_mul_pd(b,power_pair(after[j],r1,i1));
        for(unsigned j=8;j<12;j+=2) {
            __m128d p0=power_pair(before[j],r0,i0),p1=power_pair(after[j],r1,i1);
            a=_mm_mul_pd(a,p0);b=_mm_mul_pd(b,p1);
            if(!equal_special[(j-8)/2]) {
                p0=power_pair(before[j+1],r0,i0);p1=power_pair(after[j+1],r1,i1);
            }
            a=_mm_div_pd(a,_mm_max_pd(p0,floor));b=_mm_div_pd(b,_mm_max_pd(p1,floor));
        }
        double gains[2];_mm_storeu_pd(gains,_mm_sqrt_pd(_mm_div_pd(a,_mm_max_pd(b,floor))));
        for(unsigned n=0;n<2&&k+n<=FFT_SIZE/2;++n) {
            double hz=g->hz[k+n],gain=gains[n];
            if(hz>=high)gain=1;
            else {
                if(!isfinite(gain))gain=1;
                if(hz>low)gain=exp(log(fmax(gain,1e-12))*.5*(1+cos(pi*(hz-low)/(high-low))));
            }
            spectrum[k+n]=gain;
        }
    }
}
#endif
/* The same linear one-millisecond ramp is shared by every tap. Calculate
 * its slope once per frame; division inside every sample/tap is unnecessary. */
static void prepare_ramp(struct reloq_rate_transfer *s,float rate)
{
    if(!s->ready){memcpy(s->kernel,s->target,sizeof s->kernel);s->ready=1;}
    s->remaining=(unsigned)(rate*.001f);if(!s->remaining)s->remaining=1;
    for(unsigned i=0;i<RELOQ_RATE_TAPS;++i)
        s->step[i]=(s->target[i]-s->kernel[i])/s->remaining;
}
/* Large fit workspaces belong only to actual filter changes. Keep the
 * ordinary key check and clock bypass below out of their stack frames. */
static RATE_NOINLINE void prepare_colour(struct reloq_rate_transfer *s,float rate)
{
    /* Keep the shared tract/aspiration response. Periodic excitation
     * gets the inverse of this old peak followed by a monotonic
     * shelf, so the added brightness does not alter AH or AF noise. */
    const double a=1.1885022274370185;
    double w=2*pi*4000/rate,alpha=sin(w)/4,d=1+alpha/a;
    s->presence[0]=(1+alpha*a)/d;s->presence[1]=-2*cos(w)/d;
    s->presence[2]=(1-alpha*a)/d;s->presence[3]=-2*cos(w)/d;
    s->presence[4]=(1-alpha/a)/d;s->presence_rate=rate;
    double *inverse=s->voice_colour[0],b=s->presence[0];
    inverse[0]=1/b;inverse[1]=s->presence[3]/b;inverse[2]=s->presence[4]/b;
    inverse[3]=s->presence[1]/b;inverse[4]=s->presence[2]/b;
    /* +3 dB upper shelf at 4.5 kHz, with 0.5 dB compensation of
     * periodic voicing. Continuous causal states, no lookahead. */
    const double gain=0.9440608762859234;
    w=2*pi*4500/rate;
    double c=cos(w),beta=sqrt(2*a)*sin(w),*shelf=s->voice_colour[1];
    d=(a+1)-(a-1)*c+beta;
    shelf[0]=gain*a*((a+1)+(a-1)*c+beta)/d;
    shelf[1]=-2*gain*a*((a-1)+(a+1)*c)/d;
    shelf[2]=gain*a*((a+1)+(a-1)*c-beta)/d;
    shelf[3]=2*((a-1)-(a+1)*c)/d;
    shelf[4]=((a+1)-(a-1)*c-beta)/d;
    /* Strengthen the upper harmonics themselves, independently of
     * the accepted aspiration reduction. The 4.6 kHz bell is +3 dB,
     * Q=1; -0.5 dB broadband compensation keeps the lower tract level
     * controlled. AH and AF never enter this periodic-only cascade. */
    const double upper=1.1885022274370185,trim=0.9440608762859234;
    w=2*pi*4600/rate;alpha=sin(w)/2;d=1+alpha/upper;
    double *brilliance=s->voice_colour[2];
    brilliance[0]=trim*(1+alpha*upper)/d;brilliance[1]=-2*trim*cos(w)/d;
    brilliance[2]=trim*(1-alpha*upper)/d;brilliance[3]=-2*cos(w)/d;
    brilliance[4]=(1-alpha/upper)/d;
    /* Additional periodic presence: a fourth-order minimum-phase
     * band with a +2.5-dB plateau from 3.5 to 4 kHz. A Butterworth
     * low-pass shelf is mapped to a band using prewarped edge clocks.
     * Its edge level is about 2.44 dB at every supported rate; the centre
     * reaches 2.5 dB. Keep the pre-presence master gain instead of lowering
     * the bass with a broadband trim. No lookahead or saturation. */
    const double band_gain=1.333521432163324; /* 10^(2.5/20) */
    const double lo=tan(pi*3500/rate),hi=tan(pi*4000/rate);
    const double centre=sqrt(lo*hi),width=2.2*(hi-lo);
    const double complex pole=(-1+I)/sqrt(2),zero=pole*sqrt(band_gain);
    const double complex pd=csqrt(pole*pole*width*width-4*centre*centre);
    const double complex zd=csqrt(zero*zero*width*width-4*centre*centre);
    for(unsigned section=0;section<2;++section) {
        double sign=section?-1:1;
        double complex p=(pole*width+sign*pd)*.5;
        double complex z=(zero*width+sign*zd)*.5;
        double pp=creal(p)*creal(p)+cimag(p)*cimag(p);
        double zz=creal(z)*creal(z)+cimag(z)*cimag(z);
        double den=1-2*creal(p)+pp;
        double *c=section?s->voice_band:s->voice_colour[3];
        c[0]=(1-2*creal(z)+zz)/den;
        c[1]=(-2+2*zz)/den;c[2]=(1+2*creal(z)+zz)/den;
        c[3]=(-2+2*pp)/den;c[4]=(1+2*creal(p)+pp)/den;
    }
    s->aspiration_step=1-exp(-1/(rate*.005));
}
static RATE_NOINLINE void fit_tract(struct reloq_rate_transfer *s,
    const struct reloq_core_voiced_formants *f,float rate,float reference,unsigned old_count)
{
    const struct reloq_core_formant *special[4]={&f->nasal_pole,&f->nasal_zero,&f->tracheal_pole,&f->tracheal_zero};
    struct pole before[12],after[12];
    /* Only active oral poles are read by the magnitude calculation.
     * Prepare them at their actual clocks; leave the remaining oral
     * slots unused. Equal named pole/zero inputs share their exact local
     * calculation, without a persistent cache or cancelled products. */
    for(unsigned i=0;i<8;++i) {
        if(i<old_count)before[i]=pole_at(f->oral+i,reference);
        if((long)i<f->highest_formant)after[i]=pole_at(f->oral+i,rate);
    }
    for(unsigned i=8;i<12;++i) {
        if((i&1) && !memcmp(special[i-8],special[i-9],sizeof **special)) {
            before[i]=before[i-1];after[i]=after[i-1];
        } else {
            before[i]=pole_at(special[i-8],reference);
            after[i]=pole_at(special[i-8],rate);
        }
    }
    const struct frequency_grid *grid=prepare_grid(rate,reference);
    /* Equal pole/zero coefficients have the same response at every bin.
     * Reuse that local value, but keep both the multiplication and division
     * in their original order; cancelling the pair would change rounding. */
    int equal_special[2];
    for(unsigned i=0;i<2;++i) {
        unsigned j=8+2*i;
        equal_special[i]=!memcmp(before+j,before+j+1,sizeof *before) &&
            !memcmp(after+j,after+j+1,sizeof *after);
    }
    double spectrum[FFT_SIZE]={0};
    double width=fmax(f->oral[old_count-1].bandwidth_hz,reference/FFT_SIZE);
    double low=reference*.5-width,high=reference*.5+width;
#ifdef RATE_PAIRS
    if(grid)spectrum_pairs(spectrum,grid,before,after,old_count,f->highest_formant,low,high,equal_special);
    else
#endif
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE,gain=1;
        if(hz<high) {
            double at=fmin(hz,reference*.5);
            double complex z0=grid?complex_pair(grid->before_real[k],grid->before_imag[k]):cexp(-I*2*pi*at/reference),
                z1=grid?complex_pair(grid->after_real[k],grid->after_imag[k]):cexp(-I*2*pi*at/rate);
            double a=1,b=1;
            for(unsigned j=0;j<old_count;++j)a*=power(before[j],z0);
            for(long j=0;j<f->highest_formant;++j)b*=power(after[j],z1);
            for(unsigned j=8;j<12;j+=2) {
                double p0=power(before[j],z0),p1=power(after[j],z1);
                a*=p0;b*=p1;
                if(!equal_special[(j-8)/2]) {
                    p0=power(before[j+1],z0);p1=power(after[j+1],z1);
                }
                a/=p0>1e-30?p0:1e-30;b/=p1>1e-30?p1:1e-30;
            }
            gain=sqrt(a/(b>1e-30?b:1e-30));
            if(!isfinite(gain))gain=1;
            /* Fade only across the measured top pole's bandwidth. The
             * reference contains no information above its Nyquist. */
            if(hz>low)gain=exp(log(gain>1e-12?gain:1e-12)*.5*(1+cos(pi*(hz-low)/(high-low))));
        }
        spectrum[k]=gain;
        if(k && k<FFT_SIZE/2)spectrum[FFT_SIZE-k]=gain;
    }
    causal_kernel(s,spectrum,1);
    prepare_ramp(s,rate);
}
void reloq_rate_transfer_prepare(struct reloq_rate_transfer *s,
    const struct reloq_core_voiced_formants *f,float rate,float reference,unsigned old_count)
{
    if(reference<=0 || rate<=reference)return;
    if(rate>=16000 && s->presence_rate!=rate)prepare_colour(s,rate);
    if(old_count<1 || old_count>5)old_count=5;
    float key[28]={rate,reference,(float)old_count,(float)f->highest_formant};
    const struct reloq_core_formant *special[4]={&f->nasal_pole,&f->nasal_zero,&f->tracheal_pole,&f->tracheal_zero};
    for(unsigned i=0;i<8;++i){key[4+2*i]=f->oral[i].frequency_hz;key[5+2*i]=f->oral[i].bandwidth_hz;}
    for(unsigned i=0;i<4;++i){key[20+2*i]=special[i]->frequency_hz;key[21+2*i]=special[i]->bandwidth_hz;}
    if(s->ready && !memcmp(key,s->key,sizeof key))return;
    memcpy(s->key,key,sizeof key);
    fit_tract(s,f,rate,reference,old_count);
}

void reloq_rate_transfer_process_voice(struct reloq_rate_transfer *s,float *samples,size_t count)
{
    if(s->presence_rate<=0)return;
    for(size_t j=0;j<count;++j) {
        double y=samples[j];
        for(unsigned k=0;k<RELOQ_RATE_VOICE_COLOURS;++k) {
            const double *c=s->voice_colour[k];double *d=s->voice_colour_delay[k];
            double x=y;y=c[0]*x+d[0];
            d[0]=c[1]*x-c[3]*y+d[1];d[1]=c[2]*x-c[4]*y;
        }
        if(s->voice_band[0]!=0) {
            const double *c=s->voice_band;double *d=s->voice_band_delay;
            double x=y;y=c[0]*x+d[0];
            d[0]=c[1]*x-c[3]*y+d[1];d[1]=c[2]*x-c[4]*y;
        }
        samples[j]=(float)y;
    }
}
void reloq_rate_transfer_process_aspiration(struct reloq_rate_transfer *s,
    float *samples,size_t count,int voiced)
{
    if(s->presence_rate<=0)return;
    /* Reduce the breath component of voicing by 2 dB. Unvoiced h keeps
     * its level. A continuous 5-ms gain approach avoids a new step at
     * voice on/off and is independent of callback/frame boundaries. */
    const double reduction=1-0.7943282347242815;
    for(size_t j=0;j<count;++j) {
        s->aspiration_voicing+=(!!voiced-s->aspiration_voicing)*s->aspiration_step;
        samples[j]=(float)(samples[j]*(1-reduction*s->aspiration_voicing));
    }
}
#ifdef RATE_PAIRS
/* One coefficient per output, four chronological samples. Inlining each
 * of the four transposed tap vectors keeps them in registers instead of
 * round-tripping an indexed coefficient array through the stack. */
static inline void accumulate_four(__m128 k,const float *history,
    __m128d *first,__m128d *last)
{
    __m128 h=_mm_loadu_ps(history);
    *first=_mm_add_pd(*first,_mm_mul_pd(_mm_cvtps_pd(k),_mm_cvtps_pd(h)));
    *last=_mm_add_pd(*last,_mm_mul_pd(
        _mm_cvtps_pd(_mm_movehl_ps(k,k)),
        _mm_cvtps_pd(_mm_movehl_ps(h,h))));
}
#endif
static void process_baseline(struct reloq_rate_transfer *s,float *samples,size_t count)
{
    if(!s->ready)return;
    unsigned taps=s->taps?s->taps:RELOQ_RATE_TAPS;
    for(size_t j=0;j<count;) {
#ifdef RATE_PAIRS
        /* Each double lane follows one output sample through the taps in
         * their original order. Independent output sums can run together;
         * a horizontal reduction of tap products cannot preserve rounding.
         * Do not cross the history ring's wrap within one group. */
        if(count-j>=4 && s->cursor+4<=RELOQ_RATE_TAPS &&
           (!s->remaining || s->remaining>4)) {
            unsigned at=s->cursor+RELOQ_RATE_TAPS;
            /* Write the upper mirror first. Writing a future sample into
             * the lower mirror now would destroy an older sample still
             * needed by the preceding output's final taps. */
            memcpy(s->history+at,samples+j,4*sizeof *samples);
            if(s->key[0]==-1 && !s->remaining) {
                memcpy(s->history+s->cursor,s->history+at,4*sizeof *samples);
                s->cursor+=4;
                if(s->cursor==RELOQ_RATE_TAPS)s->cursor=0;
                j+=4;continue;
            }
            __m128d first=_mm_setzero_pd(),last=_mm_setzero_pd();
            if(s->remaining) {
                const unsigned remaining=s->remaining;
                /* Every addition keeps its original float rounding. The
                 * last four ramp samples use the single-sample path below
                 * so the final target copy has its original position.
                 * Inactive taps still take all four ramp steps. */
                unsigned i=0;
                /* Four taps take the same four rounded float additions
                 * in independent lanes. Transposition then presents those
                 * coefficients in time order to each output accumulator.
                 * No tap products or sums are reassociated. */
                for(;i+3<taps;i+=4) {
                    __m128 step=_mm_loadu_ps(s->step+i);
                    __m128 k0=_mm_add_ps(_mm_loadu_ps(s->kernel+i),step);
                    __m128 k1=_mm_add_ps(k0,step),k2=_mm_add_ps(k1,step);
                    __m128 k3=_mm_add_ps(k2,step);
                    _mm_storeu_ps(s->kernel+i,k3);
                    _MM_TRANSPOSE4_PS(k0,k1,k2,k3);
                    accumulate_four(k0,s->history+at-i,&first,&last);
                    accumulate_four(k1,s->history+at-i-1,&first,&last);
                    accumulate_four(k2,s->history+at-i-2,&first,&last);
                    accumulate_four(k3,s->history+at-i-3,&first,&last);
                }
                for(;i<taps;++i) {
                    float step=s->step[i];
                    float k0=s->kernel[i]+step,k1=k0+step;
                    float k2=k1+step,k3=k2+step;
                    s->kernel[i]=k3;
                    __m128 h=_mm_loadu_ps(s->history+at-i);
                    __m128 k=_mm_set_ps(k3,k2,k1,k0);
                    first=_mm_add_pd(first,_mm_mul_pd(_mm_cvtps_pd(k),_mm_cvtps_pd(h)));
                    last=_mm_add_pd(last,_mm_mul_pd(
                        _mm_cvtps_pd(_mm_movehl_ps(k,k)),
                        _mm_cvtps_pd(_mm_movehl_ps(h,h))));
                }
                /* Short noise kernels still carry the complete ramp state
                 * for a subsequent voiced frame. Update their inactive taps
                 * four at a time, retaining all four rounded float steps. */
                for(;i+3<RELOQ_RATE_TAPS;i+=4) {
                    __m128 step=_mm_loadu_ps(s->step+i);
                    __m128 k=_mm_loadu_ps(s->kernel+i);
                    k=_mm_add_ps(k,step);k=_mm_add_ps(k,step);
                    k=_mm_add_ps(k,step);k=_mm_add_ps(k,step);
                    _mm_storeu_ps(s->kernel+i,k);
                }
                for(;i<RELOQ_RATE_TAPS;++i) {
                    float step=s->step[i],k=s->kernel[i];
                    k+=step;k+=step;k+=step;k+=step;s->kernel[i]=k;
                }
                s->remaining=remaining-4;
            } else {
                for(unsigned i=0;i<taps;++i) {
                    __m128 h=_mm_loadu_ps(s->history+at-i);
                    __m128d coefficient=_mm_set1_pd((double)s->kernel[i]);
                    first=_mm_add_pd(first,_mm_mul_pd(coefficient,_mm_cvtps_pd(h)));
                    last=_mm_add_pd(last,_mm_mul_pd(coefficient,
                        _mm_cvtps_pd(_mm_movehl_ps(h,h))));
                }
            }
            double values[4];
            _mm_storeu_pd(values,first);_mm_storeu_pd(values+2,last);
            memcpy(s->history+s->cursor,s->history+at,4*sizeof *samples);
            for(unsigned n=0;n<4;++n) {
                double y=values[n];
                if(s->presence_rate>0) {
                    const double *c=s->presence;double *d=s->presence_delay;
                    double x=y;y=c[0]*x+d[0];
                    d[0]=c[1]*x-c[3]*y+d[1];d[1]=c[2]*x-c[4]*y;
                }
                samples[j+n]=(float)y;
            }
            s->cursor+=4;
            if(s->cursor==RELOQ_RATE_TAPS)s->cursor=0;
            j+=4;continue;
        }
#endif
        unsigned at=s->cursor+RELOQ_RATE_TAPS;
        s->history[s->cursor]=s->history[at]=samples[j];double y=0;
        if(s->key[0]==-1 && !s->remaining) {
            s->cursor=(s->cursor+1)%RELOQ_RATE_TAPS;++j;continue;
        }
        if(s->remaining==1)memcpy(s->kernel,s->target,sizeof s->kernel);
        else if(s->remaining)
            for(unsigned i=0;i<RELOQ_RATE_TAPS;++i)s->kernel[i]+=s->step[i];
        unsigned i=0;
#ifdef RATE_PAIRS
        /* Calculate four products together, then add them in the original
         * left-to-right double order. Reassociating this sum would recolour
         * boundary samples after PCM rounding. The last tap stays scalar. */
        for(;i+3<taps;i+=4) {
            __m128 k=_mm_loadu_ps(s->kernel+i);
            __m128 h=_mm_loadu_ps(s->history+at-i-3);
            h=_mm_shuffle_ps(h,h,_MM_SHUFFLE(0,1,2,3));
            __m128d first=_mm_mul_pd(_mm_cvtps_pd(k),_mm_cvtps_pd(h));
            __m128d last=_mm_mul_pd(_mm_cvtps_pd(_mm_movehl_ps(k,k)),
                _mm_cvtps_pd(_mm_movehl_ps(h,h)));
            double products[4];
            _mm_storeu_pd(products,first);_mm_storeu_pd(products+2,last);
            y+=products[0];y+=products[1];y+=products[2];y+=products[3];
        }
#endif
        for(;i<taps;++i)
            y+=(double)s->kernel[i]*s->history[at-i];
        if(s->presence_rate>0) {
            const double *c=s->presence;double *d=s->presence_delay;
            double x=y;y=c[0]*x+d[0];
            d[0]=c[1]*x-c[3]*y+d[1];d[1]=c[2]*x-c[4]*y;
        }
        samples[j]=(float)y;
        if(s->remaining)--s->remaining;
        s->cursor=(s->cursor+1)%RELOQ_RATE_TAPS;
        ++j;
    }
}

#ifdef RATE_AVX2
static int avx2_enabled(void)
{
    static atomic_int selected;
    int choice=atomic_load_explicit(&selected,memory_order_relaxed);
    if(!choice) {
        int supported=!!__builtin_cpu_supports("avx2");
#ifdef _WIN32
        /* Windows on ARM can advertise emulated AVX2. Keep its established
         * x64 path until that wider emulated path is measured on a device.
         * Resolve the query dynamically to retain older Windows loaders. */
        typedef BOOL (WINAPI *machine_query)(HANDLE,USHORT *,USHORT *);
        FARPROC address=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"IsWow64Process2");
        machine_query query=NULL;
        _Static_assert(sizeof query==sizeof address,"Windows function pointers");
        memcpy(&query,&address,sizeof query);
        USHORT process=0,native=0;
        if(query&&query(GetCurrentProcess(),&process,&native)&&native==IMAGE_FILE_MACHINE_ARM64)
            supported=0;
#endif
        choice=supported?2:1;
        atomic_store_explicit(&selected,choice,memory_order_relaxed);
    }
    return choice==2;
}
/* Four output samples follow their original tap sums in four double lanes.
 * Runtime dispatch retains the baseline on older CPUs and emulated x64.
 * There is no horizontal sum, fused operation or approximate reciprocal. */
__attribute__((target("avx2")))
static void process_avx2(struct reloq_rate_transfer *s,float *samples,size_t count)
{
    unsigned taps=s->taps?s->taps:RELOQ_RATE_TAPS;
    for(size_t j=0;j<count;) {
        if(count-j<4 || s->cursor+4>RELOQ_RATE_TAPS ||
           (s->remaining && s->remaining<=4)) {
            process_baseline(s,samples+j,1);++j;continue;
        }
        unsigned at=s->cursor+RELOQ_RATE_TAPS;
        memcpy(s->history+at,samples+j,4*sizeof *samples);
        if(s->key[0]==-1 && !s->remaining) {
            memcpy(s->history+s->cursor,s->history+at,4*sizeof *samples);
            s->cursor+=4;if(s->cursor==RELOQ_RATE_TAPS)s->cursor=0;
            j+=4;continue;
        }
        __m256d sum=_mm256_setzero_pd();
        if(s->remaining) {
            unsigned i=0;
            for(;i+3<taps;i+=4) {
                __m128 step=_mm_loadu_ps(s->step+i);
                __m128 k0=_mm_add_ps(_mm_loadu_ps(s->kernel+i),step);
                __m128 k1=_mm_add_ps(k0,step),k2=_mm_add_ps(k1,step);
                __m128 k3=_mm_add_ps(k2,step);
                _mm_storeu_ps(s->kernel+i,k3);
                _MM_TRANSPOSE4_PS(k0,k1,k2,k3);
                sum=_mm256_add_pd(sum,_mm256_mul_pd(_mm256_cvtps_pd(k0),_mm256_cvtps_pd(_mm_loadu_ps(s->history+at-i))));
                sum=_mm256_add_pd(sum,_mm256_mul_pd(_mm256_cvtps_pd(k1),_mm256_cvtps_pd(_mm_loadu_ps(s->history+at-i-1))));
                sum=_mm256_add_pd(sum,_mm256_mul_pd(_mm256_cvtps_pd(k2),_mm256_cvtps_pd(_mm_loadu_ps(s->history+at-i-2))));
                sum=_mm256_add_pd(sum,_mm256_mul_pd(_mm256_cvtps_pd(k3),_mm256_cvtps_pd(_mm_loadu_ps(s->history+at-i-3))));
            }
            for(;i<taps;++i) {
                float step=s->step[i],k0=s->kernel[i]+step,k1=k0+step;
                float k2=k1+step,k3=k2+step;s->kernel[i]=k3;
                __m128 k=_mm_set_ps(k3,k2,k1,k0);
                sum=_mm256_add_pd(sum,_mm256_mul_pd(_mm256_cvtps_pd(k),_mm256_cvtps_pd(_mm_loadu_ps(s->history+at-i))));
            }
            for(;i+7<RELOQ_RATE_TAPS;i+=8) {
                __m256 step=_mm256_loadu_ps(s->step+i),k=_mm256_loadu_ps(s->kernel+i);
                k=_mm256_add_ps(k,step);k=_mm256_add_ps(k,step);
                k=_mm256_add_ps(k,step);k=_mm256_add_ps(k,step);
                _mm256_storeu_ps(s->kernel+i,k);
            }
            for(;i<RELOQ_RATE_TAPS;++i) {
                float step=s->step[i],k=s->kernel[i];
                k+=step;k+=step;k+=step;k+=step;s->kernel[i]=k;
            }
            s->remaining-=4;
        } else for(unsigned i=0;i<taps;++i) {
            __m256d k=_mm256_set1_pd((double)s->kernel[i]);
            sum=_mm256_add_pd(sum,_mm256_mul_pd(k,_mm256_cvtps_pd(_mm_loadu_ps(s->history+at-i))));
        }
        double values[4];_mm256_storeu_pd(values,sum);
        memcpy(s->history+s->cursor,s->history+at,4*sizeof *samples);
        for(unsigned n=0;n<4;++n) {
            double y=values[n];
            if(s->presence_rate>0) {
                const double *c=s->presence;double *d=s->presence_delay;
                double x=y;y=c[0]*x+d[0];
                d[0]=c[1]*x-c[3]*y+d[1];d[1]=c[2]*x-c[4]*y;
            }
            samples[j+n]=(float)y;
        }
        s->cursor+=4;if(s->cursor==RELOQ_RATE_TAPS)s->cursor=0;
        j+=4;
    }
}
#endif
void reloq_rate_transfer_process(struct reloq_rate_transfer *s,float *samples,size_t count)
{
    if(!s->ready)return;
#ifdef RATE_AVX2
    if(avx2_enabled()){process_avx2(s,samples,count);return;}
#endif
    process_baseline(s,samples,count);
}

static double complex noise_response(const struct reloq_core_frication_bank *b,
    double complex z)
{
    double complex y=b->bypass_gain;
    if(b->bypass_reference>0 && b->bypass_rate>b->bypass_reference) {
        y*=b->bypass_scale;
        for(unsigned j=0;j<2;++j) {
            const float *c=b->bypass_coefficients[j];
            y*=(c[0]+c[1]*z+c[2]*z*z)/(1+c[3]*z+c[4]*z*z);
        }
    }
    for(long j=0;j<b->active_formants;++j) {
        const struct reloq_core_resonator *p=b->stages+j;
        y+=p->input_gain/(1-p->latest_gain*z-p->previous_gain*z*z);
    }
    return y;
}

static RATE_NOINLINE void fit_noise(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *old,
    const struct reloq_core_frication_bank *wide,float rate,float reference,
    float boundary_width,double norm)
{
    const double width=fmax(boundary_width,reference/FFT_SIZE);
    const double low=reference*.5-width,high=reference*.5+width;
    double before[FFT_SIZE/2+1],after[FFT_SIZE/2+1],peak=0;
    /* Above this transition the transfer is exactly unity. One bin past
     * its edge retains the reference Nyquist in the peak calculation. */
    unsigned last=(unsigned)ceil(high*FFT_SIZE/rate);
    if(last>FFT_SIZE/2)last=FFT_SIZE/2;
    for(unsigned k=0;k<=last;++k) {
        double hz=fmin((double)k*rate/FFT_SIZE,reference*.5);
        before[k]=cabs(noise_response(old,cexp(-I*2*pi*hz/reference)))/norm;
        after[k]=cabs(noise_response(wide,cexp(-I*2*pi*hz/rate)))/norm;
        if(before[k]>peak)peak=before[k];
    }
    double spectrum[FFT_SIZE]={0};
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE,gain=1;
        if(hz<high) {
            /* Noise PSD, not per-sample variance, is the quantity shared
             * by two sample clocks. A deep cancellation is regularized
             * at -60 dB of the reference peak to avoid unstable boosts. */
            double floor=peak*.001;
            gain=sqrt(rate/reference)*before[k]/fmax(after[k],floor);
            gain=fmax(.0625,fmin(16,gain));
            if(hz>low)gain=exp(log(gain)*.5*(1+cos(pi*(hz-low)/(high-low))));
        }
        spectrum[k]=gain;
        if(k && k<FFT_SIZE/2)spectrum[FFT_SIZE-k]=gain;
    }
    causal_kernel(s,spectrum,0);
    prepare_ramp(s,rate);
}
void reloq_rate_transfer_prepare_noise(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *old,
    const struct reloq_core_frication_bank *wide,float rate,float reference,
    float boundary_width)
{
    if(reference<=0 || rate<=reference)return;
    /* Retain the last response while the original AF source rings out. */
    double norm=fabs(old->bypass_gain);
    for(long i=0;i<old->active_formants;++i)norm+=fabs(old->stages[i].input_gain);
    if(norm<1e-20) {
        if(!s->ready) {
            s->kernel[0]=s->target[0]=1;
            s->ready=1;
        }
        return;
    }
    float key[80]={rate,reference,boundary_width,(float)old->active_formants,
        (float)wide->active_formants};
    unsigned at=5;
    const struct reloq_core_frication_bank *banks[2]={old,wide};
    for(unsigned b=0;b<2;++b) {
        const struct reloq_core_frication_bank *bank=banks[b];
        key[at++]=(float)(bank->bypass_gain/norm);
        for(unsigned j=0;j<8;++j) {
            key[at++]=(float)(bank->stages[j].input_gain/norm);
            key[at++]=bank->stages[j].latest_gain;
            key[at++]=bank->stages[j].previous_gain;
        }
    }
    key[at++]=wide->bypass_reference;key[at++]=wide->bypass_scale;
    int unchanged=s->ready;
    for(unsigned i=0;unchanged && i<80;++i)if(key[i]!=s->key[i]) {
        /* AF scales all channels together. Dividing that common scale out
         * can differ by a few float ulps without changing the noise shape.
         * Pole coefficients and the sample clocks still compare exactly. */
        int gain=(i>=5 && i<55 && ((i-5)%25==0 || (i-6)%25%3==0));
        if(!gain || fabsf(key[i]-s->key[i])>
           8*FLT_EPSILON*fmaxf(fabsf(key[i]),fabsf(s->key[i])))unchanged=0;
    }
    if(unchanged)return;
    memcpy(s->key,key,sizeof key);
    fit_noise(s,old,wide,rate,reference,boundary_width,norm);
}

/* AF-only spectral transfer. The reference is the wideband contour enabled
 * by fastfinge/eloquence_64: 11.025-kHz lower band, 22.05-kHz companion,
 * 8-kHz fourth-order noise limit and +14 dB above the sinc join. Language,
 * source timing, voiced cascade and aspiration never enter this calculation.
 * Normalize before mixing powers: s/z/t use 25% at 16 kHz, half above it.
 * A common source tilt shapes the mixed S/Z/T energy; other profiles stay fixed.
 * A causal residual avoids adding an audio buffer or resampling speech. */
static double complex section_response(const double c[5],double complex z)
{return (c[0]+c[1]*z+c[2]*z*z)/(1+c[3]*z+c[4]*z*z);}
static double profile_power(const struct reloq_noise_profile *p,double hz)
{
    double complex z=cexp(-I*2*pi*hz/p->rate),low=1,high=1;
    for(unsigned i=0;i<3;++i)low*=section_response(p->target[i],z);
    for(unsigned i=3;i<6;++i)high*=section_response(p->target[i],z);
    double a=cabs(low+p->upper_target*high)*1.8836490894898006;
    return a*a;
}
static double wide_noise_power(double hz)
{
    if(hz>=11025)return 0;
    /* Bilinear fourth-order Butterworth, normalized for white-noise PSD. */
    double x=tan(pi*hz/22050)/tan(pi*8000/22050);
    return 2/(1+pow(x,8));
}
/* Only the new AF fit uses this finite-input real arithmetic. The original
 * narrow-rate and voiced transfer keep their established rounding path. */
static double bank_power(const struct reloq_core_frication_bank *b,double complex z)
{
    if(b->bypass_gain) {
        double complex y=noise_response(b,z);
        return creal(y)*creal(y)+cimag(y)*cimag(y);
    }
    double zr=creal(z),zi=cimag(z),rr=zr*zr-zi*zi,ii=2*zr*zi,yr=0,yi=0;
    for(long j=0;j<b->active_formants;++j) {
        const struct reloq_core_resonator *p=b->stages+j;
        if(!p->input_gain)continue;
        double r=1-p->latest_gain*zr-p->previous_gain*rr;
        double i=-p->latest_gain*zi-p->previous_gain*ii;
        double g=p->input_gain/(r*r+i*i);
        yr+=g*r;yi-=g*i;
    }
    return yr*yr+yi*yi;
}
/* Clock-only responses are shared by all noise profiles. The two
 * profile families retain only their own coefficients and power curve. */
struct noise_clock_grid {
    atomic_int ready;
    double low_weight[FFT_SIZE/2+1],high_weight[FFT_SIZE/2+1],tilt[FFT_SIZE/2+1];
    double complex z[3][FFT_SIZE/2+1];
};
struct noise_grid {
    atomic_int ready;
    double coefficients[6][5],upper,profile[FFT_SIZE/2+1];
    const struct noise_clock_grid *clock;
};
static struct noise_clock_grid noise_clocks[5];
static struct noise_grid noise_grids[5][2];
static unsigned noise_clock_index(float rate)
{
    static const float rates[]={16000,22050,32000,44100,48000};
    unsigned i=0;while(i<5&&rates[i]!=rate)++i;return i;
}
static void fill_noise_clock(struct noise_clock_grid *g,float rate)
{
    /* This source shelf depends on the output clock alone. Calculate its
     * response with the original expression once per immutable grid. */
    const double center=rate==16000?4800:6300;
    const double gain=pow(10,-4.0/20),A=sqrt(gain);
    const double omega=2*pi*center/rate,c=cos(omega),alpha=sin(omega)*sqrt(2*A);
    const double den=(A+1)-(A-1)*c+alpha;
    const double shelf[5]={A*((A+1)+(A-1)*c+alpha)/den,
        -2*A*((A-1)+(A+1)*c)/den,A*((A+1)+(A-1)*c-alpha)/den,
        2*((A-1)-(A+1)*c)/den,((A+1)-(A-1)*c-alpha)/den};
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE;
        g->z[0][k]=cexp(-I*2*pi*hz/rate);
        g->z[1][k]=cexp(-I*2*pi*hz/11025);
        g->z[2][k]=cexp(-I*2*pi*hz/22050);
        g->tilt[k]=cabs(section_response(shelf,g->z[0][k]));
        double blend=hz<=5300?0:hz>=5512.5?1:.5-.5*cos(pi*(hz-5300)/212.5);
        g->low_weight[k]=hz<5512.5?1-blend:0;
        g->high_weight[k]=hz<11025?blend*wide_noise_power(hz)*25.1188643150958*.5:0;
    }
}
static const struct noise_clock_grid *get_noise_clock(unsigned i,float rate)
{
    struct noise_clock_grid *g=&noise_clocks[i];
    if(atomic_load_explicit(&g->ready,memory_order_acquire)!=2) {
        int expected=0;
        if(atomic_compare_exchange_strong(&g->ready,&expected,1)) {
            fill_noise_clock(g,rate);
            atomic_store_explicit(&g->ready,2,memory_order_release);
        } else while(atomic_load_explicit(&g->ready,memory_order_acquire)!=2) {}
    }
    return g;
}
static void fill_noise_grid(struct noise_grid *g,const struct reloq_noise_profile *p,
    float rate,const struct noise_clock_grid *clock)
{
    memcpy(g->coefficients,p->target,sizeof g->coefficients);g->upper=p->upper_target;
    g->clock=clock;
    for(unsigned k=0;k<=FFT_SIZE/2;++k)
        g->profile[k]=profile_power(p,(double)k*rate/FFT_SIZE);
}
static const struct noise_grid *get_noise_grid(const struct reloq_noise_profile *p,
    unsigned i,float rate,const struct noise_clock_grid *clock)
{
    if(p->kind==1||p->kind==2) {
        struct noise_grid *g=&noise_grids[i][p->kind-1];
        if(atomic_load_explicit(&g->ready,memory_order_acquire)!=2) {
            int expected=0;
            if(atomic_compare_exchange_strong(&g->ready,&expected,1)) {
                fill_noise_grid(g,p,rate,clock);
                atomic_store_explicit(&g->ready,2,memory_order_release);
            } else while(atomic_load_explicit(&g->ready,memory_order_acquire)!=2) {}
        }
        if(g->upper==p->upper_target&&!memcmp(g->coefficients,p->target,sizeof g->coefficients))return g;
    }
    return NULL;
}

void reloq_rate_transfer_prepare_noise_mix(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *low,
    const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *profile,float rate,float mix)
{
    reloq_rate_transfer_prepare_noise_mix_profile(s,low,high,current,profile,rate,mix,0);
}
static RATE_NOINLINE void fit_noise_mix(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *low,
    const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,
    const struct noise_grid *grid,float rate,float mix,int historic84,double norm)
{
    double ours[FFT_SIZE/2+1],wanted[FFT_SIZE/2+1],a=0,b=0,peak=0;
    const struct noise_clock_grid *clock=grid->clock;
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        ours[k]=bank_power(current,clock->z[0][k])/(norm*norm)*grid->profile[k];
        /* The measured sinc cutoff is .98 of the original Nyquist. Its
         * short transition is represented by complementary power weights. */
        double base=clock->low_weight[k]?bank_power(low,clock->z[1][k])/(norm*norm):0;
        double top=clock->high_weight[k]?bank_power(high,clock->z[2][k])/(norm*norm):0;
        wanted[k]=clock->low_weight[k]*base+clock->high_weight[k]*top;
        double w=k&&k<FFT_SIZE/2?1:.5;
        a+=w*ours[k];b+=w*wanted[k];if(ours[k]>peak)peak=ours[k];
    }
    if(a<=0||b<=0)return;
    /* Return to the unbroadened 0.2.84 blend. The accepted OpenEVV
     * 16-kHz/49g listening principle is a source tilt, not an extra
     * lower tract pole. Apply that tilt to the complete mixed source so
     * the wide companion cannot bypass it. Retain integrated energy. */
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        const double mag=historic84?1:clock->tilt[k];
        wanted[k]=((1-mix)*ours[k]+mix*wanted[k]*a/b)*mag*mag;
    }
    double spectrum[FFT_SIZE]={0};
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double weighted=wanted[k];
        /* Filling coherent bank notches with a very large inverse gain
         * amplifies short transition impulses. Bound this residual before
         * the integrated power compensation, not with a PCM limiter. */
        spectrum[k]=fmax(.001,fmin(4,sqrt(weighted/fmax(ours[k],peak*1e-8))));
    }
    causal_kernel(s,spectrum,0);
    /* Correct the finite-kernel residual's integrated power as well. */
    /* The spectrum is dead after factorization. Reuse its workspace
     * for the finite-kernel response, preserving all arithmetic. */
    memset(spectrum,0,sizeof spectrum);
    double response_imag[FFT_SIZE]={0};
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)spectrum[j]=s->target[j];
    real_transform(spectrum,response_imag);
    double rendered=0;
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double mag=cabs(complex_pair(spectrum[k],response_imag[k]));
        rendered+=(k&&k<FFT_SIZE/2?1:.5)*ours[k]*mag*mag;
    }
    double compensation=rendered>0?sqrt(a/rendered):1;
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)s->target[j]*=(float)compensation;
    prepare_ramp(s,rate);
    /* Moving through k/s and vowel/s must not replace the whole residual
     * in one millisecond. Continue from the actual current coefficients,
     * including when a new frame arrives before this ramp has finished. */
    s->remaining=(unsigned)(rate*.003f);
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)
        s->step[j]=(s->target[j]-s->kernel[j])/s->remaining;
}
/* Supported clocks never reserve a fallback frequency grid on the
 * common path. Only an unusual profile needs the small profile workspace;
 * arbitrary development clocks additionally need a private clock grid. */
static RATE_NOINLINE void fit_noise_profile(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *low,
    const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *profile,const struct noise_clock_grid *clock,
    float rate,float mix,int historic84,double norm)
{
    struct noise_grid grid;
    fill_noise_grid(&grid,profile,rate,clock);
    fit_noise_mix(s,low,high,current,&grid,rate,mix,historic84,norm);
}
static RATE_NOINLINE void fit_noise_clock(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *low,
    const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *profile,float rate,float mix,int historic84,double norm)
{
    struct noise_clock_grid clock;
    fill_noise_clock(&clock,rate);
    fit_noise_profile(s,low,high,current,profile,&clock,rate,mix,historic84,norm);
}
void reloq_rate_transfer_prepare_noise_mix_profile(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *low,
    const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *profile,float rate,float mix,int historic84)
{
    s->taps=65;
    if(mix<=0) {
        if(s->ready && s->key[0]!=-1) {
            memset(s->target,0,sizeof s->target);s->target[0]=1;
            s->key[0]=-1;prepare_ramp(s,rate);
        }
        return;
    }
    double norm=0;
    for(long i=0;i<current->active_formants;++i)norm+=fabs(current->stages[i].input_gain);
    if(norm<1e-20)return; /* retain release history */
    float key[144]={rate,mix,(float)profile->kind,(float)!!historic84};unsigned at=4;
    const struct reloq_core_frication_bank *banks[]={low,high,current};
    for(unsigned b=0;b<3;++b) {
        key[at++]=(float)banks[b]->active_formants;
        key[at++]=(float)(banks[b]->bypass_gain/norm);
        for(unsigned j=0;j<8;++j) {
            key[at++]=(float)(banks[b]->stages[j].input_gain/norm);
            key[at++]=banks[b]->stages[j].latest_gain;
            key[at++]=banks[b]->stages[j].previous_gain;
        }
    }
    int same=s->ready;
    for(unsigned j=0;same&&j<at;++j)
        if(key[j]!=s->key[j] && fabsf(key[j]-s->key[j])>8*FLT_EPSILON*fmaxf(fabsf(key[j]),fabsf(s->key[j])))same=0;
    if(same)return;
    memcpy(s->key,key,sizeof key);
    unsigned i=noise_clock_index(rate);
    if(i<5) {
        const struct noise_clock_grid *clock=get_noise_clock(i,rate);
        const struct noise_grid *grid=get_noise_grid(profile,i,rate,clock);
        if(grid)fit_noise_mix(s,low,high,current,grid,rate,mix,historic84,norm);
        else fit_noise_profile(s,low,high,current,profile,clock,rate,mix,historic84,norm);
    } else fit_noise_clock(s,low,high,current,profile,rate,mix,historic84,norm);
}
