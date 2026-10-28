/* Original-observed lexical context and native cross-word allophones. */
#include "reloq_english_context87.inc"
static int context_word86(const char *word,const char *set)
{
    size_t n=strlen(word);if(!n)return 0;
    for(const char *p=set;*p;) {const char *q=strchr(p,' ');size_t k=q?(size_t)(q-p):strlen(p);
        if(k==n&&!memcmp(word,p,n))return 1;
        if(!q)break;
        p=q+1;
    }
    return 0;
}
static void context_neighbor86(const unsigned char *text,size_t length,size_t at,int right,char out[64])
{
    size_t a=at,b=at;out[0]=0;
    if(right){while(a<length&&(text[a]==' '||text[a]=='\t'))++a;b=a;while(b<length&&wordchar(text[b]))++b;}
    else {while(b&&(text[b-1]==' '||text[b-1]=='\t'))--b;a=b;while(a&&wordchar(text[a-1]))--a;}
    if(b<=a||b-a>=64)return;
    for(size_t i=a;i<b;++i)out[i-a]=(char)tolower(text[i]);
    out[b-a]=0;
}
/* Conservative regular-participle licensing. The pronunciation supplement
 * contains experimental letter runs such as "spe"; these are not verb stems. */
static int regular_participle100(const char *word)
{
    size_t n=strlen(word);
    if(n<4||n>=64||memcmp(word+n-2,"ed",2))return 0;
    unsigned char root[65];memcpy(root,word,n-2);size_t k=n-2;
    const char *s=NULL,*v=NULL,*c=NULL;
    if(!reloq_english_primary_word(root,k,&s,&v,&c))return 1;
    root[k]='e';
    if(k>=3&&!reloq_english_primary_word(root,k+1,&s,&v,&c))return 1;
    if(k>2&&root[k-1]==root[k-2]&&strchr("bdgklmnprt",root[k-1])&&
       !reloq_english_primary_word(root,k-1,&s,&v,&c))return 1;
    if(k&&root[k-1]=='i') {
        root[k-1]='y';
        if(!reloq_english_primary_word(root,k,&s,&v,&c))return 1;
    }
    return 0;
}
static const char *context_reading86(const unsigned char *text,size_t length,size_t begin,size_t end)
{
    if(end-begin>=64)return NULL;
    char key[64],left[64],right[64],previous[64];
    for(size_t i=begin;i<end;++i)key[i-begin]=(char)tolower(text[i]);
    key[end-begin]=0;
    context_neighbor86(text,length,begin,0,left);context_neighbor86(text,length,end,1,right);
    if(!*left&&!*right)return NULL;
    /* A lexical article retains its weak vowel at a sentence start even
     * when the case-sensitive dictionary also contains the letter A. */
    if(!strcmp(key,"a")&&*right &&
       !(text[begin]=='A'&&context_word86(right,"and or")))return "`00 `[.0x]";
    /* Perfect-auxiliary reduction is separate from lexical possession.
     * Regular participles must have an independently known stem; the
     * spelling -ed alone must not license hundred, indeed or speed. */
    if(!strcmp(key,"had")) {
        int participle=context_word86(right,"been seen left taken given gone done made found told known shown written spoken broken chosen driven eaten fallen forgotten grown hidden lost met paid put run sat sent slept sold spent stood taught thought understood won worn");
        if(regular_participle100(right))participle=1;
        if(participle)return "`00 `[.0hxd]";
    }
    if(!strcmp(key,"that")&&
       context_word86(left,"said says say knew know knows think thinks thought believe believes believed hope hopes hoped heard hear hears noticed notice notices explained explain explains")&&
       context_word86(right,"a an the i you he she it we they my your his her its our their this these those"))
        return "`00 `[.0Dxt]";
    if(!strcmp(key,"used")&&!strcmp(right,"to"))return "`[.1yust]";
    size_t left_start=begin;while(left_start&&(text[left_start-1]==' '||text[left_start-1]=='\t'))--left_start;
    while(left_start&&wordchar(text[left_start-1]))--left_start;
    context_neighbor86(text,length,left_start,0,previous);
    size_t lo=0,hi=sizeof english_contexts86/sizeof *english_contexts86;
    const struct english_context86 *entry=NULL;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;int c=strcmp(key,english_contexts86[mid].word);
        if(c<0)hi=mid;else if(c>0)lo=mid+1;else{entry=english_contexts86+mid;break;}}
    if(!entry)return NULL;
    if(!strcmp(key,"read")&&*right&&(!*left||!strcmp(left,"please"))&&entry->modal&&*entry->modal)
        return entry->modal;
    const char *s=NULL;
    const char *be="am is are was were be been being";
    const char *modal="can could may might must shall should will would";
    if(!*right) {
        s=context_word86(left,be)?entry->after_be:entry->final;
        if(!strcmp(key,"it")&&context_word86(left,strong_object_governors86))s=entry->after_be;
        if(s&&!strncmp(s,"`0 ",3)&&context_word86(left,"to for from in on at with by of about after before over under through into"))s=entry->after_be;
    }
    if(!s||!*s) {
        if(context_word86(previous,modal)&&context_word86(left,"i you he she we they her"))s=entry->modal;
        else if(context_word86(left,"have has had"))s=entry->perfect;
        else if(context_word86(left,modal)||context_word86(left,"to"))s=entry->modal;
        else if(context_word86(left,be))s=entry->predicate;
        else if(context_word86(left,"the a an this that these those my your his her its our their"))s=entry->noun;
        else if(context_word86(left,"i you he she we they please"))s=entry->verb;
        else if(context_word86(right,be))s=entry->before_be;
        else if(context_word86(right,modal))s=entry->before_modal;
    }
    return s&&*s?s:NULL;
}
/* The weak form of "to" is a phonological join, not the allophone of a
 * fixed dictionary carrier. SO contrasts in behavior-0.2.89 distinguish
 * the following constriction and a preceding stressed open syllable. */
static const char *context_to89(const unsigned char *previous,size_t used,
    const char *following,const unsigned char *word,size_t length)
{
    char key[64];
    if(length<sizeof key) {
        for(size_t i=0;i<length;++i)key[i]=(char)tolower(word[i]);
        key[length]=0;
        if(context_word86(key,"then yesterday under"))return "`0 `[.1tu]";
    }
    const char *first=strstr(following,"`[");
    if(!first)return NULL;
    unsigned following_stress=0;
    first+=2;while(first[0]=='.' && first[1]) {
        following_stress=(unsigned)(first[1]-'0');first+=2;
    }
    /* An unstressed vowel onset keeps /u/: to another, to object. */
    int full_u=!following_stress && phone_vowel((unsigned char)*first) &&
        !(length==2&&tolower(word[0])=='i'&&tolower(word[1])=='t');
    int high=*first && strchr("dtFkgzsZSJCnGy",*first)!=NULL;
    int flap=0;
    size_t end=used;
    while(end && (previous[end-1]==' '||previous[end-1]=='\t'))--end;
    if(end && previous[end-1]==']') {
        size_t begin=end-1;
        while(begin && previous[begin]!='[')--begin;
        unsigned stress=0;
        for(size_t i=begin;i+1<end;++i)
            if(previous[i]=='.' && previous[i+1]>='0' && previous[i+1]<='2')
                stress=previous[i+1]-'0';
        flap=stress>0 && end>1 && strchr("iIeEAXHxauUoc",previous[end-2])!=NULL;
    }
    if(full_u)return flap?"`00 `[.0Fu]":"`00 `[.0tu]";
    return flap?(high?"`00 `[.0Fx]":"`00 `[.0FX]"):
        (high?"`00 `[.0tx]":"`00 `[.0tX]");
}
