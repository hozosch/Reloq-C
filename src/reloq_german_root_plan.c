#include "reloq_german_root_plan.h"
#include "reloq_german_morphology.h"
#include "reloq_german_spr.h"
#include "reloq_german_g2p.h"
#include "reloq_german_lexicon.h"
#include <stdint.h>
#include <string.h>

#define LIMIT 128u
struct segment { const char *s; size_t at; unsigned vowel, long_vowel, weak; };
struct plan {struct segment p[LIMIT];char names[LIMIT][8];unsigned accents[LIMIT];size_t n,root_end;};
static int vowel(unsigned char c) {return c && (strchr("aeiouy",c)||c==0xe4||c==0xf6||c==0xfc);}
static int tail(const unsigned char *s,size_t n,const char *t) {size_t k=strlen(t);return n>=k&&!memcmp(s+n-k,t,k);}
int reloq_german_root_plan_os_adjective(const unsigned char *word,size_t n)
{
 static const char *const roots[]={"famos","rigoros"};
 static const char *const endings[]={"","e","en","em","er","es"};
 if(!word||n>9)return 0;
 unsigned char key[10];
 for(size_t i=0;i<n;++i)key[i]=word[i]>='A'&&word[i]<='Z'?word[i]+32:word[i];
 for(size_t i=0;i<sizeof roots/sizeof *roots;++i) {
  size_t k=strlen(roots[i]);if(n<k||memcmp(key,roots[i],k))continue;
  for(size_t j=0;j<sizeof endings/sizeof *endings;++j)
   if(n-k==strlen(endings[j])&&!memcmp(key+k,endings[j],n-k))return 1;
 }
 return 0;
}
size_t reloq_german_feminine_ending(const unsigned char *w,size_t n)
{
 size_t end=tail(w,n,"innen")?5:tail(w,n,"in")?2:0;
 if(!end || n<=end+2 || vowel(w[n-end-1]))return 0;
 static const char *const protected[]={
#include "reloq_german_feminine_protected75.inc"
 };
 for(size_t i=0;i<sizeof protected/sizeof *protected;++i)
  if(tail(w,n,protected[i]))return 0;
 return end;
}
static int put(struct plan *p,const char *s,size_t at,unsigned v,unsigned lng,unsigned weak)
{if(p->n>=LIMIT)return 0;p->p[p->n++]=(struct segment){s,at,v,lng,weak};return 1;}
static const char *vowel_name(unsigned char c,int lng)
{
 switch(c) {
 case 'a':return lng?"a":"A";case 'e':return lng?"e":"E";
 case 'i':return lng?"i":"I";case 'o':return lng?"o":"O";
 case 'u':return lng?"u":"U";case 0xe4:return lng?"'E:'":"E";
 case 0xf6:return lng?"'oe'":"'OE'";default:return lng?"y":"Y";
 }
}
/* These are independent phone-class predicates, not original token IDs. */
static int onset_pair(const char *a,const char *b)
{
 if(!strcmp(b,"j")||!strcmp(b,"w"))return strlen(a)==1&&strchr("bptdgkflrTszmnv",*a);
 if(!strcmp(b,"r"))return strlen(a)==1&&strchr("bptdgkfPS",*a);
 if(!strcmp(b,"l"))return strlen(a)==1&&strchr("bpgkfPS",*a);
 if(!strcmp(a,"g"))return !strcmp(b,"m")||!strcmp(b,"n");
 if(!strcmp(a,"k"))return !strcmp(b,"n")||!strcmp(b,"v");
 if(!strcmp(a,"T"))return !strcmp(b,"v");
 if(!strcmp(a,"S"))return strlen(b)==1&&strchr("bptdgkfvmn",*b);
 return !strcmp(a,"s")&&!strcmp(b,"t");
}
static size_t onset(const struct plan *p,size_t first,size_t end)
{
 if(first==end)return end;
 size_t at=end-1;
 if(!strcmp(p->p[at].s,"G"))return end;
 if(at>first&&(!(!strcmp(p->p[at].s,"j")&&!p->p[at].weak))&&onset_pair(p->p[at-1].s,p->p[at].s))--at;
 if(at>first&&!strcmp(p->p[at-1].s,"S")&&
    (!strcmp(p->p[at].s,"p")||!strcmp(p->p[at].s,"t")))--at;
 return at;
}
static const char *front_g_class(const unsigned char *w,size_t n)
{
 static const char *const french[]={
#include "reloq_german_g_french74.inc"
 };
 static const char *const affricate[]={
#include "reloq_german_g_affricate74.inc"
 };
 const char *front_g="g";
 for(size_t k=0;k<sizeof french/sizeof *french;++k)
  if(strlen(french[k])==n&&!memcmp(w,french[k],n))front_g="Z";
 for(size_t k=0;k<sizeof affricate/sizeof *affricate;++k)
  if(strlen(affricate[k])==n&&!memcmp(w,affricate[k],n))front_g="J";
 return front_g;
}
static int spell(struct plan *p,const unsigned char *w,size_t n,int general)
{
 const char *front_g=front_g_class(w,n);
 for(size_t i=0;i<n;) {
  size_t at=i;unsigned char c=w[i++],next=i<n?w[i]:0;
  if(vowel(c)) {
   const char *s=NULL;int lng=1;
   if(c=='y' && i==n && !w[i])s="i";
   else if(c=='a'&&next=='u'){s="'aw'";++i;}
   else if((c=='a'||c=='e')&&(next=='i'||next=='y')){s="'aj'";++i;}
   else if((c=='e'||c==0xe4)&&next=='u'){s="'oj'";++i;}
   else if(c=='o'&&next=='i' &&
       !(i+1<n && (w[i+1]=='n' || (w[i+1]=='s' && i+2<n && strchr("tm",w[i+2])))) &&
       !reloq_german_morphology_is_root_boundary(w,n,i)){s="'oj'";++i;}
   else if(c=='o'&&next=='u'&&!reloq_german_morphology_is_root_boundary(w,n,i)){++i;lng=!(i+1<n&&!vowel(w[i])&&!vowel(w[i+1]));s=lng?"u":"U";}
   else if(c=='i'&&next=='e'&&i+2<n&&w[i+1]=='l'&&w[i+2]=='l') {
    if(!put(p,"j",at,0,0,1))return 0;
    continue;
   }
   else if(c=='i'&&next=='e'){s="i";++i;}
   else if(c==next&&strchr("aeo",c)){s=vowel_name(c,1);++i;}
   else if(next=='h'){s=vowel_name(c,1);++i;}
   else if(c=='i'&&next&&strchr("aou",next) &&
       !(at>=2 && w[at-1]=='r' && !vowel(w[at-2]))) {if(!put(p,"j",at,0,0,1))return 0;continue;}
   else if(c=='u'&&next&&strchr("aeio",next)) {if(!put(p,"w",at,0,0,1))return 0;continue;}
   if(!s) {
    size_t j=i;while(j<n&&!vowel(w[j]))++j;
    size_t gap=j-i;
    if(gap>1) {
     int pair=gap==2&&((w[i+1]=='r'&&strchr("bptdgkf",w[i]))||
       (w[i+1]=='l'&&strchr("bpgkf",w[i]))||(w[i]=='g'&&strchr("mn",w[i+1]))||
       (w[i]=='k'&&w[i+1]=='n'));
     lng=pair;
    }
    if(j<n || (gap>=2 && w[i+1]=='h' && strchr("ptg",w[i]))) {
     int quantity=reloq_german_g2p_cluster_quantity(w+i,gap);
     if(quantity>=0)lng=quantity;
    }
    if(next=='x')lng=0;
    if(next=='q' && i+1<n && w[i+1]=='u')lng=0;
    if(c=='i'&&i+1==n&&(strchr("kgc",next)||(general&&next=='q')))lng=0;
    if(general&&c=='u'&&i+1==n&&strchr("lkqf",next))lng=0;
    if(c=='u'&&i+1==n&&next=='s')lng=0;
    if(c=='u'&&i+1==n&&next=='m')lng=0;
    if(c=='i'&&i+4==n&&!memcmp(w+i,"smus",4))lng=0;
    s=vowel_name(c,lng);
   }
   if(!put(p,s,at,1,(unsigned)lng,0))return 0;
   continue;
  }
  const char *s=NULL;
  if(at==0&&c=='d'&&i+2<n&&!memcmp(w+i,"sch",3)){s="J";i+=3;}
  else if(c=='t'&&next=='c'&&i+1<n&&w[i+1]=='h'){s="C";i+=2;}
  else if(c=='t'&&next=='s'&&i+2<n&&w[i+1]=='c'&&w[i+2]=='h'){s="C";i+=3;}
  else if(c=='s'&&next=='c'&&i+1<n&&w[i+1]=='h'){s="S";i+=2;}
  else if(c=='c'&&next=='h') {
   unsigned char prev=at?w[at-1]:0;
   unsigned char following=i+1<n?w[i+1]:0;
   s=at==0?(following=='e'||following=='i'||following==0xe4||following==0xf6||following==0xfc?"X":"k"):
       following=='s'&&!reloq_german_morphology_is_root_boundary(w,n,i+1)?"k":
       prev&&strchr("aou",prev)?"x":"X";++i;
  } else if(c=='c'&&next=='c'&&i+1<n&&strchr("eiy",w[i+1])) {
   if(!put(p,"k",at,0,0,0))return 0;
   s="T";++i;
  } else if(c=='c'&&next=='k'){s="k";++i;}
  else if(c=='t'&&next=='z'){s="T";++i;}
  else if(c=='t'&&next=='h'){s="t";++i;if(i<n&&w[i]=='t')++i;}
  else if(c=='r'&&next=='h'){s="r";++i;}
  else if(c=='r'&&next=='r'&&i+1<n&&w[i+1]=='h'){s="r";i+=2;}
  else if(c=='g'&&next=='h'){s="g";++i;}
  else if(c=='p'&&next=='h'){s="f";++i;}
  else if(c=='p'&&next=='f'){s="P";++i;}
  else if(c=='s'&&next=='h'){s="S";++i;}
  else if(c=='w'&&next=='h'){s="v";++i;}
  /* Full root vowels keep g as an onset after the velar nasal. Weak
   * inflection has already been removed from this spelling domain. */
  else if(c=='n'&&next=='g'){s="G";if(i+1>=n||(!vowel(w[i+1])&&!strchr("rl",w[i+1]))||(w[i+1]=='l'&&i+2==n))++i;}
  else if(c=='n'&&next=='k')s="G";
  else if(c=='q'&&next=='u'){if(!put(p,"k",at,0,0,0))return 0;s="v";++i;}
  else if(c=='x'){if(!put(p,"k",at,0,0,0))return 0;s="s";}
  else if(c=='t'&&next=='i'&&i+1<n&&strchr("aou",w[i+1]))s=at&&w[at-1]=='s'?"t":"T";
  else if(c=='s'&&next=='i'&&i+1<n&&strchr("aou",w[i+1]))s=at&&w[at-1]=='s'?"s":"z";
  else switch(c) {
   case 'b':s="b";break;case 'd':s="d";break;case 'f':s="f";break;
   case 'g':s=w[i]&&strchr("eiy",w[i])?front_g:"g";break;
   case 'h':s="h";break;case 'j':s="j";break;
   case 'q':case 'k':s="k";break;case 'l':s="l";break;case 'm':s="m";break;
   case 'n':s="n";break;case 'p':s="p";break;case 'r':s="r";break;
   case 't':s="t";break;case 'v':s="v";break;case 'w':s="v";break;
   case 'z':s="T";break;case 0xdf:s="s";break;
   /* Weak inflection is outside the stress domain, but its front vowel
    * still conditions c in the preceding root. Letter normalization in
    * the original precedes the final root/ending syllable join. */
   case 'c':s=w[i]&&strchr("eiy",w[i])?"T":"k";break;
   case 's':s=at==0&&next&&strchr("pt",next)?"S":
       next&&vowel(next)&&!(at&&w[at-1]=='s')?"z":"s";break;
   default:return 0;
  }
  if(c==next&&c!='s'&&!(c=='c'&&!strcmp(s,"T")))++i;
  if(c=='s'&&next=='s'){s="s";++i;}
  if(!put(p,s,at,0,0,0))return 0;
 }
 unsigned nuclei=0;
 for(size_t i=0;i<p->n;++i)nuclei+=p->p[i].vowel;
 if(!reloq_german_g2p_v_voiced(w,n,nuclei==1))
  for(size_t i=0;i<p->n;++i)
   if(p->p[i].at<n && w[p->p[i].at]=='v')p->p[i].s="f";
 return 1;
}
static const struct {const char *stem;unsigned forms;} ie_roles[]={
#include "reloq_german_ie_roles.inc"
};
static unsigned ie_role(const unsigned char *w,size_t n)
{
 for(size_t i=0;i<sizeof ie_roles/sizeof *ie_roles;++i)
  if(strlen(ie_roles[i].stem)==n&&!memcmp(w,ie_roles[i].stem,n))return ie_roles[i].forms;
 return 0;
}
static int avoids_final_stress(const unsigned char *w,size_t n)
{
 static const char *const stems[]={
#include "reloq_german_root_stress_roles.inc"
 };
 for(size_t i=0;i<sizeof stems/sizeof *stems;++i)
  if(strlen(stems[i])==n&&!memcmp(w,stems[i],n))return 1;
 return 0;
}
static int from_reading(struct plan *p,const struct reloq_lexicon_override *e,size_t *primary,int preserve_reduced)
{
 struct reloq_german_spr_cursor c={0};struct reloq_german_spr_event ev;int rc;
 *primary=SIZE_MAX;unsigned stress=0;
 while((rc=reloq_german_spr_next(e->spr,e->spr_length,&c,&ev))==1) {
  if(ev.kind==RELOQ_DE_SPR_STRESS)stress=ev.stress;
  if(ev.kind==RELOQ_DE_SPR_STRESS&&ev.stress==1)*primary=ev.syllable;
  if(ev.kind!=RELOQ_DE_SPR_PHONE)continue;
  if(p->n==LIMIT||ev.length>=sizeof p->names[0])return 0;
  /* Standalone schwa/vocalic-r reduction can hide an underlying full root
   * vowel. Such a reading does not establish root-internal quantity. */
  if(ev.phone==RELOQ_DE_PHONE_R) {
   p->accents[p->n]=stress;
   if(preserve_reduced!=2 || !put(p,"@",SIZE_MAX,1,0,0) ||
      !put(p,"r",SIZE_MAX,0,0,0))return 0;
   continue;
  }
  if(!preserve_reduced&&ev.phone==RELOQ_DE_PHONE_AT)return 0;
  char *name=p->names[p->n];memcpy(name,e->spr+ev.offset,ev.length);name[ev.length]=0;
  int v=ev.phone<=RELOQ_DE_PHONE_AT||ev.phone>=RELOQ_DE_PHONE_Q_E_COLON;
  int lng=v&&!(strlen(name)==1&&strchr("AEIOUY@",*name))&&strcmp(name,"'OE'");
  p->accents[p->n]=stress;
  if(!put(p,name,SIZE_MAX,(unsigned)v,(unsigned)lng,!strcmp(name,"j")&&!(ev.offset>=2&&e->spr[ev.offset-2]=='.')))return 0;
 }
 return !rc&&p->n&&*primary!=SIZE_MAX;
}
static int root_plan(const struct reloq_lexicon *lexicon,const unsigned char *word,size_t length,
 unsigned char *out,size_t capacity,size_t *written,int unclassified,int extended)
{
 if(!word||!written||length<3||length>=LIMIT)return 0;
 unsigned char w[LIMIT];
 for(size_t i=0;i<length;++i) {unsigned char c=word[i];if(c>='A'&&c<='Z')c+=32;
  if(c==0xc4||c==0xd6||c==0xdc)c+=32;
  w[i]=c;}
 w[length]=0;
 /* These spellings can enter native prefix/particle analysis. That stage
  * is outside this loan-root plan: do not flatten its boundaries here. */
 static const char *const native_prefixes[]={"be","ge","ver","er","ent","zer",
  "an","auf","aus","ab","ein","um","vor","nach"};
 int native_prefix=0;
 for(size_t i=0;i<sizeof native_prefixes/sizeof *native_prefixes;++i) {
  size_t n=strlen(native_prefixes[i]);
  if(length>n+3&&!memcmp(w,native_prefixes[i],n)&&
     !(n==2&&!memcmp(w,"geo",3)) &&
     reloq_german_morphology_prefix_onset(w,length,n))native_prefix=1;
 }
 static const struct {const char *letters,*phones;} foreign[]={
  {"bio",".0bi.0o"},{"geo",".0ge.0o"},{"neo",".0ne.0o"},{"ko",".0ko"}};
 for(size_t k=0;k<sizeof foreign/sizeof *foreign;++k) {
  size_t n=strlen(foreign[k].letters);
  if(length<n+3||memcmp(w,foreign[k].letters,n)||(n==2&&w[n]!='o'))continue;
  unsigned char rest[1024];size_t rn=0;
  if(reloq_german_root_plan_word(lexicon,w+n,length-n,rest,sizeof rest,&rn)!=1)continue;
  size_t pn=strlen(foreign[k].phones),total=rn+pn;
  *written=total;if(!out||capacity<=total)return -2;
  memcpy(out,"`[",2);memcpy(out+2,foreign[k].phones,pn);memcpy(out+2+pn,rest+2,rn-1);return 1;
 }
 size_t root=length;const char *ending="";
 const size_t feminine=reloq_german_feminine_ending(w,length);
 /* Requested departure from the reference: these -os adjectives retain
  * the stressed root also observed in their inflected forms. The original
  * misclassifies their bare final s as nominal inflection. This is a
  * lexical stress class, not a stored whole-word phone transcription. */
 const int adjective_os=reloq_german_root_plan_os_adjective(w,length);
 /* Root and ending are separate until syllabification and stress. */
 static const char *const endings[]={"ungen","ung","end","est","et","en","er","em","es","st","e","s","t"};
 if(feminine) {root-=feminine;ending=feminine==5?"innen":"in";}
 for(size_t i=0;!feminine && i<sizeof endings/sizeof endings[0];++i) {
  if(!extended && (!strcmp(endings[i],"et")||!strcmp(endings[i],"est")))continue;
  size_t n=strlen(endings[i]);if((root>n+3 ||
      (n==1&&endings[i][0]=='s'&&root>3&&w[root-2]=='o'))&&tail(w,root,endings[i])){
   if((!strcmp(endings[i],"st")||!strcmp(endings[i],"end"))&&!tail(w,root-n,"ier"))continue;
   /* In -iet/-iest the e still belongs to the root vowel. The weak
    * inflection pass only starts after a consonantal stem here. */
   if((!strcmp(endings[i],"et")||!strcmp(endings[i],"est")) && vowel(w[root-n-1]))continue;
   if(n==1&&*endings[i]=='s'&&w[root-2]=='s')continue;
   if(n==1&&*endings[i]=='s'&&adjective_os)continue;
   /* The -s inflection is outside a consonant+o root's stress domain.
    * The original keeps -los and vowel+os in their separate analyses. */
   if(n==1&&*endings[i]=='s'&&(w[root-2]=='u'||
       (w[root-2]=='o'&&(strchr("aeioul",w[root-3]) ||
        (root>=4&&!memcmp(w,"her",3))))))continue;
   root-=n;ending=endings[i];break;
  }
 }
 const int extended_weak=!strcmp(ending,"et")||!strcmp(ending,"est");
 const int o_inflection=!strcmp(ending,"s")&&w[root-1]=='o';
 if(!strcmp(ending,"end")||!strcmp(ending,"st")) {
  unsigned char prior[1024];size_t pn=0;int estimated=0;
  if(reloq_german_g2p_word_best_effort(w,length,prior,sizeof prior,&pn,&estimated)==1) {
   const char *primary=strstr((const char *)prior,".1");
   if(primary && strstr(primary,".2"))return 0;
  }
  if(!strcmp(ending,"end")&&root>6)for(size_t split=3;split+3<=root-3;++split)
   if(reloq_german_morphology_is_root_boundary(w,root-3,split))return 0;
 }
 if(extended_weak) {
  unsigned char prior[1024];size_t pn=0;int estimated=0;
  if(reloq_german_g2p_word_best_effort(w,length,prior,sizeof prior,&pn,&estimated)==1 &&
     !estimated && strstr((const char *)prior,".2"))return 0;
 }
 /* The dental past/participle ending is stripped after weak inflection,
  * before finding the stressed -ier- root. */
 int dental=0;
 if(tail(w,length,"ionaries")){root=length;ending="";}
 if(ending[0]=='e'&&root>4&&w[root-1]=='t'&&tail(w,root-1,"ier")){--root;dental=1;}
 if(extended_weak && dental &&
    reloq_german_morphology_is_root_boundary(w,length,root))return 0;
 if(tail(w,length,"ie")||tail(w,length,"ier")||tail(w,length,"ismus")||tail(w,length,"ist")||tail(w,length,"it\344t")){root=length;ending="";}
 if(!strcmp(ending,"t")&&!tail(w,root,"ier")){root=length;ending="";}
 int adjective=tail(w,root,"isch")&&root>6;
 if(adjective)root-=4;
 /* A native -er derivation retains its weak vowel and inherited accent.
  * It does not establish the loan-root stress domain used below. */
 if(adjective&&tail(w,root,"er"))return 0;
 int glide=0;
 if(tail(w,length,"ien")&&(ie_role(w,length-3)&2)) {root=length-3;ending="ien";glide=1;}
 else if(tail(w,length,"ie")&&(ie_role(w,length-2)&1)) {root=length-2;ending="ie";glide=1;}
 int ion=0;
 for(size_t i=1;i+3<root;++i)if((w[i]=='t'||w[i]=='s'||w[i]=='g')&&!memcmp(w+i+1,"ion",3)) {
  static const char *const follow[]={"","al","ell","\344r","ier","ist","ismus","istisch","alistisch","alismus","alit\344t","ellst","alst","ary","aries"};
  for(size_t k=0;k<sizeof follow/sizeof *follow;++k)if(strlen(follow[k])==root-i-4&&!memcmp(w+i+4,follow[k],root-i-4))ion=1;
 }
 /* -ier- is the same stressed loan-root nucleus before -en/-t/-e.
  * Keeping it in the root plan also preserves internal j/w glides;
  * the old fallback split these into separate vowel syllables. */
 /* ui also has a two-vowel reading (ruinieren); this root family alone
  * does not establish which analysis owns that sequence. Keep it outside
  * the new coverage until its lexical glide/hiatus role is available. */
 int ier_root=tail(w,root,"ier")&&!strstr((const char *)w,"ui");
 int final_umlaut=root && (w[root-1]==0xfc || w[root-1]==0xe4);
 if (final_umlaut) for(size_t i=0;i<sizeof native_prefixes/sizeof *native_prefixes;++i) {
  size_t n=strlen(native_prefixes[i]);
  if(length>n+2&&!memcmp(w,native_prefixes[i],n))return 0;
 }
 int ic_root=tail(w,root,"ic")&&!vowel(w[root-3]);
 unsigned root_class=reloq_german_morphology_constituent_class(w,root);
 /* A compound-root lookup is not an inflection license for -et/-est.
  * Those forms must have passed the separate weak-inflection analysis;
  * otherwise a complete final word (such as the class containing fest)
  * keeps its own root domain. */
 int classified_root=!!(root_class&6u) && *ending && !extended_weak;
 int or_root=tail(w,root,"or");
 if(or_root && ((!memcmp(w,"da",2)||!memcmp(w,"wo",2))&&root<6))return 0;
 int um_root=tail(w,root,"ium")&&root>4 &&
     !(w[root-4]=='r'&&!vowel(w[root-5]));
 int accepted=feminine||um_root||o_inflection||or_root||classified_root||final_umlaut||adjective||glide||ion||tail(w,root,"it\344t")||tail(w,root,"ismus")||tail(w,root,"ist")||tail(w,root,"ik")||tail(w,root,"ie")||ier_root||ic_root;
 /* These observed loan-root endings use the same root quantity and stress
  * pass. Weak inflection must not send them through first-syllable stress
  * and internal schwa replacement in the permissive spelling fallback. */
 static const char *const loan_tails[]={"al","ar","ant","anz","iv","il","ur","ol","os","ys","ett","ekt","ut","ess","ell","oll"};
 for(size_t k=0;k<sizeof loan_tails/sizeof *loan_tails;++k)
  if(root>strlen(loan_tails[k])+1&&tail(w,root,loan_tails[k])&&
     !tail(w,root,"bar")&&!tail(w,root,"los")&&!tail(w,root,"ail")&&!tail(w,root,"oil"))accepted=1;
 if(!accepted&& !strcmp(ending,"en")&&root>3&&w[root-1]=='i')accepted=1;
 if(*ending&&tail(w,root,"ens"))accepted=1;
 /* ger_suffixes retains a witnessed monosyllabic root before -ung,
  * even if that root has no separate infinitive-role record. */
 if(!strcmp(ending,"ung")||!strcmp(ending,"ungen")) {
  const struct reloq_lexicon_override *base=NULL;
  if(reloq_german_morphology_observed_root(lexicon,w,root,&base)) {
   struct plan probe={0}, spelled={0};size_t accent=0;
   if(from_reading(&probe,base,&accent,0)) {
    unsigned nuclei=0;const char *root_vowel=NULL,*spelled_vowel=NULL;
    for(size_t i=0;i<probe.n;++i)if(probe.p[i].vowel){++nuclei;root_vowel=probe.p[i].s;}
    if(spell(&spelled,w,root,0))
     for(size_t i=0;i<spelled.n;++i)if(spelled.p[i].vowel)spelled_vowel=spelled.p[i].s;
    /* The isolated reading cannot establish a derivational nucleus when
     * its lexical quantity or diphthong differs from the open stem. */
    if(nuclei==1&&spelled_vowel&&!strcmp(root_vowel,spelled_vowel))accepted=1;
   }
  }
 }
 int expanded=0,new_tail=0;
 static const char *const more_tails[]={"akt","ect","ort","og","ag","abl","ent","enz","at","an","id","\344r","\366s"};
 if(!accepted)for(size_t k=0;k<sizeof more_tails/sizeof *more_tails;++k)
  if(root>strlen(more_tails[k])+1&&tail(w,root,more_tails[k])){expanded=1;new_tail=k>=6;}
 const int general=unclassified && !accepted && !expanded;
 if(!accepted&&!expanded&&!general)return 0;
 /* Default root stress has no authority to erase structured compounds
  * or reduced inflectional stems. Those analyses do not establish the
  * unreduced quantity needed by this additional root domain. Existing
  * specialized root classes retain their independently observed guards. */
 int structured_extension=0;
 if(general) {
  /* Derivational and glide/hiatus domains retain their separate analysis.
   * The default root pass consumes full nuclei, not guessed j/w groups. */
  if(tail(w,root,"ig") || tail(w,root,"ing") ||
     tail(w,root,"est") || tail(w,root,"enst") || tail(w,root,"erst") || tail(w,root,"sst") || tail(w,root,"\337st"))structured_extension=1;
  for(size_t k=0;k+1<root;++k)
   if((w[k]=='i'&&strchr("aou",w[k+1])) ||
      (w[k]=='u'&&strchr("aeio",w[k+1])&&(k==0||w[k-1]!='q')) ||
      (w[k]=='o'&&w[k+1]=='i'))structured_extension=1;
  /* Longer internal consonant groups can hide a constituent boundary.
   * The widened default accent domain cannot decide that ambiguity. */
  for(size_t k=1;k<root;++k)if(vowel(w[k-1])&&!vowel(w[k])) {
   size_t end=k;while(end<root&&!vowel(w[end]))++end;
   if(end<root&&end-k>2)structured_extension=1;
  }
  static const char *const particles[]={"bei","hin","mit","weg","her","zu","miss"};
  for(size_t k=0;k<sizeof particles/sizeof *particles;++k) {
   size_t n=strlen(particles[k]);
   if(length>n+1&&!memcmp(w,particles[k],n))return 0;
  }
  for(size_t k=0;k<sizeof native_prefixes/sizeof *native_prefixes;++k) {
   size_t n=strlen(native_prefixes[k]);
   if(length>n+1&&!memcmp(w,native_prefixes[k],n))return 0;
  }
  /* Pronominal adverbs retain their separate particle-stress domain. */
  if(length>3&&(!memcmp(w,"da",2)||!memcmp(w,"wo",2)||!memcmp(w,"hier",4)))return 0;
  /* An unresolved root is not allowed to erase an already formed
   * compound or a reduced inflectional stem. Those domains are owned by
   * morphology, rather than by the default root-accent pass. */
  unsigned char prior[1024];size_t pn=0;int ap=0;
  const struct reloq_lexicon_override *known=NULL;
  if(reloq_german_g2p_word_best_effort(w,length,prior,sizeof prior,&pn,&ap)==1 &&
     strstr((const char *)prior,".2") &&
     !reloq_german_morphology_observed_root(lexicon,w,root,&known))return 0;
  /* A weak ending on -es- leaves that e inside the root (synthes-e).
   * The isolated -es spelling would instead be reduced as an inflection.
   * Other final schwas, including epenthetic ones, retain their domain. */
  int (*read_stem)(const unsigned char *,size_t,unsigned char *,size_t,size_t *,int *)=
      vowel((unsigned char)ending[0])?reloq_german_g2p_open_stem_best_effort:
      reloq_german_g2p_word_best_effort;
  if(read_stem(w,root,prior,sizeof prior,&pn,&ap)==1) {
   const unsigned char *last=(const unsigned char *)strrchr((const char *)prior,'.');
   if(memchr(prior,'R',pn) ||
      (last && strchr((const char *)last,'@') &&
       !(*ending && tail(w,root,"es"))) ||
      (memchr(prior,'@',pn) && strstr((const char *)w,"ie")))return 0;
  }
  /* A written y followed by a vowel belongs to glide/diphthong analysis;
   * vocalic y inside an ordinary root has no such restriction. */
  for(size_t k=0;k<root;++k)if(w[k]=='y'&&
      ((k+1<root&&vowel(w[k+1]))||(k&&vowel(w[k-1]))))return 0;
  static const char *const derived[]={"bar","sam","haft","los","lich","keit","heit","schaft","in","um","or"};
  for(size_t k=0;k<sizeof derived/sizeof *derived;++k)
   if(tail(w,root,derived[k]) && !(ending[0]=='e'&&!strcmp(derived[k],"or")))return 0;
 }
 /* A witnessed complete compound head outranks a coincidental loan-root
  * ending. The public original keeps two root accents here; interpreting
  * an internal -ort as the complete root destroys the first constituent.
  * Retain the established fallback only when both predicates agree. */
 if(expanded) {
  int head=0;
  for(size_t k=3;k+3<=root;++k) {
   unsigned char first[1024];size_t fn=0;
   /* A coincidental final-word substring inside an unknown loan root
    * does not establish a boundary. Require a complete simple first
    * root; nested and derived compounds retain their separate parser. */
   if((reloq_german_morphology_constituent_class(w+k,root-k)&16u) &&
      reloq_german_g2p_word(w,k,first,sizeof first,&fn)==1 &&
      fn>4 && !memcmp(first,"`[.1",4) &&
      !strchr((const char *)first+4,'.'))head=1;
  }
  unsigned char prior[1024];size_t pn=0;int approximate=0;
  if(head && reloq_german_g2p_word_best_effort(w,length,prior,sizeof prior,
       &pn,&approximate)==1 && strstr((const char *)prior,".2"))return 0;
 }
 if(new_tail || tail(w,root,"ess") || tail(w,root,"ell")) {
  int boundary=0;
  for(size_t k=3;k+3<=root;++k)
   if((reloq_german_morphology_constituent_class(w,k)&1u) ||
      (reloq_german_morphology_constituent_class(w+k,root-k)&16u))boundary=1;
  unsigned char prior[1024];size_t pn=0;int approximate=0;
  /* A schwa invented by permissive G2P is not evidence of a compound.
   * Require the original constituent predicate before retaining that
   * reduced domain ahead of the ordinary loan-root stress pass. */
  if(boundary && reloq_german_g2p_word_best_effort(w,length,prior,sizeof prior,&pn,&approximate)==1 &&
     strstr((const char *)prior,".2") &&
     reloq_german_g2p_word_best_effort(w,root,prior,sizeof prior,&pn,&approximate)==1 &&
     (memchr(prior,'R',pn) || (!tail(w,root,"ell") && memchr(prior,'@',pn))))return 0;
 }
 /* A second written e after an ie root has competing unstripped and
  * compound analyses; weak-ending stripping is not established for it. */
 if(*ending&&tail(w,root,"ie"))return 0;
 if(tail(w,length,"ismus")){root=length;ending="";}
 struct plan p={0};p.root_end=root;
 size_t lexical_primary=SIZE_MAX;const struct reloq_lexicon_override *entry=NULL;
 int lexical=(!classified_root || feminine) && reloq_german_morphology_observed_root(lexicon,w,root,&entry)&&from_reading(&p,entry,&lexical_primary,feminine?2:0);
 if(feminine && !lexical && reloq_lexicon_find_override(lexicon,w,root,&entry)==1) {
  memset(&p,0,sizeof p);lexical=from_reading(&p,entry,&lexical_primary,2);
 }
 if(feminine && !lexical) {
  unsigned char derived[1024];size_t n=0;
  int status=reloq_german_root_plan_word(lexicon,w,root,derived,sizeof derived,&n);
  if(!status)status=reloq_german_lexicon_word(lexicon,w,root,derived,sizeof derived,&n);
  if(!status && tail(w,root,"er")) {
   int approximate=0;
   status=reloq_german_g2p_word_best_effort(w,root,derived,sizeof derived,&n,&approximate);
  }
  if(status==1) {
   struct reloq_lexicon_override base={0};base.spr=derived;base.spr_length=n;
   memset(&p,0,sizeof p);
   lexical=from_reading(&p,&base,&lexical_primary,2);
  }
 }
 int derived_root=0;
 if(!lexical&&ier_root&&root>5&&!memchr(w,'y',root-3)) {
  memset(&p,0,sizeof p);
  if(reloq_german_morphology_observed_root(lexicon,w,root-3,&entry)&&
     from_reading(&p,entry,&lexical_primary,1)&&p.n&&!p.p[p.n-1].vowel) {
   unsigned char c=w[root-4];
   /* A written g in ng is part of the nasal's spelling. The
    * full -ier nucleus exposes a separate /g/; it must not replace /G/.
    * Ordinary final obstruents instead regain their voicing in place. */
   if(c=='g'&&!strcmp(p.p[p.n-1].s,"G")) {
    if(!put(&p,"g",root-4,0,0,0))return 0;
   } else if(c=='b'||c=='d'||c=='g'||c=='v')p.p[p.n-1].s=c=='b'?"b":c=='d'?"d":c=='g'?"g":"v";
   if(c=='s'&&root>4&&w[root-5]!='s')p.p[p.n-1].s="z";
   if(!put(&p,"i",root-3,1,1,0)||!put(&p,"r",root-1,0,0,0))return 0;
   derived_root=1;
  }
 }
 /* A successful particle analysis has priority at the caller. A bare
  * an- spelling is not itself a successful particle: unresolved -ier-
  * loan roots still enter stress and quantity assignment as one domain. */
 int unresolved_an_loan=ier_root && !memcmp(w,"an",2) && w[2]!='n';
 if(native_prefix && !unresolved_an_loan && !classified_root && !(feminine&&lexical) && (!lexical || strcmp(p.p[0].s,"v")))return 0;
 if(!lexical&&!derived_root) {memset(&p,0,sizeof p);if(!spell(&p,w,root,general))return 0;}

 if(p.n&&root) {
  unsigned char last=w[root-1];
  if(lexical&&(last=='b'||last=='d'||last=='g'||last=='v') &&
     !(last=='g'&&!strcmp(p.p[p.n-1].s,"G"))) {
   /* Inflection restores voicing, not a spelling-derived place of
    * articulation. A loan root's final /tS/ belongs to /dZ/, not /g/. */
   const char *s=p.p[p.n-1].s;
   p.p[p.n-1].s=!strcmp(s,"C")?"J":last=='b'?"b":last=='d'?"d":
       last=='g'?(w[root]&&strchr("eiy",w[root])?front_g_class(w,root):"g"):"v";
  }
  /* Inflection is attached before the original diaphone rules decide
   * voicing. A standalone root's final s is not a voiced onset reading. */
  const int adjective_s=adjective&&p.n>1&&
      (p.p[p.n-2].vowel || (strlen(p.p[p.n-2].s)==1&&
       strchr("rlmnG",p.p[p.n-2].s[0])));
  /* -isch is a vowel-initial derivation too. Removing it for root stress
   * must not give the exposed single s a word-final voiceless reading.
   * A preceding voiceless cluster and written ss/ß retain their own s. */
  if((adjective_s||ending[0]=='e'||ending[0]=='u'||feminine)&&last=='s'&&root>1&&w[root-2]!='s'&&!strcmp(p.p[p.n-1].s,"s"))
   p.p[p.n-1].s="z";
 }
 size_t root_n=p.n;
 if(dental&&!put(&p,"t",root,0,0,1))return 0;
 if(adjective&&(!put(&p,"I",root,1,0,1)||!put(&p,"S",root+1,0,0,0)))return 0;
 if(glide) {
  if(!put(&p,"j",root,0,0,1)||!put(&p,"@",root+1,1,0,1))return 0;
  if(!strcmp(ending,"ien")&&!put(&p,"n",root+2,0,0,0))return 0;
 } else if(feminine) {
  if(!put(&p,"I",root,1,0,1)||!put(&p,"n",root+1,0,0,0))return 0;
  if(feminine==5&&(!put(&p,"@",root+3,1,0,1)||!put(&p,"n",root+4,0,0,0)))return 0;
 } else if(!strcmp(ending,"ung")||!strcmp(ending,"ungen")) {
  if(!put(&p,"U",root,1,0,1)||!put(&p,"G",root+1,0,0,0))return 0;
  if(!strcmp(ending,"ungen")&&(!put(&p,"@",root+3,1,0,1)||!put(&p,"n",root+4,0,0,0)))return 0;
 } else if(*ending) {
  if(ending[0]=='e') {
   if(!put(&p,"@",root,1,0,1))return 0;
   for(size_t k=1;ending[k];++k)
    if(!put(&p,ending[k]=='n'?"n":ending[k]=='r'?"r":ending[k]=='m'?"m":ending[k]=='t'||ending[k]=='d'?"t":"s",root+k,0,0,0))return 0;
  } else {
   if(!put(&p,ending[0]=='s'?"s":"t",root,0,0,0))return 0;
   if(ending[0]=='s'&&ending[1]=='t'&&!put(&p,"t",root+1,0,0,0))return 0;
  }
 }
 size_t nuclei[LIMIT],starts[LIMIT],count=0,primary=SIZE_MAX;
 for(size_t i=0;i<p.n;++i)if(p.p[i].vowel) {
  nuclei[count]=i;starts[count]=count?onset(&p,nuclei[count-1]+1,i):0;++count;
 }
 if(!count)return 0;
 /* Root stress is assigned from the root's phone shape, before weak-ending
  * resyllabification. A final long vowel plus coda, or short vowel with two
  * coda phones, is heavy. Written ie independently marks final root stress.
  * Unstressed a reduction happens after this decision. */
 size_t root_count=0;
 for(size_t k=0;k<count;++k)if(nuclei[k]<root_n)++root_count;
 if(!root_count || (expanded && root_count<2))return 0;
 primary=root_count>1?root_count-2:0;
 size_t last=nuclei[root_count-1],coda=root_n-last-1;
 size_t written_coda=coda;
 if(general && coda==2 && p.p[last+1].at!=SIZE_MAX &&
    p.p[last+1].at==p.p[last+2].at)written_coda=1;
 if((general || o_inflection || ion) && root_count>2 && !coda &&
    p.p[nuclei[root_count-2]].long_vowel)primary=root_count-3;
 if((p.p[last].long_vowel&&coda>=1)||(!p.p[last].long_vowel&&written_coda>=2)||
    (p.p[last].at!=SIZE_MAX&&p.p[last].at+1<root&&w[p.p[last].at]=='i'&&w[p.p[last].at+1]=='e')||
    (!strcmp(ending,"en")&&w[root-1]=='i')||
    (root>=2&&w[root-1]==w[root-2]&&!vowel(w[root-1])))primary=root_count-1;

 /* A single simple coda has an observed weak-ending quantity. Longer
  * one-nucleus stems still need the inner inflection/root legality pass;
  * do not replace that established path with this loan-root analysis. */
 int simple_weak=extended_weak && coda==1 && p.p[last].at!=SIZE_MAX &&
     p.p[last].at+2==root;
 if(general && ((root_count<2 && !simple_weak) || p.p[last].s[0]=='\'' ||
    (structured_extension && (primary!=root_count-1 ||
      (!p.p[last].long_vowel && !(root>=2&&w[root-1]==w[root-2]))))))return 0;
 if(final_umlaut)primary=root_count-1;
 if(ic_root)primary=root_count>2?root_count-3:0;
 if(or_root && !adjective && (!*ending || !strcmp(ending,"s")) && root_count>1)primary=root_count-2;
 if(primary==root_count-1&&primary&&avoids_final_stress(w,root))--primary;
 /* Ordinary weak inflection retains a witnessed root accent, including
  * lexical antepenultimate stress. A one-syllable backward shift cannot
  * reconstruct it from the default heavy-rime rule. */
 if(lexical&&!adjective&&!glide&&(ending[0]=='e'||o_inflection||feminine)&&w[root-1]!='i'&&
    !or_root&&lexical_primary<root_count)
  primary=lexical_primary;
 if(!strcmp(ending,"er")&&tail(w,root,"ik")&&primary==root_count-1&&primary) --primary;
 if(primary==SIZE_MAX||primary>=count)return 0;
 /* An unstressed lexical A does not retain the underlying a quantity.
  * If stress moves onto it, a root-level vowel observation is required. */
 if(lexical&&primary!=lexical_primary&&!strcmp(p.p[nuclei[primary]].s,"A"))return 0;
 /* A pretonic secondary accent keeps its quantity when the primary
  * long nucleus has at least two coda phones. Weak-ending syllabification
  * has already moved its onset out of that coda. */
 size_t primary_end=primary+1<count?starts[primary+1]:p.n;
 int protected_secondary=ier_root && p.p[nuclei[primary]].long_vowel &&
     primary_end>nuclei[primary]+2;
 const size_t first_at=p.p[nuclei[0]].at;
 const int missing_oi_accent=root_count==1 && count>1 && *ending &&
     root_n>nuclei[0]+1 && first_at!=SIZE_MAX && first_at+1<root &&
     w[first_at]=='o' && w[first_at+1]=='i' && !strcmp(p.p[nuclei[0]].s,"'oj'");
 char result[1024];size_t used=2;memcpy(result,"`[",2);size_t syll=0;
 for(size_t i=0;i<p.n;++i) {
  if(i==starts[syll]) {
   unsigned stress=syll==primary&&!missing_oi_accent?1:syll==0&&primary>=2?2:0;
   if(feminine&&lexical&&!or_root&&syll<root_count)stress=p.accents[nuclei[syll]];
   result[used++]='.';result[used++]=(char)('0'+stress);
  }
  const char *s=p.p[i].s;
  /* Before a gn onset, an unaccented, unmarked vowel keeps the short
   * diaphone. This decision follows stress assignment: stressed regn-
   * and explicit vowel-length spellings retain their own quantity. */
  if(p.p[i].vowel&&syll!=primary&&i+2<p.n&&!strcmp(p.p[i+1].s,"g")&&
     !strcmp(p.p[i+2].s,"n")&&p.p[i].at!=SIZE_MAX&&
     p.p[i].at+2<root&&w[p.p[i].at+1]=='g'&&w[p.p[i].at+2]=='n')
   s=vowel_name(w[p.p[i].at],0);
  if(!(feminine&&lexical) && p.p[i].vowel&&i==nuclei[syll]&&syll!=primary&&!strcmp(s,"a")&&
     !(syll==0&&primary>=2&&protected_secondary))s="A";
  if(!strcmp(s,"@")&&i+1<p.n&&!strcmp(p.p[i+1].s,"r")&&
      (i+2==p.n||!p.p[i+2].vowel)) {s="R";++i;}
  const int retained_coda=*ending && i+2==root_n && i+1<p.n &&
      (!strcmp(p.p[i+1].s,"l") || !strcmp(p.p[i+1].s,"n"));
  const int voiceless_right=i+1<p.n && strlen(p.p[i+1].s)==1 &&
      strchr("ptkfSsTxX",p.p[i+1].s[0]);
  if(!p.p[i].vowel&&!retained_coda&&(voiceless_right || i+1==p.n ||
      (syll+1<count&&i+1==starts[syll+1]))) {
   if(!strcmp(s,"b"))s="p";else if(!strcmp(s,"d"))s="t";else if(!strcmp(s,"g"))s="k";
   else if(!strcmp(s,"v"))s="f";else if(!strcmp(s,"J"))s="C";
  }
  size_t n=strlen(s);if(used+n+2>=sizeof result)return 0;memcpy(result+used,s,n);used+=n;
  if(syll+1<count&&i+1==starts[syll+1])++syll;
 }
 result[used++]=']';result[used]=0;
 *written=used;if(!out||capacity<=used)return -2;memcpy(out,result,used+1);return 1;
}

int reloq_german_root_plan_word(const struct reloq_lexicon *lexicon,const unsigned char *word,size_t length,
 unsigned char *out,size_t capacity,size_t *written)
{
 int result=root_plan(lexicon,word,length,out,capacity,written,0,1);
 return result || !word || (!tail(word,length,"et")&&!tail(word,length,"est"))?result:
     root_plan(lexicon,word,length,out,capacity,written,0,0);
}

int reloq_german_unclassified_root_word(const struct reloq_lexicon *lexicon,const unsigned char *word,size_t length,
 unsigned char *out,size_t capacity,size_t *written)
{
 int result=root_plan(lexicon,word,length,out,capacity,written,1,1);
 return result || !word || (!tail(word,length,"et")&&!tail(word,length,"est"))?result:
     root_plan(lexicon,word,length,out,capacity,written,1,0);
}
