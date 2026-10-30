/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_KOKR61_H
#define RELOQ61_RULE_CALLS_KOKR61_H
#include "delta_rules_c.h"
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_kokr61_DeltaProc_end(void *, int32_t);
int32_t reloq_kokr61_DeltaProc_flush(void *, int32_t);
int32_t reloq_kokr61_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_kokr61_DeltaProc_main(void *, int32_t);
int32_t reloq_kokr61_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_kokr61_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_kokr61_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_kokr61_DeltaProc_start(void *, int32_t);
int32_t reloq_kokr61_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_kokr61_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_kokr61_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_kokr61_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_URL(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_URL(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_URL,args,2,2);
    }
    return reloq_kokr61_URL(state, p0, p1);
}
int32_t reloq_kokr61_abbr_before_num(void *, int32_t, int32_t);
int32_t reloq_kokr61_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_kokr61_absval(state, p0, p1, p2);
}
int32_t reloq_kokr61_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_kokr61_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_kokr61_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_add_f0_for_new_voice(void *, int32_t);
static inline int32_t reloq_call_kokr61_add_f0_for_new_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_new_voice,args,1,1);
    }
    return reloq_kokr61_add_f0_for_new_voice(state, p0);
}
int32_t reloq_kokr61_add_f0_for_next_voice(void *, int32_t);
static inline int32_t reloq_call_kokr61_add_f0_for_next_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_next_voice,args,1,1);
    }
    return reloq_kokr61_add_f0_for_next_voice(state, p0);
}
int32_t reloq_kokr61_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_kokr61_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_kokr61_add_kor_currency_units(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_add_kor_currency_units(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_kor_currency_units,args,3,3);
    }
    return reloq_kokr61_add_kor_currency_units(state, p0, p1, p2);
}
int32_t reloq_kokr61_add_kor_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_add_kor_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_kor_point_digits,args,3,3);
    }
    return reloq_kokr61_add_kor_point_digits(state, p0, p1, p2);
}
int32_t reloq_kokr61_add_kor_subunits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_add_kor_subunits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_kor_subunits,args,3,3);
    }
    return reloq_kokr61_add_kor_subunits(state, p0, p1, p2);
}
int32_t reloq_kokr61_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_kokr61_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_kokr61_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_kokr61_adjust_AV_for_fem_voice(void *, int32_t);
static inline int32_t reloq_call_kokr61_adjust_AV_for_fem_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_fem_voice,args,1,1);
    }
    return reloq_kokr61_adjust_AV_for_fem_voice(state, p0);
}
int32_t reloq_kokr61_adjust_acc_phrase_types(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_adjust_acc_phrase_types(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_acc_phrase_types,args,3,3);
    }
    return reloq_kokr61_adjust_acc_phrase_types(state, p0, p1, p2);
}
int32_t reloq_kokr61_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_kokr61_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_adjust_kor_female_breathiness(void *, int32_t);
static inline int32_t reloq_call_kokr61_adjust_kor_female_breathiness(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_kor_female_breathiness,args,1,1);
    }
    return reloq_kokr61_adjust_kor_female_breathiness(state, p0);
}
int32_t reloq_kokr61_adjust_kor_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_adjust_kor_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_kor_word_stress,args,3,3);
    }
    return reloq_kokr61_adjust_kor_word_stress(state, p0, p1, p2);
}
int32_t reloq_kokr61_adjust_param(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_adjust_param(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_param,args,4,4);
    }
    return reloq_kokr61_adjust_param(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_adjust_prev_phr_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_pause,args,3,3);
    }
    return reloq_kokr61_adjust_prev_phr_pause(state, p0, p1, p2);
}
int32_t reloq_kokr61_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_kokr61_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_kokr61_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_adjust_words_in_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_words_in_phrase,args,3,3);
    }
    return reloq_kokr61_adjust_words_in_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_ambig_abbr(void *, int32_t, int32_t);
int32_t reloq_kokr61_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_kokr61_apostrophe(state, p0, p1, p2);
}
int32_t reloq_kokr61_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_kokr61_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_kokr61_apply_comma_phr_rules(void *, int32_t);
static inline int32_t reloq_call_kokr61_apply_comma_phr_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_comma_phr_rules,args,1,1);
    }
    return reloq_kokr61_apply_comma_phr_rules(state, p0);
}
int32_t reloq_kokr61_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_kokr61_apply_head_size_val(state, p0, p1);
}
int32_t reloq_kokr61_apply_kor_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_kor_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_kor_comma_rules,args,3,3);
    }
    return reloq_kokr61_apply_kor_comma_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_lang_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_lang_comma_rules,args,3,3);
    }
    return reloq_kokr61_apply_lang_comma_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_kokr61_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_kokr61_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_kokr61_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_kokr61_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_kokr61_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_kokr61_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_kokr61_apply_roughness_val(state, p0, p1);
}
int32_t reloq_kokr61_apply_rules(void *, int32_t);
static inline int32_t reloq_call_kokr61_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_kokr61_apply_rules(state, p0);
}
int32_t reloq_kokr61_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_kokr61_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_kokr61_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_kokr61_apply_speed_val(state, p0, p1);
}
int32_t reloq_kokr61_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_kokr61_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_kokr61_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_kokr61_apply_volume_val(state, p0, p1);
}
int32_t reloq_kokr61_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_kokr61_assign_acoustic_vals(state, p0);
}
int32_t reloq_kokr61_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_kokr61_assign_alt_quest_features(state, p0);
}
int32_t reloq_kokr61_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_kokr61_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_kokr61_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_kokr61_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_kokr61_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_kokr61_assign_default_quest_features(state, p0);
}
int32_t reloq_kokr61_assign_kor_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_kor_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_kor_nuc_durs,args,1,1);
    }
    return reloq_kokr61_assign_kor_nuc_durs(state, p0);
}
int32_t reloq_kokr61_assign_kor_str_syll(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_kor_str_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_kor_str_syll,args,1,1);
    }
    return reloq_kokr61_assign_kor_str_syll(state, p0);
}
int32_t reloq_kokr61_assign_kor_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_kokr61_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_kokr61_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_kokr61_assign_nuc_durs(state, p0);
}
int32_t reloq_kokr61_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_kokr61_assign_phone_durs(state, p0);
}
int32_t reloq_kokr61_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_kokr61_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_kokr61_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_kokr61_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_kokr61_assign_sent_final_features(state, p0);
}
int32_t reloq_kokr61_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_kokr61_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_kokr61_assign_user_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_kokr61_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_kokr61_assign_wh_quest_features(state, p0);
}
int32_t reloq_kokr61_beg_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_beg_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_beg_of_word,args,2,2);
    }
    return reloq_kokr61_beg_of_word(state, p0, p1);
}
int32_t reloq_kokr61_between_proper_names(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_boundary_tone(void *, int32_t);
int32_t reloq_kokr61_bracket(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_bracket(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_bracket,args,3,3);
    }
    return reloq_kokr61_bracket(state, p0, p1, p2);
}
int32_t reloq_kokr61_breakup_kor_spr_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_breakup_kor_spr_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_breakup_kor_spr_phones,args,3,3);
    }
    return reloq_kokr61_breakup_kor_spr_phones(state, p0, p1, p2);
}
int32_t reloq_kokr61_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_breakup_spr_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_breakup_spr_phones,args,3,3);
    }
    return reloq_kokr61_breakup_spr_phones(state, p0, p1, p2);
}
int32_t reloq_kokr61_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_kokr61_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_kokr61_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_kokr61_build_phrases(state, p0, p1, p2);
}
int32_t reloq_kokr61_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_kokr61_build_words(state, p0, p1, p2);
}
int32_t reloq_kokr61_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_kokr61_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_kokr61_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_kokr61_char_name(state, p0, p1, p2);
}
int32_t reloq_kokr61_clear_delta(void *, int32_t);
static inline int32_t reloq_call_kokr61_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_kokr61_clear_delta(state, p0);
}
int32_t reloq_kokr61_close_platform(void *, int32_t);
static inline int32_t reloq_call_kokr61_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_kokr61_close_platform(state, p0);
}
int32_t reloq_kokr61_comma_at_end_quote(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_comma_at_end_quote(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_comma_at_end_quote,args,3,3);
    }
    return reloq_kokr61_comma_at_end_quote(state, p0, p1, p2);
}
int32_t reloq_kokr61_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_kokr61_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_kokr61_comma_phrase(state, p0);
}
int32_t reloq_kokr61_con_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_kokr61_con_vals(state, p0);
}
int32_t reloq_kokr61_convert_1_or_2_digits(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_convert_RRN(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_RRN(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_RRN,args,3,3);
    }
    return reloq_kokr61_convert_RRN(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits,args,3,3);
    }
    return reloq_kokr61_convert_digits(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits_to_words,args,3,3);
    }
    return reloq_kokr61_convert_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_double,args,3,3);
    }
    return reloq_kokr61_convert_double(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_hundreds,args,3,3);
    }
    return reloq_kokr61_convert_hundreds(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_date,args,3,3);
    }
    return reloq_kokr61_convert_kor_date(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_digits_in_hangul(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_digits_in_hangul(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_digits_in_hangul,args,3,3);
    }
    return reloq_kokr61_convert_kor_digits_in_hangul(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_digits_to_words,args,3,3);
    }
    return reloq_kokr61_convert_kor_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_double,args,3,3);
    }
    return reloq_kokr61_convert_kor_double(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_fraction,args,3,3);
    }
    return reloq_kokr61_convert_kor_fraction(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_hundreds,args,3,3);
    }
    return reloq_kokr61_convert_kor_hundreds(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_large_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_large_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_large_numbers,args,3,3);
    }
    return reloq_kokr61_convert_kor_large_numbers(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_monetary_exp,args,5,5);
    }
    return reloq_kokr61_convert_kor_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_convert_kor_phone_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_phone_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_phone_number,args,3,3);
    }
    return reloq_kokr61_convert_kor_phone_number(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_quarter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_quarter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_quarter,args,3,3);
    }
    return reloq_kokr61_convert_kor_quarter(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_single_digit,args,3,3);
    }
    return reloq_kokr61_convert_kor_single_digit(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_thousands,args,3,3);
    }
    return reloq_kokr61_convert_kor_thousands(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_kor_time(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_kor_time(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_kor_time,args,3,3);
    }
    return reloq_kokr61_convert_kor_time(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_lang_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_lang_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_lang_digits_to_words,args,3,3);
    }
    return reloq_kokr61_convert_lang_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_monetary_exp,args,5,5);
    }
    return reloq_kokr61_convert_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_convert_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_numbers,args,3,3);
    }
    return reloq_kokr61_convert_numbers(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_single_digit,args,3,3);
    }
    return reloq_kokr61_convert_single_digit(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_thousands,args,3,3);
    }
    return reloq_kokr61_convert_thousands(state, p0, p1, p2);
}
int32_t reloq_kokr61_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_kokr61_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_kokr61_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_kokr61_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_kokr61_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_kokr61_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_kokr61_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_kokr61_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_count_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_count_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_digits,args,3,3);
    }
    return reloq_kokr61_count_digits(state, p0, p1, p2);
}
int32_t reloq_kokr61_count_eng_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_count_eng_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_eng_digits,args,3,3);
    }
    return reloq_kokr61_count_eng_digits(state, p0, p1, p2);
}
int32_t reloq_kokr61_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_kokr61_count_input(state, p0, p1);
}
int32_t reloq_kokr61_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_kokr61_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_kokr61_count_normalized_inp(state, p0);
}
int32_t reloq_kokr61_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_count_sylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_sylls,args,4,4);
    }
    return reloq_kokr61_count_sylls(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_kokr61_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_kokr61_count_user_indices(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_count_user_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_user_indices,args,4,4);
    }
    return reloq_kokr61_count_user_indices(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_kokr61_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_kokr61_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_kokr61_create_sentence(state, p0, p1);
}
int32_t reloq_kokr61_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_kokr61_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_kokr61_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_kokr61_delete_2_more(state, p0);
}
int32_t reloq_kokr61_delete_comma_sync(void *, int32_t, int32_t);
int32_t reloq_kokr61_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_delete_inp_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_left,args,3,3);
    }
    return reloq_kokr61_delete_inp_from_left(state, p0, p1, p2);
}
int32_t reloq_kokr61_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_delete_inp_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_right,args,3,3);
    }
    return reloq_kokr61_delete_inp_from_right(state, p0, p1, p2);
}
int32_t reloq_kokr61_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_kokr61_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_kokr61_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_kokr61_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_kokr61_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_delete_phone_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_right,args,3,3);
    }
    return reloq_kokr61_delete_phone_from_right(state, p0, p1, p2);
}
int32_t reloq_kokr61_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_kokr61_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_kokr61_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_kokr61_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_kokr61_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_kokr61_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_kokr61_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_kokr61_demote_word(state, p0, p1, p2);
}
int32_t reloq_kokr61_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_kokr61_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_disambig_kor_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_disambig_kor_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_kor_content_word,args,3,3);
    }
    return reloq_kokr61_disambig_kor_content_word(state, p0, p1, p2);
}
int32_t reloq_kokr61_disambig_kor_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_disambig_kor_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_kor_functword,args,3,3);
    }
    return reloq_kokr61_disambig_kor_functword(state, p0, p1, p2);
}
int32_t reloq_kokr61_disambiguate_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_disambiguate_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_content_word,args,4,4);
    }
    return reloq_kokr61_disambiguate_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_disambiguate_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_disambiguate_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_functword,args,3,3);
    }
    return reloq_kokr61_disambiguate_functword(state, p0, p1, p2);
}
int32_t reloq_kokr61_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_kokr61_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_kokr61_distribute_nucdur(state, p0);
}
int32_t reloq_kokr61_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_kokr61_divide_char_count_token(state, p0, p1);
}
int32_t reloq_kokr61_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_kokr61_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_kokr61_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_kokr61_doF(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_kokr61_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_kokr61_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_kokr61_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_kokr61_do_FTZ_FTP(state, p0);
}
int32_t reloq_kokr61_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_kokr61_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_kokr61_do_glot_onset(void *, int32_t);
static inline int32_t reloq_call_kokr61_do_glot_onset(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_glot_onset,args,1,1);
    }
    return reloq_kokr61_do_glot_onset(state, p0);
}
int32_t reloq_kokr61_do_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_do_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_h_amps,args,2,2);
    }
    return reloq_kokr61_do_h_amps(state, p0, p1);
}
int32_t reloq_kokr61_do_kor_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_do_kor_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_kor_h_amps,args,2,2);
    }
    return reloq_kokr61_do_kor_h_amps(state, p0, p1);
}
int32_t reloq_kokr61_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_kokr61_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_kokr61_do_non_son_amps(state, p0);
}
int32_t reloq_kokr61_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_kokr61_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_kokr61_do_pt_voicing(state, p0);
}
int32_t reloq_kokr61_do_release(void *, int32_t);
static inline int32_t reloq_call_kokr61_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_kokr61_do_release(state, p0);
}
int32_t reloq_kokr61_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_kokr61_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_kokr61_do_son_amps(state, p0);
}
int32_t reloq_kokr61_do_user_index_callbacks(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_do_user_index_callbacks(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_user_index_callbacks,args,2,2);
    }
    return reloq_kokr61_do_user_index_callbacks(state, p0, p1);
}
int32_t reloq_kokr61_do_voice_bar(void *, int32_t);
int32_t reloq_kokr61_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_kokr61_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_kokr61_ellipsis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_ellipsis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ellipsis,args,3,3);
    }
    return reloq_kokr61_ellipsis(state, p0, p1, p2);
}
int32_t reloq_kokr61_email_address(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_email_address(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_email_address,args,2,2);
    }
    return reloq_kokr61_email_address(state, p0, p1);
}
int32_t reloq_kokr61_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_kokr61_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_kokr61_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_kokr61_end_of_sentence(state, p0, p1);
}
int32_t reloq_kokr61_end_of_token(void *, int32_t, int32_t);
int32_t reloq_kokr61_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_kokr61_end_of_word(state, p0, p1);
}
int32_t reloq_kokr61_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_kokr61_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_estimate_kor_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_estimate_kor_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_kor_nsylls,args,4,4);
    }
    return reloq_kokr61_estimate_kor_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_estimate_lang_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_estimate_lang_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_lang_nsylls,args,4,4);
    }
    return reloq_kokr61_estimate_lang_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_estimate_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_nsylls,args,4,4);
    }
    return reloq_kokr61_estimate_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_exclam_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_exclam_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_exclam_phrase,args,2,2);
    }
    return reloq_kokr61_exclam_phrase(state, p0, p1);
}
int32_t reloq_kokr61_extend_vals(void *, int32_t);
int32_t reloq_kokr61_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_kokr61_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_kokr61_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_kokr61_final_voiced_release(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_final_voiced_release(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_voiced_release,args,3,3);
    }
    return reloq_kokr61_final_voiced_release(state, p0, p1, p2);
}
int32_t reloq_kokr61_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_kokr61_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_kokr61_find_acoustic_nuc(state, p0);
}
int32_t reloq_kokr61_find_kor_nucleus(void *, int32_t);
static inline int32_t reloq_call_kokr61_find_kor_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_kor_nucleus,args,1,1);
    }
    return reloq_kokr61_find_kor_nucleus(state, p0);
}
int32_t reloq_kokr61_find_kor_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_kor_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_kor_obligatory_phrase,args,4,4);
    }
    return reloq_kokr61_find_kor_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_find_kor_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_kor_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_kor_potential_phrase,args,4,4);
    }
    return reloq_kokr61_find_kor_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_find_lang_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_lang_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_obligatory_phrase,args,4,4);
    }
    return reloq_kokr61_find_lang_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_find_lang_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_lang_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_potential_phrase,args,4,4);
    }
    return reloq_kokr61_find_lang_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_kokr61_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_find_most_stressed_syll(void *, int32_t);
static inline int32_t reloq_call_kokr61_find_most_stressed_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_most_stressed_syll,args,1,1);
    }
    return reloq_kokr61_find_most_stressed_syll(state, p0);
}
int32_t reloq_kokr61_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_next_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_word,args,4,4);
    }
    return reloq_kokr61_find_next_word(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_kokr61_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_kokr61_find_nucleus(state, p0);
}
int32_t reloq_kokr61_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_kokr61_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_previous_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_previous_word,args,4,4);
    }
    return reloq_kokr61_find_previous_word(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_find_unpuncted_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_find_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_unpuncted_phrases,args,3,3);
    }
    return reloq_kokr61_find_unpuncted_phrases(state, p0, p1, p2);
}
int32_t reloq_kokr61_first_root_vow(void *, int32_t, int32_t);
int32_t reloq_kokr61_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_kokr61_forced_synthesis(state, p0, p1);
}
int32_t reloq_kokr61_general_consonant_reduction(void *, int32_t);
static inline int32_t reloq_call_kokr61_general_consonant_reduction(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_general_consonant_reduction,args,1,1);
    }
    return reloq_kokr61_general_consonant_reduction(state, p0);
}
int32_t reloq_kokr61_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_kokr61_generate_acoustic_vals(state, p0);
}
int32_t reloq_kokr61_generate_durations(void *, int32_t);
static inline int32_t reloq_call_kokr61_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_kokr61_generate_durations(state, p0);
}
int32_t reloq_kokr61_generate_intonation(void *, int32_t);
static inline int32_t reloq_call_kokr61_generate_intonation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_intonation,args,1,1);
    }
    return reloq_kokr61_generate_intonation(state, p0);
}
int32_t reloq_kokr61_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_kokr61_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_kokr61_get_h_dur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_h_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_h_dur,args,4,4);
    }
    return reloq_kokr61_get_h_dur(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_kokr61_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_get_input(void *, int32_t);
static inline int32_t reloq_call_kokr61_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_kokr61_get_input(state, p0);
}
int32_t reloq_kokr61_get_kor_affr_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_affr_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_affr_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_kor_affr_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_get_kor_fric_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_fric_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_fric_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_kor_fric_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_get_kor_h_dur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_h_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_h_dur,args,4,4);
    }
    return reloq_kokr61_get_kor_h_dur(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_get_kor_nas_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_nas_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_nas_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_kor_nas_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_get_kor_right_syll_boundary(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_right_syll_boundary(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_right_syll_boundary,args,3,3);
    }
    return reloq_kokr61_get_kor_right_syll_boundary(state, p0, p1, p2);
}
int32_t reloq_kokr61_get_kor_son_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_son_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_son_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_kor_son_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_get_kor_stop_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_stop_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_stop_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_kor_stop_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_get_kor_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_kor_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_get_kor_vow_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_kor_vow_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_kor_vow_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_kor_vow_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_kokr61_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_kokr61_get_nuc_AV(state, p0);
}
int32_t reloq_kokr61_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_kokr61_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_get_number_info(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_number_info(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_info,args,3,3);
    }
    return reloq_kokr61_get_number_info(state, p0, p1, p2);
}
int32_t reloq_kokr61_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_get_stankor_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_kokr61_get_stankor_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stankor_nuc_AV,args,1,1);
    }
    return reloq_kokr61_get_stankor_nuc_AV(state, p0);
}
int32_t reloq_kokr61_get_tok(void *, int32_t);
static inline int32_t reloq_call_kokr61_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_kokr61_get_tok(state, p0);
}
int32_t reloq_kokr61_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_kokr61_get_trans_dur(state, p0, p1);
}
int32_t reloq_kokr61_gettok(void *, int32_t);
static inline int32_t reloq_call_kokr61_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_kokr61_gettok(state, p0);
}
int32_t reloq_kokr61_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_kokr61_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_kokr61_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_kokr61_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_kokr61_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_kokr61_handle_kor_inflection(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_handle_kor_inflection(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_kor_inflection,args,5,5);
    }
    return reloq_kokr61_handle_kor_inflection(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_handle_kor_nbu(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_handle_kor_nbu(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_kor_nbu,args,3,3);
    }
    return reloq_kokr61_handle_kor_nbu(state, p0, p1, p2);
}
int32_t reloq_kokr61_handle_kor_preending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_handle_kor_preending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_kor_preending,args,5,5);
    }
    return reloq_kokr61_handle_kor_preending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_handle_non_phrasal_commas(void *, int32_t);
static inline int32_t reloq_call_kokr61_handle_non_phrasal_commas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_phrasal_commas,args,1,1);
    }
    return reloq_kokr61_handle_non_phrasal_commas(state, p0);
}
int32_t reloq_kokr61_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_kokr61_handle_pause(state, p0, p1);
}
int32_t reloq_kokr61_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_low_star_tone,args,1,1);
    }
    return reloq_kokr61_high_low_star_tone(state, p0);
}
int32_t reloq_kokr61_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_star_low_tone,args,1,1);
    }
    return reloq_kokr61_high_star_low_tone(state, p0);
}
int32_t reloq_kokr61_high_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_tone,args,1,1);
    }
    return reloq_kokr61_high_tone(state, p0);
}
int32_t reloq_kokr61_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen,args,3,3);
    }
    return reloq_kokr61_hyphen(state, p0, p1, p2);
}
int32_t reloq_kokr61_hyphen_between_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen_between_nums,args,3,3);
    }
    return reloq_kokr61_hyphen_between_nums(state, p0, p1, p2);
}
int32_t reloq_kokr61_identify_sino_or_native(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_identify_sino_or_native(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_identify_sino_or_native,args,2,2);
    }
    return reloq_kokr61_identify_sino_or_native(state, p0, p1);
}
int32_t reloq_kokr61_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_kokr61_in_brackets(state, p0, p1, p2);
}
int32_t reloq_kokr61_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_kokr61_in_quotes(state, p0, p1, p2);
}
int32_t reloq_kokr61_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_in_syllable(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_syllable,args,4,4);
    }
    return reloq_kokr61_in_syllable(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_kokr61_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_kokr61_increment_output_file_number(state, p0);
}
int32_t reloq_kokr61_init_accphr_inton_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_accphr_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_accphr_inton_vars,args,1,1);
    }
    return reloq_kokr61_init_accphr_inton_vars(state, p0);
}
int32_t reloq_kokr61_init_accphr_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_accphr_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_accphr_vars,args,1,1);
    }
    return reloq_kokr61_init_accphr_vars(state, p0);
}
int32_t reloq_kokr61_init_adjacent_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_adjacent_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_adjacent_word_ptrs,args,1,1);
    }
    return reloq_kokr61_init_adjacent_word_ptrs(state, p0);
}
int32_t reloq_kokr61_init_current_word_ptrs(void *, int32_t);
int32_t reloq_kokr61_init_delta(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_kokr61_init_delta(state, p0);
}
int32_t reloq_kokr61_init_gcat_vars(void *, int32_t);
int32_t reloq_kokr61_init_kor_accphr_inton_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_kor_accphr_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_kor_accphr_inton_vars,args,1,1);
    }
    return reloq_kokr61_init_kor_accphr_inton_vars(state, p0);
}
int32_t reloq_kokr61_init_kor_accphr_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_kor_accphr_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_kor_accphr_vars,args,1,1);
    }
    return reloq_kokr61_init_kor_accphr_vars(state, p0);
}
int32_t reloq_kokr61_init_kor_morph_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_kor_morph_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_kor_morph_vars,args,1,1);
    }
    return reloq_kokr61_init_kor_morph_vars(state, p0);
}
int32_t reloq_kokr61_init_kor_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_kor_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_kor_symbolic_vars,args,1,1);
    }
    return reloq_kokr61_init_kor_symbolic_vars(state, p0);
}
int32_t reloq_kokr61_init_kor_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_kor_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_kor_vars,args,1,1);
    }
    return reloq_kokr61_init_kor_vars(state, p0);
}
int32_t reloq_kokr61_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_kokr61_init_lang_vars(state, p0);
}
int32_t reloq_kokr61_init_nuclwrd_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_nuclwrd_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_nuclwrd_vars,args,1,1);
    }
    return reloq_kokr61_init_nuclwrd_vars(state, p0);
}
int32_t reloq_kokr61_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_kokr61_init_phr_feature_vars(state, p0);
}
int32_t reloq_kokr61_init_phrase_inton_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_phrase_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_inton_vars,args,1,1);
    }
    return reloq_kokr61_init_phrase_inton_vars(state, p0);
}
int32_t reloq_kokr61_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_kokr61_init_phrase_vars(state, p0);
}
int32_t reloq_kokr61_init_platform(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_kokr61_init_platform(state, p0);
}
int32_t reloq_kokr61_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_kokr61_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_kokr61_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_kokr61_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_kokr61_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_inton_vars,args,1,1);
    }
    return reloq_kokr61_init_syll_inton_vars(state, p0);
}
int32_t reloq_kokr61_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_kokr61_init_syll_vars(state, p0);
}
int32_t reloq_kokr61_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_kokr61_init_symbolic_vars(state, p0);
}
int32_t reloq_kokr61_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_kokr61_init_tm_vars(state, p0);
}
int32_t reloq_kokr61_init_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_kokr61_init_vars(state, p0);
}
int32_t reloq_kokr61_init_voice_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_voice_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_voice_vars,args,1,1);
    }
    return reloq_kokr61_init_voice_vars(state, p0);
}
int32_t reloq_kokr61_init_word(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_kokr61_init_word(state, p0);
}
int32_t reloq_kokr61_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_inton_vars,args,1,1);
    }
    return reloq_kokr61_init_word_inton_vars(state, p0);
}
int32_t reloq_kokr61_init_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_ptrs,args,1,1);
    }
    return reloq_kokr61_init_word_ptrs(state, p0);
}
int32_t reloq_kokr61_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_kokr61_init_word_vars(state, p0);
}
int32_t reloq_kokr61_insert_acc_phrases(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_acc_phrases(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acc_phrases,args,1,1);
    }
    return reloq_kokr61_insert_acc_phrases(state, p0);
}
int32_t reloq_kokr61_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_kokr61_insert_acoustic_vals(state, p0);
}
int32_t reloq_kokr61_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_kokr61_insert_burst_dur(state, p0);
}
int32_t reloq_kokr61_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_kokr61_insert_char_count(state, p0);
}
int32_t reloq_kokr61_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_comma_phrase,args,3,3);
    }
    return reloq_kokr61_insert_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_kokr61_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_insert_f0(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_f0(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_f0,args,4,4);
    }
    return reloq_kokr61_insert_f0(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_insert_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_insert_kor_acc_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_kor_acc_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_kor_acc_phrases,args,3,3);
    }
    return reloq_kokr61_insert_kor_acc_phrases(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_kor_dict_root(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_kor_dict_root(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_kor_dict_root,args,4,4);
    }
    return reloq_kokr61_insert_kor_dict_root(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_insert_kor_large_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_kor_large_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_kor_large_number,args,3,3);
    }
    return reloq_kokr61_insert_kor_large_number(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_kor_nbu(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_kor_nbu(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_kor_nbu,args,3,3);
    }
    return reloq_kokr61_insert_kor_nbu(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_kor_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_kor_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_kor_number,args,3,3);
    }
    return reloq_kokr61_insert_kor_number(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_kor_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_kor_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_kor_spr_phone,args,3,3);
    }
    return reloq_kokr61_insert_kor_spr_phone(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_lang_large_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_lang_large_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_lang_large_number,args,3,3);
    }
    return reloq_kokr61_insert_lang_large_number(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_kokr61_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_non_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_non_phrase_pause,args,3,3);
    }
    return reloq_kokr61_insert_non_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_kokr61_insert_pause_durs(state, p0);
}
int32_t reloq_kokr61_insert_penult_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_kokr61_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_final_tones,args,1,1);
    }
    return reloq_kokr61_insert_phrase_final_tones(state, p0);
}
int32_t reloq_kokr61_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_kokr61_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_preset_phrase_tone(void *, int32_t);
int32_t reloq_kokr61_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_insert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_single_digit,args,3,3);
    }
    return reloq_kokr61_insert_single_digit(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_kokr61_insert_space(state, p0, p1);
}
int32_t reloq_kokr61_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_kokr61_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_kokr61_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_kokr61_insert_syll_level_vals(state, p0);
}
int32_t reloq_kokr61_insert_target_t3(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_target_t3(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_target_t3,args,1,1);
    }
    return reloq_kokr61_insert_target_t3(state, p0);
}
int32_t reloq_kokr61_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_kokr61_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_kokr61_insert_transitions(state, p0);
}
int32_t reloq_kokr61_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_kokr61_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrases,args,4,4);
    }
    return reloq_kokr61_insert_unpuncted_phrases(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_kokr61_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_kokr61_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_kokr61_insert_word(state, p0, p1, p2);
}
int32_t reloq_kokr61_insert_zero_sylls(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_inside_orig_inp_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_inside_orig_inp_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_inside_orig_inp_token,args,2,2);
    }
    return reloq_kokr61_inside_orig_inp_token(state, p0, p1);
}
int32_t reloq_kokr61_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_kokr61_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_kokr61_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_interpret_single_char_modes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_interpret_single_char_modes,args,3,3);
    }
    return reloq_kokr61_interpret_single_char_modes(state, p0, p1, p2);
}
int32_t reloq_kokr61_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_kokr61_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_after_h(void *, int32_t, int32_t);
int32_t reloq_kokr61_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_kokr61_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_hyphenated_word(void *, int32_t, int32_t);
int32_t reloq_kokr61_is_in_parentheses(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_is_kor_acc_phr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_acc_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_acc_phr,args,5,5);
    }
    return reloq_kokr61_is_kor_acc_phr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_acc_phr_type_0(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_acc_phr_type_0(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_acc_phr_type_0,args,2,2);
    }
    return reloq_kokr61_is_kor_acc_phr_type_0(state, p0, p1);
}
int32_t reloq_kokr61_is_kor_acc_phr_type_1(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_acc_phr_type_1(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_acc_phr_type_1,args,2,2);
    }
    return reloq_kokr61_is_kor_acc_phr_type_1(state, p0, p1);
}
int32_t reloq_kokr61_is_kor_acc_phr_type_2(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_acc_phr_type_2(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_acc_phr_type_2,args,2,2);
    }
    return reloq_kokr61_is_kor_acc_phr_type_2(state, p0, p1);
}
int32_t reloq_kokr61_is_kor_acc_phr_type_3(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_acc_phr_type_3(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_acc_phr_type_3,args,2,2);
    }
    return reloq_kokr61_is_kor_acc_phr_type_3(state, p0, p1);
}
int32_t reloq_kokr61_is_kor_acc_phr_type_4(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_acc_phr_type_4(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_acc_phr_type_4,args,2,2);
    }
    return reloq_kokr61_is_kor_acc_phr_type_4(state, p0, p1);
}
int32_t reloq_kokr61_is_kor_adv_adv_phr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_adv_adv_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_adv_adv_phr,args,5,5);
    }
    return reloq_kokr61_is_kor_adv_adv_phr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_adverbs(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_adverbs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_adverbs,args,4,4);
    }
    return reloq_kokr61_is_kor_adverbs(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_is_kor_copular(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_copular(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_copular,args,5,5);
    }
    return reloq_kokr61_is_kor_copular(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_ending_a(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_ending_a(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_ending_a,args,3,3);
    }
    return reloq_kokr61_is_kor_ending_a(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_kor_forgn_forgn_phr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_forgn_forgn_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_forgn_forgn_phr,args,5,5);
    }
    return reloq_kokr61_is_kor_forgn_forgn_phr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_idiomatic_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_idiomatic_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_idiomatic_exp,args,5,5);
    }
    return reloq_kokr61_is_kor_idiomatic_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_modal(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_modal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_modal,args,4,4);
    }
    return reloq_kokr61_is_kor_modal(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_is_kor_month(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_month(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_month,args,2,2);
    }
    return reloq_kokr61_is_kor_month(state, p0, p1);
}
int32_t reloq_kokr61_is_kor_nominal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_nominal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_nominal,args,5,5);
    }
    return reloq_kokr61_is_kor_nominal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_noun_noun_phr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_noun_noun_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_noun_noun_phr,args,5,5);
    }
    return reloq_kokr61_is_kor_noun_noun_phr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_preending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_preending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_preending,args,5,5);
    }
    return reloq_kokr61_is_kor_preending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_pronouns(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_pronouns(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_pronouns,args,4,4);
    }
    return reloq_kokr61_is_kor_pronouns(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_is_kor_target_syll(void *, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_target_syll,args,1,1);
    }
    return reloq_kokr61_is_kor_target_syll(state, p0);
}
int32_t reloq_kokr61_is_kor_time_time_phr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_time_time_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_time_time_phr,args,5,5);
    }
    return reloq_kokr61_is_kor_time_time_phr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb,args,6,6);
    }
    return reloq_kokr61_is_kor_verb(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_is_kor_verb_H_infl(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_H_infl(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_H_infl,args,3,3);
    }
    return reloq_kokr61_is_kor_verb_H_infl(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_kor_verb_L_infl(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_L_infl(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_L_infl,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_L_infl(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb_O_contr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_O_contr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_O_contr,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_O_contr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb_P_infl(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_P_infl(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_P_infl,args,4,4);
    }
    return reloq_kokr61_is_kor_verb_P_infl(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_is_kor_verb_S_infl(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_S_infl(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_S_infl,args,3,3);
    }
    return reloq_kokr61_is_kor_verb_S_infl(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_kor_verb_T_infl(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_T_infl(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_T_infl,args,3,3);
    }
    return reloq_kokr61_is_kor_verb_T_infl(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_kor_verb_U_infl(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_U_infl(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_U_infl,args,3,3);
    }
    return reloq_kokr61_is_kor_verb_U_infl(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_kor_verb_a_contr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_a_contr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_a_contr,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_a_contr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb_e_contr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_e_contr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_e_contr,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_e_contr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb_ha_contr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_ha_contr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_ha_contr,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_ha_contr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb_i_contr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_i_contr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_i_contr,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_i_contr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb_rE_infl(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_rE_infl(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_rE_infl,args,3,3);
    }
    return reloq_kokr61_is_kor_verb_rE_infl(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_kor_verb_rU_infl(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_rU_infl(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_rU_infl,args,3,3);
    }
    return reloq_kokr61_is_kor_verb_rU_infl(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_kor_verb_reg(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_reg(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_reg,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_reg(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb_verb_phr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_verb_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_verb_phr,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_verb_phr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_kor_verb_wa_contr(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_kor_verb_wa_contr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_kor_verb_wa_contr,args,5,5);
    }
    return reloq_kokr61_is_kor_verb_wa_contr(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_is_native_reading_required(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_native_reading_required(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_native_reading_required,args,2,2);
    }
    return reloq_kokr61_is_native_reading_required(state, p0, p1);
}
int32_t reloq_kokr61_is_pathname(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_pathname(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pathname,args,2,2);
    }
    return reloq_kokr61_is_pathname(state, p0, p1);
}
int32_t reloq_kokr61_is_proper_name(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_proper_name(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_proper_name,args,2,2);
    }
    return reloq_kokr61_is_proper_name(state, p0, p1);
}
int32_t reloq_kokr61_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_kokr61_is_spr(state, p0, p1, p2);
}
int32_t reloq_kokr61_is_target_syll(void *, int32_t);
static inline int32_t reloq_call_kokr61_is_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_target_syll,args,1,1);
    }
    return reloq_kokr61_is_target_syll(state, p0);
}
int32_t reloq_kokr61_isitAlt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_isitAlt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitAlt,args,3,3);
    }
    return reloq_kokr61_isitAlt(state, p0, p1, p2);
}
int32_t reloq_kokr61_isitWH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_isitWH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitWH,args,3,3);
    }
    return reloq_kokr61_isitWH(state, p0, p1, p2);
}
int32_t reloq_kokr61_isit_kor_WH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_isit_kor_WH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_kor_WH,args,3,3);
    }
    return reloq_kokr61_isit_kor_WH(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_accPhr_final_syll_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_accPhr_final_syll_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_accPhr_final_syll_dur,args,1,1);
    }
    return reloq_kokr61_kor_accPhr_final_syll_dur(state, p0);
}
int32_t reloq_kokr61_kor_accu(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_accu(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_accu,args,4,4);
    }
    return reloq_kokr61_kor_accu(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_adjust_for_accPhr_len(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_adjust_for_accPhr_len(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_adjust_for_accPhr_len,args,1,1);
    }
    return reloq_kokr61_kor_adjust_for_accPhr_len(state, p0);
}
int32_t reloq_kokr61_kor_adnom(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_adnom(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_adnom,args,4,4);
    }
    return reloq_kokr61_kor_adnom(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_adv(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_adv(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_adv,args,4,4);
    }
    return reloq_kokr61_kor_adv(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_alv_Fv(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_alv_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_alv_Fv,args,1,1);
    }
    return reloq_kokr61_kor_alv_Fv(state, p0);
}
int32_t reloq_kokr61_kor_analyze_morphs1(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_analyze_morphs1(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_analyze_morphs1,args,4,4);
    }
    return reloq_kokr61_kor_analyze_morphs1(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_analyze_morphs2(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_analyze_morphs2(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_analyze_morphs2,args,3,3);
    }
    return reloq_kokr61_kor_analyze_morphs2(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_aspirate(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_aspirate(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_aspirate,args,3,3);
    }
    return reloq_kokr61_kor_aspirate(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_boundary_tone,args,1,1);
    }
    return reloq_kokr61_kor_boundary_tone(state, p0);
}
int32_t reloq_kokr61_kor_break_into_morphs(void *, int32_t);
int32_t reloq_kokr61_kor_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_char_name,args,3,3);
    }
    return reloq_kokr61_kor_char_name(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_coda_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_coda_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_coda_rules,args,3,3);
    }
    return reloq_kokr61_kor_coda_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_con_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_con_vals,args,1,1);
    }
    return reloq_kokr61_kor_con_vals(state, p0);
}
int32_t reloq_kokr61_kor_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_cons_durs,args,3,3);
    }
    return reloq_kokr61_kor_cons_durs(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_continuation_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_continuation_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_continuation_phrase,args,3,3);
    }
    return reloq_kokr61_kor_continuation_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_coord_conj(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_coord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_coord_conj,args,4,4);
    }
    return reloq_kokr61_kor_coord_conj(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_create_syllables(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_create_syllables(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_create_syllables,args,3,3);
    }
    return reloq_kokr61_kor_create_syllables(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_dur_adjustments(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_dur_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_dur_adjustments,args,1,1);
    }
    return reloq_kokr61_kor_dur_adjustments(state, p0);
}
int32_t reloq_kokr61_kor_find_word_to_insert_l(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_find_word_to_insert_l(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_find_word_to_insert_l,args,3,3);
    }
    return reloq_kokr61_kor_find_word_to_insert_l(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_find_word_to_insert_n(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_find_word_to_insert_n(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_find_word_to_insert_n,args,3,3);
    }
    return reloq_kokr61_kor_find_word_to_insert_n(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_find_word_to_lateralize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_find_word_to_lateralize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_find_word_to_lateralize,args,3,3);
    }
    return reloq_kokr61_kor_find_word_to_lateralize(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_find_word_to_neutralize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_find_word_to_neutralize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_find_word_to_neutralize,args,3,3);
    }
    return reloq_kokr61_kor_find_word_to_neutralize(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_find_word_to_palatalize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_find_word_to_palatalize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_find_word_to_palatalize,args,3,3);
    }
    return reloq_kokr61_kor_find_word_to_palatalize(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_find_word_to_resyllabify(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_find_word_to_resyllabify(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_find_word_to_resyllabify,args,3,3);
    }
    return reloq_kokr61_kor_find_word_to_resyllabify(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_find_word_to_tensify(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_find_word_to_tensify(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_find_word_to_tensify,args,3,3);
    }
    return reloq_kokr61_kor_find_word_to_tensify(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_funct_words(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_funct_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_funct_words,args,5,5);
    }
    return reloq_kokr61_kor_funct_words(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_kor_generate_diaphones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_generate_diaphones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_generate_diaphones,args,3,3);
    }
    return reloq_kokr61_kor_generate_diaphones(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_glide_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_glide_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_glide_rules,args,3,3);
    }
    return reloq_kokr61_kor_glide_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_high_low_star_tone,args,1,1);
    }
    return reloq_kokr61_kor_high_low_star_tone(state, p0);
}
int32_t reloq_kokr61_kor_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_high_star_low_tone,args,1,1);
    }
    return reloq_kokr61_kor_high_star_low_tone(state, p0);
}
int32_t reloq_kokr61_kor_high_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_high_tone,args,1,1);
    }
    return reloq_kokr61_kor_high_tone(state, p0);
}
int32_t reloq_kokr61_kor_high_tone_targets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_high_tone_targets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_high_tone_targets,args,3,3);
    }
    return reloq_kokr61_kor_high_tone_targets(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_hyphen_between_nums,args,5,5);
    }
    return reloq_kokr61_kor_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_kor_insert_fortis_obstruents(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_insert_fortis_obstruents(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_insert_fortis_obstruents,args,3,3);
    }
    return reloq_kokr61_kor_insert_fortis_obstruents(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_insert_nasalized_obstruents(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_insert_nasalized_obstruents(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_insert_nasalized_obstruents,args,3,3);
    }
    return reloq_kokr61_kor_insert_nasalized_obstruents(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_insert_voiced_obstruents(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_insert_voiced_obstruents(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_insert_voiced_obstruents,args,3,3);
    }
    return reloq_kokr61_kor_insert_voiced_obstruents(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_insert_voiceless_obstruents(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_insert_voiceless_obstruents(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_insert_voiceless_obstruents,args,3,3);
    }
    return reloq_kokr61_kor_insert_voiceless_obstruents(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_insert_vowels(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_insert_vowels(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_insert_vowels,args,3,3);
    }
    return reloq_kokr61_kor_insert_vowels(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_internal_dict_lookup,args,3,3);
    }
    return reloq_kokr61_kor_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_lab_Fv(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_lab_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_lab_Fv,args,1,1);
    }
    return reloq_kokr61_kor_lab_Fv(state, p0);
}
int32_t reloq_kokr61_kor_lengthen_vowel(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_lengthen_vowel(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_lengthen_vowel,args,3,3);
    }
    return reloq_kokr61_kor_lengthen_vowel(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_lenis_obstruents_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_lenis_obstruents_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_lenis_obstruents_rules,args,3,3);
    }
    return reloq_kokr61_kor_lenis_obstruents_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_low_high_star_tone,args,1,1);
    }
    return reloq_kokr61_kor_low_high_star_tone(state, p0);
}
int32_t reloq_kokr61_kor_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_low_star_high_tone,args,1,1);
    }
    return reloq_kokr61_kor_low_star_high_tone(state, p0);
}
int32_t reloq_kokr61_kor_low_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_low_tone,args,1,1);
    }
    return reloq_kokr61_kor_low_tone(state, p0);
}
int32_t reloq_kokr61_kor_major_phrase_final_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_major_phrase_final_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_major_phrase_final_dur,args,1,1);
    }
    return reloq_kokr61_kor_major_phrase_final_dur(state, p0);
}
int32_t reloq_kokr61_kor_misc_non_phrasal_comma(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_misc_non_phrasal_comma(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_misc_non_phrasal_comma,args,4,4);
    }
    return reloq_kokr61_kor_misc_non_phrasal_comma(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_neutralize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_neutralize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_neutralize,args,3,3);
    }
    return reloq_kokr61_kor_neutralize(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_nom(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_nom(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_nom,args,4,4);
    }
    return reloq_kokr61_kor_nom(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_non_phrasal_comma,args,2,2);
    }
    return reloq_kokr61_kor_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_kokr61_kor_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_kokr61_kor_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_kokr61_kor_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_kokr61_kor_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_kokr61_kor_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_nucl_high_tone,args,1,1);
    }
    return reloq_kokr61_kor_nucl_high_tone(state, p0);
}
int32_t reloq_kokr61_kor_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_kokr61_kor_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_kokr61_kor_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_kokr61_kor_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_kokr61_kor_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_nucl_low_tone,args,1,1);
    }
    return reloq_kokr61_kor_nucl_low_tone(state, p0);
}
int32_t reloq_kokr61_kor_nucleus_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_nucleus_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_nucleus_rules,args,3,3);
    }
    return reloq_kokr61_kor_nucleus_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_onset_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_onset_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_onset_rules,args,3,3);
    }
    return reloq_kokr61_kor_onset_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_pal_Fv,args,1,1);
    }
    return reloq_kokr61_kor_pal_Fv(state, p0);
}
int32_t reloq_kokr61_kor_palatalize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_palatalize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_palatalize,args,3,3);
    }
    return reloq_kokr61_kor_palatalize(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_ph_A(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_A(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_A,args,1,1);
    }
    return reloq_kokr61_kor_ph_A(state, p0);
}
int32_t reloq_kokr61_kor_ph_E(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_E,args,1,1);
    }
    return reloq_kokr61_kor_ph_E(state, p0);
}
int32_t reloq_kokr61_kor_ph_S(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_S,args,1,1);
    }
    return reloq_kokr61_kor_ph_S(state, p0);
}
int32_t reloq_kokr61_kor_ph_SS(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_SS(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_SS,args,1,1);
    }
    return reloq_kokr61_kor_ph_SS(state, p0);
}
int32_t reloq_kokr61_kor_ph_SS_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_SS_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_SS_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_SS_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_S_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_S_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_S_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_S_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_U(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_U(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_U,args,1,1);
    }
    return reloq_kokr61_kor_ph_U(state, p0);
}
int32_t reloq_kokr61_kor_ph_W(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_W(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_W,args,1,1);
    }
    return reloq_kokr61_kor_ph_W(state, p0);
}
int32_t reloq_kokr61_kor_ph_W_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_W_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_W_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_W_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_Z(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_Z,args,1,1);
    }
    return reloq_kokr61_kor_ph_Z(state, p0);
}
int32_t reloq_kokr61_kor_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_Z_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_Z_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_a(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_a,args,1,1);
    }
    return reloq_kokr61_kor_ph_a(state, p0);
}
int32_t reloq_kokr61_kor_ph_b(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_b,args,1,1);
    }
    return reloq_kokr61_kor_ph_b(state, p0);
}
int32_t reloq_kokr61_kor_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_b_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_b_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_d(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_d,args,1,1);
    }
    return reloq_kokr61_kor_ph_d(state, p0);
}
int32_t reloq_kokr61_kor_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_d_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_d_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_e(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_e,args,1,1);
    }
    return reloq_kokr61_kor_ph_e(state, p0);
}
int32_t reloq_kokr61_kor_ph_g(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_g,args,1,1);
    }
    return reloq_kokr61_kor_ph_g(state, p0);
}
int32_t reloq_kokr61_kor_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_g_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_g_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_i(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_i,args,1,1);
    }
    return reloq_kokr61_kor_ph_i(state, p0);
}
int32_t reloq_kokr61_kor_ph_k(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_k,args,1,1);
    }
    return reloq_kokr61_kor_ph_k(state, p0);
}
int32_t reloq_kokr61_kor_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_k_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_k_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_kh(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_kh(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_kh,args,1,1);
    }
    return reloq_kokr61_kor_ph_kh(state, p0);
}
int32_t reloq_kokr61_kor_ph_kh_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_kh_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_kh_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_kh_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_kk(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_kk(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_kk,args,1,1);
    }
    return reloq_kokr61_kor_ph_kk(state, p0);
}
int32_t reloq_kokr61_kor_ph_kk_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_kk_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_kk_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_kk_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_l(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_l,args,1,1);
    }
    return reloq_kokr61_kor_ph_l(state, p0);
}
int32_t reloq_kokr61_kor_ph_l_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_l_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_l_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_l_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_m(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_m,args,1,1);
    }
    return reloq_kokr61_kor_ph_m(state, p0);
}
int32_t reloq_kokr61_kor_ph_m_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_m_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_m_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_m_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_n(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_n,args,1,1);
    }
    return reloq_kokr61_kor_ph_n(state, p0);
}
int32_t reloq_kokr61_kor_ph_n_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_n_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_n_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_ng(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_ng(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_ng,args,1,1);
    }
    return reloq_kokr61_kor_ph_ng(state, p0);
}
int32_t reloq_kokr61_kor_ph_ng_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_ng_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_ng_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_ng_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_o(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_o,args,1,1);
    }
    return reloq_kokr61_kor_ph_o(state, p0);
}
int32_t reloq_kokr61_kor_ph_p(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_p,args,1,1);
    }
    return reloq_kokr61_kor_ph_p(state, p0);
}
int32_t reloq_kokr61_kor_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_p_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_p_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_pal_d(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_pal_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_pal_d,args,1,1);
    }
    return reloq_kokr61_kor_ph_pal_d(state, p0);
}
int32_t reloq_kokr61_kor_ph_pal_d_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_pal_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_pal_d_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_pal_d_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_pal_t(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_pal_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_pal_t,args,1,1);
    }
    return reloq_kokr61_kor_ph_pal_t(state, p0);
}
int32_t reloq_kokr61_kor_ph_pal_t_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_pal_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_pal_t_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_pal_t_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_ph(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_ph(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_ph,args,1,1);
    }
    return reloq_kokr61_kor_ph_ph(state, p0);
}
int32_t reloq_kokr61_kor_ph_ph_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_ph_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_ph_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_ph_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_pp(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_pp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_pp,args,1,1);
    }
    return reloq_kokr61_kor_ph_pp(state, p0);
}
int32_t reloq_kokr61_kor_ph_pp_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_pp_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_pp_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_pp_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_r(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_r,args,1,1);
    }
    return reloq_kokr61_kor_ph_r(state, p0);
}
int32_t reloq_kokr61_kor_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_r_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_r_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_s(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_s,args,1,1);
    }
    return reloq_kokr61_kor_ph_s(state, p0);
}
int32_t reloq_kokr61_kor_ph_s_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_s_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_s_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_s_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_ss(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_ss(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_ss,args,1,1);
    }
    return reloq_kokr61_kor_ph_ss(state, p0);
}
int32_t reloq_kokr61_kor_ph_ss_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_ss_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_ss_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_ss_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_t(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_t,args,1,1);
    }
    return reloq_kokr61_kor_ph_t(state, p0);
}
int32_t reloq_kokr61_kor_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_t_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_t_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_th(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_th(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_th,args,1,1);
    }
    return reloq_kokr61_kor_ph_th(state, p0);
}
int32_t reloq_kokr61_kor_ph_th_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_th_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_th_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_th_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_tt(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_tt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_tt,args,1,1);
    }
    return reloq_kokr61_kor_ph_tt(state, p0);
}
int32_t reloq_kokr61_kor_ph_tt_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_tt_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_tt_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_tt_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_u(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_u,args,1,1);
    }
    return reloq_kokr61_kor_ph_u(state, p0);
}
int32_t reloq_kokr61_kor_ph_w(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_w,args,1,1);
    }
    return reloq_kokr61_kor_ph_w(state, p0);
}
int32_t reloq_kokr61_kor_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_w_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_w_dur(state, p0);
}
int32_t reloq_kokr61_kor_ph_y(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_y,args,1,1);
    }
    return reloq_kokr61_kor_ph_y(state, p0);
}
int32_t reloq_kokr61_kor_ph_y_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_ph_y_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_ph_y_dur,args,1,1);
    }
    return reloq_kokr61_kor_ph_y_dur(state, p0);
}
int32_t reloq_kokr61_kor_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_phr_initial_F0,args,1,1);
    }
    return reloq_kokr61_kor_phr_initial_F0(state, p0);
}
int32_t reloq_kokr61_kor_phr_second_F0(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_phr_second_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_phr_second_F0,args,1,1);
    }
    return reloq_kokr61_kor_phr_second_F0(state, p0);
}
int32_t reloq_kokr61_kor_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_phrase_level_rules,args,3,3);
    }
    return reloq_kokr61_kor_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_phrase_tone,args,1,1);
    }
    return reloq_kokr61_kor_phrase_tone(state, p0);
}
int32_t reloq_kokr61_kor_phrase_type(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_phrase_type(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_phrase_type,args,1,1);
    }
    return reloq_kokr61_kor_phrase_type(state, p0);
}
int32_t reloq_kokr61_kor_places(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_places(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_places,args,3,3);
    }
    return reloq_kokr61_kor_places(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_pnames(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_pnames(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_pnames,args,3,3);
    }
    return reloq_kokr61_kor_pnames(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_position_t_ptrs,args,1,1);
    }
    return reloq_kokr61_kor_position_t_ptrs(state, p0);
}
int32_t reloq_kokr61_kor_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_post_comma_phrase,args,3,3);
    }
    return reloq_kokr61_kor_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_post_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_post_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_post_word_level_rules,args,3,3);
    }
    return reloq_kokr61_kor_post_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_postnucl_high_tone(void *, int32_t);
int32_t reloq_kokr61_kor_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_prenucl_f0,args,1,1);
    }
    return reloq_kokr61_kor_prenucl_f0(state, p0);
}
int32_t reloq_kokr61_kor_resyllabify(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_resyllabify(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_resyllabify,args,3,3);
    }
    return reloq_kokr61_kor_resyllabify(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_roots(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_roots(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_roots,args,3,3);
    }
    return reloq_kokr61_kor_roots(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_roots_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_roots_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_roots_dict_lookup,args,3,3);
    }
    return reloq_kokr61_kor_roots_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_sentential_adverb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_sentential_adverb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_sentential_adverb,args,3,3);
    }
    return reloq_kokr61_kor_sentential_adverb(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_start_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_start_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_start_dur,args,1,1);
    }
    return reloq_kokr61_kor_start_dur(state, p0);
}
int32_t reloq_kokr61_kor_subord_conj(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_subord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_subord_conj,args,4,4);
    }
    return reloq_kokr61_kor_subord_conj(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_symbolic_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_symbolic_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_symbolic_rules,args,3,3);
    }
    return reloq_kokr61_kor_symbolic_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_time_exp(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_time_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_time_exp,args,4,4);
    }
    return reloq_kokr61_kor_time_exp(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_topic(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_topic(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_topic,args,4,4);
    }
    return reloq_kokr61_kor_topic(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_kor_trans_features(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_trans_features,args,1,1);
    }
    return reloq_kokr61_kor_trans_features(state, p0);
}
int32_t reloq_kokr61_kor_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_unpuncted_phr_vals,args,3,3);
    }
    return reloq_kokr61_kor_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_vel_Fv(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_vel_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_vel_Fv,args,1,1);
    }
    return reloq_kokr61_kor_vel_Fv(state, p0);
}
int32_t reloq_kokr61_kor_vowel_lengthening(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_vowel_lengthening(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_vowel_lengthening,args,1,1);
    }
    return reloq_kokr61_kor_vowel_lengthening(state, p0);
}
int32_t reloq_kokr61_kor_vowel_shortening(void *, int32_t);
static inline int32_t reloq_call_kokr61_kor_vowel_shortening(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_kor_vowel_shortening,args,1,1);
    }
    return reloq_kokr61_kor_vowel_shortening(state, p0);
}
int32_t reloq_kokr61_kor_word_cat_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_word_cat_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_word_cat_anno,args,3,3);
    }
    return reloq_kokr61_kor_word_cat_anno(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_word_final_stop(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_word_final_stop(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_word_final_stop,args,5,5);
    }
    return reloq_kokr61_kor_word_final_stop(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_kor_word_init_con(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_word_init_con(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_word_init_con,args,5,5);
    }
    return reloq_kokr61_kor_word_init_con(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_kor_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words,args,3,3);
    }
    return reloq_kokr61_kor_words(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_dict_lookup,args,3,3);
    }
    return reloq_kokr61_kor_words_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_no_insert_l_bef_y(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_no_insert_l_bef_y(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_no_insert_l_bef_y,args,3,3);
    }
    return reloq_kokr61_kor_words_no_insert_l_bef_y(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_no_insert_n_bef_y(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_no_insert_n_bef_y(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_no_insert_n_bef_y,args,3,3);
    }
    return reloq_kokr61_kor_words_no_insert_n_bef_y(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_not_to_palatalize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_not_to_palatalize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_not_to_palatalize,args,3,3);
    }
    return reloq_kokr61_kor_words_not_to_palatalize(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_not_to_resyllabify(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_not_to_resyllabify(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_not_to_resyllabify,args,3,3);
    }
    return reloq_kokr61_kor_words_not_to_resyllabify(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_to_insert_l_bef_i(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_to_insert_l_bef_i(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_to_insert_l_bef_i,args,3,3);
    }
    return reloq_kokr61_kor_words_to_insert_l_bef_i(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_to_insert_n_bef_i(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_to_insert_n_bef_i(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_to_insert_n_bef_i,args,3,3);
    }
    return reloq_kokr61_kor_words_to_insert_n_bef_i(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_to_lateralize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_to_lateralize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_to_lateralize,args,3,3);
    }
    return reloq_kokr61_kor_words_to_lateralize(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_to_neutralize(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_to_neutralize(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_to_neutralize,args,3,3);
    }
    return reloq_kokr61_kor_words_to_neutralize(state, p0, p1, p2);
}
int32_t reloq_kokr61_kor_words_to_tensify(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_kor_words_to_tensify(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_kor_words_to_tensify,args,3,3);
    }
    return reloq_kokr61_kor_words_to_tensify(state, p0, p1, p2);
}
int32_t reloq_kokr61_lang_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_lang_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_hyphen_between_nums,args,5,5);
    }
    return reloq_kokr61_lang_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_lang_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_lang_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_non_phrasal_comma,args,2,2);
    }
    return reloq_kokr61_lang_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_kokr61_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_lang_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_phrase_level_rules,args,3,3);
    }
    return reloq_kokr61_lang_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_kokr61_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_kokr61_lang_word_cat_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_lang_word_cat_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_word_cat_anno,args,3,3);
    }
    return reloq_kokr61_lang_word_cat_anno(state, p0, p1, p2);
}
int32_t reloq_kokr61_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_kokr61_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_kokr61_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_kokr61_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_kokr61_longdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_longdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_longdash,args,3,3);
    }
    return reloq_kokr61_longdash(state, p0, p1, p2);
}
int32_t reloq_kokr61_lookup_kor_ending_a(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_lookup_kor_ending_a(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lookup_kor_ending_a,args,5,5);
    }
    return reloq_kokr61_lookup_kor_ending_a(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_high_star_tone,args,1,1);
    }
    return reloq_kokr61_low_high_star_tone(state, p0);
}
int32_t reloq_kokr61_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_star_high_tone,args,1,1);
    }
    return reloq_kokr61_low_star_high_tone(state, p0);
}
int32_t reloq_kokr61_low_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_tone,args,1,1);
    }
    return reloq_kokr61_low_tone(state, p0);
}
int32_t reloq_kokr61_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_mark_acc_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_acc_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_acc_phrases,args,3,3);
    }
    return reloq_kokr61_mark_acc_phrases(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_content_word,args,5,5);
    }
    return reloq_kokr61_mark_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_mark_function_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_function_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_function_word,args,5,5);
    }
    return reloq_kokr61_mark_function_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_mark_h(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_h(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_h,args,3,3);
    }
    return reloq_kokr61_mark_h(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_adnominals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_adnominals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_adnominals,args,3,3);
    }
    return reloq_kokr61_mark_kor_adnominals(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_adverbs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_adverbs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_adverbs,args,3,3);
    }
    return reloq_kokr61_mark_kor_adverbs(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_adverbs_sentential(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_adverbs_sentential(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_adverbs_sentential,args,3,3);
    }
    return reloq_kokr61_mark_kor_adverbs_sentential(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_ambiguous_nouns(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_ambiguous_nouns(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_ambiguous_nouns,args,3,3);
    }
    return reloq_kokr61_mark_kor_ambiguous_nouns(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_bound_nouns(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_bound_nouns(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_bound_nouns,args,4,4);
    }
    return reloq_kokr61_mark_kor_bound_nouns(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_mark_kor_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_content_word,args,5,5);
    }
    return reloq_kokr61_mark_kor_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_mark_kor_ending_a(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_ending_a(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_ending_a,args,5,5);
    }
    return reloq_kokr61_mark_kor_ending_a(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_mark_kor_foreign_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_foreign_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_foreign_word,args,3,3);
    }
    return reloq_kokr61_mark_kor_foreign_word(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_interjections(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_interjections(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_interjections,args,3,3);
    }
    return reloq_kokr61_mark_kor_interjections(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_particle(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_particle(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_particle,args,5,5);
    }
    return reloq_kokr61_mark_kor_particle(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_mark_kor_pronouns(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_pronouns(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_pronouns,args,3,3);
    }
    return reloq_kokr61_mark_kor_pronouns(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_stressed_syllable(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_stressed_syllable(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_stressed_syllable,args,2,2);
    }
    return reloq_kokr61_mark_kor_stressed_syllable(state, p0, p1);
}
int32_t reloq_kokr61_mark_kor_subcategory(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_subcategory,args,3,3);
    }
    return reloq_kokr61_mark_kor_subcategory(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_kor_verb_ending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_verb_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_verb_ending,args,5,5);
    }
    return reloq_kokr61_mark_kor_verb_ending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_mark_kor_words_without_clitics(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_kor_words_without_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_kor_words_without_clitics,args,4,4);
    }
    return reloq_kokr61_mark_kor_words_without_clitics(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_mark_non_letter(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_kokr61_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_mark_potential_phrase_breaks(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_potential_phrase_breaks(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_potential_phrase_breaks,args,4,4);
    }
    return reloq_kokr61_mark_potential_phrase_breaks(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_mark_subcategory(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_subcategory,args,4,4);
    }
    return reloq_kokr61_mark_subcategory(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_mark_syll_stress(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_mark_uppercase(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_mark_word_categories(void *, int32_t);
static inline int32_t reloq_call_kokr61_mark_word_categories(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_categories,args,1,1);
    }
    return reloq_kokr61_mark_word_categories(state, p0);
}
int32_t reloq_kokr61_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_word_str_annot(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_str_annot,args,3,3);
    }
    return reloq_kokr61_mark_word_str_annot(state, p0, p1, p2);
}
int32_t reloq_kokr61_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_kokr61_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_kokr61_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_kokr61_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_kokr61_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_kokr61_misc_chars(state, p0, p1, p2);
}
int32_t reloq_kokr61_modify_breathiness_for_femV(void *, int32_t);
int32_t reloq_kokr61_modulate_asp_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_modulate_asp_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_asp_dur,args,1,1);
    }
    return reloq_kokr61_modulate_asp_dur(state, p0);
}
int32_t reloq_kokr61_modulate_fortis_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_modulate_fortis_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_fortis_dur,args,1,1);
    }
    return reloq_kokr61_modulate_fortis_dur(state, p0);
}
int32_t reloq_kokr61_modulate_lenis_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_modulate_lenis_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_lenis_dur,args,1,1);
    }
    return reloq_kokr61_modulate_lenis_dur(state, p0);
}
int32_t reloq_kokr61_modulate_lenis_voiced_dur(void *, int32_t);
static inline int32_t reloq_call_kokr61_modulate_lenis_voiced_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_lenis_voiced_dur,args,1,1);
    }
    return reloq_kokr61_modulate_lenis_voiced_dur(state, p0);
}
int32_t reloq_kokr61_modulate_nas_AV(void *, int32_t);
static inline int32_t reloq_call_kokr61_modulate_nas_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_nas_AV,args,1,1);
    }
    return reloq_kokr61_modulate_nas_AV(state, p0);
}
int32_t reloq_kokr61_modulate_noise(void *, int32_t);
static inline int32_t reloq_call_kokr61_modulate_noise(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_noise,args,1,1);
    }
    return reloq_kokr61_modulate_noise(state, p0);
}
int32_t reloq_kokr61_modulate_pwind(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_modulate_pwind(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_pwind,args,2,2);
    }
    return reloq_kokr61_modulate_pwind(state, p0, p1);
}
int32_t reloq_kokr61_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_kokr61_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_nasalize_vow(void *, int32_t);
static inline int32_t reloq_call_kokr61_nasalize_vow(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nasalize_vow,args,1,1);
    }
    return reloq_kokr61_nasalize_vow(state, p0);
}
int32_t reloq_kokr61_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_kokr61_no_readin(state, p0, p1);
}
int32_t reloq_kokr61_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_kokr61_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_kokr61_no_words_in_delta(state, p0);
}
int32_t reloq_kokr61_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_kokr61_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_kokr61_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_kokr61_normalize_text(state, p0, p1, p2);
}
int32_t reloq_kokr61_noun_that_makes_inton_phr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_noun_that_makes_inton_phr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_that_makes_inton_phr,args,3,3);
    }
    return reloq_kokr61_noun_that_makes_inton_phr(state, p0, p1, p2);
}
int32_t reloq_kokr61_nucl_f0(void *, int32_t);
static inline int32_t reloq_call_kokr61_nucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_f0,args,1,1);
    }
    return reloq_kokr61_nucl_f0(state, p0);
}
int32_t reloq_kokr61_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_kokr61_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_kokr61_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_kokr61_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_kokr61_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_tone,args,1,1);
    }
    return reloq_kokr61_nucl_high_tone(state, p0);
}
int32_t reloq_kokr61_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_kokr61_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_kokr61_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_kokr61_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_kokr61_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_tone,args,1,1);
    }
    return reloq_kokr61_nucl_low_tone(state, p0);
}
int32_t reloq_kokr61_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_kokr61_nucleus_vals(state, p0);
}
int32_t reloq_kokr61_num_sylls_in_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_num_sylls_in_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_sylls_in_word,args,4,4);
    }
    return reloq_kokr61_num_sylls_in_word(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_kokr61_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_kokr61_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_parenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_parenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_parenthesis,args,3,3);
    }
    return reloq_kokr61_parenthesis(state, p0, p1, p2);
}
int32_t reloq_kokr61_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_kokr61_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_kokr61_period_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_kokr61_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_kokr61_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phr_initial_F0,args,1,1);
    }
    return reloq_kokr61_phr_initial_F0(state, p0);
}
int32_t reloq_kokr61_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_kokr61_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_phrase_initial_k_values(void *, int32_t);
static inline int32_t reloq_call_kokr61_phrase_initial_k_values(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_initial_k_values,args,1,1);
    }
    return reloq_kokr61_phrase_initial_k_values(state, p0);
}
int32_t reloq_kokr61_phrase_initial_p_values(void *, int32_t);
static inline int32_t reloq_call_kokr61_phrase_initial_p_values(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_initial_p_values,args,1,1);
    }
    return reloq_kokr61_phrase_initial_p_values(state, p0);
}
int32_t reloq_kokr61_phrase_initial_t_values(void *, int32_t);
static inline int32_t reloq_call_kokr61_phrase_initial_t_values(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_initial_t_values,args,1,1);
    }
    return reloq_kokr61_phrase_initial_t_values(state, p0);
}
int32_t reloq_kokr61_phrase_level_printouts(void *, int32_t);
static inline int32_t reloq_call_kokr61_phrase_level_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_printouts,args,1,1);
    }
    return reloq_kokr61_phrase_level_printouts(state, p0);
}
int32_t reloq_kokr61_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_kokr61_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_tone,args,1,1);
    }
    return reloq_kokr61_phrase_tone(state, p0);
}
int32_t reloq_kokr61_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_kokr61_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_position_t_ptrs,args,1,1);
    }
    return reloq_kokr61_position_t_ptrs(state, p0);
}
int32_t reloq_kokr61_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_comma_phrase,args,3,3);
    }
    return reloq_kokr61_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_kokr61_potential_sent_end(state, p0, p1);
}
int32_t reloq_kokr61_precedes_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_precedes_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_end_quote,args,2,2);
    }
    return reloq_kokr61_precedes_end_quote(state, p0, p1);
}
int32_t reloq_kokr61_precedes_number(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_kokr61_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_prenucl_f0,args,1,1);
    }
    return reloq_kokr61_prenucl_f0(state, p0);
}
int32_t reloq_kokr61_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_kokr61_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_kokr61_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_kokr61_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_kokr61_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_kokr61_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_kokr61_print_interword_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_print_interword_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_interword_chars,args,3,3);
    }
    return reloq_kokr61_print_interword_chars(state, p0, p1, p2);
}
int32_t reloq_kokr61_print_kor_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_print_kor_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_kor_SPR,args,3,3);
    }
    return reloq_kokr61_print_kor_SPR(state, p0, p1, p2);
}
int32_t reloq_kokr61_print_normalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_print_normalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_normalized_inp,args,3,3);
    }
    return reloq_kokr61_print_normalized_inp(state, p0, p1, p2);
}
int32_t reloq_kokr61_print_prompt(void *, int32_t);
static inline int32_t reloq_call_kokr61_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_kokr61_print_prompt(state, p0);
}
int32_t reloq_kokr61_print_spr(void *, int32_t);
static inline int32_t reloq_call_kokr61_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_kokr61_print_spr(state, p0);
}
int32_t reloq_kokr61_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_kokr61_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_kokr61_print_word_stress(state, p0, p1);
}
int32_t reloq_kokr61_printouts(void *, int32_t);
static inline int32_t reloq_call_kokr61_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_kokr61_printouts(state, p0);
}
int32_t reloq_kokr61_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_kokr61_process_annotation(state, p0, p1, p2);
}
int32_t reloq_kokr61_process_final_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_process_final_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_apostrophe,args,3,3);
    }
    return reloq_kokr61_process_final_apostrophe(state, p0, p1, p2);
}
int32_t reloq_kokr61_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_process_final_parenthetical(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_parenthetical,args,3,3);
    }
    return reloq_kokr61_process_final_parenthetical(state, p0, p1, p2);
}
int32_t reloq_kokr61_process_hostname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_process_hostname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_hostname,args,3,3);
    }
    return reloq_kokr61_process_hostname(state, p0, p1, p2);
}
int32_t reloq_kokr61_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_kokr61_process_input(state, p0, p1);
}
int32_t reloq_kokr61_process_pathname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_process_pathname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_pathname,args,3,3);
    }
    return reloq_kokr61_process_pathname(state, p0, p1, p2);
}
int32_t reloq_kokr61_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_kokr61_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_kokr61_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_kokr61_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_kokr61_punctuation(state, p0, p1, p2);
}
int32_t reloq_kokr61_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_kokr61_question_phrase(state, p0, p1, p2);
}
int32_t reloq_kokr61_quote_mark(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_quote_mark(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_quote_mark,args,3,3);
    }
    return reloq_kokr61_quote_mark(state, p0, p1, p2);
}
int32_t reloq_kokr61_read_end(void *, int32_t);
static inline int32_t reloq_call_kokr61_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_kokr61_read_end(state, p0);
}
int32_t reloq_kokr61_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_kokr61_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_kokr61_realize_prenucl_tone(void *, int32_t);
static inline int32_t reloq_call_kokr61_realize_prenucl_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_realize_prenucl_tone,args,1,1);
    }
    return reloq_kokr61_realize_prenucl_tone(state, p0);
}
int32_t reloq_kokr61_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_kokr61_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_kokr61_register_SAPI_voices(state, p0);
}
int32_t reloq_kokr61_register_kor_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_register_kor_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_kor_phones,args,3,3);
    }
    return reloq_kokr61_register_kor_phones(state, p0, p1, p2);
}
int32_t reloq_kokr61_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_kokr61_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_reset_enum_ptr(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_reset_enum_ptr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_enum_ptr,args,2,2);
    }
    return reloq_kokr61_reset_enum_ptr(state, p0, p1);
}
int32_t reloq_kokr61_reset_gcat_vars(void *, int32_t);
int32_t reloq_kokr61_reset_kor_sent_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_reset_kor_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_kor_sent_vars,args,1,1);
    }
    return reloq_kokr61_reset_kor_sent_vars(state, p0);
}
int32_t reloq_kokr61_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_kokr61_reset_lang_sent_vars(state, p0);
}
int32_t reloq_kokr61_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_kokr61_reset_phrase_vars(state, p0);
}
int32_t reloq_kokr61_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_kokr61_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_kokr61_reset_sent_vars(state, p0);
}
int32_t reloq_kokr61_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_kokr61_reset_token_vars(state, p0, p1);
}
int32_t reloq_kokr61_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_kokr61_reset_word_vals(state, p0, p1);
}
int32_t reloq_kokr61_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_kokr61_run_filters(state, p0, p1, p2);
}
int32_t reloq_kokr61_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_kokr61_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_kokr61_run_speech_module(state, p0);
}
int32_t reloq_kokr61_run_text_module(void *, int32_t);
static inline int32_t reloq_call_kokr61_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_kokr61_run_text_module(state, p0);
}
int32_t reloq_kokr61_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_kokr61_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_kokr61_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_kokr61_scale_word_final_trans(state, p0);
}
int32_t reloq_kokr61_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_kokr61_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_kokr61_semicolon_phrase(state, p0);
}
int32_t reloq_kokr61_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_kokr61_separate_tokens(state, p0, p1);
}
int32_t reloq_kokr61_set_acc_phrase_types(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_set_acc_phrase_types(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_acc_phrase_types,args,3,3);
    }
    return reloq_kokr61_set_acc_phrase_types(state, p0, p1, p2);
}
int32_t reloq_kokr61_set_adjacent_word_ptrs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_set_adjacent_word_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_adjacent_word_ptrs,args,3,3);
    }
    return reloq_kokr61_set_adjacent_word_ptrs(state, p0, p1, p2);
}
int32_t reloq_kokr61_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_kokr61_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_kokr61_set_f0_range_ptrs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_set_f0_range_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_range_ptrs,args,5,5);
    }
    return reloq_kokr61_set_f0_range_ptrs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_kokr61_set_f0_wind_and_base(state, p0);
}
int32_t reloq_kokr61_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_kokr61_set_global_constants(state, p0);
}
int32_t reloq_kokr61_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_kokr61_set_klattparms(state, p0);
}
int32_t reloq_kokr61_set_kor_global_constants(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_kor_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_kor_global_constants,args,1,1);
    }
    return reloq_kokr61_set_kor_global_constants(state, p0);
}
int32_t reloq_kokr61_set_kor_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_set_kor_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_kor_min_token_vals,args,4,4);
    }
    return reloq_kokr61_set_kor_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_kokr61_set_lang_global_constants(state, p0);
}
int32_t reloq_kokr61_set_letter_name_wordcats(void *, int32_t);
int32_t reloq_kokr61_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_kokr61_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_kokr61_set_num_formants(state, p0);
}
int32_t reloq_kokr61_set_p_burst(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_p_burst(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_p_burst,args,1,1);
    }
    return reloq_kokr61_set_p_burst(state, p0);
}
int32_t reloq_kokr61_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_kokr61_set_phone_offset_vals(state, p0);
}
int32_t reloq_kokr61_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_kokr61_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_kokr61_set_said_phr_vals(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_kokr61_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_kokr61_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_kokr61_set_syll_offset_vals(state, p0);
}
int32_t reloq_kokr61_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_kokr61_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_kokr61_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_kokr61_set_voice_param_constants(state, p0);
}
int32_t reloq_kokr61_set_window_size(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_kokr61_set_window_size(state, p0);
}
int32_t reloq_kokr61_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_kokr61_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_kokr61_setdur(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_kokr61_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_kokr61_shift_tone(void *, int32_t, int32_t);
int32_t reloq_kokr61_shortdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_shortdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_shortdash,args,3,3);
    }
    return reloq_kokr61_shortdash(state, p0, p1, p2);
}
int32_t reloq_kokr61_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_kokr61_single_chars(state, p0, p1, p2);
}
int32_t reloq_kokr61_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_kokr61_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_kokr61_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_kokr61_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_kokr61_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_kokr61_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_kokr61_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_kokr61_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_kokr61_single_letter(state, p0, p1, p2);
}
int32_t reloq_kokr61_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_kokr61_size_filter(state, p0, p1, p2);
}
int32_t reloq_kokr61_skip_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_skip_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_delimiters,args,2,2);
    }
    return reloq_kokr61_skip_delimiters(state, p0, p1);
}
int32_t reloq_kokr61_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_kokr61_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_kokr61_slash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_slash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash,args,3,3);
    }
    return reloq_kokr61_slash(state, p0, p1, p2);
}
int32_t reloq_kokr61_slash_before_measure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_slash_before_measure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash_before_measure,args,3,3);
    }
    return reloq_kokr61_slash_before_measure(state, p0, p1, p2);
}
int32_t reloq_kokr61_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_kokr61_space(state, p0, p1, p2);
}
int32_t reloq_kokr61_span_inverted_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_span_inverted_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_span_inverted_punct,args,3,3);
    }
    return reloq_kokr61_span_inverted_punct(state, p0, p1, p2);
}
int32_t reloq_kokr61_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_kokr61_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_kokr61_spell_out_large_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_spell_out_large_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_large_num,args,3,3);
    }
    return reloq_kokr61_spell_out_large_num(state, p0, p1, p2);
}
int32_t reloq_kokr61_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_kokr61_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_kokr61_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_kokr61_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_kokr61_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_stankor_vals(void *, int32_t);
static inline int32_t reloq_call_kokr61_stankor_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stankor_vals,args,1,1);
    }
    return reloq_kokr61_stankor_vals(state, p0);
}
int32_t reloq_kokr61_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_kokr61_stretch_focus(state, p0, p1);
}
int32_t reloq_kokr61_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_kokr61_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_string_to_short(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_short,args,4,4);
    }
    return reloq_kokr61_string_to_short(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_strip_clitics(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_strip_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_clitics,args,5,5);
    }
    return reloq_kokr61_strip_clitics(state, p0, p1, p2, p3, p4);
}
int32_t reloq_kokr61_strip_kor_clitics(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_strip_kor_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_kor_clitics,args,4,4);
    }
    return reloq_kokr61_strip_kor_clitics(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_strip_last_digit(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_kokr61_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_kokr61_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_kokr61_text_index(state, p0, p1);
}
int32_t reloq_kokr61_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_kokr61_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_three_letter_extension(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_three_letter_extension(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_letter_extension,args,2,2);
    }
    return reloq_kokr61_three_letter_extension(state, p0, p1);
}
int32_t reloq_kokr61_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict,args,3,3);
    }
    return reloq_kokr61_tok_dict(state, p0, p1, p2);
}
int32_t reloq_kokr61_tok_dict_entry(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_tok_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict_entry,args,3,3);
    }
    return reloq_kokr61_tok_dict_entry(state, p0, p1, p2);
}
int32_t reloq_kokr61_tok_lookup(void *, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_tok_lookup(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_lookup,args,2,2);
    }
    return reloq_kokr61_tok_lookup(state, p0, p1);
}
int32_t reloq_kokr61_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_kokr61_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_kokr61_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_kokr61_token_to_number(state, p0, p1, p2);
}
int32_t reloq_kokr61_user_abbr_dict(void *, int32_t, int32_t, int32_t);
int32_t reloq_kokr61_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_kokr61_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_user_index_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_user_index_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_index_anno,args,3,3);
    }
    return reloq_kokr61_user_index_anno(state, p0, p1, p2);
}
int32_t reloq_kokr61_user_rootdict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_user_rootdict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_rootdict,args,3,3);
    }
    return reloq_kokr61_user_rootdict(state, p0, p1, p2);
}
int32_t reloq_kokr61_valid_f0_posn(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_valid_f0_posn(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_f0_posn,args,3,3);
    }
    return reloq_kokr61_valid_f0_posn(state, p0, p1, p2);
}
int32_t reloq_kokr61_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_kokr61_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_kokr61_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_kokr61_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_which_get_non_vowels(void *, int32_t);
static inline int32_t reloq_call_kokr61_which_get_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_which_get_non_vowels,args,1,1);
    }
    return reloq_kokr61_which_get_non_vowels(state, p0);
}
int32_t reloq_kokr61_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_kokr61_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_word_cat_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_word_cat_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_cat_anno,args,3,3);
    }
    return reloq_kokr61_word_cat_anno(state, p0, p1, p2);
}
int32_t reloq_kokr61_word_initial_h(void *, int32_t);
static inline int32_t reloq_call_kokr61_word_initial_h(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_initial_h,args,1,1);
    }
    return reloq_kokr61_word_initial_h(state, p0);
}
int32_t reloq_kokr61_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_kokr61_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_kokr61_words_that_makes_inton_phr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_words_that_makes_inton_phr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_words_that_makes_inton_phr,args,4,4);
    }
    return reloq_kokr61_words_that_makes_inton_phr(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_kokr61_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_kokr61_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_kokr61_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_kokr61_year_mode_anno(state, p0, p1, p2, p3);
}
#endif
