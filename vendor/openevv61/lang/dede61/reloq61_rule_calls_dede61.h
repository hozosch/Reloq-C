/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_DEDE61_H
#define RELOQ61_RULE_CALLS_DEDE61_H
#include "delta_rules_c.h"
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_dede61_DeltaProc_end(void *, int32_t);
int32_t reloq_dede61_DeltaProc_flush(void *, int32_t);
int32_t reloq_dede61_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_dede61_DeltaProc_main(void *, int32_t);
int32_t reloq_dede61_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_dede61_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_dede61_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_dede61_DeltaProc_start(void *, int32_t);
int32_t reloq_dede61_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_dede61_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_dede61_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_dede61_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_URL(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_URL(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_URL,args,2,2);
    }
    return reloq_dede61_URL(state, p0, p1);
}
int32_t reloq_dede61_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbr,args,4,4);
    }
    return reloq_dede61_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_abbr_before_num(void *, int32_t, int32_t);
int32_t reloq_dede61_abbreviation(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_abbreviation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbreviation,args,4,4);
    }
    return reloq_dede61_abbreviation(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_dede61_absval(state, p0, p1, p2);
}
int32_t reloq_dede61_acronym(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_acronym(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acronym,args,3,3);
    }
    return reloq_dede61_acronym(state, p0, p1, p2);
}
int32_t reloq_dede61_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acrotest,args,3,3);
    }
    return reloq_dede61_acrotest(state, p0, p1, p2);
}
int32_t reloq_dede61_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_dede61_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_dede61_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_add_case_to_ger_fraction(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_add_case_to_ger_fraction(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_case_to_ger_fraction,args,2,2);
    }
    return reloq_dede61_add_case_to_ger_fraction(state, p0, p1);
}
int32_t reloq_dede61_add_case_to_ger_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_add_case_to_ger_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_case_to_ger_ordinal,args,3,3);
    }
    return reloq_dede61_add_case_to_ger_ordinal(state, p0, p1, p2);
}
int32_t reloq_dede61_add_f0_for_new_voice(void *, int32_t);
static inline int32_t reloq_call_dede61_add_f0_for_new_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_new_voice,args,1,1);
    }
    return reloq_dede61_add_f0_for_new_voice(state, p0);
}
int32_t reloq_dede61_add_f0_for_next_voice(void *, int32_t);
static inline int32_t reloq_call_dede61_add_f0_for_next_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_next_voice,args,1,1);
    }
    return reloq_dede61_add_f0_for_next_voice(state, p0);
}
int32_t reloq_dede61_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_dede61_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_dede61_add_ger_currency_units(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_add_ger_currency_units(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ger_currency_units,args,3,3);
    }
    return reloq_dede61_add_ger_currency_units(state, p0, p1, p2);
}
int32_t reloq_dede61_add_ger_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_add_ger_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ger_point_digits,args,3,3);
    }
    return reloq_dede61_add_ger_point_digits(state, p0, p1, p2);
}
int32_t reloq_dede61_add_ger_subunits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_add_ger_subunits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ger_subunits,args,3,3);
    }
    return reloq_dede61_add_ger_subunits(state, p0, p1, p2);
}
int32_t reloq_dede61_add_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_add_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_point_digits,args,3,3);
    }
    return reloq_dede61_add_point_digits(state, p0, p1, p2);
}
int32_t reloq_dede61_adj_vs_prep_or_post(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_adj_vs_prep_or_post(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adj_vs_prep_or_post,args,3,3);
    }
    return reloq_dede61_adj_vs_prep_or_post(state, p0, p1, p2);
}
int32_t reloq_dede61_adj_vs_verb(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_adj_vs_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adj_vs_verb,args,6,6);
    }
    return reloq_dede61_adj_vs_verb(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_dede61_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_dede61_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_dede61_adjust_AV_for_fem_voice(void *, int32_t);
int32_t reloq_dede61_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_dede61_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_adjust_ger_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_adjust_ger_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_ger_word_stress,args,3,3);
    }
    return reloq_dede61_adjust_ger_word_stress(state, p0, p1, p2);
}
int32_t reloq_dede61_adjust_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_adjust_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_numbers,args,3,3);
    }
    return reloq_dede61_adjust_numbers(state, p0, p1, p2);
}
int32_t reloq_dede61_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_adjust_prev_phr_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_pause,args,3,3);
    }
    return reloq_dede61_adjust_prev_phr_pause(state, p0, p1, p2);
}
int32_t reloq_dede61_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_dede61_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_dede61_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_adjust_words_in_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_words_in_phrase,args,3,3);
    }
    return reloq_dede61_adjust_words_in_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_ambig_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ambig_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ambig_abbr,args,2,2);
    }
    return reloq_dede61_ambig_abbr(state, p0, p1);
}
int32_t reloq_dede61_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_dede61_apostrophe(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_dede61_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_dede61_apply_comma_phr_rules(void *, int32_t);
static inline int32_t reloq_call_dede61_apply_comma_phr_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_comma_phr_rules,args,1,1);
    }
    return reloq_dede61_apply_comma_phr_rules(state, p0);
}
int32_t reloq_dede61_apply_ger_a_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_a_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_a_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_a_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_a_umlaut_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_a_umlaut_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_a_umlaut_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_a_umlaut_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_b_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_b_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_b_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_b_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_c_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_c_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_c_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_c_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_comma_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_comma_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_d_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_d_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_d_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_d_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_e_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_e_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_e_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_e_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_f_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_f_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_f_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_f_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_g_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_g_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_g_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_g_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_h_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_h_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_h_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_h_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_i_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_i_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_i_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_i_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_j_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_j_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_j_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_j_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_k_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_k_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_k_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_k_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_l_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_l_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_l_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_l_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_m_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_m_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_m_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_m_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_n_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_n_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_n_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_n_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_o_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_o_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_o_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_o_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_p_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_p_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_p_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_p_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_q_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_q_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_q_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_q_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_r_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_r_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_r_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_r_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_s_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_s_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_s_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_s_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_scharfes_s_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_scharfes_s_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_scharfes_s_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_scharfes_s_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_t_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_t_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_t_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_t_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_u_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_u_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_u_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_u_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_v_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_v_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_v_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_v_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_w_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_w_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_w_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_w_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_x_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_x_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_x_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_x_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_y_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_y_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_y_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_y_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_ger_z_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_ger_z_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ger_z_rules,args,3,3);
    }
    return reloq_dede61_apply_ger_z_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_dede61_apply_head_size_val(state, p0, p1);
}
int32_t reloq_dede61_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_lang_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_lang_comma_rules,args,3,3);
    }
    return reloq_dede61_apply_lang_comma_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_dede61_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_dede61_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_dede61_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_dede61_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_dede61_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_dede61_apply_roughness_val(state, p0, p1);
}
int32_t reloq_dede61_apply_rules(void *, int32_t);
static inline int32_t reloq_call_dede61_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_dede61_apply_rules(state, p0);
}
int32_t reloq_dede61_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_dede61_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_dede61_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_dede61_apply_speed_val(state, p0, p1);
}
int32_t reloq_dede61_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_dede61_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_dede61_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_dede61_apply_volume_val(state, p0, p1);
}
int32_t reloq_dede61_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_dede61_assign_acoustic_vals(state, p0);
}
int32_t reloq_dede61_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_dede61_assign_alt_quest_features(state, p0);
}
int32_t reloq_dede61_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_dede61_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_dede61_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_dede61_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_dede61_assign_default_compound_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_default_compound_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_compound_stress,args,3,3);
    }
    return reloq_dede61_assign_default_compound_stress(state, p0, p1, p2);
}
int32_t reloq_dede61_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_dede61_assign_default_quest_features(state, p0);
}
int32_t reloq_dede61_assign_ger_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_ger_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_ger_nuc_durs,args,1,1);
    }
    return reloq_dede61_assign_ger_nuc_durs(state, p0);
}
int32_t reloq_dede61_assign_ger_particle_stress(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_ger_particle_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_ger_particle_stress,args,1,1);
    }
    return reloq_dede61_assign_ger_particle_stress(state, p0);
}
int32_t reloq_dede61_assign_ger_root_stress(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_ger_root_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_ger_root_stress,args,1,1);
    }
    return reloq_dede61_assign_ger_root_stress(state, p0);
}
int32_t reloq_dede61_assign_ger_secondary_stress(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_ger_secondary_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_ger_secondary_stress,args,1,1);
    }
    return reloq_dede61_assign_ger_secondary_stress(state, p0);
}
int32_t reloq_dede61_assign_ger_stress(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_ger_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_ger_stress,args,1,1);
    }
    return reloq_dede61_assign_ger_stress(state, p0);
}
int32_t reloq_dede61_assign_ger_syll_init_con(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_ger_syll_init_con(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_ger_syll_init_con,args,3,3);
    }
    return reloq_dede61_assign_ger_syll_init_con(state, p0, p1, p2);
}
int32_t reloq_dede61_assign_ger_vowel_diaphons(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_ger_vowel_diaphons(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_ger_vowel_diaphons,args,3,3);
    }
    return reloq_dede61_assign_ger_vowel_diaphons(state, p0, p1, p2);
}
int32_t reloq_dede61_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_dede61_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_dede61_assign_marked_compound_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_marked_compound_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_marked_compound_stress,args,3,3);
    }
    return reloq_dede61_assign_marked_compound_stress(state, p0, p1, p2);
}
int32_t reloq_dede61_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_dede61_assign_nuc_durs(state, p0);
}
int32_t reloq_dede61_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_dede61_assign_phone_durs(state, p0);
}
int32_t reloq_dede61_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_dede61_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_dede61_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_dede61_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_dede61_assign_sent_final_features(state, p0);
}
int32_t reloq_dede61_assign_start_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_start_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_start_dur,args,1,1);
    }
    return reloq_dede61_assign_start_dur(state, p0);
}
int32_t reloq_dede61_assign_stress_in_daword(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_stress_in_daword(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_stress_in_daword,args,1,1);
    }
    return reloq_dede61_assign_stress_in_daword(state, p0);
}
int32_t reloq_dede61_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_dede61_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_dede61_assign_user_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_assign_user_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_user_stress,args,3,3);
    }
    return reloq_dede61_assign_user_stress(state, p0, p1, p2);
}
int32_t reloq_dede61_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_dede61_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_dede61_assign_wh_quest_features(state, p0);
}
int32_t reloq_dede61_ausser(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ausser(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ausser,args,4,4);
    }
    return reloq_dede61_ausser(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_beg_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_beg_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_beg_of_word,args,2,2);
    }
    return reloq_dede61_beg_of_word(state, p0, p1);
}
int32_t reloq_dede61_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_boundary_tone,args,1,1);
    }
    return reloq_dede61_boundary_tone(state, p0);
}
int32_t reloq_dede61_bracket(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_bracket(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_bracket,args,3,3);
    }
    return reloq_dede61_bracket(state, p0, p1, p2);
}
int32_t reloq_dede61_break_into_ger_morphs(void *, int32_t);
static inline int32_t reloq_call_dede61_break_into_ger_morphs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_break_into_ger_morphs,args,1,1);
    }
    return reloq_dede61_break_into_ger_morphs(state, p0);
}
int32_t reloq_dede61_break_into_ger_syllables(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_break_into_ger_syllables(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_ger_syllables,args,3,3);
    }
    return reloq_dede61_break_into_ger_syllables(state, p0, p1, p2);
}
int32_t reloq_dede61_break_into_stanger_phones(void *, int32_t);
static inline int32_t reloq_call_dede61_break_into_stanger_phones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_break_into_stanger_phones,args,1,1);
    }
    return reloq_dede61_break_into_stanger_phones(state, p0);
}
int32_t reloq_dede61_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_dede61_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_dede61_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_dede61_build_phrases(state, p0, p1, p2);
}
int32_t reloq_dede61_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_dede61_build_words(state, p0, p1, p2);
}
int32_t reloq_dede61_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_dede61_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_dede61_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_dede61_char_name(state, p0, p1, p2);
}
int32_t reloq_dede61_clear_delta(void *, int32_t);
static inline int32_t reloq_call_dede61_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_dede61_clear_delta(state, p0);
}
int32_t reloq_dede61_close_platform(void *, int32_t);
static inline int32_t reloq_call_dede61_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_dede61_close_platform(state, p0);
}
int32_t reloq_dede61_comma_at_end_quote(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_comma_at_end_quote(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_comma_at_end_quote,args,3,3);
    }
    return reloq_dede61_comma_at_end_quote(state, p0, p1, p2);
}
int32_t reloq_dede61_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_dede61_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_dede61_comma_phrase(state, p0);
}
int32_t reloq_dede61_compound_con_cluster(void *, int32_t);
static inline int32_t reloq_call_dede61_compound_con_cluster(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_compound_con_cluster,args,1,1);
    }
    return reloq_dede61_compound_con_cluster(state, p0);
}
int32_t reloq_dede61_compound_stress_adjustment(void *, int32_t);
static inline int32_t reloq_call_dede61_compound_stress_adjustment(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_compound_stress_adjustment,args,1,1);
    }
    return reloq_dede61_compound_stress_adjustment(state, p0);
}
int32_t reloq_dede61_con_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_dede61_con_vals(state, p0);
}
int32_t reloq_dede61_continuation_phr_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_continuation_phr_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_continuation_phr_phrase_tone,args,1,1);
    }
    return reloq_dede61_continuation_phr_phrase_tone(state, p0);
}
int32_t reloq_dede61_continuation_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_continuation_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_continuation_phrase,args,3,3);
    }
    return reloq_dede61_continuation_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_1_or_2_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_1_or_2_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_1_or_2_digits,args,3,3);
    }
    return reloq_dede61_convert_1_or_2_digits(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_billions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_billions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_billions,args,3,3);
    }
    return reloq_dede61_convert_billions(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits,args,3,3);
    }
    return reloq_dede61_convert_digits(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits_to_words,args,3,3);
    }
    return reloq_dede61_convert_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_double,args,3,3);
    }
    return reloq_dede61_convert_double(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_billions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_billions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_billions,args,3,3);
    }
    return reloq_dede61_convert_ger_billions(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_date,args,3,3);
    }
    return reloq_dede61_convert_ger_date(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_digits_to_words,args,3,3);
    }
    return reloq_dede61_convert_ger_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_double,args,3,3);
    }
    return reloq_dede61_convert_ger_double(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_fraction,args,3,3);
    }
    return reloq_dede61_convert_ger_fraction(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_hundreds,args,3,3);
    }
    return reloq_dede61_convert_ger_hundreds(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_millions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_millions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_millions,args,3,3);
    }
    return reloq_dede61_convert_ger_millions(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_monetary_exp,args,5,5);
    }
    return reloq_dede61_convert_ger_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_convert_ger_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_ordinal,args,3,3);
    }
    return reloq_dede61_convert_ger_ordinal(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_roman_num(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_convert_ger_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_single_digit,args,3,3);
    }
    return reloq_dede61_convert_ger_single_digit(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_teens(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_teens(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_teens,args,3,3);
    }
    return reloq_dede61_convert_ger_teens(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_thousands,args,3,3);
    }
    return reloq_dede61_convert_ger_thousands(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_ger_time(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_ger_time(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ger_time,args,3,3);
    }
    return reloq_dede61_convert_ger_time(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_hundreds,args,3,3);
    }
    return reloq_dede61_convert_hundreds(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_lang_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_lang_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_lang_digits_to_words,args,3,3);
    }
    return reloq_dede61_convert_lang_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_millions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_millions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_millions,args,3,3);
    }
    return reloq_dede61_convert_millions(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_monetary_exp,args,5,5);
    }
    return reloq_dede61_convert_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_convert_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_numbers,args,3,3);
    }
    return reloq_dede61_convert_numbers(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_single_digit,args,3,3);
    }
    return reloq_dede61_convert_single_digit(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_thousands,args,3,3);
    }
    return reloq_dede61_convert_thousands(state, p0, p1, p2);
}
int32_t reloq_dede61_convert_to_ger_year(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_to_ger_year(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_ger_year,args,4,4);
    }
    return reloq_dede61_convert_to_ger_year(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_dede61_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_dede61_coord_conj(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_coord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_coord_conj,args,10,10);
    }
    return reloq_dede61_coord_conj(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_dede61_coord_vs_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_coord_vs_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_coord_vs_verb,args,3,3);
    }
    return reloq_dede61_coord_vs_verb(state, p0, p1, p2);
}
int32_t reloq_dede61_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_dede61_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_dede61_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_dede61_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_dede61_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_dede61_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_count_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_count_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_digits,args,3,3);
    }
    return reloq_dede61_count_digits(state, p0, p1, p2);
}
int32_t reloq_dede61_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_dede61_count_input(state, p0, p1);
}
int32_t reloq_dede61_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_dede61_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_dede61_count_normalized_inp(state, p0);
}
int32_t reloq_dede61_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_dede61_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_dede61_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_dede61_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_dede61_create_ger_syllables(void *, int32_t);
static inline int32_t reloq_call_dede61_create_ger_syllables(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_create_ger_syllables,args,1,1);
    }
    return reloq_dede61_create_ger_syllables(state, p0);
}
int32_t reloq_dede61_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_dede61_create_sentence(state, p0, p1);
}
int32_t reloq_dede61_date_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_date_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_date_phrase,args,6,6);
    }
    return reloq_dede61_date_phrase(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_day_date(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_day_date(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_day_date,args,8,8);
    }
    return reloq_dede61_day_date(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_dede61_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_dede61_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_dede61_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_dede61_delete_2_more(state, p0);
}
int32_t reloq_dede61_delete_comma_sync(void *, int32_t, int32_t);
int32_t reloq_dede61_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_delete_inp_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_left,args,3,3);
    }
    return reloq_dede61_delete_inp_from_left(state, p0, p1, p2);
}
int32_t reloq_dede61_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_delete_inp_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_right,args,3,3);
    }
    return reloq_dede61_delete_inp_from_right(state, p0, p1, p2);
}
int32_t reloq_dede61_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_dede61_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_dede61_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_dede61_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_dede61_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_delete_phone_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_right,args,3,3);
    }
    return reloq_dede61_delete_phone_from_right(state, p0, p1, p2);
}
int32_t reloq_dede61_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_dede61_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_dede61_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_dede61_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_dede61_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_dede61_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_dede61_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_dede61_demote_word(state, p0, p1, p2);
}
int32_t reloq_dede61_destress_adverbs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_adverbs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_adverbs,args,5,5);
    }
    return reloq_dede61_destress_adverbs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_destress_determiners(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_determiners(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_determiners,args,3,3);
    }
    return reloq_dede61_destress_determiners(state, p0, p1, p2);
}
int32_t reloq_dede61_destress_ein_paar(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_destress_mal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_mal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_mal,args,3,3);
    }
    return reloq_dede61_destress_mal(state, p0, p1, p2);
}
int32_t reloq_dede61_destress_mehr(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_mehr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_mehr,args,6,6);
    }
    return reloq_dede61_destress_mehr(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_destress_pro2(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_pro2(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_pro2,args,6,6);
    }
    return reloq_dede61_destress_pro2(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_destress_sehr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_sehr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_sehr,args,3,3);
    }
    return reloq_dede61_destress_sehr(state, p0, p1, p2);
}
int32_t reloq_dede61_destress_sep_prfxs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_sep_prfxs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_sep_prfxs,args,3,3);
    }
    return reloq_dede61_destress_sep_prfxs(state, p0, p1, p2);
}
int32_t reloq_dede61_destress_verb_in_adv_verb(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_verb_in_adv_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_verb_in_adv_verb,args,5,5);
    }
    return reloq_dede61_destress_verb_in_adv_verb(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_destress_verbs(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_verbs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_verbs,args,6,6);
    }
    return reloq_dede61_destress_verbs(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_destress_zwanziger_words(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_destress_zwanziger_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_zwanziger_words,args,5,5);
    }
    return reloq_dede61_destress_zwanziger_words(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_devoice_obstruent(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_devoice_obstruent(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_devoice_obstruent,args,3,3);
    }
    return reloq_dede61_devoice_obstruent(state, p0, p1, p2);
}
int32_t reloq_dede61_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_dede61_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_disambig_bis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_bis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_bis,args,3,3);
    }
    return reloq_dede61_disambig_bis(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_coord_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_coord_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_coord_adv,args,3,3);
    }
    return reloq_dede61_disambig_coord_adv(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_da(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_da(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_da,args,3,3);
    }
    return reloq_dede61_disambig_da(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_ger_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_ger_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_ger_content_word,args,3,3);
    }
    return reloq_dede61_disambig_ger_content_word(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_ger_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_ger_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_ger_functword,args,3,3);
    }
    return reloq_dede61_disambig_ger_functword(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_halt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_halt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_halt,args,3,3);
    }
    return reloq_dede61_disambig_halt(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_ihr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_ihr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_ihr,args,3,3);
    }
    return reloq_dede61_disambig_ihr(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_meinen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_meinen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_meinen,args,3,3);
    }
    return reloq_dede61_disambig_meinen(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_prep_post(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_prep_post(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_prep_post,args,3,3);
    }
    return reloq_dede61_disambig_prep_post(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_sein(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_sein(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_sein,args,3,3);
    }
    return reloq_dede61_disambig_sein(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_wart(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_wart(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_wart,args,3,3);
    }
    return reloq_dede61_disambig_wart(state, p0, p1, p2);
}
int32_t reloq_dede61_disambig_was(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambig_was(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_was,args,3,3);
    }
    return reloq_dede61_disambig_was(state, p0, p1, p2);
}
int32_t reloq_dede61_disambiguate_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambiguate_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_content_word,args,4,4);
    }
    return reloq_dede61_disambiguate_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_disambiguate_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_disambiguate_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_functword,args,3,3);
    }
    return reloq_dede61_disambiguate_functword(state, p0, p1, p2);
}
int32_t reloq_dede61_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_dede61_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_dede61_distribute_nucdur(state, p0);
}
int32_t reloq_dede61_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_dede61_divide_char_count_token(state, p0, p1);
}
int32_t reloq_dede61_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_dede61_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_dede61_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_dede61_doF(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_dede61_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_dede61_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_dede61_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_dede61_do_FTZ_FTP(state, p0);
}
int32_t reloq_dede61_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_dede61_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_dede61_do_ger_glot_stop(void *, int32_t);
int32_t reloq_dede61_do_ger_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_do_ger_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_ger_h_amps,args,2,2);
    }
    return reloq_dede61_do_ger_h_amps(state, p0, p1);
}
int32_t reloq_dede61_do_glot(void *, int32_t, int32_t);
int32_t reloq_dede61_do_glot_onset(void *, int32_t);
static inline int32_t reloq_call_dede61_do_glot_onset(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_glot_onset,args,1,1);
    }
    return reloq_dede61_do_glot_onset(state, p0);
}
int32_t reloq_dede61_do_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_do_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_h_amps,args,2,2);
    }
    return reloq_dede61_do_h_amps(state, p0, p1);
}
int32_t reloq_dede61_do_morphology(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_do_morphology(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_morphology,args,3,3);
    }
    return reloq_dede61_do_morphology(state, p0, p1, p2);
}
int32_t reloq_dede61_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_dede61_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_dede61_do_non_son_amps(state, p0);
}
int32_t reloq_dede61_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_dede61_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_dede61_do_pt_voicing(state, p0);
}
int32_t reloq_dede61_do_release(void *, int32_t);
static inline int32_t reloq_call_dede61_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_dede61_do_release(state, p0);
}
int32_t reloq_dede61_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_dede61_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_dede61_do_son_amps(state, p0);
}
int32_t reloq_dede61_do_voice_bar(void *, int32_t);
static inline int32_t reloq_call_dede61_do_voice_bar(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_voice_bar,args,1,1);
    }
    return reloq_dede61_do_voice_bar(state, p0);
}
int32_t reloq_dede61_dur_adjustments(void *, int32_t);
static inline int32_t reloq_call_dede61_dur_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_dur_adjustments,args,1,1);
    }
    return reloq_dede61_dur_adjustments(state, p0);
}
int32_t reloq_dede61_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_dede61_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_dede61_ellipsis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ellipsis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ellipsis,args,3,3);
    }
    return reloq_dede61_ellipsis(state, p0, p1, p2);
}
int32_t reloq_dede61_email_address(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_email_address(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_email_address,args,2,2);
    }
    return reloq_dede61_email_address(state, p0, p1);
}
int32_t reloq_dede61_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_dede61_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_dede61_end_morph_analysis(void *, int32_t);
static inline int32_t reloq_call_dede61_end_morph_analysis(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_end_morph_analysis,args,1,1);
    }
    return reloq_dede61_end_morph_analysis(state, p0);
}
int32_t reloq_dede61_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_dede61_end_of_sentence(state, p0, p1);
}
int32_t reloq_dede61_end_of_token(void *, int32_t, int32_t);
int32_t reloq_dede61_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_dede61_end_of_word(state, p0, p1);
}
int32_t reloq_dede61_eng_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_boundary_tone,args,1,1);
    }
    return reloq_dede61_eng_boundary_tone(state, p0);
}
int32_t reloq_dede61_eng_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_first_postnucl_accent,args,1,1);
    }
    return reloq_dede61_eng_first_postnucl_accent(state, p0);
}
int32_t reloq_dede61_eng_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_high_low_star_tone,args,1,1);
    }
    return reloq_dede61_eng_high_low_star_tone(state, p0);
}
int32_t reloq_dede61_eng_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_high_star_low_tone,args,1,1);
    }
    return reloq_dede61_eng_high_star_low_tone(state, p0);
}
int32_t reloq_dede61_eng_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_high_tone,args,1,1);
    }
    return reloq_dede61_eng_high_tone(state, p0);
}
int32_t reloq_dede61_eng_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_low_high_star_tone,args,1,1);
    }
    return reloq_dede61_eng_low_high_star_tone(state, p0);
}
int32_t reloq_dede61_eng_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_low_star_high_tone,args,1,1);
    }
    return reloq_dede61_eng_low_star_high_tone(state, p0);
}
int32_t reloq_dede61_eng_low_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_low_tone,args,1,1);
    }
    return reloq_dede61_eng_low_tone(state, p0);
}
int32_t reloq_dede61_eng_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_dede61_eng_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_dede61_eng_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_dede61_eng_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_dede61_eng_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_high_tone,args,1,1);
    }
    return reloq_dede61_eng_nucl_high_tone(state, p0);
}
int32_t reloq_dede61_eng_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_dede61_eng_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_dede61_eng_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_dede61_eng_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_dede61_eng_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_nucl_low_tone,args,1,1);
    }
    return reloq_dede61_eng_nucl_low_tone(state, p0);
}
int32_t reloq_dede61_eng_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_phr_initial_F0,args,1,1);
    }
    return reloq_dede61_eng_phr_initial_F0(state, p0);
}
int32_t reloq_dede61_eng_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_phrase_tone,args,1,1);
    }
    return reloq_dede61_eng_phrase_tone(state, p0);
}
int32_t reloq_dede61_eng_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_position_t_ptrs,args,1,1);
    }
    return reloq_dede61_eng_position_t_ptrs(state, p0);
}
int32_t reloq_dede61_eng_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_postnucl_f0,args,1,1);
    }
    return reloq_dede61_eng_postnucl_f0(state, p0);
}
int32_t reloq_dede61_eng_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_dede61_eng_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_eng_prenucl_f0,args,1,1);
    }
    return reloq_dede61_eng_prenucl_f0(state, p0);
}
int32_t reloq_dede61_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_dede61_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_estimate_ger_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_estimate_ger_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_ger_nsylls,args,4,4);
    }
    return reloq_dede61_estimate_ger_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_estimate_lang_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_estimate_lang_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_lang_nsylls,args,4,4);
    }
    return reloq_dede61_estimate_lang_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_estimate_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_nsylls,args,4,4);
    }
    return reloq_dede61_estimate_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_exclam_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_exclam_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_exclam_phrase,args,2,2);
    }
    return reloq_dede61_exclam_phrase(state, p0, p1);
}
int32_t reloq_dede61_extend_vals(void *, int32_t);
int32_t reloq_dede61_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_dede61_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_dede61_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_dede61_final_compound_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_final_compound_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_compound_word,args,4,4);
    }
    return reloq_dede61_final_compound_word(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_final_voiced_release(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_final_voiced_release(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_voiced_release,args,3,3);
    }
    return reloq_dede61_final_voiced_release(state, p0, p1, p2);
}
int32_t reloq_dede61_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_dede61_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_dede61_find_acoustic_nuc(state, p0);
}
int32_t reloq_dede61_find_compound(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_compound(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_compound,args,3,3);
    }
    return reloq_dede61_find_compound(state, p0, p1, p2);
}
int32_t reloq_dede61_find_ger_nucleus(void *, int32_t);
static inline int32_t reloq_call_dede61_find_ger_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_ger_nucleus,args,1,1);
    }
    return reloq_dede61_find_ger_nucleus(state, p0);
}
int32_t reloq_dede61_find_ger_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_ger_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_ger_obligatory_phrase,args,4,4);
    }
    return reloq_dede61_find_ger_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_ger_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_ger_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_ger_potential_phrase,args,4,4);
    }
    return reloq_dede61_find_ger_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_lang_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_lang_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_obligatory_phrase,args,4,4);
    }
    return reloq_dede61_find_lang_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_lang_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_lang_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_potential_phrase,args,4,4);
    }
    return reloq_dede61_find_lang_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_dede61_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_most_stressed_syll(void *, int32_t);
static inline int32_t reloq_call_dede61_find_most_stressed_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_most_stressed_syll,args,1,1);
    }
    return reloq_dede61_find_most_stressed_syll(state, p0);
}
int32_t reloq_dede61_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_next_token(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_token,args,4,4);
    }
    return reloq_dede61_find_next_token(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_next_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_word,args,4,4);
    }
    return reloq_dede61_find_next_word(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_dede61_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_dede61_find_nucleus(state, p0);
}
int32_t reloq_dede61_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_dede61_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_previous_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_previous_word,args,4,4);
    }
    return reloq_dede61_find_previous_word(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_find_unpuncted_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_unpuncted_phrases,args,3,3);
    }
    return reloq_dede61_find_unpuncted_phrases(state, p0, p1, p2);
}
int32_t reloq_dede61_find_verb_inflection(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_find_verb_inflection(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_verb_inflection,args,3,3);
    }
    return reloq_dede61_find_verb_inflection(state, p0, p1, p2);
}
int32_t reloq_dede61_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_dede61_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_first_postnucl_accent,args,1,1);
    }
    return reloq_dede61_first_postnucl_accent(state, p0);
}
int32_t reloq_dede61_first_root_vow(void *, int32_t, int32_t);
int32_t reloq_dede61_fix_eins(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_fix_eins(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fix_eins,args,2,2);
    }
    return reloq_dede61_fix_eins(state, p0, p1);
}
int32_t reloq_dede61_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_followed_by_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_followed_by_hyphen,args,3,3);
    }
    return reloq_dede61_followed_by_hyphen(state, p0, p1, p2);
}
int32_t reloq_dede61_follows_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_follows_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_follows_end_quote,args,2,2);
    }
    return reloq_dede61_follows_end_quote(state, p0, p1);
}
int32_t reloq_dede61_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_dede61_forced_synthesis(state, p0, p1);
}
int32_t reloq_dede61_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_dede61_generate_acoustic_vals(state, p0);
}
int32_t reloq_dede61_generate_durations(void *, int32_t);
static inline int32_t reloq_call_dede61_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_dede61_generate_durations(state, p0);
}
int32_t reloq_dede61_generate_ger_diaphons(void *, int32_t);
static inline int32_t reloq_call_dede61_generate_ger_diaphons(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_ger_diaphons,args,1,1);
    }
    return reloq_dede61_generate_ger_diaphons(state, p0);
}
int32_t reloq_dede61_generate_intonation(void *, int32_t);
static inline int32_t reloq_call_dede61_generate_intonation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_intonation,args,1,1);
    }
    return reloq_dede61_generate_intonation(state, p0);
}
int32_t reloq_dede61_ger_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_abbr,args,3,3);
    }
    return reloq_dede61_ger_abbr(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_acrotest,args,3,3);
    }
    return reloq_dede61_ger_acrotest(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_char_name,args,3,3);
    }
    return reloq_dede61_ger_char_name(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_city_country(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_city_country(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_city_country,args,5,5);
    }
    return reloq_dede61_ger_city_country(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_ger_compound_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_compound_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_compound_abbr,args,3,3);
    }
    return reloq_dede61_ger_compound_abbr(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_compound_abbrev(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_compound_abbrev(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_compound_abbrev,args,3,3);
    }
    return reloq_dede61_ger_compound_abbrev(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_compounds(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_compounds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_compounds,args,4,4);
    }
    return reloq_dede61_ger_compounds(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_ger_con_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_con_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_con_durs,args,3,3);
    }
    return reloq_dede61_ger_con_durs(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_con_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_con_vals,args,1,1);
    }
    return reloq_dede61_ger_con_vals(state, p0);
}
int32_t reloq_dede61_ger_funct_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_funct_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_funct_words,args,3,3);
    }
    return reloq_dede61_ger_funct_words(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_gramm_cat_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_gramm_cat_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_gramm_cat_dict,args,3,3);
    }
    return reloq_dede61_ger_gramm_cat_dict(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_homog_roots(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_homog_roots(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_homog_roots,args,3,3);
    }
    return reloq_dede61_ger_homog_roots(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_homog_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_homog_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_homog_words,args,3,3);
    }
    return reloq_dede61_ger_homog_words(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_hyphen_between_nums,args,5,5);
    }
    return reloq_dede61_ger_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_ger_hyphenated_compound(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_hyphenated_compound(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_hyphenated_compound,args,4,4);
    }
    return reloq_dede61_ger_hyphenated_compound(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_ger_inflections(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_inflections(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_inflections,args,3,3);
    }
    return reloq_dede61_ger_inflections(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_monetary_exp,args,3,3);
    }
    return reloq_dede61_ger_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_non_phrasal_comma,args,2,2);
    }
    return reloq_dede61_ger_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_dede61_ger_nonphrasal_comma(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_nonphrasal_comma(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_nonphrasal_comma,args,5,5);
    }
    return reloq_dede61_ger_nonphrasal_comma(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_ger_particles(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_particles(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_particles,args,3,3);
    }
    return reloq_dede61_ger_particles(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_ph_G(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_G(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_G,args,1,1);
    }
    return reloq_dede61_ger_ph_G(state, p0);
}
int32_t reloq_dede61_ger_ph_G_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_G_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_G_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_G_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_S(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_S,args,1,1);
    }
    return reloq_dede61_ger_ph_S(state, p0);
}
int32_t reloq_dede61_ger_ph_S_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_S_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_S_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_S_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_X(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_X(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_X,args,1,1);
    }
    return reloq_dede61_ger_ph_X(state, p0);
}
int32_t reloq_dede61_ger_ph_X_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_X_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_X_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_X_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_Z(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_Z,args,1,1);
    }
    return reloq_dede61_ger_ph_Z(state, p0);
}
int32_t reloq_dede61_ger_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_Z_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_Z_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_b(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_b,args,1,1);
    }
    return reloq_dede61_ger_ph_b(state, p0);
}
int32_t reloq_dede61_ger_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_b_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_b_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_d(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_d,args,1,1);
    }
    return reloq_dede61_ger_ph_d(state, p0);
}
int32_t reloq_dede61_ger_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_d_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_d_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_f(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_f(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_f,args,1,1);
    }
    return reloq_dede61_ger_ph_f(state, p0);
}
int32_t reloq_dede61_ger_ph_f_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_f_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_f_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_f_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_g(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_g,args,1,1);
    }
    return reloq_dede61_ger_ph_g(state, p0);
}
int32_t reloq_dede61_ger_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_g_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_g_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_j(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_j(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_j,args,1,1);
    }
    return reloq_dede61_ger_ph_j(state, p0);
}
int32_t reloq_dede61_ger_ph_j_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_j_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_j_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_j_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_k(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_k,args,1,1);
    }
    return reloq_dede61_ger_ph_k(state, p0);
}
int32_t reloq_dede61_ger_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_k_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_k_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_l(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_l,args,1,1);
    }
    return reloq_dede61_ger_ph_l(state, p0);
}
int32_t reloq_dede61_ger_ph_l_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_l_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_l_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_l_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_m(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_m,args,1,1);
    }
    return reloq_dede61_ger_ph_m(state, p0);
}
int32_t reloq_dede61_ger_ph_m_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_m_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_m_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_m_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_n(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_n,args,1,1);
    }
    return reloq_dede61_ger_ph_n(state, p0);
}
int32_t reloq_dede61_ger_ph_n_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_n_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_n_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_p(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_p,args,1,1);
    }
    return reloq_dede61_ger_ph_p(state, p0);
}
int32_t reloq_dede61_ger_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_p_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_p_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_r(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_r,args,1,1);
    }
    return reloq_dede61_ger_ph_r(state, p0);
}
int32_t reloq_dede61_ger_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_r_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_r_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_s(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_s,args,1,1);
    }
    return reloq_dede61_ger_ph_s(state, p0);
}
int32_t reloq_dede61_ger_ph_s_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_s_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_s_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_s_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_t(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_t,args,1,1);
    }
    return reloq_dede61_ger_ph_t(state, p0);
}
int32_t reloq_dede61_ger_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_t_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_t_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_v(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_v(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_v,args,1,1);
    }
    return reloq_dede61_ger_ph_v(state, p0);
}
int32_t reloq_dede61_ger_ph_v_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_v_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_v_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_v_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_w(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_w,args,1,1);
    }
    return reloq_dede61_ger_ph_w(state, p0);
}
int32_t reloq_dede61_ger_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_w_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_w_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_x(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_x(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_x,args,1,1);
    }
    return reloq_dede61_ger_ph_x(state, p0);
}
int32_t reloq_dede61_ger_ph_x_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_x_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_x_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_x_dur(state, p0);
}
int32_t reloq_dede61_ger_ph_z(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_z,args,1,1);
    }
    return reloq_dede61_ger_ph_z(state, p0);
}
int32_t reloq_dede61_ger_ph_z_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_ph_z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_ph_z_dur,args,1,1);
    }
    return reloq_dede61_ger_ph_z_dur(state, p0);
}
int32_t reloq_dede61_ger_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_post_comma_phrase,args,3,3);
    }
    return reloq_dede61_ger_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_prefixes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_prefixes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_prefixes,args,3,3);
    }
    return reloq_dede61_ger_prefixes(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_prenom_adj_series(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_prenom_adj_series(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_prenom_adj_series,args,2,2);
    }
    return reloq_dede61_ger_prenom_adj_series(state, p0, p1);
}
int32_t reloq_dede61_ger_pseudo_morphs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_pseudo_morphs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_pseudo_morphs,args,3,3);
    }
    return reloq_dede61_ger_pseudo_morphs(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_roots(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_roots(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_roots,args,3,3);
    }
    return reloq_dede61_ger_roots(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_roots_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_roots_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_roots_dict_lookup,args,3,3);
    }
    return reloq_dede61_ger_roots_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_said_phrase,args,4,4);
    }
    return reloq_dede61_ger_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_ger_suffixes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_suffixes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_suffixes,args,3,3);
    }
    return reloq_dede61_ger_suffixes(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_symbolic_rules(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_symbolic_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_symbolic_rules,args,1,1);
    }
    return reloq_dede61_ger_symbolic_rules(state, p0);
}
int32_t reloq_dede61_ger_tag_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_tag_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_tag_phrase,args,3,3);
    }
    return reloq_dede61_ger_tag_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_tok_dict,args,3,3);
    }
    return reloq_dede61_ger_tok_dict(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_trans_features(void *, int32_t);
static inline int32_t reloq_call_dede61_ger_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ger_trans_features,args,1,1);
    }
    return reloq_dede61_ger_trans_features(state, p0);
}
int32_t reloq_dede61_ger_vocative_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_vocative_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_vocative_phrase,args,3,3);
    }
    return reloq_dede61_ger_vocative_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_words,args,3,3);
    }
    return reloq_dede61_ger_words(state, p0, p1, p2);
}
int32_t reloq_dede61_ger_words_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_ger_words_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ger_words_dict_lookup,args,3,3);
    }
    return reloq_dede61_ger_words_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_dede61_get_amp_scale(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_amp_scale(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_amp_scale,args,4,4);
    }
    return reloq_dede61_get_amp_scale(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_dede61_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_dede61_get_ger_fric_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_ger_fric_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ger_fric_trans_dur,args,2,2);
    }
    return reloq_dede61_get_ger_fric_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_get_ger_l_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_ger_l_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ger_l_trans_dur,args,2,2);
    }
    return reloq_dede61_get_ger_l_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_get_ger_nas_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_ger_nas_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ger_nas_trans_dur,args,2,2);
    }
    return reloq_dede61_get_ger_nas_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_get_ger_nuclear_accent(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_ger_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ger_nuclear_accent,args,3,3);
    }
    return reloq_dede61_get_ger_nuclear_accent(state, p0, p1, p2);
}
int32_t reloq_dede61_get_ger_r_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_ger_r_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ger_r_trans_dur,args,2,2);
    }
    return reloq_dede61_get_ger_r_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_get_ger_stop_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_ger_stop_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ger_stop_trans_dur,args,2,2);
    }
    return reloq_dede61_get_ger_stop_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_get_ger_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_ger_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ger_trans_dur,args,2,2);
    }
    return reloq_dede61_get_ger_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_get_ger_vow_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_ger_vow_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ger_vow_trans_dur,args,2,2);
    }
    return reloq_dede61_get_ger_vow_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_get_h_dur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_h_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_h_dur,args,4,4);
    }
    return reloq_dede61_get_h_dur(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_dede61_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_get_input(void *, int32_t);
static inline int32_t reloq_call_dede61_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_dede61_get_input(state, p0);
}
int32_t reloq_dede61_get_lang_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_lang_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_lang_nuclear_accent,args,4,4);
    }
    return reloq_dede61_get_lang_nuclear_accent(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_dede61_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_dede61_get_nuc_AV(state, p0);
}
int32_t reloq_dede61_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_dede61_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_get_number_info(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_number_info(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_info,args,3,3);
    }
    return reloq_dede61_get_number_info(state, p0, p1, p2);
}
int32_t reloq_dede61_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_dede61_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_get_stanger_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_dede61_get_stanger_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanger_nuc_AV,args,1,1);
    }
    return reloq_dede61_get_stanger_nuc_AV(state, p0);
}
int32_t reloq_dede61_get_tok(void *, int32_t);
static inline int32_t reloq_call_dede61_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_dede61_get_tok(state, p0);
}
int32_t reloq_dede61_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_dede61_get_trans_dur(state, p0, p1);
}
int32_t reloq_dede61_gettok(void *, int32_t);
static inline int32_t reloq_call_dede61_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_dede61_gettok(state, p0);
}
int32_t reloq_dede61_gramm_cat_dict_lookup(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_gramm_cat_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_gramm_cat_dict_lookup,args,5,5);
    }
    return reloq_dede61_gramm_cat_dict_lookup(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_dede61_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_handle_alle_words(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_alle_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_alle_words,args,4,4);
    }
    return reloq_dede61_handle_alle_words(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_dann_und_wann(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_dann_und_wann(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_dann_und_wann,args,7,7);
    }
    return reloq_dede61_handle_dann_und_wann(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_dede61_handle_en(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_en(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_en,args,3,3);
    }
    return reloq_dede61_handle_en(state, p0, p1, p2);
}
int32_t reloq_dede61_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_dede61_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_dede61_handle_er(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_er(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_er,args,2,2);
    }
    return reloq_dede61_handle_er(state, p0, p1);
}
int32_t reloq_dede61_handle_es(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_es(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_es,args,3,3);
    }
    return reloq_dede61_handle_es(state, p0, p1, p2);
}
int32_t reloq_dede61_handle_final_adj_or_adv(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_final_adj_or_adv(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_final_adj_or_adv,args,4,4);
    }
    return reloq_dede61_handle_final_adj_or_adv(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_final_be_aux(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_final_be_aux(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_final_be_aux,args,4,4);
    }
    return reloq_dede61_handle_final_be_aux(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_final_noun(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_final_noun(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_final_noun,args,4,4);
    }
    return reloq_dede61_handle_final_noun(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_final_prep_phr(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_final_prep_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_final_prep_phr,args,6,6);
    }
    return reloq_dede61_handle_final_prep_phr(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_handle_final_verb(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_final_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_final_verb,args,5,5);
    }
    return reloq_dede61_handle_final_verb(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_handle_final_verb_verb(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_final_verb_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_final_verb_verb,args,4,4);
    }
    return reloq_dede61_handle_final_verb_verb(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_im_Jahr_zuvor(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_im_Jahr_zuvor(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_im_Jahr_zuvor,args,7,7);
    }
    return reloq_dede61_handle_im_Jahr_zuvor(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_dede61_handle_insep_zu_inf(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_insep_zu_inf(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_insep_zu_inf,args,5,5);
    }
    return reloq_dede61_handle_insep_zu_inf(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_handle_intransitives(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_intransitives(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_intransitives,args,4,4);
    }
    return reloq_dede61_handle_intransitives(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_dede61_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_dede61_handle_nach_wie_vor(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_nach_wie_vor(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_nach_wie_vor,args,7,7);
    }
    return reloq_dede61_handle_nach_wie_vor(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_dede61_handle_nicht_einmal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_nicht_einmal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_nicht_einmal,args,5,5);
    }
    return reloq_dede61_handle_nicht_einmal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_handle_non_phrasal_commas(void *, int32_t);
static inline int32_t reloq_call_dede61_handle_non_phrasal_commas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_phrasal_commas,args,1,1);
    }
    return reloq_dede61_handle_non_phrasal_commas(state, p0);
}
int32_t reloq_dede61_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_dede61_handle_pause(state, p0, p1);
}
int32_t reloq_dede61_handle_prep_pro_verb(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_prep_pro_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_prep_pro_verb,args,6,6);
    }
    return reloq_dede61_handle_prep_pro_verb(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_handle_s(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_s(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_s,args,2,2);
    }
    return reloq_dede61_handle_s(state, p0, p1);
}
int32_t reloq_dede61_handle_split_adj_or_adv_compounds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_split_adj_or_adv_compounds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_split_adj_or_adv_compounds,args,3,3);
    }
    return reloq_dede61_handle_split_adj_or_adv_compounds(state, p0, p1, p2);
}
int32_t reloq_dede61_handle_split_adjective_compounds(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_split_adjective_compounds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_split_adjective_compounds,args,4,4);
    }
    return reloq_dede61_handle_split_adjective_compounds(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_split_adv_adj_compounds(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_split_adv_adj_compounds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_split_adv_adj_compounds,args,5,5);
    }
    return reloq_dede61_handle_split_adv_adj_compounds(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_handle_split_noun_compounds(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_split_noun_compounds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_split_noun_compounds,args,4,4);
    }
    return reloq_dede61_handle_split_noun_compounds(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_split_prespart_compounds(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_split_prespart_compounds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_split_prespart_compounds,args,4,4);
    }
    return reloq_dede61_handle_split_prespart_compounds(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_split_verb_verb_compounds(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_split_verb_verb_compounds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_split_verb_verb_compounds,args,4,4);
    }
    return reloq_dede61_handle_split_verb_verb_compounds(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_handle_voll_expressions(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_voll_expressions(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_voll_expressions,args,5,5);
    }
    return reloq_dede61_handle_voll_expressions(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_handle_von_Anfang_an(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_handle_von_Anfang_an(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_von_Anfang_an,args,8,8);
    }
    return reloq_dede61_handle_von_Anfang_an(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_dede61_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_low_star_tone,args,1,1);
    }
    return reloq_dede61_high_low_star_tone(state, p0);
}
int32_t reloq_dede61_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_star_low_tone,args,1,1);
    }
    return reloq_dede61_high_star_low_tone(state, p0);
}
int32_t reloq_dede61_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_tone,args,1,1);
    }
    return reloq_dede61_high_tone(state, p0);
}
int32_t reloq_dede61_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen,args,3,3);
    }
    return reloq_dede61_hyphen(state, p0, p1, p2);
}
int32_t reloq_dede61_hyphen_between_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen_between_nums,args,3,3);
    }
    return reloq_dede61_hyphen_between_nums(state, p0, p1, p2);
}
int32_t reloq_dede61_hyphenated_compound(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_hyphenated_compound(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphenated_compound,args,2,2);
    }
    return reloq_dede61_hyphenated_compound(state, p0, p1);
}
int32_t reloq_dede61_identify_ger_year(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_identify_ger_year(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_identify_ger_year,args,3,3);
    }
    return reloq_dede61_identify_ger_year(state, p0, p1, p2);
}
int32_t reloq_dede61_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_dede61_in_brackets(state, p0, p1, p2);
}
int32_t reloq_dede61_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_dede61_in_quotes(state, p0, p1, p2);
}
int32_t reloq_dede61_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_in_syllable(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_syllable,args,4,4);
    }
    return reloq_dede61_in_syllable(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_dede61_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_dede61_increment_output_file_number(state, p0);
}
int32_t reloq_dede61_infinitive_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_infinitive_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_infinitive_phrase,args,4,4);
    }
    return reloq_dede61_infinitive_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_init_current_word_ptrs(void *, int32_t);
int32_t reloq_dede61_init_delta(void *, int32_t);
static inline int32_t reloq_call_dede61_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_dede61_init_delta(state, p0);
}
int32_t reloq_dede61_init_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_gcat_vars,args,1,1);
    }
    return reloq_dede61_init_gcat_vars(state, p0);
}
int32_t reloq_dede61_init_ger_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_ger_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_ger_vars,args,1,1);
    }
    return reloq_dede61_init_ger_vars(state, p0);
}
int32_t reloq_dede61_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_dede61_init_lang_vars(state, p0);
}
int32_t reloq_dede61_init_nuclwrd_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_nuclwrd_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_nuclwrd_vars,args,1,1);
    }
    return reloq_dede61_init_nuclwrd_vars(state, p0);
}
int32_t reloq_dede61_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_dede61_init_phr_feature_vars(state, p0);
}
int32_t reloq_dede61_init_phrase_inton_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_phrase_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_inton_vars,args,1,1);
    }
    return reloq_dede61_init_phrase_inton_vars(state, p0);
}
int32_t reloq_dede61_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_dede61_init_phrase_vars(state, p0);
}
int32_t reloq_dede61_init_platform(void *, int32_t);
static inline int32_t reloq_call_dede61_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_dede61_init_platform(state, p0);
}
int32_t reloq_dede61_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_dede61_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_dede61_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_dede61_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_dede61_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_inton_vars,args,1,1);
    }
    return reloq_dede61_init_syll_inton_vars(state, p0);
}
int32_t reloq_dede61_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_dede61_init_syll_vars(state, p0);
}
int32_t reloq_dede61_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_dede61_init_symbolic_vars(state, p0);
}
int32_t reloq_dede61_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_dede61_init_tm_vars(state, p0);
}
int32_t reloq_dede61_init_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_dede61_init_vars(state, p0);
}
int32_t reloq_dede61_init_voice_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_voice_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_voice_vars,args,1,1);
    }
    return reloq_dede61_init_voice_vars(state, p0);
}
int32_t reloq_dede61_init_word(void *, int32_t);
static inline int32_t reloq_call_dede61_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_dede61_init_word(state, p0);
}
int32_t reloq_dede61_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_inton_vars,args,1,1);
    }
    return reloq_dede61_init_word_inton_vars(state, p0);
}
int32_t reloq_dede61_init_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_dede61_init_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_ptrs,args,1,1);
    }
    return reloq_dede61_init_word_ptrs(state, p0);
}
int32_t reloq_dede61_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_dede61_init_word_vars(state, p0);
}
int32_t reloq_dede61_initial_compound_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_initial_compound_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_initial_compound_word,args,4,4);
    }
    return reloq_dede61_initial_compound_word(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_dede61_insert_acoustic_vals(state, p0);
}
int32_t reloq_dede61_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_dede61_insert_burst_dur(state, p0);
}
int32_t reloq_dede61_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_dede61_insert_char_count(state, p0);
}
int32_t reloq_dede61_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_comma_phrase,args,3,3);
    }
    return reloq_dede61_insert_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_dede61_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_insert_f0(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_f0(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_f0,args,4,4);
    }
    return reloq_dede61_insert_f0(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_insert_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_insert_ger_particle_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_ger_particle_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_ger_particle_stress,args,2,2);
    }
    return reloq_dede61_insert_ger_particle_stress(state, p0, p1);
}
int32_t reloq_dede61_insert_ger_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_ger_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_ger_spr_phone,args,3,3);
    }
    return reloq_dede61_insert_ger_spr_phone(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_insert_long_vowel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_long_vowel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_long_vowel,args,3,3);
    }
    return reloq_dede61_insert_long_vowel(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_dede61_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_dede61_insert_pause_durs(state, p0);
}
int32_t reloq_dede61_insert_penult_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_dede61_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_final_tones,args,1,1);
    }
    return reloq_dede61_insert_phrase_final_tones(state, p0);
}
int32_t reloq_dede61_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_dede61_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_preset_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_preset_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_preset_phrase_tone,args,1,1);
    }
    return reloq_dede61_insert_preset_phrase_tone(state, p0);
}
int32_t reloq_dede61_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_insert_root_stress_value(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_root_stress_value(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_root_stress_value,args,3,3);
    }
    return reloq_dede61_insert_root_stress_value(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_short_vowel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_short_vowel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_short_vowel,args,3,3);
    }
    return reloq_dede61_insert_short_vowel(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_dede61_insert_space(state, p0, p1);
}
int32_t reloq_dede61_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_dede61_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_dede61_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_dede61_insert_syll_level_vals(state, p0);
}
int32_t reloq_dede61_insert_target_t3(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_target_t3(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_target_t3,args,1,1);
    }
    return reloq_dede61_insert_target_t3(state, p0);
}
int32_t reloq_dede61_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_dede61_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_dede61_insert_transitions(state, p0);
}
int32_t reloq_dede61_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_dede61_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrases,args,4,4);
    }
    return reloq_dede61_insert_unpuncted_phrases(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_dede61_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_dede61_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_dede61_insert_word(state, p0, p1, p2);
}
int32_t reloq_dede61_insert_zero_sylls(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_insert_zero_sylls(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_zero_sylls,args,3,3);
    }
    return reloq_dede61_insert_zero_sylls(state, p0, p1, p2);
}
int32_t reloq_dede61_inside_orig_inp_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_inside_orig_inp_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_inside_orig_inp_token,args,2,2);
    }
    return reloq_dede61_inside_orig_inp_token(state, p0, p1);
}
int32_t reloq_dede61_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_dede61_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_dede61_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_interpret_single_char_modes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_interpret_single_char_modes,args,3,3);
    }
    return reloq_dede61_interpret_single_char_modes(state, p0, p1, p2);
}
int32_t reloq_dede61_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_dede61_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_dede61_is_accented_prefix_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_accented_prefix_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_accented_prefix_verb,args,3,3);
    }
    return reloq_dede61_is_accented_prefix_verb(state, p0, p1, p2);
}
int32_t reloq_dede61_is_accented_prep(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_accented_prep(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_accented_prep,args,4,4);
    }
    return reloq_dede61_is_accented_prep(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_is_adverb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_adverb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_adverb,args,3,3);
    }
    return reloq_dede61_is_adverb(state, p0, p1, p2);
}
int32_t reloq_dede61_is_after_h(void *, int32_t, int32_t);
int32_t reloq_dede61_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_dede61_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_dede61_is_attributive_adj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_attributive_adj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_attributive_adj,args,3,3);
    }
    return reloq_dede61_is_attributive_adj(state, p0, p1, p2);
}
int32_t reloq_dede61_is_chen_not_en(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_chen_not_en(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_chen_not_en,args,3,3);
    }
    return reloq_dede61_is_chen_not_en(state, p0, p1, p2);
}
int32_t reloq_dede61_is_compound_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_compound_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_compound_verb,args,3,3);
    }
    return reloq_dede61_is_compound_verb(state, p0, p1, p2);
}
int32_t reloq_dede61_is_destressed_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_destressed_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_destressed_verb,args,3,3);
    }
    return reloq_dede61_is_destressed_verb(state, p0, p1, p2);
}
int32_t reloq_dede61_is_excluded_noun_construct(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_excluded_noun_construct(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_excluded_noun_construct,args,2,2);
    }
    return reloq_dede61_is_excluded_noun_construct(state, p0, p1);
}
int32_t reloq_dede61_is_hyphenated_word(void *, int32_t, int32_t);
int32_t reloq_dede61_is_in_parentheses(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_is_inflected_adj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_inflected_adj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_inflected_adj,args,3,3);
    }
    return reloq_dede61_is_inflected_adj(state, p0, p1, p2);
}
int32_t reloq_dede61_is_inflected_past_part(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_inflected_past_part(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_inflected_past_part,args,3,3);
    }
    return reloq_dede61_is_inflected_past_part(state, p0, p1, p2);
}
int32_t reloq_dede61_is_inflected_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_inflected_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_inflected_verb,args,3,3);
    }
    return reloq_dede61_is_inflected_verb(state, p0, p1, p2);
}
int32_t reloq_dede61_is_noun(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_noun(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_noun,args,3,3);
    }
    return reloq_dede61_is_noun(state, p0, p1, p2);
}
int32_t reloq_dede61_is_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_number,args,3,3);
    }
    return reloq_dede61_is_number(state, p0, p1, p2);
}
int32_t reloq_dede61_is_pathname(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_pathname(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pathname,args,2,2);
    }
    return reloq_dede61_is_pathname(state, p0, p1);
}
int32_t reloq_dede61_is_plural_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_plural_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_plural_abbr,args,2,2);
    }
    return reloq_dede61_is_plural_abbr(state, p0, p1);
}
int32_t reloq_dede61_is_predicate_adj_or_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_predicate_adj_or_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_predicate_adj_or_adv,args,3,3);
    }
    return reloq_dede61_is_predicate_adj_or_adv(state, p0, p1, p2);
}
int32_t reloq_dede61_is_proper_name(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_proper_name(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_proper_name,args,2,2);
    }
    return reloq_dede61_is_proper_name(state, p0, p1);
}
int32_t reloq_dede61_is_separable_prefix(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_separable_prefix(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_separable_prefix,args,3,3);
    }
    return reloq_dede61_is_separable_prefix(state, p0, p1, p2);
}
int32_t reloq_dede61_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_dede61_is_spr(state, p0, p1, p2);
}
int32_t reloq_dede61_is_target_syll(void *, int32_t);
static inline int32_t reloq_call_dede61_is_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_target_syll,args,1,1);
    }
    return reloq_dede61_is_target_syll(state, p0);
}
int32_t reloq_dede61_is_uninflected_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_is_uninflected_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_uninflected_verb,args,3,3);
    }
    return reloq_dede61_is_uninflected_verb(state, p0, p1, p2);
}
int32_t reloq_dede61_isitAlt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_isitAlt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitAlt,args,3,3);
    }
    return reloq_dede61_isitAlt(state, p0, p1, p2);
}
int32_t reloq_dede61_isitWH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_isitWH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitWH,args,3,3);
    }
    return reloq_dede61_isitWH(state, p0, p1, p2);
}
int32_t reloq_dede61_isit_ger_Alt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_isit_ger_Alt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_ger_Alt,args,3,3);
    }
    return reloq_dede61_isit_ger_Alt(state, p0, p1, p2);
}
int32_t reloq_dede61_isit_ger_WH(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_isit_ger_WH(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_ger_WH,args,2,2);
    }
    return reloq_dede61_isit_ger_WH(state, p0, p1);
}
int32_t reloq_dede61_lang_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_lang_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_hyphen_between_nums,args,5,5);
    }
    return reloq_dede61_lang_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_lang_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_lang_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_non_phrasal_comma,args,2,2);
    }
    return reloq_dede61_lang_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_dede61_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_lang_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_phrase_level_rules,args,3,3);
    }
    return reloq_dede61_lang_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_dede61_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_dede61_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_dede61_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_dede61_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_dede61_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_dede61_longdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_longdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_longdash,args,3,3);
    }
    return reloq_dede61_longdash(state, p0, p1, p2);
}
int32_t reloq_dede61_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_high_star_tone,args,1,1);
    }
    return reloq_dede61_low_high_star_tone(state, p0);
}
int32_t reloq_dede61_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_star_high_tone,args,1,1);
    }
    return reloq_dede61_low_star_high_tone(state, p0);
}
int32_t reloq_dede61_low_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_tone,args,1,1);
    }
    return reloq_dede61_low_tone(state, p0);
}
int32_t reloq_dede61_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_make_ger_phon_adjustments(void *, int32_t);
static inline int32_t reloq_call_dede61_make_ger_phon_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_make_ger_phon_adjustments,args,1,1);
    }
    return reloq_dede61_make_ger_phon_adjustments(state, p0);
}
int32_t reloq_dede61_mark_adv_subcat(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_adv_subcat(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_adv_subcat,args,3,3);
    }
    return reloq_dede61_mark_adv_subcat(state, p0, p1, p2);
}
int32_t reloq_dede61_mark_conjoined_compound(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_conjoined_compound(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_conjoined_compound,args,5,5);
    }
    return reloq_dede61_mark_conjoined_compound(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_mark_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_content_word,args,5,5);
    }
    return reloq_dede61_mark_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_mark_function_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_function_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_function_word,args,5,5);
    }
    return reloq_dede61_mark_function_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_mark_ger_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_ger_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_ger_content_word,args,3,3);
    }
    return reloq_dede61_mark_ger_content_word(state, p0, p1, p2);
}
int32_t reloq_dede61_mark_ger_subcategory(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_ger_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_ger_subcategory,args,3,3);
    }
    return reloq_dede61_mark_ger_subcategory(state, p0, p1, p2);
}
int32_t reloq_dede61_mark_h(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_h(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_h,args,3,3);
    }
    return reloq_dede61_mark_h(state, p0, p1, p2);
}
int32_t reloq_dede61_mark_non_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_non_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_non_letter,args,3,3);
    }
    return reloq_dede61_mark_non_letter(state, p0, p1, p2);
}
int32_t reloq_dede61_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_dede61_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_mark_potential_phrase_breaks(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_potential_phrase_breaks(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_potential_phrase_breaks,args,4,4);
    }
    return reloq_dede61_mark_potential_phrase_breaks(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_spr_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_spr_syll_vals,args,6,6);
    }
    return reloq_dede61_mark_spr_syll_vals(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_mark_subcategory(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_subcategory,args,4,4);
    }
    return reloq_dede61_mark_subcategory(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_mark_syll_stress(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_mark_uppercase(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_mark_word_categories(void *, int32_t);
static inline int32_t reloq_call_dede61_mark_word_categories(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_categories,args,1,1);
    }
    return reloq_dede61_mark_word_categories(state, p0);
}
int32_t reloq_dede61_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_word_str_annot(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_str_annot,args,3,3);
    }
    return reloq_dede61_mark_word_str_annot(state, p0, p1, p2);
}
int32_t reloq_dede61_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_dede61_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_dede61_mark_zu_infinitive(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_mark_zu_infinitive(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_zu_infinitive,args,3,3);
    }
    return reloq_dede61_mark_zu_infinitive(state, p0, p1, p2);
}
int32_t reloq_dede61_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_dede61_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_dede61_minimal_root(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_minimal_root(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_minimal_root,args,2,2);
    }
    return reloq_dede61_minimal_root(state, p0, p1);
}
int32_t reloq_dede61_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_dede61_misc_chars(state, p0, p1, p2);
}
int32_t reloq_dede61_modify_breathiness_for_femV(void *, int32_t);
static inline int32_t reloq_call_dede61_modify_breathiness_for_femV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modify_breathiness_for_femV,args,1,1);
    }
    return reloq_dede61_modify_breathiness_for_femV(state, p0);
}
int32_t reloq_dede61_modulate_nas_AV(void *, int32_t);
static inline int32_t reloq_call_dede61_modulate_nas_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_nas_AV,args,1,1);
    }
    return reloq_dede61_modulate_nas_AV(state, p0);
}
int32_t reloq_dede61_modulate_noise(void *, int32_t);
int32_t reloq_dede61_modulate_pwind(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_modulate_pwind(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_pwind,args,2,2);
    }
    return reloq_dede61_modulate_pwind(state, p0, p1);
}
int32_t reloq_dede61_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_dede61_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_monetary_exp,args,3,3);
    }
    return reloq_dede61_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_dede61_nasalize_vow(void *, int32_t);
static inline int32_t reloq_call_dede61_nasalize_vow(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nasalize_vow,args,1,1);
    }
    return reloq_dede61_nasalize_vow(state, p0);
}
int32_t reloq_dede61_no_acc_in_continuation_phr(void *, int32_t);
static inline int32_t reloq_call_dede61_no_acc_in_continuation_phr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_acc_in_continuation_phr,args,1,1);
    }
    return reloq_dede61_no_acc_in_continuation_phr(state, p0);
}
int32_t reloq_dede61_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_dede61_no_readin(state, p0, p1);
}
int32_t reloq_dede61_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_dede61_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_dede61_no_words_in_delta(state, p0);
}
int32_t reloq_dede61_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_dede61_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_dede61_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_dede61_normalize_text(state, p0, p1, p2);
}
int32_t reloq_dede61_noun_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_noun_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_phrase,args,6,6);
    }
    return reloq_dede61_noun_phrase(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_noun_vs_adj_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_noun_vs_adj_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_adj_adv,args,3,3);
    }
    return reloq_dede61_noun_vs_adj_adv(state, p0, p1, p2);
}
int32_t reloq_dede61_noun_vs_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_noun_vs_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_adv,args,3,3);
    }
    return reloq_dede61_noun_vs_adv(state, p0, p1, p2);
}
int32_t reloq_dede61_noun_vs_prep(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_noun_vs_prep(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_prep,args,3,3);
    }
    return reloq_dede61_noun_vs_prep(state, p0, p1, p2);
}
int32_t reloq_dede61_nucl_acc_t1pos(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_nucl_acc_t1pos(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_nucl_acc_t1pos,args,2,2);
    }
    return reloq_dede61_nucl_acc_t1pos(state, p0, p1);
}
int32_t reloq_dede61_nucl_f0(void *, int32_t);
static inline int32_t reloq_call_dede61_nucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_f0,args,1,1);
    }
    return reloq_dede61_nucl_f0(state, p0);
}
int32_t reloq_dede61_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_dede61_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_dede61_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_dede61_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_dede61_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_tone,args,1,1);
    }
    return reloq_dede61_nucl_high_tone(state, p0);
}
int32_t reloq_dede61_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_dede61_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_dede61_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_dede61_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_dede61_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_tone,args,1,1);
    }
    return reloq_dede61_nucl_low_tone(state, p0);
}
int32_t reloq_dede61_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_dede61_nucleus_vals(state, p0);
}
int32_t reloq_dede61_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_dede61_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_dede61_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_object_pronoun(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_object_pronoun(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_object_pronoun,args,5,5);
    }
    return reloq_dede61_object_pronoun(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_one_ger_syllable(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_one_ger_syllable(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_one_ger_syllable,args,3,3);
    }
    return reloq_dede61_one_ger_syllable(state, p0, p1, p2);
}
int32_t reloq_dede61_parenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_parenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_parenthesis,args,3,3);
    }
    return reloq_dede61_parenthesis(state, p0, p1, p2);
}
int32_t reloq_dede61_particle_resyllabification(void *, int32_t);
static inline int32_t reloq_call_dede61_particle_resyllabification(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_particle_resyllabification,args,1,1);
    }
    return reloq_dede61_particle_resyllabification(state, p0);
}
int32_t reloq_dede61_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_dede61_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_dede61_period_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_dede61_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_dede61_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phr_initial_F0,args,1,1);
    }
    return reloq_dede61_phr_initial_F0(state, p0);
}
int32_t reloq_dede61_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_dede61_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_phrase_final_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_phrase_final_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_final_dur,args,1,1);
    }
    return reloq_dede61_phrase_final_dur(state, p0);
}
int32_t reloq_dede61_phrase_level_printouts(void *, int32_t);
static inline int32_t reloq_call_dede61_phrase_level_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_printouts,args,1,1);
    }
    return reloq_dede61_phrase_level_printouts(state, p0);
}
int32_t reloq_dede61_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_dede61_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_phrase_medial_dur(void *, int32_t);
static inline int32_t reloq_call_dede61_phrase_medial_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_medial_dur,args,1,1);
    }
    return reloq_dede61_phrase_medial_dur(state, p0);
}
int32_t reloq_dede61_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_tone,args,1,1);
    }
    return reloq_dede61_phrase_tone(state, p0);
}
int32_t reloq_dede61_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_dede61_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_position_t_ptrs,args,1,1);
    }
    return reloq_dede61_position_t_ptrs(state, p0);
}
int32_t reloq_dede61_possible_final_compound(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_possible_final_compound(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_possible_final_compound,args,3,3);
    }
    return reloq_dede61_possible_final_compound(state, p0, p1, p2);
}
int32_t reloq_dede61_possible_initial_compound(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_possible_initial_compound(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_possible_initial_compound,args,3,3);
    }
    return reloq_dede61_possible_initial_compound(state, p0, p1, p2);
}
int32_t reloq_dede61_possible_particle_strip(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_possible_particle_strip(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_possible_particle_strip,args,3,3);
    }
    return reloq_dede61_possible_particle_strip(state, p0, p1, p2);
}
int32_t reloq_dede61_possible_prefix_strip(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_possible_prefix_strip(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_possible_prefix_strip,args,3,3);
    }
    return reloq_dede61_possible_prefix_strip(state, p0, p1, p2);
}
int32_t reloq_dede61_possible_suffix_strip(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_possible_suffix_strip(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_possible_suffix_strip,args,3,3);
    }
    return reloq_dede61_possible_suffix_strip(state, p0, p1, p2);
}
int32_t reloq_dede61_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_comma_phrase,args,3,3);
    }
    return reloq_dede61_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_dede61_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_postnucl_f0,args,1,1);
    }
    return reloq_dede61_postnucl_f0(state, p0);
}
int32_t reloq_dede61_postnucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_postnucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_postnucl_high_tone,args,1,1);
    }
    return reloq_dede61_postnucl_high_tone(state, p0);
}
int32_t reloq_dede61_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_dede61_potential_sent_end(state, p0, p1);
}
int32_t reloq_dede61_precedes_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_precedes_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_end_quote,args,2,2);
    }
    return reloq_dede61_precedes_end_quote(state, p0, p1);
}
int32_t reloq_dede61_precedes_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_precedes_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_number,args,3,3);
    }
    return reloq_dede61_precedes_number(state, p0, p1, p2);
}
int32_t reloq_dede61_preceding_disk_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_preceding_disk_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_preceding_disk_word,args,2,2);
    }
    return reloq_dede61_preceding_disk_word(state, p0, p1);
}
int32_t reloq_dede61_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_dede61_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_prenucl_f0,args,1,1);
    }
    return reloq_dede61_prenucl_f0(state, p0);
}
int32_t reloq_dede61_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_dede61_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_dede61_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_dede61_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_dede61_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_dede61_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_dede61_print_ger_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_print_ger_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_ger_SPR,args,3,3);
    }
    return reloq_dede61_print_ger_SPR(state, p0, p1, p2);
}
int32_t reloq_dede61_print_interword_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_print_interword_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_interword_chars,args,3,3);
    }
    return reloq_dede61_print_interword_chars(state, p0, p1, p2);
}
int32_t reloq_dede61_print_normalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_print_normalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_normalized_inp,args,3,3);
    }
    return reloq_dede61_print_normalized_inp(state, p0, p1, p2);
}
int32_t reloq_dede61_print_prompt(void *, int32_t);
static inline int32_t reloq_call_dede61_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_dede61_print_prompt(state, p0);
}
int32_t reloq_dede61_print_spr(void *, int32_t);
static inline int32_t reloq_call_dede61_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_dede61_print_spr(state, p0);
}
int32_t reloq_dede61_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_dede61_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_dede61_print_word_stress(state, p0, p1);
}
int32_t reloq_dede61_printouts(void *, int32_t);
static inline int32_t reloq_call_dede61_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_dede61_printouts(state, p0);
}
int32_t reloq_dede61_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_dede61_process_annotation(state, p0, p1, p2);
}
int32_t reloq_dede61_process_final_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_final_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_apostrophe,args,3,3);
    }
    return reloq_dede61_process_final_apostrophe(state, p0, p1, p2);
}
int32_t reloq_dede61_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_final_parenthetical(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_parenthetical,args,3,3);
    }
    return reloq_dede61_process_final_parenthetical(state, p0, p1, p2);
}
int32_t reloq_dede61_process_hostname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_hostname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_hostname,args,3,3);
    }
    return reloq_dede61_process_hostname(state, p0, p1, p2);
}
int32_t reloq_dede61_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_dede61_process_input(state, p0, p1);
}
int32_t reloq_dede61_process_number_compound(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_number_compound(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_number_compound,args,4,4);
    }
    return reloq_dede61_process_number_compound(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_process_pathname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_pathname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_pathname,args,3,3);
    }
    return reloq_dede61_process_pathname(state, p0, p1, p2);
}
int32_t reloq_dede61_process_trailing_letters(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_trailing_letters(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_letters,args,4,4);
    }
    return reloq_dede61_process_trailing_letters(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_dede61_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_dede61_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_dede61_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_pronoun_pro2(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_pronoun_pro2(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pronoun_pro2,args,6,6);
    }
    return reloq_dede61_pronoun_pro2(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_dede61_punctuation(state, p0, p1, p2);
}
int32_t reloq_dede61_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_dede61_question_phrase(state, p0, p1, p2);
}
int32_t reloq_dede61_quote_mark(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_quote_mark(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_quote_mark,args,3,3);
    }
    return reloq_dede61_quote_mark(state, p0, p1, p2);
}
int32_t reloq_dede61_read_end(void *, int32_t);
static inline int32_t reloq_call_dede61_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_dede61_read_end(state, p0);
}
int32_t reloq_dede61_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_dede61_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_dede61_realize_prenucl_tone(void *, int32_t);
static inline int32_t reloq_call_dede61_realize_prenucl_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_realize_prenucl_tone,args,1,1);
    }
    return reloq_dede61_realize_prenucl_tone(state, p0);
}
int32_t reloq_dede61_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_dede61_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_dede61_register_SAPI_voices(state, p0);
}
int32_t reloq_dede61_register_ger_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_register_ger_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_ger_phones,args,3,3);
    }
    return reloq_dede61_register_ger_phones(state, p0, p1, p2);
}
int32_t reloq_dede61_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_dede61_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_relative_clause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_relative_clause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_relative_clause,args,3,3);
    }
    return reloq_dede61_relative_clause(state, p0, p1, p2);
}
int32_t reloq_dede61_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_reset_endptr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_endptr,args,4,4);
    }
    return reloq_dede61_reset_endptr(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_reset_enum_ptr(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_reset_enum_ptr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_enum_ptr,args,2,2);
    }
    return reloq_dede61_reset_enum_ptr(state, p0, p1);
}
int32_t reloq_dede61_reset_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_reset_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_gcat_vars,args,1,1);
    }
    return reloq_dede61_reset_gcat_vars(state, p0);
}
int32_t reloq_dede61_reset_ger_sent_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_reset_ger_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_ger_sent_vars,args,1,1);
    }
    return reloq_dede61_reset_ger_sent_vars(state, p0);
}
int32_t reloq_dede61_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_dede61_reset_lang_sent_vars(state, p0);
}
int32_t reloq_dede61_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_dede61_reset_phrase_vars(state, p0);
}
int32_t reloq_dede61_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_dede61_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_dede61_reset_sent_vars(state, p0);
}
int32_t reloq_dede61_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_dede61_reset_token_vars(state, p0, p1);
}
int32_t reloq_dede61_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_dede61_reset_word_vals(state, p0, p1);
}
int32_t reloq_dede61_root_particle(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_root_particle(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_root_particle,args,3,3);
    }
    return reloq_dede61_root_particle(state, p0, p1, p2);
}
int32_t reloq_dede61_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_dede61_run_filters(state, p0, p1, p2);
}
int32_t reloq_dede61_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_dede61_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_dede61_run_speech_module(state, p0);
}
int32_t reloq_dede61_run_text_module(void *, int32_t);
static inline int32_t reloq_call_dede61_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_dede61_run_text_module(state, p0);
}
int32_t reloq_dede61_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_said_phrase,args,4,4);
    }
    return reloq_dede61_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_dede61_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_dede61_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_dede61_scale_word_final_trans(state, p0);
}
int32_t reloq_dede61_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_dede61_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_dede61_semicolon_phrase(state, p0);
}
int32_t reloq_dede61_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_dede61_separate_tokens(state, p0, p1);
}
int32_t reloq_dede61_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_dede61_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_dede61_set_f0_range_ptrs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_set_f0_range_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_range_ptrs,args,5,5);
    }
    return reloq_dede61_set_f0_range_ptrs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_dede61_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_dede61_set_f0_wind_and_base(state, p0);
}
int32_t reloq_dede61_set_first_postnuc_value(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_set_first_postnuc_value(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_first_postnuc_value,args,2,2);
    }
    return reloq_dede61_set_first_postnuc_value(state, p0, p1);
}
int32_t reloq_dede61_set_ger_global_constants(void *, int32_t);
static inline int32_t reloq_call_dede61_set_ger_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_ger_global_constants,args,1,1);
    }
    return reloq_dede61_set_ger_global_constants(state, p0);
}
int32_t reloq_dede61_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_dede61_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_dede61_set_global_constants(state, p0);
}
int32_t reloq_dede61_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_dede61_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_dede61_set_klattparms(state, p0);
}
int32_t reloq_dede61_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_dede61_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_dede61_set_lang_global_constants(state, p0);
}
int32_t reloq_dede61_set_letter_name_wordcats(void *, int32_t);
static inline int32_t reloq_call_dede61_set_letter_name_wordcats(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_letter_name_wordcats,args,1,1);
    }
    return reloq_dede61_set_letter_name_wordcats(state, p0);
}
int32_t reloq_dede61_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_dede61_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_dede61_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_dede61_set_num_formants(state, p0);
}
int32_t reloq_dede61_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_dede61_set_phone_offset_vals(state, p0);
}
int32_t reloq_dede61_set_phr_fin_form_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_set_phr_fin_form_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phr_fin_form_vals,args,1,1);
    }
    return reloq_dede61_set_phr_fin_form_vals(state, p0);
}
int32_t reloq_dede61_set_phr_init_form_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_set_phr_init_form_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phr_init_form_vals,args,1,1);
    }
    return reloq_dede61_set_phr_init_form_vals(state, p0);
}
int32_t reloq_dede61_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_dede61_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_dede61_set_said_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_set_said_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_said_phr_vals,args,3,3);
    }
    return reloq_dede61_set_said_phr_vals(state, p0, p1, p2);
}
int32_t reloq_dede61_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_dede61_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_dede61_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_dede61_set_syll_offset_vals(state, p0);
}
int32_t reloq_dede61_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_dede61_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_dede61_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_dede61_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_dede61_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_dede61_set_voice_param_constants(state, p0);
}
int32_t reloq_dede61_set_window_size(void *, int32_t);
static inline int32_t reloq_call_dede61_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_dede61_set_window_size(state, p0);
}
int32_t reloq_dede61_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_dede61_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_dede61_setdur(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_dede61_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_dede61_shift_tone(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_shift_tone(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_shift_tone,args,2,2);
    }
    return reloq_dede61_shift_tone(state, p0, p1);
}
int32_t reloq_dede61_shortdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_shortdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_shortdash,args,3,3);
    }
    return reloq_dede61_shortdash(state, p0, p1, p2);
}
int32_t reloq_dede61_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_dede61_single_chars(state, p0, p1, p2);
}
int32_t reloq_dede61_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_dede61_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_dede61_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_dede61_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_dede61_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_dede61_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_dede61_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_dede61_single_letter(state, p0, p1, p2);
}
int32_t reloq_dede61_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_dede61_size_filter(state, p0, p1, p2);
}
int32_t reloq_dede61_skip_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_skip_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_delimiters,args,2,2);
    }
    return reloq_dede61_skip_delimiters(state, p0, p1);
}
int32_t reloq_dede61_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_dede61_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_dede61_slash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_slash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash,args,3,3);
    }
    return reloq_dede61_slash(state, p0, p1, p2);
}
int32_t reloq_dede61_slash_before_measure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_slash_before_measure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash_before_measure,args,3,3);
    }
    return reloq_dede61_slash_before_measure(state, p0, p1, p2);
}
int32_t reloq_dede61_sowohl_als_auch(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_sowohl_als_auch(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_sowohl_als_auch,args,6,6);
    }
    return reloq_dede61_sowohl_als_auch(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_dede61_space(state, p0, p1, p2);
}
int32_t reloq_dede61_span_inverted_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_span_inverted_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_span_inverted_punct,args,3,3);
    }
    return reloq_dede61_span_inverted_punct(state, p0, p1, p2);
}
int32_t reloq_dede61_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_dede61_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_dede61_spell_out_large_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_spell_out_large_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_large_num,args,3,3);
    }
    return reloq_dede61_spell_out_large_num(state, p0, p1, p2);
}
int32_t reloq_dede61_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_dede61_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_dede61_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_dede61_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_split_final_compound(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_split_final_compound(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_split_final_compound,args,4,4);
    }
    return reloq_dede61_split_final_compound(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_split_hyphenated_compound(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_split_hyphenated_compound(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_split_hyphenated_compound,args,3,3);
    }
    return reloq_dede61_split_hyphenated_compound(state, p0, p1, p2);
}
int32_t reloq_dede61_split_initial_compound(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_split_initial_compound(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_split_initial_compound,args,4,4);
    }
    return reloq_dede61_split_initial_compound(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_dede61_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_stanger_ph_A(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_A(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_A,args,1,1);
    }
    return reloq_dede61_stanger_ph_A(state, p0);
}
int32_t reloq_dede61_stanger_ph_E(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_E,args,1,1);
    }
    return reloq_dede61_stanger_ph_E(state, p0);
}
int32_t reloq_dede61_stanger_ph_I(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_I(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_I,args,1,1);
    }
    return reloq_dede61_stanger_ph_I(state, p0);
}
int32_t reloq_dede61_stanger_ph_O(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_O(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_O,args,1,1);
    }
    return reloq_dede61_stanger_ph_O(state, p0);
}
int32_t reloq_dede61_stanger_ph_OE(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_OE(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_OE,args,1,1);
    }
    return reloq_dede61_stanger_ph_OE(state, p0);
}
int32_t reloq_dede61_stanger_ph_R(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_R(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_R,args,1,1);
    }
    return reloq_dede61_stanger_ph_R(state, p0);
}
int32_t reloq_dede61_stanger_ph_U(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_U(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_U,args,1,1);
    }
    return reloq_dede61_stanger_ph_U(state, p0);
}
int32_t reloq_dede61_stanger_ph_Y(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_Y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_Y,args,1,1);
    }
    return reloq_dede61_stanger_ph_Y(state, p0);
}
int32_t reloq_dede61_stanger_ph_a(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_a,args,1,1);
    }
    return reloq_dede61_stanger_ph_a(state, p0);
}
int32_t reloq_dede61_stanger_ph_e(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_e,args,1,1);
    }
    return reloq_dede61_stanger_ph_e(state, p0);
}
int32_t reloq_dede61_stanger_ph_i(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_i,args,1,1);
    }
    return reloq_dede61_stanger_ph_i(state, p0);
}
int32_t reloq_dede61_stanger_ph_j(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_j(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_j,args,1,1);
    }
    return reloq_dede61_stanger_ph_j(state, p0);
}
int32_t reloq_dede61_stanger_ph_l(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_l,args,1,1);
    }
    return reloq_dede61_stanger_ph_l(state, p0);
}
int32_t reloq_dede61_stanger_ph_long_E(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_long_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_long_E,args,1,1);
    }
    return reloq_dede61_stanger_ph_long_E(state, p0);
}
int32_t reloq_dede61_stanger_ph_nasal_E(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_nasal_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_nasal_E,args,1,1);
    }
    return reloq_dede61_stanger_ph_nasal_E(state, p0);
}
int32_t reloq_dede61_stanger_ph_nasal_a(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_nasal_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_nasal_a,args,1,1);
    }
    return reloq_dede61_stanger_ph_nasal_a(state, p0);
}
int32_t reloq_dede61_stanger_ph_nasal_o(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_nasal_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_nasal_o,args,1,1);
    }
    return reloq_dede61_stanger_ph_nasal_o(state, p0);
}
int32_t reloq_dede61_stanger_ph_nasal_oe(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_nasal_oe(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_nasal_oe,args,1,1);
    }
    return reloq_dede61_stanger_ph_nasal_oe(state, p0);
}
int32_t reloq_dede61_stanger_ph_o(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_o,args,1,1);
    }
    return reloq_dede61_stanger_ph_o(state, p0);
}
int32_t reloq_dede61_stanger_ph_oe(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_oe(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_oe,args,1,1);
    }
    return reloq_dede61_stanger_ph_oe(state, p0);
}
int32_t reloq_dede61_stanger_ph_schwa(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_schwa(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_schwa,args,1,1);
    }
    return reloq_dede61_stanger_ph_schwa(state, p0);
}
int32_t reloq_dede61_stanger_ph_u(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_u,args,1,1);
    }
    return reloq_dede61_stanger_ph_u(state, p0);
}
int32_t reloq_dede61_stanger_ph_w(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_w,args,1,1);
    }
    return reloq_dede61_stanger_ph_w(state, p0);
}
int32_t reloq_dede61_stanger_ph_y(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_ph_y,args,1,1);
    }
    return reloq_dede61_stanger_ph_y(state, p0);
}
int32_t reloq_dede61_stanger_vals(void *, int32_t);
static inline int32_t reloq_call_dede61_stanger_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanger_vals,args,1,1);
    }
    return reloq_dede61_stanger_vals(state, p0);
}
int32_t reloq_dede61_starTone_continuation_phr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_starTone_continuation_phr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_starTone_continuation_phr,args,3,3);
    }
    return reloq_dede61_starTone_continuation_phr(state, p0, p1, p2);
}
int32_t reloq_dede61_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_dede61_stretch_focus(state, p0, p1);
}
int32_t reloq_dede61_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_dede61_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_strip_d(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_d,args,1,1);
    }
    return reloq_dede61_strip_d(state, p0);
}
int32_t reloq_dede61_strip_derivational_suffix(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_derivational_suffix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_derivational_suffix,args,1,1);
    }
    return reloq_dede61_strip_derivational_suffix(state, p0);
}
int32_t reloq_dede61_strip_e(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_e,args,1,1);
    }
    return reloq_dede61_strip_e(state, p0);
}
int32_t reloq_dede61_strip_er(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_er(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_er,args,1,1);
    }
    return reloq_dede61_strip_er(state, p0);
}
int32_t reloq_dede61_strip_fem_infl(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_fem_infl(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_fem_infl,args,1,1);
    }
    return reloq_dede61_strip_fem_infl(state, p0);
}
int32_t reloq_dede61_strip_final_root_syll(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_final_root_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_final_root_syll,args,1,1);
    }
    return reloq_dede61_strip_final_root_syll(state, p0);
}
int32_t reloq_dede61_strip_foreign_prefix(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_foreign_prefix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_foreign_prefix,args,1,1);
    }
    return reloq_dede61_strip_foreign_prefix(state, p0);
}
int32_t reloq_dede61_strip_ie_ending(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_ie_ending(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ie_ending,args,1,1);
    }
    return reloq_dede61_strip_ie_ending(state, p0);
}
int32_t reloq_dede61_strip_inflection(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_inflection(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_inflection,args,1,1);
    }
    return reloq_dede61_strip_inflection(state, p0);
}
int32_t reloq_dede61_strip_inner_adj_forming_sffx(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_inner_adj_forming_sffx(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_inner_adj_forming_sffx,args,1,1);
    }
    return reloq_dede61_strip_inner_adj_forming_sffx(state, p0);
}
int32_t reloq_dede61_strip_inner_noun_forming_sffx(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_inner_noun_forming_sffx(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_inner_noun_forming_sffx,args,1,1);
    }
    return reloq_dede61_strip_inner_noun_forming_sffx(state, p0);
}
int32_t reloq_dede61_strip_inseparable_prefix(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_inseparable_prefix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_inseparable_prefix,args,1,1);
    }
    return reloq_dede61_strip_inseparable_prefix(state, p0);
}
int32_t reloq_dede61_strip_last_digit(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_strip_ler(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_strip_ler(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_ler,args,3,3);
    }
    return reloq_dede61_strip_ler(state, p0, p1, p2);
}
int32_t reloq_dede61_strip_linking_s(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_linking_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_linking_s,args,1,1);
    }
    return reloq_dede61_strip_linking_s(state, p0);
}
int32_t reloq_dede61_strip_n(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_n,args,1,1);
    }
    return reloq_dede61_strip_n(state, p0);
}
int32_t reloq_dede61_strip_outer_adj_forming_sffx(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_outer_adj_forming_sffx(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_outer_adj_forming_sffx,args,1,1);
    }
    return reloq_dede61_strip_outer_adj_forming_sffx(state, p0);
}
int32_t reloq_dede61_strip_outer_noun_forming_sffx(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_outer_noun_forming_sffx(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_outer_noun_forming_sffx,args,1,1);
    }
    return reloq_dede61_strip_outer_noun_forming_sffx(state, p0);
}
int32_t reloq_dede61_strip_prefix(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_prefix(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_prefix,args,1,1);
    }
    return reloq_dede61_strip_prefix(state, p0);
}
int32_t reloq_dede61_strip_stressed_particle(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_stressed_particle(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_stressed_particle,args,1,1);
    }
    return reloq_dede61_strip_stressed_particle(state, p0);
}
int32_t reloq_dede61_strip_t(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_t,args,1,1);
    }
    return reloq_dede61_strip_t(state, p0);
}
int32_t reloq_dede61_strip_un(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_un(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_un,args,1,1);
    }
    return reloq_dede61_strip_un(state, p0);
}
int32_t reloq_dede61_strip_variable_particle(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_variable_particle(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_variable_particle,args,1,1);
    }
    return reloq_dede61_strip_variable_particle(state, p0);
}
int32_t reloq_dede61_strip_verb_adj_forming_sffx(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_verb_adj_forming_sffx(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_verb_adj_forming_sffx,args,1,1);
    }
    return reloq_dede61_strip_verb_adj_forming_sffx(state, p0);
}
int32_t reloq_dede61_strip_verb_adj_infl(void *, int32_t);
static inline int32_t reloq_call_dede61_strip_verb_adj_infl(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_verb_adj_infl,args,1,1);
    }
    return reloq_dede61_strip_verb_adj_infl(state, p0);
}
int32_t reloq_dede61_subord_vs_prep(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_subord_vs_prep(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subord_vs_prep,args,3,3);
    }
    return reloq_dede61_subord_vs_prep(state, p0, p1, p2);
}
int32_t reloq_dede61_subordinate_clause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_subordinate_clause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subordinate_clause,args,3,3);
    }
    return reloq_dede61_subordinate_clause(state, p0, p1, p2);
}
int32_t reloq_dede61_subordinate_wh_clause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_subordinate_wh_clause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subordinate_wh_clause,args,3,3);
    }
    return reloq_dede61_subordinate_wh_clause(state, p0, p1, p2);
}
int32_t reloq_dede61_symbolic_rules(void *, int32_t);
static inline int32_t reloq_call_dede61_symbolic_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_symbolic_rules,args,1,1);
    }
    return reloq_dede61_symbolic_rules(state, p0);
}
int32_t reloq_dede61_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_dede61_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_dede61_test_for_ger_ordinal(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_test_for_ger_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_test_for_ger_ordinal,args,4,4);
    }
    return reloq_dede61_test_for_ger_ordinal(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_dede61_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_dede61_text_index(state, p0, p1);
}
int32_t reloq_dede61_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_dede61_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_three_letter_extension(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_three_letter_extension(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_letter_extension,args,2,2);
    }
    return reloq_dede61_three_letter_extension(state, p0, p1);
}
int32_t reloq_dede61_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict,args,3,3);
    }
    return reloq_dede61_tok_dict(state, p0, p1, p2);
}
int32_t reloq_dede61_tok_dict_entry(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_tok_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict_entry,args,3,3);
    }
    return reloq_dede61_tok_dict_entry(state, p0, p1, p2);
}
int32_t reloq_dede61_tok_lookup(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_tok_lookup(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_lookup,args,2,2);
    }
    return reloq_dede61_tok_lookup(state, p0, p1);
}
int32_t reloq_dede61_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_dede61_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_dede61_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_dede61_token_to_number(state, p0, p1, p2);
}
int32_t reloq_dede61_unfunct_auch(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_auch(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_auch,args,6,6);
    }
    return reloq_dede61_unfunct_auch(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_unfunct_be(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_be(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10)
{
    if(delta_rule_trace) {
        const int32_t args[]={p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_be,args,11,11);
    }
    return reloq_dede61_unfunct_be(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}
int32_t reloq_dede61_unfunct_bevor(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_bevor(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_bevor,args,4,4);
    }
    return reloq_dede61_unfunct_bevor(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_unfunct_da(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_da(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_da,args,8,8);
    }
    return reloq_dede61_unfunct_da(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_dede61_unfunct_dann(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_dann(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_dann,args,6,6);
    }
    return reloq_dede61_unfunct_dann(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_unfunct_der_and_dieser_words(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_der_and_dieser_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11, int32_t p12)
{
    if(delta_rule_trace) {
        const int32_t args[]={p12, p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_der_and_dieser_words,args,13,13);
    }
    return reloq_dede61_unfunct_der_and_dieser_words(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}
int32_t reloq_dede61_unfunct_det_before_rel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_det_before_rel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_det_before_rel,args,3,3);
    }
    return reloq_dede61_unfunct_det_before_rel(state, p0, p1, p2);
}
int32_t reloq_dede61_unfunct_doch(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_doch(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_doch,args,6,6);
    }
    return reloq_dede61_unfunct_doch(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_unfunct_ein_words(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_ein_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_ein_words,args,10,10);
    }
    return reloq_dede61_unfunct_ein_words(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_dede61_unfunct_hav(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_hav(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_hav,args,8,8);
    }
    return reloq_dede61_unfunct_hav(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_dede61_unfunct_ja(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_ja(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_ja,args,4,4);
    }
    return reloq_dede61_unfunct_ja(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_unfunct_kein_words(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_kein_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_kein_words,args,6,6);
    }
    return reloq_dede61_unfunct_kein_words(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_unfunct_keiner(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_dede61_unfunct_modals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_modals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_modals,args,8,8);
    }
    return reloq_dede61_unfunct_modals(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_dede61_unfunct_nicht(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_nicht(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_nicht,args,7,7);
    }
    return reloq_dede61_unfunct_nicht(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_dede61_unfunct_nur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_nur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_nur,args,5,5);
    }
    return reloq_dede61_unfunct_nur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_unfunct_possessives(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_possessives(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_possessives,args,6,6);
    }
    return reloq_dede61_unfunct_possessives(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_dede61_unfunct_prep_or_pro(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_prep_or_pro(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_prep_or_pro,args,3,3);
    }
    return reloq_dede61_unfunct_prep_or_pro(state, p0, p1, p2);
}
int32_t reloq_dede61_unfunct_preps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_preps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_preps,args,10,10);
    }
    return reloq_dede61_unfunct_preps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_dede61_unfunct_pronouns(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_pronouns(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11, int32_t p12)
{
    if(delta_rule_trace) {
        const int32_t args[]={p12, p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_pronouns,args,13,13);
    }
    return reloq_dede61_unfunct_pronouns(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}
int32_t reloq_dede61_unfunct_so(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_so(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_so,args,5,5);
    }
    return reloq_dede61_unfunct_so(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_unfunct_wh_words(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_unfunct_wh_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_unfunct_wh_words,args,4,4);
    }
    return reloq_dede61_unfunct_wh_words(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_user_abbr_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_user_abbr_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_abbr_dict,args,3,3);
    }
    return reloq_dede61_user_abbr_dict(state, p0, p1, p2);
}
int32_t reloq_dede61_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_dede61_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_user_rootdict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_user_rootdict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_rootdict,args,3,3);
    }
    return reloq_dede61_user_rootdict(state, p0, p1, p2);
}
int32_t reloq_dede61_valid_f0_posn(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_valid_f0_posn(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_f0_posn,args,3,3);
    }
    return reloq_dede61_valid_f0_posn(state, p0, p1, p2);
}
int32_t reloq_dede61_valid_year_sequence(void *, int32_t, int32_t);
static inline int32_t reloq_call_dede61_valid_year_sequence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_year_sequence,args,2,2);
    }
    return reloq_dede61_valid_year_sequence(state, p0, p1);
}
int32_t reloq_dede61_verb_or_aux(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_verb_or_aux(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_verb_or_aux,args,5,5);
    }
    return reloq_dede61_verb_or_aux(state, p0, p1, p2, p3, p4);
}
int32_t reloq_dede61_verb_vs_adj_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_verb_vs_adj_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_verb_vs_adj_adv,args,3,3);
    }
    return reloq_dede61_verb_vs_adj_adv(state, p0, p1, p2);
}
int32_t reloq_dede61_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_dede61_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_dede61_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_dede61_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_dede61_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_word_final_cons(void *, int32_t);
static inline int32_t reloq_call_dede61_word_final_cons(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_final_cons,args,1,1);
    }
    return reloq_dede61_word_final_cons(state, p0);
}
int32_t reloq_dede61_word_initial_h(void *, int32_t);
static inline int32_t reloq_call_dede61_word_initial_h(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_initial_h,args,1,1);
    }
    return reloq_dede61_word_initial_h(state, p0);
}
int32_t reloq_dede61_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_dede61_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_dede61_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_dede61_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_dede61_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_dede61_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_dede61_year_mode_anno(state, p0, p1, p2, p3);
}
#endif
