/* Unicode name data, stored in independently readable lossless blocks. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "reloq_unicode_packed.inc"
#define COUNT(a) (sizeof(a)/sizeof *(a))

/* This format has no external library or startup unpacking. Each control
 * byte covers up to eight literal bytes or bounded backreferences. */
static int unpack_block(size_t group,unsigned char *out,size_t *written)
{
    if(group+1>=COUNT(local_name_packed_offsets))return -1;
    size_t at=local_name_packed_offsets[group],end=local_name_packed_offsets[group+1],used=0;
    if(at>end||end>sizeof local_name_packed_text)return -1;
    while(at<end) {
        unsigned control=local_name_packed_text[at++];
        for(unsigned bit=0;bit<8&&at<end;++bit) {
            if(control&(1u<<bit)) {
                if(end-at<2)return -1;
                unsigned value=(unsigned)local_name_packed_text[at]|((unsigned)local_name_packed_text[at+1]<<8);
                at+=2;
                size_t distance=(value&4095u)+1,length=(value>>12)+3;
                if(length==18) {
                    unsigned extra;
                    do {
                        if(at==end)return -1;
                        extra=local_name_packed_text[at++];length+=extra;
                    } while(extra==255);
                }
                if(distance>used||length>LOCAL_NAME_GROUP_MAX_BYTES-used)return -1;
                while(length--){out[used]=out[used-distance];++used;}
            } else {
                if(used==LOCAL_NAME_GROUP_MAX_BYTES)return -1;
                out[used++]=local_name_packed_text[at++];
            }
        }
    }
    *written=used;return 0;
}

int reloq_unicode_name_local(uint32_t cp,unsigned locale,char *text,size_t capacity)
{
    if(locale>=10||!text||!capacity)return 0;
    size_t lo=0,hi=COUNT(local_name_codepoints);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(local_name_codepoints[m]<cp)lo=m+1;else hi=m;}
    if(lo==COUNT(local_name_codepoints)||local_name_codepoints[lo]!=cp)return 0;
    unsigned char block[LOCAL_NAME_GROUP_MAX_BYTES];size_t size=0;
    size_t group=(size_t)locale*LOCAL_NAME_GROUPS_PER_LOCALE+lo/64;
    if(unpack_block(group,block,&size))return -1;
    size_t offset=0,length=0,skip=(lo%64)/16*16;
    /* Each original 16-name block starts afresh. Skipping earlier blocks
     * must not apply their intermediate lengths to the caller's buffer. */
    for(size_t i=0;i<skip;++i) {
        if(size-offset<2)return -1;
        offset+=2;
        const unsigned char *end=memchr(block+offset,0,size-offset);
        if(!end)return -1;
        offset=(size_t)(end-block)+1;
    }
    for(size_t i=0;i<=lo%16;++i) {
        if(size-offset<2)return -1;
        unsigned prefix=(unsigned)block[offset]|((unsigned)block[offset+1]<<8);
        offset+=2;
        const unsigned char *end=memchr(block+offset,0,size-offset);
        if(!end||prefix>length)return -1;
        size_t tail=(size_t)(end-block)-offset;
        if(prefix>=capacity||tail>=capacity-prefix)return -1;
        memcpy(text+prefix,block+offset,tail);
        length=prefix+tail;text[length]=0;offset=(size_t)(end-block)+1;
    }
    return 1;
}
