#include "reloq_german_morphology.h"
#include "reloq_german_lexicon.h"
#include "reloq_german_cvc_lexicon.h"
#include "reloq_german_root_plan.h"
#include "reloq_german_g2p.h"
#include "reloq_german_spr.h"
#include "reloq_german_word_class.h"
#include "reloq_german_generic_prosody.h"
#include <stdint.h>
#include <string.h>

#define WORD_LIMIT 128u
#define SPR_LIMIT 1024u

enum { ROOT_LEFT=1, ROOT_HEAD=2, ROOT_WEAK_HEAD=4, ROOT_PREFIX=8,
       ROOT_LINK_S=16, ROOT_PARTICLE=32 };
struct root_role { const char *word; uint64_t pronunciation; unsigned roles; };
/* Independent public-output measurements, not original dictionary actions. */
static const struct root_role roots[] = {
#include "reloq_german_root_roles.inc"
};
static const struct root_role inflections[] = {
#include "reloq_german_inflection_roles.inc"
};
static const struct root_role er_derivations[] = {
#include "reloq_german_er_derivation_roles.inc"
};
static const struct root_role ung_derivations[] = {
#include "reloq_german_ung_roles.inc"
};
static const struct root_role el_contractions[] = {
#include "reloq_german_el_roles.inc"
};
static const struct root_role en_forms[] = {
#include "reloq_german_en_roles.inc"
};
/* A contextual stem is observed with an inflection attached. Its canonical
 * -en reading can differ from the isolated spelling's dictionary reading.
 * One shared transformation handles all admitted endings. */
struct contextual_stem {
    const char *word, *reading;
    uint64_t isolated;
    unsigned endings, weak_prefixes;
};
static const struct contextual_stem contextual_stems[]={
#include "reloq_german_contextual_stems.inc"
};
static const struct root_role en_heads[] = {
#include "reloq_german_en_head_roles.inc"
};
static const struct root_role weak_endings[] = {
#include "reloq_german_weak_ending_roles.inc"
};
struct weak_verb_role {
    const char *word;
    uint64_t pronunciation;
    unsigned endings[6];
};
static const struct weak_verb_role weak_verbs[] = {
#include "reloq_german_weak_verb_roles.inc"
};

static unsigned char lower(unsigned char c)
{
    if (c>='A' && c<='Z') return (unsigned char)(c+32);
    if (c==0xc4 || c==0xd6 || c==0xdc) return (unsigned char)(c+32);
    return c;
}
static int letter(unsigned char c)
{
    c=lower(c);
    return (c>='a' && c<='z') || c==0xe4 || c==0xf6 || c==0xfc || c==0xdf;
}
static int vowel(unsigned char c)
{
    return c=='a' || c=='e' || c=='i' || c=='o' || c=='u' || c=='y' ||
        c==0xe4 || c==0xf6 || c==0xfc;
}
int reloq_german_morphology_prefix_onset(const unsigned char *word,size_t length,size_t at)
{
    size_t end=at;while(end<length&&!vowel(word[end]))++end;
    size_t n=end-at;
    if(end==length)return 0;
    if(n<=1)return 1;
    if(n==2)return (word[at+1]=='r'&&strchr("bptdgkf",word[at])) ||
        (word[at+1]=='l'&&strchr("bpgkf",word[at])) ||
        (word[at]=='s'&&strchr("pt",word[at+1])) ||
        !memcmp(word+at,"ch",2)||!memcmp(word+at,"ph",2)||
        !memcmp(word+at,"th",2)||!memcmp(word+at,"kn",2);
    return n==3&&(!memcmp(word+at,"sch",3)||!memcmp(word+at,"str",3)||
        !memcmp(word+at,"spr",3)||!memcmp(word+at,"spl",3));
}
#define weak_prefix_onset reloq_german_morphology_prefix_onset
static uint64_t fingerprint(const unsigned char *s, size_t n)
{
    uint64_t h=UINT64_C(14695981039346656037);
    for (size_t i=0;i<n;++i) h=(h^s[i])*UINT64_C(1099511628211);
    return h;
}
static unsigned roles_for(const struct root_role *table,size_t count,
    const struct reloq_lexicon *lexicon,
    const unsigned char *word, size_t length,
    const struct reloq_lexicon_override **entry)
{
    if (length<3 || length>24) return 0;
    unsigned char key[25];
    memcpy(key,word,length);key[length]=0;
    size_t low=0, high=count;
    while (low<high) {
        size_t at=low+(high-low)/2;
        int order=strcmp((const char *)key,table[at].word);
        if (order<0) high=at;
        else if (order>0) low=at+1;
        else {
            int found=reloq_lexicon_find_override(lexicon,key,length,entry);
            if (!found) {
                if (key[0]>='a' && key[0]<='z') key[0]-=32;
                else if (key[0]==0xe4 || key[0]==0xf6 || key[0]==0xfc) key[0]-=32;
                found=reloq_lexicon_find_override(lexicon,key,length,entry);
            }
            if (found!=1 || fingerprint((*entry)->spr,(*entry)->spr_length)!=
                table[at].pronunciation) return 0;
            return table[at].roles;
        }
    }
    return 0;
}
static unsigned component(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,const struct reloq_lexicon_override **entry)
{
    return roles_for(roots,sizeof roots/sizeof roots[0],lexicon,word,length,entry);
}

/* Keep an existing root boundary ahead of right-to-left onset expansion.
 * A recognized left root also bounds an unresolved head. Only lexical
 * component roles are used; no synthesized phone string is
 * installed as a dictionary reading and the enclosing result stays estimated. */
static unsigned spelling_roles(const unsigned char *word,size_t n)
{
    size_t lo=0,hi=sizeof roots/sizeof *roots;
    while (lo<hi) {
        size_t mid=lo+(hi-lo)/2,k=strlen(roots[mid].word),common=k<n?k:n;
        int order=memcmp(word,roots[mid].word,common);
        if (!order) order=n<k?-1:n>k?1:0;
        if (order<0) hi=mid;
        else if (order>0) lo=mid+1;
        else return roots[mid].roles;
    }
    return 0;
}
int reloq_german_morphology_is_root_boundary(const unsigned char *word,
    size_t n,size_t split)
{
    if (!word || split<3 || split+3>n) return 0;
    return !!(spelling_roles(word,split)&ROOT_LEFT);
}

unsigned reloq_german_morphology_constituent_class(const unsigned char *w,size_t n)
{
    unsigned char key[WORD_LIMIT];
    if(!w || n>=sizeof key)return 0;
    for(size_t i=0;i<n;++i)key[i]=lower(w[i]);
    w=key;
    static const struct {const char *word;unsigned roles;} classes[]={
#include "reloq_german_compound_classes.inc"
    };
    size_t lo=0,hi=sizeof classes/sizeof *classes;
    while (lo<hi) {
        size_t mid=lo+(hi-lo)/2,k=strlen(classes[mid].word),m=n<k?n:k;
        int cmp=memcmp(w,classes[mid].word,m);
        if (!cmp) cmp=n<k?-1:n>k?1:0;
        if (cmp<0) hi=mid;else if (cmp>0) lo=mid+1;
        else return classes[mid].roles;
    }
    return 0;
}
#define compound_class reloq_german_morphology_constituent_class

static int constituent_reading(const struct reloq_lexicon *,const unsigned char *,
 size_t,unsigned char *,size_t *,int);

static const struct contextual_stem *contextual_stem(
    const struct reloq_lexicon *lexicon,unsigned char *key,size_t length,
    unsigned ending,unsigned scope)
{
    key[length]=0;
    size_t lo=0,hi=sizeof contextual_stems/sizeof *contextual_stems;
    while (lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        const struct contextual_stem *s=contextual_stems+mid;
        int cmp=strcmp((const char *)key,s->word);
        if (cmp<0) hi=mid;
        else if (cmp>0) lo=mid+1;
        else {
            if (!(s->endings&(1u<<ending)) ||
                (scope && !(s->weak_prefixes&(1u<<(scope-1))))) return NULL;
            const struct reloq_lexicon_override *entry=NULL;
            int found=reloq_lexicon_find_override(lexicon,key,length,&entry);
            if (!found) {
                unsigned char initial=key[0];
                if (initial>='a' && initial<='z') key[0]-=32;
                else if (initial==0xe4 || initial==0xf6 || initial==0xfc) key[0]-=32;
                found=reloq_lexicon_find_override(lexicon,key,length,&entry);
                key[0]=initial;
            }
            if (s->isolated ? found!=1 || fingerprint(entry->spr,entry->spr_length)!=s->isolated : found!=0)
                return NULL;
            return s;
        }
    }
    return NULL;
}

/* strip_verb_adj_infl separates the suffix before root pronunciation.
 * Reuse the independently observed infinitive's full reading, including
 * silent h, stress, consonant choice and vowel quantity. Each ending class
 * is witnessed separately; an orthographic -en alone does not qualify.
 * This describes transformations, not original dictionary bytecode. */
static int en_form(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written,
    unsigned scope)
{
    static const struct {const char *letters,*tail;unsigned kind;} ends[]={
        {"e", "]",0},{"en",NULL,4},{"est","st]",0},{"et","t]",0},
        {"st","st]",1},{"t","t]",1},{"te",".0t@]",1},
        {"ten",".0t@n]",1},{"test",".0t@st]",1},{"tet",".0t@t]",1},
        {"tes",".0t@s]",1},{"ter",".0tR]",1},{"tem",".0t@m]",1},
        {"end","t]",2},{"ende",".0d@]",2},{"enden",".0d@n]",2},
        {"endem",".0d@m]",2},{"ender",".0dR]",2},{"endes",".0d@s]",2},
        {"ung","UG]",3},{"ungen","UG.0@n]",3},{"er","R]",3},{"ern","Rn]",3}
    };
    for (size_t e=0;e<sizeof ends/sizeof *ends;++e) {
        size_t tail=strlen(ends[e].letters);
        if (length<tail+2 || memcmp(word+length-tail,ends[e].letters,tail)) continue;
        size_t stem=length-tail;
        if (stem+2>=WORD_LIMIT) continue;
        unsigned char key[WORD_LIMIT];memcpy(key,word,stem);memcpy(key+stem,"en",2);
        const struct reloq_lexicon_override *entry=NULL;
        unsigned mask=roles_for(en_forms,sizeof en_forms/sizeof *en_forms,
            lexicon,key,stem+2,&entry);
        struct reloq_lexicon_override contextual={0};
        int contextual_form=0;
        if (!(mask&(1u<<e))) {
            const struct contextual_stem *s=contextual_stem(lexicon,key,stem,(unsigned)e,scope);
            if (!s) continue;
            contextual.spr=(const unsigned char *)s->reading;
            contextual.spr_length=strlen(s->reading);entry=&contextual;contextual_form=1;
        }
        if (entry->spr_length<5 ||
            entry->spr_length+12>=SPR_LIMIT ||
            memcmp(entry->spr+entry->spr_length-3,"@n]",3)) continue;
        size_t used=entry->spr_length;
        memcpy(out,entry->spr,used);
        if (ends[e].kind==4) {out[used]=0;*written=used;return contextual_form?2:1;}
        used-=ends[e].kind==0?2:ends[e].kind==2?1:3;
        if (ends[e].kind==1) {
            size_t at=used;
            while (at && out[at-1]!='.') --at;
            if (at && at+1<=used) {
                int nucleus=0;
                for (size_t j=at+1;j<used;++j)
                    if (strchr("aAeEiIoOuUyY@R'",out[j])) nucleus=1;
                if (!nucleus) {memmove(out+at-1,out+at+1,used-at-1);used-=2;}
            }
            if (contextual_form && used>1 && out[used-2]=='I' && out[used-1]=='g') out[used-1]='X';
            else if (used && out[used-1]=='b') out[used-1]='p';
            else if (used && out[used-1]=='d') out[used-1]='t';
            else if (used && out[used-1]=='g') out[used-1]='k';
            else if (used && out[used-1]=='z') out[used-1]='s';
        }
        const char *ending=ends[e].tail;
        char adjusted[16];
        if (contextual_form && ends[e].kind==1 && used) {
            /* Adjacent dental closures collapse. Before a weak vowel,
             * s/st and sch/st share the following syllable's onset. */
            if (e==4 && out[used-1]=='s') ending="t]";
            else if (e!=4 && out[used-1]=='t') --used;
            else if (e!=4 && e!=5 && (out[used-1]=='s' || out[used-1]=='S')) {
                adjusted[0]='.';adjusted[1]='0';adjusted[2]=(char)out[--used];
                strcpy(adjusted+3,ending+2);ending=adjusted;
            }
        }
        size_t n=strlen(ending);memcpy(out+used,ending,n+1);
        *written=used+n;return contextual_form?2:1;
    }
    return 0;
}

/* Derivation is resolved before the enclosing compound/particle boundary.
 * A homographic standalone dictionary stem must not replace the recovered
 * verb's reading. The six bits select separately witnessed boundary classes. */
static int en_participle_head(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned scope,
    unsigned char *out,size_t *written)
{
    static const char *const endings[]={"endem","enden","ender","endes","ende","end"};
    for (size_t i=0;i<sizeof endings/sizeof *endings;++i) {
        size_t n=strlen(endings[i]);
        if (length<n+2 || memcmp(word+length-n,endings[i],n)) continue;
        size_t base=length-n;if (base+2>=WORD_LIMIT) continue;
        unsigned char key[WORD_LIMIT];memcpy(key,word,base);memcpy(key+base,"en",2);
        const struct reloq_lexicon_override *entry=NULL;
        if (!(roles_for(en_heads,sizeof en_heads/sizeof *en_heads,
            lexicon,key,base+2,&entry)&scope)) continue;
        return en_form(lexicon,word,length,out,written,0);
    }
    return 0;
}
static unsigned en_boundary_scope(const unsigned char *left,size_t n)
{
    static const char *const particles[]={"aus","ein","auf","ab","an"};
    for (size_t i=0;i<sizeof particles/sizeof *particles;++i)
        if (strlen(particles[i])==n && !memcmp(left,particles[i],n)) return 1u<<i;
    return 32u;
}

/* Do not bypass a measured negative boundary decision by trying the same
 * participle again as an unconstrained initial-root compound. */
static int blocked_participle_head(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t length,unsigned scope)
{
 static const char *const endings[]={"end","ende","enden","endem","ender","endes"};
 for(size_t i=0;i<sizeof endings/sizeof *endings;++i) {
  size_t en=strlen(endings[i]);
  if(length<=en+2 || memcmp(word+length-en,endings[i],en))continue;
  unsigned char key[WORD_LIMIT];size_t root=length-en;
  memcpy(key,word,root);memcpy(key+root,"en",3);
  const struct reloq_lexicon_override *entry=NULL;
  unsigned roles=roles_for(en_heads,sizeof en_heads/sizeof *en_heads,
      lexicon,key,root+2,&entry);
  return roles && !(roles&scope);
 }
 return 0;
}

/* Covered -ig/-lich/-isch stems retain their complete reading under inflection and
 * -keit derivation. An inflection moves the final consonant into the next
 * weak syllable (-ig voices it to /g/); -keit leaves it in the stem.
 * An accented final syllable (frisch, Tisch and compounds) retains its
 * quantity and accent as well; it is not reinterpreted as an unstressed
 * adjectival suffix. The lexical reading, not spelling alone, selects this path.
 * Evidence: strip_verb_adj_infl, apply_ger_i_rules/g_rules and complete
 * inflection/derivation readings of 200 weak-ending lexical stems. */
static const struct adjective_ending {const char *letters,*phones;unsigned move;} adjective_endings[]={
    {"keiten",".0k'aj'.0t@n",0},{"keit",".0k'aj't",0},
    {"en","@n",1},{"em","@m",1},{"er","R",1},{"es","@s",1},{"e","@",1}
};
static int adjective_ending(const unsigned char *word,size_t length,size_t *base)
{
    for (size_t i=0;i<sizeof adjective_endings/sizeof adjective_endings[0];++i) {
        size_t n=strlen(adjective_endings[i].letters);
        if (length<n+3 || memcmp(word+length-n,adjective_endings[i].letters,n)) continue;
        size_t b=length-n;
        if ((b>3 && !memcmp(word+b-2,"ig",2)) ||
            (b>4 && (!memcmp(word+b-4,"lich",4) || !memcmp(word+b-4,"isch",4)))) {
            *base=b;return (int)i;
        }
    }
    return -1;
}
size_t reloq_german_adjective_base(const unsigned char *word,size_t length)
{
    size_t base=0;
    if (!word || adjective_ending(word,length,&base)<0) return 0;
    return base;
}
int reloq_german_adjective_form(const unsigned char *word,size_t length,
    const unsigned char *spr,size_t size,unsigned char *out,size_t capacity,size_t *written)
{
    if (!word || !spr || !written || (!out && capacity)) return -1;
    size_t base=0;int ending=adjective_ending(word,length,&base);
    if (ending<0 || size<7 || spr[size-1]!=']' || spr[size-3]!='I' ||
        (spr[size-2]!='X' && spr[size-2]!='S')) return 0;
    size_t syllable=size-3;
    while (syllable && spr[syllable]!='.') --syllable;
    if (!syllable) return 0;
    const struct adjective_ending *e=adjective_endings+ending;
    const unsigned char final_phone=spr[size-2];
    size_t stem=size-(e->move?2:1),tail=strlen(e->phones);
    size_t needed=stem+tail+(e->move?3:0)+1;
    if (!out || capacity<=needed) {*written=needed;return -2;}
    memmove(out,spr,stem);size_t used=stem;
    if (e->move) {
        out[used++]='.';out[used++]='0';
        out[used++]=!memcmp(word+base-2,"ig",2)?'g':final_phone;
    }
    memcpy(out+used,e->phones,tail);used+=tail;
    out[used++]=']';out[used]=0;*written=used;return 1;
}
/* A negative adjective retains a lexical root with one accent, including
 * non-initial stress and vowel quantity. Roots with several accents need
 * the prefix/compound boundary map: the rightmost-accent shortcut loses
 * legitimate secondary accents (e.g. unwahrscheinlich), so is not used.
 * Whole-word dictionary exceptions retain priority in the caller. */
static int negative_adjective(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    if (length<7 || memcmp(word,"un",2) || !memcmp(word+2,"un",2) ||
        !((length>5 && !memcmp(word+length-2,"ig",2)) ||
          !memcmp(word+length-4,"lich",4) || !memcmp(word+length-4,"isch",4))) return 0;
    const struct reloq_lexicon_override *entry=NULL;
    if (reloq_lexicon_find_override(lexicon,word+2,length-2,&entry)!=1 ||
        entry->spr_length<7 || entry->spr_length+5>=SPR_LIMIT) return 0;
    size_t n=entry->spr_length,last=0,accents=0;
    if (memcmp(entry->spr+n-3,"IX]",3) && memcmp(entry->spr+n-3,"IS]",3)) return 0;
    for (size_t i=2;i+1<n;++i)
        if (entry->spr[i]=='.' && (entry->spr[i+1]=='1' || entry->spr[i+1]=='2')) {
            last=i+1;++accents;
        }
    if (accents!=1) return 0;
    memcpy(out,"`[.1Un",6);memcpy(out+6,entry->spr+2,n-2);
    for (size_t i=6;i+1<n+4;++i)
        if (out[i]=='.' && (out[i+1]=='1' || out[i+1]=='2')) out[i+1]=i+1==last+4?'2':'0';
    out[n+4]=0;*written=n+4;return 1;
}
static int inflect_weak_adjective(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    size_t base=reloq_german_adjective_base(word,length);
    if (base) {
        unsigned char key[WORD_LIMIT];memcpy(key,word,base);
        const struct reloq_lexicon_override *entry=NULL;
        int found=reloq_lexicon_find_override(lexicon,key,base,&entry);
        if (!found) {
            if (key[0]>='a' && key[0]<='z') key[0]-=32;
            else if (key[0]==0xe4 || key[0]==0xf6 || key[0]==0xfc) key[0]-=32;
            found=reloq_lexicon_find_override(lexicon,key,base,&entry);
        }
        if (found==1) return reloq_german_adjective_form(word,length,
            entry->spr,entry->spr_length,out,SPR_LIMIT,written)==1;
        unsigned char negative[SPR_LIMIT];size_t n=0;
        if (negative_adjective(lexicon,word,base,negative,&n))
            return reloq_german_adjective_form(word,length,negative,n,out,SPR_LIMIT,written)==1;
    }
    return 0;
}

/* Root retry precedes fallback letter analysis. The whole lexical reading
 * supplies vowel quantity and stress; only the observed ending boundary is
 * re-syllabified. Role fingerprints isolate caller pronunciation overrides. */
static int inflect(const struct reloq_lexicon *lexicon,const unsigned char *word,
    size_t length,unsigned char *out,size_t *written,unsigned *head_roles)
{
    static const struct {const char *ending,*spr;unsigned role;} endings[]={
        {"n","n",4},{"s","s",2},
        {"en","@n",1},{"em","@m",1},{"er","R",1},{"es","@s",1},{"e","@",1}
    };
    for (size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        const size_t n=strlen(endings[i].ending);
        if (length<n+3 || memcmp(word+length-n,endings[i].ending,n)) continue;
        const size_t base=length-n;
        /* -iker and -iren compete with a different analysis/stress path.
         * e/em witnesses do not establish these endings: do not let a
         * weak-ending class erase the original productive alternative. */
        if (endings[i].role==1 && base>=2 &&
            ((!strcmp(endings[i].ending,"er") && !memcmp(word+base-2,"ik",2)) ||
             (!strcmp(endings[i].ending,"en") && !memcmp(word+base-2,"ir",2)))) continue;
        const struct reloq_lexicon_override *entry=NULL;
        unsigned roles=roles_for(inflections,sizeof inflections/sizeof inflections[0],
            lexicon,word,base,&entry);
        unsigned precise=roles_for(weak_endings,sizeof weak_endings/sizeof *weak_endings,
            lexicon,word,base,&entry);
        if (precise&(1u<<i)) roles|=endings[i].role;
        if (!(roles&endings[i].role) || entry->spr_length+8>=SPR_LIMIT) continue;
        size_t used=entry->spr_length-1;
        memcpy(out,entry->spr,used);
        unsigned char onset=0,cluster_onset=0;
        if (endings[i].role==1) {
            const unsigned char last=out[used-1];
            if ((precise&128u) && used>=2 && out[used-2]=='s' && last=='t') {
                used-=2;cluster_onset='s';onset='t';
            } else if (last=='R') {out[used-1]='@';onset='r';}
            else if (strchr("pbtdkgfvszSZxXhlrjwmnPTCJ",last)) {
                onset=last;--used;
                unsigned char final=word[base-1];
                if (final=='b' || final=='d' || final=='v' ||
                    (final=='g' && word[base-2]!='n')) onset=final;
                else if (final=='s' && word[base-2]!='s') onset='z';
            }
            out[used++]='.';out[used++]='0';
            if (cluster_onset) out[used++]=cluster_onset;
            if (onset) out[used++]=onset;
        }
        size_t tail=strlen(endings[i].spr);
        memcpy(out+used,endings[i].spr,tail);used+=tail;
        out[used++]=']';out[used]=0;
        if (!reloq_german_lexicon_validate_spr(out,used,NULL)) continue;
        *written=used;
        if (head_roles) *head_roles=component(lexicon,word,base,&entry);
        return 1;
    }
    return 0;
}

/* Retain the verb stem's lexical syllables, vowel quantity and stress in
 * nominal -ung. Public singular/plural observations determine eligibility;
 * a mere written -en is insufficient (e.g. proper names and plural nouns).
 * Fingerprints prevent applying this class to an altered caller reading. */
static int derive_en_ung(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    size_t suffix=length>=6 && !memcmp(word+length-5,"ungen",5)?5:
        length>=4 && !memcmp(word+length-3,"ung",3)?3:0;
    if (!suffix || length-suffix+2>=WORD_LIMIT) return 0;
    unsigned char verb[WORD_LIMIT];size_t base=length-suffix;
    memcpy(verb,word,base);memcpy(verb+base,"en",2);
    const struct reloq_lexicon_override *entry=NULL;
    if (!roles_for(ung_derivations,sizeof ung_derivations/sizeof ung_derivations[0],
        lexicon,verb,base+2,&entry) || entry->spr_length<5 ||
        entry->spr_length+5>=SPR_LIMIT ||
        memcmp(entry->spr+entry->spr_length-3,"@n]",3)) return 0;
    size_t used=entry->spr_length-3;
    const char *tail=suffix==5?"UG.0@n]":"UG]";size_t n=strlen(tail);
    memcpy(out,entry->spr,used);memcpy(out+used,tail,n+1);
    *written=used+n;return 1;
}

/* Contracted -el retains the complete lexical syllable before /l/.
 * Re-estimating it from the contracted spelling loses the ng/k boundary
 * in dunkel -> dunklen. Only complete observed paradigms admit this path. */
static int inflect_contracted_el(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const struct {const char *letters,*phones;} endings[]={
        {"en","@n"},{"em","@m"},{"er","R"},{"es","@s"},{"e","@"}
    };
    for(size_t i=0;i<sizeof endings/sizeof endings[0];++i) {
        size_t n=strlen(endings[i].letters);
        if(length<n+3 || memcmp(word+length-n,endings[i].letters,n)) continue;
        size_t base=length-n;
        if(word[base-1]!='l' || base+1>=WORD_LIMIT) continue;
        unsigned char root[WORD_LIMIT];memcpy(root,word,base-1);
        memcpy(root+base-1,"el",2);
        const struct reloq_lexicon_override *entry=NULL;
        if(!roles_for(el_contractions,sizeof el_contractions/sizeof el_contractions[0],
            lexicon,root,base+1,&entry) || entry->spr_length<5 ||
            entry->spr_length+4>=SPR_LIMIT ||
            memcmp(entry->spr+entry->spr_length-3,"@l]",3)) continue;
        size_t used=entry->spr_length-3;memcpy(out,entry->spr,used);
        out[used++]='l';n=strlen(endings[i].phones);
        memcpy(out+used,endings[i].phones,n);used+=n;
        out[used++]=']';out[used]=0;*written=used;return 1;
    }
    return 0;
}

static int join(const unsigned char *left, size_t left_length,
    const unsigned char *right, size_t right_length, int stress,
    const char *link, unsigned char *result, size_t *length)
{
    size_t link_length=strlen(link);
    if (left_length<4 || right_length<4 ||
        left_length+right_length+link_length>=SPR_LIMIT) return 0;
    size_t n=left_length-1;
    memcpy(result,left,n);memcpy(result+n,link,link_length);n+=link_length;
    size_t start=n;
    memcpy(result+n,right+2,right_length-2);n+=right_length-2;
    /* Extending a resolved compound weakens the first of two adjacent
     * subsidiary accents on its left. An intervening weak syllable or
     * primary accent keeps the domains distinct. */
    if(stress==2) {
        size_t previous=0;
        for(size_t i=2;i+1<left_length-1;++i)if(result[i]=='.') {
            if(previous && result[previous+1]=='2' && result[i+1]=='2')
                result[previous+1]='0';
            previous=i;
        }
    }
    for (size_t i=start;i+1<n;++i)
        if (result[i]=='.' && result[i+1]=='1') result[i+1]=(unsigned char)('0'+stress);
    result[n]=0;
    if (!reloq_german_lexicon_validate_spr(result,n,NULL)) return 0;
    *length=n;return 1;
}
static int select_candidate(unsigned char *selected,size_t *selected_length,
    const unsigned char *candidate,size_t n)
{
    if (*selected_length)
        return *selected_length==n && !memcmp(selected,candidate,n);
    memcpy(selected,candidate,n+1);*selected_length=n;return 1;
}

/* strip_un precedes the inner inseparable-prefix analysis. An already
 * resolved weak prefix retains its vowel and root stress under un-, also
 * when the remaining participle has no complete dictionary entry. */
static int negative_weak_prefix(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    if(length<7 || memcmp(word,"un",2))return 0;
    static const struct {const char *word,*spr;} weak[]={
        {"be","`[.0b@"},{"ge","`[.0g@"},{"ver","`[.0fEr"},
        {"er","`[.0Er"},{"ent","`[.0Ent"},{"zer","`[.0TEr"}};
    for(size_t i=0;i<sizeof weak/sizeof *weak;++i) {
        size_t wn=strlen(weak[i].word),sn=strlen(weak[i].spr);
        if(length<=wn+4 || memcmp(word+2,weak[i].word,wn))continue;
        unsigned char head[SPR_LIMIT];size_t hn=0;
        if(reloq_german_lexicon_word(lexicon,word+2,length-2,head,sizeof head,&hn)!=1 ||
           hn<=sn || memcmp(head,weak[i].spr,sn) || head[sn]!='.')continue;
        return join((const unsigned char *)"`[.1Un]",7,head,hn,2,"",out,written);
    }
    return 0;
}

/* Prefix and root are separate stress domains. Resolve the complete head
 * before attaching the prefix; otherwise medial sp/st and short root
 * vowels are interpreted as letters inside one unstructured word. */
static int structured_prefix(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    size_t lead=0;const char *prefix=NULL;int accent=1,repeat=0;
    if(length>7&&!memcmp(word,"unter",5)) {lead=5;prefix="`[.0Un.0tR]";}
    else if(length>6&&!memcmp(word,"\374ber",4)) {lead=4;prefix="`[.0y.0bR]";}
    else if(length>8&&!memcmp(word,"wieder",6)) {lead=6;prefix="`[.1vi.0dR]";accent=2;repeat=1;}
    else if(length>6&&!memcmp(word,"non",3)&&!vowel(word[3])&&word[3]!='n'&&
        !(word[3]=='s'&&(word[4]=='p'||word[4]=='t'))) {
        lead=3;prefix="`[.1non]";accent=2;
    }
    if(!lead)return 0;
    unsigned char head[SPR_LIMIT];size_t n=0;
    if(repeat && length>lead+5 && !memcmp(word+lead,"zu",2) &&
       reloq_german_observed_verb(word+lead+2,length-lead-2) &&
       reloq_german_lexicon_word(lexicon,word+lead+2,length-lead-2,
           head,sizeof head,&n)==1 && n>4) {
        const int weak=head[3]=='0';
        const char *outer=weak?"`[.0vi.0dR]":"`[.1vi.0dR]";
        return join((const unsigned char *)outer,strlen(outer),head,n,
            weak?1:2,".0Tu",out,written);
    }
    if(reloq_german_lexicon_word(lexicon,word+lead,length-lead,head,sizeof head,&n)!=1&&
       reloq_german_root_plan_word(lexicon,word+lead,length-lead,head,sizeof head,&n)!=1) {
        int approximate=0;
        if((lead!=5 && lead!=4) || reloq_german_g2p_word_best_effort(word+lead,length-lead,
            head,sizeof head,&n,&approximate)!=1)return 0;
    }
    if(lead==5&&n>4&&head[3]=='0') {prefix="`[.1Un.0tR]";accent=2;}
    if(lead==4&&n>4&&head[3]=='0') {prefix="`[.1y.0bR]";accent=2;}
    if(repeat) {
        /* strip_variable_particle yields to an inner stressed particle;
         * that particle keeps its own head accent and boundary. A plain
         * compound's subsidiary accent does not establish this priority. */
        static const struct {const char *word,*spr;} inner[]={
            {"her","`[.1hEr"},{"hin","`[.1hIn"},{"auf","`[.1'aw'f"},
            {"aus","`[.1'aw's"},{"ein","`[.1'aj'n"},{"an","`[.1An"},
            {"ab","`[.1Ap"},{"vor","`[.1for"},{"nach","`[.1nax"},
            {"zu","`[.1Tu"},{"weg","`[.1vEk"},{"mit","`[.1mIt"},
            {"bei","`[.1b'aj'"}};
        int inner_particle=0;
        for(size_t i=0;i<sizeof inner/sizeof *inner;++i) {
            size_t wn=strlen(inner[i].word),sn=strlen(inner[i].spr);
            if(length-lead>=wn && !memcmp(word+lead,inner[i].word,wn) &&
               n>sn && !memcmp(head,inner[i].spr,sn) &&
               (head[sn]=='.'||head[sn]==']'))inner_particle=1;
        }
        /* A licensed nominal -ung is outside the repeated verb's accent
         * domain. Reuse the derivation recognizer, not just its letters. */
        unsigned char derived[SPR_LIMIT];size_t dn=0;
        if(inner_particle || derive_en_ung(lexicon,word+lead,length-lead,derived,&dn)) {
            prefix="`[.0vi.0dR]";accent=1;
        }
    }
    return join((const unsigned char *)prefix,strlen(prefix),head,n,accent,"",out,written);
}

/* The reference has a separate short initial-constituent class inside
 * prefix/particle analysis (initial_compound_word, table 175). It is not a
 * licence to split the same letters in an unprefixed word. */
static const struct { const char *word,*spr; } prefixes[] = {
        {"weiter","`[.1v'aj'.0tR]"}, {"kennen","`[.1kE.0n@n]"},
        {"fort","`[.1fOrt]"},
        {"gegen\374ber","`[.0ge.0g@n.1y.0bR]"},
        {"herunter","`[.0hE.1rUn.0tR]"},
        {"hin\374ber","`[.0hI.1ny.0bR]"},
        {"hervor","`[.0hEr.1for]"},
        {"heraus","`[.0hE.1r'aw's]"},
        {"herein","`[.0hE.1r'aj'n]"},
        {"herauf","`[.0hE.1r'aw'f]"},
        {"hinauf","`[.0hI.1n'aw'f]"},
        {"hinein","`[.0hI.1n'aj'n]"},
        {"hinaus","`[.0hI.1n'aw's]"},
        {"vorbei","`[.0for.1b'aj']"},
        {"herab","`[.0hE.1rAp]"},
        {"heran","`[.0hE.1rAn]"},
        {"herum","`[.0hE.1rUm]"},
        {"hinab","`[.0hI.1nAp]"},
        {"her","`[.1hEr]"},
        {"hin","`[.1hIn]"},
        {"zusammen","`[.0Tu.1zA.0m@n]"}, {"zur\374ck","`[.0Tu.1rYk]"},
        {"nach","`[.1nax]"}, {"auf","`[.1'aw'f]"},
        {"aus","`[.1'aw's]"}, {"bei","`[.1b'aj']"},
        {"ein","`[.1'aj'n]"}, {"mit","`[.1mIt]"},
        {"vor","`[.1for]"}, {"weg","`[.1vEk]"},
        {"ab","`[.1Ap]"}, {"an","`[.1An]"}, {"um","`[.1Um]"}, {"zu","`[.1Tu]"}
    };

/* Explicit particle + infinitive zu + verb has one unambiguous boundary.
 * It must precede the competing parse particle + zubauen. In the SO
 * reference the particle keeps its accent, zu is weak, and the complete
 * infinitive retains a subsidiary root accent. */
static int separable_infinitive(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    if(length<7 || memcmp(word+length-2,"en",2))return 0;
    for(size_t i=0;i<=sizeof prefixes/sizeof *prefixes;++i) {
        const char *particle=i?prefixes[i-1].word:"hinzu";
        const char *spr=i?prefixes[i-1].spr:"`[.0hIn.1Tu]";
        if(!strcmp(particle,"hin"))continue;
        size_t lead=strlen(particle),pn=strlen(spr);
        if(length<lead+5 || memcmp(word,particle,lead) ||
           memcmp(word+lead,"zu",2))continue;
        unsigned char head[SPR_LIMIT];size_t hn=0;
        if(reloq_german_lexicon_word(lexicon,word+lead+2,length-lead-2,
            head,sizeof head,&hn)!=1)continue;
        if(join((const unsigned char *)spr,pn,head,hn,2,
            ".0Tu",out,written))return 1;
    }
    return 0;
}

/* The outer particle precedes the complete infinitive domain. Resolving
 * its head independently preserves initial st/sp, h, vowel quantity and
 * weak prefixes. A suffix reader must not treat the lexical head as a
 * weak ending of the particle (setzen -> z@tzen, lernen -> lRn).
 * Variable um-/wieder- retain their separately observed accent rules. */
static int complete_particle_infinitive88(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    if(length<6 || word[length-1]!='n' ||
       (word[length-2]!='e' && word[length-2]!='r' && word[length-2]!='l'))return 0;
    for(size_t i=0;i<sizeof prefixes/sizeof *prefixes;++i) {
        const char *particle=prefixes[i].word;
        size_t lead=strlen(particle);
        if(!strcmp(particle,"um") || !strcmp(particle,"hin") ||
           length<lead+4 || memcmp(word,particle,lead))continue;
        size_t start=lead;
        int infix=length>lead+5 && !memcmp(word+lead,"zu",2);
        if(infix)start+=2;
        if(!reloq_german_observed_verb(word+start,length-start))continue;
        if(!infix && vowel(word[start]) && !strcmp(particle,"bei"))continue;
        if(!infix && lead==2 && !memcmp(word,"aber",4))continue;
        unsigned char head[SPR_LIMIT];size_t hn=0;
        if(!constituent_reading(lexicon,word+start,length-start,head,&hn,0))continue;
        if(hn<5 || (head[3]!='0' && head[3]!='1'))continue;
        if(join((const unsigned char *)prefixes[i].spr,strlen(prefixes[i].spr),
            head,hn,2,infix?".0Tu":"",out,written))return 1;
    }
    return 0;
}

/* strip_un precedes the stressed-particle pass. The particle loses its
 * own primary accent, while the inner root keeps its secondary domain.
 * Reuse a resolved multi-domain head so its consonants and vowel quantity
 * cannot fall back to the unsegmented spelling reader. Single-domain and
 * dictionary exceptional un- analyses retain their existing priority. */
static int negative_particle(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    if(length<8 || memcmp(word,"un",2) || !memcmp(word+2,"un",2))return 0;
    unsigned char head[SPR_LIMIT];size_t n=0;
    if(reloq_german_lexicon_word(lexicon,word+2,length-2,head,sizeof head,&n)!=1 ||
       n<8 || head[3]!='1')return 0;
    int secondary=0;
    for(size_t i=2;i+1<n;++i)if(head[i]=='.' && head[i+1]=='2')secondary=1;
    if(!secondary)return 0;
    /* Only a recognized particle boundary licenses loss of its accent;
     * a multi-root noun keeps both subsidiary accents under negation. */
    int particle=0;
    for(size_t i=0;i<sizeof prefixes/sizeof *prefixes;++i) {
        size_t wn=strlen(prefixes[i].word),pn=strlen(prefixes[i].spr)-1;
        if(length>wn+4 && !memcmp(word+2,prefixes[i].word,wn) &&
           n>pn && !memcmp(head,prefixes[i].spr,pn) && head[pn]=='.')particle=1;
    }
    if(length>8 && !memcmp(word+2,"voll",4) && n>8 && !memcmp(head,"`[.1fOl.",8))particle=1;
    if(!particle)return 0;
    size_t base=reloq_german_adjective_base(word,length);
    const struct reloq_lexicon_override *entry=NULL;
    if(base && reloq_lexicon_find_override(lexicon,word,base,&entry)==1)return 0;
    head[3]='0';
    return join((const unsigned char *)"`[.1Un]",7,head,n,2,"",out,written);
}


/* After a final-root split the original retries particle stripping on
 * the left domain, before stripping its weak ending. Keep that ending's
 * nasal with its root. The old final-root fallback could instead split
 * after schwa and make the nasal an onset of the particle. */
static int weak_left_particle(const struct reloq_lexicon *lexicon,
 const unsigned char *w,size_t n,unsigned char *out,size_t *written)
{
 for(size_t split=5;split+5<=n;++split) {
  if(memcmp(w+split-2,"en",2) && memcmp(w+split-2,"er",2))continue;
  if(!(compound_class(w,split)&1u) && !(compound_class(w,split-2)&6u))continue;
  for(size_t k=0;k<sizeof prefixes/sizeof *prefixes;++k) {
   size_t z=strlen(prefixes[k].word),sp=strlen(prefixes[k].spr);
   if(!vowel((unsigned char)prefixes[k].word[0]) || n<split+z+3 ||
      memcmp(w+split,prefixes[k].word,z))continue;
   unsigned char left[SPR_LIMIT],right[SPR_LIMIT];size_t ln=0,rn=0;
   if(reloq_german_lexicon_word(lexicon,w,split,left,sizeof left,&ln)!=1 ||
      ln<5 || (memcmp(left+ln-3,"@n]",3) && left[ln-2]!='R'))continue;
   if(!constituent_reading(lexicon,w+split,n-split,right,&rn,2) ||
      rn<=sp || memcmp(right,prefixes[k].spr,sp-1))continue;
   if(join(left,ln,right,rn,2,"",out,written))return 1;
  }
 }
 return 0;
}

static int prefixed_compound(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const struct {const char *word,*spr;} weak[]={
        {"ge","`[.0g@]"},{"be","`[.0b@]"},{"ver","`[.0fEr]"},
        {"er","`[.0Er]"},{"ent","`[.0Ent]"}};
    const size_t weak_count=sizeof weak/sizeof *weak;
    for(size_t p=0;p<weak_count+sizeof prefixes/sizeof *prefixes;++p) {
        const char *letters=p<weak_count?weak[p].word:prefixes[p-weak_count].word;
        const char *spr=p<weak_count?weak[p].spr:prefixes[p-weak_count].spr;
        size_t lead=strlen(letters);
        if(length<lead+6||memcmp(word,letters,lead))continue;
        for(size_t split=3;split+lead+3<=length;++split) {
            /* After removing an outer prefix, the remaining domain can
             * start with an initial constituent as well as a bound root.
             * Requiring the latter alone lost complete compounds whose
             * head is absent from the final-root table. Resolve that inner
             * compound before the prefix supplies its stress domain. */
            unsigned initial=compound_class(word+lead,split);
            if(!(initial&9u))continue;
            if(!(initial&8u)) {
                /* A weak prefix does not itself establish a new boundary
                 * before a vowel. That path can still resyllabify the
                 * stem's final consonant in the ordinary root analysis. */
                if(p<weak_count && vowel(word[lead+split]))continue;
                /* An initial-word hit is not a bound-prefix hit. Re-enter
                 * the full shorter morphology, including suffix removal,
                 * instead of making -ung or -erst into a second root. */
                unsigned char inner[SPR_LIMIT];size_t in=0;
                if(reloq_german_morphology_word(lexicon,word+lead,length-lead,
                     inner,sizeof inner,&in)!=1)continue;
                int primary=0,compound=0;
                for(size_t j=2;j+1<in;++j)if(inner[j]=='.') {
                    if(inner[j+1]=='1')primary=1;
                    if(primary && inner[j+1]=='2')compound=1;
                }
                if(compound && join((const unsigned char *)spr,strlen(spr),
                     inner,in,p<weak_count?1:2,"",out,written))return 1;
                continue;
            }
            const struct reloq_lexicon_override *left=NULL;
            struct reloq_lexicon_override derived_left={0};
            unsigned char left_reading[SPR_LIMIT];size_t ln=0;
            if(!component(lexicon,word+lead,split,&left)) {
                if(reloq_german_lexicon_word(lexicon,word+lead,split,
                    left_reading,sizeof left_reading,&ln)!=1)continue;
                derived_left.spr=left_reading;derived_left.spr_length=ln;left=&derived_left;
            }
            unsigned char head[SPR_LIMIT],compound[SPR_LIMIT];size_t n=0,cn=0;
            const unsigned char *part=word+lead+split;size_t len=length-lead-split;
            if(len>=3&&!memcmp(part,"end",3)&&
               (len==3||(len==4&&part[3]=='e')||
                (len==5&&part[3]=='e'&&strchr("nrms",part[4]))))continue;
            if(!constituent_reading(lexicon,part,len,head,&n,2)) {
                size_t base=reloq_german_adjective_base(part,len),bn=0;
                unsigned char stem[SPR_LIMIT];
                if(!base||reloq_german_lexicon_word(lexicon,part,base,stem,sizeof stem,&bn)!=1||
                   reloq_german_adjective_form(part,len,stem,bn,head,sizeof head,&n)!=1)continue;
            }
            if(join(left->spr,left->spr_length,head,n,2,"",compound,&cn)&&
               join((const unsigned char *)spr,strlen(spr),
                compound,cn,p<weak_count?1:2,"",out,written))return 1;
        }
    }
    return 0;
}

/* The -er stem survives verbal inflection. Nominal -ung unfolds its weak
 * vocalic r to /@.r/, without estimating the preceding root vowels again.
 * Eligibility is measured on complete public readings for each lexical
 * prefix/ending class and tied to the unchanged base pronunciation. */
static int derive_er(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const char *const prefixes[]={"","ge","ver","be","er"};
    static const char *const sounds[]={"",".0g@",".0fEr",".0b@",".0Er"};
    static const struct {const char *letters,*phones;unsigned nominal;} endings[]={
        {"ungen","@.0rUG.0@n]",1},{"ung","@.0rUG]",1},
        {"ten",".0t@n]",0},{"tes",".0t@s]",0},
        {"tem",".0t@m]",0},{"ter",".0tR]",0},
        {"te",".0t@]",0},{"t","t]",0}
    };
    for (size_t pi=0;pi<sizeof prefixes/sizeof prefixes[0];++pi) {
        size_t start=strlen(prefixes[pi]);
        if (length<start+6 || memcmp(word,prefixes[pi],start)) continue;
        for (size_t ei=0;ei<sizeof endings/sizeof endings[0];++ei) {
            size_t end=strlen(endings[ei].letters);
            if (length<start+end+5 || memcmp(word+length-end,endings[ei].letters,end)) continue;
            const struct reloq_lexicon_override *entry=NULL;
            unsigned roles=roles_for(er_derivations,
                sizeof er_derivations/sizeof er_derivations[0],lexicon,
                word+start,length-start-end,&entry);
            if (!(roles&(1u<<(2*pi+endings[ei].nominal)))) continue;
            size_t lead=strlen(sounds[pi]),tail=strlen(endings[ei].phones);
            size_t base=entry->spr_length-(endings[ei].nominal?2:1);
            if (base+lead+tail>=SPR_LIMIT) return 0;
            memcpy(out,"`[",2);memcpy(out+2,sounds[pi],lead);
            memcpy(out+2+lead,entry->spr+2,base-2);
            memcpy(out+base+lead,endings[ei].phones,tail+1);
            *written=base+lead+tail;return 1;
        }
    }
    return 0;
}

/* A compound head may itself be a compound.  The original morphology
 * retries find_compound on an unresolved head; here every retry is gated by
 * publicly observed constituent roles and pronunciation fingerprints.  Only
 * two constituents are constructed at this stage.  The caller supplies the
 * third left constituent.  Competing parses are rejected, not arbitrarily
 * ranked by the spelling's first viable boundary. */
static int nested_compound_head(const struct reloq_lexicon *lexicon,
    const unsigned char *word, size_t length, unsigned char *out,
    size_t *written, unsigned *head_roles)
{
    unsigned char selected[SPR_LIMIT], candidate[SPR_LIMIT];
    size_t selected_length=0, n=0;
    unsigned selected_roles=0;
    for (size_t split=3;split+3<=length;++split) {
        if (vowel(word[split]) ||
            ((word[split-1]=='t' || word[split-1]=='d') && word[split]=='s'))
            continue;
        const struct reloq_lexicon_override *head=NULL;
        unsigned right_roles=component(lexicon,word+split,length-split,&head);
        unsigned char inflected_head[SPR_LIMIT];
        size_t inflected_length=0;
        struct reloq_lexicon_override derived={0};
        if (!right_roles && inflect(lexicon,word+split,length-split,
                                   inflected_head,&inflected_length,&right_roles)) {
            derived.spr=inflected_head;
            derived.spr_length=inflected_length;
            head=&derived;
        }
        if (!(right_roles&(ROOT_HEAD|ROOT_WEAK_HEAD))) continue;
        for (unsigned link=0;link<2;++link) {
            if (link && (word[split-1]!='s' || split<4 ||
                         (right_roles&ROOT_WEAK_HEAD))) continue;
            const struct reloq_lexicon_override *left=NULL;
            unsigned left_roles=component(lexicon,word,split-link,&left);
            if (!(left_roles&(link?ROOT_LINK_S:ROOT_LEFT))) continue;
            if (!join(left->spr,left->spr_length,head->spr,head->spr_length,
                      right_roles&ROOT_WEAK_HEAD?0:2,link?"s":"",candidate,&n))
                continue;
            if (selected_length &&
                (selected_length!=n || memcmp(selected,candidate,n)))
                return -1;
            if (!selected_length) {
                memcpy(selected,candidate,n+1);
                selected_length=n;
                selected_roles=right_roles;
            }
        }
    }
    if (!selected_length) return 0;
    memcpy(out,selected,selected_length+1);
    *written=selected_length;
    *head_roles=selected_roles;
    return 1;
}

/* Weak prefixes retain a lexical base's phones, including its vowel
 * quantity. The old path parsed the base afresh with orthographic G2P,
 * losing the original root reading before adding the same prefix. Only
 * the established verbal shapes and a single initial root accent enter
 * this path; whole-word readings have already had priority. */
/* Strip a verbal ending, resolve the unchanged lexical infinitive, then
 * attach the weak ending and prefix. This retains the base's quantity and
 * syllable boundaries, including /@l/ and /R/, instead of re-reading the
 * stem as an unrelated sequence of letters. Eligibility is independently
 * observed per base, prefix and ending; exact whole-word entries win. */
static int weak_verb(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const struct {const char *letters,*spr;} prefixes[]={
        {"",""},{"be",".0b@"},{"ge",".0g@"},{"ver",".0fEr"},
        {"er",".0Er"},{"ent",".0Ent"}
    };
    static const struct {const char *letters,*spr;} endings[]={
        {"n","n]"},{"t","t]"},{"te",".0t@]"},{"ten",".0t@n]"},
        {"test",".0t@st]"},{"tet",".0t@t]"},{"tes",".0t@s]"},
        {"ter",".0tR]"},{"tem",".0t@m]"},{"nd","nt]"},
        {"nde","n.0d@]"},{"nden","n.0d@n]"},
        {"ung","UG]"},{"ungen","UG.0@n]"},{"er","R]"},
        {"ern","Rn]"},{"erin","@.0rIn]"},{"erinnen","@.0rI.0n@n]"}
    };
    for (size_t pi=0;pi<sizeof prefixes/sizeof *prefixes;++pi) {
        size_t lead=strlen(prefixes[pi].letters);
        if (length<lead+4 || memcmp(word,prefixes[pi].letters,lead)) continue;
        for (size_t ei=0;ei<sizeof endings/sizeof *endings;++ei) {
            size_t end=strlen(endings[ei].letters);
            if (length<lead+end+3 || memcmp(word+length-end,endings[ei].letters,end)) continue;
            size_t stem=length-lead-end;
            if (stem+2>WORD_LIMIT || (word[length-end-1]!='l' && word[length-end-1]!='r')) continue;
            unsigned char key[WORD_LIMIT];
            memcpy(key,word+lead,stem);
            if (ei>=12 && key[stem-1]=='l') {
                if (stem+3>WORD_LIMIT) continue;
                key[stem-1]='e';key[stem++]='l';
            }
            key[stem]='n';key[stem+1]=0;
            size_t lo=0,hi=sizeof weak_verbs/sizeof *weak_verbs;
            const struct weak_verb_role *role=NULL;
            while (lo<hi) {
                size_t mid=lo+(hi-lo)/2;
                int cmp=strcmp((const char*)key,weak_verbs[mid].word);
                if (cmp<0) hi=mid;
                else if (cmp>0) lo=mid+1;
                else {role=weak_verbs+mid;break;}
            }
            if (!role || !(role->endings[pi]&(1u<<ei))) continue;
            const struct reloq_lexicon_override *entry=NULL;
            if (reloq_lexicon_find_override(lexicon,key,stem+1,&entry)!=1 ||
                fingerprint(entry->spr,entry->spr_length)!=role->pronunciation ||
                entry->spr_length<5 || memcmp(entry->spr+entry->spr_length-2,"n]",2)) continue;
            unsigned char reading[SPR_LIMIT];size_t base=entry->spr_length-2;
            if (base+8>=SPR_LIMIT) continue;
            memcpy(reading,entry->spr,base);
            if (ei>=12) {
                if (base>=5 && !memcmp(reading+base-2,"@l",2)) {
                    size_t at=base-2;
                    while (at && reading[at]!='.') --at;
                    if (!at || at+2>=base-2) continue;
                    size_t count=base-2-(at+2);
                    int valid=1;
                    for (size_t j=at+2;j<base-2;++j)
                        if (!strchr("bcCdfgGhjJklmnPprRsStTvwxXzZ",reading[j])) valid=0;
                    if (!valid) continue;
                    unsigned char last=reading[base-3];
                    memmove(reading+at,reading+at+2,count);
                    base=at+count;
                    /* Original syllable-onset classes admit kl/pl/fl,
                     * whereas dl/tl stay split at the derived nucleus. */
                    if (strchr("bpgkfPS",last)) {
                        reading[base-1]='.';reading[base++]='0';reading[base++]=last;
                    } else {reading[base++]='.';reading[base++]='0';}
                    reading[base++]='l';
                } else if (base && reading[base-1]=='R') {
                    reading[base-1]='@';memcpy(reading+base,".0r",3);base+=3;
                } else continue;
            }
            size_t prefix=strlen(prefixes[pi].spr),tail=strlen(endings[ei].spr);
            size_t n=base+prefix+tail;
            if (n>=SPR_LIMIT) return 0;
            memcpy(out,"`[",2);memcpy(out+2,prefixes[pi].spr,prefix);
            memcpy(out+2+prefix,reading+2,base-2);
            memcpy(out+base+prefix,endings[ei].spr,tail);
            out[n]=0;*written=n;return 1;
        }
    }
    return 0;
}

/* An outer dental inflection follows the already reduced final -er
 * syllable (strip_final_root_syll). Resolve that shorter reading first;
 * a full or stressed er vowel does not qualify. This also handles stems
 * absent from the old per-verb role table without inventing a new root. */
static int er_weak_inflection(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
 (void)lexicon;
 /* Prefix domains are resolved before final-root syllable stripping. */
 for(size_t i=0;i<sizeof prefixes/sizeof *prefixes;++i) {
  size_t z=strlen(prefixes[i].word);
  if(length>z+2 && !memcmp(word,prefixes[i].word,z))return 0;
 }
 static const char *const weak[]={"be","ge","ver","er","ent","zer","unter","\374ber","wieder"};
 for(size_t i=0;i<sizeof weak/sizeof *weak;++i) {
  size_t z=strlen(weak[i]);
  if(length>z+2 && !memcmp(word,weak[i],z))return 0;
 }
 static const struct {const char *letters,*phones;} tails[]={
  {"test",".0t@st]"},{"ten",".0t@n]"},{"tem",".0t@m]"},
  {"ter",".0tR]"},{"tes",".0t@s]"},{"tet",".0t@t]"},
  {"te",".0t@]"},{"st","st]"},{"t","t]"}};
 for(size_t i=0;i<sizeof tails/sizeof *tails;++i) {
  size_t tail=strlen(tails[i].letters);
  if(length<tail+5 || memcmp(word+length-tail,tails[i].letters,tail) ||
     memcmp(word+length-tail-2,"er",2))continue;
  unsigned char base[SPR_LIMIT];size_t n=0;
  if(reloq_german_g2p_word(word,length-tail,base,sizeof base,&n)!=1 ||
     n<7 || base[n-2]!='R')continue;
  const unsigned char *syllable=(const unsigned char *)strrchr((const char *)base,'.');
  if(!syllable || syllable[1]!='0')continue;
  size_t ending=strlen(tails[i].phones);
  if(n-1+ending>=SPR_LIMIT)continue;
  memcpy(out,base,n-1);memcpy(out+n-1,tails[i].phones,ending+1);
  *written=n-1+ending;return 1;
 }
 return 0;
}

/* ger_inflections removes the dental ending before ger_suffixes resolves
 * -ig. Preserve that inner suffix even when its root has no lexical entry:
 * treating the recovered infinitive as an opaque verb would devoice /g/
 * to /k/ instead. The shorter analysis must actually end in unstressed
 * /IX/; an orthographic -ig alone is insufficient (e.g. a diphthong stem).
 * Whole-word exceptions have already won in the lexicon caller. */
static int ig_weak_inflection(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
 static const struct {const char *letters,*phones;} tails[]={
  {"test",".0t@st]"},{"ten",".0t@n]"},{"tem",".0t@m]"},
  {"ter",".0tR]"},{"tes",".0t@s]"},{"tet",".0t@t]"},
  {"te",".0t@]"},{"st","st]"},{"t","t]"}};
 for(size_t i=0;i<sizeof tails/sizeof *tails;++i) {
  size_t tail=strlen(tails[i].letters);
  if(length<tail+5 || memcmp(word+length-tail,tails[i].letters,tail) ||
     memcmp(word+length-tail-2,"ig",2))continue;
  /* This branch covers consonant-final roots. Vowel-adjacent -ig needs
   * the separate minimal-root/hiatus decisions: the old short spelling
   * reader can report /IX/ there even when the original rejects -ig. */
  if(vowel(word[length-tail-3]))continue;
  unsigned char root[SPR_LIMIT];size_t n=0;
  if(reloq_german_lexicon_word(lexicon,word,length-tail,root,sizeof root,&n)!=1 ||
     n<7 || memcmp(root+n-3,"IX]",3))continue;
  const unsigned char *syllable=(const unsigned char *)strrchr((const char *)root,'.');
  if(!syllable || syllable[1]!='0')continue;
  size_t ending=strlen(tails[i].phones);
  if(n-1+ending>=SPR_LIMIT)continue;
  memcpy(out,root,n-1);memcpy(out+n-1,tails[i].phones,ending+1);
  *written=n-1+ending;return 1;
 }
 return 0;
}

/* -ig is inside the outer inflection/nominal suffix. A recovered -en
 * stem supplies the full root reading, including an inner particle and
 * its secondary accent. No complete derived word is stored here. */
static int ig_derivation(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
 static const struct {const char *ending,*phones;} tails[]={
  {"ungen","I.0gUG.0@n]"},{"ung","I.0gUG]"},
  {"enden","I.0g@n.0d@n]"},{"endem","I.0g@n.0d@m]"},
  {"ender","I.0g@n.0dR]"},{"endes","I.0g@n.0d@s]"},
  {"ende","I.0g@n.0d@]"},{"end","I.0g@nt]"},
  {"en","I.0g@n]"},{"em","I.0g@m]"},{"er","I.0gR]"},
  {"es","I.0g@s]"},{"e","I.0g@]"},
  {"ten","IX.0t@n]"},{"tem","IX.0t@m]"},{"ter","IX.0tR]"},
  {"tes","IX.0t@s]"},{"te","IX.0t@]"},{"t","IXt]"},{"st","IXst]"},
  {"","IX]"}};
 for(size_t k=0;k<sizeof tails/sizeof *tails;++k) {
  size_t en=strlen(tails[k].ending);
  if(length<en+5||memcmp(word+length-en,tails[k].ending,en)||
     memcmp(word+length-en-2,"ig",2))continue;
  size_t stem=length-en-2;
  unsigned char base[WORD_LIMIT],reading[SPR_LIMIT];size_t n=0;
  memcpy(base,word,stem);memcpy(base+stem,"en",3);
  if(!en_form(lexicon,base,stem+2,reading,&n,0)) {
   /* ger_suffixes leaves the complete root's accent in place. A full
    * root-final r moves to the suffix onset; it is not a weak -er
    * inflection and must not erase the preceding vowel's quantity. */
   const struct reloq_lexicon_override *entry=NULL;
   if(word[stem-1]!='r' ||
      reloq_lexicon_find_override(lexicon,word,stem,&entry)!=1 ||
      entry->spr_length<6 || entry->spr[entry->spr_length-2]!='r' ||
      !strchr("aeiouy",entry->spr[entry->spr_length-3]))continue;
   const unsigned char *last=(const unsigned char *)strrchr((const char *)entry->spr,'.');
   if(!last || last[1]!='1')continue;
   int particle=0;
   for(size_t j=0;j<sizeof prefixes/sizeof *prefixes;++j) {
    size_t pn=strlen(prefixes[j].word);
    if(stem>pn+1 && !memcmp(word,prefixes[j].word,pn))particle=1;
   }
   if(particle)continue;
   n=entry->spr_length;
   if(n+5>=sizeof reading)continue;
   memcpy(reading,entry->spr,n-2);
   memcpy(reading+n-2,".0r@n]",7);n+=4;
  }
  if(n<7||memcmp(reading+n-3,"@n]",3))continue;
  size_t z=strlen(tails[k].phones);
  if(n+z>=SPR_LIMIT)continue;
  memcpy(out,reading,n-3);memcpy(out+n-3,tails[k].phones,z+1);
  *written=n-3+z;return 1;
 }
 return 0;
}

/* ger_inflections can peel more than one weak ending. Resolve the shorter
 * complete analysis before another compound scan; otherwise an outer -en
 * can expose a spurious vowel-initial final constituent inside the root.
 * The inner -en keeps its schwa and its consonant moves to the new onset. */
static int lexical_weak_prefix(const struct reloq_lexicon *,const unsigned char *,
 size_t,unsigned char *,size_t *);
static int nested_weak_inflection(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
 static const struct {const char *letters,*phones;} tails[]={
  {"en","@n]"},{"em","@m]"},{"er","R]"},{"es","@s]"},{"e","@]"}};
 for(size_t i=0;i<sizeof tails/sizeof *tails;++i) {
  size_t tail=strlen(tails[i].letters),base=length-tail;
  if(length<tail+5 || memcmp(word+base,tails[i].letters,tail))continue;
  const int adjective=!memcmp(word+base-2,"ig",2);
  if(!adjective && memcmp(word+base-2,"en",2))continue;
  unsigned char stem[SPR_LIMIT];size_t n=0;
  if(adjective) {
   if(reloq_german_lexicon_word(lexicon,word,base,stem,sizeof stem,&n)!=1 ||
      n<7 || memcmp(stem+n-3,"IX]",3))continue;
  } else if((!en_form(lexicon,word,base,stem,&n,0) &&
             !lexical_weak_prefix(lexicon,word,base,stem,&n)) ||
            n<7 || memcmp(stem+n-3,"@n]",3))continue;
  size_t tn=strlen(tails[i].phones);
  if(n+tn+3>=SPR_LIMIT)continue;
  memcpy(out,stem,n-2);memcpy(out+n-2,adjective?".0g":".0n",3);
  memcpy(out+n+1,tails[i].phones,tn+1);*written=n+1+tn;return 1;
 }
 return 0;
}

/* The original peels weak inflection and -lich inside the already
 * established prefix domain. Recover an independently covered verb,
 * retaining its accent and quantity, before attaching the weak suffix.
 * Reduced -ern loses only n; -en loses its weak nucleus and resyllabifies
 * the remaining onset as a coda. No derived whole words are stored here.
 * Evidence: behavior-0.2.91/de-stages91.json.gz and family contrasts. */
static int lich_derivation91(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
 static const struct {const char *letters,*phones;} tails[]={
  {"lichen",".0lI.0X@n]"},{"lichem",".0lI.0X@m]"},
  {"licher",".0lI.0XR]"},{"liches",".0lI.0X@s]"},
  {"liche",".0lI.0X@]"},{"lich",".0lIX]"}};
 for(size_t k=0;k<sizeof tails/sizeof *tails;++k) {
  size_t tail=strlen(tails[k].letters);
  if(length<tail+3 || memcmp(word+length-tail,tails[k].letters,tail))continue;
  size_t stem=length-tail;
  unsigned char verb[WORD_LIMIT],reading[SPR_LIMIT];size_t n=0;
  memcpy(verb,word,stem);
  int reduced_er=stem>=3 && !memcmp(word+stem-2,"er",2);
  const char *ending=reduced_er?"n":"en";
  size_t en=strlen(ending);memcpy(verb+stem,ending,en+1);
  if(reloq_german_lexicon_word(lexicon,verb,stem+en,reading,sizeof reading,&n)!=1 ||
     n<7 || strstr((const char *)reading,".2"))continue;
  if(reduced_er) {
   if(memcmp(reading+n-3,"Rn]",3))continue;
   n-=2;
  } else {
   if(memcmp(reading+n-3,"@n]",3))continue;
   n-=3;reading[n]=0;
   unsigned char *last=(unsigned char *)strrchr((const char *)reading,'.');
   if(!last || last[1]!='0')continue;
   /* Only the infinitive's onset remains after its schwa is removed. */
   size_t onset=(size_t)(last-reading)+2;
   if(strpbrk((const char *)reading+onset,"aAeEiIoOuUyY@R'"))continue;
   memmove(last,reading+onset,n-onset);n-=2;
   if(n && reading[n-1]=='b')reading[n-1]='p';
   else if(n && reading[n-1]=='d')reading[n-1]='t';
   else if(n && reading[n-1]=='g')reading[n-1]='k';
   else if(n && reading[n-1]=='z')reading[n-1]='s';
  }
  size_t tn=strlen(tails[k].phones);
  if(n+tn>=SPR_LIMIT)continue;
  memcpy(out,reading,n);memcpy(out+n,tails[k].phones,tn+1);
  *written=n+tn;return 1;
 }
 return 0;
}

static int lexical_weak_prefix(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const struct {const char *letters,*spr;} prefixes[]={
        {"be","`[.0b@"},{"ge","`[.0g@"},{"ver","`[.0fEr"},{"er","`[.0Er"},
        {"ent","`[.0Ent"}
    };
    for (size_t k=0;k<sizeof prefixes/sizeof *prefixes;++k) {
        size_t prefix=strlen(prefixes[k].letters);
        if (length<prefix+4 || memcmp(word,prefixes[k].letters,prefix)) continue;
        size_t n=length-prefix;
        const int base_ending=!memcmp(word+length-2,"en",2) ||
            (word[length-1]=='t' && strchr("bdg",word[length-2]));
        const struct reloq_lexicon_override *entry=NULL;
        unsigned char reading[SPR_LIMIT];size_t reading_length=0;
        struct reloq_lexicon_override derived={0};
        /* Inflection/derivation is inside the prefix's stress domain.
         * Recover the witnessed infinitive and transform its ending before
         * attaching the weak prefix, retaining the complete root reading. */
        int witnessed=en_form(lexicon,word+prefix,n,reading,&reading_length,(unsigned)k+1);
        if(!witnessed && ig_derivation(lexicon,word+prefix,n,reading,&reading_length))witnessed=3;
        if (witnessed) {
            derived.spr=reading;derived.spr_length=reading_length;entry=&derived;
        } else if (!base_ending ||
            (reloq_lexicon_find_override(lexicon,word+prefix,n,&entry)!=1 &&
             reloq_german_cvc_reference_lookup(word+prefix,n,&entry)!=1)) continue;
        if (entry->spr_length<5 || memcmp(entry->spr,"`[.",3)) continue;
        if (witnessed!=2 && (k==4 && vowel(word[prefix]))) continue;
        unsigned primary=0,other_accent=0;
        for (size_t j=2;j+1<entry->spr_length;++j)
            if (entry->spr[j]=='.') {
                if (entry->spr[j+1]=='1') ++primary;
                else if (entry->spr[j+1]!='0') other_accent=1;
            }
        size_t leader=strlen(prefixes[k].spr);
        if (primary!=1 || leader+entry->spr_length>=SPR_LIMIT) continue;
        /* An inner particle or prefix is a separate stress domain. Its
         * standalone accent need not survive another weak prefix. */
        if(witnessed!=2 && witnessed!=3 && (entry->spr[3]!='1' || other_accent)) {
            static const char *const inner[]={"be","ge","ver","er","ent","zer",
                "miss","um","zu","an","auf","aus","ab","ein","vor","nach","bei"};
            int nested=0;
            for(size_t j=0;j<sizeof inner/sizeof *inner;++j) {
                size_t z=strlen(inner[j]);
                if(n>z+1 && !memcmp(word+prefix,inner[j],z))nested=1;
            }
            if(nested)continue;
        }
        memcpy(out,prefixes[k].spr,leader);
        memcpy(out+leader,entry->spr+2,entry->spr_length-2);
        *written=leader+entry->spr_length-2;out[*written]=0;return 1;
    }
    return 0;
}

/* Ending analysis retries an unchanged lexical infinitive before letter
 * analysis. The stem's entire syllable sequence survives in -end; the
 * ending's /d/ voices again before a weak vowel. Eligibility is separate
 * for each prefix and tied to the source reading's fingerprint. */
static int participle(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const struct {const char *letters,*spr;} endings[]={
        {"endem",".0d@m]"},{"enden",".0d@n]"},{"ender",".0dR]"},
        {"endes",".0d@s]"},{"ende",".0d@]"},{"end","t]"}
    };
    static const struct {const char *letters,*spr;} prefixes[]={
        {"","`["},{"be","`[.0b@"},{"ge","`[.0g@"},{"ver","`[.0fEr"},
        {"er","`[.0Er"},{"ent","`[.0Ent"}
    };
    for (size_t e=0;e<sizeof endings/sizeof *endings;++e) {
        size_t tail=strlen(endings[e].letters);
        if (length<tail+3 || memcmp(word+length-tail,endings[e].letters,tail)) continue;
        for (size_t k=0;k<sizeof prefixes/sizeof *prefixes;++k) {
            size_t lead=strlen(prefixes[k].letters),base=length-tail;
            if (base<lead+3 || memcmp(word,prefixes[k].letters,lead)) continue;
            unsigned char root[WORD_LIMIT];size_t n=base-lead;
            memcpy(root,word+lead,n);memcpy(root+n,"en",2);n+=2;
            const struct reloq_lexicon_override *entry=NULL;
            unsigned role=roles_for(inflections,sizeof inflections/sizeof *inflections,
                lexicon,root,n,&entry);
            if (!(role&(16u<<k)) || entry->spr_length<5 ||
                memcmp(entry->spr+entry->spr_length-3,"@n]",3)) continue;
            size_t a=strlen(prefixes[k].spr),b=entry->spr_length-3,
                z=strlen(endings[e].spr);
            if (a+b+z>=SPR_LIMIT) continue;
            memcpy(out,prefixes[k].spr,a);memcpy(out+a,entry->spr+2,b);
            memcpy(out+a+b,endings[e].spr,z+1);*written=a+b+z;return 1;
        }
    }
    return 0;
}

/* An inflected comparative has two weak syllables. Re-running the root
 * spelling loses its original vowel quantity (frisch -> frischeren).
 * Public -ere/-erem witnesses select the lexical class; -eren/-erer/-eres
 * are independent transfer cases. */
static int comparative(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const struct {const char *letters,*spr;} endings[]={
        {"eren","@n]"},{"erem","@m]"},{"erer","R]"},
        {"eres","@s]"},{"ere","@]"}
    };
    for (size_t e=0;e<sizeof endings/sizeof *endings;++e) {
        size_t tail=strlen(endings[e].letters);
        if (length<tail+4 || memcmp(word+length-tail,endings[e].letters,tail)) continue;
        size_t base=length-tail;const struct reloq_lexicon_override *entry=NULL;
        if (!(roles_for(inflections,sizeof inflections/sizeof *inflections,
            lexicon,word,base,&entry)&8u) || entry->spr_length+16>=SPR_LIMIT) continue;
        size_t used=entry->spr_length-1;memcpy(out,entry->spr,used);
        unsigned char onset=0,last=out[used-1];
        if (last=='R') {out[used-1]='@';onset='r';}
        else if (strchr("pbtdkgfvszSZxXhlrjwmnPTCJ",last)) {
            onset=last;--used;unsigned char final=word[base-1];
            if (final=='b'||final=='d'||(final=='g'&&word[base-2]!='n')) onset=final;
            else if (final=='s'&&word[base-2]!='s') onset='z';
        }
        out[used++]='.';out[used++]='0';if (onset) out[used++]=onset;
        memcpy(out+used,"@.0r",4);used+=4;
        size_t z=strlen(endings[e].spr);memcpy(out+used,endings[e].spr,z+1);
        *written=used+z;return 1;
    }
    return 0;
}

/* Nominal suffixes are resolved before the linking s and the following
 * root. Reusing that complete analysis retains short stem vowels and weak
 * syllables, including an inner -er-, rather than spelling the compound
 * again as one word. */
static int nominal_left(const struct reloq_lexicon *lexicon,
 const unsigned char *w,size_t n,unsigned char *out,size_t *written)
{
 static const char *const endings[]={"heit","keit","schaft","ung","it\344t"};
 for(size_t i=0;i<sizeof endings/sizeof *endings;++i) {
  size_t k=strlen(endings[i]);
  if(n>k+2&&!memcmp(w+n-k,endings[i],k))
   return reloq_german_lexicon_word(lexicon,w,n,out,SPR_LIMIT,written)==1;
 }
 return 0;
}

/* The final-root predicate runs after weak ending and suffix removal.
 * Its measured classes also contain roots behind a weak verbal prefix.
 * These are constituent features, not pronunciations for complete words. */
static unsigned final_root_class(const unsigned char *w,size_t n)
{
 static const char *const endings[]={"","e","en","er","em","es","s","ung","ungen","igung","igungen"};
 static const char *const prefixes[]={"","be","ge","ver","er","ent"};
 for(size_t i=0;i<sizeof endings/sizeof *endings;++i) {
  size_t en=strlen(endings[i]);
  if(n<en+3||memcmp(w+n-en,endings[i],en))continue;
  for(size_t k=0;k<sizeof prefixes/sizeof *prefixes;++k) {
   size_t pn=strlen(prefixes[k]);
   if(n<en+pn+3||memcmp(w,prefixes[k],pn))continue;
   unsigned role=compound_class(w+pn,n-en-pn)&22u;
   if(role)return role;
  }
 }
 return 0;
}

static int constituent_reading(const struct reloq_lexicon *lexicon,
 const unsigned char *w,size_t n,unsigned char *out,size_t *written,int estimate)
{
 if(reloq_german_lexicon_word(lexicon,w,n,out,SPR_LIMIT,written)==1 ||
    reloq_german_root_plan_word(lexicon,w,n,out,SPR_LIMIT,written)==1)return 1;
 if(estimate==2 && reloq_german_unclassified_root_word(lexicon,w,n,
    out,SPR_LIMIT,written)==1)return 1;
 int approximate=0;
 return estimate && reloq_german_g2p_word_best_effort(w,n,out,SPR_LIMIT,written,&approximate)==1;
}

#include "reloq_german_compound_boundaries.inc"

/* Finite spelling-edge classes measured at compound_con_cluster. They
 * describe boundaries, never whole-word pronunciations. The full coda
 * matters (nd, ck and doubled liquids do not share a final-letter rule). */
static int cluster_boundary(const unsigned char *w,size_t n,size_t split)
{
 if(split<3 || split+3>n || vowel(w[split-1]) || vowel(w[split]))return 0;
 size_t left=split,right=split;
 while(left&&!vowel(w[left-1]))--left;
 while(right<n&&!vowel(w[right]))++right;
 if(right<n && right>split && w[right-1]=='q' && w[right]=='u')++right;
 if(!left || right==n)return 0;
 if(right-split==2 && !memcmp(w+split,"pf",2)) {
  /* possible_initial_compound admits the complete dental affricate
   * before this onset, independently of compound_con_cluster. */
  if(split-left==2 && !memcmp(w+left,"tz",2))return 1;
  static const char *const codas[]={ "b","bb","bst","ch","chs","chst","cht","chz","ck","d","dd","dn","dt","f","ff","ft","g","gd","gg","gn","h","hl","hm","hn","hnd","hnt","hr","hrd","hrt","k","kk","kt","lb","lbst","lch","ld","ldt","lf","lft","lg","lk","ll","ln","lph","ls","lst","lt","lz","mb","md","mpf","mph","ms","mt","n","nch","nd","ndt","nf","nft","ng","ngs","ngst","nk","nn","ns","nst","nt","nth","nz","pf","ph","pp","pst","pt","rb","rbst","rch","rcht","rck","rd","rdt","rf","rft","rg","rh","rk","rn","rnst","rnz","rph","rrh","rs","rst","rt","rth","rv","rz","rzt","s","sch","sk","ss","st","t","th","tt","v","wn","wsk","z","zt","zz" };
  for(size_t i=0;i<sizeof codas/sizeof *codas;++i)
   if(strlen(codas[i])==split-left&&!memcmp(codas[i],w+left,split-left))return 1;
  return 0;
 }
 size_t onset=sizeof boundary_onsets/sizeof *boundary_onsets;
 for(size_t i=0;i<sizeof boundary_onsets/sizeof *boundary_onsets;++i)
  if(strlen(boundary_onsets[i])==right-split &&
     !memcmp(boundary_onsets[i],w+split,right-split)){onset=i;break;}
 if(onset==sizeof boundary_onsets/sizeof *boundary_onsets)return 0;
 for(size_t i=0;i<sizeof boundary_codas/sizeof *boundary_codas;++i)
  if(strlen(boundary_codas[i].coda)==split-left &&
     !memcmp(boundary_codas[i].coda,w+left,split-left))
   return (int)((boundary_codas[i].allowed[onset/64]>>(onset%64))&1);
 return 0;
}

/* possible_initial_compound also accepts a completed weak -er left
 * constituent before a legal new onset. It is independent of the r+coda
 * pair classifier and is visited in the same left-to-right scan. */
static int weak_er_boundary(const unsigned char *w,size_t n,size_t split)
{
 if(split<4 || split+3>n || vowel(w[0]) || memcmp(w+split-2,"er",2))return 0;
 size_t end=split;while(end<n&&!vowel(w[end]))++end;
 if(end==split || end==n)return 0;
 if(end-split==1 && (w[split]=='n'||w[split]=='s'))return 0;
 if(end-split==2 && (!memcmp(w+split,"gh",2)||!memcmp(w+split,"sc",2)))return 0;
 for(size_t i=0;i<sizeof boundary_onsets/sizeof *boundary_onsets;++i)
  if(strlen(boundary_onsets[i])==end-split &&
     !memcmp(boundary_onsets[i],w+split,end-split))return 1;
 return end-split==2&&!memcmp(w+split,"pf",2);
}

static int cluster_compound(const struct reloq_lexicon *lexicon,
 const unsigned char *w,size_t n,unsigned char *out,size_t *written)
{
 unsigned char prior[SPR_LIMIT];size_t pn=0;
 if(reloq_german_g2p_word(w,n,prior,sizeof prior,&pn)==1 ||
    reloq_german_root_plan_word(lexicon,w,n,prior,sizeof prior,&pn)==1)return 0;
 if((n>3&&!memcmp(w+n-3,"bei",3)) ||
    (n>2&&!memcmp(w+n-2,"zu",2)))return 0;
 int approximate=0;
 if(reloq_german_g2p_word_best_effort(w,n,prior,sizeof prior,&pn,&approximate)==1) {
  unsigned secondary=0;
  for(size_t i=0;i+1<pn;++i)secondary+=prior[i]=='.'&&prior[i+1]=='2';
  /* Keep the existing multi-root accent hierarchy. This additional
   * boundary recognizer does not flatten an already composed analysis. */
  if(secondary>1)return 0;
 }
 /* Prefix, particle and loan-root analyses precede the unclassified
  * consonant-boundary scan. Their internal letters are not fresh roots. */
 static const char *const weak[]={"be","ge","ver","er","ent","zer",
  "ab","an","auf","aus","ein","vor","nach","zu","mit","un"};
 for(size_t i=0;i<sizeof weak/sizeof *weak;++i) {
  size_t z=strlen(weak[i]);
  if(n>z+2&&!memcmp(w,weak[i],z))return 0;
 }
 /* The original compound scan sees the root remaining after weak and
  * derivational suffix removal. A vowel belonging only to that suffix
  * cannot license a second constituent. */
 size_t domain=n;
 static const char *const endings[]={"ungen","enden","endem","ender","endes",
  "ende","nisse","nissen","licher","lichen","liche",
  "ischen","ischer","ische","heit","keit","schaft","ung","nis","niss",
  "lich","isch","reich","bar","sam","haft","end","est","ern","et","en","er","em","es","e","ig"};
 for(unsigned pass=0;pass<4;++pass) {
  size_t before=domain;
  for(size_t i=0;i<sizeof endings/sizeof *endings;++i) {
   size_t z=strlen(endings[i]);
   if(domain>z+2&&!memcmp(w+domain-z,endings[i],z)) {domain-=z;break;}
  }
  if(domain==before)break;
 }
 for(size_t split=3;split+3<=domain;++split) {
  int weak_er=weak_er_boundary(w,domain,split);
  if(!weak_er && !cluster_boundary(w,domain,split))continue;
  if(n>split+2 && w[n-1]=='t' && strchr("bdg",w[n-2]))continue;
  /* Positive table-173 observations protect an indivisible root across
   * a tempting consonant edge. No pronunciation is stored here. */
  if(n>=8 && split<8 && !memcmp(w,"synchron",8))continue;
  /* Derivational -haft has already been removed in the original before
   * the cluster scan, including followed inflectional material. */
  if(n>=split+4 && !memcmp(w+split,"haft",4))continue;
  /* possible_initial_compound rejects these reversed left-edge tests
   * before admitting an otherwise legal consonant boundary. */
  static const char *const excluded[]={"dis","kal","kon","sub"};
  int blocked=0;
  for(size_t k=0;k<sizeof excluded/sizeof *excluded;++k) {
   size_t z=strlen(excluded[k]);
   if(split>=z&&!memcmp(w+split-z,excluded[k],z))blocked=1;
  }
  if(blocked)continue;
  int nucleus=0;for(size_t k=split;k<domain;++k)nucleus|=vowel(w[k]);
  if(!nucleus)continue;
  const struct reloq_lexicon_override *changed=NULL;
  if(spelling_roles(w,split) && !component(lexicon,w,split,&changed) && changed)continue;
  changed=NULL;
  if(spelling_roles(w+split,n-split) &&
     !component(lexicon,w+split,n-split,&changed) && changed)continue;
  unsigned char left[SPR_LIMIT],right[SPR_LIMIT];size_t ln=0,rn=0;
  if(!constituent_reading(lexicon,w,split,left,&ln,1) ||
     !constituent_reading(lexicon,w+split,n-split,right,&rn,2))continue;
  if(weak_er && (ln<4 || left[ln-2]!='R' || !strstr((const char *)left,".0")))continue;
  if(join(left,ln,right,rn,2,"",out,written))return 1;
 }
 return 0;
}



/* Prefix stripping retries the remaining morph domain, including already
 * resolved inflection. Do not spell the entire word again merely because
 * the root is absent from the infinitive-only prefix table. This retry is
 * after the established whole-word analyses. Each recursive input is shorter.
 * Source: strip_inseparable_prefix -> ger_prefixes -> root_particle. */
static int retry_weak_prefix(const struct reloq_lexicon *lexicon,
 const unsigned char *w,size_t n,unsigned char *out,size_t *written)
{
 static const struct {const char *text,*spr;} weak[]={
  {"be","`[.0b@]"},{"ge","`[.0g@]"},{"ver","`[.0fEr]"},
  {"er","`[.0Er]"},{"ent","`[.0Ent]"},{"zer","`[.0TEr]"}};
 unsigned char prior[SPR_LIMIT];size_t prior_n=0;
 /* A completed strict analysis retains precedence over a root retry. */
 if(reloq_german_g2p_word(w,n,prior,sizeof prior,&prior_n)==1)return 0;
 static const char *const protected[]={
#include "reloq_german_prefix_protected74.inc"
 };
 for(size_t k=0;k<sizeof protected/sizeof *protected;++k) {
  size_t size=strlen(protected[k]);
  if(n>=size&&!memcmp(w,protected[k],size))return 0;
 }
 for(size_t k=0;k<sizeof weak/sizeof *weak;++k) {
  size_t lead=strlen(weak[k].text),size=n-lead;
  if(n<lead+4 || memcmp(w,weak[k].text,lead))continue;
  const unsigned char *base=w+lead;
  if(vowel(base[0]))continue;
  /* A vowel immediately before -ung competes with the letter-stage
   * diphthong/hiatus analysis; an isolated base cannot license that join. */
  if(size>3 && !memcmp(base+size-3,"ung",3) && vowel(base[size-4]))continue;
  size_t end=0;while(end<size&&!vowel(base[end]))++end;
  /* An impossible root onset cannot be created by prefix removal. */
  if(end>1 && !(end==2 &&
      ((base[1]=='r'&&strchr("bptdgkf",base[0])) ||
       (base[1]=='l'&&strchr("bpgkf",base[0])) ||
       (base[0]=='s'&&strchr("pt",base[1])) ||
       (base[0]=='k'&&base[1]=='n') ||
       (base[0]=='g'&&strchr("mn",base[1])) ||
       (base[0]=='p'&&base[1]=='f') ||
       (strchr("cpt",base[0])&&base[1]=='h'))) &&
     !(end>=3&&!memcmp(base,"sch",3)))continue;
  int inner=0;
  static const char *const particles[]={"be","ge","ver","er","ent","zer",
      "miss","um","un","zu","an","auf","aus","ab","ein","vor","nach","bei","voll"};
  for(size_t j=0;j<sizeof particles/sizeof *particles;++j) {
   size_t z=strlen(particles[j]);
   if(size>z+1&&!memcmp(base,particles[j],z))inner=1;
  }
  if(inner)continue;
  unsigned char root[SPR_LIMIT];size_t rn=0;
  if(reloq_german_lexicon_word(lexicon,base,size,root,sizeof root,&rn)!=1) {
   /* The original prefix pass also retains a root which has no lexical
    * hit. Re-enter its letter domain after the boundary is established;
    * otherwise inter-vocalic h is incorrectly treated as a length mark. */
   /* Preserve a recognized complete loan domain and compound before
    * admitting this unknown root. Their stress is already established. */
   if(reloq_german_root_plan_word(lexicon,w,n,prior,sizeof prior,&prior_n)==1)
    continue;
   if(base[0]!='h')continue;
   int estimated=0;
   if(reloq_german_g2p_word_best_effort(base,size,root,sizeof root,&rn,&estimated)!=1)
    continue;
   /* Weak inflection is inside the recovered h-root. It does not turn
    * that root into a compound or cancel the earlier weak prefix. */
   unsigned full_nuclei=0;
   struct reloq_german_spr_cursor cursor={0};struct reloq_german_spr_event event;
   while(reloq_german_spr_next(root,rn,&cursor,&event)==1)
    if(event.kind==RELOQ_DE_SPR_PHONE &&
       (event.phone<=RELOQ_DE_PHONE_AT || event.phone>=RELOQ_DE_PHONE_Q_E_COLON) &&
       event.phone!=RELOQ_DE_PHONE_AT)++full_nuclei;
   if(full_nuclei!=1)continue;
  }
  /* A secondary accent after the primary belongs to another constituent,
   * not the single-root stress domain retried here. */
  int primary=0,compound=0;
  for(size_t j=2;j+1<rn;++j)if(root[j]=='.') {
   if(root[j+1]=='1')primary=1;
   else if(primary && root[j+1]=='2')compound=1;
  }
  /* Prefix stripping re-enters compound analysis in the remaining
   * domain. A secondary root accent is not a failed prefix parse. */
  if(compound) {
   unsigned char composed[SPR_LIMIT];size_t cn=0;
   int final_word=0;
   for(size_t split=3;split+3<=size;++split) {
    if(compound_class(base+split,size-split)&16u)final_word=1;
    for(size_t weak=1;weak<=2&&split+weak+3<=size;++weak)
     if(base[size-weak]=='e' && (weak==1||strchr("nmrs",base[size-1])) &&
        (compound_class(base+split,size-split-weak)&16u))final_word=1;
   }
   if(!(final_word && reloq_german_morphology_word(lexicon,base,size,
          composed,sizeof composed,&cn)==1) &&
      !cluster_compound(lexicon,base,size,composed,&cn))continue;
   memcpy(root,composed,cn+1);rn=cn;
  }
  return join((const unsigned char *)weak[k].spr,strlen(weak[k].spr),
      root,rn,1,"",out,written);
 }
 return 0;
}

static int diminutive_t(const struct reloq_lexicon *lexicon,
 const unsigned char *w,size_t n,unsigned char *out,size_t *written)
{
 size_t suffix=n>=5&&!memcmp(w+n-5,"chens",5)?5:
               n>=4&&!memcmp(w+n-4,"chen",4)?4:0;
 if(!suffix||n<suffix+3||w[n-suffix-1]!='t')return 0;
 for(size_t i=0;i<sizeof prefixes/sizeof *prefixes;++i)
  if(strlen(prefixes[i].word)==n-suffix&&!memcmp(w,prefixes[i].word,n-suffix))return 0;
 unsigned char root[SPR_LIMIT];size_t rn=0;
 if(!constituent_reading(lexicon,w,n-suffix,root,&rn,1)||rn<4)return 0;
 const char *ending=suffix==5?".0X@ns]":".0X@n]";
 size_t en=strlen(ending);if(rn+en>=SPR_LIMIT)return 0;
 memcpy(out,root,rn-1);memcpy(out+rn-1,ending,en+1);
 *written=rn-1+en;return 1;
}

/* ger_suffixes keeps -haft outside the root stress domain. The outer
 * weak inflection resyllabifies its t, but cannot lengthen its a or give
 * the suffix a compound accent. -schaft is a separate suffix. */
static int haft_derivation(const struct reloq_lexicon *lexicon,
 const unsigned char *w,size_t n,unsigned char *out,size_t *written)
{
 static const struct {const char *letters,*spr;} endings[]={
  {"", "`[.0hAft]"}, {"e", "`[.0hAf.0t@]"},
  {"en", "`[.0hAf.0t@n]"}, {"em", "`[.0hAf.0t@m]"},
  {"es", "`[.0hAf.0t@s]"}, {"er", "`[.0hAf.0tR]"},
  {"est", "`[.0hAf.0t@st]"}, {"end", "`[.0hAf.0t@nt]"}};
 for(size_t k=0;k<sizeof endings/sizeof *endings;++k) {
  size_t e=strlen(endings[k].letters);
  if(n<e+7 || memcmp(w+n-e,endings[k].letters,e) ||
     memcmp(w+n-e-4,"haft",4))continue;
  size_t root=n-e-4;
  if(w[root-1]=='c')continue;
  int particle=0;
  for(size_t j=0;j<sizeof prefixes/sizeof *prefixes;++j)
   if(strlen(prefixes[j].word)==root && !memcmp(w,prefixes[j].word,root))particle=1;
  static const char *const weak[]={"be","ge","ver","er","ent","zer"};
  for(size_t j=0;j<sizeof weak/sizeof *weak;++j)
   if(strlen(weak[j])==root && !memcmp(w,weak[j],root))particle=1;
  if(particle)continue;
  unsigned char stem[SPR_LIMIT];size_t sn=0;
  if(reloq_german_lexicon_word(lexicon,w,root,stem,sizeof stem,&sn)!=1)continue;
  return join(stem,sn,(const unsigned char *)endings[k].spr,
      strlen(endings[k].spr),0,"",out,written);
 }
 return 0;
}

/* Resolve -st outside the lexical stem, then the outer weak ending.
 * A separate whole-word entry for the bare superlative is not required:
 * that excluded normal forms such as schoenste/wichtigste. The shorter
 * lexical stem supplies quantity, /IX/ and reduced el/er/en nuclei.
 * Exact complete-word readings retain priority in the lexicon caller. */
static int superlative_ending(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const struct {const char *letters,*phones;} endings[]={
        {"ens","@ns]"},{"en","@n]"},{"em","@m]"},
        {"er","R]"},{"es","@s]"},{"e","@]"},{"","]"}};
    for(size_t i=0;i<sizeof endings/sizeof *endings;++i) {
        size_t en=strlen(endings[i].letters);
        if(length<en+5 || memcmp(word+length-en,endings[i].letters,en))continue;
        size_t base=length-en;
        if(memcmp(word+base-2,"st",2))continue;
        const struct reloq_lexicon_override *root=NULL;
        struct reloq_lexicon_override derived={0};
        unsigned char reading[SPR_LIMIT];size_t rn=0;
        if(reloq_lexicon_find_override(lexicon,word,base-2,&root)!=1) {
            /* Productive -ig has a recognizable suffix identity. An
             * arbitrary reduced ending may belong to a compound instead
             * (Willkommensfenster); require lexical evidence for those. */
            if(base<7 || memcmp(word+base-4,"ig",2) ||
               reloq_german_lexicon_word(lexicon,word,base-2,reading,sizeof reading,&rn)!=1 ||
               rn<7 || memcmp(reading+rn-3,"IX]",3))continue;
            derived.spr=reading;derived.spr_length=rn;root=&derived;
        }
        if(root->spr_length<6)continue;
        /* A sibilant plus dental may instead be verbal -st/-te. That
         * separate path owns assimilation and stem shortening (lasste,
         * lesst), so do not reinterpret it as a superlative. */
        if(strchr("szST",root->spr[root->spr_length-2]))continue;
        size_t n=root->spr_length-1,tn=strlen(endings[i].phones);
        if(n+4+tn>=SPR_LIMIT)continue;
        memcpy(out,root->spr,n);
        const char *suffix=en?".0st":"st";size_t sn=strlen(suffix);
        memcpy(out+n,suffix,sn);
        memcpy(out+n+sn,endings[i].phones,tn+1);*written=n+sn+tn;return 1;
    }
    return 0;
}

/* strip_inflection runs before strip_final_root_syll. A weak -el nucleus
 * therefore remains weak under outer n/st/t and dental endings, and its
 * root quantity survives contraction. Reuse the independently analysed
 * bare stem instead of re-estimating the whole inflected spelling. */
static int el_weak_inflection(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    static const struct {const char *letters,*phones;} ends[]={
        {"n","n]"},{"st","st]"},{"t","t]"},{"te",".0t@]"},
        {"ten",".0t@n]"},{"test",".0t@st]"},{"tet",".0t@t]"},
        {"e",".0l@]"},{"en",".0l@n]"},{"em",".0l@m]"},
        {"er",".0lR]"},{"es",".0l@s]"}
    };
    for(size_t i=0;i<sizeof ends/sizeof *ends;++i) {
        size_t en=strlen(ends[i].letters);
        if(length<en+4 || memcmp(word+length-en,ends[i].letters,en))continue;
        size_t stem=length-en;
        if(word[stem-1]!='l')continue;
        int contracted=word[stem-2]!='e';
        if(contracted && (vowel(word[stem-2]) || strchr("hlr",word[stem-2]) ||
            (i>=8)))continue;
        /* Contracted ck+l retains the two-letter quantity marker although
         * it has only one velar phone. Other contracted stem classes keep
         * their established inflection path until that root is witnessed. */
        if(contracted && (stem<3 || memcmp(word+stem-3,"ck",2)))continue;
        size_t lead=0;
        static const char *const prefixes[]={"be","ge","ver","er","ent","zer"};
        for(size_t k=0;k<sizeof prefixes/sizeof *prefixes;++k) {
            size_t pn=strlen(prefixes[k]);
            if(stem>pn && !memcmp(word,prefixes[k],pn)){lead=pn;break;}
        }
        size_t root_end=stem-(contracted?1:2);
        if(root_end>=2 && !memcmp(word+root_end-2,"ts",2))continue;
        int nucleus=0;
        for(size_t k=lead;k<root_end;++k)nucleus|=vowel(word[k]);
        if(!nucleus || root_end<lead+2)continue;
        if(lead) {
            unsigned char head[SPR_LIMIT];size_t hn=0;
            if(!el_weak_inflection(lexicon,word+lead,length-lead,head,&hn))continue;
            const char *prefix=word[0]=='b'?"`[.0b@]":word[0]=='g'?"`[.0g@]":
                word[0]=='v'?"`[.0fEr]":word[0]=='z'?"`[.0TEr]":
                lead==3?"`[.0Ent]":"`[.0Er]";
            return join((const unsigned char *)prefix,strlen(prefix),head,hn,1,"",out,written);
        }
        unsigned char key[WORD_LIMIT],reading[SPR_LIMIT];size_t n=0;
        if(stem+(size_t)contracted>=WORD_LIMIT)continue;
        memcpy(key,word,stem);
        if(contracted){key[stem-1]='e';key[stem++]='l';}
        int ap=0;
        if(reloq_german_lexicon_word(lexicon,key,stem,reading,sizeof reading,&n)!=1 &&
           reloq_german_g2p_word_best_effort(key,stem,reading,sizeof reading,&n,&ap)!=1)continue;
        if(n<6 || n+12>=SPR_LIMIT || memcmp(reading+n-3,"@l]",3))continue;
        /* A multi-phone onset can belong to a full final root instead of
         * the weak suffix (the st+el class). It needs its separate root
         * analysis, not this single-consonant weak-syllable transform. */
        if((reading[n-5]!='0' || reading[n-6]!='.') &&
           (i>=7 || reading[n-4]!='0' || reading[n-5]!='.'))continue;
        size_t used=n-1;
        if(i>=7) {
            --used; /* The outer weak vowel takes l as its onset. */
            if(contracted) {
                --used;
                if(used>=3 && reading[used-3]=='.' && reading[used-2]=='0' &&
                   !strchr("bpgkfPS",reading[used-1])) {
                    reading[used-3]=reading[used-1];reading[used-2]='.';reading[used-1]='0';
                }
                const char *phone=ends[i].phones+2;
                size_t pn=strlen(phone);
                memcpy(reading+used,phone,pn+1);used+=pn;
            } else {
                size_t pn=strlen(ends[i].phones);
                memcpy(reading+used,ends[i].phones,pn+1);used+=pn;
            }
        } else {
            size_t pn=strlen(ends[i].phones);
            memcpy(reading+used,ends[i].phones,pn+1);used+=pn;
        }
        if(used>=SPR_LIMIT || !reloq_german_lexicon_validate_spr(reading,used,NULL))continue;
        memcpy(out,reading,used+1);*written=used;return 1;
    }
    return 0;
}


/* Pronominal particles consist of a deictic element and a preposition.
 * Analyse each constituent before vowel quantity, v voicing and root
 * accent. A following verb is an independent, secondary-stressed domain. */
static int pronominal_particle(const struct reloq_lexicon *lexicon,
 const unsigned char *w,size_t n,unsigned char *out,size_t *written)
{
 size_t lead=0;int da=0,r_link=0,hier=0;
 if(n>5&&!memcmp(w,"hier",4)){lead=4;hier=1;r_link=vowel(w[4]);}
 else if(n>4&&!memcmp(w,"dar",3)&&vowel(w[3])){lead=3;da=1;r_link=1;}
 else if(n>4&&!memcmp(w,"wor",3)&&vowel(w[3])){lead=3;r_link=1;}
 else if(n>3&&!memcmp(w,"da",2)&&!vowel(w[2])){lead=2;da=1;}
 else if(n>3&&!memcmp(w,"wo",2)&&!vowel(w[2]))lead=2;
 if(!lead)return 0;
 static const char *const heads[]={"zwischen","hinter","gegen","neben","unter","\374ber",
  "durch","nach","von","vor","mit","bei","f\374r","aus","auf","ein","an","zu","um","in"};
 for(size_t k=0;k<sizeof heads/sizeof *heads;++k) {
  size_t hn=strlen(heads[k]),used=lead+hn;
  if(used>n || memcmp(w+lead,heads[k],hn))continue;
  /* wo+zwischen is not licensed as a productive particle by the original
   * analysis; its longer forms retain their ordinary compound route. */
  if(used<n && ((!da && !hier && !strcmp(heads[k],"zwischen")) ||
      (da && r_link && !strcmp(heads[k],"in"))))continue;
  unsigned char head[SPR_LIMIT],prefix[SPR_LIMIT],verb[SPR_LIMIT];size_t hn_spr=0,pn=0,vn=0;
  if(reloq_german_lexicon_word(lexicon,w+lead,hn,head,sizeof head,&hn_spr)!=1)continue;
  if(!da && !hier && r_link && !strcmp(heads[k],"an") && hn_spr==7)head[4]='a';
  if(r_link) {if(hn_spr+1>=sizeof head)continue;memmove(head+5,head+4,hn_spr-3);head[4]='r';++hn_spr;}
  int initial=da && (!strcmp(heads[k],"von")||!strcmp(heads[k],"vor")||
      !strcmp(heads[k],"durch")||!strcmp(heads[k],"zwischen")||!strcmp(heads[k],"hinter")||!strcmp(heads[k],"neben")||
      (r_link&&strcmp(heads[k],"ein")&&strcmp(heads[k],"um")));
  const int secondary=(hier || (da&&used<n))&&!strcmp(heads[k],"f\374r");
  int infinitive=0;
  if(used<n) {
   size_t start=used;
   if(n>start+4 && !memcmp(w+start,"zu",2)){start+=2;infinitive=1;}
   const struct reloq_lexicon_override *root=NULL;
   unsigned role=component(lexicon,w+start,n-start,&root);
   if(!(role&(ROOT_PARTICLE|ROOT_PREFIX)) &&
      !en_form(lexicon,w+start,n-start,verb,&vn,0) &&
      !participle(lexicon,w+start,n-start,verb,&vn))continue;
   if(!vn&&reloq_german_lexicon_word(lexicon,w+start,n-start,verb,sizeof verb,&vn)!=1)continue;
   initial=0;
  }
  const char *base=da?"`[.0da]":hier?(r_link?"`[.0hi]":"`[.0hir]"):"`[.0vo]";
  unsigned char deictic[16];size_t dn=strlen(base);memcpy(deictic,base,dn+1);
  deictic[3]=initial||secondary?'1':'0';
  if(!join(deictic,dn,head,hn_spr,secondary?2:initial?0:1,"",prefix,&pn))continue;
  if(used<n)return join(prefix,pn,verb,vn,2,infinitive?".0Tu":"",out,written);
  memcpy(out,prefix,pn+1);*written=pn;return 1;
 }
 return 0;
}

/* The outer s inflection precedes the final-root weak-syllable pass.
 * A completed weak stem keeps its quantity, voicing and accent. Re-enter
 * that shorter domain instead of analysing els/ers as a new full vowel.
 * A full or stressed ending is deliberately not eligible. */
static int outer_weak_s(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *out,size_t *written)
{
    if(length<5 || word[length-1]!='s')return 0;
    size_t stem=length-1;
    if(word[stem-1]!='l' && word[stem-1]!='r' && word[stem-1]!='n')return 0;
    static const char *const prefixes[]={"be","ge","ver","er","ent","zer"};
    for(size_t i=0;i<sizeof prefixes/sizeof *prefixes;++i) {
        size_t pn=strlen(prefixes[i]);
        if(stem>pn+2 && !memcmp(word,prefixes[i],pn) &&
           weak_prefix_onset(word,stem,pn))return 0;
    }
    unsigned char base[SPR_LIMIT];size_t n=0;
    int approximate=0;
    if(reloq_german_lexicon_word(lexicon,word,stem,base,sizeof base,&n)!=1 &&
       reloq_german_g2p_word_best_effort(word,stem,base,sizeof base,&n,&approximate)!=1)return 0;
    if(n<7 || n+1>=SPR_LIMIT)return 0;
    const unsigned char *syllable=(const unsigned char *)strrchr((const char *)base,'.');
    if(!syllable || syllable[1]!='0' ||
       (!strchr((const char *)syllable,'@') && !strchr((const char *)syllable,'R')))return 0;
    memcpy(out,base,n-1);memcpy(out+n-1,"s]",3);*written=n+1;return 1;
}

/* strip_inflection wins over an internal compound split when a strict
 * monosyllabic stem and the complete weakly inflected reading agree.
 * This prevents a final dental + ending from becoming a second root. */
static int strict_weak_inflection91(const unsigned char *word,size_t length,
 unsigned char *out,size_t *written)
{
 size_t ending=length>=2 && word[length-2]=='e' &&
     strchr("nmrs",word[length-1])?2:
     length && word[length-1]=='e'?1:0;
 if(!ending || length<ending+3)return 0;
 unsigned char stem[SPR_LIMIT];size_t sn=0,n=0;
 if(reloq_german_g2p_word(word,length-ending,stem,sizeof stem,&sn)!=1 ||
    sn<6 || memcmp(stem,"`[.1",4) || strchr((const char *)stem+4,'.'))return 0;
 if(reloq_german_g2p_word(word,length,out,SPR_LIMIT,&n)!=1 ||
    n<sn || memcmp(out,"`[.1",4) || strstr((const char *)out,".2"))return 0;
 const unsigned char *last=(const unsigned char *)strrchr((const char *)out,'.');
 if(!last || last==out+2 || last[1]!='0' ||
    !strpbrk((const char *)last,"@R"))return 0;
 *written=n;return 1;
}

int reloq_german_morphology_word(const struct reloq_lexicon *lexicon,
    const unsigned char *word,size_t length,unsigned char *output,
    size_t capacity,size_t *byte_length)
{
    if (!word || !length || !byte_length || (!output && capacity)) return -1;
    if (!lexicon || length>=WORD_LIMIT) return 0;
    unsigned char key[WORD_LIMIT];
    for (size_t i=0;i<length;++i) {
        if (!letter(word[i])) return -1;
        key[i]=lower(word[i]);
    }
    key[length]=0;
    unsigned char selected[SPR_LIMIT],candidate[SPR_LIMIT];
    size_t selected_length=0,n=0;
    if(complete_particle_infinitive88(lexicon,key,length,selected,&selected_length))goto found;
    if(separable_infinitive(lexicon,key,length,selected,&selected_length))goto found;
    /* Inflection is resolved before prefix stripping and compound search.
     * Otherwise -in/-innen may become a stressed compound head, and ge-
     * inside an unrecognised loan root can be mistaken for a prefix. */
    if(reloq_german_feminine_ending(key,length) &&
       reloq_german_root_plan_word(lexicon,key,length,selected,sizeof selected,
           &selected_length)==1)goto found;
    if(pronominal_particle(lexicon,key,length,selected,&selected_length))goto found;
    if(superlative_ending(lexicon,key,length,selected,&selected_length))goto found;
    if(lich_derivation91(lexicon,key,length,selected,&selected_length))goto found;
    if (haft_derivation(lexicon,key,length,selected,&selected_length)) goto found;
    if (ig_weak_inflection(lexicon,key,length,selected,&selected_length)) goto found;
    if (outer_weak_s(lexicon,key,length,selected,&selected_length)) goto found;
    /* The original nominal-suffix path keeps a complete -tion/-sion
     * domain before linking -s, also before a vowel or a prefixed head.
     * Its internal primary/secondary stress must not be reanalysed as
     * one long root. Both components still use native word analysis. */
    if(!selected_length)for(size_t split=7;split+3<=length;++split) {
        if(key[split-1]!='s' ||
           (memcmp(key+split-5,"tion",4)&&memcmp(key+split-5,"sion",4)))continue;
        unsigned char left[SPR_LIMIT],head[SPR_LIMIT];size_t ln=0,hn=0;
        if(!constituent_reading(lexicon,key,split-1,left,&ln,0) ||
           !constituent_reading(lexicon,key+split,length-split,head,&hn,1))continue;
        if(join(left,ln,head,hn,2,"s",selected,&selected_length))goto found;
    }
    if (en_form(lexicon,key,length,selected,&selected_length,0)) goto found;
    if (negative_particle(lexicon,key,length,selected,&selected_length)) goto found;
    if (nested_weak_inflection(lexicon,key,length,selected,&selected_length)) goto found;
    if (participle(lexicon,key,length,selected,&selected_length)) goto found;
    if (comparative(lexicon,key,length,selected,&selected_length)) goto found;
    if (weak_verb(lexicon,key,length,selected,&selected_length)) goto found;
    /* A complete lexical initial constituent retains its internal weak
     * syllable and consonant voicing. Re-stripping that syllable would
     * substitute an unrelated bare root (ge+samt in Gesamt+...). The
     * witnessed non-linking LEFT/HEAD/PARTICLE roles and complete
     * final-word class license the constituent. A linking stem retains
     * the ordinary derivation priority instead of splitting off -igungen.
     * The remaining head is analysed independently. */
    for(size_t split=length>=7?length-3:0;split>=4;--split) {
        const struct reloq_lexicon_override *left=NULL;
        unsigned roles=component(lexicon,key,split,&left);
        if((roles&(ROOT_LEFT|ROOT_HEAD|ROOT_PARTICLE))!=(ROOT_LEFT|ROOT_HEAD|ROOT_PARTICLE) ||
           (roles&ROOT_LINK_S) ||
           !(compound_class(key,split)&16u) || !left ||
           left->spr_length<8 || memcmp(left->spr,"`[.0",4) ||
           !memchr(left->spr+4,'.',left->spr_length-4))continue;
        const struct reloq_lexicon_override *right=NULL;
        unsigned head_roles=component(lexicon,key+split,length-split,&right);
        unsigned char head[SPR_LIMIT];size_t hn=0;
        if(constituent_reading(lexicon,key+split,length-split,head,&hn,1) &&
           join(left->spr,left->spr_length,head,hn,
                head_roles&ROOT_WEAK_HEAD?0:2,"",selected,&selected_length))goto found;
    }
    if (lexical_weak_prefix(lexicon,key,length,selected,&selected_length)) goto found;
    if (ig_derivation(lexicon,key,length,selected,&selected_length)) goto found;
    if (derive_en_ung(lexicon,key,length,selected,&selected_length)) goto found;
    if (derive_er(lexicon,key,length,selected,&selected_length)) goto found;
    if (length>8 && (!memcmp(key,"un",2) || !memcmp(key,"ab",2)) &&
        derive_er(lexicon,key+2,length-2,candidate,&n)) {
        const char *prefix=key[0]=='u'?"`[.1Un]":"`[.1Ap]";
        if (join((const unsigned char *)prefix,strlen(prefix),candidate,n,2,"",
                 selected,&selected_length)) goto found;
    }
    if (inflect_weak_adjective(lexicon,key,length,selected,&selected_length)) goto found;
    if (negative_adjective(lexicon,key,length,selected,&selected_length)) goto found;
    if (inflect_contracted_el(lexicon,key,length,selected,&selected_length)) goto found;
    if (inflect(lexicon,key,length,selected,&selected_length,NULL)) goto found;
    if (diminutive_t(lexicon,key,length,selected,&selected_length)) goto found;
    if (negative_weak_prefix(lexicon,key,length,selected,&selected_length)) goto found;
    if (structured_prefix(lexicon,key,length,selected,&selected_length)) goto found;
    if(strict_weak_inflection91(key,length,selected,&selected_length))goto found;
    for (size_t i=0;i<sizeof prefixes/sizeof prefixes[0];++i) {
        size_t prefix_length=strlen(prefixes[i].word);
        if (length<prefix_length+3 || memcmp(key,prefixes[i].word,prefix_length)) continue;
        for (unsigned z=0;z<2;++z) {
            size_t start=prefix_length+2*z;
            if (length<start+3 || (z && memcmp(key+prefix_length,"zu",2))) continue;
            if(!z && prefix_length>3 &&
               (!memcmp(prefixes[i].word,"her",3)||!memcmp(prefixes[i].word,"hin",3)) &&
               length>start+2 && !memcmp(key+start,"zu",2))continue;
            if(prefix_length==3 && (!strcmp(prefixes[i].word,"her") ||
                !strcmp(prefixes[i].word,"hin")) && key[start]==key[start-1])continue;
            const struct reloq_lexicon_override *root=NULL;
            unsigned roles=component(lexicon,key+start,length-start,&root);
            unsigned scope=!strcmp(prefixes[i].word,"aus")?1u:
                !strcmp(prefixes[i].word,"ein")?2u:!strcmp(prefixes[i].word,"auf")?4u:
                !strcmp(prefixes[i].word,"ab")?8u:!strcmp(prefixes[i].word,"an")?16u:0u;
            unsigned char reading[SPR_LIMIT];size_t reading_length=0;
            struct reloq_lexicon_override derived={0};
            /* A particle owns the outer domain. Re-enter a shorter weak-
             * prefix analysis before looking for a bare lexical head; the
             * latter loses both schwa and the following consonant onset.
             * Infinitival zu is outside that same head, never its accent. */
            if(!(roles&(z?ROOT_PREFIX:ROOT_PREFIX|ROOT_PARTICLE))) {
                static const char *const weak[]={"be","ge","ver","er","ent","zer"};
                int nested=0;
                for(size_t k=0;k<sizeof weak/sizeof *weak;++k) {
                    size_t pn=strlen(weak[k]);
                    if(length>start+pn+2 && !memcmp(key+start,weak[k],pn) &&
                       weak_prefix_onset(key+start,length-start,pn))nested=1;
                }
                int infinitive=z && length>start+3 &&
                    !memcmp(key+length-2,"en",2) &&
                    en_form(lexicon,key+start,length-start,reading,&reading_length,0);
                /* Vowel-initial heads retain the later constituent
                 * boundary pass. In particular it distinguishes weak er
                 * from an outer constituent ending in vocalic r. */
                if(((nested && !vowel(key[start])) || infinitive) &&
                   reloq_german_lexicon_word(lexicon,key+start,length-start,
                       reading,sizeof reading,&reading_length)==1 &&
                   reading_length>6 && (infinitive || reading[3]=='0')) {
                    derived.spr=reading;derived.spr_length=reading_length;
                    root=&derived;roles=z?ROOT_PREFIX:ROOT_PARTICLE;
                }
            }
            if (!z && scope && en_participle_head(lexicon,key+start,length-start,
                scope,reading,&reading_length)) {
                derived.spr=reading;derived.spr_length=reading_length;
                root=&derived;roles=ROOT_PARTICLE;
            }
            /* The stressed particle is outside an already analysed ge-
             * participle. Re-enter that shorter analysis without flattening
             * the particle/weak-prefix boundary into an ng digraph. */
            if(!z && !(roles&(ROOT_PREFIX|ROOT_PARTICLE)) && length>start+4 &&
               !memcmp(key+start,"ge",2) &&
               (key[length-1]=='t' || (key[length-1]=='n'&&key[length-2]=='e')) &&
               reloq_german_lexicon_word(lexicon,key+start,length-start,
                   reading,sizeof reading,&reading_length)==1 &&
               reading_length>7 && !memcmp(reading,"`[.0g@",6)) {
                derived.spr=reading;derived.spr_length=reading_length;
                root=&derived;roles=ROOT_PARTICLE;
            }
            /* Nominal and verbal stems keep their lexical quantity and
             * consonants under the observed particles. Infix -zu- remains
             * restricted to the separately measured infinitive class. */
            if(z && prefix_length>3 &&
               (!memcmp(prefixes[i].word,"her",3)||!memcmp(prefixes[i].word,"hin",3)) &&
               !(roles&ROOT_PREFIX))continue;
            if (!(roles&(z?ROOT_PREFIX:ROOT_PREFIX|ROOT_PARTICLE))) {
                /* The particle precedes the complete inflection analysis.
                 * Reuse an analysed shorter head even if it has no separate
                 * whole-word entry in the constituent table. */
                if(z || selected_length || vowel(key[start]))continue;
                int longer=0;
                for(size_t k=0;k<i;++k) {
                    size_t size=strlen(prefixes[k].word);
                    if(size>prefix_length && length>size &&
                       !memcmp(key,prefixes[k].word,size))longer=1;
                }
                if(longer)continue;
                int productive=strcmp(prefixes[i].word,"zu") && length>start+3 &&
                    key[length-2]=='e' && key[length-1]=='n' &&
                    en_form(lexicon,key+start,length-start,reading,&reading_length,0)!=0 &&
                    reading_length>4 && reading[3]=='1';
                int head_approximate=0;
                if(!productive && strcmp(prefixes[i].word,"zu") &&
                   length>start+4 && !memcmp(key+length-2,"en",2) &&
                   weak_prefix_onset(key,length,start) &&
                   reloq_german_g2p_word_best_effort(key+start,length-start,reading,
                       sizeof reading,&reading_length,&head_approximate)==1 && reading_length>=7 &&
                   reading[3]=='1' && !memcmp(reading+reading_length-3,"@n]",3))
                {
                    size_t syllables=0;
                    for(size_t k=0;k<reading_length;++k)syllables+=reading[k]=='.';
                    productive=syllables==2;
                }
                /* A head with its own compound accent keeps that domain. */
                for(size_t k=4;productive && k+1<reading_length;++k)
                    if(reading[k]=='.' && reading[k+1]=='2')productive=0;
                int licensed=prefix_length>=5 || productive;
                if(!licensed && strcmp(prefixes[i].word,"um") && strcmp(prefixes[i].word,"zu") && derive_en_ung(lexicon,key+start,length-start,
                    reading,&reading_length))licensed=1;
                if(!licensed && length>start+5 &&
                   ((!memcmp(key+length-3,"ung",3)) ||
                    (!memcmp(key+length-5,"ungen",5))) &&
                   strcmp(prefixes[i].word,"um") && strcmp(prefixes[i].word,"zu") &&
                   reloq_german_root_plan_word(lexicon,key+start,length-start,reading,
                       sizeof reading,&reading_length)==1 && reading[3]=='1') {
                    licensed=1;productive=1;
                }
                static const char *const ends[]={"e","en","est","et","st","t",
                    "te","ten","test","tet","tes","ter","tem","end","ende",
                    "enden","endem","ender","endes"};
                for(size_t k=0;!licensed && k<sizeof ends/sizeof *ends;++k) {
                    size_t size=strlen(ends[k]);
                    const struct reloq_lexicon_override *stem=NULL;
                    if(length>start+size+2 && !memcmp(key+length-size,ends[k],size) &&
                       (component(lexicon,key+start,length-start-size,&stem)&ROOT_PARTICLE) &&
                       en_form(lexicon,key+start,length-start,reading,&reading_length,0)==2)
                        licensed=1;
                }
                size_t adjective=length-start;
                size_t inflected=reloq_german_adjective_base(key+start,adjective);
                if(inflected)adjective=inflected;
                if(!licensed && adjective>4 &&
                   ((!memcmp(key+start+adjective-4,"lich",4)) ||
                    (!memcmp(key+start+adjective-4,"isch",4)) ||
                    (!memcmp(key+start+adjective-2,"ig",2))) &&
                   reloq_german_g2p_word(key+start,length-start,reading,
                       sizeof reading,&reading_length)==1)licensed=1;
                int root_plan=0;
                if(!licensed && strcmp(prefixes[i].word,"zu")) {
                    int prior_boundary=0;
                    /* A lexical root is not necessarily admitted as an
                     * initial compound. Use that separate observed class
                     * when deciding whether it precedes the particle. */
                    for(size_t k=start+1;k+3<=length;++k)
                        if(compound_class(key,k)&1u)prior_boundary=1;
                    for(size_t k=3;k+start+3<=length;++k)
                        if(compound_class(key+start,k)&1u)prior_boundary=1;
                    if(!prior_boundary && reloq_german_root_plan_word(lexicon,
                        key+start,length-start,reading,sizeof reading,&reading_length)==1 &&
                        reading_length>4 && (reading[3]=='0' ||
                         (reading[3]=='1' && key[length-1]=='s' &&
                          !memchr(reading+4,'.',reading_length-4)))) {
                        unsigned char resolved[SPR_LIMIT];size_t rn=0;
                        if(constituent_reading(lexicon,key+start,length-start,resolved,&rn,2) &&
                           rn==reading_length && !memcmp(resolved,reading,rn))root_plan=1;
                    }
                }
                if(!root_plan && (!licensed || (!productive &&
                    reloq_german_lexicon_word(lexicon,key+start,length-start,
                        reading,sizeof reading,&reading_length)!=1)))continue;
                derived.spr=reading;derived.spr_length=reading_length;
                root=&derived;roles=ROOT_PARTICLE;
            }
            const char *particle=prefixes[i].spr;int accent=2;
            /* Variable particles relinquish their accent before an
             * independently licensed nominal derivation. Merely ending
             * in the letters -ung is not sufficient evidence. */
            if(!z && !strcmp(prefixes[i].word,"um")) {
                unsigned char nominal[SPR_LIMIT];size_t nominal_size=0;
                if(derive_en_ung(lexicon,key+start,length-start,nominal,&nominal_size)) {
                    particle="`[.0Um]";accent=1;
                }
            }
            if (join((const unsigned char *)particle,strlen(particle),
                     root->spr,root->spr_length,accent,z?".0Tu":"",candidate,&n) &&
                !select_candidate(selected,&selected_length,candidate,n)) return 0;
        }
    }
    if(!selected_length && el_weak_inflection(lexicon,key,length,selected,&selected_length))goto found;
    if(!selected_length && er_weak_inflection(lexicon,key,length,selected,&selected_length))goto found;
    if(!selected_length && weak_left_particle(lexicon,key,length,selected,&selected_length))goto found;
    if(!selected_length && prefixed_compound(lexicon,key,length,selected,&selected_length))goto found;
    /* Re-enter a complete vowel-initial final constituent. The original
     * final-word lookup admits the complete spelling, whereas the later
     * initial-word lookup admits an independently marked first root.
     * Neither a stripped ending nor a weak particle is a final-word hit.
     * The user-requested extension lets an intact ch constituent retain
     * this complete head before a competing internal h+root split. */
    if(!selected_length)for(size_t split=length>=7?length-4:0;split>=3;--split) {
        if(!vowel(key[split]) || vowel(key[split-1]))continue;
        const struct reloq_lexicon_override *left=NULL;
        if(!(component(lexicon,key,split,&left)&ROOT_LEFT))continue;
        const int final_word=!!(compound_class(key+split,length-split)&16u);
        const int initial_word=!!(compound_class(key,split)&1u);
        if(!final_word && !initial_word)continue;
        unsigned char head[SPR_LIMIT];size_t hn=0;
        if(reloq_german_lexicon_word(lexicon,key+split,length-split,
            head,sizeof head,&hn)!=1 || hn<5 || memcmp(head,"`[.",3))continue;
        int particle=0;
        for(size_t j=0;j<sizeof prefixes/sizeof *prefixes;++j) {
            size_t pn=strlen(prefixes[j].word),sn=strlen(prefixes[j].spr)-1;
            if(length-split>pn+2 && !memcmp(key+split,prefixes[j].word,pn) &&
               hn>sn && !memcmp(head,prefixes[j].spr,sn) && head[sn]=='.')particle=1;
        }
        if(!final_word && !particle)continue;
        /* A complete weak-prefix analysis precedes this new boundary,
         * including strict letter analysis of its remaining root. Do not
         * let a longer spelling hide the already recognized prefix. */
        if(reloq_german_g2p_word(key,length,candidate,sizeof candidate,&n)==1 &&
           n>4 && !memcmp(candidate,"`[.0",4))continue;
        if(retry_weak_prefix(lexicon,key,length,selected,&selected_length))goto complete;
        if(join(left->spr,left->spr_length,head,hn,2,"",selected,&selected_length))goto found;
    }
    /* Prefix analysis precedes compound search, as in the observed stage
     * order. A successful structured parse is not overwritten by G2P. */
    if (!selected_length) for (size_t split=3;split+3<=length;++split) {
        if (vowel(key[split]) ||
            ((key[split-1]=='t' || key[split-1]=='d') && key[split]=='s')) continue;
        const struct reloq_lexicon_override *head=NULL;
        unsigned head_roles=component(lexicon,key+split,length-split,&head);
        unsigned char inflected_head[SPR_LIMIT];size_t inflected_length=0;
        struct reloq_lexicon_override derived={0};
        if (en_participle_head(lexicon,key+split,length-split,en_boundary_scope(key,split),
            inflected_head,&inflected_length)) {
            derived.spr=inflected_head;derived.spr_length=inflected_length;
            head=&derived;head_roles=ROOT_HEAD;
        }
        if (!head_roles && inflect(lexicon,key+split,length-split,
            inflected_head,&inflected_length,&head_roles)) {
            derived.spr=inflected_head;derived.spr_length=inflected_length;head=&derived;
        }
        /* A derived -ung head has already recovered a verified verb root.
         * Reuse it under the compound boundary instead of losing both
         * lexical stress and quantity in whole-word letter analysis. */
        if (!head_roles && derive_en_ung(lexicon,key+split,length-split,
            inflected_head,&inflected_length)) {
            derived.spr=inflected_head;derived.spr_length=inflected_length;
            head=&derived;head_roles=ROOT_HEAD;
        }
        /* Re-analyse an unresolved compound head once, rather than
         * collapsing the remaining spelling into the letter fallback.
         * Ambiguous inner parses do not license a productive reading. */
        unsigned char nested_head[SPR_LIMIT];size_t nested_length=0;
        struct reloq_lexicon_override nested={0};
        if (!head_roles) {
            int nested_status=nested_compound_head(lexicon,key+split,
                length-split,nested_head,&nested_length,&head_roles);
            if (nested_status<0) continue;
            if (nested_status>0) {
                nested.spr=nested_head;
                nested.spr_length=nested_length;
                head=&nested;
            }
        }
        if (!(head_roles&(ROOT_HEAD|ROOT_WEAK_HEAD))) continue;
        for (unsigned link=0;link<2;++link) {
            /* Linking before a weak head can instead trigger derivation,
             * stem shortening or a new stress domain. Its root retry is
             * not covered by the compound rule; leave that path untouched. */
            if (link && (key[split-1]!='s' || split<4 ||
                         (head_roles&ROOT_WEAK_HEAD))) continue;
            const struct reloq_lexicon_override *left=NULL;
            unsigned roles=component(lexicon,key,split-link,&left);
            unsigned char left_reading[SPR_LIMIT];size_t ln=0;
            struct reloq_lexicon_override derived_left={0};
            if (!(roles&(link?ROOT_LINK_S:ROOT_LEFT))) {
                /* The consonant-cluster pass can establish an initial
                 * constituent without a dictionary LEFT class. A doubled
                 * final consonant remains wholly in the left root; its
                 * quantity and accent are planned before joining the head.
                 * Inflection alone never licenses this boundary. */
                int cluster=!link && split>=4 && !vowel(key[split-1]) &&
                    key[split-1]==key[split-2] && key[split-2]!=key[split-3] &&
                    reloq_german_root_plan_word(lexicon,key,split,left_reading,
                        sizeof left_reading,&ln)==1;
                if(!cluster && (!link || !nominal_left(lexicon,key,split-1,left_reading,&ln)))continue;
                derived_left.spr=left_reading;derived_left.spr_length=ln;left=&derived_left;
            }
            if (join(left->spr,left->spr_length,head->spr,head->spr_length,
                    head_roles&ROOT_WEAK_HEAD?0:2,link?"s":"",candidate,&n) &&
                !select_candidate(selected,&selected_length,candidate,n)) return 0;
        }
    }
    if(!selected_length)for(size_t split=6;split+3<=length;++split) {
        if(key[split-1]!='s')continue;
        unsigned char left[SPR_LIMIT],right[SPR_LIMIT];size_t ln=0,rn=0;
        if(!nominal_left(lexicon,key,split-1,left,&ln)||
           !constituent_reading(lexicon,key+split,length-split,right,&rn,0))continue;
        if(join(left,ln,right,rn,2,"s",selected,&selected_length))goto found;
    }
    /* A licensed final root is located before trying an initial root.
     * The final-root predicate sees the stripped inflection, but head
     * pronunciation still runs on its full spelling. */
    if (!selected_length) for(size_t split=3;split+4<=length;++split) {
        static const char *const endings[]={"","e","en","er","em","es","s"};
        int licensed=0;
        for(size_t k=0;k<sizeof endings/sizeof *endings;++k) {
            size_t en=strlen(endings[k]),hn=length-split;
            if(hn>en+2 && !memcmp(key+length-en,endings[k],en) &&
               (compound_class(key+split,hn-en)&6))licensed=1;
        }
        if(!licensed)continue;
        const struct reloq_lexicon_override *left=NULL;
        unsigned char left_plan[SPR_LIMIT];size_t ln=0;
        struct reloq_lexicon_override derived_left={0};
        if(!(component(lexicon,key,split,&left)&ROOT_LEFT)) {
            if(vowel(key[split-1]) || reloq_lexicon_find_override(lexicon,key,split,&left)==1)continue;
            if(reloq_german_root_plan_word(lexicon,key,split,left_plan,
                sizeof left_plan,&ln)!=1 &&
               reloq_german_g2p_word(key,split,left_plan,sizeof left_plan,&ln)!=1)continue;
            derived_left.spr=left_plan;derived_left.spr_length=ln;left=&derived_left;
        }
        unsigned char head[SPR_LIMIT];size_t hn=0;
        if(!constituent_reading(lexicon,key+split,length-split,head,&hn,0))continue;
        if(join(left->spr,left->spr_length,head,hn,2,"",selected,&selected_length))goto found;
    }
    /* The original initial-constituent predicate scans from the longest
     * prefix. A measured initial class may precede an unresolved head;
     * absence of a complete head entry does not erase the boundary. */
    if (!selected_length) for (size_t split=length>=6?length-3:0;split>=3;--split) {
        /* A weak prefix with a legal following onset is resolved before
         * a competing nominal initial constituent. */
        int prefixed=0;
        static const char *const weak[]={"be","ge","ver","er","ent","zer"};
        for(size_t k=0;k<sizeof weak/sizeof *weak;++k) {
            size_t pn=strlen(weak[k]);
            if(length>pn+2 && !memcmp(key,weak[k],pn)) {
                size_t end=pn;while(end<length&&!vowel(key[end]))++end;
                size_t cn=end-pn;
                if(cn<=1 || (cn==2 &&
                    ((key[pn]=='s'&&strchr("pt",key[pn+1])) ||
                     (key[pn+1]=='r'&&strchr("bptdgkf",key[pn])) ||
                     (key[pn+1]=='l'&&strchr("bpgkf",key[pn])))) ||
                    (cn>=3&&!memcmp(key+pn,"sch",3)))prefixed=1;
            }
        }
        if(prefixed)continue;
        const size_t rest=length-split;
        unsigned head_nuclei=0;
        for(size_t j=split;j<length;++j)
            if(vowel(key[j]) && (j==split || !vowel(key[j-1])))++head_nuclei;
        int open_left=vowel(key[split-1])&&key[split-1]!='e';
        const int simple_head=open_left && head_nuclei==1 && rest>1 &&
            !vowel(key[split]) && vowel(key[split+1]);
        if(rest<4&&!open_left)continue;
        int suffix=0;
        static const char *const derivations[]={"bar","lich","isch","end","ung","nis","niss"};
        static const char *const inflections[]={"","e","en","em","er","es"};
        for (size_t d=0;d<sizeof derivations/sizeof *derivations;++d)
            for (size_t e=0;e<sizeof inflections/sizeof *inflections;++e) {
                size_t dn=strlen(derivations[d]),en=strlen(inflections[e]);
                if (rest==dn+en && !memcmp(key+split,derivations[d],dn) &&
                    !memcmp(key+split+dn,inflections[e],en)) suffix=1;
            }
        /* An unresolved tion- sequence can be a productive loan ending,
         * not a compound head. Preserve the whole-root stress path even
         * if the fragment itself has an isolated lexical reading. */
        if (suffix || (rest>=4 && !memcmp(key+split,"tion",4)) ||
            !(compound_class(key,split)&1)) continue;
        const struct reloq_lexicon_override *left=NULL,*right=NULL;
        if (!(component(lexicon,key,split,&left)&ROOT_LEFT)) continue;
        /* A full-vowel initial root has a separate domain before a new
         * vowel. Do not reinterpret native suffixes after schwa or loan
         * roots with the same written prefix as this hiatus boundary. */
        if(open_left) {if(!vowel(key[split]) && !simple_head)continue;}
        else if(vowel(key[split-1]) && (key[split-1]!='e' ||
            left->spr_length<3 || memcmp(left->spr+left->spr_length-2,"@]",2)))continue;
        unsigned char head[SPR_LIMIT];size_t hn=0;
        unsigned roles=component(lexicon,key+split,length-split,&right);
        if(blocked_participle_head(lexicon,key+split,length-split,
            en_boundary_scope(key,split)))continue;
        if (roles) {
            hn=right->spr_length;memcpy(head,right->spr,hn+1);
        } else if (!inflect(lexicon,key+split,length-split,head,&hn,NULL) &&
                   !(!((key[split-1]=='t'||key[split-1]=='d')&&key[split]=='s') &&
                     !(rest>=4&&key[length-1]=='t'&&strchr("bdg",key[length-2])) &&
                     !(rest>4&&!memcmp(key+split,"zu",2)) &&
                     (open_left || !vowel(key[split]) ||
                      (final_root_class(key+split,length-split)&6u)) &&
                     !(rest>1 && key[split]==key[split+1] && !vowel(key[split])) &&
                     constituent_reading(lexicon,key+split,length-split,head,&hn,simple_head))) {
            if (vowel(key[split])) continue;
            if (reloq_german_root_plan_word(lexicon,key+split,length-split,
                    head,sizeof head,&hn)!=1) {
                int native=0;
                for(size_t k=0;k<sizeof weak/sizeof *weak;++k) {
                    size_t pn=strlen(weak[k]);
                    if(rest>pn+3&&!memcmp(key+split,weak[k],pn))native=1;
                }
                if(!native || reloq_german_lexicon_word(lexicon,key+split,rest,
                    head,sizeof head,&hn)!=1)continue;
            }
        }
        if (join(left->spr,left->spr_length,head,hn,
            roles&ROOT_WEAK_HEAD?0:2,"",selected,&selected_length)) goto found;
    }

    /* A final constituent can establish a boundary even when the left
     * spelling has no pre-enumerated ROOT_LEFT role. Analyse both shorter
     * constituents through the same paths, retaining the right root's
     * internal prefixes and derivations. */
    if(!selected_length) {
        static const char *const weak[]={"be","ge","ver","er","ent","zer"};
        int eligible=1;
        /* These whole-root attempts are shared by all candidate splits. */
        if(reloq_german_g2p_word(key,length,candidate,sizeof candidate,&n)==1 ||
           reloq_german_root_plan_word(lexicon,key,length,candidate,sizeof candidate,&n)==1)eligible=0;
        for(size_t k=0;k<sizeof weak/sizeof *weak;++k) {
            size_t pn=strlen(weak[k]);
            if(length>pn+2&&!memcmp(key,weak[k],pn)&&
               weak_prefix_onset(key,length,pn))eligible=0;
        }
        for(size_t k=0;k<sizeof prefixes/sizeof *prefixes;++k) {
            size_t pn=strlen(prefixes[k].word);
            /* Variable um- also has an unstressed root parse. Merely
             * matching its spelling cannot exclude that compound path. */
            if(strcmp(prefixes[k].word,"um") && length>pn+2 &&
               !memcmp(key,prefixes[k].word,pn) && key[pn]==key[pn-1])eligible=0;
        }
        if(eligible)for(size_t split=3;split+3<=length;++split) {
            if(vowel(key[split]) && !(key[split-1]=='o'&&key[split]=='u'))continue;
            if(!final_root_class(key+split,length-split))continue;
            /* An isolated weak -er- or finite dental form does not
             * establish its compound-root quantity. Repeated unresolved
             * letters can make the original reject a matched final root. */
            if((!memcmp(key+split-2,"er",2)&&(compound_class(key,split)&1)) ||
               (key[split-1]=='t'&&strchr("bdgkp",key[split-2])))continue;
            int repeated=0;
            for(size_t j=2;j<split;++j)
                if(key[j]==key[j-1]&&key[j]==key[j-2])repeated=1;
            if(repeated)continue;
            const struct reloq_lexicon_override *entry=NULL;
            /* A caller-altered pronunciation does not inherit a measured
             * constituent role merely from its spelling. */
            if(spelling_roles(key,split) && !component(lexicon,key,split,&entry) &&
               reloq_lexicon_find_override(lexicon,key,split,&entry)==1)continue;
            if(spelling_roles(key+split,length-split) &&
               !component(lexicon,key+split,length-split,&entry) &&
               reloq_lexicon_find_override(lexicon,key+split,length-split,&entry)==1)continue;
            unsigned char left[SPR_LIMIT],right[SPR_LIMIT];size_t ln=0,rn=0;
            if(!constituent_reading(lexicon,key+split,length-split,right,&rn,0)||
               !constituent_reading(lexicon,key,split,left,&ln,1))continue;
            if(join(left,ln,right,rn,2,"",selected,&selected_length))goto found;
        }
    }

    /* compound_con_cluster also establishes a boundary without a lexical
     * constituent hit. A legal two-consonant nasal coda is separated from
     * a following labial, liquid or h onset. Dental/velar continuations
     * retain the intrastem assimilation path. Both parts are then analysed
     * as complete domains, before syllabic-sonorant generation. */
    if(!selected_length && cluster_compound(lexicon,key,length,selected,&selected_length))goto found;
    if(!selected_length)for(size_t split=4;split+3<=length;++split) {
        if(key[split-1]!='n' || vowel(key[split-2]) || !vowel(key[split-3]) ||
           !strchr(key[split-2]=='n'?"bdfghklmnprwz":"bfhlmprw",key[split]) ||
           !reloq_german_root_coda_legal(key+split-2,2))continue;
        size_t head_end=length;
        if(head_end>split+2 && key[head_end-2]=='e' &&
           strchr("nmrs",key[head_end-1]))head_end-=2;
        else if(key[head_end-1]=='e')--head_end;
        size_t nuclei=0;
        for(size_t k=split;k<head_end;++k)nuclei+=(size_t)vowel(key[k]);
        if(!nuclei)continue;
        unsigned char left[SPR_LIMIT],right[SPR_LIMIT];size_t ln=0,rn=0;
        if(!constituent_reading(lexicon,key,split,left,&ln,1) ||
           !constituent_reading(lexicon,key+split,length-split,right,&rn,2))continue;
        const struct reloq_lexicon_override *entry=NULL;
        unsigned roles=component(lexicon,key+split,length-split,&entry);
        if(join(left,ln,right,rn,roles&ROOT_WEAK_HEAD?0:2,"",selected,&selected_length))break;
    }

    if(!selected_length)for(size_t split=4;split+3<=length;++split) {
        if(vowel(key[split-1]) || vowel(key[split]) ||
           key[split-1]!=key[split-2] || key[split-2]==key[split-3])continue;
        unsigned char left[SPR_LIMIT],right[SPR_LIMIT];size_t ln=0,rn=0;
        if(reloq_german_root_plan_word(lexicon,key,split,left,sizeof left,&ln)!=1 ||
           !constituent_reading(lexicon,key+split,length-split,right,&rn,0))continue;
        if(join(left,ln,right,rn,2,"",selected,&selected_length))break;
    }

found:
    /* A remaining particle domain can itself contain an unresolved
     * consonant-boundary compound. Retry only after complete root and
     * compound readings: those may establish a longer indivisible left
     * constituent before the tempting particle letters. */
    if(!selected_length)for(size_t k=0;k<sizeof prefixes/sizeof *prefixes;++k) {
        size_t lead=strlen(prefixes[k].word);
        if(length<lead+6 || memcmp(key,prefixes[k].word,lead))continue;
        int longer=0;
        for(size_t j=0;j<k;++j) {
            size_t z=strlen(prefixes[j].word);
            if(z>lead && length>z && !memcmp(key,prefixes[j].word,z))longer=1;
        }
        if(longer)continue;
        unsigned char composed[SPR_LIMIT];size_t cn=0;
        if(!cluster_compound(lexicon,key+lead,length-lead,composed,&cn))continue;
        if(reloq_german_lexicon_word(lexicon,key+lead,length-lead,
            candidate,sizeof candidate,&n)!=1)continue;
        if(join((const unsigned char *)prefixes[k].spr,strlen(prefixes[k].spr),
            candidate,n,2,"",selected,&selected_length))break;
    }
    if(!selected_length && retry_weak_prefix(lexicon,key,length,selected,&selected_length))goto complete;
    /* The outer genitive does not reanalyse an established weak ending.
     * The same stem-preserving path covers -er nouns and infinitives:
     * retain their reduced vowel, consonant voicing and lexical accent. */
    if(!selected_length && length>4 && key[length-1]=='s' &&
       ((key[length-2]=='r' && key[length-3]=='e') ||
        (key[length-2]=='n' && (key[length-3]=='e' ||
         (length>5 && key[length-4]=='e' &&
          (key[length-3]=='l'||key[length-3]=='r')))))) {
        if(reloq_german_lexicon_word(lexicon,key,length-1,candidate,
           sizeof candidate,&n)==1 && n>4 && n+1<SPR_LIMIT &&
           (memchr(candidate,'@',n)||memchr(candidate,'R',n))) {
            memcpy(selected,candidate,n-1);memcpy(selected+n-1,"s]",3);
            selected_length=n+1;
        }
    }
complete:
    if (!selected_length) return 0;
    /* Marked compounds retain the right constituent's primary accent.
     * This is the original class used by compound_stress_adjustment;
     * no example word has a separate pronunciation path. Applying it to
     * each completed inner compound also preserves it in larger nouns. */
    static const char *const marked[]={
#include "reloq_german_compound_accent74.inc"
    };
    size_t accent_root=length;
    static const char *const accent_endings[]={"ens","en","em","er","es","e","s"};
    for(size_t k=0;k<sizeof accent_endings/sizeof *accent_endings;++k) {
        size_t size=strlen(accent_endings[k]);
        if(length>size+2 && !memcmp(key+length-size,accent_endings[k],size) &&
           !(size==1 && key[length-1]=='s' && key[length-2]=='s')) {
            accent_root-=size;break;
        }
    }
    /* strip_final_root_syll precedes the membership lookup. In an -el
     * analysis those letters belong to the weak suffix, not the key. */
    if(accent_root>4 && !memcmp(key+accent_root-2,"el",2))accent_root-=2;
    for(size_t k=0;k<sizeof marked/sizeof *marked;++k) {
        size_t size=strlen(marked[k]);
        if(accent_root!=size || memcmp(key,marked[k],size))continue;
        size_t first=0;
        for(size_t j=2;j+1<selected_length;++j)if(selected[j]=='.') {
            if(selected[j+1]=='2')break; /* Inner marked compound already resolved. */
            if(selected[j+1]=='1') {first=j;break;}
        }
        if(first)for(size_t j=first+2;j+1<selected_length;++j) {
            if(selected[j]=='.' && selected[j+1]=='2') {
                selected[first+1]='2';selected[j+1]='1';break;
            }
        }
        break;
    }
    if (!output || capacity<=selected_length) {*byte_length=selected_length;return -2;}
    memmove(output,selected,selected_length+1);*byte_length=selected_length;return 1;
}

int reloq_german_morphology_observed_root(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t n,const struct reloq_lexicon_override **entry)
{
 return roles_for(roots,sizeof roots/sizeof roots[0],lexicon,word,n,entry)!=0 ||
  roles_for(inflections,sizeof inflections/sizeof inflections[0],lexicon,word,n,entry)!=0;
}

void reloq_german_morphology_repair_stem(const unsigned char *word,size_t length,
 unsigned char *spr,size_t size)
{
 static const unsigned char stem[]="konzentrier";
 if(!word || !spr || length<sizeof stem-1)return;
 int matched=0;
 for(size_t i=0;i+sizeof stem-1<=length && !matched;++i) {
  size_t k=0;
  while(k<sizeof stem-1 && lower(word[i+k])==stem[k])++k;
  matched=k==sizeof stem-1;
 }
 if(!matched)return;
 /* The inherited analysis reduces zen to /ts@n/ and puts the main root
  * accent on kon. Correct that root alone, including its secondary-accent
  * form under un-/auf-/hoch-. Inflection and vocalic/consonantal r stay
  * with the ordinary morphological analysis. Already corrected or supplied
  * pronunciations with a different phoneme sequence do not match. */
 for(size_t i=0;i+16<=size;++i) {
  if(spr[i]!='.' || spr[i+1]<'0' || spr[i+1]>'2' ||
     memcmp(spr+i+2,"kOn.0T@n.",9) || spr[i+11]<'0' || spr[i+11]>'2' ||
     memcmp(spr+i+12,"tri",3))continue;
  unsigned char accent=spr[i+1]>spr[i+11]?spr[i+1]:spr[i+11];
  /* Primary outranks secondary although its notation is numerically lower. */
  if(spr[i+1]=='1' || spr[i+11]=='1')accent='1';
  spr[i+1]='0';spr[i+8]='E';spr[i+11]=accent;
 }
}

void reloq_german_morphology_nominal_stress(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t length,unsigned char *spr,size_t size)
{
 if(!lexicon || length>=WORD_LIMIT || size<14)return;
 size_t lead=0;const char *pattern=NULL;
 if(length>7 && !memcmp(word,"Unter",5)) {lead=5;pattern="`[.0Un.0tR";}
 else if(length>6 && !memcmp(word,"\334ber",4)) {lead=4;pattern="`[.0y.0bR";}
 if(!lead)return;
 size_t prefix=strlen(pattern);
 if(size<=prefix || memcmp(spr,pattern,prefix))return;
 unsigned char head[WORD_LIMIT],analysis[SPR_LIMIT];size_t count=0;
 for(size_t i=lead;i<length;++i)head[i-lead]=lower(word[i]);
 size_t n=length-lead;head[n]=0;
 /* A finite/infinitive/participle analysis is affirmative evidence. In
  * ambiguous nominal/verb spellings we deliberately retain that evidence. */
 int verb=en_form(lexicon,head,n,analysis,&count,0)==1;
 /* A dental ending on a separately licensed inflection stem is
  * affirmative evidence even when that stem is also a noun (zeug+t).
  * The final t of a bare noun such as richt is not such an ending. */
 const struct reloq_lexicon_override *dental_entry=NULL;
 if(n>3 && head[n-1]=='t' &&
    (roles_for(inflections,sizeof inflections/sizeof *inflections,
        lexicon,head,n-1,&dental_entry)&1u))return;
 /* A contextual spelling fallback is not lexical evidence of a verb.
  * Weak nouns additionally license an -n plural in the existing observed
  * inflection classes. Retain that nominal stem before deciding accent. */
 const struct reloq_lexicon_override *nominal_entry=NULL;
 size_t nominal=n;
 if(n>3 && head[n-1]=='n' && head[n-2]=='e')nominal=n-1;
 int weak_noun=nominal>2 && head[nominal-1]=='e' &&
     (roles_for(inflections,sizeof inflections/sizeof *inflections,
         lexicon,head,nominal,&nominal_entry)&2u);
 int plural_noun=n>4 && head[n-2]=='e' && head[n-1]=='n' &&
     (roles_for(inflections,sizeof inflections/sizeof *inflections,
         lexicon,head,n-2,&nominal_entry)&8u) &&
     (spelling_roles(head,n-2)&ROOT_HEAD);
 if(n>=4 && head[n-2]=='e' && head[n-1]=='n' && !plural_noun)return;
 /* Case alone does not nominalize a verbal ending. Keep the analyzed
  * root accent before applying the requested bare-noun extension. */
 if(!weak_noun && !plural_noun && ((n>=4 && head[n-2]=='e' && head[n-1]=='n') ||
    (verb && (head[n-1]=='e' || (n>3 && head[n-2]=='s' && head[n-1]=='t')))))return;
 size_t domain=plural_noun?n-2:weak_noun?nominal:n;
 static const char *const endings[]={"en","es","e","s"};
 for(size_t i=0;i<sizeof endings/sizeof *endings;++i) {
  size_t z=strlen(endings[i]);
  if(domain>z+2&&!memcmp(head+domain-z,endings[i],z)) {domain-=z;break;}
 }
 if(vowel(head[domain-1]))return;
 unsigned nuclei=0;int inside=0;
 for(size_t i=0;i<domain;++i) {
  int v=vowel(head[i]);if(v&&!inside)++nuclei;inside=v;
 }
 if(nuclei!=1)return;
 /* Finite-only contextual roots cannot become a nominal constituent just
  * because NVDA or a sentence starts with a capital letter. */
 unsigned roles=spelling_roles(head,domain);
 unsigned char infinitive[WORD_LIMIT];
 unsigned bare_stem=0;
 if(n+2<WORD_LIMIT) {
  memcpy(infinitive,head,n);memcpy(infinitive+n,"en",3);
  const struct reloq_lexicon_override *entry=NULL;
  bare_stem=component(lexicon,infinitive,n+2,&entry)&ROOT_PREFIX;
 }
 if(!weak_noun && verb && !(roles&(ROOT_HEAD|ROOT_LEFT)) && !bare_stem)return;
 if(!weak_noun && (roles&ROOT_PARTICLE)&&!(roles&(ROOT_HEAD|ROOT_LEFT)))return;
 int primary=0;
 for(size_t i=prefix;i+1<size;++i)if(spr[i]=='.'&&spr[i+1]=='1')primary=1;
 if(!primary)return;
 spr[3]='1';
 for(size_t i=prefix;i+1<size;++i)
  if(spr[i]=='.'&&spr[i+1]=='1')spr[i+1]='2';
}

int reloq_german_morphology_user_compound(const struct reloq_lexicon *lexicon,
 const unsigned char *word,size_t length,unsigned char *out,size_t capacity,size_t *written)
{
 if(!lexicon||!word||!written||length>=WORD_LIMIT||length<7)return 0;
 unsigned char key[WORD_LIMIT];
 for(size_t i=0;i<length;++i)key[i]=lower(word[i]);
 key[length]=0;
 for(size_t split=length-3;split>=3;--split) {
  if(key[split-2]!='c'||key[split-1]!='h'||!vowel(key[split]))continue;
  const struct reloq_lexicon_override *left=NULL;
  if(!(component(lexicon,key,split,&left)&ROOT_LEFT))continue;
  /* A measured noun head establishes this requested boundary directly.
     Re-running the full ambiguous whole-word morphology here would repeat
     its expensive competing splits before arriving at the same compound. */
  unsigned char joined[SPR_LIMIT],head[SPR_LIMIT];size_t n=0,head_n=0,hn=length-split;
  const struct reloq_lexicon_override *noun=NULL;
  int admitted=!!(compound_class(key+split,hn)&16u);
  if(!admitted && hn>4 && !memcmp(key+length-2,"en",2))
   admitted=(compound_class(key+split,hn-2)&16u) &&
     (roles_for(inflections,sizeof inflections/sizeof *inflections,
         lexicon,key+split,hn-2,&noun)&8u);
  if(!admitted || reloq_german_lexicon_word(lexicon,key+split,hn,head,sizeof head,&head_n)!=1 ||
     !join(left->spr,left->spr_length,head,head_n,2,"",joined,&n))continue;
  *written=n;
  if(!out||capacity<=n)return -2;
  memcpy(out,joined,n+1);return 1;
 }
 return 0;
}
