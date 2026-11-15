/* Native implementation of the Persian .replace patterns only. */
#include "direct_fa_normalize.h"
#include "reloq_utf8.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct fa_replacement { uint32_t source; const char *target; };
#include "direct_fa_replacements.inc"

static const char *replacement(uint32_t source)
{
    size_t lo=0,hi=sizeof fa_replacements/sizeof fa_replacements[0];
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;
        if(source<fa_replacements[m].source)hi=m;
        else if(source>fa_replacements[m].source)lo=m+1;
        else return fa_replacements[m].target;
    }
    return NULL;
}

int reloq_fa_direct_normalize(const char *text,size_t length,char **output,
                              size_t **source_map,size_t *error_offset)
{
    if(output)*output=NULL;
    if(source_map)*source_map=NULL;
    if(error_offset)*error_offset=0;
    if(!text || !output || !source_map || !error_offset || length==SIZE_MAX ||
       length+1>SIZE_MAX/sizeof(size_t) || memchr(text,0,length))return -1;
    size_t *map=malloc((length+1)*sizeof *map);
    if(!map)return -2;
    /* Every source code point is replaced with at most two code points. */
    if(length>(SIZE_MAX-1)/2){free(map);return -2;}
    size_t capacity=length*2+1,used=0,at=0;
    char *result=malloc(capacity);
    if(!result){free(map);return -2;}
    while(at<length) {
        size_t begin=at;uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)text,length,&at,&cp)!=1) {
            *error_offset=begin;free(map);free(result);return -1;
        }
        const char *value=replacement(cp);
        size_t n=value?strlen(value):at-begin;
        if(n>capacity-used-1){free(map);free(result);return -2;}
        for(size_t i=begin;i<at;++i)map[i]=used;
        if(n)memcpy(result+used,value?value:text+begin,n);
        used+=n;map[at]=used;
    }
    result[used]=0;map[length]=used;*output=result;*source_map=map;return 0;
}
