/* Direct Pinglish phoneme rules, derived from eSpeak NG fa_rules (GPL-3.0+).
 * English ph/th exceptions are routed by the caller. The returned sequence
 * uses the private Finnish-based Persian phonemes and needs no eSpeak state. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "reloq_utf8.h"

static char lower(char c)
{ return c>='A' && c<='Z' ? (char)(c+('a'-'A')) : c; }

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
    size_t used=0,primary=nv>1?nv-2:0;
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
