/* Read-only input projection for English consonant spelling runs. No caller
 * registers, fence configuration, records or backtracking state are changed. */
#include "delta.h"
#include "reloq61_english_word_edges.h"
#include "reloq_english_word_edges.h"

static const unsigned char alphabet[]={
#include "reloq61_english_alphabet.inc"
};

/* A clock shortcut may omit seconds. Decline that lossy conversion when
 * the complete numeric input has a seconds field; the existing ordinary
 * number rules then retain every supplied digit, including fractions. */
int reloq61_english_time_has_seconds(void *state,int32_t begin,int32_t end)
{
    delta_state *d=state;
    if(!d||!begin||!end||!d->vars||deltaErrorThrown(d))return 0;
    const delta_loc *a=EVV_AT(const delta_loc *,begin),*z=EVV_AT(const delta_loc *,end);
    if(a->kind!=DK_SYNC||z->kind!=DK_SYNC||!a->value||!z->value)return 0;
    int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,a->value),1));
    int32_t stop=EVV_REF(vmover(d,EVV_AT(int32_t *,z->value),1));
    unsigned char text[64];size_t n=0;
    while(at&&at!=stop&&n<sizeof text) {
        int32_t token=vgetsc(d,0,0,at,1);
        if(!token||(*EVV_AT(int32_t *,token)&2))return 0;
        unsigned name=*(unsigned char *)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void *,token)));
        if(name>=sizeof alphabet)return 0;
        text[n++]=alphabet[name];
        int32_t after=EVV_AT(int32_t *,token)[1]&~3;
        if(!after)return 0;
        at=EVV_REF(vmover(d,EVV_AT(int32_t *,after),1));
    }
    if(at!=stop)return 0;
    size_t i=0;unsigned hours=0,digits=0;
    while(i<n&&text[i]>='0'&&text[i]<='9'&&digits<3) {
        hours=hours*10+text[i++]-'0';++digits;
    }
    if(!digits||digits>2||hours>23||i>=n||text[i++]!=':')return 0;
    for(unsigned field=0;field<2;++field) {
        if(n-i<2||text[i]<'0'||text[i]>'5'||text[i+1]<'0'||text[i+1]>'9')return 0;
        i+=2;
        if(!field&&(i>=n||text[i++]!=':'))return 0;
    }
    if(i<n&&text[i]=='.') {
        ++i;size_t first=i;
        while(i<n&&text[i]>='0'&&text[i]<='9')++i;
        if(i==first)return 0;
    }
    return i==n;
}

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
