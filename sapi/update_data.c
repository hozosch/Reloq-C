/* Bounded JSON release-metadata parser. This module is platform independent. */
#include "update_data.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

struct token {size_t begin,end;int next;char kind;};
struct parser {const char *s;size_t n,pos;struct token *t;int count,cap;};
static void ws(struct parser *p) {while(p->pos<p->n && (p->s[p->pos]==' '||p->s[p->pos]=='\r'||p->s[p->pos]=='\n'||p->s[p->pos]=='\t'))++p->pos;}
static int value(struct parser *p,int depth)
{
    if(depth>64)return -1;ws(p);if(p->pos>=p->n||p->count>=p->cap)return -1;
    int i=p->count++;struct token *t=p->t+i;t->begin=p->pos;char ch=p->s[p->pos++];t->kind=ch;
    if(ch=='"') {
        int closed=0;
        while(p->pos<p->n){unsigned char c=(unsigned char)p->s[p->pos++];
            if(c=='"'){closed=1;break;}if(c<32)return -1;
            if(c=='\\'){if(p->pos>=p->n)return -1;c=(unsigned char)p->s[p->pos++];
                if(c=='u'){for(int k=0;k<4;++k)if(p->pos>=p->n||!isxdigit((unsigned char)p->s[p->pos++]))return -1;}
                else if(!strchr("\"\\/bfnrt",c))return -1;}}
        if(!closed)return -1;
    } else if(ch=='{'||ch=='[') {
        char close=ch=='{'?'}':']';ws(p);
        if(p->pos<p->n&&p->s[p->pos]==close)++p->pos;
        else for(;;){
            if(ch=='{') {if(p->pos>=p->n||p->s[p->pos]!='"'||value(p,depth+1)<0)return -1;
                ws(p);if(p->pos>=p->n||p->s[p->pos++]!=':')return -1;}
            if(value(p,depth+1)<0)return -1;ws(p);if(p->pos>=p->n)return -1;
            if(p->s[p->pos]==close){++p->pos;break;}
            if(p->s[p->pos++]!=',')return -1;ws(p);
        }
    } else {
        while(p->pos<p->n&&!strchr(",]} \t\r\n",p->s[p->pos]))++p->pos;
        size_t z=p->pos-t->begin;const char *s=p->s+t->begin;
        if(!((z==4&&!memcmp(s,"true",4))||(z==5&&!memcmp(s,"false",5))||
             (z==4&&!memcmp(s,"null",4))||(ch=='-'||isdigit((unsigned char)ch))))return -1;
    }
    t->end=p->pos;t->next=p->count;return i;
}
static int equal(const struct parser *p,int i,const char *s)
{return i>=0&&p->t[i].kind=='"'&&p->t[i].end-p->t[i].begin==strlen(s)+2&&
       !memcmp(p->s+p->t[i].begin+1,s,strlen(s));}
static int field(const struct parser *p,int obj,const char *name)
{
    int found=-1;if(obj<0||p->t[obj].kind!='{')return -2;
    for(int i=obj+1;i<p->t[obj].next;){int v=i+1;if(v>=p->t[obj].next)return -2;
        if(equal(p,i,name)){if(found!=-1)return -2;found=v;}i=p->t[v].next;}
    return found;
}
static int copy_string(const struct parser *p,int i,char *out,size_t cap)
{
    if(i<0||p->t[i].kind!='"')return 0;size_t n=p->t[i].end-p->t[i].begin-2;
    if(n>=cap)return 0;const char *s=p->s+p->t[i].begin+1;
    for(size_t k=0;k<n;++k)if((unsigned char)s[k]<32||s[k]=='\\'||s[k]=='"')return 0;
    memcpy(out,s,n);out[n]=0;return 1;
}
static int number(const char **s,unsigned *out)
{
    if(!isdigit((unsigned char)**s))return 0;unsigned n=0;int digits=0;
    while(isdigit((unsigned char)**s)){if(++digits>4)return 0;n=n*10+(unsigned)(*(*s)++-'0');}
    *out=n;return 1;
}
static int version(const char *s,unsigned out[3],const char **end)
{
    if(!number(&s,out)||*s++!='.'||!number(&s,out+1)||*s++!='.'||!number(&s,out+2))return 0;
    if(out[0]>2000||out[1]>9999||out[2]>9999)return 0;if(end)*end=s;return 1;
}
static int newer(const unsigned a[3],const unsigned b[3])
{for(int i=0;i<3;++i)if(a[i]!=b[i])return a[i]>b[i];return 0;}
static int same_version(const unsigned a[3],const unsigned b[3])
{return a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2];}
/* Decode the JSON string before examining whole lines. A literal "\\n" in
 * release text must not be mistaken for a real newline. */
static uint64_t release_epoch(const struct parser *p,int token)
{
    if(token<0||p->t[token].kind!='"')return 0;
    static const char prefix[]="<!-- ReLoq-C build:",suffix[]=" -->";
    size_t begin=p->t[token].begin+1,end=p->t[token].end-1,used=0;uint64_t found=0;
    char *body=malloc(end-begin+1);if(!body)return 0;
    for(size_t i=begin;i<end;++i) {
        unsigned char c=(unsigned char)p->s[i];
        if(c=='\\') {
            c=(unsigned char)p->s[++i];
            if(c=='n')c='\n';else if(c=='r')c='\r';else if(c=='t')c='\t';
            else if(c=='b')c='\b';else if(c=='f')c='\f';
            else if(c=='u') { /* Marker is ASCII; preserve encoded ASCII too. */
                unsigned x=0;for(int k=0;k<4;++k){unsigned char h=(unsigned char)p->s[++i];
                    x=x*16+(unsigned)(isdigit(h)?h-'0':tolower(h)-'a'+10);}
                c=x&&x<128?(unsigned char)x:'?';
            }
        }
        body[used++]=(char)c;
    }
    for(size_t at=0;at<used;) {
        size_t line_end=at;while(line_end<used&&body[line_end]!='\n')++line_end;
        size_t stop=line_end;if(stop>at&&body[stop-1]=='\r')--stop;
        if(stop-at<sizeof prefix-1||memcmp(body+at,prefix,sizeof prefix-1))goto next;
        size_t pos=at+sizeof prefix-1;uint64_t value=0;size_t digits=0;
        if(pos>=stop||body[pos]<'1'||body[pos]>'9')goto next;
        while(pos<stop&&isdigit((unsigned char)body[pos])) {
            if(++digits>12){free(body);return 0;}
            value=value*10+(unsigned)(body[pos++]-'0');
        }
        if(pos+sizeof suffix-1!=stop||memcmp(body+pos,suffix,sizeof suffix-1))goto next;
        if(value>UINT64_C(253402300799)||found){found=0;break;}
        found=value;
next:   at=line_end<used?line_end+1:used;
    }
    free(body);return found;
}
static int uint_field(const struct parser *p,int i,uint32_t *out)
{
    if(i<0)return 0;size_t n=p->t[i].end-p->t[i].begin;if(!n||n>10)return 0;
    uint64_t v=0;for(size_t j=p->t[i].begin;j<p->t[i].end;++j){unsigned char c=p->s[j];
        if(!isdigit(c))return 0;v=v*10+(c-'0');if(v>RELOQ_UPDATE_LIMIT)return 0;}
    if(!v)return 0;*out=(uint32_t)v;return 1;
}
static int trusted_url(const char *url,const char *name)
{
    const char prefix[]="https://github.com/hozosch/Reloq-C/releases/download/";
    if(strncmp(url,prefix,sizeof prefix-1))return 0;const char *p=url+sizeof prefix-1,*tag=p;
    while(*p&&*p!='/'){unsigned char c=*p++;if(!isalnum(c)&&c!='.'&&c!='_'&&c!='-')return 0;}
    if(p==tag||*p++!='/'||(p-tag==2&&tag[0]=='.')||(p-tag==3&&tag[0]=='.'&&tag[1]=='.'))return 0;
    return !strcmp(p,name);
}
int reloq_update_parse_revision(const char *json,size_t length,const char *installed,
    uint64_t installed_epoch,struct reloq_update *out)
{
    if(!json||!out||!installed||length>2*1024*1024)return -1;
    unsigned current[3];const char *end;if(!version(installed,current,&end)||*end)return -1;
    struct parser p={.s=json,.n=length,.cap=100000};p.t=calloc((size_t)p.cap,sizeof *p.t);if(!p.t)return -1;
    int root=value(&p,0),result=-1;ws(&p);if(root<0||p.pos!=length||p.t[root].kind!='{')goto done;
    int draft=field(&p,root,"draft"),pre=field(&p,root,"prerelease"),assets=field(&p,root,"assets");
    if(draft<0||pre<0||assets<0||p.t[assets].kind!='['||p.t[draft].end-p.t[draft].begin!=5||
       p.t[pre].end-p.t[pre].begin!=5||memcmp(json+p.t[draft].begin,"false",5)||memcmp(json+p.t[pre].begin,"false",5))goto done;
    int body=field(&p,root,"body");if(body==-2)goto done;
    uint64_t build_epoch=release_epoch(&p,body);
    result=0;
    for(int i=assets+1;i<p.t[assets].next;i=p.t[i].next){char name[96];
        int f=field(&p,i,"name");if(f==-2){result=-1;break;}if(!copy_string(&p,f,name,sizeof name))continue;
        const char *prefix="reloq-c-sapi5-";if(strncmp(name,prefix,strlen(prefix)))continue;
        unsigned candidate[3];const char *finish;if(!version(name+strlen(prefix),candidate,&finish)||strcmp(finish,"-setup.exe"))continue;
        if(!newer(candidate,current)&&!(same_version(candidate,current)&&installed_epoch&&build_epoch>installed_epoch))continue;
        struct reloq_update next={0};char digest[72];int u=field(&p,i,"browser_download_url"),s=field(&p,i,"size"),d=field(&p,i,"digest");
        next.build_epoch=build_epoch;
        if(!copy_string(&p,u,next.url,sizeof next.url)||!trusted_url(next.url,name)||
           !uint_field(&p,s,&next.size)||!copy_string(&p,d,digest,sizeof digest)||
           strlen(digest)!=71||strncmp(digest,"sha256:",7)){result=-1;break;}
        for(int k=0;k<64;++k){unsigned char c=digest[k+7];if(!isxdigit(c)){result=-1;break;}
            next.sha256[k]=(char)tolower(c);}if(result<0)break;
        memcpy(next.name,name,strlen(name)+1);
        size_t n=(size_t)(finish-(name+strlen(prefix)));memcpy(next.version,name+strlen(prefix),n);next.version[n]=0;
        if(result==0){*out=next;result=1;}else {unsigned best[3];const char *b;version(out->version,best,&b);if(newer(candidate,best))*out=next;}
    }
done:free(p.t);return result;
}

int reloq_update_parse(const char *json,size_t length,const char *installed,struct reloq_update *out)
{return reloq_update_parse_revision(json,length,installed,0,out);}
