#include "pitch.h"
#include "pitch_scale.h"
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
    /* Dialog scale: 50 is Reed's native 65, both ends stay at 0 and 100. */
    const unsigned levels[]={0,25,50,75,100},natives[]={0,33,65,83,100};
    for(unsigned i=0;i<5;++i){assert(sapi_pitch_native_value(levels[i])==natives[i]);
        assert(sapi_pitch_level(natives[i])==levels[i]);}
    for(unsigned level=0;level<100;++level)
        assert(sapi_pitch_native_value(level)<=sapi_pitch_native_value(level+1));
    for(unsigned native=65;native<=100;++native)
        assert(sapi_pitch_native_value(sapi_pitch_level(native))==native);
    const unsigned presets[]={65,81,93,56,69,89,68,61};
    for(unsigned i=0;i<8;++i)assert(sapi_pitch_native_value(sapi_pitch_level(presets[i]))==presets[i]);
    assert(sapi_pitch_native_value(500)==100&&sapi_pitch_level(500)==100);
    puts("SAPI pitch offsets match the original search for 40,703 combinations");
    puts("SAPI dialog pitch scale: 50 is native 65, every preset round-trips");
    return 0;
}
