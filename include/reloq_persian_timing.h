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

static inline int reloq_fa_i_before_stressed_hiatus(int preceding_vowel,int stressed_next_vowel)
{
    return preceding_vowel && stressed_next_vowel;
}

#endif
