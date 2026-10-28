#include "reloq_german_affricate.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    const unsigned char text[]="  `[.0nA.1Tjon] `[.1JIn].";
    struct reloq_german_phone_node p[32],saved[32];
    struct reloq_german_phone_sequence_info info;
    assert(!reloq_german_phone_sequence_build(text,sizeof text-1,0,p,32,&info));
    size_t n=info.phone_count,old=n;
    memcpy(saved,p,n*sizeof *p);
    assert(reloq_german_affricate_expand(p,&n,old)==-1);
    assert(n==old && !memcmp(saved,p,n*sizeof *p));
    assert(!reloq_german_affricate_expand(p,&n,32) && n==old+2);
    /* Expansion preserves the source boundary on both acoustic halves;
     * the following word's source and stress must not shift. */
    assert(p[2].phone==RELOQ_DE_PHONE_t && p[3].phone==RELOQ_DE_PHONE_s);
    assert(p[2].source_offset==saved[2].source_offset &&
           p[3].source_offset==saved[2].source_offset);
    assert(p[4].phone==RELOQ_DE_PHONE_j && p[4].affricate_role==9);
    assert(p[7].phone==RELOQ_DE_PHONE_d && p[8].phone==RELOQ_DE_PHONE_Z);
    assert(p[7].source_offset==saved[6].source_offset && p[7].word_index==1);
    assert(!p[7].same_word_left && p[8].same_syllable_left);
    int32_t ticks;
    assert(reloq_german_affricate_ticks(p,n,2,0,&ticks)==1 && ticks==64);
    assert(reloq_german_affricate_ticks(p,n,3,0,&ticks)==1 && ticks==90);
    assert(reloq_german_affricate_ticks(p,n,4,0,&ticks)==1 && ticks==50);
    struct reloq_german_obstruent_tract t;
    assert(reloq_german_affricate_tract(p+4,0,0,RELOQ_DE_PHONE_NONE,&t)==1);
    assert(t.entering[1]==1900 && t.entering[2]==2500 &&
           t.bandwidth[1]==250 && t.bandwidth[2]==500);
    puts("affricates: source coordinates, word boundaries and original -tion landmarks passed");
    return 0;
}
