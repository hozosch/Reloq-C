#include "reloq_german_coda_timing.h"
#include "reloq_german_glottal.h"
#include <assert.h>
#include <string.h>

int main(void)
{
    /* Fixed original observations, including pause-sensitive pairs and
     * word/syllable boundaries. These are not computed by the C rules. */
    static const struct {const char *spr;int body,transition;} cases[]={
        {"`[.1Am.0pA].",60,1},{"`[.1Am] `[.1kA].",110,1},
        {"`[.1A.0@mk].",85,1},{"`[.1Ant].",90,1},
        {"`[.1Ant] `[.1pA].",45,1},{"`[.1An.0kA].",70,1},
        {"`[.1AG.0kA].",42,1},{"`[.1AGk].",85,1},
        {"`[.1AGk] `[.1pA].",62,1},{"`[.1AGt] `[.1pA].",42,30},
        {"`[.1Aft].",150,1},{"`[.1Aft] `[.1pA].",60,1},
        {"`00 `[.1Ist] `[.1gUt].",55,1},
        {"`[.1AS.0pA].",80,1},{"`[.1AS.0kA].",110,1},
        {"`[.1Axt].",60,1},{"`[.1IXt].",145,1},
        {"`[.1IXt] `[.1pA].",75,1},
        {"`[.0Ant.1hA].",40,1},{"`[.1AGk.0hA].",42,1},
        {"`[.1Aft.2hA].",118,1},
        {"`[.1pAf.0@].",95,0},{"`[.1pAf.0@t].",95,0},
        {"`[.1pAf.0@n].",90,0},{"`[.1pAf.0@m].",95,0},
        {"`[.1pAX.0@].",90,0},{"`[.1pAX.0@n].",111,0},
        {"`[.1pAs.0Un].",85,0}
    };
    for(size_t j=0;j<sizeof cases/sizeof cases[0];++j) {
        struct reloq_german_phone_node p[32];struct reloq_german_phone_sequence_info info;
        assert(!reloq_german_phone_sequence_build((const unsigned char *)cases[j].spr,
            strlen(cases[j].spr),0,p,32,&info));
        unsigned found=0;
        for(size_t i=0;i<info.phone_count;++i) {
            struct reloq_german_coda_timing t={-1,-1};
            int r=reloq_german_coda_timing_plan(p,info.phone_count,i,i+2==info.phone_count,&t);
            assert(r>=0);
            if(r) {assert(t.body_ticks==cases[j].body && t.outgoing_ticks==cases[j].transition);++found;}
            else assert(t.body_ticks==-1 && t.outgoing_ticks==-1);
        }
        assert(found==1);
    }
    struct reloq_german_phone_node a={.phone=RELOQ_DE_PHONE_i};
    struct reloq_german_phone_node b={.phone=RELOQ_DE_PHONE_I};
    assert(reloq_german_glottal_transition(&a,&b)==30);
    b.phone=RELOQ_DE_PHONE_A;assert(reloq_german_glottal_transition(&a,&b)==50);
    b.stress=1;assert(reloq_german_glottal_transition(&a,&b)==65);
    return 0;
}
