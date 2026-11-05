/* Public dictionary editing uses the same native parser/hash tables as
 * synthesis. Published strings belong to the ECI instance, not to a file. */
extern const char *ECICALL es_engsynDictLookup(void *,int,const char *);
extern int ECICALL es_engsynDictFindFirst(void *,int,const char **,const char **);
extern int ECICALL es_engsynDictFindNext(void *,int,const char **,const char **);
extern int ECICALL es_engsynUpdateDict(void *,int,const char *,const char *);

static int dictionary_no_output(void *context,const char *s,size_t n)
{(void)context;(void)s;(void)n;return 0;}
static void *dictionary_api_worker(const struct reloq_eci_dictionary *d)
{
    static const unsigned char empty[1]={0};
    const unsigned char *data[4];
    for(unsigned i=0;i<4;++i)data[i]=d->volumes[i];
    if(!data[0])data[0]=empty;
    void *w=reloq61_worker_new(d->language);if(!w)return NULL;
    if(reloq61_worker_dictionary_data(w,data,d->sizes)){reloq61_worker_free(w);return NULL;}
    struct reloq61_request q={.language=d->language,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,100},.text=""};
    struct reloq61_result result={0};void *previous=reloq61_worker_scope(w);
    int status=reloq61_generate_phonemes(&q,&result,dictionary_no_output,NULL);
    reloq61_worker_scope(previous);
    if(status||!reloq61_worker_dictionary_handle(w)){reloq61_worker_free(w);return NULL;}
    return w;
}
static int dictionary_codepage(int language)
{
    switch(language) {
    case 0x60000:return 936;case 0x60001:return 950;case 0x80000:return 932;
    case 0xa0000:return 949;case 0x120000:return 1253;default:return 1252;
    }
}
static char *dictionary_convert(struct reloq_eci_engine *e,const void *input,int from_wide,int to_wide)
{
    if(!input)return NULL;
    size_t bytes=0;
    if(from_wide) {
        uint16_t unit;
        do{memcpy(&unit,(const char *)input+bytes,2);if(unit)bytes+=2;}while(unit&&bytes<=8192);
    } else {bytes=strlen(input);}
    if(bytes>8192)return NULL;
    size_t room=4*bytes+4;char *out=calloc(room,1);if(!out)return NULL;
    if(from_wide==to_wide){memcpy(out,input,bytes);return out;}
    int code=dictionary_codepage(e->language);
#ifdef _WIN32
    int n=from_wide?WideCharToMultiByte((unsigned)code,0,input,(int)(bytes/2),out,(int)room,NULL,NULL):
        MultiByteToWideChar((unsigned)code,MB_ERR_INVALID_CHARS,input,(int)bytes,(wchar_t *)out,(int)(room/2));
    if(bytes&&!n){free(out);return NULL;}
#elif !defined(__ANDROID__)
    char encoding[16];snprintf(encoding,sizeof encoding,"CP%d",code);
    iconv_t converter=iconv_open(to_wide?"UTF-16LE":encoding,from_wide?"UTF-16LE":encoding);
    if(converter==(iconv_t)-1){free(out);return NULL;}
    char *from=(char *)input,*to=out;size_t left=bytes,available=room-2;
    size_t result=iconv(converter,&from,&left,&to,&available);iconv_close(converter);
    if(result==(size_t)-1||left){free(out);return NULL;}
#else
    /* JNI clients use UTF-8. Legacy wide dictionary strings require a
     * platform converter unavailable at the supported Android API level. */
    (void)code;free(out);return NULL;
#endif
    return out;
}
static int dictionary_check(struct reloq_eci_engine *e,void *handle,int volume,
    struct reloq_eci_dictionary **out)
{
    if(!e||epoch_get(&e->busy))return eci_refuse(e,eciDictAccessError);
    struct reloq_eci_dictionary *d=find_eci_dictionary(e,handle);
    if(!d)return eci_refuse(e,eciDictAccessError);
    if(volume<0||volume>2)return eciDictInvalidVolume;
    if(!d->api_worker)d->api_worker=dictionary_api_worker(d);
    if(!d->api_worker){eci_record_error(e,2);return eciDictOutOfMemory;}
    *out=d;return eciDictNoError;
}
static int dictionary_publish(struct reloq_eci_engine *e,const char *key,const char *value,
    const char **key_out,const char **value_out)
{
    int wide=!!(e->language_dialect&0x800);
    char *k=key_out?dictionary_convert(e,key,0,wide):NULL;
    char *v=dictionary_convert(e,value,0,wide);
    if((key_out&&!k)||!v){free(k);free(v);return eciDictErrLookUpKey;}
    free(e->dictionary_key_result);free(e->dictionary_value_result);
    e->dictionary_key_result=k;e->dictionary_value_result=v;
    if(key_out)*key_out=k;
    *value_out=v;return eciDictNoError;
}
int ECICALL eciDictLookupA(ECIHand handle,ECIDictHand dictionary,int volume,
    const void *key,const char **translation,ECIPartOfSpeech *part)
{
    struct reloq_eci_engine *e=handle;struct reloq_eci_dictionary *d=NULL;
    if(!translation)return eci_refuse(e,eciDictAccessError);
    *translation=NULL;if(part)*part=0;
    int result=dictionary_check(e,dictionary,volume,&d);if(result)return result;
    char *converted=dictionary_convert(e,key,!!(e->language_dialect&0x800),0);
    if(!converted||!*converted){free(converted);return eciDictErrLookUpKey;}
    const char *value=es_engsynDictLookup(reloq61_worker_dictionary_handle(d->api_worker),volume,converted);
    free(converted);if(!value)return eciDictNoEntry;
    return dictionary_publish(e,NULL,value,NULL,translation);
}
const char *ECICALL eciDictLookup(ECIHand handle,ECIDictHand dictionary,int volume,const void *key)
{
    struct reloq_eci_engine *e=handle;
    if(!e||(e->language_dialect&0x800))return NULL;
    const char *out=NULL;
    return eciDictLookupA(e,dictionary,volume,key,&out,NULL)==eciDictNoError?out:NULL;
}
static int dictionary_find(struct reloq_eci_engine *e,void *handle,int volume,
    const char **key,const char **value,ECIPartOfSpeech *part,int first)
{
    if(!key||!value)return eci_refuse(e,eciDictAccessError);
    *key=*value=NULL;if(part)*part=0;
    struct reloq_eci_dictionary *d=NULL;int result=dictionary_check(e,handle,volume,&d);
    if(result)return result;
    if(!first&&!d->walk_active[volume])return eciDictNoEntry;
    const char *k=NULL,*v=NULL;void *set=reloq61_worker_dictionary_handle(d->api_worker);
    int status=first?es_engsynDictFindFirst(set,volume,&k,&v):es_engsynDictFindNext(set,volume,&k,&v);
    d->walk_active[volume]=status==0;
    if(status)return status==2?eciDictOutOfMemory:eciDictNoEntry;
    return dictionary_publish(e,k,v,key,value);
}
int ECICALL eciDictFindFirst(ECIHand h,ECIDictHand d,int v,const char **k,const char **x)
{return dictionary_find(h,d,v,k,x,NULL,1);}
int ECICALL eciDictFindNext(ECIHand h,ECIDictHand d,int v,const char **k,const char **x)
{return dictionary_find(h,d,v,k,x,NULL,0);}
int ECICALL eciDictFindFirstA(ECIHand h,ECIDictHand d,int v,const char **k,const char **x,ECIPartOfSpeech *p)
{return dictionary_find(h,d,v,k,x,p,1);}
int ECICALL eciDictFindNextA(ECIHand h,ECIDictHand d,int v,const char **k,const char **x,ECIPartOfSpeech *p)
{return dictionary_find(h,d,v,k,x,p,0);}
static int dictionary_serialize(void *worker,int volume,unsigned char **out,size_t *size)
{
    const char *k=NULL,*v=NULL;void *set=reloq61_worker_dictionary_handle(worker);
    size_t used=0,capacity=1;unsigned char *data=malloc(1);if(!data)return eciDictOutOfMemory;
    int status=es_engsynDictFindFirst(set,volume,&k,&v);
    while(!status) {
        size_t a=strlen(k),b=strlen(v),n=a+b+2;
        if(n>8u*1024u*1024u-used){free(data);return eciDictOutOfMemory;}
        if(used+n>=capacity) {
            size_t next=(used+n+1)*2;void *p=realloc(data,next);
            if(!p){free(data);return eciDictOutOfMemory;}data=p;capacity=next;
        }
        memcpy(data+used,k,a);used+=a;data[used++]='\t';
        memcpy(data+used,v,b);used+=b;data[used++]='\n';
        status=es_engsynDictFindNext(set,volume,&k,&v);
    }
    if(status!=4){free(data);return eciDictInternalError;}
    data[used]=0;*out=data;*size=used;return eciDictNoError;
}
int ECICALL eciUpdateDictA(ECIHand handle,ECIDictHand dictionary,int volume,
    const void *key,const void *translation,ECIPartOfSpeech part)
{
    struct reloq_eci_engine *e=handle;struct reloq_eci_dictionary *d=NULL;
    int result=dictionary_check(e,dictionary,volume,&d);if(result)return result;
    (void)part;int wide=!!(e->language_dialect&0x800);
    char *k=dictionary_convert(e,key,wide,0),*v=translation?dictionary_convert(e,translation,wide,0):NULL;
    if(!k||!*k||strlen(k)>128||strpbrk(k,"\t\r\n")||
       (translation&&(!v||strlen(v)>512||strpbrk(v,"\t\r\n")))) {
        free(k);free(v);return eciDictErrLookUpKey;
    }
    void *candidate=dictionary_api_worker(d);
    if(!candidate){free(k);free(v);return eciDictOutOfMemory;}
    result=es_engsynUpdateDict(reloq61_worker_dictionary_handle(candidate),volume,k,v);
    free(k);free(v);
    unsigned char *data=NULL;size_t size=0;
    if(!result)result=dictionary_serialize(candidate,volume,&data,&size);
    reloq61_worker_free(candidate);if(result)return result==2?2:eciDictInternalError;
    struct reloq_eci_dictionary next=*d;next.volumes[volume]=data;next.sizes[volume]=size;
    void *api=dictionary_api_worker(&next);
    void *prepared=dictionary_worker(e,d->language,&next,0);
    if(!api||!prepared) {
        reloq61_worker_free(api);reloq61_worker_free(prepared);free(data);return eciDictOutOfMemory;
    }
    reloq61_worker_free(d->api_worker);d->api_worker=api;
    if(e->active_dictionary==d){reloq61_worker_free(e->rule_worker);e->rule_worker=prepared;}
    else {reloq61_worker_free(d->cached_worker);d->cached_worker=prepared;}
    free(d->volumes[volume]);d->volumes[volume]=data;d->sizes[volume]=size;
    memset(d->walk_active,0,sizeof d->walk_active);return eciDictNoError;
}
int ECICALL eciUpdateDict(ECIHand h,ECIDictHand d,int v,const void *k,const void *x)
{return eciUpdateDictA(h,d,v,k,x,0);}
int ECICALL eciSaveDict(ECIHand handle,ECIDictHand dictionary,int volume,const void *filename)
{
    struct reloq_eci_engine *e=handle;struct reloq_eci_dictionary *d=NULL;
    int result=dictionary_check(e,dictionary,volume,&d);if(result)return result;
    if(!filename)return eci_refuse(e,eciDictAccessError);
    unsigned char *data=NULL;size_t size=0;
    result=dictionary_serialize(d->api_worker,volume,&data,&size);
    d->walk_active[volume]=0;if(result)return result;
    size_t n=0;int error=0;unsigned char *path=decode_eci_text(e,filename,&n,&error);
    if(!path||!n||n>=8192){free(path);free(data);return eciDictAccessError;}
    FILE *file=NULL;
#ifdef _WIN32
    int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,(char *)path,-1,NULL,0);
    wchar_t *wide=count?malloc((size_t)count*sizeof *wide):NULL;
    if(wide&&MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,(char *)path,-1,wide,count))file=_wfopen(wide,L"wt");
    free(wide);
#else
    file=fopen((char *)path,"wt");
#endif
    free(path);if(!file){free(data);return eciDictAccessError;}
    int failed=fwrite(data,1,size,file)!=size||ferror(file);
    if(fclose(file))failed=1;
    free(data);return failed?eciDictAccessError:eciDictNoError;
}
