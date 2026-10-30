/* Scalar-only C helpers never allocate a rule frame or alter scan/fence/
 * pointer-stack state. Exceptional and diagnostic entry keeps the old scope. */
#ifndef RELOQ61_SCALAR_BLOCKS_H
#define RELOQ61_SCALAR_BLOCKS_H
#include "delta.h"
#include "reloq61_rule_trace.h"
static inline int reloq61_scalar_ready(void *state)
{
    delta_state *d=state;
    return !delta_rule_trace && d && d->vars &&
        EVV_AT(delta_vars *,d->vars)->back && !deltaErrorThrown(d);
}
static inline int32_t reloq61_scalar_done(void *state,int32_t result)
{
    delta_state *d=state;
    EVV_AT(delta_vars *,d->vars)->unknown_11e8=0;
    return result;
}
#endif
