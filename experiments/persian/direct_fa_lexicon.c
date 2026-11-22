/* Direct C lookup of an experimental generated Persian lexicon.
 * No eSpeak process, library, environment variable or global mutable state
 * is used at runtime. Generated tables retain their individual licenses.
 */
#include "direct_fa_lexicon.h"
#include "direct_fa_letters.h"
#include "reloq_utf8.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct fa_direct_entry { const char *word, *tag; };
struct fa_context_entry { const char *word, *final, *nonfinal; };
struct fa_compound_extra { const char *word, *tag; unsigned char mask; };
struct fa_stress_class { uint16_t bank,offset; unsigned char flags; };
#include "direct_fa_lexicon.inc"
#include "direct_fa_context.inc"
#include "direct_fa_plural.inc"
#include "direct_fa_suffixes.inc"
#include "direct_fa_extended_suffixes.inc"
#include "direct_fa_pause_words.inc"
#include "direct_fa_recovered_words.inc"
#include "direct_fa_vocalized.inc"
#include "direct_fa_nvda_words.inc"
#include "direct_fa_compounds.inc"
#include "direct_fa_stress_classes.inc"
#include "direct_fa_adjective_final_y.inc"
#include "direct_fa_city_stems.inc"

_Static_assert(sizeof fa_stress_adjective_final_y ==
               (sizeof fa_stress_classes / sizeof fa_stress_classes[0] + 7u) / 8u,
               "adjective stress bits must match the class table");
#ifndef RELOQ_FA_EXCLUDE_FREQUENCY_WORDS
#include "direct_fa_frequency_words.inc"
#include "direct_fa_frequency_compounds.inc"
_Static_assert(sizeof fa_frequency_compound_masks ==
               sizeof fa_frequency_entries / sizeof fa_frequency_entries[0],
               "Persian compound admissions must match the frequency table");
#endif

_Static_assert(sizeof fa_compound_main_masks == sizeof fa_direct_entries / sizeof fa_direct_entries[0],
               "Persian compound admissions must match the direct lexicon");

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

#ifndef RELOQ_FA_EXCLUDE_FREQUENCY_WORDS
static size_t direct_frequency_index(const char *word,size_t bytes)
{
    size_t lo=0,hi=sizeof fa_frequency_entries / sizeof fa_frequency_entries[0];
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        const char *key=fa_frequency_entries[m].word;
        size_t n=strlen(key),common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=m;
        else if(order>0)lo=m+1;
        else return m;
    }
    return sizeof fa_frequency_entries / sizeof fa_frequency_entries[0];
}
#endif

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

/* Independent Persian orthographic rule for an explicitly vowelled CVC
 * syllable. Persian normally omits short vowels, so unmarked text must not
 * enter this rule. Exclude ambiguous vowel carriers, silent final heh,
 * shadda, extra consonants and additional marks rather than guessing them.
 * The phone symbols here belong to the existing Persian acoustic bridge. */
static char explicit_consonant(uint32_t cp)
{
    switch(cp) {
    case 0x0628:return 'b'; case 0x067e:return 'p';
    case 0x062a:case 0x0637:return 't';
    case 0x062b:case 0x0633:case 0x0635:return 's';
    case 0x062c:return 'J'; case 0x0686:return 'C';
    case 0x062d:return 'h'; case 0x062e:return 'G';
    case 0x062f:return 'd';
    case 0x0630:case 0x0632:case 0x0636:case 0x0638:return 'z';
    case 0x0631:return 'r'; case 0x0698:return 'Z'; case 0x0634:return 'S';
    case 0x063a:case 0x0642:return 'R'; case 0x0641:return 'f';
    case 0x0643:case 0x06a9:return 'k'; case 0x06af:return 'g';
    case 0x0644:return 'l'; case 0x0645:return 'm'; case 0x0646:return 'n';
    default:return 0;
    }
}

static uint32_t explicit_arabic_scalar(const unsigned char *input,size_t bytes,size_t *at)
{
    if(*at+1>=bytes || input[*at]<0xd8 || input[*at]>0xdb ||
       (input[*at+1]&0xc0)!=0x80)return 0;
    uint32_t cp=((uint32_t)(input[*at]&0x1f)<<6) | (input[*at+1]&0x3f);
    *at+=2;return cp;
}

static int explicit_vowelled_cvc(const char *word,size_t bytes,char *output,size_t capacity,
                                 char bare[5])
{
    if(!word || !bytes || !output || !capacity || !bare)return -1;
    const unsigned char *text=(const unsigned char *)word;
    size_t at=0;
    char onset=explicit_consonant(explicit_arabic_scalar(text,bytes,&at));
    if(!onset)return 0;
    uint32_t vowel=explicit_arabic_scalar(text,bytes,&at);
    char nucleus=vowel==0x064e?'A':vowel==0x0650?'e':vowel==0x064f?'o':0;
    if(!nucleus)return 0;
    size_t coda_at=at;
    char coda=explicit_consonant(explicit_arabic_scalar(text,bytes,&at));
    if(!coda)return 0;
    if(at<bytes && explicit_arabic_scalar(text,bytes,&at)!=0x0652)return 0;
    if(at!=bytes)return 0;
    const char tag[]={'`','[','.','1',onset,nucleus,coda,']',0};
    if(capacity<sizeof tag)return -2;
    memcpy(output,tag,sizeof tag);
    memcpy(bare,word,2);memcpy(bare+2,word+coda_at,2);bare[4]=0;
    return 1;
}

/* Read fully marked multi-syllable words without a pronunciation table.
 * An unmarked internal consonant could conceal a short vowel, so accept it
 * only when it has sukun. The final consonants need no written sukun. Long
 * vowel carriers, silent heh, shadda and other ambiguous marks stay on the
 * existing conservative path. The short-vowel marks themselves select the
 * nucleus and the default citation accent is on the final syllable. */
static int explicit_vowelled_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    struct unit { char consonant,vowel; unsigned char sukun; } units[128];
    if(!word || !bytes || !output || !capacity)return -1;
    const unsigned char *input=(const unsigned char *)word;
    size_t at=0,count=0,syllables=0;
    while(at<bytes) {
        if(count==sizeof units/sizeof units[0])return 0;
        char consonant=explicit_consonant(explicit_arabic_scalar(input,bytes,&at));
        if(!consonant)return 0;
        struct unit u={consonant,0,0};
        if(at<bytes) {
            size_t next=at;
            uint32_t mark=explicit_arabic_scalar(input,bytes,&next);
            if(mark==0x064e || mark==0x0650 || mark==0x064f) {
                u.vowel=mark==0x064e?'A':mark==0x0650?'e':'o';
                ++syllables;at=next;
            } else if(mark==0x0652) {u.sukun=1;at=next;}
        }
        units[count++]=u;
    }
    if(syllables<2 || !units[0].vowel)return 0;
    size_t coda=0;
    for(size_t i=0;i<count;++i) {
        if(units[i].vowel){coda=0;continue;}
        if(++coda>2)return 0;
        if(!units[i].sukun)
            for(size_t j=i+1;j<count;++j)if(units[j].vowel)return 0;
    }
    if(count>SIZE_MAX-4-3*syllables)return -2;
    size_t needed=4+count+3*syllables;
    if(needed>capacity)return -2;
    size_t out=0,nucleus=0;output[out++]='`';output[out++]='[';
    for(size_t i=0;i<count;++i) {
        if(units[i].vowel) {
            output[out++]='.';output[out++]=++nucleus==syllables?'1':'0';
            output[out++]=units[i].consonant;output[out++]=units[i].vowel;
        } else output[out++]=units[i].consonant;
    }
    output[out++]=']';output[out]=0;
    return 1;
}

/* Decode completely indicated Persian orthography, including long-vowel
 * carriers and the consonantal و/ی/ه. The same rule is used for written
 * Persian and for the explicitly marked spelling made by Pinglish. Reject an
 * unmarked internal consonant that could hide an unknown short vowel. */
static int fully_vowelled_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    struct unit {char consonant,vowel;unsigned char sukun;} units[256];
    const unsigned char *input=(const unsigned char *)word;
    size_t at=0,count=0,vowels=0;int explicit_marks=0;
    if(!word || !bytes || !output || !capacity)return -1;
    while(at<bytes) {
        if(count==sizeof units/sizeof units[0])return 0;
        uint32_t cp=explicit_arabic_scalar(input,bytes,&at);
        struct unit u={explicit_consonant(cp),0,0};
        if(cp==0x0647)u.consonant='h';
        else if(cp==0x0648)u.consonant='v';
        else if(cp==0x06cc || cp==0x064a)u.consonant='j';
        else if(cp==0x0627)u.consonant=0;
        else if(cp==0x0622) {u.consonant=0;u.vowel='a';explicit_marks=1;}
        else if(!u.consonant)return 0;
        if(at<bytes && !u.vowel) {
            size_t next=at;uint32_t mark=explicit_arabic_scalar(input,bytes,&next);
            if(mark==0x064e || mark==0x064f || mark==0x0650) {
                u.vowel=mark==0x064e?'A':mark==0x064f?'o':'e';
                explicit_marks=1;at=next;
                if(at<bytes) {
                    size_t carrier_end=at;uint32_t carrier=explicit_arabic_scalar(input,bytes,&carrier_end);
                    size_t after=carrier_end;uint32_t following=after<bytes?
                        explicit_arabic_scalar(input,bytes,&after):0;
                    int bare=following!=0x064e && following!=0x064f && following!=0x0650 &&
                             following!=0x0651 && following!=0x0652;
                    if(bare && ((mark==0x064e && carrier==0x0627) ||
                                (mark==0x064f && carrier==0x0648) ||
                                (mark==0x0650 && (carrier==0x06cc || carrier==0x064a)))) {
                        u.vowel=mark==0x064e?'a':mark==0x064f?'u':'i';at=carrier_end;
                    }
                }
            } else if(mark==0x0652) {u.sukun=1;explicit_marks=1;at=next;}
            else if(u.consonant && (mark==0x0627 || mark==0x0648 || mark==0x06cc || mark==0x064a)) {
                size_t after=next;uint32_t following=after<bytes?
                    explicit_arabic_scalar(input,bytes,&after):0;
                if(following!=0x064e && following!=0x064f && following!=0x0650 &&
                   following!=0x0651 && following!=0x0652) {
                    u.vowel=mark==0x0627?'a':mark==0x0648?'u':'i';at=next;
                }
            }
        }
        if(!u.consonant && !u.vowel)return 0;
        if(u.vowel)++vowels;
        units[count++]=u;
    }
    if(!explicit_marks || !vowels)return 0;
    /* Bare final heh often denotes the vowel /e/ in Persian. Only
     * decode it as a pronounced /h/ when its consonant status is marked. */
    if(count && units[count-1].consonant=='h' && !units[count-1].vowel &&
       !units[count-1].sukun)return 0;
    size_t unmarked_coda=0;
    for(size_t i=0;i<count;++i)if(!units[i].vowel && !units[i].sukun) {
        for(size_t j=i+1;j<count;++j)if(units[j].vowel)return 0;
        if(++unmarked_coda>1)return 0;
    }
    if(count>(SIZE_MAX-4-3*vowels)/2)return -2;
    if(4+2*count+3*vowels>capacity)return -2;
    size_t used=0,nucleus=0;output[used++]='`';output[used++]='[';
    output[used++]='.';output[used++]=vowels==1?'1':'0';
    for(size_t i=0;i<count;++i) {
        if(units[i].vowel && nucleus) {
            output[used++]='.';output[used++]=nucleus+1==vowels?'1':'0';
        }
        if(units[i].consonant)output[used++]=units[i].consonant;
        if(units[i].vowel){output[used++]=units[i].vowel;++nucleus;}
    }
    if(count && units[count-1].consonant=='h' && !units[count-1].vowel)output[used++]='w';
    output[used++]=']';output[used]=0;return 1;
}

static int guessed_joined_word(const char *word,size_t bytes,char *output,size_t capacity);

/* The two Unicode spellings of a final heh with written ezafe are
 * canonically equivalent. Keep the base intact so known pronunciations and
 * their stress can be reused; never turn an isolated combining mark into a
 * suffix. The bound is the same as the orthographic word fallback. */
static size_t heh_ezafe_stem(const char *word,size_t bytes,char *stem,size_t capacity)
{
    static const char decomposed[]="هٔ",composed[]="ۀ",heh[]="ه";
    size_t n=0;
    if(bytes>sizeof decomposed-1 &&
       !memcmp(word+bytes-(sizeof decomposed-1),decomposed,sizeof decomposed-1)) {
        n=bytes-(sizeof "ٔ"-1);
        if(n>=capacity)return 0;
        memcpy(stem,word,n);
    } else if(bytes>sizeof composed-1 &&
              !memcmp(word+bytes-(sizeof composed-1),composed,sizeof composed-1)) {
        n=bytes-(sizeof composed-1);
        if(capacity<sizeof heh || n>capacity-sizeof heh)return 0;
        memcpy(stem,word,n);memcpy(stem+n,heh,sizeof heh-1);n+=sizeof heh-1;
    } else return 0;
    stem[n]=0;return n;
}

static int append_heh_ezafe(const char *base,char *output,size_t capacity)
{
    size_t n=strlen(base),copy;
    if(n<5 || base[0]!='`' || base[1]!='[' || base[n-1]!=']')return 0;
    if(n>=5 && !memcmp(base+n-5,"'e:']",5))copy=n-5;
    else if(base[n-2]=='e')copy=n-2;
    else return 0;
    /* Final /e/ belongs to the stem, followed by unstressed /ye/. */
    if(copy>capacity || sizeof "e.0je]">capacity-copy)return -2;
    memcpy(output,base,copy);memcpy(output+copy,"e.0je]",sizeof "e.0je]");
    return 1;
}

/* Orthographic fallback for a previously unseen word. Persian short vowels
 * are usually omitted, so this is an approximation, never a dictionary
 * override. The general consonant+short-a pattern follows fa_rules' basic
 * consonant analysis; explicit marks, long-vowel carriers and word-final
 * heh take precedence. A permissible final consonant cluster is retained.
 * This routine is shared by standalone hosts and does not invoke eSpeak. */
int reloq_fa_direct_guess_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    struct unit {char consonant,vowel;unsigned char sukun,geminate,explicit_vowel;} units[512];
    size_t at=0,count=0;
    if(!word || !bytes || !output || !capacity || memchr(word,0,bytes))return -1;
    int joined=guessed_joined_word(word,bytes,output,capacity);
    if(joined)return joined;
    char ezafe_stem[1025],ezafe_base[2048];
    size_t ezafe_bytes=heh_ezafe_stem(word,bytes,ezafe_stem,sizeof ezafe_stem);
    if(ezafe_bytes) {
        int found=reloq_fa_direct_guess_word(ezafe_stem,ezafe_bytes,ezafe_base,sizeof ezafe_base);
        if(found!=1)return found;
        return append_heh_ezafe(ezafe_base,output,capacity);
    }
    while(at<bytes) {
        size_t next=at;uint32_t cp=0;
        if(reloq_utf8_next((const unsigned char *)word,bytes,&next,&cp)!=1)return -1;
        at=next;
        if(cp==0x200c || cp==0x200d || cp==0x0640)continue;
        if(cp==0x064e || cp==0x064f || cp==0x0650 || cp==0x0651 || cp==0x0652) {
            if(!count)return 0;
            struct unit *u=&units[count-1];
            if(cp==0x0651)u->geminate=1;
            else if(cp==0x0652){if(u->explicit_vowel)return 0;u->sukun=1;u->vowel=0;}
            else {
                if(u->sukun || u->explicit_vowel)return 0;
                u->vowel=cp==0x064e?'A':cp==0x064f?'o':'e';u->explicit_vowel=1;
            }
            continue;
        }
        char consonant=explicit_consonant(cp),vowel=0;
        if(cp==0x0622)vowel='a';
        else if(cp==0x0627) {
            if(!count)vowel='A';
            else if(!units[count-1].vowel && !units[count-1].sukun)
                {units[count-1].vowel='a';continue;}
            else vowel='a';
        } else if(cp==0x0648 || cp==0x06cc || cp==0x064a) {
            char sound=cp==0x0648?'u':'i';
            size_t look=at;uint32_t following=0;
            if(look<bytes && reloq_utf8_next((const unsigned char *)word,bytes,&look,&following)!=1)return -1;
            /* Before another written vowel, و is consonantal. The common
             * productive -وند ending likewise starts with /v/, not /u/. */
            int consonantal_waw=cp==0x0648 &&
                (following==0x0627 || following==0x0622 || following==0x06cc || following==0x064a ||
                 (bytes-at==sizeof "ند"-1 && !memcmp(word+at,"ند",sizeof "ند"-1)));
            if(!consonantal_waw && count && !units[count-1].vowel && !units[count-1].sukun)
                {units[count-1].vowel=sound;continue;}
            consonant=cp==0x0648?'v':'j';
        } else if(cp==0x0647 || cp==0x06c0) {
            if(at==bytes && count && !units[count-1].vowel && !units[count-1].sukun)
                {units[count-1].vowel='e';continue;}
            consonant='h';
        } else if(cp==0x0621 || cp==0x0623 || cp==0x0625 || cp==0x0639)consonant='w';
        else if(cp==0x0626)consonant='j';
        else if(cp==0x0624)consonant='v';
        else if(cp==0x0629)consonant='t';
        if(!consonant && !vowel)return 0;
        if(count==sizeof units/sizeof units[0])return 0;
        units[count++]=(struct unit){consonant,vowel,0,0,0};
    }
    if(!count)return 0;
    size_t last=SIZE_MAX,nuclei=0;
    for(size_t i=0;i<count;++i) {
        if(units[i].vowel){last=i;++nuclei;continue;}
        if(units[i].sukun || !units[i].consonant)continue;
        if(i+1<count && (last==SIZE_MAX || (i-last>=2 && !units[i+1].vowel))) {
            units[i].vowel='A';last=i;++nuclei;
        }
    }
    if(!nuclei) {
        size_t i=0;while(i<count && (units[i].sukun || !units[i].consonant))++i;
        if(i==count)return 0;
        units[i].vowel='A';nuclei=1;
    }
    /* At most two phones per unit and a syllable prefix per nucleus. */
    if(count>(SIZE_MAX-5-3*nuclei)/3 || 5+3*count+3*nuclei>capacity)return -2;
    size_t used=0,nucleus=0;
    output[used++]='`';output[used++]='[';output[used++]='.';
    output[used++]=nuclei==1?'1':'0';
    for(size_t i=0;i<count;++i) {
        struct unit u=units[i];
        if(u.vowel && nucleus) {
            output[used++]='.';output[used++]=nucleus+1==nuclei?'1':'0';
        }
        if(u.consonant){output[used++]=u.consonant;if(u.geminate)output[used++]=u.consonant;}
        if(u.vowel){output[used++]=u.vowel;++nucleus;}
    }
    /* The acoustic bridge needs its silent carrier after a final /h/; the
     * owned phone plan removes that carrier, keeping the actual H impulse. */
    if(units[count-1].consonant=='h' && !units[count-1].vowel)output[used++]='w';
    output[used++]=']';output[used]=0;return 1;
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

/* Produce only reference-verified ezafe forms. The bitmaps are tied to the
 * base lexicon fingerprint; uncommon results are stored as exact entries. */
static int direct_vocalized_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    size_t lo=0,hi=sizeof fa_vocalized_entries/sizeof fa_vocalized_entries[0];
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        const char *key=fa_vocalized_entries[m].word;
        size_t n=strlen(key),common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=m;
        else if(order>0)lo=m+1;
        else {
            const char *tag=fa_vocalized_entries[m].tag;
            n=strlen(tag);if(n>=capacity)return -2;
            memcpy(output,tag,n+1);return 1;
        }
    }
    static const char kasra[]="\xd9\x90";
    if(bytes<=2 || memcmp(word+bytes-2,kasra,2))return 0;
    size_t index=direct_index(word,bytes-2);
    if(index>=reloq_fa_direct_word_count() || index/8>=sizeof fa_vocalized_modes[0])return 0;
    unsigned bit=1u<<(index%8),mode;
    if(fa_vocalized_modes[0][index/8]&bit)mode=0;
    else if(fa_vocalized_modes[1][index/8]&bit)mode=1;
    else return 0;
    const char *base=fa_direct_entries[index].tag;
    size_t n=strlen(base);
    if(n<5 || base[0]!='`' || base[1]!='[' || base[n-1]!=']')return -1;
    char last=base[n-2];
    int coda=strchr("bdfghjklmnprstvzSZGRCJ",last)!=NULL;
    size_t copy=n-1-(size_t)coda;
    if(copy+6>capacity)return -2;
    memcpy(output,base,copy);
    unsigned syllables=0,primary=0;
    for(size_t i=2;i<copy;++i) {
        if(output[i]=='.' && i+1<copy && output[i+1]>='0' && output[i+1]<='2') {
            if(output[i+1]=='1')primary=syllables;
            ++syllables;
        }
    }
    if(!syllables)return -1;
    if(mode) {
        primary=syllables-1;
        unsigned position=0;
        for(size_t i=2;i<copy;++i) {
            if(output[i]!='.' || i+1>=copy || output[i+1]<'0' || output[i+1]>'2')continue;
            output[i+1]=position==primary?'1':position==0 && primary>=2?'2':'0';
            ++position;
        }
    }
    output[copy++]='.';
    output[copy++]=!mode && syllables>=primary+2 && (syllables-primary)%2==0?'2':'0';
    if(coda)output[copy++]=last;
    output[copy++]='e';output[copy++]=']';output[copy]=0;
    return 1;
}

/* Productive, reference-admitted compounds. Existing full-word and suffix
 * rules take precedence; no unverified stem is guessed. */
static int direct_compound_word(const char *word,size_t bytes,int follows_word,
                                char *output,size_t capacity)
{
    static const struct {const char *suffix;size_t bytes;unsigned kind;} endings[]={
        {"\xe2\x80\x8c\xda\xaf\xd8\xb1",7,1},
        {"\xe2\x80\x8c\xd9\x86\xd9\x85\xd8\xa7\xdb\x8c\xd8\xb4",13,2},
        {"\xe2\x80\x8c\xda\xaf\xd8\xb0\xd8\xa7\xd8\xb1\xdb\x8c",13,3},
        {"\xe2\x80\x8c\xd8\xb3\xd8\xa7\xd8\xb2\xdb\x8c",11,4},
        {"\xda\xaf\xd8\xb1",4,0},
    };
    for(size_t k=0;k<sizeof endings/sizeof endings[0];++k) {
        size_t suffix_bytes=endings[k].bytes;
        if(bytes<=suffix_bytes || memcmp(word+bytes-suffix_bytes,endings[k].suffix,suffix_bytes))continue;
        size_t stem_bytes=bytes-suffix_bytes,index=direct_index(word,stem_bytes);
        unsigned mask=0;const char *base=NULL;
        if(index<reloq_fa_direct_word_count()) {
            mask=fa_compound_main_masks[index];base=fa_direct_entries[index].tag;
        } else {
            size_t lo=0,hi=sizeof fa_compound_extra_stems/sizeof fa_compound_extra_stems[0];
            while(lo<hi) {
                size_t mid=lo+(hi-lo)/2,n=strlen(fa_compound_extra_stems[mid].word);
                size_t common=stem_bytes<n?stem_bytes:n;
                int order=memcmp(word,fa_compound_extra_stems[mid].word,common);
                if(!order)order=stem_bytes<n?-1:stem_bytes>n?1:0;
                if(order<0)hi=mid;
                else if(order>0)lo=mid+1;
                else {mask=fa_compound_extra_stems[mid].mask;base=fa_compound_extra_stems[mid].tag;break;}
            }
        }
#ifndef RELOQ_FA_EXCLUDE_FREQUENCY_WORDS
        if(!base) {
            size_t frequency=direct_frequency_index(word,stem_bytes);
            if(frequency<sizeof fa_frequency_entries / sizeof fa_frequency_entries[0]) {
                mask=fa_frequency_compound_masks[frequency];
                base=fa_frequency_entries[frequency].final;
            }
        }
#endif
        unsigned kind=endings[k].kind;
        if(!base || !(mask & (1u<<kind)))continue;
        static const char *const tails[]={".0gAr]",NULL,".0nA.1ma.0jeS]",
                                          ".0go.1za.0ri]",".1sa.0zi]"};
        const char *tail=kind==1?(follows_word?".0gAr]":".1gAr]"):tails[kind];
        size_t n=strlen(base),end=strlen(tail);
        if(n<4 || base[0]!='`' || base[1]!='[' || base[n-1]!=']')return -1;
        if(n-1>=capacity || end>=capacity-(n-1))return -2;
        memcpy(output,base,n-1);
        if(kind==0) {
            size_t syllables=0;
            for(size_t i=2;i+1<n-1;++i)
                if(output[i]=='.' && output[i+1]>='0' && output[i+1]<='2')++syllables;
            if(!syllables)return -1;
            size_t position=0;
            for(size_t i=2;i+1<n-1;++i) {
                if(output[i]!='.' || output[i+1]<'0' || output[i+1]>'2')continue;
                output[i+1]=position+1==syllables?'1':position==0 && syllables>2?'2':'0';
                ++position;
            }
        }
        memcpy(output+n-1,tail,end+1);
        return 1;
    }
    return 0;
}

static int direct_context_word_exact(const char *word,size_t bytes,int follows_word,
                                     char *output,size_t capacity)
{
    if(output && capacity)output[0]=0;
    if(!word || !bytes || !output || !capacity || memchr(word,0,bytes))return -1;
    /* These isolated vowel-mark names were reported as different when NVDA
     * supplied the written label instead of the combining character. They
     * are complete labels here, never substitutions inside a longer word. */
    static const struct fa_direct_entry vowel_names[]={
        {"اَ", "`[.1A]"}, {"اُ", "`[.1o]"}, {"اِ", "`[.1e]"},
        {"اَن", "`[.1An]"}, {"اُن", "`[.1on]"}, {"اِن", "`[.1en]"},
    };
    for(size_t i=0;i<sizeof vowel_names/sizeof vowel_names[0];++i) {
        if(strlen(vowel_names[i].word)!=bytes || memcmp(vowel_names[i].word,word,bytes))continue;
        size_t n=strlen(vowel_names[i].tag);
        if(n>=capacity)return -2;
        memcpy(output,vowel_names[i].tag,n+1);return 1;
    }
    /* Reported surname, absent from the direct lexicon. The root is the
     * same /kasi:r/ found in the existing کثیر compounds; the final -یان
     * is the separately syllabified family-name ending. */
    if(bytes==sizeof "کثیریان"-1 && !memcmp(word,"کثیریان",bytes)) {
        static const char reading[]="`[.0kA.0si.0ri.1jan]";
        if(sizeof reading>capacity)return -2;
        memcpy(output,reading,sizeof reading);return 1;
    }
    /* Frequent NVDA labels missing from the direct lexicon. Their vowels
     * and explicit ezafe forms are specified here rather than requesting a
     * reference analysis on every first encounter. */
    static const struct fa_direct_entry native_nvda_words[]={
        {"عنوان", "`[.0on.1van]"}, {"عنوانِ", "`[.0on.1va.0ne]"},
        {"عمودی", "`[.0A.0mu.1di]"}, {"عمودیِ", "`[.0A.0mu.1di.0je]"},
        {"عمل", "`[.0A.1mAl]"}, {"عملِ", "`[.0A.1mA.0le]"},
        {"عملی", "`[.0A.0mA.1li]"},
    };
    for(size_t i=0;i<sizeof native_nvda_words/sizeof native_nvda_words[0];++i) {
        if(strlen(native_nvda_words[i].word)!=bytes || memcmp(native_nvda_words[i].word,word,bytes))continue;
        size_t n=strlen(native_nvda_words[i].tag);
        if(n>=capacity)return -2;
        memcpy(output,native_nvda_words[i].tag,n+1);return 1;
    }
    int result=reloq_fa_direct_word(word,bytes,output,capacity);
    if(result)return result;
    /* Common independent forms of ʿalāve use the same verified reading in
       all four reference contexts. Keep them direct so "به علاوه" need not
       initialize the reference engine on first use. */
    const char *alave=NULL;
    if(bytes==sizeof "علاوه"-1 && !memcmp(word,"علاوه",bytes))
        alave="`[.0A.1la.0v'e:']";
    else if(bytes==sizeof "علاوه‌ی"-1 && !memcmp(word,"علاوه‌ی",bytes))
        alave="`[.0A.1la.0v'e:'.1j'e:']";
    if(alave) {
        size_t n=strlen(alave);
        if(n>=capacity)return -2;
        memcpy(output,alave,n+1);
        return 1;
    }
    result=direct_vocalized_word(word,bytes,output,capacity);
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
    lo=0;hi=sizeof fa_nvda_entries / sizeof fa_nvda_entries[0];
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        const char *key=fa_nvda_entries[m].word;
        size_t n=strlen(key),common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=m;
        else if(order>0)lo=m+1;
        else {
            const char *tag=follows_word?fa_nvda_entries[m].nonfinal:fa_nvda_entries[m].final;
            n=strlen(tag);
            if(n>=capacity)return -2;
            memcpy(output,tag,n+1);
            return 1;
        }
    }
    result=direct_compound_word(word,bytes,follows_word,output,capacity);
    if(result)return result;
    /* Reference-checked independent form used in the common NVDA state
       علامت‌دار. Keep it available to the native compound predictor. */
    static const char dar[]="\xd8\xaf\xd8\xa7\xd8\xb1";
    if(bytes==sizeof dar-1 && !memcmp(word,dar,sizeof dar-1)) {
        static const char tag[]="`[.1dar]";
        if(sizeof tag>capacity)return -2;
        memcpy(output,tag,sizeof tag);return 1;
    }
#ifndef RELOQ_FA_EXCLUDE_FREQUENCY_WORDS
    /* Keep the previously verified exact forms and productive rules above
       authoritative. This independently checked inventory only handles a
       word that would otherwise need the reference engine. */
    size_t frequency=direct_frequency_index(word,bytes);
    if(frequency<sizeof fa_frequency_entries / sizeof fa_frequency_entries[0]) {
        const char *tag=follows_word?fa_frequency_entries[frequency].nonfinal:fa_frequency_entries[frequency].final;
        size_t n=strlen(tag);
        if(n>=capacity)return -2;
        memcpy(output,tag,n+1);
        return 1;
    }
#endif
    return 0;
}

/* Lilak is used only to recognize word classes, never as a pronunciation
 * source. Multiple classes are retained so homographs can be left alone. */
static size_t stress_class_index(const char *word,size_t bytes)
{
    const size_t count=sizeof fa_stress_classes/sizeof fa_stress_classes[0];
    size_t lo=0,hi=count;
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        const char *key=fa_stress_banks[fa_stress_classes[m].bank]+fa_stress_classes[m].offset;
        size_t n=strlen(key),common=bytes<n?bytes:n;
        int order=memcmp(word,key,common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=m;
        else if(order>0)lo=m+1;
        else return m;
    }
    return count;
}

static unsigned stress_class(const char *word,size_t bytes)
{
    size_t index=stress_class_index(word,bytes);
    return index<sizeof fa_stress_classes/sizeof fa_stress_classes[0]
        ? fa_stress_classes[index].flags : 0;
}

unsigned reloq_fa_direct_word_classes(const char *word,size_t bytes)
{
    if(!word || !bytes || memchr(word,0,bytes))return 0;
    return stress_class(word,bytes);
}

static int stress_adjective_final_y(const char *word,size_t bytes)
{
    size_t index=stress_class_index(word,bytes);
    return index<sizeof fa_stress_classes/sizeof fa_stress_classes[0] &&
           (fa_stress_adjective_final_y[index/8] & (1u<<(index%8)))!=0;
}

int reloq_fa_direct_is_verb(const char *word,size_t bytes)
{
    return word && bytes && !memchr(word,0,bytes) && (stress_class(word,bytes)&2u)!=0;
}

/* Only change a single generated phone tag. A tilde or another tag may
 * denote a separately analyzed word and must not be flattened. */
static size_t stress_syllables(const char *tag)
{
    size_t n=strlen(tag),count=0;
    if(n<4 || tag[0]!='`' || tag[1]!='[' || tag[n-1]!=']')return 0;
    for(size_t i=2;i+1<n;++i) {
        if(tag[i]==']' || tag[i]=='[' || tag[i]=='`' || tag[i]=='~')return 0;
        if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2')++count;
    }
    return count;
}

static void stress_primary(char *tag,size_t target)
{
    size_t n=stress_syllables(tag),at=0;
    if(!n || target>=n)return;
    for(size_t i=2;tag[i] && tag[i]!=']';++i) {
        if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2') {
            tag[i+1]=at==target?'1':'0';
            ++at;
        }
    }
}

static void stress_unaccent(char *tag)
{
    if(!stress_syllables(tag))return;
    for(size_t i=2;tag[i] && tag[i]!=']';++i)
        if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2')tag[i+1]='0';
}

/* In connected speech a written final heh representing /e/ is short.
 * A citation form can retain the provider's longer final vowel. */
static int shorten_written_final_e(const char *word,size_t bytes,char *tag)
{
    static const char heh[]="ه";
    size_t n=strlen(tag);
    if(bytes<sizeof heh-1 || memcmp(word+bytes-(sizeof heh-1),heh,sizeof heh-1) ||
       n<5 || memcmp(tag+n-5,"'e:']",5))return 0;
    tag[n-5]='e';tag[n-4]=']';tag[n-3]=0;
    return 1;
}

static int stress_word_is(const char *word,size_t bytes,const char *value)
{ return strlen(value)==bytes && !memcmp(word,value,bytes); }

/* Explicit neutral-context policy, independent of the reference engine's
 * final/nonfinal digits. Do not include ambiguous prepositions such as تو,
 * which can also be the independent, accented pronoun "you". */
int reloq_fa_direct_is_proclitic(const char *word,size_t bytes)
{
    static const char *const words[]={"و", "یا", "که", "از", "به", "با", "در", "تا", "جز"};
    if(!word || !bytes || memchr(word,0,bytes))return 0;
    for(size_t i=0;i<sizeof words/sizeof words[0];++i)
        if(stress_word_is(word,bytes,words[i]))return 1;
    /* Preserve the same phrase role when the preposition's short vowel is
     * explicitly written, e.g. دَر before a following content word. */
    char bare[5],tag[16];
    if(explicit_vowelled_cvc(word,bytes,tag,sizeof tag,bare)==1)
        for(size_t i=0;i<sizeof words/sizeof words[0];++i)
            if(stress_word_is(bare,4,words[i]))return 1;
    return 0;
}

/* The provider can still shape the phrase; these digits only describe the
 * lexical accent. A word named alone retains its ordinary citation stress. */
static void apply_function_word_context(const char *word,size_t bytes,int follows_word,char *tag)
{
    static const char *const independent_pronouns[]={"من", "ما", "این", "آن", "خود"};
    for(size_t i=0;i<sizeof independent_pronouns/sizeof independent_pronouns[0];++i) {
        if(stress_word_is(word,bytes,independent_pronouns[i])) {
            stress_primary(tag,0);return;
        }
    }
    /* Proclitics lean on a following word. When quoted or read alone, keep
     * the ordinary citation accent; punctuation is a phrase boundary. */
    if(reloq_fa_direct_is_proclitic(word,bytes)) {
        if(follows_word) {
            stress_unaccent(tag);
            (void)shorten_written_final_e(word,bytes,tag);
        }
        else {
            size_t syllables=stress_syllables(tag);
            if(syllables)stress_primary(tag,syllables-1);
        }
    }
}

/* Recognize a finite inflection only when the independently listed stem
 * is also a verb and already has an available native pronunciation. */
static size_t verb_stem_syllables(const char *word,size_t bytes)
{
    if(!(stress_class(word,bytes)&2u))return 0;
    char stem[2048];
    if(direct_context_word_exact(word,bytes,0,stem,sizeof stem)!=1)return 0;
    return stress_syllables(stem);
}

static int ends_with(const char *word,size_t bytes,const char *suffix,size_t *stem)
{
    size_t n=strlen(suffix);
    if(bytes<=n || memcmp(word+bytes-n,suffix,n))return 0;
    *stem=bytes-n;return 1;
}

/* Written compound nominalizations such as راه‌اندازی and ذخیره‌سازی
 * normally have their principal accent on the derivational -i. Only apply
 * this to a pronunciation already admitted by the direct lexicon; the
 * half-space and a nonempty first component distinguish this from an
 * independently written finite verb. An explicitly written ezafe is an
 * unstressed following syllable, not part of the derivation. */
static size_t nominal_compound_primary(const char *word,size_t bytes,const char *tag,
                                       size_t syllables)
{
    size_t end=bytes,stem=0;
    int ezafe=ends_with(word,bytes,"ِ",&end);
    if(ezafe) {
        size_t n=strlen(tag);
        if(syllables<2 || n<4 || tag[n-1]!=']' || tag[n-2]!='e' ||
           tag[n-4]!='.' || tag[n-3]<'0' || tag[n-3]>'2')return SIZE_MAX;
    }
    static const char *const suffixes[]={"‌اندازی", "‌سازی", "‌یابی", "‌نویسی", "‌گذاری"};
    for(size_t i=0;i<sizeof suffixes/sizeof suffixes[0];++i)
        if(ends_with(word,end,suffixes[i],&stem) && stem)
            return syllables-(ezafe?2u:1u);
    return SIZE_MAX;
}

/* A directly recognized noun with the written plural -hâ carries its
 * accent on that suffix; following ezafe -ye and indefinite -i are weak.
 * Require the half-space and the exact expected suffix phones so an
 * unrelated spelling or an unrecognized reference word is not guessed. */
static size_t plural_suffix_primary(const char *word,size_t bytes,const char *tag,
                                    size_t syllables)
{
    size_t stem=0,back=0;
    const char *last_phone=NULL,*previous_phone=NULL;
    if(ends_with(word,bytes,"‌هایی",&stem) || ends_with(word,bytes,"‌های",&stem))back=1;
    else if(!ends_with(word,bytes,"‌ها",&stem))return SIZE_MAX;
    if(!stem || syllables<=back)return SIZE_MAX;
    for(size_t i=2;tag[i] && tag[i]!=']';++i)
        if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2') {
            previous_phone=last_phone;last_phone=tag+i+2;
        }
    if(!last_phone)return SIZE_MAX;
    if(!back)return strcmp(last_phone,"ha]")?SIZE_MAX:syllables-1;
    if(!previous_phone || previous_phone[0]!='h' || previous_phone[1]!='a' ||
       previous_phone[2]!='.')return SIZE_MAX;
    if(strcmp(last_phone,"je]") && strcmp(last_phone,"'i:']"))return SIZE_MAX;
    return syllables-2;
}

/* A dictionary-listed -i derivative can be an adjective rather than the
 * unstressed indefinite suffix. For an otherwise unclassified full word,
 * require an independently classified multisyllabic noun stem, one extra
 * syllable, the same consonants, and a changed stem vowel. This preserves
 * the indefinite reading when the stem's sounds are simply followed by i. */
static int accent_lexical_i_derivative(const char *word,size_t bytes,const char *tag,size_t syllables)
{
    size_t stem=0;
    if(!ends_with(word,bytes,"ی",&stem) || stress_class(word,stem)!=1u ||
       direct_index(word,bytes)>=reloq_fa_direct_word_count())return 0;
    char base[2048],a[2048],b[2048];size_t na=0,nb=0;
    if(reloq_fa_direct_word(word,stem,base,sizeof base)!=1 ||
       stress_syllables(base)<2 || stress_syllables(base)+1!=syllables)return 0;
    const char *sources[]={base,tag};char *outputs[]={a,b};size_t *lengths[]={&na,&nb};
    for(unsigned k=0;k<2;++k) {
        const char *s=sources[k];size_t *n=lengths[k];
        if(s[0]!='`' || s[1]!='[')return 0;
        size_t i=2;
        for(;s[i] && s[i]!=']';++i) {
            if(s[i]=='.' && s[i+1]>='0' && s[i+1]<='2'){++i;continue;}
            if(s[i]=='\'' || s[i]==':')continue;
            if(!strchr("AaeiouOy bptdkgfsvzSZCJrmnRlGhwj",s[i]) || s[i]==' ' || *n>=2047)return 0;
            outputs[k][(*n)++]=s[i];
        }
        if(s[i]!=']' || s[i+1])return 0;
        outputs[k][*n]=0;
    }
    if(nb<2 || b[nb-1]!='i' || (na==nb-1 && !memcmp(a,b,na)))return 0;
    size_t i=0,j=0;
    while(i<na || j<nb-1) {
        while(i<na && strchr("AaeiouOy",a[i]))++i;
        while(j<nb-1 && strchr("AaeiouOy",b[j]))++j;
        if(i==na || j==nb-1)return i==na && j==nb-1;
        if(a[i++]!=b[j++])return 0;
    }
    return 1;
}

/* Initial alif + lam + alif is not a long first vowel where the verified
 * phone sequence has a separately accented following /l/ syllable. Apply
 * the same bounded orthographic rule to spelling variants and inflections. */
static void shorten_initial_alif_lam(const char *word,size_t bytes,char *tag)
{
    const size_t prefix=sizeof "الا"-1;
    if(bytes<prefix || (memcmp(word,"الا",prefix) && memcmp(word,"الآ",prefix)))return;
    if(tag[0]!='`' || tag[1]!='[' || tag[2]!='.' ||
       tag[3]<'0' || tag[3]>'2' || strncmp(tag+4,"'A:'.1l",7))return;
    tag[3]='0';tag[4]='A';memmove(tag+5,tag+8,strlen(tag+8)+1);
}

/* An unmarked initial alif can conceal a short vowel. Two primary marks
 * in one word are not a reason to stretch its initial vowel when another
 * primary follows. Preserve the vowel identity and let the shared timing
 * policy select its short nucleus. Explicit long alif madda is untouched. */
static void unstress_repeated_initial_alif(const char *word,size_t bytes,char *tag)
{
    static const char alif[]="ا";
    if(bytes<=sizeof alif-1 || memcmp(word,alif,sizeof alif-1) ||
       strncmp(tag,"`[.1'A:'",8))return;
    for(size_t i=8;tag[i] && tag[i]!=']';++i)
        if(tag[i]=='.' && tag[i+1]=='1'){tag[3]='0';return;}
}

/* The imported pronunciations sometimes mark Persian /æ/ as long before a
 * consonant cluster. That marker is acoustic, not a separate Persian vowel.
 * Use the ordinary short /æ/ for closed syllables with two following
 * consonants, including a cluster divided by a syllable boundary. Operate
 * only on a generated single-word pronunciation: explicitly entered phone
 * tags and their requested lengths remain under the caller's control. */
static void shorten_clustered_front_vowels(char *tag)
{
    size_t n=strlen(tag);
    if(n<4 || tag[0]!='`' || tag[1]!='[' || tag[n-1]!=']' ||
       memchr(tag+2,']',n-3))return;
    const char *consonants="bptdkgfsvzSZCJrmnRlGhwj";
    for(char *p=tag+2;(p=strstr(p,"'A:'"))!=NULL;) {
        char *next=p+4;
        if(!*next || !strchr(consonants,*next)) {p=next;continue;}
        ++next;
        if(next[0]=='.' && next[1]>='0' && next[1]<='2')next+=2;
        if(!*next || !strchr(consonants,*next)) {p+=4;continue;}
        *p='A';
        memmove(p+1,p+4,strlen(p+4)+1);
        ++p;
    }
}

/* City names are grammatical evidence for the derivational, stressed -i
 * in a demonym. The same spelling can also be a verb: a preceding verbal
 * prefix still supplies its usual stress in the phrase-level rule. */
static int city_stem(const char *word,size_t bytes)
{
    size_t lo=0,hi=sizeof fa_city_stems/sizeof fa_city_stems[0];
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2,n=strlen(fa_city_stems[mid]);
        size_t common=bytes<n?bytes:n;
        int order=memcmp(word,fa_city_stems[mid],common);
        if(!order)order=bytes<n?-1:bytes>n?1:0;
        if(order<0)hi=mid;
        else if(order>0)lo=mid+1;
        else return 1;
    }
    return 0;
}

static int stressed_place_derivative(const char *word,size_t bytes,const char *tag,unsigned kind)
{
    size_t stem=0,n=strlen(tag);
    if(n<4 || (tag[n-2]!='i' && (n<6 || memcmp(tag+n-5,"'i:']",5))))return 0;
    if(ends_with(word,bytes,"ئی",&stem)) {
        if(city_stem(word,stem))return 1;
        /* A separately classified noun derived from a known noun ending in
         * a vowel can also take this suffix. Unclassified indefinite forms
         * remain ambiguous and keep their previous accent. */
        return kind==1u && stress_class(word,stem)==1u;
    }
    return ends_with(word,bytes,"ی",&stem) && city_stem(word,stem);
}

/* Do not use a final-stress default when an independently classified stem
 * indicates an enclitic or a finite verb ending. */
static int potentially_weak_ending(const char *word,size_t bytes)
{
    static const char *const endings[]={"شان", "تان", "مان", "شون", "تون", "مون",
        "ایم", "اید", "اند", "یم", "ید", "ند", "ئی", "م", "ت", "ش", "ی", "د"};
    for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        size_t stem=0;
        if(ends_with(word,bytes,endings[i],&stem) && (stress_class(word,stem)&3u))return 1;
    }
    return 0;
}

static void apply_persian_stress(const char *word,size_t bytes,char *tag,int direct)
{
    shorten_clustered_front_vowels(tag);
    size_t syllables=stress_syllables(tag),stem=0;
    if(!syllables || syllables==1)return;
    shorten_initial_alif_lam(word,bytes,tag);
    unstress_repeated_initial_alif(word,bytes,tag);
    /* This common computer term is absent from the grammatical class list.
     * It is also used as NVDA's Persian name for slash and backslash. */
    if(stress_word_is(word,bytes,"اسلش")) {
        stress_primary(tag,syllables-1);return;
    }
    /* In these independently reported lexical derivatives and names, the
     * final syllable belongs to the word. In particular, the written -ی is
     * derivational, not an unstressed indefinite clitic as in کتابی. Keep
     * genuinely ambiguous unlisted forms on the existing conservative path. */
    static const char *const final_accent[]={"حجازی", "بردباری", "سلامتی", "کثیریان",
        "عباسی", "منچستریونایتد", "سایبری", "زمستانی"};
    for(size_t i=0;i<sizeof final_accent/sizeof final_accent[0];++i)
        if(stress_word_is(word,bytes,final_accent[i])) {
            stress_primary(tag,syllables-1);return;
        }
    unsigned kind=stress_class(word,bytes);
    if(stressed_place_derivative(word,bytes,tag,kind)) {
        stress_primary(tag,syllables-1);return;
    }
    if(direct && !kind && accent_lexical_i_derivative(word,bytes,tag,syllables)) {
        stress_primary(tag,syllables-1);return;
    }

    /* A productive nominal -an plural bears the accent. Require a known
     * non-verbal stem, one additional syllable, and the corresponding /an/
     * ending; independently classified words and verbs keep their own rules. */
    size_t stem_an=0;
    if(!kind && ends_with(word,bytes,"ان",&stem_an) && stress_class(word,stem_an)==1u) {
        char base[2048];size_t n=strlen(tag);
        if(n>=3 && !memcmp(tag+n-3,"an]",3) &&
           direct_context_word_exact(word,stem_an,0,base,sizeof base)==1 &&
           stress_syllables(base)+1==syllables) {
            stress_primary(tag,syllables-1);return;
        }
    }

    /* A final written -y after a vowel can represent the unstressed /ye/
     * ezafe. The independent noun and its additional /ye/ syllable provide
     * the evidence; an independently classified spelling is left alone. */
    size_t stem_y=0,vowel_stem=0;
    if(!kind && ends_with(word,bytes,"ی",&stem_y) &&
       (ends_with(word,stem_y,"ا",&vowel_stem) || ends_with(word,stem_y,"و",&vowel_stem)) &&
       stress_class(word,stem_y)==1u) {
        const char *last=NULL;
        for(size_t i=2;tag[i] && tag[i]!=']';++i)
            if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2')last=tag+i+2;
        char base[2048];
        if(last && (!strcmp(last,"je]") || !strcmp(last,"j'e:']")) &&
           direct_context_word_exact(word,stem_y,0,base,sizeof base)==1) {
            size_t count=stress_syllables(base);
            if(count && count+1==syllables){stress_primary(tag,count-1);return;}
        }
    }

    /* The written plural -یان is accented when its independently known
     * base already ends in ی and the generated last syllable is /jan/.
     * Keep verbal homographs and unrelated spellings on their old path. */
    size_t plural_stem=0,y_stem=0;
    if(!(kind&2u) && ends_with(word,bytes,"ان",&plural_stem) &&
       ends_with(word,plural_stem,"ی",&y_stem) &&
       (stress_class(word,plural_stem)&1u)) {
        size_t n=strlen(tag);
        if(n>=5 && !memcmp(tag+n-4,"jan]",4)) {
            stress_primary(tag,syllables-1);return;
        }
    }

    if(direct && !(kind&2u)) {
        size_t target=nominal_compound_primary(word,bytes,tag,syllables);
        if(target!=SIZE_MAX){stress_primary(tag,target);return;}
        target=plural_suffix_primary(word,bytes,tag,syllables);
        if(target!=SIZE_MAX){stress_primary(tag,target);return;}
    }

    /* Verbal prefixes bear the accent. The common written half-space
     * makes می/نمی unambiguous; without it, require a known verb remainder. */
    if(kind==2u) {
        static const char *const prefixes[]={"نمی", "می"};
        for(size_t i=0;i<sizeof prefixes/sizeof prefixes[0];++i) {
            size_t n=strlen(prefixes[i]);
            if(bytes<=n || memcmp(word,prefixes[i],n))continue;
            static const char join[]="\xe2\x80\x8c";
            if(bytes>n+sizeof join-1 && !memcmp(word+n,join,sizeof join-1)) {
                stress_primary(tag,0);return;
            }
            if(stress_class(word+n,bytes-n)&2u) {stress_primary(tag,0);return;}
        }
        /* Imperative/subjunctive be- and negative na- need a recognized
         * unprefixed verb; otherwise the initial letter may be the stem. */
        if(bytes>2 && (!memcmp(word,"ب",2) || !memcmp(word,"ن",2)) &&
           (stress_class(word+2,bytes-2)&2u)) {
            stress_primary(tag,0);return;
        }
        static const char *const perfect[]={"‌ایم", "‌اید", "‌اند", "‌است", "‌ام", "‌ای"};
        for(size_t i=0;i<sizeof perfect/sizeof perfect[0];++i) {
            if(!ends_with(word,bytes,perfect[i],&stem))continue;
            char base[2048];
            if(direct_context_word_exact(word,stem,0,base,sizeof base)==1) {
                size_t count=stress_syllables(base);
                if(count && count<=syllables){stress_primary(tag,count-1);return;}
            }
        }
        static const char *const endings[]={"یم", "ید", "ند", "م", "ی", "د"};
        for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
            if(!ends_with(word,bytes,endings[i],&stem))continue;
            size_t count=verb_stem_syllables(word,stem);
            if(count && count<=syllables){stress_primary(tag,count-1);return;}
        }
        /* A recognized first-person counterpart establishes that this
         * spelling is itself a bare past stem, e.g. نوشت / نوشتم. */
        if(bytes+3<2048) {
            char inflected[2048];
            memcpy(inflected,word,bytes);
            memcpy(inflected+bytes,"م",3);
            if(stress_class(inflected,bytes+2)&2u) {
                stress_primary(tag,syllables-1);return;
            }
        }
        /* If an inflection or bare stem was not identified, retain the
         * reference instead of accenting a possible person ending. */
        return;
    }

    /* A word classified only as a noun, adjective, adverb or numeral is
     * stressed on its last syllable. Verb/function-word homographs are not
     * silently treated as nouns. */
    if(kind==1u) {
        /* A classified lexical form in -انی usually derives from an -ان
         * base, rather than being only an unstressed indefinite -ی. Keep
         * this evidence separate from unknown forms and verbal homographs. */
        size_t an_stem=0;
        if(ends_with(word,bytes,"ی",&stem) && ends_with(word,stem,"ان",&an_stem) &&
           stress_class(word,stem)==1u) {
            size_t n=strlen(tag);
            if(n>=3 && !memcmp(tag+n-3,"ni]",3)) {
                stress_primary(tag,syllables-1);return;
            }
        }
        /* The spelling -ی can be an unstressed indefinite clitic or a
         * stressed derivational suffix. If the full spelling is already an
         * independent lexical entry, prefer its citation accent; otherwise
         * preserve a recognized base + indefinite form such as کتابی. */
        if(ends_with(word,bytes,"ی",&stem) && stress_class(word,stem)==1u) {
            size_t index=direct_index(word,stem);
            if(index<reloq_fa_direct_word_count() && index/8<sizeof fa_direct_suffix_bases[5] &&
               (fa_direct_suffix_bases[5][index/8] & (1u<<(index%8))) &&
               !stress_adjective_final_y(word,bytes) &&
               direct_index(word,bytes)>=reloq_fa_direct_word_count())return;
        }
        stress_primary(tag,syllables-1);return;
    }
    if(kind)return;

    /* Productive plural/comparative endings are accented. Check the same
     * per-stem admission bits used by the pronunciation generator. */
    static const char *const productive[]={"ها", "تر"};
    for(unsigned i=0;i<2;++i) {
        if(!ends_with(word,bytes,productive[i],&stem))continue;
        static const char join[]="\xe2\x80\x8c";
        int joined=stem>=sizeof join-1 && !memcmp(word+stem-(sizeof join-1),join,sizeof join-1);
        if(joined)stem-=sizeof join-1;
        size_t index=direct_index(word,stem);
        if(index>=reloq_fa_direct_word_count())continue;
        int admitted=joined ? i==0 && index/8<sizeof fa_plural_zwnj_bases &&
                              (fa_plural_zwnj_bases[index/8] & (1u<<(index%8))) :
                              index/8<sizeof fa_direct_suffix_bases[0] &&
                              (fa_direct_suffix_bases[i][index/8] & (1u<<(index%8)));
        if(admitted){stress_primary(tag,syllables-1);return;}
    }

    /* Explicit ezafe marks retain the stress of the preceding known noun. */
    static const char *const ezafe[]={"‌ی", "ٔ", "ِ"};
    for(size_t i=0;i<sizeof ezafe/sizeof ezafe[0];++i) {
        if(!ends_with(word,bytes,ezafe[i],&stem) || stress_class(word,stem)!=1u)continue;
        char base[2048];
        if(direct_context_word_exact(word,stem,0,base,sizeof base)==1) {
            size_t count=stress_syllables(base);
            if(count && count<=syllables){stress_primary(tag,count-1);return;}
        }
    }

    /* The productive possessive endings are applied only to stems whose
     * existing direct rule was admitted against four reference contexts. */
    static const char *const possessive[]={"م", "ت", "ش"};
    for(unsigned i=0;i<3;++i) {
        if(!ends_with(word,bytes,possessive[i],&stem) || stress_class(word,stem)!=1u)continue;
        size_t index=direct_index(word,stem),which=i+2;
        if(index>=reloq_fa_direct_word_count() || index/8>=sizeof fa_direct_suffix_bases[0] ||
           !(fa_direct_suffix_bases[which][index/8] & (1u<<(index%8))))continue;
        size_t count=stress_syllables(fa_direct_entries[index].tag);
        if(count && count<=syllables){stress_primary(tag,count-1);return;}
    }

    /* A recorded content word without a grammatical class normally has its
     * citation accent on the final syllable. Do not change known weak or
     * verbal endings, and never infer this merely from an arbitrary unknown
     * reference word. */
    if(!potentially_weak_ending(word,bytes)) {
        size_t end=0,n=strlen(tag);
        int joined=0;
        for(size_t i=0;i+3<=bytes;++i)
            if(!memcmp(word+i,"\xe2\x80\x8c",3)){joined=1;break;}
        int unknown_i=ends_with(word,bytes,"ی",&end) &&
                      !joined && n>=3 &&
                      (tag[n-2]=='i' || (n>=5 && !memcmp(tag+n-5,"'i:']",5)));
        if(direct_index(word,bytes)<reloq_fa_direct_word_count() || unknown_i)
            stress_primary(tag,syllables-1);
    }
}

/* Reference length marks must not impose a second Persian vowel duration.
 * This is used only on automatically produced text, not on phonetic input
 * supplied by the caller. Preserve vowel identity, stress and every other
 * control, including text outside phoneme groups and multi-group results. */
void reloq_fa_direct_normalize_durations(char *tag)
{
    if(!tag)return;
    char *read=tag,*write=tag;int in_group=0;
    while(*read) {
        if(!in_group && read[0]=='`' && read[1]=='[') {
            *write++=*read++;*write++=*read++;in_group=1;continue;
        }
        if(in_group && read[0]=='\'' && read[1] && strchr("Aaeiou",read[1]) &&
           read[2]==':' && read[3]=='\'') {
            *write++=read[1];read+=4;continue;
        }
        if(in_group && *read==']')in_group=0;
        *write++=*read++;
    }
    *write=0;
}

int reloq_fa_direct_stress(const char *word,size_t bytes,char *tag,size_t capacity)
{
    if(!word || !bytes || memchr(word,0,bytes) || !tag || !capacity ||
       !memchr(tag,0,capacity))return -1;
    apply_persian_stress(word,bytes,tag,0);
    reloq_fa_direct_normalize_durations(tag);
    return 0;
}

/* A nominal word followed by an otherwise unclassified noun-derived -i
 * form is evidence for an adjective rather than an indefinite noun. Keep
 * numerals, verbal homographs and explicit function words out of this
 * heuristic. Unresolved isolated forms keep their existing pronunciation. */
void reloq_fa_direct_stress_after_word(const char *previous,size_t previous_bytes,
                                       const char *word,size_t bytes,char *tag)
{
    static const char *const numerals[]={"یک","دو","سه","چهار","پنج","شش","هفت",
                                         "هشت","نه","ده","صد","هزار"};
    if(!previous || !previous_bytes || !word || !bytes || !tag)return;
    static const char kasra[]="ِ";
    if(previous_bytes>sizeof kasra-1 &&
       !memcmp(previous+previous_bytes-(sizeof kasra-1),kasra,sizeof kasra-1))
        previous_bytes-=sizeof kasra-1;
    for(size_t i=0;i<sizeof numerals/sizeof numerals[0];++i)
        if(stress_word_is(previous,previous_bytes,numerals[i]))return;
    unsigned previous_kind=stress_class(previous,previous_bytes);
    /* Explicit short-vowel marks on an initial alef do not change the
     * lexical class of the already recognized preceding word. This lets
     * an ezafe-marked phrase such as اِختیاراتِ مدیریتی reach the same
     * adjective rule as its unmarked spelling. */
    if(!previous_kind && previous_bytes>4 && previous_bytes<2048 &&
       !memcmp(previous,"ا",2) && (unsigned char)previous[2]==0xd9 &&
       ((unsigned char)previous[3]==0x8e || (unsigned char)previous[3]==0x8f ||
        (unsigned char)previous[3]==0x90)) {
        char bare[2048];
        memcpy(bare,previous,2);
        memcpy(bare+2,previous+4,previous_bytes-4);
        previous_kind=stress_class(bare,previous_bytes-2);
    }
    if(previous_kind!=1u || stress_class(word,bytes))return;
    size_t stem=0,n=strlen(tag),syllables=stress_syllables(tag);
    if(!ends_with(word,bytes,"ی",&stem) || stress_class(word,stem)!=1u ||
       !syllables || n<3 || tag[n-1]!=']' || tag[n-2]!='i')return;
    stress_primary(tag,syllables-1);
}

static int direct_context_depth(const char *,size_t,int,char *,size_t,unsigned);

/* Return the only written half-space, if present. Multiple boundaries are
 * handled recursively by the suffix rules, not guessed as a binary word. */
static size_t single_joiner(const char *word,size_t bytes)
{
    static const char join[]="\xe2\x80\x8c";
    size_t found=SIZE_MAX;
    for(size_t i=0;i+sizeof join-1<=bytes;++i) {
        if(memcmp(word+i,join,sizeof join-1))continue;
        if(found!=SIZE_MAX)return SIZE_MAX;
        found=i;i+=sizeof join-2;
    }
    return found;
}

static int single_phone_tag(const char *tag)
{
    return stress_syllables(tag)>0;
}

/* Compose explicitly written heh ezafe from a known word, including the
 * precomposed Unicode form. Keep its existing pronunciation and accent. */
static int known_heh_ezafe(const char *word,size_t bytes,int follows_word,
                           char *output,size_t capacity,unsigned depth)
{
    char stem[1025],base[2048];
    size_t n=heh_ezafe_stem(word,bytes,stem,sizeof stem);
    if(!n)return 0;
    int result=direct_context_depth(stem,n,follows_word,base,sizeof base,depth+1);
    if(result!=1)return result;
    return append_heh_ezafe(base,output,capacity);
}

static int append_phone_tags(const char *left,const char *right,char *output,size_t capacity)
{
    size_t a=strlen(left),b=strlen(right);
    if(!single_phone_tag(left) || !single_phone_tag(right))return 0;
    if(a>SIZE_MAX-b || a+b-2>capacity)return -2;
    memcpy(output,left,a-1);
    memcpy(output+a-1,right+2,b-1);
    return 1;
}

/* An explicitly written ezafe after a classified, known i-final noun or
 * adjective has an unstressed /ye/ after its vowel. Only compose forms for
 * which the base pronunciation is already available, and preserve its
 * lexical stress. Exact recorded vocalized forms have already taken priority.
 * The mark is required; do not guess an unwritten ezafe from plain text. */
static int known_i_final_ezafe(const char *word,size_t bytes,int follows_word,
                               char *output,size_t capacity,unsigned depth)
{
    size_t stem=0,root=0;
    if(!ends_with(word,bytes,"ِ",&stem) || !stem ||
       (!ends_with(word,stem,"ی",&root) && !ends_with(word,stem,"ي",&root)) ||
       stress_class(word,stem)!=1u)return 0;
    char base[2048];
    int result=direct_context_depth(word,stem,follows_word,base,sizeof base,depth+1);
    if(result!=1)return result;
    size_t n=strlen(base);
    if(!single_phone_tag(base) || n<3 || base[n-2]!='i')return 0;
    if(n>SIZE_MAX-5 || n+5>capacity)return -2;
    memcpy(output,base,n-1);
    memcpy(output+n-1,".0je]",6);
    return 1;
}

/* A classified -یار / -یاری derivative after an independently known -خت
 * stem can syllabify as /...x.ti.ja:r(i)/. Preserve existing full-word
 * pronunciations first; require lexical class evidence and the matching
 * native stem phones so unrelated ambiguous unmarked spellings are not
 * invented by the rule. */
static int derived_kht_yar(const char *word,size_t bytes,char *output,size_t capacity,unsigned depth)
{
    size_t stem=0;
    int yary=ends_with(word,bytes,"یاری",&stem);
    if(!yary && !ends_with(word,bytes,"یار",&stem))return 0;
    if(stem<4 || memcmp(word+stem-4,"خت",4) || stress_class(word,bytes)!=1u ||
       stress_class(word,stem)!=1u)return 0;
    char root[2048];
    int result=direct_context_depth(word,stem,0,root,sizeof root,depth+1);
    if(result!=1)return result;
    size_t n=strlen(root);
    if(n<6 || memcmp(root+n-3,"Gt]",3) || !single_phone_tag(root))return 0;
    const char *tail=yary ? ".0ti.0j'a:'.1ri]" : ".0ti.1j'a:'r]";
    size_t suffix=strlen(tail);
    if(n-2>capacity || suffix+1>capacity-(n-2))return -2;
    memcpy(output,root,n-2);
    memcpy(output+n-2,tail,suffix+1);
    size_t count=stress_syllables(output);
    if(!count)return 0;
    stress_primary(output,count-1);
    return 1;
}

/* A classified written surname ending in -زاده can be formed from a known
 * native first part. This complements the general half-space compound rule;
 * without a classified full spelling, leave a fused word unresolved. */
static int derived_zade_word(const char *word,size_t bytes,char *output,size_t capacity,unsigned depth)
{
    size_t stem=0;
    if(!ends_with(word,bytes,"زاده",&stem) || stress_class(word,bytes)!=1u ||
       stress_class(word,stem)!=1u)return 0;
    char left[2048],right[2048];
    int a=direct_context_depth(word,stem,0,left,sizeof left,depth+1);
    int b=direct_context_depth(word+stem,bytes-stem,0,right,sizeof right,depth+1);
    if(a<0 || b<0)return a<0?a:b;
    if(a!=1 || b!=1)return 0;
    (void)shorten_written_final_e(word,stem,left);
    int result=append_phone_tags(left,right,output,capacity);
    if(result!=1)return result;
    size_t count=stress_syllables(output);
    if(count)stress_primary(output,count-1);
    return 1;
}

/* Productive -ان and -ی after an independently recognized -گر noun.
 * Resyllabify its final /r/ before the new vowel, and require evidence for
 * both spellings so unmarked homographs and verbs are not guessed. Existing
 * full-word readings take precedence over this fallback rule. */
static int derived_agent_r_word(const char *word,size_t bytes,char *output,size_t capacity,unsigned depth)
{
    size_t stem=0,base=0;
    int plural=ends_with(word,bytes,"ان",&stem);
    if(!plural && !ends_with(word,bytes,"ی",&stem))return 0;
    if(!ends_with(word,stem,"گر",&base) || stress_class(word,bytes)!=1u ||
       stress_class(word,stem)!=1u)return 0;
    char root[2048];
    int result=direct_context_depth(word,stem,0,root,sizeof root,depth+1);
    if(result!=1)return result;
    size_t n=strlen(root);
    if(n<5 || memcmp(root+n-4,"gAr]",4) || !single_phone_tag(root))return 0;
    const char *tail=plural ? ".1ran]" : ".1ri]";
    size_t suffix=strlen(tail);
    if(n-2>capacity || suffix+1>capacity-(n-2))return -2;
    memcpy(output,root,n-2);
    memcpy(output+n-2,tail,suffix+1);
    size_t count=stress_syllables(output);
    if(!count)return 0;
    stress_primary(output,count-1);
    return 1;
}

static int same_phones(const char *a,const char *b)
{
    size_t n=strlen(a);
    if(n!=strlen(b))return 0;
    for(size_t i=0;i<n;++i) {
        if(i && a[i-1]=='.' && b[i-1]=='.' &&
           a[i]>='0' && a[i]<='2' && b[i]>='0' && b[i]<='2')continue;
        if(a[i]!=b[i])return 0;
    }
    return 1;
}

/* A written enclitic هم retains the host's accent. In unspaced forms the
 * host must itself have a native pronunciation; otherwise do not guess. */
static int joined_ham_word(const char *word,size_t bytes,char *output,size_t capacity,unsigned depth)
{
    static const char ham[]="هم", join[]="\xe2\x80\x8c";
    size_t stem=0;
    if(!ends_with(word,bytes,ham,&stem))return 0;
    if(stem>=sizeof join-1 && !memcmp(word+stem-(sizeof join-1),join,sizeof join-1))
        stem-=sizeof join-1;
    /* A single-letter host makes a false split especially likely: وهم is
     * one noun, for example, not و + هم. */
    if(stem<4)return 0;
    /* A finite verb or a separately recorded full lexeme may merely end in
     * these letters (e.g. an inflection), not contain the particle. */
    if((stress_class(word,bytes)&2u) ||
       direct_index(word,bytes)<reloq_fa_direct_word_count() ||
       !stress_class(word,stem))return 0;
    /* If an existing full pronunciation already has the /h/, retain its
     * established vowels and consonants rather than rebuilding the host. */
    char original[2048];
    if(direct_context_word_exact(word,bytes,0,original,sizeof original)==1) {
        const char *last=strrchr(original,'.');
        if(last && last[1]>='0' && last[1]<='2' && last[2]=='h')return 0;
    }
    char left[2048],right[2048];
    int a=direct_context_depth(word,stem,0,left,sizeof left,depth+1);
    int b=direct_context_depth(ham,sizeof ham-1,0,right,sizeof right,depth+1);
    if(a<0 || b<0)return a<0?a:b;
    if(a!=1 || b!=1 || !single_phone_tag(left) || !single_phone_tag(right))return 0;
    if(reloq_fa_direct_is_proclitic(word,stem)) {
        stress_unaccent(left);
        stress_primary(right,0);
    } else stress_unaccent(right);
    return append_phone_tags(left,right,output,capacity);
}

static int has_weak_nominal_ending(const char *word,size_t bytes,unsigned depth)
{
    static const char *const endings[]={"شان", "تان", "مان", "شون", "تون", "مون",
                                        "م", "ت", "ش", "ی"};
    for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        size_t stem=0;
        if(!ends_with(word,bytes,endings[i],&stem) || !(stress_class(word,stem)&1u))continue;
        char base[2048];
        if(direct_context_depth(word,stem,0,base,sizeof base,depth+1)==1 &&
           single_phone_tag(base))return 1;
    }
    return 0;
}

/* Recognize already pronounced written combinations of an unaccented
 * preposition or هم plus a content word. A separate concessive چه can also
 * follow a function word. Require exactly the same native phones so this
 * only moves stress, never rewrites a lexical pronunciation by assumption. */
static void stress_fused_function_compound(const char *word,size_t bytes,char *tag,unsigned depth)
{
    unsigned kind=stress_class(word,bytes);
    if((kind&2u) || kind==1u || single_joiner(word,bytes)!=SIZE_MAX)return;
    if(has_weak_nominal_ending(word,bytes,depth))return;
    size_t syllables=stress_syllables(tag);
    if(syllables<2)return;
    for(size_t split=2;split<bytes;++split) {
        if(((unsigned char)word[split]&0xc0)==0x80)continue;
        int ham=stress_word_is(word,split,"هم");
        int prefix=reloq_fa_direct_is_proclitic(word,split) || ham;
        if(prefix && !ham && !(kind&4u) && !stress_word_is(word,split,"در"))continue;
        int concessive=stress_word_is(word+split,bytes-split,"چه") &&
                       stress_class(word,split)==5u;
        if(!prefix && !concessive)continue;
        unsigned right_kind=stress_class(word+split,bytes-split);
        if(prefix && (!(right_kind&1u) || (right_kind&2u)))continue;
        char left[2048],right[2048],merged[4096];
        if(direct_context_depth(word,split,0,left,sizeof left,depth+1)!=1 ||
           direct_context_depth(word+split,bytes-split,0,right,sizeof right,depth+1)!=1)continue;
        /* A one-syllable remainder is frequently the end of an unrelated
         * word, e.g. a possessive after بابا, not a separate content word. */
        if(prefix && stress_syllables(right)<2)continue;
        if(concessive)(void)shorten_written_final_e(word+split,bytes-split,right);
        if(append_phone_tags(left,right,merged,sizeof merged)!=1 || !same_phones(tag,merged))continue;
        size_t target=0,right_at=0;
        if(prefix && !ham) {
            target=stress_syllables(left);
            for(size_t i=2;right[i] && right[i]!=']';++i)
                if(right[i]=='.' && right[i+1]>='0' && right[i+1]<='2') {
                    if(right[i+1]=='1')target=stress_syllables(left)+right_at;
                    ++right_at;
                }
        } else target=syllables-1;
        stress_primary(tag,target);return;
    }
}

/* Productive half-space suffixes are constructed from any pronounceable
 * stem instead of a bitmap of individually admitted stems. The suffix owns
 * the lexical accent; its trailing ezafe /je/ or indefinite /i/ is weak. */
static int joined_suffix_word(const char *word,size_t bytes,int follows_word,
                              char *output,size_t capacity,unsigned depth)
{
    static const struct {const char *written,*spoken;} endings[]={
        {"‌هایی", "`[.1ha.0'i:']"}, {"‌های", "`[.1ha.0je]"},
        {"‌ها", "`[.1ha]"}, {"‌ترین", "`[.0tA.1rin]"}, {"‌تر", "`[.1tAr]"}
    };
    (void)follows_word;
    for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        size_t stem=0;
        if(!ends_with(word,bytes,endings[i].written,&stem))continue;
        unsigned kind=stress_class(word,stem);
        if(kind==2u)return 0;
        char left[2048];
        int result=direct_context_depth(word,stem,0,left,sizeof left,depth+1);
        if(result!=1)return result<0?result:0;
        if(!single_phone_tag(left))return 0;
        (void)shorten_written_final_e(word,stem,left);
        stress_unaccent(left);
        return append_phone_tags(left,endings[i].spoken,output,capacity);
    }
    return 0;
}

/* Productive resultative adjectives, e.g. ذخیره‌شده‌ی. A half-space
 * joins the recognizable first component to the participle; the written
 * ezafe after final ه is an unaccented /ye/, not another content word.
 * Resolve the stem with the existing native rules rather than guessing an
 * unknown stem, and keep a single accent on the participle itself. */
static int joined_participle_word(const char *word,size_t bytes,
                                  char *output,size_t capacity,unsigned depth)
{
    static const struct {const char *written,*spoken;} endings[]={
        {"‌شده‌ی", "`[.0So.1de.0je]"},
        {"‌شدهٔ", "`[.0So.1de.0je]"},
        {"‌شده", "`[.0So.1d'e:']"}
    };
    for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        size_t stem=0;
        if(!ends_with(word,bytes,endings[i].written,&stem) || !stem)continue;
        char left[2048];
        int result=direct_context_depth(word,stem,0,left,sizeof left,depth+1);
        if(result!=1)return result<0?result:0;
        if(!single_phone_tag(left))return 0;
        (void)shorten_written_final_e(word,stem,left);
        stress_unaccent(left);
        return append_phone_tags(left,endings[i].spoken,output,capacity);
    }
    return 0;
}

/* A productive surname ending after written final heh, as in
 * خواجه‌ئیان. Preserve a known stem's established phones and accent;
 * the ending contributes an unstressed /i/ and a final accented /an/.
 * Unknown stems and endings with a different stem vowel remain unresolved. */
static int joined_eian_name(const char *word,size_t bytes,char *output,size_t capacity)
{
    size_t stem=0,heh=0;
    if((!ends_with(word,bytes,"‌ئیان",&stem) && !ends_with(word,bytes,"ئیان",&stem)) || !stem ||
       !ends_with(word,stem,"ه",&heh))return 0;
    char left[2048];
    int result=direct_context_word_exact(word,stem,0,left,sizeof left);
    if(result!=1)return result<0?result:0;
    size_t n=strlen(left);
    if(!single_phone_tag(left) || n<5 || memcmp(left+n-5,"'e:']",5))return 0;
    return append_phone_tags(left,"`[.0'i:'.1an]",output,capacity);
}

/* The plural of a known -گر agent noun is also commonly written without
 * a half-space. Reuse the established joined suffix analysis so both
 * spellings have the same phones and suffix stress. Restrict this to a
 * classified noun stem and exclude explicitly classified verbal forms. */
static int unspaced_agent_plural(const char *word,size_t bytes,int follows_word,
                                 char *output,size_t capacity,unsigned depth)
{
    static const char *const endings[]={"هایی", "های", "ها"};
    size_t stem=0,agent=0;
    for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        if(!ends_with(word,bytes,endings[i],&stem) ||
           !ends_with(word,stem,"گر",&agent) || stress_class(word,stem)!=1u ||
           (stress_class(word,bytes)&2u))continue;
        if(bytes>2048u-4u)return 0;
        char joined[2048];
        memcpy(joined,word,stem);
        memcpy(joined+stem,"\xe2\x80\x8c",3);
        memcpy(joined+stem+3,word+stem,bytes-stem);
        return joined_suffix_word(joined,bytes+3,follows_word,output,capacity,depth+1);
    }
    return 0;
}

/* For a fully known joined form, retain its existing phones and stress. If
 * both halves reproduce it exactly, the boundary also identifies a short
 * stem-final /e/ that must not be lengthened before the next component. */
static void shorten_verified_compound(const char *word,size_t bytes,char *tag,unsigned depth)
{
    size_t split=single_joiner(word,bytes);
    if(split==SIZE_MAX || !split || split+3>=bytes)return;
    if(stress_word_is(word,split,"به"))return;
    char left[2048],right[2048],combined[4096];
    if(direct_context_depth(word,split,0,left,sizeof left,depth+1)!=1 ||
       direct_context_depth(word+split+3,bytes-split-3,0,right,sizeof right,depth+1)!=1 ||
       append_phone_tags(left,right,combined,sizeof combined)!=1)return;
    size_t n=strlen(left),total=strlen(tag);
    if(n<5 || total!=strlen(combined) ||
       memcmp(left+n-5,"'e:']",5) ||
       bytes<split || split<2 || memcmp(word+split-2,"ه",2))return;
    for(size_t i=0;i<total;++i) {
        if(i && tag[i-1]=='.' && combined[i-1]=='.' &&
           tag[i]>='0' && tag[i]<='2' && combined[i]>='0' && combined[i]<='2')continue;
        if(tag[i]!=combined[i])return;
    }
    char *vowel=tag+n-5;
    if(memcmp(vowel,"'e:'",4))return;
    memmove(vowel+1,vowel+4,strlen(vowel+4)+1);*vowel='e';
}

/* Compose otherwise unknown ZWNJ compounds from separately pronounced
 * components. Productive verbal prefixes keep their own accent; ordinary
 * compounds put the lexical accent on the final component. */
static int joined_compound_word(const char *word,size_t bytes,char *output,size_t capacity,unsigned depth)
{
    size_t split=single_joiner(word,bytes);
    if(split==SIZE_MAX || !split || split+3>=bytes)return 0;
    if(stress_word_is(word,split,"به"))return 0;
    const char *tail=word+split+3;size_t tail_bytes=bytes-split-3;
    int verb_prefix=stress_word_is(word,split,"می") || stress_word_is(word,split,"نمی");
    if(verb_prefix && !(stress_class(tail,tail_bytes)&2u))return 0;
    /* After final ه, an attached ای or ی can be an unstressed indefinite
     * or ezafe ending. Do not treat it as a new accented content word. A
     * separately recorded complete lexeme has already taken precedence. */
    size_t heh_stem=0;
    int enclitic=ends_with(word,split,"ه",&heh_stem) &&
                 (stress_word_is(tail,tail_bytes,"ای") || stress_word_is(tail,tail_bytes,"ی"));
    char left[2048],right[2048];
    int a=direct_context_depth(word,split,0,left,sizeof left,depth+1);
    int b=direct_context_depth(tail,tail_bytes,0,right,sizeof right,depth+1);
    if(a<0 || b<0)return a<0?a:b;
    if(a!=1 || b!=1 || !single_phone_tag(left) || !single_phone_tag(right))return 0;
    (void)shorten_written_final_e(word,split,left);
    if(enclitic)stress_unaccent(right);
    int result=append_phone_tags(left,right,output,capacity);
    if(result!=1)return result;
    size_t count=stress_syllables(output);
    if(count && !enclitic)stress_primary(output,verb_prefix?0:count-1);
    return 1;
}

/* A written numeral can be attached directly to a known nominal component.
 * Give already recorded words precedence, and never guess either component:
 * only a classified nominal tail with its own native pronunciation is used.
 * The same stress and short connected vowel rules apply with a half-space. */
static int joined_numeral_word(const char *word,size_t bytes,char *output,size_t capacity,unsigned depth)
{
    static const char *const numerals[]={"یک","دو","سه","چهار","پنج","شش","هفت","هشت","نه","ده"};
    static const char joiner[]="\xe2\x80\x8c";
    for(size_t i=0;i<sizeof numerals/sizeof numerals[0];++i) {
        size_t prefix=strlen(numerals[i]),tail=prefix;
        if(bytes<=prefix || memcmp(word,numerals[i],prefix))continue;
        if(bytes-tail>=sizeof joiner-1 && !memcmp(word+tail,joiner,sizeof joiner-1))tail+=sizeof joiner-1;
        /* Two-letter tails also occur as ordinary verbal endings: do not
         * split an inflection just because its beginning resembles a number. */
        if(bytes-tail<6 || stress_class(word+tail,bytes-tail)!=1u)continue;
        char left[2048],right[2048];
        int a=direct_context_depth(word,prefix,0,left,sizeof left,depth+1);
        int b=direct_context_depth(word+tail,bytes-tail,0,right,sizeof right,depth+1);
        if(a<0 || b<0)return a<0?a:b;
        if(a!=1 || b!=1 || !single_phone_tag(left) || !single_phone_tag(right))continue;
        (void)shorten_written_final_e(word,prefix,left);
        int result=append_phone_tags(left,right,output,capacity);
        if(result!=1)return result;
        size_t count=stress_syllables(output);
        if(count)stress_primary(output,count-1);
        return 1;
    }
    return 0;
}

/* Apply the same compound and verbal-prefix stress as the admitted-word
 * rule, but allow an unknown component to use the general orthography. */
int reloq_fa_direct_guess_prefixed_verb(const char *word,size_t bytes,char *output,size_t capacity)
{
    if(!word || !bytes || !output || !capacity || memchr(word,0,bytes))return -1;
    if(!(stress_class(word,bytes)&2u))return 0;
    static const struct {const char *ending,*phones;} endings[]={
        {"ند","And"},{"ید","id"},{"یم","im"},
        {"م","Am"},{"ی","i"},{"د","Ad"}
    };
    for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        size_t stem=0;
        if(!ends_with(word,bytes,endings[i].ending,&stem) || !stem)continue;
        char root[2048];
        int status=reloq_fa_direct_context_word(word,stem,0,root,sizeof root);
        if(status<0)return status;
        if(status!=1 || !single_phone_tag(root))continue;
        size_t n=strlen(root),suffix=strlen(endings[i].phones);
        /* Re-syllabify a final consonant as onset of the inflection. */
        if(n<6 || !strchr("bptdkgfsvzSZCJrmnRlG",root[n-2]))continue;
        if(n>SIZE_MAX-suffix-3 || n+suffix+3>capacity)return -2;
        memcpy(output,root,n-2);
        size_t at=n-2;output[at++]='.';output[at++]='0';output[at++]=root[n-2];
        memcpy(output+at,endings[i].phones,suffix);at+=suffix;
        output[at++]=']';output[at]=0;
        return 1;
    }
    return 0;
}

/* A recognized verbal prefix and classified finite form are contextual
 * evidence. Only infer the inflection when its stem already has a native
 * pronunciation; the same spelling without this context stays unchanged. */
static int joined_context_verb(const char *word,size_t bytes,char *output,size_t capacity)
{
    size_t split=single_joiner(word,bytes);
    if(split==SIZE_MAX || !split || split+3>=bytes ||
       (!stress_word_is(word,split,"می") && !stress_word_is(word,split,"نمی")))return 0;
    char left[2048],right[2048];
    int a=reloq_fa_direct_context_word(word,split,0,left,sizeof left);
    if(a!=1)return a<0?a:0;
    int b=reloq_fa_direct_guess_prefixed_verb(word+split+3,bytes-split-3,right,sizeof right);
    if(b!=1)return b;
    int result=append_phone_tags(left,right,output,capacity);
    if(result!=1)return result;
    if(stress_syllables(output))stress_primary(output,0);
    return 1;
}

static int guessed_joined_word(const char *word,size_t bytes,char *output,size_t capacity)
{
    size_t split=single_joiner(word,bytes);
    if(split==SIZE_MAX || !split || split+3>=bytes)return 0;
    const char *tail=word+split+3;size_t tail_bytes=bytes-split-3;
    int verb_prefix=stress_word_is(word,split,"می") || stress_word_is(word,split,"نمی");
    char left[2048],right[2048];
    int a=reloq_fa_direct_context_word(word,split,0,left,sizeof left);
    if(a==0)a=reloq_fa_direct_guess_word(word,split,left,sizeof left);
    int b=reloq_fa_direct_context_word(tail,tail_bytes,0,right,sizeof right);
    if(b==0 && verb_prefix)b=reloq_fa_direct_guess_prefixed_verb(tail,tail_bytes,right,sizeof right);
    if(b==0)b=reloq_fa_direct_guess_word(tail,tail_bytes,right,sizeof right);
    if(a<0 || b<0)return a<0?a:b;
    if(a!=1 || b!=1 || !single_phone_tag(left) || !single_phone_tag(right))return 0;
    (void)shorten_written_final_e(word,split,left);
    int result=append_phone_tags(left,right,output,capacity);
    if(result!=1)return result;
    size_t count=stress_syllables(output);
    if(count)stress_primary(output,verb_prefix?0:count-1);
    return 1;
}

/* NVDA's symbol names and other text can spell short vowels explicitly even
 * when the known lexical form omits them. Accept the marked form only when a
 * simple consonant-by-consonant alignment proves every supplied vowel agrees
 * with that pronunciation. Reject long vowels, shadda, silent letters and
 * ambiguous spellings rather than silently discarding a meaningful mark. */
static int marked_known_initial_vowel(const char *word,size_t bytes,int follows_word,
                                      char *output,size_t capacity)
{
    struct unit {char consonant,vowel;} units[128];
    char bare[257],tag[2048],phones[2048];
    const unsigned char *input=(const unsigned char *)word;
    size_t at=0,count=0,bare_bytes=0,phone_count=0;
    int marked=0;
    if(!word || !bytes || bytes>sizeof bare-1)return 0;
    while(at<bytes) {
        if(count==sizeof units/sizeof units[0])return 0;
        size_t begin=at;
        uint32_t cp=explicit_arabic_scalar(input,bytes,&at);
        char consonant=count?explicit_consonant(cp):0;
        if((!count && cp!=0x0627) || (count && !consonant))return 0;
        if(at-begin>sizeof bare-1-bare_bytes)return 0;
        memcpy(bare+bare_bytes,word+begin,at-begin);bare_bytes+=at-begin;
        struct unit unit={consonant,0};
        if(at<bytes) {
            size_t next=at;uint32_t mark=explicit_arabic_scalar(input,bytes,&next);
            if(mark==0x064e || mark==0x0650 || mark==0x064f) {
                unit.vowel=mark==0x064e?'A':mark==0x0650?'e':'o';
                marked=1;at=next;
            }
        }
        units[count++]=unit;
    }
    if(!marked || count<2)return 0;
    bare[bare_bytes]=0;
    int status=direct_context_word_exact(bare,bare_bytes,follows_word,tag,sizeof tag);
    if(status!=1)return status<0?status:0;
    apply_persian_stress(bare,bare_bytes,tag,1);
    apply_function_word_context(bare,bare_bytes,follows_word,tag);
    if(tag[0]!='`' || tag[1]!='[')return 0;
    size_t i=2;
    for(;tag[i] && tag[i]!=']';++i) {
        if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2'){++i;continue;}
        if(!strchr("Aaeiou bptdkgfsvzSZCJrmnRlG",tag[i]) || tag[i]==' ' ||
           phone_count==sizeof phones)return 0;
        phones[phone_count++]=tag[i];
    }
    if(tag[i]!=']' || tag[i+1] || !phone_count || !strchr("Aeo",phones[0]))return 0;
    size_t phone=1;
    if(units[0].vowel && units[0].vowel!=phones[0])return 0;
    for(size_t u=1;u<count;++u) {
        while(phone<phone_count && strchr("Aaeiou",phones[phone]))++phone;
        if(phone==phone_count || phones[phone++]!=units[u].consonant)return 0;
        if(units[u].vowel && (phone==phone_count || phones[phone++]!=units[u].vowel))return 0;
    }
    while(phone<phone_count && strchr("Aaeiou",phones[phone]))++phone;
    if(phone!=phone_count)return 0;
    size_t n=strlen(tag);
    if(n>=capacity)return -2;
    memcpy(output,tag,n+1);return 1;
}

/* A final written -ina can extend an already known -in stem. Preserve the
 * stem's verified vowels, move its final n to the new syllable, and use the
 * normal final stress. Exact dictionary entries take precedence. */
static int known_in_stem_plus_a(const char *word,size_t bytes,char *output,size_t capacity)
{
    static const char ending[]="ینا",alef[]="ا";
    if(bytes<=sizeof ending-1 || memcmp(word+bytes-(sizeof ending-1),ending,sizeof ending-1))return 0;
    char tag[2048];size_t stem=bytes-(sizeof alef-1);
    int result=direct_context_word_exact(word,stem,0,tag,sizeof tag);
    if(result!=1)return result<0?result:0;
    size_t n=strlen(tag);
    if(n<6 || tag[0]!='`' || tag[1]!='[' || tag[n-1]!=']' || tag[n-2]!='n' ||
       tag[n-3]!='i' || strchr(tag+2,'~'))return 0;
    for(size_t i=2;i+1<n;++i)if(tag[i]==']' || tag[i]=='[' || tag[i]=='`')return 0;
    if(n+4>sizeof tag || n+3>=capacity)return -2;
    for(size_t i=2;i+1<n;++i)
        if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2')tag[i+1]='0';
    memcpy(tag+n-2,".1na]",6);
    memcpy(output,tag,n+4);return 1;
}

static int direct_context_depth(const char *word,size_t bytes,int follows_word,
                                char *output,size_t capacity,unsigned depth)
{
    if(output && capacity)output[0]=0;
    if(!word || !bytes || bytes==SIZE_MAX || !output || !capacity || memchr(word,0,bytes))return -1;
    if(depth>8)return 0;
    /* Isolated short-vowel marks are character names, not the long entries
     * used by the reference lexicon. Attached marks are handled as part of
     * their word by the vowelled-orthography rules below. */
    size_t next=0;uint32_t cp=0;
    if(reloq_utf8_next((const unsigned char *)word,bytes,&next,&cp)==1 && next==bytes &&
       cp>=0x064e && cp<=0x0650) {
        const char *name=reloq_fa_direct_letter(cp);
        if(!name)return 0;
        size_t n=strlen(name);
        if(n>=capacity)return -2;
        memcpy(output,name,n+1);return 1;
    }
    char bare[5];
    int result=explicit_vowelled_cvc(word,bytes,output,capacity,bare);
    if(result==1) {
        apply_function_word_context(bare,4,follows_word,output);
        return 1;
    }
    if(result<0)return result;
    result=explicit_vowelled_word(word,bytes,output,capacity);
    if(result)return result;
    result=joined_ham_word(word,bytes,output,capacity,depth);
    if(result)return result;
    result=joined_suffix_word(word,bytes,follows_word,output,capacity,depth);
    if(result)return result;
    result=joined_participle_word(word,bytes,output,capacity,depth);
    if(result)return result;
    result=joined_eian_name(word,bytes,output,capacity);
    if(result)return result;
    result=unspaced_agent_plural(word,bytes,follows_word,output,capacity,depth);
    if(result)return result;
    /* A precomposed heh is the same written suffix as ه + U+0654. Test it
     * before an exact lookup so both forms receive the base word's stress. */
    if(bytes>=sizeof "ۀ"-1 && !memcmp(word+bytes-(sizeof "ۀ"-1),"ۀ",sizeof "ۀ"-1)) {
        result=known_heh_ezafe(word,bytes,follows_word,output,capacity,depth);
        if(result)return result;
    }
    result=direct_context_word_exact(word,bytes,follows_word,output,capacity);
    if(result==1) {
        apply_persian_stress(word,bytes,output,1);
        apply_function_word_context(word,bytes,follows_word,output);
        stress_fused_function_compound(word,bytes,output,depth);
        shorten_verified_compound(word,bytes,output,depth);
    }
    if(result)return result;
    result=known_heh_ezafe(word,bytes,follows_word,output,capacity,depth);
    if(result)return result;
    result=known_in_stem_plus_a(word,bytes,output,capacity);
    if(result)return result;
    result=marked_known_initial_vowel(word,bytes,follows_word,output,capacity);
    if(result)return result;
    result=fully_vowelled_word(word,bytes,output,capacity);
    if(result)return result;
    result=derived_kht_yar(word,bytes,output,capacity,depth);
    if(result)return result;
    result=derived_zade_word(word,bytes,output,capacity,depth);
    if(result)return result;
    result=derived_agent_r_word(word,bytes,output,capacity,depth);
    if(result)return result;
    result=joined_numeral_word(word,bytes,output,capacity,depth);
    if(result)return result;
    result=joined_compound_word(word,bytes,output,capacity,depth);
    if(result)return result;
    result=joined_context_verb(word,bytes,output,capacity);
    if(result)return result;
    /* Previously supported inflected and compound words take priority. */
    result=known_i_final_ezafe(word,bytes,follows_word,output,capacity,depth);
    if(result)return result;

    /* NFC/NFKC places Arabic vowel marks before shadda, while the eSpeak
       dictionary and input commonly place shadda first. They are canonically
       equivalent; try the reference order without dropping either mark.
       The caller projects the complete word as one source span, so its
       indexes are unaffected by this equal-length rearrangement. */
    const unsigned char *input=(const unsigned char *)word;
    size_t first=0;
    if(bytes<4)return 0;
    for(;first<=bytes-4;++first) {
        if(input[first]==0xd9 && input[first+1]>=0x8b && input[first+1]<=0x90 &&
           input[first+2]==0xd9 && input[first+3]==0x91)break;
    }
    if(first>bytes-4)return 0;
    char *variant=malloc(bytes+1);
    if(!variant)return -2;
    memcpy(variant,word,bytes);variant[bytes]=0;
    for(size_t i=first;i+3<bytes;) {
        unsigned char *p=(unsigned char *)variant+i;
        if(p[0]==0xd9 && p[1]>=0x8b && p[1]<=0x90 && p[2]==0xd9 && p[3]==0x91) {
            unsigned char mark=p[1];p[1]=0x91;p[3]=mark;
            i=i>=2?i-2:i+2;
        } else ++i;
    }
    result=direct_context_word_exact(variant,bytes,follows_word,output,capacity);
    if(result==1) {
        apply_persian_stress(variant,bytes,output,1);
        apply_function_word_context(variant,bytes,follows_word,output);
    }
    free(variant);
    return result;
}

int reloq_fa_direct_context_word(const char *word,size_t bytes,int follows_word,
                                 char *output,size_t capacity)
{
    return direct_context_depth(word,bytes,follows_word,output,capacity,0);
}
