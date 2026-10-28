#include "reloq_german_nucleus_duration.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void topology(void)
{
    /* Garten: vowel and postvocalic r share a nucleus. Mutter's reduced
     * second syllable instead has a standalone vocalic R nucleus. */
    struct reloq_german_phone_node a[] = {
        {.phone=RELOQ_DE_PHONE_g}, {.phone=RELOQ_DE_PHONE_A},
        {.phone=RELOQ_DE_PHONE_r}, {.phone=RELOQ_DE_PHONE_t}};
    size_t start=99, length=99;
    assert(reloq_german_nucleus_find(a,4,&start,&length)==1);
    assert(start==1 && length==2);
    a[1].phone=RELOQ_DE_PHONE_R;a[2].phone=RELOQ_DE_PHONE_t;
    assert(reloq_german_nucleus_find(a,3,&start,&length)==1);
    assert(start==1 && length==1);
    a[1].phone=RELOQ_DE_PHONE_A;a[2].phone=RELOQ_DE_PHONE_w;
    a[3].phone=RELOQ_DE_PHONE_l;
    assert(reloq_german_nucleus_find(a,4,&start,&length)==1);
    assert(start==1 && length==3);
    a[1].phone=RELOQ_DE_PHONE_Q_aw;a[2].phone=RELOQ_DE_PHONE_l;
    assert(reloq_german_nucleus_find(a,3,&start,&length)==1);
    assert(start==1 && length==2);
    a[1].syllable_index=1;
    assert(reloq_german_nucleus_find(a,3,&start,&length)==-1);
    assert(start==1 && length==2);
    a[1].syllable_index=0;a[1].phone=RELOQ_DE_PHONE_s;
    assert(reloq_german_nucleus_find(a,3,&start,&length)==0);
    assert(start==1 && length==2);
}

static void bases(void)
{
    struct reloq_german_nucleus_duration b;
    assert(reloq_german_nucleus_duration_base(RELOQ_DE_PHONE_U,
        RELOQ_DE_PHONE_NONE,RELOQ_DE_PHONE_NONE,0,&b)==1);
    assert(b.base_ticks==60 && b.closing_ticks==0);
    assert(reloq_german_nucleus_duration_base(RELOQ_DE_PHONE_A,
        RELOQ_DE_PHONE_r,RELOQ_DE_PHONE_NONE,0,&b)==1);
    assert(b.base_ticks==140 && b.closing_ticks==25);
    assert(reloq_german_nucleus_duration_base(RELOQ_DE_PHONE_i,
        RELOQ_DE_PHONE_l,RELOQ_DE_PHONE_NONE,1,&b)==1);
    assert(b.base_ticks==150 && b.closing_ticks==40);
    assert(reloq_german_nucleus_duration_base(RELOQ_DE_PHONE_Q_aw,
        RELOQ_DE_PHONE_l,RELOQ_DE_PHONE_NONE,0,&b)==1);
    assert(b.base_ticks==190 && b.closing_ticks==40 && b.glide_ticks==20);
    struct reloq_german_nucleus_duration saved=b;
    assert(reloq_german_nucleus_duration_base(RELOQ_DE_PHONE_m,
        RELOQ_DE_PHONE_NONE,RELOQ_DE_PHONE_NONE,0,&b)==0);
    assert(!memcmp(&b,&saved,sizeof b));
}

static void redistribution(void)
{
    struct reloq_nucleus_interval a[]={{45,1},{40,1}};
    int32_t out[4]={-9,-9,-9,-9},free_ticks=-9;
    /* Natural "das": both transitions must be compressed, then the vowel
     * minimum reserved in a second rounding pass. */
    assert(!reloq_german_nucleus_distribute(a,2,54,3,out,&free_ticks));
    assert(out[0]==27 && out[1]==24 && free_ticks==3);
    struct reloq_nucleus_interval b[]={{25,1},{54,1},{25,3},{50,1}};
    assert(!reloq_german_nucleus_distribute(b,4,125.55,3,out,&free_ticks));
    assert(out[0]==20 && out[1]==43 && out[2]==20 && out[3]==40 && free_ticks==3);
    a[0].ticks=15;a[1].ticks=40;
    assert(!reloq_german_nucleus_distribute(a,2,60,3,out,&free_ticks));
    assert(out[0]==15 && out[1]==40 && free_ticks==5);
    assert(!reloq_german_nucleus_distribute(NULL,0,98,3,NULL,&free_ticks));
    assert(free_ticks==98);
    assert(!reloq_german_nucleus_distribute(a,2,0,3,out,&free_ticks));
    assert(out[0]==1 && out[1]==1 && free_ticks==3);
    int32_t saved[4];memcpy(saved,out,sizeof out);
    assert(reloq_german_nucleus_distribute(a,2,NAN,3,out,&free_ticks)==-1);
    assert(!memcmp(saved,out,sizeof out) && free_ticks==3);
    a[1].ticks=-1;
    assert(reloq_german_nucleus_distribute(a,2,60,3,out,&free_ticks)==-1);
    assert(!memcmp(saved,out,sizeof out) && free_ticks==3);
}

int main(void)
{
    topology();bases();redistribution();
    puts("German nucleus structure and duration checks passed");
    return 0;
}
