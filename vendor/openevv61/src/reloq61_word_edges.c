/* Read-only bridge from the existing input projection to spelling runs.
 * It neither constructs a rule frame nor changes the caller's positions,
 * scan, fences or backtracking state. Linguistic decisions live in src/. */
#include "delta.h"
#include "reloq61_word_edges.h"
#include "reloq_german_word_edges.h"

static const unsigned char alphabet[]={
#include "reloq61_german_alphabet.inc"
};

struct cursor {int32_t at;unsigned back,held;};

static int begin(delta_state *d,int32_t at,unsigned back,struct cursor *c)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    if(!at || !(*EVV_AT(int32_t *,at+(v->fence_base+1)*4)&1))return 0;
    *c=(struct cursor){at,back,1};return 1;
}

static int consonant(delta_state *d,struct cursor *c,unsigned boundary,
    unsigned char *letter)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    for(size_t steps=0;steps<200000 && c->at;++steps) {
        if(!c->held && (*EVV_AT(int32_t *,c->at+(v->fence_base+boundary)*4)&3))return 0;
        int32_t next=vgetsc(d,c->back,0,c->at,1);
        if(!next)return 0;
        c->held=0;
        if(*EVV_AT(int32_t *,next)&2){c->at=next;continue;}
        void *fields=TFLDS(EVV_AT(void *,next));
        if(*(unsigned char *)delta_record_field(&vstmtbl[1], 4, fields)!=2)return 0;
        unsigned name=*(unsigned char *)delta_record_field(&vstmtbl[1], 0, fields);
        if(name>=sizeof alphabet)return 0;
        *letter=alphabet[name];
        c->at=*EVV_AT(int32_t *,next+(c->back?0:4))&~3;
        return 1;
    }
    return 0;
}

static int read_run(delta_state *d,int32_t at,unsigned back,unsigned boundary,
    struct reloq_german_consonants *run)
{
    struct cursor c;
    run->length=0;
    if(!begin(d,at,back,&c))return 0;
    while(run->length<sizeof run->spelling &&
        consonant(d,&c,boundary,&run->spelling[run->length]))++run->length;
    if(back)for(size_t i=0;i<run->length/2;++i) {
        unsigned char tmp=run->spelling[i];
        run->spelling[i]=run->spelling[run->length-1-i];
        run->spelling[run->length-1-i]=tmp;
    }
    return 1;
}

static int32_t position(const int32_t *args,int nargs)
{
    if(nargs<2 || !args[1])return 0;
    const delta_loc *p=EVV_AT(const delta_loc *,args[1]);
    return p->kind==DK_SYNC?p->value:0;
}

int32_t reloq61_german_onset(void *state,const int32_t *args,int nargs)
{
    struct reloq_german_consonants run;
    return read_run(state,position(args,nargs),0,4,&run) && reloq_german_onset(&run)?0:94;
}

int32_t reloq61_german_root_coda(void *state,const int32_t *args,int nargs)
{
    struct reloq_german_consonants run;
    return read_run(state,position(args,nargs),1,3,&run) && reloq_german_root_coda(&run)?0:94;
}

static int32_t extended(delta_state *d,int32_t at,enum reloq_german_ending ending)
{
    struct reloq_german_consonants root,shortened;
    if(!read_run(d,at,1,3,&root))return 94;
    if(reloq_german_root_coda(&root))return 0;
    struct cursor c;
    unsigned char last,previous;
    if(!begin(d,at,1,&c) || !consonant(d,&c,4,&last))return 94;
    int32_t before_last=c.at;
    if(!consonant(d,&c,4,&previous) || !read_run(d,before_last,1,3,&shortened))return 94;
    return reloq_german_extended_coda(&root,&shortened,last,previous,true,ending)?0:94;
}

int32_t reloq61_german_word_coda(void *d,const int32_t *args,int nargs)
{return extended(d,position(args,nargs),RELOQ_GERMAN_WORD_END);}
int32_t reloq61_german_dental_coda(void *d,const int32_t *args,int nargs)
{return extended(d,position(args,nargs),RELOQ_GERMAN_DENTAL_END);}
int32_t reloq61_german_sonorant_coda(void *d,const int32_t *args,int nargs)
{return extended(d,position(args,nargs),RELOQ_GERMAN_SONORANT_END);}
