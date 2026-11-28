#include "reloq61_english_phonetics.h"
#include "reloq61_acoustic_output.h"
#include "reloq61_scalar_state.h"
#include "reloq61_spr_output.h"

/* The US and UK acoustic rule records have the same layout. These are
 * phone positions and the A2F/A3F/A4F targets, respectively. */
enum { PHONE_START=712, NOISE_A2=2934, NOISE_A3=2938, NOISE_A4=2942,
       SCHWA_F2_LEFT=3098, SCHWA_F2_RIGHT=3102,
       SCHWA_F3_LEFT=3106, SCHWA_F3_RIGHT=3110 };
enum { PHONE_SH=16, PHONE_R=23, PHONE_W=26, PHONE_SCHWA=35 };
enum { PHONE_P=2, PHONE_T=4, PHONE_K=7, PHONE_S=14,
       PHONE_Y=25, PHONE_FRONT_I=27, PHONE_FRONT_A=31,
       PHONE_REDUCED_I=32, PHONE_REDUCED_BACK=37,
       PHONE_TRANSITION=8, TRANSITION_TO_VOICE=2,
       VOICE_STREAM=1034, ASPIRATION_STREAM=1038, FRICATION_STREAM=1042,
       STOP_START=1144, RELEASE_START=1152, RELEASE_END=1160, RELEASE_AH=2978,
       TRACT_FIRST=1, TRACT_FORMANT_LAST=5, TRACT_LAST=10 };

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

static int16_t s_stop_release_level(delta_state *d,unsigned phone,
    const struct reloq61_spr_phone *following)
{
    /* The k targets match the ordinary English releases; p and t keep a
     * gentler impulse. All timing is taken from the current stop. */
    if(phone==PHONE_P || phone==PHONE_T)return 40;
    unsigned next=reloq61_spr_feature(d,following,0);
    if(next==PHONE_R)return 52;
    if(next==PHONE_REDUCED_I || next==PHONE_SCHWA || next==PHONE_REDUCED_BACK)return 41;
    if(next==PHONE_Y || (next>=PHONE_FRONT_I && next<=PHONE_FRONT_A))return 42;
    return 45;
}

static _Thread_local struct reloq61_english_stop_scope soft_stop;

struct reloq61_english_stop_scope reloq61_english_stop_scope(
    struct reloq61_english_stop_scope next)
{
    struct reloq61_english_stop_scope previous=soft_stop;
    soft_stop=next;
    return previous;
}

void reloq61_english_stop_prepare(void *state)
{
    delta_state *d=state;
    if(soft_stop.state==state)soft_stop=(struct reloq61_english_stop_scope){0};
    if(!d || !d->vars || deltaErrorThrown(d))return;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    int32_t at=long_at(p,PHONE_START);
    struct reloq61_spr_phone current,next;
    if(!reloq61_spr_phone_at(d,at,&current))return;
    unsigned phone=reloq61_spr_feature(d,&current,0);
    if(phone!=PHONE_P && phone!=PHONE_T && phone!=PHONE_K)return;
    struct reloq61_spr_phone previous={previous_phone(d,at),0};
    if(!previous.node || reloq61_spr_feature(d,&previous,0)!=PHONE_S ||
       !reloq61_spr_phone_at(d,current.after,&next) ||
       reloq61_spr_feature(d,&current,PHONE_TRANSITION)!=TRANSITION_TO_VOICE ||
       short_at(p,RELEASE_AH)!=0)return;
    int32_t begin=long_at(p,RELEASE_START),end=long_at(p,RELEASE_END);
    if(begin<0 || (int64_t)end-begin<10000)return;
    soft_stop=(struct reloq61_english_stop_scope){.state=state,.stop=current.node,
        .following=next.node,.anchor=begin/10000,.aspiration_begin=-1,
        .release_level=s_stop_release_level(d,phone,&next)};
    int32_t start=long_at(p,STOP_START)/10000,span=soft_stop.anchor-start;
    if(start>=0 && span>=2) {
        int32_t cut=span/4;
        if(cut<1)cut=1;
        soft_stop.tract_start=start;
        soft_stop.tract_earliest=soft_stop.anchor-cut;
        soft_stop.tract_enabled=soft_stop.tract_earliest>start;
    }
}

void reloq61_english_stop_release_begin(void *state,
    struct reloq61_english_release_guard *guard)
{
    *guard=(struct reloq61_english_release_guard){0};
    delta_state *d=state;
    if(!d || !d->vars || deltaErrorThrown(d))return;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    struct reloq61_spr_phone current;
    if(soft_stop.state!=state || soft_stop.aspiration_begin<0 ||
       !reloq61_spr_phone_at(d,long_at(p,PHONE_START),&current) ||
       current.node!=soft_stop.stop || short_at(p,RELEASE_AH)!=0)return;
    int32_t begin=long_at(p,RELEASE_START),end=long_at(p,RELEASE_END);
    int64_t duration=(int64_t)end-begin;
    if(duration<10000)return;
    /* Use the ordinary release source. The interval filter replaces part
     * of the closure with this noise without moving the following voice. */
    int32_t capped=duration>120000?begin+120000:end;
    if(capped>INT32_MAX-10000)return;
    *guard=(struct reloq61_english_release_guard){end,0,1};
    put_short(p,RELEASE_AH,soft_stop.release_level);
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

void reloq61_english_stop_interval(void *state,int16_t stream,int16_t value,
    int32_t *begin,int32_t *end)
{
    if(soft_stop.state!=state || !state || !begin || !end)return;
    delta_state *d=state;
    unsigned char *p=(unsigned char *)state+DG_BASE;
    int16_t voice=short_at(p,VOICE_STREAM),aspiration=short_at(p,ASPIRATION_STREAM);
    int16_t frication=short_at(p,FRICATION_STREAM);
    int tract=stream>=TRACT_FIRST && stream<=TRACT_LAST;
    if(stream!=voice && stream!=aspiration &&
       (stream<frication || stream>frication+6) && !tract)return;
    struct reloq61_spr_phone current;
    if(!reloq61_spr_phone_at(d,long_at(p,PHONE_START),&current))return;
    int32_t anchor=soft_stop.anchor;
    if(current.node==soft_stop.stop) {
        if(tract) {
            if(soft_stop.tract_enabled && !soft_stop.closure_valid) {
                int terminal=stream<=TRACT_FORMANT_LAST?
                    *begin==anchor && *end==anchor:
                    *begin==soft_stop.tract_start && *end==anchor;
                if(terminal) {
                    if(stream<=TRACT_FORMANT_LAST)*begin=soft_stop.tract_earliest;
                    *end=soft_stop.tract_earliest;
                    unsigned bit=(unsigned)(stream-TRACT_FIRST);
                    soft_stop.tract_values[bit]=value;
                    soft_stop.tract_held|=(uint16_t)(1u<<bit);
                }
            }
            return;
        }
        if(stream>=frication && stream<=frication+6) {
            if(!soft_stop.closure_valid && value==0 && *begin<*end && *end<anchor) {
                int32_t duration=*end-*begin;
                soft_stop.closure_begin=*begin;soft_stop.closure_end=*end;
                soft_stop.closure_cut=duration>=2?duration/4:0;
                if(duration>=2 && soft_stop.closure_cut==0)soft_stop.closure_cut=1;
                soft_stop.closure_valid=1;
                if(soft_stop.tract_enabled && soft_stop.tract_held) {
                    /* Keep the stop's own release tract until the added H
                     * begins. The following native targets then interpolate
                     * through the H, as in the ordinary aspirated stop. */
                    int32_t at=(anchor-soft_stop.closure_cut)*10000;
                    for(unsigned i=0;i<TRACT_LAST;++i)
                        if(soft_stop.tract_held & (1u<<i))
                            reloq61_submit_interval(state,(int16_t)(TRACT_FIRST+i),
                                soft_stop.tract_values[i],at,at);
                }
            }
            int32_t cut=soft_stop.closure_cut;
            if(cut>0 && *begin==soft_stop.closure_begin && *end==soft_stop.closure_end)
                *end-=cut;
            else if(cut>0 && *begin==soft_stop.closure_end && *end==anchor) {
                *begin-=cut;*end-=cut;
            }
            return;
        }
        int32_t cut=soft_stop.closure_cut;
        if(stream==aspiration && cut>0) {
            int32_t onset=anchor-cut;
            if(value==0 && *begin==soft_stop.closure_begin && *end==soft_stop.closure_end) {
                if(*end>onset)*end=onset;
            } else if(value==0 && *begin==soft_stop.closure_end && *end==anchor) {
                if(*begin>onset)*begin=onset;
                *end=onset;soft_stop.aspiration_begin=onset;
            } else if(value==soft_stop.release_level && *begin==anchor && *end>anchor) {
                *begin=soft_stop.aspiration_begin;*end=anchor;
            }
        } else if(stream==voice && value==0 && soft_stop.aspiration_begin>=0 &&
                  *begin==anchor && *end>anchor) {
            *end=anchor;
        }
        return;
    }
    if(current.node==soft_stop.following && soft_stop.aspiration_begin>=0) {
        if(tract && soft_stop.tract_enabled) {
            unsigned bit=(unsigned)(stream-TRACT_FIRST);
            if(!(soft_stop.tract_followed & (1u<<bit))) {
                if(*begin>anchor)*begin=anchor;
                if(*end<anchor)*end=anchor;
                soft_stop.tract_followed|=(uint16_t)(1u<<bit);
            }
        } else if(stream>=frication && stream<=frication+6) {
            /* The shortened native burst fades while AH fills the freed
             * closure, ending at the original voicing boundary. */
            unsigned bit=(unsigned)(stream-frication);
            if(!(soft_stop.frication_seen & (1u<<bit))) {
                if(value==0 && *begin>anchor)*begin=anchor;
                soft_stop.frication_seen|=(uint16_t)(1u<<bit);
            }
        }
    }
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
