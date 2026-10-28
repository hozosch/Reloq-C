/* Literal Unicode name data with bounded lossless string reconstruction. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "reloq_unicode_names.inc"
#define COUNT(a) (sizeof(a)/sizeof *(a))
int reloq_unicode_name_local(uint32_t cp,unsigned locale,char *text,size_t capacity)
{
    if(locale>=COUNT(local_name_blocks)||!text||!capacity)return 0;
    size_t lo=0,hi=COUNT(local_name_codepoints);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(local_name_codepoints[m]<cp)lo=m+1;else hi=m;}
    if(lo==COUNT(local_name_codepoints)||local_name_codepoints[lo]!=cp)return 0;
    size_t offset=local_name_blocks[locale][lo/16],length=0;
    for(size_t i=0;i<=lo%16;++i) {
        if(offset>sizeof local_name_text-2)return -1;
        unsigned prefix=(unsigned)local_name_text[offset]|((unsigned)local_name_text[offset+1]<<8);
        offset+=2;
        const unsigned char *end=memchr(local_name_text+offset,0,sizeof local_name_text-offset);
        if(!end||prefix>length)return -1;
        size_t tail=(size_t)(end-local_name_text)-offset;
        if(prefix>=capacity||tail>=capacity-prefix)return -1;
        memcpy(text+prefix,local_name_text+offset,tail);
        length=prefix+tail;text[length]=0;offset=(size_t)(end-local_name_text)+1;
    }
    return 1;
}
