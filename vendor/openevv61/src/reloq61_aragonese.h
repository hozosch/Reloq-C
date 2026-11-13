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
int reloq61_an_qu_glide(void *state,int32_t end_ref);
int reloq61_an_latent_t(void *state,int32_t begin_ref);
int reloq61_an_latent_t_final_stress(void *state,int32_t from,int32_t to);
int reloq61_an_hundreds_digit(void *state,int32_t digit_ref);
int reloq61_an_liaison_d(void *state,int32_t begin_ref);
int reloq61_an_suppress_spanish_abbr(void *state,int32_t from,int32_t to);
int reloq61_an_note_elision(void *state,int32_t input_node);
int reloq61_an_take_elision(void *state,int32_t input_node);
void reloq61_an_adjust_clitics(void *state,int32_t from,int32_t to);

#endif
