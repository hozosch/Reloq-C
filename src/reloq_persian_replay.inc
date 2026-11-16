/* Included in the ECI host. It shares the handle's configured
 * text session, EQ, callback, output device and cancellation epoch. */
#include "reloq_persian_frame_internal.h"

#if defined(RELOQ_FA_PROBE_EXPORT) && defined(_WIN32)
#define RELOQ_FA_EXPORT __declspec(dllexport)
#elif defined(RELOQ_FA_PROBE_EXPORT) && (defined(__GNUC__) || defined(__clang__))
#define RELOQ_FA_EXPORT __attribute__((visibility("default")))
#else
#define RELOQ_FA_EXPORT
#endif

struct reloq_fa_replay_delivery {
    struct pcm_bridge bridge;
    int16_t pending[1024];
    size_t used,limit,next_limit;
};

static int reloq_fa_replay_flush(struct reloq_fa_replay_delivery *delivery)
{
    if(!delivery->used)return 0;
    int status=send_pcm(&delivery->bridge,delivery->pending,delivery->used);
    delivery->used=0;delivery->limit=delivery->next_limit;
    return status;
}

static int reloq_fa_replay_pcm(void *context,const int16_t *pcm,size_t count)
{
    struct reloq_fa_replay_delivery *delivery=context;
    while(count) {
        size_t n=delivery->limit-delivery->used;if(n>count)n=count;
        memcpy(delivery->pending+delivery->used,pcm,n*sizeof *pcm);
        delivery->used+=n;pcm+=n;count-=n;
        if(delivery->used==delivery->limit && reloq_fa_replay_flush(delivery))return -1;
    }
    return 0;
}

RELOQ_FA_EXPORT uint32_t reloq_fa_replay_epoch(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;
    return e?epoch_get(&e->stop_epoch):0;
}

/* Incremental replay retains precisely the same synthesis session and PCM
 * buffer between chunks. It is used only after the initial portion of a
 * sentence is known not to be changed by later boundary planning. */
struct reloq_fa_replay_stream {
    struct reloq_eci_engine *engine;
    struct reloq_fa_replay_delivery delivery;
    unsigned epoch;
    int result,head,gender;
};

RELOQ_FA_EXPORT int reloq_fa_replay_begin(ECIHand handle,uint32_t expected_epoch,void **out)
{
    struct reloq_eci_engine *e=handle;
    if(!out)return RELOQ_ECI_INVALID;
    *out=NULL;
    if(!e || e->voice_mode!=RELOQ_ECI_VOICE_GENERIC)return RELOQ_ECI_INVALID;
    if(epoch_get(&e->stop_epoch)!=expected_epoch)return RELOQ_ECI_CANCELLED;
    struct reloq_fa_replay_stream *s=calloc(1,sizeof *s);
    if(!s)return RELOQ_ECI_MEMORY;
    if(!busy_begin(&e->busy)){free(s);return RELOQ_ECI_BUSY;}
    int result=RELOQ_ECI_OK;
    if(!e->callback || !e->output || e->output_samples<1) {result=RELOQ_ECI_OUTPUT;goto fail;}
    (void)reloq_session_high_rate_profile(e->text_session,3);
    (void)reloq_session_mellow_mode(e->text_session,e->mellow_mode);
    (void)reloq_session_text_loudness(e->text_session,1);
    lock_session(&e->session_lock);
    int reset=reloq_session_reset(e->text_session);
    if(epoch_get(&e->stop_epoch)!=expected_epoch)reloq_session_cancel(e->text_session);
    unlock_session(&e->session_lock);
    if(reset!=RELOQ_SESSION_OK){result=RELOQ_ECI_RENDER;goto fail;}
    if(reloq_session_cancelled(e->text_session)){result=RELOQ_ECI_CANCELLED;goto fail;}
    s->engine=e;s->epoch=expected_epoch;s->head=e->head_size;s->gender=e->gender;
    s->delivery=(struct reloq_fa_replay_delivery){
        .bridge={.engine=e,.session=e->text_session,.inline_volume=1},
        .limit=(size_t)sample_hz(e->sample_rate_code)/100,
        .next_limit=(size_t)sample_hz(e->sample_rate_code)/50};
    *out=s;return RELOQ_ECI_OK;
fail:
    eci_error(e,result);e->last_synthesis_error=result;
    busy_end(&e->busy);free(s);return result;
}

RELOQ_FA_EXPORT int reloq_fa_replay_append(void *stream,const struct reloq_fa_frame *frames,
    size_t frame_count,const struct reloq_fa_frame_mark *marks,size_t mark_count)
{
    struct reloq_fa_replay_stream *s=stream;
    if(!s || (frame_count&&!frames) || (mark_count&&!marks) || frame_count>120000 || mark_count>120000)
        return RELOQ_ECI_INVALID;
    if(s->result)return s->result;
    for(size_t i=0;i<mark_count;++i)
        if(marks[i].frame>frame_count || marks[i].value<0 ||
           (i && marks[i].frame<marks[i-1].frame))return s->result=RELOQ_ECI_INVALID;
    struct reloq_eci_engine *e=s->engine;
    size_t mark=0;
    for(size_t i=0;i<=frame_count;++i) {
        if(reloq_session_cancelled(e->text_session) || epoch_get(&e->stop_epoch)!=s->epoch)
            return s->result=RELOQ_ECI_CANCELLED;
        while(mark<mark_count && marks[mark].frame==i) {
            if(reloq_fa_replay_flush(&s->delivery))return s->result=RELOQ_ECI_CALLBACK;
            int result=send_index(e,marks[mark++].value);
            if(result!=RELOQ_ECI_OK)return s->result=result;
        }
        if(i==frame_count)break;
        const struct reloq_fa_frame *row=frames+i;
        if(row->head<0 || row->head>100 || (row->gender!=0 && row->gender!=1) ||
           row->formants<1 || row->formants>8 || !isfinite(row->gain) || row->gain<0 || row->gain>16)
            return s->result=RELOQ_ECI_INVALID;
        if(s->head!=row->head || s->gender!=row->gender) {
            s->head=row->head;s->gender=row->gender;
            long source=s->gender?(s->head<30?5:3):4;
            if(sample_hz(e->sample_rate_code)==8000&&s->head<=(s->gender?40:30)&&
               !(s->gender&&s->head<30))source=7;
            unsigned reference=sample_hz(e->sample_rate_code)>=16000?(s->head<=(s->gender?40:30)?4u:5u):0;
            if(reloq_session_source_levels(e->text_session,source,s->gender?-3:-5,6,
                                           s->gender?-7:0,reference))return s->result=RELOQ_ECI_INVALID;
        }
        if(reloq_session_linear_gain(e->text_session,row->gain))return s->result=RELOQ_ECI_INVALID;
        struct reloq_core_voiced_frame frame;
        if(reloq_frame_adapter_map(row->controls,row->formants,&frame))return s->result=RELOQ_ECI_INVALID;
        frame.noise.s_frication=row->s_frication;
        frame.noise.limit_frication_extension=row->limit_frication;
        int status=reloq_session_render_frames(e->text_session,&frame,1,
                                               reloq_fa_replay_pcm,&s->delivery,NULL);
        if(status)return s->result=status==RELOQ_SESSION_CANCELLED?RELOQ_ECI_CANCELLED:
                     s->delivery.bridge.callback_refused?RELOQ_ECI_CALLBACK:RELOQ_ECI_RENDER;
    }
    return RELOQ_ECI_OK;
}

RELOQ_FA_EXPORT int reloq_fa_replay_end(void *stream,int flush)
{
    struct reloq_fa_replay_stream *s=stream;
    if(!s)return RELOQ_ECI_INVALID;
    struct reloq_eci_engine *e=s->engine;
    int result=s->result;
    if(result==RELOQ_ECI_OK && flush && reloq_fa_replay_flush(&s->delivery))result=RELOQ_ECI_CALLBACK;
    if(reloq_session_cancelled(e->text_session) || epoch_get(&e->stop_epoch)!=s->epoch)
        result=RELOQ_ECI_CANCELLED;
    eci_error(e,result);e->last_synthesis_error=result;busy_end(&e->busy);free(s);
    return result;
}

RELOQ_FA_EXPORT int reloq_fa_replay_frames(ECIHand handle,uint32_t expected_epoch,
    const struct reloq_fa_frame *frames,size_t frame_count,
    const struct reloq_fa_frame_mark *marks,size_t mark_count)
{
    struct reloq_eci_engine *e=handle;
    if(!e || (frame_count&&!frames) || (mark_count&&!marks) || frame_count>120000 ||
       mark_count>120000 || e->voice_mode!=RELOQ_ECI_VOICE_GENERIC)return RELOQ_ECI_INVALID;
    for(size_t i=0;i<mark_count;++i)
        if(marks[i].frame>frame_count || marks[i].value<0 ||
           (i && marks[i].frame<marks[i-1].frame))return RELOQ_ECI_INVALID;
    unsigned before=epoch_get(&e->stop_epoch);
    if(before!=expected_epoch)return RELOQ_ECI_CANCELLED;
    if(!busy_begin(&e->busy))return RELOQ_ECI_BUSY;
    int result=RELOQ_ECI_OK;
    if((frame_count || mark_count) && (!e->callback || !e->output || e->output_samples<1)) {
        result=RELOQ_ECI_OUTPUT;goto done;
    }
    (void)reloq_session_high_rate_profile(e->text_session,3);
    (void)reloq_session_mellow_mode(e->text_session,e->mellow_mode);
    (void)reloq_session_text_loudness(e->text_session,1);
    lock_session(&e->session_lock);
    int reset=reloq_session_reset(e->text_session);
    if(epoch_get(&e->stop_epoch)!=before)reloq_session_cancel(e->text_session);
    unlock_session(&e->session_lock);
    if(reset!=RELOQ_SESSION_OK){result=RELOQ_ECI_RENDER;goto done;}
    struct reloq_fa_replay_delivery delivery={
        .bridge={.engine=e,.session=e->text_session,.inline_volume=1},
        .limit=(size_t)sample_hz(e->sample_rate_code)/100,
        .next_limit=(size_t)sample_hz(e->sample_rate_code)/50};
    int head=e->head_size,gender=e->gender;
    size_t mark=0;
    for(size_t i=0;i<=frame_count;++i) {
        if(reloq_session_cancelled(e->text_session)){result=RELOQ_ECI_CANCELLED;break;}
        while(mark<mark_count && marks[mark].frame==i) {
            if(reloq_fa_replay_flush(&delivery)){result=RELOQ_ECI_CALLBACK;goto done;}
            result=send_index(e,marks[mark++].value);
            if(result!=RELOQ_ECI_OK)goto done;
        }
        if(i==frame_count)break;
        const struct reloq_fa_frame *row=frames+i;
        if(row->head<0 || row->head>100 || (row->gender!=0 && row->gender!=1) ||
           row->formants<1 || row->formants>8 || !isfinite(row->gain) ||
           row->gain<0 || row->gain>16) {result=RELOQ_ECI_INVALID;break;}
        if(head!=row->head || gender!=row->gender) {
            head=row->head;gender=row->gender;
            long source=gender?(head<30?5:3):4;
            if(sample_hz(e->sample_rate_code)==8000&&head<=(gender?40:30)&&!(gender&&head<30))source=7;
            unsigned reference=sample_hz(e->sample_rate_code)>=16000?(head<=(gender?40:30)?4u:5u):0;
            if(reloq_session_source_levels(e->text_session,source,gender?-3:-5,6,gender?-7:0,reference)) {
                result=RELOQ_ECI_INVALID;break;
            }
        }
        if(reloq_session_linear_gain(e->text_session,row->gain)) {result=RELOQ_ECI_INVALID;break;}
        struct reloq_core_voiced_frame frame;
        if(reloq_frame_adapter_map(row->controls,row->formants,&frame)) {result=RELOQ_ECI_INVALID;break;}
        frame.noise.s_frication=row->s_frication;
        frame.noise.limit_frication_extension=row->limit_frication;
        int status=reloq_session_render_frames(e->text_session,&frame,1,
                                               reloq_fa_replay_pcm,&delivery,NULL);
        if(status) {
            result=status==RELOQ_SESSION_CANCELLED?RELOQ_ECI_CANCELLED:
                   delivery.bridge.callback_refused?RELOQ_ECI_CALLBACK:RELOQ_ECI_RENDER;
            break;
        }
    }
    if(result==RELOQ_ECI_OK && reloq_fa_replay_flush(&delivery))result=RELOQ_ECI_CALLBACK;
    if(reloq_session_cancelled(e->text_session) || epoch_get(&e->stop_epoch)!=before)
        result=RELOQ_ECI_CANCELLED;
done:
    eci_error(e,result);e->last_synthesis_error=result;
    busy_end(&e->busy);
    return result;
}
