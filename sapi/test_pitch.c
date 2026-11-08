#include "pitch.h"
#include <assert.h>
#include <stdio.h>

static int previous(int base,long adjustment)
{
    if(!adjustment)return base;
    double target=reloq_voice_pitch_f0_scale(base)*pow(2.0,adjustment/10.0);
    int best=0;double error=HUGE_VAL;
    for(int i=0;i<=100;++i){double d=fabs(log(reloq_voice_pitch_f0_scale(i)/target));
        if(d<error){error=d;best=i;}}
    return best;
}
int main(void)
{
    for(int base=0;base<=100;++base)
        for(long adjustment=-200;adjustment<=200;++adjustment)
            assert(sapi_native_pitch(base,adjustment)==previous(base,adjustment));
    const long extremes[]={-1000000,1000000};
    for(int base=0;base<=100;++base)for(unsigned i=0;i<2;++i)
        assert(sapi_native_pitch(base,extremes[i])==previous(base,extremes[i]));
    assert(sapi_hertz_pitch(150,0)==150);
    assert(sapi_hertz_pitch(150,10)==300);
    assert(sapi_hertz_pitch(150,-10)==75);
    assert(sapi_hertz_pitch(150,1000000)==24000);
    assert(sapi_hertz_pitch(150,-1000000)==1);
    puts("SAPI pitch offsets match the original search for 40,703 combinations");
    return 0;
}
