#include "reloq_lexicon.h"
#include "reloq_german_lexicon.h"
/* Optional main/root/abbreviation data is owned by the native grammar.
 * The German index is a conservative admission filter only. An actual native
 * application must be observed before changing the own planner's word path. */
#include <errno.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#endif
extern void *STDCALL es_engsynNewDict(delta_state *);
extern int32_t STDCALL es_engsynDeleteDict(void *);
extern int32_t STDCALL es_engsynDictFindFirst(void *,int32_t,const char **,const char **);
extern int32_t STDCALL es_engsynDictFindNext(void *,int32_t,const char **,const char **);
extern int reloq61_dictionary_read(void *,int,FILE *,size_t);
extern int reloq61_dictionary_attach(void *,delta_state *);
static void dictionary_clear(struct provider_worker *w)
{
    for(size_t i=0;i<w->dictionary_key_count;++i)free(w->dictionary_keys[i]);
    free(w->dictionary_keys);w->dictionary_keys=NULL;w->dictionary_key_count=0;
    memset(w->dictionary_lengths,0,sizeof w->dictionary_lengths);
    memset(w->dictionary_starts,0,sizeof w->dictionary_starts);w->dictionary_checked=0;
    if(w->dictionary) {
        delta_state *d=w->handle?ew_machine(((ECIinstance *)w->handle->fresh)->thread->engine):NULL;
        reloq61_dictionary_attach(w->dictionary,d);es_engsynDeleteDict(w->dictionary);
    }
    w->dictionary=NULL;
}
#ifdef _WIN32
static wchar_t *dictionary_wide(const char *path)
{
    int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,NULL,0);
    wchar_t *wide=count?malloc((size_t)count*sizeof *wide):NULL;
    if(wide&&!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,wide,count)){free(wide);wide=NULL;}
    return wide;
}
#endif
static int dictionary_directory(const char *path)
{
#ifdef _WIN32
    wchar_t *wide=dictionary_wide(path);if(!wide)return 0;
    DWORD attributes=GetFileAttributesW(wide);free(wide);
    return attributes!=INVALID_FILE_ATTRIBUTES&&(attributes&FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat info;return !stat(path,&info)&&S_ISDIR(info.st_mode);
#endif
}
static FILE *dictionary_open(const char *directory,const char *name)
{
    size_t n=strlen(directory),m=strlen(name);
    if(n>8191||m>32)return NULL;
    char *path=malloc(n+m+2);if(!path)return NULL;
    memcpy(path,directory,n);path[n]='/';memcpy(path+n+1,name,m+1);
    FILE *f=NULL;
#ifdef _WIN32
    wchar_t *wide=dictionary_wide(path);if(wide)f=_wfopen(wide,L"rb");free(wide);
#else
    f=fopen(path,"rb");
#endif
    free(path);return f;
}
static const char *dictionary_prefix(int language)
{
    switch(language) {
    case 0x10000:return "ENU";case 0x10001:return "ENG";
    case 0x20000:return "ESP";case 0x20001:return "ESM";
    case 0x30000:return "FRA";case 0x30001:return "FRC";
    case 0x40000:return "DEU";case 0x50000:return "ITA";
    case 0x60000:case 0x60001:return "CHS";
    case 0x70000:return "PTB";case 0x80000:return "JPN";
    case 0x90000:return "FIN";case 0xa0000:return "KOR";
    case 0x120000:return "ELL";default:return NULL;
    }
}
static int dictionary_compare(const void *a,const void *b)
{return strcmp(*(const char *const *)a,*(const char *const *)b);}
/* Fold only an admission key, never the input or a pronunciation. False
 * positives fall back unchanged when the native grammar reports no hit. */
static size_t dictionary_fold(const unsigned char *s,size_t n,char out[512])
{
    size_t used=0;
    for(size_t i=0;i<n&&used<510;++i) {
        unsigned char c=s[i];
        if((c>='A'&&c<='Z')||(c>=0xc0&&c<=0xde&&c!=0xd7))c+=32;
        if(c==0xe4)c='a';else if(c==0xf6)c='o';else if(c==0xfc)c='u';
        if(c==0xdf){out[used++]='s';out[used++]='s';}else out[used++]=(char)c;
    }
    out[used]=0;return used;
}
extern int reloq61_dictionary_contains_lexical(void *,const unsigned char *,size_t);
static int dictionary_index(struct provider_worker *w)
{
    size_t capacity=0;
    /* Lexical records use the standard lookup. Abbreviations and data that
     * cannot form one lexical record retain native token normalization. */
    for(int volume=0;volume<3;++volume) {
        const char *key,*value;int status=es_engsynDictFindFirst(w->dictionary,volume,&key,&value);
        while(!status) {
            if(volume<2&&reloq61_dictionary_contains_lexical(w->dictionary,
                (const unsigned char *)key,strlen(key)))goto next_key;
            char folded[512];size_t n=dictionary_fold((const unsigned char *)key,strlen(key),folded);
            if(n&&n<256) {
                if(w->dictionary_key_count==capacity) {
                    size_t next=capacity?capacity*2:32;
                    if(next>SIZE_MAX/sizeof *w->dictionary_keys)return -1;
                    void *p=realloc(w->dictionary_keys,next*sizeof *w->dictionary_keys);if(!p)return -1;
                    w->dictionary_keys=p;capacity=next;
                }
                char *copy=malloc(n+1);if(!copy)return -1;memcpy(copy,folded,n+1);
                w->dictionary_keys[w->dictionary_key_count++]=copy;
                w->dictionary_lengths[n/64]|=UINT64_C(1)<<(n%64);
                w->dictionary_starts[(unsigned char)folded[0]]=1;
            }
next_key:
            status=es_engsynDictFindNext(w->dictionary,volume,&key,&value);
        }
    }
    if(w->dictionary_key_count)qsort(w->dictionary_keys,w->dictionary_key_count,sizeof *w->dictionary_keys,dictionary_compare);
    return 0;
}
extern int reloq61_dictionary_compile_words(void *,void *,int (*)(void *,const char *,char *,size_t),int);
static void dictionary_single_primary(char *spr)
{
    int primary=0;
    for(char *p=spr;p[0]&&p[1];++p)if(p[0]=='.'&&p[1]=='1') {
        if(primary)p[1]='2';else primary=1;
    }
}
static int dictionary_compile_alias(void *context,const char *value,char *spr,size_t capacity)
{
    /* Dictionary SPR uses the native parser's syllabification and phone
     * aliases even when its bytes also pass the compact syntax validator.
     * Compile it once into the same complete lexical record as other data;
     * otherwise an apparently valid raw entry can lose syllables or create
     * a word shape that the acoustic planner cannot render. */
    int literal=value[0]=='`'&&value[1]=='[';
    if(literal) {
        size_t n=strlen(value);
        if(n>=capacity||n<3||value[n-1]!=']'||strchr(value+2,'['))return 0;
    }
    /* Control sequences and genuine multi-word expansions remain native
     * normalization entries. Spelling-only aliases can form a lexical word. */
    if(!literal)for(const char *p=value;*p;++p)if(*p=='`'&&strncmp(p,"`ts",3))return 0;
    struct provider_worker *compiler=context;
    char phonemes[8192];struct reloq61_request q={.language=compiler->language,
        .annotations=1,.phonemes_only=1,.voice={0,50,65,30,0,0,50,92},
        .text=value,.phonemes=phonemes,.phonemes_capacity=sizeof phonemes};
    struct reloq61_result result;void *previous=reloq61_worker_scope(compiler);
    int status=reloq61_generate(&q,&result);reloq61_worker_scope(previous);
    if(status||result.phonemes_length>=sizeof phonemes-1)return 0;
    size_t used=2;spr[0]='`';spr[1]='[';int inside=0,groups=0;
    for(const unsigned char *p=(const unsigned char *)phonemes;*p;++p) {
        if(*p=='['){inside=1;++groups;continue;}
        if(*p==']'){inside=0;continue;}
        if(inside&&*p>32){if(used+2>=capacity)return 0;spr[used++]=(char)*p;}
    }
    if(!groups||used==2)return 0;
    if(groups>1&&strncmp(value,"`ts1",4)&&strncmp(value,"`ts2",4))return 0;
    /* A lexical word has one primary accent. Native spelling/phrase aliases
     * can supply several independently stressed groups; when forming one
     * lexical record the remaining accents become secondary, as in the
     * ordinary German compound representation. */
    spr[used]=0;dictionary_single_primary(spr);
    spr[used++]=']';spr[used]=0;
    return reloq_german_lexicon_validate_spr((const unsigned char *)spr,used,NULL)==1;
}
static int dictionary_compile(struct provider_worker *w)
{
    /* These two language modules distinguish complete-word and root stages.
     * Other modules retain their original native-volume handling. */
    if(w->language!=0x40000)return 0;
    struct provider_worker *compiler=reloq61_worker_new(w->language);if(!compiler)return -1;
    int status=reloq61_dictionary_compile_words(w->dictionary,compiler,dictionary_compile_alias,w->language==0x40000);
    reloq61_worker_free(compiler);return status;
}
static int dictionary_bind(struct provider_worker *w,delta_state *d)
{
    const char *prefix=dictionary_prefix(w->language);
    if(!w->dictionary_path||!prefix)return 0;
    if(w->dictionary_checked&&!w->dictionary)return 0;
    if(!w->dictionary) {
        static const char *const names[]={"main.dic","Root.dic","abbr.dic"};
        void *set=es_engsynNewDict(d);if(!set)return -1;
        unsigned opened=0;
        for(int i=0;i<3;++i) {
            char name[32];snprintf(name,sizeof name,"%s%s",prefix,names[i]);errno=0;
            FILE *f=dictionary_open(w->dictionary_path,name);
            if(!f) {if(errno==ENOENT)continue;es_engsynDeleteDict(set);return -1;}
            ++opened;
            int status=fseek(f,0,SEEK_END);long bytes=status?-1:ftell(f);
            if(bytes>=0)status=fseek(f,0,SEEK_SET);
            if(bytes<0||status||reloq61_dictionary_read(set,i,f,(size_t)bytes)) {
                fclose(f);es_engsynDeleteDict(set);return -1;
            }
            fclose(f);
        }
        w->dictionary_checked=1;
        if(!opened){es_engsynDeleteDict(set);return 0;}
        w->dictionary=set;
        if(dictionary_compile(w))return -1;
        if(w->language==0x40000&&dictionary_index(w))return -1;
    }
    return reloq61_dictionary_attach(w->dictionary,d);
}
int reloq61_worker_dictionary(void *context,const char *path)
{
    struct provider_worker *w=context;
    if(!w||w->handle||w->dictionary_path)return -1;
    if(!path)return 0;
    size_t n=strlen(path);if(!n||n>=8192||!dictionary_directory(path))return -1;
    w->dictionary_path=malloc(n+1);if(!w->dictionary_path)return -1;memcpy(w->dictionary_path,path,n+1);
    if(w->english) {
        w->english->dictionary_path=malloc(n+1);if(!w->english->dictionary_path)return -1;
        memcpy(w->english->dictionary_path,path,n+1);
    }
    if(w->language==0x40000||w->language==0x10000) {
        char phonemes[512];struct reloq61_request q={.language=w->language,
            .annotations=1,.phonemes_only=1,.voice={0,50,65,30,0,0,50,92},
            .text="",.phonemes=phonemes,.phonemes_capacity=sizeof phonemes};
        struct reloq61_result result;void *previous=reloq61_worker_scope(w);
        int status=reloq61_generate(&q,&result);reloq61_worker_scope(previous);return status;
    }
    return 0;
}
size_t reloq61_dictionary_match(const unsigned char *text,size_t length)
{
    struct provider_worker *w=owner_worker;
    if(!w||w->language!=0x40000||!w->dictionary_key_count||!length)return 0;
    unsigned char first=text[0];
    if(!((first>='A'&&first<='Z')||(first>='a'&&first<='z')||
         (first>='0'&&first<='9')||(first>=0xc0&&first!=0xd7&&first!=0xf7)))return 0;
    size_t token=0;while(token<length&&token<255&&text[token]>32&&text[token]!='`')++token;
    if(!token)return 0;
    char folded[512];size_t count=dictionary_fold(text,token,folded);
    for(size_t begin=0;begin<count;++begin)if(w->dictionary_starts[(unsigned char)folded[begin]]) {
        for(size_t n=1;n<=count-begin&&n<256;++n)if(w->dictionary_lengths[n/64]&(UINT64_C(1)<<(n%64))) {
            char save=folded[begin+n];folded[begin+n]=0;const char *key=folded+begin;
            size_t low=0,high=w->dictionary_key_count;int found=0;
            while(low<high) {
                size_t middle=low+(high-low)/2;int order=strcmp(key,w->dictionary_keys[middle]);
                if(!order){found=1;break;}
                if(order<0)high=middle;else low=middle+1;
            }
            folded[begin+n]=save;if(found)return token;
        }
    }
    return 0;
}

extern const struct reloq_lexicon_resolver *reloq61_dictionary_resolver_data(void *);
const struct reloq_lexicon_resolver *reloq61_dictionary_resolver(void)
{
    struct provider_worker *w=owner_worker;
    return w&&w->language==0x40000&&w->dictionary?
        reloq61_dictionary_resolver_data(w->dictionary):NULL;
}
