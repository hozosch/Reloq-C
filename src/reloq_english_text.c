#include "reloq_english_voice.h"
#include "reloq_inline_tags.h"
#include "reloq_rules61.h"
#include "reloq_english_connected.h"
#include "reloq_character.h"
#include "reloq_unicode.h"
#include "reloq_utf8.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int reloq_english_g2p(const unsigned char *,size_t,char *,size_t);

struct text_buffer {unsigned char *s;size_t n,cap;};
static int append(struct text_buffer *b,const void *p,size_t n)
{
    if(n>SIZE_MAX-b->n-1)return -1;
    size_t need=b->n+n+1;
    if(need>b->cap) {
        size_t cap=b->cap?b->cap:256;
        while(cap<need){if(cap>SIZE_MAX/2){cap=need;break;}cap*=2;}
        void *next=realloc(b->s,cap);if(!next)return -1;b->s=next;b->cap=cap;
    }
    memcpy(b->s+b->n,p,n);b->n+=n;b->s[b->n]=0;return 0;
}
static int letter(unsigned char c)
{return (c>='A'&&c<='Z')||(c>='a'&&c<='z');}
static int wordchar(unsigned char c)
{return letter(c)||(c>='0'&&c<='9')||c=='\'';}
static int lookup(const unsigned char *word,size_t n,const char **s,const char **v,const char **c)
{
    /* Native time-word reading observed through the original English
     * symbolic rules. It is also a legitimate ordinary word. */
    if(n==7&&!memcmp(word,"o'clock",7)){*s="`[.0x.1klak]";*v=*c=NULL;return 1;}
    if(!reloq_english_word(word,n,s,v,c))return 1;
    if(n>=256)return 0;
    unsigned char key[256];
    for(size_t i=0;i<n;++i)key[i]=(word[i]>='A'&&word[i]<='Z')?word[i]+32:word[i];
    if(!reloq_english_word(key,n,s,v,c))return 1;
    if(n&&key[0]>='a'&&key[0]<='z')key[0]-=32;
    return !reloq_english_word(key,n,s,v,c);
}

static int phone_vowel(unsigned char c)
{return c&&strchr("iIeEAXHxauUocRYWO",c)!=NULL;}

/* Productive inflection is licensed by a known English stem. It never
 * changes an exact dictionary entry and never sends spelling to German.
 * Resyllabification precedes the intervocalic t/d flap; the added weak
 * syllable retains its own nucleus and does not acquire root stress. */
static int inflected(const char *root,const char *suffix,int syllabic,
    char *out,size_t capacity)
{
    size_t n=strlen(root),z=strlen(suffix);
    if(n<6||memcmp(root,"`[",2)||root[n-1]!=']'||strchr(root+2,'`')||
       n+z+5>=capacity)return 0;
    memcpy(out,root,n-1);size_t used=n-1;
    if(syllabic) {
        size_t split=used;
        if(!phone_vowel((unsigned char)out[used-1])&&
           strchr("bpdtFkgDTvfzsZSJCmnGlyw",out[used-1]))--split;
        if(split<used&&split>3&&strchr("ptk",out[split])&&out[split-1]=='s')--split;
        if(split<used&&out[split]=='l'&&split&&phone_vowel((unsigned char)out[split-1])&&
           (!strcmp(suffix,"R")||!strcmp(suffix,"Xst"))) {
            size_t k=split;while(k&&out[k]!='.')--k;
            if(k+1<used&&out[k+1]!='0')split=used;
        }
        size_t count=used-split;
        char onset[3]={0};if(count>2)return 0;
        memcpy(onset,out+split,count);used=split;
        if(count==1&&(onset[0]=='t'||onset[0]=='d')&&used&&
           (phone_vowel((unsigned char)out[used-1])||out[used-1]=='r'))onset[0]='F';
        memcpy(out+used,".0",2);used+=2;
        memcpy(out+used,onset,count);used+=count;
    }
    memcpy(out+used,suffix,z);used+=z;out[used++]=']';out[used]=0;return 1;
}
static int derive92(const unsigned char *word,size_t n,char *out,size_t capacity,unsigned depth)
{
    if(n<4||n>=256||depth>4)return 0;
    unsigned char key[256];
    for(size_t i=0;i<n;++i)key[i]=(word[i]>='A'&&word[i]<='Z')?word[i]+32:word[i];
    key[n]=0;
    const char *s=NULL,*v=NULL,*c=NULL;
    int kind=-1;size_t stem=0;
    if(n>6&&!memcmp(key+n-4,"ness",4)){kind=6;stem=n-4;}
    else if(n>5&&!memcmp(key+n-3,"ful",3)){kind=7;stem=n-3;}
    else if(n>6&&!memcmp(key+n-4,"less",4)){kind=8;stem=n-4;}
    else if(n>6&&!memcmp(key+n-4,"ment",4)){kind=9;stem=n-4;}
    else if(n>6&&!memcmp(key+n-4,"ship",4)){kind=10;stem=n-4;}
    else if(n>6&&!memcmp(key+n-4,"hood",4)){kind=11;stem=n-4;}
    else if(n>4&&!memcmp(key+n-2,"'s",2)){kind=0;stem=n-2;}
    else if(key[n-1]=='s'){kind=0;stem=n-1;}
    else if(n>4&&!memcmp(key+n-2,"ed",2)){kind=1;stem=n-2;}
    else if(n>5&&!memcmp(key+n-3,"ing",3)){kind=2;stem=n-3;}
    else if(n>4&&!memcmp(key+n-2,"er",2)){kind=3;stem=n-2;}
    else if(n>5&&!memcmp(key+n-3,"est",3)){kind=4;stem=n-3;}
    else if(n>4&&!memcmp(key+n-2,"ly",2)){kind=5;stem=n-2;}
    if(kind<0||stem<2)return 0;
    int found=lookup(key,stem,&s,&v,&c);
    /* Silent e before -d, -ing and comparative endings. */
    unsigned root_syllables=0;for(const char *p=found?s:NULL;p&&*p;++p)root_syllables+=*p=='.';
    int restored_e=0;
    int silent_e=(kind==1||kind==2||root_syllables==1) && stem>=3 &&
        ((strchr("aeiou",key[stem-2])&&!strchr("aeiou",key[stem-3])&&
          strchr("bcdfgklmnprstvz",key[stem-1])) ||
         !memcmp(key+stem-2,"st",2)||!memcmp(key+stem-2,"ng",2)||
         !memcmp(key+stem-2,"nc",2)||!memcmp(key+stem-2,"rg",2)||
         !memcmp(key+stem-2,"pl",2)||!memcmp(key+stem-2,"bl",2));
    if((!found||silent_e) && kind>=1&&kind<=4 && stem+1<sizeof key) {
        unsigned char saved=key[stem];key[stem]='e';
        const char *es=NULL,*ev=NULL,*ec=NULL;
        if(lookup(key,stem+1,&es,&ev,&ec)){s=es;v=ev;c=ec;found=1;restored_e=1;}
        key[stem]=saved;
    }
    if(!found && kind>=1&&kind<=4&&stem>2&&key[stem-1]==key[stem-2]&&
       strchr("bdgklmnprt",key[stem-1]))found=lookup(key,stem-1,&s,&v,&c);
    if(!found && kind==0&&n>4&&!memcmp(key+n-3,"ies",3)) {
        key[n-3]='y';found=lookup(key,n-2,&s,&v,&c);key[n-3]='i';
    }
    if(!found && kind>=1&&key[stem-1]=='i') {
        key[stem-1]='y';found=lookup(key,stem,&s,&v,&c);key[stem-1]='i';
    }
    if(!found && kind==0&&n>4&&!memcmp(key+n-2,"es",2)) {
        found=lookup(key,n-2,&s,&v,&c);
        if(found&&!strchr("szSZJC",s[strlen(s)-2]))found=0;
    }
    char derived_root[2048];
    if(!found&&kind>=5&&derive92(key,stem,derived_root,sizeof derived_root,depth+1)) {
        s=derived_root;found=1;
    }
    if(!found||!s)return 0;
    size_t z=strlen(s);if(z<6||s[z-1]!=']')return 0;
    unsigned char last=(unsigned char)s[z-2];
    if(kind==0)return inflected(s,strchr("szSZJC",last)?"Xz":
        strchr("ptkfT",last)?"s":"z",strchr("szSZJC",last)!=NULL,out,capacity);
    if(kind==1)return inflected(s,last=='t'||last=='d'?"Xd":
        strchr("pkfTsSC",last)?"t":"d",last=='t'||last=='d',out,capacity);
    if(kind==2)return inflected(s,"IG",1,out,capacity);
    /* A syllabic /l/ loses its schwa before a comparative nucleus. The
     * existing weak syllable stays in place; -al does not share -le. */
    char adjusted[2048];
    int reduce_l=0;
    if(z>=7&&!memcmp(s+z-3,"xl]",3)&&strchr("bpkfg",s[z-4])) {
        size_t last=z-4;while(last&&s[last]!='.')--last;
        size_t previous=last?last-1:0;while(previous&&s[previous]!='.')--previous;
        reduce_l=last&&(!previous||memchr(s+previous,'x',last-previous)==NULL);
    }
    if(reduce_l&&(kind==3||kind==4)&&stem>=2&&(!memcmp(key+stem-2,"le",2)||(restored_e&&key[stem-1]=='l'))&&
       z>=5&&!memcmp(s+z-3,"xl]",3)&&z<sizeof adjusted) {
        memcpy(adjusted,s,z-3);memcpy(adjusted+z-3,"l]",3);
        return inflected(adjusted,kind==3?"R":"Xst",0,out,capacity);
    }
    if(kind==3)return inflected(s,"R",1,out,capacity);
    if(kind==4)return inflected(s,"Xst",1,out,capacity);
    if(kind==6)return inflected(s,".0nXs",0,out,capacity);
    if(kind>=7) {
        const char *tail=kind==7?".0fxl":kind==8?".0lXs":kind==9?".0mXnt":
            kind==10?".2SIp":".2hUd";
        if(kind==7&&last=='f')return 0;
        if(kind==8&&strchr("lbpkfgs",last)) {
            if(last=='l')return inflected(s,"Xs",1,out,capacity);
            if(z>=3&&phone_vowel((unsigned char)s[z-3]))return inflected(s,"lXs",1,out,capacity);
            if(z>=4&&last=='p'&&s[z-3]=='l')return inflected(s,"lXs",1,out,capacity);
        }
        return inflected(s,tail,0,out,capacity);
    }
    /* -ly retains alveolar-stop codas and coalesces an adjacent /l/.
     * Weak final -y reduces before -ly, but not before -ness. */
    if(z>=5&&s[z-2]=='i'&&stem&&key[stem-1]=='i'&&z<sizeof adjusted) {
        memcpy(adjusted,s,z+1);adjusted[z-2]=strchr("pbmfv",s[z-3])?'x':'X';
        if(z>=7&&s[z-3]=='l'&&s[z-4]=='0'&&s[z-5]=='.') {
            adjusted[z-5]='l';adjusted[z-4]='.';adjusted[z-3]='0';
        }
        return inflected(adjusted,".0li",0,out,capacity);
    }
    if(last=='l')return inflected(s,"i",1,out,capacity);
    if(strchr("bpkfgs",last)&&z>=3&&phone_vowel((unsigned char)s[z-3]))
        return inflected(s,"li",1,out,capacity);
    return inflected(s,".0li",0,out,capacity);
}
static int resolve(const unsigned char *word,size_t n,const char **s,
    const char **v,const char **c,char *storage,size_t capacity)
{
    if(lookup(word,n,s,v,c))return 1;
    size_t native_n=0;
    if(reloq_rules61_word(0x10000,word,n,(unsigned char *)storage,capacity,&native_n)==1) {
        *s=storage;*v=*c=NULL;return 1;
    }
    if(!derive92(word,n,storage,capacity,0)&&!reloq_english_g2p(word,n,storage,capacity)) {
        /* normalize_eng_letters removes an internal apostrophe after
         * lexical/contraction analysis. Unknown words remain words. */
        unsigned char letters[256];size_t used=0;
        if(n>=sizeof letters)return 0;
        for(size_t k=0;k<n;++k)if(word[k]!='\'')letters[used++]=word[k];
        if(used==n||!reloq_english_g2p(letters,used,storage,capacity))return 0;
    }
    *s=storage;*v=*c=NULL;return 1;
}
static int number_part(struct text_buffer *b,unsigned value)
{
    char key[16];const char *s=NULL,*v=NULL,*c=NULL;
    int n=snprintf(key,sizeof key,"%u",value);
    if(n<1||!lookup((const unsigned char *)key,(size_t)n,&s,&v,&c))return -1;
    return append(b,s,strlen(s))||append(b," ",1);
}
static int number_name(struct text_buffer *b,const char *name)
{
    const char *s=NULL,*v=NULL,*c=NULL;
    if(!lookup((const unsigned char *)name,strlen(name),&s,&v,&c))return -1;
    return append(b,s,strlen(s))||append(b," ",1);
}
static int integer_words(struct text_buffer *b,const unsigned char *w,size_t n)
{
    if(!n||n>12||w[0]=='0')return 0;
    uint64_t value=0;
    for(size_t i=0;i<n;++i){if(w[i]<'0'||w[i]>'9')return 0;value=10*value+w[i]-'0';}
    static const struct {uint64_t scale;const char *name;} units[]={
        {UINT64_C(1000000000),"billion"},{1000000,"million"},{1000,"thousand"},{1,NULL}};
    for(size_t i=0;i<sizeof units/sizeof *units;++i) {
        unsigned part=(unsigned)(value/units[i].scale);value%=units[i].scale;
        if(!part)continue;
        if(part>=100) {
            if(number_part(b,part/100)||number_name(b,"hundred"))return -1;
            part%=100;
        }
        if(part&&number_part(b,part))return -1;
        if(units[i].name&&number_name(b,units[i].name))return -1;
    }
    return 1;
}
static int vowel_initial(const char *spr)
{
    struct reloq_english_phone *p=NULL;size_t n=0;
    if(!spr||reloq_english_parse((const unsigned char *)spr,strlen(spr),&p,&n,NULL))return 0;
    int yes=n&&(p[0].syllabic||strchr("iIeEAXHxauUocRYWO",p[0].phone));free(p);return yes;
}

#include "reloq_english_context86.h"

/* The scanner records one source offset per generated annotation byte, so
 * derived pronunciations, numbers and changing weak forms cannot move ECI
 * indices to unrelated words. One AddText transaction is analyzed together. */
static int analyze_words92(const unsigned char *text,size_t length,int process_tags,int context_enabled,
    unsigned char **annotation,size_t **source,size_t *written,size_t *spelled)
{
    if(!text||!annotation||!source||!written||memchr(text,0,length))return -1;
    struct text_buffer b={0};size_t *map=NULL,map_cap=0,guessed=0;
    int status=-1;
    for(size_t i=0;i<length;) {
        size_t begin=i,end=i+1,before=b.n;
        if(text[i]=='`'&&(process_tags||(i+1<length&&(text[i+1]=='['||(text[i+1]>='0'&&text[i+1]<='2'))))) {
            struct reloq_inline_tag tag=reloq_inline_tag_read(text+i,length-i,0);
            int valid=tag.kind==RELOQ_TAG_PROMINENCE||tag.kind==RELOQ_TAG_PITCH||
                tag.kind==RELOQ_TAG_SPEED||tag.kind==RELOQ_TAG_VOLUME;
            if(tag.kind==RELOQ_TAG_SPR) {
                struct reloq_english_phone *phones=NULL;size_t count=0;
                valid=!reloq_english_parse(text+i,tag.length,&phones,&count,NULL)&&count>0;
                free(phones);
            }
            if(valid) {
                end=i+tag.length;
                if(append(&b,text+i,end-i))goto memory;
            } else {
                /* The original character-name path keeps an unknown or
                 * malformed control in the utterance, including a legal
                 * backtick in an address. Never abort the complete text. */
                static const char backquote[]="`[.1bAk.2kwot] ";
                end=i+1;if(append(&b,backquote,sizeof backquote-1))goto memory;
            }
        } else if(wordchar(text[i])&&text[i]!='\'') {
            end=i+1;while(end<length&&wordchar(text[end]))++end;
            while(end>i&&text[end-1]=='\'')--end;
            /* Original normalization separates an unrecognized letter/digit
             * run (foo42) into a word and a number. Preserve exact identifier
             * entries before applying that general token boundary. */
            const char *whole=NULL,*wv=NULL,*wc=NULL;
            if(!lookup(text+i,end-i,&whole,&wv,&wc)) {
                size_t cut=i+1;int alpha=letter(text[i]);
                while(cut<end&&(alpha?(letter(text[cut])||text[cut]=='\''):
                    (text[cut]>='0'&&text[cut]<='9')))++cut;
                if(cut<end)end=cut;
                while(end>i&&text[end-1]=='\'')--end;
            }
            const char *s=NULL,*v=NULL,*c=NULL;
            char derived[2048];
            int numeric=0;
            if(text[i]>='1'&&text[i]<='9'&&!lookup(text+i,end-i,&s,&v,&c)) {
                numeric=integer_words(&b,text+i,end-i);
                if(numeric<0)goto memory;
            }
            if(numeric) { /* All expansion bytes belong to this input token. */ }
            else if(resolve(text+i,end-i,&s,&v,&c,derived,sizeof derived)) {
                const char *to_variant=NULL;
                size_t next=end;while(next<length&&(text[next]==' '||text[next]=='\t'))++next;
                if(next<length&&wordchar(text[next])&&text[next]!='\'') {
                    size_t ne=next+1;while(ne<length&&wordchar(text[ne]))++ne;
                    const char *ns=NULL,*nv=NULL,*nc=NULL;
                    char next_derived[2048];
                    int resolved=resolve(text+next,ne-next,&ns,&nv,&nc,next_derived,sizeof next_derived);
                    int front=resolved&&vowel_initial(ns);
                    if(front&&v)s=v;else if(!front&&c)s=c;
                    if(context_enabled && resolved && end-i==2 &&
                       tolower(text[i])=='t' && tolower(text[i+1])=='o') {
                        const char *next_context=context_reading86(text,length,next,ne);
                        if(next_context)ns=next_context;
                        to_variant=context_to89(b.s,b.n,ns,text+next,ne-next);
                    }
                }
                const char *context=context_enabled?context_reading86(text,length,i,end):NULL;
                if(context)s=context;
                if(to_variant && (!context || strncmp(context,"`0 ",3)))s=to_variant;
                if(append(&b,s,strlen(s)))goto memory;
            } else {
                ++guessed;
                /* Non-word identifiers and digits retain explicit letter names.
                 * Alphabetic unknown words use the native context rules. */
                for(size_t j=i;j<end;++j) {
                    if(text[j]=='\'')continue;
                    if(!lookup(text+j,1,&s,&v,&c))goto done;
                    if(append(&b,s,strlen(s))||append(&b," ",1))goto memory;
                }
            }
        } else if(strchr(".,!?;:\n()\034\035\036",text[i])) {
            if(append(&b,text+i,1))goto memory;
        } else if(text[i]>=128) {
            /* Native ECI admission handles Latin and Unicode character mode;
             * an unsupported direct UTF-8 scalar is skipped as a unit. */
            while(end<length&&(text[end]&0xc0)==0x80)++end;
            if(append(&b," ",1))goto memory;
        } else if(text[i]=='@'||text[i]=='+'||text[i]=='%'||text[i]=='&'||text[i]=='='||text[i]=='#') {
            const char *name=text[i]=='@'?"at":text[i]=='+'?"plus":text[i]=='%'?"percent":text[i]=='&'?"and":text[i]=='='?"equals":"number";
            const char *s=NULL,*v=NULL,*c=NULL;
            if(!lookup((const unsigned char *)name,strlen(name),&s,&v,&c))goto done;
            if(append(&b,s,strlen(s)))goto memory;
        } else if(append(&b," ",1))goto memory;
        if(b.n+1>map_cap) {
            size_t cap=b.cap;
            if(cap>SIZE_MAX/sizeof *map)goto memory;
            void *next=realloc(map,cap*sizeof *map);if(!next)goto memory;map=next;map_cap=cap;
        }
        for(size_t j=before;j<b.n;++j)map[j]=begin;
        /* A closing word owns its source end; an index inside that input
         * token is resolved after its complete acoustic word. */
        if(b.n>before)map[b.n-1]=end;
        i=end;
    }
    if(!b.s) {if(append(&b,"",0))goto memory;map=malloc(sizeof *map);if(!map)goto memory;}
    map[b.n]=length;
    if(context_enabled)reloq_english_connected_rules(b.s,&b.n,map);
    map[b.n]=length;*annotation=b.s;*source=map;*written=b.n;if(spelled)*spelled=guessed;return 0;
memory:status=-2;
done:free(map);free(b.s);return status;
}
#include "reloq_english_normalize92.h"

int reloq_english_analyze(const unsigned char *text,size_t length,int process_tags,
    unsigned char **annotation,size_t **source,size_t *written,size_t *spelled)
{
    if(!text||!annotation||!source||!written||memchr(text,0,length))return -1;
    struct normalized92 n={0};
    int r=normalize92(text,length,process_tags,&n);
    if(!r)r=analyze_words92(n.text.s,n.text.n,process_tags,memchr(text,'`',length)==NULL,
        annotation,source,written,spelled);
    if(!r)for(size_t k=0;k<=*written;++k)(*source)[k]=n.map[(*source)[k]];
    free(n.text.s);free(n.map);return r;
}
int reloq_english_text_annotation(const unsigned char *text,size_t length,
    unsigned char **annotation,size_t *written,size_t *spelled)
{
    size_t *map=NULL;int r=reloq_english_analyze(text,length,0,annotation,&map,written,spelled);free(map);return r;
}
