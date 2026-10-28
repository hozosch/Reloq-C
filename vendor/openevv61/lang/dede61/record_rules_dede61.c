/* German bindings for shared input and syllable record queries. */
#include "reloq61_record_readers.h"
int32_t reloq_dede61_in_syllable(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{(void)p0;return reloq61_record_in_syllable(state,p1,p2,p3);}
int32_t reloq_dede61_test_stress_annot(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_record_unstressed(state,p1,p2,6);}
int32_t reloq_dede61_find_most_stressed_syll(void *state,int32_t p0)
{(void)p0;return reloq61_record_last_accent(state);}
