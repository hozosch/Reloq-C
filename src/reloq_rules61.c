#include "reloq_rules61.h"
#include "reloq61.h"
#include "reloq_frame_adapter.h"
#include "reloq_german_input.h"
#include "reloq_german_voice_model.h"
#include "reloq_german_lexicon.h"
#include "reloq_inline_tags.h"
#include "reloq_character.h"
#include "reloq_utf8.h"
#include "reloq_voice_controls.h"
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Preserve admitted text until the native sentence frontend sees it. The
 * former word-by-word SPR conversion ran a second, older language analysis
 * and forced every ordinary token through its historical dictionaries. */
static int english_input(const unsigned char *text,size_t length,int tags,int unicode,
    unsigned char **out,size_t **source,size_t *written)
{
    if(length==SIZE_MAX||memchr(text,0,length))return -1;
    unsigned char *bytes=NULL;size_t *map=NULL,n=0,capacity=0;
    for(size_t at=0;at<length;) {
        size_t begin=at,end=at;uint32_t cp;
        if(reloq_utf8_next(text,length,&end,&cp)!=1)goto invalid;
        unsigned char encoded[8];size_t count=0;
        if(cp<128) {
            if(cp=='`') {
                struct reloq_inline_tag t=reloq_inline_tag_read(text+at,length-at,0);
                if(t.length&&(tags||t.kind==RELOQ_TAG_SPR||t.kind==RELOQ_TAG_PROMINENCE)) {
                    count=t.length;end=at+count;
                }
            }
            if(!count){encoded[0]=cp=='`'&&!tags?' ':(unsigned char)cp;count=1;}
        } else if(unicode) {
            count=end-at;memcpy(encoded,text+at,count);
        } else if(reloq_german_input_encode(text+at,end-at,encoded,sizeof encoded,&count,NULL)) {
            unsigned char base;
            if(reloq_character_latin_base(cp,&base)!=1)goto invalid;
            encoded[0]=base;count=1;
        }
        if(count>SIZE_MAX-n-1)goto memory;
        if(n+count+1>capacity) {
            size_t cap=capacity?capacity:256;
            while(cap<n+count+1){if(cap>SIZE_MAX/2){cap=n+count+1;break;}cap*=2;}
            if(cap>SIZE_MAX/sizeof *map)goto memory;
            void *p=realloc(bytes,cap);if(!p)goto memory;bytes=p;
            p=realloc(map,cap*sizeof *map);if(!p)goto memory;map=p;capacity=cap;
        }
        const unsigned char *part=cp=='`'&&count>1?text+at:encoded;
        memcpy(bytes+n,part,count);
        for(size_t k=0;k<count;++k)map[n+k]=begin;
        n+=count;at=end;
    }
    if(!bytes) {
        bytes=malloc(1);map=malloc(sizeof *map);
        if(!bytes||!map)goto memory;
    }
    bytes[n]=0;map[n]=length;*out=bytes;*source=map;*written=n;return 0;
memory:free(bytes);free(map);return -2;
invalid:free(bytes);free(map);return -1;
}

/* Each word is analyzed for this request. No word or pronunciation result
 * survives in thread-local storage. */
int reloq_rules61_word(int language,const unsigned char *word,size_t length,
    unsigned char *out,size_t capacity,size_t *written)
{
    if(!word||!written||(!out&&capacity))return -1;
    if(!length||length>=256)return 0;
    unsigned char bytes[256],spr[2048];size_t bytes_n=0,n=0;char phonemes[4096];
    if(language==0x40000) {memcpy(bytes,word,length);bytes[length]=0;bytes_n=length;}
    else if(reloq_german_input_encode(word,length,bytes,sizeof bytes,&bytes_n,NULL))return 0;
    struct reloq61_request q={.language=language,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,100},.text=(char *)bytes,
        .phonemes=phonemes,.phonemes_capacity=sizeof phonemes};
    struct reloq61_result r;
    if(reloq61_generate(&q,&r))return 0;
    char *a=strstr(phonemes,"`["),*b=a?strchr(a,']'):NULL;
    if(!a||!b||strstr(b+1,"`[")||(size_t)(b+1-a)>=sizeof spr)return 0;
    n=(size_t)(b+1-a);memcpy(spr,a,n);spr[n]=0;
    int status;
    if(language==0x40000)status=reloq_german_lexicon_validate_spr(spr,n,NULL)==1;
    else {
        struct reloq_english_phone *p=NULL;size_t count=0;
        status=!reloq_english_parse(spr,n,&p,&count,NULL)&&count>0;free(p);
    }
    if(!status)return 0;
    *written=n;if(capacity<=n)return -2;
    memcpy(out,spr,n+1);return 1;
}

struct rule_span {
    struct reloq61_curve curve[62];
    int32_t from,to,volume,step;
    int64_t tick;
};
struct quiet_cut { int64_t from,to,after; };
extern int reloq61_generate_with_pauses(const struct reloq61_request *,struct reloq61_result *,
    int,void (*)(void *,int64_t,int64_t,int64_t),void *);
struct rule_playback;
struct rule_plan {
    struct rule_span *spans;size_t count,capacity;
    struct quiet_cut *cuts;size_t cut_count,cut_capacity;
    int64_t ticks,*indices;size_t index_count;
    struct reloq_session *session;
    int memory;
    struct rule_playback *playback;
};
static int process_spans(struct rule_plan *,int final);
static void release_spans(struct rule_plan *);
static struct rule_span *new_span(struct rule_plan *s)
{
    if(reloq_session_cancelled(s->session))return NULL;
    release_spans(s);
    if(s->count==s->capacity) {
        size_t cap=s->capacity?s->capacity*2:16;
        if(cap<s->capacity||cap>SIZE_MAX/sizeof *s->spans){s->memory=1;return NULL;}
        void *n=realloc(s->spans,cap*sizeof *s->spans);
        if(!n){s->memory=1;return NULL;}s->spans=n;s->capacity=cap;
    }
    return s->spans+s->count++;
}
static int take_span(void *context,int32_t from,int32_t to,
    const struct reloq61_curve curves[62],int32_t volume)
{
    struct rule_plan *s=context;struct rule_span *f=new_span(s);
    if(!f)return 1;
    memcpy(f->curve,curves,sizeof f->curve);
    f->from=from;f->to=to;f->volume=volume;f->step=1;f->tick=s->ticks;
    s->ticks+=to-from;return s->playback?process_spans(s,0):0;
}
/* General fallback for a provider window whose step is not one ms. */
static int take_frame(void *context,const int32_t p[62],int32_t volume)
{
    struct rule_plan *s=context;struct rule_span *f=new_span(s);
    if(!f)return 1;
    for(unsigned k=0;k<62;++k)f->curve[k]=(struct reloq61_curve){0,p[k],p[0],p[k]};
    f->from=0;f->to=p[0];f->step=p[0];f->volume=volume;f->tick=s->ticks;
    s->ticks+=p[0];return s->playback?process_spans(s,0):0;
}
static void take_index(void *context,int32_t index,int64_t tick)
{
    struct rule_plan *s=context;
    if(index>=0&&(size_t)index<s->index_count && s->indices[index]!=INT64_MAX &&
       s->indices[index]!=0)s->indices[index]=tick;
}
static int32_t curve_value(const struct reloq61_curve *c,int32_t at)
{
    if(c->val_l==c->val_r || c->at_l==c->at_r)return c->val_l;
    float offset=(float)(at-c->at_l);
    offset*=(float)(c->val_r-c->val_l);offset/=(float)(c->at_r-c->at_l);
    return (int32_t)((float)c->val_l+offset);
}
/* A native span has immutable endpoints. Prepare its constant controls once,
 * then evaluate only changing fields at the original integer frame times. */
struct span_controls {
    float fixed[62];
    unsigned char moving[61];
    unsigned count;
};
static void prepare_controls(const struct rule_span *span,struct span_controls *p)
{
    p->count=0;p->fixed[0]=(float)span->step;
    for(unsigned k=1;k<62;++k) {
        const struct reloq61_curve *c=span->curve+k;
        p->fixed[k]=(float)c->val_l;
        if(c->val_l!=c->val_r&&c->at_l!=c->at_r)p->moving[p->count++]=(unsigned char)k;
    }
}
static void read_controls(const struct span_controls *p,const struct rule_span *span,
    int32_t at,float controls[62])
{
    memcpy(controls,p->fixed,sizeof p->fixed);
    for(unsigned j=0;j<p->count;++j) {
        unsigned k=p->moving[j];controls[k]=(float)curve_value(span->curve+k,at);
    }
}
static void take_pause(void *context,int64_t from,int64_t to,int64_t after)
{
    struct rule_plan *s=context;
    if(to-from<=3)return;
    if(s->cut_count==s->cut_capacity) {
        size_t cap=s->cut_capacity?s->cut_capacity*2:8;
        if(cap<s->cut_capacity||cap>SIZE_MAX/sizeof *s->cuts){s->memory=1;return;}
        void *p=realloc(s->cuts,cap*sizeof *s->cuts);if(!p){s->memory=1;return;}
        s->cuts=p;s->cut_capacity=cap;
    }
    s->cuts[s->cut_count++]=(struct quiet_cut){from+3,to,after};
}
/* Map original UTF-8 offsets to a complete SPR boundary. Inserting an
 * index inside a phoneme group would change the token stream. */
static size_t annotation_boundary(const unsigned char *a,size_t n,const size_t *map,size_t offset)
{
    if(!offset)return 0;
    size_t boundary=n;
    for(size_t j=0;j<n;) {
        if(map[j]>=offset){boundary=j;break;}
        struct reloq_inline_tag t=reloq_inline_tag_read(a+j,n-j,1);
        j+=t.length?t.length:1;
    }
    /* An AddText piece ending in punctuation lets the private ECI parser
     * finish a stretch before the following index arrives. Put the mark
     * at the preceding complete word edge, leaving punctuation and the
     * next text in one parser piece. This keeps the PCM independent of
     * source fragment/index positions. EOF is delivered after all audio. */
    while(boundary&&strchr(".,!?;:\n() \t",a[boundary-1]))--boundary;
    /* A source index within ordinary text belongs to the complete token.
       Native SPR groups were already skipped atomically above. */
    if(boundary&&boundary<n) {
        unsigned char previous=a[boundary-1],current=a[boundary];
        int left=(previous>='A'&&previous<='Z')||(previous>='a'&&previous<='z')||
            (previous>='0'&&previous<='9')||previous>=128||previous=='\'';
        int right=(current>='A'&&current<='Z')||(current>='a'&&current<='z')||
            (current>='0'&&current<='9')||current>=128||current=='\'';
        if(left&&right)while(boundary<n) {
            unsigned char c=a[boundary];
            if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||
                 (c>='0'&&c<='9')||c>=128||c=='\''))break;
            ++boundary;
        }
    }
    return boundary;
}
struct rule_delivery {
    reloq_session_pcm_sink sink;void *context;double gain;
    int16_t pending[1024];size_t used,limit,next_limit;
};
static int rule_flush(struct rule_delivery *d)
{
    if(!d->used)return 0;
    int status=d->sink(d->context,d->pending,d->used);
    d->used=0;d->limit=d->next_limit;return status;
}
static int rule_pcm(void *context,const int16_t *pcm,size_t count)
{
    struct rule_delivery *d=context;
    while(count) {
        size_t n=d->limit-d->used;if(n>count)n=count;
        for(size_t i=0;i<n;++i) {
            long x=lround(pcm[i]*d->gain);
            d->pending[d->used+i]=(int16_t)(x>32767?32767:x< -32768?-32768:x);
        }
        d->used+=n;
        if(d->used==d->limit&&rule_flush(d))return -1;
        pcm+=n;count-=n;
    }
    return 0;
}
/* Compact windows can be rendered as soon as their native word rules
 * finish. Only a source-silent run waits for the requested pause policy. */
struct rule_playback {
    struct reloq_german_voice_model model;
    const struct reloq_german_generic_settings *settings;
    const struct reloq_german_generic_pitch_mark *pitches;
    size_t index_count,pitch_count,next,next_pitch,next_cut,read;
    int32_t at;
    int pause_mode,final,result;
    double pitch_scale,base_pitch;
    struct rule_delivery delivery;
    reloq_german_generic_index_sink index_sink;
    void *context;
};
static void release_spans(struct rule_plan *s)
{
    /* Absolute ticks and marks are independent of these consumed windows.
     * Retain a window only while playback still holds part of a pause. */
    if(s->playback&&s->playback->read==s->count) {
        s->count=0;s->playback->read=0;
    }
}
static int render_until(struct rule_plan *s,int64_t limit)
{
    struct rule_playback *p=s->playback;
    struct span_controls prepared={0};const struct rule_span *previous=NULL;
    while(p->read<s->count) {
        const struct rule_span *f=s->spans+p->read;
        if(p->at<f->from)p->at=f->from;
        if(p->at>=f->to){++p->read;p->at=0;continue;}
        int32_t at=p->at;int64_t tick=f->tick+at-f->from;
        if(tick>=limit)break;
        p->at+=f->step;
        while(p->next_cut<s->cut_count&&tick>=s->cuts[p->next_cut].to)++p->next_cut;
        if(p->final&&p->next_cut<s->cut_count&&
           s->cuts[p->next_cut].after==s->ticks&&tick>=s->cuts[p->next_cut].from)continue;
        while(p->next<p->index_count && s->indices[p->next]>=0 && s->indices[p->next]<=tick) {
            size_t ordinal=p->next++;
            if(rule_flush(&p->delivery)){p->result=RELOQ_DE_UTTERANCE_SINK;return 1;}
            if(p->index_sink&&p->index_sink(p->context,ordinal)) {
                p->result=RELOQ_DE_UTTERANCE_SINK;return 1;
            }
        }
        while(p->next_pitch<p->pitch_count && s->indices[p->index_count+p->next_pitch]>=0 &&
              s->indices[p->index_count+p->next_pitch]<=tick)
            p->pitch_scale=reloq_voice_pitch_f0_scale(p->pitches[p->next_pitch++].native_pitch)/p->base_pitch;
        if(previous!=f){prepare_controls(f,&prepared);previous=f;}
        float controls[62];read_controls(&prepared,f,at,controls);
        if(p->pitch_scale!=1)controls[1]=(float)(controls[1]*p->pitch_scale);
        reloq_german_voice_model_frame(&p->model,controls);
        struct reloq_core_voiced_frame frame;
        if(reloq_frame_adapter_map(controls,reloq_german_voice_model_formants(&p->model),&frame)) {
            p->result=RELOQ_DE_UTTERANCE_INVALID;return 1;
        }
        frame.noise.s_frication=controls[8]>0&&(controls[38]>controls[37]+10||controls[39]>controls[37]+10);
        /* 2.70 is native volume 100; 2.02 belongs to preset volume 92. */
        p->delivery.gain=p->settings->volume/100.0*f->volume/270.0;
        size_t completed=0;
        int status=reloq_session_render_frames(s->session,&frame,1,rule_pcm,&p->delivery,&completed);
        if(status) {
            p->result=status==RELOQ_SESSION_CANCELLED?RELOQ_DE_UTTERANCE_CANCELLED:RELOQ_DE_UTTERANCE_SINK;
            return 1;
        }
    }
    return 0;
}
static int process_spans(struct rule_plan *s,int final)
{
    struct rule_playback *p=s->playback;
    if(p->result)return 1;
    if(!p->pause_mode)return render_until(s,s->ticks);
    p->final=final;
    int64_t limit=s->ticks;
    if(!final&&s->cut_count) {
        const struct quiet_cut *tail=s->cuts+s->cut_count-1;
        /* A following spoken span proves this was an internal pause.
           Until then hold only its known quiet body, never stop closures. */
        if(s->ticks<=tail->after&&limit>tail->from)limit=tail->from;
    }
    return render_until(s,limit);
}
int reloq_language_rules_render(int language,const unsigned char *text,size_t length,int tags,int native_volume,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink sink,
    reloq_german_generic_index_sink index_sink,void *context)
{
    if(!text||!settings||!session||!sink||(index_count&&!indices)||(pitch_count&&!pitches)||
       index_count>INT32_MAX||pitch_count>(size_t)INT32_MAX-index_count)return RELOQ_DE_UTTERANCE_INVALID;
    unsigned char *a=NULL;size_t *map=NULL,n=0;
    struct rule_plan plan={.session=session,.index_count=index_count+pitch_count};
    struct reloq61_mark *marks=NULL;int result=RELOQ_DE_UTTERANCE_INVALID;
    int family=language>>16;
    int input_status=english_input(text,length,tags,family==6||family==8||family==10||family==18,&a,&map,&n);
    if(input_status==-2)goto memory;
    if(input_status)goto done;
    /* Leading punctuation has no preceding spoken word to pause after. */
    for(size_t j=0;j<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+j,n-j,1);
        if(t.kind==RELOQ_TAG_SPR || (t.kind==RELOQ_TAG_SPELL&&t.value))break;
        if(!t.length&&!strchr(".,!?;:\n() \t",a[j]))break;
        if(!t.length&&strchr(".,!?;:\n()",a[j]))a[j]=' ';
        j+=t.length?t.length:1;
    }
    if(plan.index_count) {
        if(plan.index_count>SIZE_MAX/sizeof *plan.indices)goto memory;
        plan.indices=malloc(plan.index_count*sizeof *plan.indices);if(!plan.indices)goto memory;
        for(size_t i=0;i<plan.index_count;++i)plan.indices[i]=-1;
    }
    size_t mark_count=index_count+pitch_count;
    if(mark_count) {
        if(mark_count>SIZE_MAX/sizeof *marks)goto memory;
        marks=malloc(mark_count*sizeof *marks);if(!marks)goto memory;
        size_t i=0,p=0;
        for(size_t j=0;j<mark_count;++j) {
            int is_index=p==pitch_count||(i<index_count&&indices[i].annotation_offset<=pitches[p].annotation_offset);
            size_t offset=is_index?indices[i].annotation_offset:pitches[p].annotation_offset;
            if(offset>length)goto done;
            if(!is_index&&(pitches[p].native_pitch<0||pitches[p].native_pitch>100))goto done;
            marks[j]=(struct reloq61_mark){annotation_boundary(a,n,map,offset),
                (int32_t)(is_index?i:index_count+p),-1};
            if(j&&marks[j].offset<marks[j-1].offset)goto done;
            if(is_index)++i;else ++p;
        }
    }
    /* Preserve the existing frontend's Unicode, numeric and index ownership;
     * all connected phonetic timing and targets now come from native rules. */
    struct reloq61_request q={.language=language,.annotations=1,.text=(char *)a,
        .marks=marks,.mark_count=mark_count,.frame=take_frame,.index=take_index,.context=&plan,
        .span=take_span,.sample_rate=reloq_session_sample_rate(session)==8000?8000:0,
        .output_rate=(int)reloq_session_sample_rate(session)};
    q.voice[0]=settings->gender;
    q.voice[1]=settings->head_size_plus_one?settings->head_size_plus_one-1:50;
    q.voice[2]=settings->native_pitch_plus_one?settings->native_pitch_plus_one-1:settings->pitch;
    q.voice[3]=settings->native_inflection_plus_one?settings->native_inflection_plus_one-1:30;
    q.voice[4]=settings->roughness;q.voice[5]=settings->breathiness;
    q.voice[6]=settings->native_speed?settings->native_speed:settings->rate;
    q.voice[7]=native_volume;
    /* Only an explicit trailing pause protects the final silence. A p0
       reset or an earlier pause must not disable the entire policy. */
    int explicit_pause=0;
    for(size_t j=0;j<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+j,n-j,1);
        if(t.kind==RELOQ_TAG_PAUSE)explicit_pause=t.value>0;
        else if(t.kind==RELOQ_TAG_SPR || (!t.length &&
            !strchr(".,!?;:\n()[]{}\"' \t",a[j])))explicit_pause=0;
        j+=t.length?t.length:1;
    }
    for(size_t i=0;i<index_count;++i) {
        if(!indices[i].annotation_offset)plan.indices[i]=0;
        else if(indices[i].annotation_offset>=length)plan.indices[i]=INT64_MAX;
    }
    for(size_t i=0;i<pitch_count;++i) {
        if(!pitches[i].annotation_offset)plan.indices[index_count+i]=0;
        else if(pitches[i].annotation_offset>=length)plan.indices[index_count+i]=INT64_MAX;
    }
    struct rule_playback playback={
        .model={q.voice[1],q.voice[4],q.voice[5],q.voice[0],reloq_session_sample_rate(session)},
        .settings=settings,.pitches=pitches,.index_count=index_count,.pitch_count=pitch_count,
        .pause_mode=!explicit_pause&&settings->pause_mode==1,
        .pitch_scale=1,.base_pitch=reloq_voice_pitch_f0_scale(q.voice[2]),
        .delivery={.sink=sink,.context=context,.gain=1,
            .limit=(size_t)reloq_session_sample_rate(session)/100,
            .next_limit=(size_t)reloq_session_sample_rate(session)/50},
        .index_sink=index_sink,.context=context};
    plan.playback=&playback;
    struct reloq61_result r;
    int status=reloq61_generate_with_pauses(&q,&r,settings->pause_mode,
        playback.pause_mode?take_pause:NULL,&plan);
    if(reloq_session_cancelled(session)){result=RELOQ_DE_UTTERANCE_CANCELLED;goto done;}
    if(plan.memory)goto memory;
    if(playback.result){result=playback.result;goto done;}
    /* Every production phonetic rule is native C. A provider error stops
     * the stream; it never switches acoustic implementations mid-sentence. */
    if(status) {
        if(getenv("RELOQ_RULE_DEBUG"))fprintf(stderr,"rules61 status=%d unsupported=%d error=%d spans=%zu\n",status,r.unsupported,r.error,plan.count);
        goto done;
    }
    for(size_t i=0;i<plan.index_count;++i)
        if(plan.indices[i]<0 || plan.indices[i]==INT64_MAX)plan.indices[i]=plan.ticks;
    if(process_spans(&plan,1)) {
        if(plan.memory)goto memory;
        result=playback.result;goto done;
    }
    if(rule_flush(&playback.delivery)){result=RELOQ_DE_UTTERANCE_SINK;goto done;}
    while(playback.next<index_count) {
        size_t ordinal=playback.next++;
        if(index_sink&&index_sink(context,ordinal)){result=RELOQ_DE_UTTERANCE_SINK;goto done;}
    }
    result=RELOQ_DE_UTTERANCE_OK;goto done;
memory:result=RELOQ_DE_UTTERANCE_MEMORY;
done:free(a);free(map);free(marks);free(plan.spans);free(plan.cuts);free(plan.indices);return result;
}

int reloq_english_rules_render(const unsigned char *text,size_t length,int tags,int native_volume,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink sink,
    reloq_german_generic_index_sink index_sink,void *context)
{
    return reloq_language_rules_render(0x10000,text,length,tags,native_volume,
        settings,indices,index_count,pitches,pitch_count,session,sink,index_sink,context);
}
