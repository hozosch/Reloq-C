#ifndef RELOQ_PERSIAN_TIMING_H
#define RELOQ_PERSIAN_TIMING_H

/* Shared structural part of Persian's initial unstressed /i/ rule. The
 * caller first verifies the vowel identity, length and lack of stress.
 * This part must not depend on spelling, input variant, or word identity. */
static inline int reloq_fa_short_initial_i_context(int first,unsigned later_vowels,
                                                   int later_stress)
{
    return first && later_vowels>=2 && later_stress;
}

static inline int reloq_fa_short_initial_other_context(int first,unsigned later_vowels)
{
    return first && later_vowels>0;
}

enum reloq_fa_hiatus_vowel {
    RELOQ_FA_HIATUS_I=1,
    RELOQ_FA_HIATUS_SHORT_BACK_A=2
};

/* A listening candidate shared by the independent planner and the current
 * acoustic adapter. The acoustic identity stays unchanged. A short back A
 * in vowel hiatus gets a modest reduction; a long or stressed A does not. */
static inline unsigned reloq_fa_hiatus_timing_percent(unsigned vowel,int long_marker,
    int preceding_vowel,int stressed_next_vowel)
{
    if(!preceding_vowel || !stressed_next_vowel)return 100;
    if(vowel==RELOQ_FA_HIATUS_I)return 60;
    if(vowel==RELOQ_FA_HIATUS_SHORT_BACK_A && !long_marker)return 80;
    return 100;
}

#endif
