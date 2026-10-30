/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_FIFI_H
#define RELOQ61_RULE_CALLS_FIFI_H
#include "delta_rules_c.h"
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_fifi_DeltaProc_end(void *, int32_t);
int32_t reloq_fifi_DeltaProc_flush(void *, int32_t);
int32_t reloq_fifi_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_fifi_DeltaProc_main(void *, int32_t);
int32_t reloq_fifi_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_fifi_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_fifi_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_fifi_DeltaProc_start(void *, int32_t);
int32_t reloq_fifi_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_fifi_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_fifi_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_fifi_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_URL(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_URL(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_URL,args,2,2);
    }
    return reloq_fifi_URL(state, p0, p1);
}
int32_t reloq_fifi_a_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_a_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_a_rules,args,3,3);
    }
    return reloq_fifi_a_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbr,args,4,4);
    }
    return reloq_fifi_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_abbr_before_num(void *, int32_t, int32_t);
int32_t reloq_fifi_abbreviation(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_abbreviation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbreviation,args,4,4);
    }
    return reloq_fifi_abbreviation(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_fifi_absval(state, p0, p1, p2);
}
int32_t reloq_fifi_acronym(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_acronym(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acronym,args,3,3);
    }
    return reloq_fifi_acronym(state, p0, p1, p2);
}
int32_t reloq_fifi_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acrotest,args,3,3);
    }
    return reloq_fifi_acrotest(state, p0, p1, p2);
}
int32_t reloq_fifi_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_fifi_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_fifi_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_add_TL_to_trill(void *, int32_t);
static inline int32_t reloq_call_fifi_add_TL_to_trill(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_TL_to_trill,args,1,1);
    }
    return reloq_fifi_add_TL_to_trill(state, p0);
}
int32_t reloq_fifi_add_abbr_case_and_num(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_abbr_case_and_num(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_abbr_case_and_num,args,2,2);
    }
    return reloq_fifi_add_abbr_case_and_num(state, p0, p1);
}
int32_t reloq_fifi_add_case(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_case(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_case,args,6,6);
    }
    return reloq_fifi_add_case(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_add_case_to_measure_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_case_to_measure_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_case_to_measure_abbr,args,3,3);
    }
    return reloq_fifi_add_case_to_measure_abbr(state, p0, p1, p2);
}
int32_t reloq_fifi_add_f0_for_new_voice(void *, int32_t);
static inline int32_t reloq_call_fifi_add_f0_for_new_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_new_voice,args,1,1);
    }
    return reloq_fifi_add_f0_for_new_voice(state, p0);
}
int32_t reloq_fifi_add_f0_for_next_voice(void *, int32_t);
static inline int32_t reloq_call_fifi_add_f0_for_next_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_next_voice,args,1,1);
    }
    return reloq_fifi_add_f0_for_next_voice(state, p0);
}
int32_t reloq_fifi_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_fifi_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_fifi_add_fin_currency_units(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_fin_currency_units(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_fin_currency_units,args,5,5);
    }
    return reloq_fifi_add_fin_currency_units(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_add_fin_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_fin_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_fin_point_digits,args,3,3);
    }
    return reloq_fifi_add_fin_point_digits(state, p0, p1, p2);
}
int32_t reloq_fifi_add_fin_subunits(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_fin_subunits(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_fin_subunits,args,5,5);
    }
    return reloq_fifi_add_fin_subunits(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_add_final_n(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_final_n(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_final_n,args,2,2);
    }
    return reloq_fifi_add_final_n(state, p0, p1);
}
int32_t reloq_fifi_add_harmonic_suffix_vow(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_harmonic_suffix_vow(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_harmonic_suffix_vow,args,4,4);
    }
    return reloq_fifi_add_harmonic_suffix_vow(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_add_num_case(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_num_case(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_num_case,args,5,5);
    }
    return reloq_fifi_add_num_case(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_add_ordinal_ending(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_ordinal_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ordinal_ending,args,4,4);
    }
    return reloq_fifi_add_ordinal_ending(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_add_tilt(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_add_tilt(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_tilt,args,2,2);
    }
    return reloq_fifi_add_tilt(state, p0, p1);
}
int32_t reloq_fifi_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_fifi_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_fifi_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_fifi_adjust_AV_for_fem_voice(void *, int32_t);
static inline int32_t reloq_call_fifi_adjust_AV_for_fem_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_fem_voice,args,1,1);
    }
    return reloq_fifi_adjust_AV_for_fem_voice(state, p0);
}
int32_t reloq_fifi_adjust_AV_for_misc(void *, int32_t);
static inline int32_t reloq_call_fifi_adjust_AV_for_misc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_misc,args,1,1);
    }
    return reloq_fifi_adjust_AV_for_misc(state, p0);
}
int32_t reloq_fifi_adjust_AV_for_vow_vow_sequence(void *, int32_t);
static inline int32_t reloq_call_fifi_adjust_AV_for_vow_vow_sequence(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_vow_vow_sequence,args,1,1);
    }
    return reloq_fifi_adjust_AV_for_vow_vow_sequence(state, p0);
}
int32_t reloq_fifi_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_fifi_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_adjust_acro_case_ending(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_acro_case_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_acro_case_ending,args,4,4);
    }
    return reloq_fifi_adjust_acro_case_ending(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_adjust_case_before_partitive(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_case_before_partitive(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_case_before_partitive,args,5,5);
    }
    return reloq_fifi_adjust_case_before_partitive(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_adjust_case_ending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_case_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_case_ending,args,5,5);
    }
    return reloq_fifi_adjust_case_ending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_adjust_f0_for_sentence_type(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_f0_for_sentence_type(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_f0_for_sentence_type,args,2,2);
    }
    return reloq_fifi_adjust_f0_for_sentence_type(state, p0, p1);
}
int32_t reloq_fifi_adjust_fin_accents(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_fin_accents(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_fin_accents,args,3,3);
    }
    return reloq_fifi_adjust_fin_accents(state, p0, p1, p2);
}
int32_t reloq_fifi_adjust_fin_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_fin_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_fin_word_stress,args,3,3);
    }
    return reloq_fifi_adjust_fin_word_stress(state, p0, p1, p2);
}
int32_t reloq_fifi_adjust_lg_specific_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_lg_specific_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_lg_specific_accents,args,5,5);
    }
    return reloq_fifi_adjust_lg_specific_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_adjust_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_phones,args,3,3);
    }
    return reloq_fifi_adjust_phones(state, p0, p1, p2);
}
int32_t reloq_fifi_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_prev_phr_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_pause,args,3,3);
    }
    return reloq_fifi_adjust_prev_phr_pause(state, p0, p1, p2);
}
int32_t reloq_fifi_adjust_suffix_vow(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_suffix_vow(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_suffix_vow,args,3,3);
    }
    return reloq_fifi_adjust_suffix_vow(state, p0, p1, p2);
}
int32_t reloq_fifi_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_fifi_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_fifi_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adjust_words_in_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_words_in_phrase,args,3,3);
    }
    return reloq_fifi_adjust_words_in_phrase(state, p0, p1, p2);
}
int32_t reloq_fifi_adverb_vs_nounadj(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_adverb_vs_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adverb_vs_nounadj,args,4,4);
    }
    return reloq_fifi_adverb_vs_nounadj(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_ambig_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_ambig_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ambig_abbr,args,2,2);
    }
    return reloq_fifi_ambig_abbr(state, p0, p1);
}
int32_t reloq_fifi_analyze_compound(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_analyze_compound(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_analyze_compound,args,3,3);
    }
    return reloq_fifi_analyze_compound(state, p0, p1, p2);
}
int32_t reloq_fifi_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_fifi_apostrophe(state, p0, p1, p2);
}
int32_t reloq_fifi_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_fifi_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_fifi_apply_comma_phr_rules(void *, int32_t);
static inline int32_t reloq_call_fifi_apply_comma_phr_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_comma_phr_rules,args,1,1);
    }
    return reloq_fifi_apply_comma_phr_rules(state, p0);
}
int32_t reloq_fifi_apply_cons_gradation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_cons_gradation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_cons_gradation,args,3,3);
    }
    return reloq_fifi_apply_cons_gradation(state, p0, p1, p2);
}
int32_t reloq_fifi_apply_fin_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_fin_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_fin_comma_rules,args,3,3);
    }
    return reloq_fifi_apply_fin_comma_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_fifi_apply_head_size_val(state, p0, p1);
}
int32_t reloq_fifi_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_lang_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_lang_comma_rules,args,3,3);
    }
    return reloq_fifi_apply_lang_comma_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_fifi_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_fifi_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_fifi_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_fifi_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_fifi_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_fifi_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_fifi_apply_roughness_val(state, p0, p1);
}
int32_t reloq_fifi_apply_rules(void *, int32_t);
static inline int32_t reloq_call_fifi_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_fifi_apply_rules(state, p0);
}
int32_t reloq_fifi_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_fifi_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_fifi_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_fifi_apply_speed_val(state, p0, p1);
}
int32_t reloq_fifi_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_fifi_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_fifi_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_fifi_apply_volume_val(state, p0, p1);
}
int32_t reloq_fifi_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_fifi_assign_acoustic_vals(state, p0);
}
int32_t reloq_fifi_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_fifi_assign_alt_quest_features(state, p0);
}
int32_t reloq_fifi_assign_con_phonemes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_con_phonemes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_con_phonemes,args,3,3);
    }
    return reloq_fifi_assign_con_phonemes(state, p0, p1, p2);
}
int32_t reloq_fifi_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_fifi_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_fifi_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_fifi_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_fifi_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_fifi_assign_default_quest_features(state, p0);
}
int32_t reloq_fifi_assign_fin_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_fin_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_fin_nuc_durs,args,1,1);
    }
    return reloq_fifi_assign_fin_nuc_durs(state, p0);
}
int32_t reloq_fifi_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_fifi_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_fifi_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_fifi_assign_nuc_durs(state, p0);
}
int32_t reloq_fifi_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_fifi_assign_phone_durs(state, p0);
}
int32_t reloq_fifi_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_fifi_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_fifi_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_fifi_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_fifi_assign_sent_final_features(state, p0);
}
int32_t reloq_fifi_assign_stanfin_start_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_stanfin_start_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_stanfin_start_dur,args,1,1);
    }
    return reloq_fifi_assign_stanfin_start_dur(state, p0);
}
int32_t reloq_fifi_assign_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_stress,args,3,3);
    }
    return reloq_fifi_assign_stress(state, p0, p1, p2);
}
int32_t reloq_fifi_assign_syll_onset(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_syll_onset(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_syll_onset,args,2,2);
    }
    return reloq_fifi_assign_syll_onset(state, p0, p1);
}
int32_t reloq_fifi_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_fifi_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_fifi_assign_user_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_user_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_user_stress,args,3,3);
    }
    return reloq_fifi_assign_user_stress(state, p0, p1, p2);
}
int32_t reloq_fifi_assign_vow_phoneme(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_assign_vow_phoneme(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_vow_phoneme,args,3,3);
    }
    return reloq_fifi_assign_vow_phoneme(state, p0, p1, p2);
}
int32_t reloq_fifi_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_fifi_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_fifi_assign_wh_quest_features(state, p0);
}
int32_t reloq_fifi_beg_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_beg_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_beg_of_word,args,2,2);
    }
    return reloq_fifi_beg_of_word(state, p0, p1);
}
int32_t reloq_fifi_between_proper_names(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_boundary_tone,args,1,1);
    }
    return reloq_fifi_boundary_tone(state, p0);
}
int32_t reloq_fifi_bracket(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_bracket(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_bracket,args,3,3);
    }
    return reloq_fifi_bracket(state, p0, p1, p2);
}
int32_t reloq_fifi_break_into_morphs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_break_into_morphs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_morphs,args,3,3);
    }
    return reloq_fifi_break_into_morphs(state, p0, p1, p2);
}
int32_t reloq_fifi_break_into_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_break_into_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_phones,args,3,3);
    }
    return reloq_fifi_break_into_phones(state, p0, p1, p2);
}
int32_t reloq_fifi_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_fifi_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_fifi_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_fifi_build_phrases(state, p0, p1, p2);
}
int32_t reloq_fifi_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_fifi_build_words(state, p0, p1, p2);
}
int32_t reloq_fifi_c_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_c_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_c_rules,args,3,3);
    }
    return reloq_fifi_c_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_fifi_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_fifi_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_fifi_char_name(state, p0, p1, p2);
}
int32_t reloq_fifi_clear_delta(void *, int32_t);
static inline int32_t reloq_call_fifi_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_fifi_clear_delta(state, p0);
}
int32_t reloq_fifi_close_platform(void *, int32_t);
static inline int32_t reloq_call_fifi_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_fifi_close_platform(state, p0);
}
int32_t reloq_fifi_comma_at_end_quote(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_comma_at_end_quote(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_comma_at_end_quote,args,3,3);
    }
    return reloq_fifi_comma_at_end_quote(state, p0, p1, p2);
}
int32_t reloq_fifi_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_fifi_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_fifi_comma_phrase(state, p0);
}
int32_t reloq_fifi_compound_con_cluster(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_compound_con_cluster(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_compound_con_cluster,args,2,2);
    }
    return reloq_fifi_compound_con_cluster(state, p0, p1);
}
int32_t reloq_fifi_compound_vow_cluster(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_compound_vow_cluster(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_compound_vow_cluster,args,2,2);
    }
    return reloq_fifi_compound_vow_cluster(state, p0, p1);
}
int32_t reloq_fifi_con_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_fifi_con_vals(state, p0);
}
int32_t reloq_fifi_convert_1_or_2_digits(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_convert_date(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_date(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8)
{
    if(delta_rule_trace) {
        const int32_t args[]={p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_date,args,9,9);
    }
    return reloq_fifi_convert_date(state, p0, p1, p2, p3, p4, p5, p6, p7, p8);
}
int32_t reloq_fifi_convert_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits,args,3,3);
    }
    return reloq_fifi_convert_digits(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits_to_words,args,3,3);
    }
    return reloq_fifi_convert_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_double,args,3,3);
    }
    return reloq_fifi_convert_double(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_1_or_2_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_1_or_2_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_1_or_2_digits,args,3,3);
    }
    return reloq_fifi_convert_fin_1_or_2_digits(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_date,args,3,3);
    }
    return reloq_fifi_convert_fin_date(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_digits,args,3,3);
    }
    return reloq_fifi_convert_fin_digits(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_digits_to_words,args,3,3);
    }
    return reloq_fifi_convert_fin_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_double,args,3,3);
    }
    return reloq_fifi_convert_fin_double(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_fraction(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_fraction(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_fraction,args,6,6);
    }
    return reloq_fifi_convert_fin_fraction(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_convert_fin_fraction_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_fraction_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_fraction_char,args,3,3);
    }
    return reloq_fifi_convert_fin_fraction_char(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_hundreds,args,3,3);
    }
    return reloq_fifi_convert_fin_hundreds(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_monetary_exp,args,5,5);
    }
    return reloq_fifi_convert_fin_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_convert_fin_sports_time(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_sports_time(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_sports_time,args,3,3);
    }
    return reloq_fifi_convert_fin_sports_time(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_fin_time(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_fin_time(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fin_time,args,3,3);
    }
    return reloq_fifi_convert_fin_time(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_hundreds,args,3,3);
    }
    return reloq_fifi_convert_hundreds(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_lang_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_lang_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_lang_digits_to_words,args,3,3);
    }
    return reloq_fifi_convert_lang_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_large_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_large_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_large_numbers,args,3,3);
    }
    return reloq_fifi_convert_large_numbers(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_mixed_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_mixed_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_mixed_num,args,3,3);
    }
    return reloq_fifi_convert_mixed_num(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_monetary_exp,args,5,5);
    }
    return reloq_fifi_convert_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_convert_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_numbers,args,3,3);
    }
    return reloq_fifi_convert_numbers(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_roman_nums(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_convert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_single_digit,args,3,3);
    }
    return reloq_fifi_convert_single_digit(state, p0, p1, p2);
}
int32_t reloq_fifi_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_fifi_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_fifi_coord_conj(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_coord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_coord_conj,args,4,4);
    }
    return reloq_fifi_coord_conj(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_fifi_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_fifi_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_fifi_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_fifi_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_fifi_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_count_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_count_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_digits,args,3,3);
    }
    return reloq_fifi_count_digits(state, p0, p1, p2);
}
int32_t reloq_fifi_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_fifi_count_input(state, p0, p1);
}
int32_t reloq_fifi_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_fifi_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_fifi_count_normalized_inp(state, p0);
}
int32_t reloq_fifi_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_count_sylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_sylls,args,4,4);
    }
    return reloq_fifi_count_sylls(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_fifi_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_fifi_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_fifi_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_fifi_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_fifi_create_sentence(state, p0, p1);
}
int32_t reloq_fifi_create_syllables(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_create_syllables(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_syllables,args,3,3);
    }
    return reloq_fifi_create_syllables(state, p0, p1, p2);
}
int32_t reloq_fifi_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_fifi_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_fifi_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_fifi_delete_2_more(state, p0);
}
int32_t reloq_fifi_delete_comma_sync(void *, int32_t, int32_t);
int32_t reloq_fifi_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delete_inp_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_left,args,3,3);
    }
    return reloq_fifi_delete_inp_from_left(state, p0, p1, p2);
}
int32_t reloq_fifi_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delete_inp_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_right,args,3,3);
    }
    return reloq_fifi_delete_inp_from_right(state, p0, p1, p2);
}
int32_t reloq_fifi_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_fifi_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_fifi_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delete_morph_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_morph_from_left,args,3,3);
    }
    return reloq_fifi_delete_morph_from_left(state, p0, p1, p2);
}
int32_t reloq_fifi_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_fifi_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_fifi_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delete_phone_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_right,args,3,3);
    }
    return reloq_fifi_delete_phone_from_right(state, p0, p1, p2);
}
int32_t reloq_fifi_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_fifi_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_fifi_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_fifi_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_fifi_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_fifi_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_fifi_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_fifi_demote_word(state, p0, p1, p2);
}
int32_t reloq_fifi_destress_adpos(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_destress_adpos(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_adpos,args,4,4);
    }
    return reloq_fifi_destress_adpos(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_destress_adv(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_destress_adv(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_adv,args,4,4);
    }
    return reloq_fifi_destress_adv(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_destress_nps(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_destress_nps(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_nps,args,3,3);
    }
    return reloq_fifi_destress_nps(state, p0, p1, p2);
}
int32_t reloq_fifi_destress_verbs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_destress_verbs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_verbs,args,3,3);
    }
    return reloq_fifi_destress_verbs(state, p0, p1, p2);
}
int32_t reloq_fifi_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_fifi_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_disambig_adpos_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambig_adpos_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_adpos_verb,args,3,3);
    }
    return reloq_fifi_disambig_adpos_verb(state, p0, p1, p2);
}
int32_t reloq_fifi_disambig_adv_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambig_adv_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_adv_verb,args,3,3);
    }
    return reloq_fifi_disambig_adv_verb(state, p0, p1, p2);
}
int32_t reloq_fifi_disambig_coord_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambig_coord_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_coord_verb,args,3,3);
    }
    return reloq_fifi_disambig_coord_verb(state, p0, p1, p2);
}
int32_t reloq_fifi_disambig_dem_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambig_dem_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_dem_verb,args,3,3);
    }
    return reloq_fifi_disambig_dem_verb(state, p0, p1, p2);
}
int32_t reloq_fifi_disambig_fin_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambig_fin_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_fin_content_word,args,4,4);
    }
    return reloq_fifi_disambig_fin_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_disambig_fin_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambig_fin_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_fin_functword,args,3,3);
    }
    return reloq_fifi_disambig_fin_functword(state, p0, p1, p2);
}
int32_t reloq_fifi_disambig_nounadj_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambig_nounadj_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_nounadj_verb,args,3,3);
    }
    return reloq_fifi_disambig_nounadj_verb(state, p0, p1, p2);
}
int32_t reloq_fifi_disambig_pro2_rel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambig_pro2_rel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_pro2_rel,args,3,3);
    }
    return reloq_fifi_disambig_pro2_rel(state, p0, p1, p2);
}
int32_t reloq_fifi_disambiguate_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambiguate_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_content_word,args,4,4);
    }
    return reloq_fifi_disambiguate_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_disambiguate_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disambiguate_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_functword,args,3,3);
    }
    return reloq_fifi_disambiguate_functword(state, p0, p1, p2);
}
int32_t reloq_fifi_disharmonic_vows(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_disharmonic_vows(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_disharmonic_vows,args,2,2);
    }
    return reloq_fifi_disharmonic_vows(state, p0, p1);
}
int32_t reloq_fifi_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_fifi_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_fifi_distribute_nucdur(state, p0);
}
int32_t reloq_fifi_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_fifi_divide_char_count_token(state, p0, p1);
}
int32_t reloq_fifi_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_fifi_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_fifi_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_fifi_doF(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_fifi_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_fifi_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_fifi_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_fifi_do_FTZ_FTP(state, p0);
}
int32_t reloq_fifi_do_fin_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_do_fin_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fin_h_amps,args,2,2);
    }
    return reloq_fifi_do_fin_h_amps(state, p0, p1);
}
int32_t reloq_fifi_do_fin_voice_bar(void *, int32_t);
static inline int32_t reloq_call_fifi_do_fin_voice_bar(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_fin_voice_bar,args,1,1);
    }
    return reloq_fifi_do_fin_voice_bar(state, p0);
}
int32_t reloq_fifi_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_fifi_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_fifi_do_glot_onset(void *, int32_t);
static inline int32_t reloq_call_fifi_do_glot_onset(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_glot_onset,args,1,1);
    }
    return reloq_fifi_do_glot_onset(state, p0);
}
int32_t reloq_fifi_do_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_do_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_h_amps,args,2,2);
    }
    return reloq_fifi_do_h_amps(state, p0, p1);
}
int32_t reloq_fifi_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_fifi_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_fifi_do_non_son_amps(state, p0);
}
int32_t reloq_fifi_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_fifi_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_fifi_do_pt_voicing(state, p0);
}
int32_t reloq_fifi_do_release(void *, int32_t);
static inline int32_t reloq_call_fifi_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_fifi_do_release(state, p0);
}
int32_t reloq_fifi_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_fifi_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_fifi_do_son_amps(state, p0);
}
int32_t reloq_fifi_do_voice_bar(void *, int32_t);
int32_t reloq_fifi_e_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_e_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_e_rules,args,3,3);
    }
    return reloq_fifi_e_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_fifi_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_fifi_ellipsis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_ellipsis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ellipsis,args,3,3);
    }
    return reloq_fifi_ellipsis(state, p0, p1, p2);
}
int32_t reloq_fifi_email_address(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_email_address(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_email_address,args,2,2);
    }
    return reloq_fifi_email_address(state, p0, p1);
}
int32_t reloq_fifi_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_fifi_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_fifi_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_fifi_end_of_sentence(state, p0, p1);
}
int32_t reloq_fifi_end_of_token(void *, int32_t, int32_t);
int32_t reloq_fifi_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_fifi_end_of_word(state, p0, p1);
}
int32_t reloq_fifi_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_fifi_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_estimate_fin_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_estimate_fin_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_fin_nsylls,args,4,4);
    }
    return reloq_fifi_estimate_fin_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_estimate_lang_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_estimate_lang_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_lang_nsylls,args,4,4);
    }
    return reloq_fifi_estimate_lang_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_estimate_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_nsylls,args,4,4);
    }
    return reloq_fifi_estimate_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_exclam_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_exclam_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_exclam_phrase,args,2,2);
    }
    return reloq_fifi_exclam_phrase(state, p0, p1);
}
int32_t reloq_fifi_extend_vals(void *, int32_t);
int32_t reloq_fifi_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_fifi_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_fifi_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_2stress_acc(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_2stress_acc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_2stress_acc,args,1,1);
    }
    return reloq_fifi_fin_2stress_acc(state, p0);
}
int32_t reloq_fifi_fin_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_abbr,args,3,3);
    }
    return reloq_fifi_fin_abbr(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_acrotest,args,3,3);
    }
    return reloq_fifi_fin_acrotest(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_alv_sibilant_Fv(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_alv_sibilant_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_alv_sibilant_Fv,args,1,1);
    }
    return reloq_fifi_fin_alv_sibilant_Fv(state, p0);
}
int32_t reloq_fifi_fin_boundary_tone(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_boundary_tone(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_boundary_tone,args,2,2);
    }
    return reloq_fifi_fin_boundary_tone(state, p0, p1);
}
int32_t reloq_fifi_fin_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_char_name,args,3,3);
    }
    return reloq_fifi_fin_char_name(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_compounds(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_compounds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_compounds,args,4,4);
    }
    return reloq_fifi_fin_compounds(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_fin_con_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_con_vals,args,1,1);
    }
    return reloq_fifi_fin_con_vals(state, p0);
}
int32_t reloq_fifi_fin_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_cons_durs,args,3,3);
    }
    return reloq_fifi_fin_cons_durs(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_context_adjust(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_context_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_context_adjust,args,1,1);
    }
    return reloq_fifi_fin_context_adjust(state, p0);
}
int32_t reloq_fifi_fin_dental_Fv(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_dental_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_dental_Fv,args,1,1);
    }
    return reloq_fifi_fin_dental_Fv(state, p0);
}
int32_t reloq_fifi_fin_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_digits,args,3,3);
    }
    return reloq_fifi_fin_digits(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_final_glide_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_final_glide_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_final_glide_dur,args,2,2);
    }
    return reloq_fifi_fin_final_glide_dur(state, p0, p1);
}
int32_t reloq_fifi_fin_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_first_postnucl_accent,args,1,1);
    }
    return reloq_fifi_fin_first_postnucl_accent(state, p0);
}
int32_t reloq_fifi_fin_funct_words(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_funct_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_funct_words,args,4,4);
    }
    return reloq_fifi_fin_funct_words(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_fin_gramm_cat_dict(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_gramm_cat_dict(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_gramm_cat_dict,args,4,4);
    }
    return reloq_fifi_fin_gramm_cat_dict(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_fin_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_high_low_star_tone,args,1,1);
    }
    return reloq_fifi_fin_high_low_star_tone(state, p0);
}
int32_t reloq_fifi_fin_high_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_high_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_high_pal_Fv,args,1,1);
    }
    return reloq_fifi_fin_high_pal_Fv(state, p0);
}
int32_t reloq_fifi_fin_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_high_star_low_tone,args,1,1);
    }
    return reloq_fifi_fin_high_star_low_tone(state, p0);
}
int32_t reloq_fifi_fin_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_high_tone,args,1,1);
    }
    return reloq_fifi_fin_high_tone(state, p0);
}
int32_t reloq_fifi_fin_hyphenated_compound(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_hyphenated_compound(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_hyphenated_compound,args,3,3);
    }
    return reloq_fifi_fin_hyphenated_compound(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_intervocalic_r(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_intervocalic_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_intervocalic_r,args,1,1);
    }
    return reloq_fifi_fin_intervocalic_r(state, p0);
}
int32_t reloq_fifi_fin_labial_Fv(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_labial_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_labial_Fv,args,1,1);
    }
    return reloq_fifi_fin_labial_Fv(state, p0);
}
int32_t reloq_fifi_fin_legal_final_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_legal_final_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_legal_final_cons,args,2,2);
    }
    return reloq_fifi_fin_legal_final_cons(state, p0, p1);
}
int32_t reloq_fifi_fin_legal_initial_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_legal_initial_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_legal_initial_cons,args,2,2);
    }
    return reloq_fifi_fin_legal_initial_cons(state, p0, p1);
}
int32_t reloq_fifi_fin_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_low_high_star_tone,args,1,1);
    }
    return reloq_fifi_fin_low_high_star_tone(state, p0);
}
int32_t reloq_fifi_fin_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_low_star_high_tone,args,1,1);
    }
    return reloq_fifi_fin_low_star_high_tone(state, p0);
}
int32_t reloq_fifi_fin_low_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_low_tone,args,1,1);
    }
    return reloq_fifi_fin_low_tone(state, p0);
}
int32_t reloq_fifi_fin_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_monetary_exp,args,3,3);
    }
    return reloq_fifi_fin_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_nas_vow(void *, int32_t);
int32_t reloq_fifi_fin_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_non_phrasal_comma,args,2,2);
    }
    return reloq_fifi_fin_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_fifi_fin_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_fifi_fin_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_fifi_fin_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_fifi_fin_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_fifi_fin_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_nucl_high_tone,args,1,1);
    }
    return reloq_fifi_fin_nucl_high_tone(state, p0);
}
int32_t reloq_fifi_fin_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_fifi_fin_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_fifi_fin_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_fifi_fin_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_fifi_fin_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_nucl_low_tone,args,1,1);
    }
    return reloq_fifi_fin_nucl_low_tone(state, p0);
}
int32_t reloq_fifi_fin_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_pal_Fv,args,1,1);
    }
    return reloq_fifi_fin_pal_Fv(state, p0);
}
int32_t reloq_fifi_fin_ph_G(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_G(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_G,args,1,1);
    }
    return reloq_fifi_fin_ph_G(state, p0);
}
int32_t reloq_fifi_fin_ph_G_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_G_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_G_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_G_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_R(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_R(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_R,args,1,1);
    }
    return reloq_fifi_fin_ph_R(state, p0);
}
int32_t reloq_fifi_fin_ph_R_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_R_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_R_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_R_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_S(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_S,args,1,1);
    }
    return reloq_fifi_fin_ph_S(state, p0);
}
int32_t reloq_fifi_fin_ph_S_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_S_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_S_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_S_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_Z(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_Z,args,1,1);
    }
    return reloq_fifi_fin_ph_Z(state, p0);
}
int32_t reloq_fifi_fin_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_Z_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_Z_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_b(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_b,args,1,1);
    }
    return reloq_fifi_fin_ph_b(state, p0);
}
int32_t reloq_fifi_fin_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_b_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_b_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_d(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_d,args,1,1);
    }
    return reloq_fifi_fin_ph_d(state, p0);
}
int32_t reloq_fifi_fin_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_d_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_d_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_f(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_f(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_f,args,1,1);
    }
    return reloq_fifi_fin_ph_f(state, p0);
}
int32_t reloq_fifi_fin_ph_f_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_f_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_f_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_f_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_g(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_g,args,1,1);
    }
    return reloq_fifi_fin_ph_g(state, p0);
}
int32_t reloq_fifi_fin_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_g_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_g_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_j(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_j(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_j,args,1,1);
    }
    return reloq_fifi_fin_ph_j(state, p0);
}
int32_t reloq_fifi_fin_ph_j_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_j_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_j_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_j_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_k(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_k,args,1,1);
    }
    return reloq_fifi_fin_ph_k(state, p0);
}
int32_t reloq_fifi_fin_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_k_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_k_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_l(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_l,args,1,1);
    }
    return reloq_fifi_fin_ph_l(state, p0);
}
int32_t reloq_fifi_fin_ph_l_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_l_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_l_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_l_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_m(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_m,args,1,1);
    }
    return reloq_fifi_fin_ph_m(state, p0);
}
int32_t reloq_fifi_fin_ph_m_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_m_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_m_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_m_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_n(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_n,args,1,1);
    }
    return reloq_fifi_fin_ph_n(state, p0);
}
int32_t reloq_fifi_fin_ph_n_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_n_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_n_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_p(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_p,args,1,1);
    }
    return reloq_fifi_fin_ph_p(state, p0);
}
int32_t reloq_fifi_fin_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_p_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_p_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_r(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_r,args,1,1);
    }
    return reloq_fifi_fin_ph_r(state, p0);
}
int32_t reloq_fifi_fin_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_r_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_r_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_s(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_s,args,1,1);
    }
    return reloq_fifi_fin_ph_s(state, p0);
}
int32_t reloq_fifi_fin_ph_s_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_s_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_s_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_s_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_t(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_t,args,1,1);
    }
    return reloq_fifi_fin_ph_t(state, p0);
}
int32_t reloq_fifi_fin_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_t_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_t_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_v(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_v(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_v,args,1,1);
    }
    return reloq_fifi_fin_ph_v(state, p0);
}
int32_t reloq_fifi_fin_ph_v_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_v_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_v_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_v_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_w(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_w,args,1,1);
    }
    return reloq_fifi_fin_ph_w(state, p0);
}
int32_t reloq_fifi_fin_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_w_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_w_dur(state, p0);
}
int32_t reloq_fifi_fin_ph_z(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_z,args,1,1);
    }
    return reloq_fifi_fin_ph_z(state, p0);
}
int32_t reloq_fifi_fin_ph_z_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_ph_z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_ph_z_dur,args,1,1);
    }
    return reloq_fifi_fin_ph_z_dur(state, p0);
}
int32_t reloq_fifi_fin_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_phr_initial_F0,args,1,1);
    }
    return reloq_fifi_fin_phr_initial_F0(state, p0);
}
int32_t reloq_fifi_fin_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_phrase_tone,args,1,1);
    }
    return reloq_fifi_fin_phrase_tone(state, p0);
}
int32_t reloq_fifi_fin_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_position_t_ptrs,args,1,1);
    }
    return reloq_fifi_fin_position_t_ptrs(state, p0);
}
int32_t reloq_fifi_fin_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_post_comma_phrase,args,3,3);
    }
    return reloq_fifi_fin_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_postnucl_f0,args,1,1);
    }
    return reloq_fifi_fin_postnucl_f0(state, p0);
}
int32_t reloq_fifi_fin_postnucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_postnucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_postnucl_high_tone,args,1,1);
    }
    return reloq_fifi_fin_postnucl_high_tone(state, p0);
}
int32_t reloq_fifi_fin_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_prenucl_f0,args,1,1);
    }
    return reloq_fifi_fin_prenucl_f0(state, p0);
}
int32_t reloq_fifi_fin_roots(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_roots(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_roots,args,4,4);
    }
    return reloq_fifi_fin_roots(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_fin_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_said_phrase,args,4,4);
    }
    return reloq_fifi_fin_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_fin_single_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_single_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_single_digits,args,3,3);
    }
    return reloq_fifi_fin_single_digits(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_symbolic_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_symbolic_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_symbolic_rules,args,3,3);
    }
    return reloq_fifi_fin_symbolic_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_symbols(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_symbols(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_symbols,args,3,3);
    }
    return reloq_fifi_fin_symbols(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_three_ph_r(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_three_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_three_ph_r,args,1,1);
    }
    return reloq_fifi_fin_three_ph_r(state, p0);
}
int32_t reloq_fifi_fin_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_fin_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fin_tok_dict,args,3,3);
    }
    return reloq_fifi_fin_tok_dict(state, p0, p1, p2);
}
int32_t reloq_fifi_fin_trans_features(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_trans_features,args,1,1);
    }
    return reloq_fifi_fin_trans_features(state, p0);
}
int32_t reloq_fifi_fin_trill_Fv(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_trill_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_trill_Fv,args,1,1);
    }
    return reloq_fifi_fin_trill_Fv(state, p0);
}
int32_t reloq_fifi_fin_two_ph_r(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_two_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_two_ph_r,args,1,1);
    }
    return reloq_fifi_fin_two_ph_r(state, p0);
}
int32_t reloq_fifi_fin_velar_Fv(void *, int32_t);
static inline int32_t reloq_call_fifi_fin_velar_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fin_velar_Fv,args,1,1);
    }
    return reloq_fifi_fin_velar_Fv(state, p0);
}
int32_t reloq_fifi_final_voiced_release(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_final_voiced_release(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_voiced_release,args,3,3);
    }
    return reloq_fifi_final_voiced_release(state, p0, p1, p2);
}
int32_t reloq_fifi_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_fifi_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_fifi_find_acoustic_nuc(state, p0);
}
int32_t reloq_fifi_find_cardinal_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_cardinal_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_cardinal_number,args,3,3);
    }
    return reloq_fifi_find_cardinal_number(state, p0, p1, p2);
}
int32_t reloq_fifi_find_compound(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_compound(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_compound,args,4,4);
    }
    return reloq_fifi_find_compound(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_end_of_num_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_end_of_num_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_end_of_num_sequence,args,3,3);
    }
    return reloq_fifi_find_end_of_num_sequence(state, p0, p1, p2);
}
int32_t reloq_fifi_find_fin_nucleus(void *, int32_t);
static inline int32_t reloq_call_fifi_find_fin_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_fin_nucleus,args,1,1);
    }
    return reloq_fifi_find_fin_nucleus(state, p0);
}
int32_t reloq_fifi_find_fin_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_fin_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_fin_obligatory_phrase,args,4,4);
    }
    return reloq_fifi_find_fin_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_fin_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_fin_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_fin_potential_phrase,args,4,4);
    }
    return reloq_fifi_find_fin_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_lang_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_lang_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_obligatory_phrase,args,4,4);
    }
    return reloq_fifi_find_lang_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_lang_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_lang_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_potential_phrase,args,4,4);
    }
    return reloq_fifi_find_lang_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_fifi_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_most_stressed_syll(void *, int32_t);
static inline int32_t reloq_call_fifi_find_most_stressed_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_most_stressed_syll,args,1,1);
    }
    return reloq_fifi_find_most_stressed_syll(state, p0);
}
int32_t reloq_fifi_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_next_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_word,args,4,4);
    }
    return reloq_fifi_find_next_word(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_fifi_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_fifi_find_nucleus(state, p0);
}
int32_t reloq_fifi_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_fifi_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_potential_case_ending(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_potential_case_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_potential_case_ending,args,4,4);
    }
    return reloq_fifi_find_potential_case_ending(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_previous_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_previous_word,args,4,4);
    }
    return reloq_fifi_find_previous_word(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_find_root_boundary(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_root_boundary(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_root_boundary,args,3,3);
    }
    return reloq_fifi_find_root_boundary(state, p0, p1, p2);
}
int32_t reloq_fifi_find_unpuncted_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_unpuncted_phrases,args,3,3);
    }
    return reloq_fifi_find_unpuncted_phrases(state, p0, p1, p2);
}
int32_t reloq_fifi_find_verb_inflection(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_find_verb_inflection(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_verb_inflection,args,3,3);
    }
    return reloq_fifi_find_verb_inflection(state, p0, p1, p2);
}
int32_t reloq_fifi_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_fifi_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_first_postnucl_accent,args,1,1);
    }
    return reloq_fifi_first_postnucl_accent(state, p0);
}
int32_t reloq_fifi_first_root_vow(void *, int32_t, int32_t);
int32_t reloq_fifi_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_followed_by_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_followed_by_hyphen,args,3,3);
    }
    return reloq_fifi_followed_by_hyphen(state, p0, p1, p2);
}
int32_t reloq_fifi_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_fifi_forced_synthesis(state, p0, p1);
}
int32_t reloq_fifi_general_consonant_reduction(void *, int32_t);
static inline int32_t reloq_call_fifi_general_consonant_reduction(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_general_consonant_reduction,args,1,1);
    }
    return reloq_fifi_general_consonant_reduction(state, p0);
}
int32_t reloq_fifi_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_fifi_generate_acoustic_vals(state, p0);
}
int32_t reloq_fifi_generate_diaphones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_generate_diaphones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_generate_diaphones,args,3,3);
    }
    return reloq_fifi_generate_diaphones(state, p0, p1, p2);
}
int32_t reloq_fifi_generate_durations(void *, int32_t);
static inline int32_t reloq_call_fifi_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_fifi_generate_durations(state, p0);
}
int32_t reloq_fifi_generate_intonation(void *, int32_t);
static inline int32_t reloq_call_fifi_generate_intonation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_intonation,args,1,1);
    }
    return reloq_fifi_generate_intonation(state, p0);
}
int32_t reloq_fifi_get_abbr_case_and_num(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_abbr_case_and_num(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_abbr_case_and_num,args,4,4);
    }
    return reloq_fifi_get_abbr_case_and_num(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_get_case(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_case(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_case,args,4,4);
    }
    return reloq_fifi_get_case(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_get_case_and_num(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_case_and_num(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_case_and_num,args,5,5);
    }
    return reloq_fifi_get_case_and_num(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_get_case_from_word_end(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_case_from_word_end(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_case_from_word_end,args,4,4);
    }
    return reloq_fifi_get_case_from_word_end(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_get_case_of_hyphenated_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_case_of_hyphenated_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_case_of_hyphenated_num,args,3,3);
    }
    return reloq_fifi_get_case_of_hyphenated_num(state, p0, p1, p2);
}
int32_t reloq_fifi_get_case_of_hyphenated_ordinal(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_case_of_hyphenated_ordinal(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_case_of_hyphenated_ordinal,args,2,2);
    }
    return reloq_fifi_get_case_of_hyphenated_ordinal(state, p0, p1);
}
int32_t reloq_fifi_get_fin_fric_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_fin_fric_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fin_fric_trans_dur,args,2,2);
    }
    return reloq_fifi_get_fin_fric_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_get_fin_nas_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_fin_nas_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fin_nas_trans_dur,args,2,2);
    }
    return reloq_fifi_get_fin_nas_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_get_fin_son_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_fin_son_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fin_son_trans_dur,args,2,2);
    }
    return reloq_fifi_get_fin_son_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_get_fin_stop_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_fin_stop_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fin_stop_trans_dur,args,2,2);
    }
    return reloq_fifi_get_fin_stop_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_get_fin_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_fin_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fin_trans_dur,args,2,2);
    }
    return reloq_fifi_get_fin_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_get_fin_trill_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_fin_trill_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fin_trill_trans_dur,args,2,2);
    }
    return reloq_fifi_get_fin_trill_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_get_fin_vow_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_fin_vow_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fin_vow_trans_dur,args,2,2);
    }
    return reloq_fifi_get_fin_vow_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_fifi_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_fifi_get_h_dur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_h_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_h_dur,args,4,4);
    }
    return reloq_fifi_get_h_dur(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_fifi_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_get_input(void *, int32_t);
static inline int32_t reloq_call_fifi_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_fifi_get_input(state, p0);
}
int32_t reloq_fifi_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_fifi_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_fifi_get_nuc_AV(state, p0);
}
int32_t reloq_fifi_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_fifi_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_get_num_case_and_grade(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_num_case_and_grade(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_num_case_and_grade,args,3,3);
    }
    return reloq_fifi_get_num_case_and_grade(state, p0, p1, p2);
}
int32_t reloq_fifi_get_num_case_from_next_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_num_case_from_next_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_num_case_from_next_word,args,5,5);
    }
    return reloq_fifi_get_num_case_from_next_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_get_number_info(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_number_info(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_info,args,3,3);
    }
    return reloq_fifi_get_number_info(state, p0, p1, p2);
}
int32_t reloq_fifi_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_fifi_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_get_stanfin_non_vowels(void *, int32_t);
static inline int32_t reloq_call_fifi_get_stanfin_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanfin_non_vowels,args,1,1);
    }
    return reloq_fifi_get_stanfin_non_vowels(state, p0);
}
int32_t reloq_fifi_get_stanfin_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_fifi_get_stanfin_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanfin_nuc_AV,args,1,1);
    }
    return reloq_fifi_get_stanfin_nuc_AV(state, p0);
}
int32_t reloq_fifi_get_tok(void *, int32_t);
static inline int32_t reloq_call_fifi_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_fifi_get_tok(state, p0);
}
int32_t reloq_fifi_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_fifi_get_trans_dur(state, p0, p1);
}
int32_t reloq_fifi_gettok(void *, int32_t);
static inline int32_t reloq_call_fifi_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_fifi_gettok(state, p0);
}
int32_t reloq_fifi_glottalize_vow(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_glottalize_vow(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_glottalize_vow,args,2,2);
    }
    return reloq_fifi_glottalize_vow(state, p0, p1);
}
int32_t reloq_fifi_gramm_cat_dict_lookup(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_gramm_cat_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_gramm_cat_dict_lookup,args,5,5);
    }
    return reloq_fifi_gramm_cat_dict_lookup(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_h_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_h_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_h_rules,args,3,3);
    }
    return reloq_fifi_h_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_fifi_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_fifi_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_fifi_handle_han(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_han(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_han,args,5,5);
    }
    return reloq_fifi_handle_han(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_fifi_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_fifi_handle_kaan(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_kaan(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_kaan,args,5,5);
    }
    return reloq_fifi_handle_kaan(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_handle_kin(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_kin(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_kin,args,5,5);
    }
    return reloq_fifi_handle_kin(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_handle_ko(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_ko(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_ko,args,5,5);
    }
    return reloq_fifi_handle_ko(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_handle_non_phrasal_commas(void *, int32_t);
static inline int32_t reloq_call_fifi_handle_non_phrasal_commas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_phrasal_commas,args,1,1);
    }
    return reloq_fifi_handle_non_phrasal_commas(state, p0);
}
int32_t reloq_fifi_handle_pa(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_pa(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pa,args,5,5);
    }
    return reloq_fifi_handle_pa(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_handle_pas(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_pas(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pas,args,5,5);
    }
    return reloq_fifi_handle_pas(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_fifi_handle_pause(state, p0, p1);
}
int32_t reloq_fifi_handle_sti(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_handle_sti(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_sti,args,5,5);
    }
    return reloq_fifi_handle_sti(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_has_case_ending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_has_case_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_has_case_ending,args,5,5);
    }
    return reloq_fifi_has_case_ending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_has_num_case_ending(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_has_num_case_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_has_num_case_ending,args,4,4);
    }
    return reloq_fifi_has_num_case_ending(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_low_star_tone,args,1,1);
    }
    return reloq_fifi_high_low_star_tone(state, p0);
}
int32_t reloq_fifi_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_star_low_tone,args,1,1);
    }
    return reloq_fifi_high_star_low_tone(state, p0);
}
int32_t reloq_fifi_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_tone,args,1,1);
    }
    return reloq_fifi_high_tone(state, p0);
}
int32_t reloq_fifi_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen,args,3,3);
    }
    return reloq_fifi_hyphen(state, p0, p1, p2);
}
int32_t reloq_fifi_hyphen_between_nums(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_hyphenated_compound(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_hyphenated_compound(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphenated_compound,args,2,2);
    }
    return reloq_fifi_hyphenated_compound(state, p0, p1);
}
int32_t reloq_fifi_i_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_i_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_i_rules,args,3,3);
    }
    return reloq_fifi_i_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_identify_fin_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_identify_fin_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_identify_fin_ordinal,args,3,3);
    }
    return reloq_fifi_identify_fin_ordinal(state, p0, p1, p2);
}
int32_t reloq_fifi_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_fifi_in_brackets(state, p0, p1, p2);
}
int32_t reloq_fifi_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_fifi_in_quotes(state, p0, p1, p2);
}
int32_t reloq_fifi_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_in_syllable(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_syllable,args,4,4);
    }
    return reloq_fifi_in_syllable(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_fifi_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_fifi_increment_output_file_number(state, p0);
}
int32_t reloq_fifi_init_adjacent_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_fifi_init_adjacent_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_adjacent_word_ptrs,args,1,1);
    }
    return reloq_fifi_init_adjacent_word_ptrs(state, p0);
}
int32_t reloq_fifi_init_case_and_grade(void *, int32_t);
static inline int32_t reloq_call_fifi_init_case_and_grade(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_case_and_grade,args,1,1);
    }
    return reloq_fifi_init_case_and_grade(state, p0);
}
int32_t reloq_fifi_init_current_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_fifi_init_current_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_current_word_ptrs,args,1,1);
    }
    return reloq_fifi_init_current_word_ptrs(state, p0);
}
int32_t reloq_fifi_init_delta(void *, int32_t);
static inline int32_t reloq_call_fifi_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_fifi_init_delta(state, p0);
}
int32_t reloq_fifi_init_fin_textnorm_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_init_fin_textnorm_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_fin_textnorm_vars,args,2,2);
    }
    return reloq_fifi_init_fin_textnorm_vars(state, p0, p1);
}
int32_t reloq_fifi_init_fin_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_fin_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_fin_vars,args,1,1);
    }
    return reloq_fifi_init_fin_vars(state, p0);
}
int32_t reloq_fifi_init_fin_word_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_fin_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_fin_word_vars,args,1,1);
    }
    return reloq_fifi_init_fin_word_vars(state, p0);
}
int32_t reloq_fifi_init_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_gcat_vars,args,1,1);
    }
    return reloq_fifi_init_gcat_vars(state, p0);
}
int32_t reloq_fifi_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_fifi_init_lang_vars(state, p0);
}
int32_t reloq_fifi_init_morph_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_morph_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_morph_vars,args,1,1);
    }
    return reloq_fifi_init_morph_vars(state, p0);
}
int32_t reloq_fifi_init_nuclwrd_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_nuclwrd_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_nuclwrd_vars,args,1,1);
    }
    return reloq_fifi_init_nuclwrd_vars(state, p0);
}
int32_t reloq_fifi_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_fifi_init_phr_feature_vars(state, p0);
}
int32_t reloq_fifi_init_phrase_inton_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_phrase_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_inton_vars,args,1,1);
    }
    return reloq_fifi_init_phrase_inton_vars(state, p0);
}
int32_t reloq_fifi_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_fifi_init_phrase_vars(state, p0);
}
int32_t reloq_fifi_init_platform(void *, int32_t);
static inline int32_t reloq_call_fifi_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_fifi_init_platform(state, p0);
}
int32_t reloq_fifi_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_fifi_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_fifi_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_fifi_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_fifi_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_inton_vars,args,1,1);
    }
    return reloq_fifi_init_syll_inton_vars(state, p0);
}
int32_t reloq_fifi_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_fifi_init_syll_vars(state, p0);
}
int32_t reloq_fifi_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_fifi_init_symbolic_vars(state, p0);
}
int32_t reloq_fifi_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_fifi_init_tm_vars(state, p0);
}
int32_t reloq_fifi_init_two_ph_r(void *, int32_t);
static inline int32_t reloq_call_fifi_init_two_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_two_ph_r,args,1,1);
    }
    return reloq_fifi_init_two_ph_r(state, p0);
}
int32_t reloq_fifi_init_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_fifi_init_vars(state, p0);
}
int32_t reloq_fifi_init_voice_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_voice_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_voice_vars,args,1,1);
    }
    return reloq_fifi_init_voice_vars(state, p0);
}
int32_t reloq_fifi_init_word(void *, int32_t);
static inline int32_t reloq_call_fifi_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_fifi_init_word(state, p0);
}
int32_t reloq_fifi_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_inton_vars,args,1,1);
    }
    return reloq_fifi_init_word_inton_vars(state, p0);
}
int32_t reloq_fifi_init_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_fifi_init_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_ptrs,args,1,1);
    }
    return reloq_fifi_init_word_ptrs(state, p0);
}
int32_t reloq_fifi_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_fifi_init_word_vars(state, p0);
}
int32_t reloq_fifi_initial_compound_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_initial_compound_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_initial_compound_word,args,3,3);
    }
    return reloq_fifi_initial_compound_word(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_fifi_insert_acoustic_vals(state, p0);
}
int32_t reloq_fifi_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_fifi_insert_burst_dur(state, p0);
}
int32_t reloq_fifi_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_fifi_insert_char_count(state, p0);
}
int32_t reloq_fifi_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_comma_phrase,args,3,3);
    }
    return reloq_fifi_insert_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_compound(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_compound(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_compound,args,3,3);
    }
    return reloq_fifi_insert_compound(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_fifi_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_insert_f0(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_f0(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_f0,args,4,4);
    }
    return reloq_fifi_insert_f0(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_insert_fin_large_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_fin_large_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_fin_large_number,args,3,3);
    }
    return reloq_fifi_insert_fin_large_number(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_fin_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_fin_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_fin_spr_phone,args,3,3);
    }
    return reloq_fifi_insert_fin_spr_phone(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_insert_gem_con(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_gem_con(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_gem_con,args,4,4);
    }
    return reloq_fifi_insert_gem_con(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_insert_harmonic_vow(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_harmonic_vow(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_harmonic_vow,args,3,3);
    }
    return reloq_fifi_insert_harmonic_vow(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_insert_lang_large_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_lang_large_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_lang_large_number,args,3,3);
    }
    return reloq_fifi_insert_lang_large_number(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_fifi_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_insert_number_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_number_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_number_word,args,4,4);
    }
    return reloq_fifi_insert_number_word(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_fifi_insert_pause_durs(state, p0);
}
int32_t reloq_fifi_insert_penult_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_fifi_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_final_tones,args,1,1);
    }
    return reloq_fifi_insert_phrase_final_tones(state, p0);
}
int32_t reloq_fifi_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_fifi_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_preset_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_preset_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_preset_phrase_tone,args,1,1);
    }
    return reloq_fifi_insert_preset_phrase_tone(state, p0);
}
int32_t reloq_fifi_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_insert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_single_digit,args,3,3);
    }
    return reloq_fifi_insert_single_digit(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_fifi_insert_space(state, p0, p1);
}
int32_t reloq_fifi_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_fifi_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_fifi_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_insert_suffix(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_suffix(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_suffix,args,3,3);
    }
    return reloq_fifi_insert_suffix(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_fifi_insert_syll_level_vals(state, p0);
}
int32_t reloq_fifi_insert_target_t3(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_target_t3(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_target_t3,args,1,1);
    }
    return reloq_fifi_insert_target_t3(state, p0);
}
int32_t reloq_fifi_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_fifi_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_fifi_insert_transitions(state, p0);
}
int32_t reloq_fifi_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_fifi_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrases,args,4,4);
    }
    return reloq_fifi_insert_unpuncted_phrases(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_fifi_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_fifi_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_fifi_insert_word(state, p0, p1, p2);
}
int32_t reloq_fifi_insert_zero_sylls(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_insert_zero_sylls(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_zero_sylls,args,3,3);
    }
    return reloq_fifi_insert_zero_sylls(state, p0, p1, p2);
}
int32_t reloq_fifi_inside_orig_inp_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_inside_orig_inp_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_inside_orig_inp_token,args,2,2);
    }
    return reloq_fifi_inside_orig_inp_token(state, p0, p1);
}
int32_t reloq_fifi_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_fifi_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_fifi_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_interpret_single_char_modes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_interpret_single_char_modes,args,3,3);
    }
    return reloq_fifi_interpret_single_char_modes(state, p0, p1, p2);
}
int32_t reloq_fifi_intervocalic_l(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_intervocalic_l(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_intervocalic_l,args,3,3);
    }
    return reloq_fifi_intervocalic_l(state, p0, p1, p2);
}
int32_t reloq_fifi_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_fifi_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_fifi_is_accentable_syllable(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_accentable_syllable(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_accentable_syllable,args,2,2);
    }
    return reloq_fifi_is_accentable_syllable(state, p0, p1);
}
int32_t reloq_fifi_is_adposition(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_adposition(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_adposition,args,4,4);
    }
    return reloq_fifi_is_adposition(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_adverb(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_adverb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_adverb,args,4,4);
    }
    return reloq_fifi_is_adverb(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_after_h(void *, int32_t, int32_t);
int32_t reloq_fifi_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_fifi_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_fifi_is_ambig_suffix(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_ambig_suffix(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ambig_suffix,args,4,4);
    }
    return reloq_fifi_is_ambig_suffix(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_antaa_verb(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_antaa_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_antaa_verb,args,5,5);
    }
    return reloq_fifi_is_antaa_verb(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_is_back_vow(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_back_vow(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_back_vow,args,2,2);
    }
    return reloq_fifi_is_back_vow(state, p0, p1);
}
int32_t reloq_fifi_is_case_ending(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_case_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_case_ending,args,4,4);
    }
    return reloq_fifi_is_case_ending(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_comparative_nounadj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_comparative_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_comparative_nounadj,args,3,3);
    }
    return reloq_fifi_is_comparative_nounadj(state, p0, p1, p2);
}
int32_t reloq_fifi_is_early_focus(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_early_focus(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_early_focus,args,3,3);
    }
    return reloq_fifi_is_early_focus(state, p0, p1, p2);
}
int32_t reloq_fifi_is_fin_date(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_fin_date(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_fin_date,args,8,8);
    }
    return reloq_fifi_is_fin_date(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_fifi_is_fin_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_fin_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_fin_fraction,args,3,3);
    }
    return reloq_fifi_is_fin_fraction(state, p0, p1, p2);
}
int32_t reloq_fifi_is_front_vow(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_front_vow(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_front_vow,args,2,2);
    }
    return reloq_fifi_is_front_vow(state, p0, p1);
}
int32_t reloq_fifi_is_geminate_env(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_geminate_env(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_geminate_env,args,4,4);
    }
    return reloq_fifi_is_geminate_env(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_huomata_verb(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_huomata_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_huomata_verb,args,4,4);
    }
    return reloq_fifi_is_huomata_verb(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_hyphenated_word(void *, int32_t, int32_t);
int32_t reloq_fifi_is_in_parentheses(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_is_initial_construct_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_initial_construct_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_initial_construct_word,args,3,3);
    }
    return reloq_fifi_is_initial_construct_word(state, p0, p1, p2);
}
int32_t reloq_fifi_is_johtua_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_johtua_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_johtua_verb,args,3,3);
    }
    return reloq_fifi_is_johtua_verb(state, p0, p1, p2);
}
int32_t reloq_fifi_is_laajeta_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_laajeta_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_laajeta_verb,args,3,3);
    }
    return reloq_fifi_is_laajeta_verb(state, p0, p1, p2);
}
int32_t reloq_fifi_is_left_pseudo_diphthong(void *, int32_t);
static inline int32_t reloq_call_fifi_is_left_pseudo_diphthong(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_left_pseudo_diphthong,args,1,1);
    }
    return reloq_fifi_is_left_pseudo_diphthong(state, p0);
}
int32_t reloq_fifi_is_noun_suffix_sequence(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_noun_suffix_sequence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_noun_suffix_sequence,args,4,4);
    }
    return reloq_fifi_is_noun_suffix_sequence(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_nounadj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_nounadj,args,3,3);
    }
    return reloq_fifi_is_nounadj(state, p0, p1, p2);
}
int32_t reloq_fifi_is_nousta_verb(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_nousta_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_nousta_verb,args,4,4);
    }
    return reloq_fifi_is_nousta_verb(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_number,args,4,4);
    }
    return reloq_fifi_is_number(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_pathname(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_pathname(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pathname,args,2,2);
    }
    return reloq_fifi_is_pathname(state, p0, p1);
}
int32_t reloq_fifi_is_possible_compound(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_possible_compound(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_possible_compound,args,4,4);
    }
    return reloq_fifi_is_possible_compound(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_proper_name(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_proper_name(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_proper_name,args,2,2);
    }
    return reloq_fifi_is_proper_name(state, p0, p1);
}
int32_t reloq_fifi_is_puhua_verb(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_puhua_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_puhua_verb,args,5,5);
    }
    return reloq_fifi_is_puhua_verb(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_is_right_pseudo_diphthong(void *, int32_t);
static inline int32_t reloq_call_fifi_is_right_pseudo_diphthong(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_right_pseudo_diphthong,args,1,1);
    }
    return reloq_fifi_is_right_pseudo_diphthong(state, p0);
}
int32_t reloq_fifi_is_root_with_disharmonic_vows(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_root_with_disharmonic_vows(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_root_with_disharmonic_vows,args,3,3);
    }
    return reloq_fifi_is_root_with_disharmonic_vows(state, p0, p1, p2);
}
int32_t reloq_fifi_is_saada_verb(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_saada_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_saada_verb,args,4,4);
    }
    return reloq_fifi_is_saada_verb(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_fifi_is_spr(state, p0, p1, p2);
}
int32_t reloq_fifi_is_suffix_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_suffix_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_suffix_sequence,args,3,3);
    }
    return reloq_fifi_is_suffix_sequence(state, p0, p1, p2);
}
int32_t reloq_fifi_is_target_syll(void *, int32_t);
static inline int32_t reloq_call_fifi_is_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_target_syll,args,1,1);
    }
    return reloq_fifi_is_target_syll(state, p0);
}
int32_t reloq_fifi_is_tarvita_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_tarvita_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_tarvita_verb,args,3,3);
    }
    return reloq_fifi_is_tarvita_verb(state, p0, p1, p2);
}
int32_t reloq_fifi_is_verb(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_verb,args,4,4);
    }
    return reloq_fifi_is_verb(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_is_word_boundary(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_word_boundary(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_word_boundary,args,3,3);
    }
    return reloq_fifi_is_word_boundary(state, p0, p1, p2);
}
int32_t reloq_fifi_is_year(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_is_year(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_year,args,3,3);
    }
    return reloq_fifi_is_year(state, p0, p1, p2);
}
int32_t reloq_fifi_isitAlt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_isitAlt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitAlt,args,3,3);
    }
    return reloq_fifi_isitAlt(state, p0, p1, p2);
}
int32_t reloq_fifi_isitWH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_isitWH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitWH,args,3,3);
    }
    return reloq_fifi_isitWH(state, p0, p1, p2);
}
int32_t reloq_fifi_lang_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_lang_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_hyphen_between_nums,args,5,5);
    }
    return reloq_fifi_lang_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_lang_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_lang_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_non_phrasal_comma,args,2,2);
    }
    return reloq_fifi_lang_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_fifi_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_lang_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_phrase_level_rules,args,3,3);
    }
    return reloq_fifi_lang_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_fifi_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_fifi_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_fifi_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_fifi_legal_root_init_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_legal_root_init_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_root_init_cons,args,2,2);
    }
    return reloq_fifi_legal_root_init_cons(state, p0, p1);
}
int32_t reloq_fifi_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_fifi_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_fifi_longdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_longdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_longdash,args,3,3);
    }
    return reloq_fifi_longdash(state, p0, p1, p2);
}
int32_t reloq_fifi_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_high_star_tone,args,1,1);
    }
    return reloq_fifi_low_high_star_tone(state, p0);
}
int32_t reloq_fifi_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_star_high_tone,args,1,1);
    }
    return reloq_fifi_low_star_high_tone(state, p0);
}
int32_t reloq_fifi_low_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_tone,args,1,1);
    }
    return reloq_fifi_low_tone(state, p0);
}
int32_t reloq_fifi_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_mark_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_content_word,args,5,5);
    }
    return reloq_fifi_mark_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_mark_fin_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_fin_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fin_content_word,args,4,4);
    }
    return reloq_fifi_mark_fin_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_mark_function_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_function_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_function_word,args,5,5);
    }
    return reloq_fifi_mark_function_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_mark_h(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_h(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_h,args,3,3);
    }
    return reloq_fifi_mark_h(state, p0, p1, p2);
}
int32_t reloq_fifi_mark_non_letter(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_fifi_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_mark_potential_phrase_breaks(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_potential_phrase_breaks(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_potential_phrase_breaks,args,4,4);
    }
    return reloq_fifi_mark_potential_phrase_breaks(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_spr_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_spr_syll_vals,args,6,6);
    }
    return reloq_fifi_mark_spr_syll_vals(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_mark_subcategory(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_mark_syll_stress(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_mark_uppercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_uppercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_uppercase,args,3,3);
    }
    return reloq_fifi_mark_uppercase(state, p0, p1, p2);
}
int32_t reloq_fifi_mark_word_categories(void *, int32_t);
static inline int32_t reloq_call_fifi_mark_word_categories(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_categories,args,1,1);
    }
    return reloq_fifi_mark_word_categories(state, p0);
}
int32_t reloq_fifi_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_word_str_annot(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_str_annot,args,3,3);
    }
    return reloq_fifi_mark_word_str_annot(state, p0, p1, p2);
}
int32_t reloq_fifi_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_fifi_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_fifi_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_fifi_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_fifi_merge_case_ending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_merge_case_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_case_ending,args,5,5);
    }
    return reloq_fifi_merge_case_ending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_fifi_misc_chars(state, p0, p1, p2);
}
int32_t reloq_fifi_modify_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_modify_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modify_abbr,args,4,4);
    }
    return reloq_fifi_modify_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_modify_breathiness_for_femV(void *, int32_t);
static inline int32_t reloq_call_fifi_modify_breathiness_for_femV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modify_breathiness_for_femV,args,1,1);
    }
    return reloq_fifi_modify_breathiness_for_femV(state, p0);
}
int32_t reloq_fifi_modulate_burst(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_modulate_burst(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_burst,args,2,2);
    }
    return reloq_fifi_modulate_burst(state, p0, p1);
}
int32_t reloq_fifi_modulate_nas_AV(void *, int32_t);
static inline int32_t reloq_call_fifi_modulate_nas_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_nas_AV,args,1,1);
    }
    return reloq_fifi_modulate_nas_AV(state, p0);
}
int32_t reloq_fifi_modulate_noise(void *, int32_t);
static inline int32_t reloq_call_fifi_modulate_noise(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_noise,args,1,1);
    }
    return reloq_fifi_modulate_noise(state, p0);
}
int32_t reloq_fifi_modulate_pwind(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_modulate_pwind(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_pwind,args,2,2);
    }
    return reloq_fifi_modulate_pwind(state, p0, p1);
}
int32_t reloq_fifi_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_fifi_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_monetary_exp,args,3,3);
    }
    return reloq_fifi_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_fifi_n_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_n_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_n_rules,args,3,3);
    }
    return reloq_fifi_n_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_nasalize_vow(void *, int32_t);
int32_t reloq_fifi_nf_three_ph_r(void *, int32_t);
static inline int32_t reloq_call_fifi_nf_three_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nf_three_ph_r,args,1,1);
    }
    return reloq_fifi_nf_three_ph_r(state, p0);
}
int32_t reloq_fifi_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_fifi_no_readin(state, p0, p1);
}
int32_t reloq_fifi_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_fifi_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_fifi_no_words_in_delta(state, p0);
}
int32_t reloq_fifi_normalize_fin_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_normalize_fin_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_fin_letters,args,3,3);
    }
    return reloq_fifi_normalize_fin_letters(state, p0, p1, p2);
}
int32_t reloq_fifi_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_fifi_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_fifi_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_fifi_normalize_text(state, p0, p1, p2);
}
int32_t reloq_fifi_nucl_f0(void *, int32_t);
static inline int32_t reloq_call_fifi_nucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_f0,args,1,1);
    }
    return reloq_fifi_nucl_f0(state, p0);
}
int32_t reloq_fifi_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_fifi_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_fifi_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_fifi_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_fifi_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_tone,args,1,1);
    }
    return reloq_fifi_nucl_high_tone(state, p0);
}
int32_t reloq_fifi_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_fifi_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_fifi_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_fifi_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_fifi_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_tone,args,1,1);
    }
    return reloq_fifi_nucl_low_tone(state, p0);
}
int32_t reloq_fifi_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_fifi_nucleus_vals(state, p0);
}
int32_t reloq_fifi_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_fifi_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_fifi_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_p_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_p_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_p_rules,args,3,3);
    }
    return reloq_fifi_p_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_parenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_parenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_parenthesis,args,3,3);
    }
    return reloq_fifi_parenthesis(state, p0, p1, p2);
}
int32_t reloq_fifi_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_fifi_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_fifi_period_phrase(state, p0, p1, p2);
}
int32_t reloq_fifi_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_fifi_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_fifi_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phr_initial_F0,args,1,1);
    }
    return reloq_fifi_phr_initial_F0(state, p0);
}
int32_t reloq_fifi_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_fifi_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_phrase_final_adjust(void *, int32_t);
static inline int32_t reloq_call_fifi_phrase_final_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_final_adjust,args,1,1);
    }
    return reloq_fifi_phrase_final_adjust(state, p0);
}
int32_t reloq_fifi_phrase_init_adjust(void *, int32_t);
static inline int32_t reloq_call_fifi_phrase_init_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_init_adjust,args,1,1);
    }
    return reloq_fifi_phrase_init_adjust(state, p0);
}
int32_t reloq_fifi_phrase_level_printouts(void *, int32_t);
static inline int32_t reloq_call_fifi_phrase_level_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_printouts,args,1,1);
    }
    return reloq_fifi_phrase_level_printouts(state, p0);
}
int32_t reloq_fifi_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_fifi_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_tone,args,1,1);
    }
    return reloq_fifi_phrase_tone(state, p0);
}
int32_t reloq_fifi_point_af(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_point_af(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_point_af,args,2,2);
    }
    return reloq_fifi_point_af(state, p0, p1);
}
int32_t reloq_fifi_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_fifi_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_position_t_ptrs,args,1,1);
    }
    return reloq_fifi_position_t_ptrs(state, p0);
}
int32_t reloq_fifi_possible_root_to_left(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_possible_root_to_left(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_possible_root_to_left,args,2,2);
    }
    return reloq_fifi_possible_root_to_left(state, p0, p1);
}
int32_t reloq_fifi_possible_root_to_right(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_possible_root_to_right(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_possible_root_to_right,args,2,2);
    }
    return reloq_fifi_possible_root_to_right(state, p0, p1);
}
int32_t reloq_fifi_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_comma_phrase,args,3,3);
    }
    return reloq_fifi_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_fifi_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_fifi_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_postnucl_f0,args,1,1);
    }
    return reloq_fifi_postnucl_f0(state, p0);
}
int32_t reloq_fifi_postvoc_glide(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_postvoc_glide(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_postvoc_glide,args,2,2);
    }
    return reloq_fifi_postvoc_glide(state, p0, p1);
}
int32_t reloq_fifi_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_fifi_potential_sent_end(state, p0, p1);
}
int32_t reloq_fifi_precedes_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_precedes_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_end_quote,args,2,2);
    }
    return reloq_fifi_precedes_end_quote(state, p0, p1);
}
int32_t reloq_fifi_precedes_number(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_fifi_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_prenucl_f0,args,1,1);
    }
    return reloq_fifi_prenucl_f0(state, p0);
}
int32_t reloq_fifi_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_fifi_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_fifi_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_fifi_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_fifi_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_fifi_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_fifi_print_fin_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_print_fin_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_fin_SPR,args,3,3);
    }
    return reloq_fifi_print_fin_SPR(state, p0, p1, p2);
}
int32_t reloq_fifi_print_interword_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_print_interword_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_interword_chars,args,3,3);
    }
    return reloq_fifi_print_interword_chars(state, p0, p1, p2);
}
int32_t reloq_fifi_print_normalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_print_normalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_normalized_inp,args,3,3);
    }
    return reloq_fifi_print_normalized_inp(state, p0, p1, p2);
}
int32_t reloq_fifi_print_prompt(void *, int32_t);
static inline int32_t reloq_call_fifi_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_fifi_print_prompt(state, p0);
}
int32_t reloq_fifi_print_spr(void *, int32_t);
static inline int32_t reloq_call_fifi_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_fifi_print_spr(state, p0);
}
int32_t reloq_fifi_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_fifi_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_fifi_print_word_stress(state, p0, p1);
}
int32_t reloq_fifi_printouts(void *, int32_t);
static inline int32_t reloq_call_fifi_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_fifi_printouts(state, p0);
}
int32_t reloq_fifi_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_fifi_process_annotation(state, p0, p1, p2);
}
int32_t reloq_fifi_process_final_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_process_final_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_apostrophe,args,3,3);
    }
    return reloq_fifi_process_final_apostrophe(state, p0, p1, p2);
}
int32_t reloq_fifi_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_process_final_parenthetical(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_parenthetical,args,3,3);
    }
    return reloq_fifi_process_final_parenthetical(state, p0, p1, p2);
}
int32_t reloq_fifi_process_hostname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_process_hostname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_hostname,args,3,3);
    }
    return reloq_fifi_process_hostname(state, p0, p1, p2);
}
int32_t reloq_fifi_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_fifi_process_input(state, p0, p1);
}
int32_t reloq_fifi_process_pathname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_process_pathname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_pathname,args,3,3);
    }
    return reloq_fifi_process_pathname(state, p0, p1, p2);
}
int32_t reloq_fifi_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_fifi_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_fifi_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_fifi_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_fifi_punctuation(state, p0, p1, p2);
}
int32_t reloq_fifi_q_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_q_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_q_rules,args,3,3);
    }
    return reloq_fifi_q_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_fifi_question_phrase(state, p0, p1, p2);
}
int32_t reloq_fifi_quote_mark(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_quote_mark(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_quote_mark,args,3,3);
    }
    return reloq_fifi_quote_mark(state, p0, p1, p2);
}
int32_t reloq_fifi_r_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_r_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_r_rules,args,3,3);
    }
    return reloq_fifi_r_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_read_end(void *, int32_t);
static inline int32_t reloq_call_fifi_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_fifi_read_end(state, p0);
}
int32_t reloq_fifi_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_fifi_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_fifi_realize_prenucl_tone(void *, int32_t);
static inline int32_t reloq_call_fifi_realize_prenucl_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_realize_prenucl_tone,args,1,1);
    }
    return reloq_fifi_realize_prenucl_tone(state, p0);
}
int32_t reloq_fifi_reduce_long_vowel_coarticulation(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_reduce_long_vowel_coarticulation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reduce_long_vowel_coarticulation,args,5,5);
    }
    return reloq_fifi_reduce_long_vowel_coarticulation(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_fifi_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_fifi_register_SAPI_voices(state, p0);
}
int32_t reloq_fifi_register_fin_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_register_fin_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_fin_phones,args,3,3);
    }
    return reloq_fifi_register_fin_phones(state, p0, p1, p2);
}
int32_t reloq_fifi_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_fifi_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_reset_endptr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_endptr,args,4,4);
    }
    return reloq_fifi_reset_endptr(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_reset_enum_ptr(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_reset_enum_ptr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_enum_ptr,args,2,2);
    }
    return reloq_fifi_reset_enum_ptr(state, p0, p1);
}
int32_t reloq_fifi_reset_fin_sent_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_reset_fin_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_fin_sent_vars,args,1,1);
    }
    return reloq_fifi_reset_fin_sent_vars(state, p0);
}
int32_t reloq_fifi_reset_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_reset_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_gcat_vars,args,1,1);
    }
    return reloq_fifi_reset_gcat_vars(state, p0);
}
int32_t reloq_fifi_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_fifi_reset_lang_sent_vars(state, p0);
}
int32_t reloq_fifi_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_fifi_reset_phrase_vars(state, p0);
}
int32_t reloq_fifi_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_fifi_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_fifi_reset_sent_vars(state, p0);
}
int32_t reloq_fifi_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_fifi_reset_token_vars(state, p0, p1);
}
int32_t reloq_fifi_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_fifi_reset_word_vals(state, p0, p1);
}
int32_t reloq_fifi_root_disharmonic_vows(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_root_disharmonic_vows(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_root_disharmonic_vows,args,3,3);
    }
    return reloq_fifi_root_disharmonic_vows(state, p0, p1, p2);
}
int32_t reloq_fifi_roots_dict_entry(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_roots_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots_dict_entry,args,4,4);
    }
    return reloq_fifi_roots_dict_entry(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_roots_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_roots_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots_dict_lookup,args,3,3);
    }
    return reloq_fifi_roots_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_fifi_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_fifi_run_filters(state, p0, p1, p2);
}
int32_t reloq_fifi_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_fifi_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_fifi_run_speech_module(state, p0);
}
int32_t reloq_fifi_run_text_module(void *, int32_t);
static inline int32_t reloq_call_fifi_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_fifi_run_text_module(state, p0);
}
int32_t reloq_fifi_s_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_s_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_s_rules,args,3,3);
    }
    return reloq_fifi_s_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_said_phrase,args,4,4);
    }
    return reloq_fifi_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_fifi_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_fifi_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_fifi_scale_word_final_trans(state, p0);
}
int32_t reloq_fifi_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_fifi_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_fifi_semicolon_phrase(state, p0);
}
int32_t reloq_fifi_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_fifi_separate_tokens(state, p0, p1);
}
int32_t reloq_fifi_set_adjacent_word_ptrs(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_set_currency_word_case(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_currency_word_case(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_currency_word_case,args,5,5);
    }
    return reloq_fifi_set_currency_word_case(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_fifi_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_fifi_set_f0_range_ptrs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_f0_range_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_range_ptrs,args,5,5);
    }
    return reloq_fifi_set_f0_range_ptrs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_fifi_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_fifi_set_f0_wind_and_base(state, p0);
}
int32_t reloq_fifi_set_fin_global_constants(void *, int32_t);
static inline int32_t reloq_call_fifi_set_fin_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_fin_global_constants,args,1,1);
    }
    return reloq_fifi_set_fin_global_constants(state, p0);
}
int32_t reloq_fifi_set_first_postnuc_value(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_first_postnuc_value(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_first_postnuc_value,args,2,2);
    }
    return reloq_fifi_set_first_postnuc_value(state, p0, p1);
}
int32_t reloq_fifi_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_fifi_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_fifi_set_global_constants(state, p0);
}
int32_t reloq_fifi_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_fifi_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_fifi_set_klattparms(state, p0);
}
int32_t reloq_fifi_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_fifi_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_fifi_set_lang_global_constants(state, p0);
}
int32_t reloq_fifi_set_letter_name_wordcats(void *, int32_t);
static inline int32_t reloq_call_fifi_set_letter_name_wordcats(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_letter_name_wordcats,args,1,1);
    }
    return reloq_fifi_set_letter_name_wordcats(state, p0);
}
int32_t reloq_fifi_set_limits(void *, int32_t);
static inline int32_t reloq_call_fifi_set_limits(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_limits,args,1,1);
    }
    return reloq_fifi_set_limits(state, p0);
}
int32_t reloq_fifi_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_fifi_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_set_nuclhigh_levels(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_nuclhigh_levels(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_nuclhigh_levels,args,4,4);
    }
    return reloq_fifi_set_nuclhigh_levels(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_set_nuclhigh_pos(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_nuclhigh_pos(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_nuclhigh_pos,args,4,4);
    }
    return reloq_fifi_set_nuclhigh_pos(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_fifi_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_fifi_set_num_formants(state, p0);
}
int32_t reloq_fifi_set_num_root_grade(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_num_root_grade(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_num_root_grade,args,4,4);
    }
    return reloq_fifi_set_num_root_grade(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_fifi_set_phone_offset_vals(state, p0);
}
int32_t reloq_fifi_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_fifi_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_fifi_set_said_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_set_said_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_said_phr_vals,args,3,3);
    }
    return reloq_fifi_set_said_phr_vals(state, p0, p1, p2);
}
int32_t reloq_fifi_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_fifi_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_fifi_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_fifi_set_syll_offset_vals(state, p0);
}
int32_t reloq_fifi_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_fifi_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_fifi_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_fifi_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_fifi_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_fifi_set_voice_param_constants(state, p0);
}
int32_t reloq_fifi_set_window_size(void *, int32_t);
static inline int32_t reloq_call_fifi_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_fifi_set_window_size(state, p0);
}
int32_t reloq_fifi_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_fifi_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_fifi_setcdur(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_setcdur(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setcdur,args,3,3);
    }
    return reloq_fifi_setcdur(state, p0, p1, p2);
}
int32_t reloq_fifi_setdur(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_setdur(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setdur,args,3,3);
    }
    return reloq_fifi_setdur(state, p0, p1, p2);
}
int32_t reloq_fifi_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_fifi_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_fifi_shift_tone(void *, int32_t, int32_t);
int32_t reloq_fifi_shortdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_shortdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_shortdash,args,3,3);
    }
    return reloq_fifi_shortdash(state, p0, p1, p2);
}
int32_t reloq_fifi_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_fifi_single_chars(state, p0, p1, p2);
}
int32_t reloq_fifi_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_fifi_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_fifi_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_fifi_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_fifi_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_fifi_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_fifi_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_fifi_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_fifi_single_letter(state, p0, p1, p2);
}
int32_t reloq_fifi_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_fifi_size_filter(state, p0, p1, p2);
}
int32_t reloq_fifi_skip_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_skip_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_delimiters,args,2,2);
    }
    return reloq_fifi_skip_delimiters(state, p0, p1);
}
int32_t reloq_fifi_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_fifi_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_fifi_slash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_slash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash,args,3,3);
    }
    return reloq_fifi_slash(state, p0, p1, p2);
}
int32_t reloq_fifi_slash_before_measure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_slash_before_measure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash_before_measure,args,3,3);
    }
    return reloq_fifi_slash_before_measure(state, p0, p1, p2);
}
int32_t reloq_fifi_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_fifi_space(state, p0, p1, p2);
}
int32_t reloq_fifi_span_inverted_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_span_inverted_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_span_inverted_punct,args,3,3);
    }
    return reloq_fifi_span_inverted_punct(state, p0, p1, p2);
}
int32_t reloq_fifi_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_fifi_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_fifi_spell_out_large_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_spell_out_large_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_large_num,args,3,3);
    }
    return reloq_fifi_spell_out_large_num(state, p0, p1, p2);
}
int32_t reloq_fifi_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_fifi_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_fifi_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_fifi_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_split_compound(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_split_compound(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_split_compound,args,3,3);
    }
    return reloq_fifi_split_compound(state, p0, p1, p2);
}
int32_t reloq_fifi_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_fifi_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_stanfin_ph_A(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_A(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_A,args,1,1);
    }
    return reloq_fifi_stanfin_ph_A(state, p0);
}
int32_t reloq_fifi_stanfin_ph_I(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_I(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_I,args,1,1);
    }
    return reloq_fifi_stanfin_ph_I(state, p0);
}
int32_t reloq_fifi_stanfin_ph_O(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_O(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_O,args,1,1);
    }
    return reloq_fifi_stanfin_ph_O(state, p0);
}
int32_t reloq_fifi_stanfin_ph_U(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_U(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_U,args,1,1);
    }
    return reloq_fifi_stanfin_ph_U(state, p0);
}
int32_t reloq_fifi_stanfin_ph_Y(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_Y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_Y,args,1,1);
    }
    return reloq_fifi_stanfin_ph_Y(state, p0);
}
int32_t reloq_fifi_stanfin_ph_a(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_a,args,1,1);
    }
    return reloq_fifi_stanfin_ph_a(state, p0);
}
int32_t reloq_fifi_stanfin_ph_ao(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_ao(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_ao,args,1,1);
    }
    return reloq_fifi_stanfin_ph_ao(state, p0);
}
int32_t reloq_fifi_stanfin_ph_e(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_e,args,1,1);
    }
    return reloq_fifi_stanfin_ph_e(state, p0);
}
int32_t reloq_fifi_stanfin_ph_i(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_i,args,1,1);
    }
    return reloq_fifi_stanfin_ph_i(state, p0);
}
int32_t reloq_fifi_stanfin_ph_o(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_o,args,1,1);
    }
    return reloq_fifi_stanfin_ph_o(state, p0);
}
int32_t reloq_fifi_stanfin_ph_u(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_u,args,1,1);
    }
    return reloq_fifi_stanfin_ph_u(state, p0);
}
int32_t reloq_fifi_stanfin_ph_y(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_ph_y,args,1,1);
    }
    return reloq_fifi_stanfin_ph_y(state, p0);
}
int32_t reloq_fifi_stanfin_vals(void *, int32_t);
static inline int32_t reloq_call_fifi_stanfin_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfin_vals,args,1,1);
    }
    return reloq_fifi_stanfin_vals(state, p0);
}
int32_t reloq_fifi_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_fifi_stretch_focus(state, p0, p1);
}
int32_t reloq_fifi_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_fifi_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_fifi_strip_clitics(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_strip_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_clitics,args,5,5);
    }
    return reloq_fifi_strip_clitics(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_strip_fin_clitics(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_strip_fin_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_fin_clitics,args,4,4);
    }
    return reloq_fifi_strip_fin_clitics(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_strip_last_digit(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_syll_phone_adjust(void *, int32_t);
static inline int32_t reloq_call_fifi_syll_phone_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_syll_phone_adjust,args,1,1);
    }
    return reloq_fifi_syll_phone_adjust(state, p0);
}
int32_t reloq_fifi_syll_stress_adjust(void *, int32_t);
static inline int32_t reloq_call_fifi_syll_stress_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_syll_stress_adjust,args,1,1);
    }
    return reloq_fifi_syll_stress_adjust(state, p0);
}
int32_t reloq_fifi_syll_struct_adjust(void *, int32_t);
static inline int32_t reloq_call_fifi_syll_struct_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_syll_struct_adjust,args,1,1);
    }
    return reloq_fifi_syll_struct_adjust(state, p0);
}
int32_t reloq_fifi_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_fifi_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_fifi_t_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_t_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_t_rules,args,3,3);
    }
    return reloq_fifi_t_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_fifi_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_fifi_text_index(state, p0, p1);
}
int32_t reloq_fifi_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_fifi_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_three_letter_extension(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_three_letter_extension(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_letter_extension,args,2,2);
    }
    return reloq_fifi_three_letter_extension(state, p0, p1);
}
int32_t reloq_fifi_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict,args,3,3);
    }
    return reloq_fifi_tok_dict(state, p0, p1, p2);
}
int32_t reloq_fifi_tok_dict_entry(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_tok_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict_entry,args,3,3);
    }
    return reloq_fifi_tok_dict_entry(state, p0, p1, p2);
}
int32_t reloq_fifi_tok_lookup(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_tok_lookup(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_lookup,args,2,2);
    }
    return reloq_fifi_tok_lookup(state, p0, p1);
}
int32_t reloq_fifi_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_fifi_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_fifi_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_fifi_token_to_number(state, p0, p1, p2);
}
int32_t reloq_fifi_u_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_u_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_u_rules,args,3,3);
    }
    return reloq_fifi_u_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_upgrade_funct_words(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_upgrade_funct_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_upgrade_funct_words,args,4,4);
    }
    return reloq_fifi_upgrade_funct_words(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_user_abbr_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_user_abbr_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_abbr_dict,args,3,3);
    }
    return reloq_fifi_user_abbr_dict(state, p0, p1, p2);
}
int32_t reloq_fifi_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_fifi_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_user_rootdict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_user_rootdict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_rootdict,args,3,3);
    }
    return reloq_fifi_user_rootdict(state, p0, p1, p2);
}
int32_t reloq_fifi_valid_f0_posn(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_valid_f0_posn(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_f0_posn,args,3,3);
    }
    return reloq_fifi_valid_f0_posn(state, p0, p1, p2);
}
int32_t reloq_fifi_velar_burst_spectrum(void *, int32_t);
static inline int32_t reloq_call_fifi_velar_burst_spectrum(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_velar_burst_spectrum,args,1,1);
    }
    return reloq_fifi_velar_burst_spectrum(state, p0);
}
int32_t reloq_fifi_velarize_n(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_velarize_n(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_velarize_n,args,2,2);
    }
    return reloq_fifi_velarize_n(state, p0, p1);
}
int32_t reloq_fifi_verb_set_lookups(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_verb_set_lookups(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_verb_set_lookups,args,5,5);
    }
    return reloq_fifi_verb_set_lookups(state, p0, p1, p2, p3, p4);
}
int32_t reloq_fifi_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_fifi_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_fifi_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_fifi_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_vow_after_nucleus(void *, int32_t);
static inline int32_t reloq_call_fifi_vow_after_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_vow_after_nucleus,args,1,1);
    }
    return reloq_fifi_vow_after_nucleus(state, p0);
}
int32_t reloq_fifi_vow_before_nucleus(void *, int32_t);
static inline int32_t reloq_call_fifi_vow_before_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_vow_before_nucleus,args,1,1);
    }
    return reloq_fifi_vow_before_nucleus(state, p0);
}
int32_t reloq_fifi_warp_nuclear_accent_position(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_warp_nuclear_accent_position(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_warp_nuclear_accent_position,args,3,3);
    }
    return reloq_fifi_warp_nuclear_accent_position(state, p0, p1, p2);
}
int32_t reloq_fifi_warp_prenuclear_accent_position(void *, int32_t, int32_t);
static inline int32_t reloq_call_fifi_warp_prenuclear_accent_position(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_warp_prenuclear_accent_position,args,2,2);
    }
    return reloq_fifi_warp_prenuclear_accent_position(state, p0, p1);
}
int32_t reloq_fifi_which_get_non_vowels(void *, int32_t);
static inline int32_t reloq_call_fifi_which_get_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_which_get_non_vowels,args,1,1);
    }
    return reloq_fifi_which_get_non_vowels(state, p0);
}
int32_t reloq_fifi_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_fifi_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_word_initial_h(void *, int32_t);
static inline int32_t reloq_call_fifi_word_initial_h(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_initial_h,args,1,1);
    }
    return reloq_fifi_word_initial_h(state, p0);
}
int32_t reloq_fifi_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_fifi_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_word_stress_adjust(void *, int32_t);
static inline int32_t reloq_call_fifi_word_stress_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_stress_adjust,args,1,1);
    }
    return reloq_fifi_word_stress_adjust(state, p0);
}
int32_t reloq_fifi_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_fifi_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_y_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_y_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_y_rules,args,3,3);
    }
    return reloq_fifi_y_rules(state, p0, p1, p2);
}
int32_t reloq_fifi_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_fifi_year_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_fifi_z_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_fifi_z_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_z_rules,args,3,3);
    }
    return reloq_fifi_z_rules(state, p0, p1, p2);
}
#endif
