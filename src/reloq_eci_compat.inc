/* Independently expressed public ECI controls. The renderer and the existing
 * vendor entry points share these per-instance settings and dictionary data. */
static const int voice_parameters[]={RELOQ_ECI_GENDER,RELOQ_ECI_HEAD_SIZE,
    RELOQ_ECI_NATIVE_PITCH,RELOQ_ECI_NATIVE_INFLECTION,RELOQ_ECI_ROUGHNESS,
    RELOQ_ECI_BREATHINESS,RELOQ_ECI_NATIVE_SPEED,RELOQ_ECI_NATIVE_VOLUME};
static reloq_eci_atomic eci_defaults[10]={1,0,0,0,0,1,0,0,0,RELOQ_ECI_ENGLISH};
static int default_parameter(int parameter)
{
    return parameter==0||parameter==1||parameter==2||parameter==3||
        parameter==5||parameter==8||parameter==9;
}
int ECICALL eciGetDefaultParam(int parameter)
{
    return default_parameter(parameter)?(int)epoch_get(eci_defaults+parameter):-1;
}
int ECICALL eciSetDefaultParam(int parameter,int value)
{
    if(!default_parameter(parameter))return -1;
    if(parameter==5){if(!sample_hz(value))return -1;}
    else if(parameter==9) {
        int code=value&0xff00;
        if((code&&code!=0x800)||!reloq61_language_supported(value&~0xff00))return -1;
    } else if(value<0||value>1)return -1;
    return (int)epoch_exchange(eci_defaults+parameter,(unsigned)value);
}
static void initialize_eci_voices(struct reloq_eci_engine *e)
{
    static const char *const names[]={"Adult Male 1","Adult Female 1","Child 1",
        "Adult Male 2","Adult Male 3","Adult Female 2","Elderly Female 1","Elderly Male 1"};
    for(unsigned i=0;i<8;++i) {
        struct reloq_inline_word v={0};reloq_inline_preset(&v,(int)i+1);
        int values[]={v.gender,v.head_size,v.pitch,v.inflection,v.roughness,v.breathiness,v.speed,v.volume};
        memcpy(e->saved_voices[i],values,sizeof values);
        memcpy(e->saved_voices[i+8],values,sizeof values);
        strcpy(e->voice_names[i+1],names[i]);strcpy(e->voice_names[i+9],"User-Defined");
    }
    strcpy(e->voice_names[0],names[0]);
}
static int valid_voice(int voice) {return voice>=0&&voice<=16;}
/* The already linked native frontend supplies the SDK's calibration, not
 * another percentage/WPM curve or a second pitch table. */
extern int eci2RealWorld(int32_t,int32_t,int32_t);
extern int realWorld2eci(int32_t,int32_t,int32_t,int32_t,int32_t);
static int voice_value_raw(struct reloq_eci_engine *e,int voice,int parameter)
{
    return voice?e->saved_voices[voice-1][parameter]:eciGetParam(e,voice_parameters[parameter]);
}
int ECICALL eciGetVoiceParam(ECIHand handle,int voice,int parameter)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(voice)||parameter<0||parameter>=8)return eci_refuse(e,-1);
    if(e->real_world_units && (e->real_voice_flags[voice]&(1u<<parameter)))
        return e->real_voice_values[voice][parameter];
    int raw=voice_value_raw(e,voice,parameter);
    return e->real_world_units?eci2RealWorld(1,parameter,raw):raw;
}
int ECICALL eciSetVoiceParam(ECIHand handle,int voice,int parameter,int value)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(voice)||(voice>0&&voice<9)||
       parameter<0||parameter>=8)return eci_refuse(e,-1);
    int maximum=parameter==0?1:parameter==6?250:100,raw=value;
    if(e->real_world_units) {
        if(parameter!=0 && value<1)return eci_refuse(e,-1);
        int lo=eci2RealWorld(1,parameter,0),hi=eci2RealWorld(1,parameter,maximum);
        if(parameter==2) {
            if(value<RELOQ_PITCH_HZ_MIN || value>RELOQ_PITCH_HZ_MAX)return eci_refuse(e,-1);
            /* Keep the public VB shadow in 0..100. The exact requested Hz
             * value is stored separately and used by the renderer. */
            raw=value<lo?0:value>hi?maximum:realWorld2eci(1,parameter,value,0,maximum);
        } else {
            if(value<lo||value>hi)return eci_refuse(e,-1);
            raw=realWorld2eci(1,parameter,value,0,maximum);
        }
    }
    if(raw<0||raw>maximum)return eci_refuse(e,-1);
    int old=eciGetVoiceParam(e,voice,parameter);
    if(voice)e->saved_voices[voice-1][parameter]=raw;
    else {
        int result=eciSetParam(e,voice_parameters[parameter],raw);
        if(result<0)return eci_refuse(e,-1);
        e->voice_mode=RELOQ_ECI_VOICE_GENERIC;
    }
    if(e->real_world_units && (parameter==2||parameter==6||parameter==7)) {
        e->real_voice_values[voice][parameter]=value;e->real_voice_flags[voice]|=1u<<parameter;
    } else e->real_voice_flags[voice]&=~(1u<<parameter);
    return old;
}
int ECICALL eciCopyVoice(ECIHand handle,int from,int to)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(from)||!valid_voice(to)||(to>0&&to<9))return eci_refuse(e,0);
    int values[8];for(int i=0;i<8;++i)values[i]=voice_value_raw(e,from,i);
    if(to)memcpy(e->saved_voices[to-1],values,sizeof values);
    else {
        struct reloq_session *next=make_text_session(values[0],values[1],e->sample_rate_code);
        if(!next)return eci_refuse(e,0);
        lock_session(&e->session_lock);struct reloq_session *old=e->text_session;e->text_session=next;unlock_session(&e->session_lock);
        reloq_session_destroy(old);e->gender=values[0];e->head_size=values[1];
        e->native_pitch=values[2];e->native_inflection=values[3];e->roughness=values[4];e->breathiness=values[5];
        e->native_speed=values[6];e->native_volume=values[7];e->native_pitch_active=e->native_inflection_active=1;
        e->native_speed_active=e->native_volume_active=1;e->voice_mode=RELOQ_ECI_VOICE_GENERIC;
    }
    memmove(e->voice_names[to],e->voice_names[from],sizeof e->voice_names[to]);
    memmove(e->real_voice_values[to],e->real_voice_values[from],sizeof e->real_voice_values[to]);
    e->real_voice_flags[to]=e->real_voice_flags[from];return 1;
}
int ECICALL eciGetVoiceName(ECIHand handle,int voice,void *name)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(voice)||!name)return eci_refuse(e,0);
    int wide=!!(e->language_dialect&0x800),code=1252;
    switch(e->language){case 0x60000:code=936;break;case 0x60001:code=950;break;case 0x80000:code=932;break;case 0xa0000:code=949;break;case 0x120000:code=1253;break;default:break;}
#ifdef _WIN32
    wchar_t units[30];int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,e->voice_names[voice],-1,units,30);
    if(!count)return eci_refuse(e,0);
    if(wide)memcpy(name,units,(size_t)count*sizeof *units);
    else if(!WideCharToMultiByte((unsigned)code,0,units,-1,name,30,NULL,NULL))return eci_refuse(e,0);
#elif defined(__ANDROID__)
    /* The JNI transport uses UTF-8. Keep API-26 builds independent of
     * Android's later, UTF-only iconv; stock names are ASCII. */
    (void)code;
    if(wide) {
        const unsigned char *p=(const unsigned char *)e->voice_names[voice];uint16_t *out=name;
        while(*p) {
            unsigned c=*p++,more=c<128?0:c<224?1:c<240?2:3;
            c&=more==0?127:more==1?31:more==2?15:7;
            while(more--)c=(c<<6)|(*p++&63);
            if(c>0xffff){*out++=(uint16_t)(0xd800+((c-0x10000)>>10));c=0xdc00+((c-0x10000)&1023);}
            *out++=(uint16_t)c;
        }
        *out=0;
    } else {
        for(const unsigned char *p=(const unsigned char *)e->voice_names[voice];*p;++p)if(*p>=128)return eci_refuse(e,0);
        strcpy(name,e->voice_names[voice]);
    }
#else
    char encoding[16];snprintf(encoding,sizeof encoding,"CP%d",code);
    iconv_t converter=iconv_open(wide?"UTF-16LE":encoding,"UTF-8");if(converter==(iconv_t)-1)return eci_refuse(e,0);
    char *from=e->voice_names[voice],*to=name;size_t left=strlen(from),room=wide?58:29;
    size_t status=iconv(converter,&from,&left,&to,&room);iconv_close(converter);
    if(status==(size_t)-1||left)return eci_refuse(e,0);
    *to=0;if(wide)to[1]=0;
#endif
    return 1;
}
/* Convert the language's documented byte code set, or UTF-16LE selected by
 * the language word's 0x800 bit. UTF-8 remains an independent extension. */
static unsigned char *decode_eci_text(struct reloq_eci_engine *e,const void *input,size_t *length,int *error)
{
    int wide=!!(e->language_dialect&0x800),code=1252;
    switch(e->language) {
    case 0x60000:code=936;break;case 0x60001:code=950;break;
    case 0x80000:code=932;break;case 0xa0000:code=949;break;case 0x120000:code=1253;break;
    default:break;
    }
    if(!wide&&code==1252)return decode_cp1252(input,length,error);
    size_t n=0;
    if(wide) {uint16_t c;do{memcpy(&c,(const unsigned char *)input+n,2);if(c)n+=2;}while(c);}
    else n=strlen(input);
    if(n>INT_MAX/4 || n>(SIZE_MAX-1)/4){*error=RELOQ_ECI_MEMORY;return NULL;}
    size_t capacity=4*n+1;unsigned char *out=malloc(capacity);
    if(!out){*error=RELOQ_ECI_MEMORY;return NULL;}
#ifdef _WIN32
    const wchar_t *units=input;wchar_t *allocated=NULL;int count=0;
    if(wide)count=(int)(n/2);
    else {
        if(n>INT_MAX)goto invalid;
        count=MultiByteToWideChar((unsigned)code,MB_ERR_INVALID_CHARS,input,(int)n,NULL,0);
        if(n&&!count)goto invalid;
        allocated=count?malloc((size_t)count*sizeof *allocated):NULL;
        if(count&&!allocated){*error=RELOQ_ECI_MEMORY;free(out);return NULL;}
        if(count&&!MultiByteToWideChar((unsigned)code,MB_ERR_INVALID_CHARS,input,(int)n,allocated,count)){free(allocated);goto invalid;}
        units=allocated;
    }
    int bytes=count?WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,units,count,(char *)out,(int)capacity-1,NULL,NULL):0;
    free(allocated);if(count&&!bytes)goto invalid;*length=(size_t)bytes;
#elif defined(__ANDROID__)
    if(!wide)goto invalid;
    size_t used=0;
    for(size_t i=0;i<n;i+=2) {
        uint16_t first;memcpy(&first,(const unsigned char *)input+i,2);unsigned c=first;
        if(c>=0xd800&&c<=0xdbff) {
            if(i+2>=n)goto invalid;
            uint16_t second;memcpy(&second,(const unsigned char *)input+i+2,2);i+=2;
            if(second<0xdc00||second>0xdfff)goto invalid;
            c=0x10000+((c-0xd800)<<10)+(second-0xdc00);
        } else if(c>=0xdc00&&c<=0xdfff)goto invalid;
        if(c<128)out[used++]=(unsigned char)c;
        else if(c<2048){out[used++]=(unsigned char)(0xc0|(c>>6));out[used++]=(unsigned char)(0x80|(c&63));}
        else if(c<65536){out[used++]=(unsigned char)(0xe0|(c>>12));out[used++]=(unsigned char)(0x80|((c>>6)&63));out[used++]=(unsigned char)(0x80|(c&63));}
        else {out[used++]=(unsigned char)(0xf0|(c>>18));out[used++]=(unsigned char)(0x80|((c>>12)&63));out[used++]=(unsigned char)(0x80|((c>>6)&63));out[used++]=(unsigned char)(0x80|(c&63));}
    }
    *length=used;
#else
    char encoding[16];snprintf(encoding,sizeof encoding,"CP%d",code);
    iconv_t converter=iconv_open("UTF-8",wide?"UTF-16LE":encoding);
    if(converter==(iconv_t)-1)goto invalid;
    char *from=(char *)input,*to=(char *)out;size_t left=n,room=capacity-1;
    size_t result=iconv(converter,&from,&left,&to,&room);iconv_close(converter);
    if(result==(size_t)-1||left)goto invalid;
    *length=(size_t)(to-(char *)out);
#endif
    out[*length]=0;return out;
invalid:
    free(out);*error=RELOQ_ECI_INVALID;return NULL;
}
int ECICALL eciSetVoiceName(ECIHand handle,int voice,const void *name)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(voice)||(voice>0&&voice<9)||!name)return 0;
    size_t input_units=0;
    if(e->language_dialect&0x800) {
        for(;input_units<30;++input_units) {
            uint16_t value;memcpy(&value,(const unsigned char *)name+2*input_units,2);
            if(!value)break;
        }
    } else while(input_units<30&&((const unsigned char *)name)[input_units])++input_units;
    if(input_units>=30)return 0;
    size_t n=0;int error=0;unsigned char *text=decode_eci_text(e,name,&n,&error);
    if(!text||n>=sizeof e->voice_names[voice]){free(text);return 0;}
    memcpy(e->voice_names[voice],text,n+1);free(text);return 1;
}
static struct reloq_eci_dictionary *find_eci_dictionary(struct reloq_eci_engine *e,void *handle)
{
    for(struct reloq_eci_dictionary *d=e->dictionaries;d;d=d->next)if(d==handle)return d;
    return NULL;
}
static void clear_eci_dictionaries(struct reloq_eci_engine *e)
{
    while(e->dictionaries) {struct reloq_eci_dictionary *d=e->dictionaries;e->dictionaries=d->next;
        reloq61_worker_free(d->api_worker);reloq61_worker_free(d->cached_worker);
        for(unsigned i=0;i<4;++i){free(d->volumes[i]);}free(d);}
}
static void *dictionary_worker(struct reloq_eci_engine *e,int language,const struct reloq_eci_dictionary *set,int directory)
{
    void *w=reloq61_worker_new(language);if(!w)return NULL;
    int status=set?reloq61_worker_dictionary_data(w,(const unsigned char *const *)set->volumes,set->sizes):
        directory?reloq61_worker_dictionary(w,e->dictionary_path):0;
    if(status){reloq61_worker_free(w);return NULL;}return w;
}

/* Each dictionary set owns its prepared worker while inactive. The active
 * worker remains on the engine; moving it between these two owners avoids
 * reparsing the volumes on every language switch or dictionary selection. */
static void park_eci_dictionary_worker(struct reloq_eci_engine *e)
{
    if(e->active_dictionary) {
        reloq61_worker_free(e->active_dictionary->cached_worker);
        e->active_dictionary->cached_worker=e->rule_worker;
    } else reloq61_worker_free(e->rule_worker);
    e->rule_worker=NULL;
}
static void *take_eci_dictionary_worker(struct reloq_eci_engine *e,int language,
    struct reloq_eci_dictionary *set,int directory)
{
    if(set&&set->cached_worker) {
        void *w=set->cached_worker;set->cached_worker=NULL;return w;
    }
    return dictionary_worker(e,language,set,directory);
}

static int switch_eci_language(struct reloq_eci_engine *e,int language)
{
    int base=language&~0xff00;
    if(((language&0xff00)!=0&&(language&0xff00)!=0x800)||!reloq61_language_supported(base)||dictionary_language_slot(base)>=15)return -1;
    if(base==e->language){e->language_dialect=language;return 0;}
    struct reloq_eci_dictionary *set=e->language_dictionaries[dictionary_language_slot(base)];
    void *next=take_eci_dictionary_worker(e,base,set,1);if(!next)return -1;
    park_eci_dictionary_worker(e);e->rule_worker=next;
    e->active_dictionary=set;
    e->language=base;e->language_dialect=language;return 0;
}
ECIDictHand ECICALL eciNewDict(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;if(!e||epoch_get(&e->busy))return NULL;
    struct reloq_eci_dictionary *d=calloc(1,sizeof *d);if(!d)return NULL;
    d->language=e->language;d->next=e->dictionaries;e->dictionaries=d;return d;
}
ECIDictHand ECICALL eciGetDict(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;return e&&!epoch_get(&e->busy)?e->active_dictionary:NULL;
}
int ECICALL eciSetDict(ECIHand handle,ECIDictHand dictionary)
{
    struct reloq_eci_engine *e=handle;if(!e||epoch_get(&e->busy))return eci_refuse(e,eciDictInternalError);
    struct reloq_eci_dictionary *d=find_eci_dictionary(e,dictionary);if(dictionary&&!d)return eci_refuse(e,eciDictInternalError);
    unsigned slot=dictionary_language_slot(d?d->language:e->language);
    if(d&&d->language!=e->language){e->language_dictionaries[slot]=d;return eciDictNoError;}
    if(d&&d==e->active_dictionary) {
        e->language_dictionaries[slot]=d;free(e->dictionary_path);e->dictionary_path=NULL;
        return eciDictNoError;
    }
    void *w=take_eci_dictionary_worker(e,e->language,d,0);
    if(!w)return eciDictOutOfMemory;
    e->language_dictionaries[slot]=d;
    park_eci_dictionary_worker(e);e->rule_worker=w;e->active_dictionary=d;free(e->dictionary_path);e->dictionary_path=NULL;return eciDictNoError;
}
ECIDictHand ECICALL eciDeleteDict(ECIHand handle,ECIDictHand dictionary)
{
    struct reloq_eci_engine *e=handle;if(!e||epoch_get(&e->busy))return dictionary;
    struct reloq_eci_dictionary *d=find_eci_dictionary(e,dictionary);if(!d)return dictionary;
    if(e->active_dictionary==d&&eciSetDict(e,NULL)!=eciDictNoError)return dictionary;
    for(unsigned i=0;i<15;++i)if(e->language_dictionaries[i]==d)e->language_dictionaries[i]=NULL;
    struct reloq_eci_dictionary **at=&e->dictionaries;while(*at!=d)at=&(*at)->next;*at=d->next;
    reloq61_worker_free(d->api_worker);reloq61_worker_free(d->cached_worker);
    for(unsigned i=0;i<4;++i){free(d->volumes[i]);}free(d);return NULL;
}
int ECICALL eciLoadDict(ECIHand handle,ECIDictHand dictionary,int volume,const void *filename)
{
    struct reloq_eci_engine *e=handle;if(!e||epoch_get(&e->busy))return 6;
    struct reloq_eci_dictionary *d=find_eci_dictionary(e,dictionary);if(!d||!filename)return 6;
    if(volume<0||volume>2)return 7;
    size_t n=0;int error=0;unsigned char *path=decode_eci_text(e,filename,&n,&error);
    if(!path||!n||n>=8192){free(path);return error==RELOQ_ECI_MEMORY?2:6;}
    FILE *f=open_utf8_lexicon((char *)path);free(path);if(!f)return 6;
    if(fseek(f,0,SEEK_END)){fclose(f);return 6;}long bytes=ftell(f);
    if(bytes<0||bytes>8*1024*1024||fseek(f,0,SEEK_SET)){fclose(f);return 6;}
    unsigned char *data=malloc((size_t)bytes+1);if(!data){fclose(f);return 2;}
    if(fread(data,1,(size_t)bytes,f)!=(size_t)bytes||ferror(f)){fclose(f);free(data);return 6;}
    fclose(f);data[bytes]=0;
    struct reloq_eci_dictionary candidate=*d;candidate.volumes[volume]=data;candidate.sizes[volume]=(size_t)bytes;
    void *w=dictionary_worker(e,d->language,&candidate,0);if(!w){free(data);return 6;}
    if(e->active_dictionary==d){reloq61_worker_free(e->rule_worker);e->rule_worker=w;}
    else {reloq61_worker_free(d->cached_worker);d->cached_worker=w;}
    reloq61_worker_free(d->api_worker);d->api_worker=NULL;memset(d->walk_active,0,sizeof d->walk_active);
    free(d->volumes[volume]);d->volumes[volume]=data;d->sizes[volume]=(size_t)bytes;return 0;
}

/* One reset contract for every client. Keep host routing, owned dictionary
 * handles and the independent acoustic-profile choice; reset SDK settings,
 * voice slots, input and renderer histories to their configured defaults. */
static int reset_eci_defaults(struct reloq_eci_engine *e)
{
    if(!busy_begin(&e->busy))return 0;
    struct reloq_eci_engine *n=eciNew();
    if(!n){busy_end(&e->busy);return 0;}
    struct reloq_eci_dictionary *set=e->language_dictionaries[dictionary_language_slot(n->language)];
    n->rule_worker=set==e->active_dictionary?dictionary_worker(e,n->language,set,1):
        take_eci_dictionary_worker(e,n->language,set,1);
    if(!n->rule_worker){eciDelete(n);busy_end(&e->busy);return 0;}
    lock_session(&e->session_lock);
    struct reloq_session *old=e->session;e->session=n->session;n->session=old;
    old=e->text_session;e->text_session=n->text_session;n->text_session=old;
    void *worker=e->rule_worker;e->rule_worker=n->rule_worker;n->rule_worker=worker;
    if(e->active_dictionary&&e->active_dictionary!=set) {
        reloq61_worker_free(e->active_dictionary->cached_worker);
        e->active_dictionary->cached_worker=n->rule_worker;n->rule_worker=NULL;
    }
    unlock_session(&e->session_lock);
    clear_queue(e);
    e->sample_rate_code=n->sample_rate_code;e->synth_mode=n->synth_mode;e->voice_mode=n->voice_mode;
    e->generic_rate=n->generic_rate;e->generic_pitch=n->generic_pitch;e->generic_volume=n->generic_volume;
    e->native_speed=n->native_speed;e->native_wpm=0;e->native_pitch=n->native_pitch;
    e->native_inflection=n->native_inflection;e->native_volume=n->native_volume;
    e->native_speed_active=e->native_pitch_active=e->native_inflection_active=e->native_volume_active=1;
    e->input_type=n->input_type;e->process_tags=n->process_tags;e->text_mode=n->text_mode;
    e->dictionary_disabled=n->dictionary_disabled;e->real_world_units=n->real_world_units;
    e->phrase_prediction=0;e->pause_mode=0;e->mellow_mode=0;
    e->eq_low=e->eq_mid=e->eq_high=12;
    reset_equalizer_filters(e);
    e->language=n->language;e->language_dialect=n->language_dialect;
    e->active_dictionary=set;
    e->gender=n->gender;e->head_size=n->head_size;e->roughness=n->roughness;e->breathiness=n->breathiness;
    e->german_glide_opening=n->german_glide_opening;e->german_glide_backness=n->german_glide_backness;
    e->german_glide_softness=n->german_glide_softness;
    memcpy(e->saved_voices,n->saved_voices,sizeof e->saved_voices);
    memcpy(e->voice_names,n->voice_names,sizeof e->voice_names);
    memset(e->real_voice_flags,0,sizeof e->real_voice_flags);
    eci_error(e,RELOQ_ECI_OK);e->last_synthesis_error=RELOQ_ECI_OK;
    epoch_exchange(&e->program_status,0);epoch_exchange(&e->sdk_error,0);
    e->failure_stage=RELOQ_ECI_STAGE_NONE;e->failure_utf8_offset=-1;
    epoch_exchange(&e->paused,0);
    eciDelete(n);busy_end(&e->busy);return 1;
}
