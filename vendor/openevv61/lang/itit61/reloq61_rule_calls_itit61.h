/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_ITIT61_H
#define RELOQ61_RULE_CALLS_ITIT61_H
#include "delta_rules_c.h"
extern int delta_rule_trace;
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_itit61_DeltaProc_end(void *, int32_t);
int32_t reloq_itit61_DeltaProc_flush(void *, int32_t);
int32_t reloq_itit61_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_itit61_DeltaProc_main(void *, int32_t);
int32_t reloq_itit61_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_itit61_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_itit61_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_itit61_DeltaProc_start(void *, int32_t);
int32_t reloq_itit61_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_itit61_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_itit61_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_itit61_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_URL(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_URL(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_URL,args,2,2);
    }
    return reloq_itit61_URL(state, p0, p1);
}
int32_t reloq_itit61_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbr,args,4,4);
    }
    return reloq_itit61_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_abbr_before_num(void *, int32_t, int32_t);
int32_t reloq_itit61_abbreviation(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_abbreviation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbreviation,args,4,4);
    }
    return reloq_itit61_abbreviation(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_itit61_absval(state, p0, p1, p2);
}
int32_t reloq_itit61_acronym(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_acronym(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acronym,args,3,3);
    }
    return reloq_itit61_acronym(state, p0, p1, p2);
}
int32_t reloq_itit61_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acrotest,args,3,3);
    }
    return reloq_itit61_acrotest(state, p0, p1, p2);
}
int32_t reloq_itit61_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_itit61_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_itit61_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_add_TL_to_trill(void *, int32_t);
static inline int32_t reloq_call_itit61_add_TL_to_trill(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_TL_to_trill,args,1,1);
    }
    return reloq_itit61_add_TL_to_trill(state, p0);
}
int32_t reloq_itit61_add_f0_for_new_voice(void *, int32_t);
static inline int32_t reloq_call_itit61_add_f0_for_new_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_new_voice,args,1,1);
    }
    return reloq_itit61_add_f0_for_new_voice(state, p0);
}
int32_t reloq_itit61_add_f0_for_next_voice(void *, int32_t);
static inline int32_t reloq_call_itit61_add_f0_for_next_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_next_voice,args,1,1);
    }
    return reloq_itit61_add_f0_for_next_voice(state, p0);
}
int32_t reloq_itit61_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_itit61_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_itit61_add_ital_currency_units(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_add_ital_currency_units(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ital_currency_units,args,3,3);
    }
    return reloq_itit61_add_ital_currency_units(state, p0, p1, p2);
}
int32_t reloq_itit61_add_ital_gender_to_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_add_ital_gender_to_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ital_gender_to_ordinal,args,3,3);
    }
    return reloq_itit61_add_ital_gender_to_ordinal(state, p0, p1, p2);
}
int32_t reloq_itit61_add_ital_num_infl(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_add_ital_num_infl(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ital_num_infl,args,3,3);
    }
    return reloq_itit61_add_ital_num_infl(state, p0, p1, p2);
}
int32_t reloq_itit61_add_ital_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_add_ital_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ital_point_digits,args,3,3);
    }
    return reloq_itit61_add_ital_point_digits(state, p0, p1, p2);
}
int32_t reloq_itit61_add_ital_subunits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_add_ital_subunits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_ital_subunits,args,3,3);
    }
    return reloq_itit61_add_ital_subunits(state, p0, p1, p2);
}
int32_t reloq_itit61_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_itit61_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_itit61_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_itit61_adjust_AV_for_fem_voice(void *, int32_t);
static inline int32_t reloq_call_itit61_adjust_AV_for_fem_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_fem_voice,args,1,1);
    }
    return reloq_itit61_adjust_AV_for_fem_voice(state, p0);
}
int32_t reloq_itit61_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_itit61_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_adjust_ital_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adjust_ital_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_ital_accents,args,5,5);
    }
    return reloq_itit61_adjust_ital_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_adjust_ital_inton(void *, int32_t);
int32_t reloq_itit61_adjust_ital_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adjust_ital_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_ital_word_stress,args,3,3);
    }
    return reloq_itit61_adjust_ital_word_stress(state, p0, p1, p2);
}
int32_t reloq_itit61_adjust_lg_specific_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adjust_lg_specific_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_lg_specific_accents,args,5,5);
    }
    return reloq_itit61_adjust_lg_specific_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_adjust_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adjust_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_numbers,args,3,3);
    }
    return reloq_itit61_adjust_numbers(state, p0, p1, p2);
}
int32_t reloq_itit61_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adjust_prev_phr_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_pause,args,3,3);
    }
    return reloq_itit61_adjust_prev_phr_pause(state, p0, p1, p2);
}
int32_t reloq_itit61_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_itit61_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_itit61_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adjust_words_in_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_words_in_phrase,args,3,3);
    }
    return reloq_itit61_adjust_words_in_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_adv_vs_noun_vs_prep(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adv_vs_noun_vs_prep(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adv_vs_noun_vs_prep,args,7,7);
    }
    return reloq_itit61_adv_vs_noun_vs_prep(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_itit61_adv_vs_nounadj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adv_vs_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adv_vs_nounadj,args,3,3);
    }
    return reloq_itit61_adv_vs_nounadj(state, p0, p1, p2);
}
int32_t reloq_itit61_adv_vs_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_adv_vs_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adv_vs_verb,args,3,3);
    }
    return reloq_itit61_adv_vs_verb(state, p0, p1, p2);
}
int32_t reloq_itit61_ambig_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ambig_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ambig_abbr,args,2,2);
    }
    return reloq_itit61_ambig_abbr(state, p0, p1);
}
int32_t reloq_itit61_antepenult_stress(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_antepenult_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_antepenult_stress,args,5,5);
    }
    return reloq_itit61_antepenult_stress(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_itit61_apostrophe(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_itit61_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_itit61_apply_comma_phr_rules(void *, int32_t);
static inline int32_t reloq_call_itit61_apply_comma_phr_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_comma_phr_rules,args,1,1);
    }
    return reloq_itit61_apply_comma_phr_rules(state, p0);
}
int32_t reloq_itit61_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_itit61_apply_head_size_val(state, p0, p1);
}
int32_t reloq_itit61_apply_ital_a_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_a_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_a_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_a_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_b_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_b_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_b_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_b_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_c_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_c_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_c_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_c_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_comma_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_comma_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_d_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_d_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_d_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_d_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_e_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_e_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_e_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_e_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_f_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_f_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_f_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_f_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_g_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_g_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_g_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_g_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_h_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_h_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_h_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_h_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_i_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_i_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_i_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_i_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_j_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_j_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_j_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_j_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_k_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_k_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_k_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_k_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_l_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_l_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_l_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_l_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_m_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_m_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_m_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_m_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_n_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_n_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_n_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_n_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_o_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_o_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_o_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_o_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_p_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_p_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_p_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_p_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_phnol_rules(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_apply_ital_q_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_q_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_q_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_q_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_r_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_r_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_r_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_r_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_s_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_s_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_s_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_s_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_t_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_t_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_t_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_t_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_u_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_u_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_u_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_u_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_v_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_v_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_v_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_v_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_w_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_w_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_w_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_w_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_x_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_x_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_x_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_x_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_y_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_y_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_y_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_y_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_ital_z_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_ital_z_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_ital_z_rules,args,3,3);
    }
    return reloq_itit61_apply_ital_z_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_lang_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_lang_comma_rules,args,3,3);
    }
    return reloq_itit61_apply_lang_comma_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_itit61_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_itit61_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_itit61_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_itit61_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_itit61_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_itit61_apply_roughness_val(state, p0, p1);
}
int32_t reloq_itit61_apply_rules(void *, int32_t);
static inline int32_t reloq_call_itit61_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_itit61_apply_rules(state, p0);
}
int32_t reloq_itit61_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_itit61_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_itit61_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_itit61_apply_speed_val(state, p0, p1);
}
int32_t reloq_itit61_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_itit61_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_itit61_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_itit61_apply_volume_val(state, p0, p1);
}
int32_t reloq_itit61_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_itit61_assign_acoustic_vals(state, p0);
}
int32_t reloq_itit61_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_itit61_assign_alt_quest_features(state, p0);
}
int32_t reloq_itit61_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_itit61_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_itit61_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_itit61_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_itit61_assign_default_quest_features(state, p0);
}
int32_t reloq_itit61_assign_enclitic_pronun(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_enclitic_pronun(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_enclitic_pronun,args,3,3);
    }
    return reloq_itit61_assign_enclitic_pronun(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_gend_to_ital_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_gend_to_ital_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_gend_to_ital_roman_num,args,3,3);
    }
    return reloq_itit61_assign_gend_to_ital_roman_num(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_gender_to_ital_number(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_gender_to_ital_number(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_gender_to_ital_number,args,2,2);
    }
    return reloq_itit61_assign_gender_to_ital_number(state, p0, p1);
}
int32_t reloq_itit61_assign_homogr_E_e(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_homogr_E_e(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_homogr_E_e,args,5,5);
    }
    return reloq_itit61_assign_homogr_E_e(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_assign_homogr_c_o(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_homogr_c_o(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_homogr_c_o,args,5,5);
    }
    return reloq_itit61_assign_homogr_c_o(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_assign_homogr_stress(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_homogr_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_homogr_stress,args,5,5);
    }
    return reloq_itit61_assign_homogr_stress(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_assign_ital_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_ital_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_ital_nuc_durs,args,1,1);
    }
    return reloq_itit61_assign_ital_nuc_durs(state, p0);
}
int32_t reloq_itit61_assign_ital_stress(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_ital_stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_ital_stress,args,1,1);
    }
    return reloq_itit61_assign_ital_stress(state, p0);
}
int32_t reloq_itit61_assign_ital_syll_init_con(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_ital_syll_init_con(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_ital_syll_init_con,args,3,3);
    }
    return reloq_itit61_assign_ital_syll_init_con(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_itit61_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_itit61_assign_nuc_durs(state, p0);
}
int32_t reloq_itit61_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_itit61_assign_phone_durs(state, p0);
}
int32_t reloq_itit61_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_itit61_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_itit61_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_itit61_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_assign_primary_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_primary_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_primary_stress,args,3,3);
    }
    return reloq_itit61_assign_primary_stress(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_root_stress(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_root_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_root_stress,args,5,5);
    }
    return reloq_itit61_assign_root_stress(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_assign_secondary_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_itit61_assign_sent_final_features(state, p0);
}
int32_t reloq_itit61_assign_stanital_start_dur(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_stanital_start_dur(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_stanital_start_dur,args,3,3);
    }
    return reloq_itit61_assign_stanital_start_dur(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_itit61_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_user_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_assign_user_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_user_stress,args,3,3);
    }
    return reloq_itit61_assign_user_stress(state, p0, p1, p2);
}
int32_t reloq_itit61_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_itit61_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_itit61_assign_wh_quest_features(state, p0);
}
int32_t reloq_itit61_beg_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_beg_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_beg_of_word,args,2,2);
    }
    return reloq_itit61_beg_of_word(state, p0, p1);
}
int32_t reloq_itit61_between_proper_names(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_boundary_tone,args,1,1);
    }
    return reloq_itit61_boundary_tone(state, p0);
}
int32_t reloq_itit61_bracket(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_bracket(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_bracket,args,3,3);
    }
    return reloq_itit61_bracket(state, p0, p1, p2);
}
int32_t reloq_itit61_break_into_ital_morphs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_break_into_ital_morphs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_ital_morphs,args,5,5);
    }
    return reloq_itit61_break_into_ital_morphs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_break_into_stanital_phones(void *, int32_t);
static inline int32_t reloq_call_itit61_break_into_stanital_phones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_break_into_stanital_phones,args,1,1);
    }
    return reloq_itit61_break_into_stanital_phones(state, p0);
}
int32_t reloq_itit61_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_itit61_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_itit61_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_itit61_build_phrases(state, p0, p1, p2);
}
int32_t reloq_itit61_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_itit61_build_words(state, p0, p1, p2);
}
int32_t reloq_itit61_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_itit61_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_itit61_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_itit61_char_name(state, p0, p1, p2);
}
int32_t reloq_itit61_clash_tone_alert(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_clash_tone_alert(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_clash_tone_alert,args,2,2);
    }
    return reloq_itit61_clash_tone_alert(state, p0, p1);
}
int32_t reloq_itit61_clear_delta(void *, int32_t);
static inline int32_t reloq_call_itit61_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_itit61_clear_delta(state, p0);
}
int32_t reloq_itit61_close_platform(void *, int32_t);
static inline int32_t reloq_call_itit61_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_itit61_close_platform(state, p0);
}
int32_t reloq_itit61_come_pure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_come_pure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_come_pure,args,3,3);
    }
    return reloq_itit61_come_pure(state, p0, p1, p2);
}
int32_t reloq_itit61_comma_at_end_quote(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_comma_at_end_quote(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_comma_at_end_quote,args,3,3);
    }
    return reloq_itit61_comma_at_end_quote(state, p0, p1, p2);
}
int32_t reloq_itit61_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_itit61_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_itit61_comma_phrase(state, p0);
}
int32_t reloq_itit61_con_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_itit61_con_vals(state, p0);
}
int32_t reloq_itit61_convert_1_or_2_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_1_or_2_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_1_or_2_digits,args,3,3);
    }
    return reloq_itit61_convert_1_or_2_digits(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_billions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_billions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_billions,args,3,3);
    }
    return reloq_itit61_convert_billions(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits,args,3,3);
    }
    return reloq_itit61_convert_digits(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits_to_words,args,3,3);
    }
    return reloq_itit61_convert_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_double,args,3,3);
    }
    return reloq_itit61_convert_double(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_hundreds,args,3,3);
    }
    return reloq_itit61_convert_hundreds(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_apost_to_accent(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_apost_to_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_apost_to_accent,args,4,4);
    }
    return reloq_itit61_convert_ital_apost_to_accent(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_convert_ital_billions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_billions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_billions,args,3,3);
    }
    return reloq_itit61_convert_ital_billions(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_date,args,3,3);
    }
    return reloq_itit61_convert_ital_date(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_digits_to_words,args,3,3);
    }
    return reloq_itit61_convert_ital_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_double,args,3,3);
    }
    return reloq_itit61_convert_ital_double(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_fraction,args,3,3);
    }
    return reloq_itit61_convert_ital_fraction(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_hundreds,args,3,3);
    }
    return reloq_itit61_convert_ital_hundreds(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_millions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_millions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_millions,args,3,3);
    }
    return reloq_itit61_convert_ital_millions(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_monetary_exp,args,5,5);
    }
    return reloq_itit61_convert_ital_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_convert_ital_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_ordinal,args,3,3);
    }
    return reloq_itit61_convert_ital_ordinal(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_roman_num,args,3,3);
    }
    return reloq_itit61_convert_ital_roman_num(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_single_digit,args,3,3);
    }
    return reloq_itit61_convert_ital_single_digit(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_teens(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_teens(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_teens,args,3,3);
    }
    return reloq_itit61_convert_ital_teens(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_ital_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_ital_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_ital_thousands,args,3,3);
    }
    return reloq_itit61_convert_ital_thousands(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_lang_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_lang_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_lang_digits_to_words,args,3,3);
    }
    return reloq_itit61_convert_lang_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_millions(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_millions(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_millions,args,3,3);
    }
    return reloq_itit61_convert_millions(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_monetary_exp,args,5,5);
    }
    return reloq_itit61_convert_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_convert_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_numbers,args,3,3);
    }
    return reloq_itit61_convert_numbers(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_roman_num,args,3,3);
    }
    return reloq_itit61_convert_roman_num(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_roman_num_to_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_roman_num_to_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_roman_num_to_ordinal,args,3,3);
    }
    return reloq_itit61_convert_roman_num_to_ordinal(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_roman_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_roman_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_roman_nums,args,3,3);
    }
    return reloq_itit61_convert_roman_nums(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_single_digit,args,3,3);
    }
    return reloq_itit61_convert_single_digit(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_thousands(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_thousands(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_thousands,args,3,3);
    }
    return reloq_itit61_convert_thousands(state, p0, p1, p2);
}
int32_t reloq_itit61_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_itit61_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_itit61_coord_conj(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_coord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_coord_conj,args,6,6);
    }
    return reloq_itit61_coord_conj(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_itit61_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_itit61_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_itit61_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_itit61_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_itit61_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_count_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_count_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_digits,args,3,3);
    }
    return reloq_itit61_count_digits(state, p0, p1, p2);
}
int32_t reloq_itit61_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_itit61_count_input(state, p0, p1);
}
int32_t reloq_itit61_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_itit61_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_itit61_count_normalized_inp(state, p0);
}
int32_t reloq_itit61_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_itit61_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_itit61_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_itit61_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_itit61_create_ital_syllables(void *, int32_t);
static inline int32_t reloq_call_itit61_create_ital_syllables(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_create_ital_syllables,args,1,1);
    }
    return reloq_itit61_create_ital_syllables(state, p0);
}
int32_t reloq_itit61_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_itit61_create_sentence(state, p0, p1);
}
int32_t reloq_itit61_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_itit61_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_del_ital_extra_syncs(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_del_ital_extra_syncs(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_del_ital_extra_syncs,args,2,2);
    }
    return reloq_itit61_del_ital_extra_syncs(state, p0, p1);
}
int32_t reloq_itit61_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_itit61_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_itit61_delete_2_more(state, p0);
}
int32_t reloq_itit61_delete_comma_sync(void *, int32_t, int32_t);
int32_t reloq_itit61_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delete_inp_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_left,args,3,3);
    }
    return reloq_itit61_delete_inp_from_left(state, p0, p1, p2);
}
int32_t reloq_itit61_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delete_inp_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_right,args,3,3);
    }
    return reloq_itit61_delete_inp_from_right(state, p0, p1, p2);
}
int32_t reloq_itit61_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_itit61_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_itit61_delete_medial_digits(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delete_medial_digits(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_medial_digits,args,2,2);
    }
    return reloq_itit61_delete_medial_digits(state, p0, p1);
}
int32_t reloq_itit61_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_itit61_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_itit61_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_delete_stem_final_vowel(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delete_stem_final_vowel(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_stem_final_vowel,args,2,2);
    }
    return reloq_itit61_delete_stem_final_vowel(state, p0, p1);
}
int32_t reloq_itit61_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_itit61_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_itit61_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_itit61_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_itit61_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_itit61_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_itit61_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_itit61_demote_word(state, p0, p1, p2);
}
int32_t reloq_itit61_det_vs_pro(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_det_vs_pro(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_det_vs_pro,args,7,7);
    }
    return reloq_itit61_det_vs_pro(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_itit61_determine_ital_gender(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_determine_ital_gender(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_determine_ital_gender,args,3,3);
    }
    return reloq_itit61_determine_ital_gender(state, p0, p1, p2);
}
int32_t reloq_itit61_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_itit61_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_disambig_inf(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_disambig_inf(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_inf,args,5,5);
    }
    return reloq_itit61_disambig_inf(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_disambig_ital_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_disambig_ital_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_ital_content_word,args,3,3);
    }
    return reloq_itit61_disambig_ital_content_word(state, p0, p1, p2);
}
int32_t reloq_itit61_disambig_ital_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_disambig_ital_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_ital_functword,args,3,3);
    }
    return reloq_itit61_disambig_ital_functword(state, p0, p1, p2);
}
int32_t reloq_itit61_disambig_pastpart(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_disambig_pastpart(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_pastpart,args,3,3);
    }
    return reloq_itit61_disambig_pastpart(state, p0, p1, p2);
}
int32_t reloq_itit61_disambig_undef_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_disambig_undef_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8)
{
    if(delta_rule_trace) {
        const int32_t args[]={p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_undef_word,args,9,9);
    }
    return reloq_itit61_disambig_undef_word(state, p0, p1, p2, p3, p4, p5, p6, p7, p8);
}
int32_t reloq_itit61_disambiguate_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_disambiguate_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_content_word,args,4,4);
    }
    return reloq_itit61_disambiguate_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_disambiguate_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_disambiguate_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_functword,args,3,3);
    }
    return reloq_itit61_disambiguate_functword(state, p0, p1, p2);
}
int32_t reloq_itit61_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_itit61_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_itit61_distribute_nucdur(state, p0);
}
int32_t reloq_itit61_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_itit61_divide_char_count_token(state, p0, p1);
}
int32_t reloq_itit61_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_itit61_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_itit61_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_itit61_doF(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_itit61_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_itit61_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_itit61_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_itit61_do_FTZ_FTP(state, p0);
}
int32_t reloq_itit61_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_itit61_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_itit61_do_ital_voice_bar(void *, int32_t);
static inline int32_t reloq_call_itit61_do_ital_voice_bar(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_ital_voice_bar,args,1,1);
    }
    return reloq_itit61_do_ital_voice_bar(state, p0);
}
int32_t reloq_itit61_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_itit61_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_itit61_do_non_son_amps(state, p0);
}
int32_t reloq_itit61_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_itit61_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_itit61_do_pt_voicing(state, p0);
}
int32_t reloq_itit61_do_release(void *, int32_t);
static inline int32_t reloq_call_itit61_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_itit61_do_release(state, p0);
}
int32_t reloq_itit61_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_itit61_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_itit61_do_son_amps(state, p0);
}
int32_t reloq_itit61_do_voice_bar(void *, int32_t);
int32_t reloq_itit61_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_itit61_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_itit61_ellipsis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ellipsis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ellipsis,args,3,3);
    }
    return reloq_itit61_ellipsis(state, p0, p1, p2);
}
int32_t reloq_itit61_email_address(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_email_address(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_email_address,args,2,2);
    }
    return reloq_itit61_email_address(state, p0, p1);
}
int32_t reloq_itit61_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_itit61_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_itit61_enclitics(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_enclitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_enclitics,args,5,5);
    }
    return reloq_itit61_enclitics(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_itit61_end_of_sentence(state, p0, p1);
}
int32_t reloq_itit61_end_of_token(void *, int32_t, int32_t);
int32_t reloq_itit61_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_itit61_end_of_word(state, p0, p1);
}
int32_t reloq_itit61_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_itit61_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_estimate_ital_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_estimate_ital_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_ital_nsylls,args,4,4);
    }
    return reloq_itit61_estimate_ital_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_estimate_lang_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_estimate_lang_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_lang_nsylls,args,4,4);
    }
    return reloq_itit61_estimate_lang_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_estimate_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_nsylls,args,4,4);
    }
    return reloq_itit61_estimate_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_exclam_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_exclam_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_exclam_phrase,args,2,2);
    }
    return reloq_itit61_exclam_phrase(state, p0, p1);
}
int32_t reloq_itit61_extend_vals(void *, int32_t);
int32_t reloq_itit61_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_itit61_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_itit61_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_itit61_final_voiced_release(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_final_voiced_release(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_voiced_release,args,3,3);
    }
    return reloq_itit61_final_voiced_release(state, p0, p1, p2);
}
int32_t reloq_itit61_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_itit61_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_itit61_find_acoustic_nuc(state, p0);
}
int32_t reloq_itit61_find_ital_nucleus(void *, int32_t);
static inline int32_t reloq_call_itit61_find_ital_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_ital_nucleus,args,1,1);
    }
    return reloq_itit61_find_ital_nucleus(state, p0);
}
int32_t reloq_itit61_find_ital_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_ital_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_ital_obligatory_phrase,args,4,4);
    }
    return reloq_itit61_find_ital_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_ital_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_ital_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_ital_potential_phrase,args,4,4);
    }
    return reloq_itit61_find_ital_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_ital_proclitic(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_ital_proclitic(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_ital_proclitic,args,4,4);
    }
    return reloq_itit61_find_ital_proclitic(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_lang_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_lang_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_obligatory_phrase,args,4,4);
    }
    return reloq_itit61_find_lang_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_lang_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_lang_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_potential_phrase,args,4,4);
    }
    return reloq_itit61_find_lang_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_itit61_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_most_stressed_syll(void *, int32_t);
static inline int32_t reloq_call_itit61_find_most_stressed_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_most_stressed_syll,args,1,1);
    }
    return reloq_itit61_find_most_stressed_syll(state, p0);
}
int32_t reloq_itit61_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_next_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_word,args,4,4);
    }
    return reloq_itit61_find_next_word(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_itit61_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_itit61_find_nucleus(state, p0);
}
int32_t reloq_itit61_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_itit61_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_previous_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_previous_word,args,4,4);
    }
    return reloq_itit61_find_previous_word(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_proclitic(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_proclitic(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_proclitic,args,4,4);
    }
    return reloq_itit61_find_proclitic(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_find_unpuncted_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_find_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_unpuncted_phrases,args,3,3);
    }
    return reloq_itit61_find_unpuncted_phrases(state, p0, p1, p2);
}
int32_t reloq_itit61_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_itit61_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_first_postnucl_accent,args,1,1);
    }
    return reloq_itit61_first_postnucl_accent(state, p0);
}
int32_t reloq_itit61_first_root_vow(void *, int32_t, int32_t);
int32_t reloq_itit61_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_followed_by_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_followed_by_hyphen,args,3,3);
    }
    return reloq_itit61_followed_by_hyphen(state, p0, p1, p2);
}
int32_t reloq_itit61_follows_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_follows_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_follows_end_quote,args,2,2);
    }
    return reloq_itit61_follows_end_quote(state, p0, p1);
}
int32_t reloq_itit61_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_itit61_forced_synthesis(state, p0, p1);
}
int32_t reloq_itit61_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_itit61_generate_acoustic_vals(state, p0);
}
int32_t reloq_itit61_generate_durations(void *, int32_t);
static inline int32_t reloq_call_itit61_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_itit61_generate_durations(state, p0);
}
int32_t reloq_itit61_generate_intonation(void *, int32_t);
static inline int32_t reloq_call_itit61_generate_intonation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_intonation,args,1,1);
    }
    return reloq_itit61_generate_intonation(state, p0);
}
int32_t reloq_itit61_generate_ital_diaphons(void *, int32_t);
static inline int32_t reloq_call_itit61_generate_ital_diaphons(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_ital_diaphons,args,1,1);
    }
    return reloq_itit61_generate_ital_diaphons(state, p0);
}
int32_t reloq_itit61_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_itit61_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_itit61_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_itit61_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_get_input(void *, int32_t);
static inline int32_t reloq_call_itit61_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_itit61_get_input(state, p0);
}
int32_t reloq_itit61_get_ital_fric_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_fric_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_fric_trans_dur,args,2,2);
    }
    return reloq_itit61_get_ital_fric_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_ital_high_pal_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_high_pal_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_high_pal_trans_dur,args,2,2);
    }
    return reloq_itit61_get_ital_high_pal_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_ital_nas_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_nas_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_nas_trans_dur,args,2,2);
    }
    return reloq_itit61_get_ital_nas_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_ital_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_nuclear_accent,args,4,4);
    }
    return reloq_itit61_get_ital_nuclear_accent(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_get_ital_son_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_son_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_son_trans_dur,args,2,2);
    }
    return reloq_itit61_get_ital_son_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_ital_stop_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_stop_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_stop_trans_dur,args,2,2);
    }
    return reloq_itit61_get_ital_stop_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_ital_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_trans_dur,args,2,2);
    }
    return reloq_itit61_get_ital_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_ital_trill_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_trill_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_trill_trans_dur,args,2,2);
    }
    return reloq_itit61_get_ital_trill_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_ital_vow_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_ital_vow_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_ital_vow_trans_dur,args,2,2);
    }
    return reloq_itit61_get_ital_vow_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_lang_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_lang_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_lang_nuclear_accent,args,4,4);
    }
    return reloq_itit61_get_lang_nuclear_accent(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_itit61_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_itit61_get_nuc_AV(state, p0);
}
int32_t reloq_itit61_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_itit61_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_get_number_info(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_number_info(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_info,args,3,3);
    }
    return reloq_itit61_get_number_info(state, p0, p1, p2);
}
int32_t reloq_itit61_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_itit61_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_get_stanital_non_vowels(void *, int32_t);
static inline int32_t reloq_call_itit61_get_stanital_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanital_non_vowels,args,1,1);
    }
    return reloq_itit61_get_stanital_non_vowels(state, p0);
}
int32_t reloq_itit61_get_stanital_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_itit61_get_stanital_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_stanital_nuc_AV,args,1,1);
    }
    return reloq_itit61_get_stanital_nuc_AV(state, p0);
}
int32_t reloq_itit61_get_tok(void *, int32_t);
static inline int32_t reloq_call_itit61_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_itit61_get_tok(state, p0);
}
int32_t reloq_itit61_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_itit61_get_trans_dur(state, p0, p1);
}
int32_t reloq_itit61_gettok(void *, int32_t);
static inline int32_t reloq_call_itit61_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_itit61_gettok(state, p0);
}
int32_t reloq_itit61_gramm_cat_dict_lookup(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_itit61_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_handle_anche(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_anche(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_anche,args,3,3);
    }
    return reloq_itit61_handle_anche(state, p0, p1, p2);
}
int32_t reloq_itit61_handle_dei(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_dei(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_dei,args,3,3);
    }
    return reloq_itit61_handle_dei(state, p0, p1, p2);
}
int32_t reloq_itit61_handle_destressed_verbs(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_destressed_verbs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_destressed_verbs,args,4,4);
    }
    return reloq_itit61_handle_destressed_verbs(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_handle_dopo(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_dopo(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_dopo,args,3,3);
    }
    return reloq_itit61_handle_dopo(state, p0, p1, p2);
}
int32_t reloq_itit61_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_itit61_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_itit61_handle_era(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_era(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_era,args,3,3);
    }
    return reloq_itit61_handle_era(state, p0, p1, p2);
}
int32_t reloq_itit61_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_itit61_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_itit61_handle_loro(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_loro(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_loro,args,3,3);
    }
    return reloq_itit61_handle_loro(state, p0, p1, p2);
}
int32_t reloq_itit61_handle_non_phrasal_commas(void *, int32_t);
static inline int32_t reloq_call_itit61_handle_non_phrasal_commas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_phrasal_commas,args,1,1);
    }
    return reloq_itit61_handle_non_phrasal_commas(state, p0);
}
int32_t reloq_itit61_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_itit61_handle_pause(state, p0, p1);
}
int32_t reloq_itit61_handle_se(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_se(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_se,args,7,7);
    }
    return reloq_itit61_handle_se(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_itit61_handle_sei(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_handle_sei(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_sei,args,5,5);
    }
    return reloq_itit61_handle_sei(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_low_star_tone,args,1,1);
    }
    return reloq_itit61_high_low_star_tone(state, p0);
}
int32_t reloq_itit61_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_star_low_tone,args,1,1);
    }
    return reloq_itit61_high_star_low_tone(state, p0);
}
int32_t reloq_itit61_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_tone,args,1,1);
    }
    return reloq_itit61_high_tone(state, p0);
}
int32_t reloq_itit61_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen,args,3,3);
    }
    return reloq_itit61_hyphen(state, p0, p1, p2);
}
int32_t reloq_itit61_hyphen_between_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen_between_nums,args,3,3);
    }
    return reloq_itit61_hyphen_between_nums(state, p0, p1, p2);
}
int32_t reloq_itit61_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_itit61_in_brackets(state, p0, p1, p2);
}
int32_t reloq_itit61_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_itit61_in_quotes(state, p0, p1, p2);
}
int32_t reloq_itit61_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_in_syllable(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_syllable,args,4,4);
    }
    return reloq_itit61_in_syllable(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_itit61_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_itit61_increment_output_file_number(state, p0);
}
int32_t reloq_itit61_init_current_word_ptrs(void *, int32_t);
int32_t reloq_itit61_init_delta(void *, int32_t);
static inline int32_t reloq_call_itit61_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_itit61_init_delta(state, p0);
}
int32_t reloq_itit61_init_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_gcat_vars,args,1,1);
    }
    return reloq_itit61_init_gcat_vars(state, p0);
}
int32_t reloq_itit61_init_ital_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_ital_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_ital_vars,args,1,1);
    }
    return reloq_itit61_init_ital_vars(state, p0);
}
int32_t reloq_itit61_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_itit61_init_lang_vars(state, p0);
}
int32_t reloq_itit61_init_nuclwrd_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_nuclwrd_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_nuclwrd_vars,args,1,1);
    }
    return reloq_itit61_init_nuclwrd_vars(state, p0);
}
int32_t reloq_itit61_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_itit61_init_phr_feature_vars(state, p0);
}
int32_t reloq_itit61_init_phrase_inton_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_phrase_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_inton_vars,args,1,1);
    }
    return reloq_itit61_init_phrase_inton_vars(state, p0);
}
int32_t reloq_itit61_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_itit61_init_phrase_vars(state, p0);
}
int32_t reloq_itit61_init_platform(void *, int32_t);
static inline int32_t reloq_call_itit61_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_itit61_init_platform(state, p0);
}
int32_t reloq_itit61_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_itit61_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_itit61_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_itit61_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_itit61_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_inton_vars,args,1,1);
    }
    return reloq_itit61_init_syll_inton_vars(state, p0);
}
int32_t reloq_itit61_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_itit61_init_syll_vars(state, p0);
}
int32_t reloq_itit61_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_itit61_init_symbolic_vars(state, p0);
}
int32_t reloq_itit61_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_itit61_init_tm_vars(state, p0);
}
int32_t reloq_itit61_init_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_itit61_init_vars(state, p0);
}
int32_t reloq_itit61_init_voice_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_voice_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_voice_vars,args,1,1);
    }
    return reloq_itit61_init_voice_vars(state, p0);
}
int32_t reloq_itit61_init_word(void *, int32_t);
static inline int32_t reloq_call_itit61_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_itit61_init_word(state, p0);
}
int32_t reloq_itit61_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_inton_vars,args,1,1);
    }
    return reloq_itit61_init_word_inton_vars(state, p0);
}
int32_t reloq_itit61_init_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_itit61_init_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_ptrs,args,1,1);
    }
    return reloq_itit61_init_word_ptrs(state, p0);
}
int32_t reloq_itit61_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_itit61_init_word_vars(state, p0);
}
int32_t reloq_itit61_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_itit61_insert_acoustic_vals(state, p0);
}
int32_t reloq_itit61_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_itit61_insert_burst_dur(state, p0);
}
int32_t reloq_itit61_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_itit61_insert_char_count(state, p0);
}
int32_t reloq_itit61_insert_clitic_vowel(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_clitic_vowel(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_clitic_vowel,args,2,2);
    }
    return reloq_itit61_insert_clitic_vowel(state, p0, p1);
}
int32_t reloq_itit61_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_comma_phrase,args,3,3);
    }
    return reloq_itit61_insert_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_itit61_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_insert_f0(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_f0(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_f0,args,4,4);
    }
    return reloq_itit61_insert_f0(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_insert_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_insert_ital_number_one(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_ital_number_one(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_ital_number_one,args,3,3);
    }
    return reloq_itit61_insert_ital_number_one(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_ital_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_ital_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_ital_spr_phone,args,3,3);
    }
    return reloq_itit61_insert_ital_spr_phone(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_itit61_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_itit61_insert_pause_durs(state, p0);
}
int32_t reloq_itit61_insert_penult_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_itit61_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_final_tones,args,1,1);
    }
    return reloq_itit61_insert_phrase_final_tones(state, p0);
}
int32_t reloq_itit61_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_itit61_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_preset_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_preset_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_preset_phrase_tone,args,1,1);
    }
    return reloq_itit61_insert_preset_phrase_tone(state, p0);
}
int32_t reloq_itit61_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_itit61_insert_space(state, p0, p1);
}
int32_t reloq_itit61_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_itit61_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_itit61_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_itit61_insert_syll_level_vals(state, p0);
}
int32_t reloq_itit61_insert_target_t3(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_target_t3(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_target_t3,args,1,1);
    }
    return reloq_itit61_insert_target_t3(state, p0);
}
int32_t reloq_itit61_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_itit61_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_itit61_insert_transitions(state, p0);
}
int32_t reloq_itit61_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_itit61_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrases,args,4,4);
    }
    return reloq_itit61_insert_unpuncted_phrases(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_itit61_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_itit61_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_itit61_insert_word(state, p0, p1, p2);
}
int32_t reloq_itit61_insert_zero_sylls(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_inside_orig_inp_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_inside_orig_inp_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_inside_orig_inp_token,args,2,2);
    }
    return reloq_itit61_inside_orig_inp_token(state, p0, p1);
}
int32_t reloq_itit61_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_itit61_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_itit61_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_interpret_single_char_modes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_interpret_single_char_modes,args,3,3);
    }
    return reloq_itit61_interpret_single_char_modes(state, p0, p1, p2);
}
int32_t reloq_itit61_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_itit61_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_itit61_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_itit61_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_itit61_is_ambiguous_adv(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_ambiguous_adv(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ambiguous_adv,args,3,3);
    }
    return reloq_itit61_is_ambiguous_adv(state, p0, p1, p2);
}
int32_t reloq_itit61_is_hyphenated_word(void *, int32_t, int32_t);
int32_t reloq_itit61_is_in_parentheses(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_is_ital_adverb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_ital_adverb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ital_adverb,args,3,3);
    }
    return reloq_itit61_is_ital_adverb(state, p0, p1, p2);
}
int32_t reloq_itit61_is_ital_clsd_class(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_ital_clsd_class(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ital_clsd_class,args,3,3);
    }
    return reloq_itit61_is_ital_clsd_class(state, p0, p1, p2);
}
int32_t reloq_itit61_is_ital_determ(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_ital_determ(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ital_determ,args,3,3);
    }
    return reloq_itit61_is_ital_determ(state, p0, p1, p2);
}
int32_t reloq_itit61_is_ital_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_ital_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ital_fraction,args,3,3);
    }
    return reloq_itit61_is_ital_fraction(state, p0, p1, p2);
}
int32_t reloq_itit61_is_ital_nounadj(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_ital_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ital_nounadj,args,4,4);
    }
    return reloq_itit61_is_ital_nounadj(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_is_ital_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_ital_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ital_verb,args,3,3);
    }
    return reloq_itit61_is_ital_verb(state, p0, p1, p2);
}
int32_t reloq_itit61_is_not_prfxable(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_not_prfxable(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_not_prfxable,args,3,3);
    }
    return reloq_itit61_is_not_prfxable(state, p0, p1, p2);
}
int32_t reloq_itit61_is_pathname(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_pathname(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pathname,args,2,2);
    }
    return reloq_itit61_is_pathname(state, p0, p1);
}
int32_t reloq_itit61_is_proper_name(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_proper_name(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_proper_name,args,2,2);
    }
    return reloq_itit61_is_proper_name(state, p0, p1);
}
int32_t reloq_itit61_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_itit61_is_spr(state, p0, p1, p2);
}
int32_t reloq_itit61_is_target_syll(void *, int32_t);
static inline int32_t reloq_call_itit61_is_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_target_syll,args,1,1);
    }
    return reloq_itit61_is_target_syll(state, p0);
}
int32_t reloq_itit61_is_unambig_nounadj(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_unambig_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_unambig_nounadj,args,4,4);
    }
    return reloq_itit61_is_unambig_nounadj(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_is_verb_in_ambiguous_set(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_verb_in_ambiguous_set(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_verb_in_ambiguous_set,args,3,3);
    }
    return reloq_itit61_is_verb_in_ambiguous_set(state, p0, p1, p2);
}
int32_t reloq_itit61_is_verb_in_common_set(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_is_verb_in_common_set(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_verb_in_common_set,args,3,3);
    }
    return reloq_itit61_is_verb_in_common_set(state, p0, p1, p2);
}
int32_t reloq_itit61_isitAlt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_isitAlt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitAlt,args,3,3);
    }
    return reloq_itit61_isitAlt(state, p0, p1, p2);
}
int32_t reloq_itit61_isitWH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_isitWH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitWH,args,3,3);
    }
    return reloq_itit61_isitWH(state, p0, p1, p2);
}
int32_t reloq_itit61_isit_ital_Alt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_isit_ital_Alt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_ital_Alt,args,3,3);
    }
    return reloq_itit61_isit_ital_Alt(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_abbr,args,3,3);
    }
    return reloq_itit61_ital_abbr(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_acrotest,args,3,3);
    }
    return reloq_itit61_ital_acrotest(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_boundary_tone(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_boundary_tone(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_boundary_tone,args,2,2);
    }
    return reloq_itit61_ital_boundary_tone(state, p0, p1);
}
int32_t reloq_itit61_ital_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_char_name,args,3,3);
    }
    return reloq_itit61_ital_char_name(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_con_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_con_vals,args,1,1);
    }
    return reloq_itit61_ital_con_vals(state, p0);
}
int32_t reloq_itit61_ital_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_cons_durs,args,3,3);
    }
    return reloq_itit61_ital_cons_durs(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_context_adjust(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_context_adjust(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_context_adjust,args,4,4);
    }
    return reloq_itit61_ital_context_adjust(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_ital_continuation_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_continuation_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_continuation_phrase,args,3,3);
    }
    return reloq_itit61_ital_continuation_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_day_date(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_day_date(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_day_date,args,8,8);
    }
    return reloq_itit61_ital_day_date(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_itit61_ital_dental_Fv(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_dental_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_dental_Fv,args,1,1);
    }
    return reloq_itit61_ital_dental_Fv(state, p0);
}
int32_t reloq_itit61_ital_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_first_postnucl_accent,args,1,1);
    }
    return reloq_itit61_ital_first_postnucl_accent(state, p0);
}
int32_t reloq_itit61_ital_funct_words(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_funct_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_funct_words,args,5,5);
    }
    return reloq_itit61_ital_funct_words(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_ital_glid_dur(void *, int32_t);
int32_t reloq_itit61_ital_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_high_low_star_tone,args,1,1);
    }
    return reloq_itit61_ital_high_low_star_tone(state, p0);
}
int32_t reloq_itit61_ital_high_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_high_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_high_pal_Fv,args,1,1);
    }
    return reloq_itit61_ital_high_pal_Fv(state, p0);
}
int32_t reloq_itit61_ital_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_high_star_low_tone,args,1,1);
    }
    return reloq_itit61_ital_high_star_low_tone(state, p0);
}
int32_t reloq_itit61_ital_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_high_tone,args,1,1);
    }
    return reloq_itit61_ital_high_tone(state, p0);
}
int32_t reloq_itit61_ital_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_hyphen_between_nums,args,5,5);
    }
    return reloq_itit61_ital_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_ital_labial_Fv(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_labial_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_labial_Fv,args,1,1);
    }
    return reloq_itit61_ital_labial_Fv(state, p0);
}
int32_t reloq_itit61_ital_legal_final_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_legal_final_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_legal_final_cons,args,2,2);
    }
    return reloq_itit61_ital_legal_final_cons(state, p0, p1);
}
int32_t reloq_itit61_ital_legal_initial_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_legal_initial_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_legal_initial_cons,args,2,2);
    }
    return reloq_itit61_ital_legal_initial_cons(state, p0, p1);
}
int32_t reloq_itit61_ital_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_low_high_star_tone,args,1,1);
    }
    return reloq_itit61_ital_low_high_star_tone(state, p0);
}
int32_t reloq_itit61_ital_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_low_star_high_tone,args,1,1);
    }
    return reloq_itit61_ital_low_star_high_tone(state, p0);
}
int32_t reloq_itit61_ital_low_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_low_tone,args,1,1);
    }
    return reloq_itit61_ital_low_tone(state, p0);
}
int32_t reloq_itit61_ital_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_monetary_exp,args,3,3);
    }
    return reloq_itit61_ital_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_non_phrasal_comma,args,2,2);
    }
    return reloq_itit61_ital_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_itit61_ital_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_itit61_ital_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_itit61_ital_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_itit61_ital_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_itit61_ital_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_nucl_high_tone,args,1,1);
    }
    return reloq_itit61_ital_nucl_high_tone(state, p0);
}
int32_t reloq_itit61_ital_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_itit61_ital_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_itit61_ital_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_itit61_ital_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_itit61_ital_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_nucl_low_tone,args,1,1);
    }
    return reloq_itit61_ital_nucl_low_tone(state, p0);
}
int32_t reloq_itit61_ital_numbers(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_ital_numwords(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_numwords(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_numwords,args,3,3);
    }
    return reloq_itit61_ital_numwords(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_pal_Fv,args,1,1);
    }
    return reloq_itit61_ital_pal_Fv(state, p0);
}
int32_t reloq_itit61_ital_ph_D_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_D_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_D_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_D_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_F_dur(void *, int32_t);
int32_t reloq_itit61_ital_ph_G(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_G(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_G,args,1,1);
    }
    return reloq_itit61_ital_ph_G(state, p0);
}
int32_t reloq_itit61_ital_ph_G_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_G_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_G_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_G_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_L(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_L(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_L,args,1,1);
    }
    return reloq_itit61_ital_ph_L(state, p0);
}
int32_t reloq_itit61_ital_ph_L_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_L_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_L_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_L_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_N(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_N(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_N,args,1,1);
    }
    return reloq_itit61_ital_ph_N(state, p0);
}
int32_t reloq_itit61_ital_ph_N_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_N_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_N_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_N_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_R(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_R(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_R,args,1,1);
    }
    return reloq_itit61_ital_ph_R(state, p0);
}
int32_t reloq_itit61_ital_ph_R_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_R_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_R_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_R_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_S(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_S,args,1,1);
    }
    return reloq_itit61_ital_ph_S(state, p0);
}
int32_t reloq_itit61_ital_ph_S_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_S_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_S_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_S_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_T_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_T_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_T_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_T_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_Z(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_Z,args,1,1);
    }
    return reloq_itit61_ital_ph_Z(state, p0);
}
int32_t reloq_itit61_ital_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_Z_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_Z_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_b(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_b,args,1,1);
    }
    return reloq_itit61_ital_ph_b(state, p0);
}
int32_t reloq_itit61_ital_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_b_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_b_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_d(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_d,args,1,1);
    }
    return reloq_itit61_ital_ph_d(state, p0);
}
int32_t reloq_itit61_ital_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_d_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_d_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_f(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_f(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_f,args,1,1);
    }
    return reloq_itit61_ital_ph_f(state, p0);
}
int32_t reloq_itit61_ital_ph_f_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_f_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_f_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_f_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_g(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_g,args,1,1);
    }
    return reloq_itit61_ital_ph_g(state, p0);
}
int32_t reloq_itit61_ital_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_g_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_g_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_k(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_k,args,1,1);
    }
    return reloq_itit61_ital_ph_k(state, p0);
}
int32_t reloq_itit61_ital_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_k_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_k_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_l(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_l,args,1,1);
    }
    return reloq_itit61_ital_ph_l(state, p0);
}
int32_t reloq_itit61_ital_ph_l_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_l_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_l_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_l_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_m(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_m,args,1,1);
    }
    return reloq_itit61_ital_ph_m(state, p0);
}
int32_t reloq_itit61_ital_ph_m_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_m_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_m_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_m_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_n(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_n,args,1,1);
    }
    return reloq_itit61_ital_ph_n(state, p0);
}
int32_t reloq_itit61_ital_ph_n_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_n_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_n_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_n_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_p(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_p,args,1,1);
    }
    return reloq_itit61_ital_ph_p(state, p0);
}
int32_t reloq_itit61_ital_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_p_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_p_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_r(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_r,args,1,1);
    }
    return reloq_itit61_ital_ph_r(state, p0);
}
int32_t reloq_itit61_ital_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_r_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_r_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_s(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_s,args,1,1);
    }
    return reloq_itit61_ital_ph_s(state, p0);
}
int32_t reloq_itit61_ital_ph_s_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_s_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_s_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_s_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_t(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_t,args,1,1);
    }
    return reloq_itit61_ital_ph_t(state, p0);
}
int32_t reloq_itit61_ital_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_t_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_t_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_v(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_v(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_v,args,1,1);
    }
    return reloq_itit61_ital_ph_v(state, p0);
}
int32_t reloq_itit61_ital_ph_v_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_v_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_v_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_v_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_w(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_w,args,1,1);
    }
    return reloq_itit61_ital_ph_w(state, p0);
}
int32_t reloq_itit61_ital_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_w_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_w_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_y(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_y,args,1,1);
    }
    return reloq_itit61_ital_ph_y(state, p0);
}
int32_t reloq_itit61_ital_ph_y_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_y_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_y_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_y_dur(state, p0);
}
int32_t reloq_itit61_ital_ph_z(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_z,args,1,1);
    }
    return reloq_itit61_ital_ph_z(state, p0);
}
int32_t reloq_itit61_ital_ph_z_dur(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_ph_z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_ph_z_dur,args,1,1);
    }
    return reloq_itit61_ital_ph_z_dur(state, p0);
}
int32_t reloq_itit61_ital_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_phr_initial_F0,args,1,1);
    }
    return reloq_itit61_ital_phr_initial_F0(state, p0);
}
int32_t reloq_itit61_ital_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_phrase_tone,args,1,1);
    }
    return reloq_itit61_ital_phrase_tone(state, p0);
}
int32_t reloq_itit61_ital_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_position_t_ptrs,args,1,1);
    }
    return reloq_itit61_ital_position_t_ptrs(state, p0);
}
int32_t reloq_itit61_ital_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_post_comma_phrase,args,3,3);
    }
    return reloq_itit61_ital_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_postnucl_f0,args,1,1);
    }
    return reloq_itit61_ital_postnucl_f0(state, p0);
}
int32_t reloq_itit61_ital_postnucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_postnucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_postnucl_high_tone,args,1,1);
    }
    return reloq_itit61_ital_postnucl_high_tone(state, p0);
}
int32_t reloq_itit61_ital_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_prenucl_f0,args,1,1);
    }
    return reloq_itit61_ital_prenucl_f0(state, p0);
}
int32_t reloq_itit61_ital_roots(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_roots(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_roots,args,3,3);
    }
    return reloq_itit61_ital_roots(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_roots_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_roots_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_roots_dict_lookup,args,3,3);
    }
    return reloq_itit61_ital_roots_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_said_phrase,args,4,4);
    }
    return reloq_itit61_ital_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_ital_specific_word_cases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_specific_word_cases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_specific_word_cases,args,4,4);
    }
    return reloq_itit61_ital_specific_word_cases(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_ital_symbolic_rules(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_symbolic_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_symbolic_rules,args,1,1);
    }
    return reloq_itit61_ital_symbolic_rules(state, p0);
}
int32_t reloq_itit61_ital_tag_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_tag_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_tag_phrase,args,3,3);
    }
    return reloq_itit61_ital_tag_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_tok_dict,args,3,3);
    }
    return reloq_itit61_ital_tok_dict(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_trans_features(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_trans_features,args,1,1);
    }
    return reloq_itit61_ital_trans_features(state, p0);
}
int32_t reloq_itit61_ital_trill_Fv(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_trill_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_trill_Fv,args,1,1);
    }
    return reloq_itit61_ital_trill_Fv(state, p0);
}
int32_t reloq_itit61_ital_velar_Fv(void *, int32_t);
static inline int32_t reloq_call_itit61_ital_velar_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_ital_velar_Fv,args,1,1);
    }
    return reloq_itit61_ital_velar_Fv(state, p0);
}
int32_t reloq_itit61_ital_vocative_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_vocative_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_vocative_phrase,args,3,3);
    }
    return reloq_itit61_ital_vocative_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_ital_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_ital_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ital_words,args,3,3);
    }
    return reloq_itit61_ital_words(state, p0, p1, p2);
}
int32_t reloq_itit61_lang_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_lang_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_hyphen_between_nums,args,5,5);
    }
    return reloq_itit61_lang_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_lang_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_lang_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_non_phrasal_comma,args,2,2);
    }
    return reloq_itit61_lang_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_itit61_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_lang_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_phrase_level_rules,args,3,3);
    }
    return reloq_itit61_lang_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_itit61_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_itit61_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_itit61_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_itit61_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_itit61_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_itit61_longdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_longdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_longdash,args,3,3);
    }
    return reloq_itit61_longdash(state, p0, p1, p2);
}
int32_t reloq_itit61_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_high_star_tone,args,1,1);
    }
    return reloq_itit61_low_high_star_tone(state, p0);
}
int32_t reloq_itit61_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_star_high_tone,args,1,1);
    }
    return reloq_itit61_low_star_high_tone(state, p0);
}
int32_t reloq_itit61_low_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_tone,args,1,1);
    }
    return reloq_itit61_low_tone(state, p0);
}
int32_t reloq_itit61_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_make_ital_phon_adjustments(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_make_ital_phon_adjustments(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_make_ital_phon_adjustments,args,3,3);
    }
    return reloq_itit61_make_ital_phon_adjustments(state, p0, p1, p2);
}
int32_t reloq_itit61_mark_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_content_word,args,5,5);
    }
    return reloq_itit61_mark_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_mark_function_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_function_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_function_word,args,5,5);
    }
    return reloq_itit61_mark_function_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_mark_ital_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_ital_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_ital_content_word,args,5,5);
    }
    return reloq_itit61_mark_ital_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_mark_ital_subcategory(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_ital_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_ital_subcategory,args,3,3);
    }
    return reloq_itit61_mark_ital_subcategory(state, p0, p1, p2);
}
int32_t reloq_itit61_mark_non_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_non_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_non_letter,args,3,3);
    }
    return reloq_itit61_mark_non_letter(state, p0, p1, p2);
}
int32_t reloq_itit61_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_itit61_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_mark_potential_phrase_breaks(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_potential_phrase_breaks(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_potential_phrase_breaks,args,4,4);
    }
    return reloq_itit61_mark_potential_phrase_breaks(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_spr_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_spr_syll_vals,args,6,6);
    }
    return reloq_itit61_mark_spr_syll_vals(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_mark_subcategory(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_subcategory,args,4,4);
    }
    return reloq_itit61_mark_subcategory(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_mark_syll_stress(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_mark_uppercase(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_mark_word_categories(void *, int32_t);
static inline int32_t reloq_call_itit61_mark_word_categories(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_categories,args,1,1);
    }
    return reloq_itit61_mark_word_categories(state, p0);
}
int32_t reloq_itit61_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_word_str_annot(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_str_annot,args,3,3);
    }
    return reloq_itit61_mark_word_str_annot(state, p0, p1, p2);
}
int32_t reloq_itit61_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_itit61_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_itit61_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_itit61_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_itit61_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_itit61_misc_chars(state, p0, p1, p2);
}
int32_t reloq_itit61_modify_breathiness_for_femV(void *, int32_t);
static inline int32_t reloq_call_itit61_modify_breathiness_for_femV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modify_breathiness_for_femV,args,1,1);
    }
    return reloq_itit61_modify_breathiness_for_femV(state, p0);
}
int32_t reloq_itit61_modulate_burst(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_modulate_burst(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_burst,args,2,2);
    }
    return reloq_itit61_modulate_burst(state, p0, p1);
}
int32_t reloq_itit61_modulate_nas_AV(void *, int32_t);
static inline int32_t reloq_call_itit61_modulate_nas_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_nas_AV,args,1,1);
    }
    return reloq_itit61_modulate_nas_AV(state, p0);
}
int32_t reloq_itit61_modulate_noise(void *, int32_t);
static inline int32_t reloq_call_itit61_modulate_noise(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_noise,args,1,1);
    }
    return reloq_itit61_modulate_noise(state, p0);
}
int32_t reloq_itit61_modulate_pwind(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_modulate_pwind(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_pwind,args,2,2);
    }
    return reloq_itit61_modulate_pwind(state, p0, p1);
}
int32_t reloq_itit61_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_itit61_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_molto_words(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_molto_words(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_molto_words,args,5,5);
    }
    return reloq_itit61_molto_words(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_monetary_exp,args,3,3);
    }
    return reloq_itit61_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_itit61_nasalize_vow(void *, int32_t);
static inline int32_t reloq_call_itit61_nasalize_vow(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nasalize_vow,args,1,1);
    }
    return reloq_itit61_nasalize_vow(state, p0);
}
int32_t reloq_itit61_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_itit61_no_readin(state, p0, p1);
}
int32_t reloq_itit61_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_itit61_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_itit61_no_words_in_delta(state, p0);
}
int32_t reloq_itit61_normalize_ital_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_normalize_ital_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_ital_letters,args,3,3);
    }
    return reloq_itit61_normalize_ital_letters(state, p0, p1, p2);
}
int32_t reloq_itit61_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_itit61_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_itit61_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_itit61_normalize_text(state, p0, p1, p2);
}
int32_t reloq_itit61_noun_vs_subord(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_noun_vs_subord(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_subord,args,5,5);
    }
    return reloq_itit61_noun_vs_subord(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_nucl_f0(void *, int32_t);
static inline int32_t reloq_call_itit61_nucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_f0,args,1,1);
    }
    return reloq_itit61_nucl_f0(state, p0);
}
int32_t reloq_itit61_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_itit61_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_itit61_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_itit61_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_itit61_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_tone,args,1,1);
    }
    return reloq_itit61_nucl_high_tone(state, p0);
}
int32_t reloq_itit61_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_itit61_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_itit61_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_itit61_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_itit61_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_tone,args,1,1);
    }
    return reloq_itit61_nucl_low_tone(state, p0);
}
int32_t reloq_itit61_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_itit61_nucleus_vals(state, p0);
}
int32_t reloq_itit61_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_itit61_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_itit61_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_number_stress(void *, int32_t);
int32_t reloq_itit61_one_ital_syllable(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_one_ital_syllable(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_one_ital_syllable,args,3,3);
    }
    return reloq_itit61_one_ital_syllable(state, p0, p1, p2);
}
int32_t reloq_itit61_parenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_parenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_parenthesis,args,3,3);
    }
    return reloq_itit61_parenthesis(state, p0, p1, p2);
}
int32_t reloq_itit61_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_itit61_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_itit61_period_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_itit61_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_itit61_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phr_initial_F0,args,1,1);
    }
    return reloq_itit61_phr_initial_F0(state, p0);
}
int32_t reloq_itit61_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_itit61_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_phrase_final_adjust(void *, int32_t);
static inline int32_t reloq_call_itit61_phrase_final_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_final_adjust,args,1,1);
    }
    return reloq_itit61_phrase_final_adjust(state, p0);
}
int32_t reloq_itit61_phrase_level_printouts(void *, int32_t);
static inline int32_t reloq_call_itit61_phrase_level_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_printouts,args,1,1);
    }
    return reloq_itit61_phrase_level_printouts(state, p0);
}
int32_t reloq_itit61_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_itit61_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_tone,args,1,1);
    }
    return reloq_itit61_phrase_tone(state, p0);
}
int32_t reloq_itit61_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_itit61_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_position_t_ptrs,args,1,1);
    }
    return reloq_itit61_position_t_ptrs(state, p0);
}
int32_t reloq_itit61_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_comma_phrase,args,3,3);
    }
    return reloq_itit61_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_itit61_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_postnucl_f0,args,1,1);
    }
    return reloq_itit61_postnucl_f0(state, p0);
}
int32_t reloq_itit61_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_itit61_potential_sent_end(state, p0, p1);
}
int32_t reloq_itit61_precedes_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_precedes_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_end_quote,args,2,2);
    }
    return reloq_itit61_precedes_end_quote(state, p0, p1);
}
int32_t reloq_itit61_precedes_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_precedes_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_number,args,3,3);
    }
    return reloq_itit61_precedes_number(state, p0, p1, p2);
}
int32_t reloq_itit61_preceding_ital_sing(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_preceding_ital_sing(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_preceding_ital_sing,args,2,2);
    }
    return reloq_itit61_preceding_ital_sing(state, p0, p1);
}
int32_t reloq_itit61_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_itit61_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_prenucl_f0,args,1,1);
    }
    return reloq_itit61_prenucl_f0(state, p0);
}
int32_t reloq_itit61_prep_vs_nounadj(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_prep_vs_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prep_vs_nounadj,args,7,7);
    }
    return reloq_itit61_prep_vs_nounadj(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_itit61_prep_vs_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_prep_vs_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prep_vs_verb,args,3,3);
    }
    return reloq_itit61_prep_vs_verb(state, p0, p1, p2);
}
int32_t reloq_itit61_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_itit61_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_itit61_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_itit61_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_itit61_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_itit61_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_itit61_print_interword_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_print_interword_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_interword_chars,args,3,3);
    }
    return reloq_itit61_print_interword_chars(state, p0, p1, p2);
}
int32_t reloq_itit61_print_ital_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_print_ital_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_ital_SPR,args,3,3);
    }
    return reloq_itit61_print_ital_SPR(state, p0, p1, p2);
}
int32_t reloq_itit61_print_normalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_print_normalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_normalized_inp,args,3,3);
    }
    return reloq_itit61_print_normalized_inp(state, p0, p1, p2);
}
int32_t reloq_itit61_print_prompt(void *, int32_t);
static inline int32_t reloq_call_itit61_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_itit61_print_prompt(state, p0);
}
int32_t reloq_itit61_print_spr(void *, int32_t);
static inline int32_t reloq_call_itit61_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_itit61_print_spr(state, p0);
}
int32_t reloq_itit61_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_itit61_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_itit61_print_word_stress(state, p0, p1);
}
int32_t reloq_itit61_printouts(void *, int32_t);
static inline int32_t reloq_call_itit61_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_itit61_printouts(state, p0);
}
int32_t reloq_itit61_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_itit61_process_annotation(state, p0, p1, p2);
}
int32_t reloq_itit61_process_final_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_process_final_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_apostrophe,args,3,3);
    }
    return reloq_itit61_process_final_apostrophe(state, p0, p1, p2);
}
int32_t reloq_itit61_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_process_final_parenthetical(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_parenthetical,args,3,3);
    }
    return reloq_itit61_process_final_parenthetical(state, p0, p1, p2);
}
int32_t reloq_itit61_process_hostname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_process_hostname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_hostname,args,3,3);
    }
    return reloq_itit61_process_hostname(state, p0, p1, p2);
}
int32_t reloq_itit61_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_itit61_process_input(state, p0, p1);
}
int32_t reloq_itit61_process_pathname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_process_pathname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_pathname,args,3,3);
    }
    return reloq_itit61_process_pathname(state, p0, p1, p2);
}
int32_t reloq_itit61_process_proclitic(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_process_proclitic(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_proclitic,args,4,4);
    }
    return reloq_itit61_process_proclitic(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_itit61_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_proclitics(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_proclitics(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_proclitics,args,3,3);
    }
    return reloq_itit61_proclitics(state, p0, p1, p2);
}
int32_t reloq_itit61_prod_prefixes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_prod_prefixes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prod_prefixes,args,3,3);
    }
    return reloq_itit61_prod_prefixes(state, p0, p1, p2);
}
int32_t reloq_itit61_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_itit61_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_itit61_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_itit61_punctuation(state, p0, p1, p2);
}
int32_t reloq_itit61_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_itit61_question_phrase(state, p0, p1, p2);
}
int32_t reloq_itit61_quote_mark(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_quote_mark(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_quote_mark,args,3,3);
    }
    return reloq_itit61_quote_mark(state, p0, p1, p2);
}
int32_t reloq_itit61_read_end(void *, int32_t);
static inline int32_t reloq_call_itit61_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_itit61_read_end(state, p0);
}
int32_t reloq_itit61_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_itit61_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_itit61_realize_prenucl_tone(void *, int32_t);
static inline int32_t reloq_call_itit61_realize_prenucl_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_realize_prenucl_tone,args,1,1);
    }
    return reloq_itit61_realize_prenucl_tone(state, p0);
}
int32_t reloq_itit61_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_itit61_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_itit61_register_SAPI_voices(state, p0);
}
int32_t reloq_itit61_register_ital_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_register_ital_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_ital_phones,args,3,3);
    }
    return reloq_itit61_register_ital_phones(state, p0, p1, p2);
}
int32_t reloq_itit61_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_itit61_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_reset_endptr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_endptr,args,4,4);
    }
    return reloq_itit61_reset_endptr(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_reset_enum_ptr(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_reset_enum_ptr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_enum_ptr,args,2,2);
    }
    return reloq_itit61_reset_enum_ptr(state, p0, p1);
}
int32_t reloq_itit61_reset_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_reset_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_gcat_vars,args,1,1);
    }
    return reloq_itit61_reset_gcat_vars(state, p0);
}
int32_t reloq_itit61_reset_ital_sent_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_reset_ital_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_ital_sent_vars,args,1,1);
    }
    return reloq_itit61_reset_ital_sent_vars(state, p0);
}
int32_t reloq_itit61_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_itit61_reset_lang_sent_vars(state, p0);
}
int32_t reloq_itit61_reset_left_endptr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_reset_left_endptr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_left_endptr,args,3,3);
    }
    return reloq_itit61_reset_left_endptr(state, p0, p1, p2);
}
int32_t reloq_itit61_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_itit61_reset_phrase_vars(state, p0);
}
int32_t reloq_itit61_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_itit61_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_itit61_reset_sent_vars(state, p0);
}
int32_t reloq_itit61_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_itit61_reset_token_vars(state, p0, p1);
}
int32_t reloq_itit61_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_itit61_reset_word_vals(state, p0, p1);
}
int32_t reloq_itit61_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_itit61_run_filters(state, p0, p1, p2);
}
int32_t reloq_itit61_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_itit61_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_itit61_run_speech_module(state, p0);
}
int32_t reloq_itit61_run_text_module(void *, int32_t);
static inline int32_t reloq_call_itit61_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_itit61_run_text_module(state, p0);
}
int32_t reloq_itit61_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_said_phrase,args,4,4);
    }
    return reloq_itit61_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_itit61_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_itit61_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_itit61_scale_word_final_trans(state, p0);
}
int32_t reloq_itit61_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_itit61_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_itit61_semicolon_phrase(state, p0);
}
int32_t reloq_itit61_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_itit61_separate_tokens(state, p0, p1);
}
int32_t reloq_itit61_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_itit61_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_itit61_set_f0_range_ptrs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_set_f0_range_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_range_ptrs,args,5,5);
    }
    return reloq_itit61_set_f0_range_ptrs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_itit61_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_itit61_set_f0_wind_and_base(state, p0);
}
int32_t reloq_itit61_set_first_postnuc_value(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_set_first_postnuc_value(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_first_postnuc_value,args,2,2);
    }
    return reloq_itit61_set_first_postnuc_value(state, p0, p1);
}
int32_t reloq_itit61_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_itit61_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_itit61_set_global_constants(state, p0);
}
int32_t reloq_itit61_set_ital_global_constants(void *, int32_t);
static inline int32_t reloq_call_itit61_set_ital_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_ital_global_constants,args,1,1);
    }
    return reloq_itit61_set_ital_global_constants(state, p0);
}
int32_t reloq_itit61_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_itit61_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_itit61_set_klattparms(state, p0);
}
int32_t reloq_itit61_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_itit61_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_itit61_set_lang_global_constants(state, p0);
}
int32_t reloq_itit61_set_letter_name_wordcats(void *, int32_t);
static inline int32_t reloq_call_itit61_set_letter_name_wordcats(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_letter_name_wordcats,args,1,1);
    }
    return reloq_itit61_set_letter_name_wordcats(state, p0);
}
int32_t reloq_itit61_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_itit61_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_itit61_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_itit61_set_num_formants(state, p0);
}
int32_t reloq_itit61_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_itit61_set_phone_offset_vals(state, p0);
}
int32_t reloq_itit61_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_itit61_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_itit61_set_said_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_set_said_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_said_phr_vals,args,3,3);
    }
    return reloq_itit61_set_said_phr_vals(state, p0, p1, p2);
}
int32_t reloq_itit61_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_itit61_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_itit61_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_itit61_set_syll_offset_vals(state, p0);
}
int32_t reloq_itit61_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_itit61_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_itit61_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_itit61_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_itit61_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_itit61_set_voice_param_constants(state, p0);
}
int32_t reloq_itit61_set_window_size(void *, int32_t);
static inline int32_t reloq_call_itit61_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_itit61_set_window_size(state, p0);
}
int32_t reloq_itit61_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_itit61_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_itit61_setdur(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_setdur(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setdur,args,3,3);
    }
    return reloq_itit61_setdur(state, p0, p1, p2);
}
int32_t reloq_itit61_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_itit61_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_itit61_shift_tone(void *, int32_t, int32_t);
int32_t reloq_itit61_shortdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_shortdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_shortdash,args,3,3);
    }
    return reloq_itit61_shortdash(state, p0, p1, p2);
}
int32_t reloq_itit61_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_itit61_single_chars(state, p0, p1, p2);
}
int32_t reloq_itit61_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_itit61_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_itit61_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_itit61_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_itit61_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_itit61_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_itit61_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_itit61_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_itit61_single_letter(state, p0, p1, p2);
}
int32_t reloq_itit61_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_itit61_size_filter(state, p0, p1, p2);
}
int32_t reloq_itit61_skip_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_skip_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_delimiters,args,2,2);
    }
    return reloq_itit61_skip_delimiters(state, p0, p1);
}
int32_t reloq_itit61_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_itit61_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_itit61_slash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_slash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash,args,3,3);
    }
    return reloq_itit61_slash(state, p0, p1, p2);
}
int32_t reloq_itit61_slash_before_measure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_slash_before_measure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash_before_measure,args,3,3);
    }
    return reloq_itit61_slash_before_measure(state, p0, p1, p2);
}
int32_t reloq_itit61_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_itit61_space(state, p0, p1, p2);
}
int32_t reloq_itit61_span_inverted_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_span_inverted_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_span_inverted_punct,args,3,3);
    }
    return reloq_itit61_span_inverted_punct(state, p0, p1, p2);
}
int32_t reloq_itit61_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_itit61_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_itit61_spell_out_large_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_spell_out_large_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_large_num,args,3,3);
    }
    return reloq_itit61_spell_out_large_num(state, p0, p1, p2);
}
int32_t reloq_itit61_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_itit61_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_itit61_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_itit61_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_itit61_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_stanital_ph_E(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_E,args,1,1);
    }
    return reloq_itit61_stanital_ph_E(state, p0);
}
int32_t reloq_itit61_stanital_ph_a(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_a,args,1,1);
    }
    return reloq_itit61_stanital_ph_a(state, p0);
}
int32_t reloq_itit61_stanital_ph_c(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_c(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_c,args,1,1);
    }
    return reloq_itit61_stanital_ph_c(state, p0);
}
int32_t reloq_itit61_stanital_ph_e(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_e,args,1,1);
    }
    return reloq_itit61_stanital_ph_e(state, p0);
}
int32_t reloq_itit61_stanital_ph_i(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_i,args,1,1);
    }
    return reloq_itit61_stanital_ph_i(state, p0);
}
int32_t reloq_itit61_stanital_ph_o(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_o,args,1,1);
    }
    return reloq_itit61_stanital_ph_o(state, p0);
}
int32_t reloq_itit61_stanital_ph_u(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_u,args,1,1);
    }
    return reloq_itit61_stanital_ph_u(state, p0);
}
int32_t reloq_itit61_stanital_ph_w(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_w,args,1,1);
    }
    return reloq_itit61_stanital_ph_w(state, p0);
}
int32_t reloq_itit61_stanital_ph_y(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_ph_y,args,1,1);
    }
    return reloq_itit61_stanital_ph_y(state, p0);
}
int32_t reloq_itit61_stanital_vals(void *, int32_t);
static inline int32_t reloq_call_itit61_stanital_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_stanital_vals,args,1,1);
    }
    return reloq_itit61_stanital_vals(state, p0);
}
int32_t reloq_itit61_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_itit61_stretch_focus(state, p0, p1);
}
int32_t reloq_itit61_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_itit61_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_itit61_strip_clitics(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_strip_clitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_clitics,args,5,5);
    }
    return reloq_itit61_strip_clitics(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_strip_enclitics(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_strip_enclitics(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_enclitics,args,4,4);
    }
    return reloq_itit61_strip_enclitics(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_strip_ital_prefixes(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_strip_ital_prefixes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_ital_prefixes,args,5,5);
    }
    return reloq_itit61_strip_ital_prefixes(state, p0, p1, p2, p3, p4);
}
int32_t reloq_itit61_strip_ital_proclitics(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_strip_ital_proclitics(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_ital_proclitics,args,3,3);
    }
    return reloq_itit61_strip_ital_proclitics(state, p0, p1, p2);
}
int32_t reloq_itit61_strip_last_digit(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_subord_conj(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_subord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subord_conj,args,8,8);
    }
    return reloq_itit61_subord_conj(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_itit61_syll_phone_adjust(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_syll_phone_adjust(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_syll_phone_adjust,args,3,3);
    }
    return reloq_itit61_syll_phone_adjust(state, p0, p1, p2);
}
int32_t reloq_itit61_syllable_sequence_adjust(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_syllable_sequence_adjust(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_syllable_sequence_adjust,args,4,4);
    }
    return reloq_itit61_syllable_sequence_adjust(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_symbolic_rules(void *, int32_t);
static inline int32_t reloq_call_itit61_symbolic_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_symbolic_rules,args,1,1);
    }
    return reloq_itit61_symbolic_rules(state, p0);
}
int32_t reloq_itit61_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_itit61_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_itit61_test_adj_vs_inf(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_test_for_ital_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_test_for_ital_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_test_for_ital_ordinal,args,3,3);
    }
    return reloq_itit61_test_for_ital_ordinal(state, p0, p1, p2);
}
int32_t reloq_itit61_test_perdono(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_itit61_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_itit61_text_index(state, p0, p1);
}
int32_t reloq_itit61_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_itit61_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_three_letter_extension(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_three_letter_extension(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_letter_extension,args,2,2);
    }
    return reloq_itit61_three_letter_extension(state, p0, p1);
}
int32_t reloq_itit61_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict,args,3,3);
    }
    return reloq_itit61_tok_dict(state, p0, p1, p2);
}
int32_t reloq_itit61_tok_dict_entry(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_tok_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict_entry,args,3,3);
    }
    return reloq_itit61_tok_dict_entry(state, p0, p1, p2);
}
int32_t reloq_itit61_tok_lookup(void *, int32_t, int32_t);
static inline int32_t reloq_call_itit61_tok_lookup(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_lookup,args,2,2);
    }
    return reloq_itit61_tok_lookup(state, p0, p1);
}
int32_t reloq_itit61_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_itit61_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_itit61_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_itit61_token_to_number(state, p0, p1, p2);
}
int32_t reloq_itit61_user_abbr_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_user_abbr_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_abbr_dict,args,3,3);
    }
    return reloq_itit61_user_abbr_dict(state, p0, p1, p2);
}
int32_t reloq_itit61_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_itit61_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_user_rootdict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_user_rootdict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_rootdict,args,3,3);
    }
    return reloq_itit61_user_rootdict(state, p0, p1, p2);
}
int32_t reloq_itit61_valid_f0_posn(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_valid_f0_posn(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_f0_posn,args,3,3);
    }
    return reloq_itit61_valid_f0_posn(state, p0, p1, p2);
}
int32_t reloq_itit61_verb_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_verb_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_verb_phrase,args,4,4);
    }
    return reloq_itit61_verb_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_itit61_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_itit61_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_itit61_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_which_get_non_vowels(void *, int32_t);
static inline int32_t reloq_call_itit61_which_get_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_which_get_non_vowels,args,1,1);
    }
    return reloq_itit61_which_get_non_vowels(state, p0);
}
int32_t reloq_itit61_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_itit61_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_itit61_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_itit61_word_syll_adjust(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_word_syll_adjust(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_syll_adjust,args,3,3);
    }
    return reloq_itit61_word_syll_adjust(state, p0, p1, p2);
}
int32_t reloq_itit61_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_itit61_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_itit61_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_itit61_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_itit61_year_mode_anno(state, p0, p1, p2, p3);
}
#endif
