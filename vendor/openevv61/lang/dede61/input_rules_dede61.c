/* Direct input accounting; the shared implementation owns the loops. */
#include "reloq61_input_counts.h"
static const struct reloq61_input_layout layout={1252,1260,1200,1212,3366,3358,1928,1932,3382};
int32_t reloq_dede61_count_cumulative_chars(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{(void)p0;return reloq61_input_accumulate(state,p1,p2,p3,1);}
int32_t reloq_dede61_count_chars(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{(void)p0;return reloq61_input_count(state,p1,p2,p3,1);}
int32_t reloq_dede61_count_sylls(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{(void)p0;return reloq61_input_count(state,p1,p2,p3,7);}
int32_t reloq_dede61_count_normalized_inp(void *state,int32_t p0)
{(void)p0;return reloq61_input_normalized(state,&layout);}
int32_t reloq_dede61_count_unnormalized_inp(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_input_unnormalized(state,p1,p2,&layout);}
int32_t reloq_dede61_count_input(void *state,int32_t p0,int32_t p1)
{(void)p0;return reloq61_input_total(state,p1,&layout);}
int32_t reloq_dede61_single_chars_count_inp(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4)
{(void)p0;return reloq61_input_single(state,p1,p2,p3,p4,&layout);}
int32_t reloq_dede61_set_min_token_vals(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{(void)p0;return reloq61_input_minimum(state,p1,p2,p3,&layout);}
int32_t reloq_dede61_num_words_in_range(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{(void)p0;return reloq61_input_words(state,p1,p2,p3);}
int32_t reloq_dede61_num_tokens_in_range(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{(void)p0;return reloq61_input_tokens(state,p1,p2,p3,&layout);}
int32_t reloq_dede61_space(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_input_space(state,p1,p2);}
int32_t reloq_dede61_inside_orig_inp_token(void *state,int32_t p0,int32_t p1)
{(void)p0;return reloq61_input_inside(state,p1);}
int32_t reloq_dede61_mark_uppercase(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_input_uppercase(state,p1,p2);}
