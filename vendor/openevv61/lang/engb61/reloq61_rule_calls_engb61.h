/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_ENGB61_H
#define RELOQ61_RULE_CALLS_ENGB61_H
#include "delta_rules_c.h"
extern int delta_rule_trace;
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_engb61_DeltaProc_end(void *, int32_t);
int32_t reloq_engb61_DeltaProc_flush(void *, int32_t);
int32_t reloq_engb61_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_engb61_DeltaProc_main(void *, int32_t);
int32_t reloq_engb61_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_engb61_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_engb61_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_engb61_DeltaProc_start(void *, int32_t);
int32_t reloq_engb61_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_engb61_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_engb61_R_to_r(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_R_to_r(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_R_to_r,args,3,3);
    }
    return reloq_engb61_R_to_r(state, p0, p1, p2);
}
int32_t reloq_engb61_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_engb61_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_UK_except_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_UK_insert_epenthetic_fric(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_insert_epenthetic_fric(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_insert_epenthetic_fric,args,1,1);
    }
    return reloq_engb61_UK_insert_epenthetic_fric(state, p0);
}
int32_t reloq_engb61_UK_ph_A(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_A(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_A,args,1,1);
    }
    return reloq_engb61_UK_ph_A(state, p0);
}
int32_t reloq_engb61_UK_ph_E(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_E,args,1,1);
    }
    return reloq_engb61_UK_ph_E(state, p0);
}
int32_t reloq_engb61_UK_ph_H(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_H(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_H,args,1,1);
    }
    return reloq_engb61_UK_ph_H(state, p0);
}
int32_t reloq_engb61_UK_ph_I(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_I(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_I,args,1,1);
    }
    return reloq_engb61_UK_ph_I(state, p0);
}
int32_t reloq_engb61_UK_ph_L(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_L(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_L,args,1,1);
    }
    return reloq_engb61_UK_ph_L(state, p0);
}
int32_t reloq_engb61_UK_ph_R(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_R(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_R,args,1,1);
    }
    return reloq_engb61_UK_ph_R(state, p0);
}
int32_t reloq_engb61_UK_ph_U(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_U(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_U,args,1,1);
    }
    return reloq_engb61_UK_ph_U(state, p0);
}
int32_t reloq_engb61_UK_ph_X(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_X(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_X,args,1,1);
    }
    return reloq_engb61_UK_ph_X(state, p0);
}
int32_t reloq_engb61_UK_ph_a(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_a,args,1,1);
    }
    return reloq_engb61_UK_ph_a(state, p0);
}
int32_t reloq_engb61_UK_ph_ac(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_ac(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_ac,args,1,1);
    }
    return reloq_engb61_UK_ph_ac(state, p0);
}
int32_t reloq_engb61_UK_ph_c(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_c(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_c,args,1,1);
    }
    return reloq_engb61_UK_ph_c(state, p0);
}
int32_t reloq_engb61_UK_ph_e(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_e,args,1,1);
    }
    return reloq_engb61_UK_ph_e(state, p0);
}
int32_t reloq_engb61_UK_ph_i(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_i,args,1,1);
    }
    return reloq_engb61_UK_ph_i(state, p0);
}
int32_t reloq_engb61_UK_ph_l(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_l,args,1,1);
    }
    return reloq_engb61_UK_ph_l(state, p0);
}
int32_t reloq_engb61_UK_ph_m(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_m,args,1,1);
    }
    return reloq_engb61_UK_ph_m(state, p0);
}
int32_t reloq_engb61_UK_ph_n(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_n,args,1,1);
    }
    return reloq_engb61_UK_ph_n(state, p0);
}
int32_t reloq_engb61_UK_ph_o(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_o,args,1,1);
    }
    return reloq_engb61_UK_ph_o(state, p0);
}
int32_t reloq_engb61_UK_ph_r(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_r,args,1,1);
    }
    return reloq_engb61_UK_ph_r(state, p0);
}
int32_t reloq_engb61_UK_ph_u(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_u,args,1,1);
    }
    return reloq_engb61_UK_ph_u(state, p0);
}
int32_t reloq_engb61_UK_ph_w(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_w,args,1,1);
    }
    return reloq_engb61_UK_ph_w(state, p0);
}
int32_t reloq_engb61_UK_ph_x(void *, int32_t);
int32_t reloq_engb61_UK_ph_y(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_ph_y,args,1,1);
    }
    return reloq_engb61_UK_ph_y(state, p0);
}
int32_t reloq_engb61_UK_pre_r_laxing(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_UK_pre_r_laxing(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_UK_pre_r_laxing,args,2,2);
    }
    return reloq_engb61_UK_pre_r_laxing(state, p0, p1);
}
int32_t reloq_engb61_UK_r_deletion(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_UK_r_deletion(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_UK_r_deletion,args,3,3);
    }
    return reloq_engb61_UK_r_deletion(state, p0, p1, p2);
}
int32_t reloq_engb61_UK_r_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_UK_r_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_UK_r_rules,args,3,3);
    }
    return reloq_engb61_UK_r_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_UK_schwa_insertion(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_UK_schwa_insertion(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_UK_schwa_insertion,args,2,2);
    }
    return reloq_engb61_UK_schwa_insertion(state, p0, p1);
}
int32_t reloq_engb61_UK_stress_adjust(void *, int32_t, int32_t);
int32_t reloq_engb61_UK_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_UK_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_UK_vals,args,1,1);
    }
    return reloq_engb61_UK_vals(state, p0);
}
int32_t reloq_engb61_UK_vow_r_to_r(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_UK_vow_r_to_r(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_UK_vow_r_to_r,args,4,4);
    }
    return reloq_engb61_UK_vow_r_to_r(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_UKroots1(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_UKroots1(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_UKroots1,args,3,3);
    }
    return reloq_engb61_UKroots1(state, p0, p1, p2);
}
int32_t reloq_engb61_UKroots2(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_UKroots2(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_UKroots2,args,3,3);
    }
    return reloq_engb61_UKroots2(state, p0, p1, p2);
}
int32_t reloq_engb61_URL(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_URL(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_URL,args,2,2);
    }
    return reloq_engb61_URL(state, p0, p1);
}
int32_t reloq_engb61_a_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_a_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_a_rules,args,1,1);
    }
    return reloq_engb61_a_rules(state, p0);
}
int32_t reloq_engb61_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbr,args,4,4);
    }
    return reloq_engb61_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_abbr_before_num(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_abbr_before_num(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbr_before_num,args,2,2);
    }
    return reloq_engb61_abbr_before_num(state, p0, p1);
}
int32_t reloq_engb61_abbreviation(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_abbreviation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbreviation,args,4,4);
    }
    return reloq_engb61_abbreviation(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_engb61_absval(state, p0, p1, p2);
}
int32_t reloq_engb61_acronym(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_acronym(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acronym,args,3,3);
    }
    return reloq_engb61_acronym(state, p0, p1, p2);
}
int32_t reloq_engb61_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acrotest,args,3,3);
    }
    return reloq_engb61_acrotest(state, p0, p1, p2);
}
int32_t reloq_engb61_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_engb61_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_engb61_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_add_UK_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_add_UK_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_UK_phones,args,3,3);
    }
    return reloq_engb61_add_UK_phones(state, p0, p1, p2);
}
int32_t reloq_engb61_add_eng_currency_units(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_add_eng_currency_units(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_eng_currency_units,args,4,4);
    }
    return reloq_engb61_add_eng_currency_units(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_add_eng_plural(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_add_eng_plural(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_eng_plural,args,3,3);
    }
    return reloq_engb61_add_eng_plural(state, p0, p1, p2);
}
int32_t reloq_engb61_add_eng_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_add_eng_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_eng_point_digits,args,3,3);
    }
    return reloq_engb61_add_eng_point_digits(state, p0, p1, p2);
}
int32_t reloq_engb61_add_eng_subunits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_add_eng_subunits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_eng_subunits,args,3,3);
    }
    return reloq_engb61_add_eng_subunits(state, p0, p1, p2);
}
int32_t reloq_engb61_add_f0_for_new_voice(void *, int32_t);
static inline int32_t reloq_call_engb61_add_f0_for_new_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_new_voice,args,1,1);
    }
    return reloq_engb61_add_f0_for_new_voice(state, p0);
}
int32_t reloq_engb61_add_f0_for_next_voice(void *, int32_t);
static inline int32_t reloq_call_engb61_add_f0_for_next_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_next_voice,args,1,1);
    }
    return reloq_engb61_add_f0_for_next_voice(state, p0);
}
int32_t reloq_engb61_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_engb61_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_engb61_add_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_add_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_point_digits,args,3,3);
    }
    return reloq_engb61_add_point_digits(state, p0, p1, p2);
}
int32_t reloq_engb61_adj_vs_adv(void *, int32_t);
static inline int32_t reloq_call_engb61_adj_vs_adv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adj_vs_adv,args,1,1);
    }
    return reloq_engb61_adj_vs_adv(state, p0);
}
int32_t reloq_engb61_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_engb61_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_engb61_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_engb61_adjust_AV_for_fem_voice(void *, int32_t);
static inline int32_t reloq_call_engb61_adjust_AV_for_fem_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_fem_voice,args,1,1);
    }
    return reloq_engb61_adjust_AV_for_fem_voice(state, p0);
}
int32_t reloq_engb61_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_engb61_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_adjust_eng_accents(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_eng_accents(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_eng_accents,args,3,3);
    }
    return reloq_engb61_adjust_eng_accents(state, p0, p1, p2);
}
int32_t reloq_engb61_adjust_eng_female_breathiness(void *, int32_t);
static inline int32_t reloq_call_engb61_adjust_eng_female_breathiness(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_eng_female_breathiness,args,1,1);
    }
    return reloq_engb61_adjust_eng_female_breathiness(state, p0);
}
int32_t reloq_engb61_adjust_eng_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_eng_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_eng_word_stress,args,3,3);
    }
    return reloq_engb61_adjust_eng_word_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_adjust_final_y(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_final_y(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_final_y,args,2,2);
    }
    return reloq_engb61_adjust_final_y(state, p0, p1);
}
int32_t reloq_engb61_adjust_lg_specific_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_lg_specific_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_lg_specific_accents,args,5,5);
    }
    return reloq_engb61_adjust_lg_specific_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_adjust_mc_prefix(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_mc_prefix(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_mc_prefix,args,2,2);
    }
    return reloq_engb61_adjust_mc_prefix(state, p0, p1);
}
int32_t reloq_engb61_adjust_nuc_AV_for_unstr(void *, int32_t);
static inline int32_t reloq_call_engb61_adjust_nuc_AV_for_unstr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_nuc_AV_for_unstr,args,1,1);
    }
    return reloq_engb61_adjust_nuc_AV_for_unstr(state, p0);
}
int32_t reloq_engb61_adjust_past_tense_pronunc(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_past_tense_pronunc(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_past_tense_pronunc,args,3,3);
    }
    return reloq_engb61_adjust_past_tense_pronunc(state, p0, p1, p2);
}
int32_t reloq_engb61_adjust_plur_poss_pronunc(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_plur_poss_pronunc(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_plur_poss_pronunc,args,3,3);
    }
    return reloq_engb61_adjust_plur_poss_pronunc(state, p0, p1, p2);
}
int32_t reloq_engb61_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_prev_phr_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_pause,args,3,3);
    }
    return reloq_engb61_adjust_prev_phr_pause(state, p0, p1, p2);
}
int32_t reloq_engb61_adjust_root(void *, int32_t);
static inline int32_t reloq_call_engb61_adjust_root(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_root,args,1,1);
    }
    return reloq_engb61_adjust_root(state, p0);
}
int32_t reloq_engb61_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_engb61_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_adjust_words_in_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_words_in_phrase,args,3,3);
    }
    return reloq_engb61_adjust_words_in_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_after_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_after_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_after_comma_phrase,args,1,1);
    }
    return reloq_engb61_after_comma_phrase(state, p0);
}
int32_t reloq_engb61_all_nouns_lookup_sets(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_all_nouns_lookup_sets(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_all_nouns_lookup_sets,args,5,5);
    }
    return reloq_engb61_all_nouns_lookup_sets(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_ambig_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_ambig_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ambig_abbr,args,2,2);
    }
    return reloq_engb61_ambig_abbr(state, p0, p1);
}
int32_t reloq_engb61_analyze_morphs1(void *, int32_t);
static inline int32_t reloq_call_engb61_analyze_morphs1(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_analyze_morphs1,args,1,1);
    }
    return reloq_engb61_analyze_morphs1(state, p0);
}
int32_t reloq_engb61_analyze_morphs2(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_analyze_morphs2(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_analyze_morphs2,args,3,3);
    }
    return reloq_engb61_analyze_morphs2(state, p0, p1, p2);
}
int32_t reloq_engb61_antepenult_one(void *, int32_t);
static inline int32_t reloq_call_engb61_antepenult_one(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_antepenult_one,args,1,1);
    }
    return reloq_engb61_antepenult_one(state, p0);
}
int32_t reloq_engb61_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_engb61_apostrophe(state, p0, p1, p2);
}
int32_t reloq_engb61_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_engb61_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_engb61_apply_comma_phr_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_apply_comma_phr_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_comma_phr_rules,args,1,1);
    }
    return reloq_engb61_apply_comma_phr_rules(state, p0);
}
int32_t reloq_engb61_apply_eng_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_eng_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_eng_comma_rules,args,3,3);
    }
    return reloq_engb61_apply_eng_comma_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_apply_half_stress(void *, int32_t);
static inline int32_t reloq_call_engb61_apply_half_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_half_stress,args,1,1);
    }
    return reloq_engb61_apply_half_stress(state, p0);
}
int32_t reloq_engb61_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_engb61_apply_head_size_val(state, p0, p1);
}
int32_t reloq_engb61_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_lang_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_lang_comma_rules,args,3,3);
    }
    return reloq_engb61_apply_lang_comma_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_apply_long_alternation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_long_alternation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_long_alternation,args,3,3);
    }
    return reloq_engb61_apply_long_alternation(state, p0, p1, p2);
}
int32_t reloq_engb61_apply_longalt_final_two_opt(void *, int32_t);
static inline int32_t reloq_call_engb61_apply_longalt_final_two_opt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_longalt_final_two_opt,args,1,1);
    }
    return reloq_engb61_apply_longalt_final_two_opt(state, p0);
}
int32_t reloq_engb61_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_engb61_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_engb61_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_engb61_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_engb61_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_engb61_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_engb61_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_engb61_apply_roughness_val(state, p0, p1);
}
int32_t reloq_engb61_apply_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_engb61_apply_rules(state, p0);
}
int32_t reloq_engb61_apply_spanish_stress(void *, int32_t);
static inline int32_t reloq_call_engb61_apply_spanish_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_spanish_stress,args,1,1);
    }
    return reloq_engb61_apply_spanish_stress(state, p0);
}
int32_t reloq_engb61_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_engb61_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_engb61_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_engb61_apply_speed_val(state, p0, p1);
}
int32_t reloq_engb61_apply_stressVal(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_stressVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_stressVal,args,4,4);
    }
    return reloq_engb61_apply_stressVal(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_apply_strong_alternation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_strong_alternation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_strong_alternation,args,3,3);
    }
    return reloq_engb61_apply_strong_alternation(state, p0, p1, p2);
}
int32_t reloq_engb61_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_engb61_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_engb61_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_engb61_apply_volume_val(state, p0, p1);
}
int32_t reloq_engb61_are_same_tense(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_are_same_tense(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_are_same_tense,args,3,3);
    }
    return reloq_engb61_are_same_tense(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_engb61_assign_acoustic_vals(state, p0);
}
int32_t reloq_engb61_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_engb61_assign_alt_quest_features(state, p0);
}
int32_t reloq_engb61_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_engb61_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_engb61_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_engb61_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_engb61_assign_default_quest_features(state, p0);
}
int32_t reloq_engb61_assign_default_word_cats(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_default_word_cats(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_word_cats,args,2,2);
    }
    return reloq_engb61_assign_default_word_cats(state, p0, p1);
}
int32_t reloq_engb61_assign_eng_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_eng_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_eng_nuc_durs,args,1,1);
    }
    return reloq_engb61_assign_eng_nuc_durs(state, p0);
}
int32_t reloq_engb61_assign_eng_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_eng_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_eng_stress,args,3,3);
    }
    return reloq_engb61_assign_eng_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_eng_syll_init_con(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_eng_syll_init_con(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_eng_syll_init_con,args,3,3);
    }
    return reloq_engb61_assign_eng_syll_init_con(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_engb61_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_engb61_assign_nuc_durs(state, p0);
}
int32_t reloq_engb61_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_engb61_assign_phone_durs(state, p0);
}
int32_t reloq_engb61_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_engb61_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_engb61_assign_prefix_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_prefix_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prefix_stress,args,3,3);
    }
    return reloq_engb61_assign_prefix_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_engb61_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_assign_root_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_root_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_root_stress,args,3,3);
    }
    return reloq_engb61_assign_root_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_s_clitic_category(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_s_clitic_category(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_s_clitic_category,args,5,5);
    }
    return reloq_engb61_assign_s_clitic_category(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_engb61_assign_sent_final_features(state, p0);
}
int32_t reloq_engb61_assign_suffix_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_suffix_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_suffix_stress,args,2,2);
    }
    return reloq_engb61_assign_suffix_stress(state, p0, p1);
}
int32_t reloq_engb61_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_engb61_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_user_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_assign_user_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_user_stress,args,3,3);
    }
    return reloq_engb61_assign_user_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_engb61_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_engb61_assign_wh_quest_features(state, p0);
}
int32_t reloq_engb61_aux_sequence(void *, int32_t);
static inline int32_t reloq_call_engb61_aux_sequence(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_aux_sequence,args,1,1);
    }
    return reloq_engb61_aux_sequence(state, p0);
}
int32_t reloq_engb61_b_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_b_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_b_rules,args,1,1);
    }
    return reloq_engb61_b_rules(state, p0);
}
int32_t reloq_engb61_be_vs_poss(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_be_vs_poss(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_be_vs_poss,args,3,3);
    }
    return reloq_engb61_be_vs_poss(state, p0, p1, p2);
}
int32_t reloq_engb61_beg_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_beg_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_beg_of_word,args,2,2);
    }
    return reloq_engb61_beg_of_word(state, p0, p1);
}
int32_t reloq_engb61_begins_noun_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_begins_noun_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_begins_noun_phrase,args,2,2);
    }
    return reloq_engb61_begins_noun_phrase(state, p0, p1);
}
int32_t reloq_engb61_between_proper_names(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_boundary_tone,args,1,1);
    }
    return reloq_engb61_boundary_tone(state, p0);
}
int32_t reloq_engb61_bracket(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_bracket(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_bracket,args,3,3);
    }
    return reloq_engb61_bracket(state, p0, p1, p2);
}
int32_t reloq_engb61_break_into_eng_syllables(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_break_into_eng_syllables(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_eng_syllables,args,3,3);
    }
    return reloq_engb61_break_into_eng_syllables(state, p0, p1, p2);
}
int32_t reloq_engb61_break_into_morphs(void *, int32_t);
static inline int32_t reloq_call_engb61_break_into_morphs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_break_into_morphs,args,1,1);
    }
    return reloq_engb61_break_into_morphs(state, p0);
}
int32_t reloq_engb61_break_into_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_break_into_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_phones,args,3,3);
    }
    return reloq_engb61_break_into_phones(state, p0, p1, p2);
}
int32_t reloq_engb61_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_engb61_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_engb61_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_engb61_build_phrases(state, p0, p1, p2);
}
int32_t reloq_engb61_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_engb61_build_words(state, p0, p1, p2);
}
int32_t reloq_engb61_c_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_c_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_c_rules,args,1,1);
    }
    return reloq_engb61_c_rules(state, p0);
}
int32_t reloq_engb61_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_engb61_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_engb61_change_british_spelling(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_change_british_spelling(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_change_british_spelling,args,3,3);
    }
    return reloq_engb61_change_british_spelling(state, p0, p1, p2);
}
int32_t reloq_engb61_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_engb61_char_name(state, p0, p1, p2);
}
int32_t reloq_engb61_city_state(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_city_state(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_city_state,args,5,5);
    }
    return reloq_engb61_city_state(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_clear_delta(void *, int32_t);
static inline int32_t reloq_call_engb61_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_engb61_clear_delta(state, p0);
}
int32_t reloq_engb61_clitics(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_clitics,args,5,5);
    }
    return reloq_engb61_clitics(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_close_platform(void *, int32_t);
static inline int32_t reloq_call_engb61_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_engb61_close_platform(state, p0);
}
int32_t reloq_engb61_comma_at_end_quote(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_comma_at_end_quote(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_comma_at_end_quote,args,3,3);
    }
    return reloq_engb61_comma_at_end_quote(state, p0, p1, p2);
}
int32_t reloq_engb61_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_engb61_comma_phrase(state, p0);
}
int32_t reloq_engb61_complementizer(void *, int32_t);
static inline int32_t reloq_call_engb61_complementizer(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_complementizer,args,1,1);
    }
    return reloq_engb61_complementizer(state, p0);
}
int32_t reloq_engb61_compound_destress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_compound_destress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_compound_destress,args,4,4);
    }
    return reloq_engb61_compound_destress(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_compound_sets_lookup(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_compound_sets_lookup(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_compound_sets_lookup,args,6,6);
    }
    return reloq_engb61_compound_sets_lookup(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_compounds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_compounds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_compounds,args,3,3);
    }
    return reloq_engb61_compounds(state, p0, p1, p2);
}
int32_t reloq_engb61_con_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_engb61_con_vals(state, p0);
}
int32_t reloq_engb61_continuation_phr_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_continuation_phr_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_continuation_phr_phrase_tone,args,1,1);
    }
    return reloq_engb61_continuation_phr_phrase_tone(state, p0);
}
int32_t reloq_engb61_continuation_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_continuation_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_continuation_phrase,args,3,3);
    }
    return reloq_engb61_continuation_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_1_or_2_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_1_or_2_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_1_or_2_digits,args,3,3);
    }
    return reloq_engb61_convert_1_or_2_digits(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_1_to_one(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_1_to_one(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_1_to_one,args,3,3);
    }
    return reloq_engb61_convert_1_to_one(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_NA_phone_number(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_convert_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits,args,3,3);
    }
    return reloq_engb61_convert_digits(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits_to_words,args,3,3);
    }
    return reloq_engb61_convert_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_double,args,3,3);
    }
    return reloq_engb61_convert_double(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_date,args,3,3);
    }
    return reloq_engb61_convert_eng_date(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_day_of_month(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_day_of_month(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_day_of_month,args,3,3);
    }
    return reloq_engb61_convert_eng_day_of_month(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_digits_to_words,args,3,3);
    }
    return reloq_engb61_convert_eng_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_double,args,3,3);
    }
    return reloq_engb61_convert_eng_double(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_fraction,args,3,3);
    }
    return reloq_engb61_convert_eng_fraction(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_hundreds,args,3,3);
    }
    return reloq_engb61_convert_eng_hundreds(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_monetary_digits(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_monetary_digits(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_monetary_digits,args,4,4);
    }
    return reloq_engb61_convert_eng_monetary_digits(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_convert_eng_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_monetary_exp,args,5,5);
    }
    return reloq_engb61_convert_eng_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_convert_eng_phone_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_phone_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_phone_number,args,3,3);
    }
    return reloq_engb61_convert_eng_phone_number(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_single_digit,args,3,3);
    }
    return reloq_engb61_convert_eng_single_digit(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_teens(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_teens(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_teens,args,3,3);
    }
    return reloq_engb61_convert_eng_teens(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_eng_time(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_eng_time(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_eng_time,args,3,3);
    }
    return reloq_engb61_convert_eng_time(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_hundreds,args,3,3);
    }
    return reloq_engb61_convert_hundreds(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_lang_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_lang_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_lang_digits_to_words,args,3,3);
    }
    return reloq_engb61_convert_lang_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_large_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_large_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_large_numbers,args,3,3);
    }
    return reloq_engb61_convert_large_numbers(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_monetary_exp,args,5,5);
    }
    return reloq_engb61_convert_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_convert_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_numbers,args,3,3);
    }
    return reloq_engb61_convert_numbers(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_phone_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_phone_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_phone_number,args,3,3);
    }
    return reloq_engb61_convert_phone_number(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_single_digit,args,3,3);
    }
    return reloq_engb61_convert_single_digit(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_to_day_of_month(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_to_day_of_month(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_day_of_month,args,4,4);
    }
    return reloq_engb61_convert_to_day_of_month(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_convert_to_eng_ordinal(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_to_eng_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_eng_ordinal,args,4,4);
    }
    return reloq_engb61_convert_to_eng_ordinal(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_convert_to_eng_year(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_to_eng_year(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_eng_year,args,4,4);
    }
    return reloq_engb61_convert_to_eng_year(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_engb61_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_engb61_convert_zero_digit(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_convert_zero_digit(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_zero_digit,args,4,4);
    }
    return reloq_engb61_convert_zero_digit(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_coord_conj(void *, int32_t);
static inline int32_t reloq_call_engb61_coord_conj(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_coord_conj,args,1,1);
    }
    return reloq_engb61_coord_conj(state, p0);
}
int32_t reloq_engb61_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_engb61_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_engb61_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_engb61_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_copy_token(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_token,args,5,5);
    }
    return reloq_engb61_copy_token(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_engb61_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_engb61_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_count_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_count_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_digits,args,3,3);
    }
    return reloq_engb61_count_digits(state, p0, p1, p2);
}
int32_t reloq_engb61_count_eng_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_count_eng_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_eng_digits,args,3,3);
    }
    return reloq_engb61_count_eng_digits(state, p0, p1, p2);
}
int32_t reloq_engb61_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_engb61_count_input(state, p0, p1);
}
int32_t reloq_engb61_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_engb61_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_engb61_count_normalized_inp(state, p0);
}
int32_t reloq_engb61_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_count_sylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_sylls,args,4,4);
    }
    return reloq_engb61_count_sylls(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_engb61_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_engb61_create_eng_syllables(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_create_eng_syllables(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_eng_syllables,args,3,3);
    }
    return reloq_engb61_create_eng_syllables(state, p0, p1, p2);
}
int32_t reloq_engb61_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_engb61_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_engb61_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_engb61_create_sentence(state, p0, p1);
}
int32_t reloq_engb61_d_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_d_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_d_rules,args,1,1);
    }
    return reloq_engb61_d_rules(state, p0);
}
int32_t reloq_engb61_day_month(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_day_month(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_day_month,args,2,2);
    }
    return reloq_engb61_day_month(state, p0, p1);
}
int32_t reloq_engb61_default_stress(void *, int32_t);
static inline int32_t reloq_call_engb61_default_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_default_stress,args,1,1);
    }
    return reloq_engb61_default_stress(state, p0);
}
int32_t reloq_engb61_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_engb61_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_degeminate_l(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_degeminate_l(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_degeminate_l,args,3,3);
    }
    return reloq_engb61_degeminate_l(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_engb61_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_engb61_delete_2_more(state, p0);
}
int32_t reloq_engb61_delete_comma_sync(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_comma_sync(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_comma_sync,args,2,2);
    }
    return reloq_engb61_delete_comma_sync(state, p0, p1);
}
int32_t reloq_engb61_delete_extra_syncs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_extra_syncs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_extra_syncs,args,3,3);
    }
    return reloq_engb61_delete_extra_syncs(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_h(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_h(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_h,args,3,3);
    }
    return reloq_engb61_delete_h(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_h_of_funct_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_h_of_funct_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_h_of_funct_word,args,4,4);
    }
    return reloq_engb61_delete_h_of_funct_word(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_inp_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_left,args,3,3);
    }
    return reloq_engb61_delete_inp_from_left(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_inp_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_right,args,3,3);
    }
    return reloq_engb61_delete_inp_from_right(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_engb61_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_morph_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_morph_from_left,args,3,3);
    }
    return reloq_engb61_delete_morph_from_left(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_engb61_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_phone_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_right,args,3,3);
    }
    return reloq_engb61_delete_phone_from_right(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_schwa_bef_l(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_schwa_bef_l(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_schwa_bef_l,args,2,2);
    }
    return reloq_engb61_delete_schwa_bef_l(state, p0, p1);
}
int32_t reloq_engb61_delete_schwa_r_syll(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_schwa_r_syll(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_schwa_r_syll,args,2,2);
    }
    return reloq_engb61_delete_schwa_r_syll(state, p0, p1);
}
int32_t reloq_engb61_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_syll_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_syll_from_left,args,3,3);
    }
    return reloq_engb61_delete_syll_from_left(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_syll_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_syll_from_right,args,3,3);
    }
    return reloq_engb61_delete_syll_from_right(state, p0, p1, p2);
}
int32_t reloq_engb61_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_engb61_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_engb61_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_engb61_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_engb61_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_engb61_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_engb61_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_engb61_demote_word(state, p0, p1, p2);
}
int32_t reloq_engb61_destress_short_verbs(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_destress_short_verbs(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_short_verbs,args,2,2);
    }
    return reloq_engb61_destress_short_verbs(state, p0, p1);
}
int32_t reloq_engb61_destress_time_words(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_destress_time_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_time_words,args,4,4);
    }
    return reloq_engb61_destress_time_words(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_diaphon_based_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_diaphon_based_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_diaphon_based_rules,args,1,1);
    }
    return reloq_engb61_diaphon_based_rules(state, p0);
}
int32_t reloq_engb61_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_engb61_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_disambig_eng_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_disambig_eng_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_eng_content_word,args,3,3);
    }
    return reloq_engb61_disambig_eng_content_word(state, p0, p1, p2);
}
int32_t reloq_engb61_disambig_eng_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_disambig_eng_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_eng_functword,args,3,3);
    }
    return reloq_engb61_disambig_eng_functword(state, p0, p1, p2);
}
int32_t reloq_engb61_disambiguate_am(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_am(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_am,args,1,1);
    }
    return reloq_engb61_disambiguate_am(state, p0);
}
int32_t reloq_engb61_disambiguate_being(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_being(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_being,args,1,1);
    }
    return reloq_engb61_disambiguate_being(state, p0);
}
int32_t reloq_engb61_disambiguate_can(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_can(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_can,args,1,1);
    }
    return reloq_engb61_disambiguate_can(state, p0);
}
int32_t reloq_engb61_disambiguate_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_content_word,args,4,4);
    }
    return reloq_engb61_disambiguate_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_disambiguate_does(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_does(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_does,args,1,1);
    }
    return reloq_engb61_disambiguate_does(state, p0);
}
int32_t reloq_engb61_disambiguate_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_functword,args,3,3);
    }
    return reloq_engb61_disambiguate_functword(state, p0, p1, p2);
}
int32_t reloq_engb61_disambiguate_get(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_get(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_get,args,1,1);
    }
    return reloq_engb61_disambiguate_get(state, p0);
}
int32_t reloq_engb61_disambiguate_like(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_like(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_like,args,1,1);
    }
    return reloq_engb61_disambiguate_like(state, p0);
}
int32_t reloq_engb61_disambiguate_may(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_may(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_may,args,1,1);
    }
    return reloq_engb61_disambiguate_may(state, p0);
}
int32_t reloq_engb61_disambiguate_might(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_might(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_might,args,1,1);
    }
    return reloq_engb61_disambiguate_might(state, p0);
}
int32_t reloq_engb61_disambiguate_mine(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_mine(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_mine,args,1,1);
    }
    return reloq_engb61_disambiguate_mine(state, p0);
}
int32_t reloq_engb61_disambiguate_must(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_must(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_must,args,1,1);
    }
    return reloq_engb61_disambiguate_must(state, p0);
}
int32_t reloq_engb61_disambiguate_one(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_one(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_one,args,1,1);
    }
    return reloq_engb61_disambiguate_one(state, p0);
}
int32_t reloq_engb61_disambiguate_still(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_still(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_still,args,1,1);
    }
    return reloq_engb61_disambiguate_still(state, p0);
}
int32_t reloq_engb61_disambiguate_that(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_that(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_that,args,1,1);
    }
    return reloq_engb61_disambiguate_that(state, p0);
}
int32_t reloq_engb61_disambiguate_till(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_till(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_till,args,1,1);
    }
    return reloq_engb61_disambiguate_till(state, p0);
}
int32_t reloq_engb61_disambiguate_while(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_while(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_while,args,1,1);
    }
    return reloq_engb61_disambiguate_while(state, p0);
}
int32_t reloq_engb61_disambiguate_will(void *, int32_t);
static inline int32_t reloq_call_engb61_disambiguate_will(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_will,args,1,1);
    }
    return reloq_engb61_disambiguate_will(state, p0);
}
int32_t reloq_engb61_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_engb61_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_engb61_distribute_nucdur(state, p0);
}
int32_t reloq_engb61_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_engb61_divide_char_count_token(state, p0, p1);
}
int32_t reloq_engb61_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_engb61_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_engb61_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_engb61_doF(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_engb61_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_engb61_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_engb61_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_engb61_do_FTZ_FTP(state, p0);
}
int32_t reloq_engb61_do_eng_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_do_eng_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_eng_h_amps,args,2,2);
    }
    return reloq_engb61_do_eng_h_amps(state, p0, p1);
}
int32_t reloq_engb61_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_engb61_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_engb61_do_glot_onset(void *, int32_t);
static inline int32_t reloq_call_engb61_do_glot_onset(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_glot_onset,args,1,1);
    }
    return reloq_engb61_do_glot_onset(state, p0);
}
int32_t reloq_engb61_do_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_do_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_h_amps,args,2,2);
    }
    return reloq_engb61_do_h_amps(state, p0, p1);
}
int32_t reloq_engb61_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_engb61_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_engb61_do_non_son_amps(state, p0);
}
int32_t reloq_engb61_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_engb61_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_engb61_do_pt_voicing(state, p0);
}
int32_t reloq_engb61_do_release(void *, int32_t);
static inline int32_t reloq_call_engb61_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_engb61_do_release(state, p0);
}
int32_t reloq_engb61_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_engb61_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_engb61_do_son_amps(state, p0);
}
int32_t reloq_engb61_do_voice_bar(void *, int32_t);
static inline int32_t reloq_call_engb61_do_voice_bar(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_voice_bar,args,1,1);
    }
    return reloq_engb61_do_voice_bar(state, p0);
}
int32_t reloq_engb61_do_word_initial_creak(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_do_word_initial_creak(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_word_initial_creak,args,2,2);
    }
    return reloq_engb61_do_word_initial_creak(state, p0, p1);
}
int32_t reloq_engb61_dur_adjustments(void *, int32_t);
static inline int32_t reloq_call_engb61_dur_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_dur_adjustments,args,1,1);
    }
    return reloq_engb61_dur_adjustments(state, p0);
}
int32_t reloq_engb61_e_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_e_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_e_rules,args,1,1);
    }
    return reloq_engb61_e_rules(state, p0);
}
int32_t reloq_engb61_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_engb61_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_engb61_ellipsis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_ellipsis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ellipsis,args,3,3);
    }
    return reloq_engb61_ellipsis(state, p0, p1, p2);
}
int32_t reloq_engb61_email_address(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_email_address(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_email_address,args,2,2);
    }
    return reloq_engb61_email_address(state, p0, p1);
}
int32_t reloq_engb61_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_engb61_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_engb61_end_morph_analysis(void *, int32_t);
static inline int32_t reloq_call_engb61_end_morph_analysis(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_end_morph_analysis,args,1,1);
    }
    return reloq_engb61_end_morph_analysis(state, p0);
}
int32_t reloq_engb61_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_engb61_end_of_sentence(state, p0, p1);
}
int32_t reloq_engb61_end_of_token(void *, int32_t, int32_t);
int32_t reloq_engb61_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_engb61_end_of_word(state, p0, p1);
}
int32_t reloq_engb61_eng_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_abbr,args,4,4);
    }
    return reloq_engb61_eng_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_eng_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_acrotest,args,3,3);
    }
    return reloq_engb61_eng_acrotest(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_alv_Fv(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_alv_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_alv_Fv,args,1,1);
    }
    return reloq_engb61_eng_alv_Fv(state, p0);
}
int32_t reloq_engb61_eng_bilab_Fv(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_bilab_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_bilab_Fv,args,1,1);
    }
    return reloq_engb61_eng_bilab_Fv(state, p0);
}
int32_t reloq_engb61_eng_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_boundary_tone,args,1,1);
    }
    return reloq_engb61_eng_boundary_tone(state, p0);
}
int32_t reloq_engb61_eng_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_char_name,args,3,3);
    }
    return reloq_engb61_eng_char_name(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_con_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_con_vals,args,1,1);
    }
    return reloq_engb61_eng_con_vals(state, p0);
}
int32_t reloq_engb61_eng_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_cons_durs,args,3,3);
    }
    return reloq_engb61_eng_cons_durs(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_first_postnucl_accent,args,1,1);
    }
    return reloq_engb61_eng_first_postnucl_accent(state, p0);
}
int32_t reloq_engb61_eng_funct_phrs_lvl_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_funct_phrs_lvl_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_funct_phrs_lvl_rules,args,3,3);
    }
    return reloq_engb61_eng_funct_phrs_lvl_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_funct_words(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_funct_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_funct_words,args,5,5);
    }
    return reloq_engb61_eng_funct_words(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_eng_glot_stop(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_glot_stop(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_glot_stop,args,1,1);
    }
    return reloq_engb61_eng_glot_stop(state, p0);
}
int32_t reloq_engb61_eng_gramm_cat_dict(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_gramm_cat_dict(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_gramm_cat_dict,args,4,4);
    }
    return reloq_engb61_eng_gramm_cat_dict(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_eng_gramm_cat_disambig_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_gramm_cat_disambig_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_gramm_cat_disambig_dict,args,3,3);
    }
    return reloq_engb61_eng_gramm_cat_disambig_dict(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_high_low_star_tone,args,1,1);
    }
    return reloq_engb61_eng_high_low_star_tone(state, p0);
}
int32_t reloq_engb61_eng_high_pal_Fv(void *, int32_t);
int32_t reloq_engb61_eng_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_high_star_low_tone,args,1,1);
    }
    return reloq_engb61_eng_high_star_low_tone(state, p0);
}
int32_t reloq_engb61_eng_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_high_tone,args,1,1);
    }
    return reloq_engb61_eng_high_tone(state, p0);
}
int32_t reloq_engb61_eng_homog_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_homog_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_homog_words,args,3,3);
    }
    return reloq_engb61_eng_homog_words(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_hyphen_between_nums,args,5,5);
    }
    return reloq_engb61_eng_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_eng_hyphenated_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_hyphenated_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_hyphenated_word,args,3,3);
    }
    return reloq_engb61_eng_hyphenated_word(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_intd_Fv(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_intd_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_intd_Fv,args,1,1);
    }
    return reloq_engb61_eng_intd_Fv(state, p0);
}
int32_t reloq_engb61_eng_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_internal_dict_lookup,args,3,3);
    }
    return reloq_engb61_eng_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_lab_Fv(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_lab_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_lab_Fv,args,1,1);
    }
    return reloq_engb61_eng_lab_Fv(state, p0);
}
int32_t reloq_engb61_eng_lat_Fv(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_lat_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_lat_Fv,args,1,1);
    }
    return reloq_engb61_eng_lat_Fv(state, p0);
}
int32_t reloq_engb61_eng_legal_final_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_legal_final_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_legal_final_cons,args,2,2);
    }
    return reloq_engb61_eng_legal_final_cons(state, p0, p1);
}
int32_t reloq_engb61_eng_legal_initial_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_legal_initial_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_legal_initial_cons,args,2,2);
    }
    return reloq_engb61_eng_legal_initial_cons(state, p0, p1);
}
int32_t reloq_engb61_eng_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_low_high_star_tone,args,1,1);
    }
    return reloq_engb61_eng_low_high_star_tone(state, p0);
}
int32_t reloq_engb61_eng_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_low_star_high_tone,args,1,1);
    }
    return reloq_engb61_eng_low_star_high_tone(state, p0);
}
int32_t reloq_engb61_eng_low_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_low_tone,args,1,1);
    }
    return reloq_engb61_eng_low_tone(state, p0);
}
int32_t reloq_engb61_eng_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_monetary_exp,args,3,3);
    }
    return reloq_engb61_eng_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_months(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_months(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_months,args,3,3);
    }
    return reloq_engb61_eng_months(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_non_phrasal_comma,args,2,2);
    }
    return reloq_engb61_eng_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_engb61_eng_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_engb61_eng_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_engb61_eng_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_engb61_eng_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_engb61_eng_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_high_tone,args,1,1);
    }
    return reloq_engb61_eng_nucl_high_tone(state, p0);
}
int32_t reloq_engb61_eng_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_engb61_eng_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_engb61_eng_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_engb61_eng_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_engb61_eng_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_low_tone,args,1,1);
    }
    return reloq_engb61_eng_nucl_low_tone(state, p0);
}
int32_t reloq_engb61_eng_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_pal_Fv,args,1,1);
    }
    return reloq_engb61_eng_pal_Fv(state, p0);
}
int32_t reloq_engb61_eng_ph_D(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_D(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_D,args,1,1);
    }
    return reloq_engb61_eng_ph_D(state, p0);
}
int32_t reloq_engb61_eng_ph_D_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_D_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_D_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_D_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_F(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_F(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_F,args,1,1);
    }
    return reloq_engb61_eng_ph_F(state, p0);
}
int32_t reloq_engb61_eng_ph_F_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_F_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_F_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_F_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_G(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_G(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_G,args,1,1);
    }
    return reloq_engb61_eng_ph_G(state, p0);
}
int32_t reloq_engb61_eng_ph_G_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_G_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_G_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_G_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_S(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_S,args,1,1);
    }
    return reloq_engb61_eng_ph_S(state, p0);
}
int32_t reloq_engb61_eng_ph_S_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_S_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_S_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_S_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_T(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_T(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_T,args,1,1);
    }
    return reloq_engb61_eng_ph_T(state, p0);
}
int32_t reloq_engb61_eng_ph_T_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_T_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_T_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_T_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_Z(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_Z,args,1,1);
    }
    return reloq_engb61_eng_ph_Z(state, p0);
}
int32_t reloq_engb61_eng_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_Z_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_Z_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_b(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_b,args,1,1);
    }
    return reloq_engb61_eng_ph_b(state, p0);
}
int32_t reloq_engb61_eng_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_b_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_b_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_d(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_d,args,1,1);
    }
    return reloq_engb61_eng_ph_d(state, p0);
}
int32_t reloq_engb61_eng_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_d_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_d_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_f(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_f(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_f,args,1,1);
    }
    return reloq_engb61_eng_ph_f(state, p0);
}
int32_t reloq_engb61_eng_ph_f_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_f_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_f_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_f_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_g(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_g,args,1,1);
    }
    return reloq_engb61_eng_ph_g(state, p0);
}
int32_t reloq_engb61_eng_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_g_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_g_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_glot(void *, int32_t);
int32_t reloq_engb61_eng_ph_glot_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_glot_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_glot_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_glot_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_k(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_k,args,1,1);
    }
    return reloq_engb61_eng_ph_k(state, p0);
}
int32_t reloq_engb61_eng_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_k_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_k_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_l(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_l,args,1,1);
    }
    return reloq_engb61_eng_ph_l(state, p0);
}
int32_t reloq_engb61_eng_ph_l_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_l_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_l_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_l_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_m(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_m,args,1,1);
    }
    return reloq_engb61_eng_ph_m(state, p0);
}
int32_t reloq_engb61_eng_ph_m_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_m_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_m_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_m_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_n(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_n,args,1,1);
    }
    return reloq_engb61_eng_ph_n(state, p0);
}
int32_t reloq_engb61_eng_ph_n_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_n_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_n_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_p(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_p,args,1,1);
    }
    return reloq_engb61_eng_ph_p(state, p0);
}
int32_t reloq_engb61_eng_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_p_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_p_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_r(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_r,args,1,1);
    }
    return reloq_engb61_eng_ph_r(state, p0);
}
int32_t reloq_engb61_eng_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_r_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_r_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_s(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_s,args,1,1);
    }
    return reloq_engb61_eng_ph_s(state, p0);
}
int32_t reloq_engb61_eng_ph_s_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_s_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_s_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_s_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_t(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_t,args,1,1);
    }
    return reloq_engb61_eng_ph_t(state, p0);
}
int32_t reloq_engb61_eng_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_t_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_t_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_v(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_v(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_v,args,1,1);
    }
    return reloq_engb61_eng_ph_v(state, p0);
}
int32_t reloq_engb61_eng_ph_v_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_v_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_v_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_v_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_w(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_w,args,1,1);
    }
    return reloq_engb61_eng_ph_w(state, p0);
}
int32_t reloq_engb61_eng_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_w_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_w_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_x(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_x(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_x,args,1,1);
    }
    return reloq_engb61_eng_ph_x(state, p0);
}
int32_t reloq_engb61_eng_ph_y(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_y,args,1,1);
    }
    return reloq_engb61_eng_ph_y(state, p0);
}
int32_t reloq_engb61_eng_ph_y_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_y_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_y_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_y_dur(state, p0);
}
int32_t reloq_engb61_eng_ph_z(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_z,args,1,1);
    }
    return reloq_engb61_eng_ph_z(state, p0);
}
int32_t reloq_engb61_eng_ph_z_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ph_z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ph_z_dur,args,1,1);
    }
    return reloq_engb61_eng_ph_z_dur(state, p0);
}
int32_t reloq_engb61_eng_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_phr_initial_F0,args,1,1);
    }
    return reloq_engb61_eng_phr_initial_F0(state, p0);
}
int32_t reloq_engb61_eng_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_phrase_level_rules,args,3,3);
    }
    return reloq_engb61_eng_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_phrase_tone,args,1,1);
    }
    return reloq_engb61_eng_phrase_tone(state, p0);
}
int32_t reloq_engb61_eng_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_position_t_ptrs,args,1,1);
    }
    return reloq_engb61_eng_position_t_ptrs(state, p0);
}
int32_t reloq_engb61_eng_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_post_comma_phrase,args,3,3);
    }
    return reloq_engb61_eng_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_postnucl_f0,args,1,1);
    }
    return reloq_engb61_eng_postnucl_f0(state, p0);
}
int32_t reloq_engb61_eng_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_prenucl_f0,args,1,1);
    }
    return reloq_engb61_eng_prenucl_f0(state, p0);
}
int32_t reloq_engb61_eng_ret_Fv(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_ret_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_ret_Fv,args,1,1);
    }
    return reloq_engb61_eng_ret_Fv(state, p0);
}
int32_t reloq_engb61_eng_roots_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_roots_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_roots_dict_lookup,args,3,3);
    }
    return reloq_engb61_eng_roots_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_said_phrase,args,4,4);
    }
    return reloq_engb61_eng_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_eng_symbolic_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_symbolic_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_symbolic_rules,args,3,3);
    }
    return reloq_engb61_eng_symbolic_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_tok_dict,args,3,3);
    }
    return reloq_engb61_eng_tok_dict(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_trans_features(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_trans_features,args,1,1);
    }
    return reloq_engb61_eng_trans_features(state, p0);
}
int32_t reloq_engb61_eng_vel_Fv(void *, int32_t);
static inline int32_t reloq_call_engb61_eng_vel_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_vel_Fv,args,1,1);
    }
    return reloq_engb61_eng_vel_Fv(state, p0);
}
int32_t reloq_engb61_eng_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_words,args,3,3);
    }
    return reloq_engb61_eng_words(state, p0, p1, p2);
}
int32_t reloq_engb61_eng_words_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_eng_words_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eng_words_dict_lookup,args,3,3);
    }
    return reloq_engb61_eng_words_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_engb61_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_engb61_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_estimate_eng_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_estimate_eng_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_eng_nsylls,args,4,4);
    }
    return reloq_engb61_estimate_eng_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_estimate_lang_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_estimate_lang_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_lang_nsylls,args,4,4);
    }
    return reloq_engb61_estimate_lang_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_estimate_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_nsylls,args,4,4);
    }
    return reloq_engb61_estimate_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_except_stress1(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_except_stress1(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_except_stress1,args,3,3);
    }
    return reloq_engb61_except_stress1(state, p0, p1, p2);
}
int32_t reloq_engb61_except_stress2(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_except_stress2(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_except_stress2,args,3,3);
    }
    return reloq_engb61_except_stress2(state, p0, p1, p2);
}
int32_t reloq_engb61_exceptional_stress_from_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_exceptional_stress_from_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_exceptional_stress_from_dict,args,3,3);
    }
    return reloq_engb61_exceptional_stress_from_dict(state, p0, p1, p2);
}
int32_t reloq_engb61_exclam_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_exclam_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_exclam_phrase,args,2,2);
    }
    return reloq_engb61_exclam_phrase(state, p0, p1);
}
int32_t reloq_engb61_extend_vals(void *, int32_t);
int32_t reloq_engb61_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_engb61_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_engb61_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_engb61_final_one(void *, int32_t);
static inline int32_t reloq_call_engb61_final_one(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_final_one,args,1,1);
    }
    return reloq_engb61_final_one(state, p0);
}
int32_t reloq_engb61_final_two(void *, int32_t);
static inline int32_t reloq_call_engb61_final_two(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_final_two,args,1,1);
    }
    return reloq_engb61_final_two(state, p0);
}
int32_t reloq_engb61_final_voiced_release(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_final_voiced_release(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_voiced_release,args,3,3);
    }
    return reloq_engb61_final_voiced_release(state, p0, p1, p2);
}
int32_t reloq_engb61_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_engb61_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_engb61_find_acoustic_nuc(state, p0);
}
int32_t reloq_engb61_find_an_if(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_an_if(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_an_if,args,2,2);
    }
    return reloq_engb61_find_an_if(state, p0, p1);
}
int32_t reloq_engb61_find_enclitic_to_left(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_enclitic_to_left(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_enclitic_to_left,args,5,5);
    }
    return reloq_engb61_find_enclitic_to_left(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_find_enclitic_to_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_enclitic_to_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_enclitic_to_right,args,3,3);
    }
    return reloq_engb61_find_enclitic_to_right(state, p0, p1, p2);
}
int32_t reloq_engb61_find_eng_enclitic_to_left(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_eng_enclitic_to_left(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_eng_enclitic_to_left,args,5,5);
    }
    return reloq_engb61_find_eng_enclitic_to_left(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_find_eng_enclitic_to_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_eng_enclitic_to_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_eng_enclitic_to_right,args,3,3);
    }
    return reloq_engb61_find_eng_enclitic_to_right(state, p0, p1, p2);
}
int32_t reloq_engb61_find_eng_nucleus(void *, int32_t);
static inline int32_t reloq_call_engb61_find_eng_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_eng_nucleus,args,1,1);
    }
    return reloq_engb61_find_eng_nucleus(state, p0);
}
int32_t reloq_engb61_find_eng_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_eng_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_eng_obligatory_phrase,args,4,4);
    }
    return reloq_engb61_find_eng_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_eng_potential_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_find_eng_potential_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_eng_potential_phrase,args,1,1);
    }
    return reloq_engb61_find_eng_potential_phrase(state, p0);
}
int32_t reloq_engb61_find_lang_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_lang_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_obligatory_phrase,args,4,4);
    }
    return reloq_engb61_find_lang_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_lang_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_lang_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_potential_phrase,args,4,4);
    }
    return reloq_engb61_find_lang_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_engb61_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_most_stressed_syll(void *, int32_t);
static inline int32_t reloq_call_engb61_find_most_stressed_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_most_stressed_syll,args,1,1);
    }
    return reloq_engb61_find_most_stressed_syll(state, p0);
}
int32_t reloq_engb61_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_next_token(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_token,args,4,4);
    }
    return reloq_engb61_find_next_token(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_next_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_word,args,4,4);
    }
    return reloq_engb61_find_next_word(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_engb61_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_engb61_find_nucleus(state, p0);
}
int32_t reloq_engb61_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_engb61_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_preceding_city_name(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_preceding_city_name(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_preceding_city_name,args,4,4);
    }
    return reloq_engb61_find_preceding_city_name(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_prev_word_optional_comma(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_prev_word_optional_comma,args,4,4);
    }
    return reloq_engb61_find_prev_word_optional_comma(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_previous_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_previous_word,args,4,4);
    }
    return reloq_engb61_find_previous_word(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_find_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_root,args,3,3);
    }
    return reloq_engb61_find_root(state, p0, p1, p2);
}
int32_t reloq_engb61_find_unpuncted_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_find_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_unpuncted_phrases,args,3,3);
    }
    return reloq_engb61_find_unpuncted_phrases(state, p0, p1, p2);
}
int32_t reloq_engb61_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_engb61_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_first_postnucl_accent,args,1,1);
    }
    return reloq_engb61_first_postnucl_accent(state, p0);
}
int32_t reloq_engb61_first_root_vow(void *, int32_t, int32_t);
int32_t reloq_engb61_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_followed_by_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_followed_by_hyphen,args,3,3);
    }
    return reloq_engb61_followed_by_hyphen(state, p0, p1, p2);
}
int32_t reloq_engb61_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_engb61_forced_synthesis(state, p0, p1);
}
int32_t reloq_engb61_g_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_g_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_g_rules,args,1,1);
    }
    return reloq_engb61_g_rules(state, p0);
}
int32_t reloq_engb61_ga_ph_l(void *, int32_t);
static inline int32_t reloq_call_engb61_ga_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ga_ph_l,args,1,1);
    }
    return reloq_engb61_ga_ph_l(state, p0);
}
int32_t reloq_engb61_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_engb61_generate_acoustic_vals(state, p0);
}
int32_t reloq_engb61_generate_diaphones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_generate_diaphones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_generate_diaphones,args,3,3);
    }
    return reloq_engb61_generate_diaphones(state, p0, p1, p2);
}
int32_t reloq_engb61_generate_durations(void *, int32_t);
static inline int32_t reloq_call_engb61_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_engb61_generate_durations(state, p0);
}
int32_t reloq_engb61_generate_intonation(void *, int32_t);
static inline int32_t reloq_call_engb61_generate_intonation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_intonation,args,1,1);
    }
    return reloq_engb61_generate_intonation(state, p0);
}
int32_t reloq_engb61_get_UK_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_engb61_get_UK_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_UK_nuc_AV,args,1,1);
    }
    return reloq_engb61_get_UK_nuc_AV(state, p0);
}
int32_t reloq_engb61_get_currency_type(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_currency_type(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_currency_type,args,6,6);
    }
    return reloq_engb61_get_currency_type(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_get_dollar_type(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_dollar_type(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_dollar_type,args,6,6);
    }
    return reloq_engb61_get_dollar_type(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_get_eng_fric_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_eng_fric_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_eng_fric_trans_dur,args,2,2);
    }
    return reloq_engb61_get_eng_fric_trans_dur(state, p0, p1);
}
int32_t reloq_engb61_get_eng_nas_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_eng_nas_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_eng_nas_trans_dur,args,2,2);
    }
    return reloq_engb61_get_eng_nas_trans_dur(state, p0, p1);
}
int32_t reloq_engb61_get_eng_son_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_eng_son_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_eng_son_trans_dur,args,2,2);
    }
    return reloq_engb61_get_eng_son_trans_dur(state, p0, p1);
}
int32_t reloq_engb61_get_eng_stop_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_eng_stop_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_eng_stop_trans_dur,args,2,2);
    }
    return reloq_engb61_get_eng_stop_trans_dur(state, p0, p1);
}
int32_t reloq_engb61_get_eng_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_eng_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_eng_trans_dur,args,2,2);
    }
    return reloq_engb61_get_eng_trans_dur(state, p0, p1);
}
int32_t reloq_engb61_get_eng_vow_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_eng_vow_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_eng_vow_trans_dur,args,2,2);
    }
    return reloq_engb61_get_eng_vow_trans_dur(state, p0, p1);
}
int32_t reloq_engb61_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_engb61_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_engb61_get_h_dur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_h_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_h_dur,args,4,4);
    }
    return reloq_engb61_get_h_dur(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_engb61_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_get_input(void *, int32_t);
static inline int32_t reloq_call_engb61_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_engb61_get_input(state, p0);
}
int32_t reloq_engb61_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_engb61_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_engb61_get_nuc_AV(state, p0);
}
int32_t reloq_engb61_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_engb61_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_numRootSylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_numRootSylls,args,5,5);
    }
    return reloq_engb61_get_numRootSylls(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_get_number_info(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_number_info(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_info,args,3,3);
    }
    return reloq_engb61_get_number_info(state, p0, p1, p2);
}
int32_t reloq_engb61_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_engb61_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_engb61_get_root_POS_from_morph(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_root_POS_from_morph(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_root_POS_from_morph,args,6,6);
    }
    return reloq_engb61_get_root_POS_from_morph(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_get_tok(void *, int32_t);
static inline int32_t reloq_call_engb61_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_engb61_get_tok(state, p0);
}
int32_t reloq_engb61_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_engb61_get_trans_dur(state, p0, p1);
}
int32_t reloq_engb61_gettok(void *, int32_t);
static inline int32_t reloq_call_engb61_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_engb61_gettok(state, p0);
}
int32_t reloq_engb61_gramm_cat_dict_lookup(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_gramm_cat_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_gramm_cat_dict_lookup,args,5,5);
    }
    return reloq_engb61_gramm_cat_dict_lookup(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_h_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_h_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_h_rules,args,1,1);
    }
    return reloq_engb61_h_rules(state, p0);
}
int32_t reloq_engb61_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_engb61_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_handle_UK_A(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_UK_A(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_UK_A,args,1,1);
    }
    return reloq_engb61_handle_UK_A(state, p0);
}
int32_t reloq_engb61_handle_about(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_about(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_about,args,2,2);
    }
    return reloq_engb61_handle_about(state, p0, p1);
}
int32_t reloq_engb61_handle_all_ones(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_all_ones(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_all_ones,args,8,8);
    }
    return reloq_engb61_handle_all_ones(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_engb61_handle_always_and_never(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_always_and_never(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_always_and_never,args,1,1);
    }
    return reloq_engb61_handle_always_and_never(state, p0);
}
int32_t reloq_engb61_handle_as_is(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_as_is(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_as_is,args,1,1);
    }
    return reloq_engb61_handle_as_is(state, p0);
}
int32_t reloq_engb61_handle_at_least(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_at_least(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_at_least,args,1,1);
    }
    return reloq_engb61_handle_at_least(state, p0);
}
int32_t reloq_engb61_handle_aux_aux(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_aux_aux(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_aux_aux,args,1,1);
    }
    return reloq_engb61_handle_aux_aux(state, p0);
}
int32_t reloq_engb61_handle_better_than(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_better_than(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_better_than,args,2,2);
    }
    return reloq_engb61_handle_better_than(state, p0, p1);
}
int32_t reloq_engb61_handle_by(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_by(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_by,args,1,1);
    }
    return reloq_engb61_handle_by(state, p0);
}
int32_t reloq_engb61_handle_dem(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_dem(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_dem,args,2,2);
    }
    return reloq_engb61_handle_dem(state, p0, p1);
}
int32_t reloq_engb61_handle_do_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_do_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_do_words,args,3,3);
    }
    return reloq_engb61_handle_do_words(state, p0, p1, p2);
}
int32_t reloq_engb61_handle_dont_feel_well(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_dont_feel_well(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_dont_feel_well,args,1,1);
    }
    return reloq_engb61_handle_dont_feel_well(state, p0);
}
int32_t reloq_engb61_handle_each_other(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_each_other(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_each_other,args,1,1);
    }
    return reloq_engb61_handle_each_other(state, p0);
}
int32_t reloq_engb61_handle_either_or(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_either_or(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_either_or,args,1,1);
    }
    return reloq_engb61_handle_either_or(state, p0);
}
int32_t reloq_engb61_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_engb61_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_engb61_handle_get(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_get(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_get,args,1,1);
    }
    return reloq_engb61_handle_get(state, p0);
}
int32_t reloq_engb61_handle_have_been(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_have_been(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_have_been,args,3,3);
    }
    return reloq_engb61_handle_have_been(state, p0, p1, p2);
}
int32_t reloq_engb61_handle_here_and_there(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_here_and_there(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_here_and_there,args,2,2);
    }
    return reloq_engb61_handle_here_and_there(state, p0, p1);
}
int32_t reloq_engb61_handle_in(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_in(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_in,args,1,1);
    }
    return reloq_engb61_handle_in(state, p0);
}
int32_t reloq_engb61_handle_into_and_onto(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_into_and_onto(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_into_and_onto,args,1,1);
    }
    return reloq_engb61_handle_into_and_onto(state, p0);
}
int32_t reloq_engb61_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_engb61_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_engb61_handle_is_comma(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_is_comma(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_is_comma,args,3,3);
    }
    return reloq_engb61_handle_is_comma(state, p0, p1, p2);
}
int32_t reloq_engb61_handle_just(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_just(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_just,args,1,1);
    }
    return reloq_engb61_handle_just(state, p0);
}
int32_t reloq_engb61_handle_like(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_like(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_like,args,2,2);
    }
    return reloq_engb61_handle_like(state, p0, p1);
}
int32_t reloq_engb61_handle_like_this(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_like_this(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_like_this,args,1,1);
    }
    return reloq_engb61_handle_like_this(state, p0);
}
int32_t reloq_engb61_handle_math_words(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_math_words(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_math_words,args,1,1);
    }
    return reloq_engb61_handle_math_words(state, p0);
}
int32_t reloq_engb61_handle_matter(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_matter(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_matter,args,1,1);
    }
    return reloq_engb61_handle_matter(state, p0);
}
int32_t reloq_engb61_handle_medial_currency_type(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_medial_currency_type(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_medial_currency_type,args,4,4);
    }
    return reloq_engb61_handle_medial_currency_type(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_handle_modal_negat(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_modal_negat(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_modal_negat,args,2,2);
    }
    return reloq_engb61_handle_modal_negat(state, p0, p1);
}
int32_t reloq_engb61_handle_modal_prep(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_modal_prep(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_modal_prep,args,1,1);
    }
    return reloq_engb61_handle_modal_prep(state, p0);
}
int32_t reloq_engb61_handle_named(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_named(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_named,args,1,1);
    }
    return reloq_engb61_handle_named(state, p0);
}
int32_t reloq_engb61_handle_non_phrasal_commas(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_non_phrasal_commas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_phrasal_commas,args,1,1);
    }
    return reloq_engb61_handle_non_phrasal_commas(state, p0);
}
int32_t reloq_engb61_handle_non_reduction_of_preps(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_non_reduction_of_preps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_reduction_of_preps,args,5,5);
    }
    return reloq_engb61_handle_non_reduction_of_preps(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_handle_not_me(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_not_me(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_not_me,args,1,1);
    }
    return reloq_engb61_handle_not_me(state, p0);
}
int32_t reloq_engb61_handle_not_this(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_not_this(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_not_this,args,1,1);
    }
    return reloq_engb61_handle_not_this(state, p0);
}
int32_t reloq_engb61_handle_now_and_then(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_now_and_then(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_now_and_then,args,3,3);
    }
    return reloq_engb61_handle_now_and_then(state, p0, p1, p2);
}
int32_t reloq_engb61_handle_of_mine(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_of_mine(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_of_mine,args,1,1);
    }
    return reloq_engb61_handle_of_mine(state, p0);
}
int32_t reloq_engb61_handle_on(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_on(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_on,args,2,2);
    }
    return reloq_engb61_handle_on(state, p0, p1);
}
int32_t reloq_engb61_handle_only(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_only(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_only,args,1,1);
    }
    return reloq_engb61_handle_only(state, p0);
}
int32_t reloq_engb61_handle_out(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_out(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_out,args,1,1);
    }
    return reloq_engb61_handle_out(state, p0);
}
int32_t reloq_engb61_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_engb61_handle_pause(state, p0, p1);
}
int32_t reloq_engb61_handle_phr_final_be(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_phr_final_be(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_phr_final_be,args,1,1);
    }
    return reloq_engb61_handle_phr_final_be(state, p0);
}
int32_t reloq_engb61_handle_phr_final_infinitive(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_phr_final_infinitive(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_phr_final_infinitive,args,1,1);
    }
    return reloq_engb61_handle_phr_final_infinitive(state, p0);
}
int32_t reloq_engb61_handle_phr_final_modal(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_phr_final_modal(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_phr_final_modal,args,2,2);
    }
    return reloq_engb61_handle_phr_final_modal(state, p0, p1);
}
int32_t reloq_engb61_handle_phr_final_pronoun(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_phr_final_pronoun(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_phr_final_pronoun,args,2,2);
    }
    return reloq_engb61_handle_phr_final_pronoun(state, p0, p1);
}
int32_t reloq_engb61_handle_phr_final_pronoun_be(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_phr_final_pronoun_be(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_phr_final_pronoun_be,args,3,3);
    }
    return reloq_engb61_handle_phr_final_pronoun_be(state, p0, p1, p2);
}
int32_t reloq_engb61_handle_phr_initial_funct_wds(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_phr_initial_funct_wds(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_phr_initial_funct_wds,args,2,2);
    }
    return reloq_engb61_handle_phr_initial_funct_wds(state, p0, p1);
}
int32_t reloq_engb61_handle_phr_initial_wh(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_phr_initial_wh(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_phr_initial_wh,args,2,2);
    }
    return reloq_engb61_handle_phr_initial_wh(state, p0, p1);
}
int32_t reloq_engb61_handle_please(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_please(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_please,args,1,1);
    }
    return reloq_engb61_handle_please(state, p0);
}
int32_t reloq_engb61_handle_postposed_currency(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_postposed_currency(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_postposed_currency,args,5,5);
    }
    return reloq_engb61_handle_postposed_currency(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_handle_prep_prep(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_prep_prep(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_prep_prep,args,1,1);
    }
    return reloq_engb61_handle_prep_prep(state, p0);
}
int32_t reloq_engb61_handle_pro2_cases(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_pro2_cases(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pro2_cases,args,2,2);
    }
    return reloq_engb61_handle_pro2_cases(state, p0, p1);
}
int32_t reloq_engb61_handle_quant(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_quant(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_quant,args,3,3);
    }
    return reloq_engb61_handle_quant(state, p0, p1, p2);
}
int32_t reloq_engb61_handle_reflexive_pronouns(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_reflexive_pronouns(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_reflexive_pronouns,args,7,7);
    }
    return reloq_engb61_handle_reflexive_pronouns(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_engb61_handle_sg_one(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_sg_one(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_sg_one,args,4,4);
    }
    return reloq_engb61_handle_sg_one(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_handle_short_verbs(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_short_verbs(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_short_verbs,args,2,2);
    }
    return reloq_engb61_handle_short_verbs(state, p0, p1);
}
int32_t reloq_engb61_handle_since(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_since(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_since,args,1,1);
    }
    return reloq_engb61_handle_since(state, p0);
}
int32_t reloq_engb61_handle_so_and_too(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_so_and_too(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_so_and_too,args,1,1);
    }
    return reloq_engb61_handle_so_and_too(state, p0);
}
int32_t reloq_engb61_handle_so_forth(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_so_forth(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_so_forth,args,1,1);
    }
    return reloq_engb61_handle_so_forth(state, p0);
}
int32_t reloq_engb61_handle_stressed_pronouns(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_stressed_pronouns(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_stressed_pronouns,args,1,1);
    }
    return reloq_engb61_handle_stressed_pronouns(state, p0);
}
int32_t reloq_engb61_handle_tag_questions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_tag_questions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_tag_questions,args,3,3);
    }
    return reloq_engb61_handle_tag_questions(state, p0, p1, p2);
}
int32_t reloq_engb61_handle_that_way(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_that_way(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_that_way,args,1,1);
    }
    return reloq_engb61_handle_that_way(state, p0);
}
int32_t reloq_engb61_handle_things(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_things(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_things,args,2,2);
    }
    return reloq_engb61_handle_things(state, p0, p1);
}
int32_t reloq_engb61_handle_this_and_that(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_this_and_that(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_this_and_that,args,1,1);
    }
    return reloq_engb61_handle_this_and_that(state, p0);
}
int32_t reloq_engb61_handle_this_one(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_this_one(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_this_one,args,1,1);
    }
    return reloq_engb61_handle_this_one(state, p0);
}
int32_t reloq_engb61_handle_this_time(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_this_time(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_this_time,args,1,1);
    }
    return reloq_engb61_handle_this_time(state, p0);
}
int32_t reloq_engb61_handle_time_adverbials(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_time_adverbials(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_time_adverbials,args,2,2);
    }
    return reloq_engb61_handle_time_adverbials(state, p0, p1);
}
int32_t reloq_engb61_handle_titles(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_titles(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_titles,args,1,1);
    }
    return reloq_engb61_handle_titles(state, p0);
}
int32_t reloq_engb61_handle_up_and_down(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_up_and_down(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_up_and_down,args,1,1);
    }
    return reloq_engb61_handle_up_and_down(state, p0);
}
int32_t reloq_engb61_handle_wait_a_minute(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_wait_a_minute(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_wait_a_minute,args,1,1);
    }
    return reloq_engb61_handle_wait_a_minute(state, p0);
}
int32_t reloq_engb61_handle_wh_phr_final_aux(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_handle_wh_phr_final_aux(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_wh_phr_final_aux,args,2,2);
    }
    return reloq_engb61_handle_wh_phr_final_aux(state, p0, p1);
}
int32_t reloq_engb61_handle_with(void *, int32_t);
static inline int32_t reloq_call_engb61_handle_with(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_with,args,1,1);
    }
    return reloq_engb61_handle_with(state, p0);
}
int32_t reloq_engb61_has_lex_prefix(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_has_lex_prefix(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_has_lex_prefix,args,3,3);
    }
    return reloq_engb61_has_lex_prefix(state, p0, p1, p2);
}
int32_t reloq_engb61_hav_vs_modal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_hav_vs_modal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hav_vs_modal,args,3,3);
    }
    return reloq_engb61_hav_vs_modal(state, p0, p1, p2);
}
int32_t reloq_engb61_have_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_have_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_have_words,args,3,3);
    }
    return reloq_engb61_have_words(state, p0, p1, p2);
}
int32_t reloq_engb61_heavy_syllable(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_heavy_syllable(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_heavy_syllable,args,2,2);
    }
    return reloq_engb61_heavy_syllable(state, p0, p1);
}
int32_t reloq_engb61_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_low_star_tone,args,1,1);
    }
    return reloq_engb61_high_low_star_tone(state, p0);
}
int32_t reloq_engb61_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_star_low_tone,args,1,1);
    }
    return reloq_engb61_high_star_low_tone(state, p0);
}
int32_t reloq_engb61_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_tone,args,1,1);
    }
    return reloq_engb61_high_tone(state, p0);
}
int32_t reloq_engb61_homog_roots(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_homog_roots(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_homog_roots,args,5,5);
    }
    return reloq_engb61_homog_roots(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen,args,3,3);
    }
    return reloq_engb61_hyphen(state, p0, p1, p2);
}
int32_t reloq_engb61_hyphen_between_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen_between_nums,args,3,3);
    }
    return reloq_engb61_hyphen_between_nums(state, p0, p1, p2);
}
int32_t reloq_engb61_hyphenated_prefix(void *, int32_t, int32_t);
int32_t reloq_engb61_i_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_i_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_i_rules,args,1,1);
    }
    return reloq_engb61_i_rules(state, p0);
}
int32_t reloq_engb61_i_to_y(void *, int32_t);
static inline int32_t reloq_call_engb61_i_to_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_i_to_y,args,1,1);
    }
    return reloq_engb61_i_to_y(state, p0);
}
int32_t reloq_engb61_identify_eng_year(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_identify_eng_year(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_identify_eng_year,args,3,3);
    }
    return reloq_engb61_identify_eng_year(state, p0, p1, p2);
}
int32_t reloq_engb61_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_engb61_in_brackets(state, p0, p1, p2);
}
int32_t reloq_engb61_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_engb61_in_quotes(state, p0, p1, p2);
}
int32_t reloq_engb61_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_in_syllable(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_syllable,args,4,4);
    }
    return reloq_engb61_in_syllable(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_engb61_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_engb61_increment_output_file_number(state, p0);
}
int32_t reloq_engb61_inf_vs_prep(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_inf_vs_prep(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_inf_vs_prep,args,3,3);
    }
    return reloq_engb61_inf_vs_prep(state, p0, p1, p2);
}
int32_t reloq_engb61_init_adjacent_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_engb61_init_adjacent_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_adjacent_word_ptrs,args,1,1);
    }
    return reloq_engb61_init_adjacent_word_ptrs(state, p0);
}
int32_t reloq_engb61_init_current_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_engb61_init_current_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_current_word_ptrs,args,1,1);
    }
    return reloq_engb61_init_current_word_ptrs(state, p0);
}
int32_t reloq_engb61_init_delta(void *, int32_t);
static inline int32_t reloq_call_engb61_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_engb61_init_delta(state, p0);
}
int32_t reloq_engb61_init_eng_morph_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_eng_morph_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_eng_morph_vars,args,1,1);
    }
    return reloq_engb61_init_eng_morph_vars(state, p0);
}
int32_t reloq_engb61_init_eng_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_eng_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_eng_symbolic_vars,args,1,1);
    }
    return reloq_engb61_init_eng_symbolic_vars(state, p0);
}
int32_t reloq_engb61_init_eng_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_eng_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_eng_vars,args,1,1);
    }
    return reloq_engb61_init_eng_vars(state, p0);
}
int32_t reloq_engb61_init_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_gcat_vars,args,1,1);
    }
    return reloq_engb61_init_gcat_vars(state, p0);
}
int32_t reloq_engb61_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_engb61_init_lang_vars(state, p0);
}
int32_t reloq_engb61_init_nuclwrd_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_nuclwrd_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_nuclwrd_vars,args,1,1);
    }
    return reloq_engb61_init_nuclwrd_vars(state, p0);
}
int32_t reloq_engb61_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_engb61_init_phr_feature_vars(state, p0);
}
int32_t reloq_engb61_init_phrase_inton_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_phrase_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_inton_vars,args,1,1);
    }
    return reloq_engb61_init_phrase_inton_vars(state, p0);
}
int32_t reloq_engb61_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_engb61_init_phrase_vars(state, p0);
}
int32_t reloq_engb61_init_platform(void *, int32_t);
static inline int32_t reloq_call_engb61_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_engb61_init_platform(state, p0);
}
int32_t reloq_engb61_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_engb61_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_engb61_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_engb61_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_engb61_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_inton_vars,args,1,1);
    }
    return reloq_engb61_init_syll_inton_vars(state, p0);
}
int32_t reloq_engb61_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_engb61_init_syll_vars(state, p0);
}
int32_t reloq_engb61_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_engb61_init_symbolic_vars(state, p0);
}
int32_t reloq_engb61_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_engb61_init_tm_vars(state, p0);
}
int32_t reloq_engb61_init_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_engb61_init_vars(state, p0);
}
int32_t reloq_engb61_init_voice_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_voice_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_voice_vars,args,1,1);
    }
    return reloq_engb61_init_voice_vars(state, p0);
}
int32_t reloq_engb61_init_word(void *, int32_t);
static inline int32_t reloq_call_engb61_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_engb61_init_word(state, p0);
}
int32_t reloq_engb61_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_inton_vars,args,1,1);
    }
    return reloq_engb61_init_word_inton_vars(state, p0);
}
int32_t reloq_engb61_init_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_engb61_init_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_ptrs,args,1,1);
    }
    return reloq_engb61_init_word_ptrs(state, p0);
}
int32_t reloq_engb61_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_engb61_init_word_vars(state, p0);
}
int32_t reloq_engb61_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_engb61_insert_acoustic_vals(state, p0);
}
int32_t reloq_engb61_insert_adj_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_adj_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_adj_stress,args,3,3);
    }
    return reloq_engb61_insert_adj_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_engb61_insert_burst_dur(state, p0);
}
int32_t reloq_engb61_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_engb61_insert_char_count(state, p0);
}
int32_t reloq_engb61_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_comma_phrase,args,3,3);
    }
    return reloq_engb61_insert_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_compound(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_compound(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_compound,args,2,2);
    }
    return reloq_engb61_insert_compound(state, p0, p1);
}
int32_t reloq_engb61_insert_dict_root(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_dict_root(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dict_root,args,4,4);
    }
    return reloq_engb61_insert_dict_root(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_engb61_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_insert_e(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_e,args,1,1);
    }
    return reloq_engb61_insert_e(state, p0);
}
int32_t reloq_engb61_insert_eng_large_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_eng_large_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_eng_large_number,args,3,3);
    }
    return reloq_engb61_insert_eng_large_number(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_eng_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_eng_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_eng_spr_phone,args,3,3);
    }
    return reloq_engb61_insert_eng_spr_phone(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_eng_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_eng_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_eng_stress,args,3,3);
    }
    return reloq_engb61_insert_eng_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_f0(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_f0(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_f0,args,4,4);
    }
    return reloq_engb61_insert_f0(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_insert_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_insert_lang_large_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_lang_large_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_lang_large_number,args,3,3);
    }
    return reloq_engb61_insert_lang_large_number(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_engb61_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_nationality(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_nationality(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_nationality,args,4,4);
    }
    return reloq_engb61_insert_nationality(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_non_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_non_phrase_pause,args,3,3);
    }
    return reloq_engb61_insert_non_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_engb61_insert_pause_durs(state, p0);
}
int32_t reloq_engb61_insert_penult_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_engb61_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_final_tones,args,1,1);
    }
    return reloq_engb61_insert_phrase_final_tones(state, p0);
}
int32_t reloq_engb61_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_engb61_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_point_digits(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_point_digits,args,4,4);
    }
    return reloq_engb61_insert_point_digits(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_insert_prefix(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_prefix(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_prefix,args,4,4);
    }
    return reloq_engb61_insert_prefix(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_insert_preset_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_preset_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_preset_phrase_tone,args,1,1);
    }
    return reloq_engb61_insert_preset_phrase_tone(state, p0);
}
int32_t reloq_engb61_insert_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_root,args,3,3);
    }
    return reloq_engb61_insert_root(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_insert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_single_digit,args,3,3);
    }
    return reloq_engb61_insert_single_digit(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_engb61_insert_space(state, p0, p1);
}
int32_t reloq_engb61_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_engb61_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_engb61_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_insert_stress_pattern(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_stress_pattern(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress_pattern,args,3,3);
    }
    return reloq_engb61_insert_stress_pattern(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_stressval(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_stressval(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stressval,args,4,4);
    }
    return reloq_engb61_insert_stressval(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_insert_suffix(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_suffix(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_suffix,args,2,2);
    }
    return reloq_engb61_insert_suffix(state, p0, p1);
}
int32_t reloq_engb61_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_engb61_insert_syll_level_vals(state, p0);
}
int32_t reloq_engb61_insert_target_t3(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_target_t3(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_target_t3,args,1,1);
    }
    return reloq_engb61_insert_target_t3(state, p0);
}
int32_t reloq_engb61_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_engb61_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_engb61_insert_transitions(state, p0);
}
int32_t reloq_engb61_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_engb61_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrases,args,4,4);
    }
    return reloq_engb61_insert_unpuncted_phrases(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_engb61_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_engb61_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_engb61_insert_word(state, p0, p1, p2);
}
int32_t reloq_engb61_insert_zero_sylls(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_inside_orig_inp_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_inside_orig_inp_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_inside_orig_inp_token,args,2,2);
    }
    return reloq_engb61_inside_orig_inp_token(state, p0, p1);
}
int32_t reloq_engb61_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_engb61_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_engb61_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_interpret_single_char_modes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_interpret_single_char_modes,args,3,3);
    }
    return reloq_engb61_interpret_single_char_modes(state, p0, p1, p2);
}
int32_t reloq_engb61_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_engb61_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_engb61_is_adj_from_context(void *, int32_t);
static inline int32_t reloq_call_engb61_is_adj_from_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_adj_from_context,args,1,1);
    }
    return reloq_engb61_is_adj_from_context(state, p0);
}
int32_t reloq_engb61_is_after_h(void *, int32_t, int32_t);
int32_t reloq_engb61_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_engb61_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_engb61_is_clause_indicator(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_clause_indicator(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_clause_indicator,args,3,3);
    }
    return reloq_engb61_is_clause_indicator(state, p0, p1, p2);
}
int32_t reloq_engb61_is_comparative_adj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_comparative_adj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_comparative_adj,args,3,3);
    }
    return reloq_engb61_is_comparative_adj(state, p0, p1, p2);
}
int32_t reloq_engb61_is_copula_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_copula_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_copula_verb,args,3,3);
    }
    return reloq_engb61_is_copula_verb(state, p0, p1, p2);
}
int32_t reloq_engb61_is_day_of_week_abbr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_day_of_week_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_day_of_week_abbr,args,5,5);
    }
    return reloq_engb61_is_day_of_week_abbr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_is_direction_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_direction_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_direction_abbr,args,4,4);
    }
    return reloq_engb61_is_direction_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_is_hyphenated_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_hyphenated_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_hyphenated_word,args,2,2);
    }
    return reloq_engb61_is_hyphenated_word(state, p0, p1);
}
int32_t reloq_engb61_is_in_parentheses(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_is_measure_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_measure_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_measure_abbr,args,3,3);
    }
    return reloq_engb61_is_measure_abbr(state, p0, p1, p2);
}
int32_t reloq_engb61_is_minor_phrase_break(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_minor_phrase_break(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_minor_phrase_break,args,3,3);
    }
    return reloq_engb61_is_minor_phrase_break(state, p0, p1, p2);
}
int32_t reloq_engb61_is_month_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_month_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_month_abbr,args,4,4);
    }
    return reloq_engb61_is_month_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_is_noun(void *, int32_t);
static inline int32_t reloq_call_engb61_is_noun(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_noun,args,1,1);
    }
    return reloq_engb61_is_noun(state, p0);
}
int32_t reloq_engb61_is_noun_adj_from_context(void *, int32_t);
static inline int32_t reloq_call_engb61_is_noun_adj_from_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_noun_adj_from_context,args,1,1);
    }
    return reloq_engb61_is_noun_adj_from_context(state, p0);
}
int32_t reloq_engb61_is_noun_contraction(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_noun_contraction(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_noun_contraction,args,5,5);
    }
    return reloq_engb61_is_noun_contraction(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_is_noun_from_context(void *, int32_t);
static inline int32_t reloq_call_engb61_is_noun_from_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_noun_from_context,args,1,1);
    }
    return reloq_engb61_is_noun_from_context(state, p0);
}
int32_t reloq_engb61_is_noun_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_noun_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_noun_phrase,args,3,3);
    }
    return reloq_engb61_is_noun_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_is_noun_verb_from_context(void *, int32_t);
static inline int32_t reloq_call_engb61_is_noun_verb_from_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_noun_verb_from_context,args,1,1);
    }
    return reloq_engb61_is_noun_verb_from_context(state, p0);
}
int32_t reloq_engb61_is_pathname(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_pathname(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pathname,args,2,2);
    }
    return reloq_engb61_is_pathname(state, p0, p1);
}
int32_t reloq_engb61_is_pick_up_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_pick_up_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pick_up_word,args,3,3);
    }
    return reloq_engb61_is_pick_up_word(state, p0, p1, p2);
}
int32_t reloq_engb61_is_plural_noun(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_plural_noun(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_plural_noun,args,3,3);
    }
    return reloq_engb61_is_plural_noun(state, p0, p1, p2);
}
int32_t reloq_engb61_is_plural_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_plural_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_plural_verb,args,3,3);
    }
    return reloq_engb61_is_plural_verb(state, p0, p1, p2);
}
int32_t reloq_engb61_is_preceded_by_adj(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_preceded_by_adj(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_preceded_by_adj,args,2,2);
    }
    return reloq_engb61_is_preceded_by_adj(state, p0, p1);
}
int32_t reloq_engb61_is_prenominal_modifier(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_prenominal_modifier(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_prenominal_modifier,args,3,3);
    }
    return reloq_engb61_is_prenominal_modifier(state, p0, p1, p2);
}
int32_t reloq_engb61_is_prep_phrase(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_is_proper_name(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_proper_name(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_proper_name,args,2,2);
    }
    return reloq_engb61_is_proper_name(state, p0, p1);
}
int32_t reloq_engb61_is_sg_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_sg_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_sg_abbr,args,2,2);
    }
    return reloq_engb61_is_sg_abbr(state, p0, p1);
}
int32_t reloq_engb61_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_engb61_is_spr(state, p0, p1, p2);
}
int32_t reloq_engb61_is_state_abbr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_state_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_state_abbr,args,5,5);
    }
    return reloq_engb61_is_state_abbr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_is_superlative_adj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_superlative_adj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_superlative_adj,args,3,3);
    }
    return reloq_engb61_is_superlative_adj(state, p0, p1, p2);
}
int32_t reloq_engb61_is_target_syll(void *, int32_t);
static inline int32_t reloq_call_engb61_is_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_target_syll,args,1,1);
    }
    return reloq_engb61_is_target_syll(state, p0);
}
int32_t reloq_engb61_is_title_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_title_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_title_abbr,args,2,2);
    }
    return reloq_engb61_is_title_abbr(state, p0, p1);
}
int32_t reloq_engb61_is_valid_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_valid_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_valid_date,args,3,3);
    }
    return reloq_engb61_is_valid_date(state, p0, p1, p2);
}
int32_t reloq_engb61_is_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_is_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_verb,args,3,3);
    }
    return reloq_engb61_is_verb(state, p0, p1, p2);
}
int32_t reloq_engb61_is_verb_adj_from_context(void *, int32_t);
static inline int32_t reloq_call_engb61_is_verb_adj_from_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_verb_adj_from_context,args,1,1);
    }
    return reloq_engb61_is_verb_adj_from_context(state, p0);
}
int32_t reloq_engb61_is_verb_from_context(void *, int32_t);
static inline int32_t reloq_call_engb61_is_verb_from_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_verb_from_context,args,1,1);
    }
    return reloq_engb61_is_verb_from_context(state, p0);
}
int32_t reloq_engb61_is_verb_past_participle(void *, int32_t);
static inline int32_t reloq_call_engb61_is_verb_past_participle(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_verb_past_participle,args,1,1);
    }
    return reloq_engb61_is_verb_past_participle(state, p0);
}
int32_t reloq_engb61_ise_to_ize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_ise_to_ize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ise_to_ize,args,3,3);
    }
    return reloq_engb61_ise_to_ize(state, p0, p1, p2);
}
int32_t reloq_engb61_isitAlt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isitAlt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitAlt,args,3,3);
    }
    return reloq_engb61_isitAlt(state, p0, p1, p2);
}
int32_t reloq_engb61_isitWH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isitWH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitWH,args,3,3);
    }
    return reloq_engb61_isitWH(state, p0, p1, p2);
}
int32_t reloq_engb61_isit_eng_Alt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isit_eng_Alt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_eng_Alt,args,3,3);
    }
    return reloq_engb61_isit_eng_Alt(state, p0, p1, p2);
}
int32_t reloq_engb61_isit_eng_WH(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isit_eng_WH(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_eng_WH,args,2,2);
    }
    return reloq_engb61_isit_eng_WH(state, p0, p1);
}
int32_t reloq_engb61_isit_num_expr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isit_num_expr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_num_expr,args,3,3);
    }
    return reloq_engb61_isit_num_expr(state, p0, p1, p2);
}
int32_t reloq_engb61_isit_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isit_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_number,args,3,3);
    }
    return reloq_engb61_isit_number(state, p0, p1, p2);
}
int32_t reloq_engb61_isolate_e_root(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isolate_e_root(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isolate_e_root,args,5,5);
    }
    return reloq_engb61_isolate_e_root(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_isolate_es_root(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isolate_es_root(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isolate_es_root,args,5,5);
    }
    return reloq_engb61_isolate_es_root(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_isolate_non_plural_root(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isolate_non_plural_root(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isolate_non_plural_root,args,4,4);
    }
    return reloq_engb61_isolate_non_plural_root(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_isolate_z_root(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_isolate_z_root(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isolate_z_root,args,7,7);
    }
    return reloq_engb61_isolate_z_root(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_engb61_j_as_spanish_h(void *, int32_t);
static inline int32_t reloq_call_engb61_j_as_spanish_h(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_j_as_spanish_h,args,1,1);
    }
    return reloq_engb61_j_as_spanish_h(state, p0);
}
int32_t reloq_engb61_j_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_j_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_j_rules,args,1,1);
    }
    return reloq_engb61_j_rules(state, p0);
}
int32_t reloq_engb61_k_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_k_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_k_rules,args,1,1);
    }
    return reloq_engb61_k_rules(state, p0);
}
int32_t reloq_engb61_l_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_l_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_l_rules,args,1,1);
    }
    return reloq_engb61_l_rules(state, p0);
}
int32_t reloq_engb61_lang_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_lang_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_hyphen_between_nums,args,5,5);
    }
    return reloq_engb61_lang_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_lang_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_lang_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_non_phrasal_comma,args,2,2);
    }
    return reloq_engb61_lang_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_engb61_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_lang_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_phrase_level_rules,args,3,3);
    }
    return reloq_engb61_lang_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_engb61_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_engb61_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_engb61_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_engb61_legal_initial_consonants(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_legal_initial_consonants(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_initial_consonants,args,2,2);
    }
    return reloq_engb61_legal_initial_consonants(state, p0, p1);
}
int32_t reloq_engb61_letter_based_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_letter_based_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_letter_based_rules,args,1,1);
    }
    return reloq_engb61_letter_based_rules(state, p0);
}
int32_t reloq_engb61_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_engb61_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_engb61_lex_prefixes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_lex_prefixes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lex_prefixes,args,3,3);
    }
    return reloq_engb61_lex_prefixes(state, p0, p1, p2);
}
int32_t reloq_engb61_longdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_longdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_longdash,args,3,3);
    }
    return reloq_engb61_longdash(state, p0, p1, p2);
}
int32_t reloq_engb61_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_high_star_tone,args,1,1);
    }
    return reloq_engb61_low_high_star_tone(state, p0);
}
int32_t reloq_engb61_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_star_high_tone,args,1,1);
    }
    return reloq_engb61_low_star_high_tone(state, p0);
}
int32_t reloq_engb61_low_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_tone,args,1,1);
    }
    return reloq_engb61_low_tone(state, p0);
}
int32_t reloq_engb61_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_m_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_m_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_m_rules,args,1,1);
    }
    return reloq_engb61_m_rules(state, p0);
}
int32_t reloq_engb61_mark_adj_by_ending(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_adj_by_ending(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_adj_by_ending,args,1,1);
    }
    return reloq_engb61_mark_adj_by_ending(state, p0);
}
int32_t reloq_engb61_mark_adj_by_rule(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_adj_by_rule(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_adj_by_rule,args,1,1);
    }
    return reloq_engb61_mark_adj_by_rule(state, p0);
}
int32_t reloq_engb61_mark_adj_from_sets(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_adj_from_sets(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_adj_from_sets,args,1,1);
    }
    return reloq_engb61_mark_adj_from_sets(state, p0);
}
int32_t reloq_engb61_mark_adverb_by_ending(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_adverb_by_ending(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_adverb_by_ending,args,1,1);
    }
    return reloq_engb61_mark_adverb_by_ending(state, p0);
}
int32_t reloq_engb61_mark_cliticized_word_category(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_cliticized_word_category(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_cliticized_word_category,args,1,1);
    }
    return reloq_engb61_mark_cliticized_word_category(state, p0);
}
int32_t reloq_engb61_mark_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_content_word,args,5,5);
    }
    return reloq_engb61_mark_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_mark_eng_conjunctions(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_eng_conjunctions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_eng_conjunctions,args,1,1);
    }
    return reloq_engb61_mark_eng_conjunctions(state, p0);
}
int32_t reloq_engb61_mark_eng_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_eng_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_eng_content_word,args,5,5);
    }
    return reloq_engb61_mark_eng_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_mark_eng_interjections(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_eng_interjections(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_eng_interjections,args,1,1);
    }
    return reloq_engb61_mark_eng_interjections(state, p0);
}
int32_t reloq_engb61_mark_eng_prepositions(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_eng_prepositions(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_eng_prepositions,args,2,2);
    }
    return reloq_engb61_mark_eng_prepositions(state, p0, p1);
}
int32_t reloq_engb61_mark_eng_quantifiers(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_eng_quantifiers(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_eng_quantifiers,args,1,1);
    }
    return reloq_engb61_mark_eng_quantifiers(state, p0);
}
int32_t reloq_engb61_mark_eng_refl_pronouns(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_eng_refl_pronouns(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_eng_refl_pronouns,args,1,1);
    }
    return reloq_engb61_mark_eng_refl_pronouns(state, p0);
}
int32_t reloq_engb61_mark_eng_subcategory(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_eng_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_eng_subcategory,args,3,3);
    }
    return reloq_engb61_mark_eng_subcategory(state, p0, p1, p2);
}
int32_t reloq_engb61_mark_eng_wh_words(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_eng_wh_words(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_eng_wh_words,args,2,2);
    }
    return reloq_engb61_mark_eng_wh_words(state, p0, p1);
}
int32_t reloq_engb61_mark_function_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_function_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_function_word,args,5,5);
    }
    return reloq_engb61_mark_function_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_mark_h(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_h(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_h,args,3,3);
    }
    return reloq_engb61_mark_h(state, p0, p1, p2);
}
int32_t reloq_engb61_mark_non_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_non_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_non_letter,args,3,3);
    }
    return reloq_engb61_mark_non_letter(state, p0, p1, p2);
}
int32_t reloq_engb61_mark_noun_adj_by_ending(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_noun_adj_by_ending(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_noun_adj_by_ending,args,1,1);
    }
    return reloq_engb61_mark_noun_adj_by_ending(state, p0);
}
int32_t reloq_engb61_mark_noun_adj_from_sets(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_noun_adj_from_sets(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_noun_adj_from_sets,args,1,1);
    }
    return reloq_engb61_mark_noun_adj_from_sets(state, p0);
}
int32_t reloq_engb61_mark_noun_by_ending(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_noun_by_ending(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_noun_by_ending,args,1,1);
    }
    return reloq_engb61_mark_noun_by_ending(state, p0);
}
int32_t reloq_engb61_mark_noun_from_sets(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_noun_from_sets(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_noun_from_sets,args,1,1);
    }
    return reloq_engb61_mark_noun_from_sets(state, p0);
}
int32_t reloq_engb61_mark_noun_verb_adj_from_sets(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_noun_verb_adj_from_sets(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_noun_verb_adj_from_sets,args,1,1);
    }
    return reloq_engb61_mark_noun_verb_adj_from_sets(state, p0);
}
int32_t reloq_engb61_mark_noun_verb_from_sets(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_noun_verb_from_sets(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_noun_verb_from_sets,args,1,1);
    }
    return reloq_engb61_mark_noun_verb_from_sets(state, p0);
}
int32_t reloq_engb61_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_engb61_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_mark_potential_phrase_breaks(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_potential_phrase_breaks(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_potential_phrase_breaks,args,4,4);
    }
    return reloq_engb61_mark_potential_phrase_breaks(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_spr_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_spr_syll_vals,args,6,6);
    }
    return reloq_engb61_mark_spr_syll_vals(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_mark_subcategory(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_subcategory,args,4,4);
    }
    return reloq_engb61_mark_subcategory(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_mark_syll_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_syll_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_syll_stress,args,4,4);
    }
    return reloq_engb61_mark_syll_stress(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_mark_u_con(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_u_con(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_u_con,args,3,3);
    }
    return reloq_engb61_mark_u_con(state, p0, p1, p2);
}
int32_t reloq_engb61_mark_uppercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_uppercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_uppercase,args,3,3);
    }
    return reloq_engb61_mark_uppercase(state, p0, p1, p2);
}
int32_t reloq_engb61_mark_verb_adj_from_sets(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_verb_adj_from_sets(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_verb_adj_from_sets,args,1,1);
    }
    return reloq_engb61_mark_verb_adj_from_sets(state, p0);
}
int32_t reloq_engb61_mark_verb_by_ending(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_verb_by_ending(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_verb_by_ending,args,1,1);
    }
    return reloq_engb61_mark_verb_by_ending(state, p0);
}
int32_t reloq_engb61_mark_verb_from_sets(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_verb_from_sets(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_verb_from_sets,args,1,1);
    }
    return reloq_engb61_mark_verb_from_sets(state, p0);
}
int32_t reloq_engb61_mark_word_categories(void *, int32_t);
static inline int32_t reloq_call_engb61_mark_word_categories(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_categories,args,1,1);
    }
    return reloq_engb61_mark_word_categories(state, p0);
}
int32_t reloq_engb61_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_word_str_annot(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_str_annot,args,3,3);
    }
    return reloq_engb61_mark_word_str_annot(state, p0, p1, p2);
}
int32_t reloq_engb61_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_engb61_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_engb61_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_engb61_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_engb61_misc_chars(state, p0, p1, p2);
}
int32_t reloq_engb61_misc_non_phrasal_comma(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_misc_non_phrasal_comma(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_non_phrasal_comma,args,4,4);
    }
    return reloq_engb61_misc_non_phrasal_comma(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_misc_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_misc_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_misc_phrase,args,1,1);
    }
    return reloq_engb61_misc_phrase(state, p0);
}
int32_t reloq_engb61_modulate_nas_AV(void *, int32_t);
static inline int32_t reloq_call_engb61_modulate_nas_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_nas_AV,args,1,1);
    }
    return reloq_engb61_modulate_nas_AV(state, p0);
}
int32_t reloq_engb61_modulate_noise(void *, int32_t);
static inline int32_t reloq_call_engb61_modulate_noise(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_noise,args,1,1);
    }
    return reloq_engb61_modulate_noise(state, p0);
}
int32_t reloq_engb61_modulate_pwind(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_modulate_pwind(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_pwind,args,2,2);
    }
    return reloq_engb61_modulate_pwind(state, p0, p1);
}
int32_t reloq_engb61_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_engb61_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_monetary_exp,args,3,3);
    }
    return reloq_engb61_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_engb61_morph_based_rules(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_morph_based_rules(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_morph_based_rules,args,2,2);
    }
    return reloq_engb61_morph_based_rules(state, p0, p1);
}
int32_t reloq_engb61_multi_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_multi_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_multi_stress,args,3,3);
    }
    return reloq_engb61_multi_stress(state, p0, p1, p2);
}
int32_t reloq_engb61_n_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_n_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_n_rules,args,1,1);
    }
    return reloq_engb61_n_rules(state, p0);
}
int32_t reloq_engb61_nasalize_eng_vow(void *, int32_t);
static inline int32_t reloq_call_engb61_nasalize_eng_vow(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nasalize_eng_vow,args,1,1);
    }
    return reloq_engb61_nasalize_eng_vow(state, p0);
}
int32_t reloq_engb61_nasalize_vow(void *, int32_t);
int32_t reloq_engb61_neg_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_neg_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_neg_phrase,args,1,1);
    }
    return reloq_engb61_neg_phrase(state, p0);
}
int32_t reloq_engb61_no_acc_in_continuation_phr(void *, int32_t);
static inline int32_t reloq_call_engb61_no_acc_in_continuation_phr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_acc_in_continuation_phr,args,1,1);
    }
    return reloq_engb61_no_acc_in_continuation_phr(state, p0);
}
int32_t reloq_engb61_no_one_cases(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_no_one_cases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_one_cases,args,8,8);
    }
    return reloq_engb61_no_one_cases(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_engb61_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_engb61_no_readin(state, p0, p1);
}
int32_t reloq_engb61_no_reduce_vowel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_no_reduce_vowel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_reduce_vowel,args,3,3);
    }
    return reloq_engb61_no_reduce_vowel(state, p0, p1, p2);
}
int32_t reloq_engb61_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_engb61_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_engb61_no_words_in_delta(state, p0);
}
int32_t reloq_engb61_nonlex_prefixes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_nonlex_prefixes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_nonlex_prefixes,args,4,4);
    }
    return reloq_engb61_nonlex_prefixes(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_normalize_eng_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_normalize_eng_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_eng_letters,args,3,3);
    }
    return reloq_engb61_normalize_eng_letters(state, p0, p1, p2);
}
int32_t reloq_engb61_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_engb61_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_engb61_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_engb61_normalize_text(state, p0, p1, p2);
}
int32_t reloq_engb61_noun_cmpnd1(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_noun_cmpnd1(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_cmpnd1,args,6,6);
    }
    return reloq_engb61_noun_cmpnd1(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_noun_cmpnd2(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_noun_cmpnd2(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_cmpnd2,args,6,6);
    }
    return reloq_engb61_noun_cmpnd2(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_noun_vs_adj(void *, int32_t);
static inline int32_t reloq_call_engb61_noun_vs_adj(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_adj,args,1,1);
    }
    return reloq_engb61_noun_vs_adj(state, p0);
}
int32_t reloq_engb61_noun_vs_verb(void *, int32_t);
static inline int32_t reloq_call_engb61_noun_vs_verb(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_verb,args,1,1);
    }
    return reloq_engb61_noun_vs_verb(state, p0);
}
int32_t reloq_engb61_nucl_acc_t1pos(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_nucl_acc_t1pos(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_nucl_acc_t1pos,args,2,2);
    }
    return reloq_engb61_nucl_acc_t1pos(state, p0, p1);
}
int32_t reloq_engb61_nucl_f0(void *, int32_t);
static inline int32_t reloq_call_engb61_nucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_f0,args,1,1);
    }
    return reloq_engb61_nucl_f0(state, p0);
}
int32_t reloq_engb61_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_engb61_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_engb61_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_engb61_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_engb61_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_tone,args,1,1);
    }
    return reloq_engb61_nucl_high_tone(state, p0);
}
int32_t reloq_engb61_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_engb61_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_engb61_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_engb61_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_engb61_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_tone,args,1,1);
    }
    return reloq_engb61_nucl_low_tone(state, p0);
}
int32_t reloq_engb61_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_engb61_nucleus_vals(state, p0);
}
int32_t reloq_engb61_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_engb61_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_engb61_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_o_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_o_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_o_rules,args,1,1);
    }
    return reloq_engb61_o_rules(state, p0);
}
int32_t reloq_engb61_obj_vs_poss(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_obj_vs_poss(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_obj_vs_poss,args,3,3);
    }
    return reloq_engb61_obj_vs_poss(state, p0, p1, p2);
}
int32_t reloq_engb61_one_eng_syllable(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_one_eng_syllable(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_one_eng_syllable,args,3,3);
    }
    return reloq_engb61_one_eng_syllable(state, p0, p1, p2);
}
int32_t reloq_engb61_p_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_p_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_p_rules,args,1,1);
    }
    return reloq_engb61_p_rules(state, p0);
}
int32_t reloq_engb61_palatal_cons_bef_u(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_palatal_cons_bef_u(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_palatal_cons_bef_u,args,3,3);
    }
    return reloq_engb61_palatal_cons_bef_u(state, p0, p1, p2);
}
int32_t reloq_engb61_parenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_parenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_parenthesis,args,3,3);
    }
    return reloq_engb61_parenthesis(state, p0, p1, p2);
}
int32_t reloq_engb61_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_engb61_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_penult_one(void *, int32_t);
static inline int32_t reloq_call_engb61_penult_one(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_penult_one,args,1,1);
    }
    return reloq_engb61_penult_one(state, p0);
}
int32_t reloq_engb61_penult_one_two_longalt(void *, int32_t);
static inline int32_t reloq_call_engb61_penult_one_two_longalt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_penult_one_two_longalt,args,1,1);
    }
    return reloq_engb61_penult_one_two_longalt(state, p0);
}
int32_t reloq_engb61_penult_two(void *, int32_t);
static inline int32_t reloq_call_engb61_penult_two(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_penult_two,args,1,1);
    }
    return reloq_engb61_penult_two(state, p0);
}
int32_t reloq_engb61_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_engb61_period_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_pf_non_final_1stress_syll(void *, int32_t);
static inline int32_t reloq_call_engb61_pf_non_final_1stress_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_pf_non_final_1stress_syll,args,1,1);
    }
    return reloq_engb61_pf_non_final_1stress_syll(state, p0);
}
int32_t reloq_engb61_pf_word_final_syll(void *, int32_t);
static inline int32_t reloq_call_engb61_pf_word_final_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_pf_word_final_syll,args,1,1);
    }
    return reloq_engb61_pf_word_final_syll(state, p0);
}
int32_t reloq_engb61_phone_based_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_phone_based_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phone_based_rules,args,1,1);
    }
    return reloq_engb61_phone_based_rules(state, p0);
}
int32_t reloq_engb61_phone_inserted(void *, int32_t);
static inline int32_t reloq_call_engb61_phone_inserted(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phone_inserted,args,1,1);
    }
    return reloq_engb61_phone_inserted(state, p0);
}
int32_t reloq_engb61_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_engb61_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_phr_final_str_situation(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_phr_final_str_situation(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_str_situation,args,2,2);
    }
    return reloq_engb61_phr_final_str_situation(state, p0, p1);
}
int32_t reloq_engb61_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_engb61_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phr_initial_F0,args,1,1);
    }
    return reloq_engb61_phr_initial_F0(state, p0);
}
int32_t reloq_engb61_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_engb61_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_phrase_break_after_be(void *, int32_t);
static inline int32_t reloq_call_engb61_phrase_break_after_be(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_break_after_be,args,1,1);
    }
    return reloq_engb61_phrase_break_after_be(state, p0);
}
int32_t reloq_engb61_phrase_final_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_phrase_final_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_final_dur,args,1,1);
    }
    return reloq_engb61_phrase_final_dur(state, p0);
}
int32_t reloq_engb61_phrase_level_printouts(void *, int32_t);
static inline int32_t reloq_call_engb61_phrase_level_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_printouts,args,1,1);
    }
    return reloq_engb61_phrase_level_printouts(state, p0);
}
int32_t reloq_engb61_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_engb61_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_phrase_medial_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_phrase_medial_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_medial_dur,args,1,1);
    }
    return reloq_engb61_phrase_medial_dur(state, p0);
}
int32_t reloq_engb61_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_tone,args,1,1);
    }
    return reloq_engb61_phrase_tone(state, p0);
}
int32_t reloq_engb61_places(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_places(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_places,args,3,3);
    }
    return reloq_engb61_places(state, p0, p1, p2);
}
int32_t reloq_engb61_pnames(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_pnames(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pnames,args,3,3);
    }
    return reloq_engb61_pnames(state, p0, p1, p2);
}
int32_t reloq_engb61_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_engb61_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_position_t_ptrs,args,1,1);
    }
    return reloq_engb61_position_t_ptrs(state, p0);
}
int32_t reloq_engb61_poss_vs_nom(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_poss_vs_nom(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_poss_vs_nom,args,3,3);
    }
    return reloq_engb61_poss_vs_nom(state, p0, p1, p2);
}
int32_t reloq_engb61_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_comma_phrase,args,3,3);
    }
    return reloq_engb61_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_post_inflection_strip(void *, int32_t);
static inline int32_t reloq_call_engb61_post_inflection_strip(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_post_inflection_strip,args,1,1);
    }
    return reloq_engb61_post_inflection_strip(state, p0);
}
int32_t reloq_engb61_post_syll_adjustments(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_post_syll_adjustments(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_syll_adjustments,args,3,3);
    }
    return reloq_engb61_post_syll_adjustments(state, p0, p1, p2);
}
int32_t reloq_engb61_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_engb61_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_postnucl_f0,args,1,1);
    }
    return reloq_engb61_postnucl_f0(state, p0);
}
int32_t reloq_engb61_postnucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_postnucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_postnucl_high_tone,args,1,1);
    }
    return reloq_engb61_postnucl_high_tone(state, p0);
}
int32_t reloq_engb61_potential_compound(void *, int32_t);
static inline int32_t reloq_call_engb61_potential_compound(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_potential_compound,args,1,1);
    }
    return reloq_engb61_potential_compound(state, p0);
}
int32_t reloq_engb61_potential_prefix(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_potential_prefix(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_prefix,args,2,2);
    }
    return reloq_engb61_potential_prefix(state, p0, p1);
}
int32_t reloq_engb61_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_engb61_potential_sent_end(state, p0, p1);
}
int32_t reloq_engb61_potential_suffix(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_potential_suffix(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_suffix,args,2,2);
    }
    return reloq_engb61_potential_suffix(state, p0, p1);
}
int32_t reloq_engb61_pre_syll_phone_adjustments(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_pre_syll_phone_adjustments(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pre_syll_phone_adjustments,args,3,3);
    }
    return reloq_engb61_pre_syll_phone_adjustments(state, p0, p1, p2);
}
int32_t reloq_engb61_pre_voice_lengthening(void *, int32_t);
static inline int32_t reloq_call_engb61_pre_voice_lengthening(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_pre_voice_lengthening,args,1,1);
    }
    return reloq_engb61_pre_voice_lengthening(state, p0);
}
int32_t reloq_engb61_preceded_by_copula_verb(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_preceded_by_copula_verb(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_preceded_by_copula_verb,args,2,2);
    }
    return reloq_engb61_preceded_by_copula_verb(state, p0, p1);
}
int32_t reloq_engb61_precedes_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_precedes_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_end_quote,args,2,2);
    }
    return reloq_engb61_precedes_end_quote(state, p0, p1);
}
int32_t reloq_engb61_precedes_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_precedes_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_number,args,3,3);
    }
    return reloq_engb61_precedes_number(state, p0, p1, p2);
}
int32_t reloq_engb61_preceding_disk_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_preceding_disk_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_preceding_disk_word,args,2,2);
    }
    return reloq_engb61_preceding_disk_word(state, p0, p1);
}
int32_t reloq_engb61_prenom_adj_series(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_prenom_adj_series(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_prenom_adj_series,args,2,2);
    }
    return reloq_engb61_prenom_adj_series(state, p0, p1);
}
int32_t reloq_engb61_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_engb61_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_prenucl_f0,args,1,1);
    }
    return reloq_engb61_prenucl_f0(state, p0);
}
int32_t reloq_engb61_prep_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_prep_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_prep_phrase,args,1,1);
    }
    return reloq_engb61_prep_phrase(state, p0);
}
int32_t reloq_engb61_prepositions_vs_particles(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_prepositions_vs_particles(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prepositions_vs_particles,args,3,3);
    }
    return reloq_engb61_prepositions_vs_particles(state, p0, p1, p2);
}
int32_t reloq_engb61_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_engb61_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_engb61_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_engb61_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_engb61_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_engb61_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_engb61_print_eng_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_print_eng_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_eng_SPR,args,3,3);
    }
    return reloq_engb61_print_eng_SPR(state, p0, p1, p2);
}
int32_t reloq_engb61_print_interword_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_print_interword_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_interword_chars,args,3,3);
    }
    return reloq_engb61_print_interword_chars(state, p0, p1, p2);
}
int32_t reloq_engb61_print_normalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_print_normalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_normalized_inp,args,3,3);
    }
    return reloq_engb61_print_normalized_inp(state, p0, p1, p2);
}
int32_t reloq_engb61_print_prompt(void *, int32_t);
static inline int32_t reloq_call_engb61_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_engb61_print_prompt(state, p0);
}
int32_t reloq_engb61_print_spr(void *, int32_t);
static inline int32_t reloq_call_engb61_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_engb61_print_spr(state, p0);
}
int32_t reloq_engb61_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_engb61_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_engb61_print_word_stress(state, p0, p1);
}
int32_t reloq_engb61_printouts(void *, int32_t);
static inline int32_t reloq_call_engb61_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_engb61_printouts(state, p0);
}
int32_t reloq_engb61_priority1_str_situation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_priority1_str_situation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_priority1_str_situation,args,3,3);
    }
    return reloq_engb61_priority1_str_situation(state, p0, p1, p2);
}
int32_t reloq_engb61_priority2_str_situation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_priority2_str_situation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_priority2_str_situation,args,3,3);
    }
    return reloq_engb61_priority2_str_situation(state, p0, p1, p2);
}
int32_t reloq_engb61_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_engb61_process_annotation(state, p0, p1, p2);
}
int32_t reloq_engb61_process_final_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_final_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_apostrophe,args,3,3);
    }
    return reloq_engb61_process_final_apostrophe(state, p0, p1, p2);
}
int32_t reloq_engb61_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_final_parenthetical(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_parenthetical,args,3,3);
    }
    return reloq_engb61_process_final_parenthetical(state, p0, p1, p2);
}
int32_t reloq_engb61_process_hostname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_hostname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_hostname,args,3,3);
    }
    return reloq_engb61_process_hostname(state, p0, p1, p2);
}
int32_t reloq_engb61_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_engb61_process_input(state, p0, p1);
}
int32_t reloq_engb61_process_pathname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_pathname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_pathname,args,3,3);
    }
    return reloq_engb61_process_pathname(state, p0, p1, p2);
}
int32_t reloq_engb61_process_trailing_letters(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_trailing_letters(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_letters,args,4,4);
    }
    return reloq_engb61_process_trailing_letters(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_engb61_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_process_zip_code(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_process_zip_code(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_zip_code,args,3,3);
    }
    return reloq_engb61_process_zip_code(state, p0, p1, p2);
}
int32_t reloq_engb61_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_engb61_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_engb61_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_engb61_punctuation(state, p0, p1, p2);
}
int32_t reloq_engb61_q_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_q_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_q_rules,args,1,1);
    }
    return reloq_engb61_q_rules(state, p0);
}
int32_t reloq_engb61_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_engb61_question_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_quote_mark(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_quote_mark(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_quote_mark,args,3,3);
    }
    return reloq_engb61_quote_mark(state, p0, p1, p2);
}
int32_t reloq_engb61_r_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_r_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_r_rules,args,1,1);
    }
    return reloq_engb61_r_rules(state, p0);
}
int32_t reloq_engb61_re_or_le_at_endRoot(void *, int32_t);
static inline int32_t reloq_call_engb61_re_or_le_at_endRoot(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_re_or_le_at_endRoot,args,1,1);
    }
    return reloq_engb61_re_or_le_at_endRoot(state, p0);
}
int32_t reloq_engb61_read_end(void *, int32_t);
static inline int32_t reloq_call_engb61_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_engb61_read_end(state, p0);
}
int32_t reloq_engb61_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_engb61_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_engb61_realize_prenucl_tone(void *, int32_t);
static inline int32_t reloq_call_engb61_realize_prenucl_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_realize_prenucl_tone,args,1,1);
    }
    return reloq_engb61_realize_prenucl_tone(state, p0);
}
int32_t reloq_engb61_reduce_GA_vowels(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_reduce_UK_vowels(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reduce_UK_vowels(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reduce_UK_vowels,args,3,3);
    }
    return reloq_engb61_reduce_UK_vowels(state, p0, p1, p2);
}
int32_t reloq_engb61_reduce_accent_for_selected_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reduce_accent_for_selected_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reduce_accent_for_selected_words,args,3,3);
    }
    return reloq_engb61_reduce_accent_for_selected_words(state, p0, p1, p2);
}
int32_t reloq_engb61_reduce_funct_vowel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reduce_funct_vowel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reduce_funct_vowel,args,3,3);
    }
    return reloq_engb61_reduce_funct_vowel(state, p0, p1, p2);
}
int32_t reloq_engb61_reduce_vowel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reduce_vowel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reduce_vowel,args,3,3);
    }
    return reloq_engb61_reduce_vowel(state, p0, p1, p2);
}
int32_t reloq_engb61_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_engb61_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_engb61_register_SAPI_voices(state, p0);
}
int32_t reloq_engb61_register_eng_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_register_eng_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_eng_phones,args,3,3);
    }
    return reloq_engb61_register_eng_phones(state, p0, p1, p2);
}
int32_t reloq_engb61_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_engb61_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reset_endptr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_endptr,args,4,4);
    }
    return reloq_engb61_reset_endptr(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_engb61_reset_eng_sent_vars(void *, int32_t);
int32_t reloq_engb61_reset_enum_ptr(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reset_enum_ptr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_enum_ptr,args,2,2);
    }
    return reloq_engb61_reset_enum_ptr(state, p0, p1);
}
int32_t reloq_engb61_reset_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_reset_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_gcat_vars,args,1,1);
    }
    return reloq_engb61_reset_gcat_vars(state, p0);
}
int32_t reloq_engb61_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_engb61_reset_lang_sent_vars(state, p0);
}
int32_t reloq_engb61_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_engb61_reset_phrase_vars(state, p0);
}
int32_t reloq_engb61_reset_right_endptr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reset_right_endptr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_right_endptr,args,4,4);
    }
    return reloq_engb61_reset_right_endptr(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_engb61_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_engb61_reset_sent_vars(state, p0);
}
int32_t reloq_engb61_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_engb61_reset_token_vars(state, p0, p1);
}
int32_t reloq_engb61_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_engb61_reset_word_vals(state, p0, p1);
}
int32_t reloq_engb61_roots1a(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_roots1a(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots1a,args,3,3);
    }
    return reloq_engb61_roots1a(state, p0, p1, p2);
}
int32_t reloq_engb61_roots1b(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_roots1b(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots1b,args,3,3);
    }
    return reloq_engb61_roots1b(state, p0, p1, p2);
}
int32_t reloq_engb61_roots2(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_roots2(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots2,args,3,3);
    }
    return reloq_engb61_roots2(state, p0, p1, p2);
}
int32_t reloq_engb61_roots3(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_roots3(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots3,args,3,3);
    }
    return reloq_engb61_roots3(state, p0, p1, p2);
}
int32_t reloq_engb61_roots4(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_roots4(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots4,args,3,3);
    }
    return reloq_engb61_roots4(state, p0, p1, p2);
}
int32_t reloq_engb61_roots5(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_roots5(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots5,args,3,3);
    }
    return reloq_engb61_roots5(state, p0, p1, p2);
}
int32_t reloq_engb61_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_engb61_run_filters(state, p0, p1, p2);
}
int32_t reloq_engb61_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_engb61_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_engb61_run_speech_module(state, p0);
}
int32_t reloq_engb61_run_text_module(void *, int32_t);
static inline int32_t reloq_call_engb61_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_engb61_run_text_module(state, p0);
}
int32_t reloq_engb61_s_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_s_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_s_rules,args,1,1);
    }
    return reloq_engb61_s_rules(state, p0);
}
int32_t reloq_engb61_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_said_phrase,args,4,4);
    }
    return reloq_engb61_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_same_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_same_inp,args,5,5);
    }
    return reloq_engb61_same_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_engb61_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_engb61_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_engb61_scale_word_final_trans(state, p0);
}
int32_t reloq_engb61_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_engb61_semicolon_phrase(state, p0);
}
int32_t reloq_engb61_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_engb61_separate_tokens(state, p0, p1);
}
int32_t reloq_engb61_set_adjacent_word_ptrs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_adjacent_word_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_adjacent_word_ptrs,args,3,3);
    }
    return reloq_engb61_set_adjacent_word_ptrs(state, p0, p1, p2);
}
int32_t reloq_engb61_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_engb61_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_engb61_set_eng_global_constants(void *, int32_t);
static inline int32_t reloq_call_engb61_set_eng_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_eng_global_constants,args,1,1);
    }
    return reloq_engb61_set_eng_global_constants(state, p0);
}
int32_t reloq_engb61_set_extrametrical_syll(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_extrametrical_syll(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_extrametrical_syll,args,2,2);
    }
    return reloq_engb61_set_extrametrical_syll(state, p0, p1);
}
int32_t reloq_engb61_set_f0_range_ptrs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_f0_range_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_range_ptrs,args,5,5);
    }
    return reloq_engb61_set_f0_range_ptrs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_engb61_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_engb61_set_f0_wind_and_base(state, p0);
}
int32_t reloq_engb61_set_first_postnuc_value(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_first_postnuc_value(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_first_postnuc_value,args,2,2);
    }
    return reloq_engb61_set_first_postnuc_value(state, p0, p1);
}
int32_t reloq_engb61_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_engb61_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_engb61_set_global_constants(state, p0);
}
int32_t reloq_engb61_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_engb61_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_engb61_set_klattparms(state, p0);
}
int32_t reloq_engb61_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_engb61_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_engb61_set_lang_global_constants(state, p0);
}
int32_t reloq_engb61_set_letter_name_wordcats(void *, int32_t);
static inline int32_t reloq_call_engb61_set_letter_name_wordcats(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_letter_name_wordcats,args,1,1);
    }
    return reloq_engb61_set_letter_name_wordcats(state, p0);
}
int32_t reloq_engb61_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_engb61_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_engb61_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_engb61_set_num_formants(state, p0);
}
int32_t reloq_engb61_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_engb61_set_phone_offset_vals(state, p0);
}
int32_t reloq_engb61_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_engb61_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_engb61_set_said_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_said_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_said_phr_vals,args,3,3);
    }
    return reloq_engb61_set_said_phr_vals(state, p0, p1, p2);
}
int32_t reloq_engb61_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_engb61_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_engb61_set_specific_word_stress(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_set_specific_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_specific_word_stress,args,5,5);
    }
    return reloq_engb61_set_specific_word_stress(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_engb61_set_syll_offset_vals(state, p0);
}
int32_t reloq_engb61_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_engb61_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_engb61_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_engb61_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_engb61_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_engb61_set_voice_param_constants(state, p0);
}
int32_t reloq_engb61_set_window_size(void *, int32_t);
static inline int32_t reloq_call_engb61_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_engb61_set_window_size(state, p0);
}
int32_t reloq_engb61_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_engb61_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_engb61_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_engb61_setdur(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_engb61_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_engb61_shift_tone(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_shift_tone(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_shift_tone,args,2,2);
    }
    return reloq_engb61_shift_tone(state, p0, p1);
}
int32_t reloq_engb61_short_continuation_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_short_continuation_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_short_continuation_phrase,args,5,5);
    }
    return reloq_engb61_short_continuation_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_shortdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_shortdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_shortdash,args,3,3);
    }
    return reloq_engb61_shortdash(state, p0, p1, p2);
}
int32_t reloq_engb61_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_engb61_single_chars(state, p0, p1, p2);
}
int32_t reloq_engb61_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_engb61_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_engb61_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_engb61_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_engb61_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_engb61_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_engb61_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_engb61_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_engb61_single_letter(state, p0, p1, p2);
}
int32_t reloq_engb61_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_engb61_size_filter(state, p0, p1, p2);
}
int32_t reloq_engb61_skip_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_skip_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_delimiters,args,2,2);
    }
    return reloq_engb61_skip_delimiters(state, p0, p1);
}
int32_t reloq_engb61_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_engb61_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_engb61_slash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_slash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash,args,3,3);
    }
    return reloq_engb61_slash(state, p0, p1, p2);
}
int32_t reloq_engb61_slash_before_measure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_slash_before_measure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash_before_measure,args,3,3);
    }
    return reloq_engb61_slash_before_measure(state, p0, p1, p2);
}
int32_t reloq_engb61_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_engb61_space(state, p0, p1, p2);
}
int32_t reloq_engb61_span_inverted_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_span_inverted_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_span_inverted_punct,args,3,3);
    }
    return reloq_engb61_span_inverted_punct(state, p0, p1, p2);
}
int32_t reloq_engb61_special_cases(void *, int32_t);
static inline int32_t reloq_call_engb61_special_cases(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_special_cases,args,1,1);
    }
    return reloq_engb61_special_cases(state, p0);
}
int32_t reloq_engb61_special_compounds(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_special_compounds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_special_compounds,args,5,5);
    }
    return reloq_engb61_special_compounds(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_engb61_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_engb61_spell_out_large_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_spell_out_large_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_large_num,args,3,3);
    }
    return reloq_engb61_spell_out_large_num(state, p0, p1, p2);
}
int32_t reloq_engb61_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_engb61_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_engb61_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_engb61_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_splitCompoundAtUppercase(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_splitCompoundAtUppercase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_splitCompoundAtUppercase,args,2,2);
    }
    return reloq_engb61_splitCompoundAtUppercase(state, p0, p1);
}
int32_t reloq_engb61_split_final_compound(void *, int32_t);
static inline int32_t reloq_call_engb61_split_final_compound(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_split_final_compound,args,1,1);
    }
    return reloq_engb61_split_final_compound(state, p0);
}
int32_t reloq_engb61_split_initial_compound(void *, int32_t);
static inline int32_t reloq_call_engb61_split_initial_compound(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_split_initial_compound,args,1,1);
    }
    return reloq_engb61_split_initial_compound(state, p0);
}
int32_t reloq_engb61_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_engb61_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_starTone_continuation_phr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_starTone_continuation_phr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_starTone_continuation_phr,args,3,3);
    }
    return reloq_engb61_starTone_continuation_phr(state, p0, p1, p2);
}
int32_t reloq_engb61_start_dur(void *, int32_t);
static inline int32_t reloq_call_engb61_start_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_start_dur,args,1,1);
    }
    return reloq_engb61_start_dur(state, p0);
}
int32_t reloq_engb61_state_abbr_not_initials(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_state_abbr_not_initials(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_state_abbr_not_initials,args,3,3);
    }
    return reloq_engb61_state_abbr_not_initials(state, p0, p1, p2);
}
int32_t reloq_engb61_state_name_leftward(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_state_name_leftward(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_state_name_leftward,args,3,3);
    }
    return reloq_engb61_state_name_leftward(state, p0, p1, p2);
}
int32_t reloq_engb61_state_name_rightward(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_state_name_rightward(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_state_name_rightward,args,3,3);
    }
    return reloq_engb61_state_name_rightward(state, p0, p1, p2);
}
int32_t reloq_engb61_stress_at_phr_boundary(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_stress_at_phr_boundary(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_at_phr_boundary,args,3,3);
    }
    return reloq_engb61_stress_at_phr_boundary(state, p0, p1, p2);
}
int32_t reloq_engb61_stress_based_strong_i(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_stress_based_strong_i(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_based_strong_i,args,3,3);
    }
    return reloq_engb61_stress_based_strong_i(state, p0, p1, p2);
}
int32_t reloq_engb61_stress_based_strong_y(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_stress_based_strong_y(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_based_strong_y,args,3,3);
    }
    return reloq_engb61_stress_based_strong_y(state, p0, p1, p2);
}
int32_t reloq_engb61_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_engb61_stretch_focus(state, p0, p1);
}
int32_t reloq_engb61_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_engb61_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_string_to_short(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_short,args,4,4);
    }
    return reloq_engb61_string_to_short(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_strip_ambig_suffix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_ambig_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ambig_suffix,args,1,1);
    }
    return reloq_engb61_strip_ambig_suffix(state, p0);
}
int32_t reloq_engb61_strip_ation(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_ation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ation,args,1,1);
    }
    return reloq_engb61_strip_ation(state, p0);
}
int32_t reloq_engb61_strip_clitics(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_strip_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_clitics,args,5,5);
    }
    return reloq_engb61_strip_clitics(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_strip_contractions(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_strip_contractions(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_contractions,args,4,4);
    }
    return reloq_engb61_strip_contractions(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_strip_ed(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_ed(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ed,args,1,1);
    }
    return reloq_engb61_strip_ed(state, p0);
}
int32_t reloq_engb61_strip_eng_clitics(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_strip_eng_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_eng_clitics,args,4,4);
    }
    return reloq_engb61_strip_eng_clitics(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_strip_er(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_er(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_er,args,1,1);
    }
    return reloq_engb61_strip_er(state, p0);
}
int32_t reloq_engb61_strip_ess(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_ess(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ess,args,1,1);
    }
    return reloq_engb61_strip_ess(state, p0);
}
int32_t reloq_engb61_strip_ing(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_ing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ing,args,1,1);
    }
    return reloq_engb61_strip_ing(state, p0);
}
int32_t reloq_engb61_strip_inner_adj_suffix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_inner_adj_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_inner_adj_suffix,args,1,1);
    }
    return reloq_engb61_strip_inner_adj_suffix(state, p0);
}
int32_t reloq_engb61_strip_inner_noun_suffix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_inner_noun_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_inner_noun_suffix,args,1,1);
    }
    return reloq_engb61_strip_inner_noun_suffix(state, p0);
}
int32_t reloq_engb61_strip_inner_prefix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_inner_prefix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_inner_prefix,args,1,1);
    }
    return reloq_engb61_strip_inner_prefix(state, p0);
}
int32_t reloq_engb61_strip_ize(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_ize(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ize,args,1,1);
    }
    return reloq_engb61_strip_ize(state, p0);
}
int32_t reloq_engb61_strip_last_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_strip_last_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_last_digit,args,3,3);
    }
    return reloq_engb61_strip_last_digit(state, p0, p1, p2);
}
int32_t reloq_engb61_strip_ly_suffix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_ly_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ly_suffix,args,1,1);
    }
    return reloq_engb61_strip_ly_suffix(state, p0);
}
int32_t reloq_engb61_strip_medial_noun_suffix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_medial_noun_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_medial_noun_suffix,args,1,1);
    }
    return reloq_engb61_strip_medial_noun_suffix(state, p0);
}
int32_t reloq_engb61_strip_medial_prefix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_medial_prefix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_medial_prefix,args,1,1);
    }
    return reloq_engb61_strip_medial_prefix(state, p0);
}
int32_t reloq_engb61_strip_nonlex_prefixes(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_nonlex_prefixes(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_nonlex_prefixes,args,1,1);
    }
    return reloq_engb61_strip_nonlex_prefixes(state, p0);
}
int32_t reloq_engb61_strip_noun_verb_suffix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_noun_verb_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_noun_verb_suffix,args,1,1);
    }
    return reloq_engb61_strip_noun_verb_suffix(state, p0);
}
int32_t reloq_engb61_strip_nounadj_suffix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_nounadj_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_nounadj_suffix,args,1,1);
    }
    return reloq_engb61_strip_nounadj_suffix(state, p0);
}
int32_t reloq_engb61_strip_outer_prefixes(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_outer_prefixes(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_outer_prefixes,args,1,1);
    }
    return reloq_engb61_strip_outer_prefixes(state, p0);
}
int32_t reloq_engb61_strip_outer_suffix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_outer_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_outer_suffix,args,1,1);
    }
    return reloq_engb61_strip_outer_suffix(state, p0);
}
int32_t reloq_engb61_strip_pname_prefix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_pname_prefix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_pname_prefix,args,1,1);
    }
    return reloq_engb61_strip_pname_prefix(state, p0);
}
int32_t reloq_engb61_strip_pname_suffixes(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_pname_suffixes(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_pname_suffixes,args,1,1);
    }
    return reloq_engb61_strip_pname_suffixes(state, p0);
}
int32_t reloq_engb61_strip_possessive(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_strip_possessive(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_possessive,args,4,4);
    }
    return reloq_engb61_strip_possessive(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_strip_s(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_s,args,1,1);
    }
    return reloq_engb61_strip_s(state, p0);
}
int32_t reloq_engb61_strip_scientific_prefixes(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_scientific_prefixes(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_scientific_prefixes,args,1,1);
    }
    return reloq_engb61_strip_scientific_prefixes(state, p0);
}
int32_t reloq_engb61_strip_single_nonlex_prefix(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_single_nonlex_prefix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_single_nonlex_prefix,args,1,1);
    }
    return reloq_engb61_strip_single_nonlex_prefix(state, p0);
}
int32_t reloq_engb61_strip_suffix(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_strip_suffix(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_suffix,args,2,2);
    }
    return reloq_engb61_strip_suffix(state, p0, p1);
}
int32_t reloq_engb61_strip_suffixes(void *, int32_t);
static inline int32_t reloq_call_engb61_strip_suffixes(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_suffixes,args,1,1);
    }
    return reloq_engb61_strip_suffixes(state, p0);
}
int32_t reloq_engb61_strong_vowel(void *, int32_t);
static inline int32_t reloq_call_engb61_strong_vowel(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strong_vowel,args,1,1);
    }
    return reloq_engb61_strong_vowel(state, p0);
}
int32_t reloq_engb61_subj_vs_obj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_subj_vs_obj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subj_vs_obj,args,3,3);
    }
    return reloq_engb61_subj_vs_obj(state, p0, p1, p2);
}
int32_t reloq_engb61_subject_pronoun(void *, int32_t);
static inline int32_t reloq_call_engb61_subject_pronoun(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_subject_pronoun,args,1,1);
    }
    return reloq_engb61_subject_pronoun(state, p0);
}
int32_t reloq_engb61_subord_conj(void *, int32_t);
static inline int32_t reloq_call_engb61_subord_conj(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_subord_conj,args,1,1);
    }
    return reloq_engb61_subord_conj(state, p0);
}
int32_t reloq_engb61_subordinate_clause(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_subordinate_clause(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subordinate_clause,args,5,5);
    }
    return reloq_engb61_subordinate_clause(state, p0, p1, p2, p3, p4);
}
int32_t reloq_engb61_suffixes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_suffixes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_suffixes,args,3,3);
    }
    return reloq_engb61_suffixes(state, p0, p1, p2);
}
int32_t reloq_engb61_syllable_based_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_syllable_based_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_syllable_based_rules,args,1,1);
    }
    return reloq_engb61_syllable_based_rules(state, p0);
}
int32_t reloq_engb61_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_engb61_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_engb61_t_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_t_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_t_rules,args,1,1);
    }
    return reloq_engb61_t_rules(state, p0);
}
int32_t reloq_engb61_tag_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_tag_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tag_phrase,args,3,3);
    }
    return reloq_engb61_tag_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_test_left_context(void *, int32_t);
static inline int32_t reloq_call_engb61_test_left_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_test_left_context,args,1,1);
    }
    return reloq_engb61_test_left_context(state, p0);
}
int32_t reloq_engb61_test_lex_pre(void *, int32_t);
static inline int32_t reloq_call_engb61_test_lex_pre(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_test_lex_pre,args,1,1);
    }
    return reloq_engb61_test_lex_pre(state, p0);
}
int32_t reloq_engb61_test_noun_adj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_test_noun_adj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_test_noun_adj,args,3,3);
    }
    return reloq_engb61_test_noun_adj(state, p0, p1, p2);
}
int32_t reloq_engb61_test_noun_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_test_noun_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_test_noun_verb,args,3,3);
    }
    return reloq_engb61_test_noun_verb(state, p0, p1, p2);
}
int32_t reloq_engb61_test_nva(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_test_nva(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_test_nva,args,3,3);
    }
    return reloq_engb61_test_nva(state, p0, p1, p2);
}
int32_t reloq_engb61_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_engb61_test_verb_adj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_test_verb_adj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_test_verb_adj,args,3,3);
    }
    return reloq_engb61_test_verb_adj(state, p0, p1, p2);
}
int32_t reloq_engb61_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_engb61_text_index(state, p0, p1);
}
int32_t reloq_engb61_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_engb61_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_three_letter_extension(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_three_letter_extension(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_letter_extension,args,2,2);
    }
    return reloq_engb61_three_letter_extension(state, p0, p1);
}
int32_t reloq_engb61_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict,args,3,3);
    }
    return reloq_engb61_tok_dict(state, p0, p1, p2);
}
int32_t reloq_engb61_tok_dict_entry(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_tok_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict_entry,args,3,3);
    }
    return reloq_engb61_tok_dict_entry(state, p0, p1, p2);
}
int32_t reloq_engb61_tok_lookup(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_tok_lookup(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_lookup,args,2,2);
    }
    return reloq_engb61_tok_lookup(state, p0, p1);
}
int32_t reloq_engb61_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_engb61_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_engb61_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_engb61_token_to_number(state, p0, p1, p2);
}
int32_t reloq_engb61_trailing_suffix(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_trailing_suffix(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_trailing_suffix,args,3,3);
    }
    return reloq_engb61_trailing_suffix(state, p0, p1, p2);
}
int32_t reloq_engb61_two_stress_begin(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_two_stress_begin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_two_stress_begin,args,2,2);
    }
    return reloq_engb61_two_stress_begin(state, p0, p1);
}
int32_t reloq_engb61_two_stress_ending(void *, int32_t);
static inline int32_t reloq_call_engb61_two_stress_ending(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_two_stress_ending,args,1,1);
    }
    return reloq_engb61_two_stress_ending(state, p0);
}
int32_t reloq_engb61_two_to_one(void *, int32_t);
static inline int32_t reloq_call_engb61_two_to_one(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_two_to_one,args,1,1);
    }
    return reloq_engb61_two_to_one(state, p0);
}
int32_t reloq_engb61_u_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_u_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_u_rules,args,1,1);
    }
    return reloq_engb61_u_rules(state, p0);
}
int32_t reloq_engb61_user_abbr_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_user_abbr_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_abbr_dict,args,3,3);
    }
    return reloq_engb61_user_abbr_dict(state, p0, p1, p2);
}
int32_t reloq_engb61_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_engb61_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_user_rootdict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_user_rootdict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_rootdict,args,3,3);
    }
    return reloq_engb61_user_rootdict(state, p0, p1, p2);
}
int32_t reloq_engb61_valid_f0_posn(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_valid_f0_posn(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_f0_posn,args,3,3);
    }
    return reloq_engb61_valid_f0_posn(state, p0, p1, p2);
}
int32_t reloq_engb61_valid_year_sequence(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_valid_year_sequence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_year_sequence,args,2,2);
    }
    return reloq_engb61_valid_year_sequence(state, p0, p1);
}
int32_t reloq_engb61_verb_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_verb_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_verb_phrase,args,1,1);
    }
    return reloq_engb61_verb_phrase(state, p0);
}
int32_t reloq_engb61_verb_vs_adj(void *, int32_t);
static inline int32_t reloq_call_engb61_verb_vs_adj(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_verb_vs_adj,args,1,1);
    }
    return reloq_engb61_verb_vs_adj(state, p0);
}
int32_t reloq_engb61_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_engb61_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_vocative_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_vocative_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocative_phrase,args,3,3);
    }
    return reloq_engb61_vocative_phrase(state, p0, p1, p2);
}
int32_t reloq_engb61_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_engb61_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_engb61_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_voice_phone_s(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_voice_phone_s(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_phone_s,args,3,3);
    }
    return reloq_engb61_voice_phone_s(state, p0, p1, p2);
}
int32_t reloq_engb61_w_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_w_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_w_rules,args,1,1);
    }
    return reloq_engb61_w_rules(state, p0);
}
int32_t reloq_engb61_wh_word(void *, int32_t);
static inline int32_t reloq_call_engb61_wh_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_wh_word,args,1,1);
    }
    return reloq_engb61_wh_word(state, p0);
}
int32_t reloq_engb61_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_engb61_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_word_final_flap(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_engb61_word_final_lengthening(void *, int32_t);
static inline int32_t reloq_call_engb61_word_final_lengthening(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_final_lengthening,args,1,1);
    }
    return reloq_engb61_word_final_lengthening(state, p0);
}
int32_t reloq_engb61_word_initial_h(void *, int32_t);
static inline int32_t reloq_call_engb61_word_initial_h(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_initial_h,args,1,1);
    }
    return reloq_engb61_word_initial_h(state, p0);
}
int32_t reloq_engb61_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_engb61_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_engb61_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_engb61_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_x_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_x_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_x_rules,args,1,1);
    }
    return reloq_engb61_x_rules(state, p0);
}
int32_t reloq_engb61_y_ins_left_env(void *, int32_t);
static inline int32_t reloq_call_engb61_y_ins_left_env(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_y_ins_left_env,args,1,1);
    }
    return reloq_engb61_y_ins_left_env(state, p0);
}
int32_t reloq_engb61_y_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_y_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_y_rules,args,1,1);
    }
    return reloq_engb61_y_rules(state, p0);
}
int32_t reloq_engb61_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_engb61_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_engb61_year_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_engb61_yet_phrase(void *, int32_t);
static inline int32_t reloq_call_engb61_yet_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_yet_phrase,args,1,1);
    }
    return reloq_engb61_yet_phrase(state, p0);
}
int32_t reloq_engb61_z_rules(void *, int32_t);
static inline int32_t reloq_call_engb61_z_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_z_rules,args,1,1);
    }
    return reloq_engb61_z_rules(state, p0);
}
int32_t reloq_engb61_zero_stress_syll(void *, int32_t, int32_t);
static inline int32_t reloq_call_engb61_zero_stress_syll(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_zero_stress_syll,args,2,2);
    }
    return reloq_engb61_zero_stress_syll(state, p0, p1);
}
#endif
