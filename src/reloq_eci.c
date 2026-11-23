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
#include "reloq_german_generic_voice.h"
#include "reloq_german_lexicon_file.h"
#include "reloq_german_voice.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_german_text.h"
#include "reloq_session.h"
#include "reloq_english_voice.h"
#include "reloq_voice_controls.h"
#include "reloq_equalizer.h"
#include "reloq_wordseg.h"
#include "reloq_persian_direct.h"
#include "reloq_persian_clock.h"
#include "reloq_persian_transition.h"
#include "reloq_persian_acoustics.h"
#include "reloq_persian_frame_internal.h"
#include "reloq_persian_tuning.h"
#include "reloq_persian_speak.h"
#include "reloq_persian_us_proxy.h"
#include "reloq_syllable_trace.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#include <windows.h>
#include <process.h>
#include <wchar.h>
#else
#include <pthread.h>
#if !defined(__ANDROID__)
#include <iconv.h>
#endif
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
    0x90000,0xa0000,0x120000,0x130000};
#define ECI_LANGUAGE_COUNT (sizeof eci_languages/sizeof *eci_languages)
static unsigned dictionary_language_slot(int language)
{
    unsigned i=0;
    while(i<ECI_LANGUAGE_COUNT&&eci_languages[i]!=(unsigned)language)++i;
    return i;
}
_Static_assert(sizeof(short) == sizeof(int16_t), "ECI output needs 16-bit short");

struct queued_voice {
    int values[8],generic[3];
    int pitch_hz,native_wpm;
    unsigned active;
};
struct queued_item {
    unsigned char *utf8;
    size_t length;
    int index;
    int is_index;
    int pitch;
    int is_pitch;
    int unicode_input, character_mode, segmentation_guarded, source_in_context;
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
    void *api_worker,*cached_worker;
    int language,walk_active[3];
};

struct reloq_eci_engine {
    struct reloq_session *session;
    struct reloq_session *text_session;
    /* An optional, private US contour worker for the experimental native
     * Persian renderer. Ordinary language handles never allocate one. */
    struct reloq_eci_engine *persian_us_pilot;
    /* The incremental Persian bridge owns its English analysis worker as
     * well. It is separate from the concurrently usable contour pilot. */
    void *persian_english_worker;
    void *rule_worker;
    char *dictionary_path;
    struct reloq_german_lexicon_file lexicon;
    uint64_t lexicon_version;
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
    int native_speed,native_wpm;
    int native_pitch;
    int native_inflection;
    int native_volume;
    int native_speed_active;
    int native_pitch_active;
    int native_inflection_active;
    int native_volume_active;
    int input_type;
    int process_tags;
    int dictionary_disabled,phrase_prediction,original_questions,mellow_mode,original_acoustic_clock;
    int word_segmentation,camel_case;
    int eq_low,eq_mid,eq_high;
    int eq_filters[RELOQ_EQ_FILTER_COUNT][RELOQ_ECI_EQ_FILTER_STRIDE];
    int pause_mode;
    int head_size,roughness,breathiness,gender;
    int german_glide_opening,german_glide_backness,german_glide_softness;
    int language,language_dialect,text_mode,real_world_units;
    int saved_voices[16][8];char voice_names[17][120];
    int real_voice_values[17][8];unsigned real_voice_flags[17];
    struct reloq_eci_dictionary *dictionaries,*active_dictionary;
    struct reloq_eci_dictionary *language_dictionaries[ECI_LANGUAGE_COUNT];
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
        (e->real_voice_flags[0]&(1u<<2))?e->real_voice_values[0][2]:0,
        e->native_wpm,
        (unsigned)(e->native_speed_active|e->native_pitch_active<<1|
            e->native_inflection_active<<2|e->native_volume_active<<3)};
}
static void apply_voice(struct reloq_eci_engine *e,const struct queued_voice *v)
{
    e->gender=v->values[0];e->head_size=v->values[1];
    e->native_pitch=v->values[2];e->native_inflection=v->values[3];
    e->roughness=v->values[4];e->breathiness=v->values[5];
    e->native_speed=v->values[6];e->native_volume=v->values[7];
    e->native_wpm=v->native_wpm;
    e->generic_rate=v->generic[0];e->generic_pitch=v->generic[1];e->generic_volume=v->generic[2];
    if(v->pitch_hz) {
        e->real_voice_values[0][2]=v->pitch_hz;e->real_voice_flags[0]|=1u<<2;
    } else e->real_voice_flags[0]&=~(1u<<2);
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
    struct pitch_mark *pitches,size_t pitch_count,unsigned char **guards)
{
    static const char *const names[]={"vg","vh","vb","vf","vr","vy","vs","vv"};
    if(!count)return 0;
    if(count>(SIZE_MAX-*length-2)/96)return -1;
    unsigned char *next=malloc(*length+count*96+2);
    unsigned char *next_guards=guards&&*guards?calloc(*length+count*96+2,1):NULL;
    size_t *offsets=malloc((count+1)*sizeof *offsets),*added=malloc((count+1)*sizeof *added);
    if(!next||!offsets||!added||(guards&&*guards&&!next_guards)){
        free(next);free(next_guards);free(offsets);free(added);return -1;}
    size_t used=0,from=0;
    for(size_t i=0;i<count;++i) {
        size_t at=source_map?source_map[controls[i].offset]:controls[i].offset;
        memcpy(next+used,*text+from,at-from);
        if(next_guards)memcpy(next_guards+used,*guards+from,at-from);
        used+=at-from;from=at;
        size_t annotations_begin=used;
        for(unsigned k=0;k<8;++k)if(initial.values[k]!=controls[i].voice.values[k] ||
            (k==2 && initial.pitch_hz!=controls[i].voice.pitch_hz)) {
            /* An enabled WPM setting owns speed until explicitly cleared. */
            if(k==6&&(controls[i].voice.native_wpm ||
                      initial.native_wpm!=controls[i].voice.native_wpm))continue;
            int hz=k==2?controls[i].voice.pitch_hz:0;
            int n=snprintf((char *)next+used,80,"`%s%d",hz?"vbhz":names[k],
                hz?hz:controls[i].voice.values[k]);
            if(n<0||n>=80){free(next);free(next_guards);free(offsets);free(added);return -1;}
            used+=(size_t)n;
        }
        if(initial.native_wpm!=controls[i].voice.native_wpm) {
            int wpm=controls[i].voice.native_wpm;
            int n=snprintf((char *)next+used,96,"`%s%d",wpm?"vwpm":"vs",
                           wpm?wpm:controls[i].voice.values[6]);
            if(n<0||n>=96){free(next);free(next_guards);free(offsets);free(added);return -1;}
            used+=(size_t)n;
        }
        /* A digit immediately after the last generated annotation would be
         * parsed as part of its numeric parameter (e.g. vb65 + 90). Only
         * add the separator in that ambiguous case; the established
         * mid-word behavior for ordinary text remains unchanged. */
        if(used>annotations_begin && at<*length &&
           (*text)[at]>='0' && (*text)[at]<='9')next[used++]=' ';
        initial=controls[i].voice;offsets[i]=at;added[i]=used-at;
    }
    memcpy(next+used,*text+from,*length-from);
    if(next_guards)memcpy(next_guards+used,*guards+from,*length-from);
    used+=*length-from;next[used]=0;
    for(size_t i=0;i<mark_count;++i) {
        size_t extra=0;for(size_t j=0;j<count&&offsets[j]<=marks[i].offset;++j)extra=added[j];
        marks[i].offset+=extra;
    }
    for(size_t i=0;i<pitch_count;++i) {
        size_t extra=0;for(size_t j=0;j<count&&offsets[j]<=pitches[i].offset;++j)extra=added[j];
        pitches[i].offset+=extra;
    }
    free(*text);*text=next;
    if(next_guards){free(*guards);*guards=next_guards;}
    *length=used;free(offsets);free(added);return 0;
}

/* Map protected source bytes through the same Unicode admission map used by
 * public indices. Added annotations do not inherit dictionary protection. */
static unsigned char *project_guards(const unsigned char *guards,size_t original,
    size_t admitted,const size_t *map,size_t prefix)
{
    if(!guards)return NULL;
    if(admitted>SIZE_MAX-prefix-2)return NULL;
    unsigned char *out=calloc(prefix+admitted+2,1);if(!out)return NULL;
    for(size_t i=0;i<original;++i)if(guards[i]) {
        size_t start=map?map[i]:i,end=map?map[i+1]:i+1;
        if(end>admitted)end=admitted;
        if(start<end)memset(out+prefix+start,1,end-start);
    }
    return out;
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
static void reset_equalizer_filters(struct reloq_eci_engine *e)
{
    static const int frequencies[RELOQ_EQ_FILTER_COUNT]={125,500,1200,3000,6000};
    for(int i=0;i<RELOQ_EQ_FILTER_COUNT;++i){
        e->eq_filters[i][0]=0;e->eq_filters[i][1]=frequencies[i];
        e->eq_filters[i][2]=12;e->eq_filters[i][3]=100;
    }
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
static int reloq_fa_prepare_english_worker(struct reloq_eci_engine *);

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
    struct reloq_equalizer equalizer;
    int equalizer_initialized;
    size_t pcm_period, pcm_phase, pcm_used, pcm_early;
};

static int send_pcm(void *context, const int16_t *pcm, size_t samples);
static int flush_pcm(struct pcm_bridge *bridge);
static int sample_hz(int code);

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
    struct reloq_german_generic_settings *settings,int native_german)
{
    for(size_t at=0;at<length;) {
        if(text[at]==' '||text[at]=='\t'||text[at]=='\r'||text[at]=='\n'){++at;continue;}
        struct reloq_inline_tag tag=reloq_inline_tag_read(text+at,length-at,0);
        if(!tag.kind || tag.kind==RELOQ_TAG_SPR)break;
        switch(tag.kind) {
        case RELOQ_TAG_PITCH:settings->native_pitch_plus_one=tag.value+1;break;
        case RELOQ_TAG_SPEED:
            if(native_german)settings->native_speed=tag.value?tag.value:-1;
            break;
        case RELOQ_TAG_GENDER:settings->gender=tag.value;break;
        case RELOQ_TAG_HEAD_SIZE:settings->head_size_plus_one=tag.value+1;break;
        case RELOQ_TAG_ROUGHNESS:settings->roughness=tag.value;break;
        case RELOQ_TAG_BREATHINESS:settings->breathiness=tag.value;break;
        default:break;
        }
        at+=tag.length;
    }
}

/* All AddText fragments share one ECI transaction and one acoustic clock.
 * Complete admitted German phrases can begin rendering during analysis;
 * indices are delivered at PCM word boundaries, not during preflight. */
extern int reloq61_worker_lexicon_overlay(void *,const struct reloq_lexicon *,uint64_t);
static int synthesize_generic(struct reloq_eci_engine *e,
                              const unsigned char *phrase, size_t length,
                              size_t original_length,
                              const struct index_mark *marks,
                              size_t mark_count,
                              const struct pitch_mark *pitches,
                              size_t pitch_count, unsigned int epoch,int admitted,
                              const unsigned char *guards)
{
    (void)original_length;
    struct preflight_cancel cancel_check = {e, epoch};
    if(e->language==RELOQ_ECI_GERMAN&&reloq61_worker_lexicon_overlay(e->rule_worker,
        e->lexicon.rows?&e->lexicon.view:NULL,e->lexicon_version))return RELOQ_ECI_MEMORY;
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
        e->native_pitch_active?((e->real_voice_flags[0]&(1u<<2))?
            RELOQ_PITCH_HZ_OFFSET+e->real_voice_values[0][2]:e->native_pitch)+1:0,
        e->native_inflection_active?e->native_inflection+1:0,
        e->pause_mode,e->head_size+1,e->roughness,e->breathiness,e->gender};
    if(e->process_tags)leading_models(phrase,length,&settings,e->language==RELOQ_ECI_GERMAN);
    if(settings.gender!=e->gender || settings.head_size_plus_one!=e->head_size+1) {
        int head=settings.head_size_plus_one-1,gender=settings.gender;
        long source=gender?(head<30?5:3):4;
        if(e->sample_rate_code==0 && head<=(gender?40:30) && !(gender && head<30))source=7;
        unsigned reference=sample_hz(e->sample_rate_code)>=16000?(head<=(gender?40:30)?4u:5u):0;
        if(reloq_session_source_levels(e->text_session,source,gender?-3:-5,6,gender?-7:0,reference))reset=1;
    }
    struct reloq_wordseg_options wordseg=reloq_wordseg_scope((struct reloq_wordseg_options){
        e->word_segmentation,e->camel_case,
        reloq61_worker_dictionary_handle(e->rule_worker),e->dictionary_disabled,guards,length});
    int rendered=reset?RELOQ_DE_UTTERANCE_INVALID:reloq_language_rules_render(e->language,phrase,length,e->process_tags||admitted,
        frame_volume?(e->native_volume_active?e->native_volume:e->generic_volume):100,
        e->native_wpm,e->original_acoustic_clock==1||(e->original_acoustic_clock==2&&
            (e->language==0x20000||e->language==0x120000||e->language==0x130000)),&settings,english_indices,mark_count,english_pitch,pitch_count,e->text_session,
        send_pcm,send_generic_index,&bridge);
    reloq_wordseg_scope(wordseg);
    free(english_indices);free(english_pitch);
    if(preflight_cancelled(&cancel_check)||reloq_session_cancelled(e->text_session))return RELOQ_ECI_CANCELLED;
    if(rendered==RELOQ_DE_UTTERANCE_OK)return RELOQ_ECI_OK;
    e->failure_stage=RELOQ_ECI_STAGE_ACOUSTIC;e->failure_utf8_offset=-1;
    return rendered==RELOQ_DE_UTTERANCE_MEMORY?RELOQ_ECI_MEMORY:
        bridge.callback_refused?RELOQ_ECI_CALLBACK:RELOQ_ECI_UNSUPPORTED_TEXT;

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

extern const char *ECICALL es_engsynDictLookup(void *,int,const char *);
/* Dictionary substitutions run after Unicode admission. Preserve exact
 * main/abbreviation entries and the case-folded root entry for v/V. */
static unsigned aragonese_dictionary_letters(void *worker,int language)
{
    if(language!=0x130000)return 0;
    void *dictionary=reloq61_worker_dictionary_handle(worker);
    if(!dictionary)return 0;
    unsigned result=0;
    if(es_engsynDictLookup(dictionary,1,"v"))result=3;
    if(es_engsynDictLookup(dictionary,0,"v")||es_engsynDictLookup(dictionary,2,"v"))result|=1;
    if(es_engsynDictLookup(dictionary,0,"V")||es_engsynDictLookup(dictionary,2,"V"))result|=2;
    return result;
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
    if (!reloq61_language_supported(base)||dictionary_language_slot(base)>=ECI_LANGUAGE_COUNT) return NULL;
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
    /* The accepted 1.1 listening profile is the common ECI default. */
    e->high_rate_profile=3;
    e->synth_mode=eciGetDefaultParam(RELOQ_ECI_SYNTH_MODE);
    e->input_type=e->process_tags=eciGetDefaultParam(RELOQ_ECI_INPUT_TYPE);
    e->text_mode=eciGetDefaultParam(2);
    e->dictionary_disabled=eciGetDefaultParam(RELOQ_ECI_DICTIONARY);
    e->real_world_units=eciGetDefaultParam(RELOQ_ECI_REAL_WORLD_UNITS);
    e->head_size=50;
    e->german_glide_opening=85;
    e->german_glide_backness=45;
    e->german_glide_softness=45;
    e->eq_low=e->eq_mid=e->eq_high=12;
    reset_equalizer_filters(e);
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
    /* As with this handle, the owner has joined rendering and Stop before
     * deletion. No callback or separately owned handle escapes here. */
    if(e->persian_us_pilot)(void)eciDelete(e->persian_us_pilot);
    reloq61_worker_free(e->persian_english_worker);
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
    if (buffer) memcpy(buffer, "1.2.2", sizeof "1.2.2");
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

int ECICALL eciPersianProjectTextUtf8(const char *text,size_t bytes,int mode,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    if(mode==0)return reloq_fa_direct_project(text,bytes,source_offsets,is_index,event_count,
                                             output,target_offsets,error_offset);
    if(mode==1)return reloq_fa_pinglish_project(text,bytes,source_offsets,is_index,event_count,
                                                output,target_offsets,error_offset);
    if(output)*output=NULL;
    if(error_offset)*error_offset=0;
    return -1;
}

int ECICALL eciPersianProjectRunsUtf8(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    return reloq_fa_project_runs(text,bytes,mode,run_ends,run_modes,run_count,
                                source_offsets,is_index,event_count,output,target_offsets,error_offset);
}

int ECICALL eciPersianProjectTextCompleteUtf8(const char *text,size_t bytes,int mode,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    if(mode==0)return reloq_fa_direct_project_complete(text,bytes,source_offsets,is_index,event_count,
                                                      output,target_offsets,error_offset);
    if(mode==1)return reloq_fa_pinglish_project_complete(text,bytes,source_offsets,is_index,event_count,
                                                         output,target_offsets,error_offset);
    if(output)*output=NULL;
    if(error_offset)*error_offset=0;
    return -1;
}

int ECICALL eciPersianProjectRunsCompleteUtf8(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    return reloq_fa_project_runs_complete(text,bytes,mode,run_ends,run_modes,run_count,
                                         source_offsets,is_index,event_count,output,target_offsets,error_offset);
}

int ECICALL eciPersianAnalyzeRunsUtf8(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    struct reloq_fa_plan_event **events,size_t *count,size_t *target_events,size_t *error_offset)
{
    return reloq_fa_analyze_runs(text,bytes,mode,run_ends,run_modes,run_count,
                                source_offsets,is_index,event_count,events,count,target_events,error_offset);
}

int ECICALL eciPersianAnalyzeRunsCompleteUtf8(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    struct reloq_fa_plan_event **events,size_t *count,size_t *target_events,size_t *error_offset)
{
    return reloq_fa_analyze_runs_complete(text,bytes,mode,run_ends,run_modes,run_count,
                                source_offsets,is_index,event_count,events,count,target_events,error_offset);
}

int ECICALL eciPersianProjectUtf8(const char *text,size_t bytes,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    return eciPersianProjectTextUtf8(text,bytes,0,source_offsets,is_index,event_count,
                                     output,target_offsets,error_offset);
}

int ECICALL eciPersianProjectCompleteUtf8(const char *text,size_t bytes,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    return reloq_fa_direct_project_complete(text,bytes,source_offsets,is_index,event_count,
                                           output,target_offsets,error_offset);
}

int ECICALL eciPersianPinglishProjectUtf8(const char *text,size_t bytes,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    return eciPersianProjectTextUtf8(text,bytes,1,source_offsets,is_index,event_count,
                                     output,target_offsets,error_offset);
}

void ECICALL eciPersianRelease(void *pointer)
{reloq_fa_direct_free(pointer);}

int ECICALL eciPersianIgnorableTextCharacter(uint32_t codepoint)
{return reloq_fa_ignorable_text_character(codepoint);}

int ECICALL eciUnicodeIsEmoji(uint32_t codepoint)
{return reloq_unicode_is_emoji(codepoint);}
int ECICALL eciUnicodeGreekName(uint32_t codepoint,char *output,size_t capacity)
{return reloq_unicode_greek_name_english(codepoint,output,capacity);}

int ECICALL eciPersianTranslateUtf8(const char *text,size_t bytes,char **output,
    size_t **source_map,size_t *error_offset)
{return reloq_fa_direct_text(text,bytes,output,source_map,error_offset);}

int ECICALL eciPersianStressUtf8(const char *word,size_t bytes,char *tag,size_t capacity)
{
    char *normalized=NULL;size_t *map=NULL,error=0;
    int status=reloq_fa_direct_normalize(word,bytes,&normalized,&map,&error);
    if(status)return status;
    status=reloq_fa_direct_stress(normalized,strlen(normalized),tag,capacity);
    free(normalized);free(map);return status;
}

int ECICALL eciPersianNumber(const char *text,size_t bytes,size_t *consumed,
    char *output,size_t capacity)
{return reloq_fa_direct_number(text,bytes,consumed,output,capacity);}

int ECICALL eciPersianNumberEvents(const char *text,size_t bytes,size_t *consumed,
    struct reloq_fa_number_event *events,size_t capacity,size_t *count)
{
    if(consumed)*consumed=0;
    if(count)*count=0;
    if(!text || !bytes || !consumed || !count || (!events && capacity))return -1;
    char *normalized=NULL;size_t *map=NULL,error=0;
    int status=reloq_fa_direct_normalize_text(text,bytes,&normalized,&map,&error);
    if(status)return status;
    size_t length=strlen(normalized),offset=0,used=0,number_count=0;
    int sign=-1;
    if(length && (normalized[0]=='+' || normalized[0]=='-')) {
        sign=normalized[0]=='+';offset=1;
    } else if(length>=3 && !memcmp(normalized,"\xe2\x88\x92",3)) {
        sign=0;offset=3;
    }
    struct reloq_fa_number_event result[49];
    status=reloq_fa_number_events(normalized+offset,length-offset,&used,
                                  result+(sign>=0),48,&number_count);
    if(!status) {
        size_t total=number_count+(sign>=0),source_end=0;
        if(events && capacity<total)status=-2;
        else {
            if(sign>=0)result[0]=(struct reloq_fa_number_event){RELOQ_FA_NUMBER_SIGN,(uint32_t)sign,0,0,0};
            while(source_end<bytes && map[source_end]<offset+used)++source_end;
            if(map[source_end]!=offset+used)status=-1;
            else {
                if(events)memcpy(events,result,total*sizeof *events);
                *count=total;*consumed=source_end;
            }
        }
    }
    free(normalized);free(map);return status;
}

const char *ECICALL eciPersianLetter(uint32_t codepoint)
{return reloq_fa_direct_letter(codepoint);}

int ECICALL eciPersianCharacter(uint32_t codepoint,char *output,size_t capacity)
{return reloq_fa_direct_character(codepoint,output,capacity);}

int ECICALL eciPersianCharacterResolve(uint32_t codepoint,char *output,size_t capacity,int *kind)
{return reloq_fa_character_resolve(codepoint,output,capacity,kind);}

int ECICALL eciPersianPinglishWord(const char *word,size_t bytes,char *output,size_t capacity)
{return reloq_fa_pinglish_word(word,bytes,output,capacity);}

const struct reloq_fa_phone_info *ECICALL eciPersianPhoneInventory(size_t *count)
{return reloq_fa_phone_inventory(count);}

int ECICALL eciPersianPhonesFromLegacy(const char *text,size_t bytes,
    struct reloq_fa_phone *phones,size_t capacity,size_t *count,size_t *error_offset)
{return reloq_fa_phones_from_legacy(text,bytes,phones,capacity,count,error_offset);}

int ECICALL eciPersianPhonesToLegacy(const struct reloq_fa_phone *phones,size_t count,
    char *output,size_t capacity,size_t *bytes,size_t *error_index)
{return reloq_fa_phones_to_legacy(phones,count,output,capacity,bytes,error_index);}

int ECICALL eciPersianInputToLegacy(const char *text,size_t bytes,char *output,
    size_t capacity,size_t *written,size_t *error_offset)
{return reloq_fa_input_to_legacy(text,bytes,output,capacity,written,error_offset);}

int ECICALL eciPersianInputChecked(const char *text,size_t bytes,char *output,
    size_t capacity,size_t *written,size_t *error_offset)
{return reloq_fa_input_checked(text,bytes,output,capacity,written,error_offset);}

int ECICALL eciPersianWordPhonesUtf8(const char *word,size_t bytes,int pinglish,
    struct reloq_fa_phone *phones,size_t capacity,size_t *count,size_t *error_offset)
{
    if(count)*count=0;
    if(error_offset)*error_offset=0;
    if(!word || !bytes || !count || (!phones && capacity) ||
       (pinglish!=0 && pinglish!=1))return -1;
    char latin_tag[8193],*owned_tag=NULL;size_t *map=NULL,source_error=0;
    const char *tag;
    int status;
    if(pinglish) {
        status=reloq_fa_pinglish_word(word,bytes,latin_tag,sizeof latin_tag);
        if(status!=1)return status==0?1:status;
        tag=latin_tag;
    } else {
        status=reloq_fa_direct_text(word,bytes,&owned_tag,&map,&source_error);
        if(status) {
            if(error_offset)*error_offset=source_error;
            return status>0?1:status;
        }
        tag=owned_tag;
    }
    size_t bridge_error=0;
    status=reloq_fa_phones_from_legacy(tag,strlen(tag),phones,capacity,count,&bridge_error);
    free(owned_tag);free(map);
    /* A valid text result can contain multiple groups or controls: these are
       currently outside the single-word adapter, not invalid UTF-8 input. */
    return status==-1?1:status;
}

int ECICALL eciPersianAcousticScope(int active)
{return reloq_fa_acoustic_scope(active);}

int ECICALL eciPersianTuningScope(int active)
{return reloq_fa_tuning_scope(active);}

int ECICALL eciPersianNativeOnsets(const uint32_t *words,size_t count)
{return reloq_fa_native_onsets(words,count);}

int ECICALL eciPersianNativeGeminates(const uint32_t *pairs,size_t count)
{return reloq_fa_native_geminates(pairs,count);}

int ECICALL eciAddTextUtf8(ECIHand handle,const void *text,size_t bytes,int mode)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy))return 0;
    if((!text&&bytes)||(mode<0||(mode&~7))||bytes==SIZE_MAX) {
        eci_error(e,RELOQ_ECI_INVALID);return 0;
    }
    unsigned char *copy=malloc(bytes+1);
    if(!copy){eci_error(e,RELOQ_ECI_MEMORY);return 0;}
    if(bytes)memcpy(copy,text,bytes);
    copy[bytes]=0;
    if(append_item(e,(struct queued_item){.utf8=copy,.length=bytes,
       .voice=capture_voice(e),.unicode_input=1,.character_mode=mode&1,
       .segmentation_guarded=!!(mode&2),.source_in_context=!!(mode&4)})) {
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
    /* The accepted C sound is now the single production tuning. Keep the
     * legacy parameter value for callers which still set/query it, but do
     * not let stale saved A/B settings change speech in any ECI host. */
    (void)reloq_session_high_rate_profile(e->text_session,3);
    (void)reloq_session_mellow_mode(e->text_session,e->mellow_mode);
    (void)reloq_session_text_loudness(e->text_session,
        e->voice_mode==RELOQ_ECI_VOICE_GENERIC);
    struct reloq61_text_options previous_options=reloq61_text_options_scope(
        (struct reloq61_text_options){e->dictionary_disabled,e->phrase_prediction,e->original_questions});
    struct reloq61_glide_options previous_glide=reloq61_glide_options_scope(
        (struct reloq61_glide_options){e->german_glide_opening,e->german_glide_backness,e->german_glide_softness});
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
    unsigned char *modes=NULL,*guards=NULL;size_t *source_map=NULL;int have_guards=0,source_in_context=0;
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
            else if(memcmp(last_voice.values,e->queue[i].voice.values,sizeof last_voice.values) ||
                    last_voice.pitch_hz!=e->queue[i].voice.pitch_hz ||
                    last_voice.native_wpm!=e->queue[i].voice.native_wpm) {
                controls[control_count++]=(struct queued_control){text_bytes,e->queue[i].voice};
                last_voice=e->queue[i].voice;
            }
        }
        text_bytes += e->queue[i].length;
        unicode_input|=e->queue[i].unicode_input;
        have_guards|=e->queue[i].segmentation_guarded;
        if(e->queue[i].length)source_in_context|=e->queue[i].source_in_context;
    }
    if(have_voice) {
        apply_voice(e,&initial_voice);
        if(initial_voice.values[0]!=saved_voice.values[0]||initial_voice.values[1]!=saved_voice.values[1]) {
            struct reloq_session *next=make_text_session(e->gender,e->head_size,e->sample_rate_code);
            if(!next){result=RELOQ_ECI_MEMORY;goto done;}
            (void)reloq_session_high_rate_profile(next,3);
            (void)reloq_session_mellow_mode(next,e->mellow_mode);
            (void)reloq_session_text_loudness(next,1);
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
    if(have_guards&&text_bytes) {
        guards=calloc(text_bytes+2,1);
        if(!guards){result=RELOQ_ECI_MEMORY;goto done;}
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
            if(guards)memset(guards+offset,item->segmentation_guarded,item->length);
            offset += item->length;
        }
    }
    if(text_bytes && !control_count && e->process_tags && e->voice_mode==RELOQ_ECI_VOICE_GENERIC &&
       inline_language_commands(phrase,text_bytes,modes)) {
        result=synthesize_languages(e,phrase,text_bytes,modes,guards,unicode_input,
            marks,mark_count,pitches,pitch_count,before);
        goto done;
    }
    if(unicode_input && text_bytes && modes) {
        size_t at=0;uint32_t cp=0;
        /* An isolated Unicode scalar is a character request, even when the
         * host did not send a spelling flag. Count the complete queued text,
         * not the fragments separated by source indexes or voice controls. */
        if(!source_in_context && (!guards || !guards[0]) && reloq_utf8_next(phrase,text_bytes,&at,&cp)==1 &&
           at==text_bytes)
            memset(modes,1,text_bytes);
    }
    if((unicode_input||control_count)&&text_bytes) {
        unsigned char *normalized=NULL;size_t count=0;source_bytes=text_bytes;
        int r=reloq_unicode_admit_language_masked(phrase,text_bytes,modes,e->language,
            e->process_tags,aragonese_dictionary_letters(e->rule_worker,e->language),
            &normalized,&count,&source_map);
        if(r){result=r==-2?RELOQ_ECI_MEMORY:RELOQ_ECI_INVALID;
            e->failure_stage=RELOQ_ECI_STAGE_TEXT;goto done;}
        for(size_t i=0;i<mark_count;++i)marks[i].offset=source_map[marks[i].offset];
        for(size_t i=0;i<pitch_count;++i)pitches[i].offset=source_map[pitches[i].offset];
        if(guards){unsigned char *mapped=project_guards(guards,text_bytes,count,source_map,0);
            if(!mapped){free(normalized);result=RELOQ_ECI_MEMORY;goto done;}
            free(guards);guards=mapped;}
        free(phrase);phrase=normalized;text_bytes=count;
    }
    if(control_count) {
        if(insert_voice_controls(&phrase,&text_bytes,controls,control_count,initial_voice,
            source_map,marks,mark_count,pitches,pitch_count,&guards)) {result=RELOQ_ECI_MEMORY;goto done;}
        e->process_tags=1;
        /* Diagnostics below refer to the admitted stream, not the temporary
         * annotations. Public index/pitch positions have already moved. */
        free(source_map);source_map=NULL;
        if(inline_language_commands(phrase,text_bytes,NULL)) {
            result=synthesize_languages(e,phrase,text_bytes,NULL,guards,0,marks,mark_count,pitches,pitch_count,before);
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
                                        pitch_count, before,unicode_input,guards);
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
    free(source_map);free(modes);free(guards);
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
    reloq61_glide_options_scope(previous_glide);
    busy_end(&e->busy);
    return result == RELOQ_ECI_OK;
}

int ECICALL eciPersianSpeakNativeUtf8(ECIHand handle,const char *text,size_t bytes,int mode,
    const size_t *source_offsets,const int *index_values,size_t index_count,size_t *error_offset)
{
    struct reloq_eci_engine *e=handle;
    if(error_offset)*error_offset=0;
    if(!e || epoch_get(&e->busy) || e->queued)return -1;
    return reloq_fa_speak_native(handle,text,bytes,mode,source_offsets,index_values,index_count,error_offset);
}

int ECICALL eciPersianSpeakNativeRunsUtf8(ECIHand handle,const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const int *index_values,size_t index_count,size_t *error_offset)
{
    struct reloq_eci_engine *e=handle;
    if(error_offset)*error_offset=0;
    if(!e || !text || !error_offset || bytes==SIZE_MAX || (mode!=0 && mode!=1) ||
       (index_count && (!source_offsets || !index_values)) || index_count>4096 ||
       epoch_get(&e->busy) || e->queued || e->voice_mode!=RELOQ_ECI_VOICE_GENERIC ||
       (e->language_dialect&~0xff00)!=0x90000 || memchr(text,0,bytes))return -1;
    for(size_t i=0;i<index_count;++i)
        if(source_offsets[i]>bytes || index_values[i]<0 ||
           (i && source_offsets[i]<source_offsets[i-1]))return -1;
    unsigned before=epoch_get(&e->stop_epoch);
    struct reloq_fa_prepared prepared={0};
    int status=reloq_fa_prepare_native_runs(text,bytes,mode,run_ends,run_modes,run_count,
        source_offsets,index_count,error_offset,&prepared,NULL,1);
    if(!status && epoch_get(&e->stop_epoch)!=before)status=-4;
    if(!status)status=reloq_fa_speak_prepared(handle,&prepared,prepared.positions,index_values,NULL,index_count);
    if(epoch_get(&e->stop_epoch)!=before)status=-4;
    reloq_fa_prepared_clear(&prepared);
    return status;
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
    struct reloq_eci_engine *persian_us_pilot=e->persian_us_pilot;
    unlock_session(&e->session_lock);
    if(persian_us_pilot)(void)eciStop(persian_us_pilot);
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
    if(parameter>=RELOQ_ECI_EQ_FILTER_BASE&&parameter<RELOQ_ECI_EQ_FILTER_BASE+RELOQ_EQ_FILTER_COUNT*RELOQ_ECI_EQ_FILTER_STRIDE){
        int eq_index=parameter-RELOQ_ECI_EQ_FILTER_BASE;
        return e->eq_filters[eq_index/RELOQ_ECI_EQ_FILTER_STRIDE][eq_index%RELOQ_ECI_EQ_FILTER_STRIDE];
    }
    switch (parameter) {
    case RELOQ_ECI_SYNTH_MODE: return e->synth_mode;
    case RELOQ_ECI_VOICE_MODE: return e->voice_mode;
    case RELOQ_ECI_GENERIC_RATE: return e->generic_rate;
    case RELOQ_ECI_GENERIC_PITCH: return e->generic_pitch;
    case RELOQ_ECI_GENERIC_VOLUME: return e->generic_volume;
    case RELOQ_ECI_NATIVE_SPEED: return e->native_speed;
    case RELOQ_ECI_NATIVE_WPM: return e->native_wpm;
    case RELOQ_ECI_NATIVE_PITCH: return e->native_pitch;
    case RELOQ_ECI_NATIVE_INFLECTION: return e->native_inflection;
    case RELOQ_ECI_NATIVE_VOLUME: return e->native_volume;
    case RELOQ_ECI_INPUT_TYPE: return e->input_type;
    case RELOQ_ECI_REAL_WORLD_UNITS:return e->real_world_units;
    case RELOQ_ECI_PROCESS_TAGS: return e->process_tags;
    case RELOQ_ECI_DICTIONARY: return e->dictionary_disabled;
    case RELOQ_ECI_PHRASE_PREDICTION: return e->phrase_prediction;
    case RELOQ_ECI_ORIGINAL_QUESTIONS: return e->original_questions;
    case RELOQ_ECI_MELLOW_MODE: return e->mellow_mode;
    case RELOQ_ECI_ORIGINAL_ACOUSTIC_CLOCK: return e->original_acoustic_clock;
    case RELOQ_ECI_WORD_SEGMENTATION: return e->word_segmentation;
    case RELOQ_ECI_CAMEL_CASE: return e->camel_case;
    case RELOQ_ECI_UTF8_INPUT_MODES: return 7;
    case RELOQ_ECI_EQ_LOW: return e->eq_low;
    case RELOQ_ECI_EQ_MID: return e->eq_mid;
    case RELOQ_ECI_EQ_HIGH: return e->eq_high;
    case RELOQ_ECI_PAUSE_MODE: return e->pause_mode;
    case RELOQ_ECI_HEAD_SIZE: return e->head_size;
    case RELOQ_ECI_ROUGHNESS: return e->roughness;
    case RELOQ_ECI_BREATHINESS: return e->breathiness;
    case RELOQ_ECI_GENDER: return e->gender;
    case RELOQ_ECI_HIGH_RATE_PROFILE: return e->high_rate_profile;
    case RELOQ_ECI_GERMAN_GLIDE_OPENING: return e->german_glide_opening;
    case RELOQ_ECI_GERMAN_GLIDE_BACKNESS: return e->german_glide_backness;
    case RELOQ_ECI_GERMAN_GLIDE_SOFTNESS: return e->german_glide_softness;
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
    if(parameter>=RELOQ_ECI_EQ_FILTER_BASE&&parameter<RELOQ_ECI_EQ_FILTER_BASE+RELOQ_EQ_FILTER_COUNT*RELOQ_ECI_EQ_FILTER_STRIDE){
        int eq_index=parameter-RELOQ_ECI_EQ_FILTER_BASE;
        int field=eq_index%RELOQ_ECI_EQ_FILTER_STRIDE;
        if((field==0&&(value<0||value>1))||(field==1&&(value<1||value>24000))||
           (field==2&&(value<0||value>24))||(field==3&&(value<10||value>400)))return eci_refuse(e,-1);
        e->eq_filters[eq_index/RELOQ_ECI_EQ_FILTER_STRIDE][field]=value;
        eci_error(e,RELOQ_ECI_OK);return old;
    }
    switch (parameter) {
    case RELOQ_ECI_GERMAN_GLIDE_OPENING:
        if(value<0 || value>100)return eci_refuse(e,-1);
        e->german_glide_opening=value;break;
    case RELOQ_ECI_GERMAN_GLIDE_BACKNESS:
        if(value<0 || value>100)return eci_refuse(e,-1);
        e->german_glide_backness=value;break;
    case RELOQ_ECI_GERMAN_GLIDE_SOFTNESS:
        if(value<0 || value>100)return eci_refuse(e,-1);
        e->german_glide_softness=value;break;
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
    case RELOQ_ECI_NATIVE_WPM:
        if(value<0 || value>6000)return eci_refuse(e,-1);
        e->native_wpm=value;break;
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
    case RELOQ_ECI_ORIGINAL_QUESTIONS:
        if(value!=0 && value!=1)return eci_refuse(e,-1);
        e->original_questions=value;break;
    case RELOQ_ECI_MELLOW_MODE:
        if(value!=0 && value!=1)return eci_refuse(e,-1);
        e->mellow_mode=value;break;
    case RELOQ_ECI_ORIGINAL_ACOUSTIC_CLOCK:
        if(value<0 || value>2)return eci_refuse(e,-1);
        e->original_acoustic_clock=value;break;
    case RELOQ_ECI_WORD_SEGMENTATION:
        if(value!=0 && value!=1)return eci_refuse(e,-1);
        e->word_segmentation=value;break;
    case RELOQ_ECI_CAMEL_CASE:
        if(value!=0 && value!=1)return eci_refuse(e,-1);
        e->camel_case=value;break;
    case RELOQ_ECI_EQ_LOW:
    case RELOQ_ECI_EQ_MID:
    case RELOQ_ECI_EQ_HIGH:
        if(value<0 || value>24)return eci_refuse(e,-1);
        if(parameter==RELOQ_ECI_EQ_LOW)e->eq_low=value;
        else if(parameter==RELOQ_ECI_EQ_MID)e->eq_mid=value;
        else e->eq_high=value;
        break;
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

struct persian_us_probe {
    ECIHand handle;
    unsigned epoch;
    reloq_fa_proxy_sink sink;
    void *sink_context;
    char buffer[8192];
    int failed;
};
static int ECICALL persian_us_phonemes(ECIHand handle,int message,int count,void *context)
{
    struct persian_us_probe *p=context;
    struct reloq_eci_engine *e=handle;
    if(epoch_get(&e->stop_epoch)!=p->epoch)return eciDataAbort;
    if(message==eciPhonemeBuffer) {
        if(count<0 || (size_t)count>sizeof p->buffer){p->failed=-1;return eciDataAbort;}
        int result=p->sink(p->sink_context,p->buffer,(size_t)count);
        if(result){p->failed=result==-2?-2:-1;return eciDataAbort;}
    }
    return eciDataProcessed;
}
static int persian_us_probe_word(void *context,const char *text,size_t length,
    reloq_fa_proxy_sink sink,void *sink_context)
{
    struct persian_us_probe *p=context;
    struct reloq_eci_engine *e=p->handle;
    if(epoch_get(&e->stop_epoch)!=p->epoch)return -1;
    p->sink=sink;p->sink_context=sink_context;p->failed=0;
    if(!eciAddTextUtf8(p->handle,text,length,0) ||
       !eciGeneratePhonemes(p->handle,(int)sizeof p->buffer,p->buffer)) {
        (void)eciClearInput(p->handle);
        return p->failed?p->failed:-1;
    }
    return 0;
}
int ECICALL eciPersianUsProxyUtf8(ECIHand handle,const char *text,size_t bytes,
    struct reloq_fa_us_proxy *result)
{
    if(result)*result=(struct reloq_fa_us_proxy){0};
    struct reloq_eci_engine *e=handle;
    if(!e || !result || !text || bytes==SIZE_MAX || e->language!=0x10000 ||
       epoch_get(&e->busy) || e->queued)return 1;
    size_t pos=0;uint32_t scalar;
    while(pos<bytes)if(reloq_utf8_next((const unsigned char *)text,bytes,&pos,&scalar)!=1 || !scalar)return 1;
    struct persian_us_probe p={.handle=handle,.epoch=epoch_get(&e->stop_epoch)};
    ECICallback previous=e->callback;void *previous_data=e->callback_data;
    int mode=e->synth_mode,tags=e->process_tags;
    if(eciSetParam(handle,RELOQ_ECI_SYNTH_MODE,1)<0 ||
       eciSetParam(handle,RELOQ_ECI_PROCESS_TAGS,1)<0) {
        (void)eciSetParam(handle,RELOQ_ECI_SYNTH_MODE,mode);
        return -1;
    }
    eciRegisterCallback(handle,persian_us_phonemes,&p);
    int status=reloq_fa_us_proxy_make_reusing(text,bytes,persian_us_probe_word,&p,result);
    if(epoch_get(&e->stop_epoch)!=p.epoch && !status){reloq_fa_us_proxy_clear(result);status=-1;}
    (void)eciClearInput(handle);
    (void)eciSetParam(handle,RELOQ_ECI_SYNTH_MODE,mode);
    (void)eciSetParam(handle,RELOQ_ECI_PROCESS_TAGS,tags);
    eciRegisterCallback(handle,previous,previous_data);
    return status;
}
void ECICALL eciPersianUsProxyRelease(struct reloq_fa_us_proxy *result)
{reloq_fa_us_proxy_clear(result);}


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
    park_eci_dictionary_worker(e);free(e->dictionary_path);
    e->language_dictionaries[dictionary_language_slot(e->language)]=NULL;
    e->rule_worker=worker;e->dictionary_path=path;e->active_dictionary=NULL;eci_error(e,RELOQ_ECI_OK);return 1;
}

int ECICALL eciLoadLexicon(ECIHand handle, const char *utf8_path)
{
    struct reloq_eci_engine *e = handle;
    if (!e || epoch_get(&e->busy)) return eci_refuse(e,0);
    if (!utf8_path) {
        reloq_german_lexicon_file_clear(&e->lexicon);
        ++e->lexicon_version;
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
    if(!loaded)++e->lexicon_version;
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
    int status=reloq_fa_capture_pcm_storage() ?
        (reloq_fa_capture_append_pcm(e->output,count) ? RELOQ_ECI_MEMORY : RELOQ_ECI_OK) :
        notify_callback(e,RELOQ_ECI_WAVEFORM_BUFFER,(int)count);
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
        if(!b->equalizer_initialized) {
            struct reloq_equalizer_peak peaks[RELOQ_EQ_FILTER_COUNT];
            for(int i=0;i<RELOQ_EQ_FILTER_COUNT;++i){
                peaks[i]=(struct reloq_equalizer_peak){e->eq_filters[i][0],e->eq_filters[i][1],
                    e->eq_filters[i][2]-12,e->eq_filters[i][3]};
            }
            if(reloq_equalizer_init_extended(&b->equalizer,sample_hz(e->sample_rate_code),
                e->eq_low-12,e->eq_mid-12,e->eq_high-12,peaks))return -1;
            b->equalizer_initialized=1;
        }
        reloq_equalizer_process(&b->equalizer,e->output,count);
        int status=reloq_fa_capture_pcm_storage() ?
            (reloq_fa_capture_append_pcm(e->output,count) ? RELOQ_ECI_MEMORY : RELOQ_ECI_OK) :
            notify_callback(e,RELOQ_ECI_WAVEFORM_BUFFER,(int)count);
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

#include "reloq_persian_replay.inc"

int ECICALL eciPersianCaptureBegin(size_t maximum)
{
    int status=reloq_fa_capture_begin(maximum);
    if(!status) {
        reloq61_syllable_trace_set(reloq_fa_capture_syllable,NULL);
        reloq61_phone_identity_trace_set(reloq_fa_capture_phone_identity,NULL);
    }
    return status;
}
const struct reloq_fa_frame *ECICALL eciPersianCaptureEnd(size_t *count,int *failed)
{reloq61_syllable_trace_set(NULL,NULL);reloq61_phone_identity_trace_set(NULL,NULL);return reloq_fa_capture_end(count,failed);}
const struct reloq_fa_frame *ECICALL eciPersianCaptureCurrent(size_t *count,int *failed)
{return reloq_fa_capture_current(count,failed);}
const struct reloq_fa_syllable *ECICALL eciPersianCaptureSyllables(size_t *count,int *failed)
{return reloq_fa_capture_syllables(count,failed);}
void ECICALL eciPersianCaptureRelease(void)
{reloq61_syllable_trace_set(NULL,NULL);reloq61_phone_identity_trace_set(NULL,NULL);reloq_fa_capture_release();}
const uint32_t *ECICALL eciPersianCapturePhoneIdentities(size_t *count,int *failed)
{return reloq_fa_capture_phone_identities(count,failed);}
int ECICALL eciPersianAlignNucleusClocks(const struct reloq_fa_plan_event *events,size_t event_count,
    const struct reloq_fa_syllable *syllables,size_t syllable_count,
    const struct reloq_fa_syllable *phones,size_t phone_count,
    const uint32_t *provider_slots,size_t slot_count,
    struct reloq_fa_nucleus_clock *output,size_t capacity,size_t *written)
{
    return reloq_fa_align_nucleus_clocks(events,event_count,syllables,syllable_count,
                                        phones,phone_count,provider_slots,slot_count,output,capacity,written);
}
int ECICALL eciPersianEarlyCursorCreate(const struct reloq_fa_plan_event *events,size_t event_count,
    struct reloq_fa_early_cursor **result)
{return reloq_fa_early_cursor_create(events,event_count,result);}
int ECICALL eciPersianEarlyCursorNext(struct reloq_fa_early_cursor *cursor,
    const struct reloq_fa_syllable *nucleus,uint32_t identity,struct reloq_fa_early_nucleus *output)
{return reloq_fa_early_cursor_next(cursor,nucleus,identity,output);}
int ECICALL eciPersianEarlyCursorFinish(const struct reloq_fa_early_cursor *cursor)
{return reloq_fa_early_cursor_finish(cursor);}
void ECICALL eciPersianEarlyCursorRelease(struct reloq_fa_early_cursor *cursor)
{reloq_fa_early_cursor_release(cursor);}
int ECICALL eciPersianAlignEarlyNuclei(const struct reloq_fa_plan_event *events,size_t event_count,
    const struct reloq_fa_syllable *nuclei,size_t nucleus_count,
    const uint32_t *identities,size_t identity_count,
    struct reloq_fa_early_nucleus *output,size_t capacity,size_t *written)
{
    return reloq_fa_align_early_nuclei(events,event_count,nuclei,nucleus_count,
                                       identities,identity_count,output,capacity,written);
}

struct reloq_fa_owned_capture_context {
    struct reloq_eci_engine *engine;
    uint64_t *marks;
    size_t mark_count;
    unsigned epoch;
    int failed;
};

static int ECICALL reloq_fa_owned_capture_callback(ECIHand handle,int message,
    int value,void *opaque)
{
    struct reloq_fa_owned_capture_context *c=opaque;
    if(!c || handle!=c->engine || epoch_get(&c->engine->stop_epoch)!=c->epoch)
        return eciDataAbort;
    if(message==eciIndexReply) {
        size_t sample=eciPersianCaptureSampleCount();
        if(value<=0 || (size_t)value>c->mark_count || sample==SIZE_MAX ||
           c->marks[value-1]!=UINT64_MAX) {
            c->failed=1;return eciDataAbort;
        }
        c->marks[value-1]=(uint64_t)sample;
    } else if(message==eciWaveformBuffer && value>0) {
        /* A controls-only request must never deliver audible samples. */
        c->failed=1;return eciDataAbort;
    }
    return eciDataProcessed;
}

void ECICALL eciPersianReleaseOwnedCapture(struct reloq_fa_owned_capture *result)
{
    if(!result)return;
    free(result->frames);free(result->syllables);free(result->mark_samples);
    *result=(struct reloq_fa_owned_capture){0};
}

typedef int (*reloq_fa_capture_job)(void *context,const int *index_values);

static int reloq_fa_capture_owned(struct reloq_eci_engine *e,size_t index_count,
    size_t maximum_frames,unsigned before,reloq_fa_capture_job run,void *job,
    struct reloq_fa_owned_capture *result)
{
    uint64_t *marks=index_count?malloc(index_count*sizeof *marks):NULL;
    int *values=index_count?malloc(index_count*sizeof *values):NULL;
    if(index_count && (!marks || !values)){free(marks);free(values);return -2;}
    for(size_t i=0;i<index_count;++i){marks[i]=UINT64_MAX;values[i]=(int)i+1;}
    if(eciPersianCaptureBegin(maximum_frames)){free(marks);free(values);return -1;}
    int status=-3,failed=0;size_t count=0,syllable_count=0,sample_count=0;
    struct reloq_fa_owned_capture prepared={0};
    struct reloq_fa_owned_capture_context context={e,marks,index_count,before,0};
    ECICallback old_callback=e->callback;void *old_data=e->callback_data;
    short *old_output=e->output;int old_output_samples=e->output_samples;
    short output[2048];
    if(epoch_get(&e->stop_epoch)!=before){status=-4;goto done;}
    if(eciPersianCaptureSetControlsOnly(1))goto done;
    e->callback=reloq_fa_owned_capture_callback;e->callback_data=&context;
    e->output=output;e->output_samples=(int)(sizeof output/sizeof *output);
    status=run(job,values);
    e->callback=old_callback;e->callback_data=old_data;
    e->output=old_output;e->output_samples=old_output_samples;
    sample_count=eciPersianCaptureSampleCount();
    const struct reloq_fa_frame *frames=eciPersianCaptureEnd(&count,&failed);
    if(epoch_get(&e->stop_epoch)!=before){status=-4;goto done;}
    if(status)goto done;
    if(failed || context.failed || sample_count==SIZE_MAX || (count && !frames) ||
       count>SIZE_MAX/sizeof *prepared.frames){status=-3;goto done;}
    for(size_t i=0;i<index_count;++i)if(marks[i]==UINT64_MAX){status=-3;goto done;}
    int syllable_failed=0;
    const struct reloq_fa_syllable *syllables=eciPersianCaptureSyllables(&syllable_count,&syllable_failed);
    if(syllable_failed){syllable_count=0;syllables=NULL;}
    if((syllable_count && !syllables) || syllable_count>SIZE_MAX/sizeof *prepared.syllables) {
        status=-3;goto done;
    }
    if(count) {
        prepared.frames=malloc(count*sizeof *prepared.frames);
        if(!prepared.frames){status=-2;goto done;}
        memcpy(prepared.frames,frames,count*sizeof *prepared.frames);
    }
    if(syllable_count) {
        prepared.syllables=malloc(syllable_count*sizeof *prepared.syllables);
        if(!prepared.syllables){status=-2;goto done;}
        memcpy(prepared.syllables,syllables,syllable_count*sizeof *prepared.syllables);
    }
    if(epoch_get(&e->stop_epoch)!=before){status=-4;goto done;}
    prepared.frame_count=count;prepared.syllable_count=syllable_count;
    prepared.mark_samples=marks;prepared.mark_count=index_count;prepared.sample_count=(uint64_t)sample_count;
    marks=NULL;*result=prepared;prepared=(struct reloq_fa_owned_capture){0};status=0;
done:
    eciPersianCaptureRelease();free(values);free(marks);
    eciPersianReleaseOwnedCapture(&prepared);
    return status;
}

struct reloq_fa_native_capture_job {
    ECIHand handle;const char *text;size_t bytes;int mode;
    const size_t *offsets;size_t count;size_t *error_offset;
};
static int reloq_fa_capture_native_run(void *context,const int *values)
{
    struct reloq_fa_native_capture_job *j=context;
    return eciPersianSpeakNativeUtf8(j->handle,j->text,j->bytes,j->mode,
        j->offsets,values,j->count,j->error_offset);
}

int ECICALL eciPersianCaptureNativeUtf8(ECIHand handle,const char *text,size_t bytes,int mode,
    const size_t *source_offsets,size_t index_count,size_t maximum_frames,
    struct reloq_fa_owned_capture *result,size_t *error_offset)
{
    if(error_offset)*error_offset=0;
    if(!result)return -1;
    *result=(struct reloq_fa_owned_capture){0};
    struct reloq_eci_engine *e=handle;
    if(!e || !text || !error_offset || bytes==SIZE_MAX || (mode!=0 && mode!=1) ||
       (index_count && !source_offsets) || index_count>4096 ||
       !maximum_frames || maximum_frames>120000 || epoch_get(&e->busy) || e->queued ||
       e->voice_mode!=RELOQ_ECI_VOICE_GENERIC ||
       (e->language_dialect&~0xff00)!=0x90000 || memchr(text,0,bytes))return -1;
    for(size_t i=0;i<index_count;++i)
        if(source_offsets[i]>bytes || (i && source_offsets[i]<source_offsets[i-1]))return -1;
    unsigned before=epoch_get(&e->stop_epoch);
    struct reloq_fa_native_capture_job job={handle,text,bytes,mode,source_offsets,index_count,error_offset};
    return reloq_fa_capture_owned(e,index_count,maximum_frames,before,
        reloq_fa_capture_native_run,&job,result);
}

struct reloq_fa_us_capture_job {
    ECIHand handle;const struct reloq_fa_us_proxy *proxy;
};
static int reloq_fa_capture_us_run(void *context,const int *values)
{
    struct reloq_fa_us_capture_job *j=context;
    const struct reloq_fa_us_proxy *p=j->proxy;
    int old_tags=eciGetParam(j->handle,RELOQ_ECI_PROCESS_TAGS),status=-3;
    if(old_tags<0 || eciSetParam(j->handle,RELOQ_ECI_PROCESS_TAGS,1)<0)return -1;
    size_t pos=0;
    for(size_t i=0;i+1<p->word_count;++i) {
        size_t next=p->word_offsets[i];
        if(next<pos || next>p->length || !eciAddTextUtf8(j->handle,p->text+pos,next-pos,0) ||
           !eciInsertIndex(j->handle,values[i]))goto done;
        pos=next;
    }
    if(!eciAddTextUtf8(j->handle,p->text+pos,p->length-pos,0) || !eciSynthesize(j->handle))goto done;
    status=0;
done:
    if(status)(void)eciClearInput(j->handle);
    (void)eciSetParam(j->handle,RELOQ_ECI_PROCESS_TAGS,old_tags);
    return status;
}

int ECICALL eciPersianCaptureUsProxyUtf8(ECIHand handle,const char *tagged,size_t bytes,
    size_t maximum_frames,struct reloq_fa_owned_capture *result)
{
    if(!result)return -1;
    *result=(struct reloq_fa_owned_capture){0};
    struct reloq_eci_engine *e=handle;
    if(!e || !tagged || bytes==SIZE_MAX || !maximum_frames || maximum_frames>120000 ||
       epoch_get(&e->busy) || e->queued || e->voice_mode!=RELOQ_ECI_VOICE_GENERIC ||
       e->language!=0x10000 || reloq_fa_capture_active())return -1;
    unsigned before=epoch_get(&e->stop_epoch);
    struct reloq_fa_us_proxy proxy={0};
    int status=eciPersianUsProxyUtf8(handle,tagged,bytes,&proxy);
    if(epoch_get(&e->stop_epoch)!=before){status=-4;goto done;}
    if(status){status=status==1?1:status==-2?-2:-3;goto done;}
    if(!proxy.word_count || proxy.word_count-1>4096 || !proxy.text ||
       (proxy.word_count>1 && !proxy.word_offsets)){status=1;goto done;}
    struct reloq_fa_us_capture_job job={handle,&proxy};
    status=reloq_fa_capture_owned(e,proxy.word_count-1,maximum_frames,before,
        reloq_fa_capture_us_run,&job,result);
done:
    eciPersianUsProxyRelease(&proxy);
    return status;
}
int ECICALL eciPersianCaptureSetControlsOnly(int enabled)
{return reloq_fa_capture_set_controls_only(enabled);}
int ECICALL eciPersianCaptureSetPhoneTrace(int enabled)
{
    if(!reloq_fa_capture_active() || (enabled!=0 && enabled!=1))return -1;
    reloq61_syllable_trace_phone_mode(enabled);
    return 0;
}
int ECICALL eciPersianCaptureSetNucleusTrace(int enabled)
{
    if(!reloq_fa_capture_active() || (enabled!=0 && enabled!=1))return -1;
    reloq61_syllable_trace_nucleus_mode(enabled);
    return 0;
}
int ECICALL eciPersianApplyGlides(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_glide_candidate *candidates,size_t candidate_count,
    int pitch,size_t *applied)
{return reloq_fa_apply_glides(rows,count,candidates,candidate_count,pitch,applied);}
size_t ECICALL eciPersianCaptureSampleCount(void)
{return reloq_fa_capture_sample_count();}
int ECICALL eciPersianCaptureSetPcmStorage(int enabled)
{return reloq_fa_capture_set_pcm_storage(enabled);}
const int16_t *ECICALL eciPersianCapturePcm(size_t *count)
{return reloq_fa_capture_pcm(count);}
int64_t ECICALL eciPersianLastAudibleSample(const unsigned char *pcm,size_t bytes)
{return reloq_fa_last_audible_sample(pcm,bytes);}
int ECICALL eciPersianFrameTicks(const struct reloq_fa_frame *rows,size_t count,int64_t *ticks)
{return reloq_fa_frame_ticks(rows,count,ticks);}
int ECICALL eciPersianFrameTimeline(const struct reloq_fa_frame *rows,size_t count,double *times)
{return reloq_fa_frame_timeline(rows,count,times);}
int ECICALL eciPersianDonorSpan(const struct reloq_fa_frame *rows,size_t count,
    size_t leading,size_t syllables,size_t trailing,size_t *first,size_t *last)
{return reloq_fa_donor_span(rows,count,leading,syllables,trailing,first,last);}
void ECICALL eciPersianRecomputeFrication(struct reloq_fa_frame *rows,size_t count)
{reloq_fa_recompute_frication(rows,count);}
int ECICALL eciPersianBridgeContinuation(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed)
{return reloq_fa_bridge_continuation(input,output,times,ends,skip,frame_count,segment_count,window_ms,changed);}
int ECICALL eciPersianBridgeStopOnsets(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed)
{return reloq_persian_bridge_stop_onsets(input,output,times,ends,skip,frame_count,segment_count,window_ms,changed);}
int ECICALL eciPersianBridgeRemaining(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed)
{return reloq_persian_bridge_remaining(input,output,times,ends,skip,frame_count,segment_count,window_ms,changed);}
int ECICALL eciPersianMatchStopNasal(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary)
{return reloq_persian_match_stop_nasal(frames,count,boundary,end,reference,reference_count,reference_boundary);}
int ECICALL eciPersianMatchUsZOnset(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary)
{return reloq_persian_match_us_z_onset(frames,count,boundary,end,reference,reference_count,reference_boundary);}

int ECICALL eciPersianMatchHToUsZ(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary)
{return reloq_persian_match_h_to_us_z(frames,count,boundary,end,reference,reference_count,reference_boundary);}
int ECICALL eciPersianMatchUsUnvoicedOnset(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary,size_t reference_end,unsigned char phone)
{return reloq_persian_match_us_unvoiced_onset(frames,count,boundary,end,reference,
    reference_count,reference_boundary,reference_end,phone);}
int ECICALL eciPersianSmoothFinalRJoin(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end)
{return reloq_persian_smooth_final_r_join(frames,count,boundary,end);}
int ECICALL eciPersianPlanUnvoicedOnsets(struct reloq_fa_frame *frames,size_t count,
    const size_t *ends,size_t segments,const unsigned char *onsets,
    const struct reloq_fa_frame *reference,size_t reference_count,
    const size_t *reference_edges,size_t *counts,double *removed,size_t *new_count)
{return reloq_persian_plan_unvoiced_onsets(frames,count,ends,segments,onsets,reference,
    reference_count,reference_edges,counts,removed,new_count);}
int ECICALL eciPersianPlanStopROnsets(struct reloq_fa_frame *frames,size_t count,
    const size_t *ends,size_t segments,const size_t *starts,
    size_t *counts,double *removed,size_t *new_count)
{return reloq_persian_plan_stop_r_onsets(frames,count,ends,segments,starts,counts,removed,new_count);}
int ECICALL eciPersianDenseCurve(const struct reloq_fa_frame *rows,size_t count,
    unsigned column,float *out,size_t length)
{return reloq_fa_dense_curve(rows,count,column,out,length);}
int ECICALL eciPersianFitPitch(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,int64_t start,int64_t end,int64_t a,int64_t b)
{return reloq_fa_fit_pitch(rows,count,pilot,length,start,end,a,b);}
int ECICALL eciPersianFitPitchAnchors(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,int64_t start,int64_t end,int64_t a,int64_t b,
    const int64_t *target,const int64_t *source,size_t anchors)
{return reloq_fa_fit_pitch_anchors(rows,count,pilot,length,start,end,a,b,target,source,anchors);}
int ECICALL eciPersianTransferUsWords(struct reloq_fa_frame *target,size_t target_count,
    const struct reloq_fa_frame *donor,size_t donor_count,const int64_t *target_bounds,
    const int64_t *donor_bounds,size_t word_count,const struct reloq_fa_syllable *target_syllables,
    size_t target_syllable_count,const struct reloq_fa_syllable *donor_syllables,
    size_t donor_syllable_count,const uint32_t *syllables_per_word)
{return reloq_fa_transfer_us_words(target,target_count,donor,donor_count,target_bounds,donor_bounds,
    word_count,target_syllables,target_syllable_count,donor_syllables,donor_syllable_count,syllables_per_word);}
int ECICALL eciPersianTransferUsSamples(struct reloq_fa_frame *target,size_t target_count,
    const struct reloq_fa_frame *donor,size_t donor_count,const uint64_t *target_samples,
    const uint64_t *donor_samples,size_t word_count,unsigned sample_rate,
    const struct reloq_fa_syllable *target_syllables,size_t target_syllable_count,
    const struct reloq_fa_syllable *donor_syllables,size_t donor_syllable_count,
    const uint32_t *syllables_per_word)
{return reloq_fa_transfer_us_samples(target,target_count,donor,donor_count,target_samples,donor_samples,
    word_count,sample_rate,target_syllables,target_syllable_count,donor_syllables,
    donor_syllable_count,syllables_per_word);}

static int reloq_fa_replay_us_capture(ECIHand handle,
    const struct reloq_fa_owned_capture *target,const struct reloq_fa_owned_capture *donor,
    const uint32_t *counts,const struct reloq_fa_frame_mark *marks,size_t mark_count,
    unsigned before)
{
    struct reloq_eci_engine *e=handle;
    if(!e)return -1;
    if(epoch_get(&e->stop_epoch)!=before)return -4;
    if(!target || !donor || epoch_get(&e->busy) || e->queued ||
       e->voice_mode!=RELOQ_ECI_VOICE_GENERIC || (mark_count && !marks) ||
       mark_count>120000 || !target->frames || !target->frame_count ||
       !donor->frames || !donor->frame_count || target->frame_count>120000 ||
       donor->frame_count>120000 || target->mark_count!=donor->mark_count ||
       target->mark_count>=4096 ||
       (target->mark_count && (!target->mark_samples || !donor->mark_samples)) ||
       (target->syllable_count && !target->syllables) ||
       (donor->syllable_count && !donor->syllables))return -1;
    size_t words=target->mark_count+1;
    for(size_t i=0;i<target->mark_count;++i)
        if(target->mark_samples[i]>target->sample_count || donor->mark_samples[i]>donor->sample_count ||
           (i && (target->mark_samples[i]<target->mark_samples[i-1] ||
                  donor->mark_samples[i]<donor->mark_samples[i-1])))return -1;
    for(size_t i=0;i<mark_count;++i)
        if(marks[i].frame>target->frame_count || marks[i].value<0 ||
           (i && marks[i].frame<marks[i-1].frame))return -1;
    unsigned rate=(unsigned)sample_hz(e->sample_rate_code);
    if(!rate || target->frame_count>SIZE_MAX/sizeof *target->frames)return -1;
    struct reloq_fa_frame *planned=malloc(target->frame_count*sizeof *planned);
    uint64_t *bounds=malloc((words+1)*2*sizeof *bounds);
    if(!planned || !bounds){free(planned);free(bounds);return -2;}
    memcpy(planned,target->frames,target->frame_count*sizeof *planned);
    uint64_t *tb=bounds,*db=bounds+words+1;
    tb[0]=db[0]=0;
    for(size_t i=0;i<target->mark_count;++i) {
        tb[i+1]=target->mark_samples[i];db[i+1]=donor->mark_samples[i];
    }
    tb[words]=target->sample_count;db[words]=donor->sample_count;
    int status=eciPersianTransferUsSamples(planned,target->frame_count,donor->frames,donor->frame_count,
        tb,db,words,rate,target->syllables,target->syllable_count,
        donor->syllables,donor->syllable_count,counts);
    if(status==1 && words>1) {
        /* A failed word fit can already have modified an earlier word. */
        memcpy(planned,target->frames,target->frame_count*sizeof *planned);
        uint64_t whole_target[]={0,target->sample_count},whole_donor[]={0,donor->sample_count};
        status=eciPersianTransferUsSamples(planned,target->frame_count,donor->frames,donor->frame_count,
            whole_target,whole_donor,1,rate,NULL,0,NULL,0,NULL);
    }
    if(status==1) {
        int voiced=0;
        for(size_t i=0;i<target->frame_count;++i)
            if(target->frames[i].controls[1]>0 && target->frames[i].controls[2]>0){voiced=1;break;}
        if(!voiced){memcpy(planned,target->frames,target->frame_count*sizeof *planned);status=0;}
    }
    if(epoch_get(&e->stop_epoch)!=before)status=-4;
    else if(status<0)status=-1;
    else if(!status) {
        if(e->original_acoustic_clock==1) {
            /* Transfer inserts 1-ms pitch changes after capture. Match the
             * active Persian voice's 5-ms hold, while leaving its two
             * request-local phone-boundary metadata tracks untouched. */
            int64_t group=INT64_MIN;float held[62];
            for(size_t i=0;i<target->frame_count;++i) {
                int64_t current=planned[i].tick/5;
                if(current!=group) {
                    group=current;
                    memcpy(held,planned[i].controls,sizeof held);
                } else for(unsigned column=1;column<62;++column)
                    if(column!=11 && column!=12)planned[i].controls[column]=held[column];
            }
        }
        int rendered=eciPersianReplayFrames(handle,before,planned,target->frame_count,marks,mark_count);
        status=rendered==RELOQ_ECI_OK?0:rendered==RELOQ_ECI_CANCELLED?-4:-3;
    }
    free(bounds);free(planned);return status;
}

int ECICALL eciPersianReplayUsCapture(ECIHand handle,
    const struct reloq_fa_owned_capture *target,const struct reloq_fa_owned_capture *donor,
    const uint32_t *counts,const struct reloq_fa_frame_mark *marks,size_t mark_count)
{
    struct reloq_eci_engine *e=handle;
    if(!e)return -1;
    return reloq_fa_replay_us_capture(handle,target,donor,counts,marks,mark_count,
        epoch_get(&e->stop_epoch));
}

/* The projected Persian plan is ASCII ECI phonetics. These boundaries match
 * the sentence planner: punctuation belongs to the following word and a
 * weak-word control is marked before the control, not after it. */
struct reloq_fa_native_words {
    size_t *boundaries;
    unsigned char *pause,*glide;
    uint32_t *syllables;
    size_t count;
};

static void reloq_fa_native_words_clear(struct reloq_fa_native_words *words)
{
    free(words->boundaries);free(words->pause);free(words->glide);free(words->syllables);
    *words=(struct reloq_fa_native_words){0};
}

static int reloq_fa_native_words_make(const char *text,size_t bytes,
    struct reloq_fa_native_words *words)
{
    *words=(struct reloq_fa_native_words){0};
    if(!text || memchr(text,0,bytes) || strstr(text,"`l"))return 1;
    size_t count=0;
    for(size_t i=0;i+2<bytes;) {
        if(text[i]!='`' || text[i+1]!='['){++i;continue;}
        size_t end=i+2;while(end<bytes && text[end]!=']')++end;
        if(end==bytes || end==i+2 || ++count>4096)return 1;
        i=end+1;
    }
    if(!count)return 1;
    words->syllables=calloc(count,sizeof *words->syllables);
    words->boundaries=count>1?malloc((count-1)*sizeof *words->boundaries):NULL;
    words->pause=count>1?calloc(count-1,sizeof *words->pause):NULL;
    words->glide=count>1?calloc(count-1,sizeof *words->glide):NULL;
    if(!words->syllables || (count>1 && (!words->boundaries || !words->pause || !words->glide))) {
        reloq_fa_native_words_clear(words);return -2;
    }
    size_t word=0,previous_start=0,previous_end=0;
    for(size_t i=0;i+2<bytes;) {
        if(text[i]!='`' || text[i+1]!='['){++i;continue;}
        size_t end=i+2;while(end<bytes && text[end]!=']')++end;
        if(word) {
            size_t boundary=i;
            for(size_t p=previous_end;p<i;++p)
                if(strchr(".,?!:;()[]{}",text[p])){boundary=previous_end;break;}
            if(boundary==i) {
                size_t p=i;
                while(p>previous_end && strchr(" \t\n\r\v\f",text[p-1]))--p;
                if(p>=previous_end+3 && !memcmp(text+p-3,"`00",3))boundary=p-3;
            }
            words->boundaries[word-1]=boundary;
            words->pause[word-1]=(unsigned char)(boundary<bytes &&
                strchr(".,?!:;()[]{}",text[boundary])!=NULL);
            int space=i>previous_end;
            for(size_t p=previous_end;p<i;++p)
                if(!strchr(" \t\n\r\v\f",text[p])){space=0;break;}
            int final_u=(previous_end>=2 && text[previous_end-2]=='u') ||
                (previous_end>=5 && !memcmp(text+previous_end-5,"'u:']",5));
            int initial_a=i+4<end && text[i+2]=='.' && text[i+3]>='0' && text[i+3]<='2' &&
                (text[i+4]=='a' || (i+8<=end && !memcmp(text+i+4,"'a:'",4)));
            if(space && final_u && initial_a &&
               !memchr(text+previous_start,'w',previous_end-previous_start) &&
               !memchr(text+i,'w',end+1-i))words->glide[word-1]=1;
        }
        for(size_t p=i+2;p+1<end;++p)
            if(text[p]=='.' && text[p+1]>='0' && text[p+1]<='2')++words->syllables[word];
        ++word;previous_start=i;previous_end=end+1;i=end+1;
    }
    words->count=count;return 0;
}

struct reloq_fa_prepared_capture_job {
    ECIHand handle;const struct reloq_fa_prepared *prepared;
    const size_t *positions;const unsigned char *pause;size_t count;
};
static int reloq_fa_capture_prepared_run(void *context,const int *values)
{
    struct reloq_fa_prepared_capture_job *job=context;
    return reloq_fa_speak_prepared(job->handle,job->prepared,job->positions,
        values,job->pause,job->count);
}

static int reloq_fa_sample_tick(uint64_t sample,unsigned rate,int64_t *tick)
{
    if(!rate || sample>UINT64_MAX/1000u)return -1;
    uint64_t scaled=sample*1000u,whole=scaled/rate,remainder=scaled%rate;
    if(remainder*2u>rate || (remainder*2u==rate && (whole&1u)))++whole;
    if(whole>INT64_MAX)return -1;
    *tick=(int64_t)whole;return 0;
}

/* An English-only character name belongs to the ordinary US engine, just as
 * it does for the existing NVDA voice. Keep the caller's handle, callback,
 * cancellation and source indices; a language switch is part of this one
 * native utterance and never changes the selected voice. */
static int reloq_fa_speak_english_character(ECIHand voice,const char *name,
    size_t name_bytes,int spell,const size_t *offsets,const int *values,
    size_t index_count,size_t source_bytes,unsigned before)
{
    struct reloq_eci_engine *e=voice;
    int old_tags=eciGetParam(voice,RELOQ_ECI_PROCESS_TAGS);
    if(old_tags<0 || eciSetParam(voice,RELOQ_ECI_PROCESS_TAGS,1)<0)return -1;
    int status=-3;size_t i=0;
    while(i<index_count && offsets[i]<source_bytes) {
        if(!eciInsertIndex(voice,values[i++]))goto done;
    }
    if(!eciAddTextUtf8(voice,"`l1.0 ",6,0) ||
       !eciAddTextUtf8(voice,name,name_bytes,spell?1:0))goto done;
    while(i<index_count)if(!eciInsertIndex(voice,values[i++]))goto done;
    if(epoch_get(&e->stop_epoch)!=before){status=-4;goto done;}
    if(!eciSynthesize(voice)) {
        if(epoch_get(&e->stop_epoch)!=before)status=-4;
        goto done;
    }
    status=epoch_get(&e->stop_epoch)!=before?-4:0;
done:
    if(status)(void)eciClearInput(voice);
    (void)eciSetParam(voice,RELOQ_ECI_PROCESS_TAGS,old_tags);
    return status;
}

static int reloq_fa_silent_context(ECIHand voice,const int *values,size_t count,unsigned before)
{
    struct reloq_eci_engine *e=voice;
    if(epoch_get(&e->stop_epoch)!=before)return -4;
    for(size_t i=0;i<count;++i)if(!eciInsertIndex(voice,values[i])) {
        (void)eciClearInput(voice);return -3;
    }
    if(count && !eciSynthesize(voice)) {
        (void)eciClearInput(voice);
        return epoch_get(&e->stop_epoch)!=before?-4:-3;
    }
    return epoch_get(&e->stop_epoch)!=before?-4:0;
}

struct reloq_fa_parallel_donor {
    ECIHand handle;const char *text;size_t maximum;
    struct reloq_fa_owned_capture *capture;int status;
};
#if defined(_WIN32)
typedef HANDLE reloq_fa_donor_thread;
static unsigned __stdcall reloq_fa_capture_donor_thread(void *context)
{
    struct reloq_fa_parallel_donor *job=context;
    job->status=eciPersianCaptureUsProxyUtf8(job->handle,job->text,strlen(job->text),
                                             job->maximum,job->capture);
    return 0;
}
static int reloq_fa_start_donor(reloq_fa_donor_thread *thread,struct reloq_fa_parallel_donor *job)
{
    uintptr_t handle=_beginthreadex(NULL,0,reloq_fa_capture_donor_thread,job,0,NULL);
    if(!handle)return 0;
    *thread=(HANDLE)handle;return 1;
}
static void reloq_fa_join_donor(reloq_fa_donor_thread thread)
{(void)WaitForSingleObject(thread,INFINITE);(void)CloseHandle(thread);}
#else
typedef pthread_t reloq_fa_donor_thread;
static void *reloq_fa_capture_donor_thread(void *context)
{
    struct reloq_fa_parallel_donor *job=context;
    job->status=eciPersianCaptureUsProxyUtf8(job->handle,job->text,strlen(job->text),
                                             job->maximum,job->capture);
    return NULL;
}
static int reloq_fa_start_donor(reloq_fa_donor_thread *thread,struct reloq_fa_parallel_donor *job)
{return pthread_create(thread,NULL,reloq_fa_capture_donor_thread,job)==0;}
static void reloq_fa_join_donor(reloq_fa_donor_thread thread)
{(void)pthread_join(thread,NULL);}
#endif

/* Prepare the native English worker on the owner thread and retain no text
 * or audio. Its own provider reset rules handle each subsequent request. */
static int reloq_fa_prepare_english_worker(struct reloq_eci_engine *e)
{
    if(e->persian_english_worker)return 0;
    void *worker=reloq61_worker_new(RELOQ_ECI_ENGLISH);
    if(!worker)return -1;
    char phonemes[512];struct reloq61_result result;
    struct reloq61_request request={.language=RELOQ_ECI_ENGLISH,
        .annotations=1,.phonemes_only=1,.voice={0,50,65,30,0,0,50,92},
        .text="",.phonemes=phonemes,.phonemes_capacity=sizeof phonemes};
    void *previous=reloq61_worker_scope(worker);
    int status=reloq61_generate(&request,&result);
    reloq61_worker_scope(previous);
    if(status){reloq61_worker_free(worker);return -1;}
    e->persian_english_worker=worker;return 0;
}

/* The native sentence API may own its silent US pilot. Synchronize the
 * effective voice and shared settings before each request, but keep the
 * provider and its dictionaries across requests. This is opt-in and does
 * not turn the experimental Finnish bridge into a registered Persian voice. */
static int reloq_fa_prepare_us_pilot(struct reloq_eci_engine *e,unsigned before,
    struct reloq_eci_engine **result)
{
    *result=NULL;
    if(!e || epoch_get(&e->busy) || e->queued ||
       (e->language_dialect&~0xff00)!=0x90000 ||
       e->voice_mode!=RELOQ_ECI_VOICE_GENERIC)return -1;
    lock_session(&e->session_lock);
    struct reloq_eci_engine *pilot=e->persian_us_pilot;
    unlock_session(&e->session_lock);
    if(!pilot) {
        struct reloq_eci_engine *candidate=eciNewEx(RELOQ_ECI_ENGLISH);
        if(!candidate)return -2;
        lock_session(&e->session_lock);
        if(!e->persian_us_pilot)e->persian_us_pilot=candidate;
        else pilot=e->persian_us_pilot;
        unlock_session(&e->session_lock);
        if(pilot)(void)eciDelete(candidate);else pilot=candidate;
    }
    if(epoch_get(&e->stop_epoch)!=before)return -4;
    if(epoch_get(&pilot->busy) || pilot->queued)return -1;
    if(eciSetParam(pilot,RELOQ_ECI_SAMPLE_RATE,e->sample_rate_code)<0 ||
       eciSetParam(pilot,RELOQ_ECI_GENDER,e->gender)<0 ||
       eciSetParam(pilot,RELOQ_ECI_HEAD_SIZE,e->head_size)<0)return -2;
    struct queued_voice voice=capture_voice(e);
    apply_voice(pilot,&voice);
    pilot->real_world_units=e->real_world_units;
    pilot->high_rate_profile=e->high_rate_profile;
    pilot->dictionary_disabled=e->dictionary_disabled;
    pilot->phrase_prediction=e->phrase_prediction;
    pilot->original_questions=e->original_questions;
    pilot->mellow_mode=e->mellow_mode;
    pilot->original_acoustic_clock=e->original_acoustic_clock;
    pilot->word_segmentation=e->word_segmentation;
    pilot->camel_case=e->camel_case;
    pilot->pause_mode=e->pause_mode;
    pilot->eq_low=e->eq_low;pilot->eq_mid=e->eq_mid;pilot->eq_high=e->eq_high;
    memcpy(pilot->eq_filters,e->eq_filters,sizeof pilot->eq_filters);
    const char *a=e->dictionary_path,*b=pilot->dictionary_path;
    if((a && !b) || (!a && b) || (a && b && strcmp(a,b))) {
        if(!eciSetDictionaryPath(pilot,a))return -2;
    }
    if(!pilot->rule_worker) {
        pilot->rule_worker=reloq61_worker_new(pilot->language);
        if(!pilot->rule_worker)return -2;
    }
    if(epoch_get(&e->stop_epoch)!=before)return -4;
    *result=pilot;
    return 0;
}

int ECICALL eciPersianPrepareUsPilot(ECIHand handle)
{
    struct reloq_eci_engine *e=handle,*pilot=NULL;
    if(!e)return -1;
    return reloq_fa_prepare_us_pilot(e,epoch_get(&e->stop_epoch),&pilot);
}

static int reloq_fa_speak_us_native(ECIHand handle,ECIHand pilot_handle,
    const char *text,size_t bytes,int mode,const size_t *run_ends,
    const unsigned char *run_modes,size_t run_count,int use_runs,
    const size_t *source_offsets,const int *index_values,size_t index_count,
    size_t maximum_frames,size_t *error_offset)
{
    struct reloq_eci_engine *e=handle,*pilot=pilot_handle;
    if(error_offset)*error_offset=0;
    if(!e || !text || !error_offset || bytes==SIZE_MAX ||
       (mode!=0 && mode!=1) || (index_count && (!source_offsets || !index_values)) ||
       index_count>4096 || !maximum_frames || maximum_frames>120000 ||
       epoch_get(&e->busy) || e->queued ||
       e->voice_mode!=RELOQ_ECI_VOICE_GENERIC ||
       (e->language_dialect&~0xff00)!=0x90000 || memchr(text,0,bytes))return -1;
    for(size_t i=0;i<index_count;++i)
        if(source_offsets[i]>bytes || index_values[i]<0 ||
           (i && source_offsets[i]<source_offsets[i-1]))return -1;
    unsigned before=epoch_get(&e->stop_epoch);
    if(!pilot) {
        int prepared=reloq_fa_prepare_us_pilot(e,before,&pilot);
        if(prepared)return prepared;
        pilot_handle=pilot;
    }
    if(e==pilot || epoch_get(&pilot->busy) || pilot->queued ||
       pilot->voice_mode!=RELOQ_ECI_VOICE_GENERIC || pilot->language!=0x10000 ||
       e->sample_rate_code!=pilot->sample_rate_code)return -1;
    if(epoch_get(&e->stop_epoch)!=before)return -4;
    unsigned pilot_before=epoch_get(&pilot->stop_epoch);
    /* Like eciAddTextUtf8's context bit, run bit 4 says that a scalar
     * belongs to a larger host utterance. Explicit character mode and emoji
     * remain named. Strip the bit before passing modes to the analyzer. */
    unsigned char *owned_modes=NULL;int source_in_context=0,has_context=0;
    if(use_runs && run_count) {
        if(!run_modes || !run_ends || run_count>bytes)return -1;
        for(size_t i=0;i<run_count;++i) {
            if((run_modes[i]&~7u) || (run_modes[i]&3u)==3u)return -1;
            has_context|=(run_modes[i]&4u)!=0;
        }
        if(has_context) {
            owned_modes=malloc(run_count);
            if(!owned_modes)return -2;
            for(size_t i=0;i<run_count;++i)owned_modes[i]=(unsigned char)(run_modes[i]&3u);
            if(run_count==1 && (run_modes[0]&4u) && !(run_modes[0]&3u))source_in_context=1;
            run_modes=owned_modes;
        }
    }
    /* A whole one-scalar request is a character request even without an
     * explicit spelling run. A symbol in a larger input stays normal text. */
    size_t scalar_end=0;uint32_t scalar=0;int one_scalar=0,single=0;
    if(bytes && (!use_runs || (run_count==1 && run_ends && run_modes &&
       run_ends[0]==bytes && run_modes[0]<=1)) &&
       reloq_utf8_next((const unsigned char *)text,bytes,&scalar_end,&scalar)==1 && scalar_end==bytes)
        one_scalar=1;
    single=one_scalar && (!source_in_context || reloq_unicode_is_default_emoji(scalar));
    if(single) {
        char name[2048];int kind=0;
        int resolved=eciPersianCharacterResolve(scalar,name,sizeof name,&kind);
        if(resolved<0){free(owned_modes);return -1;}
        if(resolved==1 && (kind || (mode==0 && ((scalar>='A' && scalar<='Z') ||
                                                (scalar>='a' && scalar<='z'))))) {
            int result=kind?reloq_fa_speak_english_character(handle,name,strlen(name),0,
                source_offsets,index_values,index_count,bytes,before):
                reloq_fa_speak_english_character(handle,text,bytes,1,
                source_offsets,index_values,index_count,bytes,before);
            free(owned_modes);return result;
        }
    }
    struct reloq_fa_prepared prepared={0};struct reloq_fa_native_words words={0};
    struct reloq_fa_owned_capture target={0},donor={0};
    char *donor_text=NULL;
    size_t *positions=NULL,*source_slots=NULL,*word_slots=NULL;
    unsigned char *pauses=NULL;uint64_t *word_samples=NULL;
    struct reloq_fa_frame_mark *replay_marks=NULL;
    struct reloq_fa_glide_candidate *glides=NULL;
    /* Both acoustic and contour inputs must come from the same linguistic
     * analysis. The ordinary text API previously projected the whole input
     * a second time for the donor; besides repeated work, that could let
     * target and donor take different analysis paths. Preserve its existing
     * rejection of inline controls, then prepare one ordinary-text run. */
    size_t ordinary_end=bytes;
    unsigned char ordinary_mode=single?1:0;
    if(single && use_runs)run_modes=&ordinary_mode;
    int status,pilot_clock_changed=0;
    int pilot_clock=eciGetParam(pilot_handle,RELOQ_ECI_ORIGINAL_ACOUSTIC_CLOCK);
    /* The US pilot is part of this Persian utterance. Its temporary clock
     * follows the Persian handle, independently of the caller's stored US
     * preference. Persian's automatic (2) mode uses the fine clock. */
    int wanted_clock=e->original_acoustic_clock==1;
    if(pilot_clock<0){status=-1;goto done;}
    if(pilot_clock!=wanted_clock) {
        if(eciSetParam(pilot_handle,RELOQ_ECI_ORIGINAL_ACOUSTIC_CLOCK,wanted_clock)<0) {
            status=-1;goto done;
        }
        pilot_clock_changed=1;
    }
    if(!use_runs && !single && memchr(text,'`',bytes))status=1;
    else status=reloq_fa_prepare_native_runs(text,bytes,mode,
        use_runs?run_ends:(bytes?&ordinary_end:NULL),
        use_runs?run_modes:(bytes?&ordinary_mode:NULL),
        use_runs?run_count:(bytes?1:0),source_offsets,index_count,error_offset,
        &prepared,&donor_text,use_runs||single);
    if(status)goto done;
    status=reloq_fa_native_words_make(prepared.backend.text,prepared.backend.length,&words);
    if(status==1 && source_in_context && one_scalar &&
       (!prepared.backend.length || (scalar<128 && strchr(".,?!:;()[]{}-",(int)scalar)) ||
        scalar==0x060c || scalar==0x061b || scalar==0x061f)) {
        status=reloq_fa_silent_context(handle,index_values,index_count,before);
        goto done;
    }
    if(status)goto done;
    /* The target plan collapses and marks geminated affricates for the
     * acoustic provider. The donor generated from the same events keeps the
     * original pronunciation, including both phones and glottal boundaries. */
    if(!donor_text){status=1;goto done;}
    size_t word_count=words.count-1;
    if(index_count+word_count>4096){status=1;goto done;}
    size_t total=index_count+word_count;
    positions=total?malloc(total*sizeof *positions):NULL;
    pauses=total?calloc(total,sizeof *pauses):NULL;
    source_slots=index_count?malloc(index_count*sizeof *source_slots):NULL;
    word_slots=word_count?malloc(word_count*sizeof *word_slots):NULL;
    word_samples=word_count?malloc(word_count*sizeof *word_samples):NULL;
    glides=word_count?malloc(word_count*sizeof *glides):NULL;
    replay_marks=index_count?malloc(index_count*sizeof *replay_marks):NULL;
    if((total && (!positions || !pauses)) || (index_count && (!source_slots || !replay_marks)) ||
       (word_count && (!word_slots || !word_samples || !glides))){status=-2;goto done;}
    for(size_t source=0,word=0,at=0;at<total;++at) {
        if(word<word_count && (source==index_count || words.boundaries[word]<=prepared.positions[source])) {
            positions[at]=words.boundaries[word];pauses[at]=words.pause[word];word_slots[word++]=at;
        } else {positions[at]=prepared.positions[source];source_slots[source++]=at;}
    }
    if(epoch_get(&e->stop_epoch)!=before || epoch_get(&pilot->stop_epoch)!=pilot_before){status=-4;goto done;}
    struct reloq_fa_prepared_capture_job job={handle,&prepared,positions,pauses,total};
    /* The two owned captures consume the same completed text analysis but
     * use independent handles and thread-local frame storage. Running the
     * US contour alongside the target avoids adding their processing times
     * before the first output. Thread creation failure keeps the established
     * sequential route. No speech or linguistic results outlive the job. */
    struct reloq_fa_parallel_donor donor_job={pilot_handle,donor_text,maximum_frames,&donor,0};
    reloq_fa_donor_thread donor_thread;
    /* For a single short word thread startup can cost more than the donor. */
    int donor_started=words.count>1 && reloq_fa_start_donor(&donor_thread,&donor_job);
    status=reloq_fa_capture_owned(e,total,maximum_frames,before,
        reloq_fa_capture_prepared_run,&job,&target);
    if(donor_started) {
        if(status || epoch_get(&e->stop_epoch)!=before)(void)eciStop(pilot_handle);
        reloq_fa_join_donor(donor_thread);
    }
    if(status)goto done;
    if(epoch_get(&e->stop_epoch)!=before || epoch_get(&pilot->stop_epoch)!=pilot_before){status=-4;goto done;}
    status=donor_started?donor_job.status:
        eciPersianCaptureUsProxyUtf8(pilot_handle,donor_text,strlen(donor_text),maximum_frames,&donor);
    if(status)goto done;
    if(epoch_get(&e->stop_epoch)!=before || epoch_get(&pilot->stop_epoch)!=pilot_before){status=-4;goto done;}
    if(donor.mark_count!=word_count){status=1;goto done;}
    unsigned rate=(unsigned)sample_hz(e->sample_rate_code);
    for(size_t i=0;i<word_count;++i)word_samples[i]=target.mark_samples[word_slots[i]];
    size_t glide_count=0;
    for(size_t i=0;i<word_count;++i)if(words.glide[i]) {
        int64_t left=0,boundary=0,right=0;
        if(!target.frame_count || reloq_fa_sample_tick(word_samples[i],rate,&boundary) ||
           (i && reloq_fa_sample_tick(word_samples[i-1],rate,&left)) ||
           (i+1<word_count && reloq_fa_sample_tick(word_samples[i+1],rate,&right))) {
            status=1;goto done;
        }
        if(i+1==word_count)right=target.frames[target.frame_count-1].tick+1;
        if(left>=boundary || boundary>=right){status=1;goto done;}
        glides[glide_count++]=(struct reloq_fa_glide_candidate){(uint32_t)i+1,left,boundary,right};
    }
    if(glide_count) {
        size_t applied=0;
        status=eciPersianApplyGlides(target.frames,target.frame_count,glides,glide_count,1,&applied);
        if(status)goto done;
    }
    for(size_t i=0;i<index_count;++i) {
        size_t frame=0;
        if(source_offsets[i]==bytes)frame=target.frame_count;
        else if(source_offsets[i]) {
            int64_t tick=0;
            if(reloq_fa_sample_tick(target.mark_samples[source_slots[i]],rate,&tick)){status=-1;goto done;}
            size_t lo=0,hi=target.frame_count;
            while(lo<hi){size_t mid=lo+(hi-lo)/2;if(target.frames[mid].tick<tick)lo=mid+1;else hi=mid;}
            frame=lo;
        }
        if(frame>UINT32_MAX){status=-1;goto done;}
        replay_marks[i]=(struct reloq_fa_frame_mark){(uint32_t)frame,index_values[i]};
    }
    struct reloq_fa_owned_capture target_words=target;
    target_words.mark_samples=word_samples;target_words.mark_count=word_count;
    status=reloq_fa_replay_us_capture(handle,&target_words,&donor,words.syllables,
        replay_marks,index_count,before);
done:
    if(pilot_clock_changed && eciSetParam(pilot_handle,RELOQ_ECI_ORIGINAL_ACOUSTIC_CLOCK,pilot_clock)<0 && !status)
        status=-3;
    reloq_fa_prepared_clear(&prepared);reloq_fa_native_words_clear(&words);
    eciPersianRelease(donor_text);
    eciPersianReleaseOwnedCapture(&target);eciPersianReleaseOwnedCapture(&donor);
    free(positions);free(pauses);free(source_slots);free(word_slots);free(word_samples);
    free(replay_marks);free(glides);free(owned_modes);
    return status;
}

int ECICALL eciPersianSpeakUsNativeUtf8(ECIHand handle,ECIHand pilot,
    const char *text,size_t bytes,int mode,const size_t *source_offsets,
    const int *index_values,size_t index_count,size_t maximum_frames,size_t *error_offset)
{
    return reloq_fa_speak_us_native(handle,pilot,text,bytes,mode,NULL,NULL,0,0,
        source_offsets,index_values,index_count,maximum_frames,error_offset);
}

int ECICALL eciPersianSpeakUsNativeRunsUtf8(ECIHand handle,ECIHand pilot,
    const char *text,size_t bytes,int mode,const size_t *run_ends,
    const unsigned char *run_modes,size_t run_count,const size_t *source_offsets,
    const int *index_values,size_t index_count,size_t maximum_frames,size_t *error_offset)
{
    return reloq_fa_speak_us_native(handle,pilot,text,bytes,mode,run_ends,run_modes,run_count,1,
        source_offsets,index_values,index_count,maximum_frames,error_offset);
}
int ECICALL eciPersianFitPitchEdges(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,const float *original,int64_t start,int64_t end,
    int64_t a,int64_t b,unsigned edges,int64_t window)
{return reloq_fa_fit_pitch_edges(rows,count,pilot,length,original,start,end,a,b,edges,window);}
int ECICALL eciPersianTransferKoreanPitch(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_frame *donor,size_t donor_count,unsigned edges,int64_t window)
{return reloq_fa_transfer_korean_pitch(rows,count,donor,donor_count,edges,window);}
int ECICALL eciPersianTransferKoreanAligned(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_frame *donor,size_t donor_count,const int64_t *word_ticks,
    const uint32_t *syllables,size_t word_count)
{return reloq_fa_transfer_korean_aligned(rows,count,donor,donor_count,word_ticks,syllables,word_count);}
uint32_t ECICALL eciPersianReplayEpoch(ECIHand handle)
{return reloq_fa_replay_epoch(handle);}
int ECICALL eciPersianReplayBegin(ECIHand handle,uint32_t epoch,void **stream)
{return reloq_fa_replay_begin(handle,epoch,stream);}
int ECICALL eciPersianReplayAppend(void *stream,const struct reloq_fa_frame *frames,
    size_t frame_count,const struct reloq_fa_frame_mark *marks,size_t mark_count)
{return reloq_fa_replay_append(stream,frames,frame_count,marks,mark_count);}
int ECICALL eciPersianReplayEnd(void *stream,int flush)
{return reloq_fa_replay_end(stream,flush);}
int ECICALL eciPersianReplayFrames(ECIHand handle,uint32_t epoch,
    const struct reloq_fa_frame *frames,size_t frame_count,
    const struct reloq_fa_frame_mark *marks,size_t mark_count)
{return reloq_fa_replay_frames(handle,epoch,frames,frame_count,marks,mark_count);}
