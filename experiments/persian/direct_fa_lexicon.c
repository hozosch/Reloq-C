/* Direct C lookup of an experimental generated Persian lexicon.
 * No eSpeak process, library, environment variable or global mutable state
 * is used at runtime. The underlying data remains GPL-3.0-or-later.
 */
#include "direct_fa_lexicon.h"
#include <string.h>

struct fa_direct_entry { const char *word, *tag; };
#include "direct_fa_lexicon.inc"

size_t reloq_fa_direct_word_count(void)
{ return sizeof fa_direct_entries / sizeof fa_direct_entries[0]; }

int reloq_fa_direct_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    if(output && capacity)output[0]=0;
    if(!word || !bytes || !output || !capacity || memchr(word,0,bytes))return -1;
    size_t lo=0,hi=reloq_fa_direct_word_count();
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        const char *key=fa_direct_entries[m].word;
        size_t n=strlen(key), common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=m;
        else if(order>0)lo=m+1;
        else {
            const char *tag=fa_direct_entries[m].tag;
            n=strlen(tag);
            if(n>=capacity)return -2;
            memcpy(output,tag,n+1);
            return 1;
        }
    }
    return 0;
}
