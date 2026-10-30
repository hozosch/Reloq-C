/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_ZHTW61_H
#define RELOQ61_RULE_CALLS_ZHTW61_H
#include "delta_rules_c.h"
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_zhtw61_DeltaProc_end(void *, int32_t);
int32_t reloq_zhtw61_DeltaProc_flush(void *, int32_t);
int32_t reloq_zhtw61_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_zhtw61_DeltaProc_main(void *, int32_t);
int32_t reloq_zhtw61_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_zhtw61_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_zhtw61_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_zhtw61_DeltaProc_start(void *, int32_t);
int32_t reloq_zhtw61_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_zhtw61_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_zhtw61_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_zhtw61_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_URL(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_URL(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_URL,args,2,2);
    }
    return reloq_zhtw61_URL(state, p0, p1);
}
int32_t reloq_zhtw61_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbr,args,4,4);
    }
    return reloq_zhtw61_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_abbr_before_num(void *, int32_t, int32_t);
int32_t reloq_zhtw61_abbreviation(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_abbreviation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbreviation,args,4,4);
    }
    return reloq_zhtw61_abbreviation(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_zhtw61_absval(state, p0, p1, p2);
}
int32_t reloq_zhtw61_accent_adjust(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_accent_adjust(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_accent_adjust,args,2,2);
    }
    return reloq_zhtw61_accent_adjust(state, p0, p1);
}
int32_t reloq_zhtw61_accent_adjust_trandur(void *, int32_t, int32_t);
int32_t reloq_zhtw61_accent_amp_adjust(void *, int32_t);
static inline int32_t reloq_call_zhtw61_accent_amp_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_accent_amp_adjust,args,1,1);
    }
    return reloq_zhtw61_accent_amp_adjust(state, p0);
}
int32_t reloq_zhtw61_accent_con_adjust(void *, int32_t);
static inline int32_t reloq_call_zhtw61_accent_con_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_accent_con_adjust,args,1,1);
    }
    return reloq_zhtw61_accent_con_adjust(state, p0);
}
int32_t reloq_zhtw61_acronym(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_acronym(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acronym,args,3,3);
    }
    return reloq_zhtw61_acronym(state, p0, p1, p2);
}
int32_t reloq_zhtw61_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acrotest,args,3,3);
    }
    return reloq_zhtw61_acrotest(state, p0, p1, p2);
}
int32_t reloq_zhtw61_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_zhtw61_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_zhtw61_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_add_chi_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_add_chi_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_chi_point_digits,args,3,3);
    }
    return reloq_zhtw61_add_chi_point_digits(state, p0, p1, p2);
}
int32_t reloq_zhtw61_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_zhtw61_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_zhtw61_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_zhtw61_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_zhtw61_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_zhtw61_adjust_AV_for_fem_voice(void *, int32_t);
int32_t reloq_zhtw61_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_zhtw61_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_adjust_chi_female_voice(void *, int32_t);
static inline int32_t reloq_call_zhtw61_adjust_chi_female_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_chi_female_voice,args,1,1);
    }
    return reloq_zhtw61_adjust_chi_female_voice(state, p0);
}
int32_t reloq_zhtw61_adjust_chi_male_AH(void *, int32_t);
static inline int32_t reloq_call_zhtw61_adjust_chi_male_AH(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_chi_male_AH,args,1,1);
    }
    return reloq_zhtw61_adjust_chi_male_AH(state, p0);
}
int32_t reloq_zhtw61_adjust_chi_male_tl(void *, int32_t);
static inline int32_t reloq_call_zhtw61_adjust_chi_male_tl(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_chi_male_tl,args,1,1);
    }
    return reloq_zhtw61_adjust_chi_male_tl(state, p0);
}
int32_t reloq_zhtw61_adjust_chi_word_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_adjust_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_adjust_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_phones,args,3,3);
    }
    return reloq_zhtw61_adjust_phones(state, p0, p1, p2);
}
int32_t reloq_zhtw61_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_adjust_prev_phr_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_pause,args,3,3);
    }
    return reloq_zhtw61_adjust_prev_phr_pause(state, p0, p1, p2);
}
int32_t reloq_zhtw61_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_zhtw61_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_zhtw61_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_all_identical_syllables(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_ambig_abbr(void *, int32_t, int32_t);
int32_t reloq_zhtw61_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_zhtw61_apostrophe(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_zhtw61_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_zhtw61_apply_chi_a_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_a_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_a_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_a_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_c_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_c_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_c_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_c_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_comma_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_comma_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_e_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_e_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_e_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_e_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_h_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_h_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_h_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_h_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_i_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_i_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_i_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_i_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_j_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_j_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_j_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_j_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_n_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_n_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_n_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_n_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_o_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_o_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_o_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_o_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_q_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_q_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_q_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_q_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_s_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_s_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_s_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_s_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_u_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_u_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_u_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_u_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_u_umlaut_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_u_umlaut_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_u_umlaut_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_u_umlaut_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_w_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_w_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_w_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_w_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_y_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_y_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_y_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_y_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_chi_z_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_chi_z_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_chi_z_rules,args,3,3);
    }
    return reloq_zhtw61_apply_chi_z_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_comma_phr_rules(void *, int32_t);
static inline int32_t reloq_call_zhtw61_apply_comma_phr_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_comma_phr_rules,args,1,1);
    }
    return reloq_zhtw61_apply_comma_phr_rules(state, p0);
}
int32_t reloq_zhtw61_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_zhtw61_apply_head_size_val(state, p0, p1);
}
int32_t reloq_zhtw61_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_lang_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_lang_comma_rules,args,3,3);
    }
    return reloq_zhtw61_apply_lang_comma_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_zhtw61_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_zhtw61_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_zhtw61_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_zhtw61_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_zhtw61_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_zhtw61_apply_roughness_val(state, p0, p1);
}
int32_t reloq_zhtw61_apply_rules(void *, int32_t);
static inline int32_t reloq_call_zhtw61_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_zhtw61_apply_rules(state, p0);
}
int32_t reloq_zhtw61_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_zhtw61_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_zhtw61_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_zhtw61_apply_speed_val(state, p0, p1);
}
int32_t reloq_zhtw61_apply_stressVal(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_stressVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_stressVal,args,4,4);
    }
    return reloq_zhtw61_apply_stressVal(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_zhtw61_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_zhtw61_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_zhtw61_apply_volume_val(state, p0, p1);
}
int32_t reloq_zhtw61_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_zhtw61_assign_acoustic_vals(state, p0);
}
int32_t reloq_zhtw61_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_zhtw61_assign_alt_quest_features(state, p0);
}
int32_t reloq_zhtw61_assign_chi_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_chi_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_chi_nuc_durs,args,1,1);
    }
    return reloq_zhtw61_assign_chi_nuc_durs(state, p0);
}
int32_t reloq_zhtw61_assign_chi_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_assign_chi_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_chi_stress,args,3,3);
    }
    return reloq_zhtw61_assign_chi_stress(state, p0, p1, p2);
}
int32_t reloq_zhtw61_assign_chi_syll_coda(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_assign_chi_syll_coda(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_chi_syll_coda,args,2,2);
    }
    return reloq_zhtw61_assign_chi_syll_coda(state, p0, p1);
}
int32_t reloq_zhtw61_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_zhtw61_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_zhtw61_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_zhtw61_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_zhtw61_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_zhtw61_assign_default_quest_features(state, p0);
}
int32_t reloq_zhtw61_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_zhtw61_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_zhtw61_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_zhtw61_assign_nuc_durs(state, p0);
}
int32_t reloq_zhtw61_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_zhtw61_assign_phone_durs(state, p0);
}
int32_t reloq_zhtw61_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_zhtw61_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_zhtw61_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_zhtw61_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_zhtw61_assign_sent_final_features(state, p0);
}
int32_t reloq_zhtw61_assign_stanchi_start_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_stanchi_start_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_stanchi_start_dur,args,1,1);
    }
    return reloq_zhtw61_assign_stanchi_start_dur(state, p0);
}
int32_t reloq_zhtw61_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_zhtw61_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_zhtw61_assign_user_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_zhtw61_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_zhtw61_assign_wh_quest_features(state, p0);
}
int32_t reloq_zhtw61_beg_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_beg_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_beg_of_word,args,2,2);
    }
    return reloq_zhtw61_beg_of_word(state, p0, p1);
}
int32_t reloq_zhtw61_boundary_tone(void *, int32_t);
int32_t reloq_zhtw61_bracket(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_bracket(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_bracket,args,3,3);
    }
    return reloq_zhtw61_bracket(state, p0, p1, p2);
}
int32_t reloq_zhtw61_break_into_chi_syllables(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_break_into_chi_syllables(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_chi_syllables,args,3,3);
    }
    return reloq_zhtw61_break_into_chi_syllables(state, p0, p1, p2);
}
int32_t reloq_zhtw61_break_spr_into_sylls(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_break_spr_into_sylls(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_spr_into_sylls,args,3,3);
    }
    return reloq_zhtw61_break_spr_into_sylls(state, p0, p1, p2);
}
int32_t reloq_zhtw61_breakup_chi_spr_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_breakup_chi_spr_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_breakup_chi_spr_phones,args,3,3);
    }
    return reloq_zhtw61_breakup_chi_spr_phones(state, p0, p1, p2);
}
int32_t reloq_zhtw61_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_breakup_spr_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_breakup_spr_phones,args,3,3);
    }
    return reloq_zhtw61_breakup_spr_phones(state, p0, p1, p2);
}
int32_t reloq_zhtw61_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_zhtw61_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_zhtw61_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_zhtw61_build_phrases(state, p0, p1, p2);
}
int32_t reloq_zhtw61_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_zhtw61_build_words(state, p0, p1, p2);
}
int32_t reloq_zhtw61_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_zhtw61_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_zhtw61_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_zhtw61_char_name(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_abbr,args,3,3);
    }
    return reloq_zhtw61_chi_abbr(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_acrotest,args,3,3);
    }
    return reloq_zhtw61_chi_acrotest(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_char_name,args,3,3);
    }
    return reloq_zhtw61_chi_char_name(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_con_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_con_vals,args,1,1);
    }
    return reloq_zhtw61_chi_con_vals(state, p0);
}
int32_t reloq_zhtw61_chi_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_cons_durs,args,3,3);
    }
    return reloq_zhtw61_chi_cons_durs(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_dental_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_dental_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_dental_vals,args,1,1);
    }
    return reloq_zhtw61_chi_dental_vals(state, p0);
}
int32_t reloq_zhtw61_chi_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_dict_lookup,args,3,3);
    }
    return reloq_zhtw61_chi_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_hyphen_between_nums,args,5,5);
    }
    return reloq_zhtw61_chi_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_chi_labial_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_labial_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_labial_vals,args,1,1);
    }
    return reloq_zhtw61_chi_labial_vals(state, p0);
}
int32_t reloq_zhtw61_chi_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_non_phrasal_comma,args,2,2);
    }
    return reloq_zhtw61_chi_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_zhtw61_chi_pal_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_pal_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_pal_vals,args,1,1);
    }
    return reloq_zhtw61_chi_pal_vals(state, p0);
}
int32_t reloq_zhtw61_chi_ph_D(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_D(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_D,args,1,1);
    }
    return reloq_zhtw61_chi_ph_D(state, p0);
}
int32_t reloq_zhtw61_chi_ph_D_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_D_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_D_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_D_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_G(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_G(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_G,args,1,1);
    }
    return reloq_zhtw61_chi_ph_G(state, p0);
}
int32_t reloq_zhtw61_chi_ph_G_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_G_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_G_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_G_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_H(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_H(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_H,args,1,1);
    }
    return reloq_zhtw61_chi_ph_H(state, p0);
}
int32_t reloq_zhtw61_chi_ph_H_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_H_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_H_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_H_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_S(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_S,args,1,1);
    }
    return reloq_zhtw61_chi_ph_S(state, p0);
}
int32_t reloq_zhtw61_chi_ph_S_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_S_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_S_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_S_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_T(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_T(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_T,args,1,1);
    }
    return reloq_zhtw61_chi_ph_T(state, p0);
}
int32_t reloq_zhtw61_chi_ph_T_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_T_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_T_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_T_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_X(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_X(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_X,args,1,1);
    }
    return reloq_zhtw61_chi_ph_X(state, p0);
}
int32_t reloq_zhtw61_chi_ph_X_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_X_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_X_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_X_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_Z(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_Z,args,1,1);
    }
    return reloq_zhtw61_chi_ph_Z(state, p0);
}
int32_t reloq_zhtw61_chi_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_Z_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_Z_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_b(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_b,args,1,1);
    }
    return reloq_zhtw61_chi_ph_b(state, p0);
}
int32_t reloq_zhtw61_chi_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_b_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_b_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_d(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_d,args,1,1);
    }
    return reloq_zhtw61_chi_ph_d(state, p0);
}
int32_t reloq_zhtw61_chi_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_d_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_d_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_f(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_f(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_f,args,1,1);
    }
    return reloq_zhtw61_chi_ph_f(state, p0);
}
int32_t reloq_zhtw61_chi_ph_f_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_f_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_f_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_f_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_g(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_g,args,1,1);
    }
    return reloq_zhtw61_chi_ph_g(state, p0);
}
int32_t reloq_zhtw61_chi_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_g_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_g_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_h(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_h(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_h,args,1,1);
    }
    return reloq_zhtw61_chi_ph_h(state, p0);
}
int32_t reloq_zhtw61_chi_ph_h_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_h_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_h_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_h_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_j(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_j(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_j,args,1,1);
    }
    return reloq_zhtw61_chi_ph_j(state, p0);
}
int32_t reloq_zhtw61_chi_ph_j_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_j_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_j_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_j_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_k(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_k,args,1,1);
    }
    return reloq_zhtw61_chi_ph_k(state, p0);
}
int32_t reloq_zhtw61_chi_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_k_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_k_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_l(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_l,args,1,1);
    }
    return reloq_zhtw61_chi_ph_l(state, p0);
}
int32_t reloq_zhtw61_chi_ph_l_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_l_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_l_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_l_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_m(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_m,args,1,1);
    }
    return reloq_zhtw61_chi_ph_m(state, p0);
}
int32_t reloq_zhtw61_chi_ph_m_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_m_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_m_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_m_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_n(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_n,args,1,1);
    }
    return reloq_zhtw61_chi_ph_n(state, p0);
}
int32_t reloq_zhtw61_chi_ph_n_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_n_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_n_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_p(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_p,args,1,1);
    }
    return reloq_zhtw61_chi_ph_p(state, p0);
}
int32_t reloq_zhtw61_chi_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_p_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_p_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_q(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_q(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_q,args,1,1);
    }
    return reloq_zhtw61_chi_ph_q(state, p0);
}
int32_t reloq_zhtw61_chi_ph_q_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_q_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_q_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_q_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_r(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_r,args,1,1);
    }
    return reloq_zhtw61_chi_ph_r(state, p0);
}
int32_t reloq_zhtw61_chi_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_r_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_r_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_s(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_s,args,1,1);
    }
    return reloq_zhtw61_chi_ph_s(state, p0);
}
int32_t reloq_zhtw61_chi_ph_s_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_s_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_s_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_s_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_t(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_t,args,1,1);
    }
    return reloq_zhtw61_chi_ph_t(state, p0);
}
int32_t reloq_zhtw61_chi_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_t_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_t_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_v(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_v(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_v,args,1,1);
    }
    return reloq_zhtw61_chi_ph_v(state, p0);
}
int32_t reloq_zhtw61_chi_ph_v_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_v_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_v_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_v_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_w(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_w,args,1,1);
    }
    return reloq_zhtw61_chi_ph_w(state, p0);
}
int32_t reloq_zhtw61_chi_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_w_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_w_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_x(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_x(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_x,args,1,1);
    }
    return reloq_zhtw61_chi_ph_x(state, p0);
}
int32_t reloq_zhtw61_chi_ph_x_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_x_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_x_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_x_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_y(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_y,args,1,1);
    }
    return reloq_zhtw61_chi_ph_y(state, p0);
}
int32_t reloq_zhtw61_chi_ph_y_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_y_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_y_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_y_dur(state, p0);
}
int32_t reloq_zhtw61_chi_ph_z(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_z,args,1,1);
    }
    return reloq_zhtw61_chi_ph_z(state, p0);
}
int32_t reloq_zhtw61_chi_ph_z_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_ph_z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_ph_z_dur,args,1,1);
    }
    return reloq_zhtw61_chi_ph_z_dur(state, p0);
}
int32_t reloq_zhtw61_chi_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_phrase_level_rules,args,3,3);
    }
    return reloq_zhtw61_chi_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_post_comma_phrase,args,3,3);
    }
    return reloq_zhtw61_chi_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_quest_syll_tone(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_chi_retro_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_retro_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_retro_vals,args,1,1);
    }
    return reloq_zhtw61_chi_retro_vals(state, p0);
}
int32_t reloq_zhtw61_chi_specific_word_cases(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_chi_symbolic_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_symbolic_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_symbolic_rules,args,3,3);
    }
    return reloq_zhtw61_chi_symbolic_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_tok_dict,args,3,3);
    }
    return reloq_zhtw61_chi_tok_dict(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_trans_features(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_trans_features,args,1,1);
    }
    return reloq_zhtw61_chi_trans_features(state, p0);
}
int32_t reloq_zhtw61_chi_velar_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_chi_velar_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_chi_velar_vals,args,1,1);
    }
    return reloq_zhtw61_chi_velar_vals(state, p0);
}
int32_t reloq_zhtw61_chi_word_cat_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_word_cat_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_word_cat_anno,args,3,3);
    }
    return reloq_zhtw61_chi_word_cat_anno(state, p0, p1, p2);
}
int32_t reloq_zhtw61_chi_word_stress_tone(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_word_stress_tone(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_word_stress_tone,args,2,2);
    }
    return reloq_zhtw61_chi_word_stress_tone(state, p0, p1);
}
int32_t reloq_zhtw61_chi_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_chi_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_chi_words,args,3,3);
    }
    return reloq_zhtw61_chi_words(state, p0, p1, p2);
}
int32_t reloq_zhtw61_clear_delta(void *, int32_t);
static inline int32_t reloq_call_zhtw61_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_zhtw61_clear_delta(state, p0);
}
int32_t reloq_zhtw61_close_platform(void *, int32_t);
static inline int32_t reloq_call_zhtw61_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_zhtw61_close_platform(state, p0);
}
int32_t reloq_zhtw61_comma_at_end_quote(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_comma_at_end_quote(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_comma_at_end_quote,args,3,3);
    }
    return reloq_zhtw61_comma_at_end_quote(state, p0, p1, p2);
}
int32_t reloq_zhtw61_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_zhtw61_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_zhtw61_comma_phrase(state, p0);
}
int32_t reloq_zhtw61_con_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_zhtw61_con_vals(state, p0);
}
int32_t reloq_zhtw61_convert_1_or_2_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_1_or_2_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_1_or_2_digits,args,3,3);
    }
    return reloq_zhtw61_convert_1_or_2_digits(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_100millions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_100millions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_100millions,args,3,3);
    }
    return reloq_zhtw61_convert_chi_100millions(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_date,args,3,3);
    }
    return reloq_zhtw61_convert_chi_date(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_degrees(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_degrees(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_degrees,args,3,3);
    }
    return reloq_zhtw61_convert_chi_degrees(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_digits,args,3,3);
    }
    return reloq_zhtw61_convert_chi_digits(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_digits_to_words,args,3,3);
    }
    return reloq_zhtw61_convert_chi_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_double,args,3,3);
    }
    return reloq_zhtw61_convert_chi_double(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_fraction,args,3,3);
    }
    return reloq_zhtw61_convert_chi_fraction(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_hundreds,args,3,3);
    }
    return reloq_zhtw61_convert_chi_hundreds(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_monetary_exp,args,5,5);
    }
    return reloq_zhtw61_convert_chi_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_convert_chi_percent(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_percent(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_percent,args,3,3);
    }
    return reloq_zhtw61_convert_chi_percent(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_phone_and_postcode(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_phone_and_postcode(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_phone_and_postcode,args,3,3);
    }
    return reloq_zhtw61_convert_chi_phone_and_postcode(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_roman_num(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_convert_chi_sports_score(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_sports_score(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_sports_score,args,3,3);
    }
    return reloq_zhtw61_convert_chi_sports_score(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_tenthousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_tenthousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_tenthousands,args,3,3);
    }
    return reloq_zhtw61_convert_chi_tenthousands(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_thousands,args,3,3);
    }
    return reloq_zhtw61_convert_chi_thousands(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_time(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_time(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_time,args,3,3);
    }
    return reloq_zhtw61_convert_chi_time(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_chi_year(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_chi_year(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_chi_year,args,3,3);
    }
    return reloq_zhtw61_convert_chi_year(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits_to_words,args,3,3);
    }
    return reloq_zhtw61_convert_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_double,args,3,3);
    }
    return reloq_zhtw61_convert_double(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_lang_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_lang_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_lang_digits_to_words,args,3,3);
    }
    return reloq_zhtw61_convert_lang_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_zhtw61_convert_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_monetary_exp,args,5,5);
    }
    return reloq_zhtw61_convert_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_zhtw61_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_zhtw61_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_zhtw61_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_zhtw61_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_zhtw61_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_zhtw61_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_count_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_count_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_digits,args,3,3);
    }
    return reloq_zhtw61_count_digits(state, p0, p1, p2);
}
int32_t reloq_zhtw61_count_eng_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_count_eng_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_eng_digits,args,3,3);
    }
    return reloq_zhtw61_count_eng_digits(state, p0, p1, p2);
}
int32_t reloq_zhtw61_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_zhtw61_count_input(state, p0, p1);
}
int32_t reloq_zhtw61_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_zhtw61_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_zhtw61_count_normalized_inp(state, p0);
}
int32_t reloq_zhtw61_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_count_sylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_sylls,args,4,4);
    }
    return reloq_zhtw61_count_sylls(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_zhtw61_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_zhtw61_count_user_indices(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_count_user_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_user_indices,args,4,4);
    }
    return reloq_zhtw61_count_user_indices(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_create_chi_syllables(void *, int32_t);
static inline int32_t reloq_call_zhtw61_create_chi_syllables(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_create_chi_syllables,args,1,1);
    }
    return reloq_zhtw61_create_chi_syllables(state, p0);
}
int32_t reloq_zhtw61_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_zhtw61_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_zhtw61_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_zhtw61_create_sentence(state, p0, p1);
}
int32_t reloq_zhtw61_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_zhtw61_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_del_chi_extra_syncs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_del_chi_extra_syncs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_del_chi_extra_syncs,args,3,3);
    }
    return reloq_zhtw61_del_chi_extra_syncs(state, p0, p1, p2);
}
int32_t reloq_zhtw61_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_zhtw61_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_zhtw61_delete_2_more(state, p0);
}
int32_t reloq_zhtw61_delete_comma_sync(void *, int32_t, int32_t);
int32_t reloq_zhtw61_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_delete_inp_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_left,args,3,3);
    }
    return reloq_zhtw61_delete_inp_from_left(state, p0, p1, p2);
}
int32_t reloq_zhtw61_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_zhtw61_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_zhtw61_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_zhtw61_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_zhtw61_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_delete_syll_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_syll_from_left,args,3,3);
    }
    return reloq_zhtw61_delete_syll_from_left(state, p0, p1, p2);
}
int32_t reloq_zhtw61_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_zhtw61_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_zhtw61_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_zhtw61_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_zhtw61_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_zhtw61_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_zhtw61_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_zhtw61_demote_word(state, p0, p1, p2);
}
int32_t reloq_zhtw61_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_zhtw61_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_zhtw61_distribute_nucdur(state, p0);
}
int32_t reloq_zhtw61_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_zhtw61_divide_char_count_token(state, p0, p1);
}
int32_t reloq_zhtw61_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_zhtw61_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_zhtw61_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_zhtw61_doF(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_zhtw61_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_zhtw61_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_zhtw61_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_zhtw61_do_FTZ_FTP(state, p0);
}
int32_t reloq_zhtw61_do_chi_h_dur(void *, int32_t, int32_t);
int32_t reloq_zhtw61_do_f0_stmarray_insertion(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_do_f0_stmarray_insertion(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_f0_stmarray_insertion,args,3,3);
    }
    return reloq_zhtw61_do_f0_stmarray_insertion(state, p0, p1, p2);
}
int32_t reloq_zhtw61_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_zhtw61_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_zhtw61_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_zhtw61_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_zhtw61_do_non_son_amps(state, p0);
}
int32_t reloq_zhtw61_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_zhtw61_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_zhtw61_do_pt_voicing(state, p0);
}
int32_t reloq_zhtw61_do_release(void *, int32_t);
static inline int32_t reloq_call_zhtw61_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_zhtw61_do_release(state, p0);
}
int32_t reloq_zhtw61_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_zhtw61_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_zhtw61_do_son_amps(state, p0);
}
int32_t reloq_zhtw61_do_user_index_callbacks(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_do_user_index_callbacks(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_user_index_callbacks,args,2,2);
    }
    return reloq_zhtw61_do_user_index_callbacks(state, p0, p1);
}
int32_t reloq_zhtw61_do_voice_bar(void *, int32_t);
static inline int32_t reloq_call_zhtw61_do_voice_bar(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_voice_bar,args,1,1);
    }
    return reloq_zhtw61_do_voice_bar(state, p0);
}
int32_t reloq_zhtw61_dunhao_phrase(void *, int32_t);
int32_t reloq_zhtw61_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_zhtw61_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_zhtw61_ellipsis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_ellipsis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ellipsis,args,3,3);
    }
    return reloq_zhtw61_ellipsis(state, p0, p1, p2);
}
int32_t reloq_zhtw61_email_address(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_email_address(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_email_address,args,2,2);
    }
    return reloq_zhtw61_email_address(state, p0, p1);
}
int32_t reloq_zhtw61_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_zhtw61_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_zhtw61_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_zhtw61_end_of_sentence(state, p0, p1);
}
int32_t reloq_zhtw61_end_of_token(void *, int32_t, int32_t);
int32_t reloq_zhtw61_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_zhtw61_end_of_word(state, p0, p1);
}
int32_t reloq_zhtw61_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_zhtw61_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_estimate_numsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_estimate_numsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_numsylls,args,4,4);
    }
    return reloq_zhtw61_estimate_numsylls(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_exclam_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_exclam_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_exclam_phrase,args,2,2);
    }
    return reloq_zhtw61_exclam_phrase(state, p0, p1);
}
int32_t reloq_zhtw61_extend_vals(void *, int32_t);
int32_t reloq_zhtw61_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_zhtw61_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_zhtw61_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_zhtw61_final_voiced_release(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_final_voiced_release(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_voiced_release,args,3,3);
    }
    return reloq_zhtw61_final_voiced_release(state, p0, p1, p2);
}
int32_t reloq_zhtw61_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_zhtw61_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_zhtw61_find_acoustic_nuc(state, p0);
}
int32_t reloq_zhtw61_find_chi_nucleus(void *, int32_t);
static inline int32_t reloq_call_zhtw61_find_chi_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_chi_nucleus,args,1,1);
    }
    return reloq_zhtw61_find_chi_nucleus(state, p0);
}
int32_t reloq_zhtw61_find_chi_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_find_chi_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_chi_obligatory_phrase,args,4,4);
    }
    return reloq_zhtw61_find_chi_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_find_chi_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_find_chi_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_chi_potential_phrase,args,4,4);
    }
    return reloq_zhtw61_find_chi_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_find_lang_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_find_lang_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_obligatory_phrase,args,4,4);
    }
    return reloq_zhtw61_find_lang_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_find_lang_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_find_lang_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_potential_phrase,args,4,4);
    }
    return reloq_zhtw61_find_lang_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_zhtw61_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_find_next_token(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_token,args,4,4);
    }
    return reloq_zhtw61_find_next_token(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_zhtw61_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_zhtw61_find_nucleus(state, p0);
}
int32_t reloq_zhtw61_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_zhtw61_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_find_unpuncted_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_find_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_unpuncted_phrases,args,3,3);
    }
    return reloq_zhtw61_find_unpuncted_phrases(state, p0, p1, p2);
}
int32_t reloq_zhtw61_first_root_vow(void *, int32_t, int32_t);
int32_t reloq_zhtw61_fname_adjust_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_fname_adjust_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fname_adjust_dur,args,1,1);
    }
    return reloq_zhtw61_fname_adjust_dur(state, p0);
}
int32_t reloq_zhtw61_followed_by_er(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_followed_by_er(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_followed_by_er,args,2,2);
    }
    return reloq_zhtw61_followed_by_er(state, p0, p1);
}
int32_t reloq_zhtw61_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_zhtw61_forced_synthesis(state, p0, p1);
}
int32_t reloq_zhtw61_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_zhtw61_generate_acoustic_vals(state, p0);
}
int32_t reloq_zhtw61_generate_chi_diaphones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_generate_chi_diaphones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_generate_chi_diaphones,args,3,3);
    }
    return reloq_zhtw61_generate_chi_diaphones(state, p0, p1, p2);
}
int32_t reloq_zhtw61_generate_durations(void *, int32_t);
static inline int32_t reloq_call_zhtw61_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_zhtw61_generate_durations(state, p0);
}
int32_t reloq_zhtw61_generate_tone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_generate_tone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_generate_tone,args,3,3);
    }
    return reloq_zhtw61_generate_tone(state, p0, p1, p2);
}
int32_t reloq_zhtw61_get_chi_fric_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_chi_fric_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_chi_fric_trans_dur,args,2,2);
    }
    return reloq_zhtw61_get_chi_fric_trans_dur(state, p0, p1);
}
int32_t reloq_zhtw61_get_chi_nas_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_chi_nas_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_chi_nas_trans_dur,args,2,2);
    }
    return reloq_zhtw61_get_chi_nas_trans_dur(state, p0, p1);
}
int32_t reloq_zhtw61_get_chi_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_chi_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_chi_nuclear_accent,args,4,4);
    }
    return reloq_zhtw61_get_chi_nuclear_accent(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_get_chi_son_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_chi_son_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_chi_son_trans_dur,args,2,2);
    }
    return reloq_zhtw61_get_chi_son_trans_dur(state, p0, p1);
}
int32_t reloq_zhtw61_get_chi_stop_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_chi_stop_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_chi_stop_trans_dur,args,2,2);
    }
    return reloq_zhtw61_get_chi_stop_trans_dur(state, p0, p1);
}
int32_t reloq_zhtw61_get_chi_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_chi_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_chi_trans_dur,args,2,2);
    }
    return reloq_zhtw61_get_chi_trans_dur(state, p0, p1);
}
int32_t reloq_zhtw61_get_chi_vow_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_chi_vow_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_chi_vow_trans_dur,args,2,2);
    }
    return reloq_zhtw61_get_chi_vow_trans_dur(state, p0, p1);
}
int32_t reloq_zhtw61_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_zhtw61_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_zhtw61_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_zhtw61_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_get_input(void *, int32_t);
static inline int32_t reloq_call_zhtw61_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_zhtw61_get_input(state, p0);
}
int32_t reloq_zhtw61_get_lang_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_lang_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_lang_nuclear_accent,args,4,4);
    }
    return reloq_zhtw61_get_lang_nuclear_accent(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_zhtw61_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_zhtw61_get_nuc_AV(state, p0);
}
int32_t reloq_zhtw61_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_zhtw61_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_zhtw61_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_zhtw61_get_preformant_var(void *, int32_t);
static inline int32_t reloq_call_zhtw61_get_preformant_var(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_preformant_var,args,1,1);
    }
    return reloq_zhtw61_get_preformant_var(state, p0);
}
int32_t reloq_zhtw61_get_stanchi_non_vowels(void *, int32_t);
static inline int32_t reloq_call_zhtw61_get_stanchi_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanchi_non_vowels,args,1,1);
    }
    return reloq_zhtw61_get_stanchi_non_vowels(state, p0);
}
int32_t reloq_zhtw61_get_stanchi_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_zhtw61_get_stanchi_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanchi_nuc_AV,args,1,1);
    }
    return reloq_zhtw61_get_stanchi_nuc_AV(state, p0);
}
int32_t reloq_zhtw61_get_tok(void *, int32_t);
static inline int32_t reloq_call_zhtw61_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_zhtw61_get_tok(state, p0);
}
int32_t reloq_zhtw61_get_tone_level(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_tone_level(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_tone_level,args,5,5);
    }
    return reloq_zhtw61_get_tone_level(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_get_tone_value(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_tone_value(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_tone_value,args,5,5);
    }
    return reloq_zhtw61_get_tone_value(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_zhtw61_get_trans_dur(state, p0, p1);
}
int32_t reloq_zhtw61_gettok(void *, int32_t);
static inline int32_t reloq_call_zhtw61_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_zhtw61_gettok(state, p0);
}
int32_t reloq_zhtw61_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_zhtw61_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_zhtw61_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_zhtw61_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_zhtw61_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_zhtw61_handle_non_phrasal_commas(void *, int32_t);
static inline int32_t reloq_call_zhtw61_handle_non_phrasal_commas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_phrasal_commas,args,1,1);
    }
    return reloq_zhtw61_handle_non_phrasal_commas(state, p0);
}
int32_t reloq_zhtw61_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_zhtw61_handle_pause(state, p0, p1);
}
int32_t reloq_zhtw61_high_low_star_tone(void *, int32_t);
int32_t reloq_zhtw61_high_star_low_tone(void *, int32_t);
int32_t reloq_zhtw61_high_tone(void *, int32_t);
int32_t reloq_zhtw61_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen,args,3,3);
    }
    return reloq_zhtw61_hyphen(state, p0, p1, p2);
}
int32_t reloq_zhtw61_hyphen_between_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen_between_nums,args,3,3);
    }
    return reloq_zhtw61_hyphen_between_nums(state, p0, p1, p2);
}
int32_t reloq_zhtw61_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_zhtw61_in_brackets(state, p0, p1, p2);
}
int32_t reloq_zhtw61_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_zhtw61_in_quotes(state, p0, p1, p2);
}
int32_t reloq_zhtw61_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_zhtw61_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_zhtw61_increment_output_file_number(state, p0);
}
int32_t reloq_zhtw61_init_chi_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_chi_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_chi_vars,args,1,1);
    }
    return reloq_zhtw61_init_chi_vars(state, p0);
}
int32_t reloq_zhtw61_init_current_word_ptrs(void *, int32_t);
int32_t reloq_zhtw61_init_delta(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_zhtw61_init_delta(state, p0);
}
int32_t reloq_zhtw61_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_zhtw61_init_lang_vars(state, p0);
}
int32_t reloq_zhtw61_init_mcvoice_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_init_mcvoice_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_mcvoice_vars,args,2,2);
    }
    return reloq_zhtw61_init_mcvoice_vars(state, p0, p1);
}
int32_t reloq_zhtw61_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_zhtw61_init_phr_feature_vars(state, p0);
}
int32_t reloq_zhtw61_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_zhtw61_init_phrase_vars(state, p0);
}
int32_t reloq_zhtw61_init_platform(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_zhtw61_init_platform(state, p0);
}
int32_t reloq_zhtw61_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_zhtw61_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_zhtw61_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_zhtw61_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_zhtw61_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_zhtw61_init_syll_vars(state, p0);
}
int32_t reloq_zhtw61_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_zhtw61_init_symbolic_vars(state, p0);
}
int32_t reloq_zhtw61_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_zhtw61_init_tm_vars(state, p0);
}
int32_t reloq_zhtw61_init_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_zhtw61_init_vars(state, p0);
}
int32_t reloq_zhtw61_init_word(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_zhtw61_init_word(state, p0);
}
int32_t reloq_zhtw61_init_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_ptrs,args,1,1);
    }
    return reloq_zhtw61_init_word_ptrs(state, p0);
}
int32_t reloq_zhtw61_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_zhtw61_init_word_vars(state, p0);
}
int32_t reloq_zhtw61_input_type_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_input_type_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_input_type_anno,args,4,4);
    }
    return reloq_zhtw61_input_type_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_zhtw61_insert_acoustic_vals(state, p0);
}
int32_t reloq_zhtw61_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_zhtw61_insert_burst_dur(state, p0);
}
int32_t reloq_zhtw61_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_zhtw61_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_zhtw61_insert_char_count(state, p0);
}
int32_t reloq_zhtw61_insert_chi_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_chi_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_chi_spr_phone,args,3,3);
    }
    return reloq_zhtw61_insert_chi_spr_phone(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_chi_syll(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_chi_syll(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_chi_syll,args,3,3);
    }
    return reloq_zhtw61_insert_chi_syll(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_comma_phrase,args,3,3);
    }
    return reloq_zhtw61_insert_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_zhtw61_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_insert_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_zhtw61_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_non_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_non_phrase_pause,args,3,3);
    }
    return reloq_zhtw61_insert_non_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_zhtw61_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_zhtw61_insert_pause_durs(state, p0);
}
int32_t reloq_zhtw61_insert_penult_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_zhtw61_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_zhtw61_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_zhtw61_insert_space(state, p0, p1);
}
int32_t reloq_zhtw61_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_zhtw61_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_zhtw61_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_insert_stress_pattern(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_stress_pattern(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress_pattern,args,3,3);
    }
    return reloq_zhtw61_insert_stress_pattern(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_zhtw61_insert_syll_level_vals(state, p0);
}
int32_t reloq_zhtw61_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_zhtw61_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_zhtw61_insert_transitions(state, p0);
}
int32_t reloq_zhtw61_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_zhtw61_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrases,args,4,4);
    }
    return reloq_zhtw61_insert_unpuncted_phrases(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_zhtw61_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_zhtw61_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_zhtw61_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_zhtw61_insert_word(state, p0, p1, p2);
}
int32_t reloq_zhtw61_inside_orig_inp_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_inside_orig_inp_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_inside_orig_inp_token,args,2,2);
    }
    return reloq_zhtw61_inside_orig_inp_token(state, p0, p1);
}
int32_t reloq_zhtw61_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_zhtw61_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_zhtw61_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_interpret_single_char_modes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_interpret_single_char_modes,args,3,3);
    }
    return reloq_zhtw61_interpret_single_char_modes(state, p0, p1, p2);
}
int32_t reloq_zhtw61_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_zhtw61_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_zhtw61_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_zhtw61_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_zhtw61_is_foreign_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_is_foreign_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_foreign_word,args,3,3);
    }
    return reloq_zhtw61_is_foreign_word(state, p0, p1, p2);
}
int32_t reloq_zhtw61_is_hyphenated_word(void *, int32_t, int32_t);
int32_t reloq_zhtw61_is_in_parentheses(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_is_math_exp(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_is_math_exp(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_math_exp,args,2,2);
    }
    return reloq_zhtw61_is_math_exp(state, p0, p1);
}
int32_t reloq_zhtw61_is_pathname(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_is_pathname(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pathname,args,2,2);
    }
    return reloq_zhtw61_is_pathname(state, p0, p1);
}
int32_t reloq_zhtw61_is_phone_or_postcode(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_is_phone_or_postcode(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_phone_or_postcode,args,3,3);
    }
    return reloq_zhtw61_is_phone_or_postcode(state, p0, p1, p2);
}
int32_t reloq_zhtw61_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_zhtw61_is_spr(state, p0, p1, p2);
}
int32_t reloq_zhtw61_isitAlt(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_isitWH(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_lang_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_lang_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_hyphen_between_nums,args,5,5);
    }
    return reloq_zhtw61_lang_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_lang_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_lang_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_non_phrasal_comma,args,2,2);
    }
    return reloq_zhtw61_lang_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_zhtw61_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_lang_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_phrase_level_rules,args,3,3);
    }
    return reloq_zhtw61_lang_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_zhtw61_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_zhtw61_lang_word_cat_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_lang_word_cat_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_word_cat_anno,args,3,3);
    }
    return reloq_zhtw61_lang_word_cat_anno(state, p0, p1, p2);
}
int32_t reloq_zhtw61_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_zhtw61_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_zhtw61_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_zhtw61_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_zhtw61_longdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_longdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_longdash,args,3,3);
    }
    return reloq_zhtw61_longdash(state, p0, p1, p2);
}
int32_t reloq_zhtw61_low_high_star_tone(void *, int32_t);
int32_t reloq_zhtw61_low_star_high_tone(void *, int32_t);
int32_t reloq_zhtw61_low_tone(void *, int32_t);
int32_t reloq_zhtw61_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_mark_non_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_mark_non_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_non_letter,args,3,3);
    }
    return reloq_zhtw61_mark_non_letter(state, p0, p1, p2);
}
int32_t reloq_zhtw61_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_zhtw61_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_mark_potential_phrase_breaks(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_mark_potential_phrase_breaks(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_potential_phrase_breaks,args,4,4);
    }
    return reloq_zhtw61_mark_potential_phrase_breaks(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_mark_spr_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_spr_syll_vals,args,6,6);
    }
    return reloq_zhtw61_mark_spr_syll_vals(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_mark_uppercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_mark_uppercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_uppercase,args,3,3);
    }
    return reloq_zhtw61_mark_uppercase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_mark_word_categories(void *, int32_t);
static inline int32_t reloq_call_zhtw61_mark_word_categories(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_categories,args,1,1);
    }
    return reloq_zhtw61_mark_word_categories(state, p0);
}
int32_t reloq_zhtw61_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_mark_word_str_annot(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_str_annot,args,3,3);
    }
    return reloq_zhtw61_mark_word_str_annot(state, p0, p1, p2);
}
int32_t reloq_zhtw61_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_zhtw61_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_zhtw61_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_zhtw61_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_zhtw61_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_zhtw61_misc_chars(state, p0, p1, p2);
}
int32_t reloq_zhtw61_modulate_nas_AV(void *, int32_t);
static inline int32_t reloq_call_zhtw61_modulate_nas_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_nas_AV,args,1,1);
    }
    return reloq_zhtw61_modulate_nas_AV(state, p0);
}
int32_t reloq_zhtw61_modulate_noise(void *, int32_t);
static inline int32_t reloq_call_zhtw61_modulate_noise(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_noise,args,1,1);
    }
    return reloq_zhtw61_modulate_noise(state, p0);
}
int32_t reloq_zhtw61_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_zhtw61_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_multisyll_sandhi(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_multisyll_sandhi(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_multisyll_sandhi,args,3,3);
    }
    return reloq_zhtw61_multisyll_sandhi(state, p0, p1, p2);
}
int32_t reloq_zhtw61_multisyllword_adjust(void *, int32_t);
static inline int32_t reloq_call_zhtw61_multisyllword_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_multisyllword_adjust,args,1,1);
    }
    return reloq_zhtw61_multisyllword_adjust(state, p0);
}
int32_t reloq_zhtw61_multisyllword_adjust_trandur(void *, int32_t, int32_t);
int32_t reloq_zhtw61_nasalize_chi_vow(void *, int32_t);
static inline int32_t reloq_call_zhtw61_nasalize_chi_vow(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nasalize_chi_vow,args,1,1);
    }
    return reloq_zhtw61_nasalize_chi_vow(state, p0);
}
int32_t reloq_zhtw61_new_chi_word_cat_anno(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_zhtw61_no_readin(state, p0, p1);
}
int32_t reloq_zhtw61_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_zhtw61_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_zhtw61_no_words_in_delta(state, p0);
}
int32_t reloq_zhtw61_normalize_chi_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_normalize_chi_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_chi_letters,args,3,3);
    }
    return reloq_zhtw61_normalize_chi_letters(state, p0, p1, p2);
}
int32_t reloq_zhtw61_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_zhtw61_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_zhtw61_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_zhtw61_normalize_text(state, p0, p1, p2);
}
int32_t reloq_zhtw61_nucl_high_low_star_tone(void *, int32_t);
int32_t reloq_zhtw61_nucl_high_star_low_tone(void *, int32_t);
int32_t reloq_zhtw61_nucl_high_tone(void *, int32_t);
int32_t reloq_zhtw61_nucl_low_high_star_tone(void *, int32_t);
int32_t reloq_zhtw61_nucl_low_star_high_tone(void *, int32_t);
int32_t reloq_zhtw61_nucl_low_tone(void *, int32_t);
int32_t reloq_zhtw61_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_zhtw61_nucleus_vals(state, p0);
}
int32_t reloq_zhtw61_num_sylls_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_num_sylls_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_sylls_in_range,args,4,4);
    }
    return reloq_zhtw61_num_sylls_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_zhtw61_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_zhtw61_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_paren_phrase(void *, int32_t);
static inline int32_t reloq_call_zhtw61_paren_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_paren_phrase,args,1,1);
    }
    return reloq_zhtw61_paren_phrase(state, p0);
}
int32_t reloq_zhtw61_parenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_parenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_parenthesis,args,3,3);
    }
    return reloq_zhtw61_parenthesis(state, p0, p1, p2);
}
int32_t reloq_zhtw61_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_zhtw61_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_zhtw61_period_phrase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_zhtw61_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_phr_initial_F0(void *, int32_t);
int32_t reloq_zhtw61_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_zhtw61_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_phrase_break_before_shi4(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_phrase_break_before_shi4(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_break_before_shi4,args,4,4);
    }
    return reloq_zhtw61_phrase_break_before_shi4(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_phrase_break_on_conj(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_phrase_break_on_le0(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_phrase_break_on_noun(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_phrase_break_on_noun(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_break_on_noun,args,4,4);
    }
    return reloq_zhtw61_phrase_break_on_noun(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_phrase_break_on_shortphrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_phrase_break_on_shortphrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_break_on_shortphrase,args,4,4);
    }
    return reloq_zhtw61_phrase_break_on_shortphrase(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_zhtw61_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_phrase_tone(void *, int32_t);
int32_t reloq_zhtw61_position_t_ptrs(void *, int32_t);
int32_t reloq_zhtw61_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_comma_phrase,args,3,3);
    }
    return reloq_zhtw61_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_postnucl_f0(void *, int32_t);
int32_t reloq_zhtw61_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_zhtw61_potential_sent_end(state, p0, p1);
}
int32_t reloq_zhtw61_pre_nasal(void *, int32_t);
int32_t reloq_zhtw61_precedes_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_precedes_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_end_quote,args,2,2);
    }
    return reloq_zhtw61_precedes_end_quote(state, p0, p1);
}
int32_t reloq_zhtw61_precedes_number(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_prenucl_f0(void *, int32_t);
int32_t reloq_zhtw61_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_zhtw61_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_zhtw61_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_zhtw61_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_zhtw61_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_zhtw61_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_zhtw61_print_chi_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_print_chi_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_chi_SPR,args,3,3);
    }
    return reloq_zhtw61_print_chi_SPR(state, p0, p1, p2);
}
int32_t reloq_zhtw61_print_prompt(void *, int32_t);
static inline int32_t reloq_call_zhtw61_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_zhtw61_print_prompt(state, p0);
}
int32_t reloq_zhtw61_print_spr(void *, int32_t);
static inline int32_t reloq_call_zhtw61_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_zhtw61_print_spr(state, p0);
}
int32_t reloq_zhtw61_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_zhtw61_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_zhtw61_print_word_stress(state, p0, p1);
}
int32_t reloq_zhtw61_printouts(void *, int32_t);
static inline int32_t reloq_call_zhtw61_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_zhtw61_printouts(state, p0);
}
int32_t reloq_zhtw61_process_amer_zip_code(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_process_amer_zip_code(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_amer_zip_code,args,3,3);
    }
    return reloq_zhtw61_process_amer_zip_code(state, p0, p1, p2);
}
int32_t reloq_zhtw61_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_zhtw61_process_annotation(state, p0, p1, p2);
}
int32_t reloq_zhtw61_process_final_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_process_final_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_apostrophe,args,3,3);
    }
    return reloq_zhtw61_process_final_apostrophe(state, p0, p1, p2);
}
int32_t reloq_zhtw61_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_process_final_parenthetical(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_parenthetical,args,3,3);
    }
    return reloq_zhtw61_process_final_parenthetical(state, p0, p1, p2);
}
int32_t reloq_zhtw61_process_hostname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_process_hostname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_hostname,args,3,3);
    }
    return reloq_zhtw61_process_hostname(state, p0, p1, p2);
}
int32_t reloq_zhtw61_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_zhtw61_process_input(state, p0, p1);
}
int32_t reloq_zhtw61_process_pathname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_process_pathname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_pathname,args,3,3);
    }
    return reloq_zhtw61_process_pathname(state, p0, p1, p2);
}
int32_t reloq_zhtw61_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_zhtw61_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_zhtw61_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_zhtw61_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_zhtw61_punctuation(state, p0, p1, p2);
}
int32_t reloq_zhtw61_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_zhtw61_question_phrase(state, p0, p1, p2);
}
int32_t reloq_zhtw61_quote_mark(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_quote_mark(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_quote_mark,args,3,3);
    }
    return reloq_zhtw61_quote_mark(state, p0, p1, p2);
}
int32_t reloq_zhtw61_read_end(void *, int32_t);
static inline int32_t reloq_call_zhtw61_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_zhtw61_read_end(state, p0);
}
int32_t reloq_zhtw61_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_zhtw61_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_zhtw61_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_zhtw61_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_zhtw61_register_SAPI_voices(state, p0);
}
int32_t reloq_zhtw61_register_chi_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_register_chi_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_chi_phones,args,3,3);
    }
    return reloq_zhtw61_register_chi_phones(state, p0, p1, p2);
}
int32_t reloq_zhtw61_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_zhtw61_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_reset_chi_sent_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_reset_chi_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_chi_sent_vars,args,1,1);
    }
    return reloq_zhtw61_reset_chi_sent_vars(state, p0);
}
int32_t reloq_zhtw61_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_reset_endptr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_endptr,args,4,4);
    }
    return reloq_zhtw61_reset_endptr(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_zhtw61_reset_lang_sent_vars(state, p0);
}
int32_t reloq_zhtw61_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_zhtw61_reset_phrase_vars(state, p0);
}
int32_t reloq_zhtw61_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_zhtw61_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_zhtw61_reset_sent_vars(state, p0);
}
int32_t reloq_zhtw61_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_zhtw61_reset_token_vars(state, p0, p1);
}
int32_t reloq_zhtw61_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_zhtw61_reset_word_vals(state, p0, p1);
}
int32_t reloq_zhtw61_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_zhtw61_run_filters(state, p0, p1, p2);
}
int32_t reloq_zhtw61_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_zhtw61_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_zhtw61_run_speech_module(state, p0);
}
int32_t reloq_zhtw61_run_text_module(void *, int32_t);
static inline int32_t reloq_call_zhtw61_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_zhtw61_run_text_module(state, p0);
}
int32_t reloq_zhtw61_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_same_phones(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_same_phones(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_same_phones,args,5,5);
    }
    return reloq_zhtw61_same_phones(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_zhtw61_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_zhtw61_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_zhtw61_scale_word_final_trans(state, p0);
}
int32_t reloq_zhtw61_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_zhtw61_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_zhtw61_semicolon_phrase(state, p0);
}
int32_t reloq_zhtw61_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_zhtw61_separate_tokens(state, p0, p1);
}
int32_t reloq_zhtw61_set_chi_global_constants(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_chi_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_chi_global_constants,args,1,1);
    }
    return reloq_zhtw61_set_chi_global_constants(state, p0);
}
int32_t reloq_zhtw61_set_chi_min_syll_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_set_chi_min_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_chi_min_syll_vals,args,4,4);
    }
    return reloq_zhtw61_set_chi_min_syll_vals(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_zhtw61_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_zhtw61_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_zhtw61_set_f0_wind_and_base(state, p0);
}
int32_t reloq_zhtw61_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_zhtw61_set_global_constants(state, p0);
}
int32_t reloq_zhtw61_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_zhtw61_set_klattparms(state, p0);
}
int32_t reloq_zhtw61_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_zhtw61_set_lang_global_constants(state, p0);
}
int32_t reloq_zhtw61_set_letter_name_wordcats(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_letter_name_wordcats(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_letter_name_wordcats,args,1,1);
    }
    return reloq_zhtw61_set_letter_name_wordcats(state, p0);
}
int32_t reloq_zhtw61_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_zhtw61_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_zhtw61_set_num_formants(state, p0);
}
int32_t reloq_zhtw61_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_zhtw61_set_phone_offset_vals(state, p0);
}
int32_t reloq_zhtw61_set_phr_init_form_vals(void *, int32_t);
int32_t reloq_zhtw61_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_zhtw61_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_zhtw61_set_said_phr_vals(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_zhtw61_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_zhtw61_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_zhtw61_set_syll_offset_vals(state, p0);
}
int32_t reloq_zhtw61_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_zhtw61_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_zhtw61_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_zhtw61_set_voice_param_constants(state, p0);
}
int32_t reloq_zhtw61_set_window_size(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_zhtw61_set_window_size(state, p0);
}
int32_t reloq_zhtw61_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_zhtw61_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_zhtw61_setdur(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_setdur(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setdur,args,3,3);
    }
    return reloq_zhtw61_setdur(state, p0, p1, p2);
}
int32_t reloq_zhtw61_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_zhtw61_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_zhtw61_shortdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_shortdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_shortdash,args,3,3);
    }
    return reloq_zhtw61_shortdash(state, p0, p1, p2);
}
int32_t reloq_zhtw61_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_zhtw61_single_chars(state, p0, p1, p2);
}
int32_t reloq_zhtw61_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_zhtw61_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_zhtw61_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_zhtw61_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_zhtw61_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_zhtw61_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_zhtw61_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_zhtw61_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_zhtw61_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_zhtw61_single_letter(state, p0, p1, p2);
}
int32_t reloq_zhtw61_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_zhtw61_size_filter(state, p0, p1, p2);
}
int32_t reloq_zhtw61_skip_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_skip_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_delimiters,args,2,2);
    }
    return reloq_zhtw61_skip_delimiters(state, p0, p1);
}
int32_t reloq_zhtw61_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_zhtw61_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_zhtw61_slash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_slash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash,args,3,3);
    }
    return reloq_zhtw61_slash(state, p0, p1, p2);
}
int32_t reloq_zhtw61_slash_before_measure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_slash_before_measure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash_before_measure,args,3,3);
    }
    return reloq_zhtw61_slash_before_measure(state, p0, p1, p2);
}
int32_t reloq_zhtw61_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_zhtw61_space(state, p0, p1, p2);
}
int32_t reloq_zhtw61_span_inverted_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_span_inverted_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_span_inverted_punct,args,3,3);
    }
    return reloq_zhtw61_span_inverted_punct(state, p0, p1, p2);
}
int32_t reloq_zhtw61_specialword_adjust(void *, int32_t);
static inline int32_t reloq_call_zhtw61_specialword_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_specialword_adjust,args,1,1);
    }
    return reloq_zhtw61_specialword_adjust(state, p0);
}
int32_t reloq_zhtw61_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_zhtw61_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_zhtw61_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_zhtw61_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_zhtw61_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_zhtw61_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_zhtw61_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_stanchi_adjust_for_tone(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_adjust_for_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_adjust_for_tone,args,1,1);
    }
    return reloq_zhtw61_stanchi_adjust_for_tone(state, p0);
}
int32_t reloq_zhtw61_stanchi_gram_adjust_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_gram_adjust_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_gram_adjust_dur,args,1,1);
    }
    return reloq_zhtw61_stanchi_gram_adjust_dur(state, p0);
}
int32_t reloq_zhtw61_stanchi_obj_adjust_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_obj_adjust_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_obj_adjust_dur,args,1,1);
    }
    return reloq_zhtw61_stanchi_obj_adjust_dur(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_A(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_A(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_A,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_A(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_B(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_B(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_B,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_B(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_E(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_E,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_E(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_I(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_I(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_I,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_I(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_O(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_O(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_O,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_O(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_R(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_R(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_R,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_R(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_U(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_U(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_U,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_U(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_V(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_V(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_V,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_V(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_W(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_W(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_W,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_W(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_Y(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_Y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_Y,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_Y(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_a(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_a,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_a(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_ac(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_ac(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_ac,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_ac(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_c(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_c(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_c,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_c(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_e(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_e,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_e(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_i(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_i,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_i(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_o(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_o,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_o(state, p0);
}
int32_t reloq_zhtw61_stanchi_ph_u(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_ph_u,args,1,1);
    }
    return reloq_zhtw61_stanchi_ph_u(state, p0);
}
int32_t reloq_zhtw61_stanchi_phrase_final_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_phrase_final_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_phrase_final_dur,args,1,1);
    }
    return reloq_zhtw61_stanchi_phrase_final_dur(state, p0);
}
int32_t reloq_zhtw61_stanchi_shorten_after_asp(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_shorten_after_asp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_shorten_after_asp,args,1,1);
    }
    return reloq_zhtw61_stanchi_shorten_after_asp(state, p0);
}
int32_t reloq_zhtw61_stanchi_vals(void *, int32_t);
static inline int32_t reloq_call_zhtw61_stanchi_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanchi_vals,args,1,1);
    }
    return reloq_zhtw61_stanchi_vals(state, p0);
}
int32_t reloq_zhtw61_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_zhtw61_stretch_focus(state, p0, p1);
}
int32_t reloq_zhtw61_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_zhtw61_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_string_to_short(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_short,args,4,4);
    }
    return reloq_zhtw61_string_to_short(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_strip_last_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_strip_last_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_last_digit,args,3,3);
    }
    return reloq_zhtw61_strip_last_digit(state, p0, p1, p2);
}
int32_t reloq_zhtw61_syll_chi_con_tone_adjust(void *, int32_t);
static inline int32_t reloq_call_zhtw61_syll_chi_con_tone_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_syll_chi_con_tone_adjust,args,1,1);
    }
    return reloq_zhtw61_syll_chi_con_tone_adjust(state, p0);
}
int32_t reloq_zhtw61_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_zhtw61_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_zhtw61_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_zhtw61_text_index(state, p0, p1);
}
int32_t reloq_zhtw61_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_zhtw61_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_three_letter_extension(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_three_letter_extension(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_letter_extension,args,2,2);
    }
    return reloq_zhtw61_three_letter_extension(state, p0, p1);
}
int32_t reloq_zhtw61_title_adjust_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_title_adjust_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_title_adjust_dur,args,1,1);
    }
    return reloq_zhtw61_title_adjust_dur(state, p0);
}
int32_t reloq_zhtw61_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict,args,3,3);
    }
    return reloq_zhtw61_tok_dict(state, p0, p1, p2);
}
int32_t reloq_zhtw61_tok_dict_entry(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_tok_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict_entry,args,3,3);
    }
    return reloq_zhtw61_tok_dict_entry(state, p0, p1, p2);
}
int32_t reloq_zhtw61_tok_lookup(void *, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_tok_lookup(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_lookup,args,2,2);
    }
    return reloq_zhtw61_tok_lookup(state, p0, p1);
}
int32_t reloq_zhtw61_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_zhtw61_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_zhtw61_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_zhtw61_token_to_number(state, p0, p1, p2);
}
int32_t reloq_zhtw61_tone_sandhi(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_tone_sandhi(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tone_sandhi,args,3,3);
    }
    return reloq_zhtw61_tone_sandhi(state, p0, p1, p2);
}
int32_t reloq_zhtw61_user_abbr_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_user_abbr_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_abbr_dict,args,3,3);
    }
    return reloq_zhtw61_user_abbr_dict(state, p0, p1, p2);
}
int32_t reloq_zhtw61_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_zhtw61_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_user_index_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_user_index_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_index_anno,args,3,3);
    }
    return reloq_zhtw61_user_index_anno(state, p0, p1, p2);
}
int32_t reloq_zhtw61_user_rootdict(void *, int32_t, int32_t, int32_t);
int32_t reloq_zhtw61_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_zhtw61_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_zhtw61_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_zhtw61_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_which_get_non_vowels(void *, int32_t);
static inline int32_t reloq_call_zhtw61_which_get_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_which_get_non_vowels,args,1,1);
    }
    return reloq_zhtw61_which_get_non_vowels(state, p0);
}
int32_t reloq_zhtw61_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_zhtw61_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_word_cat_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_word_cat_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_cat_anno,args,3,3);
    }
    return reloq_zhtw61_word_cat_anno(state, p0, p1, p2);
}
int32_t reloq_zhtw61_word_con_stress_adjust_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_word_con_stress_adjust_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_con_stress_adjust_dur,args,1,1);
    }
    return reloq_zhtw61_word_con_stress_adjust_dur(state, p0);
}
int32_t reloq_zhtw61_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_zhtw61_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_zhtw61_word_stress_adjust_dur(void *, int32_t);
static inline int32_t reloq_call_zhtw61_word_stress_adjust_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_stress_adjust_dur,args,1,1);
    }
    return reloq_zhtw61_word_stress_adjust_dur(state, p0);
}
int32_t reloq_zhtw61_word_stress_adjust_trandur(void *, int32_t, int32_t);
int32_t reloq_zhtw61_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_zhtw61_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_zhtw61_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_zhtw61_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_zhtw61_year_mode_anno(state, p0, p1, p2, p3);
}
#endif
