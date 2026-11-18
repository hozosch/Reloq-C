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
        if(follows_word)stress_unaccent(tag);
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

static void apply_persian_stress(const char *word,size_t bytes,char *tag,int direct)
{
    size_t syllables=stress_syllables(tag),stem=0;
    if(!syllables || syllables==1)return;
    /* This common computer term is absent from the grammatical class list.
     * It is also used as NVDA's Persian name for slash and backslash. */
    if(stress_word_is(word,bytes,"اسلش")) {
        stress_primary(tag,syllables-1);return;
    }
    unsigned kind=stress_class(word,bytes);

    if(direct && !(kind&2u)) {
        size_t target=nominal_compound_primary(word,bytes,tag,syllables);
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
         * stressed derivational suffix (کتابی is a standard example).
         * Do not override a reference form admitted as the former. */
        if(ends_with(word,bytes,"ی",&stem) && stress_class(word,stem)==1u) {
            size_t index=direct_index(word,stem);
            if(index<reloq_fa_direct_word_count() && index/8<sizeof fa_direct_suffix_bases[5] &&
               (fa_direct_suffix_bases[5][index/8] & (1u<<(index%8))) &&
               !stress_adjective_final_y(word,bytes))return;
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

int reloq_fa_direct_context_word(const char *word,size_t bytes,int follows_word,
                                 char *output,size_t capacity)
{
    if(output && capacity)output[0]=0;
    char bare[5];
    int result=explicit_vowelled_cvc(word,bytes,output,capacity,bare);
    if(result==1) {
        apply_function_word_context(bare,4,follows_word,output);
        return 1;
    }
    if(result<0)return result;
    result=direct_context_word_exact(word,bytes,follows_word,output,capacity);
    if(result==1) {
        apply_persian_stress(word,bytes,output,1);
        apply_function_word_context(word,bytes,follows_word,output);
    }
    if(result || !word || bytes==SIZE_MAX)return result;

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
