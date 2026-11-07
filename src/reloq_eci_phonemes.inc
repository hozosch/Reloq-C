/* Phoneme generation changes the output route for one synchronous call.
 * The PCM buffer/callback remain registered and usable afterward. */
struct phoneme_delivery {
    struct reloq_eci_engine *engine;
    unsigned char *buffer;
    size_t capacity,used;
    const struct reloq61_mark *marks;
    size_t mark_count;
    unsigned epoch;
    int status;
};
static int phoneme_cancelled(void *context)
{
    struct phoneme_delivery *p=context;
    return p->status!=RELOQ_ECI_OK||epoch_get(&p->engine->stop_epoch)!=p->epoch;
}
static int flush_phonemes(struct phoneme_delivery *p)
{
    if(phoneme_cancelled(p))return 1;
    if(p->used) {
        p->status=notify_callback(p->engine,eciPhonemeBuffer,(int)p->used);
        if(p->status!=RELOQ_ECI_OK)return 1;
        p->used=0;
    }
    return 0;
}
static int deliver_phonemes(void *context,const char *text,size_t length)
{
    struct phoneme_delivery *p=context;
    while(length) {
        if(phoneme_cancelled(p))return 1;
        size_t n=p->capacity-p->used;if(n>length)n=length;
        memcpy(p->buffer+p->used,text,n);p->used+=n;text+=n;length-=n;
        if(p->used==p->capacity&&flush_phonemes(p))return 1;
    }
    return 0;
}
static void phoneme_index(void *context,int32_t index,int64_t at)
{
    struct phoneme_delivery *p=context;(void)at;
    if(index<0||(size_t)index>=p->mark_count||flush_phonemes(p))return;
    p->status=notify_callback(p->engine,eciIndexReply,p->marks[index].index);
}
int ECICALL eciGeneratePhonemes(ECIHand handle,int room,void *buffer)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy))return eci_refuse(e,0);
    /* The reference refuses these two setup states without recording a
     * parameter error or consuming accepted input. */
    if(!e->callback||e->synth_mode!=1)return 0;
    if(room<1||!buffer)return eci_refuse(e,0);
    unsigned before=epoch_get(&e->stop_epoch);
    if(!busy_begin(&e->busy))return eci_refuse(e,0);
    int status=RELOQ_ECI_OK;unsigned char *text=NULL,*modes=NULL;
    struct reloq61_mark *marks=NULL;size_t *map=NULL,length=0,count=0;
    struct phoneme_delivery output={.engine=e,.buffer=buffer,.capacity=(size_t)room,
        .epoch=before,.status=RELOQ_ECI_OK};
    for(size_t i=0;i<e->queued;++i) {
        if(e->queue[i].is_index)++count;
        else if(e->queue[i].is_pitch){status=RELOQ_ECI_INVALID;goto done;}
        else if(e->queue[i].length>SIZE_MAX-length-1){status=RELOQ_ECI_MEMORY;goto done;}
        else length+=e->queue[i].length;
    }
    if(count>INT32_MAX||count>SIZE_MAX/sizeof *marks){status=RELOQ_ECI_MEMORY;goto done;}
    text=malloc(length+1);modes=malloc(length?length:1);
    if(count)marks=malloc(count*sizeof *marks);
    if(!text||!modes||(count&&!marks)){status=RELOQ_ECI_MEMORY;goto done;}
    size_t at=0,mark=0;
    for(size_t i=0;i<e->queued;++i) {
        struct queued_item *q=e->queue+i;
        if(q->is_index)marks[mark++]=(struct reloq61_mark){at,q->index,-1};
        else if(q->length){memcpy(text+at,q->utf8,q->length);memset(modes+at,q->character_mode,q->length);at+=q->length;}
    }
    text[length]=0;
    unsigned char *admitted=NULL;size_t admitted_length=0;
    int admission=reloq_unicode_admit_language(text,length,modes,e->language,
        e->process_tags,&admitted,&admitted_length,&map);
    if(admission){status=admission==-2?RELOQ_ECI_MEMORY:RELOQ_ECI_INVALID;goto done;}
    for(size_t i=0;i<count;++i)marks[i].offset=map[marks[i].offset];
    free(text);text=admitted;length=admitted_length;
    output.marks=marks;output.mark_count=count;
    /* Provider indices are nonnegative ordinals; user indices retain all
     * 32 payload bits and are translated only at publication. */
    struct reloq61_mark *ordinals=count?malloc(count*sizeof *ordinals):NULL;
    if(count&&!ordinals){status=RELOQ_ECI_MEMORY;goto done;}
    for(size_t i=0;i<count;++i){ordinals[i]=marks[i];ordinals[i].index=(int32_t)i;}
    if(!e->rule_worker)e->rule_worker=reloq61_worker_new(e->language);
    if(!e->rule_worker){free(ordinals);status=RELOQ_ECI_MEMORY;goto done;}
    if(e->language==RELOQ_ECI_GERMAN&&reloq61_worker_lexicon_overlay(e->rule_worker,
        e->lexicon.rows?&e->lexicon.view:NULL,e->lexicon_version)) {
        free(ordinals);status=RELOQ_ECI_MEMORY;goto done;
    }
    void *worker=reloq61_worker_scope(e->rule_worker);
    struct reloq61_text_options options=reloq61_text_options_scope(
        (struct reloq61_text_options){e->dictionary_disabled,e->phrase_prediction,e->original_questions});
    struct reloq61_cancel_check cancel=reloq61_cancel_scope(
        (struct reloq61_cancel_check){phoneme_cancelled,&output});
    int voice[8]={e->gender,e->head_size,e->native_pitch,e->native_inflection,
        e->roughness,e->breathiness,e->native_speed,e->native_volume};
    int generated=reloq_rules61_phonemes(e->language,text,length,e->process_tags,
        voice,ordinals,count,deliver_phonemes,phoneme_index,&output);
    reloq61_cancel_scope(cancel);reloq61_text_options_scope(options);reloq61_worker_scope(worker);
    free(ordinals);
    if(phoneme_cancelled(&output))status=output.status?output.status:RELOQ_ECI_CANCELLED;
    else if(generated)status=RELOQ_ECI_RENDER;
    else if(flush_phonemes(&output))status=output.status?output.status:RELOQ_ECI_CANCELLED;
done:
    free(text);free(modes);free(map);free(marks);clear_queue(e);
    eci_error(e,status);e->last_synthesis_error=status;busy_end(&e->busy);
    return status==RELOQ_ECI_OK;
}
