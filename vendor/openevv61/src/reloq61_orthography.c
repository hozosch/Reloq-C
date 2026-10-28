/* Transitional record adapter for the independently formulated German
 * orthography pass. Ordinary words need no rule frame, flags, test stack
 * or repeated scans. Only actual edits enter the legacy mutation scope. */
#include <stdlib.h>
#include <string.h>
#include "delta.h"
#include "delta_rules_c.h"
#include "reloq_orthography.h"

static const unsigned char german_alphabet[]={
#include "reloq61_german_alphabet.inc"
};

struct word_buffer {
    struct reloq_orthographic_letter *letters;
    struct reloq_orthographic_change *changes;
    size_t length,capacity;
};

static int grow(struct word_buffer *b,
    struct reloq_orthographic_letter local_letters[128],
    struct reloq_orthographic_change local_changes[128])
{
    if(b->length<b->capacity)return 0;
    if(b->capacity>=200000)return 1;
    size_t next=b->capacity*2;
    void *letters=malloc(next*sizeof *b->letters),*changes=malloc(next*sizeof *b->changes);
    if(!letters || !changes){free(letters);free(changes);return 1;}
    memcpy(letters,b->letters,b->length*sizeof *b->letters);
    if(b->letters!=local_letters)free(b->letters);
    if(b->changes!=local_changes)free(b->changes);
    b->letters=letters;b->changes=changes;b->capacity=next;return 0;
}

/* Walk the input projection only. The positions and scan registers owned
 * by the caller remain untouched; no backtracking state is constructed. */
static int read_word(delta_state *d,int32_t begin,int32_t end,struct word_buffer *b,
    struct reloq_orthographic_letter local_letters[128],
    struct reloq_orthographic_change local_changes[128])
{
    int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,begin),1));
    int32_t stop=EVV_REF(vmover(d,EVV_AT(int32_t *,end),1));
    for(size_t steps=0;at!=stop && steps<200000;++steps) {
        int32_t token=vgetsc(d,0,0,at,1);
        if(!token || (*EVV_AT(int32_t *,token)&2))return 1;
        const void *fields=TFLDS(EVV_AT(void *,token));
        unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[1], 0, (void *)fields);
        if(name>=sizeof german_alphabet || grow(b,local_letters,local_changes))return 1;
        b->letters[b->length++]=(struct reloq_orthographic_letter){german_alphabet[name],
            *(const unsigned char *)delta_record_field(&vstmtbl[1], 4, (void *)fields)==1};
        int32_t after=*EVV_AT(int32_t *,token+4)&~3;
        if(!after)return 1;
        at=EVV_REF(vmover(d,EVV_AT(int32_t *,after),1));
    }
    return at!=stop;
}

static unsigned char native_letter(unsigned char spelling)
{
    for(size_t i=0;i<sizeof german_alphabet;++i)
        if(german_alphabet[i]==spelling)return (unsigned char)i;
    return 0;
}

struct edit_frame {
    delta_rule_block scope;
    delta_loc left,right,end;
    int32_t begin,finish,result;
};

static void load_left(delta_state *d,const delta_loc *left)
{
    const delta_token token={0,left->value};lpta_loadp(d,&token);
}

static void load_range(delta_state *d,const delta_loc *left,const delta_loc *right)
{
    const delta_token a={0,left->value},b={0,right->value};
    lpta_rpta_loadp(d,&a,&b);
}

static int32_t edit_word(delta_state *d,int32_t begin,int32_t end,
    const struct word_buffer *b)
{
    struct edit_frame *f=(struct edit_frame *)evv_frame_push(sizeof *f);
    if(!f)return 94;
    memset(f,0,sizeof *f);f->begin=begin;f->finish=end;f->result=94;
    volatile int entered=0;
    if(EVV_LAND_SAVE((intptr_t)f->scope.landing))goto leave;
    if(ventproc(d,&f->scope.rec,f->scope.fence[0],f->scope.fence[1],
        f->scope.fence[2],f->scope.landing))goto leave;
    entered=1;
    push_ptr_init(d,&f->left);push_ptr_init(d,&f->right);push_ptr_init(d,&f->end);
    f->left.value=EVV_REF(vmover(d,EVV_AT(int32_t *,f->begin),1));
    f->end.value=EVV_REF(vmover(d,EVV_AT(int32_t *,f->finish),1));
    fence(d,0,NULL);startloop(d,1);
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    for(size_t i=0;i<b->length;++i) {
        load_left(d,&f->left);
        if(setscan_r(d,1) || advance_tok(d))goto leave;
        f->right.value=v->scan_ptr;
        struct reloq_orthographic_change change=b->changes[i];
        load_range(d,&f->left,&f->right);
        if(change.action==RELOQ_ORTHOGRAPHY_REPLACE) {
            unsigned char letter=native_letter(change.spelling);
            if(!letter || insert_2pt_s(d,1,1,&letter,0))goto leave;
        } else if(change.action==RELOQ_ORTHOGRAPHY_DELETE) {
            if(delete_2pt(d,1,0))goto leave;
            load_left(d,&f->right);delete_1pt(d,1);
        } else if(change.action==RELOQ_ORTHOGRAPHY_CONSONANT) {
            if(mark_s(d,1,4,2,0))goto leave;
        }
        if(change.action!=RELOQ_ORTHOGRAPHY_DELETE)f->left.value=f->right.value;
        f->left.value=EVV_REF(vmover(d,EVV_AT(int32_t *,f->left.value),1));
    }
    f->result=0;
leave:
    if(entered && vretproc(d,f->result?94:1))f->result=94;
    int32_t result=f->result;
    evv_land_forget((uintptr_t)f,(uintptr_t)f+sizeof *f);evv_frame_pop(f);
    return result;
}

int32_t reloq61_normalize_german_letters(void *state,const int32_t *args,int nargs)
{
    if(nargs<3)return 94;
    delta_state *d=state;
    const delta_loc *from=EVV_AT(const delta_loc *,args[1]);
    const delta_loc *to=EVV_AT(const delta_loc *,args[2]);
    if(from->kind!=DK_SYNC || to->kind!=DK_SYNC || !from->value || !to->value)return 94;
    struct reloq_orthographic_letter local_letters[128];
    struct reloq_orthographic_change local_changes[128];
    struct word_buffer b={local_letters,local_changes,0,128};
    int32_t result=94;
    if(!read_word(d,from->value,to->value,&b,local_letters,local_changes))
        result=reloq_german_orthography(b.letters,b.length,b.changes)?
            edit_word(d,from->value,to->value,&b):0;
    if(b.letters!=local_letters)free(b.letters);
    if(b.changes!=local_changes)free(b.changes);
    return result;
}
