/* Direct C lookup of an experimental generated Persian lexicon.
 * No eSpeak process, library, environment variable or global mutable state
 * is used at runtime. The underlying data remains GPL-3.0-or-later.
 */
#include "direct_fa_lexicon.h"
#include <string.h>

struct fa_direct_entry { const char *word, *tag; };
struct fa_context_entry { const char *word, *final, *nonfinal; };
#include "direct_fa_lexicon.inc"
#include "direct_fa_context.inc"
#include "direct_fa_plural.inc"
#include "direct_fa_suffixes.inc"
#include "direct_fa_extended_suffixes.inc"
#include "direct_fa_pause_words.inc"
#include "direct_fa_recovered_words.inc"

size_t reloq_fa_direct_word_count(void)
{ return sizeof fa_direct_entries / sizeof fa_direct_entries[0]; }

static size_t direct_index(const char *word,size_t bytes)
{
    size_t lo=0,hi=reloq_fa_direct_word_count();
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        const char *key=fa_direct_entries[m].word;
        size_t n=strlen(key), common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=m;
        else if(order>0)lo=m+1;
        else return m;
    }
    return reloq_fa_direct_word_count();
}

int reloq_fa_direct_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    if(output && capacity)output[0]=0;
    if(!word || !bytes || !output || !capacity || memchr(word,0,bytes))return -1;
    size_t index=direct_index(word,bytes);
    if(index==reloq_fa_direct_word_count())return 0;
    const char *tag=fa_direct_entries[index].tag;
    size_t n=strlen(tag);
    if(n>=capacity)return -2;
    memcpy(output,tag,n+1);
    return 1;
}

/* This transform is enabled only by the generated per-stem admission bits.
 * It mirrors direct_fa_suffix_rules.candidate, preserving every native phone
 * and length marker while assigning the reference's syllable stresses. */
static int direct_suffix_tag(const char *base,unsigned kind,char *output,size_t capacity)
{
    size_t n=strlen(base);
    if(n<5 || base[0]!='`' || base[1]!='[' || base[n-1]!=']' || kind>=6)return -1;
    char last=base[n-2];
    if(kind==2 && strchr("aAeEiou",last)) {
        if(n+1>=capacity)return -2;
        memcpy(output,base,n-1);output[n-1]='m';output[n]=']';output[n+1]=0;
        return 1;
    }
    int coda=kind>=2;
    if(coda && !strchr("bdfghjklmnprstvzSZGRCJ",last))return -1;
    size_t copy=n-1-(size_t)coda;
    const char *tail=kind==0?".0ha]":kind==1?".0tAr]":
                     kind==2?"Am]":kind==3?"At]":kind==4?"AS]":"i]";
    size_t tail_bytes=strlen(tail),extra=coda?3:0;
    if(copy>capacity-1 || extra+tail_bytes>capacity-1-copy)return -2;
    memcpy(output,base,copy);
    unsigned syllables=0;
    for(size_t i=2;i<copy;++i)
        if(output[i]=='.' && i+1<copy && output[i+1]>='0' && output[i+1]<='2')++syllables;
    if(!syllables)return -1;
    unsigned position=0;
    for(size_t i=2;i<copy;++i) {
        if(output[i]!='.' || i+1>=copy || output[i+1]<'0' || output[i+1]>'2')continue;
        ++position;
        output[i+1]=position==syllables?'1':position==1 && syllables>2?'2':'0';
    }
    if(coda){output[copy++]='.';output[copy++]='0';output[copy++]=last;}
    memcpy(output+copy,tail,tail_bytes+1);
    return 1;
}

static int direct_suffix_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    static const struct {const char *text;size_t bytes;} suffixes[]={
        {"\xd9\x87\xd8\xa7",4}, /* ها */
        {"\xd8\xaa\xd8\xb1",4}, /* تر */
        {"\xd9\x85",2}, {"\xd8\xaa",2}, {"\xd8\xb4",2}, {"\xdb\x8c",2},
    };
    for(unsigned kind=0;kind<sizeof suffixes/sizeof suffixes[0];++kind) {
        size_t n=suffixes[kind].bytes;
        if(bytes<=n || memcmp(word+bytes-n,suffixes[kind].text,n))continue;
        size_t index=direct_index(word,bytes-n);
        if(index>=reloq_fa_direct_word_count() || index/8>=sizeof fa_direct_suffix_bases[0] ||
           !(fa_direct_suffix_bases[kind][index/8] & (1u<<(index%8))))continue;
        return direct_suffix_tag(fa_direct_entries[index].tag,kind,output,capacity);
    }
    return 0;
}

static int direct_extended_tag(const char *base,unsigned kind,unsigned mode,char *output,size_t capacity)
{
    static const char *const tails[13][3]={
        {"tA","rin",NULL},{"ha","'i:'",NULL},{"ha","jAm",NULL},
        {"ha","jAt",NULL},{"ha","jAS",NULL},{"ha","je","man"},
        {"ha","je","tan"},{"ha","je","San"},{"haS",NULL,NULL},
        {"ha","Sun",NULL},{NULL,"man",NULL},{NULL,"tan",NULL},{NULL,"San",NULL}
    };
    if(kind>=13 || mode>1)return -1;
    size_t n=strlen(base);
    if(n<5 || base[0]!='`' || base[1]!='[' || base[n-1]!=']')return -1;
    size_t copy=n-1,stem=0,primary=0;char coda[3]={0,'e',0};
    if(kind>=10) {
        char last=base[n-2];
        if(!strchr("bdfghjklmnprstvzSZGRCJ",last) || n<6)return -1;
        coda[0]=last;--copy;
    }
    if(copy>=capacity)return -2;
    memcpy(output,base,copy);
    for(size_t i=2;i<copy;++i) {
        if(output[i]=='.' && i+1<copy && output[i+1]>='0' && output[i+1]<='2') {
            if(output[i+1]=='1')primary=stem;
            ++stem;
        }
    }
    if(!stem)return -1;
    size_t tail_count=0;
    while(tail_count<3 && (tails[kind][tail_count] || (kind>=10 && !tail_count)))++tail_count;
    size_t total=stem+tail_count;
    if(mode==1)primary=total>1?total-2:0;
    size_t position=0;
    if(mode==1) {
        for(size_t i=2;i<copy;++i) {
            if(output[i]!='.' || i+1>=copy || output[i+1]<'0' || output[i+1]>'2')continue;
            output[i+1]=position==primary?'1':position==0 && primary>=2?'2':'0';
            ++position;
        }
    }
    for(size_t i=0;i<tail_count;++i) {
        const char *body=(kind>=10 && !i)?coda:tails[kind][i];
        size_t length=strlen(body),index=stem+i;
        char stress;
        if(mode==1)stress=index==primary?'1':index==0 && primary>=2?'2':'0';
        else stress=index>=primary+2 && (index-primary)%2==0?'2':'0';
        if(copy>capacity-1 || length+2>capacity-1-copy)return -2;
        output[copy++]='.';output[copy++]=stress;
        memcpy(output+copy,body,length);copy+=length;
    }
    if(copy+1>=capacity)return -2;
    output[copy++]=']';output[copy]=0;
    return 1;
}

static int direct_extended_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    static const struct {const char *text;size_t bytes;} suffixes[]={
        {"\xd8\xaa\xd8\xb1\xdb\x8c\xd9\x86",8}, /* ترین */
        {"\xd9\x87\xd8\xa7\xdb\x8c\xdb\x8c",8}, /* هایی */
        {"\xd9\x87\xd8\xa7\xdb\x8c\xd9\x85",8}, /* هایم */
        {"\xd9\x87\xd8\xa7\xdb\x8c\xd8\xaa",8}, /* هایت */
        {"\xd9\x87\xd8\xa7\xdb\x8c\xd8\xb4",8}, /* هایش */
        {"\xd9\x87\xd8\xa7\xdb\x8c\xd9\x85\xd8\xa7\xd9\x86",12},
        {"\xd9\x87\xd8\xa7\xdb\x8c\xd8\xaa\xd8\xa7\xd9\x86",12},
        {"\xd9\x87\xd8\xa7\xdb\x8c\xd8\xb4\xd8\xa7\xd9\x86",12},
        {"\xd9\x87\xd8\xa7\xd8\xb4",6},
        {"\xd9\x87\xd8\xa7\xd8\xb4\xd9\x88\xd9\x86",10},
        {"\xd9\x85\xd8\xa7\xd9\x86",6}, {"\xd8\xaa\xd8\xa7\xd9\x86",6},
        {"\xd8\xb4\xd8\xa7\xd9\x86",6}
    };
    for(unsigned kind=0;kind<sizeof suffixes/sizeof suffixes[0];++kind) {
        size_t n=suffixes[kind].bytes;
        if(bytes<=n || memcmp(word+bytes-n,suffixes[kind].text,n))continue;
        size_t index=direct_index(word,bytes-n);
        if(index>=reloq_fa_direct_word_count() || index/8>=sizeof fa_extended_suffix_bases[0][0])continue;
        for(unsigned mode=0;mode<2;++mode)
            if(fa_extended_suffix_bases[kind][mode][index/8] & (1u<<(index%8)))
                return direct_extended_tag(fa_direct_entries[index].tag,kind,mode,output,capacity);
    }
    return 0;
}

int reloq_fa_direct_context_word(const char *word,size_t bytes,int follows_word,
                                 char *output,size_t capacity)
{
    int result=reloq_fa_direct_word(word,bytes,output,capacity);
    if(result)return result;
    size_t pause_lo=0,pause_hi=sizeof fa_pause_entries / sizeof fa_pause_entries[0];
    while(pause_lo<pause_hi) {
        size_t m=pause_lo+(pause_hi-pause_lo)/2;
        const char *key=fa_pause_entries[m].word;
        size_t n=strlen(key),common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)pause_hi=m;
        else if(order>0)pause_lo=m+1;
        else {
            const char *tag=fa_pause_entries[m].tag;
            n=strlen(tag);
            if(n>=capacity)return -2;
            memcpy(output,tag,n+1);
            return 1;
        }
    }
    size_t lo=0,hi=sizeof fa_context_entries / sizeof fa_context_entries[0];
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        const char *key=fa_context_entries[m].word;
        size_t n=strlen(key), common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=m;
        else if(order>0)lo=m+1;
        else {
            const char *tag=follows_word?fa_context_entries[m].nonfinal:fa_context_entries[m].final;
            n=strlen(tag);
            if(n>=capacity)return -2;
            memcpy(output,tag,n+1);
            return 1;
        }
    }
    /* A half-space plus ها is a productive suffix only for bases verified
     * in four reference contexts. Never guess for an unadmitted base. */
    static const char suffix[]="\xe2\x80\x8c\xd9\x87\xd8\xa7";
    size_t suffix_bytes=sizeof suffix-1;
    if(bytes>suffix_bytes && !memcmp(word+bytes-suffix_bytes,suffix,suffix_bytes)) {
        size_t index=direct_index(word,bytes-suffix_bytes);
        if(index<reloq_fa_direct_word_count() && index/8<sizeof fa_plural_zwnj_bases &&
           (fa_plural_zwnj_bases[index/8] & (1u<<(index%8)))) {
            const char *base=fa_direct_entries[index].tag;
            size_t n=strlen(base);
            const char *ending=follows_word?".0ha]":".1ha]";
            size_t ending_bytes=strlen(ending);
            if(!n || base[n-1]!=']')return -1;
            if(n-1>capacity-1 || ending_bytes>capacity-n)return -2;
            memcpy(output,base,n-1);
            memcpy(output+n-1,ending,ending_bytes+1);
            return 1;
        }
    }
    result=direct_suffix_word(word,bytes,output,capacity);
    if(result)return result;
    result=direct_extended_word(word,bytes,output,capacity);
    if(result)return result;
    lo=0;hi=sizeof fa_recovered_entries / sizeof fa_recovered_entries[0];
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        const char *key=fa_recovered_entries[m].word;
        size_t n=strlen(key),common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=m;
        else if(order>0)lo=m+1;
        else {
            const char *tag=follows_word?fa_recovered_entries[m].nonfinal:fa_recovered_entries[m].final;
            n=strlen(tag);
            if(n>=capacity)return -2;
            memcpy(output,tag,n+1);
            return 1;
        }
    }
    return 0;
}
