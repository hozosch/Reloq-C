/* Direct language bindings for shared segment and duration decisions. */
#include "reloq61_segment_rules.h"
#include "reloq61_spr_output.h"
#include "reloq61.h"
#include <string.h>
extern int16_t reloq_dede61_filter_endpoint(void *,int16_t,int16_t);
extern int reloq_dede61_final_text_a(void *);
extern int reloq61_german_affricate_markers(void *);

static int prior_phone(delta_state *d,const struct reloq61_spr_phone *p,
    struct reloq61_spr_phone *prior)
{
    int32_t at=*EVV_AT(const int32_t *,p->node)&~3;
    int32_t node=vgetsc(d,1,0,at,2);
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,1,0,node,2);
    if(!node)return 0;
    *prior=(struct reloq61_spr_phone){node,EVV_AT(const int32_t *,node)[1]&~3};
    return prior->after!=0;
}
static int phone_is(delta_state *d,const struct reloq61_spr_phone *p,unsigned name,unsigned combo)
{return reloq61_spr_feature(d,p,0)==name&&reloq61_spr_feature(d,p,14)==combo;}
static int seven_und_n(delta_state *d,const struct reloq61_spr_phone *n,
    const struct reloq61_spr_phone *u)
{
    struct reloq61_spr_phone schwa,b,i,z,next,t;
    return reloq61_spr_feature(d,n,0)==22&&reloq61_spr_feature(d,u,0)==36&&
        reloq61_spr_feature(d,u,15)==0&&prior_phone(d,n,&schwa)&&
        reloq61_spr_feature(d,&schwa,0)==43&&prior_phone(d,&schwa,&b)&&
        reloq61_spr_feature(d,&b,0)==2&&prior_phone(d,&b,&i)&&
        reloq61_spr_feature(d,&i,0)==28&&prior_phone(d,&i,&z)&&
        reloq61_spr_feature(d,&z,0)==10&&
        reloq61_spr_phone_at(d,u->after,&next)&&reloq61_spr_feature(d,&next,0)==22&&
        reloq61_spr_phone_at(d,next.after,&t)&&reloq61_spr_feature(d,&t,0)==3;
}
/* The coda of neun contracts to the same 30-ms body before connected und;
 * a standalone neun and an explicitly glottal next word are unaffected. */
static int nine_und_n(delta_state *d,const struct reloq61_spr_phone *n,
    const struct reloq61_spr_phone *u)
{
    struct reloq61_spr_phone j,o,onset,next,t;
    return reloq61_spr_feature(d,n,0)==22&&reloq61_spr_feature(d,u,0)==36&&
        reloq61_spr_feature(d,u,15)==0&&prior_phone(d,n,&j)&&
        reloq61_spr_feature(d,&j,0)==26&&prior_phone(d,&j,&o)&&
        reloq61_spr_feature(d,&o,0)==38&&prior_phone(d,&o,&onset)&&
        reloq61_spr_feature(d,&onset,0)==22&&
        reloq61_spr_phone_at(d,u->after,&next)&&reloq61_spr_feature(d,&next,0)==22&&
        reloq61_spr_phone_at(d,next.after,&t)&&reloq61_spr_feature(d,&t,0)==3;
}
static int begins_projection(delta_state *d,const struct reloq61_spr_phone *p,unsigned projection)
{
    int32_t left=*EVV_AT(const int32_t *,p->node)&~3;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    return (*EVV_AT(const int32_t *,left+(v->fence_base+projection)*4)&1)!=0;
}
static int z_affricate(delta_state *d)
{
    int32_t at;memcpy(&at,(const char *)d+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone p,head,next,last;
    if(!reloq61_spr_phone_at(d,at,&p))return 0;
    if(phone_is(d,&p,3,2))head=p; /* t of /ts/ */
    else if(phone_is(d,&p,9,2)) {
        if(!prior_phone(d,&p,&head)||!phone_is(d,&head,3,2))return 0;
    } else if(phone_is(d,&p,8,0)) {
        if(begins_projection(d,&p,7)||!prior_phone(d,&p,&last)||
           !phone_is(d,&last,9,2)||!prior_phone(d,&last,&head)||
           !phone_is(d,&head,3,2))return 0;
        return 1; /* /v/ in the same syllable as /ts/ */
    } else return 0;
    if(begins_projection(d,&head,4))return 1; /* word-initial /ts/ */
    return reloq61_spr_phone_at(d,head.after,&next)&&phone_is(d,&next,9,2)&&
        reloq61_spr_phone_at(d,next.after,&last)&&phone_is(d,&last,8,0)&&
        !begins_projection(d,&last,7);
}
int reloq_dede61_number_four_r(void *state)
{
    delta_state *d=state;int32_t at;
    memcpy(&at,(const unsigned char *)d+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone r,i,f,u,n,t;
    return reloq61_spr_phone_at(d,at,&r)&&reloq61_spr_feature(d,&r,0)==15&&
        prior_phone(d,&r,&i)&&reloq61_spr_feature(d,&i,0)==28&&
        prior_phone(d,&i,&f)&&reloq61_spr_feature(d,&f,0)==7&&
        reloq61_spr_phone_at(d,r.after,&u)&&reloq61_spr_feature(d,&u,0)==36&&
        reloq61_spr_feature(d,&u,15)==0&&
        reloq61_spr_phone_at(d,u.after,&n)&&reloq61_spr_feature(d,&n,0)==22&&
        reloq61_spr_phone_at(d,n.after,&t)&&reloq61_spr_feature(d,&t,0)==3;
}
int reloq_dede61_number_four_und(void *state)
{
    delta_state *d=state;int32_t at;
    memcpy(&at,(const unsigned char *)state+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone u,r,i,f,n,t;
    return reloq61_spr_phone_at(d,at,&u)&&reloq61_spr_feature(d,&u,0)==36&&
        reloq61_spr_feature(d,&u,15)==0&&prior_phone(d,&u,&r)&&
        reloq61_spr_feature(d,&r,0)==15&&prior_phone(d,&r,&i)&&
        reloq61_spr_feature(d,&i,0)==28&&prior_phone(d,&i,&f)&&
        reloq61_spr_feature(d,&f,0)==7&&
        reloq61_spr_phone_at(d,u.after,&n)&&reloq61_spr_feature(d,&n,0)==22&&
        reloq61_spr_phone_at(d,n.after,&t)&&reloq61_spr_feature(d,&t,0)==3;
}
int reloq_dede61_consonant_r(void *state)
{
    delta_state *d=state;int32_t at;
    memcpy(&at,(const unsigned char *)state+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone p,previous;
    if(!reloq61_spr_phone_at(d,at,&p)||reloq61_spr_feature(d,&p,0)!=15)return 0;
    /* Postvocalic /r/ in the same syllable is the vowel source. */
    if(!begins_projection(d,&p,7)&&prior_phone(d,&p,&previous)&&
       reloq61_spr_feature(d,&previous,1)==1&&!reloq_dede61_number_four_r(state))return 0;
    return 1;
}
int reloq_dede61_initial_r(void *state)
{
    delta_state *d=state;int32_t at;
    memcpy(&at,(const unsigned char *)state+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone r;
    return reloq_dede61_consonant_r(state)&&reloq61_spr_phone_at(d,at,&r)&&
        begins_projection(d,&r,4);
}
/* Source modes from the previous German composer: a four-phase long onset,
 * a two-phase medial onset, or a continuously voiced medial onset. Cluster
 * consonants keep their separate excitation and diplophonia. */
static int syllable_stress(delta_state *d,const struct reloq61_spr_phone *p)
{
    int32_t left=*EVV_AT(const int32_t *,p->node)&~3;
    int32_t owner=vgetsc(d,1,1,left,7);
    int32_t node=owner?vgetsc(d,0,0,owner,7):0;
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,0,0,node,7);
    if(!node)return -1;
    int16_t stress;memcpy(&stress,delta_record_field(&vstmtbl[7],0,TFLDS(EVV_AT(void *,node))),2);
    return stress;
}
int reloq_dede61_old_r_source(void *state)
{
    delta_state *d=state;int32_t at;
    memcpy(&at,(const unsigned char *)state+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone r,previous,next;
    if(!reloq_dede61_consonant_r(state)||!reloq61_spr_phone_at(d,at,&r))return 0;
    if(reloq_dede61_number_four_r(state))return 3;
    if(!begins_projection(d,&r,7)||!reloq61_spr_phone_at(d,r.after,&next))return 0;
    unsigned vowel=reloq61_spr_feature(d,&next,0);
    /* The former cluster planner uses a continuous R before an onset J. */
    if(vowel==26) {
        struct reloq61_spr_phone after_j;
        return reloq61_spr_phone_at(d,next.after,&after_j)&&
            !begins_projection(d,&next,7)&&
            reloq61_spr_feature(d,&after_j,1)==1?3:0;
    }
    if(reloq61_spr_feature(d,&next,1)!=1)return 0;
    if(!prior_phone(d,&r,&previous))return 1;
    unsigned prior_name=reloq61_spr_feature(d,&previous,0);
    if(prior_name==51)return 1; /* fresh utterance after a pause */
    int before=syllable_stress(d,&previous),after=syllable_stress(d,&r);
    int prior_vowel=reloq61_spr_feature(d,&previous,1)==1;
    if(prior_vowel&&before==1&&after==0)return vowel==36?3:2;
    if((before==0&&after==1)||(vowel>=27&&vowel<=32)||
       vowel==39||vowel==40||vowel==41||vowel==42||vowel==48)return 1;
    if(vowel==36&&prior_vowel)return 3;
    return 2;
}
static const struct reloq61_segment_layout layout={
    .pause_class=8,.speed_field=15,
    .speed_before=464,
    .speed_after=472,
    .nucleus=184,
    .unscaled=478,
    .short_pause=426,
    .pause=418,
    .first_count=222,
    .stress=374,
    .ordinary_stress=354,
    .word_type=210,
    .phone_boundary=720,
    .voice=3104,
    .force_voicing=2040,
    .point_voicing=1096,
    .av_stream=938,
    .af_stream=946,
    .pole_stream=906,
    .zero_stream=910,
    .phone_end=1056,
    .minimum=2250,
    .short_minimum=2254,
    .pause_duration=3070,
    .focus=3382,
    .neutral=3358,
    .previous_focus=3370,
    .syllable_count=1794
};
int32_t reloq_dede61_addStreamArrayPtVal(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_point(state,p1,p2,p3,p4,p5);}

/* A vocalized syllable-final r after long ö shares the vowel's AV curve.
 * Lower its final target gently, so the existing interpolation spreads the
 * adjustment over the connected vowel/r rather than creating a level step. */
static int long_oe_before_coda_r(delta_state *d)
{
    int32_t at;memcpy(&at,(const unsigned char *)d+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone vowel,r;
    if(!reloq61_spr_phone_at(d,at,&vowel)||
       reloq61_spr_feature(d,&vowel,0)!=41||
       !reloq61_spr_phone_at(d,vowel.after,&r)||begins_projection(d,&r,7))return 0;
    unsigned name=reloq61_spr_feature(d,&r,0);
    return name==15||(name==25&&reloq61_spr_feature(d,&r,1)==2);
}

/* Four-phase source is used only in the old composer's matching contexts. */
static int consonant_r_voicing(void *state,int16_t stream,int16_t value,int32_t from,int32_t to)
{
    int32_t phone_end;memcpy(&phone_end,(const unsigned char *)state+DG_BASE+1056,4);
    if(value>2&&from==to&&from>=phone_end&&long_oe_before_coda_r(state)) {
        reloq61_submit_interval(state,stream,(int16_t)(value-2),from,to);
        return 1;
    }
    int mode=reloq_dede61_old_r_source(state);
    if(mode!=1&&mode!=3)return 0;
    int32_t first,last;memcpy(&first,(const unsigned char *)state+DG_BASE+1048,4);
    memcpy(&last,(const unsigned char *)state+DG_BASE+1056,4);
    if(last<=first)return 0;
    if(mode==3) {
        /* The old R+J and medial short-U gestures are voiced throughout,
         * even if the native onset would otherwise start with two phases. */
        if(from==first&&to<last&&value==0) {
            reloq61_submit_interval(state,stream,41,first,last);
            return 1;
        }
        if(from>first&&to==last&&value==28)return 1;
        return 0;
    }
    if(last-first<60000)return 0;
    int64_t span=(int64_t)last-first;
    int64_t third=(int64_t)first+span*7/10+10000;
    if(third+10000>=last)return 0;
    if(from==first&&to<last&&value==0) {
        reloq61_submit_interval(state,stream,0,first,(int32_t)third);
        reloq61_submit_interval(state,stream,41,(int32_t)(third+10000),last);
        return 1;
    }
    if(from>first&&to==last&&value==28)return 1;
    return 0;
}
int32_t reloq_dede61_doAV(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_voicing_custom(state,&layout,p1,p2,p3,p4,p5,
    reloq_dede61_consonant_r(state)-reloq_dede61_final_text_a(state),consonant_r_voicing);}
int32_t reloq_dede61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;
    int mark=reloq61_german_affricate_markers(state)&&z_affricate(state);
    int gain=(mark?RELOQ61_DE_AF_MARK:0)+reloq_dede61_consonant_r(state);
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone phone;
    if(reloq61_spr_phone_at(state,at,&phone)) {
        unsigned name=reloq61_spr_feature(state,&phone,0);
        struct reloq61_spr_phone next;
        /* The earlier German planner uses the ordinary /s/ excitation for
         * word-initial /sm/ and /sn/, while the native rule lowers it by
         * 12. Keep the native transition, and leave medial clusters and
         * affricates alone. Match phone records for text and SPR alike. */
        if(name==9 && reloq61_spr_feature(state,&phone,14)==0 &&
           begins_projection(state,&phone,4) &&
           reloq61_spr_phone_at(state,phone.after,&next) &&
           !begins_projection(state,&next,7) &&
           (reloq61_spr_feature(state,&next,0)==21 ||
            reloq61_spr_feature(state,&next,0)==22))gain+=12;
        /* Apply the established high-rate source compensation to German
         * /f/ and /v/ (orthographic W), for text and phonetic input alike.
         * It is independent of the male /ts/ marker, including /tsv/. */
        if(name==7 || name==8)gain+=reloq61_labiodental_gain(state);
    }
    /* Explicit SPR places four's R at the following syllable onset, where
     * the native rule otherwise supplies zero frication. Restore the text
     * path's source only for that recognised connected-number gesture. */
    return reloq61_segment_frication_gain_default(state,&layout,p1,p2,p3,p4,p5,
        gain,reloq_dede61_number_four_r(state)?45:0);
}
int32_t reloq_dede61_doFNZ(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_nasal_zero(state,&layout,reloq_dede61_filter_endpoint,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_doFNP(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_nasal_pole(state,&layout,reloq_dede61_filter_endpoint,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_apply_speed_anno(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_segment_speed(state,&layout,p1,p2);}
/* The connected unit ends a word or precedes the stressed forty syllable.
 * Other complete number compounds already have the intended timing. */
static int connected_number_aj_context(delta_state *d,const struct reloq61_spr_phone *j)
{
    struct reloq61_spr_phone a,liquid,start,u,n,t,after,i,r,ts;
    if(!phone_is(d,j,26,7)||!prior_phone(d,j,&a)||!phone_is(d,&a,34,7)||
       !prior_phone(d,&a,&liquid)||!prior_phone(d,&liquid,&start))return 0;
    unsigned name=reloq61_spr_feature(d,&liquid,0);
    if(!((name==15&&reloq61_spr_feature(d,&start,0)==4)||
         (name==8&&phone_is(d,&start,9,2))))return 0;
    if(!reloq61_spr_phone_at(d,j->after,&u)||reloq61_spr_feature(d,&u,0)!=36||
       reloq61_spr_feature(d,&u,15)!=0||!reloq61_spr_phone_at(d,u.after,&n)||
       reloq61_spr_feature(d,&n,0)!=22||!reloq61_spr_phone_at(d,n.after,&t)||
       reloq61_spr_feature(d,&t,0)!=3)return 0;
    if(!reloq61_spr_phone_at(d,t.after,&after)||
       reloq61_spr_feature(d,&after,0)==51||begins_projection(d,&after,4))return 1;
    /* /f I r ts/ is the native forty root, for digits and written numbers.
     * Its stressed und syllable otherwise shortens the preceding glide. */
    if(reloq61_spr_feature(d,&after,0)==7&&
       reloq61_spr_phone_at(d,after.after,&i)&&reloq61_spr_feature(d,&i,0)==29&&
       reloq61_spr_phone_at(d,i.after,&r)&&reloq61_spr_feature(d,&r,0)==25&&
       reloq61_spr_phone_at(d,r.after,&ts)&&reloq61_spr_feature(d,&ts,0)==3&&
       reloq61_spr_feature(d,&ts,14)==2)return 2;
    return 0;
}
/* The native nucleus distributor runs when the following U is current.
 * Restore the earlier connected-word nucleus length before it distributes
 * time across A and J; changing either segment alone only redistributes time. */
void reloq_dede61_adjust_connected_number_nucleus(void *state)
{
    delta_state *d=state;int32_t at,duration;
    memcpy(&at,(const unsigned char *)d+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone u,j;
    if(!reloq61_spr_phone_at(d,at,&u)||reloq61_spr_feature(d,&u,0)!=36||
       !prior_phone(d,&u,&j))return;
    int context=connected_number_aj_context(d,&j);
    int32_t bonus=context==1?230000:context==2?140000:0;
    if(!bonus)return;
    memcpy(&duration,(const unsigned char *)d+DG_BASE+2696,sizeof duration);
    if(duration<=0||duration>INT32_MAX-bonus)return;
    duration+=bonus;
    memcpy((unsigned char *)d+DG_BASE+2696,&duration,sizeof duration);
}
static int32_t german_duration(void *state,int16_t kind,int32_t value)
{
    delta_state *d=state;int32_t pos;
    memcpy(&pos,(const unsigned char *)state+DG_BASE+712,sizeof pos);
    struct reloq61_spr_phone current,next,prior;
    if(kind==3&&reloq_dede61_final_text_a(state)&&value<=INT32_MAX-162500)value+=162500;
    if(kind==3&&reloq61_spr_phone_at(d,pos,&current)&&
       phone_is(d,&current,26,7)&&connected_number_aj_context(d,&current)==2&&
       value>=200000)value-=200000;
    if(kind==0&&
       reloq61_spr_phone_at(d,pos,&current)&&reloq61_spr_phone_at(d,current.after,&next)) {
        unsigned id=reloq61_spr_feature(d,&current,0),following=reloq61_spr_feature(d,&next,0);
        int32_t replacement=0;
        if(id==15&&following==26&&reloq_dede61_old_r_source(state)==3) {
            /* The old continuous R+J onset holds R for 44 unscaled ticks. */
            replacement=440000;
        } else if((seven_und_n(d,&current,&next)||nine_und_n(d,&current,&next))&&value>300000) {
            /* In the connected number word the preceding n is the short
             * 30-ms nasal, rather than the standalone seven's longer coda. */
            replacement=300000;
        } else if(id==7&&reloq61_spr_feature(d,&current,14)==1&&following!=51&&
           reloq61_spr_feature(d,&next,1)!=1&&begins_projection(d,&next,7)) {
            int16_t stress;memcpy(&stress,(const unsigned char *)d+DG_BASE+374,2);
            int stop=following==1||following==3||following==5;
            replacement=(stop?(stress?100:120):(stress?81:105))*10000;
        } else if(id==9&&reloq61_spr_feature(d,&current,14)==0&&following==7&&
                  begins_projection(d,&next,7)&&prior_phone(d,&current,&prior)&&
                  reloq61_spr_feature(d,&prior,0)==22) {
            replacement=650000;
        }
        if(replacement)value=replacement;
    }
    return value;
}
int32_t reloq_dede61_insert_dur(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4)
{(void)p0;return reloq61_segment_duration_with(state,&layout,p1,p2,p3,p4,german_duration);}
