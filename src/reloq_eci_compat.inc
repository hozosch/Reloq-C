/* Independently expressed public ECI controls. The renderer and the existing
 * vendor entry points share these per-instance settings and dictionary data. */
static const int voice_parameters[]={RELOQ_ECI_GENDER,RELOQ_ECI_HEAD_SIZE,
    RELOQ_ECI_NATIVE_PITCH,RELOQ_ECI_NATIVE_INFLECTION,RELOQ_ECI_ROUGHNESS,
    RELOQ_ECI_BREATHINESS,RELOQ_ECI_NATIVE_SPEED,RELOQ_ECI_NATIVE_VOLUME};
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
int ECICALL eciGetVoiceParam(ECIHand handle,int voice,int parameter)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(voice)||parameter<0||parameter>=8)return -1;
    return voice?e->saved_voices[voice-1][parameter]:eciGetParam(e,voice_parameters[parameter]);
}
int ECICALL eciSetVoiceParam(ECIHand handle,int voice,int parameter,int value)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(voice)||(voice>0&&voice<9)||
       parameter<0||parameter>=8||value<0||value>(parameter==0?1:parameter==6?250:100))return -1;
    if(voice) {int old=e->saved_voices[voice-1][parameter];e->saved_voices[voice-1][parameter]=value;return old;}
    int old=eciSetParam(e,voice_parameters[parameter],value);
    if(old>=0)e->voice_mode=RELOQ_ECI_VOICE_GENERIC;
    return old;
}
int ECICALL eciCopyVoice(ECIHand handle,int from,int to)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(from)||!valid_voice(to)||(to>0&&to<9))return 0;
    int values[8];for(int i=0;i<8;++i)values[i]=eciGetVoiceParam(e,from,i);
    if(to)memcpy(e->saved_voices[to-1],values,sizeof values);
    else {
        struct reloq_session *next=make_text_session(values[0],values[1],e->sample_rate_code);
        if(!next)return 0;
        lock_session(&e->session_lock);struct reloq_session *old=e->text_session;e->text_session=next;unlock_session(&e->session_lock);
        reloq_session_destroy(old);e->gender=values[0];e->head_size=values[1];
        e->native_pitch=values[2];e->native_inflection=values[3];e->roughness=values[4];e->breathiness=values[5];
        e->native_speed=values[6];e->native_volume=values[7];e->native_pitch_active=e->native_inflection_active=1;
        e->native_speed_active=e->native_volume_active=1;e->voice_mode=RELOQ_ECI_VOICE_GENERIC;
    }
    memmove(e->voice_names[to],e->voice_names[from],30);return 1;
}
int ECICALL eciGetVoiceName(ECIHand handle,int voice,void *name)
{
    struct reloq_eci_engine *e=handle;
    if(!e||epoch_get(&e->busy)||!valid_voice(voice)||!name)return 0;
    int wide=!!(e->language_dialect&0x800),code=1252;
    switch(e->language){case 0x60000:code=936;break;case 0x60001:code=950;break;case 0x80000:code=932;break;case 0xa0000:code=949;break;case 0x120000:code=1253;break;default:break;}
#ifdef _WIN32
    wchar_t units[30];int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,e->voice_names[voice],-1,units,30);
    if(!count)return 0;
    if(wide)memcpy(name,units,(size_t)count*sizeof *units);
    else if(!WideCharToMultiByte((unsigned)code,0,units,-1,name,30,NULL,NULL))return 0;
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
        for(const unsigned char *p=(const unsigned char *)e->voice_names[voice];*p;++p)if(*p>=128)return 0;
        strcpy(name,e->voice_names[voice]);
    }
#else
    char encoding[16];snprintf(encoding,sizeof encoding,"CP%d",code);
    iconv_t converter=iconv_open(wide?"UTF-16LE":encoding,"UTF-8");if(converter==(iconv_t)-1)return 0;
    char *from=e->voice_names[voice],*to=name;size_t left=strlen(from),room=wide?58:29;
    size_t status=iconv(converter,&from,&left,&to,&room);iconv_close(converter);
    if(status==(size_t)-1||left)return 0;
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
    size_t n=0;int error=0;unsigned char *text=decode_eci_text(e,name,&n,&error);
    if(!text||n>=30){free(text);return 0;}
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
        for(unsigned i=0;i<4;++i){free(d->volumes[i]);}free(d);}
}
static void *dictionary_worker(struct reloq_eci_engine *e,int language,const struct reloq_eci_dictionary *set,int directory)
{
    void *w=reloq61_worker_new(language);if(!w)return NULL;
    int status=set?reloq61_worker_dictionary_data(w,(const unsigned char *const *)set->volumes,set->sizes):
        directory?reloq61_worker_dictionary(w,e->dictionary_path):0;
    if(status){reloq61_worker_free(w);return NULL;}return w;
}

static int switch_eci_language(struct reloq_eci_engine *e,int language)
{
    int base=language&~0xff00;
    if(((language&0xff00)!=0&&(language&0xff00)!=0x800)||!reloq61_language_supported(base))return -1;
    if(base==e->language){e->language_dialect=language;return 0;}
    void *next=dictionary_worker(e,base,e->active_dictionary,1);if(!next)return -1;
    reloq61_worker_free(e->rule_worker);e->rule_worker=next;
    e->language=base;e->language_dialect=language;return 0;
}
ECIDictHand ECICALL eciNewDict(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;if(!e||epoch_get(&e->busy))return NULL;
    struct reloq_eci_dictionary *d=calloc(1,sizeof *d);if(!d)return NULL;
    d->next=e->dictionaries;e->dictionaries=d;return d;
}
ECIDictHand ECICALL eciGetDict(ECIHand handle)
{
    struct reloq_eci_engine *e=handle;return e&&!epoch_get(&e->busy)?e->active_dictionary:NULL;
}
int ECICALL eciSetDict(ECIHand handle,ECIDictHand dictionary)
{
    struct reloq_eci_engine *e=handle;if(!e||epoch_get(&e->busy))return 0;
    struct reloq_eci_dictionary *d=find_eci_dictionary(e,dictionary);if(dictionary&&!d)return 0;
    void *w=dictionary_worker(e,e->language,d,0);
    if(!w)return 0;
    reloq61_worker_free(e->rule_worker);e->rule_worker=w;e->active_dictionary=d;free(e->dictionary_path);e->dictionary_path=NULL;return 1;
}
ECIDictHand ECICALL eciDeleteDict(ECIHand handle,ECIDictHand dictionary)
{
    struct reloq_eci_engine *e=handle;if(!e||epoch_get(&e->busy))return dictionary;
    struct reloq_eci_dictionary *d=find_eci_dictionary(e,dictionary);if(!d)return dictionary;
    if(e->active_dictionary==d&&!eciSetDict(e,NULL))return dictionary;
    struct reloq_eci_dictionary **at=&e->dictionaries;while(*at!=d)at=&(*at)->next;*at=d->next;
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
    void *w=dictionary_worker(e,e->language,&candidate,0);if(!w){free(data);return 6;}
    if(e->active_dictionary==d){reloq61_worker_free(e->rule_worker);e->rule_worker=w;}else reloq61_worker_free(w);
    free(d->volumes[volume]);d->volumes[volume]=data;d->sizes[volume]=(size_t)bytes;return 0;
}
