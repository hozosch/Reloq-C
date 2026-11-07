#ifndef RELOQ_SAPI_PITCH_SCALE_H
#define RELOQ_SAPI_PITCH_SCALE_H

/* The settings dialog shows pitch on the same 0..100 scale as the NVDA
 * slider and Android: 50 is native 65, the baseline of the default Reed
 * voice. 0..50 covers native 0..65 and 50..100 covers native 65..100.
 * Settings are stored as native values, as before. */
enum {SAPI_PITCH_MIDDLE=65};

static inline unsigned sapi_pitch_native_value(unsigned level)
{
    if(level>100)level=100;
    if(level<=50)return (level*SAPI_PITCH_MIDDLE+25)/50;
    return SAPI_PITCH_MIDDLE+((level-50)*(100-SAPI_PITCH_MIDDLE)+25)/50;
}

static inline unsigned sapi_pitch_level(unsigned native)
{
    if(native>100)native=100;
    if(native<=SAPI_PITCH_MIDDLE)return (native*50+SAPI_PITCH_MIDDLE/2)/SAPI_PITCH_MIDDLE;
    const unsigned span=100-SAPI_PITCH_MIDDLE;
    return 50+((native-SAPI_PITCH_MIDDLE)*50+span/2)/span;
}
#endif
