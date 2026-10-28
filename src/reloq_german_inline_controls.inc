/* Private annotation controls. Offsets and SPR groups remain in one plan;
 * no AddText fragment is synthesized as a separate utterance. */
static int inline_words(unsigned char *text,size_t n,
    const struct reloq_german_generic_settings *settings,int volume,
    struct reloq_inline_word **out,size_t *count)
{
    size_t groups=0;
    for(size_t i=0;i+1<n;++i) if(text[i]=='`' && text[i+1]=='[')++groups;
    if(groups>=SIZE_MAX/sizeof(struct reloq_inline_word)-1)return -1;
    struct reloq_inline_word *words=calloc(groups+1,sizeof *words);
    if(!words)return -1;
    struct reloq_inline_word current={
        .pitch=settings && settings->native_pitch_plus_one?settings->native_pitch_plus_one-1:65,
        .speed=-1,.volume=volume,
        .inflection=settings && settings->native_inflection_plus_one?settings->native_inflection_plus_one-1:30,
        .pause=-1,.pitch_offset=SIZE_MAX,
        .gender=settings?settings->gender:0,
        .head_size=settings && settings->head_size_plus_one?settings->head_size_plus_one-1:50,
        .roughness=settings?settings->roughness:0,
        .breathiness=settings?settings->breathiness:0};
    size_t word=0;
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag tag=reloq_inline_tag_read(text+i,n-i,1);
        if(tag.kind==RELOQ_TAG_SPR) {
            current.annotation_offset=i;words[word++]=current;current.pause=-1;
            current.explicit_pause=0;
        } else if(tag.kind>=RELOQ_TAG_PITCH) {
            switch(tag.kind) {
            case RELOQ_TAG_PITCH:current.pitch=tag.value;current.pitch_offset=i;break;
            case RELOQ_TAG_SPEED:current.speed=tag.value;break;
            case RELOQ_TAG_VOLUME:current.volume=tag.value;break;
            case RELOQ_TAG_INFLECTION:current.inflection=tag.value;break;
            case RELOQ_TAG_GENDER:current.gender=tag.value;break;
            case RELOQ_TAG_HEAD_SIZE:current.head_size=tag.value;break;
            case RELOQ_TAG_ROUGHNESS:current.roughness=tag.value;break;
            case RELOQ_TAG_BREATHINESS:current.breathiness=tag.value;break;
            case RELOQ_TAG_PRESET:
                reloq_inline_preset(&current,tag.value);current.pitch_offset=i;
                break;
            case RELOQ_TAG_PAUSE:
                if(tag.value) {
                    int duration=tag.value<3?3:tag.value;
                    if(current.explicit_pause)duration+=30;
                    if(current.pause<0)current.pause=0;
                    if(current.pause>60000-duration){free(words);return -1;}
                    current.pause+=duration;
                    current.explicit_pause=1;
                }
                break;
            default:break;
            }
            if(word && (tag.kind==RELOQ_TAG_PITCH || tag.kind==RELOQ_TAG_INFLECTION ||
               tag.kind==RELOQ_TAG_HEAD_SIZE || tag.kind==RELOQ_TAG_PRESET) &&
                current.pause<0)current.pause=50;
            memset(text+i,' ',tag.length);
        }
        i+=tag.length?tag.length:1;
    }
    if(word!=groups){free(words);return -1;}
    current.annotation_offset=n;words[word]=current;
    *out=words;*count=word;return 0;
}

static const unsigned inline_model_defaults[5]={46,47,48,11,34};
struct inline_models {
    const struct reloq_inline_word *words;
    const int64_t *starts;
    size_t count,word;
    int64_t tick;
    uint32_t step;
    struct reloq_german_voice_model model;
    float neutral[5];
    unsigned char fixed[5];
    struct reloq_session *session;
    int error;
};
static void inline_model_frame(void *context,float c[62])
{
    struct inline_models *m=context;
    while(m->word+1<m->count && m->starts[m->word+1]<=m->tick)++m->word;
    m->model=inline_model(m->words+m->word,m->model.sample_rate);
    const int three=reloq_german_voice_model_formants(&m->model)==3;
    const int child=m->model.gender && m->model.head_size<30;
    for(unsigned i=0;i<5;++i)if(m->fixed[i])c[inline_model_defaults[i]]=m->neutral[i];
    if(child || three) {
        if(m->fixed[0])c[46]=1000;
        if(m->fixed[1])c[47]=1000;
        if(m->model.sample_rate<=11025 && m->fixed[2])c[48]=0;
    }
    if(three && !child) {
        if(m->fixed[3])c[11]=100;
        if(m->fixed[4])c[34]=15;
    }
    reloq_german_voice_model_frame(&m->model,c);
    long source=m->model.gender?(m->model.head_size<30?5:3):4;
    if(m->model.sample_rate==8000 && m->model.head_size<=(m->model.gender?40:30) && !child)source=7;
    unsigned reference=m->model.sample_rate>=16000?
        (m->model.head_size<=(m->model.gender?40:30)?4u:5u):0;
    m->error=reloq_session_source_levels(m->session,source,
        m->model.gender?-3:-5,6,m->model.gender?-7:0,reference);
    if(m->tick<=INT64_MAX-m->step)m->tick+=m->step;
}
static long inline_model_formants(void *context)
{
    struct inline_models *m=context;
    return m->error?0:(long)reloq_german_voice_model_formants(&m->model);
}

static int inline_pitch(const struct reloq_inline_word *words,size_t count,
    const struct reloq_german_generic_pitch_mark *marks,size_t mark_count,int *pitch)
{
    size_t next=0,latest=0;int have=0;
    for(size_t i=0;i<count;++i) {
        while(next<mark_count && marks[next].annotation_offset<=words[i].annotation_offset) {
            latest=marks[next++].annotation_offset;have=1;
        }
        if(words[i].pitch_offset!=SIZE_MAX && (!have || words[i].pitch_offset>=latest))
            pitch[i]=words[i].pitch;
    }
    return 0;
}

static int inline_time_value(int64_t tick,const int64_t *starts,const int64_t *mapped,
    const double *scale,size_t count,int64_t *out)
{
    size_t lo=0,hi=count;
    while(lo<hi) {size_t mid=lo+(hi-lo)/2;if(starts[mid]<=tick)lo=mid+1;else hi=mid;}
    size_t i=lo?lo-1:0;
    double value=(double)mapped[i]+(tick-starts[i])*scale[i];
    if(!isfinite(value)||value<0 ||value>(double)INT64_MAX-2048)return -1;
    *out=(int64_t)llround(value);return 0;
}

static int inline_timing_begin(struct inline_timing *m,
    const int64_t *word_ticks,size_t count,const struct reloq_inline_word *words,
    double baseline)
{
    if(!count || count>SIZE_MAX/(2*sizeof(int64_t)) || count>SIZE_MAX/sizeof(double))return -1;
    m->starts=malloc(2*count*sizeof *m->starts);m->scales=malloc(count*sizeof *m->scales);
    if(!m->starts || !m->scales)return -1;
    m->mapped=m->starts+count;m->count=count;
    memcpy(m->starts,word_ticks,count*sizeof *m->starts);
    m->starts[0]=0;m->mapped[0]=0;
    for(size_t i=0;i<count;++i) {
        m->scales[i]=words[i].speed<0?baseline:reloq_voice_speed_time_scale(words[i].speed);
        if(!isfinite(m->scales[i])||m->scales[i]<=0)return -1;
        if(i && inline_time_value(m->starts[i],m->starts,m->mapped,m->scales,i,m->mapped+i))return -1;
    }
    return 0;
}

struct inline_audio {
    reloq_session_pcm_sink pcm;
    reloq_german_generic_index_sink index;
    void *context;
    const struct reloq_inline_word *words;
    const int64_t *ticks;
    size_t count,word,samples;
    int per_tick;
};
static int inline_leading_quiet(struct reloq_target_builder *builder,int64_t begin)
{
    if(begin<=0)return 0;
    for(size_t s=RELOQ_TARGET_AV;s<=RELOQ_TARGET_AB;++s) {
        struct reloq_target_buffer *b=builder->stream+s;
        size_t skip=0;while(skip<b->count && b->points[skip].tick<begin)++skip;
        size_t n=b->count-skip;
        if(n>b->capacity || b->capacity-n<2)return -1;
        memmove(b->points+2,b->points+skip,n*sizeof *b->points);
        b->points[0]=(struct reloq_acoustic_point){0,0};
        b->points[1]=(struct reloq_acoustic_point){begin-1,0};b->count=n+2;
    }
    return 0;
}
static int inline_pcm(void *context,const int16_t *pcm,size_t count)
{
    struct inline_audio *a=context;int16_t buffer[1024];size_t at=0;
    while(at<count) {
        size_t n=count-at;if(n>1024)n=1024;
        double gain=reloq_voice_volume_gain(a->words[a->word].volume);
        for(size_t i=0;i<n;++i) {
            size_t sample=a->samples+i;
            while(a->word+1<a->count &&
                (uint64_t)a->ticks[a->word+1]*(unsigned)a->per_tick<=sample) {
                ++a->word;gain=reloq_voice_volume_gain(a->words[a->word].volume);
            }
            long value=lround(pcm[at+i]*gain);
            buffer[i]=(int16_t)(value>32767?32767:value< -32768?-32768:value);
        }
        if(a->pcm(a->context,buffer,n))return -1;
        a->samples+=n;at+=n;
    }
    return 0;
}
static int inline_index(void *context,size_t index)
{
    struct inline_audio *a=context;return a->index?a->index(a->context,index):-1;
}
