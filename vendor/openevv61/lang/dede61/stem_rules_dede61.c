#include "reloq61_spr_output.h"
#include "reloq61_german_final.h"
#include "reloq_lexicon.h"
#include <string.h>
extern const struct reloq_lexicon *reloq61_dictionary_overlay_current(void);
extern const struct reloq_lexicon_resolver *reloq61_dictionary_resolver(void);
extern void reloq61_dictionary_resolver_release(const struct reloq_lexicon_resolver *);
static const unsigned char alphabet[]={
#include "reloq61_german_alphabet.inc"
};
static _Thread_local struct reloq61_german_final_a final_a;
struct reloq61_german_final_a reloq61_german_final_scope(struct reloq61_german_final_a next)
{struct reloq61_german_final_a previous=final_a;final_a=next;return previous;}
void reloq61_german_final_note(void *state,int32_t phone)
{final_a=(struct reloq61_german_final_a){state,phone};}
int reloq61_german_final_matches(void *state,int32_t phone)
{return final_a.state==state&&final_a.phone==phone&&phone!=0;}
static int32_t left_of(const struct reloq61_spr_phone *p)
{return *EVV_AT(const int32_t *,p->node)&~3;}
static int begins_syllable(delta_state *d,const struct reloq61_spr_phone *p)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    return (*EVV_AT(const int32_t *,left_of(p)+(v->fence_base+7)*4)&1)!=0;
}
static int syllable_stress(delta_state *d,const struct reloq61_spr_phone *p)
{
    int32_t node=vgetsc(d,0,0,left_of(p),7);
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,0,0,node,7);
    if(!node)return -1;
    int16_t stress;memcpy(&stress,delta_record_field(&vstmtbl[7],0,TFLDS(EVV_AT(void *,node))),2);
    return stress;
}
static int input_symbol(delta_state *d,const struct reloq61_spr_phone *p)
{
    /* A phone at syllable onset also covers the leading .0/.1/.2 syntax.
     * Walk its input span rather than mistaking that marker for the vowel. */
    int32_t at=left_of(p);
    while(at&&at!=p->after) {
        int32_t node=vgetsc(d,0,0,at,1);
        if(!node)return -1;
        if(*EVV_AT(const int32_t *,node)&2){at=node;continue;}
        unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
        int c=name<sizeof alphabet?alphabet[name]:-1;
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))return c;
        int32_t next=EVV_AT(const int32_t *,node)[1]&~3;
        if(next==at)break;
        at=next;
    }
    return -1;
}
static void put_phone(delta_state *d,const struct reloq61_spr_phone *p,uint8_t name)
{
    unsigned char record[20];vinitflds(d,2,record,&name);
    for(unsigned field=0;field<=11;++field)
        vmark(d,2,(uint8_t)field,left_of(p),p->after,
              delta_record_field(&vstmtbl[2],field,record));
}
static int ascii_letter(int c)
{return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c>=0xc0;}
static int input_at(delta_state *d,int32_t *at,int back)
{
    int32_t node=vgetsc(d,back,0,*at,1);
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,back,0,node,1);
    if(!node)return -1;
    unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
    *at=*EVV_AT(const int32_t *,node+(back?0:4))&~3;
    return name<sizeof alphabet?alphabet[name]:-1;
}
/* A phone's input projection can start before its own grapheme. For example,
 * the final vowel in Kamera can share the span for "ra": inspecting only
 * the first letter sees r, not the final a. Keep phonetic uppercase A
 * distinct from a normal-text lowercase a. */
static int last_input_symbol(delta_state *d,const struct reloq61_spr_phone *p)
{
    int32_t at=p->after,left=left_of(p);
    for(unsigned i=0;i<128&&at&&at!=left;++i) {
        int32_t previous=at;int c=input_at(d,&at,1);
        if(c<0||at==previous)break;
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))return c;
    }
    return -1;
}
static size_t word_spelling(delta_state *d,int32_t word_begin,unsigned char spelling[128],int *initial)
{
    int32_t start=word_begin,scan=start;
    for(unsigned i=0;i<127;++i) {
        int32_t previous=scan;int c=input_at(d,&previous,1);
        if(!ascii_letter(c))break;
        start=scan=previous;
    }
    int32_t first=vgetsc(d,0,0,start,1);
    while(first&&(*EVV_AT(const int32_t *,first)&2))first=vgetsc(d,0,0,first,1);
    unsigned first_case=first?*(const unsigned char *)delta_record_field(&vstmtbl[1],1,TFLDS(EVV_AT(void *,first))):0;
    size_t n=0;scan=start;*initial=0;
    while(n<127) {
        int c=input_at(d,&scan,0);if(!ascii_letter(c))break;
        if(!n)*initial=first_case==2&&((c>='a'&&c<='z')||c==0xe4||c==0xf6||c==0xfc)?c-32:c;
        spelling[n++]=(unsigned char)((c>='A'&&c<='Z')||c==0xc4||c==0xd6||c==0xdc?c+32:c);
    }
    spelling[n]=0;return n;
}
static void set_syllable_stress(delta_state *d,const struct reloq61_spr_phone *p,int16_t stress)
{
    int32_t syll=vgetsc(d,0,0,left_of(p),7);
    while(syll&&(*EVV_AT(const int32_t *,syll)&2))syll=vgetsc(d,0,0,syll,7);
    if(syll) {
        int32_t l=*EVV_AT(const int32_t *,syll)&~3;
        int32_t r=EVV_AT(const int32_t *,syll)[1]&~3;
        vmark(d,7,0,l,r,&stress);
    }
}
static void set_quantity(delta_state *d,const struct reloq61_spr_phone *p,uint8_t value)
{vmark(d,2,10,left_of(p),p->after,&value);}
static void repair_konzentrier(delta_state *d,int32_t word_begin,int32_t word_end,
    const unsigned char *spelling)
{
    if(!strstr((const char *)spelling,"konzentrier"))return;
    struct reloq61_spr_phone p[64];size_t count=0;int32_t scan=word_begin;
    while(scan&&scan!=word_end&&count<64&&reloq61_spr_phone_at(d,scan,&p[count]))
        scan=p[count++].after;
    for(size_t i=0;i+5<count;++i) {
        static const unsigned char pattern[]={5,38,22,17,43,22};
        unsigned j=0;
        while(j<6&&reloq61_spr_feature(d,p+i+j,0)==pattern[j])++j;
        if(j!=6)continue;
        put_phone(d,p+i+4,31);
        set_syllable_stress(d,p+i,0);
        return;
    }
}

/* Keep the voiced onset of the independent word consistent with its
 * productive forms such as samtig. Explicit phonetic input is left alone. */
static int word_has_user_entry(delta_state *d,int32_t word_begin,size_t length)
{
    /* Recover each character's case only for this word. The native spelling
     * scanner normalizes the letters, but an RLEX lookup is case-sensitive. */
    int32_t start=word_begin;
    if(!length||length>127)return 0;
    for(size_t i=0;i<length;++i) {
        int32_t previous=start;int c=input_at(d,&previous,1);
        if(!ascii_letter(c))break;
        start=previous;
    }
    unsigned char key[128];int32_t scan=start;
    for(size_t i=0;i<length;++i) {
        int32_t node=vgetsc(d,0,0,scan,1);
        while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,0,0,node,1);
        unsigned letter_case=node?*(const unsigned char *)delta_record_field(&vstmtbl[1],1,TFLDS(EVV_AT(void *,node))):0;
        int c=input_at(d,&scan,0);
        if(c<0)return 0;
        key[i]=(unsigned char)(letter_case==2&&c>='a'&&c<='z'?c-32:c);
    }
    key[length]=0;
    const struct reloq_lexicon *overlay=reloq61_dictionary_overlay_current();
    if(overlay) {
        size_t low=0,high=overlay->override_count;
        while(low<high) {
            size_t middle=low+(high-low)/2;
            const struct reloq_lexicon_override *entry=overlay->overrides+middle;
            size_t common=entry->key_length<length?entry->key_length:length;
            int order=memcmp(entry->key,key,common);
            if(!order)order=(entry->key_length>length)-(entry->key_length<length);
            if(!order)return 1;
            if(order<0)low=middle+1;else high=middle;
        }
    }
    const struct reloq_lexicon_resolver *resolver=reloq61_dictionary_resolver();
    int found=resolver&&resolver->find&&resolver->find(resolver->context,key,length)!=NULL;
    reloq61_dictionary_resolver_release(resolver);
    return found;
}
static void repair_samt(delta_state *d,int32_t begin,int32_t end,
    const unsigned char *spelling,int phonetic)
{
    struct reloq61_spr_phone s,a,m,t;
    if(phonetic||strcmp((const char *)spelling,"samt")||
       word_has_user_entry(d,begin,4)||
       !reloq61_spr_phone_at(d,begin,&s)||reloq61_spr_feature(d,&s,0)!=9||
       !reloq61_spr_phone_at(d,s.after,&a)||reloq61_spr_feature(d,&a,0)!=34||
       !reloq61_spr_phone_at(d,a.after,&m)||reloq61_spr_feature(d,&m,0)!=21||
       !reloq61_spr_phone_at(d,m.after,&t)||reloq61_spr_feature(d,&t,0)!=3||t.after!=end)return;
    put_phone(d,&s,10);
}

/* The built-in entry for Touch has the long German a of Tatsch. English
 * touch uses the short German a here. Leave explicit SPR and custom entries
 * untouched and only change the native touch phone sequence. */
static void repair_touch(delta_state *d,int32_t begin,int32_t end,
    const unsigned char *spelling,int phonetic)
{
    struct reloq61_spr_phone t,a,ch;
    size_t length=strlen((const char *)spelling);
    if(phonetic||length<5||memcmp(spelling,"touch",5)||
       word_has_user_entry(d,begin,length)||word_has_user_entry(d,begin,5)||
       !reloq61_spr_phone_at(d,begin,&t)||reloq61_spr_feature(d,&t,0)!=3||
       !reloq61_spr_phone_at(d,t.after,&a)||reloq61_spr_feature(d,&a,0)!=33||
       !reloq61_spr_phone_at(d,a.after,&ch)||reloq61_spr_feature(d,&ch,0)!=18||
       ch.after==a.after||a.after==end)return;
    put_phone(d,&a,34);
}

/* The participle and its inflections must use the same reduced vowel in
 * öffentlich as veröffentlichen and Veröffentlichung. Match the native
 * phone sequence as well as the written stem, leaving other E sounds alone. */
static void repair_oeffentlich(delta_state *d,int32_t begin,int32_t end,
    const unsigned char *spelling,int phonetic)
{
    if(phonetic||!strstr((const char *)spelling,"\366ffentlich"))return;
    struct reloq61_spr_phone p[64];size_t count=0;int32_t at=begin;
    while(at&&at!=end&&count<64&&reloq61_spr_phone_at(d,at,&p[count])) {
        if(p[count].after==at)break;
        at=p[count++].after;
    }
    if(at!=end)return;
    static const unsigned char pattern[]={42,7,31,22,3,24,29,13};
    for(size_t i=0;i+sizeof pattern<=count;++i) {
        size_t j=0;while(j<sizeof pattern&&reloq61_spr_feature(d,p+i+j,0)==pattern[j])++j;
        if(j==sizeof pattern&&input_symbol(d,p+i+2)=='e')put_phone(d,p+i+2,43);
    }
}

/* In connected 81..89, the old planner adds a glottal onset to the A of
 * achtzig, independently of the onset on und. The explicit SPR form already
 * carries this onset. Restrict the repair to the complete /unt a x ts I x/
 * sequence, which also handles written and embedded numeric forms. */
static void repair_eighty_onset(delta_state *d,int32_t begin,int32_t end)
{
    struct reloq61_spr_phone p[64];size_t count=0;int32_t at=begin;
    while(at&&at!=end&&count<64&&reloq61_spr_phone_at(d,at,&p[count])) {
        if(p[count].after==at)break;
        at=p[count++].after;
    }
    if(at!=end)return;
    static const unsigned char pattern[]={36,22,3,34,14,17,29,13};
    for(size_t i=0;i+sizeof pattern<=count;++i) {
        size_t j=0;while(j<sizeof pattern&&reloq61_spr_feature(d,p+i+j,0)==pattern[j])++j;
        if(j==sizeof pattern&&begins_syllable(d,p+i+3)) {
            uint8_t onset=2;vmark(d,2,15,left_of(p+i+3),p[i+3].after,&onset);
        }
    }
}
static void note_final_a(delta_state *d,int32_t begin,int32_t end)
{
    int32_t at=begin;unsigned syllables=0;int stress=-1;
    struct reloq61_spr_phone p,last={0};
    while(at&&at!=end&&reloq61_spr_phone_at(d,at,&p)) {
        int next_stress=syllable_stress(d,&p);
        if(next_stress>=0){stress=next_stress;++syllables;}
        last=p;
        if(p.after==at)break;
        at=p.after;
    }
    if(at!=end||syllables<2||stress!=0||reloq61_spr_feature(d,&last,0)!=34||
       (input_symbol(d,&last)!='a'&&last_input_symbol(d,&last)!='a'))return;
    int32_t scan=last.after;
    for(unsigned i=0;i<4096&&scan;++i) {
        int c=input_at(d,&scan,0);
        if(c<0){scan=0;break;}
        /* A full sentence boundary ends this vowel's immediate context,
         * even when another sentence follows in the same request. */
        if(c=='?'||c=='!') {scan=0;break;}
        if(c=='.') {
            int32_t following=scan;int next=input_at(d,&following,0);
            /* Do not treat a dot inside a domain or decimal-like token as
             * the end of a sentence merely because it follows an a. */
            if(next<0||(!ascii_letter(next)&&!(next>='0'&&next<='9'))) {scan=0;break;}
        }
        if(ascii_letter(c)||(c>='0'&&c<='9'))return;
    }
    if(scan)return;
    reloq61_german_final_note(d,last.node);
}
/* The previous number planner connected und without a glottal break for
 * these five units. Five, six and eight intentionally retain the break. */
static void repair_number_onset(delta_state *d,int32_t begin,int32_t end,
    const unsigned char *spelling)
{
    static const char *const units[]={"zweiund","dreiund","vierund","siebenund","neunund"};
    static const char *const tens[]={"zwanzig","drei\337ig","vierzig","f\374nfzig","sechzig","siebzig","achtzig","neunzig"};
    int numeric=0;
    for(size_t i=0;i<sizeof units/sizeof *units&&!numeric;++i) {
        size_t n=strlen(units[i]);
        if(strncmp((const char *)spelling,units[i],n))continue;
        if(!spelling[n])numeric=1;
        for(size_t j=0;j<sizeof tens/sizeof *tens;++j) {
            size_t length=strlen(tens[j]);
            if(!strncmp((const char *)spelling+n,tens[j],length)) {
                const char *ending=(const char *)spelling+n+length;
                if(!*ending||!strncmp(ending,"tausend",7)||!strcmp(ending,"ste")||!strcmp(ending,"sten")||
                   !strcmp(ending,"ster")||!strcmp(ending,"stes")||!strcmp(ending,"stem"))numeric=1;
            }
        }
    }
    if(!numeric)return;
    struct reloq61_spr_phone u,n,t;
    if(reloq61_spr_phone_at(d,begin,&u)&&u.after!=end&&
       reloq61_spr_feature(d,&u,0)==36&&reloq61_spr_feature(d,&u,15)==2&&
       input_symbol(d,&u)=='u'&&reloq61_spr_phone_at(d,u.after,&n)&&
       reloq61_spr_feature(d,&n,0)==22&&reloq61_spr_phone_at(d,n.after,&t)&&
       reloq61_spr_feature(d,&t,0)==3) {
        uint8_t no_onset=0;
        vmark(d,2,15,left_of(&u),u.after,&no_onset);
    }
}
/* Make four's r a consonant when it immediately precedes und. The
 * source spelling is kept intact, including dictionary and index mapping. */
static void repair_four_liaison(delta_state *d,int32_t begin,int32_t end,
    const unsigned char *spelling)
{
    if(strncmp((const char *)spelling,"vierund",7))return;
    struct reloq61_spr_phone f,i,r;
    if(!reloq61_spr_phone_at(d,begin,&f)||reloq61_spr_feature(d,&f,0)!=7||
       !reloq61_spr_phone_at(d,f.after,&i)||reloq61_spr_feature(d,&i,0)!=28||
       !reloq61_spr_phone_at(d,i.after,&r)||reloq61_spr_feature(d,&r,0)!=25||
       r.after!=end)return;
    put_phone(d,&r,15);
}
/* Long /o/ in a closed -os syllable was kept full in the old planner.
 * Short /O/ is a different phone and must retain its original quantity. */
static void repair_closed_o(delta_state *d,int32_t begin,int32_t end)
{
    int32_t at=begin;struct reloq61_spr_phone p,next;
    while(at&&at!=end&&reloq61_spr_phone_at(d,at,&p)) {
        if(reloq61_spr_feature(d,&p,0)==37&&reloq61_spr_feature(d,&p,10)==2&&
           p.after!=end&&reloq61_spr_phone_at(d,p.after,&next)&&
           reloq61_spr_feature(d,&next,0)==9&&!begins_syllable(d,&next))
            set_quantity(d,&p,1);
        if(p.after==at)break;
        at=p.after;
    }
}
/* These two recorded adjective roots are not nominal -s plurals. Their
 * inflected native forms already have the requested root stress. */
static void repair_os_adjective(delta_state *d,int32_t begin,int32_t end,
    const unsigned char *spelling)
{
    int famos=!strcmp((const char *)spelling,"famos");
    int rigoros=!strcmp((const char *)spelling,"rigoros");
    if(!famos&&!rigoros)return;
    struct reloq61_spr_phone p[16];size_t n=0;int32_t at=begin;
    while(at&&at!=end&&n<16&&reloq61_spr_phone_at(d,at,&p[n]))at=p[n++].after;
    if(famos&&n==5&&reloq61_spr_feature(d,p,0)==7&&
       reloq61_spr_feature(d,p+1,0)==33&&reloq61_spr_feature(d,p+2,0)==21&&
       reloq61_spr_feature(d,p+3,0)==37&&reloq61_spr_feature(d,p+4,0)==9) {
        put_phone(d,p+1,34);set_syllable_stress(d,p,0);
        set_syllable_stress(d,p+2,1);set_quantity(d,p+3,1);
    } else if(rigoros&&n==7&&reloq61_spr_feature(d,p,0)==15&&
       reloq61_spr_feature(d,p+1,0)==28&&reloq61_spr_feature(d,p+2,0)==6&&
       reloq61_spr_feature(d,p+3,0)==37&&reloq61_spr_feature(d,p+4,0)==15&&
       reloq61_spr_feature(d,p+5,0)==37&&reloq61_spr_feature(d,p+6,0)==9) {
        set_syllable_stress(d,p,2);set_quantity(d,p+1,2);
        set_syllable_stress(d,p+4,1);set_quantity(d,p+5,1);
    }
}

/* Bare, capitalized nominal forms of these productive prefix families
 * carried prefix stress in the previous German planner. Verbal -en forms
 * and compounds with a longer head keep the native stress. */
static int nominal_prefix_head(const char *head,int over)
{
    static const char *const under[]={"arm","arzt","druck","gang","g\344ng","grenz",
        "grund","halt","holz","hos","lag","last","leib","lipp","mann","punkt",
        "rang","richt","rock","satz","schicht","schied","stuf","text","ton","welt","woll","zahl"};
    static const char *const upper[]={"all","druck","fahrt","fall","f\344ll","flug",
        "fluss","gab","griff","gr\366\337","hand","hang","hitz","last","lauf","l\344uf","l\344uft",
        "macht","mensch","mut","nahm","nam","rest","rund","satz","schall",
        "schrift","sicht","trag","tritt","weg","zeugt"};
    const char *const *roots=over?upper:under;
    size_t count=over?sizeof upper/sizeof *upper:sizeof under/sizeof *under;
    static const char *const endings[]={"","s","es","e"};
    for(size_t i=0;i<count;++i) {
        size_t n=strlen(roots[i]);
        if(strncmp(head,roots[i],n))continue;
        for(size_t j=0;j<sizeof endings/sizeof *endings;++j)
            if(!strcmp(head+n,endings[j]))return 1;
    }
    return 0;
}
static void repair_nominal_prefix(delta_state *d,int32_t begin,int32_t end,
    const unsigned char *spelling,int initial)
{
    int over=initial==0xdc&&!strncmp((const char *)spelling,"\374ber",4);
    int under=initial=='U'&&!strncmp((const char *)spelling,"unter",5);
    if((!over&&!under)||!nominal_prefix_head((const char *)spelling+(over?4:5),over))return;
    struct reloq61_spr_phone p[64];size_t count=0;int32_t at=begin;
    while(at&&at!=end&&count<64&&reloq61_spr_phone_at(d,at,&p[count]))at=p[count++].after;
    if(at!=end||count<4||reloq61_spr_feature(d,p,0)!=(over?39u:36u)||
       reloq61_spr_feature(d,p+1,0)!=(over?2u:22u)||syllable_stress(d,p)!=0)return;
    int found=0;
    for(size_t i=1;i<count;++i)
        if(syllable_stress(d,p+i)==1){set_syllable_stress(d,p+i,2);found=1;}
    if(found)set_syllable_stress(d,p,1);
}
/* Native compound analysis can split ch as c + h when a following
 * constituent starts with aus-. Repair that particular phone boundary
 * only when the spelling and the k-h-au graph both establish the split. */
static void repair_ch_compound(delta_state *d,int32_t begin,int32_t end,
    const unsigned char *spelling)
{
    static const char *const roots[]={"sprach","such","fach","bauch"};
    size_t root=0;int short_a=0;
    for(size_t i=0;i<sizeof roots/sizeof *roots;++i) {
        size_t n=strlen(roots[i]);
        if(!strncmp((const char *)spelling,roots[i],n)&&
           !strncmp((const char *)spelling+n,"auswahl",7)) {
            root=n;short_a=i==2;break;
        }
    }
    if(!root)return;
    struct reloq61_spr_phone p[32];size_t count=0;int32_t at=begin;
    while(at&&at!=end&&count<32&&reloq61_spr_phone_at(d,at,&p[count]))at=p[count++].after;
    if(!count||at!=end)return;
    /* The two constituents are generated separately. Their boundary is
     * already visible in spelling, but the later phones do not exist yet. */
    if(reloq61_spr_feature(d,p+count-1,0)==5 && input_symbol(d,p+count-1)=='c') {
        if(short_a&&count>=2&&reloq61_spr_feature(d,p+count-2,0)==33)
            put_phone(d,p+count-2,34);
        put_phone(d,p+count-1,14);
    } else if(count>=2&&reloq61_spr_feature(d,p,0)==20&&
              reloq61_spr_feature(d,p+1,0)==45&&input_symbol(d,p)=='h') {
        int32_t previous=vgetsc(d,1,0,left_of(p),2);
        while(previous&&(*EVV_AT(const int32_t *,previous)&2))previous=vgetsc(d,1,0,previous,2);
        if(previous) {
            struct reloq61_spr_phone prev={previous,EVV_AT(const int32_t *,previous)[1]&~3};
            if(reloq61_spr_feature(d,&prev,0)==14) {
                vdel_2pt(d,2,left_of(p),p->after);
                uint8_t onset=2;
                vmark(d,2,15,left_of(p+1),p[1].after,&onset);
            }
        }
    }
}
static int previous_phone(delta_state *d,const struct reloq61_spr_phone *p,
    struct reloq61_spr_phone *previous)
{
    int32_t node=vgetsc(d,1,0,left_of(p),2);
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,1,0,node,2);
    if(!node)return 0;
    *previous=(struct reloq61_spr_phone){node,EVV_AT(const int32_t *,node)[1]&~3};
    return previous->after!=0;
}

/* Phonetic input can represent the same connected number sequence in a
 * single SPR group. Keep its stress while matching the text path's onset. */
static int phonetic_connected_number_onset(delta_state *d,const struct reloq61_spr_phone *u)
{
    if(reloq61_spr_feature(d,u,0)!=36||reloq61_spr_feature(d,u,15)!=2)return 0;
    struct reloq61_spr_phone n,t;
    if(!reloq61_spr_phone_at(d,u->after,&n)||reloq61_spr_feature(d,&n,0)!=22||
       !reloq61_spr_phone_at(d,n.after,&t)||reloq61_spr_feature(d,&t,0)!=3)return 0;
    struct reloq61_spr_phone glide,liquid,start;
    /* At stem repair the diphthong is one /aj/ phone; the acoustic
     * planner expands it into A and J only later. */
    if(previous_phone(d,u,&glide)&&reloq61_spr_feature(d,&glide,0)==44&&
       previous_phone(d,&glide,&liquid)&&previous_phone(d,&liquid,&start)) {
        unsigned name=reloq61_spr_feature(d,&liquid,0),first=reloq61_spr_feature(d,&start,0);
        if((name==15&&first==4)||(name==8&&first==17))return 1;
    }
    static const unsigned reverse[]={22,43,2,28,10};
    int32_t at=left_of(u);
    for(size_t i=0;i<sizeof reverse/sizeof *reverse;++i) {
        int32_t node=vgetsc(d,1,0,at,2);
        while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,1,0,node,2);
        if(!node)return 0;
        struct reloq61_spr_phone p={node,EVV_AT(const int32_t *,node)[1]&~3};
        if(reloq61_spr_feature(d,&p,0)!=reverse[i])return 0;
        at=left_of(&p);
    }
    return 1;
}
/* For five, six and eight the approved connected-number reading does have
 * an onset on und. The generated SPR has no separate onset notation, so
 * recover it from the exact preceding unit when that SPR is read back. */
static int phonetic_number_und_glottal(delta_state *d,const struct reloq61_spr_phone *u)
{
    if(reloq61_spr_feature(d,u,0)!=36||reloq61_spr_feature(d,u,15)!=0)return 0;
    int32_t left=left_of(u);delta_vars *v=EVV_AT(delta_vars *,d->vars);
    if(*EVV_AT(const int32_t *,left+(v->fence_base+4)*4)&1)return 0;
    struct reloq61_spr_phone n,t;
    if(!reloq61_spr_phone_at(d,u->after,&n)||reloq61_spr_feature(d,&n,0)!=22||
       !reloq61_spr_phone_at(d,n.after,&t)||reloq61_spr_feature(d,&t,0)!=3)return 0;
    static const unsigned char paths[][4]={{7,22,40,7},{9,5,31,10},{3,14,34,0}};
    static const size_t sizes[]={4,4,3};
    for(size_t i=0;i<sizeof sizes/sizeof *sizes;++i) {
        struct reloq61_spr_phone current=*u,previous;size_t j=0;
        while(j<sizes[i]&&previous_phone(d,&current,&previous)&&
              reloq61_spr_feature(d,&previous,0)==paths[i][j]) {current=previous;++j;}
        if(j==sizes[i])return 1;
    }
    return 0;
}
/* Preserve native text vowel quantity for equivalent explicit phonemes.
 * Stress belongs to the syllable onset, so propagate it to later phones. */
static void phonetic_quantity(delta_state *d,int32_t begin,int32_t end)
{
    int32_t at=begin;
    int32_t first=vgetsc(d,0,0,at,1);
    while(first&&(*EVV_AT(const int32_t *,first)&2))first=vgetsc(d,0,0,first,1);
    if(!first)return;
    unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void *,first)));
    if(name>=sizeof alphabet||alphabet[name]!='`')return;
    /* Two linear passes avoid both a fixed phone limit and a quadratic
     * search for a later primary stress in long phonetic groups. */
    size_t count=0,last_primary=SIZE_MAX;
    int current_stress=-1;struct reloq61_spr_phone phone;
    while(at&&at!=end&&reloq61_spr_phone_at(d,at,&phone)) {
        int onset_stress=syllable_stress(d,&phone);
        if(onset_stress>=0)current_stress=onset_stress;
        if(current_stress==1)last_primary=count;
        ++count;
        if(phone.after==at)break;
        at=phone.after;
    }
    int primary_before=0,previous_vowel=0;current_stress=-1;at=begin;
    for(size_t i=0;i<count&&at&&at!=end&&reloq61_spr_phone_at(d,at,&phone);++i) {
        const struct reloq61_spr_phone *p=&phone;
        int onset_stress=syllable_stress(d,p);
        if(onset_stress>=0)current_stress=onset_stress;
        unsigned id=reloq61_spr_feature(d,p,0),length=reloq61_spr_feature(d,p,10);
        int symbol=input_symbol(d,p),primary_after=last_primary!=SIZE_MAX&&last_primary>i;
        struct reloq61_spr_phone next;
        int same_right=i+1<count&&p->after!=end&&
            reloq61_spr_phone_at(d,p->after,&next)&&!begins_syllable(d,&next);
        /* After a vowel, the onset of an unstressed hiatus such as the
         * final syllable in Gabriel stays short in the text path. */
        int hiatus=previous_vowel&&begins_syllable(d,p);
        int full=(current_stress==2&&(same_right||(primary_before&&!primary_after))) ||
                 (current_stress==0&&same_right&&!hiatus);
        if(phonetic_connected_number_onset(d,p)) {
            uint8_t no_onset=0;
            vmark(d,2,15,left_of(p),p->after,&no_onset);
        } else if(phonetic_number_und_glottal(d,p)) {
            uint8_t onset=2;
            vmark(d,2,15,left_of(p),p->after,&onset);
        }
        if(id==34&&symbol=='a'&&!reloq61_spr_feature(d,p,14))
            put_phone(d,p,33);
        else if(full&&length==2&&(id==28||id==30||id==35||id==37||id==39||id==41)) {
            uint8_t long_value=1;vmark(d,2,10,left_of(p),p->after,&long_value);
        }
        if(current_stress==1)primary_before=1;
        previous_vowel=reloq61_spr_feature(d,p,1)==1;
        if(p->after==at)break;
        at=p->after;
    }
}
void reloq_dede61_repair_stem(void *state,int32_t begin,int32_t end)
{
    delta_state *d=state;
    int32_t first=vgetsc(d,0,0,begin,1);
    while(first&&(*EVV_AT(const int32_t *,first)&2))first=vgetsc(d,0,0,first,1);
    unsigned name=first?*(const unsigned char *)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void *,first))):0;
    int phonetic=name<sizeof alphabet&&alphabet[name]=='`';
    phonetic_quantity(d,begin,end);
    int32_t repair_begin,repair_end;
    memcpy(&repair_begin,(const unsigned char *)state+DG_BASE+584,4);
    memcpy(&repair_end,(const unsigned char *)state+DG_BASE+592,4);
    if(!repair_begin||!repair_end)return;
    unsigned char spelling[128];int initial;
    word_spelling(d,repair_begin,spelling,&initial);
    repair_konzentrier(d,repair_begin,repair_end,spelling);
    repair_samt(d,repair_begin,repair_end,spelling,phonetic);
    repair_touch(d,repair_begin,repair_end,spelling,phonetic);
    repair_oeffentlich(d,repair_begin,repair_end,spelling,phonetic);
    repair_eighty_onset(d,repair_begin,repair_end);
    repair_os_adjective(d,repair_begin,repair_end,spelling);
    repair_nominal_prefix(d,repair_begin,repair_end,spelling,initial);
    repair_four_liaison(d,repair_begin,repair_end,spelling);
    repair_number_onset(d,repair_begin,repair_end,spelling);
    repair_closed_o(d,repair_begin,repair_end);
    repair_ch_compound(d,repair_begin,repair_end,spelling);
    note_final_a(d,repair_begin,repair_end);
}
