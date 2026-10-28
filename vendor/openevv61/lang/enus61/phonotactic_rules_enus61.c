/* Ordinary C entrypoints for productive English spelling constraints. */
#include "reloq61_english_word_edges.h"
int32_t reloq_enus61_eng_legal_initial_cons(void *state,int32_t p0,int32_t p1)
{(void)p0;return reloq61_english_onset(state,p1);}
int32_t reloq_enus61_eng_legal_final_cons(void *state,int32_t p0,int32_t p1)
{(void)p0;return reloq61_english_coda(state,p1);}
int32_t reloq_enus61_legal_initial_consonants(void *state,int32_t p0,int32_t p1)
{(void)p0;return reloq61_english_syllable_onset(state,p1);}
