#include "reloq_parameter_marks.h"
#include "reloq_inline_tags.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int reloq_parameter_marks_insert(const unsigned char *text,size_t n,
    const struct reloq_parameter_mark *marks,size_t count,
    unsigned char **out,size_t *length,size_t **source)
{
    if((n&&!text)||(count&&!marks)||!out||!length||!source)return -1;
    if(n>SIZE_MAX-1 || count>(SIZE_MAX-n-1)/10)return -2;
    size_t cap=n+count*10+1;
    if(cap>SIZE_MAX/sizeof(size_t))return -2;
    unsigned char *bytes=malloc(cap);size_t *map=malloc(cap*sizeof *map);
    if(!bytes||!map){free(bytes);free(map);return -2;}
    size_t at=0,used=0;
    for(size_t i=0;i<count;++i) {
        const struct reloq_parameter_mark *m=marks+i;
        if(m->offset>n || (i&&m->offset<marks[i-1].offset) || m->value<0 ||
           (m->kind!=RELOQ_TAG_SPEED && m->kind!=RELOQ_TAG_VOLUME) ||
           m->value>(m->kind==RELOQ_TAG_SPEED?250:100)) {
            free(bytes);free(map);return -1;
        }
        while(at<m->offset){bytes[used]=text[at];map[used++]=at++;}
        char command[10];int size=snprintf(command,sizeof command,"`v%c%d ",
            m->kind==RELOQ_TAG_SPEED?'s':'v',m->value);
        if(size<0 || (size_t)size>=sizeof command){free(bytes);free(map);return -1;}
        memcpy(bytes+used,command,(size_t)size);
        for(int j=0;j<size;++j)map[used++]=at;
    }
    while(at<n){bytes[used]=text[at];map[used++]=at++;}
    bytes[used]=0;map[used]=n;
    *out=bytes;*length=used;*source=map;return 0;
}
