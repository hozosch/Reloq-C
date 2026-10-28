/* Adapt the phonetic source planner to sparse acoustic targets. Existing
 * amplitude values remain intact; ownership determines their clock range. */
static int coordinate_source(struct events *e,const struct unit *u,size_t count,
    size_t planned_events)
{
    if(!count)return 0;
    if(count>SIZE_MAX/sizeof(struct reloq_english_source_phone) ||
       count>SIZE_MAX/sizeof(struct reloq_english_source_span))return -1;
    struct reloq_english_source_phone *p=calloc(count,sizeof *p);
    struct reloq_english_source_span *s=calloc(count,sizeof *s);
    if(!p||!s){free(p);free(s);return -1;}
    int result=-1;
    for(size_t i=0;i<count;++i)p[i]=(struct reloq_english_source_phone){
        .phone=u[i].phone,.stress=u[i].stress,.aspirated=u[i].aspirated,
        .word_start=!i||u[i].word!=u[i-1].word,
        .syllable_start=!i||u[i].syllable!=u[i-1].syllable,
        .pause=u[i].pause!=0,.before_h=u[i].before_h,
        .start=u[i].start,.contact=u[i].contact,.end=u[i].end,.release=u[i].release};
    if(reloq_english_source_plan(p,count,s))goto done;

    for(size_t i=0;i<count;++i)if(s[i].owner==i && s[i].nucleus && s[i].last>i) {
        const size_t limit=i+1<count?u[i+1].event_begin:planned_events;
        const unsigned streams[]={RELOQ_TARGET_AV,RELOQ_TARGET_AH};
        for(unsigned k=0;k<2;++k) {
            size_t selected=SIZE_MAX;
            for(size_t j=u[i].event_begin;j<limit;++j)
                if(e->p[j].stream==streams[k] && e->p[j].value>0 &&
                   e->p[j].tick>=u[i].end &&
                   (selected==SIZE_MAX||e->p[j].tick>e->p[selected].tick))selected=j;
            if(selected!=SIZE_MAX)e->p[selected].tick=s[i].end;
        }
    }

    for(size_t i=0;i<count;++i)if(s[i].owner==i && s[i].voiced_sonorant) {
        const size_t limit=i+1<count?u[i+1].event_begin:planned_events;
        int positive=0,zero=0;
        for(size_t j=u[i].event_begin;j<limit;++j)
            if(e->p[j].stream==RELOQ_TARGET_AV && e->p[j].tick<=u[i].end) {
                if(e->p[j].value>0)positive=1;
                /* A zero exactly at the end starts the release. */
                else if(e->p[j].tick<u[i].end)zero=1;
            }
        int next_silent=u[i].pause||i+1==count;
        if(i+1<count&&!u[i].pause) {
            const size_t last=i+2<count?u[i+2].event_begin:planned_events;
            int64_t first=INT64_MAX;
            for(size_t j=u[i+1].event_begin;j<last;++j)
                if(e->p[j].stream==RELOQ_TARGET_AV && e->p[j].tick<first) {
                    first=e->p[j].tick;next_silent=e->p[j].value==0;
                }
        }
        const int previous_sonorant=i&&!u[i-1].pause &&
            (vowel(u[i-1].phone)||strchr("lrwy",u[i-1].phone));
        if(!reloq_english_source_needs_plateau(u[i].phone,s+i,positive,zero,
                next_silent,previous_sonorant))continue;
        int level=0;
        const struct english_gesture *g=u[i].outgoing;
        for(unsigned j=0;j<g->count;++j) {
            const struct english_landmark *q=english_landmarks+g->first+j;
            if(q->stream==RELOQ_TARGET_AV && q->tick>=g->contact &&
               q->tick<=g->duration && q->value>level)level=q->value;
        }
        if(!level)level=u[i].phone=='y'?53:50;
        for(size_t j=u[i].event_begin;j<limit;++j)
            if(e->p[j].stream==RELOQ_TARGET_AV && !e->p[j].value &&
               e->p[j].tick<u[i].end) {
                e->p[j].tick=s[i].first;e->p[j].value=level;
            }
        if(add(e,RELOQ_TARGET_AV,s[i].first,level) ||
           add(e,RELOQ_TARGET_AV,s[i].end,level))goto done;
        e->p[e->n-1].serial=e->p[e->n-2].serial=u[i].event_begin;
    }

    for(size_t i=1;i<count;++i)if(s[i].owner==i && s[i].nucleus &&
        s[i].early && !u[i-1].pause) {
        const size_t limit=i+1<count?u[i+1].event_begin:planned_events;
        for(size_t j=u[i].event_begin;j<limit;++j) {
            struct event *q=e->p+j;
            if(q->stream!=RELOQ_TARGET_AV && q->stream!=RELOQ_TARGET_AH)continue;
            if(q->tick<=u[i].contact+1)q->tick=u[i].start+(q->value>0);
        }
        if(s[i].unaspirated_stop)
            for(size_t j=u[i-1].event_begin;j<u[i].event_begin;++j) {
                struct event *q=e->p+j;
                if((q->stream==RELOQ_TARGET_AV||q->stream==RELOQ_TARGET_AH) &&
                   q->tick>=u[i].start) {
                    q->tick=u[i].start;q->value=0;
                }
            }
    }
    /* A closing liquid or glide cannot turn off its voiced nucleus before
     * the nucleus's source endpoint. Positive member targets remain valid
     * closing levels, including the handoff before h. Preserve these, the
     * independently varying AH curve, and all targets at/after that endpoint.
     * Deferred points carry their member's original serial. */
    for(size_t i=0;i<count;++i)if(s[i].owner!=i) {
        size_t limit=i+1<count?u[i+1].event_begin:planned_events;
        for(size_t j=u[i].event_begin;j<limit;++j)
            if(e->p[j].stream==RELOQ_TARGET_AV && !e->p[j].value &&
               e->p[j].tick<s[i].end)
                e->p[j].stream=RELOQ_TARGET_STREAM_COUNT;
        for(size_t j=planned_events;j<e->n;++j)
            if(e->p[j].serial==u[i].event_begin &&
               e->p[j].stream==RELOQ_TARGET_AV && !e->p[j].value &&
               e->p[j].tick<s[i].end)
                e->p[j].stream=RELOQ_TARGET_STREAM_COUNT;
    }
    /* Event ranges are no longer consulted after source coordination. */
    size_t keep=0;
    for(size_t j=0;j<e->n;++j)
        if(e->p[j].stream<RELOQ_TARGET_STREAM_COUNT)e->p[keep++]=e->p[j];
    e->n=keep;
    result=0;
done:
    free(p);free(s);return result;
}
