/* Match the Classic Presence main-band shape of the 11-kHz source.
 * At 16 kHz this is the whole consonant target. Higher native bands enter
 * separately from 22.05 kHz with downward power compensation. */
int reloq_rate_transfer_prepare_classic_bandwidth(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *reference,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *profile,float rate,float extension_scale)
{
    if(!s||!reference||!current||!profile||rate<16000||
       !isfinite(extension_scale)||extension_scale<0||extension_scale>1)return 0;
    double norm=fabs(current->bypass_gain),ref_norm=fabs(reference->bypass_gain);
    for(long j=0;j<current->active_formants;++j)norm+=fabs(current->stages[j].input_gain);
    for(long j=0;j<reference->active_formants;++j)ref_norm+=fabs(reference->stages[j].input_gain);
    if(norm<1e-20 || ref_norm<1e-20)return 0;
    double extension=extension_scale*(rate>=48000?1:rate>=44100?.8:
        rate>=32000?.5:rate>=22050?.25:0);
    if(s->sibilant_extension)extension*=rate>=48000?.35:rate>=44100?.4:
        rate>=32000?.5:.6;
    float key[144]={-11026,rate,(float)profile->kind,(float)extension,
        (float)s->sibilant_extension,s->frication_trim_db};unsigned at=6;
    const struct reloq_core_frication_bank *banks[]={reference,current};
    for(unsigned i=0;i<2;++i) {
        key[at++]=(float)banks[i]->active_formants;
        key[at++]=(float)(banks[i]->bypass_gain/norm);
        for(long j=0;j<8;++j) {
            key[at++]=(float)(banks[i]->stages[j].input_gain/norm);
            key[at++]=banks[i]->stages[j].latest_gain;
            key[at++]=banks[i]->stages[j].previous_gain;
        }
    }
    int same=s->ready;
    for(unsigned j=0;same&&j<at;++j)
        if(key[j]!=s->key[j] && fabsf(key[j]-s->key[j])>
            8*FLT_EPSILON*fmaxf(fabsf(key[j]),fabsf(s->key[j])))same=0;
    if(same)return 1;
    memcpy(s->key,key,sizeof key);s->taps=129;
    double ours[FFT_SIZE/2+1],native_power[FFT_SIZE/2+1],target[FFT_SIZE/2+1];
    double lower_ours=0,lower_ref=0,peak=0;
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE,w=k&&k<FFT_SIZE/2?1:.5;
        /* profile_power's historical reference includes an extra 1 dB;
         * use the actual production source gain for this absolute fit. */
        ours[k]=bank_power(current,cexp(-I*2*pi*hz/rate))/(norm*norm)*
            profile_power(profile,hz)*0.7943282347242815;
        double complex z=cexp(-I*2*pi*hz/11025);
        native_power[k]=hz<=11025?bank_power(reference,z)/(norm*norm)*
            classic_colour_power(hz)*(rate/11025.0):0;
        if(hz<=4600){lower_ours+=w*ours[k];lower_ref+=w*native_power[k];}
        if(ours[k]>peak)peak=ours[k];
    }
    if(lower_ref<=0 || lower_ours<=0)return 0;
    const double upper_scale=lower_ref/lower_ours;
    double wanted=0,baseline=0,added=0,extra[FFT_SIZE/2+1];
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double hz=(double)k*rate/FFT_SIZE,w=k&&k<FFT_SIZE/2?1:.5;
        double upper=hz<=5512.5?0:hz>=6500?1:
            .5-.5*cos(pi*(hz-5512.5)/987.5);
        /* Preserve the complete recognizable S main band. Only its new
         * upper energy rolls off, in physical Hz rather than Nyquist.
         * Other AF keeps the established extension and source contour. */
        double roll=1;
        if(s->sibilant_extension) {
            double ratio=hz/8500,quadratic=ratio*ratio;
            roll=1/(1+quadratic*quadratic);
        }
        extra[k]=extension*upper*upper_scale*ours[k]*roll;
        added+=w*extra[k];
    }
    double budget=extension_scale*lower_ref*(rate>=48000?.20:rate>=44100?.18:
        rate>=32000?.14:rate>=22050?.08:0);
    double scale=s->sibilant_extension&&added>budget?budget/added:1;
    for(unsigned k=0;k<=FFT_SIZE/2;++k) {
        double w=k&&k<FFT_SIZE/2?1:.5;
        target[k]=native_power[k]+scale*extra[k];
        wanted+=w*target[k];
        baseline+=w*native_power[k];
    }
    double spectrum[FFT_SIZE]={0};
    for(unsigned k=0;k<=FFT_SIZE/2;++k)
        spectrum[k]=fmax(.0001,fmin(4,sqrt(target[k]/fmax(ours[k],peak*1e-8))));
    causal_kernel(s,spectrum,0);
    memset(spectrum,0,sizeof spectrum);double imaginary[FFT_SIZE]={0};
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)spectrum[j]=s->target[j];
    real_transform(spectrum,imaginary);double rendered=0;
    for(unsigned k=0;k<=FFT_SIZE/2;++k)
        rendered+=(k&&k<FFT_SIZE/2?1:.5)*ours[k]*(spectrum[k]*spectrum[k]+imaginary[k]*imaginary[k]);
    /* At high rates compensate the extra noise power down to the reference
     * power. This retains the wider spectral shape without a louder hiss.
     * Keep 16 kHz byte-identical to the established Classic foundation. */
    double compensated=rate>=22050?baseline:wanted;
    double compensation=rendered>0?sqrt(compensated/rendered):1;
    if(s->frication_trim_db>0)compensation*=pow(10,-s->frication_trim_db/20.0);
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)s->target[j]*=(float)compensation;
    prepare_ramp(s,rate);s->remaining=(unsigned)(rate*.003f);
    for(unsigned j=0;j<RELOQ_RATE_TAPS;++j)s->step[j]=(s->target[j]-s->kernel[j])/s->remaining;
    return 1;
}

int reloq_rate_transfer_prepare_classic(struct reloq_rate_transfer *s,
    const struct reloq_core_frication_bank *reference,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *profile,float rate)
{
    return reloq_rate_transfer_prepare_classic_bandwidth(s,reference,current,
        profile,rate,1);
}
