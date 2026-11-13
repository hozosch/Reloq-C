#ifndef RELOQ61_ARAGONESE_H
#define RELOQ61_ARAGONESE_H

#include <stdint.h>

/* Independent language identity with shared Castilian acoustic resources. */
#define RELOQ61_ARAGONESE 0x130000

int reloq61_an_initial_aw(void *state,int32_t token_ref);
int reloq61_an_aw(void *state,int32_t token_ref);
int reloq61_an_standalone_sh(void *state,int32_t token_ref);
/* 0 = inherited ambiguous reading, 1 = /sh/, 2 = /ks/. */
int reloq61_an_x_pronunciation(void *state,int32_t begin_ref,int32_t end_ref);
int reloq61_an_consonant_y(void *state,int32_t end_ref);
int reloq61_an_qu_glide(void *state,int32_t end_ref);
int reloq61_an_suppress_spanish_abbr(void *state,int32_t from,int32_t to);
void reloq61_an_adjust_clitics(void *state,int32_t from,int32_t to);
int reloq61_an_take_silent_phone(void *state,int32_t phone_begin);

#endif
