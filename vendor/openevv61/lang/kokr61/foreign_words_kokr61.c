/* Foreign words keep their native word and phrase identity. English lexical
 * analysis supplies phones; the Korean loanword rules adapt the sequence
 * before the existing Korean syllabification and acoustic rules run. */
#include <stdint.h>
#include <string.h>
#include "reloq61_english_plan.h"
#include "delta.h"
#include "delta_rules_c.h"
#include "reloq61_scalar_state.h"
extern int reloq61_english_word(void *,const char *,char *,size_t);

static const char input_ascii[]="\0aeiouAEIOUbcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ0123456789-,.?:;!() '#_`~*@[]{}`$+%/=^&|\\<>\"";
static int32_t position(int32_t argument)
{int32_t p;memcpy(&p,EVV_AT(const unsigned char *,argument)+4,4);return p;}
static int32_t right(delta_state *d,int32_t p,unsigned projection)
{return EVV_REF(vmover(d,EVV_AT(int32_t *,p),(uint8_t)projection));}
static int32_t record(delta_state *d,int32_t p,unsigned projection)
{
    int32_t n=vgetsc(d,0,0,right(d,p,projection),(uint8_t)projection);
    return n&&!(*EVV_AT(int32_t *,n)&2)?n:0;
}
static unsigned char *body(int32_t p){return TFLDS(EVV_AT(void *,p));}
static int32_t after(int32_t p){return EVV_AT(const int32_t *,p)[1]&~3;}
static int pending_charset(delta_state *d)
{
    unsigned char *word=(unsigned char *)d+DG_BASE+2288+4;
    return *(const unsigned char *)delta_record_field(&vstmtbl[4],12,word);
}
#include "foreign_units_kokr61.inc"
/* Whole-key lexical expansion inside the native input graph. The caller's
 * annotation and explicit spelling decisions remain authoritative. */
#include "foreign_expansions_kokr61.inc"
static int32_t lexical_rewrite(delta_state*d,int32_t begin)
{
 if(pending_charset(d)!=5)return 0;
 char key[32];int32_t at=position(begin),last=at;size_t n=0;
 for(;n+1<sizeof key;++n){
  int32_t node=record(d,last,1);if(!node)break;
  unsigned id=*(const unsigned char*)delta_record_field(&vstmtbl[1],0,body(node));
  const char*const*names=vstmtbl[1].fields[0].values;
  if(id>=(unsigned)vstmtbl[1].fields[0].nvalues)return 0;
  const char*name=names[id];char c=name&&name[0]&&!name[1]?name[0]:0;
  if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='/'||c=='-'||c=='\''))break;
  key[n]=c;last=after(node);
 }
 if(!n||n+1==sizeof key)return 0;key[n]=0;
 size_t lo=0,hi=sizeof foreign_expansions/sizeof*foreign_expansions;
 while(lo<hi){
  size_t mid=lo+(hi-lo)/2;int cmp=strcmp(key,foreign_expansions[mid].key);
  if(cmp<0)hi=mid;else if(cmp>0)lo=mid+1;else{
   const struct foreign_expansion*entry=foreign_expansions+mid;
   uint8_t input[128];size_t count=strlen(entry->text);if(!count||count>sizeof input)return 94;
   const char*const*names=vstmtbl[1].fields[0].values;
   for(size_t i=0;i<count;++i){
    unsigned id=1;for(;id<(unsigned)vstmtbl[1].fields[0].nvalues;++id)if(names[id]&&names[id][0]==entry->text[i]&&!names[id][1])break;
    if(id==(unsigned)vstmtbl[1].fields[0].nvalues)return 94;input[i]=(uint8_t)id;
   }
   const delta_token a={0,at},b={0,last};lpta_rpta_loadp(d,&a,&b);
   if(insert_2pt_s(d,1,(uint8_t)count,input,0))return 94;
   if(entry->charset!=5){
    /* The spare pending-word byte scopes the original alphabet to this
     * expansion. It is cleared as soon as its word has been built. */
    unsigned char*p=(unsigned char*)d+DG_BASE+2288+4;p[19]=p[18];p[18]=entry->charset;
   }
   return 0;
  }
 }
 return 0;
}

/* This rule runs after explicit spelling modes and annotations, so their
 * native behavior wins. H3 romanized Hangul never takes the ASCII branch. */
static int32_t foreign_sequence(void *state,int32_t begin,int32_t end,int leading_digits)
{
    delta_state *d=state;
    if(pending_charset(d)!=5)return -1;
    int32_t at=position(begin),last=at;unsigned count=0,letters=0,digits=0;char spelling[256];
    while(count<255) {
        int32_t node=record(d,last,1);if(!node)break;
        unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,body(node));
        char c=name<sizeof input_ascii?input_ascii[name]:0;
        int letter=(c>='a'&&c<='z')||(c>='A'&&c<='Z');int decimal=0;if(c=='.'&&count&&spelling[count-1]>='0'&&spelling[count-1]<='9'){int32_t next=record(d,after(node),1);if(next){unsigned v=body(next)[4];char nc=v<sizeof input_ascii?input_ascii[v]:0;decimal=nc>='0'&&nc<='9';}}
        if(!letter&&!(c>='0'&&c<='9'&&(count||leading_digits))&&!(count&&(c=='\''||c=='-'))&&!decimal)break;
        spelling[count]=c;letters+=letter;digits+=c>='0'&&c<='9';last=after(node);++count;
    }
    if(!count||last==at||!letters)return -1;
    spelling[count]=0;
    if(leading_digits){size_t number=0;while(spelling[number]>='0'&&spelling[number]<='9')++number;
      size_t lo=0,hi=sizeof foreign_number_units/sizeof *foreign_number_units;while(lo<hi){size_t mid=lo+(hi-lo)/2;int cmp=strcmp(spelling+number,foreign_number_units[mid]);if(!cmp)return -1;if(cmp>0)lo=mid+1;else hi=mid;}
    }
    /* One normal word, followed by the existing word builder. */
    memcpy(EVV_AT(unsigned char *,end)+4,&last,4);
    memcpy((unsigned char *)d+DG_BASE+1386,(unsigned char *)d+DG_BASE+1406,2);
    return 0;
}

int32_t reloq_kokr61_foreign_letters(void*d,int32_t a,int32_t b)
{int32_t status=lexical_rewrite(d,a);return status?status:foreign_sequence(d,a,b,0);}
int32_t reloq_kokr61_foreign_number(void*d,int32_t a,int32_t b){return foreign_sequence(d,a,b,1);}

enum { SOURCE_WITH_FINAL=1u<<22 };
enum phone {K_ph=1,K_pp,K_p,K_b,K_th,K_tt,K_t,K_d,K_kh,K_kk,K_k,K_g,
 K_ch,K_cc,K_C,K_J,K_ss,K_s,K_SS,K_S,K_Z,K_m,K_n,K_ng,K_l,K_r,K_y,K_W,K_w,K_h,
 K_i,K_e,K_A,K_U,K_E,K_a,K_I,K_O,K_u,K_o,K_boundary};
struct foreign_detail {unsigned char origin,length;int16_t stress;};
struct english_phone {char name,base;unsigned char stress,letter;unsigned short group;uint32_t source_class;};
static int vowel(char c){return strchr("iIeEAxXHRuUoacB",c)!=NULL;}
static size_t english_phones(const char *text,struct english_phone *phones,size_t cap)
{
    size_t n=0;int inside=0;unsigned stress=0,group=0;
    for(size_t i=0;text[i];++i) {
        char c=text[i];if(c=='['){inside=1;++group;continue;}if(c==']'){inside=0;continue;}
        if(!inside)continue;
        if(c=='.'&&text[i+1]>='0'&&text[i+1]<='2'){stress=(unsigned)(text[++i]-'0');continue;}
        const char *expansion=NULL;char literal[2]={c,0};
        switch(c) {
        /* Reduced high X remains distinct from schwa x. */
        case 'F':expansion="t";break;
        case 'R':expansion=stress?"R":"xr";break;
        case 'Y':expansion="ay";break;case 'W':expansion="aw";break;
        case 'O':expansion="cy";break;case '?':continue;
        default:expansion=literal;break;
        }
        for(const char *p=expansion;*p;++p){if(n>=cap)return 0;phones[n++]=(struct english_phone){*p,*p,(unsigned char)stress,0,(unsigned short)group};}
    }
    return n;
}
/* Use the lexical graph before English display splitting and glottalization. */
static size_t lexical_phones(const struct reloq61_english_segment *in,size_t n,const uint32_t*classes,struct english_phone*out,size_t cap)
{
 size_t used=0;
 for(size_t i=0;i<n;++i){
  const char *b=in[i].base,*p=in[i].name;char base=b[0],name=p[0];
  if(!strcmp(b,"GAP")||!strcmp(b,"#"))continue;
  if(!strcmp(b,"a@"))base='B';else if(!strcmp(b,"c@"))base='c';else if(!strcmp(b,"Xx"))base='X';else if(b[1])return 0;
  if(!strcmp(p,"a@"))name='B';else if(!strcmp(p,"c@"))name='c';else if(!strcmp(p,"Xx"))name='X';else if(p[1])return 0;
  /* Flaps and glottal stops retain the lexical stop for Korean adaptation. */
  if((name=='F'||name=='?')&&(base=='t'||base=='d'))name=base;
  const char *exp=NULL;char literal[2]={name,0};
  switch(name){case 'Y':exp="ay";break;case 'W':exp="aw";break;case 'O':exp="cy";break;default:exp=literal;}
  if(!strcmp(p,"c@"))exp="cr";
  for(const char *q=exp;*q;++q){if(used==cap)return 0;out[used++]=(struct english_phone){*q,base,(unsigned char)in[i].stress,(unsigned char)((in[i].origin>>2)&31),(unsigned short)in[i].word,classes[i]};}
 }
 return used;
}
static const unsigned char letter_names[26][9];
static const unsigned char following_v[]={2,K_p,K_i};
static int add(unsigned char *out,size_t cap,size_t *n,unsigned p)
{if(*n>=cap)return 0;out[(*n)++]=(unsigned char)p;return 1;}
/* The adaptation is in terms of phones and neighboring nuclei, rather than
 * word spellings or a pronunciation cache. */
#include "foreign_vowels_kokr61.inc"
static size_t loanword_one(const struct english_phone *in,size_t n,unsigned char *out,struct foreign_detail *details,size_t cap)
{
    size_t used=0;
    for(size_t i=0;i<n;++i) {
        char c=in[i].name,prev=i?in[i-1].name:0,next=i+1<n?in[i+1].name:0;
        int pv=prev&&vowel(prev),nv=next&&vowel(next);unsigned p=0,tail=0,flags=in[i].source_class;
        if(vowel(c)) {
            switch(c) {
            case 'i':case 'I':p=K_i;break;case 'B':p=in[i].stress?K_a:prev=='r'||(prev=='k'&&next=='m')?K_o:K_E;break;case 'X':p=(in[i].base=='i'||(in[i].base=='u'&&prev!='y')||(in[i].base=='B'&&prev=='r'&&(flags&RELOQ_SOURCE_HEAD_O)))?K_i:in[i].base=='A'||in[i].base=='B'?K_E:prev=='r'&&next=='d'&&in[i].base!='I'&&i>1&&!vowel(in[i-2].name)?K_e:prev=='v'&&next=='n'?K_U:in[i].base=='u'?K_u:in[i].base=='E'?(next=='n'?(prev=='v'?K_U:K_E):next=='t'&&i+2==n?K_e:K_i):in[i].base=='I'?K_i:next=='l'?(i?K_U:K_i):next=='n'?K_E:K_i;break;case 'e':p=!in[i].stress?K_E:K_e;tail=in[i].stress&&next!='r'?K_i:in[i].base=='A'&&next=='r'&&i+2<n&&in[i+2].name!='X'?K_E:0;break;
            case 'E':p=K_e;break;case 'A':p=in[i].stress?K_A:K_E;break;case 'H':case 'R':p=K_E;break;
            case 'a':p=K_a;break;case 'o':p=K_o;break;case 'c':p=prev=='w'&&(i<2||!strchr("kg",in[i-2].name))?K_E:K_o;break;case 'u':p=K_u;break;case 'U':p=prev=='w'?K_u:K_U;break;
            case 'x':p=in[i].base=='o'&&(!i||next=='r')?K_o:in[i].base=='A'?(prev=='r'&&i+1==n?K_a:K_E):in[i].base=='B'?prev=='r'||(prev=='k'&&next=='m')?K_o:K_E:next=='l'?K_U:prev=='k'&&next=='m'?K_o:K_E;break;
            }
            /* Spelling classes belong to the vowel rule itself. The input
             * word is never replaced and neighboring phone groups stay intact. */
            unsigned origin=c=='I'?16:c=='E'?15:c=='H'?17:c=='X'?18:c=='x'?19:c=='c'?13:0;
            if(c=='i'&&(flags&FV_LONG_I_PAIR)&&next=='l')tail=K_E;
            if(c=='X'&&(flags&FV_REDUCED_HIGH)){p=K_E;origin=18;}
            if(c=='x'&&(flags&FV_SCHWA_OPEN)){p=K_a;origin=19;}
            if(c=='x'&&(flags&FV_SCHWA_ROUND)&&i==1){p=K_o;origin=19;}
            if(c=='x'&&(flags&FV_SCHWA_HIGH)&&i==1){p=K_i;origin=19;}
            if(c=='H'&&(flags&FV_UH_OPEN)){p=K_a;origin=17;}
            if(c=='H'&&(flags&FV_UH_ROUND)){p=K_o;origin=17;}
            if(c=='A'&&(flags&FV_AE_OPEN)){p=K_a;origin=14;}
            if((c=='a'||c=='B')&&(flags&FV_AA_ROUND)){p=K_o;origin=12;}
            if((c=='a'||c=='B')&&(flags&FV_AA_CENTRAL)){p=K_E;origin=12;}
            if(c=='c'&&(flags&FV_AW_CENTRAL)){p=K_E;origin=13;}
            if(c=='x'&&(flags&RELOQ_SOURCE_AFTER_A)){p=K_a;origin=19;}
            if(c=='x'&&next=='l'&&(flags&RELOQ_SOURCE_ROOT_AL)){p=K_E;origin=19;}
            if(c=='x'&&next=='l'){
                size_t after_liquid=i+2;while(after_liquid<n&&in[after_liquid].name=='l')++after_liquid;
                if(after_liquid<n&&(in[after_liquid].name=='i'||in[after_liquid].name=='y')){p=K_E;origin=19;}
                else if(after_liquid==n&&prev=='f'&&used>=3&&out[used-2]==K_U&&(out[used-3]==K_ph||out[used-3]==K_th||out[used-3]==K_kh)){p=K_u;origin=19;}
            }
            if(origin)details[used].origin=(unsigned char)origin;
            size_t begin=used;
            if(!add(out,cap,&used,p)||(tail&&!add(out,cap,&used,tail)))return 0;
            if((c=='i'&&!tail)||c=='o'||c=='u'||c=='R'||(c=='e'&&!tail))details[begin].length=2;
            if(c=='e'&&tail==K_i)details[begin+1].origin=22;
            continue;
        }
        switch(c) {
        case 'b':p=i?K_b:K_p;break;case 'd':p=i?K_d:K_t;break;case 'g':p=i?K_g:K_k;break;
        case 'p':p=K_ph;break;case 't':p=next=='S'?K_t:K_th;break;case 'k':p=K_kh;break;
        case 'f':p=K_ph;break;case 'v':p=i?K_b:K_p;break;case 'T':p=K_ss;break;case 'D':p=K_d;break;
        case 's':p=next=='i'||next=='I'?K_SS:K_s;break;case 'S':p=K_S;break;
        case 'Z':p=in[i].base=='Z'?(i?K_J:K_C):K_Z;break;case 'z':p=i?K_J:K_C;if(nv&&used&&(out[used-1]==K_p||out[used-1]==K_t||out[used-1]==K_k))p=K_C;break;
        case 'C':p=K_ch;break;case 'J':p=K_J;break;
        case 'm':p=K_m;break;case 'n':p=K_n;break;case 'G':p=K_ng;break;
        case 'h':p=K_h;break;case 'y':p=prev=='a'||prev=='c'?K_i:nv?K_y:K_i;break;
        case 'w':p=prev=='a'?K_u:nv?K_w:K_u;break;
        case 'r':
            if(!nv){if(prev=='e'||(prev=='c'&&i>1&&in[i-2].name=='w'&&(i<3||!strchr("kg",in[i-3].name))))p=K_E;else continue;break;}
            if(prev&&!pv&&prev!='y'&&prev!='w'&&prev!='m'&&prev!='n'&&
               !(used&&out[used-1]>=K_i&&out[used-1]<=K_o)&&
               !add(out,cap,&used,K_U))return 0;
            p=K_r;break;
        case 'l':
            if(prev=='l'){p=K_l;break;}
            if(nv) {
                if(prev&&!pv&&prev!='n'&&prev!='m'&&!(used&&out[used-1]>=K_i&&out[used-1]<=K_o)&&
                   !add(out,cap,&used,K_U))return 0;
                if(i&&prev!='n'&&prev!='m'&&!add(out,cap,&used,K_l))return 0;
            }
            p=!i&&nv?K_r:K_l;break;
        default:return 0;
        }
        if(next=='w'&&strchr("ptdbfvTDszSZCJ",c))tail=K_U;
        if(!i&&strchr("mn",c)&&next&&!nv&&!strchr("ywrl",next))tail=K_U;
        int coda=!nv&&next!='y'&&next!='w';
        if(coda&&strchr("ptk",c)) {
            int closed=pv&&(prev=='a'||prev=='U'||(c=='k'&&(prev=='I'||prev=='X'||prev=='A'||prev=='B'||prev=='E'))||(c=='p'&&prev!='e'&&prev!='o')||(c=='t'&&(prev=='c'||prev=='H'))||(c=='t'&&i&&in[i-1].stress==0&&(in[i-1].base=='I'||in[i-1].base=='E'||prev=='X'))||next=='s'||next=='t'||next=='k');
            if(c=='p'&&prev=='R')closed=0;
            if(c=='t'&&prev=='I'&&i+1==n){for(size_t j=0;j+1<i;++j)if(vowel(in[j].name)){closed=1;break;}}
            if(c=='p'&&!pv&&next!='l'&&next!='r'&&used&&out[used-1]==K_U)closed=1;
            if(c=='t'&&pv&&i+1==n&&strchr("IEAXHxaUcB",prev)&&!(flags&RELOQ_SOURCE_HEAD_TT)){
                size_t first=0;while(first<n&&!vowel(in[first].name))++first;
                if(i==1||first>=2||(first==1&&strchr("JCy",in[0].name)))closed=1;
            }
            if(closed)p=c=='p'?K_p:c=='t'?K_t:K_k;else tail=K_U;
        } else if(coda&&c=='b'&&pv&&next!='l'&&next!='r'&&next!='m'&&next!='n'&&strchr("IEAXHxaUcB",prev))p=K_p;
        else if(coda&&c=='g'&&pv&&(i+1==n||next=='k')&&strchr("IX",prev))p=K_k;
        else if(coda&&c=='d'&&prev=='U'&&!(i>1&&in[i-2].name=='w'))p=K_t;
        else if(coda&&strchr("bdfgvTszSZCJ",c))tail=(c=='S'||c=='C'||c=='J'||(c=='Z'&&in[i].base=='Z'))?(next=='r'?(c=='S'?(i==0||prev=='x'?K_U:K_i):K_E):K_i):K_U;
        if(c=='T'&&(flags&SOURCE_WITH_FINAL)){p=K_t;tail=0;}
        if(strchr("bdg",c)&&prev&&!pv&&used&&!(out[used-1]>=K_i&&out[used-1]<=K_o)&&!strchr("mnlrwyG",prev))p=c=='b'?K_p:c=='d'?K_t:K_k;
        size_t begin=used;
        if(!add(out,cap,&used,p)||(tail&&!add(out,cap,&used,tail)))return 0;
        struct foreign_detail *detail=details+begin;
        detail->origin=c=='f'?6:c=='v'?7:c=='T'?8:c=='D'?9:
            c=='b'&&p==K_p&&nv?4:c=='d'&&p==K_t&&nv?2:c=='g'&&p==K_k&&nv?5:
            c=='l'&&p==K_r?20:((c=='y'&&p==K_i)||(c=='w'&&p==K_u))?22:0;
        if((c=='p'&&p==K_p)||(c=='t'&&p==K_t)||(c=='k'&&p==K_k)||
           (c=='b'&&p==K_b)||(c=='d'&&p==K_d)||(c=='g'&&p==K_g)||
           c=='m'||c=='n'||(c=='r'&&p==K_r)||(c=='w'&&p==K_w)||(c=='y'&&p==K_y)||
           (c=='l'&&p==K_l&&!nv))detail->stress=in[i].stress;
        if(tail)details[begin+1].origin=3;
    }
    return used;
}
static size_t loanword(const struct english_phone *in,size_t n,unsigned char*out,struct foreign_detail*details,size_t cap,int preceding)
{
 size_t used=0;
 for(size_t first=0;first<n;){
  size_t last=first+1;while(last<n&&in[last].group==in[first].group)++last;
  size_t count;unsigned letter=in[first].letter;
  if(letter&&letter<=26){const unsigned char *p=letter==22&&(used||preceding)?following_v:letter_names[letter-1];count=p[0];if(count>cap-used)return 0;memcpy(out+used,p+1,count);}
  else count=loanword_one(in+first,last-first,out+used,details+used,cap-used);
  if(!count)return 0;
  if(used){unsigned before=out[used-1];if((before>=K_i&&before<=K_o)||(before>=K_m&&before<=K_w)){unsigned p=out[used];if(p==K_p)out[used]=K_b;else if(p==K_t)out[used]=K_d;else if(p==K_k)out[used]=K_g;else if(p==K_C)out[used]=K_J|128;}}
  used+=count;first=last;
 }
 return used;
}
struct foreign_edit {
    delta_rule_block scope;delta_loc left,right;
    int32_t result;
};
/* English distinguishes dictionary words from initialisms. Letter names
 * then use the Korean alphabet inventory, including their support vowels
 * and liquid codas. Root ranges stay inside the original lexical word. */
static const unsigned char letter_names[26][9]={
{2,K_e,K_i},
{2,K_p,K_i},
{2,K_SS,K_i},
{2,K_t,K_i},
{1,K_i},
{3,K_e,K_ph,K_U},
{2,K_J,K_i},
{4,K_e,K_i,K_ch,K_i},
{2,K_a,K_i},
{3,K_J,K_e,K_i},
{3,K_kh,K_e,K_i},
{2,K_e,K_l},
{2,K_e,K_m},
{2,K_e,K_n},
{1,K_o},
{2,K_ph,K_i},
{3,K_kh,K_y,K_u},
{2,K_a,K_l},
{3,K_e,K_s,K_U},
{2,K_th,K_i},
{2,K_y,K_u},
{3,K_p,K_U,K_i},
{8,K_t,K_E,K_b,K_U,K_l,K_l,K_y,K_u},
{4,K_e,K_k,K_s,K_U},
{3,K_w,K_a,K_i},
{2,K_C,K_i}
};
static size_t initialism_phones(char *text,const char *spelling,size_t letters,unsigned char *out,size_t capacity,uint16_t *ends,int preceding)
{
 size_t groups=0,used=0;for(char *p=text;(p=strchr(p,'['))!=NULL;++p)++groups;
 if(groups!=letters)return 0;
 for(size_t i=0;i<letters;++i) {
     const unsigned char *name=spelling[i]=='V'&&(i||preceding)?following_v:letter_names[spelling[i]-'A'];
     if(used+name[0]>capacity)return 0;
     memcpy(out+used,name+1,name[0]);used+=name[0];ends[i+1]=(uint16_t)used;
 }
 return groups;
}
#include "foreign_words_lexicon_kokr61.inc"
/* Dictionary phone plans retain their specified onset voicing. */
static int korean_dictionary_range(delta_state *d,int32_t left,int32_t last)
{
 char key[10];size_t length=0;int32_t at=left;
 while(at!=last) {
  if(length+1>=sizeof key)return 0;
  int32_t node=record(d,at,1);if(!node)return 0;
  unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,body(node));
  char c=name<sizeof input_ascii?input_ascii[name]:0;
  if(!c)return 0;key[length++]=c;at=after(node);
 }
 key[length]=0;return length&&korean_source_word(key,NULL,0)!=0;
}
static int preceding_sonorant(delta_state*d,int32_t left)
{
 int32_t at=EVV_REF(vmovel(d,EVV_AT(delta_node*,left),2)),n=vgetsc(d,1,0,at,2);
 if(!n||(*EVV_AT(int32_t*,n)&2))return 0;unsigned p=body(n)[7];
 return (p>=K_i&&p<=K_o)||(p>=K_m&&p<=K_w);
}
int32_t reloq_kokr61_assign_foreign(void *state,int32_t begin,int32_t end)
{
    delta_state *d=state;int32_t left=position(begin),right_end=position(end);
    int32_t word=record(d,left,4);if(!word)return 0;
    const unsigned char *w=body(word);
    if(w[18]!=5)return 0;
    int32_t assigned=record(d,left,3);
    if(assigned&&body(assigned)[3]==3)return 0;
    char spelling[256];int32_t boundaries[256];size_t length=0;int32_t at=left;
    int capitals=1;boundaries[0]=left;
    while(at!=right_end&&length+1<sizeof spelling) {
        int32_t node=record(d,at,1);if(!node)break;
        unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,body(node));
        char c=name<sizeof input_ascii?input_ascii[name]:0;
        if(!c)return 0;spelling[length++]=c;at=after(node);boundaries[length]=at;
        if(c<'A'||c>'Z')capitals=0;
    }
    if(at!=right_end||!length)return 0;spelling[length]=0;
    char text[16384];struct english_phone source[768];unsigned char phones[2304];
    struct foreign_detail details[2304]={0};
    uint32_t source_classes[768]={0};
    struct reloq61_english_segment segments[768];size_t segment_count=0;
    struct reloq61_english_spelling letter_info={0};
    uint16_t ends[256]={0};size_t groups=0;int preceding=preceding_sonorant(d,left);
    size_t count=korean_source_word(spelling,phones,sizeof phones);
    if(!count) {
      if(reloq61_english_word_context(d,spelling,text,sizeof text,segments,768,&segment_count,&letter_info,source_classes,foreign_vowel_class,!capitals))return 0;
      groups=capitals?initialism_phones(text,spelling,length,phones,sizeof phones,ends,preceding):0;
      count=groups?ends[groups]:lexical_phones(segments,segment_count,source_classes,source,768);
      if(!groups){if(length==4&&(spelling[0]=='w'||spelling[0]=='W')&&(spelling[1]=='i'||spelling[1]=='I')&&(spelling[2]=='t'||spelling[2]=='T')&&(spelling[3]=='h'||spelling[3]=='H'))for(size_t i=0;i<count;++i)source[i].source_class|=SOURCE_WITH_FINAL;if(!count){count=english_phones(text,source,768);size_t letter_count=letter_info.count;unsigned groups_found=count?source[count-1].group:0;if(letter_count==groups_found)for(size_t i=0;i<count;++i)source[i].letter=(unsigned char)letter_info.letters[source[i].group-1];}count=loanword(source,count,phones,details,sizeof phones,preceding);}
    }
    if(!count||(!groups&&count>255))return 0;
    struct foreign_edit *f=evv_frame_push(sizeof *f);if(!f)return 94;
    memset(f,0,sizeof *f);f->result=94;volatile int entered=0;
    if(EVV_LAND_SAVE((intptr_t)f->scope.landing))goto leave;
    if(ventproc(d,&f->scope.rec,f->scope.fence[0],f->scope.fence[1],f->scope.fence[2],f->scope.landing))goto leave;
    entered=1;fence(d,0,NULL);
    if(groups) {
        /* Letter-name adaptation has its own root/phone boundaries. The
         * existing lexical word and its phrase links remain authoritative. */
        const delta_token a={0,left},b={0,right_end};lpta_rpta_loadp(d,&a,&b);
        vdel_2pt(d,3,left,right_end);
    }
    for(size_t group=0;group<(groups?groups:1);++group) {
     int32_t from=groups?boundaries[group]:left,to=groups?boundaries[group+1]:right_end;
     size_t offset=groups?ends[group]:0,amount=groups?ends[group+1]-offset:count;
     const delta_token a={0,from},b={0,to};
     /* Replacing dictionary morphs can remove their internal sync points.
      * Finish that replacement before building the new phone chain, so its
      * later duration and syllable ranges retain live endpoints. */
     static const unsigned char root=2;lpta_rpta_loadp(d,&a,&b);
     if(insert_2pt_s(d,3,1,&root,0)||mark_s(d,3,3,3,0))goto leave;
     unsigned char names[255];for(size_t i=0;i<amount;++i)names[i]=phones[offset+i]&127;
     lpta_rpta_loadp(d,&a,&b);if(insert_2pt_s(d,2,(uint8_t)amount,names,0))goto leave;
     int32_t cursor=from;
     for(size_t i=0;i<amount;++i) {
         int32_t node=record(d,cursor,2);if(!node)goto leave;
         if(phones[offset+i]==K_ch)body(node)[6]=!(groups&&spelling[group]=='Z');
         body(node)[11]=details[offset+i].origin;
         body(node)[13]=details[offset+i].length;
         memcpy(body(node)+8,&details[offset+i].stress,2);
         if(phones[offset+i]&128)body(node)[11]=26;
         if(groups&&spelling[group]=='Z'&&i==0)body(node)[6]=3;
         cursor=after(node);
     }}
    f->result=0;
leave:
    if(entered&&vretproc(d,f->result?94:1))f->result=94;
    int32_t result=f->result;evv_land_forget((uintptr_t)f,(uintptr_t)f+sizeof *f);evv_frame_pop(f);return result;
}

/* The original spelling omits a rounded glide before its matching vowel;
 * that compact notation must not delete the acoustic glide itself. */
int reloq_kokr61_foreign_spr(void*state,int32_t a,int32_t b)
{
 (void)b;delta_state*d=state;int32_t at=position(a),p=record(d,at,2);if(!p||body(p)[7]!=K_w)return 0;
 int32_t q=record(d,after(p),2);if(!q||body(q)[7]!=K_u)return 0;
 int32_t context=vgetsc(d,1,1,at,4),word=record(d,context,4);
 if(!word||body(word)[18]!=5)return 0;reloq_scalar_finish(d,0);return 1;
}

/* Morphological analysis may replace roots. Assign a foreign word after
 * that pass, once, so the following diaphone pass keeps its phones. */
int32_t reloq_kokr61_foreign_phrase(void *state,int32_t begin,int32_t end)
{
    delta_state *d=state;int32_t at=position(begin),limit=right(d,position(end),4);
    delta_token *range=evv_frame_push(2*sizeof *range);if(!range)return 94;
    int32_t result=0;
    for(unsigned i=0;i<4096&&right(d,at,4)!=limit;++i) {
        int32_t word=record(d,at,4);if(!word)break;
        int32_t next=after(word);
        if(body(word)[18]==5) {
            range[0]=(delta_token){0,*EVV_AT(const int32_t *,word)&~3};
            range[1]=(delta_token){0,next};
            result=reloq_kokr61_assign_foreign(d,EVV_REF(range),EVV_REF(range+1));
            if(result)break;
        }
        if(!next||next==at)break;at=next;
    }
    evv_frame_pop(range);return result;
}

/* Keep each foreign affricate inside its already established syllable.
 * The acoustic inventory uses a closure and a frication component, with
 * matching palatal place and a shared diaphone identity. Expanding after
 * Korean diaphone construction prevents an intervocalic closure becoming
 * the preceding syllable's coda. */
static int foreign_affricates(delta_state *d,int32_t left,int32_t end)
{
 int32_t at=left,limit=right(d,end,2);int initial=1;
 for(unsigned guard=0;guard<2304&&right(d,at,2)!=limit;++guard){
  int32_t node=record(d,at,2);if(!node)break;
  int32_t next=after(node);unsigned name=body(node)[7],voiced_affricate=body(node)[11]==26;
  if(name==K_ch||name==K_J||name==K_C){
   /* A support vowel plus w separates the frication in the native ch+w
    * group. Other ch contexts retain the affricate traits on both parts. */
   int32_t support=record(d,next,2),glide=support?record(d,after(support),2):0;
   int separate=name==K_ch&&support&&glide&&body(support)[7]==K_U&&body(support)[11]==3&&body(glide)[7]==K_w;
   unsigned char pair[2]={name==K_ch?K_t:K_d,name==K_ch||name==K_C?K_S:K_Z};
   const delta_token a={0,*EVV_AT(int32_t*,node)&~3},b={0,next};
   lpta_rpta_loadp(d,&a,&b);if(insert_2pt_s(d,2,2,pair,0))return 94;
   int32_t from=a.value;
   for(unsigned i=0;i<2;++i){int32_t component=record(d,from,2);if(!component)return 94;
    unsigned char *p=body(component);p[3]=name==K_ch&&i==1&&separate?2:3;p[4]=3;p[6]=name==K_ch?(i==1&&separate?0:1):3;
    p[11]=name==K_ch?(i==1&&separate?0:23):name==K_C?25:initial&&i==0&&!voiced_affricate?25:26;
    if(name==K_C)p[1]=1;from=after(component);
   }
  }
  if(!next||next==at)return 94;at=next;initial=0;
 }
 return 0;
}

/* Korean lenis stops become voiced between sonorants. A voiced lenis
 * affricate keeps its fully voiced closure, unlike lexical English j.
 * Each adapted root
 * retains its onset, including a letter name inside an initialism. Native
 * word/phrase links and the preceding Hangul phones are already available. */
int32_t reloq_kokr61_foreign_voicing(void *state,int32_t begin,int32_t end)
{
    delta_state *d=state;int32_t at=position(begin),limit=right(d,position(end),4);
    for(unsigned i=0;i<4096&&right(d,at,4)!=limit;++i) {
        int32_t word=record(d,at,4);if(!word)break;
        int32_t next=after(word),left=*EVV_AT(const int32_t *,word)&~3;
        if(body(word)[18]==5&&!korean_dictionary_range(d,left,next)) {
            int32_t root_at=left,root_end=right(d,next,3);
            for(unsigned j=0;j<256&&right(d,root_at,3)!=root_end;++j) {
                int32_t root=record(d,root_at,3);if(!root)break;
                int32_t from=*EVV_AT(const int32_t *,root)&~3;
                int32_t first=record(d,from,2);
                int32_t point=EVV_REF(vmovel(d,EVV_AT(delta_node *,from),2));
                int32_t previous=vgetsc(d,1,0,point,2);
                if(first&&previous&&!(*EVV_AT(int32_t *,previous)&2)) {
                    unsigned before=body(previous)[7],name=body(first)[7];
                    int sonorant=(before>=K_i&&before<=K_o)||(before>=K_m&&before<=K_w);
                    unsigned char voiced=name==K_p?K_b:name==K_t&&body(first)[11]!=23?K_d:name==K_k?K_g:name==K_C?K_J:0;
                    if(sonorant&&voiced) {
                        unsigned char traits[16];memcpy(traits,body(first),sizeof traits);
                        const delta_token a={0,*EVV_AT(const int32_t *,first)&~3},b={0,after(first)};
                        lpta_rpta_loadp(d,&a,&b);
                        if(insert_2pt_s(d,2,1,&voiced,0))return 94;
                        int32_t replacement=record(d,a.value,2);if(!replacement)return 94;
                        memcpy(body(replacement)+8,traits+8,8);
                        if(name==K_C){int32_t n=record(d,a.value,2);if(!n)return 94;body(n)[11]=26;}
                    }
                }
                int32_t after_root=after(root);if(!after_root||after_root==root_at)break;root_at=after_root;
            }
            if(foreign_affricates(d,left,next))return 94;
        }
        if(!next||next==at)break;at=next;
    }
    return 0;
}

int reloq_kokr61_ascii_word(void*state){delta_state*d=state;int16_t type,letters;memcpy(&type,(char*)d+DG_BASE+1386,2);memcpy(&letters,(char*)d+DG_BASE+1406,2);return pending_charset(d)==5&&type==letters;}

/* Korean numeral expansions retain their own alphabet instead of inheriting
 * the preceding Latin property. Restore the pending property after building. */
extern int32_t reloq_kokr61_build_words_native(void*,int32_t,int32_t,int32_t);
int32_t reloq_kokr61_build_words(void*state,int32_t p0,int32_t a,int32_t b)
{
 unsigned char*charset=(unsigned char*)state+DG_BASE+2310;unsigned char saved=*charset;
 int16_t kind,numeric;memcpy(&kind,(char*)state+DG_BASE+1386,2);memcpy(&numeric,(char*)state+DG_BASE+1414,2);
 if(saved==5&&kind==numeric)*charset=1;
 int32_t result=reloq_kokr61_build_words_native(state,p0,a,b);
 unsigned char*pending=(unsigned char*)state+DG_BASE+2288+4;
 *charset=pending[19]?pending[19]:saved;pending[19]=0;return result;
}

int reloq_kokr61_ascii_dictionary_scope(void*state,int32_t a){delta_state*d=state;int32_t w=record(d,position(a),4);return w&&body(w)[18]==5;}
