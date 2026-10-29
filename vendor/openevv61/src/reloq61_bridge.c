/* Private C rule provider. Public ECI exports belong to the ReLoq host. */
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "reloq61.h"
#include "reloq61_english_plan.h"
#include "reloq61_english_bank.h"
#include "evv_klatttap.h"
#include "reloq_voice_controls.h"
#include "evv_abi.h"
#include "evv_port.h"
#include "delta.h"
#include "klatt_lang.h"
#include "eci_old.h"
#include "eci_instance.h"
#include "eci_eloqc.h"
#include "reloq_greek.h"
extern void evvRunStaticInitialisers(void);
extern OldInst *STDCALL eo_newEx(int32_t);
extern int STDCALL es_delete(OldInst *);
extern int STDCALL eo_clearInput(OldInst *);
extern int32_t STDCALL api_reset(void *,int32_t);
extern void STDCALL eo_registerCallback(OldInst *,void *,void *);
extern int32_t STDCALL ev_setParam(OldInst *,int32_t,int32_t);
extern int STDCALL vc_setVoiceParam(OldInst *,int32_t,int32_t,int32_t);
extern int STDCALL ev_setOutputBuffer(OldInst *,int32_t,void *);
extern int STDCALL et_addText(OldInst *,const void *);
extern int STDCALL et_insertIndex(OldInst *,int32_t);
extern int STDCALL et_synthesize(OldInst *);
extern int STDCALL es_synchronize(OldInst *);
extern int ev_sendParameters(OldInst *);
extern int STDCALL ev_generatePhonemes(OldInst *,int32_t,void *);
extern delta_state *ew_machine(void *);
extern void stb_staticWordCallback(int32_t,void *);
extern void reloq61_array_reset(delta_state *);
extern Index *sti_getIndex(void *,uint32_t);
extern void *reloq61_inline_scope(void *);
static atomic_int initialized;
struct provider_worker {
    OldInst *handle;int language,prepared,last_voice[8],last_phonemes,last_annotations,last_sample_rate;int16_t pcm[4096];
    struct provider_worker *english;
    char *dictionary_path;void *dictionary;
    char **dictionary_keys;size_t dictionary_key_count;uint64_t dictionary_lengths[4];unsigned char dictionary_starts[256];int dictionary_checked;
    int32_t env[OI_ENV_WORDS],env_saved[OI_ENV_WORDS];
    char voice[VOICE_BYTES],voice_saved[VOICE_BYTES];
    int32_t ready,ready2,wideband,wideband_sent,pauses,pauses_sent;
};
static _Thread_local struct provider_worker *owner_worker;
typedef void (*pause_sink)(void *,int64_t,int64_t,int64_t);
static _Thread_local struct {int policy;pause_sink sink;void *context;} pause_request;
/* The host selects a policy on its serialized provider worker. Explicit
   pause annotations already have their own token and bypass this default
   phrase-pause insertion rule. No text or abbreviation is rewritten. */
void *reloq61_worker_new(int language)
{
    if(!reloq61_language_supported(language))return NULL;
    struct provider_worker *p=calloc(1,sizeof *p);
    if(p){p->language=language;if(language==0x120000||language==0xa0000||language==0x60000||language==0x60001){p->english=reloq61_worker_new(0x10000);if(!p->english){free(p);return NULL;}}}
    return p;
}
static void dictionary_clear(struct provider_worker *);
static void dictionary_detach(struct provider_worker *,OldInst *);
static void release_handle(struct provider_worker *worker,OldInst *h)
{
    /* An interrupted utterance invalidates its machine, not its immutable
     * dictionary data. Rebind that data when a new machine is needed. */
    if(worker){dictionary_detach(worker,h);worker->handle=NULL;worker->prepared=0;}
    if(h)es_delete(h);
}
void reloq61_worker_free(void *context)
{
    struct provider_worker *p=context;
    if(p){
        dictionary_clear(p);
        release_handle(p,p->handle);
        free(p->dictionary_path);
        reloq61_worker_free(p->english);
        free(p);
    }
}
void *reloq61_worker_scope(void *context)
{
    void *previous=owner_worker;owner_worker=context;return previous;
}
static void initialize(void)
{
    int expect=0;
    if(atomic_compare_exchange_strong(&initialized,&expect,1)) {
        evv_port_start();evvRunStaticInitialisers();
        atomic_store(&initialized,2);
    } else while(atomic_load(&initialized)!=2)evv_sleep_ms(1);
}
#include "reloq61_dictionary.inc"
static void restore_controller(struct provider_worker *worker)
{
    OldInst *h=worker->handle;
    memcpy(h->env,worker->env,sizeof h->env);
    memcpy(h->env_saved,worker->env_saved,sizeof h->env_saved);
    memcpy(h->voice,worker->voice,sizeof h->voice);
    memcpy(h->voice_saved,worker->voice_saved,sizeof h->voice_saved);
    h->ready=worker->ready;h->ready2=worker->ready2;
    h->wideband=worker->wideband;h->wideband_sent=worker->wideband_sent;
    h->pauses=worker->pauses;h->pauses_sent=worker->pauses_sent;
}
static int configure(OldInst *h,const struct reloq61_request *q,int16_t *pcm)
{
    int ok=ev_setOutputBuffer(h,4096,pcm);
    ok=ok && ev_setParam(h,0,1)>=0;
    ok=ok && ev_setParam(h,1,q->annotations)!=-1;
    ok=ok && ev_setParam(h,2,0)>=0;
    ok=ok && ev_setParam(h,3,0)>=0;
    ok=ok && ev_setParam(h,5,q->sample_rate==8000?0:1)>=0;
    ok=ok && ev_setParam(h,33,0)>=0;
    ok=ok && ev_setParam(h,7,q->phonemes_only?1:0)>=0;
    for(int k=0;k<8;++k)ok=(vc_setVoiceParam(h,0,k,q->voice[k])>=0)&&ok;
    return ok;
}
static int32_t STDCALL idle_message(void *h,int32_t kind,int32_t value,void *context)
{
    (void)h;(void)kind;(void)value;(void)context;return 1;
}
static int prepare_next(struct provider_worker *worker,const struct reloq61_request *q)
{
    OldInst *h=worker->handle;
    if(api_reset(h->fresh,q->language))return 0;
    if(dictionary_bind(worker,ew_machine(((ECIinstance *)h->fresh)->thread->engine))) {
        dictionary_clear(worker);return 0;
    }
    restore_controller(worker);
    /* Finish the complete reset and setup after this request's controls have
       been delivered. The next utterance can start from a fresh prepared
       state without redoing setup ahead of its first audio callback. */
    eo_registerCallback(h,(void *)idle_message,NULL);
    int ok=configure(h,q,worker->pcm)&&ev_sendParameters(h)&&
       et_synthesize(h)&&es_synchronize(h);
    eo_registerCallback(h,NULL,NULL);
    if(!ok)return 0;
    worker->prepared=1;worker->last_phonemes=q->phonemes_only;
    worker->last_annotations=q->annotations;
    worker->last_sample_rate=q->sample_rate;
    memcpy(worker->last_voice,q->voice,sizeof worker->last_voice);
    return 1;
}
struct capture {
    struct reloq61_english_bank **banks;size_t bank_count,bank_capacity;
    const struct reloq61_request *q;
    struct reloq61_result *r;
    char phon[4096];
    void *index_original, *index_data;
    struct SynthThread *thread;
    int64_t clock_base;
    size_t next_mark,consumed,setup_pending;
    int metadata,source_started,pause_policy;
    pause_sink pause;void *pause_context;
};
int reloq61_labiodental_gain(void *state)
{
    delta_state *d=state;DeltaLang *dl=EVV_AT(DeltaLang *,d->dlang);
    const struct capture *c=dl->reloq_context;
    return dl->reloq_frame&&c?reloq_voice_labiodental_gain((float)c->q->output_rate):0;
}
static void report_index(struct capture *c,int32_t index,int64_t tick)
{
    if(c->q->index)c->q->index(c->q->context,index,tick);
}
void reloq61_shorten_phrase_pause(void *state,void *token,int duration_field)
{
    delta_state *d=state;DeltaLang *dl=EVV_AT(DeltaLang *,d->dlang);
    struct capture *c=dl->reloq_context;
    if(!dl->reloq_frame||!c||c->pause_policy!=2)return;
    delta_loc *loc=token;delta_operand value;
    /* The punctuation rule supplies its own duration field. French,
       Canadian French and Finnish use different native record layouts. */
    loc->field=(int16_t)duration_field;vinitloc_new(state,&value,loc);
    if((value.kind==DK_SHORT||value.kind==DK_SHORT2) && *(int16_t *)value.ptr>1)
        *(int16_t *)value.ptr=1;
    loc->field=-1;
}
void reloq61_shorten_default_pause(void *state,void *token)
{
    reloq61_shorten_phrase_pause(state,token,11);
}
void reloq61_note_pause(void *state,int32_t begin,int32_t end,int32_t after)
{
    delta_state *d=state;DeltaLang *dl=EVV_AT(DeltaLang *,d->dlang);
    struct capture *c=dl->reloq_context;
    if(!dl->reloq_frame||!c||!c->pause||end<=begin)return;
    int64_t a=c->clock_base+begin/10000,b=c->clock_base+end/10000,z=c->clock_base+after/10000;
    c->pause(c->pause_context,a,b,z);
}
static int32_t STDCALL message(void *h,int32_t kind,int32_t value,void *context)
{
    struct capture *c=context;(void)h;
    if(kind==1 && value>0) {
        size_t n=(size_t)value;
        if(n>sizeof c->phon || !c->q->phonemes ||
           c->r->phonemes_length+n>=c->q->phonemes_capacity) {
            c->r->error=1;return 2;
        }
        memcpy(c->q->phonemes+c->r->phonemes_length,c->phon,n);
        c->r->phonemes_length+=n;
        c->q->phonemes[c->r->phonemes_length]=0;
    }
    return 1;
}
static int frame(void *context,const int32_t *p,int32_t volume)
{
    struct capture *c=context;
    if(c->q->frame && c->q->frame(c->q->context,p,volume)) {
        c->r->error=2;return 0;
    }
    ++c->r->frames;c->r->ticks+=p[0];return 1;
}
/* Compact planning bypasses dense frame interpolation and PCM work. The
 * original C rules and their sparse targets still run exactly once. */
int reloq61_wants_spans(void *context)
{
    const struct capture *c=context;return c->q->span!=NULL;
}
void reloq61_clock_begin(void *context,int32_t from)
{
    struct capture *c=context;
    /* The module restarts its local array clock for a new text stretch.
       Source indices exposed to the host use one utterance-wide clock. */
    if(!from)c->clock_base=c->r->ticks;
}
int reloq61_array_span(void *context,int32_t from,int32_t to,
    const struct reloq61_curve curves[62],int32_t volume)
{
    struct capture *c=context;
    if(c->q->span(c->q->context,from,to,curves,volume)) {
        c->r->error=2;return 0;
    }
    c->r->frames+=(size_t)(to-from);c->r->ticks+=to-from;return 1;
}
static void index_at(int32_t value,void *context)
{
    struct capture *c=context;
    if(c->index_original)
        ((void (*)(int32_t,void *))c->index_original)(value,c->index_data);
}
void reloq61_index_at(void *context,int32_t value,int64_t tick)
{
    struct capture *c=context;
    /* Device IDs belong to IndexManager. Its payload is the caller's
       index; reporting the internal ID would silently lose/reorder marks. */
    if(value>0 && c->q->index) {
        Index *ix=sti_getIndex(&c->thread->indexmgr,(uint32_t)value);
        if(ix&&ix->kind==0)report_index(c,ix->payload,c->clock_base+tick);
    }
}
void reloq61_word_at(void *context,int32_t characters)
{
    struct capture *c=context;
    if(!c->q->phonemes_only) {
        size_t source_characters=characters>0?(size_t)characters:0;
        size_t setup=source_characters<c->setup_pending?source_characters:c->setup_pending;
        source_characters-=setup;c->setup_pending-=setup;
        /* The callback describes the word about to be spoken. Positions
           consumed by the preceding callback belong at this word's start,
           rather than at the start of the preceding word. */
        if(c->metadata&&c->source_started) {
            while(c->next_mark<c->q->mark_count &&
                  c->q->marks[c->next_mark].offset<=c->consumed) {
                const struct reloq61_mark *m=c->q->marks+c->next_mark++;
                if(m->index<0)continue;
                /* Frames already emitted for preceding words are the host's
                   authoritative clock. The unused native PCM device keeps
                   its own lookahead watermark across normal utterances. */
                if(c->q->index)report_index(c,m->index,c->r->ticks);
            }
            c->consumed+=source_characters;
        }
        stb_staticWordCallback(characters,c->thread);
    }
}
void reloq61_parameter_begin(void *engine,int32_t characters)
{
    delta_state *d=ew_machine(engine);
    if(!d)return;
    DeltaLang *dl=EVV_AT(DeltaLang *,d->dlang);
    if(dl->reloq_frame&&characters>0)
        ((struct capture *)dl->reloq_context)->setup_pending+=(size_t)characters;
}
void reloq61_source_begin(void *engine)
{
    delta_state *d=ew_machine(engine);
    if(!d)return;
    DeltaLang *dl=EVV_AT(DeltaLang *,d->dlang);
    if(dl->reloq_frame)((struct capture *)dl->reloq_context)->source_started=1;
}
static int greek_english(void *context,const char *text,char *phonemes,size_t capacity)
{
    const struct reloq61_request *request=context;
    struct reloq61_request q=*request;struct reloq61_result r;
    q.language=0x10000;q.text=text;q.marks=NULL;q.mark_count=0;
    q.phonemes_only=1;q.phonemes=phonemes;q.phonemes_capacity=capacity;
    q.annotations=0;q.index=NULL;q.frame=NULL;q.span=NULL;
    struct provider_worker *worker=owner_worker;
    void *previous=reloq61_worker_scope(worker&&worker->language==0x120000?worker->english:NULL);
    int result=reloq61_generate(&q,&r);
    reloq61_worker_scope(previous);return result;
}
/* The foreign-word rule calls the same native English analyzer used by an
   English voice. Only the current lexical word is analyzed: the caller's
   sentence, language, marks and acoustic clock stay in their own kernel. */
int reloq61_english_word(void *state,const char *text,char *phonemes,size_t capacity)
{
    delta_state *d=state;DeltaLang *dl=EVV_AT(DeltaLang *,d->dlang);
    const struct capture *c=dl->reloq_context;
    if(!dl->reloq_frame||!c)return -1;
    struct reloq61_request q=*c->q;struct reloq61_result r;
    q.language=0x10000;q.text=text;q.marks=NULL;q.mark_count=0;
    q.phonemes_only=1;q.phonemes=phonemes;q.phonemes_capacity=capacity;
    q.annotations=0;q.index=NULL;q.frame=NULL;q.span=NULL;
    struct provider_worker *worker=owner_worker;
    void *previous=reloq61_worker_scope(worker?worker->english:NULL);
    int status=reloq61_generate(&q,&r);
    reloq61_worker_scope(previous);return status;
}
extern int reloq61_english_word(void *,const char *,char *,size_t);
int reloq61_english_word_context(void *state,const char *text,char *phonemes,size_t capacity,
    struct reloq61_english_segment *phones,size_t phone_capacity,size_t *count,struct reloq61_english_spelling *spelling,
    uint32_t *classes,unsigned (*classify)(const char *),int prefer_segments)
{
    struct reloq61_english_letter_mark letters[256];
    struct reloq61_english_capture plan={.phones=phones,.capacity=phone_capacity,.letters=letters,.letter_capacity=256,
        .source_classes=classes,.classify_source=classify,.prefer_segments=prefer_segments};
    void *previous=reloq61_english_plan_scope(&plan);
    int status=reloq61_english_word(state,text,phonemes,capacity);
    reloq61_english_plan_scope(previous);
    *count=status||plan.failed?0:plan.count;
    for(size_t i=0;i<*count;++i)phones[i].origin&=127;
    if(spelling){spelling->count=status?0:plan.letter_count;for(size_t i=0;i<spelling->count;++i)spelling->letters[i]=letters[i].letter;}
    return status;
}
int reloq61_english_word_plan(void *state,const char *text,char *phonemes,size_t capacity,
    struct reloq61_english_segment *phones,size_t phone_capacity,size_t *count,struct reloq61_english_spelling *spelling)
{
    return reloq61_english_word_context(state,text,phonemes,capacity,phones,phone_capacity,count,spelling,NULL,NULL,0);
}

/* Word sync points are recycled at sentence reset. Bank ownership ends at
 * that same boundary, in speech and phoneme-only requests alike. Keeping old
 * mappings would mistake a later word for an already-emitted earlier bank. */
void reloq61_english_banks_reset(void*state)
{
 delta_state*d=state;DeltaLang*dl=EVV_AT(DeltaLang*,d->dlang);
 struct capture*c=dl->reloq_context;if(!dl->reloq_frame||!c)return;
 for(size_t i=0;i<c->bank_count;++i)reloq61_english_bank_free(c->banks[i]);
 c->bank_count=0;
}
unsigned reloq61_english_bank_attach(void*state,struct reloq61_english_bank*p)
{
 delta_state*d=state;DeltaLang*dl=EVV_AT(DeltaLang*,d->dlang);struct capture*c=dl->reloq_context;if(!dl->reloq_frame||!c||!p)return 0;
 if(c->bank_count==c->bank_capacity){size_t cap=c->bank_capacity?c->bank_capacity*2:8;if(cap>65535)return 0;void*v=realloc(c->banks,cap*sizeof*c->banks);if(!v)return 0;c->banks=v;c->bank_capacity=cap;}
 c->banks[c->bank_count++]=p;return (unsigned)c->bank_count;
}
struct reloq61_english_bank *reloq61_english_bank_at(void*state,unsigned index)
{
 delta_state*d=state;DeltaLang*dl=EVV_AT(DeltaLang*,d->dlang);struct capture*c=dl->reloq_context;return dl->reloq_frame&&c&&index&&index<=c->bank_count?c->banks[index-1]:NULL;
}
struct reloq61_english_bank *reloq61_english_bank_for_word(void*state,int32_t record)
{
 delta_state*d=state;DeltaLang*dl=EVV_AT(DeltaLang*,d->dlang);struct capture*c=dl->reloq_context;
 if(!dl->reloq_frame||!c||!record)return NULL;
 int32_t left=*EVV_AT(int32_t*,record)&~3;
 for(size_t i=0;i<c->bank_count;++i){struct reloq61_english_bank*p=c->banks[i];for(size_t j=0;j<p->word_count;++j)if(p->word_points[j]==left)return p;}
 return NULL;
}
static struct reloq61_bank_span *bank_span(struct reloq61_english_bank*p)
{
 if(p->span_count==p->span_capacity){size_t cap=p->span_capacity?p->span_capacity*2:32;if(cap>16384){p->failed=1;return NULL;}void*v=realloc(p->spans,cap*sizeof*p->spans);if(!v){p->failed=1;return NULL;}p->spans=v;p->span_capacity=cap;}
 return p->spans+p->span_count++;
}
static int bank_take_span(void*context,int32_t from,int32_t to,const struct reloq61_curve curves[62],int32_t volume)
{
 struct reloq61_bank_span*p=bank_span(context);if(!p)return 1;memcpy(p->curve,curves,sizeof p->curve);p->from=from;p->to=to;p->volume=volume;p->step=1;return 0;
}
static int bank_take_frame(void*context,const int32_t values[62],int32_t volume)
{
 struct reloq61_bank_span*p=bank_span(context);if(!p)return 1;for(unsigned k=0;k<62;++k)p->curve[k]=(struct reloq61_curve){0,values[k],values[0],values[k]};p->from=0;p->to=values[0];p->volume=volume;p->step=values[0];return 0;
}
extern int reloq61_generate_with_pauses(const struct reloq61_request *,struct reloq61_result *,int,void(*)(void*,int64_t,int64_t,int64_t),void*);
static void bank_pause(void*context,int64_t from,int64_t to,int64_t after)
{
 (void)after;struct reloq61_english_bank*p=context;if(to>from){p->pause_from=from;p->pause_to=to;}
}
/* Lexical analysis supplies the host graph first. Acoustic work is deferred
 * until its Mandarin phrase boundaries are known, retaining no future word's
 * acoustic spans. This is request-local state, not a pronunciation cache. */
static int bank_generate(void*state,struct reloq61_english_bank*p,int lexical)
{
 delta_state*d=state;DeltaLang*dl=EVV_AT(DeltaLang*,d->dlang);const struct capture*c=dl->reloq_context;
 if(!dl->reloq_frame||!c)return -1;
 struct reloq61_request q=*c->q;struct reloq61_result result;
 q.language=0x10000;q.text=p->text;q.marks=NULL;q.mark_count=0;q.annotations=0;q.index=NULL;q.phonemes_only=lexical;
 /* Embedded English keeps the selected model, pitch and rate. Its neutral
  * inflection is local to this bank; the Mandarin request remains intact. */
 if(!lexical&&p->neutral_inflection)q.voice[3]=0;
 char phonemes[16384];q.phonemes=phonemes;q.phonemes_capacity=sizeof phonemes;q.context=p;q.span=bank_take_span;q.frame=bank_take_frame;
 struct provider_worker*w=owner_worker;void*previous_worker=reloq61_worker_scope(w?w->english:NULL);void*previous_bank=reloq61_english_bank_scope(p);
 int status=reloq61_generate_with_pauses(&q,&result,c->pause_policy,bank_pause,p);
 reloq61_english_bank_scope(previous_bank);reloq61_worker_scope(previous_worker);
 return status||p->failed?-1:0;
}
struct reloq61_english_bank *reloq61_english_bank_build(void*state,const char*text)
{
 struct reloq61_english_bank*p=calloc(1,sizeof*p);if(!p)return NULL;
 size_t n=strlen(text);if(n>=sizeof p->text){free(p);return NULL;}memcpy(p->text,text,n+1);p->neutral_inflection=1;
 if(bank_generate(state,p,1)||!p->phone_count||!p->word_count){reloq61_english_bank_free(p);return NULL;}return p;
}
int reloq61_english_bank_render(void*state,struct reloq61_english_bank*p,int nonfinal)
{
 if(!p||p->rendering||p->emitted||p->span_count)return -1;
 p->rendering=1;p->nonfinal=nonfinal;int result=bank_generate(state,p,0);p->rendering=0;
 return result||!p->span_count||!p->atom_count||!p->syllable_count?-1:0;
}

extern int32_t arrayStreamFind(void*,const char*);
extern void reloq61_array_bank_begin(delta_state*,int32_t);
extern int reloq61_array_bank_put(void*,int,int32_t,int32_t);
extern int32_t sendArrayParameters(delta_state*,int32_t,int32_t,int32_t,int32_t,int32_t,int32_t,int32_t,const int32_t*,const int32_t*);
static int32_t bank_value(const struct reloq61_curve*c,int32_t at)
{
 if(c->at_l==c->at_r||c->val_l==c->val_r)return c->val_l;
 float value=(float)(at-c->at_l);value*=(float)(c->val_r-c->val_l);value/=(float)(c->at_r-c->at_l);return (int32_t)((float)c->val_l+value);
}
int reloq61_english_bank_prepare(void *state,const struct reloq61_english_bank *p,int32_t from)
{
 delta_state *d=state;DeltaLang *dl=EVV_AT(DeltaLang*,d->dlang);
 /* Native arrays restart at zero for each new text stretch. Mirror that
  * reset before checking the bank's local monotonic clock. */
 if(!from)reloq61_array_reset(d);
 if(from<dl->at||!p->span_count)return -1;if(from==dl->at)return 0;
 int32_t map[63],defaults[62];map[0]=1;
 for(unsigned k=0;k<62;++k){map[k+1]=k?arrayStreamFind(d,evv_klatt_parm_names[k]):-1;defaults[k]=bank_value(p->spans[0].curve+k,0);}
 /* This window completes existing native targets. Keep their interpolation
  * cursors: resetting them discards a Chinese vowel's already-read F0 tail. */
 return sendArrayParameters(d,dl->at,from,1,0,1,0,1,map,defaults)?0:-1;
}
static int32_t bank_final_pitch(const struct reloq61_english_bank *p,int32_t limit)
{
 size_t i=0;while(i+1<p->pitch_count&&p->pitch[i+1].at<=limit)++i;
 if(i+1==p->pitch_count)return p->pitch[i].value;
 const struct reloq61_bank_pitch *a=p->pitch+i,*b=a+1;
 struct reloq61_curve c={a->at,a->value,b->at,b->value};return bank_value(&c,limit);
}
int reloq61_english_bank_emit(void*state,const struct reloq61_english_bank*p,int32_t from,int32_t to)
{
 delta_state*d=state;DeltaLang*dl=EVV_AT(DeltaLang*,d->dlang);struct capture*c=dl->reloq_context;
 if(!dl->reloq_frame||!c||!p||!p->span_count)return -1;
 if(from<dl->at||to<from)return -1;
 int32_t map[63],defaults[62];map[0]=1;for(unsigned k=0;k<62;++k){map[k+1]=k?arrayStreamFind(d,evv_klatt_parm_names[k]):-1;defaults[k]=bank_value(p->spans[0].curve+k,0);}
 /* The preceding Chinese word may retain a bounded acoustic tail. Emit
  * that tail in its own clock before replacing any future targets. */
 if(from>dl->at&&!sendArrayParameters(d,dl->at,from,1,0,1,0,1,map,defaults))return -1;
 int32_t limit=p->spans[p->span_count-1].to;if(p->trim_pause&&p->pause_to>p->pause_from)limit=(int32_t)p->pause_from;
 if(limit<=0||from>INT32_MAX-limit||to-from!=limit)return -1;
 reloq61_array_bank_begin(d,from);
 for(unsigned k=1;k<62;++k){int stream=map[k+1];if(stream<0)continue;
  if(k==1&&p->pitch_count){
   if(!reloq61_array_bank_put(d,stream,from,p->pitch[0].value))return -1;
   for(size_t i=0;i<p->pitch_count;++i)if(p->pitch[i].at>=0&&p->pitch[i].at<=limit&&!reloq61_array_bank_put(d,stream,from+p->pitch[i].at,p->pitch[i].value))return -1;
   if(!reloq61_array_bank_put(d,stream,from+limit,bank_final_pitch(p,limit)))return -1;continue;
  }
  int32_t last_at=0,last_value=bank_value(p->spans[0].curve+k,0);
  if(!reloq61_array_bank_put(d,stream,from,last_value))return -1;
  const struct reloq61_curve*last_curve=p->spans[0].curve+k;
  for(size_t i=0;i<p->span_count;++i){const struct reloq61_bank_span*s=p->spans+i;if(s->from>=limit)break;const struct reloq61_curve*q=s->curve+k;last_curve=q;
   int32_t times[2]={q->at_l,q->at_r},values[2]={q->val_l,q->val_r};
   for(unsigned j=0;j<2;++j){int32_t at=times[j],value=values[j];if(at<last_at||at<0||at>limit||(at==last_at&&value==last_value))continue;if(!reloq61_array_bank_put(d,stream,from+at,value))return -1;last_at=at;last_value=value;}
  }
  int32_t value=bank_value(last_curve,limit);if((last_at!=limit||last_value!=value)&&!reloq61_array_bank_put(d,stream,from+limit,value))return -1;
 }
 dl->reloq_volume=p->spans[0].volume;
 if(!sendArrayParameters(d,from,from+limit,1,1,1,0,1,map,defaults))return -1;
 /* A following native window may restart its cursors. Leave the exact
  * boundary values queued so that its first interpolation is continuous. */
 for(unsigned k=1;k<62;++k)if(map[k+1]>=0){const struct reloq61_curve*last=p->spans[0].curve+k;for(size_t i=0;i<p->span_count&&p->spans[i].from<limit;++i)last=p->spans[i].curve+k;if(!reloq61_array_bank_put(d,map[k+1],from+limit,(k==1&&p->pitch_count)?bank_final_pitch(p,limit):bank_value(last,limit)))return -1;}


 if(!p->trim_pause&&p->pause_to>p->pause_from&&c->pause)c->pause(c->pause_context,c->clock_base+from+p->pause_from,c->clock_base+from+p->pause_to,c->clock_base+from+p->pause_to);
 return 0;
}
static int generate_core(const struct reloq61_request *,struct reloq61_result *);
static int greek_generate(const struct reloq61_request *q,struct reloq61_result *r)
{
    char *text=NULL;size_t *map=NULL;
    if(reloq_greek_text(q->text,q->annotations,greek_english,(void *)q,&text,&map))return -1;
    struct reloq61_mark *marks=NULL;
    if(q->mark_count){marks=malloc(q->mark_count*sizeof *marks);if(!marks){free(text);free(map);return -1;}}
    size_t length=strlen(q->text);
    for(size_t i=0;i<q->mark_count;++i){if(q->marks[i].offset>length){free(text);free(map);free(marks);return -1;}marks[i]=q->marks[i];marks[i].offset=map[marks[i].offset];}
    struct reloq61_request request=*q;request.language=0x120000;request.text=text;request.marks=marks;request.annotations=1;
    int result=generate_core(&request,r);
    free(text);free(map);free(marks);return result;
}
int reloq61_generate(const struct reloq61_request *q,struct reloq61_result *r)
{
    if(q&&r&&q->language==0x120000){initialize();memset(r,0,sizeof *r);return greek_generate(q,r);}
    return generate_core(q,r);
}
static int generate_core(const struct reloq61_request *q,struct reloq61_result *r)
{
    struct capture c;OldInst *h;delta_state *d;DeltaLang *dl;
    int16_t pcm[4096];size_t begin=0,n;int ok=1;
    if(!q||!r||!q->text||(q->mark_count&&!q->marks)||
       (q->sample_rate && q->sample_rate!=8000 && q->sample_rate!=11025))return -1;
    memset(r,0,sizeof *r);memset(&c,0,sizeof c);c.q=q;c.r=r;
    c.pause_policy=pause_request.policy;c.pause=pause_request.sink;c.pause_context=pause_request.context;
    if(q->phonemes && q->phonemes_capacity)q->phonemes[0]=0;
    struct provider_worker *worker=owner_worker&&owner_worker->language==q->language?owner_worker:NULL;
    int annotated=strchr(q->text,'`')!=NULL;
    int reset=worker&&(!worker->prepared||annotated||worker->last_phonemes!=q->phonemes_only||
        worker->last_annotations!=q->annotations||worker->last_sample_rate!=q->sample_rate||
        memcmp(worker->last_voice,q->voice,sizeof worker->last_voice));
    int prepared=worker&&worker->handle&&!reset;
    initialize();h=worker?worker->handle:NULL;
    void *previous_inline=reloq61_inline_scope(h?&((ECIinstance *)h->fresh)->queue:NULL);
    if(h) {
        /* Restore the complete engine environment before reuse, including
           annotation/spelling modes changed by the preceding text. */
        eo_registerCallback(h,NULL,NULL);
        eo_clearInput(h);
        if(reset && api_reset(h->fresh,q->language)) {
            reloq61_inline_scope(NULL);release_handle(worker,h);h=NULL;
        }
        else if(reset) {
            /* 6.1 intentionally sends voice settings only on change, even
               when ready is set. Reset its controller's sent-value state
               to the same defaults as the freshly reset native engine. */
            restore_controller(worker);
        }
    }
    if(!h) {
        h=eo_newEx(q->language);
        if(worker&&h) {
            memcpy(worker->env,h->env,sizeof h->env);
            memcpy(worker->env_saved,h->env_saved,sizeof h->env_saved);
            memcpy(worker->voice,h->voice,sizeof h->voice);
            memcpy(worker->voice_saved,h->voice_saved,sizeof h->voice_saved);
            worker->ready=h->ready;worker->ready2=h->ready2;
            worker->wideband=h->wideband;worker->wideband_sent=h->wideband_sent;
            worker->pauses=h->pauses;worker->pauses_sent=h->pauses_sent;
        }
    }
    if(worker)worker->handle=h;
    if(worker)worker->prepared=0;
    if(!h){reloq61_inline_scope(previous_inline);r->error=100;return -1;}
    /* Drain constructor messages before moving this private instance onto
       the already serialized host worker. The engine setup and message
       bodies remain identical to the ordinary ECI path. */
    if(!es_synchronize(h)) {
        release_handle(worker,h);
        reloq61_inline_scope(previous_inline);r->error=102;return -1;
    }
    d=ew_machine(((ECIinstance *)h->fresh)->thread->engine);
    c.thread=((ECIinstance *)h->fresh)->thread;
    if(!d){r->error=101;release_handle(worker,h);reloq61_inline_scope(previous_inline);return -1;}
    reloq61_inline_scope(&((ECIinstance *)h->fresh)->queue);
    if(worker&&dictionary_bind(worker,d)) {
        dictionary_clear(worker);
        r->error=103;release_handle(worker,h);reloq61_inline_scope(previous_inline);return -1;
    }
    dl=EVV_AT(DeltaLang *,d->dlang);
    /* The private host owns PCM and starts each request's clock at zero.
       The native device would otherwise retain its preceding absolute
       sample/index watermark even though its array clock restarts. */
    reloq61_array_reset(d);
    r->unsupported=dl->unsupported61;
    dl->reloq_frame=frame;dl->reloq_context=&c;
    c.index_original=dl->device->index_cb;c.index_data=dl->device->index_data;
    dl->device->index_cb=(void *)index_at;dl->device->index_data=&c;
    eo_registerCallback(h,(void *)message,&c);
    ok=prepared?1:configure(h,q,worker?worker->pcm:pcm);
    n=strlen(q->text);
    int metadata=1;
    for(size_t j=0;j<q->mark_count;++j) {
        if(q->marks[j].offset>n || (j&&q->marks[j].offset<q->marks[j-1].offset))ok=0;
        if(q->marks[j].pitch>=0)metadata=0;
    }
    if(metadata) {
        /* Source positions are metadata, not AddText boundaries. Splitting
           raw input at an index changes native normalization of times,
           addresses and abbreviations. Let the existing consumed-character
           callbacks advance the mark queue while parsing the whole text. */
        c.metadata=1;
        if(ok&&n)ok=et_addText(h,q->text);
    }
    /* Explicit provider pitch annotations are the legacy adapter route.
       Production ReLoq pitch changes use metadata and its own renderer. */
    for(size_t j=0;!metadata&&j<q->mark_count && ok;++j) {
        const struct reloq61_mark *m=q->marks+j;
        if(m->offset<begin||m->offset>n){ok=0;break;}
        char *part=malloc(m->offset-begin+1);
        if(!part){ok=0;break;}
        memcpy(part,q->text+begin,m->offset-begin);part[m->offset-begin]=0;
        if(*part)ok=et_addText(h,part);free(part);
        if(m->pitch>=0) {
            char control[32];
            if(m->pitch>100){ok=0;break;}
            /* Queue a pitch annotation within the same phrase. Changing
               the host voice here would finish the preceding text stretch
               and change its timing as well as its pitch. */
            snprintf(control,sizeof control,"`vb%d ",m->pitch);
            ok=et_addText(h,control)&&ok;
        }
        if(m->index>=0)ok=et_insertIndex(h,m->index)&&ok;
        begin=m->offset;
    }
    if(!metadata && ok && n>begin)ok=et_addText(h,q->text+begin);
    /* These 6.1 modules report consumed input through synthesizingWord.
       Explicitly wire that route for the private provider; capability
       negotiation inherited from the 4.3 host otherwise leaves it quiet
       and holds every source index until the final flush. */
    if(!q->phonemes_only) {
        ELOQ_CB(d,0x14)=(void *)stb_staticWordCallback;
        ELOQ_CB(d,0x18)=((ECIinstance *)h->fresh)->thread;
    }
    if(ok) {
        if(q->phonemes_only)ok=ev_generatePhonemes(h,sizeof c.phon,c.phon);
        else ok=et_synthesize(h)&&es_synchronize(h);
    }
    if(ok&&c.metadata&&q->index)
        while(c.next_mark<q->mark_count) {
            const struct reloq61_mark *m=q->marks+c.next_mark++;
            if(m->index>=0)report_index(&c,m->index,r->ticks);
        }
    for(size_t i=0;i<c.bank_count;++i)reloq61_english_bank_free(c.banks[i]);
    free(c.banks);
    if(dl->unsupported61)r->unsupported=dl->unsupported61;
    dl->reloq_frame=0;dl->reloq_context=0;
    dl->device->index_cb=c.index_original;dl->device->index_data=c.index_data;
    eo_registerCallback(h,NULL,NULL);
    if(!worker || !ok || r->error || r->unsupported) {
        release_handle(worker,h);
    } else if(!annotated&&metadata&&!prepare_next(worker,q)) {
        release_handle(worker,h);
    }
    reloq61_inline_scope(previous_inline);
    if(r->unsupported)return 1;
    return ok&&!r->error?0:-1;
}
int reloq61_generate_with_pauses(const struct reloq61_request *q,struct reloq61_result *r,
    int policy,pause_sink sink,void *context)
{
    int previous_policy=pause_request.policy;pause_sink previous_sink=pause_request.sink;
    void *previous_context=pause_request.context;
    pause_request.policy=policy;pause_request.sink=sink;pause_request.context=context;
    int status=reloq61_generate(q,r);
    pause_request.policy=previous_policy;pause_request.sink=previous_sink;pause_request.context=previous_context;
    return status;
}
