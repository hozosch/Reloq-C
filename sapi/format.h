#ifndef RELOQ_SAPI_FORMAT_H
#define RELOQ_SAPI_FORMAT_H
#include <stdint.h>
static unsigned sapi_format_bit(uint32_t rate)
{
    static const uint32_t rates[]={8000,11025,16000,22050,32000,44100,48000};
    for(unsigned i=0;i<sizeof rates/sizeof *rates;++i)if(rates[i]==rate)return 1u<<i;
    return 0;
}
/* Advertise the selected synthesis clock. The target belongs to SAPI's
 * downstream converter and must never choose another acoustic profile. */
static uint32_t sapi_format_offer(unsigned *advertised,uint32_t native,uint32_t target)
{(void)target;*advertised|=sapi_format_bit(native);return native;}
static int sapi_format_accepts(unsigned advertised,uint32_t rate)
{return (advertised&sapi_format_bit(rate))!=0;}
#endif
