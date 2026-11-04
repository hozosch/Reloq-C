#include "dictionary_diff.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct key {const unsigned char *text;size_t length;int fold;};
static unsigned char lower(unsigned char c)
{return (c>='A'&&c<='Z')||(c>=0xc0&&c<=0xde&&c!=0xd7)?(unsigned char)(c+32):c;}
static int compare(const void *one,const void *two)
{
    const struct key *a=one,*b=two;size_t n=a->length<b->length?a->length:b->length;
    for(size_t i=0;i<n;++i){unsigned x=a->text[i],y=b->text[i];
        if(a->fold)x=lower((unsigned char)x);if(b->fold)y=lower((unsigned char)y);
        if(x!=y)return x<y?-1:1;}
    return a->length<b->length?-1:a->length>b->length?1:0;
}
static int space(unsigned char c){return c==' '||(c>='\t'&&c<='\r');}
static int collect(const unsigned char *data,size_t length,int fold,struct key **out,size_t *count)
{
    struct key *rows=NULL;size_t used=0,cap=0;
    if(!data&&length)return 0;
    for(size_t i=0;i<length;++i)if(!data[i])return 0;
    for(size_t start=0;start<length;) {
        size_t end=start;while(end<length&&data[end]!='\r'&&data[end]!='\n')++end;
        size_t tab=start;while(tab<end&&data[tab]!='\t')++tab;
        if(tab<end) {
            size_t key_start=start,key_end=tab,value_start=tab+1,value_end=end;
            while(key_start<key_end&&space(data[key_start]))++key_start;
            while(key_end>key_start&&space(data[key_end-1]))--key_end;
            while(value_start<value_end&&space(data[value_start]))++value_start;
            while(value_end>value_start&&space(data[value_end-1]))--value_end;
            if(key_start<key_end&&value_start<value_end) {
                if(key_end-key_start>128||value_end-value_start>512){free(rows);return 0;}
                if(used==cap){size_t next=cap?cap*2:256;
                    if(next<cap||next>SIZE_MAX/sizeof *rows){free(rows);return 0;}
                    void *p=realloc(rows,next*sizeof *rows);if(!p){free(rows);return 0;}rows=p;cap=next;}
                rows[used++]=(struct key){data+key_start,key_end-key_start,fold};
            }
        }
        start=end;while(start<length&&(data[start]=='\r'||data[start]=='\n'))++start;
    }
    if(used)qsort(rows,used,sizeof *rows,compare);
    *out=rows;*count=used;return 1;
}
int sapi_dictionary_diff(const unsigned char *before,size_t before_size,
    const unsigned char *after,size_t after_size,int fold_root,size_t *added,size_t *total)
{
    if(!added||!total)return 0;*added=*total=0;
    struct key *old=NULL,*current=NULL;size_t old_n=0,new_n=0;
    if(!collect(before,before_size,fold_root,&old,&old_n)||
       !collect(after,after_size,fold_root,&current,&new_n)){free(old);free(current);return 0;}
    size_t old_at=0;
    for(size_t i=0;i<new_n;++i){
        if(i&&compare(current+i-1,current+i)==0)continue;
        while(old_at<old_n&&compare(old+old_at,current+i)<0)++old_at;
        if(old_at==old_n||compare(old+old_at,current+i)!=0)++*added;
        ++*total;
    }
    free(old);free(current);return 1;
}
uint32_t sapi_dictionary_pack_counts(size_t entries,size_t files,int compared)
{
    if(entries>=0xffffffu||files>=128)return 0;
    return (uint32_t)(entries+1)|((uint32_t)files<<24)|(compared?0:0x80000000u);
}
int sapi_dictionary_unpack_counts(uint32_t value,size_t *entries,size_t *files,int *compared)
{
    if(!value||!(value&0xffffffu)||!entries||!files||!compared)return 0;
    *entries=(value&0xffffffu)-1;*files=(value>>24)&0x7fu;*compared=(value&0x80000000u)==0;
    return 1;
}
