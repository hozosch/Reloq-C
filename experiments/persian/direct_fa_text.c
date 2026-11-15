/* Complete-word Persian text probe, not a production NVDA frontend. */
#include "direct_fa_text.h"
#include "direct_fa_lexicon.h"
#include "direct_fa_normalize.h"
#include "reloq_utf8.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct output_buffer { char *data; size_t used,capacity; };

static int append(struct output_buffer *b,const char *s,size_t n)
{
    if(n>SIZE_MAX-b->used-1)return -1;
    size_t needed=b->used+n+1;
    if(needed>b->capacity) {
        size_t next=b->capacity?b->capacity:128;
        while(next<needed)next=next>SIZE_MAX/2?needed:next*2;
        char *data=realloc(b->data,next);
        if(!data)return -1;
        b->data=data;b->capacity=next;
    }
    memcpy(b->data+b->used,s,n);b->used+=n;b->data[b->used]=0;
    return 0;
}

static int word_character(uint32_t c)
{
    /* The lexical lookup makes the final decision. Digits and symbols in
     * this range are therefore rejected instead of being guessed. */
    return (c>=0x600 && c<=0x6ff && c!=0x60c && c!=0x61b && c!=0x61f) || c==0x200c;
}

/* Phrase-final function words receive the reference's final stress.  Keep
 * punctuation and line breaks as boundaries; a following word in the same
 * phrase must be separated by spaces or tabs. */
static int follows_word_in_phrase(const char *text,size_t length,size_t at)
{
    int saw_space=0;
    while(at<length && (text[at]==' ' || text[at]=='\t')){++at;saw_space=1;}
    if(!saw_space || at==length)return 0;
    uint32_t cp;size_t next=at;
    return reloq_utf8_next((const unsigned char *)text,length,&next,&cp)==1 &&
           word_character(cp);
}

void reloq_fa_direct_free(void *pointer) { free(pointer); }

static int direct_normalized_text(const char *text,size_t length,char **output,
                                  size_t **source_map,size_t *error_offset)
{
    if(output)*output=NULL;
    if(source_map)*source_map=NULL;
    if(error_offset)*error_offset=0;
    if(!text || !output || !source_map || !error_offset || length==SIZE_MAX ||
       length+1>SIZE_MAX/sizeof(size_t) || memchr(text,0,length))return -1;
    size_t *map=malloc((length+1)*sizeof *map);
    if(!map)return -2;
    struct output_buffer b={0};
    if(append(&b,"",0)){free(map);return -2;}
    int status=0;
    size_t at=0;
    while(at<length) {
        size_t start=at,next=at;
        uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)text,length,&next,&cp)!=1) {status=-1;break;}
        if(word_character(cp)) {
            size_t end=next;
            while(end<length) {
                size_t probe=end;uint32_t c;
                if(reloq_utf8_next((const unsigned char *)text,length,&probe,&c)!=1){at=end;status=-1;break;}
                if(!word_character(c))break;
                end=probe;
            }
            if(status)break;
            char tag[2048];
            int found=reloq_fa_direct_context_word(text+start,end-start,
                         follows_word_in_phrase(text,length,end),tag,sizeof tag);
            if(found!=1){status=found==0?1:-1;break;}
            size_t tag_start=b.used;
            if(append(&b,tag,strlen(tag))){status=-2;break;}
            for(size_t i=start;i<end;++i)map[i]=tag_start;
            map[end]=b.used;at=end;
        } else {
            char punctuation=0;
            if(cp==' ' || cp=='\t' || cp=='\r' || cp=='\n' ||
               cp=='.' || cp==',' || cp=='?' || cp=='!' || cp==':' || cp==';')punctuation=(char)cp;
            else if(cp==0x60c)punctuation=',';
            else if(cp==0x61b)punctuation=';';
            else if(cp==0x61f)punctuation='?';
            else {status=1;break;}
            size_t point=b.used;
            if(append(&b,&punctuation,1)){status=-2;break;}
            for(size_t i=start;i<next;++i)map[i]=point;
            map[next]=b.used;at=next;
        }
    }
    if(status) {
        *error_offset=at;free(map);free(b.data);return status;
    }
    map[length]=b.used;*source_map=map;*output=b.data;return 0;
}

int reloq_fa_direct_text(const char *text,size_t length,char **output,
                         size_t **source_map,size_t *error_offset)
{
    if(output)*output=NULL;
    if(source_map)*source_map=NULL;
    if(error_offset)*error_offset=0;
    if(!text || !output || !source_map || !error_offset || length==SIZE_MAX ||
       length+1>SIZE_MAX/sizeof(size_t) || memchr(text,0,length))return -1;
    /* fa_rules maps both numeric separators to an Arabic comma. In this
     * frontend they must not become clause boundaries before number support
     * exists, even when they occur without a digit. */
    size_t at=0;
    while(at<length) {
        size_t start=at;uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)text,length,&at,&cp)!=1) {
            *error_offset=start;return -1;
        }
        if(cp==0x066b || cp==0x066c){*error_offset=start;return 1;}
    }
    char *normalized=NULL,*tagged=NULL;
    size_t *source_to_normalized=NULL,*normalized_to_tagged=NULL,offset=0;
    int status=reloq_fa_direct_normalize(text,length,&normalized,&source_to_normalized,&offset);
    if(status){*error_offset=offset;return status;}
    status=direct_normalized_text(normalized,strlen(normalized),&tagged,&normalized_to_tagged,&offset);
    if(status) {
        size_t original=0;
        while(original<length && source_to_normalized[original]<offset)++original;
        *error_offset=original;free(source_to_normalized);free(normalized);return status;
    }
    for(size_t i=0;i<=length;++i)
        source_to_normalized[i]=normalized_to_tagged[source_to_normalized[i]];
    free(normalized_to_tagged);free(normalized);
    *output=tagged;*source_map=source_to_normalized;return 0;
}
