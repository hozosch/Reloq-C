/* Direct, shared input accounting. Counting follows the selected record
 * projection once; it does not create rule frames, alternatives or pointer
 * variables. The language files supply only their compatibility layout. */
#include "reloq61_input_counts.h"
#include "reloq61_scalar_state.h"

/* Compatibility cells may start at a two-byte boundary. Copy their payload
 * before typed access and write only the output half-word back. */
static int32_t position(int32_t ref)
{int32_t n;memcpy(&n,EVV_AT(const unsigned char *,ref)+4,4);return n;}
static void put_field(int32_t ref,int16_t n)
{memcpy(EVV_AT(unsigned char *,ref)+2,&n,2);}
static unsigned char *fields(delta_state *d){return (unsigned char *)d+DG_BASE;}
static int16_t short_at(delta_state *d,unsigned offset)
{int16_t n;memcpy(&n,fields(d)+offset,2);return n;}
static int32_t long_at(delta_state *d,unsigned offset)
{int32_t n;memcpy(&n,fields(d)+offset,4);return n;}
static void put_short(delta_state *d,unsigned offset,int16_t n)
{memcpy(fields(d)+offset,&n,2);}
static int read_short(delta_state *d,int32_t ref,int16_t *n)
{
    delta_loc input,value={0};
    if(!ref)return 0;
    memcpy(&input,EVV_AT(const void *,ref),sizeof input);
    switch(input.kind) {
    case DK_SHORT2:*n=input.field;return 1;
    case DK_LONG:*n=(int16_t)input.value;return 1;
    case DK_FIXED:*n=(int16_t)delta_fixed_int(input.value);return 1;
    }
    int status=get_parm(d,&value,&input,DK_SHORT2);
    put_field(ref,input.field);
    if(status)return 0;
    *n=value.field;return 1;
}

/* A projection alternates timing points and statements. Consecutive timing
 * points belong to the same boundary and contribute no count. */
static int16_t count_range(delta_state *d,int32_t first,int32_t last,
    unsigned projection,int16_t initial)
{
    if(first==last)return initial;
    uint16_t count=(uint16_t)initial;
    int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,first),(uint8_t)projection));
    int32_t end=EVV_REF(vmover(d,EVV_AT(int32_t *,last),(uint8_t)projection));
    const delta_vars *v=EVV_AT(delta_vars *,d->vars);
    for(size_t steps=0;at && at!=end && steps<200000;++steps) {
        const int32_t *boundary=EVV_AT(const int32_t *,at);
        int32_t next=boundary[v->fence_base+projection]&~3;
        if(!next)break;
        if(*EVV_AT(const int32_t *,next)&2){at=next;continue;}
        /* A projection absent at this boundary cannot start another token. */
        if(!(boundary[v->fence_base+projection]&1))break;
        ++count;
        int32_t after=EVV_AT(const int32_t *,next)[1]&~3;
        if(!after)break;
        at=EVV_REF(vmover(d,EVV_AT(int32_t *,after),(uint8_t)projection));
    }
    return (int16_t)count;
}

static int32_t count_args(void *state,int32_t from,int32_t to,int32_t out,
    unsigned projection,int accumulate)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;int16_t seed;
    if(!from||!to||!position(from)||!position(to)||!read_short(d,out,&seed))
        return reloq_scalar_finish(d,94);
    put_field(out,count_range(d,position(from),position(to),
        projection,accumulate?seed:0));
    return reloq_scalar_finish(d,0);
}
int32_t reloq61_input_accumulate(void *state,int32_t from,int32_t to,int32_t out,int projection)
{return count_args(state,from,to,out,(unsigned)projection,1);}
int32_t reloq61_input_count(void *state,int32_t from,int32_t to,int32_t out,int projection)
{return count_args(state,from,to,out,(unsigned)projection,0);}

static void count_normalized(delta_state *d,const struct reloq61_input_layout *layout)
{
    int32_t from=long_at(d,layout->counted_through),to=long_at(d,layout->normalized_end);
    if(from&&to)put_short(d,layout->normalized_count,
        count_range(d,from,to,1,short_at(d,layout->normalized_count)));
    memcpy(fields(d)+layout->counted_through,&to,4);
}
int32_t reloq61_input_normalized(void *state,const struct reloq61_input_layout *layout)
{
    if(!reloq_scalar_begin(state))return 94;
    count_normalized(state,layout);return reloq_scalar_finish(state,0);
}

/* Context lookup is still supplied by the record layer. Keep its read cursor
 * local to this operation; no language tests or temporary registered cells
 * are involved. Empty fencing and disabled testing make these reads inert. */
struct read_cursor {delta_tpos left;uint8_t scan[8];int8_t fence,testing;};
static struct read_cursor cursor_begin(delta_state *d)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    struct read_cursor saved={.left=d->lpta,.fence=v->fence_count,.testing=v->testing};
    memcpy(saved.scan,&v->scan_ptr,8);v->fence_count=0;v->testing=0;return saved;
}
static void cursor_end(delta_state *d,const struct read_cursor *saved)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    d->lpta=saved->left;memcpy(&v->scan_ptr,saved->scan,8);
    v->fence_count=saved->fence;v->testing=saved->testing;
}
static void cursor_at(delta_state *d,int32_t position)
{d->lpta.flags=1;d->lpta.node=position;d->lpta.offset=0;}
static int16_t count_unnormalized(delta_state *d,int32_t position,int16_t initial,
    const struct reloq61_input_layout *layout)
{
    struct read_cursor saved=cursor_begin(d);
    cursor_at(d,position);
    int complete=0;
    if(!setscan_r(d,1)) {
        cursor_at(d,long_at(d,616));
        complete=!test_ptr(d);
    }
    uint16_t result=(uint16_t)initial;
    if(!complete) {
        cursor_at(d,position);
        int partial=short_at(d,layout->normalized_count)!=0;
        delta_loc original={0,-1,0};
        int found=!(partial?lpta_tstctxtr(d,0):lpta_tstmover(d,1)) &&
            !setscan_l(d,0) && !savetok(d,&original) && !advance_tok(d);
        result=(uint16_t)short_at(d,layout->input_count);
        if(partial||found)result=(uint16_t)(result-(uint32_t)original.value);
        if(partial)result=(uint16_t)(result+(uint16_t)initial);
    }
    cursor_end(d,&saved);return (int16_t)result;
}
int32_t reloq61_input_unnormalized(void *state,int32_t from,int32_t out,
    const struct reloq61_input_layout *layout)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;int16_t seed;
    if(!from||!position(from)||!read_short(d,out,&seed))return reloq_scalar_finish(d,94);
    put_field(out,count_unnormalized(d,position(from),seed,layout));
    return reloq_scalar_finish(d,0);
}
int32_t reloq61_input_total(void *state,int32_t out,const struct reloq61_input_layout *layout)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;int16_t ignored;
    if(!read_short(d,out,&ignored))return reloq_scalar_finish(d,94);
    count_normalized(d,layout);
    int16_t remaining=count_unnormalized(d,long_at(d,layout->counted_through),0,layout);
    put_field(out,(int16_t)((uint32_t)(int32_t)remaining+
        (uint32_t)long_at(d,layout->normalized_count)));
    return reloq_scalar_finish(d,0);
}
int32_t reloq61_input_single(void *state,int32_t from,int32_t to,int32_t before,
    int32_t after,const struct reloq61_input_layout *layout)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;int16_t ignored;
    if(!from||!to||!position(from)||!position(to)||
        !read_short(d,before,&ignored)||!read_short(d,after,&ignored))
        return reloq_scalar_finish(d,94);
    int32_t first=position(from);
    int16_t left=count_range(d,long_at(d,layout->counted_through),first,1,
        short_at(d,layout->normalized_count));
    struct read_cursor saved=cursor_begin(d);
    cursor_at(d,position(to));lpta_ctxtr(d,0);
    int32_t end=d->lpta.node;cursor_end(d,&saved);
    int16_t right=count_range(d,first,end,1,0);
    right=count_unnormalized(d,end,right,layout);
    put_field(before,left);put_field(after,right);
    return reloq_scalar_finish(d,0);
}
int32_t reloq61_input_minimum(void *state,int32_t mode,int32_t short_out,
    int32_t long_out,const struct reloq61_input_layout *layout)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;int16_t requested,first,last;
    if(!read_short(d,mode,&requested)||!read_short(d,short_out,&first)||
        !read_short(d,long_out,&last))return reloq_scalar_finish(d,94);
    if(requested==1){first=1;last=0;}
    else if(short_at(d,layout->minimum_mode)!=short_at(d,layout->ordinary_mode)) {
        switch(requested) {
        case 2:first=short_at(d,layout->short_minimum);last=short_at(d,layout->long_minimum);break;
        case 3:first=4;last=5;break;
        case 4:first=6;last=6;break;
        }
    }
    put_field(short_out,first);put_field(long_out,last);
    return reloq_scalar_finish(d,0);
}

/* Word records carry their ordinal in the first half-word. A missing record
 * has ordinal zero; the inclusive difference retains signed 16-bit wrapping. */
static uint16_t word_ordinal(delta_state *d)
{
    struct {delta_loc cell;unsigned char rest[12];} word={{4,-1,0},{0}};
    uint16_t ordinal=0;
    if(!savetok(d,&word.cell))memcpy(&ordinal,&word.cell.value,2);
    return ordinal;
}
static int16_t words_between(delta_state *d,int32_t from,int32_t to)
{
    struct read_cursor saved=cursor_begin(d);
    uint16_t first=0,last=0;
    cursor_at(d,from);
    if(!setscan_r(d,4)&&!advance_tok(d))first=word_ordinal(d);
    cursor_at(d,to);
    if(!setscan_l(d,4))last=word_ordinal(d);
    cursor_end(d,&saved);
    int16_t count=(int16_t)(uint16_t)(last-first+1u);
    return count>0?count:0;
}
static int32_t word_args(void *state,int32_t from,int32_t to,int32_t out,
    const struct reloq61_input_layout *layout)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;int16_t result;
    if(!from||!to||!position(from)||!position(to)||!read_short(d,out,&result))
        return reloq_scalar_finish(d,94);
    if(!layout||short_at(d,layout->token_count_mode)!=short_at(d,layout->ordinary_mode))
        result=words_between(d,position(from),position(to));
    put_field(out,result);return reloq_scalar_finish(d,0);
}
int32_t reloq61_input_words(void *state,int32_t from,int32_t to,int32_t out)
{return word_args(state,from,to,out,0);}
int32_t reloq61_input_tokens(void *state,int32_t from,int32_t to,int32_t out,
    const struct reloq61_input_layout *layout)
{return word_args(state,from,to,out,layout);}

int32_t reloq61_input_space(void *state,int32_t from,int32_t out)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;
    if(!from||!out||!position(from))return reloq_scalar_finish(d,94);
    struct read_cursor saved=cursor_begin(d);
    cursor_at(d,position(from));
    int status=94;
    if(!setscan_r(d,1)) {
        int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,position(from)),1));
        int32_t record=vgetsc(d,0,0,at,1);
        if(record&&!(*EVV_AT(const int32_t *,record)&2)&&
            *(const uint8_t *)delta_record_field(&vstmtbl[1], 0, TFLDS(EVV_AT(void *,record)))==72) {
            int32_t after=EVV_AT(const int32_t *,record)[1]&~3;
            if(after){memcpy(EVV_AT(unsigned char *,out)+4,&after,4);status=0;}
        }
    }
    cursor_end(d,&saved);return reloq_scalar_finish(d,status?94:0);
}

int32_t reloq61_input_inside(void *state,int32_t from)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;
    if(!from||!position(from))return reloq_scalar_finish(d,94);
    struct read_cursor saved=cursor_begin(d);
    int32_t start=EVV_REF(vmovel(d,EVV_AT(delta_node *,position(from)),0));
    cursor_at(d,start);
    int status=94;
    if(!lpta_tstctxtr(d,4)&&!setscan_r(d,4)) {
        delta_vars *v=EVV_AT(delta_vars *,d->vars);uint8_t scan[8];
        memcpy(scan,&v->scan_ptr,8);cursor_at(d,position(from));
        int at_boundary=!test_ptr(d);
        memcpy(&v->scan_ptr,scan,8);
        if(!at_boundary&&!advance_tok(d))status=0;
    }
    cursor_end(d,&saved);return reloq_scalar_finish(d,status);
}

int32_t reloq61_input_uppercase(void *state,int32_t from,int32_t mode)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;int16_t requested;
    if(!from||!position(from)||!read_short(d,mode,&requested))
        return reloq_scalar_finish(d,94);
    int status=0;
    if(requested==1) {
        status=94;
        struct read_cursor saved=cursor_begin(d);
        cursor_at(d,position(from));
        if(!setscan_r(d,1)&&!advance_tok(d)) {
            int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,position(from)),1));
            int32_t record=vgetsc(d,0,0,at,1);
            if(record&& !(*EVV_AT(const int32_t *,record)&2)) {
                uint8_t uppercase=2;
                vstmtbl[1].put[1](TFLDS(EVV_AT(void *,record)),&uppercase);
                EVV_AT(delta_owner *,d->owner)->changed=1;
                status=0;
            }
        }
        cursor_end(d,&saved);
    }
    return reloq_scalar_finish(d,status);
}
