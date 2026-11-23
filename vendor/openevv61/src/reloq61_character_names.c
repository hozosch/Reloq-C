/* Language-specific letter names within the provider's character rule.
 * Keep its original token boundaries, punctuation and dictionary precedence. */
#include <stdint.h>
#include "delta.h"
#include "delta_lang.h"
#include "reloq61_aragonese.h"

extern int32_t reloq_elgr61_insert_name(void *,const delta_token *,const delta_token *,
    uint8_t,uint8_t,const uint8_t *,uint8_t);

struct letter_name {
    uint8_t lower,upper,length;
    uint8_t value[16];
};

/* These bytes are input-alphabet symbols, not acoustic or phoneme data. */
static const struct letter_name aragonese[] = {
    {15,36,3,{15,5,2}},                    /* G: gue */
    {17,38,7,{3,72,19,1,24,15,1}},       /* J: i larga */
    {27,48,2,{27,2}},                    /* V: ve within a spelling run */
    {28,49,8,{27,2,72,13,4,22,19,2}},   /* W: ve dople */
    {29,50,2,{29,2}},                    /* X: xe */
    {30,51,2,{30,2}},                    /* Y: ye */
};
static const struct letter_name mexican_spanish[] = {
    {27,48,8,{27,2,72,12,16,3,12,1}},   /* V: ve chica */
    {28,49,7,{13,4,11,19,2,72,5}},      /* W: doble u */
};

int32_t reloq61_character_insert_name(void *state,const delta_token *left,
    const delta_token *right,uint8_t projection,uint8_t length,
    const uint8_t *value,uint8_t flags)
{
    delta_state *d=state;
    int language=delta_lang_now()->id;
    const struct letter_name *table=NULL;size_t count=0;
    if(language==RELOQ61_ARAGONESE){table=aragonese;count=sizeof aragonese/sizeof *aragonese;}
    else if(language==0x20001){table=mexican_spanish;count=sizeof mexican_spanish/sizeof *mexican_spanish;}
    if(table&&projection==1&&left&&right) {
        int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,left->value),1));
        int32_t end=EVV_REF(vmover(d,EVV_AT(int32_t *,right->value),1));
        int32_t node=vgetsc(d,0,0,at,1);
        if(node&&!(*EVV_AT(int32_t *,node)&2)) {
            unsigned key=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
            int32_t next=*EVV_AT(int32_t *,node+4)&~3;
            if(next&&EVV_REF(vmover(d,EVV_AT(int32_t *,next),1))==end)
                for(size_t i=0;i<count;++i)
                    if(key==table[i].lower||key==table[i].upper) {
                        length=table[i].length;value=table[i].value;break;
                    }
        }
    }
    return reloq_elgr61_insert_name(state,left,right,projection,length,value,flags);
}
