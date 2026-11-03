#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#define RELOQ_ECI_BUILD
#include "reloq_eci.h"

#include "reloq_cp1252.h"
#include "reloq_unicode.h"
#include "reloq_utf8.h"
#include "reloq_inline_tags.h"
#include "reloq_frame_adapter.h"
#include "reloq_rules61.h"
#include "reloq61.h"
#include "reloq_german_best_effort_text.h"
#include "reloq_german_generic_voice.h"
#include "reloq_german_lexicon_file.h"
#include "reloq_german_voice.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_german_text.h"
#include "reloq_session.h"
#include "reloq_english_voice.h"
#include "reloq_voice_controls.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#include <windows.h>
#include <wchar.h>
#elif !defined(__ANDROID__)
#include <iconv.h>
#endif

#if defined(_MSC_VER)
#include <intrin.h>
typedef volatile long reloq_eci_atomic;
static void epoch_init(reloq_eci_atomic *p) { *p = 0; }
static unsigned int epoch_get(const reloq_eci_atomic *p)
{
    return (unsigned int)_InterlockedCompareExchange((volatile long *)p, 0, 0);
}
static void epoch_inc(reloq_eci_atomic *p) { (void)_InterlockedIncrement(p); }
static unsigned int epoch_exchange(reloq_eci_atomic *p,unsigned int value)
{return (unsigned int)_InterlockedExchange(p,(long)value);}
static int busy_begin(reloq_eci_atomic *p)
{
    return _InterlockedCompareExchange(p, 1, 0) == 0;
}
static void busy_end(reloq_eci_atomic *p) { (void)_InterlockedExchange(p, 0); }
static void lock_session(reloq_eci_atomic *p)
{
    while (_InterlockedCompareExchange(p, 1, 0) != 0) { }
}
static void unlock_session(reloq_eci_atomic *p)
{
    (void)_InterlockedExchange(p, 0);
}
#else
#include <stdatomic.h>
typedef atomic_uint reloq_eci_atomic;
static void epoch_init(reloq_eci_atomic *p) { atomic_init(p, 0); }
static unsigned int epoch_get(const reloq_eci_atomic *p)
{
    return atomic_load_explicit(p, memory_order_acquire);
}
static void epoch_inc(reloq_eci_atomic *p)
{
    (void)atomic_fetch_add_explicit(p, 1, memory_order_release);
}
static unsigned int epoch_exchange(reloq_eci_atomic *p,unsigned int value)
{return atomic_exchange_explicit(p,value,memory_order_acq_rel);}
static int busy_begin(reloq_eci_atomic *p)
{
    unsigned int expected = 0;
    return atomic_compare_exchange_strong_explicit(
        p, &expected, 1, memory_order_acq_rel, memory_order_acquire);
}
static void busy_end(reloq_eci_atomic *p)
{
    atomic_store_explicit(p, 0, memory_order_release);
}
static void lock_session(reloq_eci_atomic *p)
{
    unsigned int expected;
    do {
        expected = 0;
    } while (!atomic_compare_exchange_weak_explicit(
        p, &expected, 1, memory_order_acquire, memory_order_relaxed));
}
static void unlock_session(reloq_eci_atomic *p)
{
    atomic_store_explicit(p, 0, memory_order_release);
}
#endif

enum { CONTROL_FIELDS = 62, MAX_FRAME_SAMPLES = 4096 };
static const unsigned int eci_languages[]={0x10000,0x10001,0x20000,0x20001,
    0x30000,0x30001,0x40000,0x50000,0x60000,0x60001,0x70000,0x80000,
    0x90000,0xa0000,0x120000};
static unsigned dictionary_language_slot(int language)
{
    unsigned i=0;
    while(i<sizeof eci_languages/sizeof *eci_languages&&eci_languages[i]!=(unsigned)language)++i;
    return i;
}
_Static_assert(sizeof(short) == sizeof(int16_t), "ECI output needs 16-bit short");

struct queued_voice {
    int values[8],generic[3];
    unsigned active;
};
struct queued_item {
    unsigned char *utf8;
    size_t length;
    int index;
    int is_index;
    int pitch;
    int is_pitch;
    int unicode_input, character_mode;
    struct queued_voice voice;
};

struct index_mark {
    size_t offset;
    int value;
};

struct pitch_mark {
    size_t offset;
    int value;
};

struct connected_index_boundary {
    size_t word_end, next_word;
    size_t sample;
    int used;
};

struct reloq_eci_dictionary {
    struct reloq_eci_dictionary *next;
    unsigned char *volumes[4];size_t sizes[4];
    void *api_worker;
    int language,walk_active[3];
};

struct reloq_eci_engine {
    struct reloq_session *session;
    struct reloq_session *text_session;
    void *rule_worker;
    char *dictionary_path;
    struct reloq_german_lexicon_file lexicon;
    reloq_eci_atomic busy;
    reloq_eci_atomic stop_epoch;
    reloq_eci_atomic session_lock;
    reloq_eci_atomic last_index;
    reloq_eci_atomic paused;
    reloq_eci_atomic program_status,sdk_error;
    ECICallback callback;
    void *callback_data;
    short *output;
    int output_samples;
    int sample_rate_code;
    int synth_mode;
    int voice_mode;
    int generic_rate;
    int generic_pitch;
    int generic_volume;
    int native_speed;
    int native_pitch;
    int native_inflection;
    int native_volume;
    int native_speed_active;
    int native_pitch_active;
    int native_inflection_active;
    int native_volume_active;
    int input_type;
    int process_tags;
    int dictionary_disabled,phrase_prediction;
    int pause_mode;
    int head_size,roughness,breathiness,gender;
    int language,language_dialect,text_mode,real_world_units;
    int saved_voices[16][8];char voice_names[17][120];
    int real_voice_values[17][8];unsigned real_voice_flags[17];
    struct reloq_eci_dictionary *dictionaries,*active_dictionary;
    struct reloq_eci_dictionary *language_dictionaries[15];
    int high_rate_profile;
    int error;
    int last_synthesis_error;
    int failure_stage;
    int failure_utf8_offset;
    struct queued_item *queue;
    size_t queued, capacity;
    char *dictionary_key_result,*dictionary_value_result;
};

static struct queued_voice capture_voice(const struct reloq_eci_engine *e)
{
    return (struct queued_voice){
        {e->gender,e->head_size,e->native_pitch,e->native_inflection,
         e->roughness,e->breathiness,e->native_speed,e->native_volume},
        {e->generic_rate,e->generic_pitch,e->generic_volume},
        (unsigned)(e->native_speed_active|e->native_pitch_active<<1|
            e->native_inflection_active<<2|e->native_volume_active<<3)};
}
static void apply_voice(struct reloq_eci_engine *e,const struct queued_voice *v)
{
    e->gender=v->values[0];e->head_size=v->values[1];
    e->native_pitch=v->values[2];e->native_inflection=v->values[3];
    e->roughness=v->values[4];e->breathiness=v->values[5];
    e->native_speed=v->values[6];e->native_volume=v->values[7];
    e->generic_rate=v->generic[0];e->generic_pitch=v->generic[1];e->generic_volume=v->generic[2];
    e->native_speed_active=!!(v->active&1);e->native_pitch_active=!!(v->active&2);
    e->native_inflection_active=!!(v->active&4);e->native_volume_active=!!(v->active&8);
}

struct queued_control {size_t offset;struct queued_voice voice;};
/* Trusted parameter snapshots are inserted only after caller text admission.
 * They stay in one rule stream, so SetVoiceParam/AddText never adds a phrase
 * boundary or reinterprets a caller's disabled annotations. */
static int insert_voice_controls(unsigned char **text,size_t *length,
    struct queued_control *controls,size_t count,struct queued_voice initial,
    const size_t *source_map,struct index_mark *marks,size_t mark_count,
    struct pitch_mark *pitches,size_t pitch_count)
{
    static const char *const names[]={"vg","vh","vb","vf","vr","vy","vs","vv"};
    if(!count)return 0;
    if(count>(SIZE_MAX-*length-2)/80)return -1;
    unsigned char *next=malloc(*length+count*80+2);
    size_t *offsets=malloc((count+1)*sizeof *offsets),*added=malloc((count+1)*sizeof *added);
    if(!next||!offsets||!added){free(next);free(offsets);free(added);return -1;}
    size_t used=0,from=0;
    for(size_t i=0;i<count;++i) {
        size_t at=source_map?source_map[controls[i].offset]:controls[i].offset;
        memcpy(next+used,*text+from,at-from);used+=at-from;from=at;
        for(unsigned k=0;k<8;++k)if(initial.values[k]!=controls[i].voice.values[k]) {
            int n=snprintf((char *)next+used,80,"`%s%d",names[k],controls[i].voice.values[k]);
            if(n<0||n>=80){free(next);free(offsets);free(added);return -1;}
            used+=(size_t)n;
        }
        initial=controls[i].voice;offsets[i]=at;added[i]=used-at;
    }
    memcpy(next+used,*text+from,*length-from);used+=*length-from;next[used]=0;
    for(size_t i=0;i<mark_count;++i) {
        size_t extra=0;for(size_t j=0;j<count&&offsets[j]<=marks[i].offset;++j)extra=added[j];
        marks[i].offset+=extra;
    }
    for(size_t i=0;i<pitch_count;++i) {
        size_t extra=0;for(size_t j=0;j<count&&offsets[j]<=pitches[i].offset;++j)extra=added[j];
        pitches[i].offset+=extra;
    }
    free(*text);*text=next;*length=used;free(offsets);free(added);return 0;
}

static void eci_record_error(struct reloq_eci_engine *e,unsigned flag)
{
    if(!e)return;
    lock_session(&e->session_lock);
    epoch_exchange(&e->sdk_error,flag);
    epoch_exchange(&e->program_status,epoch_get(&e->program_status)|flag);
    unlock_session(&e->session_lock);
}
static int eci_refuse(struct reloq_eci_engine *e,int result)
{
    eci_record_error(e,e&&epoch_get(&e->busy)?0x800:0x80);return result;
}
static void eci_error(struct reloq_eci_engine *e,int result)
{
    e->error=result;
    if(result<0)eci_record_error(e,result==RELOQ_ECI_MEMORY?2:
        result==RELOQ_ECI_OUTPUT?0x20:result==RELOQ_ECI_BUSY?0x800:
        result==RELOQ_ECI_INVALID||result==RELOQ_ECI_UNSUPPORTED_TEXT?0x80:0x10);
}
static int notify_callback(struct reloq_eci_engine *e,int message,int value)
{
    unsigned before=epoch_get(&e->stop_epoch);
    for(;;) {
        if(epoch_get(&e->stop_epoch)!=before)return RELOQ_ECI_CANCELLED;
        while(epoch_get(&e->paused)) {
#ifdef _WIN32
            Sleep(2);
#else
            struct timespec pause={0,2000000L};(void)nanosleep(&pause,NULL);
#endif
            if(epoch_get(&e->stop_epoch)!=before)return RELOQ_ECI_CANCELLED;
        }
        int answer=e->callback(e,message,value,e->callback_data);
        /* The reference publishes an index after its callback, including
         * a not-processed response. Its retry therefore sees that index. */
        if(message==RELOQ_ECI_INDEX_REPLY)epoch_exchange(&e->last_index,(unsigned)value);
        if(epoch_get(&e->stop_epoch)!=before)return RELOQ_ECI_CANCELLED;
        if(answer!=eciDataNotProcessed)
            return answer==eciDataAbort?RELOQ_ECI_CALLBACK:RELOQ_ECI_OK;
#ifdef _WIN32
        Sleep(30);
#else
        struct timespec delay={0,30000000L};(void)nanosleep(&delay,NULL);
#endif
    }
}

static void initialize_eci_voices(struct reloq_eci_engine *);
static void clear_eci_dictionaries(struct reloq_eci_engine *);
static int switch_eci_language(struct reloq_eci_engine *,int);
static int reset_eci_defaults(struct reloq_eci_engine *);
static unsigned char *decode_eci_text(struct reloq_eci_engine *,const void *,size_t *,int *);

struct pcm_bridge {
    struct reloq_eci_engine *engine;
    struct reloq_session *session;
    int callback_refused;
    const struct index_mark *marks;
    size_t mark_count;
    size_t *next_mark;
    const struct connected_index_boundary *boundary;
    size_t span_samples;
    int inline_volume;
    size_t pcm_period, pcm_phase, pcm_used, pcm_early;
};

/* Original ECI offsets refer to the decoded UTF-8 phrase, but the native
 * German text scanner works in one-byte Windows-1252 characters. The queue
 * only splits between input bytes, so every source mark must fall on a UTF-8
 * scalar boundary. Convert all marks in one forward scan. */
static int generic_source_boundaries(
    const unsigned char *phrase, size_t length,
    const struct index_mark *marks, size_t count,
    struct reloq_german_best_effort_boundary *boundaries)
{
    size_t source = 0, encoded = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t target = marks[i].offset;
        if (target > length || (i && target < marks[i - 1].offset))
            return -1;
        while (source < target) {
            unsigned char lead = phrase[source];
            size_t width = lead < 0x80 ? 1 : lead < 0xe0 ? 2 :
                           lead < 0xf0 ? 3 : 4;
            if (width > target - source) return -1;
            source += width;
            ++encoded;
        }
        boundaries[i].encoded_offset = encoded;
        boundaries[i].annotation_offset = 0;
    }
    return 0;
}

static int send_pcm(void *context, const int16_t *pcm, size_t samples);
static int send_batched_pcm(void *context, const int16_t *pcm, size_t samples);
static int flush_pcm(struct pcm_bridge *bridge);
static int sample_hz(int code);

/* Keep the driver's first 40-ms reserve on the existing immediate path,
 * then batch rendered samples in its existing buffer on a 20-ms grid.
 * Index flushes do not reset that grid. Small buffers remain authoritative;
 * no extra buffer is allocated and no state survives the transaction. */
static size_t pcm_batch_period(const struct reloq_eci_engine *e)
{
    /* Native English already renders 20-ms blocks. German's authored
     * renderer emits millisecond frames; coalesce only that fine-grained
     * source, keeping the existing English callback boundaries intact. */
    if (e->language != RELOQ_ECI_GERMAN) return 0;
    size_t period = (size_t)(sample_hz(e->sample_rate_code) + 49) / 50;
    return period < (size_t)e->output_samples ? period : (size_t)e->output_samples;
}

static size_t pcm_startup_samples(const struct reloq_eci_engine *e)
{
    return (size_t)(sample_hz(e->sample_rate_code) + 24) / 25;
}

struct preflight_cancel {
    struct reloq_eci_engine *engine;
    unsigned int epoch;
};

static int preflight_cancelled(void *context)
{
    const struct preflight_cancel *check = context;
    return epoch_get(&check->engine->stop_epoch) != check->epoch;
}

static int diagnostic_offset(size_t offset, size_t original_length)
{
    if (offset > original_length) offset = original_length;
    return offset <= INT_MAX ? (int)offset : -1;
}

/* ECI text is CP1252, so each source byte becomes exactly one decoded UTF-8
 * scalar. The spelling adapter's encoded byte index is consequently a scalar
 * index into this phrase, even when German diacritics and quotation marks
 * occupy several UTF-8 bytes. */
static int diagnostic_encoded_offset(const unsigned char *phrase,
                                      size_t length, size_t encoded_offset,
                                      size_t original_length)
{
    size_t at = 0;
    for (size_t index = 0; at < length && index < encoded_offset; ++index) {
        unsigned char lead = phrase[at];
        size_t width = lead < 0x80 ? 1 :
                       lead < 0xe0 ? 2 : lead < 0xf0 ? 3 : 4;
        if (width > length - at) return -1;
        at += width;
    }
    return diagnostic_offset(at, original_length);
}

static int diagnostic_acoustic_start(const unsigned char *phrase,
                                     size_t length, size_t at,
                                     size_t original_length)
{
    if (at > length) at = length;
    while (at) {
        unsigned char c = phrase[at - 1];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              c >= 0x80)) break;
        --at;
    }
    return diagnostic_offset(at, original_length);
}

static void diagnose_voice_rejection(struct reloq_eci_engine *e,
                                     const unsigned char *phrase,
                                     size_t length, size_t original_length,
                                     size_t voice_bad,
                                     struct preflight_cancel *cancel)
{
    size_t needed = 0, text_bad = 0;
    int status = reloq_german_text_annotate_checked(
        phrase, length, NULL, 0, &needed, &text_bad, NULL,
        preflight_cancelled, cancel);
    if (status == RELOQ_DE_TEXT_UNCOVERED ||
        status == RELOQ_DE_TEXT_INVALID) {
        e->failure_stage = RELOQ_ECI_STAGE_TEXT;
        e->failure_utf8_offset = diagnostic_offset(text_bad, original_length);
    } else if (status == RELOQ_DE_TEXT_OK) {
        e->failure_stage = RELOQ_ECI_STAGE_ACOUSTIC;
        e->failure_utf8_offset = diagnostic_acoustic_start(
            phrase, length, voice_bad, original_length);
    }
}

static int send_index(struct reloq_eci_engine *e, int index)
{
    if (reloq_session_cancelled(e->text_session))
        return RELOQ_ECI_CANCELLED;
    int status=notify_callback(e,RELOQ_ECI_INDEX_REPLY,index);
    if (reloq_session_cancelled(e->text_session))
        return RELOQ_ECI_CANCELLED;
    return status;
}

static int send_indices_until(struct reloq_eci_engine *e,
                              const struct index_mark *marks, size_t count,
                              size_t *next, size_t boundary)
{
    while (*next < count && marks[*next].offset <= boundary) {
        int result = send_index(e, marks[*next].value);
        if (result != RELOQ_ECI_OK) return result;
        ++*next;
    }
    return RELOQ_ECI_OK;
}

static void clear_queue(struct reloq_eci_engine *e)
{
    for (size_t i = 0; i < e->queued; ++i) free(e->queue[i].utf8);
    free(e->queue);
    e->queue = NULL;
    e->queued = e->capacity = 0;
}

static int append_item(struct reloq_eci_engine *e, struct queued_item item)
{
    if (e->queued == e->capacity) {
        if (e->capacity > SIZE_MAX / 2 / sizeof *e->queue) return -1;
        size_t next = e->capacity ? e->capacity * 2 : 8;
        if (next > SIZE_MAX / sizeof *e->queue) return -1;
        void *items = realloc(e->queue, next * sizeof *e->queue);
        if (!items) return -1;
        e->queue = items;
        e->capacity = next;
    }
    e->queue[e->queued++] = item;
    return 0;
}

static int send_generic_index(void *context, size_t ordinal)
{
    struct pcm_bridge *bridge = context;
    if (ordinal >= bridge->mark_count) return -1;
    if (flush_pcm(bridge)) return -1;
    int status = send_index(bridge->engine, bridge->marks[ordinal].value);
    if (status == RELOQ_ECI_CALLBACK) bridge->callback_refused = 1;
    return status == RELOQ_ECI_OK ? 0 : -1;
}

/* Leading model annotations share the native model setup, without
 * changing the configured NVDA parameters or splitting the request. */
static void leading_models(const unsigned char *text,size_t length,
    struct reloq_german_generic_settings *settings)
{
    for(size_t at=0;at<length;) {
        if(text[at]==' '||text[at]=='\t'||text[at]=='\r'||text[at]=='\n'){++at;continue;}
        struct reloq_inline_tag tag=reloq_inline_tag_read(text+at,length-at,0);
        if(!tag.kind || tag.kind==RELOQ_TAG_SPR)break;
        switch(tag.kind) {
        case RELOQ_TAG_PITCH:settings->native_pitch_plus_one=tag.value+1;break;
        case RELOQ_TAG_GENDER:settings->gender=tag.value;break;
        case RELOQ_TAG_HEAD_SIZE:settings->head_size_plus_one=tag.value+1;break;
        case RELOQ_TAG_ROUGHNESS:settings->roughness=tag.value;break;
        case RELOQ_TAG_BREATHINESS:settings->breathiness=tag.value;break;
        default:break;
        }
        at+=tag.length;
    }
}

struct generic_stream_output {
    struct reloq_eci_engine *engine;
    const struct index_mark *marks;
    size_t mark_count;
    const struct pitch_mark *pitches;
    size_t pitch_count,next_index,next_pitch,next_boundary;
    struct preflight_cancel *cancel;
    struct pcm_bridge bridge;
    struct reloq_german_generic_settings settings;
    struct reloq_german_generic_continuation continuation;
    int status,native_volume;
};
static int generic_stream_part(void *context,const unsigned char *annotation,size_t length,
    const struct reloq_german_word_features *features,size_t feature_count,
    const struct reloq_german_best_effort_boundary *boundaries,size_t count,
    size_t first,int final)
{
    struct generic_stream_output *s=context;
    struct reloq_german_generic_index *indices=NULL;
    struct reloq_german_generic_pitch_mark *pitches=NULL;
    if(first!=s->next_boundary || first>s->mark_count+s->pitch_count ||
       count>s->mark_count+s->pitch_count-first ||
       count>SIZE_MAX/sizeof *indices || count>SIZE_MAX/sizeof *pitches) {
        s->status=RELOQ_ECI_INVALID;return 1;
    }
    if(count) {
        indices=malloc(count*sizeof *indices);
        pitches=malloc(count*sizeof *pitches);
        if(!indices||!pitches){free(indices);free(pitches);s->status=RELOQ_ECI_MEMORY;return 1;}
    }
    size_t ni=0,np=0,first_index=s->next_index;
    for(size_t i=0;i<count;++i) {
        int is_index=s->next_pitch==s->pitch_count ||
            (s->next_index<s->mark_count&&s->marks[s->next_index].offset<=s->pitches[s->next_pitch].offset);
        if(is_index){indices[ni++].annotation_offset=boundaries[i].annotation_offset;++s->next_index;}
        else pitches[np++]=(struct reloq_german_generic_pitch_mark){
            boundaries[i].annotation_offset,s->pitches[s->next_pitch++].value};
    }
    s->next_boundary+=count;
    s->bridge.marks=ni?s->marks+first_index:NULL;s->bridge.mark_count=ni;
    struct reloq_german_utterance_info info={0};
    int rendered=reloq_german_generic_render_text_part(annotation,length,&s->settings,
        features,feature_count,indices,ni,pitches,np,s->engine->text_session,
        send_batched_pcm,send_generic_index,&s->bridge,&info,s->native_volume,
        &s->continuation,final);
    free(indices);free(pitches);
    s->status=rendered==RELOQ_DE_UTTERANCE_CANCELLED ||
        reloq_session_cancelled(s->engine->text_session) || preflight_cancelled(s->cancel)?RELOQ_ECI_CANCELLED:
        rendered==RELOQ_DE_UTTERANCE_OK?RELOQ_ECI_OK:
        rendered==RELOQ_DE_UTTERANCE_MEMORY?RELOQ_ECI_MEMORY:
        rendered==RELOQ_DE_UTTERANCE_UNCOVERED?RELOQ_ECI_UNSUPPORTED_TEXT:
        rendered==RELOQ_DE_UTTERANCE_SINK && s->bridge.callback_refused?RELOQ_ECI_CALLBACK:RELOQ_ECI_RENDER;
    return s->status!=RELOQ_ECI_OK;
}

/* All AddText fragments share one ECI transaction and one acoustic clock.
 * Complete admitted German phrases can begin rendering during analysis;
 * indices are delivered at PCM word boundaries, not during preflight. */
static int synthesize_generic(struct reloq_eci_engine *e,
                              const unsigned char *phrase, size_t length,
                              size_t original_length,
                              const struct index_mark *marks,
                              size_t mark_count,
                              const struct pitch_mark *pitches,
                              size_t pitch_count, unsigned int epoch,int admitted)
{
    struct preflight_cancel cancel_check = {e, epoch};
    if(e->language!=RELOQ_ECI_GERMAN) {
        struct reloq_german_generic_index *english_indices=NULL;
        struct reloq_german_generic_pitch_mark *english_pitch=NULL;
        if(mark_count) {
            if(mark_count>SIZE_MAX/sizeof *english_indices)return RELOQ_ECI_MEMORY;
            english_indices=malloc(mark_count*sizeof *english_indices);
            if(!english_indices)return RELOQ_ECI_MEMORY;
            for(size_t i=0;i<mark_count;++i)english_indices[i].annotation_offset=marks[i].offset;
        }
        if(pitch_count) {
            if(pitch_count>SIZE_MAX/sizeof *english_pitch){free(english_indices);return RELOQ_ECI_MEMORY;}
            english_pitch=malloc(pitch_count*sizeof *english_pitch);
            if(!english_pitch){free(english_indices);return RELOQ_ECI_MEMORY;}
            for(size_t i=0;i<pitch_count;++i)english_pitch[i]=(struct reloq_german_generic_pitch_mark){pitches[i].offset,pitches[i].value};
        }
        lock_session(&e->session_lock);
        int reset=reloq_session_reset(e->text_session);
        if(preflight_cancelled(&cancel_check))reloq_session_cancel(e->text_session);
        unlock_session(&e->session_lock);
        int frame_volume=e->process_tags||(e->native_volume_active&&reloq_session_float_volume(e->text_session));
        struct pcm_bridge bridge={.engine=e,.session=e->text_session,.marks=marks,.mark_count=mark_count,.inline_volume=frame_volume};
        struct reloq_german_generic_settings settings={e->generic_rate,e->generic_pitch,
            (e->native_volume_active||e->process_tags)?100:e->generic_volume,e->native_speed_active?(e->native_speed?e->native_speed:-1):0,
            e->native_pitch_active?e->native_pitch+1:0,e->native_inflection_active?e->native_inflection+1:0,
            e->pause_mode,e->head_size+1,e->roughness,e->breathiness,e->gender};
        if(e->process_tags)leading_models(phrase,length,&settings);
        if(settings.gender!=e->gender || settings.head_size_plus_one!=e->head_size+1) {
            int head=settings.head_size_plus_one-1,gender=settings.gender;
            long source=gender?(head<30?5:3):4;
            if(e->sample_rate_code==0 && head<=(gender?40:30) && !(gender && head<30))source=7;
            unsigned reference=sample_hz(e->sample_rate_code)>=16000?(head<=(gender?40:30)?4u:5u):0;
            if(reloq_session_source_levels(e->text_session,source,gender?-3:-5,6,gender?-7:0,reference))reset=1;
        }
        int rendered=reset?RELOQ_DE_UTTERANCE_INVALID:reloq_language_rules_render(e->language,phrase,length,e->process_tags||admitted,
            frame_volume?(e->native_volume_active?e->native_volume:e->generic_volume):100,
            &settings,english_indices,mark_count,english_pitch,pitch_count,e->text_session,
            send_pcm,send_generic_index,&bridge);
        free(english_indices);free(english_pitch);
        if(preflight_cancelled(&cancel_check)||reloq_session_cancelled(e->text_session))return RELOQ_ECI_CANCELLED;
        if(rendered==RELOQ_DE_UTTERANCE_OK)return RELOQ_ECI_OK;
        e->failure_stage=RELOQ_ECI_STAGE_ACOUSTIC;e->failure_utf8_offset=-1;
        return rendered==RELOQ_DE_UTTERANCE_MEMORY?RELOQ_ECI_MEMORY:
            bridge.callback_refused?RELOQ_ECI_CALLBACK:RELOQ_ECI_UNSUPPORTED_TEXT;
    }
    struct reloq_german_best_effort_boundary *boundaries = NULL;
    struct reloq_german_generic_index *indices = NULL;
    struct reloq_german_generic_pitch_mark *pitch_indices = NULL;
    struct index_mark *source_marks = NULL;
    if (pitch_count > SIZE_MAX - mark_count) return RELOQ_ECI_MEMORY;
    size_t boundary_count = mark_count + pitch_count;
    if (boundary_count) {
        if (boundary_count > SIZE_MAX / sizeof *boundaries ||
            boundary_count > SIZE_MAX / sizeof *source_marks ||
            mark_count > SIZE_MAX / sizeof *indices ||
            pitch_count > SIZE_MAX / sizeof *pitch_indices)
            return RELOQ_ECI_MEMORY;
        boundaries = malloc(boundary_count * sizeof *boundaries);
        source_marks = malloc(boundary_count * sizeof *source_marks);
        indices = malloc(mark_count * sizeof *indices);
        pitch_indices = malloc(pitch_count * sizeof *pitch_indices);
        if (!boundaries || !source_marks ||
            (mark_count && !indices) || (pitch_count && !pitch_indices)) {
            free(boundaries); free(source_marks);
            free(indices); free(pitch_indices);
            return RELOQ_ECI_MEMORY;
        }
        size_t next_index = 0, next_pitch = 0;
        for (size_t b = 0; b < boundary_count; ++b) {
            if (next_pitch == pitch_count ||
                (next_index < mark_count &&
                 marks[next_index].offset <= pitches[next_pitch].offset))
                source_marks[b] = marks[next_index++];
            else
                source_marks[b] = (struct index_mark){
                    pitches[next_pitch++].offset, 0};
        }
    }

    unsigned char *annotation = NULL;
    unsigned char *ordinary_text = NULL;
    struct reloq_german_word_features *features=NULL;
    size_t feature_count=0;
    size_t annotation_length = 0, guessed = 0, bad_offset = 0;
    int phonetic_input = 0;
    size_t first = 0;
    while (first < length && (phrase[first] == ' ' || phrase[first] == '\t' ||
                              phrase[first] == '\r' || phrase[first] == '\n'))
        ++first;
    /* Only a complete SPR opener opts into the phonetic path. A bare
     * backtick at the start of ordinary screen-reader text must not turn
     * the whole utterance into an ECI annotation. In ordinary text it is
     * punctuation; map it to the scanner's safe apostrophe token before
     * annotation, without allowing arbitrary ECI controls through. */
    phonetic_input = !admitted && !e->process_tags && first < length && length - first >= 2 &&
                     phrase[first] == '`' && phrase[first + 1] == '[';
    int result = RELOQ_ECI_OK;
#ifndef RELOQ_GERMAN_STREAMING
#define RELOQ_GERMAN_STREAMING 1
#endif
    /* A diagnostic opt-out keeps the complete admission path available
     * for byte-exact comparisons and client-side latency investigations. */
    const char *stream_option=getenv("RELOQ_GERMAN_STREAMING");
    int can_stream=RELOQ_GERMAN_STREAMING && !phonetic_input && length>=1024 &&
        (!stream_option||strcmp(stream_option,"0"));
    struct generic_stream_output streaming={.engine=e,.marks=marks,.mark_count=mark_count,
        .pitches=pitches,.pitch_count=pitch_count,
        .cancel=&cancel_check,.status=RELOQ_ECI_OK,
        .native_volume=(e->process_tags||(e->native_volume_active&&reloq_session_float_volume(e->text_session)))?(e->native_volume_active?e->native_volume:e->generic_volume):-1,
        .bridge={.engine=e,.session=e->text_session,.inline_volume=e->process_tags||(e->native_volume_active&&reloq_session_float_volume(e->text_session)),
            .pcm_period=pcm_batch_period(e),.pcm_early=pcm_startup_samples(e)},
        .settings={e->generic_rate,e->generic_pitch,
            (e->native_volume_active || e->process_tags)?100:e->generic_volume,
            e->native_speed_active?(e->native_speed?e->native_speed:-1):0,
            e->native_pitch_active?e->native_pitch+1:0,
            e->native_inflection_active?e->native_inflection+1:0,
            e->pause_mode,e->head_size+1,e->roughness,e->breathiness,e->gender}};
    const struct reloq_german_text_stream stream={generic_stream_part,&streaming};
    if(can_stream) {
        lock_session(&e->session_lock);
        int reset=reloq_session_reset(e->text_session);
        if(preflight_cancelled(&cancel_check))reloq_session_cancel(e->text_session);
        unlock_session(&e->session_lock);
        if(reset!=RELOQ_SESSION_OK){result=RELOQ_ECI_RENDER;goto generic_done;}
    }
    if (!phonetic_input && boundary_count &&
        generic_source_boundaries(phrase, length, source_marks, boundary_count,
                                  boundaries)) {
        result = RELOQ_ECI_INVALID;
        goto generic_done;
    }
    if (!phonetic_input) {
        const unsigned char *input = phrase;
        if (!admitted && !e->process_tags && memchr(phrase, '`', length)) {
            ordinary_text = malloc(length + 1);
            if (!ordinary_text) {
                result = RELOQ_ECI_MEMORY;
                goto generic_done;
            }
            memcpy(ordinary_text, phrase, length + 1);
            for (size_t i = 0; i < length; ++i) {
                if ((i & 255u) == 0u && preflight_cancelled(&cancel_check)) {
                    free(ordinary_text);
                    result = RELOQ_ECI_CANCELLED;
                    goto generic_done;
                }
                if (ordinary_text[i] == '`') ordinary_text[i] = '\'';
            }
            input = ordinary_text;
        }
        if(can_stream) {
            size_t ni=0,np=0;
            for(size_t b=0;b<boundary_count;++b) {
                int is_index=np==pitch_count||(ni<mark_count&&marks[ni].offset<=pitches[np].offset);
                if(is_index)++ni;
                else {
                    pitch_indices[np]=(struct reloq_german_generic_pitch_mark){boundaries[b].encoded_offset,pitches[np].value};
                    ++np;
                }
            }
            int scoped=reloq_german_control_scope(input,length,
                e->native_pitch_active?e->native_pitch:65,
                e->native_speed_active?e->native_speed:50,e->head_size,e->gender,pitch_indices,pitch_count,
                &streaming.continuation.mixed_pitch,&streaming.continuation.mixed_speed);
            if(scoped<0){free(ordinary_text);result=scoped==-2?RELOQ_ECI_MEMORY:RELOQ_ECI_INVALID;goto generic_done;}
            if(scoped)can_stream=0;
        }
        int converted = reloq_german_best_effort_stream_tags_checked(
            input, length, e->lexicon.rows ? &e->lexicon.view : NULL,
            e->process_tags?1:admitted?2:0,
            &annotation, &annotation_length,
            &guessed, &bad_offset, boundaries, boundary_count,
            &features,&feature_count,preflight_cancelled, &cancel_check,can_stream?&stream:NULL);
        free(ordinary_text);
        if(converted==3 || streaming.status!=RELOQ_ECI_OK) {
            result=streaming.status;
            if(result==RELOQ_ECI_OK && flush_pcm(&streaming.bridge))
                result=streaming.bridge.callback_refused?RELOQ_ECI_CALLBACK:RELOQ_ECI_CANCELLED;
            if(preflight_cancelled(&cancel_check))result=RELOQ_ECI_CANCELLED;
            if(result==RELOQ_ECI_UNSUPPORTED_TEXT) {
                e->failure_stage=RELOQ_ECI_STAGE_ACOUSTIC;e->failure_utf8_offset=-1;
            }
            goto generic_done;
        }
        if (converted) {
            if (preflight_cancelled(&cancel_check) || converted == 2)
                result = RELOQ_ECI_CANCELLED;
            else if (converted == -2) result = RELOQ_ECI_MEMORY;
            else {
                e->failure_stage = RELOQ_ECI_STAGE_TEXT;
                e->failure_utf8_offset = diagnostic_encoded_offset(
                    phrase, length, bad_offset, original_length);
                result = converted == 1 ? RELOQ_ECI_UNSUPPORTED_TEXT :
                                          RELOQ_ECI_INVALID;
            }
            goto generic_done;
        }
    }
    if (preflight_cancelled(&cancel_check)) {
        result = RELOQ_ECI_CANCELLED;
        goto generic_done;
    }

    size_t next_index = 0, next_pitch = 0;
    for (size_t b = 0; b < boundary_count; ++b) {
        int is_index = next_pitch == pitch_count ||
            (next_index < mark_count &&
             marks[next_index].offset <= pitches[next_pitch].offset);
        size_t mapped = phonetic_input ? source_marks[b].offset :
            boundaries[b].annotation_offset;
        if (is_index) indices[next_index++].annotation_offset = mapped;
        else {
            pitch_indices[next_pitch] =
                (struct reloq_german_generic_pitch_mark){
                    mapped, pitches[next_pitch].value};
            ++next_pitch;
        }
    }

    lock_session(&e->session_lock);
    int reset = reloq_session_reset(e->text_session);
    if (preflight_cancelled(&cancel_check))
        reloq_session_cancel(e->text_session);
    unlock_session(&e->session_lock);
    if (reset != RELOQ_SESSION_OK) {
        result = RELOQ_ECI_RENDER;
        goto generic_done;
    }
    {
        struct pcm_bridge bridge = {
            .engine = e, .session = e->text_session,
            .marks = marks, .mark_count = mark_count,
            .inline_volume=e->process_tags||(e->native_volume_active&&reloq_session_float_volume(e->text_session)),
            .pcm_period=pcm_batch_period(e),
            .pcm_early=pcm_startup_samples(e)
        };
        struct reloq_german_utterance_info info = {0};
        const struct reloq_german_generic_settings settings = {
            e->generic_rate, e->generic_pitch,
            (e->native_volume_active || e->process_tags) ? 100 : e->generic_volume,
            e->native_speed_active ? (e->native_speed?e->native_speed:-1) : 0,
            e->native_pitch_active ? e->native_pitch + 1 : 0,
            e->native_inflection_active ? e->native_inflection + 1 : 0,
            e->pause_mode,e->head_size+1,e->roughness,e->breathiness,e->gender};
        int rendered = reloq_german_generic_render_tagged_text_plan(
            annotation ? annotation : phrase,
            annotation ? annotation_length : length,
            &settings, features, feature_count, indices, mark_count,
            pitch_indices, pitch_count, e->text_session,
            send_batched_pcm, send_generic_index, &bridge, &info,
            (e->process_tags||(e->native_volume_active&&reloq_session_float_volume(e->text_session)))?(e->native_volume_active?e->native_volume:e->generic_volume):-1);
        if (rendered == RELOQ_DE_UTTERANCE_OK && flush_pcm(&bridge))
            rendered = RELOQ_DE_UTTERANCE_SINK;
        result = rendered == RELOQ_DE_UTTERANCE_CANCELLED ||
                 reloq_session_cancelled(e->text_session) ||
                 preflight_cancelled(&cancel_check) ? RELOQ_ECI_CANCELLED :
                 rendered == RELOQ_DE_UTTERANCE_OK ? RELOQ_ECI_OK :
                 rendered == RELOQ_DE_UTTERANCE_UNCOVERED ?
                     RELOQ_ECI_UNSUPPORTED_TEXT :
                 rendered == RELOQ_DE_UTTERANCE_SINK &&
                         bridge.callback_refused ? RELOQ_ECI_CALLBACK :
                 rendered == RELOQ_DE_UTTERANCE_MEMORY ?
                     RELOQ_ECI_MEMORY : RELOQ_ECI_RENDER;
        if (result == RELOQ_ECI_UNSUPPORTED_TEXT) {
            e->failure_stage = RELOQ_ECI_STAGE_ACOUSTIC;
            e->failure_utf8_offset = -1;
        }
    }
generic_done:
    free(features);
    free(annotation);
    free(boundaries);
    free(source_marks);
    free(indices);
    free(pitch_indices);
    return result;
}

/* The German ECI text input is byte-oriented. Convert every valid CP1252
 * character, including punctuation, without changing token boundaries. */
static unsigned char *decode_cp1252(const unsigned char *input,
                                     size_t *out_length, int *error)
{
    size_t bytes = strlen((const char *)input);
    size_t required = 0, invalid_at = 0;
    int status = reloq_cp1252_decode(input, bytes, NULL, 0, &required,
                                      &invalid_at);
    if (status != RELOQ_CP1252_OK) {
        *error = RELOQ_ECI_INVALID;
        return NULL;
    }
    unsigned char *output = malloc(required + 1);
    if (!output) {
        *error = RELOQ_ECI_MEMORY;
        return NULL;
    }
    status = reloq_cp1252_decode(input, bytes, output, required + 1,
                                  out_length, &invalid_at);
    if (status != RELOQ_CP1252_OK) {
        *error = RELOQ_ECI_INVALID;
        free(output);
        return NULL;
    }
    return output;
}

static int sample_hz(int code)
{
    switch (code) {
    case 0: return 8000;
    case 1: return 11025;
    case 2: return 22050;
    case 3: return 16000;
    case 4: return 32000;
    case 5: return 44100;
    case 6: return 48000;
    default: return 0;
    }
}

static int ends_german_word(const unsigned char *text, size_t length)
{
    if (!length) return 0;
    const unsigned char last = text[length - 1];
    if ((last >= 'A' && last <= 'Z') ||
        (last >= 'a' && last <= 'z')) return 1;
    if (length < 2 || text[length - 2] != 0xc3) return 0;
    switch (last) {
    case 0x84: case 0x96: case 0x9c: /* Ä Ö Ü */
    case 0xa4: case 0xb6: case 0xbc: /* ä ö ü */
    case 0x9f: /* ß */
        return 1;
    default:
        return 0;
    }
}

#include "reloq_eci_languages.inc"

static struct reloq_session *make_session(int rate_code)
{
    struct reloq_session_config c = {
        .sample_rate = (float)sample_hz(rate_code),
        .duration_scale = 1.0f,
        .output_gain = 1.0f,
        .initial_seed = 100,
        .max_frame_samples = MAX_FRAME_SAMPLES
    };
    return reloq_session_create(&c);
}

static struct reloq_session *make_text_session(int gender,int head,int rate)
{
    float controls[RELOQ_PARAMETER_TRACKS];
    uint8_t field_for_stream[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config c;
    if (reloq_german_voice_defaults(controls, field_for_stream, &c))
        return NULL;
    c.sample_rate=(float)sample_hz(rate);
    if(c.sample_rate>=16000) {
        /* Retain the pre-presence master level, including the upper bass.
         * The presence band itself supplies the requested level reduction;
         * independent AF compensation preserves the accepted S/Z level. */
        c.glottal_reference_rate=11025;
        c.frication_reference_rate=16000;
        c.reference_formants=head<=(gender?40:30)?4:5;
    }
    if(gender) {c.source_offset=head<30?5:3;c.av_trim=-3;c.af_trim=-7;}
    if(c.sample_rate==8000 && head<=(gender?40:30) && !(gender && head<30))
        c.source_offset=7;
    c.max_frame_samples = MAX_FRAME_SAMPLES;
    return reloq_session_create(&c);
}

ECIHand ECICALL eciNewEx(int language)
{
    /* Language words carry an optional Unicode code-set byte. */
    int base=language&~0xff00;
    if ((language&0xff00)!=0 && (language&0xff00)!=0x800)return NULL;
    if (!reloq61_language_supported(base)||dictionary_language_slot(base)>=15) return NULL;
    struct reloq_eci_engine *e = calloc(1, sizeof *e);
    if (!e) return NULL;
    epoch_init(&e->busy);
    epoch_init(&e->stop_epoch);
    epoch_init(&e->session_lock);
    epoch_init(&e->last_index);
    epoch_init(&e->paused);
    epoch_init(&e->program_status);epoch_init(&e->sdk_error);
    int default_rate=eciGetDefaultParam(RELOQ_ECI_SAMPLE_RATE);
    e->session = make_session(default_rate);
    e->text_session = make_text_session(0,50,default_rate);
    if (!e->session || !e->text_session) {
        reloq_session_destroy(e->session);
        reloq_session_destroy(e->text_session);
        free(e);
        return NULL;
    }
    e->sample_rate_code = default_rate;
    e->synth_mode=eciGetDefaultParam(RELOQ_ECI_SYNTH_MODE);
    e->input_type=e->process_tags=eciGetDefaultParam(RELOQ_ECI_INPUT_TYPE);
    e->text_mode=eciGetDefaultParam(2);
    e->dictionary_disabled=eciGetDefaultParam(RELOQ_ECI_DICTIONARY);
    e->real_world_units=eciGetDefaultParam(RELOQ_ECI_REAL_WORLD_UNITS);
    e->head_size=50;
    e->generic_rate = 50;
    e->generic_pitch = 50;
    e->generic_volume = 100;
    e->native_speed = 50;
    e->native_pitch = 65;
    e->native_inflection = 30;
    e->native_volume = 100;
    e->native_speed_active=e->native_pitch_active=e->native_inflection_active=e->native_volume_active=1;
    e->language = base;e->language_dialect=language;
    e->voice_mode=RELOQ_ECI_VOICE_GENERIC;
    initialize_eci_voices(e);
    eci_error(e,RELOQ_ECI_OK);
    e->failure_utf8_offset = -1;
    return e;
}

ECIHand ECICALL eciNew(void)
{
    return eciNewEx(eciGetDefaultParam(RELOQ_ECI_LANGUAGE_DIALECT));
}

ECIHand ECICALL eciDelete(ECIHand handle)
{
    struct reloq_eci_engine *e = handle;
    if (!e) return NULL;
    /* The owner must first join an active render and any concurrent Stop. */
    if (epoch_get(&e->busy)){(void)eci_refuse(e,0);return NULL;}
    clear_queue(e);
    reloq_german_lexicon_file_clear(&e->lexicon);
    reloq_session_destroy(e->session);
    reloq_session_destroy(e->text_session);
    reloq61_worker_free(e->rule_worker);
    free(e->dictionary_path);
    clear_eci_dictionaries(e);
    free(e->dictionary_key_result);free(e->dictionary_value_result);
    free(e);
    return NULL;
}

int ECICALL eciReset(ECIHand handle)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,0);
    return reset_eci_defaults(e);
}

int reloq_eci_reset_audio(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy))return eci_refuse(e,0);
    lock_session(&e->session_lock);
    int result = reloq_session_reset(e->session);
    int text_result = reloq_session_reset(e->text_session);
    unlock_session(&e->session_lock);
    if (result == RELOQ_SESSION_OK && text_result == RELOQ_SESSION_OK)
        clear_queue(e);
    eci_error(e,result == RELOQ_SESSION_OK &&
               text_result == RELOQ_SESSION_OK ? RELOQ_ECI_OK :
               RELOQ_ECI_RENDER);
    e->last_synthesis_error = RELOQ_ECI_OK;
    e->failure_stage = RELOQ_ECI_STAGE_NONE;
    e->failure_utf8_offset = -1;
    if(e->error==RELOQ_ECI_OK) {
        epoch_exchange(&e->program_status,0);epoch_exchange(&e->sdk_error,0);
    }
    return e->error==RELOQ_ECI_OK;
}

void ECICALL eciVersion(char *buffer)
{
    /* The public ECI convention promises at least twenty writable bytes. */
    if (buffer) memcpy(buffer, "1.1.0.rc4", sizeof "1.1.0.rc4");
}

int ECICALL eciGetAvailableLanguages(unsigned int *languages, int *count)
{
    if(!count||*count<0||(!languages&&*count))return 0x80;
    int total=(int)(sizeof eci_languages/sizeof *eci_languages);
    if(!*count){*count=total;return 0;}
    int n=*count<total?*count:total;
    memcpy(languages,eci_languages,(size_t)n*sizeof *languages);*count=n;return 0;
}

void ECICALL eciRegisterCallback(ECIHand handle, ECICallback callback,
                                  void *data)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)){(void)eci_refuse(e,0);return;}
    e->callback = callback;
    e->callback_data = data;
}

int ECICALL eciSetOutputBuffer(ECIHand handle, int samples, short *buffer)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy) || samples < 0 ||
        ((samples == 0) != (buffer == NULL))) return eci_refuse(e,0);
    e->output_samples = samples;
    e->output = buffer;
    eci_error(e,RELOQ_ECI_OK);
    return 1;
}

int ECICALL eciAddText(ECIHand handle, const void *text)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,0);
    if (!text) {
        eci_error(e,RELOQ_ECI_INVALID);
        return 0;
    }
    size_t length = 0;
    int error = RELOQ_ECI_OK;
    unsigned char *utf8 = decode_eci_text(e,text,&length,&error);
    if (!utf8) {
        eci_error(e,error);
        return 0;
    }
    if (append_item(e, (struct queued_item){.utf8 = utf8,
                       .length = length,.voice=capture_voice(e),.unicode_input=e->text_mode||(e->language_dialect&0x800)||e->language==0x120000||e->language==0x60000||e->language==0x60001||e->language==0x80000||e->language==0xa0000,.character_mode=e->text_mode})) {
        free(utf8);
        eci_error(e,RELOQ_ECI_MEMORY);
        return 0;
    }
    eci_error(e,RELOQ_ECI_OK);
    return 1;
}

int ECICALL eciAddTextUtf8(ECIHand handle,const void *text,size_t bytes,int mode)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy))return 0;
    if((!text&&bytes)||(mode!=0&&mode!=1)||bytes==SIZE_MAX) {
        eci_error(e,RELOQ_ECI_INVALID);return 0;
    }
    unsigned char *copy=malloc(bytes+1);
    if(!copy){eci_error(e,RELOQ_ECI_MEMORY);return 0;}
    if(bytes)memcpy(copy,text,bytes);
    copy[bytes]=0;
    if(append_item(e,(struct queued_item){.utf8=copy,.length=bytes,
       .voice=capture_voice(e),.unicode_input=1,.character_mode=mode})) {
        free(copy);eci_error(e,RELOQ_ECI_MEMORY);return 0;
    }
    eci_error(e,RELOQ_ECI_OK);return 1;
}

int ECICALL eciInsertIndex(ECIHand handle, int index)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,0);
    if (append_item(e, (struct queued_item){.index = index, .is_index = 1})) {
        eci_error(e,RELOQ_ECI_MEMORY);
        return 0;
    }
    eci_error(e,RELOQ_ECI_OK);
    return 1;
}

int ECICALL eciInsertPitch(ECIHand handle, int native_pitch)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,0);
    if (native_pitch < 0 || native_pitch > 100 ||
        e->voice_mode != RELOQ_ECI_VOICE_GENERIC) {
        clear_queue(e);
        eci_error(e,RELOQ_ECI_INVALID);
        return 0;
    }
    if (append_item(e, (struct queued_item){
            .pitch = native_pitch, .is_pitch = 1})) {
        clear_queue(e);
        eci_error(e,RELOQ_ECI_MEMORY);
        return 0;
    }
    eci_error(e,RELOQ_ECI_OK);
    return 1;
}

int ECICALL eciSynthesize(ECIHand handle)
{
    struct reloq_eci_engine *e = handle;
    if (!e) return 0;
    unsigned int before = epoch_get(&e->stop_epoch);
    if (!busy_begin(&e->busy)) return 0;
    (void)reloq_session_high_rate_profile(e->text_session,(unsigned)e->high_rate_profile);
    struct reloq61_text_options previous_options=reloq61_text_options_scope(
        (struct reloq61_text_options){e->dictionary_disabled,e->phrase_prediction});
    e->failure_stage = RELOQ_ECI_STAGE_NONE;
    e->failure_utf8_offset = -1;
    int result = RELOQ_ECI_OK;
    struct queued_voice saved_voice=capture_voice(e),initial_voice=saved_voice,last_voice=saved_voice;
    int saved_tags=e->process_tags,have_voice=0;
    struct queued_control *controls=NULL;size_t control_count=0;
    struct reloq_session *saved_text_session=NULL;
    size_t text_bytes = 0;
    size_t original_text_bytes = 0, source_bytes = 0;
    int unicode_input=0;
    unsigned char *modes=NULL;size_t *source_map=NULL;
    size_t mark_count = 0, next_mark = 0, pitch_count = 0;
    unsigned char *phrase = NULL;
    struct index_mark *marks = NULL;
    struct pitch_mark *pitches = NULL;
    struct reloq_german_voice_span *spans = NULL;
    struct connected_index_boundary *boundaries = NULL;
    size_t span_count = 0;
    struct preflight_cancel cancel_check = {e, before};
    if(e->queued) {
        if(e->queued>SIZE_MAX/sizeof *controls){result=RELOQ_ECI_MEMORY;goto done;}
        controls=malloc(e->queued*sizeof *controls);
        if(!controls){result=RELOQ_ECI_MEMORY;goto done;}
    }
    for (size_t i = 0; i < e->queued; ++i) {
        if (e->queue[i].is_index) {
            ++mark_count;
            continue;
        }
        if (e->queue[i].is_pitch) {
            ++pitch_count;
            continue;
        }
        if (e->queue[i].length > SIZE_MAX - 1 - text_bytes) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
        if(e->queue[i].length&&e->voice_mode==RELOQ_ECI_VOICE_GENERIC) {
            if(!have_voice){initial_voice=last_voice=e->queue[i].voice;have_voice=1;}
            else if(memcmp(last_voice.values,e->queue[i].voice.values,sizeof last_voice.values)) {
                controls[control_count++]=(struct queued_control){text_bytes,e->queue[i].voice};
                last_voice=e->queue[i].voice;
            }
        }
        text_bytes += e->queue[i].length;
        unicode_input|=e->queue[i].unicode_input;
    }
    if(have_voice) {
        apply_voice(e,&initial_voice);
        if(initial_voice.values[0]!=saved_voice.values[0]||initial_voice.values[1]!=saved_voice.values[1]) {
            struct reloq_session *next=make_text_session(e->gender,e->head_size,e->sample_rate_code);
            if(!next){result=RELOQ_ECI_MEMORY;goto done;}
            (void)reloq_session_high_rate_profile(next,(unsigned)e->high_rate_profile);
            lock_session(&e->session_lock);saved_text_session=e->text_session;e->text_session=next;unlock_session(&e->session_lock);
        }
    }
    if ((text_bytes || mark_count) && !e->callback) {
        result = RELOQ_ECI_OUTPUT;
        goto done;
    }
    if (text_bytes && (!e->output || e->output_samples < 1)) {
        result = RELOQ_ECI_OUTPUT;
        goto done;
    }
    /* The authored voice currently has only one verified sample profile. */
    if (text_bytes && !e->voice_mode && e->sample_rate_code != 1) {
        result = RELOQ_ECI_UNSUPPORTED_TEXT;
        e->failure_stage = RELOQ_ECI_STAGE_PROFILE;
        goto done;
    }
    /* ECI may deliver successive text fragments at arbitrary boundaries,
     * including within one word. Keep those fragments intact as a single
     * phrase for the voice frontend; never synthesize one fragment at a time.
     * Index offsets are measured in the resulting UTF-8 phrase. */
    if (text_bytes) {
        /* Natural ECI input without an explicit terminal mark has the same
         * period-final prosody in the observed German reference. Reserve a
         * byte for that implicit mark as well as the terminating NUL. */
        if (text_bytes > SIZE_MAX - 2) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
        phrase = malloc(text_bytes + 2);
        if (!phrase) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
    }
    if (mark_count) {
        if (mark_count > SIZE_MAX / sizeof *marks) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
        marks = malloc(mark_count * sizeof *marks);
        if (!marks) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
    }
    if (pitch_count) {
        if (pitch_count > SIZE_MAX / sizeof *pitches) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
        pitches = malloc(pitch_count * sizeof *pitches);
        if (!pitches) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
    }
    if(unicode_input&&text_bytes) {
        modes=malloc(text_bytes);
        if(!modes){result=RELOQ_ECI_MEMORY;goto done;}
    }
    size_t offset = 0;
    size_t marker = 0, pitch_marker = 0;
    for (size_t i = 0; i < e->queued; ++i) {
        if (epoch_get(&e->stop_epoch) != before) {
            result = RELOQ_ECI_CANCELLED;
            goto done;
        }
        const struct queued_item *item = &e->queue[i];
        if (item->is_index) {
            marks[marker++] = (struct index_mark){offset, item->index};
        } else if (item->is_pitch) {
            pitches[pitch_marker++] =
                (struct pitch_mark){offset, item->pitch};
        } else if (item->length) {
            memcpy(phrase + offset, item->utf8, item->length);
            if(modes)memset(modes+offset,item->character_mode,item->length);
            offset += item->length;
        }
    }
    if(text_bytes && !control_count && e->process_tags && e->voice_mode==RELOQ_ECI_VOICE_GENERIC &&
       inline_language_commands(phrase,text_bytes,modes)) {
        result=synthesize_languages(e,phrase,text_bytes,modes,unicode_input,
            marks,mark_count,pitches,pitch_count,before);
        goto done;
    }
    if((unicode_input||control_count)&&text_bytes) {
        unsigned char *normalized=NULL;size_t count=0;source_bytes=text_bytes;
        int r=reloq_unicode_admit_language(phrase,text_bytes,modes,e->language,
            e->process_tags,&normalized,&count,&source_map);
        if(r){result=r==-2?RELOQ_ECI_MEMORY:RELOQ_ECI_INVALID;
            e->failure_stage=RELOQ_ECI_STAGE_TEXT;goto done;}
        for(size_t i=0;i<mark_count;++i)marks[i].offset=source_map[marks[i].offset];
        for(size_t i=0;i<pitch_count;++i)pitches[i].offset=source_map[pitches[i].offset];
        free(phrase);phrase=normalized;text_bytes=count;
    }
    if(control_count) {
        if(insert_voice_controls(&phrase,&text_bytes,controls,control_count,initial_voice,
            source_map,marks,mark_count,pitches,pitch_count)) {result=RELOQ_ECI_MEMORY;goto done;}
        e->process_tags=1;
        /* Diagnostics below refer to the admitted stream, not the temporary
         * annotations. Public index/pitch positions have already moved. */
        free(source_map);source_map=NULL;
        if(inline_language_commands(phrase,text_bytes,NULL)) {
            result=synthesize_languages(e,phrase,text_bytes,NULL,0,marks,mark_count,pitches,pitch_count,before);
            goto done;
        }
    }
    if (text_bytes) {
        original_text_bytes = text_bytes;
        phrase[text_bytes] = 0;
        /* Final punctuation is a prosodic boundary. Do this once after all
         * AddText fragments have been joined, never per fragment. Marks at
         * the caller's EOF move behind the implicit boundary; marks inside
         * a word retain their original offsets and remain unsupported. */
        if (ends_german_word(phrase, text_bytes)) {
            for (size_t i = 0; i < mark_count; ++i)
                if (marks[i].offset == text_bytes) marks[i].offset++;
            phrase[text_bytes++] = '.';
            phrase[text_bytes] = 0;
        }
        if (e->voice_mode == RELOQ_ECI_VOICE_GENERIC) {
            if(!e->rule_worker)e->rule_worker=reloq61_worker_new(e->language);
            if(!e->rule_worker){result=RELOQ_ECI_MEMORY;goto done;}
            void *previous_rule_worker=reloq61_worker_scope(e->rule_worker);
            result = synthesize_generic(e, phrase, text_bytes,
                                        original_text_bytes, marks,
                                        mark_count, pitches,
                                        pitch_count, before,unicode_input);
            reloq61_worker_scope(previous_rule_worker);
            goto done;
        }
        if(e->language!=RELOQ_ECI_GERMAN) {result=RELOQ_ECI_UNSUPPORTED_TEXT;goto done;}
        if (pitch_count) {
            result = RELOQ_ECI_INVALID;
            goto done;
        }
        size_t bad_offset = 0;
        int supported = reloq_german_voice_sentence_spans_checked(
            phrase, text_bytes, NULL, 0, &span_count, &bad_offset,
            preflight_cancelled, &cancel_check);
        if (supported != -2 || !span_count) {
            result = supported == 0 ? RELOQ_ECI_UNSUPPORTED_TEXT :
                     supported == -3 ? RELOQ_ECI_CANCELLED :
                     supported == -4 ? RELOQ_ECI_MEMORY : RELOQ_ECI_INVALID;
            if (result == RELOQ_ECI_UNSUPPORTED_TEXT)
                diagnose_voice_rejection(e, phrase, text_bytes,
                                         original_text_bytes, bad_offset,
                                         &cancel_check);
            goto done;
        }
        if (span_count > SIZE_MAX / sizeof *spans) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
        spans = malloc(span_count * sizeof *spans);
        if (!spans) {
            result = RELOQ_ECI_MEMORY;
            goto done;
        }
        size_t received = span_count;
        supported = reloq_german_voice_sentence_spans_checked(
            phrase, text_bytes, spans, span_count, &received, &bad_offset,
            preflight_cancelled, &cancel_check);
        if (supported != 1 || received != span_count) {
            result = supported == 0 ? RELOQ_ECI_UNSUPPORTED_TEXT :
                     supported == -3 ? RELOQ_ECI_CANCELLED :
                     supported == -4 || supported == -2 ?
                         RELOQ_ECI_MEMORY : RELOQ_ECI_INVALID;
            if (result == RELOQ_ECI_UNSUPPORTED_TEXT)
                diagnose_voice_rejection(e, phrase, text_bytes,
                                         original_text_bytes, bad_offset,
                                         &cancel_check);
            goto done;
        }
        size_t previous_end = 0;
        for (size_t s = 0; s < span_count; ++s) {
            if (!spans[s].length || spans[s].offset < previous_end ||
                spans[s].offset > text_bytes ||
                spans[s].length > text_bytes - spans[s].offset) {
                result = RELOQ_ECI_RENDER;
                goto done;
            }
            previous_end = spans[s].offset + spans[s].length;
        }
        /* Every queue mark is attached to a UTF-8 offset in the complete
         * phrase, independent of AddText fragmentation. For a connected
         * natural pair the only inside-sentence locations with an observed
         * PCM boundary are directly before/after its single separator. All
         * marks are checked before the first callback of this transaction. */
        if (mark_count) {
            if (span_count > SIZE_MAX / sizeof *boundaries) {
                result = RELOQ_ECI_MEMORY;
                goto done;
            }
            boundaries = calloc(span_count, sizeof *boundaries);
            if (!boundaries) {
                result = RELOQ_ECI_MEMORY;
                goto done;
            }
        }
        size_t next_sentence = 0;
        for (size_t i = 0; i < mark_count; ++i) {
            if (epoch_get(&e->stop_epoch) != before) {
                result = RELOQ_ECI_CANCELLED;
                goto done;
            }
            size_t at = marks[i].offset;
            while (next_sentence < span_count &&
                   at >= spans[next_sentence].offset +
                         spans[next_sentence].length)
                ++next_sentence;
            if (next_sentence == span_count ||
                at <= spans[next_sentence].offset) continue;
            struct connected_index_boundary *b = &boundaries[next_sentence];
            if (!b->used) {
                size_t word_end = 0, sample = 0;
                int known = reloq_german_voice_connected_index_sample(
                    phrase + spans[next_sentence].offset,
                    spans[next_sentence].length, &word_end, &sample,
                    preflight_cancelled, &cancel_check);
                if (known != 1) {
                    result = known == -3 ? RELOQ_ECI_CANCELLED :
                             known == -4 ? RELOQ_ECI_MEMORY :
                             RELOQ_ECI_UNSUPPORTED_TEXT;
                    if (result == RELOQ_ECI_UNSUPPORTED_TEXT) {
                        e->failure_stage = RELOQ_ECI_STAGE_INDEX;
                        e->failure_utf8_offset = diagnostic_offset(
                            at, original_text_bytes);
                    }
                    goto done;
                }
                if (word_end >= spans[next_sentence].length - 1 ||
                    !sample) {
                    result = RELOQ_ECI_RENDER;
                    goto done;
                }
                b->word_end = spans[next_sentence].offset + word_end;
                b->next_word = b->word_end + 1;
                b->sample = sample;
                b->used = 1;
            }
            if (at != b->word_end && at != b->next_word) {
                result = RELOQ_ECI_UNSUPPORTED_TEXT;
                e->failure_stage = RELOQ_ECI_STAGE_INDEX;
                e->failure_utf8_offset = diagnostic_offset(
                    at, original_text_bytes);
                goto done;
            }
        }
    }
    if (epoch_get(&e->stop_epoch) != before) {
        result = RELOQ_ECI_CANCELLED;
        goto done;
    }
    lock_session(&e->session_lock);
    int reset = reloq_session_reset(e->text_session);
    if (epoch_get(&e->stop_epoch) != before)
        reloq_session_cancel(e->text_session);
    unlock_session(&e->session_lock);
    if (reset != RELOQ_SESSION_OK) {
        result = RELOQ_ECI_RENDER;
        goto done;
    }
    struct pcm_bridge bridge = {
        .engine = e, .session = e->text_session,
        .marks = marks, .mark_count = mark_count, .next_mark = &next_mark
    };
    if (!text_bytes) {
        result = send_indices_until(e, marks, mark_count, &next_mark, 0);
        goto done;
    }
    for (size_t s = 0; s < span_count; ++s) {
        result = send_indices_until(e, marks, mark_count, &next_mark,
                                    spans[s].offset);
        if (result != RELOQ_ECI_OK) goto done;
        bridge.boundary = boundaries && boundaries[s].used ?
            &boundaries[s] : NULL;
        bridge.span_samples = 0;
        struct reloq_german_utterance_info info;
        int status = reloq_german_voice_render(
            phrase + spans[s].offset, spans[s].length, e->text_session,
            send_pcm, &bridge, &info);
        result = status == RELOQ_DE_UTTERANCE_CANCELLED ||
                 reloq_session_cancelled(e->text_session) ?
                     RELOQ_ECI_CANCELLED :
                 status == RELOQ_DE_UTTERANCE_OK ? RELOQ_ECI_OK :
                 status == RELOQ_DE_UTTERANCE_UNCOVERED ?
                     RELOQ_ECI_UNSUPPORTED_TEXT :
                 status == RELOQ_DE_UTTERANCE_SINK &&
                         bridge.callback_refused ?
                     RELOQ_ECI_CALLBACK :
                 status == RELOQ_DE_UTTERANCE_MEMORY ?
                     RELOQ_ECI_MEMORY : RELOQ_ECI_RENDER;
        if (result == RELOQ_ECI_UNSUPPORTED_TEXT) {
            e->failure_stage = RELOQ_ECI_STAGE_ACOUSTIC;
            e->failure_utf8_offset = diagnostic_acoustic_start(
                phrase, text_bytes, spans[s].offset + info.error_offset,
                original_text_bytes);
        }
        if (result != RELOQ_ECI_OK) goto done;
        if (bridge.boundary) {
            result = RELOQ_ECI_RENDER;
            goto done;
        }
    }
    result = send_indices_until(e, marks, mark_count, &next_mark,
                                text_bytes);
done:
    if(source_map&&e->failure_utf8_offset>=0) {
        size_t i=0;
        while(i<source_bytes&&source_map[i+1]<=(size_t)e->failure_utf8_offset)++i;
        e->failure_utf8_offset=diagnostic_offset(i,source_bytes);
    }
    free(source_map);free(modes);
    if (result == RELOQ_ECI_OK && epoch_get(&e->stop_epoch) != before)
        result = RELOQ_ECI_CANCELLED;
    free(boundaries);
    free(spans);
    free(marks);
    free(pitches);
    free(phrase);
    clear_queue(e);
    free(controls);apply_voice(e,&saved_voice);e->process_tags=saved_tags;
    if(saved_text_session) {
        lock_session(&e->session_lock);struct reloq_session *temporary=e->text_session;e->text_session=saved_text_session;unlock_session(&e->session_lock);
        reloq_session_destroy(temporary);
    }
    eci_error(e,result);
    e->last_synthesis_error = result;
    reloq61_text_options_scope(previous_options);
    busy_end(&e->busy);
    return result == RELOQ_ECI_OK;
}

int ECICALL eciSynchronize(ECIHand handle)
{
    struct reloq_eci_engine *e = handle;
    if(!e||epoch_get(&e->busy))return eci_refuse(e,0);
    return e->error==RELOQ_ECI_OK;
}

int ECICALL eciStop(ECIHand handle)
{
    struct reloq_eci_engine *e = handle;
    if (!e) return 0;
    /* Increment before touching the session so a simultaneous render reset
     * cannot swallow a Stop received after it claimed the busy state. */
    epoch_inc(&e->stop_epoch);
    epoch_exchange(&e->paused,0);
    lock_session(&e->session_lock);
    reloq_session_cancel(e->session);
    reloq_session_cancel(e->text_session);
    unlock_session(&e->session_lock);
    return 1;
}

int ECICALL eciSpeaking(ECIHand handle)
{
    struct reloq_eci_engine *e = handle;
    return e && epoch_get(&e->busy) ? 1 : 0;
}

int ECICALL eciPause(ECIHand handle,int pause)
{
    struct reloq_eci_engine *e=handle;if(!e)return 0;
    epoch_exchange(&e->paused,!!pause);return 1;
}

int ECICALL eciGetIndex(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;
    return e?(int)epoch_get(&e->last_index):0;
}

int ECICALL eciProgStatus(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;
    return e?(int)epoch_get(&e->program_status):0x80;
}
int ECICALL eciIsBeingReentered(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;return e?(int)epoch_get(&e->busy):0;
}
void ECICALL eciClearErrors(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;if(!e)return;
    if(epoch_get(&e->busy)){(void)eci_refuse(e,0);return;}
    epoch_exchange(&e->program_status,0);epoch_exchange(&e->sdk_error,0);
}
void ECICALL eciErrorMessage(ECIHand handle,void *buffer)
{
    struct reloq_eci_engine *e=handle;if(!buffer)return;
    const char *message="System resources are low.";
    if(e)switch(epoch_get(&e->sdk_error)) {
    case 0:message="";break;
    case 1:message="System error.";break;
    case 2:break;
    case 0x10:message="Synthesis engine error.";break;
    case 0x20:message="Audio device error.";break;
    case 0x80:message="Invalid or out of range parameter.";break;
    case 0x100:case 0x2000:message="Synthesis engine is busy.";break;
    case 0x200:message="Audio device busy.";break;
    case 0x400:message="Synthesis engine is paused.";break;
    case 0x800:message="Cannot reenter ECI on the same thread.";break;
    case 0x4000:message="Cannot load module.";break;
    default:message="Synthesis engine error.";break;
    }
    if(e&&(e->language_dialect&0x800)) {
        for(size_t i=0;;++i) {
            uint16_t unit=(unsigned char)message[i];memcpy((char *)buffer+2*i,&unit,2);
            if(!unit)break;
        }
    } else strcpy(buffer,message);
}

int ECICALL eciClearInput(ECIHand handle)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,0);
    clear_queue(e);
    eci_error(e,RELOQ_ECI_OK);
    return 1;
}

int ECICALL eciGetParam(ECIHand handle, int parameter)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,-1);
    switch (parameter) {
    case RELOQ_ECI_SYNTH_MODE: return e->synth_mode;
    case RELOQ_ECI_VOICE_MODE: return e->voice_mode;
    case RELOQ_ECI_GENERIC_RATE: return e->generic_rate;
    case RELOQ_ECI_GENERIC_PITCH: return e->generic_pitch;
    case RELOQ_ECI_GENERIC_VOLUME: return e->generic_volume;
    case RELOQ_ECI_NATIVE_SPEED: return e->native_speed;
    case RELOQ_ECI_NATIVE_PITCH: return e->native_pitch;
    case RELOQ_ECI_NATIVE_INFLECTION: return e->native_inflection;
    case RELOQ_ECI_NATIVE_VOLUME: return e->native_volume;
    case RELOQ_ECI_INPUT_TYPE: return e->input_type;
    case RELOQ_ECI_REAL_WORLD_UNITS:return e->real_world_units;
    case RELOQ_ECI_PROCESS_TAGS: return e->process_tags;
    case RELOQ_ECI_DICTIONARY: return e->dictionary_disabled;
    case RELOQ_ECI_PHRASE_PREDICTION: return e->phrase_prediction;
    case RELOQ_ECI_PAUSE_MODE: return e->pause_mode;
    case RELOQ_ECI_HEAD_SIZE: return e->head_size;
    case RELOQ_ECI_ROUGHNESS: return e->roughness;
    case RELOQ_ECI_BREATHINESS: return e->breathiness;
    case RELOQ_ECI_GENDER: return e->gender;
    case RELOQ_ECI_HIGH_RATE_PROFILE: return e->high_rate_profile;
    case RELOQ_ECI_SAMPLE_RATE: return e->sample_rate_code;
    case RELOQ_ECI_LANGUAGE_DIALECT: return e->language_dialect;
    case 2:return e->text_mode;
    case RELOQ_ECI_LAST_ERROR: return e->last_synthesis_error;
    case RELOQ_ECI_FAILURE_UTF8_OFFSET: return e->failure_utf8_offset;
    case RELOQ_ECI_FAILURE_STAGE: return e->failure_stage;
    default: return eci_refuse(e,-1);
    }
}

int ECICALL eciSetParam(ECIHand handle, int parameter, int value)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,-1);
    int old = eciGetParam(handle, parameter);
    if (old < 0) return eci_refuse(e,-1);
    switch (parameter) {
    case RELOQ_ECI_HIGH_RATE_PROFILE:
        if(value<0 || value>3)return eci_refuse(e,-1);
        e->high_rate_profile=value;
        break;
    case RELOQ_ECI_SYNTH_MODE:
        if (value < 0 || value > 1) return eci_refuse(e,-1);
        e->synth_mode = value;
        break;
    case RELOQ_ECI_VOICE_MODE:
        if (value != RELOQ_ECI_VOICE_EXACT &&
            value != RELOQ_ECI_VOICE_GENERIC) return eci_refuse(e,-1);
        e->voice_mode = value;
        break;
    case RELOQ_ECI_GENERIC_RATE:
        if (value < 0 || value > 100) return eci_refuse(e,-1);
        e->generic_rate = value;
        e->native_speed_active = 0;
        break;
    case RELOQ_ECI_GENERIC_PITCH:
        if (value < 0 || value > 100) return eci_refuse(e,-1);
        e->generic_pitch = value;
        e->native_pitch_active = 0;
        break;
    case RELOQ_ECI_GENERIC_VOLUME:
        if (value < 0 || value > 100) return eci_refuse(e,-1);
        e->generic_volume = value;
        e->native_volume_active = 0;
        break;
    case RELOQ_ECI_NATIVE_SPEED:
        if (value < 0 || value > 250) return eci_refuse(e,-1);
        e->native_speed = value;
        e->native_speed_active = 1;
        e->real_voice_flags[0]&=~(1u<<6);
        break;
    case RELOQ_ECI_NATIVE_PITCH:
        if (value < 0 || value > 100) return eci_refuse(e,-1);
        e->native_pitch = value;
        e->native_pitch_active = 1;
        e->real_voice_flags[0]&=~(1u<<2);
        break;
    case RELOQ_ECI_NATIVE_INFLECTION:
        if (value < 0 || value > 100) return eci_refuse(e,-1);
        e->native_inflection = value;
        e->native_inflection_active = 1;
        break;
    case RELOQ_ECI_NATIVE_VOLUME:
        if (value < 0 || value > 100) return eci_refuse(e,-1);
        e->native_volume = value;
        e->native_volume_active = 1;
        e->real_voice_flags[0]&=~(1u<<7);
        break;
    case RELOQ_ECI_HEAD_SIZE:
        if(value<0 || value>100)return eci_refuse(e,-1);
        /* The session also owns the 8-kHz source offset and the wideband
         * reference bank count. Rebuild on every anatomy change so a
         * selected head cannot depend on the preceding slider position. */
        if(value!=e->head_size) {
            struct reloq_session *next=make_text_session(e->gender,value,e->sample_rate_code);
            if(!next)return eci_refuse(e,-1);
            lock_session(&e->session_lock);
            struct reloq_session *previous=e->text_session;
            e->text_session=next;
            unlock_session(&e->session_lock);
            reloq_session_destroy(previous);
        }
        e->head_size=value;break;
    case RELOQ_ECI_ROUGHNESS:
        if(value<0 || value>100)return eci_refuse(e,-1);
        e->roughness=value;break;
    case RELOQ_ECI_BREATHINESS:
        if(value<0 || value>100)return eci_refuse(e,-1);
        e->breathiness=value;break;
    case RELOQ_ECI_GENDER:
        if(value<0 || value>1)return eci_refuse(e,-1);
        if(value!=old) {
            struct reloq_session *next=make_text_session(value,e->head_size,e->sample_rate_code);
            if(!next)return eci_refuse(e,-1);
            lock_session(&e->session_lock);
            struct reloq_session *previous=e->text_session;
            e->text_session=next;
            unlock_session(&e->session_lock);
            reloq_session_destroy(previous);e->gender=value;
        }
        break;
    case RELOQ_ECI_DICTIONARY:
        if(value!=0 && value!=1)return eci_refuse(e,-1);
        e->dictionary_disabled=value;break;
    case RELOQ_ECI_PHRASE_PREDICTION:
        if(value!=0 && value!=1)return eci_refuse(e,-1);
        e->phrase_prediction=value;break;
    case RELOQ_ECI_PROCESS_TAGS:
        if(value!=0 && value!=1)return eci_refuse(e,-1);
        e->process_tags=value;e->input_type=value;break;
    case RELOQ_ECI_REAL_WORLD_UNITS:
        if(value!=0 && value!=1)return eci_refuse(e,-1);
        e->real_world_units=value;break;
    case RELOQ_ECI_PAUSE_MODE:
        if(value<0 || value>2)return eci_refuse(e,-1);
        e->pause_mode=value;break;
    case RELOQ_ECI_INPUT_TYPE:
        if (value != 0 && value != 1) return eci_refuse(e,-1);
        e->input_type = value;e->process_tags=value;
        break;
    case RELOQ_ECI_SAMPLE_RATE:
        if (!sample_hz(value)) return eci_refuse(e,-1);
        if (value != old) {
            struct reloq_session *next = make_session(value);
            struct reloq_session *text_next = make_text_session(e->gender,e->head_size,value);
            if (!next || !text_next) {
                reloq_session_destroy(next); reloq_session_destroy(text_next);
                return eci_refuse(e,-1);
            }
            lock_session(&e->session_lock);
            struct reloq_session *previous = e->session, *text_previous=e->text_session;
            e->session = next; e->text_session=text_next;
            unlock_session(&e->session_lock);
            reloq_session_destroy(previous); reloq_session_destroy(text_previous);
            e->sample_rate_code = value;
        }
        break;
    case RELOQ_ECI_LANGUAGE_DIALECT:
        if(switch_eci_language(e,value))return eci_refuse(e,-1);
        break;
    case 2:
        if(value<0||value>1)return eci_refuse(e,-1);
        e->text_mode=value;break;
    default: return eci_refuse(e,-1);
    }
    eci_error(e,RELOQ_ECI_OK);
    return old;
}

static FILE *open_utf8_lexicon(const char *path)
{
#if defined(_WIN32)
    int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                     path, -1, NULL, 0);
    if (needed <= 0 || (size_t)needed > SIZE_MAX / sizeof(wchar_t))
        return NULL;
    wchar_t *wide = malloc((size_t)needed * sizeof *wide);
    if (!wide) return NULL;
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                            path, -1, wide, needed) != needed) {
        free(wide);
        return NULL;
    }
    FILE *file = _wfopen(wide, L"rb");
    free(wide);
    return file;
#else
    return fopen(path, "rb");
#endif
}

#include "reloq_eci_compat.inc"
#include "reloq_eci_dictionary.inc"
#include "reloq_eci_phonemes.inc"

int ECICALL eciSetDictionaryPath(ECIHand handle,const char *utf8_directory)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy))return 0;
    size_t n=0;char *path=NULL;
    if(utf8_directory) {
        while(n<8192&&utf8_directory[n])++n;
        if(!n||n==8192){eci_error(e,RELOQ_ECI_INVALID);return 0;}
        path=malloc(n+1);if(!path){eci_error(e,RELOQ_ECI_MEMORY);return 0;}
        memcpy(path,utf8_directory,n+1);
    }
    void *worker=reloq61_worker_new(e->language);
    if(!worker||reloq61_worker_dictionary(worker,path)) {
        reloq61_worker_free(worker);free(path);eci_error(e,RELOQ_ECI_INVALID);return 0;
    }
    reloq61_worker_free(e->rule_worker);free(e->dictionary_path);
    e->language_dictionaries[dictionary_language_slot(e->language)]=NULL;
    e->rule_worker=worker;e->dictionary_path=path;e->active_dictionary=NULL;eci_error(e,RELOQ_ECI_OK);return 1;
}

int ECICALL eciLoadLexicon(ECIHand handle, const char *utf8_path)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,0);
    if (!utf8_path) {
        reloq_german_lexicon_file_clear(&e->lexicon);
        eci_error(e,RELOQ_ECI_OK);
        return 1;
    }
    /* Apart from the lexicon's own 16 MiB content bound, reject an excessive
     * or empty path before handing it to the platform file API. */
    size_t length = 0;
    while (length < 8192 && utf8_path[length]) ++length;
    if (!length || length == 8192) {
        eci_error(e,RELOQ_ECI_INVALID);
        return 0;
    }
    FILE *stream = open_utf8_lexicon(utf8_path);
    if (!stream) {
        eci_error(e,RELOQ_ECI_INVALID);
        return 0;
    }
    int loaded = reloq_german_lexicon_file_load_stream(&e->lexicon, stream);
    (void)fclose(stream);
    eci_error(e,loaded == 0 ? RELOQ_ECI_OK : RELOQ_ECI_INVALID);
    return loaded == 0;
}

static int flush_pcm(struct pcm_bridge *b)
{
    if (!b->pcm_used) return 0;
    if (reloq_session_cancelled(b->session)) return -1;
    size_t count = b->pcm_used;
    b->pcm_used = 0;
    struct reloq_eci_engine *e = b->engine;
    int status=notify_callback(e,RELOQ_ECI_WAVEFORM_BUFFER,(int)count);
    if(status!=RELOQ_ECI_OK) {
        b->callback_refused=status==RELOQ_ECI_CALLBACK;
        return -1;
    }
    return 0;
}

static int send_pcm(void *context, const int16_t *pcm, size_t samples)
{
    struct pcm_bridge *b = context;
    struct reloq_eci_engine *e = b->engine;
    while (samples) {
        if (reloq_session_cancelled(b->session)) return -1;
        size_t count = samples;
        if (count > (size_t)e->output_samples)
            count = (size_t)e->output_samples;
        /* The voice preflight supplies Apple's measured sample offset
         * after the first connected coda send. Split even an 11-sample
         * Klatt frame there; the caller's buffer can be smaller. */
        if (b->boundary) {
            if (b->span_samples >= b->boundary->sample) return -1;
            size_t until = b->boundary->sample - b->span_samples;
            if (count > until) count = until;
        }
        memcpy(e->output, pcm, count * sizeof *pcm);
        /* Apple changes volume after Klatt, leaving control frames intact.
         * Exact reference mode must retain bitwise-identical PCM. */
        if (e->voice_mode == RELOQ_ECI_VOICE_GENERIC &&
            !b->inline_volume && e->native_volume_active && e->native_volume != 100) {
            double gain = reloq_voice_volume_gain(e->native_volume);
            for (size_t i = 0; i < count; ++i)
                e->output[i] = (short)lround((double)e->output[i] * gain);
        }
        int status=notify_callback(e,RELOQ_ECI_WAVEFORM_BUFFER,(int)count);
        if(status!=RELOQ_ECI_OK) {
            b->callback_refused=status==RELOQ_ECI_CALLBACK;
            return -1;
        }
        b->span_samples += count;
        pcm += count;
        samples -= count;
        if (b->boundary && b->span_samples == b->boundary->sample) {
            int status = send_indices_until(
                e, b->marks, b->mark_count, b->next_mark,
                b->boundary->next_word);
            if (status != RELOQ_ECI_OK) {
                b->callback_refused = status == RELOQ_ECI_CALLBACK;
                return -1;
            }
            b->boundary = NULL;
        }
    }
    return 0;
}
static int send_batched_pcm(void *context, const int16_t *pcm, size_t samples)
{
    struct pcm_bridge *b = context;
    struct reloq_eci_engine *e = b->engine;
    if (b->pcm_early) {
        int status = send_pcm(context, pcm, samples);
        if (!status)
            b->pcm_early = samples < b->pcm_early ? b->pcm_early - samples : 0;
        return status;
    }
    while (samples) {
        if (reloq_session_cancelled(b->session)) return -1;
        size_t count = samples;
        size_t limit = b->pcm_period - b->pcm_phase;
        if (count > limit) count = limit;
        /* The voice preflight supplies Apple's measured sample offset
         * after the first connected coda send. Split even an 11-sample
         * Klatt frame there; the caller's buffer can be smaller. */
        if (b->boundary) {
            if (b->span_samples >= b->boundary->sample) return -1;
            size_t until = b->boundary->sample - b->span_samples;
            if (count > until) count = until;
        }
        short *output = e->output + b->pcm_used;
        memcpy(output, pcm, count * sizeof *pcm);
        /* Apple changes volume after Klatt, leaving control frames intact.
         * Exact reference mode must retain bitwise-identical PCM. */
        if (e->voice_mode == RELOQ_ECI_VOICE_GENERIC &&
            !b->inline_volume && e->native_volume_active && e->native_volume != 100) {
            double gain = reloq_voice_volume_gain(e->native_volume);
            for (size_t i = 0; i < count; ++i)
                output[i] = (short)lround((double)output[i] * gain);
        }
        b->pcm_used += count;
        b->pcm_phase += count;
        if (b->pcm_phase == b->pcm_period) {
            b->pcm_phase = 0;
            if (flush_pcm(b)) return -1;
        }
        b->span_samples += count;
        pcm += count;
        samples -= count;
        if (b->boundary && b->span_samples == b->boundary->sample) {
            if (flush_pcm(b)) return -1;
            int status = send_indices_until(
                e, b->marks, b->mark_count, b->next_mark,
                b->boundary->next_word);
            if (status != RELOQ_ECI_OK) {
                b->callback_refused = status == RELOQ_ECI_CALLBACK;
                return -1;
            }
            b->boundary = NULL;
        }
    }
    return 0;
}

int reloq_eci_render_controls(ECIHand handle, const float *controls,
                               uint32_t frame_count, uint32_t highest_formant,
                               uint32_t *frames_completed)
{
    struct reloq_eci_engine *e = handle;
    if (frames_completed) *frames_completed = 0;
    if (!e || (frame_count && !controls) || highest_formant < 1 ||
        highest_formant > 8) return RELOQ_ECI_INVALID;
#if SIZE_MAX < UINT64_MAX
    if ((size_t)frame_count > SIZE_MAX / CONTROL_FIELDS)
        return RELOQ_ECI_INVALID;
#endif
    unsigned int before = epoch_get(&e->stop_epoch);
    if (!busy_begin(&e->busy)) return RELOQ_ECI_BUSY;
    int result = RELOQ_ECI_OK;
    if (frame_count && (!e->output || e->output_samples < 1 || !e->callback)) {
        result = RELOQ_ECI_OUTPUT;
        goto done;
    }
    lock_session(&e->session_lock);
    int reset = reloq_session_reset(e->session);
    if (epoch_get(&e->stop_epoch) != before)
        reloq_session_cancel(e->session);
    unlock_session(&e->session_lock);
    if (reset != RELOQ_SESSION_OK) {
        result = RELOQ_ECI_RENDER;
        goto done;
    }
    struct pcm_bridge bridge = {.engine = e, .session = e->session};
    for (uint32_t i = 0; i < frame_count; ++i) {
        if (reloq_session_cancelled(e->session)) {
            result = RELOQ_ECI_CANCELLED;
            break;
        }
        struct reloq_core_voiced_frame frame;
        if (reloq_frame_adapter_map(
                controls + (size_t)i * CONTROL_FIELDS,
                (long)highest_formant, &frame)) {
            result = RELOQ_ECI_INVALID;
            break;
        }
        size_t delivered = 0;
        int status = reloq_session_render_frames(
            e->session, &frame, 1, send_pcm, &bridge, &delivered);
        if (frames_completed) *frames_completed += (uint32_t)delivered;
        if (status == RELOQ_SESSION_OK) continue;
        result = status == RELOQ_SESSION_CANCELLED ||
                         reloq_session_cancelled(e->session) ?
                     RELOQ_ECI_CANCELLED :
                 status == RELOQ_SESSION_SINK && bridge.callback_refused ?
                     RELOQ_ECI_CALLBACK : RELOQ_ECI_RENDER;
        break;
    }
done:
    eci_error(e,result);
    busy_end(&e->busy);
    return result;
}

int reloq_eci_last_error(ECIHand handle)
{
    struct reloq_eci_engine *e = handle;
    return e && !epoch_get(&e->busy) ? e->error : RELOQ_ECI_INVALID;
}
