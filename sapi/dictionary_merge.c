#include "dictionary_merge.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LIMIT (8u*1024u*1024u)
struct row {const unsigned char *key,*value;size_t key_len,value_len,order;int fold;};
static unsigned char lower(unsigned char c)
{return (c>='A'&&c<='Z')||(c>=0xc0&&c<=0xde&&c!=0xd7)?(unsigned char)(c+32):c;}
static int key_compare(const struct row *a,const struct row *b)
{
    size_t n=a->key_len<b->key_len?a->key_len:b->key_len;
    for(size_t i=0;i<n;++i){unsigned x=a->key[i],y=b->key[i];
        if(a->fold)x=lower((unsigned char)x);if(b->fold)y=lower((unsigned char)y);
        if(x!=y)return x<y?-1:1;}
    return a->key_len<b->key_len?-1:a->key_len>b->key_len?1:0;
}
static int row_compare(const void *one,const void *two)
{
    const struct row *a=one,*b=two;int key=key_compare(a,b);
    return key?key:a->order<b->order?-1:a->order>b->order?1:0;
}
static int space(unsigned char c){return c==' '||(c>='\t'&&c<='\r');}
int sapi_dictionary_merge(const struct sapi_dictionary_input *inputs,size_t count,
                          int fold_root,unsigned char **output,size_t *size)
{
    if(!output||!size||(!inputs&&count))return 0;*output=NULL;*size=0;
    struct row *rows=NULL;size_t used=0,capacity=0;
    for(size_t source=0;source<count;++source){
        const unsigned char *data=inputs[source].data;size_t length=inputs[source].size;
        if((!data&&length)||length>LIMIT)goto fail;
        for(size_t i=0;i<length;++i)if(!data[i])goto fail;
        for(size_t start=0;start<length;){
            size_t end=start;while(end<length&&data[end]!='\r'&&data[end]!='\n')++end;
            size_t tab=start;while(tab<end&&data[tab]!='\t')++tab;
            if(tab<end){
                size_t a=start,b=tab,c=tab+1,d=end;
                while(a<b&&space(data[a]))++a;while(b>a&&space(data[b-1]))--b;
                while(c<d&&space(data[c]))++c;while(d>c&&space(data[d-1]))--d;
                if(a<b&&c<d){
                    if(b-a>128||d-c>512)goto fail;
                    if(used==capacity){size_t next=capacity?capacity*2:256;
                        if(next<capacity||next>SIZE_MAX/sizeof *rows)goto fail;
                        void *p=realloc(rows,next*sizeof *rows);if(!p)goto fail;rows=p;capacity=next;}
                    rows[used]=(struct row){data+a,data+c,b-a,d-c,used,fold_root!=0};++used;
                }
            }
            start=end;while(start<length&&(data[start]=='\r'||data[start]=='\n'))++start;
        }
    }
    if(used)qsort(rows,used,sizeof *rows,row_compare);
    size_t total=0;
    for(size_t i=0;i<used;){size_t end=i+1;while(end<used&&!key_compare(rows+i,rows+end))++end;
        const struct row *last=rows+end-1;size_t need=last->key_len+last->value_len+2;
        if(need>LIMIT-total)goto fail;total+=need;i=end;}
    unsigned char *result=malloc(total?total:1);if(!result)goto fail;
    size_t at=0;
    for(size_t i=0;i<used;){size_t end=i+1;while(end<used&&!key_compare(rows+i,rows+end))++end;
        const struct row *last=rows+end-1;memcpy(result+at,last->key,last->key_len);at+=last->key_len;
        result[at++]='\t';memcpy(result+at,last->value,last->value_len);at+=last->value_len;result[at++]='\n';i=end;}
    free(rows);*output=result;*size=total;return 1;
fail:free(rows);return 0;
}
