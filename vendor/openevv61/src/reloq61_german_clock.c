#include "reloq61_german_clock.h"
#include "reloq_inline_tags.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port/evv_arena.h"

struct clock_fields {size_t hour_end,minute_end,seconds_start,end;unsigned hour,minute,second;int seconds;};
static int digit(unsigned char c){return c>='0'&&c<='9';}
static int word(unsigned char c)
{return digit(c)||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'||c>=128;}
static int space(unsigned char c){return c==' '||c=='\t'||c==0xa0;}
/* The native number grammar understands dot-separated thousands. Preserve
 * the established German treatment of space and NBSP groups by projecting
 * those separators to dots before handing the number to that grammar. */
static int grouped_number_at(const char *s,size_t n,size_t at,size_t *end)
{
    if(!digit((unsigned char)s[at]) ||
       (at&&(word((unsigned char)s[at-1])||s[at-1]==':'||s[at-1]=='.'||s[at-1]=='/')))return 0;
    size_t i=at;
    while(i<n&&digit((unsigned char)s[i])&&i-at<3)++i;
    if(i==at||(i<n&&digit((unsigned char)s[i])))return 0;
    size_t last=i;int groups=0;
    while(i<n&&space((unsigned char)s[i])) {
        while(i<n&&space((unsigned char)s[i]))++i;
        size_t first=i;
        while(i<n&&digit((unsigned char)s[i])&&i-first<3)++i;
        if(i-first!=3||(i<n&&((word((unsigned char)s[i])&&!space((unsigned char)s[i]))||s[i]==':'||s[i]=='/')))break;
        last=i;++groups;
    }
    *end=last;return groups!=0;
}
static int clock_at(const char *s,size_t n,size_t at,struct clock_fields *c)
{
    if(!digit((unsigned char)s[at]) ||
       (at&&(word((unsigned char)s[at-1])||s[at-1]==':'||s[at-1]=='.'||s[at-1]=='/')))return 0;
    size_t i=at;unsigned hour=0,minute=0,second=0;
    while(i<n&&digit((unsigned char)s[i])&&i-at<2)hour=hour*10+(unsigned)(s[i++]-'0');
    if(hour>24||i==n||(s[i]!=':'&&s[i]!='.'))return 0;
    size_t hour_end=i;char separator=s[i++];size_t minute_start=i;
    while(i<n&&digit((unsigned char)s[i])&&i-minute_start<2)minute=minute*10+(unsigned)(s[i++]-'0');
    if(i==minute_start||minute>59||(i<n&&digit((unsigned char)s[i])))return 0;
    size_t minute_end=i,seconds_start=i;int seconds=0;
    if(separator==':'&&i<n&&s[i]==':') {
        seconds=1;seconds_start=++i;
        while(i<n&&digit((unsigned char)s[i])&&i-seconds_start<2)second=second*10+(unsigned)(s[i++]-'0');
        if(i==seconds_start||second>59||(i<n&&digit((unsigned char)s[i])))return 0;
    }
    if(i<n&&(s[i]==':'||s[i]=='/'||(s[i]=='.'&&i+1<n&&digit((unsigned char)s[i+1]))))return 0;
    size_t unit=i;while(unit<n&&space((unsigned char)s[unit]))++unit;
    int has_unit=unit+3<=n&&(s[unit]=='u'||s[unit]=='U')&&
        (s[unit+1]=='h'||s[unit+1]=='H')&&(s[unit+2]=='r'||s[unit+2]=='R')&&
        (unit+3==n||!word((unsigned char)s[unit+3]));
    if(separator=='.'&&(!has_unit||minute_end-minute_start!=2))return 0;
    if(!has_unit&&i<n&&word((unsigned char)s[i]))return 0;
    c->hour=hour;c->minute=minute;c->second=second;c->seconds=seconds;
    c->hour_end=hour_end;c->minute_end=minute_end;c->seconds_start=seconds_start;
    c->end=has_unit?unit+3:i;return 1;
}
static size_t tag_at(const char *s,size_t n,size_t at,int annotations,int *language,int *spelling)
{
    if(!annotations||s[at]!='`')return 0;
    struct reloq_inline_tag t=reloq_inline_tag_read((const unsigned char *)s+at,n-at,1);
    if(t.kind==RELOQ_TAG_SPELL)*spelling=t.value;
    if(t.kind==RELOQ_TAG_LANGUAGE)*language=t.value;
    return t.length;
}
/* As in the English seconds rule, the native grammar remains responsible for
 * the resulting words and sentence prosody. A map keeps indices and inline
 * controls tied to the original input rather than the expanded spelling. */
int reloq61_german_clock_text(const char *s,int annotations,int language,char **out,size_t **positions)
{
    if(!s||!out||!positions)return -1;
    *out=NULL;*positions=NULL;
    if(!strchr(s,':')&&!strchr(s,'.')&&!strchr(s,' ')&&!strchr(s,'\t')&&!strchr(s,'\xa0'))return 0;
    size_t n=strlen(s),count=0;int spelling=0,initial=language;struct clock_fields c;
    for(size_t at=0;at<n;) {
        size_t tag=tag_at(s,n,at,annotations,&language,&spelling);
        if(tag){at+=tag;continue;}
        size_t end;
        if(!spelling&&language==0x40000&&clock_at(s,n,at,&c)){++count;at=c.end;}
        else if(!spelling&&language==0x40000&&grouped_number_at(s,n,at,&end)){++count;at=end;}
        else ++at;
    }
    if(!count)return 0;
    if(count>(SIZE_MAX-n-1)/40||n>SIZE_MAX/sizeof(size_t)-1)return -1;
    char *text=malloc(n+count*40+1);size_t *map=malloc((n+1)*sizeof *map);
    if(!text||!map){free(text);free(map);return -1;}
    size_t at=0,used=0;spelling=0;language=initial;map[0]=0;
    while(at<n) {
        size_t tag=tag_at(s,n,at,annotations,&language,&spelling);
        if(tag) {while(tag--){map[at]=used;text[used++]=s[at++];map[at]=used;}continue;}
        size_t end;
        if(!spelling&&language==0x40000&&clock_at(s,n,at,&c)) {
            char phrase[64];
            /* German says ein Uhr, rather than eins Uhr. For a nonzero
             * minute the weak Uhr is an explicit accent; retain the hour's
             * normal accented form when that accent separates it from Uhr. */
            int k;
            if(c.hour==1&&c.minute&&annotations)
                k=snprintf(phrase,sizeof phrase,"`[.1'aj'n]");
            else k=snprintf(phrase,sizeof phrase,"%u",c.hour);
            memcpy(text+used,phrase,(size_t)k);used+=(size_t)k;
            while(at<c.hour_end)map[++at]=used;
            if(c.minute&&annotations) {memcpy(text+used," `00 Uhr",8);used+=8;}
            else {memcpy(text+used," Uhr",4);used+=4;}
            map[++at]=used;
            if(c.minute) {k=snprintf(phrase,sizeof phrase," %u",c.minute);memcpy(text+used,phrase,(size_t)k);used+=(size_t)k;}
            while(at<c.minute_end)map[++at]=used;
            if(c.seconds) {
                memcpy(text+used," und ",5);used+=5;
                while(at<c.seconds_start)map[++at]=used;
                if(c.second==1) {memcpy(text+used,"eine Sekunde",12);used+=12;}
                else {k=snprintf(phrase,sizeof phrase,"%u Sekunden",c.second);memcpy(text+used,phrase,(size_t)k);used+=(size_t)k;}
            }
            while(at<c.end)map[++at]=used;
        } else if(!spelling&&language==0x40000&&grouped_number_at(s,n,at,&end)) {
            while(at<end) {
                if(space((unsigned char)s[at])) {
                    text[used++]='.';
                    do {map[++at]=used;} while(at<end&&space((unsigned char)s[at]));
                } else {text[used++]=s[at++];map[at]=used;}
            }
        } else {map[at]=used;text[used++]=s[at++];map[at]=used;}
    }
    text[used]=0;*out=text;*positions=map;return 1;
}
