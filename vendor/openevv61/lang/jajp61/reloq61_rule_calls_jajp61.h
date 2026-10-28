/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_JAJP61_H
#define RELOQ61_RULE_CALLS_JAJP61_H
#include "delta_rules_c.h"
extern int delta_rule_trace;
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_jajp61_DeltaProc_end(void *, int32_t);
int32_t reloq_jajp61_DeltaProc_flush(void *, int32_t);
int32_t reloq_jajp61_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_jajp61_DeltaProc_main(void *, int32_t);
int32_t reloq_jajp61_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_jajp61_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_jajp61_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_jajp61_DeltaProc_start(void *, int32_t);
int32_t reloq_jajp61_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_jajp61_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_jajp61_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_jajp61_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_abbr_before_num(void *, int32_t, int32_t);
int32_t reloq_jajp61_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_jajp61_absval(state, p0, p1, p2);
}
int32_t reloq_jajp61_accent_adjustments(void *, int32_t);
static inline int32_t reloq_call_jajp61_accent_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_accent_adjustments,args,1,1);
    }
    return reloq_jajp61_accent_adjustments(state, p0);
}
int32_t reloq_jajp61_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_jajp61_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_jajp61_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_add_f0_for_new_voice(void *, int32_t);
static inline int32_t reloq_call_jajp61_add_f0_for_new_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_new_voice,args,1,1);
    }
    return reloq_jajp61_add_f0_for_new_voice(state, p0);
}
int32_t reloq_jajp61_add_f0_for_next_voice(void *, int32_t);
static inline int32_t reloq_call_jajp61_add_f0_for_next_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_next_voice,args,1,1);
    }
    return reloq_jajp61_add_f0_for_next_voice(state, p0);
}
int32_t reloq_jajp61_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_jajp61_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_jajp61_add_tilt(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_add_tilt(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_tilt,args,2,2);
    }
    return reloq_jajp61_add_tilt(state, p0, p1);
}
int32_t reloq_jajp61_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_jajp61_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_jajp61_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_jajp61_adjust_AV_for_fem_voice(void *, int32_t);
static inline int32_t reloq_call_jajp61_adjust_AV_for_fem_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_fem_voice,args,1,1);
    }
    return reloq_jajp61_adjust_AV_for_fem_voice(state, p0);
}
int32_t reloq_jajp61_adjust_AV_for_misc_segments(void *, int32_t);
static inline int32_t reloq_call_jajp61_adjust_AV_for_misc_segments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_misc_segments,args,1,1);
    }
    return reloq_jajp61_adjust_AV_for_misc_segments(state, p0);
}
int32_t reloq_jajp61_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_jajp61_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_adjust_female_breathiness(void *, int32_t);
static inline int32_t reloq_call_jajp61_adjust_female_breathiness(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_female_breathiness,args,1,1);
    }
    return reloq_jajp61_adjust_female_breathiness(state, p0);
}
int32_t reloq_jajp61_adjust_nuc_AV_for_unstr(void *, int32_t);
static inline int32_t reloq_call_jajp61_adjust_nuc_AV_for_unstr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_nuc_AV_for_unstr,args,1,1);
    }
    return reloq_jajp61_adjust_nuc_AV_for_unstr(state, p0);
}
int32_t reloq_jajp61_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_jajp61_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_jajp61_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_jajp61_apostrophe(state, p0, p1, p2);
}
int32_t reloq_jajp61_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_jajp61_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_jajp61_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_jajp61_apply_head_size_val(state, p0, p1);
}
int32_t reloq_jajp61_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_jajp61_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_jajp61_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_jajp61_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_jajp61_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_jajp61_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_jajp61_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_jajp61_apply_roughness_val(state, p0, p1);
}
int32_t reloq_jajp61_apply_rules(void *, int32_t);
static inline int32_t reloq_call_jajp61_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_jajp61_apply_rules(state, p0);
}
int32_t reloq_jajp61_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_jajp61_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_jajp61_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_jajp61_apply_speed_val(state, p0, p1);
}
int32_t reloq_jajp61_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_jajp61_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_jajp61_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_jajp61_apply_volume_val(state, p0, p1);
}
int32_t reloq_jajp61_assign_accent_val(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_assign_accent_val(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_accent_val,args,3,3);
    }
    return reloq_jajp61_assign_accent_val(state, p0, p1, p2);
}
int32_t reloq_jajp61_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_jajp61_assign_acoustic_vals(state, p0);
}
int32_t reloq_jajp61_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_jajp61_assign_alt_quest_features(state, p0);
}
int32_t reloq_jajp61_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_jajp61_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_jajp61_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_jajp61_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_jajp61_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_jajp61_assign_default_quest_features(state, p0);
}
int32_t reloq_jajp61_assign_jap_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_jap_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_jap_nuc_durs,args,1,1);
    }
    return reloq_jajp61_assign_jap_nuc_durs(state, p0);
}
int32_t reloq_jajp61_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_jajp61_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_jajp61_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_jajp61_assign_nuc_durs(state, p0);
}
int32_t reloq_jajp61_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_jajp61_assign_phone_durs(state, p0);
}
int32_t reloq_jajp61_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_jajp61_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_jajp61_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_jajp61_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_jajp61_assign_sent_final_features(state, p0);
}
int32_t reloq_jajp61_assign_syll_coda(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_assign_syll_coda(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_syll_coda,args,2,2);
    }
    return reloq_jajp61_assign_syll_coda(state, p0, p1);
}
int32_t reloq_jajp61_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_jajp61_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_jajp61_assign_user_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_jajp61_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_jajp61_assign_wh_quest_features(state, p0);
}
int32_t reloq_jajp61_beg_of_word(void *, int32_t, int32_t);
int32_t reloq_jajp61_boundary_tone(void *, int32_t);
int32_t reloq_jajp61_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_jajp61_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_jajp61_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_jajp61_build_phrases(state, p0, p1, p2);
}
int32_t reloq_jajp61_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_jajp61_build_words(state, p0, p1, p2);
}
int32_t reloq_jajp61_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_jajp61_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_jajp61_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_jajp61_char_name(state, p0, p1, p2);
}
int32_t reloq_jajp61_clear_delta(void *, int32_t);
static inline int32_t reloq_call_jajp61_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_jajp61_clear_delta(state, p0);
}
int32_t reloq_jajp61_close_platform(void *, int32_t);
static inline int32_t reloq_call_jajp61_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_jajp61_close_platform(state, p0);
}
int32_t reloq_jajp61_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_jajp61_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_jajp61_comma_phrase(state, p0);
}
int32_t reloq_jajp61_con_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_jajp61_con_vals(state, p0);
}
int32_t reloq_jajp61_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_jajp61_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_jajp61_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_jajp61_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_jajp61_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_jajp61_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_jajp61_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_jajp61_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_jajp61_count_input(state, p0, p1);
}
int32_t reloq_jajp61_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_jajp61_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_jajp61_count_normalized_inp(state, p0);
}
int32_t reloq_jajp61_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_count_sylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_sylls,args,4,4);
    }
    return reloq_jajp61_count_sylls(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_jajp61_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_jajp61_count_user_indices(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_count_user_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_user_indices,args,4,4);
    }
    return reloq_jajp61_count_user_indices(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_jajp61_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_jajp61_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_jajp61_create_sentence(state, p0, p1);
}
int32_t reloq_jajp61_create_syllables(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_create_syllables(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_syllables,args,3,3);
    }
    return reloq_jajp61_create_syllables(state, p0, p1, p2);
}
int32_t reloq_jajp61_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_jajp61_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_jajp61_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_jajp61_delete_2_more(state, p0);
}
int32_t reloq_jajp61_delete_comma_sync(void *, int32_t, int32_t);
int32_t reloq_jajp61_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_jajp61_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_jajp61_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_jajp61_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_jajp61_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_delete_phone_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_right,args,3,3);
    }
    return reloq_jajp61_delete_phone_from_right(state, p0, p1, p2);
}
int32_t reloq_jajp61_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_jajp61_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_jajp61_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_jajp61_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_jajp61_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_jajp61_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_jajp61_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_jajp61_demote_word(state, p0, p1, p2);
}
int32_t reloq_jajp61_devoice_jap_vowel(void *, int32_t);
static inline int32_t reloq_call_jajp61_devoice_jap_vowel(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_devoice_jap_vowel,args,1,1);
    }
    return reloq_jajp61_devoice_jap_vowel(state, p0);
}
int32_t reloq_jajp61_devoiced_vowel_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_devoiced_vowel_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_devoiced_vowel_dur,args,1,1);
    }
    return reloq_jajp61_devoiced_vowel_dur(state, p0);
}
int32_t reloq_jajp61_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_jajp61_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_jajp61_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_jajp61_distribute_nucdur(state, p0);
}
int32_t reloq_jajp61_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_jajp61_divide_char_count_token(state, p0, p1);
}
int32_t reloq_jajp61_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_jajp61_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_jajp61_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_jajp61_doF(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_jajp61_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_jajp61_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_jajp61_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_jajp61_do_FTZ_FTP(state, p0);
}
int32_t reloq_jajp61_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_jajp61_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_jajp61_do_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_do_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_h_amps,args,2,2);
    }
    return reloq_jajp61_do_h_amps(state, p0, p1);
}
int32_t reloq_jajp61_do_jap_h_amps(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_do_jap_h_amps(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_jap_h_amps,args,2,2);
    }
    return reloq_jajp61_do_jap_h_amps(state, p0, p1);
}
int32_t reloq_jajp61_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_jajp61_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_jajp61_do_non_son_amps(state, p0);
}
int32_t reloq_jajp61_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_jajp61_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_jajp61_do_pt_voicing(state, p0);
}
int32_t reloq_jajp61_do_release(void *, int32_t);
static inline int32_t reloq_call_jajp61_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_jajp61_do_release(state, p0);
}
int32_t reloq_jajp61_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_jajp61_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_jajp61_do_son_amps(state, p0);
}
int32_t reloq_jajp61_do_user_index_callbacks(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_do_user_index_callbacks(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_user_index_callbacks,args,2,2);
    }
    return reloq_jajp61_do_user_index_callbacks(state, p0, p1);
}
int32_t reloq_jajp61_do_voice_bar(void *, int32_t);
int32_t reloq_jajp61_dur_adjustments(void *, int32_t);
static inline int32_t reloq_call_jajp61_dur_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_dur_adjustments,args,1,1);
    }
    return reloq_jajp61_dur_adjustments(state, p0);
}
int32_t reloq_jajp61_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_jajp61_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_jajp61_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_jajp61_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_jajp61_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_jajp61_end_of_sentence(state, p0, p1);
}
int32_t reloq_jajp61_end_of_token(void *, int32_t, int32_t);
int32_t reloq_jajp61_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_jajp61_end_of_word(state, p0, p1);
}
int32_t reloq_jajp61_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_jajp61_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_estimate_jap_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_estimate_jap_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_jap_nsylls,args,4,4);
    }
    return reloq_jajp61_estimate_jap_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_estimate_lang_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_estimate_lang_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_lang_nsylls,args,4,4);
    }
    return reloq_jajp61_estimate_lang_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_estimate_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_nsylls,args,4,4);
    }
    return reloq_jajp61_estimate_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_exceptions_to_pre_cons_shortening(void *, int32_t);
static inline int32_t reloq_call_jajp61_exceptions_to_pre_cons_shortening(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_exceptions_to_pre_cons_shortening,args,1,1);
    }
    return reloq_jajp61_exceptions_to_pre_cons_shortening(state, p0);
}
int32_t reloq_jajp61_extend_vals(void *, int32_t);
int32_t reloq_jajp61_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_jajp61_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_jajp61_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_jajp61_final_voiced_release(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_jajp61_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_jajp61_find_acoustic_nuc(state, p0);
}
int32_t reloq_jajp61_find_jap_nucleus(void *, int32_t);
static inline int32_t reloq_call_jajp61_find_jap_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_jap_nucleus,args,1,1);
    }
    return reloq_jajp61_find_jap_nucleus(state, p0);
}
int32_t reloq_jajp61_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_jajp61_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_find_most_stressed_syll(void *, int32_t);
static inline int32_t reloq_call_jajp61_find_most_stressed_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_most_stressed_syll,args,1,1);
    }
    return reloq_jajp61_find_most_stressed_syll(state, p0);
}
int32_t reloq_jajp61_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_jajp61_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_jajp61_find_nucleus(state, p0);
}
int32_t reloq_jajp61_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_jajp61_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_jajp61_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_first_postnucl_accent,args,1,1);
    }
    return reloq_jajp61_first_postnucl_accent(state, p0);
}
int32_t reloq_jajp61_first_root_vow(void *, int32_t, int32_t);
int32_t reloq_jajp61_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_jajp61_forced_synthesis(state, p0, p1);
}
int32_t reloq_jajp61_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_jajp61_generate_acoustic_vals(state, p0);
}
int32_t reloq_jajp61_generate_diaphones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_generate_diaphones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_generate_diaphones,args,3,3);
    }
    return reloq_jajp61_generate_diaphones(state, p0, p1, p2);
}
int32_t reloq_jajp61_generate_durations(void *, int32_t);
static inline int32_t reloq_call_jajp61_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_jajp61_generate_durations(state, p0);
}
int32_t reloq_jajp61_generate_intonation(void *, int32_t);
static inline int32_t reloq_call_jajp61_generate_intonation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_intonation,args,1,1);
    }
    return reloq_jajp61_generate_intonation(state, p0);
}
int32_t reloq_jajp61_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_jajp61_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_jajp61_get_h_dur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_h_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_h_dur,args,4,4);
    }
    return reloq_jajp61_get_h_dur(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_jajp61_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_get_input(void *, int32_t);
static inline int32_t reloq_call_jajp61_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_jajp61_get_input(state, p0);
}
int32_t reloq_jajp61_get_jap_fric_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_jap_fric_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_jap_fric_trans_dur,args,2,2);
    }
    return reloq_jajp61_get_jap_fric_trans_dur(state, p0, p1);
}
int32_t reloq_jajp61_get_jap_h_dur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_jap_h_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_jap_h_dur,args,4,4);
    }
    return reloq_jajp61_get_jap_h_dur(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_get_jap_nas_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_jap_nas_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_jap_nas_trans_dur,args,2,2);
    }
    return reloq_jajp61_get_jap_nas_trans_dur(state, p0, p1);
}
int32_t reloq_jajp61_get_jap_son_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_jap_son_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_jap_son_trans_dur,args,2,2);
    }
    return reloq_jajp61_get_jap_son_trans_dur(state, p0, p1);
}
int32_t reloq_jajp61_get_jap_stop_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_jap_stop_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_jap_stop_trans_dur,args,2,2);
    }
    return reloq_jajp61_get_jap_stop_trans_dur(state, p0, p1);
}
int32_t reloq_jajp61_get_jap_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_jap_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_jap_trans_dur,args,2,2);
    }
    return reloq_jajp61_get_jap_trans_dur(state, p0, p1);
}
int32_t reloq_jajp61_get_jap_vow_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_jap_vow_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_jap_vow_trans_dur,args,2,2);
    }
    return reloq_jajp61_get_jap_vow_trans_dur(state, p0, p1);
}
int32_t reloq_jajp61_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_jajp61_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_jajp61_get_nuc_AV(state, p0);
}
int32_t reloq_jajp61_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_jajp61_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_jajp61_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_jajp61_get_stanjap_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_jajp61_get_stanjap_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanjap_nuc_AV,args,1,1);
    }
    return reloq_jajp61_get_stanjap_nuc_AV(state, p0);
}
int32_t reloq_jajp61_get_tok(void *, int32_t);
static inline int32_t reloq_call_jajp61_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_jajp61_get_tok(state, p0);
}
int32_t reloq_jajp61_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_jajp61_get_trans_dur(state, p0, p1);
}
int32_t reloq_jajp61_gettok(void *, int32_t);
static inline int32_t reloq_call_jajp61_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_jajp61_gettok(state, p0);
}
int32_t reloq_jajp61_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_jajp61_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_jajp61_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_jajp61_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_jajp61_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_jajp61_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_jajp61_handle_pause(state, p0, p1);
}
int32_t reloq_jajp61_high_vowel_devoicing(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_high_vowel_devoicing(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_high_vowel_devoicing,args,3,3);
    }
    return reloq_jajp61_high_vowel_devoicing(state, p0, p1, p2);
}
int32_t reloq_jajp61_ident_to_left(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_ident_to_left(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ident_to_left,args,2,2);
    }
    return reloq_jajp61_ident_to_left(state, p0, p1);
}
int32_t reloq_jajp61_ident_to_right(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_ident_to_right(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ident_to_right,args,2,2);
    }
    return reloq_jajp61_ident_to_right(state, p0, p1);
}
int32_t reloq_jajp61_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_jajp61_in_brackets(state, p0, p1, p2);
}
int32_t reloq_jajp61_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_jajp61_in_quotes(state, p0, p1, p2);
}
int32_t reloq_jajp61_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_in_syllable(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_syllable,args,4,4);
    }
    return reloq_jajp61_in_syllable(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_jajp61_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_jajp61_increment_output_file_number(state, p0);
}
int32_t reloq_jajp61_init_accphr_inton_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_accphr_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_accphr_inton_vars,args,1,1);
    }
    return reloq_jajp61_init_accphr_inton_vars(state, p0);
}
int32_t reloq_jajp61_init_accphr_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_accphr_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_accphr_vars,args,1,1);
    }
    return reloq_jajp61_init_accphr_vars(state, p0);
}
int32_t reloq_jajp61_init_current_word_ptrs(void *, int32_t);
int32_t reloq_jajp61_init_delta(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_jajp61_init_delta(state, p0);
}
int32_t reloq_jajp61_init_jap_accphr_inton_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_jap_accphr_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_jap_accphr_inton_vars,args,1,1);
    }
    return reloq_jajp61_init_jap_accphr_inton_vars(state, p0);
}
int32_t reloq_jajp61_init_jap_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_jap_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_jap_symbolic_vars,args,1,1);
    }
    return reloq_jajp61_init_jap_symbolic_vars(state, p0);
}
int32_t reloq_jajp61_init_jap_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_jap_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_jap_vars,args,1,1);
    }
    return reloq_jajp61_init_jap_vars(state, p0);
}
int32_t reloq_jajp61_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_jajp61_init_lang_vars(state, p0);
}
int32_t reloq_jajp61_init_nuclwrd_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_nuclwrd_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_nuclwrd_vars,args,1,1);
    }
    return reloq_jajp61_init_nuclwrd_vars(state, p0);
}
int32_t reloq_jajp61_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_jajp61_init_phr_feature_vars(state, p0);
}
int32_t reloq_jajp61_init_phrase_inton_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_phrase_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_inton_vars,args,1,1);
    }
    return reloq_jajp61_init_phrase_inton_vars(state, p0);
}
int32_t reloq_jajp61_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_jajp61_init_phrase_vars(state, p0);
}
int32_t reloq_jajp61_init_platform(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_jajp61_init_platform(state, p0);
}
int32_t reloq_jajp61_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_jajp61_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_jajp61_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_jajp61_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_jajp61_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_inton_vars,args,1,1);
    }
    return reloq_jajp61_init_syll_inton_vars(state, p0);
}
int32_t reloq_jajp61_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_jajp61_init_syll_vars(state, p0);
}
int32_t reloq_jajp61_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_jajp61_init_symbolic_vars(state, p0);
}
int32_t reloq_jajp61_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_jajp61_init_tm_vars(state, p0);
}
int32_t reloq_jajp61_init_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_jajp61_init_vars(state, p0);
}
int32_t reloq_jajp61_init_voice_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_voice_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_voice_vars,args,1,1);
    }
    return reloq_jajp61_init_voice_vars(state, p0);
}
int32_t reloq_jajp61_init_word(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_jajp61_init_word(state, p0);
}
int32_t reloq_jajp61_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_inton_vars,args,1,1);
    }
    return reloq_jajp61_init_word_inton_vars(state, p0);
}
int32_t reloq_jajp61_init_word_ptrs(void *, int32_t);
int32_t reloq_jajp61_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_jajp61_init_word_vars(state, p0);
}
int32_t reloq_jajp61_insert_acc_phrases(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_acc_phrases(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acc_phrases,args,1,1);
    }
    return reloq_jajp61_insert_acc_phrases(state, p0);
}
int32_t reloq_jajp61_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_jajp61_insert_acoustic_vals(state, p0);
}
int32_t reloq_jajp61_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_jajp61_insert_burst_dur(state, p0);
}
int32_t reloq_jajp61_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_jajp61_insert_char_count(state, p0);
}
int32_t reloq_jajp61_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_jajp61_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_insert_f0(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_f0(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_f0,args,4,4);
    }
    return reloq_jajp61_insert_f0(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_insert_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_insert_jap_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_jap_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_jap_spr_phone,args,3,3);
    }
    return reloq_jajp61_insert_jap_spr_phone(state, p0, p1, p2);
}
int32_t reloq_jajp61_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_jajp61_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_jajp61_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_jajp61_insert_pause_durs(state, p0);
}
int32_t reloq_jajp61_insert_penult_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_jajp61_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_final_tones,args,1,1);
    }
    return reloq_jajp61_insert_phrase_final_tones(state, p0);
}
int32_t reloq_jajp61_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_jajp61_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_jajp61_insert_preset_phrase_tone(void *, int32_t);
int32_t reloq_jajp61_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_jajp61_insert_space(state, p0, p1);
}
int32_t reloq_jajp61_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_jajp61_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_jajp61_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_jajp61_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_jajp61_insert_syll_level_vals(state, p0);
}
int32_t reloq_jajp61_insert_target_t3(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_target_t3(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_target_t3,args,1,1);
    }
    return reloq_jajp61_insert_target_t3(state, p0);
}
int32_t reloq_jajp61_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_jajp61_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_jajp61_insert_transitions(state, p0);
}
int32_t reloq_jajp61_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_jajp61_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_jajp61_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_jajp61_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_jajp61_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_jajp61_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_jajp61_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_jajp61_insert_word(state, p0, p1, p2);
}
int32_t reloq_jajp61_insert_zero_sylls(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_inside_orig_inp_token(void *, int32_t, int32_t);
int32_t reloq_jajp61_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_jajp61_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_jajp61_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_jajp61_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_jajp61_is_accented_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_is_accented_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_accented_phrase,args,2,2);
    }
    return reloq_jajp61_is_accented_phrase(state, p0, p1);
}
int32_t reloq_jajp61_is_after_h(void *, int32_t, int32_t);
int32_t reloq_jajp61_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_jajp61_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_jajp61_is_hyphenated_word(void *, int32_t, int32_t);
int32_t reloq_jajp61_is_in_parentheses(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_is_jap_target_syll(void *, int32_t);
static inline int32_t reloq_call_jajp61_is_jap_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_jap_target_syll,args,1,1);
    }
    return reloq_jajp61_is_jap_target_syll(state, p0);
}
int32_t reloq_jajp61_is_retroflex_d(void *, int32_t);
static inline int32_t reloq_call_jajp61_is_retroflex_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_retroflex_d,args,1,1);
    }
    return reloq_jajp61_is_retroflex_d(state, p0);
}
int32_t reloq_jajp61_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_jajp61_is_spr(state, p0, p1, p2);
}
int32_t reloq_jajp61_is_target_syll(void *, int32_t);
static inline int32_t reloq_call_jajp61_is_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_target_syll,args,1,1);
    }
    return reloq_jajp61_is_target_syll(state, p0);
}
int32_t reloq_jajp61_jap_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_boundary_tone,args,1,1);
    }
    return reloq_jajp61_jap_boundary_tone(state, p0);
}
int32_t reloq_jajp61_jap_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_jap_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_jap_char_name,args,3,3);
    }
    return reloq_jajp61_jap_char_name(state, p0, p1, p2);
}
int32_t reloq_jajp61_jap_con_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_con_vals,args,1,1);
    }
    return reloq_jajp61_jap_con_vals(state, p0);
}
int32_t reloq_jajp61_jap_cons_dur_adjust(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_cons_dur_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_cons_dur_adjust,args,1,1);
    }
    return reloq_jajp61_jap_cons_dur_adjust(state, p0);
}
int32_t reloq_jajp61_jap_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_jap_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_jap_cons_durs,args,3,3);
    }
    return reloq_jajp61_jap_cons_durs(state, p0, p1, p2);
}
int32_t reloq_jajp61_jap_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_first_postnucl_accent,args,1,1);
    }
    return reloq_jajp61_jap_first_postnucl_accent(state, p0);
}
int32_t reloq_jajp61_jap_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_insert_phrase_final_tones,args,1,1);
    }
    return reloq_jajp61_jap_insert_phrase_final_tones(state, p0);
}
int32_t reloq_jajp61_jap_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_nucl_high_tone,args,1,1);
    }
    return reloq_jajp61_jap_nucl_high_tone(state, p0);
}
int32_t reloq_jajp61_jap_ph_N(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_N(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_N,args,1,1);
    }
    return reloq_jajp61_jap_ph_N(state, p0);
}
int32_t reloq_jajp61_jap_ph_N_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_N_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_N_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_N_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_S(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_S,args,1,1);
    }
    return reloq_jajp61_jap_ph_S(state, p0);
}
int32_t reloq_jajp61_jap_ph_S_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_S_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_S_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_S_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_Z(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_Z,args,1,1);
    }
    return reloq_jajp61_jap_ph_Z(state, p0);
}
int32_t reloq_jajp61_jap_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_Z_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_Z_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_a(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_a,args,1,1);
    }
    return reloq_jajp61_jap_ph_a(state, p0);
}
int32_t reloq_jajp61_jap_ph_b(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_b,args,1,1);
    }
    return reloq_jajp61_jap_ph_b(state, p0);
}
int32_t reloq_jajp61_jap_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_b_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_b_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_c(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_c(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_c,args,1,1);
    }
    return reloq_jajp61_jap_ph_c(state, p0);
}
int32_t reloq_jajp61_jap_ph_c_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_c_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_c_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_c_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_d(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_d,args,1,1);
    }
    return reloq_jajp61_jap_ph_d(state, p0);
}
int32_t reloq_jajp61_jap_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_d_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_d_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_e(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_e,args,1,1);
    }
    return reloq_jajp61_jap_ph_e(state, p0);
}
int32_t reloq_jajp61_jap_ph_f(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_f(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_f,args,1,1);
    }
    return reloq_jajp61_jap_ph_f(state, p0);
}
int32_t reloq_jajp61_jap_ph_f_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_f_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_f_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_f_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_g(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_g,args,1,1);
    }
    return reloq_jajp61_jap_ph_g(state, p0);
}
int32_t reloq_jajp61_jap_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_g_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_g_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_h_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_h_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_h_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_h_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_i(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_i,args,1,1);
    }
    return reloq_jajp61_jap_ph_i(state, p0);
}
int32_t reloq_jajp61_jap_ph_k(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_k,args,1,1);
    }
    return reloq_jajp61_jap_ph_k(state, p0);
}
int32_t reloq_jajp61_jap_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_k_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_k_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_m(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_m,args,1,1);
    }
    return reloq_jajp61_jap_ph_m(state, p0);
}
int32_t reloq_jajp61_jap_ph_m_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_m_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_m_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_m_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_n(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_n,args,1,1);
    }
    return reloq_jajp61_jap_ph_n(state, p0);
}
int32_t reloq_jajp61_jap_ph_n_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_n_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_n_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_o(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_o,args,1,1);
    }
    return reloq_jajp61_jap_ph_o(state, p0);
}
int32_t reloq_jajp61_jap_ph_p(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_p,args,1,1);
    }
    return reloq_jajp61_jap_ph_p(state, p0);
}
int32_t reloq_jajp61_jap_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_p_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_p_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_r(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_r,args,1,1);
    }
    return reloq_jajp61_jap_ph_r(state, p0);
}
int32_t reloq_jajp61_jap_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_r_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_r_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_s(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_s,args,1,1);
    }
    return reloq_jajp61_jap_ph_s(state, p0);
}
int32_t reloq_jajp61_jap_ph_s_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_s_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_s_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_s_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_t(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_t,args,1,1);
    }
    return reloq_jajp61_jap_ph_t(state, p0);
}
int32_t reloq_jajp61_jap_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_t_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_t_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_u(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_u,args,1,1);
    }
    return reloq_jajp61_jap_ph_u(state, p0);
}
int32_t reloq_jajp61_jap_ph_w(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_w,args,1,1);
    }
    return reloq_jajp61_jap_ph_w(state, p0);
}
int32_t reloq_jajp61_jap_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_w_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_w_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_y(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_y,args,1,1);
    }
    return reloq_jajp61_jap_ph_y(state, p0);
}
int32_t reloq_jajp61_jap_ph_y_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_y_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_y_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_y_dur(state, p0);
}
int32_t reloq_jajp61_jap_ph_z(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_z,args,1,1);
    }
    return reloq_jajp61_jap_ph_z(state, p0);
}
int32_t reloq_jajp61_jap_ph_z_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_ph_z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_ph_z_dur,args,1,1);
    }
    return reloq_jajp61_jap_ph_z_dur(state, p0);
}
int32_t reloq_jajp61_jap_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_phr_initial_F0,args,1,1);
    }
    return reloq_jajp61_jap_phr_initial_F0(state, p0);
}
int32_t reloq_jajp61_jap_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_jap_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_jap_phrase_level_rules,args,3,3);
    }
    return reloq_jajp61_jap_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_jajp61_jap_phrase_type(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_phrase_type(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_phrase_type,args,1,1);
    }
    return reloq_jajp61_jap_phrase_type(state, p0);
}
int32_t reloq_jajp61_jap_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_position_t_ptrs,args,1,1);
    }
    return reloq_jajp61_jap_position_t_ptrs(state, p0);
}
int32_t reloq_jajp61_jap_symbolic_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_jap_symbolic_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_jap_symbolic_rules,args,3,3);
    }
    return reloq_jajp61_jap_symbolic_rules(state, p0, p1, p2);
}
int32_t reloq_jajp61_jap_trans_features(void *, int32_t);
static inline int32_t reloq_call_jajp61_jap_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jap_trans_features,args,1,1);
    }
    return reloq_jajp61_jap_trans_features(state, p0);
}
int32_t reloq_jajp61_jpn_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_jpn_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_jpn_init_syll_inton_vars,args,1,1);
    }
    return reloq_jajp61_jpn_init_syll_inton_vars(state, p0);
}
int32_t reloq_jajp61_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_lang_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_phrase_level_rules,args,3,3);
    }
    return reloq_jajp61_lang_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_jajp61_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_jajp61_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_jajp61_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_jajp61_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_jajp61_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_jajp61_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_jajp61_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_mark_h(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_mark_h(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_h,args,3,3);
    }
    return reloq_jajp61_mark_h(state, p0, p1, p2);
}
int32_t reloq_jajp61_mark_non_letter(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_jajp61_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_mark_spr_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_spr_syll_vals,args,6,6);
    }
    return reloq_jajp61_mark_spr_syll_vals(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_mark_syll_stress(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_mark_uppercase(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_mark_word_categories(void *, int32_t);
int32_t reloq_jajp61_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_jajp61_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_jajp61_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_jajp61_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_jajp61_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_jajp61_misc_chars(state, p0, p1, p2);
}
int32_t reloq_jajp61_modify_breathiness_for_femV(void *, int32_t);
int32_t reloq_jajp61_modulate_nas_AV(void *, int32_t);
int32_t reloq_jajp61_modulate_noise(void *, int32_t);
static inline int32_t reloq_call_jajp61_modulate_noise(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_noise,args,1,1);
    }
    return reloq_jajp61_modulate_noise(state, p0);
}
int32_t reloq_jajp61_modulate_pwind(void *, int32_t, int32_t);
int32_t reloq_jajp61_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_jajp61_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_nasalize_jap_vow(void *, int32_t);
static inline int32_t reloq_call_jajp61_nasalize_jap_vow(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nasalize_jap_vow,args,1,1);
    }
    return reloq_jajp61_nasalize_jap_vow(state, p0);
}
int32_t reloq_jajp61_nasalize_vow(void *, int32_t);
int32_t reloq_jajp61_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_jajp61_no_readin(state, p0, p1);
}
int32_t reloq_jajp61_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_jajp61_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_jajp61_no_words_in_delta(state, p0);
}
int32_t reloq_jajp61_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_jajp61_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_jajp61_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_jajp61_normalize_text(state, p0, p1, p2);
}
int32_t reloq_jajp61_nucl_f0(void *, int32_t);
static inline int32_t reloq_call_jajp61_nucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_f0,args,1,1);
    }
    return reloq_jajp61_nucl_f0(state, p0);
}
int32_t reloq_jajp61_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_jajp61_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_tone,args,1,1);
    }
    return reloq_jajp61_nucl_high_tone(state, p0);
}
int32_t reloq_jajp61_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_jajp61_nucleus_vals(state, p0);
}
int32_t reloq_jajp61_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_jajp61_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_jajp61_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_jajp61_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_jajp61_period_phrase(state, p0, p1, p2);
}
int32_t reloq_jajp61_pf_non_final_accented_syll(void *, int32_t);
int32_t reloq_jajp61_pf_word_final_syll(void *, int32_t);
static inline int32_t reloq_call_jajp61_pf_word_final_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_pf_word_final_syll,args,1,1);
    }
    return reloq_jajp61_pf_word_final_syll(state, p0);
}
int32_t reloq_jajp61_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_jajp61_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_jajp61_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phr_initial_F0,args,1,1);
    }
    return reloq_jajp61_phr_initial_F0(state, p0);
}
int32_t reloq_jajp61_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_jajp61_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_phrase_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_phrase_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_accent_anno,args,4,4);
    }
    return reloq_jajp61_phrase_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_phrase_final_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_phrase_final_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_final_dur,args,1,1);
    }
    return reloq_jajp61_phrase_final_dur(state, p0);
}
int32_t reloq_jajp61_phrase_initial_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_phrase_initial_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_initial_dur,args,1,1);
    }
    return reloq_jajp61_phrase_initial_dur(state, p0);
}
int32_t reloq_jajp61_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_jajp61_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_jajp61_phrase_medial_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_phrase_medial_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_medial_dur,args,1,1);
    }
    return reloq_jajp61_phrase_medial_dur(state, p0);
}
int32_t reloq_jajp61_phrase_tone(void *, int32_t);
int32_t reloq_jajp61_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_jajp61_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_position_t_ptrs,args,1,1);
    }
    return reloq_jajp61_position_t_ptrs(state, p0);
}
int32_t reloq_jajp61_post_comma_phrase(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_jajp61_potential_sent_end(state, p0, p1);
}
int32_t reloq_jajp61_pre_con_shortening_rules(void *, int32_t);
static inline int32_t reloq_call_jajp61_pre_con_shortening_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_pre_con_shortening_rules,args,1,1);
    }
    return reloq_jajp61_pre_con_shortening_rules(state, p0);
}
int32_t reloq_jajp61_pre_voice_lengthening(void *, int32_t);
static inline int32_t reloq_call_jajp61_pre_voice_lengthening(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_pre_voice_lengthening,args,1,1);
    }
    return reloq_jajp61_pre_voice_lengthening(state, p0);
}
int32_t reloq_jajp61_precedes_number(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_jajp61_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_jajp61_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_jajp61_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_jajp61_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_jajp61_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_jajp61_print_jap_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_print_jap_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_jap_SPR,args,3,3);
    }
    return reloq_jajp61_print_jap_SPR(state, p0, p1, p2);
}
int32_t reloq_jajp61_print_prompt(void *, int32_t);
static inline int32_t reloq_call_jajp61_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_jajp61_print_prompt(state, p0);
}
int32_t reloq_jajp61_print_spr(void *, int32_t);
static inline int32_t reloq_call_jajp61_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_jajp61_print_spr(state, p0);
}
int32_t reloq_jajp61_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_jajp61_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_jajp61_print_word_stress(state, p0, p1);
}
int32_t reloq_jajp61_printouts(void *, int32_t);
static inline int32_t reloq_call_jajp61_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_jajp61_printouts(state, p0);
}
int32_t reloq_jajp61_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_jajp61_process_annotation(state, p0, p1, p2);
}
int32_t reloq_jajp61_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_jajp61_process_input(state, p0, p1);
}
int32_t reloq_jajp61_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_jajp61_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_jajp61_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_jajp61_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_jajp61_punctuation(state, p0, p1, p2);
}
int32_t reloq_jajp61_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_jajp61_question_phrase(state, p0, p1, p2);
}
int32_t reloq_jajp61_read_end(void *, int32_t);
static inline int32_t reloq_call_jajp61_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_jajp61_read_end(state, p0);
}
int32_t reloq_jajp61_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_jajp61_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_jajp61_realize_prenucl_tone(void *, int32_t);
int32_t reloq_jajp61_reduce_long_vowel_coarticulation(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_reduce_long_vowel_coarticulation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reduce_long_vowel_coarticulation,args,4,4);
    }
    return reloq_jajp61_reduce_long_vowel_coarticulation(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_reduce_nucleus(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_reduce_nucleus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reduce_nucleus,args,2,2);
    }
    return reloq_jajp61_reduce_nucleus(state, p0, p1);
}
int32_t reloq_jajp61_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_jajp61_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_jajp61_register_SAPI_voices(state, p0);
}
int32_t reloq_jajp61_register_jap_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_register_jap_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_jap_phones,args,3,3);
    }
    return reloq_jajp61_register_jap_phones(state, p0, p1, p2);
}
int32_t reloq_jajp61_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_jajp61_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_reset_jap_sent_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_reset_jap_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_jap_sent_vars,args,1,1);
    }
    return reloq_jajp61_reset_jap_sent_vars(state, p0);
}
int32_t reloq_jajp61_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_jajp61_reset_lang_sent_vars(state, p0);
}
int32_t reloq_jajp61_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_jajp61_reset_phrase_vars(state, p0);
}
int32_t reloq_jajp61_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_jajp61_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_jajp61_reset_sent_vars(state, p0);
}
int32_t reloq_jajp61_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_jajp61_reset_token_vars(state, p0, p1);
}
int32_t reloq_jajp61_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_jajp61_reset_word_vals(state, p0, p1);
}
int32_t reloq_jajp61_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_jajp61_run_filters(state, p0, p1, p2);
}
int32_t reloq_jajp61_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_jajp61_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_jajp61_run_speech_module(state, p0);
}
int32_t reloq_jajp61_run_text_module(void *, int32_t);
static inline int32_t reloq_call_jajp61_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_jajp61_run_text_module(state, p0);
}
int32_t reloq_jajp61_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_jajp61_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_jajp61_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_jajp61_scale_word_final_trans(state, p0);
}
int32_t reloq_jajp61_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_jajp61_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_jajp61_semicolon_phrase(state, p0);
}
int32_t reloq_jajp61_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_jajp61_separate_tokens(state, p0, p1);
}
int32_t reloq_jajp61_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_jajp61_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_jajp61_set_f0_range_ptrs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_set_f0_range_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_range_ptrs,args,5,5);
    }
    return reloq_jajp61_set_f0_range_ptrs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_jajp61_set_f0_wind_and_base(state, p0);
}
int32_t reloq_jajp61_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_jajp61_set_global_constants(state, p0);
}
int32_t reloq_jajp61_set_jap_global_constants(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_jap_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_jap_global_constants,args,1,1);
    }
    return reloq_jajp61_set_jap_global_constants(state, p0);
}
int32_t reloq_jajp61_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_jajp61_set_klattparms(state, p0);
}
int32_t reloq_jajp61_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_jajp61_set_lang_global_constants(state, p0);
}
int32_t reloq_jajp61_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_jajp61_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_jajp61_set_num_formants(state, p0);
}
int32_t reloq_jajp61_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_jajp61_set_phone_offset_vals(state, p0);
}
int32_t reloq_jajp61_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_jajp61_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_jajp61_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_jajp61_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_jajp61_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_jajp61_set_syll_offset_vals(state, p0);
}
int32_t reloq_jajp61_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_jajp61_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_jajp61_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_jajp61_set_voice_param_constants(state, p0);
}
int32_t reloq_jajp61_set_window_size(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_jajp61_set_window_size(state, p0);
}
int32_t reloq_jajp61_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_jajp61_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_jajp61_setdur(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_jajp61_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_jajp61_shift_tone(void *, int32_t, int32_t);
int32_t reloq_jajp61_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_jajp61_single_chars(state, p0, p1, p2);
}
int32_t reloq_jajp61_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_jajp61_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_jajp61_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_jajp61_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_jajp61_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_jajp61_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_jajp61_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_jajp61_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_jajp61_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_jajp61_single_letter(state, p0, p1, p2);
}
int32_t reloq_jajp61_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_jajp61_size_filter(state, p0, p1, p2);
}
int32_t reloq_jajp61_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_jajp61_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_jajp61_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_jajp61_space(state, p0, p1, p2);
}
int32_t reloq_jajp61_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_jajp61_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_jajp61_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_jajp61_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_jajp61_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_jajp61_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_jajp61_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_stanjap_vals(void *, int32_t);
static inline int32_t reloq_call_jajp61_stanjap_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanjap_vals,args,1,1);
    }
    return reloq_jajp61_stanjap_vals(state, p0);
}
int32_t reloq_jajp61_start_dur(void *, int32_t);
static inline int32_t reloq_call_jajp61_start_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_start_dur,args,1,1);
    }
    return reloq_jajp61_start_dur(state, p0);
}
int32_t reloq_jajp61_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_jajp61_stretch_focus(state, p0, p1);
}
int32_t reloq_jajp61_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_jajp61_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_string_to_short(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_short,args,4,4);
    }
    return reloq_jajp61_string_to_short(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_strip_last_digit(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_jajp61_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_jajp61_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_jajp61_text_index(state, p0, p1);
}
int32_t reloq_jajp61_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_jajp61_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_jajp61_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_jajp61_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_jajp61_token_to_number(state, p0, p1, p2);
}
int32_t reloq_jajp61_user_abbr_dict(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_jajp61_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_user_index_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_user_index_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_index_anno,args,3,3);
    }
    return reloq_jajp61_user_index_anno(state, p0, p1, p2);
}
int32_t reloq_jajp61_user_rootdict(void *, int32_t, int32_t, int32_t);
int32_t reloq_jajp61_valid_f0_posn(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_valid_f0_posn(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_f0_posn,args,3,3);
    }
    return reloq_jajp61_valid_f0_posn(state, p0, p1, p2);
}
int32_t reloq_jajp61_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_jajp61_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_jajp61_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_jajp61_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_vow_to_vow_adjustments(void *, int32_t);
static inline int32_t reloq_call_jajp61_vow_to_vow_adjustments(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_vow_to_vow_adjustments,args,1,1);
    }
    return reloq_jajp61_vow_to_vow_adjustments(state, p0);
}
int32_t reloq_jajp61_what_syll_accented_in_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_what_syll_accented_in_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_what_syll_accented_in_phrase,args,3,3);
    }
    return reloq_jajp61_what_syll_accented_in_phrase(state, p0, p1, p2);
}
int32_t reloq_jajp61_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_jajp61_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_word_cat_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_word_cat_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_cat_anno,args,3,3);
    }
    return reloq_jajp61_word_cat_anno(state, p0, p1, p2);
}
int32_t reloq_jajp61_word_final_lengthening(void *, int32_t);
static inline int32_t reloq_call_jajp61_word_final_lengthening(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_final_lengthening,args,1,1);
    }
    return reloq_jajp61_word_final_lengthening(state, p0);
}
int32_t reloq_jajp61_word_initial_h(void *, int32_t);
static inline int32_t reloq_call_jajp61_word_initial_h(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_initial_h,args,1,1);
    }
    return reloq_jajp61_word_initial_h(state, p0);
}
int32_t reloq_jajp61_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_jajp61_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_jajp61_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_jajp61_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_jajp61_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_jajp61_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_jajp61_year_mode_anno(state, p0, p1, p2, p3);
}
#endif
