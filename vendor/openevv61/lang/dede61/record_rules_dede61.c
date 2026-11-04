/* German bindings for shared input and syllable record queries. */
#include "reloq61_record_readers.h"
#include "reloq61_scalar_state.h"
int32_t reloq_dede61_in_syllable(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{(void)p0;return reloq61_record_in_syllable(state,p1,p2,p3);}
int32_t reloq_dede61_test_stress_annot(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_record_unstressed(state,p1,p2,6);}
int32_t reloq_dede61_find_most_stressed_syll(void *state,int32_t p0)
{(void)p0;return reloq61_record_last_accent(state);}

/* The older German melody keeps a deaccented verb after the focus on the
 * falling contour. A separate postnuclear high tone creates a new rise.
 * Select only plain statements with an unannotated, deaccented verb; question
 * and exclamation boundaries and explicit emphasis keep their own tones. */
int reloq_dede61_skip_weak_postnuclear(void *state)
{
    delta_state *d=state;int32_t begin=0,end=0;
    if(!d||!d->vars)return 0;
    memcpy(&begin,(const unsigned char *)d+DG_BASE+160,4);
    memcpy(&end,(const unsigned char *)d+DG_BASE+168,4);
    if(!begin||!end)return 0;
    int32_t word=vgetsc(d,0,0,begin,4);
    while(word&&(*EVV_AT(const int32_t *,word)&2))word=vgetsc(d,0,0,word,4);
    int32_t phrase=vgetsc(d,1,0,end,5);
    while(phrase&&(*EVV_AT(const int32_t *,phrase)&2))phrase=vgetsc(d,1,0,phrase,5);
    if(!word||!phrase)return 0;
    void *wf=TFLDS(EVV_AT(void *,word)),*pf=TFLDS(EVV_AT(void *,phrase));
    int16_t stress;memcpy(&stress,delta_record_field(&vstmtbl[4],3,wf),2);
    return *(const unsigned char *)delta_record_field(&vstmtbl[5],1,pf)==0&&
        *(const unsigned char *)delta_record_field(&vstmtbl[4],1,wf)==25&&stress==0&&
        *(const unsigned char *)delta_record_field(&vstmtbl[4],4,wf)==6&&
        *(const unsigned char *)delta_record_field(&vstmtbl[4],6,wf)==0;
}
