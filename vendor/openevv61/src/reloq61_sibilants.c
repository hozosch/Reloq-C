/* Native AF endpoint bindings. No S/Z gain is added. Keep each language's
 * source stream and boundary rules, without a phone lookup per endpoint. */
#include "reloq61_segment_rules.h"

#define NATIVE_AF(name,stream,end) \
int32_t name(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5) \
{ \
    static const struct reloq61_segment_layout layout={ \
        .af_stream=stream,.phone_boundary=720,.phone_end=end}; \
    (void)p0; \
    return reloq61_segment_frication(state,&layout,p1,p2,p3,p4,p5); \
}

NATIVE_AF(reloq_engb61_doAF,1042,1152)
NATIVE_AF(reloq_esmx61_doAF,946,1056)
NATIVE_AF(reloq_frfr61_doAF,1042,1152)
NATIVE_AF(reloq_frca61_doAF,1042,1152)
NATIVE_AF(reloq_itit61_doAF,952,1064)
NATIVE_AF(reloq_ptbr_doAF,946,1056)
NATIVE_AF(reloq_fifi_doAF,946,1056)
NATIVE_AF(reloq_zhcn61_doAF,954,1064)
NATIVE_AF(reloq_kokr61_doAF,954,1064)

#undef NATIVE_AF
