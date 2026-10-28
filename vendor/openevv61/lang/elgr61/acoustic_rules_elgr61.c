/* Greek's front chi uses the same Mexican J rule as our earlier Greek
   module. The back variant keeps Castilian J. Inspect the following
   phoneme after Greek spelling analysis, so αι/ει/οι have their actual
   vowel values rather than the first letter of their spelling. */
#include <string.h>
#include "delta.h"

static int32_t neighbor(void *state,int32_t token,int reverse)
{
    delta_state *d=state;
    if(!token||!d->vars)return 0;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    delta_tpos left_saved=d->lpta,right_saved=d->rpta;
    unsigned char scan[8];memcpy(scan,&v->scan_ptr,sizeof scan);
    int8_t compared=v->compared_equal,fences=v->fence_count,testing=v->testing;
    v->fence_count=0;v->testing=0;
    lpta_loadp(d,EVV_AT(const delta_token *,token));
    int32_t found=0;
    if(!(reverse?setscan_l(d,4):setscan_r(d,4))) {
        /* Sync markers can precede the next real phoneme, including at
           a syllable boundary. They are not phoneme records. */
        int32_t node=v->scan_ptr;
        do {
            const int32_t *link=EVV_AT(const int32_t *,node);
            node=link[v->scan_rev?v->fence_base+v->scan_field:3+v->scan_field]&~3;
        } while(node&&(*EVV_AT(const int32_t *,node)&2));
        found=node;
    }
    d->lpta=left_saved;d->rpta=right_saved;
    memcpy(&v->scan_ptr,scan,sizeof scan);v->compared_equal=compared;
    v->fence_count=fences;v->testing=testing;
    return found;
}

static int phone_field(int32_t node,unsigned field)
{
    return node?*(const uint8_t *)delta_record_field(&vstmtbl[4],field,
        TFLDS(EVV_AT(void *,node))):-1;
}
int reloq_elgr61_front_chi(void *state,int32_t right)
{
    int32_t node=neighbor(state,right,0);
    int phone=phone_field(node,0);
    const delta_fielddesc *f=vstmtbl[4].fields;
    if(phone<0||phone>=f->nvalues)return 0;
    const char *name=((const char *const *)f->values)[phone];
    return name&&((name[0]=='e'||name[0]=='i'||name[0]=='y')&&!name[1]);
}
/* The language adapter owns cell placement; these named control slots are
   the existing Castilian scheduler boundary, shared by the Greek state. */
static void *control(void *state,unsigned offset)
{ return (unsigned char *)state+DG_BASE+offset-DG_BASE_IBM; }
static int32_t context_phone(void *state,int right)
{
    return neighbor(state,EVV_REF(control(state,right?892:884)),!right);
}
int reloq_elgr61_v_duration(void *state)
{
    int consonant=phone_field(context_phone(state,0),1)==2 ||
        phone_field(context_phone(state,1),1)==2;
    *(int32_t *)control(state,2252)=consonant?500000:600000;
    return 0;
}
extern int32_t reloq_eses61_span_labial_Fv(void *,int32_t);
int reloq_elgr61_v_targets(void *state,int32_t machine)
{
    int32_t left=context_phone(state,0),right=context_phone(state,1);
    *(int16_t *)control(state,3178)=300;
    *(int16_t *)control(state,3182)=300;
    *(int16_t *)control(state,3002)=40;
    *(int16_t *)control(state,3038)=60;
    *(int16_t *)control(state,2978)=95;
    *(int16_t *)control(state,2982)=125;
    *(int16_t *)control(state,2994)=phone_field(right,6)==0?50:45;
    int32_t duration=*(int32_t *)control(state,2252);
    int32_t begin=*(int32_t *)control(state,1224);
    if(duration<=800000)
        *(int32_t *)control(state,1272)=begin+(int32_t)((int64_t)duration*4/10);
    else {
        if(phone_field(left,3)==0)*(int32_t *)control(state,1272)=begin+400000;
        if(phone_field(right,3)==0)*(int32_t *)control(state,1272)=
            *(int32_t *)control(state,1232)-400000;
    }
    return reloq_eses61_span_labial_Fv(state,machine);
}
