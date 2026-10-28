/* US English combined-phone spelling. Decisions use segment features and
 * a single look-ahead, without rule frames, scans or backtracking. */
#include "reloq61_spr_output.h"
#include "reloq61_scalar_state.h"

/* A combined spelling stays inside the current syllable, including sync
 * markers between its two segments. This is a read-only projection walk. */
static int next_in_syllable(delta_state *d,int32_t at,struct reloq61_spr_phone *phone)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    for(size_t steps=0;at && steps<200000;++steps) {
        if(*EVV_AT(int32_t *,at+(v->fence_base+7)*4)&1)return 0;
        int32_t node=vgetsc(d,0,0,at,2);
        if(!node)return 0;
        if(!(*EVV_AT(int32_t *,node)&2)) {
            *phone=(struct reloq61_spr_phone){node,*EVV_AT(int32_t *,node+4)&~3};
            return phone->after!=0;
        }
        at=node;
    }
    return 0;
}

int32_t reloq_enus61_print_eng_SPR(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;
    if(!p1||!p2)return reloq_scalar_finish(d,94);
    int32_t begin=EVV_AT(delta_loc *,p1)->value,consumed=0;
    struct reloq61_spr_phone first,second;
    const char *spelling=NULL;
    if(begin && (*EVV_AT(int32_t *,begin+(EVV_AT(delta_vars *,d->vars)->fence_base+2)*4)&1) &&
       reloq61_spr_phone_at(d,begin,&first)) {
        if(reloq61_spr_feature(d,&first,10)==1)print_lit(d,8,"h");
        unsigned a=reloq61_spr_feature(d,&first,0);
        if(next_in_syllable(d,first.after,&second)) {
            unsigned b=reloq61_spr_feature(d,&second,0);
            if(a==4&&b==16)spelling="C";
            else if(a==3&&b==15)spelling="J";
            else if(a==43&&b==25)spelling="Y";
            else if(a==43&&b==26)spelling="W";
            else if(a==47&&b==25)spelling="O";
            if(spelling)consumed=second.after;
        }
        if(!spelling && reloq61_spr_feature(d,&first,1)==1) {
            if(a==20)spelling="M";
            else if(a==21)spelling="N";
        }
    }
    return reloq61_spr_write(d,begin,p2,spelling,consumed);
}
int32_t reloq_enus61_lang_specific_SPR(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    if(!reloq_scalar_begin(state))return 94;
    int16_t mode,phonetic;
    memcpy(&mode,(unsigned char *)state+DG_BASE+3454,2);
    memcpy(&phonetic,(unsigned char *)state+DG_BASE+3450,2);
    return mode==phonetic?reloq_enus61_print_eng_SPR(state,p0,p1,p2):reloq_scalar_finish(state,0);
}
