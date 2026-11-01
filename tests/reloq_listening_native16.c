/* Listening host only. The native 16-kHz parallel bank anchors consonant
 * colour. Production synthesis does not compile or expose this selector. */
#include "../src/reloq_rate_transfer.c"

static _Thread_local struct {
    unsigned enabled;
    int active;
    struct reloq_core_frication_bank bank;
} native16;

__attribute__((visibility("default")))
int reloq_native16_listening_set(unsigned enabled)
{
    if(enabled>1)return -1;
    memset(&native16,0,sizeof native16);native16.enabled=enabled;return 0;
}

void __real_reloq_noise_profile_prepare(struct reloq_noise_profile *,float,
    long,long,long,float[8],float[8],long[8],long *);
void __wrap_reloq_noise_profile_prepare(struct reloq_noise_profile *s,float rate,
    long av,long af,long ab,float f[8],float b[8],long a[8],long *count)
{
    /* Broad AB is the accepted p/f family. Its source, bank and extension
     * are left on the established path. Other families retain five native
     * poles before any wideband upper poles are introduced. */
    int broad=ab && !a[2] && !a[3] && !a[4];
    native16.active=native16.enabled && rate>=16000 && af && !broad;
    if(native16.active) {
        struct reloq_core_clock clock=reloq_core_clock_for_rate(16000);
        native16.bank=(struct reloq_core_frication_bank){0};
        if(reloq_core_frication_bank_configure(&native16.bank,&clock,f,b,a,5,
            af,0,0,0))native16.active=0;
    }
    __real_reloq_noise_profile_prepare(s,rate,av,af,ab,f,b,a,count);
}

void __wrap_reloq_rate_transfer_prepare_noise_mix_profile(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *low,const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *profile,float rate,float mix,int historic84)
{
    if(!native16.active) {
        reloq_rate_transfer_prepare_noise_mix_profile(s,low,high,current,profile,
            rate,mix,historic84);return;
    }
    double norm=0,ref_norm=0;
    for(long j=0;j<current->active_formants;++j)norm+=fabs(current->stages[j].input_gain);
    for(long j=0;j<native16.bank.active_formants;++j)ref_norm+=fabs(native16.bank.stages[j].input_gain);
    if(norm<1e-20 || ref_norm<1e-20)return;
    float key[144]={-16000,rate,(float)profile->kind};unsigned at=3;
    const struct reloq_core_frication_bank *banks[]={&native16.bank,current};
    for(unsigned i=0;i<2;++i) {
        double n=i?norm:ref_norm;
        key[at++]=(float)banks[i]->active_formants;
        for(long j=0;j<8;++j) {
            key[at++]=(float)(banks[i]->stages[j].input_gain/n);
            key[at++]=banks[i]->stages[j].latest_gain;
            key[at++]=banks[i]->stages[j].previous_gain;
        }
    }
    int same=s->ready;
    for(unsigned j=0;same&&j<at;++j)
        if(key[j]!=s->key[j] && fabsf(key[j]-s->key[j])>
            8*FLT_EPSILON*fmaxf(fabsf(key[j]),fabsf(s->key[j])))same=0;
    if(same)return;
    memcpy(s->key,key,sizeof key);s->taps=65;
    double ours[FFT_SIZE/2+1],reference[FFT_SIZE/2+1],target[FFT_SIZE/2+1];
    double sum_ours=0,sum_ref=0,peak=0;
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE,w=k&&k<FFT_SIZE/2?1:.5;
        ours[k]=bank_power(current,cexp(-I*2*pi*hz/rate))/(norm*norm)*profile_power(profile,hz);
        reference[k]=hz<=8000?bank_power(&native16.bank,cexp(-I*2*pi*hz/16000))/(ref_norm*ref_norm):0;
        /* Equal energy over the common lower passband; the physical pole
         * locations and native 16-kHz notch structure remain unchanged. */
        if(hz<=6500){sum_ours+=w*ours[k];sum_ref+=w*reference[k];}
        if(ours[k]>peak)peak=ours[k];
    }
    if(sum_ref<=0 || sum_ours<=0)return;
    double scale=sum_ours/sum_ref,total=0,wanted=0;
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE,w=k&&k<FFT_SIZE/2?1:.5;
        double blend=rate==16000?0:hz<=6500?0:hz>=8000?1:.5-.5*cos(pi*(hz-6500)/1500);
        target[k]=(1-blend)*reference[k]*scale+blend*ours[k];
        total+=w*ours[k];wanted+=w*target[k];
    }
    double spectrum[FFT_SIZE]={0},normalization=wanted>0?total/wanted:1;
    for(unsigned k=0;k<=FFT_SIZE/2;++k)
        spectrum[k]=fmax(.001,fmin(4,sqrt(target[k]*normalization/fmax(ours[k],peak*1e-8))));
    causal_kernel(s,spectrum,0);
    memset(spectrum,0,sizeof spectrum);double imaginary[FFT_SIZE]={0};
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)spectrum[j]=s->target[j];
    real_transform(spectrum,imaginary);double rendered=0;
    for(unsigned k=0;k<=FFT_SIZE/2;++k)
        rendered+=(k&&k<FFT_SIZE/2?1:.5)*ours[k]*(spectrum[k]*spectrum[k]+imaginary[k]*imaginary[k]);
    double compensation=rendered>0?sqrt(total/rendered):1;
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)s->target[j]*=(float)compensation;
    prepare_ramp(s,rate);s->remaining=(unsigned)(rate*.003f);
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)s->step[j]=(s->target[j]-s->kernel[j])/s->remaining;
}
