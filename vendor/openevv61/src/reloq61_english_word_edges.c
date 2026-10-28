/* Read-only input projection for English consonant spelling runs. No caller
 * registers, fence configuration, records or backtracking state are changed. */
#include "delta.h"
#include "reloq61_english_word_edges.h"
#include "reloq_english_word_edges.h"

static const unsigned char alphabet[]={
#include "reloq61_english_alphabet.inc"
};

static int read_run(delta_state *d,int32_t argument,unsigned back,unsigned boundary,
    unsigned char spelling[5],size_t *length)
{
    *length=0;
    if(!d || !d->vars || deltaErrorThrown(d) || !argument)return 0;
    const delta_loc *p=EVV_AT(const delta_loc *,argument);
    if(p->kind!=DK_SYNC || !p->value)return 0;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    int32_t at=p->value;
    if(!(*EVV_AT(int32_t *,at+(v->fence_base+1)*4)&1))return 0;
    /* The spelling category ends the run. Word classification has no
     * additional fence; syllabification respects its enclosing stem. */
    int held=1;
    for(size_t steps=0;steps<200000 && at && *length<5;++steps) {
        if(!held && boundary && (*EVV_AT(int32_t *,at+(v->fence_base+boundary)*4)&3))break;
        int32_t token=vgetsc(d,back,0,at,1);
        if(!token)break;
        held=0;
        if(*EVV_AT(int32_t *,token)&2){at=token;continue;}
        void *fields=TFLDS(EVV_AT(void *,token));
        if(*(unsigned char *)delta_record_field(&vstmtbl[1], 4, fields)!=2)break;
        unsigned name=*(unsigned char *)delta_record_field(&vstmtbl[1], 0, fields);
        if(name>=sizeof alphabet)return 0;
        spelling[(*length)++]=alphabet[name];
        at=*EVV_AT(int32_t *,token+(back?0:4))&~3;
    }
    if(back)for(size_t i=0;i<*length/2;++i) {
        unsigned char c=spelling[i];
        spelling[i]=spelling[*length-1-i];spelling[*length-1-i]=c;
    }
    return 1;
}

int32_t reloq61_english_onset(void *state,int32_t argument)
{
    unsigned char s[5];size_t n;
    return read_run(state,argument,0,0,s,&n) && reloq_english_onset(s,n)?0:94;
}
int32_t reloq61_english_coda(void *state,int32_t argument)
{
    unsigned char s[5];size_t n;
    return read_run(state,argument,1,0,s,&n) && reloq_english_coda(s,n)?0:94;
}
int32_t reloq61_english_syllable_onset(void *state,int32_t argument)
{
    unsigned char s[5];size_t n;
    return read_run(state,argument,0,3,s,&n) && reloq_english_syllable_onset(s,n)?0:94;
}
