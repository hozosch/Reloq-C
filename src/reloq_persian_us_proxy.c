#include "reloq_persian_us_proxy.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct buf {char *s;size_t n,cap;};
struct word {size_t start,end;char *candidate;size_t candidate_len;int safe,checked;};
struct syllable {char stress;const char *body;size_t len;};

static int append(struct buf *b,const char *s,size_t n)
{
    if(n>SIZE_MAX-b->n-1)return -2;
    size_t want=b->n+n+1;
    if(want>b->cap) {
        size_t cap=b->cap?b->cap:128;
        while(cap<want)cap=cap>SIZE_MAX/2?want:cap*2;
        void *p=realloc(b->s,cap);if(!p)return -2;
        b->s=p;b->cap=cap;
    }
    if(n)memcpy(b->s+b->n,s,n);
    b->n+=n;b->s[b->n]=0;return 0;
}
static int emit(void *context,const char *data,size_t n)
{
    if(!data && n)return -1;
    return append(context,data,n);
}
static int word_at(const char *s,size_t n,size_t from,size_t *start,size_t *end)
{
    for(size_t i=from;i+2<n;++i)if(s[i]=='`' && s[i+1]=='[') {
        size_t j=i+2;while(j<n && s[j]!=']')++j;
        if(j<n && j>i+2){*start=i;*end=j+1;return 1;}
    }
    return 0;
}
static int isvowel(char c) {return c && strchr("AaEeiou",c)!=NULL;}
static int issmallspace(unsigned char c)
{return c==' '||c=='\t'||c=='\n'||c=='\r'||c=='\v'||c=='\f'||(c>=0x1c && c<=0x1f);}

/* Equivalent to the existing .([012])([^.]+) syllable parser. */
static int syllables(const char *s,size_t n,struct syllable **out,size_t *count)
{
    if(n>SIZE_MAX/sizeof(struct syllable))return -2;
    struct syllable *a=malloc((n?n:1)*sizeof *a);if(!a)return -2;
    size_t used=0;
    for(size_t i=0;i+2<n;) {
        if(s[i]=='.' && s[i+1]>='0' && s[i+1]<='2' && s[i+2]!='.') {
            size_t end=i+2;while(end<n && s[end]!='.')++end;
            a[used++]=(struct syllable){s[i+1],s+i+2,end-i-2};i=end;
        } else ++i;
    }
    *out=a;*count=used;return 0;
}
static int equivalent(char a,char b,char stress)
{return a==b || ((a=='d'||a=='t') && b=='F') || (stress=='0' && a=='A' && b=='e');}
static int compatible(const char *expected,size_t en,const char *actual,size_t an)
{
    struct syllable *e=NULL,*a=NULL;size_t ec=0,ac=0;
    int status=syllables(expected,en,&e,&ec);
    if(!status)status=syllables(actual,an,&a,&ac);
    if(status){free(e);free(a);return -2;}
    int ok=ec && ec==ac;
    for(size_t i=0;ok && i<ec;++i) {
        if(e[i].stress=='1' && a[i].stress!='1'){ok=0;break;}
        size_t x=0,y=0;
        for(;;) {
            while(x<e[i].len && !isvowel(e[i].body[x]))++x;
            while(y<a[i].len && !isvowel(a[i].body[y]))++y;
            if(x==e[i].len || y==a[i].len){if(x!=e[i].len || y!=a[i].len)ok=0;break;}
            if(!equivalent(e[i].body[x++],a[i].body[y++],e[i].stress)){ok=0;break;}
        }
    }
    size_t ei=0,ep=0,ai=0,ap=0;
    while(ok && ei<ec && ai<ac) {
        if(ep==e[ei].len){++ei;ep=0;continue;}
        if(ap==a[ai].len){++ai;ap=0;continue;}
        if(!equivalent(e[ei].body[ep],a[ai].body[ap],e[ei].stress))ok=0;
        ++ep;++ap;
    }
    while(ei<ec && ep==e[ei].len){++ei;ep=0;}
    while(ai<ac && ap==a[ai].len){++ai;ap=0;}
    if(ei!=ec || ai!=ac)ok=0;
    free(e);free(a);return ok;
}
static char convert(char c)
{
    if(c && strchr("aAioubdfghklmnprstvzSZ.012",c))return c;
    switch(c) {case 'e':return 'E';case 'j':return 'y';case 'G':return 'h';
        case 'R':return 'k';case 'J':return 'y';case 'C':return 'S';default:return 0;}
}
static int candidate(const char *s,size_t n,struct word *w)
{
    struct buf b={0};int status=append(&b,"`[",2);
    for(size_t i=2;!status && i+1<n;++i) {
        char c=s[i];
        if(c=='\'' || c==':')continue;
        if(i+2==n && c=='r' && i>2 && strchr("fptkbdg",s[i-1]))c='s';
        c=convert(c);if(!c){free(b.s);return 0;}
        status=append(&b,&c,1);
    }
    if(!status)status=append(&b,"]",1);
    if(status){free(b.s);return status;}
    w->candidate=b.s;w->candidate_len=b.n;return 0;
}
static int fallback(const char *s,size_t n,struct buf *b,int *weak)
{
    struct syllable *a=NULL;size_t count=0;int status=syllables(s,n,&a,&count);
    if(status)return status;
    if(!count){free(a);return 1;}
    size_t primary=0,secondary=SIZE_MAX;
    for(size_t i=0;i<count;++i) {
        if(a[i].stress=='1')primary=1;
        if(a[i].stress=='2' && secondary==SIZE_MAX)secondary=i;
    }
    *weak=count>1 && !primary;
    size_t anchor=secondary==SIZE_MAX?0:secondary;
    status=append(b,"`[",2);
    for(size_t i=0;!status && i<count;++i) {
        char vowel='A';
        for(size_t j=0;j<a[i].len;++j) {
            char c=a[i].body[j];
            if(c && strchr("Aaeiou",c)){vowel=c=='e'?'E':c;break;}
        }
        char syll[4]={'.',(*weak && i==anchor)?'1':a[i].stress,'m',vowel};
        status=append(b,syll,sizeof syll);
    }
    if(!status)status=append(b,"]",1);
    free(a);return status;
}
static int all_zero(const char *s,size_t n)
{
    struct syllable *a=NULL;size_t count=0;int status=syllables(s,n,&a,&count);
    if(status)return -2;
    int result=count>0;
    for(size_t i=0;i<count;++i)if(a[i].stress!='0')result=0;
    free(a);return result;
}
static size_t unicode_space(const unsigned char *s,size_t n);
static size_t trailing_space(const char *s,size_t start,size_t end)
{
    while(end>start) {
        if(issmallspace((unsigned char)s[end-1])){--end;continue;}
        size_t width=0;
        for(size_t k=2;k<=3 && k<=end-start;++k)
            if(unicode_space((const unsigned char *)s+end-k,k)==k){width=k;break;}
        if(!width)break;
        end-=width;
    }
    return end;
}
static int ends_weak(const char *s,size_t n)
{
    while(n && issmallspace((unsigned char)s[n-1]))--n;
    return n>=3 && !memcmp(s+n-3,"`00",3);
}
static int run_probe(reloq_fa_proxy_probe probe,void *ctx,const char *s,size_t n,struct buf *out)
{
    int status=probe(ctx,s,n,emit,out);
    if(status)return status==-2?-2:-1;
    return 0;
}

/* Unicode whitespace accepted by Python's str.isspace(), other than ASCII
 * tab/newline/return, which the existing planner preserves. */
static size_t unicode_space(const unsigned char *s,size_t n)
{
    if(n>=2 && s[0]==0xc2 && (s[1]==0x85 || s[1]==0xa0))return 2;
    if(n>=3 && s[0]==0xe1 && s[1]==0x9a && s[2]==0x80)return 3;
    if(n>=3 && s[0]==0xe2 && s[1]==0x80 &&
       ((s[2]>=0x80 && s[2]<=0x8a) || s[2]==0xa8 || s[2]==0xa9 || s[2]==0xaf))return 3;
    if(n>=3 && s[0]==0xe2 && s[1]==0x81 && s[2]==0x9f)return 3;
    if(n>=3 && s[0]==0xe3 && s[1]==0x80 && s[2]==0x80)return 3;
    return 0;
}
static int append_gap(struct buf *b,const char *s,size_t n)
{
    for(size_t i=0;i<n;) {
        size_t ws=unicode_space((const unsigned char *)s+i,n-i);
        if(ws){int r=append(b," ",1);if(r)return r;i+=ws;continue;}
        char c=s[i++];
        if(c && strchr("()[]{}",c))c=',';
        else if((unsigned char)c==0x0b || (unsigned char)c==0x0c ||
                ((unsigned char)c>=0x1c && (unsigned char)c<=0x1f))c=' ';
        int r=append(b,&c,1);if(r)return r;
    }
    return 0;
}
void reloq_fa_us_proxy_clear(struct reloq_fa_us_proxy *p)
{if(p){free(p->text);free(p->word_offsets);*p=(struct reloq_fa_us_proxy){0};}}

static int proxy_make(const char *s,size_t n,reloq_fa_proxy_probe probe,void *ctx,
    struct reloq_fa_us_proxy *result,int reuse)
{
    if(!result)return 1;
    *result=(struct reloq_fa_us_proxy){0};
    if(!s || !probe || n==SIZE_MAX || memchr(s,0,n))return 1;
    struct word *w=NULL;size_t count=0,cap=0,start,end,cursor=0;
    int status=0;struct buf joined={0},display={0},out={0};size_t *offsets=NULL;
    while(word_at(s,n,cursor,&start,&end)) {
        if(count==cap) {
            if(cap>SIZE_MAX/2/sizeof *w){status=-2;goto done;}
            cap=cap?cap*2:8;void *p=realloc(w,cap*sizeof *w);if(!p){status=-2;goto done;}w=p;
        }
        w[count]=(struct word){.start=start,.end=end};
        status=candidate(s+start,end-start,w+count);if(status)goto done;
        ++count;cursor=end;
    }
    if(!count){status=1;goto done;}
    size_t candidates=0;
    for(size_t i=0;i<count;++i)if(w[i].candidate) {
        if(candidates++ && (status=append(&joined," ",1)))goto done;
        if((status=append(&joined,w[i].candidate,w[i].candidate_len)))goto done;
    }
    if(candidates) {
        status=run_probe(probe,ctx,joined.s,joined.n,&display);if(status)goto done;
        size_t found=0,pos=0;
        while(word_at(display.s,display.n,pos,&start,&end)){++found;pos=end;}
        if(found==candidates) {
            pos=0;
            for(size_t i=0;i<count;++i)if(w[i].candidate) {
                (void)word_at(display.s,display.n,pos,&start,&end);
                int ok=compatible(w[i].candidate+2,w[i].candidate_len-3,display.s+start+2,end-start-3);
                if(ok<0){status=ok;goto done;}w[i].safe=ok;pos=end;
            }
        }
    }
    for(size_t i=0;i<count;++i)if(w[i].candidate && !w[i].safe) {
        if(reuse)for(size_t j=0;j<i;++j) {
            if(w[j].checked && w[j].candidate_len==w[i].candidate_len &&
               !memcmp(w[j].candidate,w[i].candidate,w[i].candidate_len)) {
                w[i].safe=w[j].safe;w[i].checked=1;break;
            }
        }
        if(w[i].checked)continue;
        struct buf one={0};status=run_probe(probe,ctx,w[i].candidate,w[i].candidate_len,&one);
        if(status){free(one.s);goto done;}
        size_t a,b,next_start,next_end;
        if(word_at(one.s,one.n,0,&a,&b) && !word_at(one.s,one.n,b,&next_start,&next_end)) {
            int ok=compatible(w[i].candidate+2,w[i].candidate_len-3,one.s+a+2,b-a-3);
            if(ok<0){free(one.s);status=ok;goto done;}w[i].safe=ok;
        }
        free(one.s);w[i].checked=1;
    }
    if(count>1) {offsets=malloc((count-1)*sizeof *offsets);if(!offsets){status=-2;goto done;}}
    cursor=0;
    for(size_t i=0;i<count;++i) {
        size_t gap_start=cursor,gap_end=w[i].start,boundary=gap_end;
        if(i) {
            for(size_t j=gap_start;j<gap_end;++j)if(s[j] && strchr(".,?!:;()[]{}",s[j])){boundary=gap_start;break;}
            if(boundary==gap_end) {
                size_t p=trailing_space(s,gap_start,gap_end);
                if(p>=gap_start+3 && !memcmp(s+p-3,"`00",3))boundary=p-3;
            }
            if((status=append_gap(&out,s+gap_start,boundary-gap_start)))goto done;
            offsets[i-1]=out.n;
            if(boundary<gap_end && s[boundary] && strchr(".,?!:;()[]{}",s[boundary]))
                if((status=append(&out," ",1)))goto done;
            if((status=append_gap(&out,s+boundary,gap_end-boundary)))goto done;
        } else if((status=append_gap(&out,s,gap_end)))goto done;
        int weak=0;
        struct buf tag={0};
        if(w[i].safe)status=append(&tag,w[i].candidate,w[i].candidate_len);
        else status=fallback(s+w[i].start+2,w[i].end-w[i].start-3,&tag,&weak);
        if(status){free(tag.s);goto done;}
        int zero=all_zero(tag.s+2,tag.n-3);
        if(zero<0){free(tag.s);status=zero;goto done;}
        /* A fallback's weak control is inserted before the tilde pass; an
         * all-zero candidate receives its control after that pass. */
        if(out.n && out.s[out.n-1]=='~' && i && gap_end-gap_start==1 && !weak)out.s[out.n-1]=' ';
        if((weak || zero) && !ends_weak(out.s,out.n))status=append(&out,"`00 ",4);
        if(!status)status=append(&out,tag.s,tag.n);
        free(tag.s);if(status)goto done;
        cursor=w[i].end;
    }
    if((status=append_gap(&out,s+cursor,n-cursor)))goto done;
    result->text=out.s;result->length=out.n;result->word_offsets=offsets;result->word_count=count;
    out.s=NULL;offsets=NULL;
done:
    for(size_t i=0;i<count;++i)free(w[i].candidate);
    free(w);free(joined.s);free(display.s);free(out.s);free(offsets);return status;
}

int reloq_fa_us_proxy_make(const char *s,size_t n,reloq_fa_proxy_probe probe,void *ctx,
    struct reloq_fa_us_proxy *result)
{return proxy_make(s,n,probe,ctx,result,0);}

int reloq_fa_us_proxy_make_reusing(const char *s,size_t n,reloq_fa_proxy_probe probe,void *ctx,
    struct reloq_fa_us_proxy *result)
{return proxy_make(s,n,probe,ctx,result,1);}
