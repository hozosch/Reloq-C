/* Unicode text analysis for the native Chinese and Korean rule modules. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "eci_rom.h"
#include "evv_abi.h"
#include "reloq_pinyin.h"
#include "reloq_inline_tags.h"

extern void *cpp_new(uint32_t);
extern void cpp_delete(void *);
#include "reloq_cjk_data.inc"
#include "reloq_korean_extra.inc"
#define COUNT(a) (sizeof(a)/sizeof *(a))
struct text_buffer {char *data;size_t length,capacity;};
struct cjk_romanizer {
    EvvRom base;
    int language,parameters[16],extra[4],error,stopped;
    struct text_buffer input,output,wide;
};
static int append(struct text_buffer *b,const void *text,size_t length)
{
    if(length>UINT32_MAX-b->length-1)return -1;
    size_t need=b->length+length+1;
    if(need>b->capacity) {
        size_t capacity=b->capacity?b->capacity:256;
        while(capacity<need)capacity=capacity>UINT32_MAX/2?need:capacity*2;
        char *p=cpp_new((uint32_t)capacity);if(!p)return -1;
        if(b->length)memcpy(p,b->data,b->length);
        cpp_delete(b->data);b->data=p;b->capacity=capacity;
    }
    if(length)memcpy(b->data+b->length,text,length);
    b->length+=length;b->data[b->length]=0;return 0;
}
static int say(struct text_buffer *b,const char *s)
{return append(b,s,strlen(s));}
static size_t decode(const unsigned char *p,size_t size,uint32_t *cp)
{
    if(!size)return 0;
    unsigned c=p[0],n=c<128?1:c>=0xc2&&c<0xe0?2:c>=0xe0&&c<0xf0?3:c>=0xf0&&c<0xf5?4:0;
    if(!n||n>size)return 0;
    uint32_t value=n==1?c:c&((1u<<(7-n))-1);
    for(unsigned i=1;i<n;++i){if((p[i]&0xc0)!=0x80)return 0;value=(value<<6)|(p[i]&63);}
    if((n==2&&value<128)||(n==3&&value<2048)||(n==4&&value<65536)||
       value>0x10ffff||(value>=0xd800&&value<=0xdfff))return 0;
    *cp=value;return n;
}
static size_t encode(char out[4],uint32_t cp)
{
    if(cp<128){out[0]=(char)cp;return 1;}
    if(cp<2048){out[0]=(char)(0xc0|(cp>>6));out[1]=(char)(0x80|(cp&63));return 2;}
    if(cp<65536){out[0]=(char)(0xe0|(cp>>12));out[1]=(char)(0x80|((cp>>6)&63));out[2]=(char)(0x80|(cp&63));return 3;}
    out[0]=(char)(0xf0|(cp>>18));out[1]=(char)(0x80|((cp>>12)&63));out[2]=(char)(0x80|((cp>>6)&63));out[3]=(char)(0x80|(cp&63));return 4;
}
static uint32_t chinese_form(uint32_t cp)
{
    size_t lo=0,hi=COUNT(chinese_variants);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(chinese_variants[m].from<cp)lo=m+1;else hi=m;}
    return lo<COUNT(chinese_variants)&&chinese_variants[lo].from==cp?chinese_variants[lo].to:cp;
}
static const struct chinese_word *find_word(const char *key)
{
    size_t lo=0,hi=COUNT(chinese_words);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(strcmp((const char *)chinese_strings+chinese_words[m].word,key)<0)lo=m+1;else hi=m;}
    return lo<COUNT(chinese_words)&&!strcmp((const char *)chinese_strings+chinese_words[lo].word,key)?chinese_words+lo:NULL;
}
static int ideograph(uint32_t c)
{return (c>=0x3400&&c<=0x9fff)||(c>=0xf900&&c<=0xfaff);}
static int punctuation(uint32_t cp)
{
    switch(cp){
    case 0x3002:case 0xff0e:return '.';
    case 0x3001:case 0xff0c:return ',';
    case 0xff01:return '!';case 0xff1f:return '?';
    case 0xff1a:return ':';case 0xff1b:return ';';
    case 0xff08:return '(';case 0xff09:return ')';
    case 0x300c:case 0x300d:case 0x300e:case 0x300f:return '"';
    case 0x3000:return ' ';default:return 0;
    }
}
static uint32_t width_fold(uint32_t cp)
{return cp>=0xff01&&cp<=0xff5e?cp-0xfee0:cp;}
static int ascii_letter(uint32_t cp)
{return(cp>='a'&&cp<='z')||(cp>='A'&&cp<='Z');}
/* The foreign-word property includes its internal digits and punctuation.
 * Classifying each letter run separately split Wi-Fi, ABC123def and email
 * addresses into different native word properties. A change between full
 * width and ordinary Korean Latin input starts a new property. */
static size_t latin_token(struct cjk_romanizer *r,const unsigned char *text,
    size_t n,int korean,int *wide)
{
    r->wide.length=0;size_t i=0;int first=1,wide_number=0;
    while(i<n){
        uint32_t source;size_t z=decode(text+i,n-i,&source);if(!z)return 0;
        uint32_t cp=width_fold(source);int full=source!=cp;
        if(!korean) {
            if(cp==0x2018||cp==0x2019)cp='\'';
            else if(cp==0x201c||cp==0x201d||cp==0x300c||cp==0x300d||
                    cp==0x300e||cp==0x300f)cp='"';
        }
        if(cp<=32||cp>=127||cp=='`')break;
        if(first){*wide=full;first=0;}
        if(korean){if(ascii_letter(cp)&&full!=*wide)break;}
        else {
            if(cp==','||cp==';'||cp=='!'||cp=='?'||
               (i&&(cp=='('||cp=='['||cp=='{'||cp=='<')))break;
            /* A signed numeral closes the original Latin property. */
            if(cp=='-'&&i){uint32_t following=0;if(i+z<n)decode(text+i+z,n-i-z,&following);following=width_fold(following);if(following>='0'&&following<='9')break;}
            const unsigned previous=r->wide.length?(unsigned char)r->wide.data[r->wide.length-1]:0;
            if(cp>='0'&&cp<='9') {
                if(full&&previous==':')wide_number=1;
                if(*wide&&!wide_number)break;
                /* A number following punctuation starts its own native
                 * number token. Full-width colon numbers stay together. */
                if(!wide_number&&(previous==':'||previous=='%'||previous=='='||previous=='.'||
                   (previous&&strchr("_+#$^~*|",(int)previous))))break;
            }
        }
        char c=(char)cp;if(append(&r->wide,&c,1))return 0;i+=z;
    }
    return i;
}
static int chinese_literal_letters(const struct text_buffer *word)
{
    unsigned letters=0,capitals=0;
    for(size_t i=0;i<word->length;++i){unsigned c=(unsigned char)word->data[i];letters+=ascii_letter(c);capitals+=c>='A'&&c<='Z';}
    if(letters==1)return 1;
    /* Two capital initials stay with the Chinese letter-name grammar.
       Longer abbreviations belong to the original English property. */
    size_t first=0,last=word->length;
    while(first<last&&strchr("([{<\"",word->data[first]))++first;
    while(last>first&&!ascii_letter((unsigned char)word->data[last-1])&&
          !(word->data[last-1]>='0'&&word->data[last-1]<='9'))--last;
    return letters==2&&capitals==2&&
        (last-first==2||(last-first==3&&word->data[last-1]=='.'));
}
/* Foreign properties retain their own width changes and symbol boundaries.
 * These are romanizer properties, not caller-side language switches. */
static int chinese_operator(unsigned cp)
{return cp>0&&cp<128&&strchr("_+#$^~*|",(int)cp)!=NULL;}
static int chinese_symbol(struct cjk_romanizer *r,unsigned cp,int *previous)
{
    if(*previous==2&&say(&r->output," `p40 "))return -1;
    else if(*previous==1&&say(&r->output," "))return -1;
    const char *escape=cp=='|'?"\\|":cp=='^'?"\\\\^":NULL;
    char literal=(char)cp;
    if(escape?say(&r->output,escape):append(&r->output,&literal,1))return -1;
    *previous=1;return 0;
}
static int chinese_latin(struct cjk_romanizer *r,const unsigned char *text,
    size_t length,int foreign,int *previous)
{
    int active=0,last_width=-1;
    for(size_t i=0;i<length;) {
        uint32_t source;size_t z=decode(text+i,length-i,&source);if(!z)return -1;
        uint32_t cp=width_fold(source);int full=source!=cp;
        if(cp==0x2018||cp==0x2019)cp='\'';
        else if(cp==0x201c||cp==0x201d||cp==0x300c||cp==0x300d||cp==0x300e||cp==0x300f)cp='"';
        if(chinese_operator(cp)) {
            if(chinese_symbol(r,cp,previous))return -1;
            active=0;last_width=-1;i+=z;continue;
        }
        const int changed=foreign&&ascii_letter(cp)&&last_width>=0&&last_width!=full;
        if(!active||changed) {
            if(foreign) {
                if(!active&&*previous==1&&say(&r->output," `p40 "))return -1;
                if(say(&r->output," `g9_"))return -1;
            } else if(!active&&*previous==1&&say(&r->output," "))return -1;
            *previous=foreign?2:1;active=1;
        }
        if(ascii_letter(cp))last_width=full;
        else if(!(cp>='0'&&cp<='9'))last_width=-1;
        if(cp=='\\'&&foreign) {
            if(say(&r->output,"\\\\\\\\"))return -1;
        } else {char c=(char)cp;if(append(&r->output,&c,1))return -1;}
        i+=z;
    }
    return 0;
}
/* The original numeral property precedes a Chinese lexical token. It
 * expands two digits above nine, and four digits up to 3000; other numbers
 * stay available to the native Chinese number grammar. */
static int chinese_number(struct cjk_romanizer *r,const unsigned char *text,
    size_t size,size_t start,size_t *end,int previous)
{
    static const char *const digits[]={"ling2","yi1","er4","san1","si4","wu3","liu4","qi1","ba1","jiu3"};
    size_t at=start;unsigned value=0;r->wide.length=0;
    while(at<size) {
        uint32_t cp;size_t z=decode(text+at,size-at,&cp);if(!z)return -1;
        cp=width_fold(cp);if(cp<'0'||cp>'9')break;
        if(r->wide.length<4)value=value*10+cp-'0';
        char c=(char)cp;if(append(&r->wide,&c,1))return -1;at+=z;
    }
    *end=at;
    while(at<size&&(text[at]==' '||text[at]=='\t'||text[at]=='\n'||text[at]=='\r'))++at;
    uint32_t next=0;if(at<size&&!decode(text+at,size-at,&next))return -1;
    int lexical=0;
    if(ideograph(next)) {
        char key[5];size_t n=encode(key,chinese_form(next));key[n]=0;
        lexical=find_word(key)!=NULL;
    }
    int expand=lexical&&((r->wide.length==2&&value>9)||
                         (r->wide.length==4&&value<=3000));
    if(previous==2&&say(&r->output," `p40 "))return -1;
    if(expand) {
        if(say(&r->output," `gD_"))return -1;
        for(size_t i=0;i<r->wide.length;++i) {
            unsigned digit=(unsigned)(r->wide.data[i]-'0');
            const char *reading=r->wide.length==2&&i==0&&digit==1?"shi2":digits[digit];
            if(say(&r->output,reading))return -1;
        }
        return 0;
    }
    const char last=r->output.length?r->output.data[r->output.length-1]:0;
    if(previous==1&&last!='+'&&last!='-'&&say(&r->output," "))return -1;
    return append(&r->output,r->wide.data,r->wide.length);
}
/* The original chooses the English property for a Latin phrase, including
 * short initials beside another Latin word. Pinyin and bare numbers close
 * that phrase; ordinary spaces and soft punctuation remain inside it. */
static size_t chinese_latin_phrase(const unsigned char *text,size_t n,unsigned *words)
{
    size_t i=0;int inside=0;*words=0;
    while(i<n) {
        uint32_t cp;size_t used=decode(text+i,n-i,&cp);if(!used)break;
        cp=width_fold(cp);
        if(cp>127||cp=='`'||cp==','||cp==';'||cp=='.'||cp=='!'||cp=='?')break;
        if(ascii_letter(cp)) {
            if(!inside) {
                if(reloq_pinyin_prefix(text+i,n-i))break;
                ++*words;inside=1;
            }
        } else if(cp>='0'&&cp<='9') {if(!inside||(i&&strchr(":.%=_+#$^~*|",text[i-1])))break;}
        else if(cp==' '||cp=='\t'||cp=='\n'||cp=='\r'||cp=='\f'||cp=='\v')inside=0;
        i+=used;
    }
    return i;
}
static int chinese(struct cjk_romanizer *r,const unsigned char *text,size_t n)
{
    int previous_word=0,phrase_english=0,spelling=0;size_t phrase_end=0;
    for(size_t i=0;i<n;) {
        uint32_t cp;size_t used=decode(text+i,n-i,&cp);if(!used)return -1;
        if(cp=='`') {
            struct reloq_inline_tag tag=reloq_inline_tag_read(text+i,n-i,0);
            if(tag.kind==RELOQ_TAG_SPELL){spelling=tag.value;phrase_end=0;phrase_english=0;}
            size_t end=i+1;
            if(end<n&&text[end]=='['){while(end<n&&text[end]!=']')++end;if(end<n)++end;}
            else while(end<n&&text[end]>32&&text[end]<128)++end;
            if(append(&r->output,text+i,end-i))return -1;
            i=end;continue;
        }
        if(spelling&&(ascii_letter(width_fold(cp))||
           (width_fold(cp)>='0'&&width_fold(cp)<='9'))) {
            /* A character run is not a foreign lexical word or a Pinyin
               token. Reuse the isolated-letter property, one scalar at a
               time, while the native character grammar supplies its name. */
            /* Lowercase g has the unit reading for gram in the Chinese
               text grammar. Character navigation needs the letter-name
               entry shared with G; ordinary measurements keep their unit. */
            static const unsigned char letter_g='G';
            const unsigned char *character=width_fold(cp)=='g'?&letter_g:text+i;
            if(chinese_latin(r,character,width_fold(cp)=='g'?1:used,0,&previous_word))return -1;
            i+=used;continue;
        }
        if(ideograph(cp)) {
            /* The original romanizer marks its own script boundary with
               forty milliseconds; this is independent of pause policy. */
            if(previous_word==2&&say(&r->output," `p40 "))return -1;
            previous_word=1;
            char key[8*4+1];size_t at=0,p=i,end=i+used,best_end=end;
            const struct chinese_word *best=NULL;
            for(unsigned count=0;count<8&&p<n;++count) {
                uint32_t next;size_t z=decode(text+p,n-p,&next);if(!z||!ideograph(next))break;
                at+=encode(key+at,chinese_form(next));key[at]=0;p+=z;
                const struct chinese_word *word=find_word(key);
                if(word){best=word;best_end=p;}
            }
            if(best) {
                char tag[8];snprintf(tag,sizeof tag," `g%c_",best->category);
                if(say(&r->output,tag)||say(&r->output,(const char *)chinese_strings+best->reading))return -1;
            } else if(say(&r->output," "))return -1;
            i=best_end;continue;
        }
        uint32_t following=0;
        if(i+used<n)decode(text+i+used,n-i-used,&following);
        const int enclosing=(cp=='('||cp=='['||cp=='{'||cp=='<'||cp=='"'||
            cp==0x201c||cp==0x300c||cp==0x300e)&&ascii_letter(width_fold(following))&&
            !reloq_pinyin_prefix(text+i+used,n-i-used);
        if(ascii_letter(width_fold(cp))||enclosing) {
            size_t pinyin=cp<128&&!enclosing?reloq_pinyin_prefix(text+i,n-i):0;
            if(pinyin){
                if(previous_word==2&&say(&r->output," `p40 "))return -1;
                if(append(&r->output,text+i,pinyin))return -1;
                previous_word=1;i+=pinyin;continue;
            }
            int full=0;size_t z=latin_token(r,text+i,n-i,0,&full);if(!z)return -1;
            if(i>=phrase_end) {unsigned words=0;phrase_end=i+chinese_latin_phrase(text+i,n-i,&words);phrase_english=words>1;}
            int literal=!phrase_english&&chinese_literal_letters(&r->wide);
            if(chinese_latin(r,text+i,z,!literal,&previous_word))return -1;
            i+=z;continue;
        }
        if(cp=='\''&&previous_word==1&&i+used<n&&
           reloq_pinyin_prefix(text+i+used,n-i-used)) {
            if(say(&r->output,"' "))return -1;
            i+=used;continue;
        }
        int p=punctuation(cp);
        cp=width_fold(cp);
        if(p)cp=(unsigned)p;
        if(cp=='-'&&previous_word&&width_fold(following)>='0'&&width_fold(following)<='9') {
            if(say(&r->output,previous_word==2?" `p40  ":" ")||say(&r->output,"-"))return -1;
            previous_word=1;i+=used;continue;
        }
        if(cp>='0'&&cp<='9') {
            size_t end;if(chinese_number(r,text,n,i,&end,previous_word))return -1;
            previous_word=1;i=end;continue;
        } else if(chinese_operator(cp)){
            if(chinese_symbol(r,cp,&previous_word))return -1;
            i+=used;continue;
        } else if(cp!=' '&&cp!='\t'&&cp!='\n'&&cp!='\r')previous_word=0;
        if(cp<128) {char c=(char)cp;if(append(&r->output,&c,1))return -1;}
        else if(say(&r->output," "))return -1;
        i+=used;
    }
    return 0;
}
/* Bits one and two describe the left and right edges of the original word,
 * independently of its script. Adjacent script properties share that word;
 * a space closes it. Keep the flags on the native properties themselves. */
static int korean_property(struct cjk_romanizer *r,char category,size_t *active)
{
    unsigned flags=3;
    if(*active!=SIZE_MAX) {
        r->output.data[*active]=(char)('0'+((r->output.data[*active]-'0')&1));
        flags=2;
    }
    char tag[8];snprintf(tag,sizeof tag," `g%c%u_",category,flags);
    size_t offset=r->output.length+4;
    if(say(&r->output,tag))return -1;
    *active=offset;return 0;
}
static int korean(struct cjk_romanizer *r,const unsigned char *text,size_t n)
{
    int kind=0;size_t active=SIZE_MAX;
    for(size_t i=0;i<n;) {
        uint32_t cp;size_t z=decode(text+i,n-i,&cp);if(!z)return -1;
        if(cp=='`') {
            size_t end=i+1;
            if(end<n&&text[end]=='['){while(end<n&&text[end]!=']')++end;if(end<n)++end;}
            else while(end<n&&text[end]>32&&text[end]<128)++end;
            if(append(&r->output,text+i,end-i))return -1;
            i=end;kind=0;active=SIZE_MAX;continue;
        }
        if(ascii_letter(width_fold(cp))){
            int full=0;size_t length=latin_token(r,text+i,n-i,1,&full);if(!length)return -1;
            if(korean_property(r,'A',&active)||append(&r->output,r->wide.data,r->wide.length))return -1;
            i+=length;kind=2;continue;
        }
        if(cp>='0'&&cp<='9'){
            size_t end=i+z;
            while(end<n&&text[end]>='0'&&text[end]<='9')++end;
            uint32_t next=0;if(end<n)decode(text+end,n-end,&next);
            int next_kind=(next>=0xac00&&next<=0xd7a3)?1:ideograph(next)?4:0;
            if(kind==1||kind==4||next_kind){
                if(kind!=1&&kind!=4) {
                    if(korean_property(r,next_kind==4?'J':'H',&active))return -1;
                    kind=next_kind;
                }
                if(append(&r->output,text+i,end-i))return -1;
                i=end;continue;
            }
            uint32_t after_hyphen=0;if(next=='-'&&end+1<n)decode(text+end+1,n-end-1,&after_hyphen);
            if(ascii_letter(width_fold(next))||(next=='-'&&ascii_letter(width_fold(after_hyphen)))) {
                int full=0;size_t length=latin_token(r,text+i,n-i,1,&full);if(!length)return -1;
                if(korean_property(r,'A',&active)||append(&r->output,r->wide.data,r->wide.length))return -1;
                i+=length;kind=2;continue;
            }
        }
        int hanja=0;
        if(ideograph(cp)){
            size_t lo=0,hi=COUNT(korean_hanja);
            while(lo<hi){size_t m=lo+(hi-lo)/2;if(korean_hanja[m].cp<cp)lo=m+1;else hi=m;}
            if(lo<COUNT(korean_hanja)&&korean_hanja[lo].cp==cp){cp=korean_hanja[lo].syllable;hanja=1;}
        }
        if(cp>=0xac00&&cp<=0xd7a3) {
            int next_kind=hanja?4:1;
            if(kind!=next_kind&&korean_property(r,hanja?'J':'H',&active))return -1;
            unsigned syllable=cp-0xac00;
            if(say(&r->output,korean_onsets[syllable/588])||say(&r->output,korean_vowels[syllable/28%21])||say(&r->output,korean_codas[syllable%28]))return -1;
            kind=next_kind;
        } else if(cp>=0x3131&&cp<=0x318e) {
            size_t lo=0,hi=COUNT(korean_jamo);
            while(lo<hi){size_t m=lo+(hi-lo)/2;if(korean_jamo[m].cp<cp)lo=m+1;else hi=m;}
            if(lo<COUNT(korean_jamo)&&korean_jamo[lo].cp==cp){if(say(&r->output," `gL3_")||say(&r->output,korean_jamo[lo].reading))return -1;}
            kind=3;active=SIZE_MAX;
        } else {
            int p=punctuation(cp);cp=width_fold(cp);if(p)cp=(unsigned)p;
            if(cp<128) {
                int letter=(cp>='A'&&cp<='Z')||(cp>='a'&&cp<='z');
                if(letter&&kind!=2&&korean_property(r,'A',&active))return -1;
                char c=(char)cp;if(append(&r->output,&c,1))return -1;
                kind=letter?2:0;if(!letter)active=SIZE_MAX;
            } else {if(say(&r->output," "))return -1;kind=0;active=SIZE_MAX;}
        }
        i+=z;
    }
    return 0;
}
static void release(EvvRom *base)
{
    struct cjk_romanizer *r=(void *)base;
    cpp_delete(r->input.data);cpp_delete(r->output.data);cpp_delete(r->wide.data);cpp_delete(r);
}
static int32_t add_text(EvvRom *base,const char *s,int32_t n,int32_t flag)
{
    struct cjk_romanizer *r=(void *)base;(void)flag;
    if(n<0||(!s&&n))return 0;
    if(append(&r->input,s,(size_t)n)){r->error=1;return 0;}return 1;
}
static int32_t add_parameter(EvvRom *r,const char *s,int32_t n)
{return add_text(r,s,n,1);}
static int32_t insert_index(EvvRom *r){return add_text(r," `ui ",5,1);}
static int32_t process(EvvRom *base,char **out,int32_t annotated)
{
    struct cjk_romanizer *r=(void *)base;(void)annotated;*out=NULL;
    if(!r->input.length)return 0;
    r->output.length=0;
    int result=r->language==10?korean(r,(const unsigned char *)r->input.data,r->input.length):chinese(r,(const unsigned char *)r->input.data,r->input.length);
    r->input.length=0;if(r->input.data)r->input.data[0]=0;
    if(result){r->error=1;return -1;}
    *out=r->output.data;return 2;
}
static int32_t stop(EvvRom *base){((struct cjk_romanizer *)base)->stopped=1;return 1;}
static int32_t resume(EvvRom *base){((struct cjk_romanizer *)base)->stopped=0;return 1;}
static int32_t wide_text(EvvRom *base,const uint16_t *s,char **out,int32_t n)
{
    struct cjk_romanizer *r=(void *)base;(void)n;r->wide.length=0;
    for(size_t i=0;s&&s[i];++i){uint32_t c=s[i];if(c>=0xd800&&c<=0xdbff&&s[i+1]>=0xdc00&&s[i+1]<=0xdfff){c=0x10000+((c-0xd800)<<10)+(s[++i]-0xdc00);}char b[4];if(append(&r->wide,b,encode(b,c)))return -1;}
    *out=r->wide.data;return 0;
}
static int32_t get_parameter(EvvRom *base,int32_t which)
{
    struct cjk_romanizer *r=(void *)base;
    return which>=0&&which<16?r->parameters[which]:which>=1000&&which<1004?r->extra[which-1000]:-1;
}
static int32_t set_parameter(EvvRom *base,int32_t which,int32_t value)
{
    struct cjk_romanizer *r=(void *)base;int old=get_parameter(base,which);
    if(which>=0&&which<16)r->parameters[which]=value;
    else if(which>=1000&&which<1004)r->extra[which-1000]=value;
    return old;
}
static void clear_errors(EvvRom *base){((struct cjk_romanizer *)base)->error=0;}
static uint32_t status(EvvRom *base){return (uint32_t)((struct cjk_romanizer *)base)->error;}
static void error_message(EvvRom *base,char *out){(void)base;if(out)strcpy(out,"Text conversion failed");}
static const EvvRomOps operations={.release=release,.addText=add_text,.insertIndex=insert_index,.processSentence=process,.stop=stop,.resume=resume,.UCS2ToMBCS=wide_text,.setParam=set_parameter,.getParam=get_parameter,.clearErrors=clear_errors,.progStatus=status,.errorMessage=error_message,.addParam=add_parameter};
static EvvRom *create(int language)
{
    struct cjk_romanizer *r=cpp_new((uint32_t)sizeof *r);if(!r)return NULL;
    memset(r,0,sizeof *r);r->base.ops=&operations;r->language=language;r->parameters[2]=language<<16;r->parameters[3]=1;return &r->base;
}
EvvRom *reloq_chinese_romanizer(const char *dir){(void)dir;return create(6);}
EvvRom *reloq_korean_romanizer(const char *dir){(void)dir;return create(10);}
