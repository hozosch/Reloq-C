/* Native Persian phrase projection with an optional Pinglish input mode.
 * Script words and Latin words share the same Persian lexical/stress rules;
 * uncovered input is returned untouched to the caller's existing fallback. */
#include "reloq_persian_direct.h"
#include "reloq_utf8.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct fa_alias {const char *roman,*persian;};
static const struct fa_alias aliases[]={
    {"salam","سلام"},{"salaam","سلام"},{"donya","دنیا"},{"donyaa","دنیا"},
    {"iran","ایران"},{"iraan","ایران"},{"khub","خوب"},{"khoob","خوب"},
    {"nokte","نقطه"},{"nokteh","نقطه"},{"noghte","نقطه"},{"noghteh","نقطه"},
    {"kaghaz","کاغذ"},{"kakhaz","کاغذ"},{"sefr","صفر"},{"yek","یک"},
    {"soal","سوال"},{"soaal","سوال"},{"sual","سوال"},
    {"badbakht","بدبخت"},{"badbacht","بدبخت"},
    {"farsi","فارسی"},{"faarsi","فارسی"},{"ketab","کتاب"},{"ketaab","کتاب"},
    {"eslash","اسلش"},{"ast","است"},{"man","من"},
    {"u","او"},{"oo","او"},{"ou","او"},{"amad","آمد"},{"aamad","آمد"},
    {"va","و"},{"ya","یا"},{"yaa","یا"},{"ke","که"},
    {"ra","را"},{"raa","را"},{"az","از"},{"be","به"},{"ba","با"},
    {"dar","در"},{"ta","تا"},{"taa","تا"},{"joz","جز"}
};
static int ascii_letter(unsigned char c)
{return (c>='a' && c<='z') || (c>='A' && c<='Z');}
static int ascii_digit(unsigned char c)
{return c>='0' && c<='9';}
static int persian_scalar(uint32_t c)
{
    return (c>=0x0600 && c<=0x06ff) || (c>=0x0750 && c<=0x077f) ||
           (c>=0x08a0 && c<=0x08ff) || (c>=0xfb50 && c<=0xfdff) ||
           (c>=0xfe70 && c<=0xfeff) || c==0x200c || c==0x200d;
}
static int connected_scalar(uint32_t c)
{
    return persian_scalar(c) && c!=0x060c && c!=0x061b && c!=0x061f &&
           c!=0x066b && c!=0x066c && c!=0x066a;
}
static const char *alias(const char *word,size_t n)
{
    for(size_t i=0;i<sizeof aliases/sizeof aliases[0];++i) {
        const char *p=aliases[i].roman;
        if(strlen(p)!=n)continue;
        size_t j=0;
        for(;j<n;++j) {
            unsigned char c=(unsigned char)word[j];
            if(c>='A' && c<='Z')c=(unsigned char)(c+('a'-'A'));
            if(c!=(unsigned char)p[j])break;
        }
        if(j==n)return aliases[i].persian;
    }
    return NULL;
}

struct output_buffer {char *text;size_t length,capacity;};
static int append(struct output_buffer *b,const char *text,size_t bytes)
{
    if(bytes>SIZE_MAX-b->length-1)return -2;
    size_t need=b->length+bytes+1;
    if(need>b->capacity) {
        size_t capacity=b->capacity?b->capacity:128;
        while(capacity<need)capacity=capacity>SIZE_MAX/2?need:capacity*2;
        char *next=realloc(b->text,capacity);
        if(!next)return -2;
        b->text=next;b->capacity=capacity;
    }
    memcpy(b->text+b->length,text,bytes);b->length+=bytes;b->text[b->length]=0;
    return 0;
}
static int separate(struct output_buffer *b,const char *text)
{
    return text[0]=='`' && text[1]=='[' && b->length && b->text[b->length-1]==']'
        ? append(b," ",1) : 0;
}
static int enclitic_start(const char *text,size_t bytes)
{
    size_t at=0;
    while(at<bytes && (text[at]==' ' || text[at]=='\t'))++at;
    if(!at)return 0;
    static const char *const words[]={"را","است"};
    for(size_t i=0;i<sizeof words/sizeof words[0];++i) {
        size_t n=strlen(words[i]);
        if(n>bytes-at || memcmp(text+at,words[i],n))continue;
        size_t end=at+n;
        if(end==bytes || strchr(" \t.,?!:;)]}",text[end]))return 1;
        if(end+1<bytes && (unsigned char)text[end]==0xd8 &&
           ((unsigned char)text[end+1]==0x8c || (unsigned char)text[end+1]==0x9b ||
            (unsigned char)text[end+1]==0x9f))return 1;
    }
    return 0;
}
static int ordinary(struct output_buffer *b,size_t *mapping,const char *text,
                    size_t first,size_t last,int follows)
{
    if(first==last)return 0;
    const char *prefix=(first && enclitic_start(text+first,last-first))?"سلام":"";
    const char *suffix=follows?" سلام":"";
    size_t before=strlen(prefix),after=strlen(suffix),bytes=last-first;
    if(bytes>SIZE_MAX-before-after-1)return -2;
    size_t length=before+bytes+after;
    char *trial=malloc(length+1),*tagged=NULL;size_t *local=NULL,error=0;
    if(!trial)return -2;
    memcpy(trial,prefix,before);memcpy(trial+before,text+first,bytes);
    memcpy(trial+before+bytes,suffix,after+1);
    int status=reloq_fa_direct_text(trial,length,&tagged,&local,&error);
    free(trial);
    if(status){free(tagged);free(local);return status==-2?-2:1;}
    size_t begin=local[before],end=local[before+bytes];
    if(end<begin || end>strlen(tagged)){free(tagged);free(local);return -1;}
    char saved=tagged[end];tagged[end]=0;
    status=separate(b,tagged+begin);
    if(!status) {
        size_t base=b->length;
        status=append(b,tagged+begin,end-begin);
        if(!status)for(size_t i=0;i<=bytes;++i)mapping[first+i]=base+local[before+i]-begin;
    }
    tagged[end]=saved;
    free(tagged);free(local);return status;
}
static int mixed_text(const char *text,size_t length,char **tagged,size_t **source_map)
{
    static const char *const letters[]={
        "`[.1ej]","`[.1bi]","`[.1si]","`[.1di]","`[.1i]","`[.1ef]",
        "`[.1Ji]","`[.1ejC]","`[.1aj]","`[.1Jej]","`[.1kej]","`[.1el]",
        "`[.1em]","`[.1en]","`[.1o]","`[.1pi]","`[.1ki.0ju]","`[.1ar]",
        "`[.1es]","`[.1ti]","`[.1ju]","`[.1vi]","`[.0dA.1be.0lju]",
        "`[.1eks]","`[.1vaj]","`[.1zi]"
    };
    if(length+1>SIZE_MAX/sizeof(size_t))return -2;
    size_t *mapping=calloc(length+1,sizeof *mapping);
    if(!mapping)return -2;
    struct output_buffer b={0};
    int status=append(&b,"",0);size_t cursor=0,at=0;
    while(!status && at<length) {
        if(!ascii_letter((unsigned char)text[at])){++at;continue;}
        size_t first=at;
        while(at<length && ascii_letter((unsigned char)text[at]))++at;
        status=ordinary(&b,mapping,text,cursor,first,1);
        if(status)break;
        size_t bytes=at-first;char phone[8193];const char *tag=phone;
        if(bytes==1) {
            unsigned char c=(unsigned char)text[first];
            if(c>='A' && c<='Z')c=(unsigned char)(c+('a'-'A'));
            tag=letters[c-'a'];
        } else {
            int found=reloq_fa_pinglish_word(text+first,bytes,phone,sizeof phone);
            if(found!=1){status=found==-2?-2:1;break;}
        }
        status=separate(&b,tag);
        if(status)break;
        size_t start=b.length;
        status=append(&b,tag,strlen(tag));
        if(status)break;
        for(size_t i=first;i<at;++i)mapping[i]=start;
        mapping[at]=b.length;cursor=at;
    }
    if(!status)status=ordinary(&b,mapping,text,cursor,length,0);
    if(status){free(b.text);free(mapping);return status;}
    *tagged=b.text;*source_map=mapping;return 0;
}

int reloq_fa_pinglish_project(const char *text,size_t length,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    if(output)*output=NULL;
    if(error_offset)*error_offset=0;
    if(!text || !output || !error_offset || (event_count &&
       (!source_offsets || !is_index || !target_offsets)) || length>(SIZE_MAX-1)/4 ||
       length+1>SIZE_MAX/sizeof(size_t) || event_count>SIZE_MAX/sizeof(size_t) ||
       memchr(text,0,length))return -1;
    char *normalized=malloc(length*4+1),*tagged=NULL;
    size_t *positions=malloc((length+1)*sizeof *positions),*map=NULL;
    size_t *mapped=event_count?malloc(event_count*sizeof *mapped):NULL;
    if(!normalized || !positions || (event_count&&!mapped)) {
        free(normalized);free(positions);free(mapped);return -2;
    }
    size_t at=0,used=0;int status=0,saw_word=0;
    positions[0]=0;
    while(at<length) {
        unsigned char c=(unsigned char)text[at];
        if(ascii_letter(c)) {
            size_t start=at;
            while(at<length && ascii_letter((unsigned char)text[at]))++at;
            int connected=(start && ascii_digit((unsigned char)text[start-1])) ||
                          (at<length && ascii_digit((unsigned char)text[at]));
            if(start && (unsigned char)text[start-1]>=0x80) {
                size_t left=start-1;
                while(left && ((unsigned char)text[left]&0xc0)==0x80)--left;
                size_t end=left;uint32_t cp=0;
                if(reloq_utf8_next((const unsigned char *)text,length,&end,&cp)!=1 || end!=start)
                    {status=-1;at=left;break;}
                connected|=connected_scalar(cp);
            }
            if(at<length && (unsigned char)text[at]>=0x80) {
                size_t end=at;uint32_t cp=0;
                if(reloq_utf8_next((const unsigned char *)text,length,&end,&cp)!=1)
                    {status=-1;break;}
                connected|=connected_scalar(cp);
            }
            /* A Latin run joined to Persian letters needs the existing
             * mixed-word analysis; do not silently split that token. */
            if(connected && ((start && (unsigned char)text[start-1]>=0x80) ||
               (at<length && (unsigned char)text[at]>=0x80))) {status=1;at=start;break;}
            const char *replacement=connected?NULL:alias(text+start,at-start);
            if(replacement) {
                for(size_t i=start;i<at;++i)positions[i]=used;
                size_t n=strlen(replacement);
                memcpy(normalized+used,replacement,n);used+=n;positions[at]=used;
            } else {
                for(size_t i=start;i<at;++i)positions[i]=used+i-start;
                memcpy(normalized+used,text+start,at-start);used+=at-start;positions[at]=used;
            }
            saw_word=1;
        } else if(c>=0x80) {
            size_t first=at,next=at;uint32_t cp=0;
            if(reloq_utf8_next((const unsigned char *)text,length,&next,&cp)!=1)
                {status=-1;break;}
            if(!persian_scalar(cp)){status=1;break;}
            for(size_t i=first;i<next;++i)positions[i]=used+i-first;
            memcpy(normalized+used,text+first,next-first);used+=next-first;at=next;
            positions[at]=used;saw_word=1;
        } else {
            /* Do not admit raw controls, brackets, or punctuation that was
             * not admitted by the previous Pinglish frontend. */
            if(!(ascii_digit(c) || c==' ' || c=='\t' || c==',' || c=='.' ||
                 c=='?' || c=='!' || c==':' || c==';' || c=='+' || c=='-'))
                {status=1;break;}
            normalized[used++]=(char)c;positions[++at]=used;
            if(ascii_digit(c))saw_word=1;
        }
    }
    if(!status && !saw_word)status=1;
    normalized[used]=0;
    if(status){*error_offset=at;goto done;}
    size_t inner_error=0;
    status=reloq_fa_direct_text(normalized,used,&tagged,&map,&inner_error);
    if(status==1)status=mixed_text(normalized,used,&tagged,&map);
    if(status) {
        size_t source=0;
        while(source<length && positions[source]<inner_error)++source;
        *error_offset=source;goto done;
    }
    size_t output_length=strlen(tagged),previous=0,cursor=0,tag_start=0,tag_end=0;
    if(map[used]!=output_length){status=-1;goto done;}
    for(size_t i=0;i<event_count;++i) {
        size_t source=source_offsets[i];
        if(source>length || (i && source<previous) || is_index[i]>1){status=3;goto done;}
        size_t target=map[positions[source]];
        if(target>output_length || (i && target<mapped[i-1])){status=-1;goto done;}
        if(!is_index[i] && source>0 && source<length &&
           map[positions[source]]==map[positions[source-1]]) {
            status=3;*error_offset=source;goto done;
        }
        while(cursor<output_length) {
            if(tag_end>target)break;
            while(cursor<output_length && !(tagged[cursor]=='`' && cursor+1<output_length && tagged[cursor+1]=='['))++cursor;
            if(cursor==output_length)break;
            tag_start=cursor;
            while(cursor<output_length && tagged[cursor]!=']')++cursor;
            if(cursor==output_length){status=-1;goto done;}
            tag_end=++cursor;
        }
        if(tag_start<target && target<tag_end){status=3;*error_offset=source;goto done;}
        mapped[i]=target;previous=source;
    }
    if(event_count)memcpy(target_offsets,mapped,event_count*sizeof *mapped);
    *output=tagged;tagged=NULL;
done:
    free(normalized);free(positions);free(map);free(mapped);free(tagged);return status;
}
