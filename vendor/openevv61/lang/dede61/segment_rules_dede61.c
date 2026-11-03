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
/* Source modes measured in the previous German composer. Four-phase r
 * occurs initially and after an unstressed vowel before a stressed syllable;
 * stop/fricative clusters keep their own native source and diplophonia. */
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
    if(begins_projection(d,&r,4))return 1;
    if(!prior_phone(d,&r,&previous)||reloq61_spr_feature(d,&previous,1)!=1||
       !reloq61_spr_phone_at(d,r.after,&next))return 0;
    if(reloq61_spr_feature(d,&next,0)==36)return 3; /* short /u/ */
    int before=syllable_stress(d,&previous),after=syllable_stress(d,&r);
    unsigned vowel=reloq61_spr_feature(d,&next,0);
    if(before==1&&after==0)return 0;
    if((before==0&&after==1)||(vowel>=27&&vowel<=32)||
       vowel==39||vowel==40||vowel==41||vowel==42||vowel==48)return 1;
    return 0;
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

/* Four-phase source is used only in the old composer's matching contexts. */
static int consonant_r_voicing(void *state,int16_t stream,int16_t value,int32_t from,int32_t to)
{
    if(reloq_dede61_old_r_source(state)!=1)return 0;
    int32_t first,last;memcpy(&first,(const unsigned char *)state+DG_BASE+1048,4);
    memcpy(&last,(const unsigned char *)state+DG_BASE+1056,4);
    if(last<=first||last-first<60000)return 0;
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
        /* Apply the established high-rate source compensation to German
         * /f/ and /v/ (orthographic W), for text and phonetic input alike.
         * It is independent of the male /ts/ marker, including /tsv/. */
        if(name==7 || name==8)gain+=reloq61_labiodental_gain(state);
    }
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}
int32_t reloq_dede61_doFNZ(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_nasal_zero(state,&layout,reloq_dede61_filter_endpoint,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_doFNP(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_nasal_pole(state,&layout,reloq_dede61_filter_endpoint,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_apply_speed_anno(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_segment_speed(state,&layout,p1,p2);}
static int32_t german_duration(void *state,int16_t kind,int32_t value)
{
    delta_state *d=state;int32_t pos;
    memcpy(&pos,(const unsigned char *)state+DG_BASE+712,sizeof pos);
    struct reloq61_spr_phone current,next,prior;
    if(kind==3&&reloq_dede61_final_text_a(state)&&value<=INT32_MAX-162500)value+=162500;
    if(kind==0&&
       reloq61_spr_phone_at(d,pos,&current)&&reloq61_spr_phone_at(d,current.after,&next)) {
        unsigned id=reloq61_spr_feature(d,&current,0),following=reloq61_spr_feature(d,&next,0);
        int32_t replacement=0;
        if((seven_und_n(d,&current,&next)||nine_und_n(d,&current,&next))&&value>300000) {
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
