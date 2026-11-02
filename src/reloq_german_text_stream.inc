/* Schedule completed admission phrases without changing their lexical or
 * punctuation rules. Main replacements have already acted once; nested
 * plans inherit that text scope and the same initialized dictionary. */
static int plan_tags_checked(int,const unsigned char *,size_t,
    const struct reloq_lexicon *,int,unsigned char **,size_t *,size_t *,size_t *,
    struct reloq_german_best_effort_boundary *,size_t,
    struct reloq_german_word_features **,size_t *,int (*)(void *),void *,
    const struct reloq_german_text_stream *,int,size_t *);

/* Voice controls do not change lexical admission. Their running values
 * travel with the acoustic continuation; spelling/language/explicit SPR
 * and dictionary-policy switches retain the complete transaction path. */
static int stream_voice_controls(const unsigned char *text,size_t n)
{
    for(size_t i=0;i<n;) {
        if(text[i]!='`'){++i;continue;}
        struct reloq_inline_tag t=reloq_inline_tag_read(text+i,n-i,0);
        if(!t.length){++i;continue;}
        if(t.kind<RELOQ_TAG_PITCH||t.kind>RELOQ_TAG_PRESET||t.kind==RELOQ_TAG_PAUSE)return 0;
        i+=t.length;
    }
    return 1;
}

static size_t stream_source_offset(const struct reloq_german_context_source *s,
    const struct reloq_german_prosody_unit *unit)
{
    const unsigned char *buffers[]={s->encoded,s->original,s->words};
    uintptr_t at=(uintptr_t)unit->text;
    for(unsigned i=0;i<3;++i) {
        uintptr_t base=(uintptr_t)buffers[i];
        if(base && at>=base && at-base<=s->length && unit->length<=s->length-(at-base))
            return (size_t)(at-base);
    }
    return SIZE_MAX;
}
/* Prosody also classifies a semicolon as PERIOD. It is not a sentence
 * boundary: its following clause still shares the analysis/contour.
 * Confirm a real stop in the admitted source before creating a new plan. */
static int stream_sentence_stop(const uint32_t *scalars,size_t begin,size_t end)
{
    if(begin==SIZE_MAX||begin>=end)return 0;
    int stop=0;
    for(size_t i=begin;i<end;++i) {
        if(scalars[i]=='.'||scalars[i]=='?'||scalars[i]=='!')stop=1;
        else if(scalars[i]==';'||scalars[i]==':')stop=0;
    }
    return stop;
}
/* Ask the admitted lexical/prosodic plan, rather than a punctuation regex.
 * Abbreviations, decimals and open quotations cannot become new chunks. */
static size_t stream_first_cut(struct annotation_builder *b,
    const struct reloq_german_context_source *source,const uint32_t *scalars)
{
    if(b->word_count!=b->unit_count||b->unit_count<2)return SIZE_MAX;
    enum reloq_german_terminal_mark *marks=malloc(b->unit_count*sizeof *marks);
    if(!marks)return SIZE_MAX;
    size_t cut=SIZE_MAX,scanned=0;unsigned depth=0,quote=0;
    if(reloq_german_generic_prosody_marks(b->output,b->used,b->unit_count,marks,b->unit_count))goto done;
    for(size_t i=0;i+1<b->unit_count;++i) {
        if(marks[i]!=RELOQ_DE_TERMINAL_PERIOD&&marks[i]!=RELOQ_DE_TERMINAL_QUESTION&&
           marks[i]!=RELOQ_DE_TERMINAL_EXCLAMATION)continue;
        if(b->units[i].fixed_phrase||b->units[i+1].fixed_phrase)continue;
        size_t end=stream_source_offset(source,b->units+i+1);
        if(!end||end==SIZE_MAX||end>=source->length)continue;
        if(!stream_sentence_stop(scalars,stream_source_offset(source,b->units+i),end))continue;
        for(;scanned<end;++scanned) {
            uint32_t c=scalars[scanned];
            if(c=='('||c=='['||c=='{')++depth;
            else if((c==')'||c==']'||c=='}')&&depth)--depth;
            else if(c=='"')quote=quote?0:'"';
            else if(c==0x201e)quote=0x201e;
            else if(c==0x201c)quote=quote==0x201e?0:0x201c;
            else if(c==0x201d)quote=0;
            else if(c==0xab||c==0xbb)quote=quote?0:c;
        }
        if(!depth&&!quote){cut=end;break;}
    }
done:free(marks);return cut;
}
static size_t stream_utf8(unsigned char *p,uint32_t c)
{
    if(c<0x80){p[0]=(unsigned char)c;return 1;}
    if(c<0x800){p[0]=(unsigned char)(0xc0|(c>>6));p[1]=(unsigned char)(0x80|(c&63));return 2;}
    if(c<0x10000){p[0]=(unsigned char)(0xe0|(c>>12));p[1]=(unsigned char)(0x80|((c>>6)&63));p[2]=(unsigned char)(0x80|(c&63));return 3;}
    p[0]=(unsigned char)(0xf0|(c>>18));p[1]=(unsigned char)(0x80|((c>>12)&63));
    p[2]=(unsigned char)(0x80|((c>>6)&63));p[3]=(unsigned char)(0x80|(c&63));return 4;
}
/* The existing melody gives a wholly weak trailing clause utterance-wide
 * lookahead. Keep that dependency instead of treating a scheduled part as
 * a new utterance. Most parts settle with one following phrase window. */
static int stream_needs_tail_context(const unsigned char *text,size_t length)
{
    struct reloq_german_annotation_cursor cursor={0};
    struct reloq_german_annotation_event event;int tail=0,weak=0,state;
    while((state=reloq_german_annotation_next(text,length,&cursor,0,&event))==1) {
        if(event.kind==RELOQ_DE_ANNOTATION_CONTROL) {
            if(event.number==10)return 1;
            weak=event.number==0;
        } else if(event.kind==RELOQ_DE_ANNOTATION_SPR && event.spr.kind==RELOQ_DE_SPR_WORD_OPEN) {
            if(!weak)tail=0;
            weak=0;
        } else if(event.kind==RELOQ_DE_ANNOTATION_TEXT) {
            for(size_t i=event.offset;i<event.offset+event.length;++i) {
                if(text[i]==',')tail=1;
                else if(strchr("?!:;()",text[i]))tail=0;
            }
        }
    }
    return state<0 || tail;
}
static int stream_plan(struct annotation_builder *b,
    const struct reloq_german_context_source *source,const uint32_t *scalars,
    int process_tags,struct reloq_german_best_effort_boundary *boundaries,size_t boundary_count,
    size_t *error,const struct reloq_german_text_stream *stream)
{
    if(!stream || !stream->part || source->length<1024 ||
       b->word_count!=b->unit_count || !b->unit_count ||
       !stream_voice_controls(source->original,source->length))return 0;
    if(b->unit_count>SIZE_MAX/sizeof(enum reloq_german_terminal_mark) ||
       b->unit_count>SIZE_MAX/sizeof(size_t)-1)return -2;
    enum reloq_german_terminal_mark *marks=malloc(b->unit_count*sizeof *marks);
    size_t *cuts=malloc((b->unit_count+1)*sizeof *cuts);
    if(!marks||!cuts){free(marks);free(cuts);return -2;}
    int result=0;
    if(reloq_german_generic_prosody_marks(b->output,b->used,b->unit_count,
        marks,b->unit_count))goto done;
    /* Do not transport an open quotation or bracket's syntactic state to a
     * new native request. This only declines a scheduling opportunity. */
    size_t scanned=0,count=0,first_word=0;unsigned depth=0,quote=0;
    for(size_t i=0;i+1<b->unit_count;++i) {
        if(b->cancelled&&b->cancelled(b->cancel_context)){result=2;goto done;}
        if(marks[i]!=RELOQ_DE_TERMINAL_PERIOD && marks[i]!=RELOQ_DE_TERMINAL_QUESTION &&
           marks[i]!=RELOQ_DE_TERMINAL_EXCLAMATION)continue;
        if(b->units[i].fixed_phrase || b->units[i+1].fixed_phrase)continue;
        size_t cut=stream_source_offset(source,b->units+i+1);
        if(cut==SIZE_MAX || !cut || cut>=source->length || (count && cut<=cuts[count-1]))continue;
        if(!stream_sentence_stop(scalars,stream_source_offset(source,b->units+i),cut))continue;
        for(;scanned<cut;++scanned) {
            uint32_t cp=scalars[scanned];
            if(cp=='('||cp=='['||cp=='{')++depth;
            else if((cp==')'||cp==']'||cp=='}')&&depth)--depth;
            else if(cp=='"')quote=quote?0:'"';
            else if(cp==0x201e)quote=0x201e;
            else if(cp==0x201c)quote=quote==0x201e?0:0x201c;
            else if(cp==0x201d)quote=0;
            else if(cp==0xab||cp==0xbb)quote=quote?0:cp;
        }
        if(depth||quote || (count && i+1-first_word<64))continue;
        cuts[count++]=cut;first_word=i+1;
    }
    if(!count)goto done;
    cuts[count++]=source->length;
    size_t begin=0,next_boundary=0;
    for(size_t part=0;part<count;++part) {
        unsigned extended=0;
retry_part: ;
        size_t end=cuts[part],n=end-begin;
        if(n>SIZE_MAX/4-1){result=-2;goto done;}
        unsigned char *text=malloc(n*4+1),*annotation=NULL;
        struct reloq_german_word_features *features=NULL;
        struct reloq_german_best_effort_boundary *local=NULL;
        size_t bytes=0,length=0,features_count=0,guessed=0,bad=0;
        if(!text){result=-2;goto done;}
        for(size_t j=begin;j<end;++j)bytes+=stream_utf8(text+bytes,scalars[j]);
        text[bytes]=0;
        size_t first=next_boundary;
        while(next_boundary<boundary_count &&
            (boundaries[next_boundary].encoded_offset<end || part+1==count))++next_boundary;
        size_t index_count=next_boundary-first;
        if(index_count) {
            local=malloc(index_count*sizeof *local);
            if(!local){free(text);result=-2;goto done;}
            for(size_t j=0;j<index_count;++j) {
                if(boundaries[first+j].encoded_offset<begin){free(local);free(text);result=-1;goto done;}
                local[j]=(struct reloq_german_best_effort_boundary){boundaries[first+j].encoded_offset-begin,0};
            }
        }
        result=plan_tags_checked(1,text,bytes,b->lexicon,process_tags,
            &annotation,&length,&guessed,&bad,local,index_count,&features,&features_count,
            b->cancelled,b->cancel_context,NULL,1,NULL);
        free(text);
        if(!result && part+1<count && stream_needs_tail_context(annotation,length)) {
            free(annotation);free(features);free(local);next_boundary=first;
            if(extended++) {cuts[part]=source->length;count=part+1;}
            else {memmove(cuts+part,cuts+part+1,(count-part-1)*sizeof *cuts);--count;}
            goto retry_part;
        }
        if(result) {
            if(error)*error=begin+bad;
        } else if(stream->part(stream->context,annotation,length,features,features_count,
                    local,index_count,first,part+1==count))
            result=b->cancelled&&b->cancelled(b->cancel_context)?2:-1;
        free(annotation);free(features);free(local);
        if(result)goto done;
        begin=end;
    }
    result=3; /* All parts were rendered; the whole-text output is unused. */
done:free(marks);free(cuts);return result;
}

/* Geometric admission windows bound time to first audio for ordinary long
 * prose. Each delivered part still uses the complete existing sentence
 * planner, dictionary, prosody and continuation renderer. Unsupported
 * contexts retain the whole-text path; no arbitrary word count is a cut. */
static int stream_windows(const unsigned char *text,size_t length,
    const struct reloq_lexicon *lexicon,int tags,
    struct reloq_german_best_effort_boundary *boundaries,size_t boundary_count,
    size_t *error,int (*cancelled)(void *),void *context,
    const struct reloq_german_text_stream *stream)
{
    if(!stream||!stream->part||length<1024||!stream_voice_controls(text,length))return 0;
    size_t begin=0,scalar_begin=0,next_boundary=0;
    while(begin<length) {
        size_t window=96,bytes=length-begin,scalars=0;
        unsigned char *annotation=NULL;struct reloq_german_word_features *features=NULL;
        struct reloq_german_best_effort_boundary *local=NULL;
        size_t annotation_length=0,features_count=0,guessed=0,bad=0,cut=SIZE_MAX;
        int result=0;
        for(;;) {
            bytes=length-begin;
            if(bytes>window) {
                bytes=window;
                while(bytes&&text[begin+bytes]!=' '&&text[begin+bytes]!='\n'&&text[begin+bytes]!='\t')--bytes;
                if(bytes) {
                    size_t probe=SIZE_MAX;
                    result=plan_tags_checked(1,text+begin,bytes,lexicon,tags,
                        &annotation,&annotation_length,&guessed,&bad,NULL,0,&features,&features_count,
                        cancelled,context,NULL,0,&probe);
                    free(annotation);free(features);annotation=NULL;features=NULL;
                    if(result==4){cut=probe;break;}
                    if(result)return result;
                }
                if(window>SIZE_MAX/2)window=length-begin;else window*=2;
                continue;
            }
            break;
        }
        if(cut!=SIZE_MAX) {
            size_t at=0;scalars=0;
            while(scalars<cut&&at<bytes) {uint32_t c;if(reloq_utf8_next(text+begin,bytes,&at,&c)!=1)return -1;++scalars;}
            if(scalars!=cut||!at)return -1;
            bytes=at;
        } else {
            size_t at=0;scalars=0;
            while(at<bytes){uint32_t c;if(reloq_utf8_next(text+begin,bytes,&at,&c)!=1)return -1;++scalars;}
        }
retry_remaining: ;
        size_t first=next_boundary;
        while(next_boundary<boundary_count&&
            (boundaries[next_boundary].encoded_offset<scalar_begin+scalars||begin+bytes==length))++next_boundary;
        size_t count=next_boundary-first;
        if(count) {
            local=malloc(count*sizeof *local);if(!local)return -2;
            for(size_t i=0;i<count;++i) {
                if(boundaries[first+i].encoded_offset<scalar_begin){free(local);return -1;}
                local[i]=(struct reloq_german_best_effort_boundary){boundaries[first+i].encoded_offset-scalar_begin,0};
            }
        }
        result=plan_tags_checked(1,text+begin,bytes,lexicon,tags,
            &annotation,&annotation_length,&guessed,&bad,local,count,&features,&features_count,
            cancelled,context,NULL,0,NULL);
        if(!result&&begin+bytes<length&&stream_needs_tail_context(annotation,annotation_length)) {
            /* This rare dependency needs whole remaining context. Re-plan
             * that tail before delivering anything from the uncertain part. */
            free(annotation);free(features);free(local);next_boundary=first;
            annotation=NULL;features=NULL;local=NULL;
            bytes=length-begin;scalars=0;size_t at=0;
            while(at<bytes){uint32_t c;if(reloq_utf8_next(text+begin,bytes,&at,&c)!=1)return -1;++scalars;}
            goto retry_remaining;
        }
        if(result){if(error)*error=scalar_begin+bad;}
        else if(stream->part(stream->context,annotation,annotation_length,features,features_count,
                    local,count,first,begin+bytes==length))result=cancelled&&cancelled(context)?2:-1;
        free(annotation);free(features);free(local);
        if(result)return result;
        begin+=bytes;scalar_begin+=scalars;
    }
    return 3;
}
