/* Shared acoustic and duration decisions. Values remain ordinary C scalars
 * throughout this block; there are no registered temporary cells, rule
 * frames, speculative alternatives or per-call heap allocations. */
#include "reloq61_segment_rules.h"
#include "reloq61_scalar_state.h"

struct cursor {
    delta_tpos left,right;
    unsigned char scan[8];
    int8_t compared,fences,testing;
};
static struct cursor save_cursor(delta_state *d)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    struct cursor saved={.left=d->lpta,.right=d->rpta,
        .compared=v->compared_equal,.fences=v->fence_count,.testing=v->testing};
    memcpy(saved.scan,&v->scan_ptr,8);v->fence_count=0;v->testing=0;
    return saved;
}
static int32_t finish(delta_state *d,const struct cursor *saved,int32_t status)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    d->lpta=saved->left;d->rpta=saved->right;
    memcpy(&v->scan_ptr,saved->scan,8);v->compared_equal=saved->compared;
    v->fence_count=saved->fences;v->testing=saved->testing;
    return reloq_scalar_finish(d,status);
}
static unsigned char *fields(void *d){return(unsigned char *)d+DG_BASE;}
static int16_t short_at(void *d,unsigned offset)
{int16_t value;memcpy(&value,fields(d)+offset,2);return value;}
static int32_t long_at(void *d,unsigned offset)
{int32_t value;memcpy(&value,fields(d)+offset,4);return value;}
static void set_short(void *d,unsigned offset,int16_t value)
{memcpy(fields(d)+offset,&value,2);}
static int read_cell(delta_state *d,int32_t ref,int16_t kind,delta_loc *out)
{return !ref||get_parm(d,out,EVV_AT(delta_loc *,ref),kind);}
static int read_position(int32_t ref,delta_token *out)
{
    if(!ref)return 1;
    memset(out,0,sizeof *out);
    memcpy(&out->value,EVV_AT(const unsigned char *,ref)+4,4);return 0;
}
struct target {delta_loc value,begin,end;delta_token left,right;};
static int read_target(delta_state *d,struct target *t,int32_t value,
    int32_t begin,int32_t end,int32_t left,int32_t right)
{
    return read_cell(d,value,DK_SHORT2,&t->value)||
        read_cell(d,begin,DK_FIXED,&t->begin)||read_cell(d,end,DK_FIXED,&t->end)||
        read_position(left,&t->left)||read_position(right,&t->right);
}
static void submit(delta_state *d,int16_t stream,int16_t value,int32_t begin,int32_t end)
{reloq61_submit_interval(d,stream,value,begin,end);}

int32_t reloq61_segment_point(void *state,int32_t stream_arg,int32_t value_arg,
    int32_t time_arg,int32_t flag_arg,int32_t position_arg)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;struct cursor saved=save_cursor(d);
    delta_loc stream={0},value={0},time={0},flag={0};delta_token position={0};
    if(read_cell(d,stream_arg,DK_SHORT2,&stream)||read_cell(d,value_arg,DK_SHORT2,&value)||
       read_cell(d,time_arg,DK_FIXED,&time)||read_cell(d,flag_arg,DK_SHORT2,&flag)||
       read_position(position_arg,&position))return finish(d,&saved,94);
    /* The historical flag adjustment never changes the submitted point. */
    submit(d,stream.field,value.field,time.value,time.value);
    return finish(d,&saved,0);
}
int32_t reloq61_segment_voicing(void *state,const struct reloq61_segment_layout *layout,
    int32_t value,int32_t begin,int32_t end,int32_t left,int32_t right)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;struct cursor saved=save_cursor(d);struct target t={0};
    if(read_target(d,&t,value,begin,end,left,right))return finish(d,&saved,94);
    delta_loc *voice=(delta_loc *)(fields(d)+layout->voice);
    voice->field=6;
    if(if_testeq_v_i(d,voice,100)||short_at(d,layout->force_voicing)) {
        int32_t stop=long_at(d,layout->point_voicing)?t.begin.value:t.end.value;
        submit(d,short_at(d,layout->av_stream),t.value.field,t.begin.value,stop);
    }
    return finish(d,&saved,0);
}
int32_t reloq61_segment_frication_gain(void *state,const struct reloq61_segment_layout *layout,
    int32_t value,int32_t begin,int32_t end,int32_t left,int32_t right,int gain)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;struct cursor saved=save_cursor(d);struct target t={0};
    if(read_target(d,&t,value,begin,end,left,right))return finish(d,&saved,94);
    if(t.value.field>0)t.value.field=(int16_t)(t.value.field+gain);
    lpta_loadpn(d,&t.left);rpta_loadpn(d,&t.right);
    int at_boundary=0;
    if(!compare_ptas(d)&&!testeq(d)) {
        delta_token boundary={0};boundary.value=long_at(d,layout->phone_boundary);
        lpta_loadpn(d,&t.right);rpta_loadpn(d,&boundary);
        at_boundary=!compare_ptas(d)&&!testeq(d);
    }
    int32_t start=at_boundary?long_at(d,layout->phone_end):t.begin.value;
    submit(d,short_at(d,layout->af_stream),t.value.field,start,at_boundary?start:t.end.value);
    return finish(d,&saved,0);
}
int32_t reloq61_segment_frication(void *state,const struct reloq61_segment_layout *layout,
    int32_t value,int32_t begin,int32_t end,int32_t left,int32_t right)
{
    return reloq61_segment_frication_gain(state,layout,value,begin,end,left,right,0);
}
int32_t reloq61_segment_nasal_zero(void *state,const struct reloq61_segment_layout *layout,
    reloq61_endpoint_filter filter,int32_t value,int32_t begin,int32_t end,int32_t left,int32_t right)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;struct cursor saved=save_cursor(d);struct target t={0};
    if(read_target(d,&t,value,begin,end,left,right))return finish(d,&saved,94);
    int16_t stream=short_at(d,layout->zero_stream);
    int16_t target=filter(d,stream,t.value.field);
    submit(d,stream,target,t.begin.value,t.left.value==t.right.value?t.begin.value:t.end.value);
    return finish(d,&saved,0);
}
int32_t reloq61_segment_nasal_pole(void *state,const struct reloq61_segment_layout *layout,
    reloq61_endpoint_filter filter,int32_t value,int32_t begin,int32_t end,int32_t left,int32_t right)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;struct cursor saved=save_cursor(d);struct target t={0};
    if(read_target(d,&t,value,begin,end,left,right))return finish(d,&saved,94);
    int16_t stream=short_at(d,layout->pole_stream);
    submit(d,stream,filter(d,stream,t.value.field),t.begin.value,t.end.value);
    return finish(d,&saved,0);
}

static int scan_phone(delta_state *d,const delta_token *position)
{lpta_loadp(d,position);return !setscan_r(d,2);}
static int32_t speed_value(delta_state *d,const struct reloq61_segment_layout *layout,
    int32_t duration,const delta_token *position)
{
    int32_t before=long_at(d,layout->speed_before),after=long_at(d,layout->speed_after);
    if(before==1000000&&after==1000000)return duration;
    if(scan_phone(d,position)&&!testFldeq(d,2,0,51)&&
        !testFldeq(d,2,4,layout->pause_class)&&!advance_tok(d))
        return duration;
    int32_t factor=before;
    if(before!=after) {
        delta_token nucleus={0};nucleus.value=long_at(d,layout->nucleus);
        lpta_loadpn(d,position);rpta_loadpn(d,&nucleus);
        if(!compare_ptas(d)&&!testeq(d)) {
            if(scan_phone(d,position)&&!testFldeq(d,2,layout->speed_field,1)&&!advance_tok(d))factor=after;
            else if(scan_phone(d,position)&&!testFldeq(d,2,1,1)&&!advance_tok(d)) {
                set_short(d,layout->unscaled,1);return duration;
            }
        }
    }
    if(factor==1000000)return duration;
    int32_t scaled=delta_muldiv(factor,duration,10000,1);
    return delta_muldiv(scaled,10000,1000000,1);
}
int32_t reloq61_segment_speed(void *state,const struct reloq61_segment_layout *layout,
    int32_t duration_arg,int32_t position_arg)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;struct cursor saved=save_cursor(d);
    delta_loc duration={0};delta_token position={0};
    if(read_cell(d,duration_arg,DK_FIXED,&duration)||read_position(position_arg,&position))
        return finish(d,&saved,94);
    int32_t value=speed_value(d,layout,duration.value,&position);
    memcpy(EVV_AT(unsigned char *,duration_arg)+4,&value,4);
    return finish(d,&saved,0);
}
int32_t reloq61_segment_duration(void *state,const struct reloq61_segment_layout *layout,
    int32_t duration_arg,int32_t left_arg,int32_t right_arg,int32_t phone_arg)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;struct cursor saved=save_cursor(d);
    delta_loc duration={0},phone={0};delta_token left={0},right={0};
    if(read_cell(d,duration_arg,DK_FIXED,&duration)||read_position(left_arg,&left)||
       read_position(right_arg,&right)||read_cell(d,phone_arg,DK_SHORT2,&phone))
        return finish(d,&saved,94);
    int plain=short_at(d,layout->focus)==short_at(d,layout->neutral);
    int focused=short_at(d,layout->syllable_count)<short_at(d,layout->first_count)&&
        short_at(d,layout->stress)==short_at(d,layout->ordinary_stress)&&
        (short_at(d,layout->previous_focus)!=short_at(d,layout->neutral)||short_at(d,layout->word_type)==1);
    if(!plain&&focused) {
        duration.value=delta_muldiv(duration.value,120000,10000,1);
        duration.value=delta_muldiv(duration.value,10000,100000,1);
    }
    duration.value=speed_value(d,layout,duration.value,&left);
    duration.value=(int32_t)((uint32_t)duration.value+5000u);
    int32_t original=duration.value;
    int32_t minimum=delta_muldiv(short_at(d,layout->minimum),10000,1,0);
    int32_t short_minimum=delta_muldiv(short_at(d,layout->short_minimum),10000,1,0);
    int is_short_pause=phone.field==short_at(d,layout->short_pause);
    if(duration.value<minimum) {
        int32_t floor=is_short_pause?short_minimum:minimum;
        if(duration.value<floor)duration.value=floor;
    }
    /* A refused range retains the bounded alternatives before clamping. */
    int32_t alternatives[3]={duration.value,original,original};
    unsigned attempts=2;
    if(original<minimum&&!is_short_pause) {
        alternatives[1]=original<short_minimum?short_minimum:original;
        attempts=3;
    }
    unsigned attempt;
    for(attempt=0;attempt<attempts;++attempt) {
        duration.value=alternatives[attempt];lpta_rpta_loadp(d,&left,&right);
        if(!insert_2ptv(d,8,&duration,0))break;
    }
    if(attempt==attempts)return finish(d,&saved,94);
    if(phone.field==short_at(d,layout->pause)) {
        int32_t rounded=(int32_t)((uint32_t)duration.value+1u);
        if(rounded<1)rounded=(int32_t)((uint32_t)rounded-2u);
        set_short(d,layout->pause_duration,(int16_t)delta_muldiv(rounded,1,10000,0));
    }
    return finish(d,&saved,0);
}
