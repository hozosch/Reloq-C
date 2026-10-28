/* ETI 6.1 inlined scan-register stores, retained at their original rule
 * positions rather than combined across control-flow edges. */
#include "delta.h"
#include "klatt_lang.h"
int32_t unsupported61(delta_state *d, int32_t rule)
{
    DeltaLang *lang = EVV_AT(DeltaLang *, d->dlang);
    lang->unsupported61 = rule ? rule : -1;
    return 94;
}
void inline61_lnode(delta_state *d, int32_t v) { d->lpta.node = v; }
void inline61_loffset(delta_state *d, int32_t v) { d->lpta.offset = v; }
void inline61_lflags(delta_state *d, int32_t v) { d->lpta.flags = v; }
void inline61_rnode(delta_state *d, int32_t v) { d->rpta.node = v; }
void inline61_roffset(delta_state *d, int32_t v) { d->rpta.offset = v; }
void inline61_rflags(delta_state *d, int32_t v) { d->rpta.flags = v; }
int32_t inline61_bit(delta_state *d, int32_t value, int32_t bit)
{ (void)d; return ((uint32_t)value >> ((uint32_t)bit & 31)) & 1; }
