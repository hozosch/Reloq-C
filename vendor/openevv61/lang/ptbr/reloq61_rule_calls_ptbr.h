/* Direct C language calls; record values remain compact until their own port. */
#ifndef RELOQ61_RULE_CALLS_PTBR_H
#define RELOQ61_RULE_CALLS_PTBR_H
#include "delta_rules_c.h"
#ifndef RELOQ_RULE_RETURN
#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \
    evv_land_release(reloq_landing_mark); return out_; } while (0)
#endif
int32_t reloq_ptbr_DeltaProc_end(void *, int32_t);
int32_t reloq_ptbr_DeltaProc_flush(void *, int32_t);
int32_t reloq_ptbr_DeltaProc_getInputCharCount(void *, int32_t);
int32_t reloq_ptbr_DeltaProc_main(void *, int32_t);
int32_t reloq_ptbr_DeltaProc_process_remaining(void *, int32_t);
int32_t reloq_ptbr_DeltaProc_process_sentences(void *, int32_t);
int32_t reloq_ptbr_DeltaProc_registerLanguages(void *, int32_t);
int32_t reloq_ptbr_DeltaProc_start(void *, int32_t);
int32_t reloq_ptbr_PhonemeIndex(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_PhonemeIndex(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_PhonemeIndex,args,3,3);
    }
    return reloq_ptbr_PhonemeIndex(state, p0, p1, p2);
}
int32_t reloq_ptbr_SPR_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_SPR_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_SPR_anno,args,4,4);
    }
    return reloq_ptbr_SPR_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_URL(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_URL(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_URL,args,2,2);
    }
    return reloq_ptbr_URL(state, p0, p1);
}
int32_t reloq_ptbr_a_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_a_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_a_rules,args,3,3);
    }
    return reloq_ptbr_a_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_a_tilda_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_a_tilda_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_a_tilda_rules,args,3,3);
    }
    return reloq_ptbr_a_tilda_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_abbr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_abbr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbr,args,4,4);
    }
    return reloq_ptbr_abbr(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_abbr_before_num(void *, int32_t, int32_t);
int32_t reloq_ptbr_abbreviation(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_abbreviation(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_abbreviation,args,4,4);
    }
    return reloq_ptbr_abbreviation(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_absval(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_absval(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_absval,args,3,3);
    }
    return reloq_ptbr_absval(state, p0, p1, p2);
}
int32_t reloq_ptbr_accented_a_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_accented_a_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_accented_a_rules,args,3,3);
    }
    return reloq_ptbr_accented_a_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_accented_u_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_accented_u_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_accented_u_rules,args,3,3);
    }
    return reloq_ptbr_accented_u_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_accented_vowel(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_accented_vowel(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_accented_vowel,args,4,4);
    }
    return reloq_ptbr_accented_vowel(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_acronym(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_acronym(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acronym,args,3,3);
    }
    return reloq_ptbr_acronym(state, p0, p1, p2);
}
int32_t reloq_ptbr_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_acrotest,args,3,3);
    }
    return reloq_ptbr_acrotest(state, p0, p1, p2);
}
int32_t reloq_ptbr_addStreamArrayPtVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_addStreamArrayPtVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArrayPtVal,args,6,6);
    }
    return reloq_ptbr_addStreamArrayPtVal(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_addStreamArraySsVal(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_addStreamArraySsVal(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_addStreamArraySsVal,args,5,5);
    }
    return reloq_ptbr_addStreamArraySsVal(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_add_TL_to_trill(void *, int32_t);
static inline int32_t reloq_call_ptbr_add_TL_to_trill(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_TL_to_trill,args,1,1);
    }
    return reloq_ptbr_add_TL_to_trill(state, p0);
}
int32_t reloq_ptbr_add_f0_for_new_voice(void *, int32_t);
static inline int32_t reloq_call_ptbr_add_f0_for_new_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_new_voice,args,1,1);
    }
    return reloq_ptbr_add_f0_for_new_voice(state, p0);
}
int32_t reloq_ptbr_add_f0_for_next_voice(void *, int32_t);
static inline int32_t reloq_call_ptbr_add_f0_for_next_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_add_f0_for_next_voice,args,1,1);
    }
    return reloq_ptbr_add_f0_for_next_voice(state, p0);
}
int32_t reloq_ptbr_add_filtered_val(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_add_filtered_val(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_filtered_val,args,7,7);
    }
    return reloq_ptbr_add_filtered_val(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_ptbr_add_gender_to_hundreds(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_add_gender_to_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_gender_to_hundreds,args,4,4);
    }
    return reloq_ptbr_add_gender_to_hundreds(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_add_por_currency_units(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_add_por_currency_units(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_por_currency_units,args,3,3);
    }
    return reloq_ptbr_add_por_currency_units(state, p0, p1, p2);
}
int32_t reloq_ptbr_add_por_plural(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_add_por_plural(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_por_plural,args,3,3);
    }
    return reloq_ptbr_add_por_plural(state, p0, p1, p2);
}
int32_t reloq_ptbr_add_por_point_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_add_por_point_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_por_point_digits,args,3,3);
    }
    return reloq_ptbr_add_por_point_digits(state, p0, p1, p2);
}
int32_t reloq_ptbr_add_por_subunits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_add_por_subunits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_add_por_subunits,args,3,3);
    }
    return reloq_ptbr_add_por_subunits(state, p0, p1, p2);
}
int32_t reloq_ptbr_adjust_AH_AF_for_TLv_w(void *, int32_t);
static inline int32_t reloq_call_ptbr_adjust_AH_AF_for_TLv_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AH_AF_for_TLv_w,args,1,1);
    }
    return reloq_ptbr_adjust_AH_AF_for_TLv_w(state, p0);
}
int32_t reloq_ptbr_adjust_AV_for_fem_voice(void *, int32_t);
static inline int32_t reloq_call_ptbr_adjust_AV_for_fem_voice(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_AV_for_fem_voice,args,1,1);
    }
    return reloq_ptbr_adjust_AV_for_fem_voice(state, p0);
}
int32_t reloq_ptbr_adjust_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_accents,args,5,5);
    }
    return reloq_ptbr_adjust_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_adjust_breathiness_av(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_breathiness_av(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_breathiness_av,args,2,2);
    }
    return reloq_ptbr_adjust_breathiness_av(state, p0, p1);
}
int32_t reloq_ptbr_adjust_for_glides(void *, int32_t);
static inline int32_t reloq_call_ptbr_adjust_for_glides(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_for_glides,args,1,1);
    }
    return reloq_ptbr_adjust_for_glides(state, p0);
}
int32_t reloq_ptbr_adjust_lg_specific_accents(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_lg_specific_accents(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_lg_specific_accents,args,5,5);
    }
    return reloq_ptbr_adjust_lg_specific_accents(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_adjust_num_in_ordinal(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_num_in_ordinal(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_num_in_ordinal,args,2,2);
    }
    return reloq_ptbr_adjust_num_in_ordinal(state, p0, p1);
}
int32_t reloq_ptbr_adjust_num_in_por_ordinal(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_num_in_por_ordinal(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_num_in_por_ordinal,args,2,2);
    }
    return reloq_ptbr_adjust_num_in_por_ordinal(state, p0, p1);
}
int32_t reloq_ptbr_adjust_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_numbers,args,3,3);
    }
    return reloq_ptbr_adjust_numbers(state, p0, p1, p2);
}
int32_t reloq_ptbr_adjust_phones(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_phones(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_phones,args,5,5);
    }
    return reloq_ptbr_adjust_phones(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_adjust_por_accents(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_por_accents(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_por_accents,args,3,3);
    }
    return reloq_ptbr_adjust_por_accents(state, p0, p1, p2);
}
int32_t reloq_ptbr_adjust_por_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_por_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_por_word_stress,args,3,3);
    }
    return reloq_ptbr_adjust_por_word_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_adjust_prev_phr_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_prev_phr_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_prev_phr_pause,args,3,3);
    }
    return reloq_ptbr_adjust_prev_phr_pause(state, p0, p1, p2);
}
int32_t reloq_ptbr_adjust_single_word(void *, int32_t);
static inline int32_t reloq_call_ptbr_adjust_single_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_adjust_single_word,args,1,1);
    }
    return reloq_ptbr_adjust_single_word(state, p0);
}
int32_t reloq_ptbr_adjust_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_word_stress,args,3,3);
    }
    return reloq_ptbr_adjust_word_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_adjust_words_in_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_adjust_words_in_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_adjust_words_in_phrase,args,3,3);
    }
    return reloq_ptbr_adjust_words_in_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_affrication(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_affrication(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_affrication,args,3,3);
    }
    return reloq_ptbr_affrication(state, p0, p1, p2);
}
int32_t reloq_ptbr_ambig_abbr(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_ambig_abbr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_ambig_abbr,args,2,2);
    }
    return reloq_ptbr_ambig_abbr(state, p0, p1);
}
int32_t reloq_ptbr_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apostrophe,args,3,3);
    }
    return reloq_ptbr_apostrophe(state, p0, p1, p2);
}
int32_t reloq_ptbr_apply_breathiness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_breathiness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_breathiness_val,args,2,2);
    }
    return reloq_ptbr_apply_breathiness_val(state, p0, p1);
}
int32_t reloq_ptbr_apply_comma_phr_rules(void *, int32_t);
static inline int32_t reloq_call_ptbr_apply_comma_phr_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_comma_phr_rules,args,1,1);
    }
    return reloq_ptbr_apply_comma_phr_rules(state, p0);
}
int32_t reloq_ptbr_apply_head_size_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_head_size_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_head_size_val,args,2,2);
    }
    return reloq_ptbr_apply_head_size_val(state, p0, p1);
}
int32_t reloq_ptbr_apply_lang_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_lang_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_lang_comma_rules,args,3,3);
    }
    return reloq_ptbr_apply_lang_comma_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_apply_min_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_min_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_min_durs,args,3,3);
    }
    return reloq_ptbr_apply_min_durs(state, p0, p1, p2);
}
int32_t reloq_ptbr_apply_pitch_baseline_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_pitch_baseline_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_baseline_val,args,2,2);
    }
    return reloq_ptbr_apply_pitch_baseline_val(state, p0, p1);
}
int32_t reloq_ptbr_apply_pitch_fluct_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_pitch_fluct_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_pitch_fluct_val,args,2,2);
    }
    return reloq_ptbr_apply_pitch_fluct_val(state, p0, p1);
}
int32_t reloq_ptbr_apply_por_comma_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_por_comma_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_por_comma_rules,args,3,3);
    }
    return reloq_ptbr_apply_por_comma_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_apply_roughness_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_roughness_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_roughness_val,args,2,2);
    }
    return reloq_ptbr_apply_roughness_val(state, p0, p1);
}
int32_t reloq_ptbr_apply_rules(void *, int32_t);
static inline int32_t reloq_call_ptbr_apply_rules(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_apply_rules,args,1,1);
    }
    return reloq_ptbr_apply_rules(state, p0);
}
int32_t reloq_ptbr_apply_speed_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_speed_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_anno,args,3,3);
    }
    return reloq_ptbr_apply_speed_anno(state, p0, p1, p2);
}
int32_t reloq_ptbr_apply_speed_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_speed_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_speed_val,args,2,2);
    }
    return reloq_ptbr_apply_speed_val(state, p0, p1);
}
int32_t reloq_ptbr_apply_vocal_tract_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_vocal_tract_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_vocal_tract_val,args,2,2);
    }
    return reloq_ptbr_apply_vocal_tract_val(state, p0, p1);
}
int32_t reloq_ptbr_apply_volume_val(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_apply_volume_val(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_apply_volume_val,args,2,2);
    }
    return reloq_ptbr_apply_volume_val(state, p0, p1);
}
int32_t reloq_ptbr_assign_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_acoustic_vals,args,1,1);
    }
    return reloq_ptbr_assign_acoustic_vals(state, p0);
}
int32_t reloq_ptbr_assign_alt_quest_features(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_alt_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_alt_quest_features,args,1,1);
    }
    return reloq_ptbr_assign_alt_quest_features(state, p0);
}
int32_t reloq_ptbr_assign_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_cons_durs,args,3,3);
    }
    return reloq_ptbr_assign_cons_durs(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_currPhr_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_currPhr_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_currPhr_vals,args,2,2);
    }
    return reloq_ptbr_assign_currPhr_vals(state, p0, p1);
}
int32_t reloq_ptbr_assign_default_quest_features(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_default_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_default_quest_features,args,1,1);
    }
    return reloq_ptbr_assign_default_quest_features(state, p0);
}
int32_t reloq_ptbr_assign_gend_to_por_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_gend_to_por_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_gend_to_por_roman_num,args,3,3);
    }
    return reloq_ptbr_assign_gend_to_por_roman_num(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_gend_to_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_gend_to_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_gend_to_roman_num,args,3,3);
    }
    return reloq_ptbr_assign_gend_to_roman_num(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_gender_to_currency(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_gender_to_currency(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_gender_to_currency,args,2,2);
    }
    return reloq_ptbr_assign_gender_to_currency(state, p0, p1);
}
int32_t reloq_ptbr_assign_gender_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_gender_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_gender_to_number,args,3,3);
    }
    return reloq_ptbr_assign_gender_to_number(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_gender_to_por_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_gender_to_por_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_gender_to_por_number,args,3,3);
    }
    return reloq_ptbr_assign_gender_to_por_number(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_inf_category(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_inf_category(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_inf_category,args,3,3);
    }
    return reloq_ptbr_assign_inf_category(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_major_phr_features(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_major_phr_features(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_major_phr_features,args,3,3);
    }
    return reloq_ptbr_assign_major_phr_features(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_nuc_durs,args,1,1);
    }
    return reloq_ptbr_assign_nuc_durs(state, p0);
}
int32_t reloq_ptbr_assign_phone_durs(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_phone_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phone_durs,args,1,1);
    }
    return reloq_ptbr_assign_phone_durs(state, p0);
}
int32_t reloq_ptbr_assign_phr_final_anno_features(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_phr_final_anno_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_phr_final_anno_features,args,1,1);
    }
    return reloq_ptbr_assign_phr_final_anno_features(state, p0);
}
int32_t reloq_ptbr_assign_por_nuc_durs(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_por_nuc_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_por_nuc_durs,args,1,1);
    }
    return reloq_ptbr_assign_por_nuc_durs(state, p0);
}
int32_t reloq_ptbr_assign_por_start_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_por_start_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_por_start_dur,args,1,1);
    }
    return reloq_ptbr_assign_por_start_dur(state, p0);
}
int32_t reloq_ptbr_assign_prestored_voice_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_prestored_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_prestored_voice_vals,args,4,4);
    }
    return reloq_ptbr_assign_prestored_voice_vals(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_assign_root_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_root_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_root_stress,args,4,4);
    }
    return reloq_ptbr_assign_root_stress(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_assign_secondary_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_secondary_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_secondary_stress,args,3,3);
    }
    return reloq_ptbr_assign_secondary_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_sent_final_features(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_sent_final_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_sent_final_features,args,1,1);
    }
    return reloq_ptbr_assign_sent_final_features(state, p0);
}
int32_t reloq_ptbr_assign_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_stress,args,3,3);
    }
    return reloq_ptbr_assign_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_suffix_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_suffix_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_suffix_stress,args,3,3);
    }
    return reloq_ptbr_assign_suffix_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_syll_onset(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_syll_onset(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_syll_onset,args,2,2);
    }
    return reloq_ptbr_assign_syll_onset(state, p0, p1);
}
int32_t reloq_ptbr_assign_unpuncted_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_unpuncted_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_unpuncted_phr_vals,args,3,3);
    }
    return reloq_ptbr_assign_unpuncted_phr_vals(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_user_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_assign_user_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_assign_user_stress,args,3,3);
    }
    return reloq_ptbr_assign_user_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_assign_wh_quest_features(void *, int32_t);
static inline int32_t reloq_call_ptbr_assign_wh_quest_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_assign_wh_quest_features,args,1,1);
    }
    return reloq_ptbr_assign_wh_quest_features(state, p0);
}
int32_t reloq_ptbr_b_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_b_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_b_rules,args,3,3);
    }
    return reloq_ptbr_b_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_beg_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_beg_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_beg_of_word,args,2,2);
    }
    return reloq_ptbr_beg_of_word(state, p0, p1);
}
int32_t reloq_ptbr_between_proper_names(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_boundary_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_boundary_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_boundary_tone,args,1,1);
    }
    return reloq_ptbr_boundary_tone(state, p0);
}
int32_t reloq_ptbr_bracket(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_bracket(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_bracket,args,3,3);
    }
    return reloq_ptbr_bracket(state, p0, p1, p2);
}
int32_t reloq_ptbr_break_into_morphs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_break_into_morphs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_morphs,args,5,5);
    }
    return reloq_ptbr_break_into_morphs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_break_into_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_break_into_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_break_into_phones,args,3,3);
    }
    return reloq_ptbr_break_into_phones(state, p0, p1, p2);
}
int32_t reloq_ptbr_breakup_spr_phones(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_build_phrase_final_structure(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_build_phrase_final_structure(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrase_final_structure,args,2,2);
    }
    return reloq_ptbr_build_phrase_final_structure(state, p0, p1);
}
int32_t reloq_ptbr_build_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_build_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_phrases,args,3,3);
    }
    return reloq_ptbr_build_phrases(state, p0, p1, p2);
}
int32_t reloq_ptbr_build_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_build_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_build_words,args,3,3);
    }
    return reloq_ptbr_build_words(state, p0, p1, p2);
}
int32_t reloq_ptbr_c_cedille_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_c_cedille_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_c_cedille_rules,args,3,3);
    }
    return reloq_ptbr_c_cedille_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_c_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_c_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_c_rules,args,3,3);
    }
    return reloq_ptbr_c_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_callInternalSynthesizer(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_callInternalSynthesizer(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_callInternalSynthesizer,args,7,7);
    }
    return reloq_ptbr_callInternalSynthesizer(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_ptbr_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_char_name,args,3,3);
    }
    return reloq_ptbr_char_name(state, p0, p1, p2);
}
int32_t reloq_ptbr_clear_delta(void *, int32_t);
static inline int32_t reloq_call_ptbr_clear_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_clear_delta,args,1,1);
    }
    return reloq_ptbr_clear_delta(state, p0);
}
int32_t reloq_ptbr_close_platform(void *, int32_t);
static inline int32_t reloq_call_ptbr_close_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_close_platform,args,1,1);
    }
    return reloq_ptbr_close_platform(state, p0);
}
int32_t reloq_ptbr_comma_at_end_quote(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_comma_at_end_quote(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_comma_at_end_quote,args,3,3);
    }
    return reloq_ptbr_comma_at_end_quote(state, p0, p1, p2);
}
int32_t reloq_ptbr_comma_phrase(void *, int32_t);
static inline int32_t reloq_call_ptbr_comma_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_comma_phrase,args,1,1);
    }
    return reloq_ptbr_comma_phrase(state, p0);
}
int32_t reloq_ptbr_con_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_con_vals,args,1,1);
    }
    return reloq_ptbr_con_vals(state, p0);
}
int32_t reloq_ptbr_conjoined_NPs(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_conjoined_NPs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_conjoined_NPs,args,7,7);
    }
    return reloq_ptbr_conjoined_NPs(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_ptbr_cons_syll_phone_adjust(void *, int32_t);
static inline int32_t reloq_call_ptbr_cons_syll_phone_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_cons_syll_phone_adjust,args,1,1);
    }
    return reloq_ptbr_cons_syll_phone_adjust(state, p0);
}
int32_t reloq_ptbr_context_adjust(void *, int32_t);
static inline int32_t reloq_call_ptbr_context_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_context_adjust,args,1,1);
    }
    return reloq_ptbr_context_adjust(state, p0);
}
int32_t reloq_ptbr_convert_1_or_2_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_1_or_2_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_1_or_2_digits,args,3,3);
    }
    return reloq_ptbr_convert_1_or_2_digits(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits,args,3,3);
    }
    return reloq_ptbr_convert_digits(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_digits_to_words,args,3,3);
    }
    return reloq_ptbr_convert_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_double,args,3,3);
    }
    return reloq_ptbr_convert_double(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_hundreds,args,3,3);
    }
    return reloq_ptbr_convert_hundreds(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_lang_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_lang_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_lang_digits_to_words,args,3,3);
    }
    return reloq_ptbr_convert_lang_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_large_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_large_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_large_numbers,args,3,3);
    }
    return reloq_ptbr_convert_large_numbers(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_monetary_exp,args,5,5);
    }
    return reloq_ptbr_convert_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_convert_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_numbers,args,3,3);
    }
    return reloq_ptbr_convert_numbers(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_date(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_date(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_date,args,3,3);
    }
    return reloq_ptbr_convert_por_date(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_digits_to_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_digits_to_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_digits_to_words,args,3,3);
    }
    return reloq_ptbr_convert_por_digits_to_words(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_double(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_double(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_double,args,3,3);
    }
    return reloq_ptbr_convert_por_double(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_fraction,args,3,3);
    }
    return reloq_ptbr_convert_por_fraction(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_hundreds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_hundreds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_hundreds,args,3,3);
    }
    return reloq_ptbr_convert_por_hundreds(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_monetary_exp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_monetary_exp,args,5,5);
    }
    return reloq_ptbr_convert_por_monetary_exp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_convert_por_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_roman_num,args,3,3);
    }
    return reloq_ptbr_convert_por_roman_num(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_single_digit,args,3,3);
    }
    return reloq_ptbr_convert_por_single_digit(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_teens(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_teens(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_teens,args,3,3);
    }
    return reloq_ptbr_convert_por_teens(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_por_time(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_por_time(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_por_time,args,3,3);
    }
    return reloq_ptbr_convert_por_time(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_roman_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_roman_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_roman_num,args,3,3);
    }
    return reloq_ptbr_convert_roman_num(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_roman_num_to_ordinal(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_roman_num_to_ordinal(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_roman_num_to_ordinal,args,3,3);
    }
    return reloq_ptbr_convert_roman_num_to_ordinal(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_roman_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_roman_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_roman_nums,args,3,3);
    }
    return reloq_ptbr_convert_roman_nums(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_single_digit,args,3,3);
    }
    return reloq_ptbr_convert_single_digit(state, p0, p1, p2);
}
int32_t reloq_ptbr_convert_to_lowercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_convert_to_lowercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_convert_to_lowercase,args,3,3);
    }
    return reloq_ptbr_convert_to_lowercase(state, p0, p1, p2);
}
int32_t reloq_ptbr_coord_conj(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_coord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_coord_conj,args,8,8);
    }
    return reloq_ptbr_coord_conj(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_ptbr_copy_stress_to_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_copy_stress_to_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_stress_to_phones,args,3,3);
    }
    return reloq_ptbr_copy_stress_to_phones(state, p0, p1, p2);
}
int32_t reloq_ptbr_copy_string(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_copy_string(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_copy_string,args,5,5);
    }
    return reloq_ptbr_copy_string(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_copy_token(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_count_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_count_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_chars,args,4,4);
    }
    return reloq_ptbr_count_chars(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_count_cumulative_chars(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_count_cumulative_chars(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_cumulative_chars,args,4,4);
    }
    return reloq_ptbr_count_cumulative_chars(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_count_digits(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_count_digits(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_digits,args,3,3);
    }
    return reloq_ptbr_count_digits(state, p0, p1, p2);
}
int32_t reloq_ptbr_count_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_count_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_input,args,2,2);
    }
    return reloq_ptbr_count_input(state, p0, p1);
}
int32_t reloq_ptbr_count_normalized_inp(void *, int32_t);
static inline int32_t reloq_call_ptbr_count_normalized_inp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_count_normalized_inp,args,1,1);
    }
    return reloq_ptbr_count_normalized_inp(state, p0);
}
int32_t reloq_ptbr_count_sylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_count_sylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_sylls,args,4,4);
    }
    return reloq_ptbr_count_sylls(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_count_unnormalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_count_unnormalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_count_unnormalized_inp,args,3,3);
    }
    return reloq_ptbr_count_unnormalized_inp(state, p0, p1, p2);
}
int32_t reloq_ptbr_create_final_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_create_final_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_final_sync,args,3,3);
    }
    return reloq_ptbr_create_final_sync(state, p0, p1, p2);
}
int32_t reloq_ptbr_create_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_create_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_sentence,args,2,2);
    }
    return reloq_ptbr_create_sentence(state, p0, p1);
}
int32_t reloq_ptbr_create_syllables(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_create_syllables(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_create_syllables,args,3,3);
    }
    return reloq_ptbr_create_syllables(state, p0, p1, p2);
}
int32_t reloq_ptbr_d_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_d_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_d_rules,args,3,3);
    }
    return reloq_ptbr_d_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_define_all_params_for_silence(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_define_all_params_for_silence(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_define_all_params_for_silence,args,5,5);
    }
    return reloq_ptbr_define_all_params_for_silence(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_delete_2_more(void *, int32_t);
static inline int32_t reloq_call_ptbr_delete_2_more(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_delete_2_more,args,1,1);
    }
    return reloq_ptbr_delete_2_more(state, p0);
}
int32_t reloq_ptbr_delete_comma_sync(void *, int32_t, int32_t);
int32_t reloq_ptbr_delete_inp_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_delete_inp_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_left,args,3,3);
    }
    return reloq_ptbr_delete_inp_from_left(state, p0, p1, p2);
}
int32_t reloq_ptbr_delete_inp_from_right(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_delete_inp_from_right(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_inp_from_right,args,3,3);
    }
    return reloq_ptbr_delete_inp_from_right(state, p0, p1, p2);
}
int32_t reloq_ptbr_delete_invalid_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_delete_invalid_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_invalid_SPR,args,3,3);
    }
    return reloq_ptbr_delete_invalid_SPR(state, p0, p1, p2);
}
int32_t reloq_ptbr_delete_morph_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_delete_morph_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_delete_phone_from_left(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_delete_phone_from_left(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_phone_from_left,args,3,3);
    }
    return reloq_ptbr_delete_phone_from_left(state, p0, p1, p2);
}
int32_t reloq_ptbr_delete_phone_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_delete_syll_from_left(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_delete_syll_from_right(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_delete_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_delete_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delete_voice_change_pause,args,3,3);
    }
    return reloq_ptbr_delete_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_ptbr_delimit_major_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_delimit_major_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_major_phrase,args,2,2);
    }
    return reloq_ptbr_delimit_major_phrase(state, p0, p1);
}
int32_t reloq_ptbr_delimit_sentence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_delimit_sentence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_delimit_sentence,args,3,3);
    }
    return reloq_ptbr_delimit_sentence(state, p0, p1, p2);
}
int32_t reloq_ptbr_demote_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_demote_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_demote_word,args,3,3);
    }
    return reloq_ptbr_demote_word(state, p0, p1, p2);
}
int32_t reloq_ptbr_destress_adverbs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_destress_adverbs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_adverbs,args,3,3);
    }
    return reloq_ptbr_destress_adverbs(state, p0, p1, p2);
}
int32_t reloq_ptbr_destress_compounds(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_destress_compounds(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_compounds,args,3,3);
    }
    return reloq_ptbr_destress_compounds(state, p0, p1, p2);
}
int32_t reloq_ptbr_destress_nps(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_destress_nps(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_nps,args,3,3);
    }
    return reloq_ptbr_destress_nps(state, p0, p1, p2);
}
int32_t reloq_ptbr_destress_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_destress_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_numbers,args,3,3);
    }
    return reloq_ptbr_destress_numbers(state, p0, p1, p2);
}
int32_t reloq_ptbr_destress_postnuclear_expr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_destress_postnuclear_expr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_postnuclear_expr,args,3,3);
    }
    return reloq_ptbr_destress_postnuclear_expr(state, p0, p1, p2);
}
int32_t reloq_ptbr_destress_proper_names(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_destress_proper_names(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_proper_names,args,3,3);
    }
    return reloq_ptbr_destress_proper_names(state, p0, p1, p2);
}
int32_t reloq_ptbr_destress_verbs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_destress_verbs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_destress_verbs,args,3,3);
    }
    return reloq_ptbr_destress_verbs(state, p0, p1, p2);
}
int32_t reloq_ptbr_det_vs_nounadj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_det_vs_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_det_vs_nounadj,args,3,3);
    }
    return reloq_ptbr_det_vs_nounadj(state, p0, p1, p2);
}
int32_t reloq_ptbr_dict_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_dict_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_anno,args,4,4);
    }
    return reloq_ptbr_dict_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_dict_assigned_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_dict_assigned_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_dict_assigned_stress,args,2,2);
    }
    return reloq_ptbr_dict_assigned_stress(state, p0, p1);
}
int32_t reloq_ptbr_disambig_por_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambig_por_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_por_content_word,args,3,3);
    }
    return reloq_ptbr_disambig_por_content_word(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambig_por_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambig_por_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_por_functword,args,3,3);
    }
    return reloq_ptbr_disambig_por_functword(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambig_por_single_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambig_por_single_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambig_por_single_word,args,3,3);
    }
    return reloq_ptbr_disambig_por_single_word(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_a(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_a(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_a,args,3,3);
    }
    return reloq_ptbr_disambiguate_a(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_acerca(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_acerca(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_acerca,args,3,3);
    }
    return reloq_ptbr_disambiguate_acerca(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_cedo(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_cedo(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_cedo,args,3,3);
    }
    return reloq_ptbr_disambiguate_cedo(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_colher(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_colher(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_colher,args,3,3);
    }
    return reloq_ptbr_disambiguate_colher(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_como(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_como(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_como,args,3,3);
    }
    return reloq_ptbr_disambiguate_como(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_content_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_content_word,args,4,4);
    }
    return reloq_ptbr_disambiguate_content_word(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_disambiguate_desse_or_deste(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_desse_or_deste(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_desse_or_deste,args,3,3);
    }
    return reloq_ptbr_disambiguate_desse_or_deste(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_esse(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_esse(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_esse,args,3,3);
    }
    return reloq_ptbr_disambiguate_esse(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_este(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_este(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_este,args,3,3);
    }
    return reloq_ptbr_disambiguate_este(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_functword(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_functword(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_functword,args,3,3);
    }
    return reloq_ptbr_disambiguate_functword(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_mais(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_mais(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_mais,args,3,3);
    }
    return reloq_ptbr_disambiguate_mais(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_mesmo(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_mesmo(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_mesmo,args,3,3);
    }
    return reloq_ptbr_disambiguate_mesmo(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_muito(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_muito(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_muito,args,3,3);
    }
    return reloq_ptbr_disambiguate_muito(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_nos(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_nos(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_nos,args,3,3);
    }
    return reloq_ptbr_disambiguate_nos(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_noun_from_adj(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_noun_from_adj(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_noun_from_adj,args,2,2);
    }
    return reloq_ptbr_disambiguate_noun_from_adj(state, p0, p1);
}
int32_t reloq_ptbr_disambiguate_pelo(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_pelo(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_pelo,args,3,3);
    }
    return reloq_ptbr_disambiguate_pelo(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_por(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_por(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_por,args,3,3);
    }
    return reloq_ptbr_disambiguate_por(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_sao(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_sao(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_sao,args,3,3);
    }
    return reloq_ptbr_disambiguate_sao(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_se(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_se(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_se,args,3,3);
    }
    return reloq_ptbr_disambiguate_se(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_segundo(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_segundo(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_segundo,args,3,3);
    }
    return reloq_ptbr_disambiguate_segundo(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_single_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_single_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_single_word,args,3,3);
    }
    return reloq_ptbr_disambiguate_single_word(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_sobre(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_sobre(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_sobre,args,3,3);
    }
    return reloq_ptbr_disambiguate_sobre(state, p0, p1, p2);
}
int32_t reloq_ptbr_disambiguate_travesso(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_travesso(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_travesso,args,4,4);
    }
    return reloq_ptbr_disambiguate_travesso(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_disambiguate_um(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_disambiguate_um(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_disambiguate_um,args,3,3);
    }
    return reloq_ptbr_disambiguate_um(state, p0, p1, p2);
}
int32_t reloq_ptbr_distribute_nucdur(void *, int32_t);
static inline int32_t reloq_call_ptbr_distribute_nucdur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_distribute_nucdur,args,1,1);
    }
    return reloq_ptbr_distribute_nucdur(state, p0);
}
int32_t reloq_ptbr_divide_char_count_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_divide_char_count_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_divide_char_count_token,args,2,2);
    }
    return reloq_ptbr_divide_char_count_token(state, p0, p1);
}
int32_t reloq_ptbr_doAF(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_doAF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAF,args,6,6);
    }
    return reloq_ptbr_doAF(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_doAV(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_doAV(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doAV,args,6,6);
    }
    return reloq_ptbr_doAV(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_doF(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_doF(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doF,args,4,4);
    }
    return reloq_ptbr_doF(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_doFNP(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_doFNP(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNP,args,6,6);
    }
    return reloq_ptbr_doFNP(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_doFNZ(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_doFNZ(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_doFNZ,args,6,6);
    }
    return reloq_ptbr_doFNZ(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_do_FTZ_FTP(void *, int32_t);
static inline int32_t reloq_call_ptbr_do_FTZ_FTP(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_FTZ_FTP,args,1,1);
    }
    return reloq_ptbr_do_FTZ_FTP(state, p0);
}
int32_t reloq_ptbr_do_fric_amps(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_do_fric_amps(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9, int32_t p10, int32_t p11)
{
    if(delta_rule_trace) {
        const int32_t args[]={p11, p10, p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_do_fric_amps,args,12,12);
    }
    return reloq_ptbr_do_fric_amps(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}
int32_t reloq_ptbr_do_non_son_amps(void *, int32_t);
static inline int32_t reloq_call_ptbr_do_non_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_non_son_amps,args,1,1);
    }
    return reloq_ptbr_do_non_son_amps(state, p0);
}
int32_t reloq_ptbr_do_por_voice_bar(void *, int32_t);
int32_t reloq_ptbr_do_pt_voicing(void *, int32_t);
static inline int32_t reloq_call_ptbr_do_pt_voicing(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_pt_voicing,args,1,1);
    }
    return reloq_ptbr_do_pt_voicing(state, p0);
}
int32_t reloq_ptbr_do_release(void *, int32_t);
static inline int32_t reloq_call_ptbr_do_release(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_release,args,1,1);
    }
    return reloq_ptbr_do_release(state, p0);
}
int32_t reloq_ptbr_do_son_amps(void *, int32_t);
static inline int32_t reloq_call_ptbr_do_son_amps(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_do_son_amps,args,1,1);
    }
    return reloq_ptbr_do_son_amps(state, p0);
}
int32_t reloq_ptbr_do_voice_bar(void *, int32_t);
int32_t reloq_ptbr_e_acute_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_e_acute_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_e_acute_rules,args,3,3);
    }
    return reloq_ptbr_e_acute_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_e_circon_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_e_circon_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_e_circon_rules,args,3,3);
    }
    return reloq_ptbr_e_circon_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_e_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_e_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_e_rules,args,3,3);
    }
    return reloq_ptbr_e_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_eightkHz_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_eightkHz_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_eightkHz_filter,args,3,3);
    }
    return reloq_ptbr_eightkHz_filter(state, p0, p1, p2);
}
int32_t reloq_ptbr_ellipsis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_ellipsis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_ellipsis,args,3,3);
    }
    return reloq_ptbr_ellipsis(state, p0, p1, p2);
}
int32_t reloq_ptbr_email_address(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_email_address(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_email_address,args,2,2);
    }
    return reloq_ptbr_email_address(state, p0, p1);
}
int32_t reloq_ptbr_emphasis_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_emphasis_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_emphasis_anno,args,3,3);
    }
    return reloq_ptbr_emphasis_anno(state, p0, p1, p2);
}
int32_t reloq_ptbr_end_of_sentence(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_end_of_sentence(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_sentence,args,2,2);
    }
    return reloq_ptbr_end_of_sentence(state, p0, p1);
}
int32_t reloq_ptbr_end_of_token(void *, int32_t, int32_t);
int32_t reloq_ptbr_end_of_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_end_of_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_end_of_word,args,2,2);
    }
    return reloq_ptbr_end_of_word(state, p0, p1);
}
int32_t reloq_ptbr_environment_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_environment_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_environment_anno,args,4,4);
    }
    return reloq_ptbr_environment_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_estimate_lang_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_estimate_lang_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_lang_nsylls,args,4,4);
    }
    return reloq_ptbr_estimate_lang_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_estimate_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_estimate_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_nsylls,args,4,4);
    }
    return reloq_ptbr_estimate_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_estimate_por_nsylls(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_estimate_por_nsylls(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_estimate_por_nsylls,args,4,4);
    }
    return reloq_ptbr_estimate_por_nsylls(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_exclam_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_exclam_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_exclam_phrase,args,2,2);
    }
    return reloq_ptbr_exclam_phrase(state, p0, p1);
}
int32_t reloq_ptbr_extend_vals(void *, int32_t);
int32_t reloq_ptbr_f0_step(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_f0_step(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f0_step,args,6,6);
    }
    return reloq_ptbr_f0_step(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_f_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_f_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_f_rules,args,3,3);
    }
    return reloq_ptbr_f_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_filter_by_range(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_filter_by_range(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_filter_by_range,args,3,3);
    }
    return reloq_ptbr_filter_by_range(state, p0, p1, p2);
}
int32_t reloq_ptbr_final_voiced_release(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_final_voiced_release(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_final_voiced_release,args,3,3);
    }
    return reloq_ptbr_final_voiced_release(state, p0, p1, p2);
}
int32_t reloq_ptbr_find_acoustic_nuc(void *, int32_t);
static inline int32_t reloq_call_ptbr_find_acoustic_nuc(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_acoustic_nuc,args,1,1);
    }
    return reloq_ptbr_find_acoustic_nuc(state, p0);
}
int32_t reloq_ptbr_find_lang_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_lang_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_obligatory_phrase,args,4,4);
    }
    return reloq_ptbr_find_lang_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_find_lang_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_lang_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_lang_potential_phrase,args,4,4);
    }
    return reloq_ptbr_find_lang_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_find_last_syll(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_last_syll(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_last_syll,args,4,4);
    }
    return reloq_ptbr_find_last_syll(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_find_most_stressed_syll(void *, int32_t);
static inline int32_t reloq_call_ptbr_find_most_stressed_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_most_stressed_syll,args,1,1);
    }
    return reloq_ptbr_find_most_stressed_syll(state, p0);
}
int32_t reloq_ptbr_find_next_token(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_find_next_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_next_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_next_word,args,4,4);
    }
    return reloq_ptbr_find_next_word(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_find_nucleus(void *, int32_t);
static inline int32_t reloq_call_ptbr_find_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_nucleus,args,1,1);
    }
    return reloq_ptbr_find_nucleus(state, p0);
}
int32_t reloq_ptbr_find_pair_of_single_quotes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_pair_of_single_quotes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_pair_of_single_quotes,args,4,4);
    }
    return reloq_ptbr_find_pair_of_single_quotes(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_find_por_nucleus(void *, int32_t);
static inline int32_t reloq_call_ptbr_find_por_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_find_por_nucleus,args,1,1);
    }
    return reloq_ptbr_find_por_nucleus(state, p0);
}
int32_t reloq_ptbr_find_por_obligatory_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_por_obligatory_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_por_obligatory_phrase,args,4,4);
    }
    return reloq_ptbr_find_por_obligatory_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_find_por_potential_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_por_potential_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_por_potential_phrase,args,4,4);
    }
    return reloq_ptbr_find_por_potential_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_find_prev_word_optional_comma(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_find_previous_word(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_previous_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_previous_word,args,4,4);
    }
    return reloq_ptbr_find_previous_word(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_find_unpuncted_phrases(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_find_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_find_unpuncted_phrases,args,3,3);
    }
    return reloq_ptbr_find_unpuncted_phrases(state, p0, p1, p2);
}
int32_t reloq_ptbr_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_ptbr_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_first_postnucl_accent,args,1,1);
    }
    return reloq_ptbr_first_postnucl_accent(state, p0);
}
int32_t reloq_ptbr_first_root_vow(void *, int32_t, int32_t);
int32_t reloq_ptbr_followed_by_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_followed_by_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_followed_by_hyphen,args,3,3);
    }
    return reloq_ptbr_followed_by_hyphen(state, p0, p1, p2);
}
int32_t reloq_ptbr_following_minor_phrase_edge(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_following_minor_phrase_edge(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_following_minor_phrase_edge,args,2,2);
    }
    return reloq_ptbr_following_minor_phrase_edge(state, p0, p1);
}
int32_t reloq_ptbr_follows_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_follows_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_follows_end_quote,args,2,2);
    }
    return reloq_ptbr_follows_end_quote(state, p0, p1);
}
int32_t reloq_ptbr_forced_synthesis(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_forced_synthesis(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_forced_synthesis,args,2,2);
    }
    return reloq_ptbr_forced_synthesis(state, p0, p1);
}
int32_t reloq_ptbr_g_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_g_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_g_rules,args,3,3);
    }
    return reloq_ptbr_g_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_generate_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_generate_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_acoustic_vals,args,1,1);
    }
    return reloq_ptbr_generate_acoustic_vals(state, p0);
}
int32_t reloq_ptbr_generate_diaphones(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_generate_diaphones(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_generate_diaphones,args,4,4);
    }
    return reloq_ptbr_generate_diaphones(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_generate_durations(void *, int32_t);
static inline int32_t reloq_call_ptbr_generate_durations(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_durations,args,1,1);
    }
    return reloq_ptbr_generate_durations(state, p0);
}
int32_t reloq_ptbr_generate_intonation(void *, int32_t);
static inline int32_t reloq_call_ptbr_generate_intonation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_generate_intonation,args,1,1);
    }
    return reloq_ptbr_generate_intonation(state, p0);
}
int32_t reloq_ptbr_get_formant_voice_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_formant_voice_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_formant_voice_vals,args,10,10);
    }
    return reloq_ptbr_get_formant_voice_vals(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_ptbr_get_gender_number(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_gender_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_gender_number,args,5,5);
    }
    return reloq_ptbr_get_gender_number(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_get_indices(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_indices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_indices,args,6,6);
    }
    return reloq_ptbr_get_indices(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_get_input(void *, int32_t);
static inline int32_t reloq_call_ptbr_get_input(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_input,args,1,1);
    }
    return reloq_ptbr_get_input(state, p0);
}
int32_t reloq_ptbr_get_nonpickup_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_get_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_ptbr_get_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_nuc_AV,args,1,1);
    }
    return reloq_ptbr_get_nuc_AV(state, p0);
}
int32_t reloq_ptbr_get_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_nuclear_accent,args,5,5);
    }
    return reloq_ptbr_get_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_get_numRootSylls(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_get_number_info(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_number_info(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_number_info,args,3,3);
    }
    return reloq_ptbr_get_number_info(state, p0, p1, p2);
}
int32_t reloq_ptbr_get_pause_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_pause_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_pause_trans_dur,args,2,2);
    }
    return reloq_ptbr_get_pause_trans_dur(state, p0, p1);
}
int32_t reloq_ptbr_get_por_nas_tilt_amp(void *, int32_t);
static inline int32_t reloq_call_ptbr_get_por_nas_tilt_amp(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_por_nas_tilt_amp,args,1,1);
    }
    return reloq_ptbr_get_por_nas_tilt_amp(state, p0);
}
int32_t reloq_ptbr_get_por_non_vowels(void *, int32_t);
static inline int32_t reloq_call_ptbr_get_por_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_por_non_vowels,args,1,1);
    }
    return reloq_ptbr_get_por_non_vowels(state, p0);
}
int32_t reloq_ptbr_get_por_nuc_AV(void *, int32_t);
static inline int32_t reloq_call_ptbr_get_por_nuc_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_por_nuc_AV,args,1,1);
    }
    return reloq_ptbr_get_por_nuc_AV(state, p0);
}
int32_t reloq_ptbr_get_por_obstr_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_por_obstr_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_por_obstr_trans_dur,args,2,2);
    }
    return reloq_ptbr_get_por_obstr_trans_dur(state, p0, p1);
}
int32_t reloq_ptbr_get_por_son_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_por_son_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_por_son_trans_dur,args,2,2);
    }
    return reloq_ptbr_get_por_son_trans_dur(state, p0, p1);
}
int32_t reloq_ptbr_get_por_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_por_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_por_trans_dur,args,2,2);
    }
    return reloq_ptbr_get_por_trans_dur(state, p0, p1);
}
int32_t reloq_ptbr_get_por_trill_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_por_trill_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_por_trill_trans_dur,args,2,2);
    }
    return reloq_ptbr_get_por_trill_trans_dur(state, p0, p1);
}
int32_t reloq_ptbr_get_por_vow_trans_dur(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_por_vow_trans_dur(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_por_vow_trans_dur,args,3,3);
    }
    return reloq_ptbr_get_por_vow_trans_dur(state, p0, p1, p2);
}
int32_t reloq_ptbr_get_portl_high_pal_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_portl_high_pal_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_portl_high_pal_trans_dur,args,2,2);
    }
    return reloq_ptbr_get_portl_high_pal_trans_dur(state, p0, p1);
}
int32_t reloq_ptbr_get_tok(void *, int32_t);
static inline int32_t reloq_call_ptbr_get_tok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_get_tok,args,1,1);
    }
    return reloq_ptbr_get_tok(state, p0);
}
int32_t reloq_ptbr_get_trans_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_get_trans_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_get_trans_dur,args,2,2);
    }
    return reloq_ptbr_get_trans_dur(state, p0, p1);
}
int32_t reloq_ptbr_gettok(void *, int32_t);
static inline int32_t reloq_call_ptbr_gettok(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_gettok,args,1,1);
    }
    return reloq_ptbr_gettok(state, p0);
}
int32_t reloq_ptbr_gliding(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_gliding(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_gliding,args,3,3);
    }
    return reloq_ptbr_gliding(state, p0, p1, p2);
}
int32_t reloq_ptbr_gramm_cat_dict_lookup(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_gramm_cat_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_gramm_cat_dict_lookup,args,5,5);
    }
    return reloq_ptbr_gramm_cat_dict_lookup(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_h_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_h_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_h_rules,args,3,3);
    }
    return reloq_ptbr_h_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_handle_SPR(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_handle_SPR(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_SPR,args,5,5);
    }
    return reloq_ptbr_handle_SPR(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_handle_end_of_delta(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_handle_end_of_delta(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_end_of_delta,args,2,2);
    }
    return reloq_ptbr_handle_end_of_delta(state, p0, p1);
}
int32_t reloq_ptbr_handle_invalid_anno(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_handle_invalid_anno(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_invalid_anno,args,3,3);
    }
    return reloq_ptbr_handle_invalid_anno(state, p0, p1, p2);
}
int32_t reloq_ptbr_handle_non_phrasal_commas(void *, int32_t);
static inline int32_t reloq_call_ptbr_handle_non_phrasal_commas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_handle_non_phrasal_commas,args,1,1);
    }
    return reloq_ptbr_handle_non_phrasal_commas(state, p0);
}
int32_t reloq_ptbr_handle_pause(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_handle_pause(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_handle_pause,args,2,2);
    }
    return reloq_ptbr_handle_pause(state, p0, p1);
}
int32_t reloq_ptbr_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_low_star_tone,args,1,1);
    }
    return reloq_ptbr_high_low_star_tone(state, p0);
}
int32_t reloq_ptbr_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_star_low_tone,args,1,1);
    }
    return reloq_ptbr_high_star_low_tone(state, p0);
}
int32_t reloq_ptbr_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_high_tone,args,1,1);
    }
    return reloq_ptbr_high_tone(state, p0);
}
int32_t reloq_ptbr_hyphen(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_hyphen(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen,args,3,3);
    }
    return reloq_ptbr_hyphen(state, p0, p1, p2);
}
int32_t reloq_ptbr_hyphen_between_nums(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_hyphen_between_nums,args,3,3);
    }
    return reloq_ptbr_hyphen_between_nums(state, p0, p1, p2);
}
int32_t reloq_ptbr_i_acute_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_i_acute_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_i_acute_rules,args,3,3);
    }
    return reloq_ptbr_i_acute_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_i_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_i_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_i_rules,args,3,3);
    }
    return reloq_ptbr_i_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_identify_ordinal(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_identify_ordinal(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_identify_ordinal,args,2,2);
    }
    return reloq_ptbr_identify_ordinal(state, p0, p1);
}
int32_t reloq_ptbr_in_brackets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_in_brackets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_brackets,args,3,3);
    }
    return reloq_ptbr_in_brackets(state, p0, p1, p2);
}
int32_t reloq_ptbr_in_quotes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_in_quotes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_quotes,args,3,3);
    }
    return reloq_ptbr_in_quotes(state, p0, p1, p2);
}
int32_t reloq_ptbr_in_syllable(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_in_syllable(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_in_syllable,args,4,4);
    }
    return reloq_ptbr_in_syllable(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_increment_output_file_number(void *, int32_t);
static inline int32_t reloq_call_ptbr_increment_output_file_number(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_increment_output_file_number,args,1,1);
    }
    return reloq_ptbr_increment_output_file_number(state, p0);
}
int32_t reloq_ptbr_init_current_word_ptrs(void *, int32_t);
int32_t reloq_ptbr_init_delta(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_delta,args,1,1);
    }
    return reloq_ptbr_init_delta(state, p0);
}
int32_t reloq_ptbr_init_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_gcat_vars,args,1,1);
    }
    return reloq_ptbr_init_gcat_vars(state, p0);
}
int32_t reloq_ptbr_init_lang_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_lang_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_lang_vars,args,1,1);
    }
    return reloq_ptbr_init_lang_vars(state, p0);
}
int32_t reloq_ptbr_init_nuclwrd_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_nuclwrd_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_nuclwrd_vars,args,1,1);
    }
    return reloq_ptbr_init_nuclwrd_vars(state, p0);
}
int32_t reloq_ptbr_init_phr_feature_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_phr_feature_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phr_feature_vars,args,1,1);
    }
    return reloq_ptbr_init_phr_feature_vars(state, p0);
}
int32_t reloq_ptbr_init_phrase_inton_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_phrase_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_inton_vars,args,1,1);
    }
    return reloq_ptbr_init_phrase_inton_vars(state, p0);
}
int32_t reloq_ptbr_init_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_phrase_vars,args,1,1);
    }
    return reloq_ptbr_init_phrase_vars(state, p0);
}
int32_t reloq_ptbr_init_platform(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_platform(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_platform,args,1,1);
    }
    return reloq_ptbr_init_platform(state, p0);
}
int32_t reloq_ptbr_init_por_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_por_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_por_vars,args,1,1);
    }
    return reloq_ptbr_init_por_vars(state, p0);
}
int32_t reloq_ptbr_init_por_word_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_por_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_por_word_vars,args,1,1);
    }
    return reloq_ptbr_init_por_word_vars(state, p0);
}
int32_t reloq_ptbr_init_ptr_End_reproc_string(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_init_ptr_End_reproc_string(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_init_ptr_End_reproc_string,args,3,3);
    }
    return reloq_ptbr_init_ptr_End_reproc_string(state, p0, p1, p2);
}
int32_t reloq_ptbr_init_syll_acoustic_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_syll_acoustic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_acoustic_vars,args,1,1);
    }
    return reloq_ptbr_init_syll_acoustic_vars(state, p0);
}
int32_t reloq_ptbr_init_syll_inton_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_syll_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_inton_vars,args,1,1);
    }
    return reloq_ptbr_init_syll_inton_vars(state, p0);
}
int32_t reloq_ptbr_init_syll_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_syll_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_syll_vars,args,1,1);
    }
    return reloq_ptbr_init_syll_vars(state, p0);
}
int32_t reloq_ptbr_init_symbolic_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_symbolic_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_symbolic_vars,args,1,1);
    }
    return reloq_ptbr_init_symbolic_vars(state, p0);
}
int32_t reloq_ptbr_init_tm_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_tm_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_tm_vars,args,1,1);
    }
    return reloq_ptbr_init_tm_vars(state, p0);
}
int32_t reloq_ptbr_init_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_vars,args,1,1);
    }
    return reloq_ptbr_init_vars(state, p0);
}
int32_t reloq_ptbr_init_voice_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_voice_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_voice_vars,args,1,1);
    }
    return reloq_ptbr_init_voice_vars(state, p0);
}
int32_t reloq_ptbr_init_word(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_word(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word,args,1,1);
    }
    return reloq_ptbr_init_word(state, p0);
}
int32_t reloq_ptbr_init_word_inton_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_word_inton_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_inton_vars,args,1,1);
    }
    return reloq_ptbr_init_word_inton_vars(state, p0);
}
int32_t reloq_ptbr_init_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_ptrs,args,1,1);
    }
    return reloq_ptbr_init_word_ptrs(state, p0);
}
int32_t reloq_ptbr_init_word_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_init_word_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_init_word_vars,args,1,1);
    }
    return reloq_ptbr_init_word_vars(state, p0);
}
int32_t reloq_ptbr_insert_1_2(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_1_2(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_1_2,args,5,5);
    }
    return reloq_ptbr_insert_1_2(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_insert_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_acoustic_vals,args,1,1);
    }
    return reloq_ptbr_insert_acoustic_vals(state, p0);
}
int32_t reloq_ptbr_insert_burst_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_burst_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_burst_dur,args,1,1);
    }
    return reloq_ptbr_insert_burst_dur(state, p0);
}
int32_t reloq_ptbr_insert_char_count(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_char_count(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_char_count,args,1,1);
    }
    return reloq_ptbr_insert_char_count(state, p0);
}
int32_t reloq_ptbr_insert_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_comma_phrase,args,3,3);
    }
    return reloq_ptbr_insert_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_dur(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_dur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_dur,args,5,5);
    }
    return reloq_ptbr_insert_dur(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_insert_e_before_hundreds(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_e_before_hundreds(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_e_before_hundreds,args,2,2);
    }
    return reloq_ptbr_insert_e_before_hundreds(state, p0, p1);
}
int32_t reloq_ptbr_insert_f0(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_f0(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_f0,args,4,4);
    }
    return reloq_ptbr_insert_f0(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_insert_final_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_final_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_final_stress,args,3,3);
    }
    return reloq_ptbr_insert_final_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_gender(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_gender(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_gender,args,3,3);
    }
    return reloq_ptbr_insert_gender(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_hundreds_ending(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_hundreds_ending(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_hundreds_ending,args,3,3);
    }
    return reloq_ptbr_insert_hundreds_ending(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_initial_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_insert_lang_large_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_lang_large_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_lang_large_number,args,3,3);
    }
    return reloq_ptbr_insert_lang_large_number(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_major_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_major_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_major_phrase,args,3,3);
    }
    return reloq_ptbr_insert_major_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_non_phrase_pause(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_insert_ordinal_ending(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_ordinal_ending(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_ordinal_ending,args,3,3);
    }
    return reloq_ptbr_insert_ordinal_ending(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_ordinal_stem(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_ordinal_stem(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_ordinal_stem,args,3,3);
    }
    return reloq_ptbr_insert_ordinal_stem(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_pause_durs(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_pause_durs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_pause_durs,args,1,1);
    }
    return reloq_ptbr_insert_pause_durs(state, p0);
}
int32_t reloq_ptbr_insert_penult_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_penult_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_penult_stress,args,3,3);
    }
    return reloq_ptbr_insert_penult_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase,args,5,5);
    }
    return reloq_ptbr_insert_phrase(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_insert_phrase_final_tones(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_phrase_final_tones(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_final_tones,args,1,1);
    }
    return reloq_ptbr_insert_phrase_final_tones(state, p0);
}
int32_t reloq_ptbr_insert_phrase_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_phrase_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_phrase_pause,args,3,3);
    }
    return reloq_ptbr_insert_phrase_pause(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_por_final_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_por_final_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_por_final_stress,args,3,3);
    }
    return reloq_ptbr_insert_por_final_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_por_large_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_por_large_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_por_large_number,args,3,3);
    }
    return reloq_ptbr_insert_por_large_number(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_por_penult_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_por_penult_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_por_penult_stress,args,3,3);
    }
    return reloq_ptbr_insert_por_penult_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_por_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_por_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_por_spr_phone,args,3,3);
    }
    return reloq_ptbr_insert_por_spr_phone(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_preset_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_preset_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_preset_phrase_tone,args,1,1);
    }
    return reloq_ptbr_insert_preset_phrase_tone(state, p0);
}
int32_t reloq_ptbr_insert_root_final_stress(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_insert_single_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_single_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_single_digit,args,3,3);
    }
    return reloq_ptbr_insert_single_digit(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_space(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_space(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_space,args,2,2);
    }
    return reloq_ptbr_insert_space(state, p0, p1);
}
int32_t reloq_ptbr_insert_spr_phone(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_spr_phone(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_spr_phone,args,3,3);
    }
    return reloq_ptbr_insert_spr_phone(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_stress(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_stress(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_stress,args,4,4);
    }
    return reloq_ptbr_insert_stress(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_insert_syll_level_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_syll_level_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_syll_level_vals,args,1,1);
    }
    return reloq_ptbr_insert_syll_level_vals(state, p0);
}
int32_t reloq_ptbr_insert_target_t3(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_target_t3(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_target_t3,args,1,1);
    }
    return reloq_ptbr_insert_target_t3(state, p0);
}
int32_t reloq_ptbr_insert_transitions(void *, int32_t);
static inline int32_t reloq_call_ptbr_insert_transitions(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_insert_transitions,args,1,1);
    }
    return reloq_ptbr_insert_transitions(state, p0);
}
int32_t reloq_ptbr_insert_unpuncted_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_unpuncted_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrase,args,3,3);
    }
    return reloq_ptbr_insert_unpuncted_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_unpuncted_phrases(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_unpuncted_phrases(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_unpuncted_phrases,args,4,4);
    }
    return reloq_ptbr_insert_unpuncted_phrases(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_insert_userdict_root(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_userdict_root(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_userdict_root,args,3,3);
    }
    return reloq_ptbr_insert_userdict_root(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_voice_change_pause(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_voice_change_pause(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_voice_change_pause,args,3,3);
    }
    return reloq_ptbr_insert_voice_change_pause(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_word,args,3,3);
    }
    return reloq_ptbr_insert_word(state, p0, p1, p2);
}
int32_t reloq_ptbr_insert_zero_sylls(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_insert_zero_sylls(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_insert_zero_sylls,args,3,3);
    }
    return reloq_ptbr_insert_zero_sylls(state, p0, p1, p2);
}
int32_t reloq_ptbr_inside_orig_inp_token(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_inside_orig_inp_token(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_inside_orig_inp_token,args,2,2);
    }
    return reloq_ptbr_inside_orig_inp_token(state, p0, p1);
}
int32_t reloq_ptbr_internal_dict_lookup(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_internal_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_internal_dict_lookup,args,3,3);
    }
    return reloq_ptbr_internal_dict_lookup(state, p0, p1, p2);
}
int32_t reloq_ptbr_interpret_single_char_modes(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_interpret_single_char_modes(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_interpret_single_char_modes,args,3,3);
    }
    return reloq_ptbr_interpret_single_char_modes(state, p0, p1, p2);
}
int32_t reloq_ptbr_isLegalWord(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_isLegalWord(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isLegalWord,args,3,3);
    }
    return reloq_ptbr_isLegalWord(state, p0, p1, p2);
}
int32_t reloq_ptbr_is_accented_wh_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_accented_wh_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_accented_wh_word,args,2,2);
    }
    return reloq_ptbr_is_accented_wh_word(state, p0, p1);
}
int32_t reloq_ptbr_is_afterslash_char(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_afterslash_char(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_afterslash_char,args,3,3);
    }
    return reloq_ptbr_is_afterslash_char(state, p0, p1, p2);
}
int32_t reloq_ptbr_is_ambig_noun_verb_ending(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_ambig_noun_verb_ending(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_ambig_noun_verb_ending,args,5,5);
    }
    return reloq_ptbr_is_ambig_noun_verb_ending(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_is_antepenult_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_antepenult_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_antepenult_stress,args,3,3);
    }
    return reloq_ptbr_is_antepenult_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_is_emphatic_word(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_emphatic_word(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_emphatic_word,args,2,2);
    }
    return reloq_ptbr_is_emphatic_word(state, p0, p1);
}
int32_t reloq_ptbr_is_exclam_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_exclam_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_exclam_verb,args,3,3);
    }
    return reloq_ptbr_is_exclam_verb(state, p0, p1, p2);
}
int32_t reloq_ptbr_is_first_adjacent_noun_accent(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_first_adjacent_noun_accent(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_first_adjacent_noun_accent,args,2,2);
    }
    return reloq_ptbr_is_first_adjacent_noun_accent(state, p0, p1);
}
int32_t reloq_ptbr_is_hyphenated_word(void *, int32_t, int32_t);
int32_t reloq_ptbr_is_in_parentheses(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_is_initial_matrix_verb(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_initial_matrix_verb(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_initial_matrix_verb,args,2,2);
    }
    return reloq_ptbr_is_initial_matrix_verb(state, p0, p1);
}
int32_t reloq_ptbr_is_minor_phrase(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_minor_phrase(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_minor_phrase,args,2,2);
    }
    return reloq_ptbr_is_minor_phrase(state, p0, p1);
}
int32_t reloq_ptbr_is_minor_phrase_edge(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_minor_phrase_edge(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_minor_phrase_edge,args,2,2);
    }
    return reloq_ptbr_is_minor_phrase_edge(state, p0, p1);
}
int32_t reloq_ptbr_is_nasalized_vowel(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_nasalized_vowel(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_nasalized_vowel,args,2,2);
    }
    return reloq_ptbr_is_nasalized_vowel(state, p0, p1);
}
int32_t reloq_ptbr_is_nounadj(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_nounadj(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_nounadj,args,3,3);
    }
    return reloq_ptbr_is_nounadj(state, p0, p1, p2);
}
int32_t reloq_ptbr_is_open_class(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_open_class(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_open_class,args,2,2);
    }
    return reloq_ptbr_is_open_class(state, p0, p1);
}
int32_t reloq_ptbr_is_pathname(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_pathname(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_pathname,args,2,2);
    }
    return reloq_ptbr_is_pathname(state, p0, p1);
}
int32_t reloq_ptbr_is_phon_phrase_edge(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_phon_phrase_edge(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_phon_phrase_edge,args,2,2);
    }
    return reloq_ptbr_is_phon_phrase_edge(state, p0, p1);
}
int32_t reloq_ptbr_is_postnuclear_expr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_postnuclear_expr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_postnuclear_expr,args,3,3);
    }
    return reloq_ptbr_is_postnuclear_expr(state, p0, p1, p2);
}
int32_t reloq_ptbr_is_proper_name(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_proper_name(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_proper_name,args,2,2);
    }
    return reloq_ptbr_is_proper_name(state, p0, p1);
}
int32_t reloq_ptbr_is_quotative_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_quotative_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_quotative_verb,args,3,3);
    }
    return reloq_ptbr_is_quotative_verb(state, p0, p1, p2);
}
int32_t reloq_ptbr_is_sentential_adverb(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_sentential_adverb(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_sentential_adverb,args,2,2);
    }
    return reloq_ptbr_is_sentential_adverb(state, p0, p1);
}
int32_t reloq_ptbr_is_simple_nucleus(void *, int32_t);
static inline int32_t reloq_call_ptbr_is_simple_nucleus(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_simple_nucleus,args,1,1);
    }
    return reloq_ptbr_is_simple_nucleus(state, p0);
}
int32_t reloq_ptbr_is_spr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_spr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_spr,args,3,3);
    }
    return reloq_ptbr_is_spr(state, p0, p1, p2);
}
int32_t reloq_ptbr_is_start_VP(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_is_start_VP(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_is_start_VP,args,2,2);
    }
    return reloq_ptbr_is_start_VP(state, p0, p1);
}
int32_t reloq_ptbr_is_target_syll(void *, int32_t);
static inline int32_t reloq_call_ptbr_is_target_syll(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_is_target_syll,args,1,1);
    }
    return reloq_ptbr_is_target_syll(state, p0);
}
int32_t reloq_ptbr_isitAlt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_isitAlt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitAlt,args,3,3);
    }
    return reloq_ptbr_isitAlt(state, p0, p1, p2);
}
int32_t reloq_ptbr_isitWH(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_isitWH(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isitWH,args,3,3);
    }
    return reloq_ptbr_isitWH(state, p0, p1, p2);
}
int32_t reloq_ptbr_isit_por_Alt(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_isit_por_Alt(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_por_Alt,args,3,3);
    }
    return reloq_ptbr_isit_por_Alt(state, p0, p1, p2);
}
int32_t reloq_ptbr_isit_por_WH(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_isit_por_WH(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_isit_por_WH,args,2,2);
    }
    return reloq_ptbr_isit_por_WH(state, p0, p1);
}
int32_t reloq_ptbr_j_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_j_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_j_rules,args,3,3);
    }
    return reloq_ptbr_j_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_k_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_k_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_k_rules,args,3,3);
    }
    return reloq_ptbr_k_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_l_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_l_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_l_rules,args,3,3);
    }
    return reloq_ptbr_l_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_lang_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_lang_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_hyphen_between_nums,args,5,5);
    }
    return reloq_ptbr_lang_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_lang_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_lang_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_non_phrasal_comma,args,2,2);
    }
    return reloq_ptbr_lang_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_ptbr_lang_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_lang_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_phrase_level_rules,args,3,3);
    }
    return reloq_ptbr_lang_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_lang_specific_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_lang_specific_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_lang_specific_SPR,args,3,3);
    }
    return reloq_ptbr_lang_specific_SPR(state, p0, p1, p2);
}
int32_t reloq_ptbr_legal_end_of_anno(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_legal_end_of_anno(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_legal_end_of_anno,args,2,2);
    }
    return reloq_ptbr_legal_end_of_anno(state, p0, p1);
}
int32_t reloq_ptbr_legal_initial_consonants(void *, int32_t, int32_t);
int32_t reloq_ptbr_letter_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_letter_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_letter_sequence,args,3,3);
    }
    return reloq_ptbr_letter_sequence(state, p0, p1, p2);
}
int32_t reloq_ptbr_longdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_longdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_longdash,args,3,3);
    }
    return reloq_ptbr_longdash(state, p0, p1, p2);
}
int32_t reloq_ptbr_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_high_star_tone,args,1,1);
    }
    return reloq_ptbr_low_high_star_tone(state, p0);
}
int32_t reloq_ptbr_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_star_high_tone,args,1,1);
    }
    return reloq_ptbr_low_star_high_tone(state, p0);
}
int32_t reloq_ptbr_low_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_low_tone,args,1,1);
    }
    return reloq_ptbr_low_tone(state, p0);
}
int32_t reloq_ptbr_lowercase_and_normalize(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_m_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_m_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_m_rules,args,3,3);
    }
    return reloq_ptbr_m_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_adverb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_adverb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_adverb,args,3,3);
    }
    return reloq_ptbr_mark_adverb(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_aux_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_aux_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_aux_verb,args,3,3);
    }
    return reloq_ptbr_mark_aux_verb(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_content_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_content_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_content_word,args,5,5);
    }
    return reloq_ptbr_mark_content_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_mark_determiners(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_determiners(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_determiners,args,3,3);
    }
    return reloq_ptbr_mark_determiners(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_function_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_function_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_function_word,args,5,5);
    }
    return reloq_ptbr_mark_function_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_mark_inf(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_inf(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_inf,args,3,3);
    }
    return reloq_ptbr_mark_inf(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_non_letter(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_mark_noun_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_noun_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_noun_verb,args,3,3);
    }
    return reloq_ptbr_mark_noun_verb(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_noun_verb_from_sets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_noun_verb_from_sets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_noun_verb_from_sets,args,3,3);
    }
    return reloq_ptbr_mark_noun_verb_from_sets(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_nounadj_by_rule(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_nounadj_by_rule(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nounadj_by_rule,args,3,3);
    }
    return reloq_ptbr_mark_nounadj_by_rule(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_nounadj_from_sets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_nounadj_from_sets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nounadj_from_sets,args,3,3);
    }
    return reloq_ptbr_mark_nounadj_from_sets(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_nuclear_accent(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_nuclear_accent(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_nuclear_accent,args,5,5);
    }
    return reloq_ptbr_mark_nuclear_accent(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_mark_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_number,args,4,4);
    }
    return reloq_ptbr_mark_number(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_mark_numbers(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_numbers(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_numbers,args,3,3);
    }
    return reloq_ptbr_mark_numbers(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_por_content_word(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_por_content_word(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_por_content_word,args,3,3);
    }
    return reloq_ptbr_mark_por_content_word(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_por_funct_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_por_funct_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_por_funct_words,args,3,3);
    }
    return reloq_ptbr_mark_por_funct_words(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_por_subcategory(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_por_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_por_subcategory,args,3,3);
    }
    return reloq_ptbr_mark_por_subcategory(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_potential_phrase_breaks(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_potential_phrase_breaks(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_potential_phrase_breaks,args,4,4);
    }
    return reloq_ptbr_mark_potential_phrase_breaks(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_mark_prespart(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_prespart(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_prespart,args,3,3);
    }
    return reloq_ptbr_mark_prespart(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_proper_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_proper_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_proper_name,args,3,3);
    }
    return reloq_ptbr_mark_proper_name(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_spr_syll_vals(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_spr_syll_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_spr_syll_vals,args,6,6);
    }
    return reloq_ptbr_mark_spr_syll_vals(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_mark_subcategory(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_subcategory(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_subcategory,args,4,4);
    }
    return reloq_ptbr_mark_subcategory(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_mark_syll_stress(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_mark_uppercase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_uppercase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_uppercase,args,3,3);
    }
    return reloq_ptbr_mark_uppercase(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_verb_by_rule(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_verb_by_rule(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_verb_by_rule,args,3,3);
    }
    return reloq_ptbr_mark_verb_by_rule(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_verb_from_sets(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_verb_from_sets(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_verb_from_sets,args,3,3);
    }
    return reloq_ptbr_mark_verb_from_sets(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_word_categories(void *, int32_t);
static inline int32_t reloq_call_ptbr_mark_word_categories(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_categories,args,1,1);
    }
    return reloq_ptbr_mark_word_categories(state, p0);
}
int32_t reloq_ptbr_mark_word_str_annot(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_word_str_annot(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_str_annot,args,3,3);
    }
    return reloq_ptbr_mark_word_str_annot(state, p0, p1, p2);
}
int32_t reloq_ptbr_mark_word_stress(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mark_word_stress(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mark_word_stress,args,3,3);
    }
    return reloq_ptbr_mark_word_stress(state, p0, p1, p2);
}
int32_t reloq_ptbr_match_por_paradigm_verb(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_match_por_paradigm_verb(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_match_por_paradigm_verb,args,4,4);
    }
    return reloq_ptbr_match_por_paradigm_verb(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_match_verb_pattern(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_match_verb_pattern(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_match_verb_pattern,args,5,5);
    }
    return reloq_ptbr_match_verb_pattern(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_merge_adjacent_spaces(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_merge_adjacent_spaces(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_merge_adjacent_spaces,args,2,2);
    }
    return reloq_ptbr_merge_adjacent_spaces(state, p0, p1);
}
int32_t reloq_ptbr_mid_vowel_lowering(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_mid_vowel_lowering(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_mid_vowel_lowering,args,5,5);
    }
    return reloq_ptbr_mid_vowel_lowering(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_misc_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_misc_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_misc_chars,args,3,3);
    }
    return reloq_ptbr_misc_chars(state, p0, p1, p2);
}
int32_t reloq_ptbr_misc_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_modify_breathiness_for_femV(void *, int32_t);
static inline int32_t reloq_call_ptbr_modify_breathiness_for_femV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modify_breathiness_for_femV,args,1,1);
    }
    return reloq_ptbr_modify_breathiness_for_femV(state, p0);
}
int32_t reloq_ptbr_modulate_burst(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_modulate_burst(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_burst,args,2,2);
    }
    return reloq_ptbr_modulate_burst(state, p0, p1);
}
int32_t reloq_ptbr_modulate_nas_AV(void *, int32_t);
static inline int32_t reloq_call_ptbr_modulate_nas_AV(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_nas_AV,args,1,1);
    }
    return reloq_ptbr_modulate_nas_AV(state, p0);
}
int32_t reloq_ptbr_modulate_noise(void *, int32_t);
static inline int32_t reloq_call_ptbr_modulate_noise(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_modulate_noise,args,1,1);
    }
    return reloq_ptbr_modulate_noise(state, p0);
}
int32_t reloq_ptbr_modulate_pwind(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_modulate_pwind(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulate_pwind,args,2,2);
    }
    return reloq_ptbr_modulate_pwind(state, p0, p1);
}
int32_t reloq_ptbr_modulus(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_modulus(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_modulus,args,4,4);
    }
    return reloq_ptbr_modulus(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_monetary_exp,args,3,3);
    }
    return reloq_ptbr_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_ptbr_n_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_n_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_n_rules,args,3,3);
    }
    return reloq_ptbr_n_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_nasalize_vow(void *, int32_t);
static inline int32_t reloq_call_ptbr_nasalize_vow(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nasalize_vow,args,1,1);
    }
    return reloq_ptbr_nasalize_vow(state, p0);
}
int32_t reloq_ptbr_no_readin(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_no_readin(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_no_readin,args,2,2);
    }
    return reloq_ptbr_no_readin(state, p0, p1);
}
int32_t reloq_ptbr_no_words_in_delta(void *, int32_t);
static inline int32_t reloq_call_ptbr_no_words_in_delta(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_no_words_in_delta,args,1,1);
    }
    return reloq_ptbr_no_words_in_delta(state, p0);
}
int32_t reloq_ptbr_nonverb_e_lowering(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_nonverb_e_lowering(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_nonverb_e_lowering,args,6,6);
    }
    return reloq_ptbr_nonverb_e_lowering(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_nonverb_o_lowering(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_nonverb_o_lowering(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_nonverb_o_lowering,args,6,6);
    }
    return reloq_ptbr_nonverb_o_lowering(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_normalize_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_normalize_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_letters,args,3,3);
    }
    return reloq_ptbr_normalize_letters(state, p0, p1, p2);
}
int32_t reloq_ptbr_normalize_por_letters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_normalize_por_letters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_por_letters,args,3,3);
    }
    return reloq_ptbr_normalize_por_letters(state, p0, p1, p2);
}
int32_t reloq_ptbr_normalize_text(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_normalize_text(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_normalize_text,args,3,3);
    }
    return reloq_ptbr_normalize_text(state, p0, p1, p2);
}
int32_t reloq_ptbr_noun_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_noun_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_phrase,args,4,4);
    }
    return reloq_ptbr_noun_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_noun_vs_verb(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_noun_vs_verb(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_noun_vs_verb,args,3,3);
    }
    return reloq_ptbr_noun_vs_verb(state, p0, p1, p2);
}
int32_t reloq_ptbr_nucl_f0(void *, int32_t);
static inline int32_t reloq_call_ptbr_nucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_f0,args,1,1);
    }
    return reloq_ptbr_nucl_f0(state, p0);
}
int32_t reloq_ptbr_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_ptbr_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_ptbr_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_ptbr_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_ptbr_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_high_tone,args,1,1);
    }
    return reloq_ptbr_nucl_high_tone(state, p0);
}
int32_t reloq_ptbr_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_ptbr_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_ptbr_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_ptbr_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_ptbr_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucl_low_tone,args,1,1);
    }
    return reloq_ptbr_nucl_low_tone(state, p0);
}
int32_t reloq_ptbr_nucleus_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_nucleus_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_nucleus_vals,args,1,1);
    }
    return reloq_ptbr_nucleus_vals(state, p0);
}
int32_t reloq_ptbr_num_tokens_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_num_tokens_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_tokens_in_range,args,4,4);
    }
    return reloq_ptbr_num_tokens_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_num_words_in_range(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_num_words_in_range(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_num_words_in_range,args,4,4);
    }
    return reloq_ptbr_num_words_in_range(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_o_acute_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_o_acute_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_o_acute_rules,args,3,3);
    }
    return reloq_ptbr_o_acute_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_o_circon_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_o_circon_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_o_circon_rules,args,3,3);
    }
    return reloq_ptbr_o_circon_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_o_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_o_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_o_rules,args,3,3);
    }
    return reloq_ptbr_o_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_o_tilda_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_o_tilda_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_o_tilda_rules,args,3,3);
    }
    return reloq_ptbr_o_tilda_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_one_syllable(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_one_syllable(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_one_syllable,args,3,3);
    }
    return reloq_ptbr_one_syllable(state, p0, p1, p2);
}
int32_t reloq_ptbr_p_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_p_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_p_rules,args,3,3);
    }
    return reloq_ptbr_p_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_parenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_parenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_parenthesis,args,3,3);
    }
    return reloq_ptbr_parenthesis(state, p0, p1, p2);
}
int32_t reloq_ptbr_pause_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_pause_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pause_anno,args,4,4);
    }
    return reloq_ptbr_pause_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_period_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_period_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_period_phrase,args,3,3);
    }
    return reloq_ptbr_period_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_phr_final_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_phr_final_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_final_anno,args,4,4);
    }
    return reloq_ptbr_phr_final_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_ptbr_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phr_initial_F0,args,1,1);
    }
    return reloq_ptbr_phr_initial_F0(state, p0);
}
int32_t reloq_ptbr_phr_prediction_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_phr_prediction_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phr_prediction_anno,args,4,4);
    }
    return reloq_ptbr_phr_prediction_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_phrase_level_printouts(void *, int32_t);
static inline int32_t reloq_call_ptbr_phrase_level_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_printouts,args,1,1);
    }
    return reloq_ptbr_phrase_level_printouts(state, p0);
}
int32_t reloq_ptbr_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_phrase_level_rules,args,3,3);
    }
    return reloq_ptbr_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_tone,args,1,1);
    }
    return reloq_ptbr_phrase_tone(state, p0);
}
int32_t reloq_ptbr_phrase_word_final_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_phrase_word_final_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_phrase_word_final_dur,args,1,1);
    }
    return reloq_ptbr_phrase_word_final_dur(state, p0);
}
int32_t reloq_ptbr_por_1_and_2(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_1_and_2(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_1_and_2,args,4,4);
    }
    return reloq_ptbr_por_1_and_2(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_por_abbr(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_abbr(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_abbr,args,3,3);
    }
    return reloq_ptbr_por_abbr(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_acrotest(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_acrotest(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_acrotest,args,3,3);
    }
    return reloq_ptbr_por_acrotest(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_add_target(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_add_target(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_add_target,args,1,1);
    }
    return reloq_ptbr_por_add_target(state, p0);
}
int32_t reloq_ptbr_por_boundary_tone(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_boundary_tone(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_boundary_tone,args,2,2);
    }
    return reloq_ptbr_por_boundary_tone(state, p0, p1);
}
int32_t reloq_ptbr_por_char_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_char_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_char_name,args,3,3);
    }
    return reloq_ptbr_por_char_name(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_coart_Fv(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_coart_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_coart_Fv,args,1,1);
    }
    return reloq_ptbr_por_coart_Fv(state, p0);
}
int32_t reloq_ptbr_por_coarticulation(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_coarticulation(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_coarticulation,args,1,1);
    }
    return reloq_ptbr_por_coarticulation(state, p0);
}
int32_t reloq_ptbr_por_con_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_con_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_con_vals,args,1,1);
    }
    return reloq_ptbr_por_con_vals(state, p0);
}
int32_t reloq_ptbr_por_cons_durs(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_cons_durs(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_cons_durs,args,3,3);
    }
    return reloq_ptbr_por_cons_durs(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_dental_Fv(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_dental_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_dental_Fv,args,1,1);
    }
    return reloq_ptbr_por_dental_Fv(state, p0);
}
int32_t reloq_ptbr_por_first_postnucl_accent(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_first_postnucl_accent(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_first_postnucl_accent,args,1,1);
    }
    return reloq_ptbr_por_first_postnucl_accent(state, p0);
}
int32_t reloq_ptbr_por_glide_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_glide_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_glide_dur,args,2,2);
    }
    return reloq_ptbr_por_glide_dur(state, p0, p1);
}
int32_t reloq_ptbr_por_glide_w_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_glide_w_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_glide_w_dur,args,2,2);
    }
    return reloq_ptbr_por_glide_w_dur(state, p0, p1);
}
int32_t reloq_ptbr_por_glide_y_dur(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_glide_y_dur(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_glide_y_dur,args,2,2);
    }
    return reloq_ptbr_por_glide_y_dur(state, p0, p1);
}
int32_t reloq_ptbr_por_gramm_cat_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_gramm_cat_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_gramm_cat_dict,args,3,3);
    }
    return reloq_ptbr_por_gramm_cat_dict(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_high_low_star_tone,args,1,1);
    }
    return reloq_ptbr_por_high_low_star_tone(state, p0);
}
int32_t reloq_ptbr_por_high_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_high_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_high_pal_Fv,args,1,1);
    }
    return reloq_ptbr_por_high_pal_Fv(state, p0);
}
int32_t reloq_ptbr_por_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_high_star_low_tone,args,1,1);
    }
    return reloq_ptbr_por_high_star_low_tone(state, p0);
}
int32_t reloq_ptbr_por_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_high_tone,args,1,1);
    }
    return reloq_ptbr_por_high_tone(state, p0);
}
int32_t reloq_ptbr_por_hyphen_between_nums(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_hyphen_between_nums(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_hyphen_between_nums,args,5,5);
    }
    return reloq_ptbr_por_hyphen_between_nums(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_por_labial_Fv(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_labial_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_labial_Fv,args,1,1);
    }
    return reloq_ptbr_por_labial_Fv(state, p0);
}
int32_t reloq_ptbr_por_legal_final_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_legal_final_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_legal_final_cons,args,2,2);
    }
    return reloq_ptbr_por_legal_final_cons(state, p0, p1);
}
int32_t reloq_ptbr_por_legal_initial_cons(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_legal_initial_cons(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_legal_initial_cons,args,2,2);
    }
    return reloq_ptbr_por_legal_initial_cons(state, p0, p1);
}
int32_t reloq_ptbr_por_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_low_high_star_tone,args,1,1);
    }
    return reloq_ptbr_por_low_high_star_tone(state, p0);
}
int32_t reloq_ptbr_por_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_low_star_high_tone,args,1,1);
    }
    return reloq_ptbr_por_low_star_high_tone(state, p0);
}
int32_t reloq_ptbr_por_low_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_low_tone,args,1,1);
    }
    return reloq_ptbr_por_low_tone(state, p0);
}
int32_t reloq_ptbr_por_monetary_exp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_monetary_exp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_monetary_exp,args,3,3);
    }
    return reloq_ptbr_por_monetary_exp(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_nasal_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_nasal_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_nasal_dur,args,1,1);
    }
    return reloq_ptbr_por_nasal_dur(state, p0);
}
int32_t reloq_ptbr_por_non_phrasal_comma(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_non_phrasal_comma(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_non_phrasal_comma,args,2,2);
    }
    return reloq_ptbr_por_non_phrasal_comma(state, p0, p1);
}
int32_t reloq_ptbr_por_nucl_high_low_star_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_nucl_high_low_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_nucl_high_low_star_tone,args,1,1);
    }
    return reloq_ptbr_por_nucl_high_low_star_tone(state, p0);
}
int32_t reloq_ptbr_por_nucl_high_star_low_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_nucl_high_star_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_nucl_high_star_low_tone,args,1,1);
    }
    return reloq_ptbr_por_nucl_high_star_low_tone(state, p0);
}
int32_t reloq_ptbr_por_nucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_nucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_nucl_high_tone,args,1,1);
    }
    return reloq_ptbr_por_nucl_high_tone(state, p0);
}
int32_t reloq_ptbr_por_nucl_low_high_star_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_nucl_low_high_star_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_nucl_low_high_star_tone,args,1,1);
    }
    return reloq_ptbr_por_nucl_low_high_star_tone(state, p0);
}
int32_t reloq_ptbr_por_nucl_low_star_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_nucl_low_star_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_nucl_low_star_high_tone,args,1,1);
    }
    return reloq_ptbr_por_nucl_low_star_high_tone(state, p0);
}
int32_t reloq_ptbr_por_nucl_low_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_nucl_low_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_nucl_low_tone,args,1,1);
    }
    return reloq_ptbr_por_nucl_low_tone(state, p0);
}
int32_t reloq_ptbr_por_pal_Fv(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_pal_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_pal_Fv,args,1,1);
    }
    return reloq_ptbr_por_pal_Fv(state, p0);
}
int32_t reloq_ptbr_por_peak(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_peak(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_peak,args,2,2);
    }
    return reloq_ptbr_por_peak(state, p0, p1);
}
int32_t reloq_ptbr_por_ph_E(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_E(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_E,args,1,1);
    }
    return reloq_ptbr_por_ph_E(state, p0);
}
int32_t reloq_ptbr_por_ph_M(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_M(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_M,args,1,1);
    }
    return reloq_ptbr_por_ph_M(state, p0);
}
int32_t reloq_ptbr_por_ph_N(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_N(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_N,args,1,1);
    }
    return reloq_ptbr_por_ph_N(state, p0);
}
int32_t reloq_ptbr_por_ph_N_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_N_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_N_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_N_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_R(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_R(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_R,args,1,1);
    }
    return reloq_ptbr_por_ph_R(state, p0);
}
int32_t reloq_ptbr_por_ph_R_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_R_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_R_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_R_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_S(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_S(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_S,args,1,1);
    }
    return reloq_ptbr_por_ph_S(state, p0);
}
int32_t reloq_ptbr_por_ph_W(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_W(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_W,args,1,1);
    }
    return reloq_ptbr_por_ph_W(state, p0);
}
int32_t reloq_ptbr_por_ph_Y(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_Y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_Y,args,1,1);
    }
    return reloq_ptbr_por_ph_Y(state, p0);
}
int32_t reloq_ptbr_por_ph_Z(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_Z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_Z,args,1,1);
    }
    return reloq_ptbr_por_ph_Z(state, p0);
}
int32_t reloq_ptbr_por_ph_Z_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_Z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_Z_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_Z_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_a(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_a(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_a,args,1,1);
    }
    return reloq_ptbr_por_ph_a(state, p0);
}
int32_t reloq_ptbr_por_ph_a_nas(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_a_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_a_nas,args,1,1);
    }
    return reloq_ptbr_por_ph_a_nas(state, p0);
}
int32_t reloq_ptbr_por_ph_b(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_b(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_b,args,1,1);
    }
    return reloq_ptbr_por_ph_b(state, p0);
}
int32_t reloq_ptbr_por_ph_b_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_b_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_b_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_b_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_c(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_c(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_c,args,1,1);
    }
    return reloq_ptbr_por_ph_c(state, p0);
}
int32_t reloq_ptbr_por_ph_d(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_d(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_d,args,1,1);
    }
    return reloq_ptbr_por_ph_d(state, p0);
}
int32_t reloq_ptbr_por_ph_d_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_d_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_d_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_d_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_e(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_e(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_e,args,1,1);
    }
    return reloq_ptbr_por_ph_e(state, p0);
}
int32_t reloq_ptbr_por_ph_e_nas(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_e_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_e_nas,args,1,1);
    }
    return reloq_ptbr_por_ph_e_nas(state, p0);
}
int32_t reloq_ptbr_por_ph_f(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_f(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_f,args,1,1);
    }
    return reloq_ptbr_por_ph_f(state, p0);
}
int32_t reloq_ptbr_por_ph_f_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_f_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_f_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_f_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_g(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_g(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_g,args,1,1);
    }
    return reloq_ptbr_por_ph_g(state, p0);
}
int32_t reloq_ptbr_por_ph_g_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_g_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_g_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_g_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_i(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_i(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_i,args,1,1);
    }
    return reloq_ptbr_por_ph_i(state, p0);
}
int32_t reloq_ptbr_por_ph_i_nas(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_i_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_i_nas,args,1,1);
    }
    return reloq_ptbr_por_ph_i_nas(state, p0);
}
int32_t reloq_ptbr_por_ph_k(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_k(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_k,args,1,1);
    }
    return reloq_ptbr_por_ph_k(state, p0);
}
int32_t reloq_ptbr_por_ph_k_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_k_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_k_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_k_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_l(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_l,args,1,1);
    }
    return reloq_ptbr_por_ph_l(state, p0);
}
int32_t reloq_ptbr_por_ph_l_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_l_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_l_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_l_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_ly_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_ly_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_ly_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_ly_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_m(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_m(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_m,args,1,1);
    }
    return reloq_ptbr_por_ph_m(state, p0);
}
int32_t reloq_ptbr_por_ph_n(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_n(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_n,args,1,1);
    }
    return reloq_ptbr_por_ph_n(state, p0);
}
int32_t reloq_ptbr_por_ph_ng(void *, int32_t);
int32_t reloq_ptbr_por_ph_o(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_o(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_o,args,1,1);
    }
    return reloq_ptbr_por_ph_o(state, p0);
}
int32_t reloq_ptbr_por_ph_o_nas(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_o_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_o_nas,args,1,1);
    }
    return reloq_ptbr_por_ph_o_nas(state, p0);
}
int32_t reloq_ptbr_por_ph_p(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_p(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_p,args,1,1);
    }
    return reloq_ptbr_por_ph_p(state, p0);
}
int32_t reloq_ptbr_por_ph_p_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_p_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_p_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_p_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_pal_l(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_pal_l(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_pal_l,args,1,1);
    }
    return reloq_ptbr_por_ph_pal_l(state, p0);
}
int32_t reloq_ptbr_por_ph_r(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_r(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_r,args,1,1);
    }
    return reloq_ptbr_por_ph_r(state, p0);
}
int32_t reloq_ptbr_por_ph_r_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_r_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_r_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_r_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_s(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_s(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_s,args,1,1);
    }
    return reloq_ptbr_por_ph_s(state, p0);
}
int32_t reloq_ptbr_por_ph_sS_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_sS_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_sS_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_sS_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_t(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_t(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_t,args,1,1);
    }
    return reloq_ptbr_por_ph_t(state, p0);
}
int32_t reloq_ptbr_por_ph_t_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_t_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_t_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_t_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_u(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_u(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_u,args,1,1);
    }
    return reloq_ptbr_por_ph_u(state, p0);
}
int32_t reloq_ptbr_por_ph_u_nas(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_u_nas(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_u_nas,args,1,1);
    }
    return reloq_ptbr_por_ph_u_nas(state, p0);
}
int32_t reloq_ptbr_por_ph_v(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_v(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_v,args,1,1);
    }
    return reloq_ptbr_por_ph_v(state, p0);
}
int32_t reloq_ptbr_por_ph_v_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_v_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_v_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_v_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_w(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_w(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_w,args,1,1);
    }
    return reloq_ptbr_por_ph_w(state, p0);
}
int32_t reloq_ptbr_por_ph_w_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_w_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_w_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_w_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_y(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_y(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_y,args,1,1);
    }
    return reloq_ptbr_por_ph_y(state, p0);
}
int32_t reloq_ptbr_por_ph_y_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_y_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_y_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_y_dur(state, p0);
}
int32_t reloq_ptbr_por_ph_z(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_z(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_z,args,1,1);
    }
    return reloq_ptbr_por_ph_z(state, p0);
}
int32_t reloq_ptbr_por_ph_z_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_ph_z_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_ph_z_dur,args,1,1);
    }
    return reloq_ptbr_por_ph_z_dur(state, p0);
}
int32_t reloq_ptbr_por_phr_initial_F0(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_phr_initial_F0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_phr_initial_F0,args,1,1);
    }
    return reloq_ptbr_por_phr_initial_F0(state, p0);
}
int32_t reloq_ptbr_por_phrase_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_phrase_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_phrase_level_rules,args,3,3);
    }
    return reloq_ptbr_por_phrase_level_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_phrase_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_phrase_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_phrase_tone,args,1,1);
    }
    return reloq_ptbr_por_phrase_tone(state, p0);
}
int32_t reloq_ptbr_por_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_position_t_ptrs,args,1,1);
    }
    return reloq_ptbr_por_position_t_ptrs(state, p0);
}
int32_t reloq_ptbr_por_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_post_comma_phrase,args,3,3);
    }
    return reloq_ptbr_por_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_postnucl_f0,args,1,1);
    }
    return reloq_ptbr_por_postnucl_f0(state, p0);
}
int32_t reloq_ptbr_por_postnucl_high_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_postnucl_high_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_postnucl_high_tone,args,1,1);
    }
    return reloq_ptbr_por_postnucl_high_tone(state, p0);
}
int32_t reloq_ptbr_por_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_prenucl_f0,args,1,1);
    }
    return reloq_ptbr_por_prenucl_f0(state, p0);
}
int32_t reloq_ptbr_por_roots(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_roots(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_roots,args,4,4);
    }
    return reloq_ptbr_por_roots(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_por_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_said_phrase,args,4,4);
    }
    return reloq_ptbr_por_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_por_symbolic_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_symbolic_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_symbolic_rules,args,3,3);
    }
    return reloq_ptbr_por_symbolic_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_tok_dict,args,3,3);
    }
    return reloq_ptbr_por_tok_dict(state, p0, p1, p2);
}
int32_t reloq_ptbr_por_trans_features(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_trans_features(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_trans_features,args,1,1);
    }
    return reloq_ptbr_por_trans_features(state, p0);
}
int32_t reloq_ptbr_por_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_vals,args,1,1);
    }
    return reloq_ptbr_por_vals(state, p0);
}
int32_t reloq_ptbr_por_velar_Fv(void *, int32_t);
static inline int32_t reloq_call_ptbr_por_velar_Fv(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_por_velar_Fv,args,1,1);
    }
    return reloq_ptbr_por_velar_Fv(state, p0);
}
int32_t reloq_ptbr_por_words(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_por_words(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_por_words,args,3,3);
    }
    return reloq_ptbr_por_words(state, p0, p1, p2);
}
int32_t reloq_ptbr_position_t_ptrs(void *, int32_t);
static inline int32_t reloq_call_ptbr_position_t_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_position_t_ptrs,args,1,1);
    }
    return reloq_ptbr_position_t_ptrs(state, p0);
}
int32_t reloq_ptbr_post_comma_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_post_comma_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_post_comma_phrase,args,3,3);
    }
    return reloq_ptbr_post_comma_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_postnucl_f0(void *, int32_t);
static inline int32_t reloq_call_ptbr_postnucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_postnucl_f0,args,1,1);
    }
    return reloq_ptbr_postnucl_f0(state, p0);
}
int32_t reloq_ptbr_potential_sent_end(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_potential_sent_end(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_potential_sent_end,args,2,2);
    }
    return reloq_ptbr_potential_sent_end(state, p0, p1);
}
int32_t reloq_ptbr_precedes_end_quote(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_precedes_end_quote(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_precedes_end_quote,args,2,2);
    }
    return reloq_ptbr_precedes_end_quote(state, p0, p1);
}
int32_t reloq_ptbr_precedes_number(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_prenucl_f0(void *, int32_t);
static inline int32_t reloq_call_ptbr_prenucl_f0(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_prenucl_f0,args,1,1);
    }
    return reloq_ptbr_prenucl_f0(state, p0);
}
int32_t reloq_ptbr_prep_phrase(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_prep_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7)
{
    if(delta_rule_trace) {
        const int32_t args[]={p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prep_phrase,args,8,8);
    }
    return reloq_ptbr_prep_phrase(state, p0, p1, p2, p3, p4, p5, p6, p7);
}
int32_t reloq_ptbr_prestored_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_prestored_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_prestored_voice_anno,args,4,4);
    }
    return reloq_ptbr_prestored_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_print_F_trans(void *, int32_t, int32_t);
int32_t reloq_ptbr_print_after_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_print_after_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_after_word_chars,args,3,3);
    }
    return reloq_ptbr_print_after_word_chars(state, p0, p1, p2);
}
int32_t reloq_ptbr_print_between_word_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_print_between_word_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_between_word_chars,args,3,3);
    }
    return reloq_ptbr_print_between_word_chars(state, p0, p1, p2);
}
int32_t reloq_ptbr_print_interword_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_print_interword_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_interword_chars,args,3,3);
    }
    return reloq_ptbr_print_interword_chars(state, p0, p1, p2);
}
int32_t reloq_ptbr_print_normalized_inp(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_print_normalized_inp(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_normalized_inp,args,3,3);
    }
    return reloq_ptbr_print_normalized_inp(state, p0, p1, p2);
}
int32_t reloq_ptbr_print_por_SPR(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_print_por_SPR(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_por_SPR,args,3,3);
    }
    return reloq_ptbr_print_por_SPR(state, p0, p1, p2);
}
int32_t reloq_ptbr_print_prompt(void *, int32_t);
static inline int32_t reloq_call_ptbr_print_prompt(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_prompt,args,1,1);
    }
    return reloq_ptbr_print_prompt(state, p0);
}
int32_t reloq_ptbr_print_spr(void *, int32_t);
static inline int32_t reloq_call_ptbr_print_spr(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_print_spr,args,1,1);
    }
    return reloq_ptbr_print_spr(state, p0);
}
int32_t reloq_ptbr_print_streamarrays_from_delta(void *, int32_t);
int32_t reloq_ptbr_print_word_stress(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_print_word_stress(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_print_word_stress,args,2,2);
    }
    return reloq_ptbr_print_word_stress(state, p0, p1);
}
int32_t reloq_ptbr_printouts(void *, int32_t);
static inline int32_t reloq_call_ptbr_printouts(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_printouts,args,1,1);
    }
    return reloq_ptbr_printouts(state, p0);
}
int32_t reloq_ptbr_pro_vs_det(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_pro_vs_det(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_pro_vs_det,args,3,3);
    }
    return reloq_ptbr_pro_vs_det(state, p0, p1, p2);
}
int32_t reloq_ptbr_process_annotation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_process_annotation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_annotation,args,3,3);
    }
    return reloq_ptbr_process_annotation(state, p0, p1, p2);
}
int32_t reloq_ptbr_process_final_apostrophe(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_process_final_apostrophe(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_apostrophe,args,3,3);
    }
    return reloq_ptbr_process_final_apostrophe(state, p0, p1, p2);
}
int32_t reloq_ptbr_process_final_parenthetical(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_process_final_parenthetical(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_final_parenthetical,args,3,3);
    }
    return reloq_ptbr_process_final_parenthetical(state, p0, p1, p2);
}
int32_t reloq_ptbr_process_hostname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_process_hostname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_hostname,args,3,3);
    }
    return reloq_ptbr_process_hostname(state, p0, p1, p2);
}
int32_t reloq_ptbr_process_input(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_process_input(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_input,args,2,2);
    }
    return reloq_ptbr_process_input(state, p0, p1);
}
int32_t reloq_ptbr_process_pathname(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_process_pathname(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_pathname,args,3,3);
    }
    return reloq_ptbr_process_pathname(state, p0, p1, p2);
}
int32_t reloq_ptbr_process_por_fraction(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_process_por_fraction(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_por_fraction,args,3,3);
    }
    return reloq_ptbr_process_por_fraction(state, p0, p1, p2);
}
int32_t reloq_ptbr_process_trailing_period(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_process_trailing_period(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_process_trailing_period,args,4,4);
    }
    return reloq_ptbr_process_trailing_period(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_project_word_boundary_sync(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_project_word_boundary_sync(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_project_word_boundary_sync,args,3,3);
    }
    return reloq_ptbr_project_word_boundary_sync(state, p0, p1, p2);
}
int32_t reloq_ptbr_promote_word(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_punctuation(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_punctuation(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_punctuation,args,3,3);
    }
    return reloq_ptbr_punctuation(state, p0, p1, p2);
}
int32_t reloq_ptbr_q_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_q_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_q_rules,args,3,3);
    }
    return reloq_ptbr_q_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_question_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_question_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_question_phrase,args,3,3);
    }
    return reloq_ptbr_question_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_quote_mark(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_quote_mark(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_quote_mark,args,3,3);
    }
    return reloq_ptbr_quote_mark(state, p0, p1, p2);
}
int32_t reloq_ptbr_r_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_r_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_r_rules,args,3,3);
    }
    return reloq_ptbr_r_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_read_end(void *, int32_t);
static inline int32_t reloq_call_ptbr_read_end(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_read_end,args,1,1);
    }
    return reloq_ptbr_read_end(state, p0);
}
int32_t reloq_ptbr_read_punct_by_name(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_read_punct_by_name(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_read_punct_by_name,args,3,3);
    }
    return reloq_ptbr_read_punct_by_name(state, p0, p1, p2);
}
int32_t reloq_ptbr_realize_prenucl_tone(void *, int32_t);
static inline int32_t reloq_call_ptbr_realize_prenucl_tone(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_realize_prenucl_tone,args,1,1);
    }
    return reloq_ptbr_realize_prenucl_tone(state, p0);
}
int32_t reloq_ptbr_register_SAPI_voices(void *, int32_t);
static inline int32_t reloq_call_ptbr_register_SAPI_voices(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_register_SAPI_voices,args,1,1);
    }
    return reloq_ptbr_register_SAPI_voices(state, p0);
}
int32_t reloq_ptbr_register_por_phones(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_register_por_phones(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_por_phones,args,3,3);
    }
    return reloq_ptbr_register_por_phones(state, p0, p1, p2);
}
int32_t reloq_ptbr_register_voices(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_register_voices(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_register_voices,args,5,5);
    }
    return reloq_ptbr_register_voices(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_relpro(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_relpro(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_relpro,args,6,6);
    }
    return reloq_ptbr_relpro(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_reset_endptr(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_reset_endptr(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_endptr,args,4,4);
    }
    return reloq_ptbr_reset_endptr(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_reset_endptr_bounded(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_reset_enum_ptr(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_reset_enum_ptr(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_enum_ptr,args,2,2);
    }
    return reloq_ptbr_reset_enum_ptr(state, p0, p1);
}
int32_t reloq_ptbr_reset_gcat_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_reset_gcat_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_gcat_vars,args,1,1);
    }
    return reloq_ptbr_reset_gcat_vars(state, p0);
}
int32_t reloq_ptbr_reset_lang_sent_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_reset_lang_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_lang_sent_vars,args,1,1);
    }
    return reloq_ptbr_reset_lang_sent_vars(state, p0);
}
int32_t reloq_ptbr_reset_phrase_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_reset_phrase_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_phrase_vars,args,1,1);
    }
    return reloq_ptbr_reset_phrase_vars(state, p0);
}
int32_t reloq_ptbr_reset_por_sent_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_reset_por_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_por_sent_vars,args,1,1);
    }
    return reloq_ptbr_reset_por_sent_vars(state, p0);
}
int32_t reloq_ptbr_reset_sent_vars(void *, int32_t);
static inline int32_t reloq_call_ptbr_reset_sent_vars(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_reset_sent_vars,args,1,1);
    }
    return reloq_ptbr_reset_sent_vars(state, p0);
}
int32_t reloq_ptbr_reset_token_vars(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_reset_token_vars(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_token_vars,args,2,2);
    }
    return reloq_ptbr_reset_token_vars(state, p0, p1);
}
int32_t reloq_ptbr_reset_word_vals(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_reset_word_vals(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_reset_word_vals,args,2,2);
    }
    return reloq_ptbr_reset_word_vals(state, p0, p1);
}
int32_t reloq_ptbr_restore_antepenult_accent(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_restore_antepenult_accent(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_restore_antepenult_accent,args,3,3);
    }
    return reloq_ptbr_restore_antepenult_accent(state, p0, p1, p2);
}
int32_t reloq_ptbr_restore_final_accent(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_restore_final_accent(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_restore_final_accent,args,3,3);
    }
    return reloq_ptbr_restore_final_accent(state, p0, p1, p2);
}
int32_t reloq_ptbr_roots_dict_lookup(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_roots_dict_lookup(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_roots_dict_lookup,args,4,4);
    }
    return reloq_ptbr_roots_dict_lookup(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_run_filters(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_run_filters(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_run_filters,args,3,3);
    }
    return reloq_ptbr_run_filters(state, p0, p1, p2);
}
int32_t reloq_ptbr_run_speech_module(void *, int32_t);
static inline int32_t reloq_call_ptbr_run_speech_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_speech_module,args,1,1);
    }
    return reloq_ptbr_run_speech_module(state, p0);
}
int32_t reloq_ptbr_run_text_module(void *, int32_t);
static inline int32_t reloq_call_ptbr_run_text_module(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_run_text_module,args,1,1);
    }
    return reloq_ptbr_run_text_module(state, p0);
}
int32_t reloq_ptbr_s_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_s_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_s_rules,args,3,3);
    }
    return reloq_ptbr_s_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_said_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_said_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_said_phrase,args,4,4);
    }
    return reloq_ptbr_said_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_same_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_scale_transdur(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_scale_transdur(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_scale_transdur,args,4,4);
    }
    return reloq_ptbr_scale_transdur(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_scale_word_final_trans(void *, int32_t);
static inline int32_t reloq_call_ptbr_scale_word_final_trans(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_scale_word_final_trans,args,1,1);
    }
    return reloq_ptbr_scale_word_final_trans(state, p0);
}
int32_t reloq_ptbr_semicolon_phrase(void *, int32_t);
static inline int32_t reloq_call_ptbr_semicolon_phrase(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_semicolon_phrase,args,1,1);
    }
    return reloq_ptbr_semicolon_phrase(state, p0);
}
int32_t reloq_ptbr_separate_tokens(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_separate_tokens(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_separate_tokens,args,2,2);
    }
    return reloq_ptbr_separate_tokens(state, p0, p1);
}
int32_t reloq_ptbr_set_av_limit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_set_av_limit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_av_limit,args,3,3);
    }
    return reloq_ptbr_set_av_limit(state, p0, p1, p2);
}
int32_t reloq_ptbr_set_end_of_pause_trans(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_set_end_of_pause_trans(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_end_of_pause_trans,args,3,3);
    }
    return reloq_ptbr_set_end_of_pause_trans(state, p0, p1, p2);
}
int32_t reloq_ptbr_set_f0_range_ptrs(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_set_f0_range_ptrs(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_range_ptrs,args,5,5);
    }
    return reloq_ptbr_set_f0_range_ptrs(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_set_f0_wind_and_base(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_f0_wind_and_base(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_f0_wind_and_base,args,1,1);
    }
    return reloq_ptbr_set_f0_wind_and_base(state, p0);
}
int32_t reloq_ptbr_set_first_postnuc_value(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_set_first_postnuc_value(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_first_postnuc_value,args,2,2);
    }
    return reloq_ptbr_set_first_postnuc_value(state, p0, p1);
}
int32_t reloq_ptbr_set_global_constants(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_global_constants,args,1,1);
    }
    return reloq_ptbr_set_global_constants(state, p0);
}
int32_t reloq_ptbr_set_klattparms(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_klattparms(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_klattparms,args,1,1);
    }
    return reloq_ptbr_set_klattparms(state, p0);
}
int32_t reloq_ptbr_set_lang_global_constants(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_lang_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_lang_global_constants,args,1,1);
    }
    return reloq_ptbr_set_lang_global_constants(state, p0);
}
int32_t reloq_ptbr_set_letter_name_wordcats(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_letter_name_wordcats(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_letter_name_wordcats,args,1,1);
    }
    return reloq_ptbr_set_letter_name_wordcats(state, p0);
}
int32_t reloq_ptbr_set_min_token_vals(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_set_min_token_vals(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_min_token_vals,args,4,4);
    }
    return reloq_ptbr_set_min_token_vals(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_set_num_formants(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_num_formants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_num_formants,args,1,1);
    }
    return reloq_ptbr_set_num_formants(state, p0);
}
int32_t reloq_ptbr_set_phone_offset_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_phone_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_phone_offset_vals,args,1,1);
    }
    return reloq_ptbr_set_phone_offset_vals(state, p0);
}
int32_t reloq_ptbr_set_phrase_defaults(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_set_phrase_defaults(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_phrase_defaults,args,2,2);
    }
    return reloq_ptbr_set_phrase_defaults(state, p0, p1);
}
int32_t reloq_ptbr_set_por_global_constants(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_por_global_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_por_global_constants,args,1,1);
    }
    return reloq_ptbr_set_por_global_constants(state, p0);
}
int32_t reloq_ptbr_set_said_phr_vals(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_set_said_phr_vals(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_set_said_phr_vals,args,3,3);
    }
    return reloq_ptbr_set_said_phr_vals(state, p0, p1, p2);
}
int32_t reloq_ptbr_set_seg_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_seg_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_seg_default_acoustic_vals,args,1,1);
    }
    return reloq_ptbr_set_seg_default_acoustic_vals(state, p0);
}
int32_t reloq_ptbr_set_syll_offset_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_syll_offset_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_offset_vals,args,1,1);
    }
    return reloq_ptbr_set_syll_offset_vals(state, p0);
}
int32_t reloq_ptbr_set_syll_stream_word_ptrs(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_syll_stream_word_ptrs(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_syll_stream_word_ptrs,args,1,1);
    }
    return reloq_ptbr_set_syll_stream_word_ptrs(state, p0);
}
int32_t reloq_ptbr_set_voice_param_constants(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_voice_param_constants(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_voice_param_constants,args,1,1);
    }
    return reloq_ptbr_set_voice_param_constants(state, p0);
}
int32_t reloq_ptbr_set_window_size(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_window_size(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_window_size,args,1,1);
    }
    return reloq_ptbr_set_window_size(state, p0);
}
int32_t reloq_ptbr_set_word_default_acoustic_vals(void *, int32_t);
static inline int32_t reloq_call_ptbr_set_word_default_acoustic_vals(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_set_word_default_acoustic_vals,args,1,1);
    }
    return reloq_ptbr_set_word_default_acoustic_vals(state, p0);
}
int32_t reloq_ptbr_setdur(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_setdur(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setdur,args,3,3);
    }
    return reloq_ptbr_setdur(state, p0, p1, p2);
}
int32_t reloq_ptbr_setup_lang_reg(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_setup_lang_reg(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_setup_lang_reg,args,3,3);
    }
    return reloq_ptbr_setup_lang_reg(state, p0, p1, p2);
}
int32_t reloq_ptbr_shift_tone(void *, int32_t, int32_t);
int32_t reloq_ptbr_shortdash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_shortdash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_shortdash,args,3,3);
    }
    return reloq_ptbr_shortdash(state, p0, p1, p2);
}
int32_t reloq_ptbr_shorten_2stress(void *, int32_t);
static inline int32_t reloq_call_ptbr_shorten_2stress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_shorten_2stress,args,1,1);
    }
    return reloq_ptbr_shorten_2stress(state, p0);
}
int32_t reloq_ptbr_shorten_adjacent_vows(void *, int32_t);
static inline int32_t reloq_call_ptbr_shorten_adjacent_vows(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_shorten_adjacent_vows,args,1,1);
    }
    return reloq_ptbr_shorten_adjacent_vows(state, p0);
}
int32_t reloq_ptbr_shorten_func_unstressed_vows(void *, int32_t);
static inline int32_t reloq_call_ptbr_shorten_func_unstressed_vows(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_shorten_func_unstressed_vows,args,1,1);
    }
    return reloq_ptbr_shorten_func_unstressed_vows(state, p0);
}
int32_t reloq_ptbr_shorten_unstress(void *, int32_t);
static inline int32_t reloq_call_ptbr_shorten_unstress(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_shorten_unstress,args,1,1);
    }
    return reloq_ptbr_shorten_unstress(state, p0);
}
int32_t reloq_ptbr_shorten_weak_syllables(void *, int32_t);
static inline int32_t reloq_call_ptbr_shorten_weak_syllables(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_shorten_weak_syllables,args,1,1);
    }
    return reloq_ptbr_shorten_weak_syllables(state, p0);
}
int32_t reloq_ptbr_single_chars(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_single_chars(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars,args,3,3);
    }
    return reloq_ptbr_single_chars(state, p0, p1, p2);
}
int32_t reloq_ptbr_single_chars_count_inp(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_single_chars_count_inp(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_count_inp,args,5,5);
    }
    return reloq_ptbr_single_chars_count_inp(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_single_chars_forced_syn(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_single_chars_forced_syn(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_forced_syn,args,6,6);
    }
    return reloq_ptbr_single_chars_forced_syn(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_single_chars_to_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_single_chars_to_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_to_punct,args,3,3);
    }
    return reloq_ptbr_single_chars_to_punct(state, p0, p1, p2);
}
int32_t reloq_ptbr_single_chars_token(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_single_chars_token(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_chars_token,args,3,3);
    }
    return reloq_ptbr_single_chars_token(state, p0, p1, p2);
}
int32_t reloq_ptbr_single_letter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_single_letter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_single_letter,args,3,3);
    }
    return reloq_ptbr_single_letter(state, p0, p1, p2);
}
int32_t reloq_ptbr_size_filter(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_size_filter(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_size_filter,args,3,3);
    }
    return reloq_ptbr_size_filter(state, p0, p1, p2);
}
int32_t reloq_ptbr_skip_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_skip_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_delimiters,args,2,2);
    }
    return reloq_ptbr_skip_delimiters(state, p0, p1);
}
int32_t reloq_ptbr_skip_punct_and_delimiters(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_skip_punct_and_delimiters(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_skip_punct_and_delimiters,args,2,2);
    }
    return reloq_ptbr_skip_punct_and_delimiters(state, p0, p1);
}
int32_t reloq_ptbr_slash(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_slash(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash,args,3,3);
    }
    return reloq_ptbr_slash(state, p0, p1, p2);
}
int32_t reloq_ptbr_slash_before_measure(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_slash_before_measure(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_slash_before_measure,args,3,3);
    }
    return reloq_ptbr_slash_before_measure(state, p0, p1, p2);
}
int32_t reloq_ptbr_space(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_space(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_space,args,3,3);
    }
    return reloq_ptbr_space(state, p0, p1, p2);
}
int32_t reloq_ptbr_span_inverted_punct(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_span_inverted_punct(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_span_inverted_punct,args,3,3);
    }
    return reloq_ptbr_span_inverted_punct(state, p0, p1, p2);
}
int32_t reloq_ptbr_spell_out_alphanum(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_spell_out_alphanum(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_alphanum,args,3,3);
    }
    return reloq_ptbr_spell_out_alphanum(state, p0, p1, p2);
}
int32_t reloq_ptbr_spell_out_large_num(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_spell_out_large_num(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_large_num,args,3,3);
    }
    return reloq_ptbr_spell_out_large_num(state, p0, p1, p2);
}
int32_t reloq_ptbr_spell_out_sequence(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_spell_out_sequence(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spell_out_sequence,args,3,3);
    }
    return reloq_ptbr_spell_out_sequence(state, p0, p1, p2);
}
int32_t reloq_ptbr_spell_out_token(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_spellout_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_spellout_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spellout_mode_anno,args,4,4);
    }
    return reloq_ptbr_spellout_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_spr_output_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_spr_output_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_spr_output_anno,args,4,4);
    }
    return reloq_ptbr_spr_output_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_stress_vowel(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_stress_vowel(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_stress_vowel,args,5,5);
    }
    return reloq_ptbr_stress_vowel(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_stretch_focus(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_stretch_focus(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_stretch_focus,args,2,2);
    }
    return reloq_ptbr_stretch_focus(state, p0, p1);
}
int32_t reloq_ptbr_string_to_number(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_string_to_number(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_string_to_number,args,4,4);
    }
    return reloq_ptbr_string_to_number(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_string_to_short(void *, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_strip_diminutive(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_strip_diminutive(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_diminutive,args,3,3);
    }
    return reloq_ptbr_strip_diminutive(state, p0, p1, p2);
}
int32_t reloq_ptbr_strip_last_digit(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_strip_mente(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_strip_mente(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_mente,args,4,4);
    }
    return reloq_ptbr_strip_mente(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_strip_nonlex_prefix(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_strip_other_prefix(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_strip_plural(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_strip_plural(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_plural,args,4,4);
    }
    return reloq_ptbr_strip_plural(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_strip_plural_suffix(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_strip_plural_suffix(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_plural_suffix,args,3,3);
    }
    return reloq_ptbr_strip_plural_suffix(state, p0, p1, p2);
}
int32_t reloq_ptbr_strip_prod_prefix(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_strip_suffix(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_strip_suffix(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_strip_suffix,args,4,4);
    }
    return reloq_ptbr_strip_suffix(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_subord_conj(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_subord_conj(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6, int32_t p7, int32_t p8, int32_t p9)
{
    if(delta_rule_trace) {
        const int32_t args[]={p9, p8, p7, p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_subord_conj,args,10,10);
    }
    return reloq_ptbr_subord_conj(state, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
}
int32_t reloq_ptbr_suffixes(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_suffixes(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_suffixes,args,4,4);
    }
    return reloq_ptbr_suffixes(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_synthesizeRange(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_synthesizeRange(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5, int32_t p6)
{
    if(delta_rule_trace) {
        const int32_t args[]={p6, p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_synthesizeRange,args,7,7);
    }
    return reloq_ptbr_synthesizeRange(state, p0, p1, p2, p3, p4, p5, p6);
}
int32_t reloq_ptbr_t_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_t_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_t_rules,args,3,3);
    }
    return reloq_ptbr_t_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_tag_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_tag_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tag_phrase,args,3,3);
    }
    return reloq_ptbr_tag_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_test_stress_annot(void *, int32_t, int32_t, int32_t);
int32_t reloq_ptbr_text_index(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_text_index(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_text_index,args,2,2);
    }
    return reloq_ptbr_text_index(state, p0, p1);
}
int32_t reloq_ptbr_textnorm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_textnorm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_textnorm_anno,args,4,4);
    }
    return reloq_ptbr_textnorm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_three_cluster(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_three_cluster(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_cluster,args,2,2);
    }
    return reloq_ptbr_three_cluster(state, p0, p1);
}
int32_t reloq_ptbr_three_letter_extension(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_three_letter_extension(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_three_letter_extension,args,2,2);
    }
    return reloq_ptbr_three_letter_extension(state, p0, p1);
}
int32_t reloq_ptbr_tok_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_tok_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict,args,3,3);
    }
    return reloq_ptbr_tok_dict(state, p0, p1, p2);
}
int32_t reloq_ptbr_tok_dict_entry(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_tok_dict_entry(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_dict_entry,args,3,3);
    }
    return reloq_ptbr_tok_dict_entry(state, p0, p1, p2);
}
int32_t reloq_ptbr_tok_lookup(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_tok_lookup(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_tok_lookup,args,2,2);
    }
    return reloq_ptbr_tok_lookup(state, p0, p1);
}
int32_t reloq_ptbr_token_to_digit(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_token_to_digit(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_digit,args,3,3);
    }
    return reloq_ptbr_token_to_digit(state, p0, p1, p2);
}
int32_t reloq_ptbr_token_to_number(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_token_to_number(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_token_to_number,args,3,3);
    }
    return reloq_ptbr_token_to_number(state, p0, p1, p2);
}
int32_t reloq_ptbr_u_acute_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_u_acute_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_u_acute_rules,args,3,3);
    }
    return reloq_ptbr_u_acute_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_u_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_u_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_u_rules,args,3,3);
    }
    return reloq_ptbr_u_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_upgrade_misc(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_upgrade_misc(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_upgrade_misc,args,3,3);
    }
    return reloq_ptbr_upgrade_misc(state, p0, p1, p2);
}
int32_t reloq_ptbr_upgrade_wh_word(void *, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_upgrade_wh_word(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4)
{
    if(delta_rule_trace) {
        const int32_t args[]={p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_upgrade_wh_word,args,5,5);
    }
    return reloq_ptbr_upgrade_wh_word(state, p0, p1, p2, p3, p4);
}
int32_t reloq_ptbr_user_abbr_dict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_user_abbr_dict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_abbr_dict,args,3,3);
    }
    return reloq_ptbr_user_abbr_dict(state, p0, p1, p2);
}
int32_t reloq_ptbr_user_dict_error(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_user_dict_error(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_dict_error,args,4,4);
    }
    return reloq_ptbr_user_dict_error(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_user_rootdict(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_user_rootdict(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_user_rootdict,args,3,3);
    }
    return reloq_ptbr_user_rootdict(state, p0, p1, p2);
}
int32_t reloq_ptbr_v_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_v_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_v_rules,args,3,3);
    }
    return reloq_ptbr_v_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_valid_f0_posn(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_valid_f0_posn(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_valid_f0_posn,args,3,3);
    }
    return reloq_ptbr_valid_f0_posn(state, p0, p1, p2);
}
int32_t reloq_ptbr_verb_phrase(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_verb_phrase(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_verb_phrase,args,4,4);
    }
    return reloq_ptbr_verb_phrase(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_verb_stem_vowel_lowering(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_verb_stem_vowel_lowering(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3, int32_t p4, int32_t p5)
{
    if(delta_rule_trace) {
        const int32_t args[]={p5, p4, p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_verb_stem_vowel_lowering,args,6,6);
    }
    return reloq_ptbr_verb_stem_vowel_lowering(state, p0, p1, p2, p3, p4, p5);
}
int32_t reloq_ptbr_vocal_tract_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_vocal_tract_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocal_tract_anno,args,4,4);
    }
    return reloq_ptbr_vocal_tract_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_vocative_phrase(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_vocative_phrase(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vocative_phrase,args,3,3);
    }
    return reloq_ptbr_vocative_phrase(state, p0, p1, p2);
}
int32_t reloq_ptbr_voice_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_voice_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_anno,args,4,4);
    }
    return reloq_ptbr_voice_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_voice_feature_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_voice_feature_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_voice_feature_anno,args,4,4);
    }
    return reloq_ptbr_voice_feature_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_voiced_stop_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_voiced_stop_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_voiced_stop_dur,args,1,1);
    }
    return reloq_ptbr_voiced_stop_dur(state, p0);
}
int32_t reloq_ptbr_voiceless_stop_dur(void *, int32_t);
static inline int32_t reloq_call_ptbr_voiceless_stop_dur(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_voiceless_stop_dur,args,1,1);
    }
    return reloq_ptbr_voiceless_stop_dur(state, p0);
}
int32_t reloq_ptbr_voicing_assimilation(void *, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_voicing_assimilation(void *state, int32_t p0, int32_t p1)
{
    if(delta_rule_trace) {
        const int32_t args[]={p1, p0};
        return delta_rule_called(DELTA_ENTRY_voicing_assimilation,args,2,2);
    }
    return reloq_ptbr_voicing_assimilation(state, p0, p1);
}
int32_t reloq_ptbr_vowel_epenthesis(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_vowel_epenthesis(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_vowel_epenthesis,args,3,3);
    }
    return reloq_ptbr_vowel_epenthesis(state, p0, p1, p2);
}
int32_t reloq_ptbr_w_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_w_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_w_rules,args,3,3);
    }
    return reloq_ptbr_w_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_which_get_non_vowels(void *, int32_t);
static inline int32_t reloq_call_ptbr_which_get_non_vowels(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_which_get_non_vowels,args,1,1);
    }
    return reloq_ptbr_which_get_non_vowels(state, p0);
}
int32_t reloq_ptbr_word_accent_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_word_accent_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_accent_anno,args,4,4);
    }
    return reloq_ptbr_word_accent_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_word_level_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_word_level_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_word_level_rules,args,3,3);
    }
    return reloq_ptbr_word_level_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_word_stress_adjust(void *, int32_t);
static inline int32_t reloq_call_ptbr_word_stress_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_stress_adjust,args,1,1);
    }
    return reloq_ptbr_word_stress_adjust(state, p0);
}
int32_t reloq_ptbr_word_syll_adjust(void *, int32_t);
static inline int32_t reloq_call_ptbr_word_syll_adjust(void *state, int32_t p0)
{
    if(delta_rule_trace) {
        const int32_t args[]={p0};
        return delta_rule_called(DELTA_ENTRY_word_syll_adjust,args,1,1);
    }
    return reloq_ptbr_word_syll_adjust(state, p0);
}
int32_t reloq_ptbr_wpm_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_wpm_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_wpm_anno,args,4,4);
    }
    return reloq_ptbr_wpm_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_x_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_x_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_x_rules,args,3,3);
    }
    return reloq_ptbr_x_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_y_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_y_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_y_rules,args,3,3);
    }
    return reloq_ptbr_y_rules(state, p0, p1, p2);
}
int32_t reloq_ptbr_year_mode_anno(void *, int32_t, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_year_mode_anno(void *state, int32_t p0, int32_t p1, int32_t p2, int32_t p3)
{
    if(delta_rule_trace) {
        const int32_t args[]={p3, p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_year_mode_anno,args,4,4);
    }
    return reloq_ptbr_year_mode_anno(state, p0, p1, p2, p3);
}
int32_t reloq_ptbr_z_rules(void *, int32_t, int32_t, int32_t);
static inline int32_t reloq_call_ptbr_z_rules(void *state, int32_t p0, int32_t p1, int32_t p2)
{
    if(delta_rule_trace) {
        const int32_t args[]={p2, p1, p0};
        return delta_rule_called(DELTA_ENTRY_z_rules,args,3,3);
    }
    return reloq_ptbr_z_rules(state, p0, p1, p2);
}
#endif
