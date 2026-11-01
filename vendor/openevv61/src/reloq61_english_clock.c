#include "reloq61_english_clock.h"
#include "reloq_inline_tags.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "port/evv_arena.h"

struct clock_field {size_t colon,end;unsigned seconds;int singular;};
static int digit(unsigned char c) {return c>='0'&&c<='9';}
static int word(unsigned char c)
{return digit(c)||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'||c>=128;}
static int clock_at(const char *s,size_t n,size_t at,struct clock_field *out)
{
    if(!digit((unsigned char)s[at]) || (at&&(word((unsigned char)s[at-1])||s[at-1]==':')))return 0;
    size_t i=at;unsigned hour=0;
    while(i<n&&digit((unsigned char)s[i])&&i-at<2)hour=hour*10+(unsigned)(s[i++]-'0');
    if(hour>23||i+6>n||s[i]!=':'||!digit(s[i+1])||!digit(s[i+2])||s[i+3]!=':'||
       !digit(s[i+4])||!digit(s[i+5]))return 0;
    unsigned minute=(unsigned)(s[i+1]-'0')*10+(unsigned)(s[i+2]-'0');
    unsigned seconds=(unsigned)(s[i+4]-'0')*10+(unsigned)(s[i+5]-'0');
    if(minute>59||seconds>59)return 0;
    size_t end=i+6;int fractional_nonzero=0;
    if(end+1<n&&s[end]=='.'&&digit(s[end+1])) {
        ++end;
        while(end<n&&digit(s[end])){if(s[end]!='0')fractional_nonzero=1;++end;}
    }
    if(end<n&&(word((unsigned char)s[end])||s[end]==':'))return 0;
    *out=(struct clock_field){i+3,end,seconds,seconds==1&&!fractional_nonzero};return 1;
}
static size_t tag_at(const char *s,size_t n,size_t at,int annotations,int *language,int *spelling)
{
    if(!annotations||s[at]!='`')return 0;
    struct reloq_inline_tag t=reloq_inline_tag_read((const unsigned char *)s+at,n-at,1);
    if(t.kind==RELOQ_TAG_SPELL)*spelling=t.value;
    if(t.kind==RELOQ_TAG_LANGUAGE)*language=t.value;
    return t.length;
}
int reloq61_english_clock_text(const char *s,int annotations,int language,char **out,size_t **positions)
{
    if(!s||!out||!positions)return -1;
    *out=NULL;*positions=NULL;
    if(!strchr(s,':'))return 0;
    size_t n=strlen(s),count=0;int spelling=0,initial=language;
    struct clock_field c;
    for(size_t at=0;at<n;) {
        size_t tag=tag_at(s,n,at,annotations,&language,&spelling);
        if(tag){at+=tag;continue;}
        if(!spelling&&(language==0x10000||language==0x10001)&&clock_at(s,n,at,&c)) {++count;at=c.end;}
        else ++at;
    }
    if(!count)return 0;
    if(count>(SIZE_MAX-n-1)/18||n>SIZE_MAX/sizeof(size_t)-1)return -1;
    char *text=malloc(n+count*18+1);size_t *map=malloc((n+1)*sizeof *map);
    if(!text||!map){free(text);free(map);return -1;}
    size_t at=0,used=0;spelling=0;language=initial;map[0]=0;
    while(at<n) {
        size_t tag=tag_at(s,n,at,annotations,&language,&spelling);
        if(tag) {
            while(tag--){map[at]=used;text[used++]=s[at++];map[at]=used;}
            continue;
        }
        if(!spelling&&(language==0x10000||language==0x10001)&&clock_at(s,n,at,&c)) {
            while(at<c.colon){map[at]=used;text[used++]=s[at++];map[at]=used;}
            map[at++]=used;
            memcpy(text+used," and ",5);used+=5;map[at]=used;
            if(s[at]=='0'){map[at++]=used;map[at]=used;}
            while(at<c.end){map[at]=used;text[used++]=s[at++];map[at]=used;}
            const char *unit=c.singular?" second":" seconds";size_t length=strlen(unit);
            memcpy(text+used,unit,length);used+=length;map[at]=used;
        } else {map[at]=used;text[used++]=s[at++];map[at]=used;}
    }
    text[used]=0;*out=text;*positions=map;return 1;
}
