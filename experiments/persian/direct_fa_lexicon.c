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

int reloq_fa_direct_context_word(const char *word,size_t bytes,int follows_word,
                                 char *output,size_t capacity)
{
    int result=reloq_fa_direct_word(word,bytes,output,capacity);
    if(result)return result;
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
    return 0;
}
