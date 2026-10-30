/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_FRCA61_H
#define RELOQ61_RULE_CALLS_FRCA61_H
#include "delta_rules_c.h"
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_frca61_DeltaProc_end(void *, int32_t);
int32_t reloq_frca61_DeltaProc_flush(void *, int32_t);
int32_t reloq_frca61_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_frca61_DeltaProc_main(void *, int32_t);
int32_t reloq_frca61_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_frca61_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_frca61_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_frca61_DeltaProc_start(void *, int32_t);
int32_t reloq_frca61_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_frca61_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_frca61_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_frca61_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_URL(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_URL(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_URL,args,2,2);
    }
    return reloq_frca61_URL(state, p0, p1);
}
int32_t reloq_frca61_a_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_a_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_a_rules,args,3,3);
    }
    return reloq_frca61_a_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_a_to_A_in_stems(void *, int32_t);
static inline int32_t reloq_call_frca61_a_to_A_in_stems(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_a_to_A_in_stems,args,1,1);
    }
    return reloq_frca61_a_to_A_in_stems(state, p0);
}
int32_t reloq_frca61_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbr,args,4,4);
    }
    return reloq_frca61_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_abbr_before_num(void *, int32_t, int32_t);
int32_t reloq_frca61_abbreviation(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_abbreviation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbreviation,args,4,4);
    }
    return reloq_frca61_abbreviation(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_frca61_absval(state, p0, p1, p2);
}
int32_t reloq_frca61_accented_e_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_accented_e_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_accented_e_rules,args,3,3);
    }
    return reloq_frca61_accented_e_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_acronym(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_acronym(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acronym,args,3,3);
    }
    return reloq_frca61_acronym(state, p0, p1, p2);
}
int32_t reloq_frca61_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acrotest,args,3,3);
    }
    return reloq_frca61_acrotest(state, p0, p1, p2);
}
int32_t reloq_frca61_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_frca61_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_frca61_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_add_f0_for_new_voice(void *, int32_t);
static inline int32_t reloq_call_frca61_add_f0_for_new_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_new_voice,args,1,1);
    }
    return reloq_frca61_add_f0_for_new_voice(state, p0);
}
int32_t reloq_frca61_add_f0_for_next_voice(void *, int32_t);
static inline int32_t reloq_call_frca61_add_f0_for_next_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_next_voice,args,1,1);
    }
    return reloq_frca61_add_f0_for_next_voice(state, p0);
}
int32_t reloq_frca61_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_frca61_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_frca61_add_fren_currency_units(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_add_fren_currency_units(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_fren_currency_units,args,4,4);
    }
    return reloq_frca61_add_fren_currency_units(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_add_fren_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_add_fren_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_fren_point_digits,args,3,3);
    }
    return reloq_frca61_add_fren_point_digits(state, p0, p1, p2);
}
int32_t reloq_frca61_add_fren_subunits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_add_fren_subunits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_fren_subunits,args,3,3);
    }
    return reloq_frca61_add_fren_subunits(state, p0, p1, p2);
}
int32_t reloq_frca61_add_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_add_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_point_digits,args,3,3);
    }
    return reloq_frca61_add_point_digits(state, p0, p1, p2);
}
int32_t reloq_frca61_addval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_addval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addval,args,3,3);
    }
    return reloq_frca61_addval(state, p0, p1, p2);
}
int32_t reloq_frca61_adj_vs_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adj_vs_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adj_vs_adv,args,3,3);
    }
    return reloq_frca61_adj_vs_adv(state, p0, p1, p2);
}
int32_t reloq_frca61_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_frca61_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_frca61_adjust_AV_IP_final(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_AV_IP_final(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_IP_final,args,5,5);
    }
    return reloq_frca61_adjust_AV_IP_final(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_adjust_AV_for_fem_voice(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_AV_for_fem_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_fem_voice,args,1,1);
    }
    return reloq_frca61_adjust_AV_for_fem_voice(state, p0);
}
int32_t reloq_frca61_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_frca61_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_adjust_canfren_phrase_fin_vowels(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_canfren_phrase_fin_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_canfren_phrase_fin_vowels,args,1,1);
    }
    return reloq_frca61_adjust_canfren_phrase_fin_vowels(state, p0);
}
int32_t reloq_frca61_adjust_canfren_phrase_med_vowels(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_canfren_phrase_med_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_canfren_phrase_med_vowels,args,1,1);
    }
    return reloq_frca61_adjust_canfren_phrase_med_vowels(state, p0);
}
int32_t reloq_frca61_adjust_canfren_single_word(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_canfren_single_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_canfren_single_word,args,1,1);
    }
    return reloq_frca61_adjust_canfren_single_word(state, p0);
}
int32_t reloq_frca61_adjust_coda_r_AV(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_coda_r_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_coda_r_AV,args,1,1);
    }
    return reloq_frca61_adjust_coda_r_AV(state, p0);
}
int32_t reloq_frca61_adjust_dur_pitch_accent(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_dur_pitch_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_dur_pitch_accent,args,1,1);
    }
    return reloq_frca61_adjust_dur_pitch_accent(state, p0);
}
int32_t reloq_frca61_adjust_fren_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_fren_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_fren_accents,args,5,5);
    }
    return reloq_frca61_adjust_fren_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_adjust_fren_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_fren_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_fren_word_stress,args,3,3);
    }
    return reloq_frca61_adjust_fren_word_stress(state, p0, p1, p2);
}
int32_t reloq_frca61_adjust_lg_specific_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_lg_specific_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_lg_specific_accents,args,5,5);
    }
    return reloq_frca61_adjust_lg_specific_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_adjust_long_vowels(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_long_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_long_vowels,args,1,1);
    }
    return reloq_frca61_adjust_long_vowels(state, p0);
}
int32_t reloq_frca61_adjust_nasal_tilt(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_nasal_tilt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_nasal_tilt,args,1,1);
    }
    return reloq_frca61_adjust_nasal_tilt(state, p0);
}
int32_t reloq_frca61_adjust_param(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_param(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_param,args,4,4);
    }
    return reloq_frca61_adjust_param(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_adjust_phones(void *, int32_t);
static inline int32_t reloq_call_frca61_adjust_phones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_phones,args,1,1);
    }
    return reloq_frca61_adjust_phones(state, p0);
}
int32_t reloq_frca61_adjust_prev_phr_ending(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_prev_phr_ending(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_ending,args,2,2);
    }
    return reloq_frca61_adjust_prev_phr_ending(state, p0, p1);
}
int32_t reloq_frca61_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_prev_phr_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_pause,args,3,3);
    }
    return reloq_frca61_adjust_prev_phr_pause(state, p0, p1, p2);
}
int32_t reloq_frca61_adjust_t3_for_prosodic_factors(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_t3_for_prosodic_factors(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_t3_for_prosodic_factors,args,3,3);
    }
    return reloq_frca61_adjust_t3_for_prosodic_factors(state, p0, p1, p2);
}
int32_t reloq_frca61_adjust_unstressed_ie(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_unstressed_ie(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_unstressed_ie,args,2,2);
    }
    return reloq_frca61_adjust_unstressed_ie(state, p0, p1);
}
int32_t reloq_frca61_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_frca61_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_frca61_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_adjust_words_in_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_words_in_phrase,args,3,3);
    }
    return reloq_frca61_adjust_words_in_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_adv_liaison(void *, int32_t);
static inline int32_t reloq_call_frca61_adv_liaison(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adv_liaison,args,1,1);
    }
    return reloq_frca61_adv_liaison(state, p0);
}
int32_t reloq_frca61_ambig_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_ambig_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ambig_abbr,args,2,2);
    }
    return reloq_frca61_ambig_abbr(state, p0, p1);
}
int32_t reloq_frca61_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_frca61_apostrophe(state, p0, p1, p2);
}
int32_t reloq_frca61_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_frca61_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_frca61_apply_comma_phr_rules(void *, int32_t);
static inline int32_t reloq_call_frca61_apply_comma_phr_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_comma_phr_rules,args,1,1);
    }
    return reloq_frca61_apply_comma_phr_rules(state, p0);
}
int32_t reloq_frca61_apply_fren_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_fren_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_fren_comma_rules,args,3,3);
    }
    return reloq_frca61_apply_fren_comma_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_frca61_apply_head_size_val(state, p0, p1);
}
int32_t reloq_frca61_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_lang_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_lang_comma_rules,args,3,3);
    }
    return reloq_frca61_apply_lang_comma_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_frca61_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_frca61_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_frca61_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_frca61_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_frca61_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_frca61_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_frca61_apply_roughness_val(state, p0, p1);
}
int32_t reloq_frca61_apply_rules(void *, int32_t);
static inline int32_t reloq_call_frca61_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_frca61_apply_rules(state, p0);
}
int32_t reloq_frca61_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_frca61_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_frca61_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_frca61_apply_speed_val(state, p0, p1);
}
int32_t reloq_frca61_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_frca61_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_frca61_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_frca61_apply_volume_val(state, p0, p1);
}
int32_t reloq_frca61_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_frca61_assign_acoustic_vals(state, p0);
}
int32_t reloq_frca61_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_frca61_assign_alt_quest_features(state, p0);
}
int32_t reloq_frca61_assign_canfren_affricates(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_canfren_affricates(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_canfren_affricates,args,1,1);
    }
    return reloq_frca61_assign_canfren_affricates(state, p0);
}
int32_t reloq_frca61_assign_canfren_start_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_canfren_start_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_canfren_start_dur,args,1,1);
    }
    return reloq_frca61_assign_canfren_start_dur(state, p0);
}
int32_t reloq_frca61_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_frca61_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_frca61_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_frca61_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_frca61_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_frca61_assign_default_quest_features(state, p0);
}
int32_t reloq_frca61_assign_fren_gender_to_number(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_assign_fren_gender_to_number(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_fren_gender_to_number,args,2,2);
    }
    return reloq_frca61_assign_fren_gender_to_number(state, p0, p1);
}
int32_t reloq_frca61_assign_fren_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_fren_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_fren_nuc_durs,args,1,1);
    }
    return reloq_frca61_assign_fren_nuc_durs(state, p0);
}
int32_t reloq_frca61_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_frca61_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_frca61_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_frca61_assign_nuc_durs(state, p0);
}
int32_t reloq_frca61_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_frca61_assign_phone_durs(state, p0);
}
int32_t reloq_frca61_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_frca61_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_frca61_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_frca61_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_assign_primary_stress(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_primary_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_primary_stress,args,1,1);
    }
    return reloq_frca61_assign_primary_stress(state, p0);
}
int32_t reloq_frca61_assign_secondary_stress(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_secondary_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_secondary_stress,args,1,1);
    }
    return reloq_frca61_assign_secondary_stress(state, p0);
}
int32_t reloq_frca61_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_frca61_assign_sent_final_features(state, p0);
}
int32_t reloq_frca61_assign_stress(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_stress,args,1,1);
    }
    return reloq_frca61_assign_stress(state, p0);
}
int32_t reloq_frca61_assign_syll_coda(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_assign_syll_coda(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_syll_coda,args,2,2);
    }
    return reloq_frca61_assign_syll_coda(state, p0, p1);
}
int32_t reloq_frca61_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_frca61_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_frca61_assign_user_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_assign_user_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_user_stress,args,3,3);
    }
    return reloq_frca61_assign_user_stress(state, p0, p1, p2);
}
int32_t reloq_frca61_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_frca61_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_frca61_assign_wh_quest_features(state, p0);
}
int32_t reloq_frca61_average(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_average(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_average,args,3,3);
    }
    return reloq_frca61_average(state, p0, p1, p2);
}
int32_t reloq_frca61_b_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_b_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_b_rules,args,3,3);
    }
    return reloq_frca61_b_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_before_h_aspire(void *, int32_t);
static inline int32_t reloq_call_frca61_before_h_aspire(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_before_h_aspire,args,1,1);
    }
    return reloq_frca61_before_h_aspire(state, p0);
}
int32_t reloq_frca61_beg_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_beg_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_beg_of_word,args,2,2);
    }
    return reloq_frca61_beg_of_word(state, p0, p1);
}
int32_t reloq_frca61_between_proper_names(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_block_hls_accent(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_block_vocative(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_block_vocative(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_block_vocative,args,3,3);
    }
    return reloq_frca61_block_vocative(state, p0, p1, p2);
}
int32_t reloq_frca61_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_boundary_tone,args,1,1);
    }
    return reloq_frca61_boundary_tone(state, p0);
}
int32_t reloq_frca61_bracket(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_bracket(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_bracket,args,3,3);
    }
    return reloq_frca61_bracket(state, p0, p1, p2);
}
int32_t reloq_frca61_break_into_canfren_phones(void *, int32_t);
static inline int32_t reloq_call_frca61_break_into_canfren_phones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_break_into_canfren_phones,args,1,1);
    }
    return reloq_frca61_break_into_canfren_phones(state, p0);
}
int32_t reloq_frca61_break_into_morphs(void *, int32_t);
static inline int32_t reloq_call_frca61_break_into_morphs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_break_into_morphs,args,1,1);
    }
    return reloq_frca61_break_into_morphs(state, p0);
}
int32_t reloq_frca61_break_into_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_break_into_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_phones,args,3,3);
    }
    return reloq_frca61_break_into_phones(state, p0, p1, p2);
}
int32_t reloq_frca61_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_frca61_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_frca61_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_frca61_build_phrases(state, p0, p1, p2);
}
int32_t reloq_frca61_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_frca61_build_words(state, p0, p1, p2);
}
int32_t reloq_frca61_c_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_c_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_c_rules,args,3,3);
    }
    return reloq_frca61_c_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_calc_peak(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_calc_peak(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_calc_peak,args,3,3);
    }
    return reloq_frca61_calc_peak(state, p0, p1, p2);
}
int32_t reloq_frca61_calculate_first_hls_value(void *, int32_t, int32_t);
int32_t reloq_frca61_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_frca61_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_frca61_can_ph_unvoic_fric_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_can_ph_unvoic_fric_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_can_ph_unvoic_fric_dur,args,1,1);
    }
    return reloq_frca61_can_ph_unvoic_fric_dur(state, p0);
}
int32_t reloq_frca61_can_ph_voic_fric_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_can_ph_voic_fric_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_can_ph_voic_fric_dur,args,1,1);
    }
    return reloq_frca61_can_ph_voic_fric_dur(state, p0);
}
int32_t reloq_frca61_canfren_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_nucl_low_tone,args,1,1);
    }
    return reloq_frca61_canfren_nucl_low_tone(state, p0);
}
int32_t reloq_frca61_canfren_ph_A(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_A(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_A,args,1,1);
    }
    return reloq_frca61_canfren_ph_A(state, p0);
}
int32_t reloq_frca61_canfren_ph_E(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_E,args,1,1);
    }
    return reloq_frca61_canfren_ph_E(state, p0);
}
int32_t reloq_frca61_canfren_ph_E_long(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_E_long(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_E_long,args,1,1);
    }
    return reloq_frca61_canfren_ph_E_long(state, p0);
}
int32_t reloq_frca61_canfren_ph_E_nas(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_E_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_E_nas,args,1,1);
    }
    return reloq_frca61_canfren_ph_E_nas(state, p0);
}
int32_t reloq_frca61_canfren_ph_E_nas_dip(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_E_nas_dip(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_E_nas_dip,args,2,2);
    }
    return reloq_frca61_canfren_ph_E_nas_dip(state, p0, p1);
}
int32_t reloq_frca61_canfren_ph_E_nas_nodip(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_E_nas_nodip(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_E_nas_nodip,args,1,1);
    }
    return reloq_frca61_canfren_ph_E_nas_nodip(state, p0);
}
int32_t reloq_frca61_canfren_ph_I(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_I(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_I,args,1,1);
    }
    return reloq_frca61_canfren_ph_I(state, p0);
}
int32_t reloq_frca61_canfren_ph_U(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_U(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_U,args,1,1);
    }
    return reloq_frca61_canfren_ph_U(state, p0);
}
int32_t reloq_frca61_canfren_ph_Y(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_Y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_Y,args,1,1);
    }
    return reloq_frca61_canfren_ph_Y(state, p0);
}
int32_t reloq_frca61_canfren_ph_a(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_a,args,1,1);
    }
    return reloq_frca61_canfren_ph_a(state, p0);
}
int32_t reloq_frca61_canfren_ph_a_nas(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_a_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_a_nas,args,1,1);
    }
    return reloq_frca61_canfren_ph_a_nas(state, p0);
}
int32_t reloq_frca61_canfren_ph_c(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_c(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_c,args,1,1);
    }
    return reloq_frca61_canfren_ph_c(state, p0);
}
int32_t reloq_frca61_canfren_ph_e(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_e,args,1,1);
    }
    return reloq_frca61_canfren_ph_e(state, p0);
}
int32_t reloq_frca61_canfren_ph_eu(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_eu(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_eu,args,1,1);
    }
    return reloq_frca61_canfren_ph_eu(state, p0);
}
int32_t reloq_frca61_canfren_ph_i(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_i,args,1,1);
    }
    return reloq_frca61_canfren_ph_i(state, p0);
}
int32_t reloq_frca61_canfren_ph_nas_glide(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_nas_glide(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_nas_glide,args,1,1);
    }
    return reloq_frca61_canfren_ph_nas_glide(state, p0);
}
int32_t reloq_frca61_canfren_ph_o_nas(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_o_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_o_nas,args,1,1);
    }
    return reloq_frca61_canfren_ph_o_nas(state, p0);
}
int32_t reloq_frca61_canfren_ph_oe(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_oe(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_oe,args,1,1);
    }
    return reloq_frca61_canfren_ph_oe(state, p0);
}
int32_t reloq_frca61_canfren_ph_oe_nas(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_oe_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_oe_nas,args,1,1);
    }
    return reloq_frca61_canfren_ph_oe_nas(state, p0);
}
int32_t reloq_frca61_canfren_ph_r(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_r,args,1,1);
    }
    return reloq_frca61_canfren_ph_r(state, p0);
}
int32_t reloq_frca61_canfren_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_ph_r_dur,args,1,1);
    }
    return reloq_frca61_canfren_ph_r_dur(state, p0);
}
int32_t reloq_frca61_canfren_phon_rules(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_phon_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_phon_rules,args,1,1);
    }
    return reloq_frca61_canfren_phon_rules(state, p0);
}
int32_t reloq_frca61_canfren_roots(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_canfren_roots(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_canfren_roots,args,3,3);
    }
    return reloq_frca61_canfren_roots(state, p0, p1, p2);
}
int32_t reloq_frca61_canfren_stress_one_syll(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_stress_one_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_stress_one_syll,args,1,1);
    }
    return reloq_frca61_canfren_stress_one_syll(state, p0);
}
int32_t reloq_frca61_canfren_stress_two_syll(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_stress_two_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_stress_two_syll,args,1,1);
    }
    return reloq_frca61_canfren_stress_two_syll(state, p0);
}
int32_t reloq_frca61_canfren_uvu_Fv(void *, int32_t);
static inline int32_t reloq_call_frca61_canfren_uvu_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_canfren_uvu_Fv,args,1,1);
    }
    return reloq_frca61_canfren_uvu_Fv(state, p0);
}
int32_t reloq_frca61_canfren_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_canfren_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_canfren_words,args,3,3);
    }
    return reloq_frca61_canfren_words(state, p0, p1, p2);
}
int32_t reloq_frca61_cc_across_accentual_phrase(void *, int32_t);
static inline int32_t reloq_call_frca61_cc_across_accentual_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_cc_across_accentual_phrase,args,1,1);
    }
    return reloq_frca61_cc_across_accentual_phrase(state, p0);
}
int32_t reloq_frca61_ce_vow_elision(void *, int32_t);
static inline int32_t reloq_call_frca61_ce_vow_elision(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ce_vow_elision,args,1,1);
    }
    return reloq_frca61_ce_vow_elision(state, p0);
}
int32_t reloq_frca61_cedille_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_cedille_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_cedille_rules,args,3,3);
    }
    return reloq_frca61_cedille_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_cf_default_shortening(void *, int32_t);
static inline int32_t reloq_call_frca61_cf_default_shortening(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_cf_default_shortening,args,1,1);
    }
    return reloq_frca61_cf_default_shortening(state, p0);
}
int32_t reloq_frca61_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_frca61_char_name(state, p0, p1, p2);
}
int32_t reloq_frca61_clear_delta(void *, int32_t);
static inline int32_t reloq_call_frca61_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_frca61_clear_delta(state, p0);
}
int32_t reloq_frca61_close_platform(void *, int32_t);
static inline int32_t reloq_call_frca61_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_frca61_close_platform(state, p0);
}
int32_t reloq_frca61_coarticulate_can_r(void *, int32_t);
static inline int32_t reloq_call_frca61_coarticulate_can_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_coarticulate_can_r,args,1,1);
    }
    return reloq_frca61_coarticulate_can_r(state, p0);
}
int32_t reloq_frca61_comma_at_end_quote(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_comma_at_end_quote(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_comma_at_end_quote,args,3,3);
    }
    return reloq_frca61_comma_at_end_quote(state, p0, p1, p2);
}
int32_t reloq_frca61_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_frca61_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_frca61_comma_phrase(state, p0);
}
int32_t reloq_frca61_con_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_frca61_con_vals(state, p0);
}
int32_t reloq_frca61_conjoined_words_same_category(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_conjoined_words_same_category(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_conjoined_words_same_category,args,3,3);
    }
    return reloq_frca61_conjoined_words_same_category(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_1_or_2_digits(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_convert_NA_phone_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_NA_phone_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_NA_phone_number,args,3,3);
    }
    return reloq_frca61_convert_NA_phone_number(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_billions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_billions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_billions,args,3,3);
    }
    return reloq_frca61_convert_billions(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits,args,3,3);
    }
    return reloq_frca61_convert_digits(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits_to_words,args,3,3);
    }
    return reloq_frca61_convert_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_double,args,3,3);
    }
    return reloq_frca61_convert_double(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_billions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_billions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_billions,args,3,3);
    }
    return reloq_frca61_convert_fren_billions(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_date,args,3,3);
    }
    return reloq_frca61_convert_fren_date(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_digits_to_words,args,3,3);
    }
    return reloq_frca61_convert_fren_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_double,args,3,3);
    }
    return reloq_frca61_convert_fren_double(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_fraction,args,3,3);
    }
    return reloq_frca61_convert_fren_fraction(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_hundreds,args,3,3);
    }
    return reloq_frca61_convert_fren_hundreds(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_millions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_millions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_millions,args,3,3);
    }
    return reloq_frca61_convert_fren_millions(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_monetary_exp,args,5,5);
    }
    return reloq_frca61_convert_fren_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_convert_fren_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_ordinal,args,3,3);
    }
    return reloq_frca61_convert_fren_ordinal(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_phone_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_phone_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_phone_number,args,3,3);
    }
    return reloq_frca61_convert_fren_phone_number(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_roman_num,args,3,3);
    }
    return reloq_frca61_convert_fren_roman_num(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_single_digit,args,3,3);
    }
    return reloq_frca61_convert_fren_single_digit(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_teens(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_teens(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_teens,args,3,3);
    }
    return reloq_frca61_convert_fren_teens(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_thousands,args,3,3);
    }
    return reloq_frca61_convert_fren_thousands(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_fren_time(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_fren_time(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_fren_time,args,3,3);
    }
    return reloq_frca61_convert_fren_time(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_hundreds,args,3,3);
    }
    return reloq_frca61_convert_hundreds(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_lang_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_lang_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_lang_digits_to_words,args,3,3);
    }
    return reloq_frca61_convert_lang_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_millions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_millions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_millions,args,3,3);
    }
    return reloq_frca61_convert_millions(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_monetary_exp,args,5,5);
    }
    return reloq_frca61_convert_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_convert_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_numbers,args,3,3);
    }
    return reloq_frca61_convert_numbers(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_phone_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_phone_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_phone_number,args,3,3);
    }
    return reloq_frca61_convert_phone_number(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_roman_num,args,3,3);
    }
    return reloq_frca61_convert_roman_num(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_roman_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_roman_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_roman_nums,args,3,3);
    }
    return reloq_frca61_convert_roman_nums(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_single_digit,args,3,3);
    }
    return reloq_frca61_convert_single_digit(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_thousands,args,3,3);
    }
    return reloq_frca61_convert_thousands(state, p0, p1, p2);
}
int32_t reloq_frca61_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_frca61_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_frca61_coord_conj(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_coord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_coord_conj,args,4,4);
    }
    return reloq_frca61_coord_conj(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_frca61_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_frca61_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_frca61_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_frca61_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_frca61_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_frca61_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_count_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_count_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_digits,args,3,3);
    }
    return reloq_frca61_count_digits(state, p0, p1, p2);
}
int32_t reloq_frca61_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_frca61_count_input(state, p0, p1);
}
int32_t reloq_frca61_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_frca61_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_frca61_count_normalized_inp(state, p0);
}
int32_t reloq_frca61_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_count_sylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_sylls,args,4,4);
    }
    return reloq_frca61_count_sylls(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_frca61_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_frca61_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_frca61_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_frca61_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_frca61_create_sentence(state, p0, p1);
}
int32_t reloq_frca61_create_syllables(void *, int32_t);
static inline int32_t reloq_call_frca61_create_syllables(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_create_syllables,args,1,1);
    }
    return reloq_frca61_create_syllables(state, p0);
}
int32_t reloq_frca61_d_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_d_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_d_rules,args,3,3);
    }
    return reloq_frca61_d_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_frca61_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_frca61_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_frca61_delete_2_more(state, p0);
}
int32_t reloq_frca61_delete_comma_sync(void *, int32_t, int32_t);
int32_t reloq_frca61_delete_h_aspire(void *, int32_t);
static inline int32_t reloq_call_frca61_delete_h_aspire(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_h_aspire,args,1,1);
    }
    return reloq_frca61_delete_h_aspire(state, p0);
}
int32_t reloq_frca61_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delete_inp_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_left,args,3,3);
    }
    return reloq_frca61_delete_inp_from_left(state, p0, p1, p2);
}
int32_t reloq_frca61_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delete_inp_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_right,args,3,3);
    }
    return reloq_frca61_delete_inp_from_right(state, p0, p1, p2);
}
int32_t reloq_frca61_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_frca61_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_frca61_delete_liaison_con(void *, int32_t);
static inline int32_t reloq_call_frca61_delete_liaison_con(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_liaison_con,args,1,1);
    }
    return reloq_frca61_delete_liaison_con(state, p0);
}
int32_t reloq_frca61_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delete_morph_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_morph_from_left,args,3,3);
    }
    return reloq_frca61_delete_morph_from_left(state, p0, p1, p2);
}
int32_t reloq_frca61_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_frca61_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_frca61_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delete_phone_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_right,args,3,3);
    }
    return reloq_frca61_delete_phone_from_right(state, p0, p1, p2);
}
int32_t reloq_frca61_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_frca61_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_frca61_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_frca61_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_frca61_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_frca61_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_frca61_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_frca61_demote_word(state, p0, p1, p2);
}
int32_t reloq_frca61_denasalization(void *, int32_t);
static inline int32_t reloq_call_frca61_denasalization(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_denasalization,args,1,1);
    }
    return reloq_frca61_denasalization(state, p0);
}
int32_t reloq_frca61_destress_FW(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_destress_FW(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_FW,args,3,3);
    }
    return reloq_frca61_destress_FW(state, p0, p1, p2);
}
int32_t reloq_frca61_destress_NPs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_destress_NPs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_NPs,args,5,5);
    }
    return reloq_frca61_destress_NPs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_destress_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_destress_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_adv,args,3,3);
    }
    return reloq_frca61_destress_adv(state, p0, p1, p2);
}
int32_t reloq_frca61_destress_hyphenated_wds(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_destress_hyphenated_wds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_hyphenated_wds,args,4,4);
    }
    return reloq_frca61_destress_hyphenated_wds(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_destress_number_series(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_destress_number_series(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_number_series,args,3,3);
    }
    return reloq_frca61_destress_number_series(state, p0, p1, p2);
}
int32_t reloq_frca61_destress_titles(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_destress_titles(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_titles,args,3,3);
    }
    return reloq_frca61_destress_titles(state, p0, p1, p2);
}
int32_t reloq_frca61_destress_verbs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_destress_verbs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_verbs,args,3,3);
    }
    return reloq_frca61_destress_verbs(state, p0, p1, p2);
}
int32_t reloq_frca61_det_vs_pro_clitic(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_det_vs_pro_clitic(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_det_vs_pro_clitic,args,3,3);
    }
    return reloq_frca61_det_vs_pro_clitic(state, p0, p1, p2);
}
int32_t reloq_frca61_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_frca61_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_disambig_fren_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambig_fren_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_fren_content_word,args,3,3);
    }
    return reloq_frca61_disambig_fren_content_word(state, p0, p1, p2);
}
int32_t reloq_frca61_disambig_fren_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambig_fren_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_fren_functword,args,3,3);
    }
    return reloq_frca61_disambig_fren_functword(state, p0, p1, p2);
}
int32_t reloq_frca61_disambig_fren_single_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambig_fren_single_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_fren_single_word,args,3,3);
    }
    return reloq_frca61_disambig_fren_single_word(state, p0, p1, p2);
}
int32_t reloq_frca61_disambig_ment_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambig_ment_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_ment_words,args,3,3);
    }
    return reloq_frca61_disambig_ment_words(state, p0, p1, p2);
}
int32_t reloq_frca61_disambig_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambig_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_number,args,3,3);
    }
    return reloq_frca61_disambig_number(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_ant_prep(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_ant_prep(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_ant_prep,args,4,4);
    }
    return reloq_frca61_disambiguate_ant_prep(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_disambiguate_as(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_as(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_as,args,3,3);
    }
    return reloq_frca61_disambiguate_as(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_car(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_car(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_car,args,3,3);
    }
    return reloq_frca61_disambiguate_car(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_content_word,args,4,4);
    }
    return reloq_frca61_disambiguate_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_disambiguate_en(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_en(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_en,args,3,3);
    }
    return reloq_frca61_disambiguate_en(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_est(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_est(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_est,args,3,3);
    }
    return reloq_frca61_disambiguate_est(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_functword,args,3,3);
    }
    return reloq_frca61_disambiguate_functword(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_le(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_le(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_le,args,3,3);
    }
    return reloq_frca61_disambiguate_le(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_pastpart(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_pastpart(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_pastpart,args,3,3);
    }
    return reloq_frca61_disambiguate_pastpart(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_pos_pron(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_pos_pron(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_pos_pron,args,3,3);
    }
    return reloq_frca61_disambiguate_pos_pron(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_que(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_que(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_que,args,3,3);
    }
    return reloq_frca61_disambiguate_que(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_si(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_si(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_si,args,3,3);
    }
    return reloq_frca61_disambiguate_si(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_single_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_single_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_single_word,args,3,3);
    }
    return reloq_frca61_disambiguate_single_word(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_soit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_soit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_soit,args,3,3);
    }
    return reloq_frca61_disambiguate_soit(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_sus(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_sus(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_sus,args,3,3);
    }
    return reloq_frca61_disambiguate_sus(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_tiens(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_tiens(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_tiens,args,3,3);
    }
    return reloq_frca61_disambiguate_tiens(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_tous(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_tous(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_tous,args,3,3);
    }
    return reloq_frca61_disambiguate_tous(state, p0, p1, p2);
}
int32_t reloq_frca61_disambiguate_un(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_disambiguate_un(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_un,args,3,3);
    }
    return reloq_frca61_disambiguate_un(state, p0, p1, p2);
}
int32_t reloq_frca61_discourse_pitch_modifications(void *, int32_t, int32_t);
int32_t reloq_frca61_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_frca61_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_frca61_distribute_nucdur(state, p0);
}
int32_t reloq_frca61_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_frca61_divide_char_count_token(state, p0, p1);
}
int32_t reloq_frca61_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_frca61_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_frca61_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_frca61_doF(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_frca61_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_frca61_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_frca61_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_frca61_do_FTZ_FTP(state, p0);
}
int32_t reloq_frca61_do_canfren_cons_clust_r(void *, int32_t);
static inline int32_t reloq_call_frca61_do_canfren_cons_clust_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_canfren_cons_clust_r,args,1,1);
    }
    return reloq_frca61_do_canfren_cons_clust_r(state, p0);
}
int32_t reloq_frca61_do_canfren_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_do_canfren_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_canfren_dur,args,1,1);
    }
    return reloq_frca61_do_canfren_dur(state, p0);
}
int32_t reloq_frca61_do_canfren_intervocalic_r(void *, int32_t);
static inline int32_t reloq_call_frca61_do_canfren_intervocalic_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_canfren_intervocalic_r,args,1,1);
    }
    return reloq_frca61_do_canfren_intervocalic_r(state, p0);
}
int32_t reloq_frca61_do_canfren_phrase_final_r(void *, int32_t);
static inline int32_t reloq_call_frca61_do_canfren_phrase_final_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_canfren_phrase_final_r,args,1,1);
    }
    return reloq_frca61_do_canfren_phrase_final_r(state, p0);
}
int32_t reloq_frca61_do_canfren_phrase_initial_r(void *, int32_t);
static inline int32_t reloq_call_frca61_do_canfren_phrase_initial_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_canfren_phrase_initial_r,args,1,1);
    }
    return reloq_frca61_do_canfren_phrase_initial_r(state, p0);
}
int32_t reloq_frca61_do_canfren_r_coda(void *, int32_t);
static inline int32_t reloq_call_frca61_do_canfren_r_coda(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_canfren_r_coda,args,1,1);
    }
    return reloq_frca61_do_canfren_r_coda(state, p0);
}
int32_t reloq_frca61_do_canfren_r_glide(void *, int32_t);
static inline int32_t reloq_call_frca61_do_canfren_r_glide(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_canfren_r_glide,args,1,1);
    }
    return reloq_frca61_do_canfren_r_glide(state, p0);
}
int32_t reloq_frca61_do_fren_voice_fric_AV(void *, int32_t);
static inline int32_t reloq_call_frca61_do_fren_voice_fric_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_fren_voice_fric_AV,args,1,1);
    }
    return reloq_frca61_do_fren_voice_fric_AV(state, p0);
}
int32_t reloq_frca61_do_fren_voice_stop_AV(void *, int32_t);
static inline int32_t reloq_call_frca61_do_fren_voice_stop_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_fren_voice_stop_AV,args,1,1);
    }
    return reloq_frca61_do_fren_voice_stop_AV(state, p0);
}
int32_t reloq_frca61_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_frca61_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_frca61_do_glot_onset(void *, int32_t);
static inline int32_t reloq_call_frca61_do_glot_onset(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_glot_onset,args,1,1);
    }
    return reloq_frca61_do_glot_onset(state, p0);
}
int32_t reloq_frca61_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_frca61_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_frca61_do_non_son_amps(state, p0);
}
int32_t reloq_frca61_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_frca61_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_frca61_do_pt_voicing(state, p0);
}
int32_t reloq_frca61_do_release(void *, int32_t);
static inline int32_t reloq_call_frca61_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_frca61_do_release(state, p0);
}
int32_t reloq_frca61_do_schwa_durations(void *, int32_t);
static inline int32_t reloq_call_frca61_do_schwa_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_schwa_durations,args,1,1);
    }
    return reloq_frca61_do_schwa_durations(state, p0);
}
int32_t reloq_frca61_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_frca61_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_frca61_do_son_amps(state, p0);
}
int32_t reloq_frca61_do_stanfren_phrase_initial_r(void *, int32_t);
int32_t reloq_frca61_do_voice_bar(void *, int32_t);
int32_t reloq_frca61_e_acute_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_e_acute_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_e_acute_rules,args,3,3);
    }
    return reloq_frca61_e_acute_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_e_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_e_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_e_rules,args,3,3);
    }
    return reloq_frca61_e_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_frca61_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_frca61_ellipsis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_ellipsis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ellipsis,args,3,3);
    }
    return reloq_frca61_ellipsis(state, p0, p1, p2);
}
int32_t reloq_frca61_em_rules(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_em_rules(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_em_rules,args,4,4);
    }
    return reloq_frca61_em_rules(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_email_address(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_email_address(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_email_address,args,2,2);
    }
    return reloq_frca61_email_address(state, p0, p1);
}
int32_t reloq_frca61_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_frca61_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_frca61_en_rules(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_en_rules(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_en_rules,args,4,4);
    }
    return reloq_frca61_en_rules(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_frca61_end_of_sentence(state, p0, p1);
}
int32_t reloq_frca61_end_of_token(void *, int32_t, int32_t);
int32_t reloq_frca61_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_frca61_end_of_word(state, p0, p1);
}
int32_t reloq_frca61_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_frca61_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_er_rules(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_er_rules(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_er_rules,args,4,4);
    }
    return reloq_frca61_er_rules(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_estimate_fren_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_estimate_fren_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_fren_nsylls,args,4,4);
    }
    return reloq_frca61_estimate_fren_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_estimate_lang_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_estimate_lang_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_lang_nsylls,args,4,4);
    }
    return reloq_frca61_estimate_lang_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_estimate_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_nsylls,args,4,4);
    }
    return reloq_frca61_estimate_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_eu_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_eu_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eu_rules,args,3,3);
    }
    return reloq_frca61_eu_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_exclam_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_exclam_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_exclam_phrase,args,2,2);
    }
    return reloq_frca61_exclam_phrase(state, p0, p1);
}
int32_t reloq_frca61_extend_vals(void *, int32_t);
int32_t reloq_frca61_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_frca61_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_f_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_f_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f_rules,args,3,3);
    }
    return reloq_frca61_f_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_fils_is_thread(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fils_is_thread(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fils_is_thread,args,3,3);
    }
    return reloq_frca61_fils_is_thread(state, p0, p1, p2);
}
int32_t reloq_frca61_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_frca61_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_frca61_final_er_pronounced_Er(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_final_er_pronounced_Er(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_er_pronounced_Er,args,3,3);
    }
    return reloq_frca61_final_er_pronounced_Er(state, p0, p1, p2);
}
int32_t reloq_frca61_final_er_pronounced_oer(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_final_er_pronounced_oer(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_er_pronounced_oer,args,3,3);
    }
    return reloq_frca61_final_er_pronounced_oer(state, p0, p1, p2);
}
int32_t reloq_frca61_final_son_in_3_cluster(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_final_son_in_3_cluster(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_son_in_3_cluster,args,2,2);
    }
    return reloq_frca61_final_son_in_3_cluster(state, p0, p1);
}
int32_t reloq_frca61_final_voiced_release(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_final_voiced_release(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_voiced_release,args,3,3);
    }
    return reloq_frca61_final_voiced_release(state, p0, p1, p2);
}
int32_t reloq_frca61_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_frca61_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_frca61_find_acoustic_nuc(state, p0);
}
int32_t reloq_frca61_find_aux_before_pastpart(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_aux_before_pastpart(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_aux_before_pastpart,args,2,2);
    }
    return reloq_frca61_find_aux_before_pastpart(state, p0, p1);
}
int32_t reloq_frca61_find_avoir_before_pastpart(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_avoir_before_pastpart(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_avoir_before_pastpart,args,2,2);
    }
    return reloq_frca61_find_avoir_before_pastpart(state, p0, p1);
}
int32_t reloq_frca61_find_be_inflections(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_be_inflections(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_be_inflections,args,4,4);
    }
    return reloq_frca61_find_be_inflections(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_enir_verbs(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_enir_verbs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_enir_verbs,args,4,4);
    }
    return reloq_frca61_find_enir_verbs(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_fren_nucleus(void *, int32_t);
static inline int32_t reloq_call_frca61_find_fren_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_fren_nucleus,args,1,1);
    }
    return reloq_frca61_find_fren_nucleus(state, p0);
}
int32_t reloq_frca61_find_fren_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_fren_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_fren_obligatory_phrase,args,4,4);
    }
    return reloq_frca61_find_fren_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_fren_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_fren_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_fren_potential_phrase,args,4,4);
    }
    return reloq_frca61_find_fren_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_have_inflections(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_have_inflections(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_have_inflections,args,4,4);
    }
    return reloq_frca61_find_have_inflections(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_lang_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_lang_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_obligatory_phrase,args,4,4);
    }
    return reloq_frca61_find_lang_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_lang_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_lang_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_potential_phrase,args,4,4);
    }
    return reloq_frca61_find_lang_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_frca61_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_modal_inflections(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_modal_inflections(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_modal_inflections,args,4,4);
    }
    return reloq_frca61_find_modal_inflections(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_modals_from_sets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_modals_from_sets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_modals_from_sets,args,3,3);
    }
    return reloq_frca61_find_modals_from_sets(state, p0, p1, p2);
}
int32_t reloq_frca61_find_most_stressed_syll(void *, int32_t);
static inline int32_t reloq_call_frca61_find_most_stressed_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_most_stressed_syll,args,1,1);
    }
    return reloq_frca61_find_most_stressed_syll(state, p0);
}
int32_t reloq_frca61_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_frca61_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_next_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_word,args,4,4);
    }
    return reloq_frca61_find_next_word(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_frca61_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_frca61_find_nucleus(state, p0);
}
int32_t reloq_frca61_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_frca61_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_frca61_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_previous_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_previous_word,args,4,4);
    }
    return reloq_frca61_find_previous_word(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_find_unpuncted_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_find_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_unpuncted_phrases,args,3,3);
    }
    return reloq_frca61_find_unpuncted_phrases(state, p0, p1, p2);
}
int32_t reloq_frca61_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_frca61_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_first_postnucl_accent,args,1,1);
    }
    return reloq_frca61_first_postnucl_accent(state, p0);
}
int32_t reloq_frca61_first_root_vow(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_first_root_vow(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_first_root_vow,args,2,2);
    }
    return reloq_frca61_first_root_vow(state, p0, p1);
}
int32_t reloq_frca61_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_follows_cest_vrai(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_follows_cest_vrai(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_follows_cest_vrai,args,3,3);
    }
    return reloq_frca61_follows_cest_vrai(state, p0, p1, p2);
}
int32_t reloq_frca61_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_frca61_forced_synthesis(state, p0, p1);
}
int32_t reloq_frca61_fren_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_abbr,args,3,3);
    }
    return reloq_frca61_fren_abbr(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_acrotest,args,3,3);
    }
    return reloq_frca61_fren_acrotest(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_boundary_tone(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_boundary_tone(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_boundary_tone,args,2,2);
    }
    return reloq_frca61_fren_boundary_tone(state, p0, p1);
}
int32_t reloq_frca61_fren_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_char_name,args,3,3);
    }
    return reloq_frca61_fren_char_name(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_con_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_con_vals,args,1,1);
    }
    return reloq_frca61_fren_con_vals(state, p0);
}
int32_t reloq_frca61_fren_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_cons_durs,args,3,3);
    }
    return reloq_frca61_fren_cons_durs(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_dental_Fv(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_dental_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_dental_Fv,args,1,1);
    }
    return reloq_frca61_fren_dental_Fv(state, p0);
}
int32_t reloq_frca61_fren_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_first_postnucl_accent,args,1,1);
    }
    return reloq_frca61_fren_first_postnucl_accent(state, p0);
}
int32_t reloq_frca61_fren_funct_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_funct_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_funct_words,args,3,3);
    }
    return reloq_frca61_fren_funct_words(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_gramm_cat_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_gramm_cat_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_gramm_cat_dict,args,3,3);
    }
    return reloq_frca61_fren_gramm_cat_dict(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_gramm_cat_disambig_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_gramm_cat_disambig_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_gramm_cat_disambig_dict,args,3,3);
    }
    return reloq_frca61_fren_gramm_cat_disambig_dict(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_high_low_star_tone,args,1,1);
    }
    return reloq_frca61_fren_high_low_star_tone(state, p0);
}
int32_t reloq_frca61_fren_high_minus_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_high_minus_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_high_minus_tone,args,1,1);
    }
    return reloq_frca61_fren_high_minus_tone(state, p0);
}
int32_t reloq_frca61_fren_high_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_high_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_high_pal_Fv,args,1,1);
    }
    return reloq_frca61_fren_high_pal_Fv(state, p0);
}
int32_t reloq_frca61_fren_high_plus_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_high_plus_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_high_plus_tone,args,1,1);
    }
    return reloq_frca61_fren_high_plus_tone(state, p0);
}
int32_t reloq_frca61_fren_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_high_star_low_tone,args,1,1);
    }
    return reloq_frca61_fren_high_star_low_tone(state, p0);
}
int32_t reloq_frca61_fren_high_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_high_tone,args,1,1);
    }
    return reloq_frca61_fren_high_tone(state, p0);
}
int32_t reloq_frca61_fren_homog_roots(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_homog_roots(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_homog_roots,args,3,3);
    }
    return reloq_frca61_fren_homog_roots(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_homog_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_homog_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_homog_words,args,3,3);
    }
    return reloq_frca61_fren_homog_words(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_hyphen_between_nums,args,5,5);
    }
    return reloq_frca61_fren_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_fren_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_init_word_inton_vars,args,1,1);
    }
    return reloq_frca61_fren_init_word_inton_vars(state, p0);
}
int32_t reloq_frca61_fren_labial_Fv(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_labial_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_labial_Fv,args,1,1);
    }
    return reloq_frca61_fren_labial_Fv(state, p0);
}
int32_t reloq_frca61_fren_lat_Fv(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_lat_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_lat_Fv,args,1,1);
    }
    return reloq_frca61_fren_lat_Fv(state, p0);
}
int32_t reloq_frca61_fren_legal_final_cons_pair(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_legal_final_cons_pair(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_legal_final_cons_pair,args,2,2);
    }
    return reloq_frca61_fren_legal_final_cons_pair(state, p0, p1);
}
int32_t reloq_frca61_fren_legal_initial_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_legal_initial_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_legal_initial_cons,args,2,2);
    }
    return reloq_frca61_fren_legal_initial_cons(state, p0, p1);
}
int32_t reloq_frca61_fren_legal_word_final_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_legal_word_final_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_legal_word_final_cons,args,2,2);
    }
    return reloq_frca61_fren_legal_word_final_cons(state, p0, p1);
}
int32_t reloq_frca61_fren_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_low_high_star_tone,args,1,1);
    }
    return reloq_frca61_fren_low_high_star_tone(state, p0);
}
int32_t reloq_frca61_fren_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_low_star_high_tone,args,1,1);
    }
    return reloq_frca61_fren_low_star_high_tone(state, p0);
}
int32_t reloq_frca61_fren_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_low_tone,args,1,1);
    }
    return reloq_frca61_fren_low_tone(state, p0);
}
int32_t reloq_frca61_fren_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_monetary_exp,args,3,3);
    }
    return reloq_frca61_fren_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_non_phrasal_comma,args,2,2);
    }
    return reloq_frca61_fren_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_frca61_fren_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_frca61_fren_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_frca61_fren_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_frca61_fren_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_frca61_fren_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_nucl_high_tone,args,1,1);
    }
    return reloq_frca61_fren_nucl_high_tone(state, p0);
}
int32_t reloq_frca61_fren_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_frca61_fren_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_frca61_fren_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_frca61_fren_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_frca61_fren_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_nucl_low_tone,args,1,1);
    }
    return reloq_frca61_fren_nucl_low_tone(state, p0);
}
int32_t reloq_frca61_fren_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_pal_Fv,args,1,1);
    }
    return reloq_frca61_fren_pal_Fv(state, p0);
}
int32_t reloq_frca61_fren_ph_H(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_H(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_H,args,1,1);
    }
    return reloq_frca61_fren_ph_H(state, p0);
}
int32_t reloq_frca61_fren_ph_H_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_H_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_H_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_H_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_S(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_S,args,1,1);
    }
    return reloq_frca61_fren_ph_S(state, p0);
}
int32_t reloq_frca61_fren_ph_S_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_S_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_S_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_S_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_Z(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_Z,args,1,1);
    }
    return reloq_frca61_fren_ph_Z(state, p0);
}
int32_t reloq_frca61_fren_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_Z_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_Z_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_b(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_b,args,1,1);
    }
    return reloq_frca61_fren_ph_b(state, p0);
}
int32_t reloq_frca61_fren_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_b_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_b_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_d(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_d,args,1,1);
    }
    return reloq_frca61_fren_ph_d(state, p0);
}
int32_t reloq_frca61_fren_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_d_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_d_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_f(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_f(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_f,args,1,1);
    }
    return reloq_frca61_fren_ph_f(state, p0);
}
int32_t reloq_frca61_fren_ph_f_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_f_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_f_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_f_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_g(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_g,args,1,1);
    }
    return reloq_frca61_fren_ph_g(state, p0);
}
int32_t reloq_frca61_fren_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_g_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_g_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_j(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_j(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_j,args,1,1);
    }
    return reloq_frca61_fren_ph_j(state, p0);
}
int32_t reloq_frca61_fren_ph_j_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_j_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_j_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_j_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_j_nuc(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_j_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_j_nuc,args,1,1);
    }
    return reloq_frca61_fren_ph_j_nuc(state, p0);
}
int32_t reloq_frca61_fren_ph_k(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_k,args,1,1);
    }
    return reloq_frca61_fren_ph_k(state, p0);
}
int32_t reloq_frca61_fren_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_k_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_k_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_l(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_l,args,1,1);
    }
    return reloq_frca61_fren_ph_l(state, p0);
}
int32_t reloq_frca61_fren_ph_l_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_l_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_l_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_l_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_m(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_m,args,1,1);
    }
    return reloq_frca61_fren_ph_m(state, p0);
}
int32_t reloq_frca61_fren_ph_m_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_m_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_m_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_m_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_n(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_n,args,1,1);
    }
    return reloq_frca61_fren_ph_n(state, p0);
}
int32_t reloq_frca61_fren_ph_n_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_n_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_n_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_o(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_o,args,1,1);
    }
    return reloq_frca61_fren_ph_o(state, p0);
}
int32_t reloq_frca61_fren_ph_p(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_p,args,1,1);
    }
    return reloq_frca61_fren_ph_p(state, p0);
}
int32_t reloq_frca61_fren_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_p_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_p_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_pal_n(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_pal_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_pal_n,args,1,1);
    }
    return reloq_frca61_fren_ph_pal_n(state, p0);
}
int32_t reloq_frca61_fren_ph_pal_n_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_pal_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_pal_n_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_pal_n_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_r(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_r,args,1,1);
    }
    return reloq_frca61_fren_ph_r(state, p0);
}
int32_t reloq_frca61_fren_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_r_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_r_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_s(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_s,args,1,1);
    }
    return reloq_frca61_fren_ph_s(state, p0);
}
int32_t reloq_frca61_fren_ph_s_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_s_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_s_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_s_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_t(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_t,args,1,1);
    }
    return reloq_frca61_fren_ph_t(state, p0);
}
int32_t reloq_frca61_fren_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_t_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_t_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_u(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_u,args,1,1);
    }
    return reloq_frca61_fren_ph_u(state, p0);
}
int32_t reloq_frca61_fren_ph_v(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_v(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_v,args,1,1);
    }
    return reloq_frca61_fren_ph_v(state, p0);
}
int32_t reloq_frca61_fren_ph_v_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_v_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_v_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_v_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_vel_n(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_vel_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_vel_n,args,1,1);
    }
    return reloq_frca61_fren_ph_vel_n(state, p0);
}
int32_t reloq_frca61_fren_ph_vel_n_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_vel_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_vel_n_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_vel_n_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_w(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_w,args,1,1);
    }
    return reloq_frca61_fren_ph_w(state, p0);
}
int32_t reloq_frca61_fren_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_w_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_w_dur(state, p0);
}
int32_t reloq_frca61_fren_ph_x(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_x(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_x,args,1,1);
    }
    return reloq_frca61_fren_ph_x(state, p0);
}
int32_t reloq_frca61_fren_ph_y(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_y,args,1,1);
    }
    return reloq_frca61_fren_ph_y(state, p0);
}
int32_t reloq_frca61_fren_ph_z(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_z,args,1,1);
    }
    return reloq_frca61_fren_ph_z(state, p0);
}
int32_t reloq_frca61_fren_ph_z_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_ph_z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_ph_z_dur,args,1,1);
    }
    return reloq_frca61_fren_ph_z_dur(state, p0);
}
int32_t reloq_frca61_fren_phnol_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_phnol_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_phnol_rules,args,3,3);
    }
    return reloq_frca61_fren_phnol_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_phr_final_wd_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_phr_final_wd_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_phr_final_wd_stress,args,4,4);
    }
    return reloq_frca61_fren_phr_final_wd_stress(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_fren_phr_init_wd_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_phr_init_wd_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_phr_init_wd_stress,args,4,4);
    }
    return reloq_frca61_fren_phr_init_wd_stress(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_fren_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_phr_initial_F0,args,1,1);
    }
    return reloq_frca61_fren_phr_initial_F0(state, p0);
}
int32_t reloq_frca61_fren_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_phrase_level_rules,args,3,3);
    }
    return reloq_frca61_fren_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_phrase_tone,args,1,1);
    }
    return reloq_frca61_fren_phrase_tone(state, p0);
}
int32_t reloq_frca61_fren_pnames(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_pnames(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_pnames,args,3,3);
    }
    return reloq_frca61_fren_pnames(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_position_t_ptrs,args,1,1);
    }
    return reloq_frca61_fren_position_t_ptrs(state, p0);
}
int32_t reloq_frca61_fren_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_post_comma_phrase,args,3,3);
    }
    return reloq_frca61_fren_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_postnucl_f0,args,1,1);
    }
    return reloq_frca61_fren_postnucl_f0(state, p0);
}
int32_t reloq_frca61_fren_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_prenucl_f0,args,1,1);
    }
    return reloq_frca61_fren_prenucl_f0(state, p0);
}
int32_t reloq_frca61_fren_proclitic(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_proclitic(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_proclitic,args,3,3);
    }
    return reloq_frca61_fren_proclitic(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_roots(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_roots(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_roots,args,3,3);
    }
    return reloq_frca61_fren_roots(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_roots_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_roots_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_roots_dict_lookup,args,3,3);
    }
    return reloq_frca61_fren_roots_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_said_phrase,args,4,4);
    }
    return reloq_frca61_fren_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_fren_symbolic_rules(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_symbolic_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_symbolic_rules,args,1,1);
    }
    return reloq_frca61_fren_symbolic_rules(state, p0);
}
int32_t reloq_frca61_fren_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_tok_dict,args,3,3);
    }
    return reloq_frca61_fren_tok_dict(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_trans_features(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_trans_features,args,1,1);
    }
    return reloq_frca61_fren_trans_features(state, p0);
}
int32_t reloq_frca61_fren_velar_Fv(void *, int32_t);
static inline int32_t reloq_call_frca61_fren_velar_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_fren_velar_Fv,args,1,1);
    }
    return reloq_frca61_fren_velar_Fv(state, p0);
}
int32_t reloq_frca61_fren_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_words,args,3,3);
    }
    return reloq_frca61_fren_words(state, p0, p1, p2);
}
int32_t reloq_frca61_fren_words_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_fren_words_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_fren_words_dict_lookup,args,3,3);
    }
    return reloq_frca61_fren_words_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_frca61_g_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_g_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_g_rules,args,3,3);
    }
    return reloq_frca61_g_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_frca61_generate_acoustic_vals(state, p0);
}
int32_t reloq_frca61_generate_diaphones(void *, int32_t);
static inline int32_t reloq_call_frca61_generate_diaphones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_diaphones,args,1,1);
    }
    return reloq_frca61_generate_diaphones(state, p0);
}
int32_t reloq_frca61_generate_durations(void *, int32_t);
static inline int32_t reloq_call_frca61_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_frca61_generate_durations(state, p0);
}
int32_t reloq_frca61_generate_intonation(void *, int32_t);
static inline int32_t reloq_call_frca61_generate_intonation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_intonation,args,1,1);
    }
    return reloq_frca61_generate_intonation(state, p0);
}
int32_t reloq_frca61_get_canfren_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_frca61_get_canfren_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_canfren_nuc_AV,args,1,1);
    }
    return reloq_frca61_get_canfren_nuc_AV(state, p0);
}
int32_t reloq_frca61_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_frca61_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_frca61_get_fren_fric_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_fric_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_fric_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_fric_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_fren_l_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_l_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_l_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_l_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_fren_nas_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_nas_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_nas_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_nas_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_fren_nasal_glide_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_nasal_glide_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_nasal_glide_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_nasal_glide_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_fren_r_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_r_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_r_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_r_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_fren_son_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_son_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_son_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_son_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_fren_stop_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_stop_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_stop_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_stop_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_fren_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_fren_vow_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_fren_vow_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_fren_vow_trans_dur,args,2,2);
    }
    return reloq_frca61_get_fren_vow_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_frca61_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_get_initial_f0_value(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_initial_f0_value(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_initial_f0_value,args,2,2);
    }
    return reloq_frca61_get_initial_f0_value(state, p0, p1);
}
int32_t reloq_frca61_get_input(void *, int32_t);
static inline int32_t reloq_call_frca61_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_frca61_get_input(state, p0);
}
int32_t reloq_frca61_get_nonpickup_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_nonpickup_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nonpickup_word,args,5,5);
    }
    return reloq_frca61_get_nonpickup_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_frca61_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_frca61_get_nuc_AV(state, p0);
}
int32_t reloq_frca61_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_frca61_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_frca61_get_number_from_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_number_from_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_from_inp,args,3,3);
    }
    return reloq_frca61_get_number_from_inp(state, p0, p1, p2);
}
int32_t reloq_frca61_get_number_from_verb_ending(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_number_from_verb_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_from_verb_ending,args,4,4);
    }
    return reloq_frca61_get_number_from_verb_ending(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_get_number_info(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_number_info(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_info,args,3,3);
    }
    return reloq_frca61_get_number_info(state, p0, p1, p2);
}
int32_t reloq_frca61_get_parfren_nuc_AV(void *, int32_t);
int32_t reloq_frca61_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_frca61_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_stanfren_non_vowels(void *, int32_t);
static inline int32_t reloq_call_frca61_get_stanfren_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanfren_non_vowels,args,1,1);
    }
    return reloq_frca61_get_stanfren_non_vowels(state, p0);
}
int32_t reloq_frca61_get_stanfren_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_frca61_get_stanfren_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanfren_nuc_AV,args,1,1);
    }
    return reloq_frca61_get_stanfren_nuc_AV(state, p0);
}
int32_t reloq_frca61_get_tok(void *, int32_t);
static inline int32_t reloq_call_frca61_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_frca61_get_tok(state, p0);
}
int32_t reloq_frca61_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_frca61_get_trans_dur(state, p0, p1);
}
int32_t reloq_frca61_get_vow_AV(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_vow_AV(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_vow_AV,args,3,3);
    }
    return reloq_frca61_get_vow_AV(state, p0, p1, p2);
}
int32_t reloq_frca61_get_word_after_2_clitics(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_get_word_after_2_clitics(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_word_after_2_clitics,args,2,2);
    }
    return reloq_frca61_get_word_after_2_clitics(state, p0, p1);
}
int32_t reloq_frca61_gettok(void *, int32_t);
static inline int32_t reloq_call_frca61_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_frca61_gettok(state, p0);
}
int32_t reloq_frca61_gramm_cat_dict_lookup(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_gramm_cat_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_gramm_cat_dict_lookup,args,5,5);
    }
    return reloq_frca61_gramm_cat_dict_lookup(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_h_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_h_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_h_rules,args,3,3);
    }
    return reloq_frca61_h_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_frca61_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_frca61_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_frca61_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_frca61_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_frca61_handle_non_phrasal_commas(void *, int32_t);
static inline int32_t reloq_call_frca61_handle_non_phrasal_commas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_phrasal_commas,args,1,1);
    }
    return reloq_frca61_handle_non_phrasal_commas(state, p0);
}
int32_t reloq_frca61_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_frca61_handle_pause(state, p0, p1);
}
int32_t reloq_frca61_has_unambig_verb_ending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_has_unambig_verb_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_has_unambig_verb_ending,args,5,5);
    }
    return reloq_frca61_has_unambig_verb_ending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_low_star_tone,args,1,1);
    }
    return reloq_frca61_high_low_star_tone(state, p0);
}
int32_t reloq_frca61_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_star_low_tone,args,1,1);
    }
    return reloq_frca61_high_star_low_tone(state, p0);
}
int32_t reloq_frca61_high_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_tone,args,1,1);
    }
    return reloq_frca61_high_tone(state, p0);
}
int32_t reloq_frca61_high_vowel_av(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_high_vowel_av(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_high_vowel_av,args,3,3);
    }
    return reloq_frca61_high_vowel_av(state, p0, p1, p2);
}
int32_t reloq_frca61_high_vowel_deletion(void *, int32_t);
static inline int32_t reloq_call_frca61_high_vowel_deletion(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_vowel_deletion,args,1,1);
    }
    return reloq_frca61_high_vowel_deletion(state, p0);
}
int32_t reloq_frca61_high_vowel_gliding(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_high_vowel_gliding(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_high_vowel_gliding,args,3,3);
    }
    return reloq_frca61_high_vowel_gliding(state, p0, p1, p2);
}
int32_t reloq_frca61_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen,args,3,3);
    }
    return reloq_frca61_hyphen(state, p0, p1, p2);
}
int32_t reloq_frca61_hyphen_between_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen_between_nums,args,3,3);
    }
    return reloq_frca61_hyphen_between_nums(state, p0, p1, p2);
}
int32_t reloq_frca61_i_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_i_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_i_rules,args,3,3);
    }
    return reloq_frca61_i_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_frca61_in_brackets(state, p0, p1, p2);
}
int32_t reloq_frca61_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_frca61_in_quotes(state, p0, p1, p2);
}
int32_t reloq_frca61_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_in_syllable(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_syllable,args,4,4);
    }
    return reloq_frca61_in_syllable(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_frca61_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_frca61_increment_output_file_number(state, p0);
}
int32_t reloq_frca61_infinitive_ending(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_infinitive_ending(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_infinitive_ending,args,3,3);
    }
    return reloq_frca61_infinitive_ending(state, p0, p1, p2);
}
int32_t reloq_frca61_init_adjacent_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_frca61_init_adjacent_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_adjacent_word_ptrs,args,1,1);
    }
    return reloq_frca61_init_adjacent_word_ptrs(state, p0);
}
int32_t reloq_frca61_init_current_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_frca61_init_current_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_current_word_ptrs,args,1,1);
    }
    return reloq_frca61_init_current_word_ptrs(state, p0);
}
int32_t reloq_frca61_init_delta(void *, int32_t);
static inline int32_t reloq_call_frca61_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_frca61_init_delta(state, p0);
}
int32_t reloq_frca61_init_fren_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_fren_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_fren_syll_inton_vars,args,1,1);
    }
    return reloq_frca61_init_fren_syll_inton_vars(state, p0);
}
int32_t reloq_frca61_init_fren_tm_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_fren_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_fren_tm_vars,args,1,1);
    }
    return reloq_frca61_init_fren_tm_vars(state, p0);
}
int32_t reloq_frca61_init_fren_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_fren_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_fren_vars,args,1,1);
    }
    return reloq_frca61_init_fren_vars(state, p0);
}
int32_t reloq_frca61_init_fren_word_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_fren_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_fren_word_vars,args,1,1);
    }
    return reloq_frca61_init_fren_word_vars(state, p0);
}
int32_t reloq_frca61_init_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_gcat_vars,args,1,1);
    }
    return reloq_frca61_init_gcat_vars(state, p0);
}
int32_t reloq_frca61_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_frca61_init_lang_vars(state, p0);
}
int32_t reloq_frca61_init_nuclwrd_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_nuclwrd_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_nuclwrd_vars,args,1,1);
    }
    return reloq_frca61_init_nuclwrd_vars(state, p0);
}
int32_t reloq_frca61_init_phnol_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_phnol_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phnol_vars,args,1,1);
    }
    return reloq_frca61_init_phnol_vars(state, p0);
}
int32_t reloq_frca61_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_frca61_init_phr_feature_vars(state, p0);
}
int32_t reloq_frca61_init_phrase_inton_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_phrase_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_inton_vars,args,1,1);
    }
    return reloq_frca61_init_phrase_inton_vars(state, p0);
}
int32_t reloq_frca61_init_phrase_level_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_phrase_level_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_level_vars,args,1,1);
    }
    return reloq_frca61_init_phrase_level_vars(state, p0);
}
int32_t reloq_frca61_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_frca61_init_phrase_vars(state, p0);
}
int32_t reloq_frca61_init_platform(void *, int32_t);
static inline int32_t reloq_call_frca61_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_frca61_init_platform(state, p0);
}
int32_t reloq_frca61_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_frca61_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_frca61_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_frca61_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_frca61_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_inton_vars,args,1,1);
    }
    return reloq_frca61_init_syll_inton_vars(state, p0);
}
int32_t reloq_frca61_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_frca61_init_syll_vars(state, p0);
}
int32_t reloq_frca61_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_frca61_init_symbolic_vars(state, p0);
}
int32_t reloq_frca61_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_frca61_init_tm_vars(state, p0);
}
int32_t reloq_frca61_init_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_frca61_init_vars(state, p0);
}
int32_t reloq_frca61_init_voice_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_voice_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_voice_vars,args,1,1);
    }
    return reloq_frca61_init_voice_vars(state, p0);
}
int32_t reloq_frca61_init_word(void *, int32_t);
static inline int32_t reloq_call_frca61_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_frca61_init_word(state, p0);
}
int32_t reloq_frca61_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_inton_vars,args,1,1);
    }
    return reloq_frca61_init_word_inton_vars(state, p0);
}
int32_t reloq_frca61_init_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_frca61_init_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_ptrs,args,1,1);
    }
    return reloq_frca61_init_word_ptrs(state, p0);
}
int32_t reloq_frca61_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_frca61_init_word_vars(state, p0);
}
int32_t reloq_frca61_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_frca61_insert_acoustic_vals(state, p0);
}
int32_t reloq_frca61_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_frca61_insert_burst_dur(state, p0);
}
int32_t reloq_frca61_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_frca61_insert_char_count(state, p0);
}
int32_t reloq_frca61_insert_clitic_vowel(void *, int32_t);
int32_t reloq_frca61_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_comma_phrase,args,3,3);
    }
    return reloq_frca61_insert_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_dict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_dict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dict_root,args,3,3);
    }
    return reloq_frca61_insert_dict_root(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_frca61_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_insert_f0(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_f0(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_f0,args,4,4);
    }
    return reloq_frca61_insert_f0(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_insert_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_insert_fren_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_fren_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_fren_spr_phone,args,3,3);
    }
    return reloq_frca61_insert_fren_spr_phone(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_insert_lax_vowel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_lax_vowel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_lax_vowel,args,3,3);
    }
    return reloq_frca61_insert_lax_vowel(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_long_vowel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_long_vowel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_long_vowel,args,3,3);
    }
    return reloq_frca61_insert_long_vowel(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_frca61_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_nasal_glide(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_nasal_glide(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_nasal_glide,args,2,2);
    }
    return reloq_frca61_insert_nasal_glide(state, p0, p1);
}
int32_t reloq_frca61_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_non_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_non_phrase_pause,args,3,3);
    }
    return reloq_frca61_insert_non_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_frca61_insert_pause_durs(state, p0);
}
int32_t reloq_frca61_insert_penult_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_frca61_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_final_tones,args,1,1);
    }
    return reloq_frca61_insert_phrase_final_tones(state, p0);
}
int32_t reloq_frca61_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_frca61_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_preset_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_preset_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_preset_phrase_tone,args,1,1);
    }
    return reloq_frca61_insert_preset_phrase_tone(state, p0);
}
int32_t reloq_frca61_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_frca61_insert_space(state, p0, p1);
}
int32_t reloq_frca61_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_frca61_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_frca61_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_insert_suffix(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_suffix(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_suffix,args,2,2);
    }
    return reloq_frca61_insert_suffix(state, p0, p1);
}
int32_t reloq_frca61_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_frca61_insert_syll_level_vals(state, p0);
}
int32_t reloq_frca61_insert_target_t3(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_target_t3(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_target_t3,args,1,1);
    }
    return reloq_frca61_insert_target_t3(state, p0);
}
int32_t reloq_frca61_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_frca61_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_frca61_insert_transitions(state, p0);
}
int32_t reloq_frca61_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_frca61_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrases,args,4,4);
    }
    return reloq_frca61_insert_unpuncted_phrases(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_frca61_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_frca61_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_frca61_insert_word(state, p0, p1, p2);
}
int32_t reloq_frca61_insert_zero_sylls(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_insert_zero_sylls(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_zero_sylls,args,3,3);
    }
    return reloq_frca61_insert_zero_sylls(state, p0, p1, p2);
}
int32_t reloq_frca61_inside_orig_inp_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_inside_orig_inp_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_inside_orig_inp_token,args,2,2);
    }
    return reloq_frca61_inside_orig_inp_token(state, p0, p1);
}
int32_t reloq_frca61_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_frca61_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_frca61_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_interpret_single_char_modes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_interpret_single_char_modes,args,3,3);
    }
    return reloq_frca61_interpret_single_char_modes(state, p0, p1, p2);
}
int32_t reloq_frca61_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_frca61_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_frca61_is_2prev_syll_accented(void *, int32_t);
static inline int32_t reloq_call_frca61_is_2prev_syll_accented(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_2prev_syll_accented,args,1,1);
    }
    return reloq_frca61_is_2prev_syll_accented(state, p0);
}
int32_t reloq_frca61_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_frca61_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_frca61_is_beginning_of_long_NP(void *, int32_t, int32_t);
int32_t reloq_frca61_is_cleft_focus(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_cleft_focus(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_cleft_focus,args,3,3);
    }
    return reloq_frca61_is_cleft_focus(state, p0, p1, p2);
}
int32_t reloq_frca61_is_compound_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_compound_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_compound_word,args,3,3);
    }
    return reloq_frca61_is_compound_word(state, p0, p1, p2);
}
int32_t reloq_frca61_is_emphatic_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_emphatic_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_emphatic_word,args,2,2);
    }
    return reloq_frca61_is_emphatic_word(state, p0, p1);
}
int32_t reloq_frca61_is_fixed_tone_on_this_syllable(void *, int32_t);
static inline int32_t reloq_call_frca61_is_fixed_tone_on_this_syllable(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_fixed_tone_on_this_syllable,args,1,1);
    }
    return reloq_frca61_is_fixed_tone_on_this_syllable(state, p0);
}
int32_t reloq_frca61_is_floating_high_on_current_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_floating_high_on_current_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_floating_high_on_current_word,args,3,3);
    }
    return reloq_frca61_is_floating_high_on_current_word(state, p0, p1, p2);
}
int32_t reloq_frca61_is_floating_tone_on_this_syllable(void *, int32_t);
static inline int32_t reloq_call_frca61_is_floating_tone_on_this_syllable(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_floating_tone_on_this_syllable,args,1,1);
    }
    return reloq_frca61_is_floating_tone_on_this_syllable(state, p0);
}
int32_t reloq_frca61_is_foll_syll_accented(void *, int32_t);
static inline int32_t reloq_call_frca61_is_foll_syll_accented(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_foll_syll_accented,args,1,1);
    }
    return reloq_frca61_is_foll_syll_accented(state, p0);
}
int32_t reloq_frca61_is_following_NP_within_NP(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_following_NP_within_NP(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_following_NP_within_NP,args,2,2);
    }
    return reloq_frca61_is_following_NP_within_NP(state, p0, p1);
}
int32_t reloq_frca61_is_fren_number(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_is_french_target_syll(void *, int32_t);
static inline int32_t reloq_call_frca61_is_french_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_french_target_syll,args,1,1);
    }
    return reloq_frca61_is_french_target_syll(state, p0);
}
int32_t reloq_frca61_is_hyphenated_word(void *, int32_t, int32_t);
int32_t reloq_frca61_is_in_middle_of_long_NP(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_in_middle_of_long_NP(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_in_middle_of_long_NP,args,2,2);
    }
    return reloq_frca61_is_in_middle_of_long_NP(state, p0, p1);
}
int32_t reloq_frca61_is_in_parentheses(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_in_parentheses(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_in_parentheses,args,3,3);
    }
    return reloq_frca61_is_in_parentheses(state, p0, p1, p2);
}
int32_t reloq_frca61_is_infinitive_from_context(void *, int32_t);
static inline int32_t reloq_call_frca61_is_infinitive_from_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_infinitive_from_context,args,1,1);
    }
    return reloq_frca61_is_infinitive_from_context(state, p0);
}
int32_t reloq_frca61_is_infl_or_pastpart(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_is_liaison_con(void *, int32_t);
static inline int32_t reloq_call_frca61_is_liaison_con(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_liaison_con,args,1,1);
    }
    return reloq_frca61_is_liaison_con(state, p0);
}
int32_t reloq_frca61_is_low_rising_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_low_rising_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_low_rising_comma_phrase,args,3,3);
    }
    return reloq_frca61_is_low_rising_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_is_member_of_list(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_member_of_list(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_member_of_list,args,2,2);
    }
    return reloq_frca61_is_member_of_list(state, p0, p1);
}
int32_t reloq_frca61_is_minor_phrase_break(void *, int32_t);
static inline int32_t reloq_call_frca61_is_minor_phrase_break(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_minor_phrase_break,args,1,1);
    }
    return reloq_frca61_is_minor_phrase_break(state, p0);
}
int32_t reloq_frca61_is_minor_phrase_edge(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_minor_phrase_edge(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_minor_phrase_edge,args,2,2);
    }
    return reloq_frca61_is_minor_phrase_edge(state, p0, p1);
}
int32_t reloq_frca61_is_monosyllable(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_monosyllable(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_monosyllable,args,3,3);
    }
    return reloq_frca61_is_monosyllable(state, p0, p1, p2);
}
int32_t reloq_frca61_is_nasal_vow(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_nasal_vow(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_nasal_vow,args,2,2);
    }
    return reloq_frca61_is_nasal_vow(state, p0, p1);
}
int32_t reloq_frca61_is_noun_or_adj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_noun_or_adj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_noun_or_adj,args,3,3);
    }
    return reloq_frca61_is_noun_or_adj(state, p0, p1, p2);
}
int32_t reloq_frca61_is_nounadj_from_context(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_nounadj_from_context(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_nounadj_from_context,args,2,2);
    }
    return reloq_frca61_is_nounadj_from_context(state, p0, p1);
}
int32_t reloq_frca61_is_one_syll(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_one_syll(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_one_syll,args,3,3);
    }
    return reloq_frca61_is_one_syll(state, p0, p1, p2);
}
int32_t reloq_frca61_is_past_participle(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_is_pathname(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_pathname(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pathname,args,2,2);
    }
    return reloq_frca61_is_pathname(state, p0, p1);
}
int32_t reloq_frca61_is_prenom_modifier(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_prenom_modifier(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_prenom_modifier,args,2,2);
    }
    return reloq_frca61_is_prenom_modifier(state, p0, p1);
}
int32_t reloq_frca61_is_prev_syll_accented(void *, int32_t);
static inline int32_t reloq_call_frca61_is_prev_syll_accented(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_prev_syll_accented,args,1,1);
    }
    return reloq_frca61_is_prev_syll_accented(state, p0);
}
int32_t reloq_frca61_is_previous_NP_within_NP(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_previous_NP_within_NP(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_previous_NP_within_NP,args,2,2);
    }
    return reloq_frca61_is_previous_NP_within_NP(state, p0, p1);
}
int32_t reloq_frca61_is_proper_name(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_proper_name(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_proper_name,args,2,2);
    }
    return reloq_frca61_is_proper_name(state, p0, p1);
}
int32_t reloq_frca61_is_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_single_digit,args,3,3);
    }
    return reloq_frca61_is_single_digit(state, p0, p1, p2);
}
int32_t reloq_frca61_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_frca61_is_spr(state, p0, p1, p2);
}
int32_t reloq_frca61_is_t_pron_s_in_ti(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_t_pron_s_in_ti(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_t_pron_s_in_ti,args,4,4);
    }
    return reloq_frca61_is_t_pron_s_in_ti(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_is_target_syll(void *, int32_t);
static inline int32_t reloq_call_frca61_is_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_target_syll,args,1,1);
    }
    return reloq_frca61_is_target_syll(state, p0);
}
int32_t reloq_frca61_is_verb_from_context(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_verb_from_context(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_verb_from_context,args,2,2);
    }
    return reloq_frca61_is_verb_from_context(state, p0, p1);
}
int32_t reloq_frca61_is_weak_accented_funct_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_weak_accented_funct_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_weak_accented_funct_word,args,2,2);
    }
    return reloq_frca61_is_weak_accented_funct_word(state, p0, p1);
}
int32_t reloq_frca61_is_weak_initial_accent(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_weak_initial_accent(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_weak_initial_accent,args,3,3);
    }
    return reloq_frca61_is_weak_initial_accent(state, p0, p1, p2);
}
int32_t reloq_frca61_is_weak_number_accent(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_weak_number_accent(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_weak_number_accent,args,2,2);
    }
    return reloq_frca61_is_weak_number_accent(state, p0, p1);
}
int32_t reloq_frca61_is_weak_verb(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_is_weak_verb(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_weak_verb,args,2,2);
    }
    return reloq_frca61_is_weak_verb(state, p0, p1);
}
int32_t reloq_frca61_isitAlt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_isitAlt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitAlt,args,3,3);
    }
    return reloq_frca61_isitAlt(state, p0, p1, p2);
}
int32_t reloq_frca61_isitWH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_isitWH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitWH,args,3,3);
    }
    return reloq_frca61_isitWH(state, p0, p1, p2);
}
int32_t reloq_frca61_isit_fren_Alt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_isit_fren_Alt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_fren_Alt,args,3,3);
    }
    return reloq_frca61_isit_fren_Alt(state, p0, p1, p2);
}
int32_t reloq_frca61_isit_fren_WH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_isit_fren_WH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_fren_WH,args,3,3);
    }
    return reloq_frca61_isit_fren_WH(state, p0, p1, p2);
}
int32_t reloq_frca61_j_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_j_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_j_rules,args,3,3);
    }
    return reloq_frca61_j_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_k_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_k_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_k_rules,args,3,3);
    }
    return reloq_frca61_k_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_l_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_l_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_l_rules,args,3,3);
    }
    return reloq_frca61_l_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_lang_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_lang_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_hyphen_between_nums,args,5,5);
    }
    return reloq_frca61_lang_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_lang_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_lang_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_non_phrasal_comma,args,2,2);
    }
    return reloq_frca61_lang_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_frca61_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_frca61_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_frca61_le_de_vow_elision(void *, int32_t);
static inline int32_t reloq_call_frca61_le_de_vow_elision(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_le_de_vow_elision,args,1,1);
    }
    return reloq_frca61_le_de_vow_elision(state, p0);
}
int32_t reloq_frca61_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_frca61_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_frca61_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_frca61_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_frca61_liaison_expressions(void *, int32_t);
static inline int32_t reloq_call_frca61_liaison_expressions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_liaison_expressions,args,1,1);
    }
    return reloq_frca61_liaison_expressions(state, p0);
}
int32_t reloq_frca61_liaison_gcat_context(void *, int32_t);
static inline int32_t reloq_call_frca61_liaison_gcat_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_liaison_gcat_context,args,1,1);
    }
    return reloq_frca61_liaison_gcat_context(state, p0);
}
int32_t reloq_frca61_liaison_phone_context(void *, int32_t);
static inline int32_t reloq_call_frca61_liaison_phone_context(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_liaison_phone_context,args,1,1);
    }
    return reloq_frca61_liaison_phone_context(state, p0);
}
int32_t reloq_frca61_liaison_rules(void *, int32_t);
static inline int32_t reloq_call_frca61_liaison_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_liaison_rules,args,1,1);
    }
    return reloq_frca61_liaison_rules(state, p0);
}
int32_t reloq_frca61_longdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_longdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_longdash,args,3,3);
    }
    return reloq_frca61_longdash(state, p0, p1, p2);
}
int32_t reloq_frca61_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_high_star_tone,args,1,1);
    }
    return reloq_frca61_low_high_star_tone(state, p0);
}
int32_t reloq_frca61_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_star_high_tone,args,1,1);
    }
    return reloq_frca61_low_star_high_tone(state, p0);
}
int32_t reloq_frca61_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_tone,args,1,1);
    }
    return reloq_frca61_low_tone(state, p0);
}
int32_t reloq_frca61_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_m_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_m_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_m_rules,args,3,3);
    }
    return reloq_frca61_m_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_make_canfren_word_stress_adjustments(void *, int32_t);
static inline int32_t reloq_call_frca61_make_canfren_word_stress_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_make_canfren_word_stress_adjustments,args,1,1);
    }
    return reloq_frca61_make_canfren_word_stress_adjustments(state, p0);
}
int32_t reloq_frca61_make_canfren_word_syll_adjustments(void *, int32_t);
static inline int32_t reloq_call_frca61_make_canfren_word_syll_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_make_canfren_word_syll_adjustments,args,1,1);
    }
    return reloq_frca61_make_canfren_word_syll_adjustments(state, p0);
}
int32_t reloq_frca61_make_syllable_stress_adjustments(void *, int32_t);
static inline int32_t reloq_call_frca61_make_syllable_stress_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_make_syllable_stress_adjustments,args,1,1);
    }
    return reloq_frca61_make_syllable_stress_adjustments(state, p0);
}
int32_t reloq_frca61_mark_acc_phr(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_acc_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_acc_phr,args,7,7);
    }
    return reloq_frca61_mark_acc_phr(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_frca61_mark_acc_phr_pos(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_acc_phr_pos(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_acc_phr_pos,args,4,4);
    }
    return reloq_frca61_mark_acc_phr_pos(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_acc_phr_strength(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_acc_phr_strength(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_acc_phr_strength,args,4,4);
    }
    return reloq_frca61_mark_acc_phr_strength(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_adverbs_by_rule(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_adverbs_by_rule(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_adverbs_by_rule,args,3,3);
    }
    return reloq_frca61_mark_adverbs_by_rule(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_adverbs_from_sets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_adverbs_from_sets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_adverbs_from_sets,args,3,3);
    }
    return reloq_frca61_mark_adverbs_from_sets(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_ant_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_ant_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_ant_words,args,3,3);
    }
    return reloq_frca61_mark_ant_words(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_cats_on_compounds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_cats_on_compounds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_cats_on_compounds,args,3,3);
    }
    return reloq_frca61_mark_cats_on_compounds(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_compound_nouns(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_compound_nouns(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_compound_nouns,args,3,3);
    }
    return reloq_frca61_mark_compound_nouns(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_content_word,args,5,5);
    }
    return reloq_frca61_mark_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_mark_ent_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_ent_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_ent_words,args,3,3);
    }
    return reloq_frca61_mark_ent_words(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_er_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_er_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_er_words,args,3,3);
    }
    return reloq_frca61_mark_er_words(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_floating_tone(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_floating_tone(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_floating_tone,args,4,4);
    }
    return reloq_frca61_mark_floating_tone(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_fren_conjunctions(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_fren_conjunctions(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fren_conjunctions,args,4,4);
    }
    return reloq_frca61_mark_fren_conjunctions(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_fren_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_fren_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fren_content_word,args,3,3);
    }
    return reloq_frca61_mark_fren_content_word(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_fren_demonstratives(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_fren_demonstratives(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fren_demonstratives,args,3,3);
    }
    return reloq_frca61_mark_fren_demonstratives(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_fren_prepositions(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_fren_prepositions(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fren_prepositions,args,4,4);
    }
    return reloq_frca61_mark_fren_prepositions(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_fren_pronouns(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_fren_pronouns(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fren_pronouns,args,4,4);
    }
    return reloq_frca61_mark_fren_pronouns(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_fren_relative_pronouns(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_fren_relative_pronouns(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fren_relative_pronouns,args,4,4);
    }
    return reloq_frca61_mark_fren_relative_pronouns(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_fren_subcategory(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_fren_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fren_subcategory,args,3,3);
    }
    return reloq_frca61_mark_fren_subcategory(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_fren_wh_adjectives(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_fren_wh_adjectives(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_fren_wh_adjectives,args,3,3);
    }
    return reloq_frca61_mark_fren_wh_adjectives(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_function_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_function_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_function_word,args,5,5);
    }
    return reloq_frca61_mark_function_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_mark_high_vowel_deletion(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_high_vowel_deletion(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_high_vowel_deletion,args,3,3);
    }
    return reloq_frca61_mark_high_vowel_deletion(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_non_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_non_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_non_letter,args,3,3);
    }
    return reloq_frca61_mark_non_letter(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_nounadj_subcat(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_nounadj_subcat(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nounadj_subcat,args,3,3);
    }
    return reloq_frca61_mark_nounadj_subcat(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_nounadjs_by_rule(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_nounadjs_by_rule(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nounadjs_by_rule,args,3,3);
    }
    return reloq_frca61_mark_nounadjs_by_rule(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_nounadjs_from_sets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_nounadjs_from_sets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nounadjs_from_sets,args,3,3);
    }
    return reloq_frca61_mark_nounadjs_from_sets(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_frca61_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_mark_number_from_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_number_from_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_number_from_inp,args,3,3);
    }
    return reloq_frca61_mark_number_from_inp(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_number_from_var(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_number_from_var(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_number_from_var,args,4,4);
    }
    return reloq_frca61_mark_number_from_var(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_pastparts_by_rule(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_pastparts_by_rule(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_pastparts_by_rule,args,3,3);
    }
    return reloq_frca61_mark_pastparts_by_rule(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_potential_phrase_breaks(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_potential_phrase_breaks(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_potential_phrase_breaks,args,4,4);
    }
    return reloq_frca61_mark_potential_phrase_breaks(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_spr_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_spr_syll_vals,args,6,6);
    }
    return reloq_frca61_mark_spr_syll_vals(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_mark_subcat_nouns_from_sets(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_subcat_nouns_from_sets(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_subcat_nouns_from_sets,args,4,4);
    }
    return reloq_frca61_mark_subcat_nouns_from_sets(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_subcategory(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_subcategory,args,4,4);
    }
    return reloq_frca61_mark_subcategory(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_mark_syll_stress(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_frca61_mark_tion_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_tion_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_tion_words,args,3,3);
    }
    return reloq_frca61_mark_tion_words(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_uppercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_uppercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_uppercase,args,3,3);
    }
    return reloq_frca61_mark_uppercase(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_verbs_by_rule(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_verbs_by_rule(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_verbs_by_rule,args,3,3);
    }
    return reloq_frca61_mark_verbs_by_rule(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_verbs_from_sets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_verbs_from_sets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_verbs_from_sets,args,3,3);
    }
    return reloq_frca61_mark_verbs_from_sets(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_word_categories(void *, int32_t);
static inline int32_t reloq_call_frca61_mark_word_categories(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_categories,args,1,1);
    }
    return reloq_frca61_mark_word_categories(state, p0);
}
int32_t reloq_frca61_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_word_str_annot(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_str_annot,args,3,3);
    }
    return reloq_frca61_mark_word_str_annot(state, p0, p1, p2);
}
int32_t reloq_frca61_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_frca61_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_frca61_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_frca61_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_frca61_mid_vowel_alternations(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_mid_vowel_alternations(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mid_vowel_alternations,args,3,3);
    }
    return reloq_frca61_mid_vowel_alternations(state, p0, p1, p2);
}
int32_t reloq_frca61_minor_phrase_r_edge(void *, int32_t);
static inline int32_t reloq_call_frca61_minor_phrase_r_edge(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_minor_phrase_r_edge,args,1,1);
    }
    return reloq_frca61_minor_phrase_r_edge(state, p0);
}
int32_t reloq_frca61_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_frca61_misc_chars(state, p0, p1, p2);
}
int32_t reloq_frca61_modify_breathiness_for_femV(void *, int32_t);
static inline int32_t reloq_call_frca61_modify_breathiness_for_femV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modify_breathiness_for_femV,args,1,1);
    }
    return reloq_frca61_modify_breathiness_for_femV(state, p0);
}
int32_t reloq_frca61_modulate_burst(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_modulate_burst(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_burst,args,2,2);
    }
    return reloq_frca61_modulate_burst(state, p0, p1);
}
int32_t reloq_frca61_modulate_nas_AV(void *, int32_t);
static inline int32_t reloq_call_frca61_modulate_nas_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_nas_AV,args,1,1);
    }
    return reloq_frca61_modulate_nas_AV(state, p0);
}
int32_t reloq_frca61_modulate_noise(void *, int32_t);
static inline int32_t reloq_call_frca61_modulate_noise(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_noise,args,1,1);
    }
    return reloq_frca61_modulate_noise(state, p0);
}
int32_t reloq_frca61_modulate_pwind(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_modulate_pwind(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_pwind,args,2,2);
    }
    return reloq_frca61_modulate_pwind(state, p0, p1);
}
int32_t reloq_frca61_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_frca61_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_monetary_exp,args,3,3);
    }
    return reloq_frca61_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_frca61_move_past_nounadj_pl(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_move_past_nounadj_pl(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_move_past_nounadj_pl,args,3,3);
    }
    return reloq_frca61_move_past_nounadj_pl(state, p0, p1, p2);
}
int32_t reloq_frca61_move_t1_begin_prev_syllable(void *, int32_t);
static inline int32_t reloq_call_frca61_move_t1_begin_prev_syllable(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_move_t1_begin_prev_syllable,args,1,1);
    }
    return reloq_frca61_move_t1_begin_prev_syllable(state, p0);
}
int32_t reloq_frca61_move_t1_end_prev_syllable(void *, int32_t);
static inline int32_t reloq_call_frca61_move_t1_end_prev_syllable(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_move_t1_end_prev_syllable,args,1,1);
    }
    return reloq_frca61_move_t1_end_prev_syllable(state, p0);
}
int32_t reloq_frca61_n_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_n_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_n_rules,args,3,3);
    }
    return reloq_frca61_n_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_nasalize_vow(void *, int32_t);
int32_t reloq_frca61_ne_vow_elision(void *, int32_t);
static inline int32_t reloq_call_frca61_ne_vow_elision(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ne_vow_elision,args,1,1);
    }
    return reloq_frca61_ne_vow_elision(state, p0);
}
int32_t reloq_frca61_next_phone_is_vowel(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_next_phone_is_vowel(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_next_phone_is_vowel,args,4,4);
    }
    return reloq_frca61_next_phone_is_vowel(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_no_pause_btw_coordinated_NPs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_no_pause_btw_coordinated_NPs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_pause_btw_coordinated_NPs,args,3,3);
    }
    return reloq_frca61_no_pause_btw_coordinated_NPs(state, p0, p1, p2);
}
int32_t reloq_frca61_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_frca61_no_readin(state, p0, p1);
}
int32_t reloq_frca61_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_frca61_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_frca61_no_words_in_delta(state, p0);
}
int32_t reloq_frca61_normalize_fren_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_normalize_fren_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_fren_letters,args,3,3);
    }
    return reloq_frca61_normalize_fren_letters(state, p0, p1, p2);
}
int32_t reloq_frca61_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_frca61_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_frca61_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_frca61_normalize_text(state, p0, p1, p2);
}
int32_t reloq_frca61_noun_no_override_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_noun_no_override_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_no_override_rules,args,3,3);
    }
    return reloq_frca61_noun_no_override_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_noun_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_noun_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_phrase,args,4,4);
    }
    return reloq_frca61_noun_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_noun_phrase_liaison(void *, int32_t);
static inline int32_t reloq_call_frca61_noun_phrase_liaison(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_noun_phrase_liaison,args,1,1);
    }
    return reloq_frca61_noun_phrase_liaison(state, p0);
}
int32_t reloq_frca61_noun_vs_prep(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_noun_vs_prep(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_prep,args,3,3);
    }
    return reloq_frca61_noun_vs_prep(state, p0, p1, p2);
}
int32_t reloq_frca61_noun_vs_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_noun_vs_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_verb,args,3,3);
    }
    return reloq_frca61_noun_vs_verb(state, p0, p1, p2);
}
int32_t reloq_frca61_nounadj_stem_change(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_nounadj_stem_change(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_nounadj_stem_change,args,3,3);
    }
    return reloq_frca61_nounadj_stem_change(state, p0, p1, p2);
}
int32_t reloq_frca61_nounadj_stem_change_lookups(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_nounadj_stem_change_lookups(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_nounadj_stem_change_lookups,args,3,3);
    }
    return reloq_frca61_nounadj_stem_change_lookups(state, p0, p1, p2);
}
int32_t reloq_frca61_nucdur_shortening(void *, int32_t);
int32_t reloq_frca61_nucl_f0(void *, int32_t);
static inline int32_t reloq_call_frca61_nucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_f0,args,1,1);
    }
    return reloq_frca61_nucl_f0(state, p0);
}
int32_t reloq_frca61_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_frca61_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_frca61_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_frca61_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_frca61_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_tone,args,1,1);
    }
    return reloq_frca61_nucl_high_tone(state, p0);
}
int32_t reloq_frca61_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_frca61_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_frca61_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_frca61_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_frca61_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_tone,args,1,1);
    }
    return reloq_frca61_nucl_low_tone(state, p0);
}
int32_t reloq_frca61_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_frca61_nucleus_vals(state, p0);
}
int32_t reloq_frca61_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_frca61_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_frca61_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_o_circ_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_o_circ_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_o_circ_rules,args,3,3);
    }
    return reloq_frca61_o_circ_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_o_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_o_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_o_rules,args,3,3);
    }
    return reloq_frca61_o_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_onset_effects(void *, int32_t);
static inline int32_t reloq_call_frca61_onset_effects(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_onset_effects,args,1,1);
    }
    return reloq_frca61_onset_effects(state, p0);
}
int32_t reloq_frca61_open_final_syll(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_open_final_syll(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_open_final_syll,args,2,2);
    }
    return reloq_frca61_open_final_syll(state, p0, p1);
}
int32_t reloq_frca61_other_liaison(void *, int32_t);
static inline int32_t reloq_call_frca61_other_liaison(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_other_liaison,args,1,1);
    }
    return reloq_frca61_other_liaison(state, p0);
}
int32_t reloq_frca61_p_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_p_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_p_rules,args,3,3);
    }
    return reloq_frca61_p_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_paren_phrase(void *, int32_t);
static inline int32_t reloq_call_frca61_paren_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_paren_phrase,args,1,1);
    }
    return reloq_frca61_paren_phrase(state, p0);
}
int32_t reloq_frca61_parenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_parenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_parenthesis,args,3,3);
    }
    return reloq_frca61_parenthesis(state, p0, p1, p2);
}
int32_t reloq_frca61_particle_vow_elision(void *, int32_t);
static inline int32_t reloq_call_frca61_particle_vow_elision(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_particle_vow_elision,args,1,1);
    }
    return reloq_frca61_particle_vow_elision(state, p0);
}
int32_t reloq_frca61_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_frca61_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_frca61_period_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_frca61_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_frca61_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phr_initial_F0,args,1,1);
    }
    return reloq_frca61_phr_initial_F0(state, p0);
}
int32_t reloq_frca61_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_frca61_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_phrase_level_printouts(void *, int32_t);
static inline int32_t reloq_call_frca61_phrase_level_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_printouts,args,1,1);
    }
    return reloq_frca61_phrase_level_printouts(state, p0);
}
int32_t reloq_frca61_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_frca61_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_tone,args,1,1);
    }
    return reloq_frca61_phrase_tone(state, p0);
}
int32_t reloq_frca61_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_frca61_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_position_t_ptrs,args,1,1);
    }
    return reloq_frca61_position_t_ptrs(state, p0);
}
int32_t reloq_frca61_poss_vs_obj_vs_nom(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_poss_vs_obj_vs_nom(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_poss_vs_obj_vs_nom,args,3,3);
    }
    return reloq_frca61_poss_vs_obj_vs_nom(state, p0, p1, p2);
}
int32_t reloq_frca61_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_comma_phrase,args,3,3);
    }
    return reloq_frca61_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_frca61_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_postnucl_f0,args,1,1);
    }
    return reloq_frca61_postnucl_f0(state, p0);
}
int32_t reloq_frca61_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_frca61_potential_sent_end(state, p0, p1);
}
int32_t reloq_frca61_pre_syll_phonology(void *, int32_t);
static inline int32_t reloq_call_frca61_pre_syll_phonology(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_pre_syll_phonology,args,1,1);
    }
    return reloq_frca61_pre_syll_phonology(state, p0);
}
int32_t reloq_frca61_pre_vocalic_adjustments(void *, int32_t);
static inline int32_t reloq_call_frca61_pre_vocalic_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_pre_vocalic_adjustments,args,1,1);
    }
    return reloq_frca61_pre_vocalic_adjustments(state, p0);
}
int32_t reloq_frca61_precedes_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_precedes_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_end_quote,args,2,2);
    }
    return reloq_frca61_precedes_end_quote(state, p0, p1);
}
int32_t reloq_frca61_precedes_number(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_preceding_disk_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_preceding_disk_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_preceding_disk_word,args,2,2);
    }
    return reloq_frca61_preceding_disk_word(state, p0, p1);
}
int32_t reloq_frca61_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_frca61_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_prenucl_f0,args,1,1);
    }
    return reloq_frca61_prenucl_f0(state, p0);
}
int32_t reloq_frca61_prep_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_prep_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prep_phrase,args,4,4);
    }
    return reloq_frca61_prep_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_frca61_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_prev_phrase_triggers_voc(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_prev_phrase_triggers_voc(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_prev_phrase_triggers_voc,args,2,2);
    }
    return reloq_frca61_prev_phrase_triggers_voc(state, p0, p1);
}
int32_t reloq_frca61_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_frca61_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_frca61_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_frca61_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_frca61_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_frca61_print_fren_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_print_fren_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_fren_SPR,args,3,3);
    }
    return reloq_frca61_print_fren_SPR(state, p0, p1, p2);
}
int32_t reloq_frca61_print_interword_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_print_interword_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_interword_chars,args,3,3);
    }
    return reloq_frca61_print_interword_chars(state, p0, p1, p2);
}
int32_t reloq_frca61_print_normalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_print_normalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_normalized_inp,args,3,3);
    }
    return reloq_frca61_print_normalized_inp(state, p0, p1, p2);
}
int32_t reloq_frca61_print_prompt(void *, int32_t);
static inline int32_t reloq_call_frca61_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_frca61_print_prompt(state, p0);
}
int32_t reloq_frca61_print_spr(void *, int32_t);
static inline int32_t reloq_call_frca61_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_frca61_print_spr(state, p0);
}
int32_t reloq_frca61_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_frca61_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_frca61_print_word_stress(state, p0, p1);
}
int32_t reloq_frca61_printouts(void *, int32_t);
static inline int32_t reloq_call_frca61_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_frca61_printouts(state, p0);
}
int32_t reloq_frca61_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_frca61_process_annotation(state, p0, p1, p2);
}
int32_t reloq_frca61_process_final_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_process_final_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_apostrophe,args,3,3);
    }
    return reloq_frca61_process_final_apostrophe(state, p0, p1, p2);
}
int32_t reloq_frca61_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_process_final_parenthetical(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_parenthetical,args,3,3);
    }
    return reloq_frca61_process_final_parenthetical(state, p0, p1, p2);
}
int32_t reloq_frca61_process_hostname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_process_hostname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_hostname,args,3,3);
    }
    return reloq_frca61_process_hostname(state, p0, p1, p2);
}
int32_t reloq_frca61_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_frca61_process_input(state, p0, p1);
}
int32_t reloq_frca61_process_pathname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_process_pathname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_pathname,args,3,3);
    }
    return reloq_frca61_process_pathname(state, p0, p1, p2);
}
int32_t reloq_frca61_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_frca61_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_proclitic(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_proclitic(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_proclitic,args,3,3);
    }
    return reloq_frca61_proclitic(state, p0, p1, p2);
}
int32_t reloq_frca61_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_frca61_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_frca61_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_frca61_punctuation(state, p0, p1, p2);
}
int32_t reloq_frca61_q_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_q_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_q_rules,args,3,3);
    }
    return reloq_frca61_q_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_frca61_question_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_quote_mark(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_quote_mark(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_quote_mark,args,3,3);
    }
    return reloq_frca61_quote_mark(state, p0, p1, p2);
}
int32_t reloq_frca61_r_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_r_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_r_rules,args,3,3);
    }
    return reloq_frca61_r_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_read_end(void *, int32_t);
static inline int32_t reloq_call_frca61_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_frca61_read_end(state, p0);
}
int32_t reloq_frca61_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_frca61_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_frca61_realize_floating_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_realize_floating_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_realize_floating_tone,args,1,1);
    }
    return reloq_frca61_realize_floating_tone(state, p0);
}
int32_t reloq_frca61_realize_prenucl_tone(void *, int32_t);
static inline int32_t reloq_call_frca61_realize_prenucl_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_realize_prenucl_tone,args,1,1);
    }
    return reloq_frca61_realize_prenucl_tone(state, p0);
}
int32_t reloq_frca61_reduce_duplicate_cons(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_reduce_duplicate_cons(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reduce_duplicate_cons,args,3,3);
    }
    return reloq_frca61_reduce_duplicate_cons(state, p0, p1, p2);
}
int32_t reloq_frca61_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_frca61_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_frca61_register_SAPI_voices(state, p0);
}
int32_t reloq_frca61_register_fren_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_register_fren_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_fren_phones,args,3,3);
    }
    return reloq_frca61_register_fren_phones(state, p0, p1, p2);
}
int32_t reloq_frca61_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_frca61_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_relpro(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_relpro(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_relpro,args,4,4);
    }
    return reloq_frca61_relpro(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_reset_endptr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_endptr,args,4,4);
    }
    return reloq_frca61_reset_endptr(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_frca61_reset_enum_ptr(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_reset_enum_ptr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_enum_ptr,args,2,2);
    }
    return reloq_frca61_reset_enum_ptr(state, p0, p1);
}
int32_t reloq_frca61_reset_fren_sent_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_reset_fren_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_fren_sent_vars,args,1,1);
    }
    return reloq_frca61_reset_fren_sent_vars(state, p0);
}
int32_t reloq_frca61_reset_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_reset_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_gcat_vars,args,1,1);
    }
    return reloq_frca61_reset_gcat_vars(state, p0);
}
int32_t reloq_frca61_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_frca61_reset_lang_sent_vars(state, p0);
}
int32_t reloq_frca61_reset_left_endptr(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_reset_phnol_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_reset_phnol_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phnol_vars,args,1,1);
    }
    return reloq_frca61_reset_phnol_vars(state, p0);
}
int32_t reloq_frca61_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_frca61_reset_phrase_vars(state, p0);
}
int32_t reloq_frca61_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_frca61_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_frca61_reset_sent_vars(state, p0);
}
int32_t reloq_frca61_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_frca61_reset_token_vars(state, p0, p1);
}
int32_t reloq_frca61_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_frca61_reset_word_vals(state, p0, p1);
}
int32_t reloq_frca61_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_frca61_run_filters(state, p0, p1, p2);
}
int32_t reloq_frca61_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_frca61_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_frca61_run_speech_module(state, p0);
}
int32_t reloq_frca61_run_text_module(void *, int32_t);
static inline int32_t reloq_call_frca61_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_frca61_run_text_module(state, p0);
}
int32_t reloq_frca61_s_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_s_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_s_rules,args,3,3);
    }
    return reloq_frca61_s_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_said_phrase,args,4,4);
    }
    return reloq_frca61_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_same_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_same_inp,args,5,5);
    }
    return reloq_frca61_same_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_frca61_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_frca61_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_frca61_scale_word_final_trans(state, p0);
}
int32_t reloq_frca61_schwa_deletion(void *, int32_t);
static inline int32_t reloq_call_frca61_schwa_deletion(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_schwa_deletion,args,1,1);
    }
    return reloq_frca61_schwa_deletion(state, p0);
}
int32_t reloq_frca61_schwa_reduction(void *, int32_t);
static inline int32_t reloq_call_frca61_schwa_reduction(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_schwa_reduction,args,1,1);
    }
    return reloq_frca61_schwa_reduction(state, p0);
}
int32_t reloq_frca61_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_frca61_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_frca61_semicolon_phrase(state, p0);
}
int32_t reloq_frca61_separate_number_sequences(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_separate_number_sequences(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_number_sequences,args,2,2);
    }
    return reloq_frca61_separate_number_sequences(state, p0, p1);
}
int32_t reloq_frca61_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_frca61_separate_tokens(state, p0, p1);
}
int32_t reloq_frca61_set_adjacent_word_ptrs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_adjacent_word_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_adjacent_word_ptrs,args,3,3);
    }
    return reloq_frca61_set_adjacent_word_ptrs(state, p0, p1, p2);
}
int32_t reloq_frca61_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_frca61_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_frca61_set_f0_range_ptrs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_f0_range_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_range_ptrs,args,5,5);
    }
    return reloq_frca61_set_f0_range_ptrs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_frca61_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_frca61_set_f0_wind_and_base(state, p0);
}
int32_t reloq_frca61_set_first_postnuc_value(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_first_postnuc_value(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_first_postnuc_value,args,2,2);
    }
    return reloq_frca61_set_first_postnuc_value(state, p0, p1);
}
int32_t reloq_frca61_set_floating_tone_targets(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_floating_tone_targets(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_floating_tone_targets,args,2,2);
    }
    return reloq_frca61_set_floating_tone_targets(state, p0, p1);
}
int32_t reloq_frca61_set_floating_tones(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_floating_tones(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_floating_tones,args,5,5);
    }
    return reloq_frca61_set_floating_tones(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_set_fren_global_constants(void *, int32_t);
static inline int32_t reloq_call_frca61_set_fren_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_fren_global_constants,args,1,1);
    }
    return reloq_frca61_set_fren_global_constants(state, p0);
}
int32_t reloq_frca61_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_frca61_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_frca61_set_global_constants(state, p0);
}
int32_t reloq_frca61_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_frca61_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_frca61_set_klattparms(state, p0);
}
int32_t reloq_frca61_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_frca61_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_frca61_set_lang_global_constants(state, p0);
}
int32_t reloq_frca61_set_letter_name_wordcats(void *, int32_t);
static inline int32_t reloq_call_frca61_set_letter_name_wordcats(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_letter_name_wordcats,args,1,1);
    }
    return reloq_frca61_set_letter_name_wordcats(state, p0);
}
int32_t reloq_frca61_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_frca61_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_frca61_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_frca61_set_num_formants(state, p0);
}
int32_t reloq_frca61_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_frca61_set_phone_offset_vals(state, p0);
}
int32_t reloq_frca61_set_phr_init_form_vals(void *, int32_t);
int32_t reloq_frca61_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_frca61_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_frca61_set_said_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_set_said_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_said_phr_vals,args,3,3);
    }
    return reloq_frca61_set_said_phr_vals(state, p0, p1, p2);
}
int32_t reloq_frca61_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_frca61_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_frca61_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_frca61_set_syll_offset_vals(state, p0);
}
int32_t reloq_frca61_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_frca61_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_frca61_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_frca61_set_t3_pointer(void *, int32_t);
static inline int32_t reloq_call_frca61_set_t3_pointer(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_t3_pointer,args,1,1);
    }
    return reloq_frca61_set_t3_pointer(state, p0);
}
int32_t reloq_frca61_set_t3_position(void *, int32_t);
static inline int32_t reloq_call_frca61_set_t3_position(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_t3_position,args,1,1);
    }
    return reloq_frca61_set_t3_position(state, p0);
}
int32_t reloq_frca61_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_frca61_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_frca61_set_voice_param_constants(state, p0);
}
int32_t reloq_frca61_set_window_size(void *, int32_t);
static inline int32_t reloq_call_frca61_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_frca61_set_window_size(state, p0);
}
int32_t reloq_frca61_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_frca61_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_frca61_setdur(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_setdur(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setdur,args,3,3);
    }
    return reloq_frca61_setdur(state, p0, p1, p2);
}
int32_t reloq_frca61_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_frca61_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_frca61_sf_default_shortening(void *, int32_t);
int32_t reloq_frca61_shift_tone(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_shift_tone(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_shift_tone,args,2,2);
    }
    return reloq_frca61_shift_tone(state, p0, p1);
}
int32_t reloq_frca61_shortdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_shortdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_shortdash,args,3,3);
    }
    return reloq_frca61_shortdash(state, p0, p1, p2);
}
int32_t reloq_frca61_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_frca61_single_chars(state, p0, p1, p2);
}
int32_t reloq_frca61_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_frca61_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_frca61_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_frca61_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_frca61_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_frca61_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_frca61_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_frca61_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_frca61_single_letter(state, p0, p1, p2);
}
int32_t reloq_frca61_six_dix_final_con(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_six_dix_final_con(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_six_dix_final_con,args,2,2);
    }
    return reloq_frca61_six_dix_final_con(state, p0, p1);
}
int32_t reloq_frca61_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_frca61_size_filter(state, p0, p1, p2);
}
int32_t reloq_frca61_skip_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_skip_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_delimiters,args,2,2);
    }
    return reloq_frca61_skip_delimiters(state, p0, p1);
}
int32_t reloq_frca61_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_frca61_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_frca61_slash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_slash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash,args,3,3);
    }
    return reloq_frca61_slash(state, p0, p1, p2);
}
int32_t reloq_frca61_slash_before_measure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_slash_before_measure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash_before_measure,args,3,3);
    }
    return reloq_frca61_slash_before_measure(state, p0, p1, p2);
}
int32_t reloq_frca61_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_frca61_space(state, p0, p1, p2);
}
int32_t reloq_frca61_span_inverted_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_span_inverted_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_span_inverted_punct,args,3,3);
    }
    return reloq_frca61_span_inverted_punct(state, p0, p1, p2);
}
int32_t reloq_frca61_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_frca61_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_frca61_spell_out_large_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_spell_out_large_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_large_num,args,3,3);
    }
    return reloq_frca61_spell_out_large_num(state, p0, p1, p2);
}
int32_t reloq_frca61_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_frca61_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_frca61_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_frca61_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_frca61_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_stanfren_vals(void *, int32_t);
static inline int32_t reloq_call_frca61_stanfren_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanfren_vals,args,1,1);
    }
    return reloq_frca61_stanfren_vals(state, p0);
}
int32_t reloq_frca61_starts_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_starts_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_starts_major_phrase,args,2,2);
    }
    return reloq_frca61_starts_major_phrase(state, p0, p1);
}
int32_t reloq_frca61_starts_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_starts_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_starts_phrase,args,2,2);
    }
    return reloq_frca61_starts_phrase(state, p0, p1);
}
int32_t reloq_frca61_strengthen_last_in_a_sequence(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_strengthen_last_in_a_sequence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_strengthen_last_in_a_sequence,args,2,2);
    }
    return reloq_frca61_strengthen_last_in_a_sequence(state, p0, p1);
}
int32_t reloq_frca61_stress_before_FW(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_stress_before_FW(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_before_FW,args,3,3);
    }
    return reloq_frca61_stress_before_FW(state, p0, p1, p2);
}
int32_t reloq_frca61_stress_conj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_stress_conj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_conj,args,3,3);
    }
    return reloq_frca61_stress_conj(state, p0, p1, p2);
}
int32_t reloq_frca61_stress_functed_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_stress_functed_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_functed_adv,args,3,3);
    }
    return reloq_frca61_stress_functed_adv(state, p0, p1, p2);
}
int32_t reloq_frca61_stress_pros(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_stress_pros(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_pros,args,3,3);
    }
    return reloq_frca61_stress_pros(state, p0, p1, p2);
}
int32_t reloq_frca61_stress_relpros(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_stress_relpros(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_relpros,args,3,3);
    }
    return reloq_frca61_stress_relpros(state, p0, p1, p2);
}
int32_t reloq_frca61_stress_zero_syll(void *, int32_t);
static inline int32_t reloq_call_frca61_stress_zero_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stress_zero_syll,args,1,1);
    }
    return reloq_frca61_stress_zero_syll(state, p0);
}
int32_t reloq_frca61_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_frca61_stretch_focus(state, p0, p1);
}
int32_t reloq_frca61_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_frca61_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_string_to_short(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_short,args,4,4);
    }
    return reloq_frca61_string_to_short(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_strip_last_digit(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_strip_ment(void *, int32_t);
static inline int32_t reloq_call_frca61_strip_ment(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_ment,args,1,1);
    }
    return reloq_frca61_strip_ment(state, p0);
}
int32_t reloq_frca61_strip_s(void *, int32_t);
static inline int32_t reloq_call_frca61_strip_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_strip_s,args,1,1);
    }
    return reloq_frca61_strip_s(state, p0);
}
int32_t reloq_frca61_subj_pro(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_subj_pro(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subj_pro,args,4,4);
    }
    return reloq_frca61_subj_pro(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_subj_vs_obj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_subj_vs_obj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subj_vs_obj,args,3,3);
    }
    return reloq_frca61_subj_vs_obj(state, p0, p1, p2);
}
int32_t reloq_frca61_subord_conj(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_subord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subord_conj,args,4,4);
    }
    return reloq_frca61_subord_conj(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_subval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_subval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subval,args,3,3);
    }
    return reloq_frca61_subval(state, p0, p1, p2);
}
int32_t reloq_frca61_symbolic_rules(void *, int32_t);
static inline int32_t reloq_call_frca61_symbolic_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_symbolic_rules,args,1,1);
    }
    return reloq_frca61_symbolic_rules(state, p0);
}
int32_t reloq_frca61_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_frca61_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_frca61_t_intercole(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_t_intercole(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_t_intercole,args,3,3);
    }
    return reloq_frca61_t_intercole(state, p0, p1, p2);
}
int32_t reloq_frca61_t_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_t_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_t_rules,args,3,3);
    }
    return reloq_frca61_t_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_tag_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_tag_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tag_phrase,args,3,3);
    }
    return reloq_frca61_tag_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_test_for_fren_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_test_for_fren_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_test_for_fren_ordinal,args,3,3);
    }
    return reloq_frca61_test_for_fren_ordinal(state, p0, p1, p2);
}
int32_t reloq_frca61_test_for_t_intercole(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_test_for_t_intercole(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_test_for_t_intercole,args,2,2);
    }
    return reloq_frca61_test_for_t_intercole(state, p0, p1);
}
int32_t reloq_frca61_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_frca61_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_frca61_text_index(state, p0, p1);
}
int32_t reloq_frca61_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_frca61_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_three_letter_extension(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_three_letter_extension(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_letter_extension,args,2,2);
    }
    return reloq_frca61_three_letter_extension(state, p0, p1);
}
int32_t reloq_frca61_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict,args,3,3);
    }
    return reloq_frca61_tok_dict(state, p0, p1, p2);
}
int32_t reloq_frca61_tok_dict_entry(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_tok_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict_entry,args,3,3);
    }
    return reloq_frca61_tok_dict_entry(state, p0, p1, p2);
}
int32_t reloq_frca61_tok_lookup(void *, int32_t, int32_t);
static inline int32_t reloq_call_frca61_tok_lookup(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_lookup,args,2,2);
    }
    return reloq_frca61_tok_lookup(state, p0, p1);
}
int32_t reloq_frca61_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_frca61_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_frca61_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_frca61_token_to_number(state, p0, p1, p2);
}
int32_t reloq_frca61_tout_liaison(void *, int32_t);
static inline int32_t reloq_call_frca61_tout_liaison(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_tout_liaison,args,1,1);
    }
    return reloq_frca61_tout_liaison(state, p0);
}
int32_t reloq_frca61_u_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_u_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_u_rules,args,3,3);
    }
    return reloq_frca61_u_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_user_abbr_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_user_abbr_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_abbr_dict,args,3,3);
    }
    return reloq_frca61_user_abbr_dict(state, p0, p1, p2);
}
int32_t reloq_frca61_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_frca61_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_user_rootdict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_user_rootdict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_rootdict,args,3,3);
    }
    return reloq_frca61_user_rootdict(state, p0, p1, p2);
}
int32_t reloq_frca61_v_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_v_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_v_rules,args,3,3);
    }
    return reloq_frca61_v_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_valid_f0_posn(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_valid_f0_posn(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_f0_posn,args,3,3);
    }
    return reloq_frca61_valid_f0_posn(state, p0, p1, p2);
}
int32_t reloq_frca61_verb_liaison(void *, int32_t);
static inline int32_t reloq_call_frca61_verb_liaison(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_verb_liaison,args,1,1);
    }
    return reloq_frca61_verb_liaison(state, p0);
}
int32_t reloq_frca61_verb_no_override_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_verb_no_override_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_verb_no_override_rules,args,3,3);
    }
    return reloq_frca61_verb_no_override_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_verb_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_verb_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_verb_phrase,args,4,4);
    }
    return reloq_frca61_verb_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_frca61_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_vocative_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_vocative_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocative_phrase,args,3,3);
    }
    return reloq_frca61_vocative_phrase(state, p0, p1, p2);
}
int32_t reloq_frca61_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_frca61_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_frca61_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_w_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_w_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_w_rules,args,3,3);
    }
    return reloq_frca61_w_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_which_get_non_vowels(void *, int32_t);
static inline int32_t reloq_call_frca61_which_get_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_which_get_non_vowels,args,1,1);
    }
    return reloq_frca61_which_get_non_vowels(state, p0);
}
int32_t reloq_frca61_which_number_verb_ending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_which_number_verb_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_which_number_verb_ending,args,5,5);
    }
    return reloq_frca61_which_number_verb_ending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_frca61_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_frca61_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_frca61_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_word_medial_oe_deletion(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_word_medial_oe_deletion(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_medial_oe_deletion,args,3,3);
    }
    return reloq_frca61_word_medial_oe_deletion(state, p0, p1, p2);
}
int32_t reloq_frca61_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_frca61_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_x_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_x_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_x_rules,args,3,3);
    }
    return reloq_frca61_x_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_y_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_y_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_y_rules,args,3,3);
    }
    return reloq_frca61_y_rules(state, p0, p1, p2);
}
int32_t reloq_frca61_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_frca61_year_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_frca61_z_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_frca61_z_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_z_rules,args,3,3);
    }
    return reloq_frca61_z_rules(state, p0, p1, p2);
}
#endif
