/* Unicode text analysis for the native Chinese and Korean rule modules. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "eci_rom.h"
#include "evv_abi.h"
#include "reloq_pinyin.h"
#include "reloq_inline_tags.h"
#include "reloq61_latin.h"

extern void *cpp_new(uint32_t);
extern void cpp_delete(void *);
#include "reloq_cjk_data.inc"
#include "reloq_chinese_traditional.inc"
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
static const struct traditional_reading *traditional_identity(uint32_t cp)
{
    size_t lo=0,hi=COUNT(traditional_readings);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(traditional_readings[m].scalar<cp)lo=m+1;else hi=m;}
    return lo<COUNT(traditional_readings)&&traditional_readings[lo].scalar==cp?traditional_readings+lo:NULL;
}
static int traditional_word(const char *reading,
    const struct traditional_reading *const identities[8],unsigned count)
{
    int restricted=0;
    for(unsigned i=0;i<count;++i)restricted|=identities[i]!=NULL;
    if(!restricted)return 1;
    const char *syllables[8],*p=reading;size_t lengths[8];
    for(unsigned i=0;i<count;++i) {
        syllables[i]=p;while(*p>='a'&&*p<='z')++p;
        /* Special lexical controls are left to their existing grammar. */
        if(p==syllables[i]||*p<'0'||*p>'5')return 1;
        lengths[i]=(size_t)(++p-syllables[i]);
    }
    if(*p)return 1;
    for(unsigned i=0;i<count;++i)if(identities[i]) {
        const char *option=identities[i]->readings;int found=0;
        while(*option) {
            const char *end=strchr(option,' ');size_t n=end?(size_t)(end-option):strlen(option);
            if(n==lengths[i]&&!memcmp(option,syllables[i],n-1)&&
               (option[n-1]==syllables[i][n-1]||syllables[i][n-1]=='0'||syllables[i][n-1]=='5')){found=1;break;}
            if(!end)break;option=end+1;
        }
        if(!found)return 0;
    }
    return 1;
}
static int ideograph(uint32_t c);
/* A standalone aspect-particle 了 follows a spoken word and has no dictionary
 * phrase of its own. Preserve lexical 了... phrases such as 了解 and 了结. */
static int prev_word_before(const unsigned char *text,size_t at)
{
    size_t start=at-1;
    uint32_t cp;
    if(!at)return 0;
    while(start && (text[start]&0xc0)==0x80)--start;
    if(start==at || !decode(text+start,at-start,&cp))return 0;
    return ideograph(cp) || (cp>='a'&&cp<='z') || (cp>='A'&&cp<='Z') ||
        (cp>='0'&&cp<='9');
}
/* Modern literary 血 is xuè in most compounds. The legacy lexicon widely
 * uses xiě; normalize those entries while preserving other syllables. */
static const char *blood_reading(const char *key,const char *reading,
    char *buffer,size_t capacity)
{
    if(!strstr(key,"\xe8\xa1\x80") || !strstr(reading,"xie3"))return reading;
    size_t n=0;
    for(size_t i=0;reading[i]&&n+1<capacity;) {
        if(!strncmp(reading+i,"xie3",4) && n+4<capacity) {
            memcpy(buffer+n,"xue4",4);n+=4;i+=4;
        } else buffer[n++]=reading[i++];
    }
    buffer[n]=0;return buffer;
}
/* A few phrase readings are systematically off in the legacy dictionary. */
static const char *phrase_reading(const char *key,const char *reading)
{
    static const struct {const char *word,*reading;} lexical_readings[]={
        {"名分","ming2fen4"}, {"天分","tian1fen4"},
        {"养分","yang3fen4"}, {"充分","chong1fen1"},
        {"命中","ming4zhong4"}, {"命中率","ming4zhong4lv4"},
        {"得去","dei3qu4"},
        {"着手","zhuo2shou3"},
        {"着重","zhuo2zhong4"},
        {"着想","zhuo2xiang3"},
        {"着眼","zhuo2yan3"},
        {"着装","zhuo2zhuang1"},
        {"着落","zhuo2luo4"},
        {"着实","zhuo2shi2"},
        {"重创","zhong4chuang1"},
        {"重复","chong2fu4"},
        {"重量","zhong4liang4"},
        {"折本","she2ben3"},
        {"复选框","fu4xuan3kuang4"},
        {"边框","bian1kuang4"},
        {"酒吧","jiu3ba1"},
    };
    for(size_t i=0;i<COUNT(lexical_readings);++i)
        if(!strcmp(key,lexical_readings[i].word))return lexical_readings[i].reading;
    if(!strcmp(key,"\xe7\xa9\xba\xe6\xa0\xbc") && !strcmp(reading,"kong1ge2"))
        return "kong4ge2";
    if(!strcmp(key,"\xe4\xb8\x9c\xe8\xa5\xbf") && !strcmp(reading,"dong1xi1"))
        return "dong1xi0";
    if(!strcmp(key,"\xe7\xb2\x98\xe8\xb4\xb4") && !strcmp(reading,"nian2tie1"))
        return "zhan1tie1";
    if(!strcmp(key,"\xe7\xb2\x98\xe4\xbd\x8f") && !strcmp(reading,"nian2zhu4"))
        return "zhan1zhu4";
    if(!strcmp(key,"\xe5\xa5\xbd\xe4\xba\x86") && !strcmp(reading,"hao3liao3"))
        return "hao3le0";
    if(!strcmp(key,"\xe5\x88\x86\xe5\xbc\x80"))
        return "fen1kai1";
    if(!strcmp(key,"\xe5\x9c\xa8\xe4\xba\x8e"))
        return "zai4yu2";
    if(!strcmp(key,"\xe7\xbc\x98\xe5\x88\x86"))
        return "yuan2fen4";
    if(!strcmp(key,"\xe4\xb8\x80\xe5\x8f\xb7"))
        return "yi1hao4";
    return reading;
}
static size_t next_lexeme(const unsigned char *text,size_t n,size_t at,
    char *category)
{
    char key[8*4+1];size_t key_size=0,p=at,best_end=0;
    *category=0;
    for(unsigned count=0;count<8&&p<n;++count) {
        uint32_t cp;size_t used=decode(text+p,n-p,&cp);
        if(!used||!ideograph(cp))break;
        key_size+=encode(key+key_size,chinese_form(cp));key[key_size]=0;p+=used;
        const struct chinese_word *word=find_word(key);
        if(word){best_end=p;*category=(char)word->category;}
    }
    return best_end;
}
static int modal_dei_after(const unsigned char *text,size_t n,size_t at,
    int require_adverb)
{
    char first_category=0,next_category=0;
    size_t first=next_lexeme(text,n,at,&first_category);
    if(!first)return 0;
    /* Modal predicates include states/adjectives, not only action verbs. */
    if(first_category=='V'||first_category=='a'||first_category=='3'||
       first_category=='e'||first_category=='M'||first_category=='A')
        return !require_adverb;
    if(first_category!='B'&&first_category!='d')return 0;
    return next_lexeme(text,n,first,&next_category)!=0 &&
        (next_category=='V'||next_category=='a'||next_category=='3');
}
static const char *modal_dei_reading(const char *key,const char *reading,
    char category,char *buffer,size_t capacity)
{
    size_t key_length=strlen(key),reading_length=strlen(reading);
    const char *syllable=strstr(reading,"de2");
    if((category!='B'&&category!='e') || key_length<3 ||
       memcmp(key+key_length-3,"得",3) || !syllable ||
       reading_length+1>capacity)return reading;
    size_t prefix=(size_t)(syllable-reading);
    memcpy(buffer,reading,prefix);
    memcpy(buffer+prefix,"dei3",4);
    strcpy(buffer+prefix+4,syllable+3);
    return buffer;
}
static const char *hit_reading(const char *key,const char *reading,
    char *buffer,size_t capacity)
{
    static const char hit[]="命中",rate[]="命中率";
    if(!key||!reading||strncmp(key,hit,sizeof(hit)-1)||
       strncmp(reading,"ming4zhong1",11))return reading;
    size_t n=strlen(reading);
    if(n+1>capacity)return reading;
    memcpy(buffer,reading,n+1);
    buffer[10]='4';
    if(!strncmp(key,rate,sizeof(rate)-1)) {
        static const char wrong[]="ming4zhong4shuai4";
        if(n>=sizeof(wrong)-1&&!memcmp(buffer,wrong,sizeof(wrong)-1)) {
            size_t suffix=n-(sizeof(wrong)-1);
            memmove(buffer+14,buffer+sizeof(wrong)-1,suffix+1);
            memcpy(buffer+11,"lv4",3);
        }
    }
    return buffer;
}
static int starts_with_at(const unsigned char *text,size_t size,size_t at,const char *value)
{
    size_t length=strlen(value);
    return at+length<=size&&!memcmp(text+at,value,length);
}
static int row_context(const unsigned char *text,size_t size,size_t at)
{
    static const char *const row_after[]={"号","號","数","數","列","首","尾","末"};
    static const char *const numeral_before[]={"一","二","两","兩","三","四","五","六","七","八","九","十"};
    static const char *const row_words[]={"文字","文本","数据","數據","内容","內容"};
    static const char *const current_row[]={"当前","當前","本","这一","這一","上一","下一"};
    static const char *const command_row[]={"命令","指令"};
    for(size_t i=0;i<COUNT(command_row);++i) {
        size_t length=strlen(command_row[i]);char category=0;
        /* Keep a real following word such as 行走 separate from 命令行. */
        if(at>=length&&!memcmp(text+at-length,command_row[i],length)&&
           next_lexeme(text,size,at,&category)<=at+strlen("行"))return 1;
    }
    for(size_t i=0;i<COUNT(row_after);++i)if(starts_with_at(text,size,at+strlen("行"),row_after[i]))return 1;
    for(size_t i=0;i<COUNT(current_row);++i) {
        size_t length=strlen(current_row[i]);
        if(at>=length&&!memcmp(text+at-length,current_row[i],length))return 1;
    }
    int numbered=at>0&&text[at-1]>='0'&&text[at-1]<='9';
    for(size_t i=0;i<COUNT(numeral_before)&&!numbered;++i) {
        size_t length=strlen(numeral_before[i]);
        numbered=at>=length&&!memcmp(text+at-length,numeral_before[i],length);
    }
    if(numbered)return 1;
    for(size_t i=0;i<COUNT(row_words);++i)
        if(starts_with_at(text,size,at+strlen("行"),row_words[i]))return 1;
    return 0;
}
static const char *row_reading(const unsigned char *text,size_t size,size_t at,
    const char *key,const char *reading,char *buffer,size_t capacity)
{
    size_t key_size=strlen(key),reading_size=strlen(reading),key_at=0,source_at=at;
    size_t character=0,target=SIZE_MAX;
    while(key_at<key_size) {
        uint32_t key_cp,source_cp;
        size_t key_step=decode((const unsigned char *)key+key_at,key_size-key_at,&key_cp);
        size_t source_step=decode(text+source_at,size-source_at,&source_cp);
        if(!key_step||!source_step)return reading;
        if(key_cp==0x884c&&row_context(text,size,source_at))target=character;
        key_at+=key_step;source_at+=source_step;++character;
    }
    if(target==SIZE_MAX)return reading;
    size_t which=0,reading_at=0,match=SIZE_MAX;
    while(reading_at<reading_size) {
        size_t start=reading_at;
        while(reading_at<reading_size&&reading[reading_at]>='a'&&reading[reading_at]<='z')++reading_at;
        if(reading_at==start)return reading;
        if(reading_at<reading_size&&reading[reading_at]>='0'&&reading[reading_at]<='5')++reading_at;
        if(which==target) {
            if(reading_at-start==5&&!memcmp(reading+start,"xing2",5))match=start;
            else return reading;
        }
        ++which;
    }
    if(which!=character||match==SIZE_MAX||reading_size+1>capacity)return reading;
    memcpy(buffer,reading,reading_size+1);buffer[match]='h';buffer[match+1]='a';return buffer;
}
/* A compound ending in a sentence particle takes the particle's neutral
 * tone. Restrict this to phrases longer than the particle itself so a
 * standalone exclamation keeps its own lexical tone. */
static const char *particle_reading(const char *key,const char *reading,
    char *buffer,size_t capacity)
{
    static const char *const particles[]={
        "\xe5\x91\x80","\xe5\x95\x8a","\xe5\x90\xa7","\xe5\x90\x97",
        "\xe5\x91\xa2","\xe5\x95\xa6","\xe5\x98\x9b","\xe5\x93\xa6",
        "\xe5\x93\x9f","\xe5\x91\x97","\xe5\x93\x88"
    };
    static const char *const lexical_particles[]={
        "\xe5\x93\x88\xe5\x93\x88", /* 哈哈 */
        "\xe9\x85\x92\xe5\x90\xa7", /* 酒吧 */
        "\xe7\xbd\x91\xe5\x90\xa7"  /* 网吧 */
    };
    if(!key||!reading)return reading;
    size_t key_length=strlen(key),reading_length=strlen(reading);
    for(size_t lexical=0;lexical<COUNT(lexical_particles);++lexical)
        if(key_length==strlen(lexical_particles[lexical])&&
           !memcmp(key,lexical_particles[lexical],key_length))
            return reading;
    for(size_t i=0;i<COUNT(particles);++i) {
        size_t particle_length=strlen(particles[i]);
        if(key_length<=particle_length ||
           key_length<particle_length ||
           memcmp(key+key_length-particle_length,particles[i],particle_length))
            continue;
        if(!reading_length||reading_length+1>capacity)return reading;
        if(reading[reading_length-1]>='1'&&reading[reading_length-1]<='4') {
            memcpy(buffer,reading,reading_length+1);
            buffer[reading_length-1]='0';
            return buffer;
        }
        return reading;
    }
    return reading;
}
/* 重 is "again" in action compounds and "heavy/important" elsewhere. Use
 * the following morpheme to choose the tone instead of enumerating every
 * word. */
static const char *chong_reading(const char *key,const char *reading,
    char *buffer,size_t capacity)
{
    static const char *const chong_seconds[]={
        "\xe7\xbd\xae", /* 置 */
        "\xe6\x9d\xa5", /* 来 */
        "\xe5\xa4\x8d", /* 复 */
        "\xe6\x96\xb0", /* 新 */
        "\xe5\x90\xaf", /* 启 */
        "\xe5\xbc\x80", /* 开 */
        "\xe5\xbb\xba", /* 建 */
        "\xe4\xbf\xae", /* 修 */
        "\xe8\x80\x83", /* 考 */
        "\xe8\xaf\xbb", /* 读 */
        "\xe5\x86\x99", /* 写 */
        "\xe5\x8d\xb0", /* 印 */
        "\xe5\x8a\x9e", /* 办 */
        "\xe7\xae\x97", /* 算 */
        "\xe6\x8e\x92", /* 排 */
        "\xe5\x81\x9a", /* 做 */
        "\xe6\xbc\x94", /* 演 */
        "\xe5\x94\xb1", /* 唱 */
        "\xe8\xb7\x91", /* 跑 */
        "\xe8\xa3\x85", /* 装 */
        "\xe5\x88\xb6", /* 制 */
        "\xe5\x8f\x91", /* 发 */
        "\xe7\x94\x9f", /* 生 */
        "\xe6\x9e\x84", /* 构 */
        "\xe5\x8f\xa0", /* 叠 */
        "\xe7\x94\xb3", /* 申 */
        "\xe6\xb8\xa1", /* 渡 */
        "\xe5\xa5\x8f", /* 奏 */
        "\xe7\xbb\x87"  /* 织 */
    };
    if(!key||!reading||strlen(key)<6||memcmp(key,"\xe9\x87\x8d",3))
        return reading;
    for(size_t i=0;i<COUNT(chong_seconds);++i)
        if(!memcmp(key+3,chong_seconds[i],strlen(chong_seconds[i]))) {
            if(strncmp(reading,"zhong4",6))return reading;
            if(strlen(reading)<6||strlen(reading)+1>capacity)return reading;
            memcpy(buffer,"chong2",6);
            strcpy(buffer+6,reading+6);
            return buffer;
        }
    return reading;
}
/* Reusable context rules for high-frequency polyphones. A rule fires only
 * when the dictionary did not already consume a longer phrase, so entries
 * such as 呢子 or 率先 keep their lexical readings. */
static const struct polyphone_rule {
    const char *left;
    const char *key;
    const char *right;
    const char *notRight;
    const char *reading;
} polyphone_rules[]={
    {NULL,"框",NULL,NULL,"kuang4"},
    {NULL,"还","款",NULL,"huan2"},
    {NULL,"还","贷",NULL,"huan2"},
    {NULL,"还","貸",NULL,"huan2"},
    {NULL,"分","内",NULL,"fen4"},
    {NULL,"分","內",NULL,"fen4"},
    {"\xe5\x8f\xaf","\xe6\x81\xb6",NULL,NULL,"wu4"},
    {NULL,"\xe5\x91\xa2",NULL,"\xe5\xad\x90","ne0"},
    {"\xe9\x87\x87\xe6\xa0\xb7","\xe7\x8e\x87",NULL,NULL,"luu4"},
    {NULL,"\xe9\x87\x8d","\xe8\xbe\x93",NULL,"chong2"},
    {NULL,"\xe5\xa4\xa7",NULL,"\xe5\xa4\xab","da4"},
    {NULL,"\xe7\xbb\x99",NULL,"\xe4\xba\x88","gei3"},
};
static int chong_action_after(const unsigned char *text,size_t n,size_t at)
{
    static const char *const actions[]={
        "\xe7\xbd\xae","\xe6\x9d\xa5","\xe8\xaf\x95","\xe8\xbe\x93",
        "\xe5\xa4\x8d","\xe6\x96\xb0","\xe5\x90\xaf","\xe5\xbc\x80",
        "\xe5\xbb\xba","\xe4\xbf\xae","\xe8\x80\x83","\xe8\xaf\xbb",
        "\xe5\x86\x99","\xe5\x8d\xb0","\xe5\x8a\x9e","\xe7\xae\x97",
        "\xe6\x8e\x92","\xe5\x81\x9a","\xe6\xbc\x94","\xe5\x94\xb1",
        "\xe8\xb7\x91","\xe8\xa3\x85","\xe5\x88\xb6","\xe5\x8f\x91",
        "\xe7\x94\x9f","\xe6\x9e\x84","\xe5\x8f\xa0","\xe7\x94\xb3",
        "\xe6\xb8\xa1","\xe5\xa5\x8f","\xe7\xbb\x87"
    };
    if(at+3>n)return 0;
    for(size_t i=0;i<COUNT(actions);++i)
        if(!memcmp(text+at,actions[i],3))return 1;
    return 0;
}
static const char *polyphone_reading(const unsigned char *text,size_t n,
    size_t at,const char *key,size_t best_end,size_t end,char previous_category,
    const char *reading)
{
    /* A rule may refine a one-character match. It must not replace a
     * multi-character phrase, or the remaining phrase would disappear. */
    if(best_end!=end)return reading;
    if(!strcmp(key,"\xe7\x9d\x80") && prev_word_before(text,at))
        return "zhe0";
    if(prev_word_before(text,at)) {
        if(!strcmp(key,"\xe5\x91\x80"))return "ya0";
        if(!strcmp(key,"\xe5\x95\x8a"))return "a0";
        if(!strcmp(key,"\xe5\x90\x97"))return "ma0";
        if(!strcmp(key,"\xe5\x90\xa7"))return "ba0";
        if(!strcmp(key,"\xe5\xbe\x97")) {
            char following_category=0;
            int modal_context=previous_category=='P'||previous_category=='B'||
                previous_category=='e'||(previous_category=='A'&&at>=3&&
                    !memcmp(text+at-3,"\xe9\x9d\x9e",3));
            if(modal_context&&
               (!next_lexeme(text,n,at+3,&following_category)||
                modal_dei_after(text,n,at+3,0)))return "dei3";
            if(previous_category=='V'&&modal_dei_after(text,n,at+3,1))
                return "dei3";
            return "de0";
        }
    }
    if(!strcmp(key,"\xe5\xbe\x97") &&
       modal_dei_after(text,n,at+3,0))return "dei3";
    if(!strcmp(key,"\xe4\xb8\xb6"))return "zhu3";
    if(!strcmp(key,"\xe5\x91\x80"))return "ya0";
    if(!strcmp(key,"\xe5\x95\xa6"))return "la0";
    if(!strcmp(key,"\xe9\x87\x8d") && chong_action_after(text,n,at+3))
        return "chong2";
    for(size_t i=0;i<COUNT(polyphone_rules);++i) {
        const struct polyphone_rule *rule=polyphone_rules+i;
        if(strcmp(key,rule->key))continue;
        size_t left=rule->left?strlen(rule->left):0;
        if(left && (at<left || memcmp(text+at-left,rule->left,left)))continue;
        size_t key_length=strlen(key);
        size_t after=at+key_length;
        size_t right=rule->right?strlen(rule->right):0;
        if(right && (after+right>n || memcmp(text+after,rule->right,right)))continue;
        size_t not_right=rule->notRight?strlen(rule->notRight):0;
        if(not_right && after+not_right<=n && !memcmp(text+after,rule->notRight,not_right))continue;
        return rule->reading;
    }
    return reading;
}
/* UI vocabulary often has no legacy dictionary entry. Group common compounds
 * as one word so the acoustic model does not insert a phrase break. */
static const struct phrase_override {const char *text;const char *reading;char category;} phrase_overrides[]={
    {"解锁","jie3suo3",'V'},
    {"解鎖","jie3suo3",'V'},
    {"\xe5\x8d\x95\xe9\x80\x89\xe6\x8c\x89\xe9\x92\xae","dan1xuan3an4niu3",'N'},
    {"\xe5\x93\x94\xe5\x93\xa9\xe5\x93\x94\xe5\x93\xa9","bi4li3bi4li3",'N'},
    {"\xe5\xa4\xa7\xe9\xbb\x84\xe8\x9c\x82","da4huang2feng1",'N'},
    {"\xe7\x85\xa7\xe7\x89\x87","zhao4pian4",'N'},
    {"\xe7\x9d\xa1\xe7\x9d\x80","shui4zhao2",'N'},
    {"\xe4\xb8\x80\xe5\x8f\xb7","yi1hao4",'N'}
};
static int phrase_group(const unsigned char *text,size_t n,size_t at,
    size_t *end,const char **reading,char *category)
{
    for(size_t i=0;i<COUNT(phrase_overrides);++i) {
        size_t length=strlen(phrase_overrides[i].text);
        if(at+length<=n && !memcmp(text+at,phrase_overrides[i].text,length)) {
            *end=at+length;*reading=phrase_overrides[i].reading;
            *category=phrase_overrides[i].category;return 1;
        }
    }
    return 0;
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
static int latin_letter(uint32_t cp)
{return reloq61_latin_byte(cp)!=0;}
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
        int letter=reloq61_latin_byte(cp);
        if(cp<=32||(cp>=127&&!letter)||cp=='`')break;
        if(first){*wide=full;first=0;}
        if(korean){if(letter&&full!=*wide)break;}
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
        char c=(char)(letter?letter:cp);if(append(&r->wide,&c,1))return 0;i+=z;
    }
    return i;
}
static int chinese_literal_letters(const struct text_buffer *word)
{
    unsigned letters=0,capitals=0;
    for(size_t i=0;i<word->length;++i){unsigned c=(unsigned char)word->data[i];if(c>=128)return 0;letters+=ascii_letter(c);capitals+=c>='A'&&c<='Z';}
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
/* Preserve token separation across scripts without inserting a timed pause.
 * Normal word and sentence boundaries still control their own durations. */
static const char chinese_english_boundary[]=" ";
static int chinese_symbol(struct cjk_romanizer *r,unsigned cp,int *previous)
{
    if(*previous==2&&say(&r->output,chinese_english_boundary))return -1;
    else if(*previous==1&&say(&r->output," "))return -1;
    const char *escape=cp=='|'?"\\|":cp=='^'?"\\\\^":NULL;
    char literal=(char)cp;
    if(escape?say(&r->output,escape):append(&r->output,&literal,1))return -1;
    *previous=1;return 0;
}
static int chinese_latin(struct cjk_romanizer *r,const unsigned char *text,
    size_t length,int foreign,int characters,int *previous)
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
        const int changed=foreign&&latin_letter(cp)&&last_width>=0&&last_width!=full;
        if(!active||changed) {
            if(foreign) {
                if(!active&&*previous==1&&say(&r->output,chinese_english_boundary))return -1;
                if(say(&r->output," `g9_"))return -1;
            } else if(!active&&*previous==1&&say(&r->output," "))return -1;
            *previous=foreign?2:1;active=1;
        }
        if(latin_letter(cp))last_width=full;
        else if(!(cp>='0'&&cp<='9'))last_width=-1;
        if(cp=='\\'&&foreign) {
            if(say(&r->output,"\\\\\\\\"))return -1;
        } else {
            int letter=reloq61_latin_byte(cp);
            /* Both character navigation and an isolated letter use the
               same alphabet entry. Numeric g units retain their grammar. */
            if(characters&&letter=='g')letter='G';
            char c=(char)(letter?letter:cp);if(append(&r->output,&c,1))return -1;
        }
        i+=z;
    }
    return 0;
}
static int pinyin_char(unsigned char c)
{return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='5');}
static int pinyin_tone(unsigned char c)
{return c>='0'&&c<='5';}
static int pinyin_boundary(unsigned char c)
{
    static const char *const marks=",.;:!?()[]{}<>\"'/\\|~@$%^&*-+=_";
    return c=='\n'||c=='\r'||c=='\f'||c=='\v'||strchr(marks,c)!=NULL;
}
/* Apply systematic Mandarin sandhi after the lexical choice has been made.
 * A 3+3 pair changes the first syllable and then closes the pair; it does
 * not keep chaining through the rest of the sentence. */
static void apply_tone_sandhi(struct text_buffer *out)
{
    if(!out->data)return;
    char *s=out->data;size_t n=out->length;
    size_t previous_tone=SIZE_MAX;int previous_value=0;
    const char *previous_base=NULL;size_t previous_base_length=0;
    const char *current_base=NULL;size_t current_base_length=0;
    for(size_t i=0;i<n;) {
        if(s[i]=='`' && i+3<n && s[i+1]=='g' &&
           ((s[i+2]>='a'&&s[i+2]<='z')||(s[i+2]>='A'&&s[i+2]<='Z')||
            (s[i+2]>='0'&&s[i+2]<='8')) &&
           s[i+3]=='_' && i+4<n && ((s[i+4]>='a'&&s[i+4]<='z')||
                                    (s[i+4]>='A'&&s[i+4]<='Z'))) {
            size_t at=i+4;
            size_t base_start=0,base_length=0;
            while(at<n && pinyin_char((unsigned char)s[at])) {
                if(pinyin_tone((unsigned char)s[at])) {
                    int value=s[at]-'0';
                    if(previous_tone!=SIZE_MAX) {
                        current_base=s+base_start;current_base_length=base_length;
                        if(previous_value==3 && value==3) {
                            s[previous_tone]='2';
                            previous_tone=SIZE_MAX;previous_value=0;
                            previous_base=NULL;previous_base_length=0;
                            base_start=0;base_length=0;
                            ++at;
                            continue;
                        }
                        if(previous_base_length==2 && !memcmp(previous_base,"bu",2) &&
                           previous_value==4 && value==4)
                            s[previous_tone]='2';
                        if(previous_base_length==2 && !memcmp(previous_base,"yi",2) &&
                           previous_value==1 &&
                           !(current_base_length==3 && !memcmp(current_base,"hao",3))) {
                            if(value==4)s[previous_tone]='2';
                            else if(value>=1 && value<=3)s[previous_tone]='4';
                        }
                    }
                    previous_tone=at;previous_value=value;
                    previous_base=s+base_start;previous_base_length=base_length;
                    base_start=0;base_length=0;
                } else {
                    if(!base_length)base_start=at;
                    base_length++;
                }
                ++at;
            }
            i=at;
            continue;
        }
        if(pinyin_boundary((unsigned char)s[i])) {
            previous_tone=SIZE_MAX;previous_value=0;
            previous_base=NULL;previous_base_length=0;
        }
        ++i;
    }
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
    if(previous==2&&say(&r->output,chinese_english_boundary))return -1;
    if(expand) {
        if(say(&r->output," `gD_"))return -1;
        if(r->wide.length==2) {
            unsigned tens=(unsigned)(r->wide.data[0]-'0');
            unsigned units=(unsigned)(r->wide.data[1]-'0');
            if(tens>1 && say(&r->output,digits[tens]))return -1;
            if(say(&r->output,"shi2"))return -1;
            if(units && say(&r->output,digits[units]))return -1;
        } else {
            for(size_t i=0;i<r->wide.length;++i) {
                unsigned digit=(unsigned)(r->wide.data[i]-'0');
                if(say(&r->output,digits[digit]))return -1;
            }
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
        if(latin_letter(cp)) {
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
    int previous_word=0,previous_number=0,phrase_english=0,spelling=0;char previous_category=0;size_t phrase_end=0;
    for(size_t i=0;i<n;) {
        uint32_t cp;size_t used=decode(text+i,n-i,&cp);if(!used)return -1;
        int after_number=previous_number;
        if(cp>32&&cp!=0x3000)previous_number=0;
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
            if(chinese_latin(r,text+i,used,0,1,&previous_word))return -1;
            i+=used;continue;
        }
        if(ideograph(cp)) {
            /* The romanizer marks its own script boundary independently of
               ordinary punctuation pause policy. */
            if(previous_word==2&&say(&r->output,chinese_english_boundary))return -1;
            previous_word=1;
            size_t group_end=0;
            const char *group_reading=NULL;
            char group_category=0;
            char single_key[5];size_t single_length=encode(single_key,chinese_form(cp));
            single_key[single_length]=0;
            if(phrase_group(text,n,i,&group_end,&group_reading,&group_category)) {
                char tag[8];snprintf(tag,sizeof tag," `g%c_",group_category);
                if(say(&r->output,tag)||say(&r->output,group_reading))return -1;
                previous_category=group_category;
                i=group_end;continue;
            }
            char key[8*4+1],original[8*4+1];size_t at=0,original_at=0,p=i,end=i+used,best_end=end;
            const struct traditional_reading *identities[8]={0};
            const char *best_reading=NULL;size_t best_size=0,best_key_size=0;char best_category='N';int changed=0;
            char reading[192],modal_reading[192],identity_reading[32];
            for(unsigned count=0;count<8&&p<n;++count) {
                /* Keep 和 + 解锁 separate without splitting 了解 + 锁定. */
                if(count&&best_category=='C'&&
                   phrase_group(text,n,p,&group_end,&group_reading,&group_category))break;
                uint32_t next;size_t z=decode(text+p,n-p,&next);if(!z||!ideograph(next))break;
                uint32_t folded=chinese_form(next);changed|=folded!=next;
                identities[count]=folded!=next?traditional_identity(next):NULL;
                original_at+=encode(original+original_at,next);original[original_at]=0;
                at+=encode(key+at,folded);key[at]=0;p+=z;
                /* Search the source spelling before its compatibility
                   form. A folded candidate must retain every constrained
                   source identity, including lexical neutral tones. */
                const struct chinese_word *word=find_word(original);
                if(!word&&changed) {
                    word=find_word(key);
                    if(word&&!traditional_word((const char *)chinese_strings+word->reading,identities,count+1))word=NULL;
                }
                if(word){best_reading=(const char *)chinese_strings+word->reading;best_size=strlen(best_reading);best_category=(char)word->category;best_end=p;best_key_size=at;}
                else if(!count&&identities[0]) {
                    const struct chinese_word *base=find_word(key);
                    /* The table lists alternatives, but a standalone Han
                     * character has exactly one default pronunciation.
                     * Later context rules expect a NUL-terminated reading. */
                    best_size=strcspn(identities[0]->readings," ");
                    if(best_size>=sizeof identity_reading)return -1;
                    memcpy(identity_reading,identities[0]->readings,best_size);
                    identity_reading[best_size]=0;best_reading=identity_reading;
                    best_category=base?(char)base->category:'N';best_end=p;best_key_size=at;
                }
            }
            if(best_reading) {
                /* Corrections describe the matched word, not speculative lookahead. */
                key[best_key_size]=0;
                if(best_end==end && !identities[0] && !strcmp(single_key,"了") && !strcmp(best_reading,"liao3")) {
                    best_reading="le0";best_size=3;
                } else {
                    best_reading=phrase_reading(key,best_reading);
                    best_reading=hit_reading(key,best_reading,reading,sizeof reading);
                    best_reading=polyphone_reading(text,n,i,single_key,best_end,end,previous_category,best_reading);
                    best_reading=blood_reading(key,best_reading,reading,sizeof reading);
                    best_reading=chong_reading(key,best_reading,reading,sizeof reading);
                    best_reading=particle_reading(key,best_reading,reading,sizeof reading);
                    best_reading=row_reading(text,n,i,key,best_reading,reading,sizeof reading);
                    best_reading=modal_dei_reading(key,best_reading,best_category,modal_reading,sizeof modal_reading);
                    best_size=strlen(best_reading);
                }
                char tag[8];snprintf(tag,sizeof tag," `g%c_",best_category);
                if(say(&r->output,tag)||append(&r->output,best_reading,best_size))return -1;
                previous_category=best_category;
            } else if(say(&r->output," "))return -1;
            i=best_end;continue;
        }
        previous_category=0;
        uint32_t following=0;
        if(i+used<n)decode(text+i+used,n-i-used,&following);
        const int enclosing=(cp=='('||cp=='['||cp=='{'||cp=='<'||cp=='"'||
            cp==0x201c||cp==0x300c||cp==0x300e)&&latin_letter(width_fold(following))&&
            !reloq_pinyin_prefix(text+i+used,n-i-used);
        if(latin_letter(width_fold(cp))||enclosing) {
            size_t pinyin=cp<128&&!enclosing?reloq_pinyin_prefix(text+i,n-i):0;
            if(pinyin){
                if(previous_word==2&&say(&r->output,chinese_english_boundary))return -1;
                if(append(&r->output,text+i,pinyin))return -1;
                previous_word=1;i+=pinyin;continue;
            }
            int full=0;size_t z=latin_token(r,text+i,n-i,0,&full);if(!z)return -1;
            if(i>=phrase_end) {unsigned words=0;phrase_end=i+chinese_latin_phrase(text+i,n-i,&words);phrase_english=words>1;}
            int literal=!phrase_english&&chinese_literal_letters(&r->wide);
            int characters=literal&&!after_number&&!strpbrk(r->wide.data,"0123456789");
            if(chinese_latin(r,text+i,z,!literal,characters,&previous_word))return -1;
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
            if(say(&r->output," ")||say(&r->output,"-"))return -1;
            previous_word=1;i+=used;continue;
        }
        if(cp>='0'&&cp<='9') {
            size_t end;if(chinese_number(r,text,n,i,&end,previous_word))return -1;
            previous_word=1;previous_number=1;previous_category=0;i=end;continue;
        } else if(chinese_operator(cp)) {
            if(chinese_symbol(r,cp,&previous_word))return -1;
            i+=used;continue;
        } else if(cp=='@') {
            if(previous_word==2&&say(&r->output,chinese_english_boundary))return -1;
            else if(previous_word==1&&say(&r->output," "))return -1;
            if(say(&r->output," `g9_at "))return -1;
            previous_word=2;i+=used;continue;
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
        if(latin_letter(width_fold(cp))){
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
            if(latin_letter(width_fold(next))||(next=='-'&&latin_letter(width_fold(after_hyphen)))) {
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
    if(r->language!=10)apply_tone_sandhi(&r->output);
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
