/* Translate only the name selected by the parent character rule. Its mode,
 * context, punctuation and spacing decisions run unchanged. */
#include <stdint.h>
#include <string.h>
#include "delta.h"
#include "delta_rules_c.h"
#include "greek_character_names.inc"

static int32_t input_record(delta_state *d,int32_t position,int backwards)
{
    if(!position)return 0;
    int32_t point=EVV_REF(backwards?
        vmovel(d,EVV_AT(delta_node *,position),1):
        vmover(d,EVV_AT(delta_node *,position),1));
    int32_t node=vgetsc(d,(int8_t)backwards,0,point,1);
    return node&&!(*EVV_AT(int32_t *,node)&2)?node:0;
}
static unsigned char *input_fields(int32_t node)
{return TFLDS(EVV_AT(void *,node));}
static unsigned input_name(int32_t node)
{return node?input_fields(node)[4]:0;}
static int32_t next_position(int32_t node)
{return *EVV_AT(const int32_t *,node+4)&~3;}

/* The native name for digit two is dío. Its delta is a fricative, including
 * at the start of an utterance. The inherited Castilian initial-d rule would
 * select a stop. Match the complete name in the word projection; ordinary
 * Greek words and cardinal numbers already carry their own phones. */
int reloq_elgr61_digit_two(void *state,const delta_token *left)
{
    delta_state *d=state;
    int32_t point=EVV_REF(vmover(d,EVV_AT(delta_node *,left->value),2));
    int32_t word=vgetsc(d,0,0,point,2);
    if(!word||(*EVV_AT(const int32_t *,word)&2)||
       (*EVV_AT(const int32_t *,word)&~3)!=left->value)return 0;
    int32_t end=next_position(word),at=left->value;
    static const unsigned char name[]={13,109,4};
    for(unsigned i=0;i<sizeof name;++i) {
        int32_t node=input_record(d,at,0);
        if(input_name(node)!=name[i])return 0;
        at=next_position(node);
    }
    return at==end;
}

/* An internal phoneme word is a lexical word when the punctuation grammar
 * looks next to it. Validate the whole group, so a literal bracket or an
 * unrelated ECI control never acquires letter properties. */
static int32_t phoneme_word_end(delta_state *d,int32_t start)
{
    static const char alphabet[]="\0aeiouAEIOUbcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ0123456789-,.?:;!() '#_`~*@[]{}`$+%/=^&|\\<>\"";
    int32_t node=input_record(d,start,0);
    if(input_name(node)!=76)return 0;
    node=input_record(d,next_position(node),0);
    if(input_name(node)!=80)return 0;
    unsigned letters=0;
    for(unsigned i=0;i<512;++i) {
        node=input_record(d,next_position(node),0);
        unsigned name=input_name(node);
        if(name==81)return letters?node:0;
        if(!node||name>=sizeof alphabet)return 0;
        char c=alphabet[name];
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))++letters;
        else if(c!='.'&&c!='0'&&c!='1')return 0;
    }
    return 0;
}
static int previous_phoneme_word(delta_state *d,int32_t boundary,int32_t closing)
{
    (void)boundary;
    int32_t position=*EVV_AT(const int32_t *,closing)&~3;
    for(unsigned i=0;i<512&&position;++i) {
        int32_t node=input_record(d,position,1);
        if(!node)return 0;
        int32_t begin=*EVV_AT(const int32_t *,node)&~3;
        if(input_name(node)==76)return phoneme_word_end(d,begin)==closing;
        unsigned name=input_name(node);
        if(!name||name==72||name==81)return 0;
        position=begin;
    }
    return 0;
}

int32_t reloq_elgr61_punctuation(void *state,int32_t p0,int32_t p1,int32_t p2,
    int32_t (*parent)(void *,int32_t,int32_t,int32_t))
{
    if(delta_lang_now()->id!=0x120000)return parent(state,p0,p1,p2);
    delta_state *d=state;
    struct {unsigned char *fields,original[6];} view[2];unsigned count=0;
    int32_t left=EVV_AT(const delta_token *,p1)->value;
    int32_t right=EVV_AT(const delta_token *,p2)->value;
    if(left==right) {
        int32_t mark=input_record(d,right,0);
        if(mark)right=next_position(mark);
    }
    int32_t before=input_record(d,left,1),after=input_record(d,right,0);
    int32_t selected[2]={
        input_name(before)==81&&previous_phoneme_word(d,left,before)?before:0,
        input_name(after)==76&&phoneme_word_end(d,right)?after:0};
    for(unsigned i=0;i<2;++i)if(selected[i]) {
        unsigned char *fields=input_fields(selected[i]);
        view[count].fields=fields;memcpy(view[count++].original,fields,6);
        /* Rule-local lexical view; restore the phoneme delimiters before
           the next token is processed. No caller text is rewritten. */
        fields[0]=1;fields[1]=1;fields[2]=1;fields[4]=1;
    }
    int32_t result=parent(state,p0,p1,p2);
    while(count){--count;memcpy(view[count].fields,view[count].original,6);}
    return result;
}

int32_t reloq_elgr61_insert_name(void *state,const delta_token *left,
    const delta_token *right,uint8_t projection,uint8_t length,
    const uint8_t *value,uint8_t flags)
{
    delta_state *d=state;
    if(delta_lang_now()->id==0x120000&&projection==1) {
        int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,left->value),1));
        int32_t end=EVV_REF(vmover(d,EVV_AT(int32_t *,right->value),1));
        unsigned char key[8];size_t used=0;
        while(at!=end&&used<sizeof key) {
            int32_t node=vgetsc(d,0,0,at,1);
            if(!node||(*EVV_AT(int32_t *,node)&2))break;
            key[used++]=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
            int32_t next=*EVV_AT(int32_t *,node+4)&~3;
            if(!next)break;
            at=EVV_REF(vmover(d,EVV_AT(int32_t *,next),1));
        }
        if(at==end)
            for(size_t i=0;i<sizeof greek_names/sizeof *greek_names;++i)
                if(greek_names[i].key_length==used&&!memcmp(greek_names[i].key,key,used)) {
                    length=greek_names[i].length;value=greek_names[i].value;break;
                }
    }
    return insert_2pt_s(d,projection,length,value,flags);
}
