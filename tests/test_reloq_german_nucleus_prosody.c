#include "reloq_german_nucleus_prosody.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define P(p,s,q,v) {RELOQ_DE_PHONE_##p,s,q,v,0}

static void sentence(void)
{
    /* A normalized linguistic fixture for normal text, not for SPR replay:
     * "Die Mutter öffnet das Fenster am Nachmittag." No duration values
     * or original state offsets enter the context. */
    const struct reloq_german_duration_phone phones[] = {
        P(d,0,0,0),P(i,0,0,0),
        P(m,1,0,0),P(U,1,1,0),P(t,0,0,1),P(R,0,1,0),
        P(Q_OE,1,1,0),P(f,1,0,1),P(n,0,0,0),P(AT,0,1,0),P(t,0,0,1),
        P(d,0,0,0),P(A,0,1,0),P(s,0,0,1),
        P(f,1,0,1),P(E,1,1,0),P(n,1,0,0),P(s,0,0,1),P(t,0,0,1),P(R,0,1,0),
        P(A,0,1,0),P(m,0,0,0),
        P(n,1,0,0),P(a,1,0,0),P(x,1,0,1),P(m,2,0,0),P(I,2,1,0),
        P(t,0,0,1),P(a,0,0,0),P(k,0,0,1)
    };
    const struct reloq_german_nucleus_context contexts[] = {
        {phones,30,0,2,0,2,1,2,1,0,0,-1,0,0,0,11,0},
        {phones,30,2,6,2,4,3,4,2,0,1,1,0,0,1,10,0},
        {phones,30,2,6,4,6,5,6,2,1,0,1,0,0,0,9,1},
        {phones,30,6,11,6,8,6,7,2,0,1,1,0,0,2,8,0},
        {phones,30,6,11,8,11,9,10,2,1,0,1,0,0,1,7,0},
        {phones,30,11,14,11,14,12,13,1,0,0,-1,0,0,0,6,0},
        {phones,30,14,20,14,17,15,16,2,0,1,1,0,0,2,5,0},
        {phones,30,14,20,17,20,19,20,2,1,0,1,0,0,1,4,0},
        {phones,30,20,22,20,22,20,21,1,0,0,-1,0,0,0,3,0},
        {phones,30,22,30,22,25,23,24,3,0,1,2,1,0,0,2,0},
        {phones,30,22,30,25,27,26,27,3,1,2,2,1,0,1,1,0},
        {phones,30,22,30,27,30,28,29,3,2,0,2,1,1,0,0,1}
    };
    const double expected[] = {97.2,60,98,82.5,60,54,82.5,98,67.5,
                               123.2,84,124.74};
    for (size_t i=0;i<sizeof contexts/sizeof *contexts;++i) {
        struct reloq_german_nucleus_budget out;
        assert(reloq_german_nucleus_budget(contexts+i,&out)==1);
        assert(fabs(out.adjusted_ticks-expected[i])<1e-12);
        assert(out.region==(i<9 ? RELOQ_DE_NUCLEUS_MEDIAL : i==9 ?
            RELOQ_DE_NUCLEUS_BEFORE_LATER_STRESS : RELOQ_DE_NUCLEUS_FINAL));
        if (i==5) {
            const struct reloq_nucleus_interval fixed[]={{45,1},{40,1}};
            int32_t allocated[2],vowel;
            assert(!reloq_german_nucleus_distribute(fixed,2,out.adjusted_ticks,
                                                   3,allocated,&vowel));
            assert(allocated[0]==27 && vowel==3 && allocated[1]==24);
        }
    }
}

static void boundaries_and_failures(void)
{
    struct reloq_german_duration_phone phones[]={P(m,1,0,0),P(E,1,1,0),P(m,1,0,0)};
    struct reloq_german_nucleus_context ctx={phones,3,0,2,0,2,1,2,1,0,1,1,0,0,0,1,0};
    double value=-9;
    /* A voiced consonant in the next word must not lengthen this nucleus.
     * The same consonant within the word does. */
    assert(reloq_german_nucleus_adjust(&ctx,75,&value)==1 && value==75);
    ctx.word_end=ctx.syllable_end=3;
    assert(reloq_german_nucleus_adjust(&ctx,75,&value)==1 && value==82.5);
    ctx.stress=3;
    assert(reloq_german_nucleus_adjust(&ctx,75,&value)==-1 && value==82.5);
    ctx.stress=1;
    assert(reloq_german_nucleus_phrase_final(&ctx,NAN,&value)==-1 && value==82.5);
    assert(reloq_german_nucleus_phrase_medial(NULL,75,&value)==-1 && value==82.5);

    struct reloq_german_nucleus_budget out,saved;
    memset(&out,0x5a,sizeof out);memcpy(&saved,&out,sizeof out);
    phones[1].phone=RELOQ_DE_PHONE_m;
    assert(reloq_german_nucleus_budget(&ctx,&out)==0);
    assert(!memcmp(&out,&saved,sizeof out));
    assert(reloq_german_nucleus_phrase_final(&ctx,75,&value)==0 && value==82.5);
    phones[1].phone=RELOQ_DE_PHONE_Q_aw;
    assert(reloq_german_nucleus_budget(&ctx,&out)==0);
    assert(!memcmp(&out,&saved,sizeof out));
    phones[1].phone=RELOQ_DE_PHONE_E;phones[1].short_quantity=2;
    assert(reloq_german_nucleus_budget(&ctx,&out)==-1);
    assert(!memcmp(&out,&saved,sizeof out));
    phones[1].short_quantity=1;ctx.nucleus_end=4;
    assert(reloq_german_nucleus_budget(&ctx,&out)==-1);
    assert(!memcmp(&out,&saved,sizeof out));
}

int main(void)
{
    sentence();boundaries_and_failures();
    puts("German nucleus phrase planning checks passed");
    return 0;
}
