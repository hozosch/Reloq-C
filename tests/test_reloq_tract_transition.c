/* The extended speech tract keeps the reference transition in real time,
 * independently of output rate or render chunk boundaries. */
#include "reloq_core.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void check_rate(int rate, int reference_enabled)
{
    struct reloq_core_clock clock=reloq_core_clock_for_rate((float)rate);
    if(reference_enabled)clock.voice_reference_rate=11025;
    struct reloq_core_voiced_formants f={0};
    f.highest_formant=2;
    f.oral[0]=(struct reloq_core_formant){500,80};
    f.oral[1]=(struct reloq_core_formant){1600,100};
    f.nasal_pole=(struct reloq_core_formant){300,100};
    f.nasal_zero=(struct reloq_core_formant){400,120};
    f.tracheal_pole=(struct reloq_core_formant){700,100};
    f.tracheal_zero=(struct reloq_core_formant){800,120};
    struct reloq_core_voiced_bank bank={0};
    assert(!reloq_core_voiced_bank_configure(&bank,&clock,&f,1,0));
    float warm[80];
    for(unsigned i=0;i<80;++i)warm[i]=sinf((float)i*.31f);
    assert(!reloq_core_voiced_bank_process(&bank,warm,80));
    f.oral[0].frequency_hz=750;
    f.nasal_zero.frequency_hz=650;
    assert(!reloq_core_voiced_bank_configure(&bank,&clock,&f,1,1));
    unsigned total=reference_enabled?(unsigned)lroundf(3.0f*rate/11025.0f):3;
    if(total<3)total=3;
    assert(bank.oral[0].ramp_remaining==3);
    assert(bank.oral[0].ramp_samples_total==(total>3?total:0));
    assert(bank.nasal_zero.ramp_samples_total==(total>3?total:0));

    struct reloq_core_voiced_bank chunked=bank;
    float full[64],pieces[64];
    for(unsigned i=0;i<64;++i)full[i]=pieces[i]=cosf((float)i*.17f);
    assert(!reloq_core_voiced_bank_process(&bank,full,64));
    assert(!reloq_core_voiced_bank_process(&chunked,pieces,total-1));
    assert(chunked.oral[0].ramp_remaining>0);
    assert(!reloq_core_voiced_bank_process(&chunked,pieces+total-1,1));
    assert(chunked.oral[0].ramp_remaining==0);
    assert(!reloq_core_voiced_bank_process(&chunked,pieces+total,64-total));
    for(unsigned i=0;i<64;++i) {
        assert(isfinite(full[i]) && isfinite(pieces[i]));
        assert(full[i]==pieces[i]);
    }
}

int main(void)
{
    const int rates[]={8000,11025,16000,22050,32000,44100,48000};
    for(unsigned i=0;i<sizeof(rates)/sizeof(rates[0]);++i)
        check_rate(rates[i],rates[i]>=16000);
    /* Unprofiled numeric synthesis keeps its established sample contract. */
    check_rate(48000,0);
    puts("Tract transitions: reference-time ramps and chunk continuity passed");
    return 0;
}
