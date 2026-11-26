#include "reloq61_english_phonetics.h"
#include "reloq61_scalar_state.h"
#include "reloq61_spr_output.h"

/* The US and UK acoustic rule records have the same layout. These are
 * phone positions and the A2F/A3F/A4F targets, respectively. */
enum { PHONE_START=712, NOISE_A2=2934, NOISE_A3=2938, NOISE_A4=2942,
       SCHWA_F2_LEFT=3098, SCHWA_F2_RIGHT=3102,
       SCHWA_F3_LEFT=3106, SCHWA_F3_RIGHT=3110 };
enum { PHONE_SH=16, PHONE_R=23, PHONE_W=26, PHONE_SCHWA=35 };

static int16_t short_at(const unsigned char *p,unsigned off)
{int16_t v;memcpy(&v,p+off,sizeof v);return v;}
static void put_short(unsigned char *p,unsigned off,int16_t v)
{memcpy(p+off,&v,sizeof v);}

void reloq61_english_shr_noise(void *state)
{
    delta_state *d=state;
    if(!d || !d->vars || deltaErrorThrown(d))return;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    if(short_at(p,NOISE_A2)!=60 || short_at(p,NOISE_A3)!=62 ||
       short_at(p,NOISE_A4)!=0)return;
    int32_t at;memcpy(&at,p+PHONE_START,sizeof at);
    struct reloq61_spr_phone first,next;
    if(!reloq61_spr_phone_at(d,at,&first) ||
       reloq61_spr_feature(d,&first,0)!=PHONE_SH ||
       !reloq61_spr_phone_at(d,first.after,&next) ||
       reloq61_spr_feature(d,&next,0)!=PHONE_R)return;
    /* The ordinary /S/ uses its third and fourth frication resonators.
     * Do not change timing, level, formants or the following /r/. */
    put_short(p,NOISE_A2,0);
    put_short(p,NOISE_A4,59);
}

static int32_t previous_phone(delta_state *d,int32_t at)
{
    for(unsigned i=0;at && i<128;++i) {
        int32_t node=vgetsc(d,1,0,at,2);
        if(!node)return 0;
        if(!(*EVV_AT(int32_t *,node)&2))return node;
        if(node==at)return 0;
        at=node;
    }
    return 0;
}

void reloq61_english_schwa_timbre(void *state)
{
    delta_state *d=state;
    if(!d || !d->vars || deltaErrorThrown(d))return;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    const int left_missing=short_at(p,SCHWA_F2_LEFT)==-2 &&
        short_at(p,SCHWA_F3_LEFT)==-2;
    const int right_missing=short_at(p,SCHWA_F2_RIGHT)==-2 &&
        short_at(p,SCHWA_F3_RIGHT)==-2;
    if(!left_missing && !right_missing)return;
    int32_t at;memcpy(&at,p+PHONE_START,sizeof at);
    struct reloq61_spr_phone first,next={0,0},previous={0,0};
    if(!reloq61_spr_phone_at(d,at,&first) ||
       reloq61_spr_feature(d,&first,0)!=PHONE_SCHWA)return;
    previous.node=previous_phone(d,at);
    unsigned left=previous.node?reloq61_spr_feature(d,&previous,0):0;
    unsigned right=reloq61_spr_phone_at(d,first.after,&next)?
        reloq61_spr_feature(d,&next,0):0;
    /* The native rule omits both oral targets when two schwas meet. After
     * /w/ this can hold the /w/ resonance through both vowels. Restore the
     * ordinary schwa endpoints at their join, while retaining the native
     * voicing, creak, duration and surrounding consonant transitions. */
    if(left==PHONE_SCHWA && left_missing) {
        int32_t before=*EVV_AT(int32_t *,previous.node)&~3;
        struct reloq61_spr_phone earlier={previous_phone(d,before),0};
        unsigned flank=earlier.node?reloq61_spr_feature(d,&earlier,0):0;
        if(right==PHONE_W || flank==PHONE_W) {
            put_short(p,SCHWA_F2_LEFT,1300);
            put_short(p,SCHWA_F3_LEFT,2450);
        }
    }
    if(right==PHONE_SCHWA && right_missing) {
        struct reloq61_spr_phone later;
        unsigned flank=reloq61_spr_phone_at(d,next.after,&later)?
            reloq61_spr_feature(d,&later,0):0;
        if(left==PHONE_W || flank==PHONE_W) {
            put_short(p,SCHWA_F2_RIGHT,1300);
            put_short(p,SCHWA_F3_RIGHT,2550);
        }
    }
}
