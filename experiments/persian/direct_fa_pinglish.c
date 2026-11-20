/* Pinglish spelling recognition and its shared Persian orthography path.
 * English ph/th exceptions are routed by the caller. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "direct_fa_text.h"
#include "reloq_utf8.h"

struct fa_pinglish_entry {const char *roman,*persian;};
#include "direct_fa_pinglish_lexicon.inc"
#include "direct_fa_pinglish_extra.inc"

static char lower(char c)
{ return c>='A' && c<='Z' ? (char)(c+('a'-'A')) : c; }

static const char *find_roman(const struct fa_pinglish_entry *entries,size_t count,const char *key)
{
    size_t lo=0,hi=count;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        int order=strcmp(key,entries[mid].roman);
        if(order<0)hi=mid;
        else if(order>0)lo=mid+1;
        else return entries[mid].persian;
    }
    return NULL;
}

static const char *persian_for_roman(const char *key)
{
    const char *word=find_roman(fa_pinglish_entries,
        sizeof fa_pinglish_entries/sizeof fa_pinglish_entries[0],key);
    return word?word:find_roman(fa_pinglish_extra,
        sizeof fa_pinglish_extra/sizeof fa_pinglish_extra[0],key);
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

struct pinglish_unit {const char *letter;char vowel;};

static int append_text(char *out,size_t capacity,size_t *used,const char *s)
{
    size_t n=strlen(s);
    if(n>=capacity-*used)return -2;
    memcpy(out+*used,s,n);*used+=n;out[*used]=0;return 0;
}

/* Represent each unambiguous Latin sound by a Persian letter, adding short
 * vowel and sukun marks where a bare Persian spelling would be ambiguous.
 * This produces text for the same Persian word analyzer as native script. */
static int spelled_persian(const char *word,size_t bytes,char *output,size_t capacity)
{
    struct pinglish_unit units[1024];size_t count=0,vowels=0;
    for(size_t i=0;i<bytes;) {
        char c=lower(word[i]),n=i+1<bytes?lower(word[i+1]):0;
        char vowel=0;size_t advance=1;
        if(c=='a' && n=='a'){vowel='a';advance=2;}
        else if(c=='e' && n=='e'){vowel='i';advance=2;}
        else if(c=='o' && (n=='o'||n=='u')){vowel='u';advance=2;}
        else if(c=='a')vowel='A';
        else if(c=='e')vowel='e';
        else if(c=='i')vowel='i';
        else if(c=='o')vowel='o';
        else if(c=='u')vowel='u';
        if(vowel) {
            if(!count || units[count-1].vowel) {
                if(count==sizeof units/sizeof units[0])return 0;
                units[count++]=(struct pinglish_unit){"ا",0};
            }
            units[count-1].vowel=vowel;++vowels;i+=advance;continue;
        }
        const char *letter=NULL,*extra=NULL;
        if(c=='c' && n=='h'){letter="چ";advance=2;}
        else if(c=='g' && n=='h'){letter="غ";advance=2;}
        else if(c=='j' && n=='h'){letter="ژ";advance=2;}
        else if(c=='k' && n=='h'){letter="خ";advance=2;}
        else if(c=='s' && n=='h'){letter="ش";advance=2;}
        else switch(c) {
        case 'b':letter="ب";break;case 'c':letter=(n=='e'||n=='i'||n=='y')?"س":"ک";break;
        case 'd':letter="د";break;case 'f':letter="ف";break;case 'g':letter="گ";break;
        case 'h':letter="ه";break;case 'j':letter="ج";break;case 'k':letter="ک";break;
        case 'l':letter="ل";break;case 'm':letter="م";break;case 'n':letter="ن";break;
        case 'p':letter="پ";break;case 'q':letter="ق";break;case 'r':letter="ر";break;
        case 's':letter="س";break;case 't':letter="ت";break;case 'v':case 'w':letter="و";break;
        case 'x':letter="ک";extra="س";break;case 'y':letter="ی";break;case 'z':letter="ز";break;
        default:return 0;
        }
        if(count+(extra?2u:1u)>sizeof units/sizeof units[0])return 0;
        units[count++]=(struct pinglish_unit){letter,0};
        if(extra)units[count++]=(struct pinglish_unit){extra,0};
        i+=advance;
    }
    if(!vowels)return 0;
    size_t used=0;
    for(size_t i=0;i<count;++i) {
        if(append_text(output,capacity,&used,units[i].letter))return -2;
        const char *mark=units[i].vowel=='A'?"َ":units[i].vowel=='e'?"ِ":
            units[i].vowel=='o'?"ُ":units[i].vowel=='a'?"َا":
            units[i].vowel=='i'?"ِی":units[i].vowel=='u'?"ُو":"ْ";
        if(append_text(output,capacity,&used,mark))return -2;
    }
    return 1;
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
    char spelling[4096];
    int spelled=spelled_persian(word,bytes,spelling,sizeof spelling);
    return spelled==1 ? persian_pinglish_word(spelling,output,capacity) : spelled;
}
