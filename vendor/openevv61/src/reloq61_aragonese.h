#ifndef RELOQ61_ARAGONESE_H
#define RELOQ61_ARAGONESE_H

#include <stdint.h>

/* Independent language identity with shared Castilian acoustic resources. */
#define RELOQ61_ARAGONESE 0x130000

int reloq61_an_initial_aw(void *state,int32_t token_ref);
int reloq61_an_aw(void *state,int32_t token_ref);
int reloq61_an_standalone_sh(void *state,int32_t token_ref);
int reloq61_an_x_is_sh(void *state,int32_t begin_ref,int32_t end_ref);

#endif
