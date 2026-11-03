#include "reloq61_spr_output.h"
#include <string.h>
static const unsigned char alphabet[]={
#include "reloq61_german_alphabet.inc"
};
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
static void repair_konzentrier(delta_state *d,int32_t word_begin,int32_t word_end)
{
    if(!word_begin||!word_end)return;
    int32_t start=word_begin,scan=start;
    for(unsigned i=0;i<127;++i) {
        int32_t previous=scan;int c=input_at(d,&previous,1);
        if(!ascii_letter(c))break;
        start=scan=previous;
    }
    unsigned char spelling[128];size_t n=0;scan=start;
    while(n<127) {
        int c=input_at(d,&scan,0);if(!ascii_letter(c))break;
        spelling[n++]=(unsigned char)(c>='A'&&c<='Z'?c+32:c);
    }
    spelling[n]=0;
    if(!strstr((const char *)spelling,"konzentrier"))return;
    struct reloq61_spr_phone p[64];size_t count=0;scan=word_begin;
    while(scan&&scan!=word_end&&count<64&&reloq61_spr_phone_at(d,scan,&p[count]))
        scan=p[count++].after;
    for(size_t i=0;i+5<count;++i) {
        static const unsigned char pattern[]={5,38,22,17,43,22};
        unsigned j=0;
        while(j<6&&reloq61_spr_feature(d,p+i+j,0)==pattern[j])++j;
        if(j!=6)continue;
        put_phone(d,p+i+4,31); /* schwa in zen becomes /E/ */
        int32_t syll=vgetsc(d,0,0,left_of(p+i),7);
        while(syll&&(*EVV_AT(const int32_t *,syll)&2))syll=vgetsc(d,0,0,syll,7);
        if(syll) {
            int16_t weak=0;
            int32_t l=*EVV_AT(const int32_t *,syll)&~3;
            int32_t r=EVV_AT(const int32_t *,syll)[1]&~3;
            vmark(d,7,0,l,r,&weak);
        }
        return;
    }
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
    phonetic_quantity(d,begin,end);
    int32_t repair_begin,repair_end;
    memcpy(&repair_begin,(const unsigned char *)state+DG_BASE+584,4);
    memcpy(&repair_end,(const unsigned char *)state+DG_BASE+592,4);
    repair_konzentrier(d,repair_begin,repair_end);
}
