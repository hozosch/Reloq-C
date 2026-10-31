#include "format.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    static const uint32_t rates[]={8000,11025,16000,22050,32000,44100,48000};
    unsigned checks=0;
    for(unsigned i=0;i<7;++i){
        unsigned advertised=0;
        for(unsigned j=0;j<7;++j){
            assert(sapi_format_offer(&advertised,rates[i],rates[j])==rates[i]);++checks;
            for(unsigned k=0;k<7;++k){assert(sapi_format_accepts(advertised,rates[k])==(k==i));++checks;}
        }
        assert(sapi_format_offer(&advertised,rates[i],0)==rates[i]);++checks;
        assert(sapi_format_offer(&advertised,rates[i],96000)==rates[i]);++checks;
        assert(!sapi_format_accepts(advertised,24000));++checks;
        sapi_format_offer(&advertised,rates[(i+1)%7],48000);
        assert(sapi_format_accepts(advertised,rates[i])&&sapi_format_accepts(advertised,rates[(i+1)%7]));++checks;
    }
    printf("{\"format_checks\":%u,\"native_target_pairs\":49,\"target_does_not_change_native_rate\":true,\"only_advertised_formats_accepted\":true}\n",checks);
    return 0;
}
