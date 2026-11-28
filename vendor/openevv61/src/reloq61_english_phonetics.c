#include "reloq61_english_phonetics.h"
#include "reloq61_scalar_state.h"
#include "reloq61_spr_output.h"

/* The US and UK acoustic rule records have the same layout. These are
 * phone positions and the A2F/A3F/A4F targets, respectively. */
enum { PHONE_START=712, NOISE_A2=2934, NOISE_A3=2938, NOISE_A4=2942,
       SCHWA_F2_LEFT=3098, SCHWA_F2_RIGHT=3102,
       SCHWA_F3_LEFT=3106, SCHWA_F3_RIGHT=3110 };
enum { PHONE_SH=16, PHONE_R=23, PHONE_W=26, PHONE_SCHWA=35 };
enum { PHONE_P=2, PHONE_T=4, PHONE_K=7, PHONE_S=14,
       PHONE_TRANSITION=8, TRANSITION_TO_VOICE=2, PHONE_DURATION=2172,
       VOICE_STREAM=1034, ASPIRATION_STREAM=1038,
       RELEASE_START=1152, RELEASE_END=1160, RELEASE_AH=2978 };

static int16_t short_at(const unsigned char *p,unsigned off)
{int16_t v;memcpy(&v,p+off,sizeof v);return v;}
static void put_short(unsigned char *p,unsigned off,int16_t v)
{memcpy(p+off,&v,sizeof v);}
static int32_t long_at(const unsigned char *p,unsigned off)
{int32_t v;memcpy(&v,p+off,sizeof v);return v;}
static void put_long(unsigned char *p,unsigned off,int32_t v)
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

void reloq61_english_s_duration(void *state)
{
    delta_state *d=state;
    if(!d || !d->vars || deltaErrorThrown(d))return;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    struct reloq61_spr_phone current,next,after;
    if(!reloq61_spr_phone_at(d,long_at(p,PHONE_START),&current) ||
       reloq61_spr_feature(d,&current,0)!=PHONE_S ||
       !reloq61_spr_phone_at(d,current.after,&next))return;
    unsigned stop=reloq61_spr_feature(d,&next,0);
    if((stop!=PHONE_P && stop!=PHONE_T && stop!=PHONE_K) ||
       !reloq61_spr_phone_at(d,next.after,&after) ||
       reloq61_spr_feature(d,&next,PHONE_TRANSITION)!=TRANSITION_TO_VOICE)return;
    int32_t duration=long_at(p,PHONE_DURATION);
    if(duration<160000)return;
    /* Change the native duration before it is inserted. The release,
     * following phones, pitch and indices are then rescheduled together;
     * the shortened frication does not turn into extra closure silence. */
    put_long(p,PHONE_DURATION,(int32_t)(((int64_t)duration*3)/4));
}

static _Thread_local struct reloq61_english_stop_scope soft_stop;

struct reloq61_english_stop_scope reloq61_english_stop_scope(
    struct reloq61_english_stop_scope next)
{
    struct reloq61_english_stop_scope previous=soft_stop;
    soft_stop=next;
    return previous;
}

void reloq61_english_stop_release_begin(void *state,
    struct reloq61_english_release_guard *guard)
{
    *guard=(struct reloq61_english_release_guard){0};
    delta_state *d=state;
    if(!d || !d->vars || deltaErrorThrown(d))return;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    int32_t at=long_at(p,PHONE_START);
    struct reloq61_spr_phone current,next;
    if(!reloq61_spr_phone_at(d,at,&current))return;
    /* The following phone's acoustic targets have already been emitted. */
    if(soft_stop.state==state && soft_stop.following==current.node)
        soft_stop=(struct reloq61_english_stop_scope){0};
    unsigned phone=reloq61_spr_feature(d,&current,0);
    if(phone!=PHONE_P && phone!=PHONE_T && phone!=PHONE_K)return;
    struct reloq61_spr_phone previous={previous_phone(d,at),0};
    if(!previous.node || reloq61_spr_feature(d,&previous,0)!=PHONE_S ||
       !reloq61_spr_phone_at(d,current.after,&next))return;
    /* Respect an existing native or dictionary-requested aspiration. */
    if(short_at(p,RELEASE_AH)!=0)return;
    int32_t begin=long_at(p,RELEASE_START),end=long_at(p,RELEASE_END);
    int64_t duration=(int64_t)end-begin;
    if(duration<10000)return;
    /* Use the core's ordinary release source at a lower level and limit the
     * extra voiceless interval to 12 ms inside its existing transition. */
    int32_t capped=duration>120000?begin+120000:end;
    if(capped>INT32_MAX-10000)return;
    *guard=(struct reloq61_english_release_guard){end,0,1};
    soft_stop=(struct reloq61_english_stop_scope){state,next.node,capped};
    put_short(p,RELEASE_AH,40);
    put_long(p,RELEASE_END,capped);
}

void reloq61_english_stop_release_end(void *state,
    const struct reloq61_english_release_guard *guard,int32_t result)
{
    if(!guard->active)return;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    put_short(p,RELEASE_AH,guard->level);
    put_long(p,RELEASE_END,guard->end);
    if(result)soft_stop=(struct reloq61_english_stop_scope){0};
}

void reloq61_english_stop_interval(void *state,int16_t stream,
    int32_t *begin,int32_t *end)
{
    if(soft_stop.state!=state || !state || !begin || !end)return;
    delta_state *d=state;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    if(stream!=short_at(p,VOICE_STREAM) && stream!=short_at(p,ASPIRATION_STREAM))return;
    struct reloq61_spr_phone current;
    int32_t release_end=soft_stop.release_end/10000;
    if(!reloq61_spr_phone_at(d,long_at(p,PHONE_START),&current) ||
       current.node!=soft_stop.following || *begin>release_end)return;
    /* The release has already queued silence and aspiration. Keep the
     * following native target rather than losing it to the ordered queue. */
    *begin=release_end+1;
    if(*end<*begin)*end=*begin;
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
