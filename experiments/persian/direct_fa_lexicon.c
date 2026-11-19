/* Direct C lookup of an experimental generated Persian lexicon.
 * No eSpeak process, library, environment variable or global mutable state
 * is used at runtime. Generated tables retain their individual licenses.
 */
#include "direct_fa_lexicon.h"
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
        {"اَ", "`[.1'A:']"}, {"اُ", "`[.1'o:']"}, {"اِ", "`[.1'e:']"},
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
    static const char *const suffixes[]={"‌اندازی", "‌سازی", "‌یابی", "‌نویسی"};
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

static void apply_persian_stress(const char *word,size_t bytes,char *tag,int direct)
{
    size_t syllables=stress_syllables(tag),stem=0;
    if(!syllables || syllables==1)return;
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
}

int reloq_fa_direct_stress(const char *word,size_t bytes,char *tag,size_t capacity)
{
    if(!word || !bytes || memchr(word,0,bytes) || !tag || !capacity ||
       !memchr(tag,0,capacity))return -1;
    apply_persian_stress(word,bytes,tag,0);
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
    if(stress_class(previous,previous_bytes)!=1u || stress_class(word,bytes))return;
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

static int append_phone_tags(const char *left,const char *right,char *output,size_t capacity)
{
    size_t a=strlen(left),b=strlen(right);
    if(!single_phone_tag(left) || !single_phone_tag(right))return 0;
    if(a>SIZE_MAX-b || a+b-2>capacity)return -2;
    memcpy(output,left,a-1);
    memcpy(output+a-1,right+2,b-1);
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
    if(!stem)return 0;
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
    char left[2048],right[2048];
    int a=direct_context_depth(word,split,0,left,sizeof left,depth+1);
    int b=direct_context_depth(tail,tail_bytes,0,right,sizeof right,depth+1);
    if(a<0 || b<0)return a<0?a:b;
    if(a!=1 || b!=1 || !single_phone_tag(left) || !single_phone_tag(right))return 0;
    (void)shorten_written_final_e(word,split,left);
    int result=append_phone_tags(left,right,output,capacity);
    if(result!=1)return result;
    size_t count=stress_syllables(output);
    if(count)stress_primary(output,verb_prefix?0:count-1);
    return 1;
}

static int direct_context_depth(const char *word,size_t bytes,int follows_word,
                                char *output,size_t capacity,unsigned depth)
{
    if(output && capacity)output[0]=0;
    if(!word || !bytes || bytes==SIZE_MAX || !output || !capacity || memchr(word,0,bytes))return -1;
    if(depth>8)return 0;
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
    result=direct_context_word_exact(word,bytes,follows_word,output,capacity);
    if(result==1) {
        apply_persian_stress(word,bytes,output,1);
        apply_function_word_context(word,bytes,follows_word,output);
        stress_fused_function_compound(word,bytes,output,depth);
        shorten_verified_compound(word,bytes,output,depth);
    }
    if(result)return result;
    result=joined_compound_word(word,bytes,output,capacity,depth);
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
