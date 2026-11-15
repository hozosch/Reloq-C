/* Private listening experiment: render already planned control frames with
 * one continuous Klatt session. Not an ECI extension or production API. */
#include "reloq_frame_adapter.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_german_voice_model.h"
#include "reloq_session.h"
#include "reloq_voice_controls.h"

#include <stdint.h>
#include <stddef.h>

__attribute__((visibility("default")))
int reloq_fa_probe_replay(const float *controls,size_t count,int rate,int gender,int head,
                          reloq_session_pcm_sink sink,void *context)
{
    if(!controls || !count || !sink || rate<8000 || rate>48000 ||
       (gender!=0 && gender!=1) || head<0 || head>100)return -1;
    float defaults[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config cfg;
    if(reloq_german_voice_defaults(defaults,fields,&cfg))return -1;
    cfg.sample_rate=(float)rate;
    if(rate>=16000){cfg.glottal_reference_rate=11025;cfg.frication_reference_rate=16000;
                    cfg.reference_formants=head<=(gender?40:30)?4:5;}
    if(gender){cfg.source_offset=head<30?5:3;cfg.av_trim=-3;cfg.af_trim=-7;}
    if(rate==8000 && head<=(gender?40:30) && !(gender && head<30))cfg.source_offset=7;
    cfg.max_frame_samples=4096;
    struct reloq_session *session=reloq_session_create(&cfg);
    if(!session)return -1;
    int status=reloq_session_high_rate_profile(session,3) ||
               reloq_session_text_loudness(session,1) ||
               reloq_session_mellow_mode(session,0);
    /* The real text path resets after selecting the rate profile. This
       initializes the wideband source state using that selected profile. */
    if(!status)status=reloq_session_reset(session);
    if(!status)status=reloq_session_linear_gain(session,(float)reloq_voice_volume_gain(100));
    struct reloq_german_voice_model model={head,0,0,gender,(float)rate};
    for(size_t i=0;!status && i<count;i++) {
        const float *c=controls+62*i;
        struct reloq_core_voiced_frame frame;
        if(reloq_frame_adapter_map(c,reloq_german_voice_model_formants(&model),&frame)) {status=-1;break;}
        frame.noise.s_frication=c[8]>0 && (c[38]>c[37]+10 || c[39]>c[37]+10);
        status=reloq_session_render_frames(session,&frame,1,sink,context,NULL);
    }
    reloq_session_destroy(session);
    return status;
}
