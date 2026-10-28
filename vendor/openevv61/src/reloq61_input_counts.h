/* Character positions and syllable counts shared by the two frontends. */
#ifndef RELOQ61_INPUT_COUNTS_H
#define RELOQ61_INPUT_COUNTS_H
#include <stdint.h>
struct reloq61_input_layout {
    unsigned normalized_end, counted_through, input_count, normalized_count;
    unsigned minimum_mode, ordinary_mode, short_minimum, long_minimum;
    unsigned token_count_mode;
};
int32_t reloq61_input_accumulate(void *,int32_t,int32_t,int32_t,int);
int32_t reloq61_input_count(void *,int32_t,int32_t,int32_t,int);
int32_t reloq61_input_normalized(void *,const struct reloq61_input_layout *);
int32_t reloq61_input_unnormalized(void *,int32_t,int32_t,const struct reloq61_input_layout *);
int32_t reloq61_input_total(void *,int32_t,const struct reloq61_input_layout *);
int32_t reloq61_input_single(void *,int32_t,int32_t,int32_t,int32_t,const struct reloq61_input_layout *);
int32_t reloq61_input_minimum(void *,int32_t,int32_t,int32_t,const struct reloq61_input_layout *);
int32_t reloq61_input_words(void *,int32_t,int32_t,int32_t);
int32_t reloq61_input_tokens(void *,int32_t,int32_t,int32_t,const struct reloq61_input_layout *);
int32_t reloq61_input_space(void *,int32_t,int32_t);
int32_t reloq61_input_inside(void *,int32_t);
int32_t reloq61_input_uppercase(void *,int32_t,int32_t);
#endif
