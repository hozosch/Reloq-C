#ifndef RELOQ_SAPI_SPEED_H
#define RELOQ_SAPI_SPEED_H
#include <stdint.h>
/* SAPI5IBMTTS uses ECI 60 + 10*(SAPI rate + XML adjustment), bounded by
 * ECI's 0..250 range. Clamp after combining controls, not at SAPI +10. */
static int sapi_native_speed(int32_t rate,int32_t adjustment,int boost)
{
    int64_t native=60+10*((int64_t)rate+adjustment);
    if(native<0)native=0;if(native>250)native=250;
    if(boost){native=(8*native+2)/5;if(native>250)native=250;}
    return (int)native;
}
#endif
