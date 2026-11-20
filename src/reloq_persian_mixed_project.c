/* Shared Persian/Pinglish projection for normal and character-mode text.
 * The host provides only source byte boundaries and ordered events. */
#include "reloq_persian_direct.h"
#include "reloq_persian_phones.h"
#include "reloq_utf8.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct buffer {char *data;size_t length,capacity;};

static int append(struct buffer *buffer,const char *text,size_t bytes)
{
    if(bytes>SIZE_MAX-buffer->length-1)return -2;
    size_t needed=buffer->length+bytes+1;
    if(needed>buffer->capacity) {
        size_t size=buffer->capacity?buffer->capacity:128;
        while(size<needed)size=size>SIZE_MAX/2?needed:size*2;
        char *next=realloc(buffer->data,size);
        if(!next)return -2;
        buffer->data=next;buffer->capacity=size;
    }
    memcpy(buffer->data+buffer->length,text,bytes);
    buffer->length+=bytes;buffer->data[buffer->length]=0;
    return 0;
}

/* Character names and byte offsets are decided here, independently of the
 * host. The caller routes an English-only Unicode name separately. */
static int spelling(const char *text,size_t bytes,char **output,size_t **mapping,
                    size_t *error,int numeric_fallback)
{
    if(bytes==SIZE_MAX || bytes+1>SIZE_MAX/sizeof(size_t))return -2;
    size_t *map=calloc(bytes+1,sizeof *map);
    if(!map)return -2;
    struct buffer result={0};int status=append(&result,"",0);
    size_t at=0;
    while(!status && at<bytes) {
        size_t first=at;uint32_t cp=0;char name[2048];int kind=0;
        if(reloq_utf8_next((const unsigned char *)text,bytes,&at,&cp)!=1) {
            *error=first;status=-1;break;
        }
        int resolved=reloq_fa_character_resolve(cp,name,sizeof name,&kind);
        if(resolved==1 && kind && numeric_fallback) {
            resolved=reloq_fa_character_code_name(cp,name,sizeof name);
            kind=0;
        }
        if(resolved!=1 || kind!=0) {
            *error=first;status=resolved<0?-1:1;break;
        }
        if(result.length && append(&result," ",1)){status=-2;break;}
        for(size_t i=first;i<at;++i)map[i]=result.length;
        if(append(&result,name,strlen(name))){status=-2;break;}
        map[at]=result.length;
    }
    if(status){free(map);free(result.data);return status;}
    *output=result.data;*mapping=map;return 0;
}

static int phonetic(const char *text,size_t bytes,char **output,size_t **mapping,
                    size_t *error)
{
    char tag[8194];size_t written=0,offset=0;
    int status=reloq_fa_input_checked(text,bytes,tag,sizeof tag,&written,&offset);
    if(status==1)return spelling(text,bytes,output,mapping,error,1);
    if(status){*error=offset;return status;}
    if(bytes==SIZE_MAX || bytes+1>SIZE_MAX/sizeof(size_t))return -2;
    size_t *map=calloc(bytes+1,sizeof *map);char *result=malloc(written+1);
    if(!map || !result){free(map);free(result);return -2;}
    memcpy(result,tag,written+1);map[bytes]=written;
    *output=result;*mapping=map;return 0;
}

/* The independent planner keeps even unfamiliar character names in Persian
 * by speaking their Unicode code rather than selecting another language. */
int reloq_fa_spell_for_plan(const char *text,size_t bytes,char **output,
                           size_t **mapping,size_t *error)
{
    return spelling(text,bytes,output,mapping,error,1);
}

/* Ask the established Pinglish projector for every byte boundary. All are
 * indexes: a real non-index event is checked after the runs are joined. */
static int pinglish_text(const char *text,size_t bytes,char **output,size_t **mapping,
                         size_t *error)
{
    if(bytes==SIZE_MAX || bytes+1>SIZE_MAX/sizeof(size_t))return -2;
    size_t count=bytes+1;
    size_t *positions=malloc(count*sizeof *positions),*map=malloc(count*sizeof *map);
    unsigned char *indexes=malloc(count);
    if(!positions || !map || !indexes) {
        free(positions);free(map);free(indexes);return -2;
    }
    for(size_t i=0;i<count;++i)positions[i]=i;
    memset(indexes,1,count);
    int status=reloq_fa_pinglish_project(text,bytes,positions,indexes,count,
                                         output,map,error);
    free(positions);free(indexes);
    if(status){free(map);return status;}
    *mapping=map;return 0;
}

int reloq_fa_project_runs(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{
    if(output)*output=NULL;
    if(error_offset)*error_offset=0;
    if(!text || !output || !error_offset || (mode!=0 && mode!=1) ||
       (run_count && (!run_ends || !run_modes)) || (!!run_count != !!bytes) ||
       (event_count && (!source_offsets || !is_index || !target_offsets)) ||
       bytes==SIZE_MAX || bytes+1>SIZE_MAX/sizeof(size_t) ||
       event_count>SIZE_MAX/sizeof(size_t) || memchr(text,0,bytes))return -1;
    size_t previous=0;
    for(size_t i=0;i<run_count;++i) {
        size_t end=run_ends[i];
        if(end<=previous || end>bytes || run_modes[i]>2 ||
           (end<bytes && ((unsigned char)text[end]&0xc0)==0x80))return -1;
        previous=end;
    }
    if(previous!=bytes)return -1;
    size_t *map=calloc(bytes+1,sizeof *map);
    size_t *mapped=event_count?malloc(event_count*sizeof *mapped):NULL;
    if(!map || (event_count&&!mapped)){free(map);free(mapped);return -2;}
    struct buffer result={0};int status=append(&result,"",0);
    size_t first=0;
    for(size_t run=0;!status && run<run_count;) {
        size_t last=run_ends[run];unsigned char spelling_mode=run_modes[run++];
        while(spelling_mode!=2 && run<run_count && run_modes[run]==spelling_mode)last=run_ends[run++];
        size_t length=last-first,*local=NULL,error=0;char *tagged=NULL;
        if(spelling_mode==2)status=phonetic(text+first,length,&tagged,&local,&error);
        else if(spelling_mode==1)status=spelling(text+first,length,&tagged,&local,&error,0);
        else if(mode)status=pinglish_text(text+first,length,&tagged,&local,&error);
        else status=reloq_fa_direct_text(text+first,length,&tagged,&local,&error);
        if(status) {*error_offset=first+error;free(tagged);free(local);break;}
        size_t output_length=strlen(tagged);
        if(local[length]!=output_length){status=-1;free(tagged);free(local);break;}
        if(result.length && output_length &&
           !isspace((unsigned char)result.data[result.length-1]) &&
           !isspace((unsigned char)tagged[0]))status=append(&result," ",1);
        if(!status) {
            size_t base=result.length;
            map[first]=base;
            status=append(&result,tagged,output_length);
            if(!status)for(size_t i=1;i<=length;++i)map[first+i]=base+local[i];
        }
        first=last;free(tagged);free(local);
    }
    if(!status && map[bytes]!=result.length)status=-1;
    size_t cursor=0,tag_start=0,tag_end=0;previous=0;
    for(size_t i=0;!status && i<event_count;++i) {
        size_t source=source_offsets[i];
        if(source>bytes || (i && source<previous) || is_index[i]>1){status=3;break;}
        size_t target=map[source];
        if(target>result.length || (i && target<mapped[i-1])){status=-1;break;}
        if(!is_index[i] && source>0 && source<bytes && map[source]==map[source-1]) {
            *error_offset=source;status=3;break;
        }
        while(cursor<result.length) {
            if(tag_end>target)break;
            while(cursor<result.length && !(result.data[cursor]=='`' &&
                  cursor+1<result.length && result.data[cursor+1]=='['))++cursor;
            if(cursor==result.length)break;
            tag_start=cursor;
            while(cursor<result.length && result.data[cursor]!=']')++cursor;
            if(cursor==result.length){status=-1;break;}
            tag_end=++cursor;
        }
        if(status)break;
        if(tag_start<target && target<tag_end){*error_offset=source;status=3;break;}
        mapped[i]=target;previous=source;
    }
    if(!status) {
        if(event_count)memcpy(target_offsets,mapped,event_count*sizeof *mapped);
        *output=result.data;result.data=NULL;
    }
    free(result.data);free(map);free(mapped);return status;
}
