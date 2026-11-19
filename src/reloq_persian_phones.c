/* Persian phoneme identities and a lossless bridge to the approved legacy
 * acoustic provider. No text lookup, sound synthesis or global state lives
 * here. In particular, the provider's H carrier is not a Persian phoneme. */
#include "reloq_persian_phones.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define V RELOQ_FA_PHONE_VOWEL
#define C RELOQ_FA_PHONE_CONSONANT
static const struct reloq_fa_phone_info fa_phones[] = {
    {RELOQ_FA_PHONE_AE,V,"front-a","æ"},
    {RELOQ_FA_PHONE_BACK_A,V,"back-a","ɒː"},
    {RELOQ_FA_PHONE_E,V,"e","e"},
    {RELOQ_FA_PHONE_I,V,"i","iː"},
    {RELOQ_FA_PHONE_O,V,"o","o"},
    {RELOQ_FA_PHONE_U,V,"u","uː"},
    {RELOQ_FA_PHONE_P,C,"p","p"},
    {RELOQ_FA_PHONE_B,C,"b","b"},
    {RELOQ_FA_PHONE_T,C,"t","t"},
    {RELOQ_FA_PHONE_D,C,"d","d"},
    {RELOQ_FA_PHONE_K,C,"k","k"},
    {RELOQ_FA_PHONE_G,C,"g","ɡ"},
    {RELOQ_FA_PHONE_UVULAR,C,"uvular","ɢ~ɣ"},
    {RELOQ_FA_PHONE_GLOTTAL_STOP,C,"glottal-stop","ʔ"},
    {RELOQ_FA_PHONE_F,C,"f","f"},
    {RELOQ_FA_PHONE_V,C,"v","v"},
    {RELOQ_FA_PHONE_S,C,"s","s"},
    {RELOQ_FA_PHONE_Z,C,"z","z"},
    {RELOQ_FA_PHONE_SH,C,"sh","ʃ"},
    {RELOQ_FA_PHONE_ZH,C,"zh","ʒ"},
    {RELOQ_FA_PHONE_KH,C,"kh","x"},
    {RELOQ_FA_PHONE_H,C,"h","h"},
    {RELOQ_FA_PHONE_CH,C,"ch","tʃ"},
    {RELOQ_FA_PHONE_J,C,"j","dʒ"},
    {RELOQ_FA_PHONE_M,C,"m","m"},
    {RELOQ_FA_PHONE_N,C,"n","n"},
    {RELOQ_FA_PHONE_L,C,"l","l"},
    {RELOQ_FA_PHONE_R,C,"r","ɾ~r"},
    {RELOQ_FA_PHONE_Y,C,"y","j"}
};
#undef V
#undef C

/* Existing scoped provider symbols, indexed by Persian identity minus one.
 * Their spelling is deliberately private to this compatibility adapter. */
static const char *fa_symbols(void)
{
    static const char symbols[] = {
        'A','a','e','i','o','u','p','b','t','d','k','g','R','w',
        'f','v','s','z','S','Z','G','h','C','J','m','n','l','r','j'
    };
    return symbols;
}

_Static_assert(sizeof fa_phones/sizeof fa_phones[0] == RELOQ_FA_PHONE_COUNT,
               "Persian inventory and identifiers differ");

const struct reloq_fa_phone_info *reloq_fa_phone_inventory(size_t *count)
{
    if(count)*count=sizeof fa_phones/sizeof fa_phones[0];
    return fa_phones;
}

static uint32_t fa_id(char symbol)
{
    const char *map=fa_symbols();
    for(uint32_t i=0;i<RELOQ_FA_PHONE_COUNT;++i)
        if(map[i]==symbol)return i+1;
    return 0;
}
static int fa_vowel(uint32_t id)
{return id>=RELOQ_FA_PHONE_AE && id<=RELOQ_FA_PHONE_U;}

/* Run twice: the first pass validates the complete group, the second writes
 * a caller-sized buffer. No output can be partially modified by bad input. */
static int fa_parse(const char *s,size_t bytes,struct reloq_fa_phone *out,
                    size_t *count,size_t *error)
{
    size_t n=0,at=2;uint32_t syllable=0,previous=0;
    if(bytes<5){*error=0;return -1;}
    size_t end=bytes-1;
    if(s[0]!='`' || s[1]!='[' || s[end]!=']') {*error=0;return -1;}
    while(at<end) {
        if(s[at]!='.' || at+1>=end || s[at+1]<'0' || s[at+1]>'2') {*error=at;return -1;}
        uint32_t stress=(uint32_t)(s[at+1]-'0');at+=2;
        size_t begin=n;unsigned vowels=0;
        while(at<end && s[at]!='.') {
            size_t start=at;uint32_t flags=0,id;
            if(s[at]=='\'') {
                if(end-at<4 || s[at+2]!=':' || s[at+3]!='\'') {*error=at;return -1;}
                id=fa_id(s[at+1]);
                if(!fa_vowel(id)){*error=at;return -1;}
                flags=RELOQ_FA_PHONE_LONG;at+=4;
            } else {
                id=fa_id(s[at]);
                if(!id){*error=at;return 1;}
                ++at;
                if(id==RELOQ_FA_PHONE_H && at<end && s[at]=='w') {
                    if(at+1!=end){*error=start;return 1;}
                    ++at; /* A private final-H carrier, not a second phone. */
                } else if(id==RELOQ_FA_PHONE_H && at==end) {
                    /* A bare final H cannot retain the approved connected
                       H impulse when this group is re-serialized. */
                    *error=start;return 1;
                }
            }
            if(id==RELOQ_FA_PHONE_GLOTTAL_STOP && previous==RELOQ_FA_PHONE_H) {*error=start;return 1;}
            if(fa_vowel(id))++vowels;
            if(out)out[n]=(struct reloq_fa_phone){id,flags,syllable,stress};
            ++n;previous=id;
        }
        if(n==begin || vowels!=1){*error=at;return -1;}
        ++syllable;
    }
    if(!n){*error=at;return -1;}
    *count=n;return 0;
}

int reloq_fa_phones_from_legacy(const char *text,size_t bytes,
    struct reloq_fa_phone *phones,size_t capacity,size_t *count,size_t *error_offset)
{
    if(count)*count=0;
    if(error_offset)*error_offset=0;
    if(!text || !count || (!phones && capacity) || bytes>8192 || memchr(text,0,bytes))return -1;
    size_t needed=0,error=0;
    int result=fa_parse(text,bytes,NULL,&needed,&error);
    if(result){if(error_offset)*error_offset=error;return result;}
    *count=needed;
    if(!phones)return 0;
    if(capacity<needed)return -2;
    return fa_parse(text,bytes,phones,&needed,&error);
}

/* Validate and size the output before writing. Explicit length applies only
 * to vowels and every syllable must contain exactly one vowel. */
static int fa_validate(const struct reloq_fa_phone *phones,size_t count,size_t *needed,size_t *error)
{
    if(!phones || !count || count>8192){*error=0;return -1;}
    size_t length=3;uint32_t syllable=0,stress=0;unsigned vowels=0;
    for(size_t i=0;i<count;++i) {
        const struct reloq_fa_phone *p=phones+i;
        if(!p->id || p->id>RELOQ_FA_PHONE_COUNT || p->stress>2 ||
           (p->flags & ~RELOQ_FA_PHONE_LONG) || ((p->flags & RELOQ_FA_PHONE_LONG) && !fa_vowel(p->id)))
            {*error=i;return -1;}
        if(!i || p->syllable!=syllable) {
            if((!i && p->syllable!=0) || (i && (p->syllable!=syllable+1 || vowels!=1)))
                {*error=i;return -1;}
            syllable=p->syllable;stress=p->stress;vowels=0;length+=2;
        } else if(p->stress!=stress) {*error=i;return -1;}
        if(i && phones[i-1].id==RELOQ_FA_PHONE_H && p->id==RELOQ_FA_PHONE_GLOTTAL_STOP)
            {*error=i;return 1;}
        if(fa_vowel(p->id))++vowels;
        if(vowels>1){*error=i;return -1;}
        length+=(p->flags & RELOQ_FA_PHONE_LONG)?4:1;
    }
    if(vowels!=1){*error=count-1;return -1;}
    if(phones[count-1].id==RELOQ_FA_PHONE_H)++length;
    if(length>8192){*error=count-1;return -1;}
    *needed=length;return 0;
}

int reloq_fa_phones_to_legacy(const struct reloq_fa_phone *phones,size_t count,
    char *output,size_t capacity,size_t *bytes,size_t *error_index)
{
    if(bytes)*bytes=0;
    if(error_index)*error_index=0;
    if(!bytes || (!output && capacity))return -1;
    size_t needed=0,error=0;
    int result=fa_validate(phones,count,&needed,&error);
    if(result){if(error_index)*error_index=error;return result;}
    *bytes=needed;
    if(!output)return 0;
    if(capacity<=needed)return -2;
    const char *map=fa_symbols();size_t at=0;uint32_t syllable=UINT32_MAX;
    output[at++]='`';output[at++]='[';
    for(size_t i=0;i<count;++i) {
        const struct reloq_fa_phone *p=phones+i;
        if(p->syllable!=syllable){output[at++]='.';output[at++]=(char)('0'+p->stress);syllable=p->syllable;}
        char symbol=map[p->id-1];
        if(p->flags&RELOQ_FA_PHONE_LONG){output[at++]='\'';output[at++]=symbol;output[at++]=':';output[at++]='\'';}
        else output[at++]=symbol;
    }
    if(phones[count-1].id==RELOQ_FA_PHONE_H)output[at++]='w';
    output[at++]=']';output[at]=0;
    return at==needed?0:-1;
}
