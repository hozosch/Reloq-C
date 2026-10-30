/* Inline ECI language selection belongs to the C transaction. Queue pieces
 * are joined before this pass; only an actual language command starts a
 * new native language plan. Indices, pitch marks and cancellation retain
 * their source positions. No text result survives this request. */
struct language_run {size_t begin,end;int language;};
struct language_controls {struct reloq_inline_word voice;unsigned mask;int spelling,emphasis,dictionary,prediction;};
static struct reloq_inline_tag source_tag(const unsigned char *text,size_t n,
    const unsigned char *modes,size_t at)
{
    if((modes&&modes[at]) || text[at]!='`')return (struct reloq_inline_tag){0};
    size_t preceding=at;
    while(preceding&&text[preceding-1]=='\\')--preceding;
    if((at-preceding)&1)return (struct reloq_inline_tag){0};
    struct reloq_inline_tag tag=reloq_inline_tag_read(text+at,n-at,0);
    if(modes)for(size_t i=at+1;i<at+tag.length;++i)
        if(modes[i])return (struct reloq_inline_tag){0};
    return tag;
}
static int inline_language_commands(const unsigned char *text,size_t n,
    const unsigned char *modes)
{
    if(!memchr(text,'`',n))return 0;
    for(size_t at=0;at<n;) {
        struct reloq_inline_tag tag=source_tag(text,n,modes,at);
        if(tag.kind==RELOQ_TAG_LANGUAGE)return 1;
        at+=tag.length?tag.length:1;
    }
    return 0;
}
static void language_control(struct language_controls *c,struct reloq_inline_tag tag)
{
    struct reloq_inline_word *v=&c->voice;
    switch(tag.kind) {
    case RELOQ_TAG_DICTIONARY:c->dictionary=tag.value+1;break;
    case RELOQ_TAG_PREDICTION:c->prediction=tag.value+1;break;
    case RELOQ_TAG_PRESET:reloq_inline_preset(v,tag.value);c->mask=255;break;
    case RELOQ_TAG_GENDER:v->gender=tag.value;c->mask|=1;break;
    case RELOQ_TAG_HEAD_SIZE:v->head_size=tag.value;c->mask|=2;break;
    case RELOQ_TAG_PITCH:v->pitch=tag.value;c->mask|=4;break;
    case RELOQ_TAG_INFLECTION:v->inflection=tag.value;c->mask|=8;break;
    case RELOQ_TAG_ROUGHNESS:v->roughness=tag.value;c->mask|=16;break;
    case RELOQ_TAG_BREATHINESS:v->breathiness=tag.value;c->mask|=32;break;
    case RELOQ_TAG_SPEED:v->speed=tag.value;c->mask|=64;break;
    case RELOQ_TAG_VOLUME:v->volume=tag.value;c->mask|=128;break;
    case RELOQ_TAG_SPELL:c->spelling=tag.value;break;
    case RELOQ_TAG_PROMINENCE:
        if(tag.value<=4)c->emphasis=tag.length==3?1:tag.value+2;
        break;
    case RELOQ_TAG_SPR:c->emphasis=0;break;
    default:break;
    }
}
static size_t language_prefix(const struct language_controls *c,char prefix[192])
{
    const struct reloq_inline_word *v=&c->voice;
    const int values[]={v->gender,v->head_size,v->pitch,v->inflection,
        v->roughness,v->breathiness,v->speed,v->volume};
    static const char letters[]="ghbfrysv";size_t n=0;
    for(unsigned i=0;i<8;++i)if(c->mask&(1u<<i))
        n+=(size_t)snprintf(prefix+n,192-n,"`v%c%d ",letters[i],values[i]);
    if(c->spelling)n+=(size_t)snprintf(prefix+n,192-n,"`ts%d ",c->spelling);
    if(c->emphasis) {
        static const char *const emphasis[]={"","`00 ","`0 ","`1 ","`2 ","`3 ","`4 "};
        n+=(size_t)snprintf(prefix+n,192-n,"%s",emphasis[c->emphasis]);
    }
    if(c->dictionary)n+=(size_t)snprintf(prefix+n,192-n,"`da%d ",c->dictionary-1);
    if(c->prediction)n+=(size_t)snprintf(prefix+n,192-n,"`pp%d ",c->prediction-1);
    prefix[n]=0;return n;
}
static void language_advance(struct language_controls *controls,
    const unsigned char *text,size_t length,const unsigned char *modes,
    size_t begin,size_t end,const struct pitch_mark *pitches,size_t first,size_t last)
{
    for(size_t at=begin;at<end;) {
        while(first<last&&pitches[first].offset<=at)
            language_control(controls,(struct reloq_inline_tag){RELOQ_TAG_PITCH,0,pitches[first++].value});
        struct reloq_inline_tag t=source_tag(text,length,modes,at);
        language_control(controls,t);
        if(!t.kind && (text[at]>=0x80 || (text[at]>='0'&&text[at]<='9') ||
           (text[at]>='a'&&text[at]<='z') || (text[at]>='A'&&text[at]<='Z')))
            controls->emphasis=0;
        at+=t.length?t.length:1;
    }
    while(first<last)
        language_control(controls,(struct reloq_inline_tag){RELOQ_TAG_PITCH,0,pitches[first++].value});
}
static int synthesize_languages(struct reloq_eci_engine *e,
    unsigned char *text,size_t length,const unsigned char *modes,int unicode,
    const struct index_mark *marks,size_t mark_count,
    const struct pitch_mark *pitches,size_t pitch_count,unsigned epoch)
{
    size_t count=1;
    for(size_t at=0;at<length;) {
        struct reloq_inline_tag t=source_tag(text,length,modes,at);
        if(t.kind==RELOQ_TAG_LANGUAGE)++count;
        at+=t.length?t.length:1;
    }
    if(count>SIZE_MAX/sizeof(struct language_run))return RELOQ_ECI_MEMORY;
    struct language_run *runs=malloc(count*sizeof *runs);
    struct index_mark *local_marks=mark_count?malloc(mark_count*sizeof *local_marks):NULL;
    struct pitch_mark *local_pitch=pitch_count?malloc(pitch_count*sizeof *local_pitch):NULL;
    if(!runs||(mark_count&&!local_marks)||(pitch_count&&!local_pitch)) {
        free(runs);free(local_marks);free(local_pitch);return RELOQ_ECI_MEMORY;
    }
    int language=e->language,configured_language=e->language,result=RELOQ_ECI_OK;
    void *configured_worker=e->rule_worker;
    void *workers[15]={0};int languages[15]={0};size_t worker_count=0;
    runs[0]=(struct language_run){0,length,language};count=1;
    for(size_t at=0;at<length;) {
        struct reloq_inline_tag t=source_tag(text,length,modes,at);
        if(t.kind==RELOQ_TAG_LANGUAGE) {
            if(reloq61_language_supported(t.value)&&t.value!=language) {
                runs[count-1].end=at;language=t.value;
                runs[count++]=(struct language_run){at,length,language};
            }
            memset(text+at,' ',t.length);
        }
        at+=t.length?t.length:1;
    }
    struct language_controls controls={0};size_t marker=0,pitch_marker=0;
    lock_session(&e->session_lock);
    int reset=reloq_session_reset(e->text_session);
    if(epoch_get(&e->stop_epoch)!=epoch)reloq_session_cancel(e->text_session);
    unlock_session(&e->session_lock);
    if(reset){result=RELOQ_ECI_RENDER;goto done;}
    for(size_t run=0;run<count;++run) {
        const struct language_run *r=runs+run;
        if(epoch_get(&e->stop_epoch)!=epoch){result=RELOQ_ECI_CANCELLED;break;}
        size_t n=r->end-r->begin,active=0;
        for(size_t at=r->begin;at<r->end;) {
            struct reloq_inline_tag t=source_tag(text,length,modes,at);
            if(t.kind&&t.kind!=RELOQ_TAG_SPR){at+=t.length;continue;}
            if(text[at]!=' '&&text[at]!='\t'&&text[at]!='\r'&&text[at]!='\n'){active=1;break;}
            ++at;
        }
        if(!active) {
            while(marker<mark_count && (marks[marker].offset<r->end||run+1==count)) {
                result=send_index(e,marks[marker++].value);if(result)break;
            }
            if(result)break;
            size_t first=pitch_marker;
            while(pitch_marker<pitch_count && (pitches[pitch_marker].offset<r->end||run+1==count))++pitch_marker;
            language_advance(&controls,text,length,modes,r->begin,r->end,pitches,first,pitch_marker);
            continue;
        }
        unsigned char *normalized=NULL;size_t bytes=n,*map=NULL;
        if(unicode) {
            int status=reloq_unicode_admit_language(text+r->begin,n,modes?modes+r->begin:NULL,
                r->language,e->process_tags,&normalized,&bytes,&map);
            if(status){result=status==-2?RELOQ_ECI_MEMORY:RELOQ_ECI_INVALID;
                e->failure_stage=RELOQ_ECI_STAGE_TEXT;break;}
        }
        char prefix[192];size_t prefix_n=language_prefix(&controls,prefix);
        if(bytes>SIZE_MAX-prefix_n-2){free(normalized);free(map);result=RELOQ_ECI_MEMORY;break;}
        unsigned char *phrase=malloc(prefix_n+bytes+2);
        if(!phrase){free(normalized);free(map);result=RELOQ_ECI_MEMORY;break;}
        memcpy(phrase,prefix,prefix_n);
        memcpy(phrase+prefix_n,normalized?normalized:text+r->begin,bytes);
        size_t source_length=prefix_n+bytes;phrase[source_length]=0;
        size_t mc=0,pc=0;
        while(marker<mark_count && (marks[marker].offset<r->end||run+1==count)) {
            size_t at=marks[marker].offset-r->begin;
            local_marks[mc++]=(struct index_mark){prefix_n+(map?map[at]:at),marks[marker++].value};
        }
        size_t pitch_begin=pitch_marker;
        while(pitch_marker<pitch_count && (pitches[pitch_marker].offset<r->end||run+1==count)) {
            size_t at=pitches[pitch_marker].offset-r->begin;
            local_pitch[pc++]=(struct pitch_mark){prefix_n+(map?map[at]:at),pitches[pitch_marker++].value};
        }
        if(run+1==count && ends_german_word(phrase,source_length)) {
            for(size_t i=0;i<mc;++i)if(local_marks[i].offset==source_length)++local_marks[i].offset;
            phrase[source_length++]='.';phrase[source_length]=0;
        }
        e->language=r->language;
        if(e->language==configured_language) {
            if(!configured_worker)configured_worker=reloq61_worker_new(e->language);
            e->rule_worker=configured_worker;
        } else {
            size_t i=0;while(i<worker_count&&languages[i]!=e->language)++i;
            if(i==worker_count) {
                if(worker_count==15){result=RELOQ_ECI_MEMORY;goto run_done;}
                languages[i]=e->language;workers[i]=reloq61_worker_new(e->language);++worker_count;
                if(workers[i]&&reloq61_worker_dictionary(workers[i],e->dictionary_path)) {result=RELOQ_ECI_INVALID;goto run_done;}
            }
            e->rule_worker=workers[i];
        }
        if(!e->rule_worker){result=RELOQ_ECI_MEMORY;goto run_done;}
        {
            void *previous=reloq61_worker_scope(e->rule_worker);
            result=synthesize_generic(e,phrase,source_length,prefix_n+bytes,
                local_marks,mc,local_pitch,pc,epoch,unicode);
            reloq61_worker_scope(previous);
        }
        /* Controls before the next language command keep their normal ECI
           lifetime. The configured voice and selected NVDA language do not
           change when this transaction ends. */
        language_advance(&controls,text,length,modes,r->begin,r->end,pitches,pitch_begin,pitch_marker);
run_done:
        if(result&&e->failure_utf8_offset>=0) {
            size_t offset=(size_t)e->failure_utf8_offset,at=0;
            if(offset>prefix_n) {
                offset-=prefix_n;
                if(map)while(at<n&&map[at+1]<=offset)++at;
                else at=offset<n?offset:n;
            }
            e->failure_utf8_offset=diagnostic_offset(r->begin+at,length);
        }
        free(phrase);free(normalized);free(map);
        if(result)break;
    }
done:
    e->language=configured_language;e->rule_worker=configured_worker;
    for(size_t i=0;i<worker_count;++i)reloq61_worker_free(workers[i]);
    free(runs);free(local_marks);free(local_pitch);return result;
}
