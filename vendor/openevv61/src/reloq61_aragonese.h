#ifndef RELOQ61_ARAGONESE_H
#define RELOQ61_ARAGONESE_H

#include <stdint.h>

/* Private development identity. It is intentionally absent from the public
 * ECI and platform catalogues until the frontend and dictionaries are ready. */
#define RELOQ61_ARAGONESE 0x130000

int reloq61_an_initial_aw(void *state,int32_t token_ref);
int reloq61_an_aw(void *state,int32_t token_ref);
int reloq61_an_standalone_sh(void *state,int32_t token_ref);

#endif
