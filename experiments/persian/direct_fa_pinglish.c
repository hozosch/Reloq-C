/* Direct Pinglish phoneme rules, derived from eSpeak NG fa_rules (GPL-3.0+).
 * English ph/th exceptions are routed by the caller. The returned sequence
 * uses the private Finnish-based Persian phonemes and needs no eSpeak state. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "direct_fa_text.h"
#include "reloq_utf8.h"

struct fa_pinglish_entry {const char *roman,*persian;};
#include "direct_fa_pinglish_lexicon.inc"

static char lower(char c)
{ return c>='A' && c<='Z' ? (char)(c+('a'-'A')) : c; }

static const char *persian_for_roman(const char *key)
{
    size_t lo=0,hi=sizeof fa_pinglish_entries/sizeof fa_pinglish_entries[0];
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        int order=strcmp(key,fa_pinglish_entries[mid].roman);
        if(order<0)hi=mid;
        else if(order>0)lo=mid+1;
        else return fa_pinglish_entries[mid].persian;
    }
    return NULL;
}

/* Use the complete Persian word pipeline: it also recognizes productive
 * compounds which are not direct_context_word entries on their own. */
static int persian_pinglish_word(const char *word,char *output,size_t capacity)
{
    char *tag=NULL;
    size_t *map=NULL,error=0;
    int status=reloq_fa_direct_text(word,strlen(word),&tag,&map,&error);
    if(status)return status==-2?-2:0;
    size_t bytes=strlen(tag);
    if(bytes>=capacity)status=-2;
    else {memcpy(output,tag,bytes+1);status=1;}
    reloq_fa_direct_free(tag);reloq_fa_direct_free(map);
    return status;
}

/* Normalize common long-vowel spellings only for the verified dictionary
 * lookup. If it misses, the existing grapheme rules receive the original
 * spelling. This avoids interpreting an unresolved word as a known one. */
static void pinglish_lookup_key(const char *word,size_t bytes,char *key)
{
    size_t out=0;
    for(size_t i=0;i<bytes;++i) {
        char c=lower(word[i]),n=i+1<bytes?lower(word[i+1]):0;
        if(c=='a' && n=='a')++i;
        else if(c=='e' && n=='e'){c='i';++i;}
        else if(c=='o' && (n=='o'||n=='u')){c='u';++i;}
        else if(c=='j' && n=='h'){key[out++]='z';c='h';++i;}
        key[out++]=c;
    }
    key[out]=0;
}

static int persian_dictionary_pinglish(const char *key,char *output,size_t capacity)
{
    const char *word=persian_for_roman(key);
    if(word) {
        int status=persian_pinglish_word(word,output,capacity);
        if(status!=0)return status;
    }
    /* A final silent H is a common Roman spelling of Persian final -e.
     * Prefer an exact entry above, and strip it only after a known -e word. */
    size_t length=strlen(key);
    if(length>3 && key[length-2]=='e' && key[length-1]=='h') {
        char without_h[1025];
        memcpy(without_h,key,length-1);without_h[length-1]=0;
        word=persian_for_roman(without_h);
        if(word) {
            int status=persian_pinglish_word(word,output,capacity);
            if(status!=0)return status;
        }
    }
    /* Productive suffixes reuse the same per-stem admissions and stress
     * policy as Persian script. Do not guess an unrecognized base. */
    static const struct {const char *roman,*persian;} endings[]={
        {"ash","ش"},{"tar","تر"},{"am","م"},{"at","ت"},
        {"ha","ها"},{"ye","ی"},{"an","ان"}
    };
    for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        size_t suffix=strlen(endings[i].roman);
        if(length<=suffix+2 || memcmp(key+length-suffix,endings[i].roman,suffix))continue;
        char base[1025],formed[2048];
        size_t n=length-suffix;
        memcpy(base,key,n);base[n]=0;
        word=persian_for_roman(base);
        if(!word)continue;
        n=strlen(word);
        size_t extra=strlen(endings[i].persian);
        if(n+extra>=sizeof formed)continue;
        memcpy(formed,word,n);memcpy(formed+n,endings[i].persian,extra+1);
        int status=persian_pinglish_word(formed,output,capacity);
        if(status!=0)return status;
    }
    return 0;
}

static int put(char *output,size_t capacity,size_t *used,char c)
{
    if(*used+1>=capacity)return -2;
    output[(*used)++]=c;output[*used]=0;return 0;
}

/* Fold common Latin letters without needing a platform-specific Unicode
 * service. The same words can be analyzed directly by native clients. */
static const char *latin_fold(uint32_t cp)
{
    static const char *const latin1[]={
        "a","a","a","a","a","a","ae","c","e","e","e","e","i","i","i","i",
        "d","n","o","o","o","o","o",NULL,"o","u","u","u","u","y","t","ss",
        "a","a","a","a","a","a","ae","c","e","e","e","e","i","i","i","i",
        "d","n","o","o","o","o","o",NULL,"o","u","u","u","u","y","t","y"
    };
    if(cp>=0xc0 && cp<=0xff)return latin1[cp-0xc0];
    switch(cp) {
    case 0x100:case 0x101:return "a";
    case 0x10c:case 0x10d:return "ch";
    case 0x131:return "i";
    case 0x141:case 0x142:return "l";
    case 0x152:case 0x153:return "oe";
    case 0x160:case 0x161:return "sh";
    case 0x17d:case 0x17e:return "jh";
    case 0x1e9e:return "ss";
    default:return NULL;
    }
}

/* Return 1 for a candidate, 0 if another text path is needed, -2 for a
 * small buffer, and -1 for malformed input. The bounded stack arrays also
 * keep extremely long input from blocking a speech worker. */
int reloq_fa_pinglish_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    if(output && capacity)output[0]=0;
    if(!word || !output || !capacity || !bytes || memchr(word,0,bytes))return -1;
    if(bytes<2 || bytes>512)return 0;
    char normalized[1025];size_t normalized_len=0;
    for(size_t at=0;at<bytes;) {
        uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)word,bytes,&at,&cp)!=1)return -1;
        if(cp>=0x300 && cp<=0x36f && normalized_len)continue;
        char ascii[2]={(char)cp,0};
        const char *fold=cp<128?ascii:latin_fold(cp);
        if(!fold)return 0;
        for(;*fold;++fold) {
            if(normalized_len>=sizeof normalized-1)return 0;
            normalized[normalized_len++]=*fold;
        }
    }
    normalized[normalized_len]=0;word=normalized;bytes=normalized_len;
    if(bytes<2 || bytes>512)return 0;
    for(size_t i=0;i<bytes;++i) {
        char c=lower(word[i]),n=i+1<bytes?lower(word[i+1]):0;
        char n2=i+2<bytes?lower(word[i+2]):0;
        if(c<'a' || c>'z' || ((c=='p'||c=='t') && n=='h') ||
           (c=='o' && n=='o' && n2=='o'))return 0;
    }
    char key[1025];
    pinglish_lookup_key(word,bytes,key);
    int known=persian_dictionary_pinglish(key,output,capacity);
    if(known!=0)return known;
    char phones[1024],glides[1024]={0};size_t count=0;
    for(size_t i=0;i<bytes;) {
        char c=lower(word[i]),n=i+1<bytes?lower(word[i+1]):0;
        char n2=i+2<bytes?lower(word[i+2]):0;
        if(c<'a' || c>'z')return 0;
        if((c=='p'||c=='t') && n=='h')return 0;
        if(c=='o' && n=='o' && n2=='o')return 0;
        char p=0;size_t advance=1;
        if(c=='a' && n=='a'){p='a';advance=2;}
        else if(c=='c' && n=='h'){p='C';advance=2;}
        else if(c=='e' && n=='e'){p='i';advance=2;}
        else if(c=='g' && n=='h'){p='R';advance=2;}
        else if(c=='j' && n=='h'){p='Z';advance=2;}
        else if(c=='k' && n=='h'){p='G';advance=2;}
        else if(c=='o' && (n=='o'||n=='u')){p='u';advance=2;}
        else if(c=='s' && n=='h'){p='S';advance=2;}
        else if(c=='c')p=(n=='e'||n=='i'||n=='y')?'s':'k';
        else if(c=='a')p='A';
        else if(c=='j')p='J';
        else if(c=='q')p='R';
        else if(c=='y')p='j';
        else if(c=='w')p='v'; /* Closest available Persian approximant. */
        else if(c=='x') {
            if(count+2>sizeof phones)return 0;
            phones[count++]='k';phones[count++]='s';i+=advance;continue;
        } else p=c;
        if(count>=sizeof phones)return 0;
        phones[count]=p;glides[count]=(c=='w');++count;i+=advance;
    }
    size_t vowels[512],starts[513],nv=0;
    for(size_t i=0;i<count;++i) {
        if(strchr("aAeiou",phones[i])) {
            if(nv>=sizeof vowels/sizeof vowels[0])return 0;
            vowels[nv++]=i;
        }
    }
    if(!nv)return 0;
    starts[0]=0;
    for(size_t i=1;i<nv;++i) {
        size_t between=vowels[i]-vowels[i-1]-1;
        size_t onset=between?1:0;
        if(between>=2 && (phones[vowels[i]-1]=='j' || glides[vowels[i]-1]))onset=2;
        starts[i]=vowels[i]-onset;
    }
    starts[nv]=count;
    /* If no lexical or morphological entry is known, use Persian's usual
     * final word accent. Known weak words, suffixes and exceptional stress
     * have already gone through the shared Persian word pipeline above. */
    size_t used=0,primary=nv-1;
    if(put(output,capacity,&used,'`') || put(output,capacity,&used,'['))return -2;
    for(size_t i=0;i<nv;++i) {
        char stress=i==primary?'1':(i+1<primary && !(i%2))?'2':'0';
        if(put(output,capacity,&used,'.') || put(output,capacity,&used,stress))return -2;
        for(size_t j=starts[i];j<starts[i+1];++j) {
            if(put(output,capacity,&used,phones[j]))return -2;
            if(phones[j]=='h' && j+1==count && put(output,capacity,&used,'w'))return -2;
        }
    }
    return put(output,capacity,&used,']')?-2:1;
}
