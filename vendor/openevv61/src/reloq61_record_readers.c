/* Direct record traversal. These readers need no registered local cells,
 * speculative alternatives, rule frames or temporary heap allocations.
 * Cursor changes are confined to this call; only the requested answer is
 * committed. The two language adapters share the same projection schema. */
#include "reloq61_record_readers.h"
#include "reloq61_scalar_state.h"

struct cursor {
    delta_tpos left,right;
    unsigned char scan[8];
    int8_t compared,fences,testing;
};
static struct cursor save_cursor(delta_state *d)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    struct cursor s;
    s.left=d->lpta;s.right=d->rpta;s.compared=v->compared_equal;
    s.fences=v->fence_count;s.testing=v->testing;
    memcpy(s.scan,&v->scan_ptr,8);
    v->fence_count=0;v->testing=0;
    return s;
}
static void restore_cursor(delta_state *d,const struct cursor *s)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    d->lpta=s->left;d->rpta=s->right;memcpy(&v->scan_ptr,s->scan,8);
    v->compared_equal=s->compared;v->fence_count=s->fences;v->testing=s->testing;
}
static void at(delta_state *d,int32_t position)
{d->lpta.flags=1;d->lpta.node=position;d->lpta.offset=0;}
static int32_t position(delta_state *d,int32_t ref)
{
    int32_t value;
    (void)d;
    if(!ref)return 0;
    memcpy(&value,EVV_AT(const unsigned char *,ref)+4,4);
    return value;
}
static int32_t peek(delta_state *d)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    const int32_t *node=EVV_AT(const int32_t *,v->scan_ptr);
    return node[v->scan_rev?v->fence_base+v->scan_field:3+v->scan_field]&~3;
}
int32_t reloq61_record_in_syllable(void *state,int32_t from,int32_t start,int32_t end)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;
    int32_t p=position(d,from),a=position(d,start),b=position(d,end);
    if(!p||!a||!b)return reloq_scalar_finish(d,94);
    int status=0;
    if(p!=b) {
        struct cursor saved=save_cursor(d);
        at(d,p);lpta_ctxtl(d,7);
        status=d->lpta.node==a?0:94;
        restore_cursor(d,&saved);
    }
    return reloq_scalar_finish(d,status);
}
int32_t reloq61_record_unstressed(void *state,int32_t from,int32_t end,unsigned stress_field)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;
    int32_t a=position(d,from),b=position(d,end);
    if(!a||!b)return reloq_scalar_finish(d,94);
    struct cursor saved=save_cursor(d);int status=94;
    at(d,a);
    if(!setscan_r(d,4)) {
        for(;;) {
            unsigned char scan[8];
            memcpy(scan,&EVV_AT(delta_vars *,d->vars)->scan_ptr,8);
            at(d,b);
            if(!test_ptr(d)){status=0;break;}
            memcpy(&EVV_AT(delta_vars *,d->vars)->scan_ptr,scan,8);
            if(testFldeq(d,4,(uint8_t)stress_field,0) || advance_tok(d))break;
        }
    }
    restore_cursor(d,&saved);return reloq_scalar_finish(d,status);
}
int32_t reloq61_record_last_accent(void *state)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;unsigned char *g=(unsigned char *)d+DG_BASE;
    int32_t start,end,result;
    memcpy(&start,g+176,4);memcpy(&end,g+184,4);result=start;
    struct cursor saved=save_cursor(d);
    unsigned char index[256],marks[256],chars[256];
    memcpy(index,EVV_AT(const void *,d->fence_index),d->nstmts);
    memcpy(marks,EVV_AT(const void *,d->fence_marks),d->nstmts);
    memcpy(chars,EVV_AT(const void *,d->fence_chars),d->nstmts);
    static const uint8_t word_fence[]={4};
    fence(d,1,word_fence);at(d,end);
    if(!setscan_l(d,7))for(;;) {
        int32_t node=peek(d);int16_t stress=0;
        if(!node)break;
        int record=!(*EVV_AT(const int32_t *,node)&2);
        if(record)memcpy(&stress,delta_record_field(&vstmtbl[7], 0, TFLDS(EVV_AT(void *,node))),2);
        if(!vscanadv(d,1,1))break;
        if(record && stress==1){result=EVV_AT(delta_vars *,d->vars)->scan_ptr;break;}
    }
    restore_cursor(d,&saved);
    memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
    memcpy(g+192,&result,4);
    return reloq_scalar_finish(d,0);
}
