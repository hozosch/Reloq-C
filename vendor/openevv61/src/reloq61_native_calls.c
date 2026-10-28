/* Shared C bindings with native typed arguments. Diagnostic packing is
 * confined to the optional trace path; ordinary calls use the C ABI. */
#include "delta.h"
#include "reloq61_native_calls.h"
extern int32_t delta_rule_called(int,const int32_t *,int,int);
extern int delta_rule_trace;
extern int32_t backtrack_function(delta_state *,int32_t *);
extern void inline61_lnode(delta_state *,int32_t);
extern void inline61_loffset(delta_state *,int32_t);
extern void inline61_lflags(delta_state *,int32_t);
extern void inline61_rnode(delta_state *,int32_t);
extern void inline61_roffset(delta_state *,int32_t);
extern void inline61_rflags(delta_state *,int32_t);

extern int32_t callUserDictLookup(delta_state *,const void *,const delta_token *,const delta_token *);

int32_t reloq61_typed_actd_goto(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return actd_goto(d);
}

int32_t reloq61_typed_actd_lookup(int which, delta_state *d, int16_t a0, delta_token * a1, delta_token * a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a2), EVV_REF(a1), (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return actd_lookup(d, a0, a1, a2);
}

int32_t reloq61_typed_advanc(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return advanc(d);
}

int32_t reloq61_typed_advance_tok(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return advance_tok(d);
}

int32_t reloq61_typed_backtrack_function(int which, delta_state *d, int32_t * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return backtrack_function(d, a0);
}

int32_t reloq61_typed_bspop_boa(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return EVV_REF(bspop_boa(d));
}

int32_t reloq61_typed_bspush_boa(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    bspush_boa(d);return 0;
}

int32_t reloq61_typed_bspush_ca(int which, delta_state *d, int16_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    bspush_ca(d, a0);return 0;
}

int32_t reloq61_typed_bspush_ca_boa(int which, delta_state *d, int16_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    bspush_ca_boa(d, a0);return 0;
}

int32_t reloq61_typed_bspush_ca_scan(int which, delta_state *d, int16_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    bspush_ca_scan(d, a0);return 0;
}

int32_t reloq61_typed_bspush_ca_scan_boa(int which, delta_state *d, int16_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    bspush_ca_scan_boa(d, a0);return 0;
}

int32_t reloq61_typed_bspush_nboa(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    bspush_nboa(d);return 0;
}

int32_t reloq61_typed_c_assvar(int which, delta_state *d, delta_loc * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    c_assvar(d, a0);return 0;
}

int32_t reloq61_typed_callUserDictLookup(int which, delta_state *d, const void * a0, const delta_token * a1, const delta_token * a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a2), EVV_REF(a1), EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return callUserDictLookup(d, a0, a1, a2);
}

int32_t reloq61_typed_chstream(int which, delta_state *d, int16_t a0, uint8_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return chstream(d, a0, a1);
}

int32_t reloq61_typed_compare_ptas(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return compare_ptas(d);
}

int32_t reloq61_typed_copyvar(int which, delta_state *d, delta_loc * a0, delta_loc * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    copyvar(d, a0, a1);return 0;
}

int32_t reloq61_typed_delete_1pt(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    delete_1pt(d, a0);return 0;
}

int32_t reloq61_typed_delete_2pt(int which, delta_state *d, uint8_t a0, uint8_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return delete_2pt(d, a0, a1);
}

int32_t reloq61_typed_fence(int which, delta_state *d, int8_t a0, const uint8_t * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    fence(d, a0, a1);return 0;
}

int32_t reloq61_typed_forall_cont_from(int which, delta_state *d, int16_t a0, int16_t a1, int32_t a2, delta_loc * a3, const delta_loc * a4)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a4), EVV_REF(a3), (int32_t)a2, (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,6,6);
    }
    return forall_cont_from(d, a0, a1, a2, a3, a4);
}

int32_t reloq61_typed_forall_to_test(int which, delta_state *d, delta_loc * a0, delta_loc * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return forall_to_test(d, a0, a1);
}

int32_t reloq61_typed_forto_adv_r(int which, delta_state *d, int16_t a0, int16_t a1, int16_t a2, uint8_t a3, delta_token * a4, const delta_token * a5)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a5), EVV_REF(a4), (int32_t)a3, (int32_t)a2, (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,7,7);
    }
    return forto_adv_r(d, a0, a1, a2, a3, a4, a5);
}

int32_t reloq61_typed_forto_adv_upto_r(int which, delta_state *d, int16_t a0, int16_t a1, int16_t a2, uint8_t a3, delta_token * a4, const delta_token * a5)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a5), EVV_REF(a4), (int32_t)a3, (int32_t)a2, (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,7,7);
    }
    return forto_adv_upto_r(d, a0, a1, a2, a3, a4, a5);
}

int32_t reloq61_typed_get_parm(int which, delta_state *d, delta_loc * a0, delta_loc * a1, int16_t a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a2, EVV_REF(a1), EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return get_parm(d, a0, a1, a2);
}

int32_t reloq61_typed_if_testeq(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return if_testeq(d);
}

int32_t reloq61_typed_if_testeq_v_i(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return if_testeq_v_i(d, a0, a1);
}

int32_t reloq61_typed_if_testge(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return if_testge(d);
}

int32_t reloq61_typed_if_testge_v_i(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return if_testge_v_i(d, a0, a1);
}

int32_t reloq61_typed_if_testgt(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return if_testgt(d);
}

int32_t reloq61_typed_if_testgt_v_i(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return if_testgt_v_i(d, a0, a1);
}

int32_t reloq61_typed_if_testle(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return if_testle(d);
}

int32_t reloq61_typed_if_testle_v_i(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return if_testle_v_i(d, a0, a1);
}

int32_t reloq61_typed_if_testlt(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return if_testlt(d);
}

int32_t reloq61_typed_if_testlt_v_i(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return if_testlt_v_i(d, a0, a1);
}

int32_t reloq61_typed_if_testneq(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return if_testneq(d);
}

int32_t reloq61_typed_if_testneq_v_i(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return if_testneq_v_i(d, a0, a1);
}

int32_t reloq61_typed_inline61_lflags(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    inline61_lflags(d, a0);return 0;
}

int32_t reloq61_typed_inline61_lnode(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    inline61_lnode(d, a0);return 0;
}

int32_t reloq61_typed_inline61_loffset(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    inline61_loffset(d, a0);return 0;
}

int32_t reloq61_typed_inline61_rflags(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    inline61_rflags(d, a0);return 0;
}

int32_t reloq61_typed_inline61_rnode(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    inline61_rnode(d, a0);return 0;
}

int32_t reloq61_typed_inline61_roffset(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    inline61_roffset(d, a0);return 0;
}

int32_t reloq61_typed_insert_2pt_i(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2, uint8_t a3)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a3, EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,5,5);
    }
    return insert_2pt_i(d, a0, a1, a2, a3);
}

int32_t reloq61_typed_insert_2pt_l(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2, uint8_t a3)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a3, EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,5,5);
    }
    return insert_2pt_l(d, a0, a1, a2, a3);
}

int32_t reloq61_typed_insert_2pt_lng(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2, uint8_t a3)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a3, EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,5,5);
    }
    return insert_2pt_lng(d, a0, a1, a2, a3);
}

int32_t reloq61_typed_insert_2pt_s(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2, uint8_t a3)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a3, EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,5,5);
    }
    return insert_2pt_s(d, a0, a1, a2, a3);
}

int32_t reloq61_typed_insert_2ptv(int which, delta_state *d, uint8_t a0, delta_loc * a1, uint8_t a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a2, EVV_REF(a1), (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return insert_2ptv(d, a0, a1, a2);
}

int32_t reloq61_typed_insert_l(int which, delta_state *d, int8_t a0, uint8_t a1, const uint8_t * a2, uint8_t a3)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a3, EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,5,5);
    }
    insert_l(d, a0, a1, a2, a3);return 0;
}

int32_t reloq61_typed_insert_r(int which, delta_state *d, int8_t a0, uint8_t a1, const uint8_t * a2, uint8_t a3)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a3, EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,5,5);
    }
    insert_r(d, a0, a1, a2, a3);return 0;
}

int32_t reloq61_typed_lpta_ctxtl(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    lpta_ctxtl(d, a0);return 0;
}

int32_t reloq61_typed_lpta_ctxtr(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    lpta_ctxtr(d, a0);return 0;
}

int32_t reloq61_typed_lpta_loadp(int which, delta_state *d, const delta_token * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    lpta_loadp(d, a0);return 0;
}

int32_t reloq61_typed_lpta_loadpn(int which, delta_state *d, const delta_token * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    lpta_loadpn(d, a0);return 0;
}

int32_t reloq61_typed_lpta_movel(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    lpta_movel(d, a0);return 0;
}

int32_t reloq61_typed_lpta_mover(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    lpta_mover(d, a0);return 0;
}

int32_t reloq61_typed_lpta_rpta_loadp(int which, delta_state *d, const delta_token * a0, const delta_token * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    lpta_rpta_loadp(d, a0, a1);return 0;
}

int32_t reloq61_typed_lpta_storep(int which, delta_state *d, delta_loc * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    lpta_storep(d, a0);return 0;
}

int32_t reloq61_typed_lpta_tstctxtl(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return lpta_tstctxtl(d, a0);
}

int32_t reloq61_typed_lpta_tstctxtr(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return lpta_tstctxtr(d, a0);
}

int32_t reloq61_typed_lpta_tstmovel(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return lpta_tstmovel(d, a0);
}

int32_t reloq61_typed_lpta_tstmover(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return lpta_tstmover(d, a0);
}

int32_t reloq61_typed_mark_s(int which, delta_state *d, uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a3, (int32_t)a2, (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,5,5);
    }
    return mark_s(d, a0, a1, a2, a3);
}

int32_t reloq61_typed_mark_v(int which, delta_state *d, uint8_t a0, uint8_t a1, delta_loc * a2, uint8_t a3)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a3, EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,5,5);
    }
    return mark_v(d, a0, a1, a2, a3);
}

int32_t reloq61_typed_move_f(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    move_f(d, a0, a1);return 0;
}

int32_t reloq61_typed_move_i(int which, delta_state *d, delta_loc * a0, int16_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    move_i(d, a0, a1);return 0;
}

int32_t reloq61_typed_move_lng(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    move_lng(d, a0, a1);return 0;
}

int32_t reloq61_typed_ncompare(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    ncompare(d);return 0;
}

int32_t reloq61_typed_ncompare_s(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    ncompare_s(d, a0);return 0;
}

int32_t reloq61_typed_npop(int which, delta_state *d, delta_loc * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    npop(d, a0);return 0;
}

int32_t reloq61_typed_npush_f(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    npush_f(d, a0);return 0;
}

int32_t reloq61_typed_npush_fld(int which, delta_state *d, uint8_t a0, uint8_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return npush_fld(d, a0, a1);
}

int32_t reloq61_typed_npush_i(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    npush_i(d, a0);return 0;
}

int32_t reloq61_typed_npush_l(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    npush_l(d, a0);return 0;
}

int32_t reloq61_typed_npush_lng(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    npush_lng(d, a0);return 0;
}

int32_t reloq61_typed_npush_s(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    npush_s(d, a0);return 0;
}

int32_t reloq61_typed_npush_v(int which, delta_state *d, delta_loc * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    npush_v(d, a0);return 0;
}

int32_t reloq61_typed_npush_vf(int which, delta_state *d, delta_loc * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    npush_vf(d, a0);return 0;
}

int32_t reloq61_typed_print_lit(int which, delta_state *d, int32_t a0, const char * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    print_lit(d, a0, a1);return 0;
}

int32_t reloq61_typed_print_var(int which, delta_state *d, int32_t a0, delta_loc * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    print_var(d, a0, a1);return 0;
}

int32_t reloq61_typed_proj_def(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    proj_def(d, a0);return 0;
}

int32_t reloq61_typed_push_ptr_init(int which, delta_state *d, delta_loc * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    push_ptr_init(d, a0);return 0;
}

int32_t reloq61_typed_read_tvar(int which, delta_state *d, int8_t a0, delta_loc * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return read_tvar(d, a0, a1);
}

int32_t reloq61_typed_rpta_ctxtl(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    rpta_ctxtl(d, a0);return 0;
}

int32_t reloq61_typed_rpta_ctxtr(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    rpta_ctxtr(d, a0);return 0;
}

int32_t reloq61_typed_rpta_loadpn(int which, delta_state *d, const delta_token * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    rpta_loadpn(d, a0);return 0;
}

int32_t reloq61_typed_rpta_movel(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    rpta_movel(d, a0);return 0;
}

int32_t reloq61_typed_rpta_mover(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    rpta_mover(d, a0);return 0;
}

int32_t reloq61_typed_rpta_tstctxtl(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return rpta_tstctxtl(d, a0);
}

int32_t reloq61_typed_rpta_tstctxtr(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return rpta_tstctxtr(d, a0);
}

int32_t reloq61_typed_rpta_tstmovel(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return rpta_tstmovel(d, a0);
}

int32_t reloq61_typed_rpta_tstmover(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return rpta_tstmover(d, a0);
}

int32_t reloq61_typed_savescptr(int which, delta_state *d, int16_t a0, delta_loc * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    savescptr(d, a0, a1);return 0;
}

int32_t reloq61_typed_savetok(int which, delta_state *d, delta_loc * a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return savetok(d, a0);
}

int32_t reloq61_typed_setd_lookup(int which, delta_state *d, int32_t a0, int16_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return setd_lookup(d, a0, a1);
}

int32_t reloq61_typed_setscan_l(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return setscan_l(d, a0);
}

int32_t reloq61_typed_setscan_nof_l(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return setscan_nof_l(d, a0);
}

int32_t reloq61_typed_setscan_nof_r(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return setscan_nof_r(d, a0);
}

int32_t reloq61_typed_setscan_r(int which, delta_state *d, uint8_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return setscan_r(d, a0);
}

int32_t reloq61_typed_settvar_s(int which, delta_state *d, delta_loc * a0, int32_t a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a1, EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    settvar_s(d, a0, a1);return 0;
}

int32_t reloq61_typed_startloop(int which, delta_state *d, int16_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    startloop(d, a0);return 0;
}

int32_t reloq61_typed_starttest(int which, delta_state *d, int16_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    starttest(d, a0);return 0;
}

int32_t reloq61_typed_starttest_e(int which, delta_state *d, int16_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    starttest_e(d, a0);return 0;
}

int32_t reloq61_typed_starttest_l(int which, delta_state *d, int16_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    starttest_l(d, a0);return 0;
}

int32_t reloq61_typed_succeed(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return succeed(d);
}

int32_t reloq61_typed_testFldeq(int which, delta_state *d, uint8_t a0, uint8_t a1, uint8_t a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a2, (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return testFldeq(d, a0, a1, a2);
}

int32_t reloq61_typed_test_ptr(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return test_ptr(d);
}

int32_t reloq61_typed_test_string(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return test_string(d, a0, a1, a2);
}

int32_t reloq61_typed_test_string_i(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return test_string_i(d, a0, a1, a2);
}

int32_t reloq61_typed_test_string_l(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return test_string_l(d, a0, a1, a2);
}

int32_t reloq61_typed_test_string_lng(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return test_string_lng(d, a0, a1, a2);
}

int32_t reloq61_typed_test_string_s(int which, delta_state *d, uint8_t a0, uint8_t a1, const uint8_t * a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return test_string_s(d, a0, a1, a2);
}

int32_t reloq61_typed_test_synch(int which, delta_state *d, int16_t a0, uint8_t a1, const uint8_t * a2)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a2), (int32_t)a1, (int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,4,4);
    }
    return test_synch(d, a0, a1, a2);
}

int32_t reloq61_typed_testeq(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return testeq(d);
}

int32_t reloq61_typed_testeq_tvars(int which, delta_state *d, delta_loc * a0, delta_loc * a1)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a1), EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,3,3);
    }
    return testeq_tvars(d, a0, a1);
}

int32_t reloq61_typed_testge(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return testge(d);
}

int32_t reloq61_typed_testgt(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return testgt(d);
}

int32_t reloq61_typed_testle(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return testle(d);
}

int32_t reloq61_typed_testlt(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return testlt(d);
}

int32_t reloq61_typed_testneq(int which, delta_state *d)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(d)};
        return delta_rule_called(which,args,1,1);
    }
    return testneq(d);
}

int32_t reloq61_typed_ventproc(int which, delta_state *d, delta_actrec * a0, uint8_t * a1, uint8_t * a2, uint8_t * a3, void * a4)
{
    if(delta_rule_trace) {
        const int32_t args[]={EVV_REF(a4), EVV_REF(a3), EVV_REF(a2), EVV_REF(a1), EVV_REF(a0), EVV_REF(d)};
        return delta_rule_called(which,args,6,6);
    }
    return ventproc(d, a0, a1, a2, a3, a4);
}

int32_t reloq61_typed_vretproc(int which, delta_state *d, int32_t a0)
{
    if(delta_rule_trace) {
        const int32_t args[]={(int32_t)a0, EVV_REF(d)};
        return delta_rule_called(which,args,2,2);
    }
    return vretproc(d, a0);
}
