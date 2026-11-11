#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include "nvda_dictionary.h"
#include "nvda_speech.h"
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_LIMIT (16u*1024u*1024u)
struct entry {pcre2_code *pattern;char *replacement;uint32_t options;};
struct reloq_nvda_speech {struct entry *entries;size_t count,capacity;};
struct buffer {char *data;size_t size,capacity;};

static int append(struct buffer *b,const char *data,size_t n)
{
    if(n>OUTPUT_LIMIT-b->size)return 0;
    size_t need=b->size+n+1;
    if(need>b->capacity){size_t size=b->capacity?b->capacity:128;
        while(size<need){if(size>OUTPUT_LIMIT/2){size=OUTPUT_LIMIT+1;break;}size*=2;}
        char *p=realloc(b->data,size);if(!p)return 0;b->data=p;b->capacity=size;}
    if(n)memcpy(b->data+b->size,data,n);b->size+=n;b->data[b->size]=0;return 1;
}
static int addstr(struct buffer *b,const char *s){return append(b,s,strlen(s));}
static int quoted(struct buffer *b,const char *p,size_t n)
{
    if(!addstr(b,"\\Q"))return 0;
    for(size_t i=0;i<n;++i){
        if(p[i]=='\\'&&i+1<n&&p[i+1]=='E'){
            if(!addstr(b,"\\E\\\\E\\Q"))return 0;++i;
        }else if(!append(b,p+i,1))return 0;
    }
    return addstr(b,"\\E");
}
static int hex_digit(char c)
{return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');}
static int regex_expression(struct buffer *b,const char *p,size_t n)
{
    for(size_t i=0;i<n;++i){
        if(p[i]!='\\'||i+1==n){if(!append(b,p+i,1))return 0;continue;}
        char next=p[++i];
        if(next=='u'||next=='U'){
            size_t digits=next=='u'?4:8;if(digits>n-i-1)return 0;
            for(size_t j=0;j<digits;++j)if(!hex_digit(p[i+1+j]))return 0;
            if(!addstr(b,"\\x{")||!append(b,p+i+1,digits)||!addstr(b,"}"))return 0;i+=digits;
        }else if(next=='Z'){if(!addstr(b,"\\z"))return 0;}
        else if(!addstr(b,"\\")||!append(b,&next,1))return 0;
    }
    return 1;
}
static int expression(struct buffer *b,const struct reloq_nvda_entry *entry)
{
    const char *p=entry->pattern;size_t n=entry->pattern_length;unsigned kind=entry->kind;
    if(kind==1)return regex_expression(b,p,n);
    if(kind==6){
        if(!addstr(b,"(?s:"))return 0;
        for(size_t i=0;i<n;++i){
            if(p[i]=='*'){while(i+1<n&&p[i+1]=='*')++i;if(!addstr(b,".*"))return 0;}
            else if(p[i]=='?'){if(!addstr(b,"."))return 0;}
            else if(p[i]=='['){size_t end=i+1;if(end<n&&p[end]=='!')++end;if(end<n&&p[end]==']')++end;
                while(end<n&&p[end]!=']')++end;
                if(end==n){if(!quoted(b,p+i,1))return 0;}
                else {if(!addstr(b,"["))return 0;size_t j=i+1;
                    if(j<end&&p[j]=='!'){if(!addstr(b,"^"))return 0;++j;}
                    else if(j<end&&p[j]=='^'){if(!addstr(b,"\\"))return 0;}
                    for(;j<end;++j){if(p[j]=='\\'&&!addstr(b,"\\"))return 0;if(!append(b,p+j,1))return 0;}
                    if(!addstr(b,"]"))return 0;i=end;}}
            else {size_t bytes=((unsigned char)p[i]<0x80)?1:
                    ((unsigned char)p[i]<0xe0)?2:((unsigned char)p[i]<0xf0)?3:4;
                if(bytes>n-i||!quoted(b,p+i,bytes))return 0;i+=bytes-1;}
        }
        return addstr(b,")");
    }
    if(kind==2&&!addstr(b,"\\b"))return 0;
    if((kind==3||kind==5)&&!addstr(b,"(?<=\\w)"))return 0;
    if(kind==4&&!addstr(b,"\\b"))return 0;
    if(!quoted(b,p,n))return 0;
    if(kind==3){if(!addstr(b,"|")||!quoted(b,p,n)||!addstr(b,"(?=\\w)"))return 0;}
    if(kind==4&&!addstr(b,"(?=\\w)"))return 0;
    if((kind==2||kind==5)&&!addstr(b,"\\b"))return 0;
    return 1;
}
static int replacement(struct buffer *b,const struct reloq_nvda_entry *entry)
{
    if(entry->kind!=1)return append(b,entry->replacement,entry->replacement_length);
    const char *p=entry->replacement;size_t n=entry->replacement_length;
    for(size_t i=0;i<n;++i){char c=p[i];
        if(c=='$'){if(!addstr(b,"$$"))return 0;continue;}
        if(c!='\\'){if(!append(b,p+i,1))return 0;continue;}
        if(++i>=n)return 0;c=p[i];
        if(c=='g'&&i+1<n&&p[i+1]=='<'){
            size_t start=i+2,end=start;while(end<n&&p[end]!='>')++end;
            if(end==n||end==start||!addstr(b,"${")||!append(b,p+start,end-start)||!addstr(b,"}"))return 0;
            i=end;
        }else if(c>='1'&&c<='9'){
            if(!addstr(b,"${")||!append(b,&c,1))return 0;
            if(i+1<n&&p[i+1]>='0'&&p[i+1]<='9'){++i;if(!append(b,p+i,1))return 0;}
            if(!addstr(b,"}"))return 0;
        }else {if(!addstr(b,"\\")||!append(b,&c,1))return 0;}
    }
    return 1;
}
struct reloq_nvda_speech *reloq_nvda_speech_new(void){return calloc(1,sizeof(struct reloq_nvda_speech));}
void reloq_nvda_speech_free(struct reloq_nvda_speech *d)
{
    if(!d)return;for(size_t i=0;i<d->count;++i){pcre2_code_free(d->entries[i].pattern);free(d->entries[i].replacement);}
    free(d->entries);free(d);
}
static int valid_replacement(const struct reloq_nvda_entry *e,pcre2_code *code)
{
    if(e->kind!=1)return 1;uint32_t groups=0;
    if(pcre2_pattern_info(code,PCRE2_INFO_CAPTURECOUNT,&groups))return 0;
    const char *p=e->replacement;size_t n=e->replacement_length;
    for(size_t i=0;i<n;++i)if(p[i]=='\\'){
        if(++i>=n)return 0;unsigned char c=(unsigned char)p[i];
        if(c>='1'&&c<='9'){
            unsigned group=c-'0';if(i+1<n&&p[i+1]>='0'&&p[i+1]<='9')group=group*10+(unsigned)(p[++i]-'0');
            if(group>groups)return 0;
        }else if(c=='g'){
            if(i+1>=n||p[++i]!='<')return 0;size_t start=++i;
            while(i<n&&p[i]!='>')++i;if(i==n||i==start||i-start>=128)return 0;
            char name[128];memcpy(name,p+start,i-start);name[i-start]=0;
            unsigned group=0;size_t j=0;while(name[j]>='0'&&name[j]<='9'){
                if(group>100000)return 0;group=group*10+(unsigned)(name[j++]-'0');}
            if(!name[j]){if(group>groups)return 0;}
            else if(pcre2_substring_number_from_name(code,(PCRE2_SPTR)name)<0)return 0;
        }else if((c>='A'&&c<='Z')||(c>='a'&&c<='z')){
            if(!strchr("abfnrtv",c))return 0;
        }else if(c=='0')return 0; /* Embedded NULs cannot be sent to ECI. */
    }
    return 1;
}
static int compile_one(void *context,const struct reloq_nvda_entry *e)
{
    struct reloq_nvda_speech *d=context;struct buffer pattern={0},value={0};int error=0;PCRE2_SIZE error_at=0;
    if(!expression(&pattern,e)||!replacement(&value,e))goto fail;
    if(!pattern.data&&!addstr(&pattern,""))goto fail;if(!value.data&&!addstr(&value,""))goto fail;
    uint32_t options=PCRE2_UTF|PCRE2_UCP|(e->case_sensitive?0:PCRE2_CASELESS);
    pcre2_code *code=pcre2_compile((PCRE2_SPTR)pattern.data,pattern.size,options,&error,&error_at,NULL);
    if(!code)goto fail;
    if(!valid_replacement(e,code)){pcre2_code_free(code);goto fail;}
    if(d->count==d->capacity){size_t n=d->capacity?d->capacity*2:16;
        if(n>SIZE_MAX/sizeof *d->entries){pcre2_code_free(code);goto fail;}
        void *p=realloc(d->entries,n*sizeof *d->entries);if(!p){pcre2_code_free(code);goto fail;}
        d->entries=p;d->capacity=n;}
    d->entries[d->count++]=(struct entry){code,value.data,
        e->kind==1?(PCRE2_SUBSTITUTE_EXTENDED|PCRE2_SUBSTITUTE_UNSET_EMPTY):PCRE2_SUBSTITUTE_LITERAL};
    free(pattern.data);return 1;
fail:free(pattern.data);free(value.data);return 0;
}
int reloq_nvda_speech_add(struct reloq_nvda_speech *d,const unsigned char *data,size_t length)
{
    if(!d)return 0;size_t old=d->count;
    if(reloq_nvda_parse(data,length,compile_one,d,NULL)==RELOQ_NVDA_VALID)return 1;
    while(d->count>old){struct entry *e=&d->entries[--d->count];pcre2_code_free(e->pattern);free(e->replacement);}return 0;
}
static int replace_one(const struct entry *e,const char *text,size_t length,
    struct buffer *out,size_t *map,int *matched)
{
    pcre2_match_data *data=pcre2_match_data_create_from_pattern(e->pattern,NULL);
    pcre2_match_context *context=pcre2_match_context_create(NULL);
    if(!data||!context){pcre2_match_data_free(data);pcre2_match_context_free(context);return 0;}
    pcre2_set_match_limit(context,1000000);
    size_t pos=0,last=0;uint32_t flags=0;int ok=1;
    while(pos<=length){
        int rc=pcre2_match(e->pattern,(PCRE2_SPTR)text,length,pos,flags,data,context);
        if(rc==PCRE2_ERROR_NOMATCH&&flags){flags=0;if(pos==length)break;
            ++pos;while(pos<length&&((unsigned char)text[pos]&0xc0)==0x80)++pos;continue;}
        if(rc==PCRE2_ERROR_NOMATCH)break;if(rc<0){ok=-1;break;}
        PCRE2_SIZE *v=pcre2_get_ovector_pointer(data);size_t start=v[0],end=v[1];
        if(start<last||end<start||end>length){ok=0;break;}
        size_t old=out->size;
        if(!append(out,text+last,start-last)){ok=0;break;}
        for(size_t i=last;i<=start;++i)map[i]=old+(i-last);
        PCRE2_SIZE need=0;uint32_t suboptions=e->options|PCRE2_SUBSTITUTE_MATCHED|
            PCRE2_SUBSTITUTE_REPLACEMENT_ONLY|PCRE2_SUBSTITUTE_OVERFLOW_LENGTH;
        rc=pcre2_substitute(e->pattern,(PCRE2_SPTR)text,length,start,suboptions,data,context,
            (PCRE2_SPTR)e->replacement,PCRE2_ZERO_TERMINATED,NULL,&need);
        if(rc!=PCRE2_ERROR_NOMEMORY){ok=-1;break;}
        if(need>OUTPUT_LIMIT-out->size+1){ok=0;break;}
        char *piece=malloc(need?need:1);if(!piece){ok=0;break;}
        PCRE2_SIZE size=need;
        rc=pcre2_substitute(e->pattern,(PCRE2_SPTR)text,length,start,suboptions,data,context,
            (PCRE2_SPTR)e->replacement,PCRE2_ZERO_TERMINATED,(PCRE2_UCHAR *)piece,&size);
        if(rc<0){free(piece);ok=-1;break;}
        if(!append(out,piece,size)){free(piece);ok=0;break;}free(piece);
        for(size_t i=start+1;i<end;++i)map[i]=map[start];map[end]=out->size;
        *matched=1;last=end;pos=end;flags=start==end?(PCRE2_NOTEMPTY_ATSTART|PCRE2_ANCHORED):0;
    }
    if(ok>0){size_t old=out->size;if(!append(out,text+last,length-last))ok=0;
        else for(size_t i=last;i<=length;++i)map[i]=old+i-last;}
    pcre2_match_data_free(data);pcre2_match_context_free(context);return ok;
}
int reloq_nvda_speech_apply(const struct reloq_nvda_speech *d,const char *text,size_t length,
    char **result,size_t *result_length,size_t **offsets,int *matched)
{
    if(!d||(!text&&length)||!result||!result_length||!offsets||!matched||length>OUTPUT_LIMIT||
       length>=SIZE_MAX/sizeof(size_t))return 0;
    *result=NULL;*offsets=NULL;*matched=0;size_t *map=malloc((length+1)*sizeof *map);
    char *current=malloc(length+1);if(!map||!current){free(map);free(current);return 0;}
    if(length)memcpy(current,text,length);current[length]=0;for(size_t i=0;i<=length;++i)map[i]=i;
    size_t current_length=length;
    for(size_t e=0;e<d->count;++e){
        size_t *next_map=malloc((current_length+1)*sizeof *next_map);struct buffer next={0};
        int found=0;
        if(!next_map){free(current);free(map);return 0;}
        int applied=replace_one(&d->entries[e],current,current_length,&next,next_map,&found);
        if(applied<0){free(next_map);free(next.data);continue;}
        if(!applied){
            free(next_map);free(next.data);free(current);free(map);return 0;}
        for(size_t i=0;i<=length;++i)map[i]=next_map[map[i]];
        free(next_map);free(current);current=next.data;current_length=next.size;*matched|=found;
    }
    if(length){map[0]=0;map[length]=current_length;}
    *result=current;*result_length=current_length;*offsets=map;return 1;
}
