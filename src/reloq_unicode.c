#include "reloq_unicode.h"
#include "reloq_character.h"
#include "reloq_inline_tags.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reloq_unicode_data.inc"
#include "reloq_unicode_multilingual.inc"
#include "reloq_unicode_greek.inc"
#define COUNT(a) (sizeof(a)/sizeof *(a))
#define STR(i) ((const char *)unicode_strings+(i))
#define NAME_CAPACITY 1024
/* String-only name extension. Native names keep their existing priority. */
int reloq_unicode_name_local(uint32_t,unsigned,char *,size_t);
struct buffer {unsigned char *p;size_t n,cap;};
static int add(struct buffer *b,const void *s,size_t n)
{
    if(n>SIZE_MAX-b->n-2)return -1;
    if(b->n+n+2>b->cap) {
        size_t cap=b->cap?b->cap:256;
        while(cap<b->n+n+2) {if(cap>SIZE_MAX/2){cap=b->n+n+2;break;}cap*=2;}
        void *p=realloc(b->p,cap);if(!p)return -1;b->p=p;b->cap=cap;
    }
    memcpy(b->p+b->n,s,n);b->n+=n;b->p[b->n]=0;return 0;
}
static int say(struct buffer *b,const char *s)
{return add(b," ",1)||add(b,s,strlen(s))||add(b," ",1);}
static int decode(const unsigned char *s,size_t n,size_t *used,uint32_t *cp)
{
    if(!n)return -1;
    unsigned c=s[0],need=c<128?1:c>=0xc2&&c<0xe0?2:c<0xf0&&c>=0xe0?3:c>=0xf0&&c<0xf5?4:0;
    if(!need||n<need)return -1;
    uint32_t v=need==1?c:c&((1u<<(7-need))-1);
    for(unsigned i=1;i<need;++i){if((s[i]&0xc0)!=0x80)return -1;v=(v<<6)|(s[i]&63);}
    if((need==2&&v<128)||(need==3&&v<2048)||(need==4&&v<65536)||
       v>0x10ffff||(v>=0xd800&&v<=0xdfff))return -1;
    *cp=v;*used=need;return 0;
}
static size_t encode(unsigned char s[4],uint32_t c)
{
    size_t n=1;
    if(c<128)s[0]=(unsigned char)c;
    else if(c<2048){s[0]=(unsigned char)(0xc0|(c>>6));s[1]=(unsigned char)(0x80|(c&63));n=2;}
    else if(c<65536){s[0]=(unsigned char)(0xe0|(c>>12));s[1]=(unsigned char)(0x80|((c>>6)&63));s[2]=(unsigned char)(0x80|(c&63));n=3;}
    else {s[0]=(unsigned char)(0xf0|(c>>18));s[1]=(unsigned char)(0x80|((c>>12)&63));s[2]=(unsigned char)(0x80|((c>>6)&63));s[3]=(unsigned char)(0x80|(c&63));n=4;}
    return n;
}
static int scalar(struct buffer *b,uint32_t c)
{
    unsigned char s[4];size_t n=encode(s,c);
    return add(b,s,n);
}
uint32_t reloq_unicode_compose(uint32_t a,uint32_t b)
{
    /* Hangul composition is algorithmic and has no UnicodeData table rows. */
    if(a>=0x1100&&a<=0x1112&&b>=0x1161&&b<=0x1175)
        return 0xac00+((a-0x1100)*21+b-0x1161)*28;
    if(a>=0xac00&&a<=0xd7a3&&!((a-0xac00)%28)&&b>=0x11a8&&b<=0x11c2)
        return a+b-0x11a7;
    size_t lo=0,hi=COUNT(unicode_compositions);
    while(lo<hi){size_t m=lo+(hi-lo)/2;const struct unicode_compose *r=unicode_compositions+m;
        if(r->a<a||(r->a==a&&r->b<b))lo=m+1;else hi=m;}
    return lo<COUNT(unicode_compositions)&&unicode_compositions[lo].a==a&&unicode_compositions[lo].b==b?unicode_compositions[lo].c:0;
}
static const struct unicode_latin *latin(uint32_t c)
{
    size_t lo=0,hi=COUNT(unicode_latin);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(unicode_latin[m].cp<c)lo=m+1;else hi=m;}
    return lo<COUNT(unicode_latin)&&unicode_latin[lo].cp==c?unicode_latin+lo:NULL;
}
static const char *cldr(const char *key,int english)
{
    size_t lo=0,hi=COUNT(unicode_cldr);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(strcmp(STR(unicode_cldr[m].key),key)<0)lo=m+1;else hi=m;}
    if(lo==COUNT(unicode_cldr)||strcmp(STR(unicode_cldr[lo].key),key))return NULL;
    uint32_t ix=english?unicode_cldr[lo].en:unicode_cldr[lo].de;
    return ix?STR(ix):NULL;
}
static const char *name(uint32_t c,char tmp[NAME_CAPACITY],int en)
{
    size_t lo=0,hi=COUNT(unicode_names);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(unicode_names[m].cp<c)lo=m+1;else hi=m;}
    if(lo<COUNT(unicode_names)&&unicode_names[lo].cp==c) {
        uint32_t label=en?unicode_names[lo].name:unicode_names[lo].de;
        if(label)return STR(label);
        return !en&&reloq_unicode_name_local(c,0,tmp,NAME_CAPACITY)==1?tmp:NULL;
    }
    const char *prefix=NULL;
    if((c>=0x3400&&c<=0x4dbf)||(c>=0x4e00&&c<=0x9fff)||(c>=0x20000&&c<=0x2a6df)||
       (c>=0x2a700&&c<=0x2b73f)||(c>=0x2b740&&c<=0x2b81d)||(c>=0x2b820&&c<=0x2cead)||
       (c>=0x2ceb0&&c<=0x2ebe0)||(c>=0x2ebf0&&c<=0x2ee5d)||(c>=0x30000&&c<=0x3134a)||
       (c>=0x31350&&c<=0x33479))prefix=en?"cjk unified ideograph":"CJK vereinheitlichtes Ideogramm";
    else if((c>=0x17000&&c<=0x187ff)||(c>=0x18d00&&c<=0x18d1e))prefix=en?"tangut ideograph":"Tangut Ideogramm";
    if(prefix){snprintf(tmp,NAME_CAPACITY,"%s %X",prefix,c);return tmp;}
    if(c>=0xac00&&c<=0xd7a3) {
        static const char *const l[]={"g","gg","n","d","dd","r","m","b","bb","s","ss","","j","jj","c","k","t","p","h"};
        static const char *const v[]={"a","ae","ya","yae","eo","e","yeo","ye","o","wa","wae","oe","yo","u","weo","we","wi","yu","eu","yi","i"};
        static const char *const t[]={"","g","gg","gs","n","nj","nh","d","l","lg","lm","lb","ls","lt","lp","lh","m","b","bs","s","ss","ng","j","c","k","t","p","h"};
        c-=0xac00;snprintf(tmp,NAME_CAPACITY,"%s %s%s%s",en?"hangul syllable":"Hangul Silbe",l[c/588],v[c%588/28],t[c%28]);return tmp;
    }
    return NULL;
}
static const char *language_name(uint32_t cp,int locale,char tmp[NAME_CAPACITY])
{
    if(locale==255)return name(cp,tmp,1);
    if((unsigned)locale>=9)return NULL;
    if(reloq_unicode_name_local(cp,(unsigned)locale+1,tmp,NAME_CAPACITY)==1)return tmp;
    /* The English table computes CJK/Tangut/Hangul ranges instead of storing
     * one string per ideograph or syllable. Extend those same ranges locally. */
    char english[NAME_CAPACITY];const char *s=name(cp,english,1),*suffix=NULL;
    unsigned kind=0;
    if(!s)return NULL;
    if(!strncmp(s,"cjk unified ideograph ",22)){suffix=s+22;kind=0;}
    else if(!strncmp(s,"tangut ideograph ",17)){suffix=s+17;kind=1;}
    else if(!strncmp(s,"hangul syllable ",16)){suffix=s+16;kind=2;}
    if(!suffix)return NULL;
    static const char *const prefix[3][9]={
        {"ideograma CJK unificado","idéogramme CJC unifié","ideogramma CJK unificato","中日韩统一表意文字","ideograma CJK unificado","中日韓統合漢字","yhtenäistetty CJK ideogrammi","한중일 통합 한자","ενοποιημένο ιδεόγραμμα CJK"},
        {"ideograma tangut","idéogramme tangoute","ideogramma tangut","西夏表意文字","ideograma tangute","西夏文字","tangut ideogrammi","탕구트 표의 문자","ιδεόγραμμα τανγκούτ"},
        {"sílaba hangul","syllabe hangeul","sillaba hangul","谚文音节","sílaba hangul","ハングル音節","hangul tavu","한글 음절","συλλαβή χάνγκουλ"}
    };
    int n=snprintf(tmp,NAME_CAPACITY,"%s %s",prefix[kind][locale],suffix);
    return n>0&&(size_t)n<NAME_CAPACITY?tmp:NULL;
}
static int cp1252(uint32_t c)
{
    return c<256||c==0x152||c==0x153||c==0x160||c==0x161||c==0x178||c==0x17d||c==0x17e||
        c==0x192||c==0x2c6||c==0x2dc||c==0x20ac||c==0x2013||c==0x2014||c==0x2018||c==0x2019||c==0x201a||c==0x201c||c==0x201d||c==0x201e||c==0x2020||c==0x2021||
        c==0x2022||c==0x2026||c==0x2030||c==0x2039||c==0x203a||c==0x2122;
}
static int one(struct buffer *b,uint32_t c,int mode,int en,int tags)
{
    if(c==0x1e9e)c=0xdf;
    const struct unicode_latin *l=latin(c);
    if(!mode) {
        if((c<32&&c!=9&&c!=10&&c!=13)||(c>=0x7f&&c<=0x9f)||
           c==0x1680||(c>=0x2000&&c<=0x200a)||c==0x2028||c==0x2029||
           c==0x202f||c==0x205f||c==0x3000)return add(b," ",1);
        if(c==0x2010||c==0x2011||c==0x2212)return add(b,"-",1);
        if(l) {
            unsigned char base;
            if(cp1252(c)||reloq_character_latin_base(c,&base)==1)return scalar(b,c);
            return add(b,STR(l->fold),strlen(STR(l->fold)));
        }
        if(c=='`'&&!tags) {
            struct reloq_character_reading r;
            if(reloq_character_read_native(c,en,&r)==1&&r.original_spr)
                return say(b,r.original_spr);
            return add(b," ",1);
        }
        if(en&&(c==0x2018||c==0x2019))return add(b,"'",1);
        if(en&&(c==0x201c||c==0x201d))return add(b,"\"",1);
        if(en&&(c==0x2013||c==0x2014))return add(b,"-",1);
        if(cp1252(c))return scalar(b,c);
        if(c==0xfe0e||c==0xfe0f||c==0x200d)return 0;
        if(c==0x200b||c==0x200c||c==0x2060||c==0xfeff)return 0;
        size_t lo=0,hi=COUNT(unicode_compat);
        while(lo<hi){size_t m=lo+(hi-lo)/2;if(unicode_compat[m].cp<c)lo=m+1;else hi=m;}
        if(lo<COUNT(unicode_compat)&&unicode_compat[lo].cp==c) {
            const char *fold=STR(unicode_compat[lo].fold);return add(b,fold,strlen(fold));
        }
        /* Greek is the deliberate exception: letter names are meaningful in
         * ordinary prose as well as in explicit character navigation. */
        lo=0;hi=COUNT(unicode_greek);
        while(lo<hi){size_t m=lo+(hi-lo)/2;if(unicode_greek[m]<c)lo=m+1;else hi=m;}
        if(lo<COUNT(unicode_greek)&&unicode_greek[lo]==c) {
            char temp[NAME_CAPACITY];const char *s=name(c,temp,en);if(s)return say(b,s);
        }
        /* Other symbol names belong to explicit character navigation. */
        return add(b," ",1);
    } else {
        {
            struct reloq_character_reading reading;
            int found=reloq_character_read_native(c,en,&reading);
            if(found<0)return -1;
            if(found==1&&((reading.original_spr&&*reading.original_spr)||reading.count)) {
                if(reading.original_spr)return say(b,reading.original_spr);
                for(size_t k=0;k<reading.count;++k) {
                    const char *end=strchr(reading.groups[k],']');
                    if(!end||add(b,k?" `2 `[":" `[",k?6:3)||
                       add(b,reading.groups[k],(size_t)(end-reading.groups[k]))||add(b,"] ",2))return -1;
                }
                return 0;
            }
        }
        /* An empty legacy SPR is an unavailable reading, not a native name.
         * Use the existing local Latin/accent entry for these few holes. */
        if(l&&!reloq_character_native_available(c,en)) {
            const char *label=STR(en?l->en:l->de);
            if(*label)return say(b,label);
        }
        /* Native German already has the original short CP1252 letter and
         * accent readings. Keep them as letters, without an added 'mit'. */
        unsigned char base;
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||
           (l&&(cp1252(c)||reloq_character_latin_base(c,&base)==1)))
            return add(b," ",1)||scalar(b,c)||add(b," ",1);
        if(l && *STR(en?l->en:l->de))return say(b,STR(en?l->en:l->de));
        if(c==' ')return say(b,en?"space":"Leerzeichen");
        if(c=='\n')return say(b,en?"line feed":"Zeilenvorschub");
        if(c=='\r')return say(b,en?"carriage return":"Wagenrücklauf");
        if(c=='\t')return say(b,en?"tab":"Tabulator");
        if(c==0)return say(b,en?"null character":"Nullzeichen");
    }
    char temp[NAME_CAPACITY];const char *s=name(c,temp,en);
    if(s)return say(b,s);
    char hex[16];snprintf(hex,sizeof hex,"%04X",c);
    if(say(b,"Unicode U plus"))return -1;
    for(size_t i=0;hex[i];++i)if(add(b,hex+i,1)||add(b," ",1))return -1;
    return 0;
}
/* The same Greek-letter inventory is named in every foreign language,
 * including polytonic and mathematical variants already named in DE/US. */
static const char *greek_label(uint32_t cp,int locale)
{
    if(locale!=255 && (unsigned)locale>=9)return NULL;
    size_t lo=0,hi=COUNT(unicode_greek);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(unicode_greek[m]<cp)lo=m+1;else hi=m;}
    if(lo==COUNT(unicode_greek)||unicode_greek[lo]!=cp)return NULL;
    lo=0;hi=COUNT(unicode_names);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(unicode_names[m].cp<cp)lo=m+1;else hi=m;}
    if(lo==COUNT(unicode_names)||unicode_names[lo].cp!=cp)return NULL;
    const char *alias=STR(unicode_names[lo].name);
    if(locale==255)return alias;
    lo=0;hi=COUNT(unicode_greek_labels);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(strcmp(unicode_greek_labels[m].english,alias)<0)lo=m+1;else hi=m;}
    return lo<COUNT(unicode_greek_labels)&&!strcmp(unicode_greek_labels[lo].english,alias)?
        unicode_greek_labels[lo].local[locale+1]:NULL;
}
static const char *language_label(const char *key,int locale)
{
    if(locale!=255 && (unsigned)locale>=9)return NULL;
    size_t bytes=strlen(key),used=0;uint32_t cp;
    if(bytes && !decode((const unsigned char *)key,bytes,&used,&cp) && used==bytes) {
        const char *greek=greek_label(cp,locale);if(greek)return greek;
    }
    if(locale==255)return cldr(key,1);
    size_t lo=0,hi=COUNT(multi_names);
    while(lo<hi){size_t m=lo+(hi-lo)/2;
        if(strcmp((const char *)multi_name_strings+multi_names[m].key,key)<0)lo=m+1;else hi=m;}
    if(lo==COUNT(multi_names)||strcmp((const char *)multi_name_strings+multi_names[lo].key,key))return NULL;
    uint32_t offset=multi_names[lo].label[locale];
    return offset?(const char *)multi_name_strings+offset:NULL;
}
static int language_unknown(struct buffer *b,uint32_t c,int locale)
{
    static const char *const prefix[]={"Unicode U más","Unicode U plus","Unicode U più","统一码 U 加","Unicode U mais","ユニコード U プラス","Unicode U plus","유니코드 U 플러스","Γιούνικοντ U συν"};
    if(say(b,(unsigned)locale<COUNT(prefix)?prefix[locale]:"Unicode U plus"))return -1;
    char hex[16];snprintf(hex,sizeof hex,"%04X",c);
    for(size_t i=0;hex[i];++i)if(add(b,hex+i,1)||add(b," ",1))return -1;
    return 0;
}
static int language_owns(uint32_t c,int language)
{
    int family=language>>16;
    if(family==18){
        if(c==0x384||c==0x385)return 1;
        size_t lo=0,hi=COUNT(multi_greek_native);
        while(lo<hi){size_t m=lo+(hi-lo)/2;if(multi_greek_native[m]<c)lo=m+1;else hi=m;}
        return lo<COUNT(multi_greek_native)&&multi_greek_native[lo]==c;
    }
    if(family!=6&&family!=8&&family!=10)return 0;
    if((c>=0x3400&&c<=0x9fff)||(c>=0xf900&&c<=0xfaff)||(c>=0x3000&&c<=0x303f)||(c>=0xff01&&c<=0xffef))return 1;
    if(family==8)return(c>=0x3041&&c<=0x3096)||(c>=0x3099&&c<=0x30ff)||(c>=0x31f0&&c<=0x31ff);
    if(family==10)return(c>=0x1100&&c<=0x11ff)||(c>=0x3130&&c<=0x318f)||(c>=0xac00&&c<=0xd7a3);
    return 0;
}
static int reading(struct buffer *b,const struct reloq_character_extension *r)
{
    if(r->base&&(add(b," `ts2 ",6)||scalar(b,r->base)||add(b," `ts0 ",6)))return -1;
    for(size_t k=0;k<r->count;++k)if(say(b,r->accents[k]))return -1;
    return 0;
}
static int unicode_admit(const unsigned char *text,size_t length,
    const unsigned char *modes,int english,int language,int tags,unsigned char **output,
    size_t *written,size_t **map)
{
    if(!text||!output||!written||!map||length>SIZE_MAX/sizeof(size_t)-1)return -1;
    size_t *offsets=malloc((length+1)*sizeof *offsets);if(!offsets)return -2;
    struct buffer b={0};int rc=-1,native_run=0,source_spelling=0;
    int locale=language?reloq_character_locale(language):-1;
    for(size_t i=0;i<length;) {
        uint32_t c;size_t n,begin=i,old;
        if(decode(text+i,length-i,&n,&c))goto fail;
        int mode=modes?modes[i]:0;size_t end=i+n;
        /* A caller's spelling command is independent of the UTF-8 API's
           character flag. Generated Greek names must be words in either
           route, rather than spelling the letters of their own name. */
        if(tags&&!mode&&c=='`') {
            size_t back=i;while(back&&text[back-1]=='\\')--back;
            if(!((i-back)&1)) {
                struct reloq_inline_tag t=reloq_inline_tag_read(text+i,length-i,0);
                if(t.kind==RELOQ_TAG_SPELL)source_spelling=t.value;
            }
        }
        /* Canonical base+accent composition spans ECI fragment boundaries. */
        while(end<length&&(!modes||modes[end]==mode)) {
            uint32_t mark;size_t z;if(decode(text+end,length-end,&z,&mark))goto fail;
            uint32_t merged=reloq_unicode_compose(c,mark);if(!merged)break;c=merged;end+=z;
        }
        {
            /* Foreign Greek letters have one alphabet-name route in both
             * sentence and character reading. This includes the CP1252
             * micro sign, whose Unicode alphabet alias is Greek mu. */
            const char *greek=(c==0xb5||c>=0x370)&&(!language||!language_owns(c,language))?
                greek_label(c,language?locale:255):NULL;
            if(greek) {
                char native_name[NAME_CAPACITY];
                if(!language&&!english)greek=name(c,native_name,0);
                if(!greek)goto fail;
                old=b.n;
                if((native_run||source_spelling)&&add(&b," `ts0 ",6)){rc=-2;goto fail;}
                native_run=0;
                if(say(&b,greek)||(source_spelling&&add(&b,source_spelling==1?" `ts1 ":" `ts2 ",6))){rc=-2;goto fail;}
                offsets[begin]=old;
                for(size_t j=begin+1;j<end;++j)offsets[j]=b.n;
                offsets[end]=b.n;i=end;continue;
            }
        }
        if(language&&(english==2||english==3)) {
            const char *label=NULL;size_t matched=0;
            if(mode&&(!cp1252(c)||c<=32||(c<128&&strchr("0123456789#*",(int)c)))){
                char key[160];size_t used=0,p=i;
                if(end>i+n){used=encode((unsigned char *)key,c);key[used]=0;p=end;
                    label=language_label(key,locale);if(label)matched=end;}
                while(p<length&&p-i<sizeof key-5&&(!modes||modes[p]==mode)){
                    size_t z;uint32_t next;if(decode(text+p,length-p,&z,&next))break;
                    if(!next){if(used)break;key[0]=0;p+=z;label=language_label(key,locale);if(label)matched=p;break;}
                    if(next!=0xfe0f){memcpy(key+used,text+p,z);used+=z;}
                    key[used]=0;p+=z;const char *candidate=used?language_label(key,locale):NULL;
                    if(candidate){label=candidate;matched=p;}
                }
            }
            if(!mode&&c<128&&strchr("0123456789#*",(int)c)&&end<length){
                size_t p=end,z;uint32_t next;
                if(!decode(text+p,length-p,&z,&next)&&next==0xfe0f)p+=z;
                if(p<length&&!decode(text+p,length-p,&z,&next)&&next==0x20e3){end=p+z;c=' ';}
            }
            int native=c>32&&!(c>=0x7f&&c<=0x9f)&&(cp1252(c)||language_owns(c,language));
            struct reloq_character_extension extension={0};int terminal=0;
            const struct unicode_latin *l=latin(c);
            if(mode&&cp1252(c)&&!reloq_character_native_available_language(c,language)){
                native=0;
                if(!label){char key[5]={0};encode((unsigned char *)key,c);
                    label=language_label(key,locale);if(label)matched=end;}
            }
            if(mode&&!reloq_character_native_available_language(c,language)){
                terminal=reloq_character_extension_read(c,language,&extension);
                if(terminal<0){rc=-2;goto fail;}
                if(terminal)native=0;
            }
            if(mode&&(c=='`'||c=='\\')){
                native=0;terminal=0;
                char key[2]={(char)c,0};label=locale==255?(c=='`'?"backquote":"backslash"):language_label(key,locale);
                matched=end;
            }
            int raw=mode&&native&&(!label||matched<=end)&&!terminal;
            if(native_run&&!raw){if(add(&b," `ts0 ",6)){rc=-2;goto fail;}native_run=0;}
            old=b.n;
            if(raw){
                if(!native_run&&add(&b," `ts2 ",6)){rc=-2;goto fail;}native_run=1;
                if(scalar(&b,c)){rc=-2;goto fail;}
            } else if(mode){
                if(label&&(!terminal||matched>end)){
                    if(say(&b,label)){rc=-2;goto fail;}end=matched;
                } else if(terminal){if(reading(&b,&extension)){rc=-2;goto fail;}}
                else {char temporary[NAME_CAPACITY];const char *s=language_name(c,locale,temporary);
                    if(s?say(&b,s):language_unknown(&b,c,locale)){rc=-2;goto fail;}}
            } else {
                int failed=0;
                if((c<32&&c!=9&&c!=10&&c!=13)||(c>=0x7f&&c<=0x9f)||
                   c==0x1680||(c>=0x2000&&c<=0x200a)||c==0x2028||c==0x2029||
                   c==0x202f||c==0x205f||c==0x3000)c=' ';
                if(c==0x2010||c==0x2011||c==0x2212)c='-';
                if(c==0xfe0e||c==0xfe0f||c==0x200b||c==0x200c||c==0x200d||c==0x2060||c==0xfeff){}
                else if(c=='`'&&!tags)failed=add(&b," ` ",3);
                else if(cp1252(c)||language_owns(c,language))failed=scalar(&b,c);
                else if(english==3&&!(c>=0x1f000&&c<=0x1faff))failed=scalar(&b,c);
                else if(l)failed=add(&b,STR(l->fold),strlen(STR(l->fold)));
                else {
                    size_t lo=0,hi=COUNT(unicode_compat);
                    while(lo<hi){size_t m=lo+(hi-lo)/2;if(unicode_compat[m].cp<c)lo=m+1;else hi=m;}
                    if(lo<COUNT(unicode_compat)&&unicode_compat[lo].cp==c)failed=add(&b,STR(unicode_compat[lo].fold),strlen(STR(unicode_compat[lo].fold)));
                    else if(c>=0x370&&c<=0x3ff){char key[5]={0};encode((unsigned char *)key,c);
                        const char *s=language_label(key,locale);failed=s?say(&b,s):add(&b," ",1);}
                    else failed=add(&b," ",1);
                }
                if(failed){rc=-2;goto fail;}
            }
            offsets[begin]=old;for(size_t j=begin+1;j<end;++j)offsets[j]=b.n;
            offsets[end]=b.n;i=end;continue;
        }
        if(english==2||english==3) {
            /* New western modules own their character names and spelling
             * through the native character mode. Never inject German or
             * English phonemes into another language. Compose accents
             * and retain the same source-boundary map as the other paths. */
            if(native_run&&!mode) {
                if(add(&b," `ts0 ",6)){rc=-2;goto fail;}
                native_run=0;
            }
            old=b.n;
            if(mode&&!native_run) {
                if(add(&b," `ts2 ",6)){rc=-2;goto fail;}
                native_run=1;
            }
            if((c<32&&c!=9&&c!=10&&c!=13)||(c>=0x7f&&c<=0x9f)||
               c==0x1680||(c>=0x2000&&c<=0x200a)||c==0x2028||c==0x2029||
               c==0x202f||c==0x205f||c==0x3000)c=' ';
            if(c==0x2010||c==0x2011||c==0x2212)c='-';
            const struct unicode_latin *l=latin(c);
            /* Separate a literal user backtick from any following tag
             * letters before native character-mode annotations are added.
             * The renderer may then accept those internal annotations. */
            int failed=c=='`'&&!tags?add(&b," ` ",3):(english==3||cp1252(c))?scalar(&b,c):l?
                add(&b,STR(l->fold),strlen(STR(l->fold))):add(&b," ",1);
            if(failed){rc=-2;goto fail;}
            offsets[begin]=old;
            for(size_t j=begin+1;j<end;++j)offsets[j]=b.n;
            offsets[end]=b.n;i=end;continue;
        }
        const char *label=NULL;size_t matched=0;
        if(mode) {
            char key[160];size_t used=0,p=i;
            /* Match names against the composed prefix too. Otherwise a
             * named bare kana (ka) wins over ka+dakuten even though c has
             * already been composed to ga. The consumed mark must not be
             * discarded by that shorter match. */
            if(end>i+n) {
                used=encode((unsigned char *)key,c);key[used]=0;p=end;
                label=cldr(key,english);
                if(label)matched=end;
            }
            while(p<length&&p-i<sizeof key-5&&(!modes||modes[p]==mode)) {
                size_t z;uint32_t next;if(decode(text+p,length-p,&z,&next))break;
                if(next!=0xfe0f){memcpy(key+used,text+p,z);used+=z;}
                key[used]=0;p+=z;
                const char *candidate=used?cldr(key,english):NULL;
                if(candidate){label=candidate;matched=p;}
            }
        }
        /* A keycap is one silent symbol in running text, including its digit. */
        if(!mode&&strchr("0123456789#*",(int)c)&&c<128&&end<length) {
            size_t p=end,z;uint32_t next;
            if(!decode(text+p,length-p,&z,&next)&&next==0xfe0f)p+=z;
            if(p<length&&!decode(text+p,length-p,&z,&next)&&next==0x20e3) {
                end=p+z;c=' ';
            }
        }
        struct reloq_character_reading native_reading;
        int native=mode&&(!label||matched<=end)&&
            reloq_character_read_native(c,english,&native_reading)==1;
        int raw=mode&&!english&&c!='`'&&(!label||matched<=end)&&
            reloq_character_native_available(c,0);
        if(native_run&&!raw) {
            if(add(&b," `ts0 ",6)){rc=-2;goto fail;}
            native_run=0;
        }
        old=b.n;
        if(raw) {
            /* Preserve original character-run context. Literal backticks
             * and Unicode extensions remain on the escaped/name paths. */
            if(!native_run&&add(&b," `ts2 ",6)){rc=-2;goto fail;}
            native_run=1;
            if(scalar(&b,c)){rc=-2;goto fail;}
        } else if(label&&!native){if(say(&b,label)){rc=-2;goto fail;}end=matched;}
        else if(one(&b,c,mode,english,tags)){rc=-2;goto fail;}
        offsets[begin]=old;
        for(size_t j=begin+1;j<end;++j)offsets[j]=b.n;
        offsets[end]=b.n;i=end;
    }
    if(native_run&&add(&b," `ts0 ",6)){rc=-2;goto fail;}
    if(add(&b,"",0)){rc=-2;goto fail;}
    offsets[length]=b.n;*output=b.p;*written=b.n;*map=offsets;return 0;
fail:free(offsets);free(b.p);return rc;
}
int reloq_unicode_admit(const unsigned char *text,size_t length,
    const unsigned char *modes,int english,int tags,unsigned char **output,
    size_t *written,size_t **map)
{
    return unicode_admit(text,length,modes,english,0,tags,output,written,map);
}
int reloq_unicode_admit_language(const unsigned char *text,size_t length,
    const unsigned char *modes,int language,int tags,unsigned char **output,
    size_t *written,size_t **map)
{
    if(language==0x40000||language==0x10000)return unicode_admit(text,length,modes,language==0x10000,0,tags,output,written,map);
    if(reloq_character_locale(language)<0)return -1;
    int family=language>>16,policy=family==6||family==8||family==10||family==18?3:2;
    return unicode_admit(text,length,modes,policy,language,tags,output,written,map);
}
