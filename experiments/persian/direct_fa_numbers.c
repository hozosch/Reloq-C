/* Direct Persian cardinal/decimal experiment, GPL-3.0-or-later. */
#include "direct_fa_numbers.h"
#include <stdint.h>
#include <string.h>
#include "direct_fa_numbers.inc"

struct number_output { char *out;size_t used,capacity; };

static int digit(char c) { return c>='0' && c<='9'; }
static size_t arabic_mark(const char *text,size_t length,size_t at,unsigned char last)
{ return at+1<length && (unsigned char)text[at]==0xd9 && (unsigned char)text[at+1]==last ? 2:0; }

static int emit(struct number_output *b,const char *text,int weaken)
{
    size_t n=strlen(text),space=b->used?1:0;
    if(n>SIZE_MAX-b->used-space-1 || b->used+space+n>=b->capacity)return -2;
    if(space)b->out[b->used++]=' ';
    for(size_t i=0;i<n;++i) {
        char value=text[i];
        if(weaken && value=='1' && i && text[i-1]=='.')value=weaken==1?'0':'2';
        b->out[b->used++]=value;
    }
    b->out[b->used]=0;return 0;
}

static int emit_cardinal(struct number_output *b,const char *digits,size_t n,int following)
{
    if(n>1 && digits[0]=='0') {
        /* Preserve leading zeroes (identifiers and phone-like strings). */
        for(size_t i=0;i<n;++i)
            if(emit(b,fa_number_groups[digits[i]-'0'],i+1<n?1:following?2:0))return -2;
        return 0;
    }
    unsigned groups[4]={0};size_t group=0;
    while(n) {
        size_t first=n>3?n-3:0;unsigned value=0;
        for(size_t i=first;i<n;++i)value=value*10+(unsigned)(digits[i]-'0');
        groups[group++]=value;n=first;
    }
    if(group==1 && groups[0]==0)return emit(b,fa_number_groups[0],following?2:0);
    while(group) {
        size_t position=--group;
        if(!groups[position])continue;
        int lower=0;
        for(size_t j=0;j<position;++j)if(groups[j])lower=1;
        if(emit(b,fa_number_groups[groups[position]],position?(lower?2:1):(following?2:0)))return -2;
        if(position && emit(b,fa_number_scales[position-1][lower?1:0],following&&!lower?2:0))return -2;
    }
    return 0;
}

int reloq_fa_direct_number(const char *text,size_t length,size_t *consumed,
                           char *output,size_t capacity)
{
    if(consumed)*consumed=0;
    if(output && capacity)output[0]=0;
    if(!text || !length || !consumed || !output || !capacity || !digit(text[0]))return -1;
    char integer[12],fraction[32];size_t ni=0,nf=0,at=0;
    while(at<length && digit(text[at])) {
        if(ni==sizeof integer)return 2;
        integer[ni++]=text[at++];
    }
    size_t group_width=0;int group_kind=0;
    while(at<length) {
        int kind=text[at]==','?1:arabic_mark(text,length,at,0xac)?2:0;
        if(!kind)break;
        size_t width=kind==1?1:2;
        if(at+width>=length || !digit(text[at+width])) {
            if(kind==1)break; /* a normal comma after a number */
            return 2;
        }
        if(!group_kind) {if(ni>3)return 2;group_kind=kind;}
        if(group_kind!=kind)return 2;
        at+=width;group_width=0;
        while(at<length && digit(text[at])) {
            if(ni==sizeof integer || group_width==3)return 2;
            integer[ni++]=text[at++];++group_width;
        }
        if(group_width!=3)return 2;
    }
    size_t decimal_width=at<length && text[at]=='.'?1:arabic_mark(text,length,at,0xab);
    if(decimal_width) {
        if(at+decimal_width>=length || !digit(text[at+decimal_width])) {
            if(decimal_width==2)return 2;
            decimal_width=0; /* sentence-final full stop */
        } else {
            at+=decimal_width;
            while(at<length && digit(text[at])) {
                if(nf==sizeof fraction)return 2;
                fraction[nf++]=text[at++];
            }
        }
    }
    if(nf && at<length) {
        size_t width=text[at]=='.'||text[at]==','?1:
                     (arabic_mark(text,length,at,0xab)||arabic_mark(text,length,at,0xac));
        if(width && at+width<length && digit(text[at+width]))return 2;
    }
    struct number_output b={output,0,capacity};
    if(emit_cardinal(&b,integer,ni,nf!=0))return -2;
    if(nf) {
        if(emit(&b,fa_number_decimal,0))return -2;
        for(size_t i=0;i<nf;++i)
            if(emit(&b,fa_number_groups[fraction[i]-'0'],i+1<nf?1:0))return -2;
    }
    *consumed=at;return 0;
}
