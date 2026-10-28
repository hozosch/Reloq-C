/* Read-only phone projection and direct phonetic spelling output. */
#ifndef RELOQ61_SPR_OUTPUT_H
#define RELOQ61_SPR_OUTPUT_H
#include "delta.h"
struct reloq61_spr_phone { int32_t node,after; };
int reloq61_spr_phone_at(delta_state *,int32_t,struct reloq61_spr_phone *);
unsigned reloq61_spr_feature(delta_state *,const struct reloq61_spr_phone *,unsigned);
int32_t reloq61_spr_write(delta_state *,int32_t,int32_t,const char *,int32_t);
#endif
