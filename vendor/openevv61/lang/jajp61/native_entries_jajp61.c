/* Compact native C entries. Regenerate with tools/compact_native_entries.py. */
#include "delta_lang.h"
#if RELOQ61_NATIVE_ENTRIES
#include "reloq61_entry_bindings.h"
RELOQ61_ENTRY3(entry_char_name, reloq_jajp61_char_name)
RELOQ61_ENTRY3(entry_normalize_letters, reloq_jajp61_normalize_letters)
RELOQ61_ENTRY3(entry_insert_spr_phone, reloq_jajp61_insert_spr_phone)
RELOQ61_ENTRY3(entry_breakup_spr_phones, reloq_jajp61_breakup_spr_phones)
RELOQ61_ENTRY3(entry_lang_specific_SPR, reloq_jajp61_lang_specific_SPR)
RELOQ61_ENTRY3(entry_apply_lang_comma_rules, reloq_jajp61_apply_lang_comma_rules)
RELOQ61_ENTRY4(entry_estimate_lang_nsylls, reloq_jajp61_estimate_lang_nsylls)
RELOQ61_ENTRY3(entry_lang_phrase_level_rules, reloq_jajp61_lang_phrase_level_rules)
RELOQ61_ENTRY1(entry_get_nuc_AV, reloq_jajp61_get_nuc_AV)
RELOQ61_ENTRY1(entry_con_vals, reloq_jajp61_con_vals)
RELOQ61_ENTRY1(entry_nucleus_vals, reloq_jajp61_nucleus_vals)
RELOQ61_ENTRY2(entry_do_h_amps, reloq_jajp61_do_h_amps)
RELOQ61_ENTRY1(entry_find_nucleus, reloq_jajp61_find_nucleus)
RELOQ61_ENTRY3(entry_assign_cons_durs, reloq_jajp61_assign_cons_durs)
RELOQ61_ENTRY1(entry_assign_nuc_durs, reloq_jajp61_assign_nuc_durs)
RELOQ61_ENTRY2(entry_get_trans_dur, reloq_jajp61_get_trans_dur)
RELOQ61_ENTRY1(entry_init_accphr_inton_vars, reloq_jajp61_init_accphr_inton_vars)
RELOQ61_ENTRY1(entry_phr_initial_F0, reloq_jajp61_phr_initial_F0)
RELOQ61_ENTRY1(entry_position_t_ptrs, reloq_jajp61_position_t_ptrs)
RELOQ61_ENTRY1(entry_nucl_high_tone, reloq_jajp61_nucl_high_tone)
RELOQ61_ENTRY1(entry_phrase_tone, reloq_jajp61_phrase_tone)
RELOQ61_ENTRY1(entry_boundary_tone, reloq_jajp61_boundary_tone)
RELOQ61_ENTRY1(entry_init_lang_vars, reloq_jajp61_init_lang_vars)
RELOQ61_ENTRY1(entry_reset_lang_sent_vars, reloq_jajp61_reset_lang_sent_vars)
RELOQ61_ENTRY1(entry_set_lang_global_constants, reloq_jajp61_set_lang_global_constants)
RELOQ61_ENTRY1(entry_DeltaProc_flush, reloq_jajp61_DeltaProc_flush)
RELOQ61_ENTRY1(entry_read_end, reloq_jajp61_read_end)
RELOQ61_ENTRY1(entry_close_platform, reloq_jajp61_close_platform)
RELOQ61_ENTRY1(entry_print_prompt, reloq_jajp61_print_prompt)
RELOQ61_ENTRY3(entry_PhonemeIndex, reloq_jajp61_PhonemeIndex)
RELOQ61_ENTRY1(entry_DeltaProc_registerLanguages, reloq_jajp61_DeltaProc_registerLanguages)
RELOQ61_ENTRY1(entry_register_SAPI_voices, reloq_jajp61_register_SAPI_voices)
RELOQ61_ENTRY3(entry_setup_lang_reg, reloq_jajp61_setup_lang_reg)
RELOQ61_ENTRY5(entry_register_voices, reloq_jajp61_register_voices)
RELOQ61_ENTRY1(entry_init_vars, reloq_jajp61_init_vars)
RELOQ61_ENTRY1(entry_set_global_constants, reloq_jajp61_set_global_constants)
RELOQ61_ENTRY1(entry_set_voice_param_constants, reloq_jajp61_set_voice_param_constants)
RELOQ61_ENTRY1(entry_reset_sent_vars, reloq_jajp61_reset_sent_vars)
RELOQ61_ENTRY1(entry_increment_output_file_number, reloq_jajp61_increment_output_file_number)
RELOQ61_ENTRY1(entry_init_symbolic_vars, reloq_jajp61_init_symbolic_vars)
RELOQ61_ENTRY1(entry_init_tm_vars, reloq_jajp61_init_tm_vars)
RELOQ61_ENTRY1(entry_init_word_ptrs, reloq_jajp61_init_word_ptrs)
RELOQ61_ENTRY1(entry_init_current_word_ptrs, reloq_jajp61_init_current_word_ptrs)
RELOQ61_ENTRY1(entry_init_word_vars, reloq_jajp61_init_word_vars)
RELOQ61_ENTRY1(entry_init_phrase_vars, reloq_jajp61_init_phrase_vars)
RELOQ61_ENTRY1(entry_set_syll_stream_word_ptrs, reloq_jajp61_set_syll_stream_word_ptrs)
RELOQ61_ENTRY1(entry_init_phr_feature_vars, reloq_jajp61_init_phr_feature_vars)
RELOQ61_ENTRY1(entry_init_syll_vars, reloq_jajp61_init_syll_vars)
RELOQ61_ENTRY1(entry_init_accphr_vars, reloq_jajp61_init_accphr_vars)
RELOQ61_ENTRY1(entry_generate_durations, reloq_jajp61_generate_durations)
RELOQ61_ENTRY1(entry_insert_transitions, reloq_jajp61_insert_transitions)
RELOQ61_ENTRY1(entry_insert_pause_durs, reloq_jajp61_insert_pause_durs)
RELOQ61_ENTRY1(entry_find_acoustic_nuc, reloq_jajp61_find_acoustic_nuc)
RELOQ61_ENTRY1(entry_assign_phone_durs, reloq_jajp61_assign_phone_durs)
RELOQ61_ENTRY1(entry_set_syll_offset_vals, reloq_jajp61_set_syll_offset_vals)
RELOQ61_ENTRY4(entry_get_h_dur, reloq_jajp61_get_h_dur)
RELOQ61_ENTRY5(entry_insert_dur, reloq_jajp61_insert_dur)
RELOQ61_ENTRY2(entry_get_pause_trans_dur, reloq_jajp61_get_pause_trans_dur)
RELOQ61_ENTRY2(entry_stretch_focus, reloq_jajp61_stretch_focus)
RELOQ61_ENTRY3(entry_apply_speed_anno, reloq_jajp61_apply_speed_anno)
RELOQ61_ENTRY1(entry_insert_burst_dur, reloq_jajp61_insert_burst_dur)
RELOQ61_ENTRY1(entry_scale_word_final_trans, reloq_jajp61_scale_word_final_trans)
RELOQ61_ENTRY4(entry_scale_transdur, reloq_jajp61_scale_transdur)
RELOQ61_ENTRY1(entry_distribute_nucdur, reloq_jajp61_distribute_nucdur)
RELOQ61_ENTRY3(entry_apply_min_durs, reloq_jajp61_apply_min_durs)
RELOQ61_ENTRY3(entry_setdur, reloq_jajp61_setdur)
RELOQ61_ENTRY3(entry_delete_voice_change_pause, reloq_jajp61_delete_voice_change_pause)
RELOQ61_ENTRY3(entry_set_end_of_pause_trans, reloq_jajp61_set_end_of_pause_trans)
RELOQ61_ENTRY3(entry_run_filters, reloq_jajp61_run_filters)
RELOQ61_ENTRY3(entry_size_filter, reloq_jajp61_size_filter)
RELOQ61_ENTRY3(entry_filter_by_range, reloq_jajp61_filter_by_range)
RELOQ61_ENTRY3(entry_eightkHz_filter, reloq_jajp61_eightkHz_filter)
RELOQ61_ENTRY1(entry_set_word_default_acoustic_vals, reloq_jajp61_set_word_default_acoustic_vals)
RELOQ61_ENTRY1(entry_adjust_AH_AF_for_TLv_w, reloq_jajp61_adjust_AH_AF_for_TLv_w)
RELOQ61_ENTRY1(entry_modify_breathiness_for_femV, reloq_jajp61_modify_breathiness_for_femV)
RELOQ61_ENTRY1(entry_nasalize_vow, reloq_jajp61_nasalize_vow)
RELOQ61_ENTRY1(entry_adjust_AV_for_fem_voice, reloq_jajp61_adjust_AV_for_fem_voice)
RELOQ61_ENTRY1(entry_insert_acoustic_vals, reloq_jajp61_insert_acoustic_vals)
RELOQ61_ENTRY7(entry_add_filtered_val, reloq_jajp61_add_filtered_val)
RELOQ61_ENTRY6(entry_doAV, reloq_jajp61_doAV)
RELOQ61_ENTRY5(entry_addStreamArraySsVal, reloq_jajp61_addStreamArraySsVal)
RELOQ61_ENTRY6(entry_addStreamArrayPtVal, reloq_jajp61_addStreamArrayPtVal)
RELOQ61_ENTRY6(entry_doFNP, reloq_jajp61_doFNP)
RELOQ61_ENTRY6(entry_doFNZ, reloq_jajp61_doFNZ)
RELOQ61_ENTRY6(entry_doAF, reloq_jajp61_doAF)
RELOQ61_ENTRY1(entry_extend_vals, reloq_jajp61_extend_vals)
RELOQ61_ENTRY1(entry_modulate_noise, reloq_jajp61_modulate_noise)
RELOQ61_ENTRY1(entry_modulate_nas_AV, reloq_jajp61_modulate_nas_AV)
RELOQ61_ENTRY7(entry_synthesizeRange, reloq_jajp61_synthesizeRange)
RELOQ61_ENTRY1(entry_set_klattparms, reloq_jajp61_set_klattparms)
RELOQ61_ENTRY1(entry_set_num_formants, reloq_jajp61_set_num_formants)
RELOQ61_ENTRY1(entry_generate_intonation, reloq_jajp61_generate_intonation)
RELOQ61_ENTRY1(entry_add_f0_for_new_voice, reloq_jajp61_add_f0_for_new_voice)
RELOQ61_ENTRY1(entry_init_phrase_inton_vars, reloq_jajp61_init_phrase_inton_vars)
RELOQ61_ENTRY1(entry_init_word_inton_vars, reloq_jajp61_init_word_inton_vars)
RELOQ61_ENTRY1(entry_init_syll_inton_vars, reloq_jajp61_init_syll_inton_vars)
RELOQ61_ENTRY1(entry_is_target_syll, reloq_jajp61_is_target_syll)
RELOQ61_ENTRY1(entry_nucl_f0, reloq_jajp61_nucl_f0)
RELOQ61_ENTRY1(entry_first_postnucl_accent, reloq_jajp61_first_postnucl_accent)
RELOQ61_ENTRY1(entry_insert_target_t3, reloq_jajp61_insert_target_t3)
RELOQ61_ENTRY1(entry_insert_phrase_final_tones, reloq_jajp61_insert_phrase_final_tones)
RELOQ61_ENTRY1(entry_add_f0_for_next_voice, reloq_jajp61_add_f0_for_next_voice)
RELOQ61_ENTRY1(entry_init_voice_vars, reloq_jajp61_init_voice_vars)
RELOQ61_ENTRY1(entry_init_nuclwrd_vars, reloq_jajp61_init_nuclwrd_vars)
RELOQ61_ENTRY1(entry_set_f0_wind_and_base, reloq_jajp61_set_f0_wind_and_base)
RELOQ61_ENTRY6(entry_f0_step, reloq_jajp61_f0_step)
RELOQ61_ENTRY1(entry_realize_prenucl_tone, reloq_jajp61_realize_prenucl_tone)
RELOQ61_ENTRY2(entry_modulate_pwind, reloq_jajp61_modulate_pwind)
RELOQ61_ENTRY4(entry_insert_f0, reloq_jajp61_insert_f0)
RELOQ61_ENTRY5(entry_set_f0_range_ptrs, reloq_jajp61_set_f0_range_ptrs)
RELOQ61_ENTRY3(entry_valid_f0_posn, reloq_jajp61_valid_f0_posn)
RELOQ61_ENTRY2(entry_shift_tone, reloq_jajp61_shift_tone)
RELOQ61_ENTRY1(entry_insert_preset_phrase_tone, reloq_jajp61_insert_preset_phrase_tone)
RELOQ61_ENTRY1(entry_find_most_stressed_syll, reloq_jajp61_find_most_stressed_syll)
RELOQ61_ENTRY4(entry_doF, reloq_jajp61_doF)
RELOQ61_ENTRY1(entry_do_son_amps, reloq_jajp61_do_son_amps)
RELOQ61_ENTRY12(entry_do_fric_amps, reloq_jajp61_do_fric_amps)
RELOQ61_ENTRY1(entry_set_seg_default_acoustic_vals, reloq_jajp61_set_seg_default_acoustic_vals)
RELOQ61_ENTRY3(entry_final_voiced_release, reloq_jajp61_final_voiced_release)
RELOQ61_ENTRY1(entry_do_FTZ_FTP, reloq_jajp61_do_FTZ_FTP)
RELOQ61_ENTRY1(entry_do_non_son_amps, reloq_jajp61_do_non_son_amps)
RELOQ61_ENTRY1(entry_do_pt_voicing, reloq_jajp61_do_pt_voicing)
RELOQ61_ENTRY1(entry_do_release, reloq_jajp61_do_release)
RELOQ61_ENTRY1(entry_do_voice_bar, reloq_jajp61_do_voice_bar)
RELOQ61_ENTRY1(entry_generate_acoustic_vals, reloq_jajp61_generate_acoustic_vals)
RELOQ61_ENTRY2(entry_handle_pause, reloq_jajp61_handle_pause)
RELOQ61_ENTRY1(entry_init_syll_acoustic_vars, reloq_jajp61_init_syll_acoustic_vars)
RELOQ61_ENTRY1(entry_insert_syll_level_vals, reloq_jajp61_insert_syll_level_vals)
RELOQ61_ENTRY1(entry_set_phone_offset_vals, reloq_jajp61_set_phone_offset_vals)
RELOQ61_ENTRY1(entry_word_initial_h, reloq_jajp61_word_initial_h)
RELOQ61_ENTRY1(entry_assign_acoustic_vals, reloq_jajp61_assign_acoustic_vals)
RELOQ61_ENTRY5(entry_define_all_params_for_silence, reloq_jajp61_define_all_params_for_silence)
RELOQ61_ENTRY2(entry_print_F_trans, reloq_jajp61_print_F_trans)
RELOQ61_ENTRY7(entry_callInternalSynthesizer, reloq_jajp61_callInternalSynthesizer)
RELOQ61_ENTRY1(entry_print_streamarrays_from_delta, reloq_jajp61_print_streamarrays_from_delta)
RELOQ61_ENTRY3(entry_process_annotation, reloq_jajp61_process_annotation)
RELOQ61_ENTRY3(entry_is_afterslash_char, reloq_jajp61_is_afterslash_char)
RELOQ61_ENTRY4(entry_SPR_anno, reloq_jajp61_SPR_anno)
RELOQ61_ENTRY3(entry_word_cat_anno, reloq_jajp61_word_cat_anno)
RELOQ61_ENTRY3(entry_emphasis_anno, reloq_jajp61_emphasis_anno)
RELOQ61_ENTRY4(entry_phr_final_anno, reloq_jajp61_phr_final_anno)
RELOQ61_ENTRY4(entry_phr_prediction_anno, reloq_jajp61_phr_prediction_anno)
RELOQ61_ENTRY4(entry_pause_anno, reloq_jajp61_pause_anno)
RELOQ61_ENTRY4(entry_textnorm_anno, reloq_jajp61_textnorm_anno)
RELOQ61_ENTRY4(entry_environment_anno, reloq_jajp61_environment_anno)
RELOQ61_ENTRY3(entry_user_index_anno, reloq_jajp61_user_index_anno)
RELOQ61_ENTRY4(entry_phrase_accent_anno, reloq_jajp61_phrase_accent_anno)
RELOQ61_ENTRY4(entry_voice_anno, reloq_jajp61_voice_anno)
RELOQ61_ENTRY4(entry_word_accent_anno, reloq_jajp61_word_accent_anno)
RELOQ61_ENTRY4(entry_dict_anno, reloq_jajp61_dict_anno)
RELOQ61_ENTRY3(entry_handle_invalid_anno, reloq_jajp61_handle_invalid_anno)
RELOQ61_ENTRY2(entry_legal_end_of_anno, reloq_jajp61_legal_end_of_anno)
RELOQ61_ENTRY4(entry_prestored_voice_anno, reloq_jajp61_prestored_voice_anno)
RELOQ61_ENTRY4(entry_vocal_tract_anno, reloq_jajp61_vocal_tract_anno)
RELOQ61_ENTRY4(entry_voice_feature_anno, reloq_jajp61_voice_feature_anno)
RELOQ61_ENTRY4(entry_wpm_anno, reloq_jajp61_wpm_anno)
RELOQ61_ENTRY4(entry_assign_prestored_voice_vals, reloq_jajp61_assign_prestored_voice_vals)
RELOQ61_ENTRY10(entry_get_formant_voice_vals, reloq_jajp61_get_formant_voice_vals)
RELOQ61_ENTRY2(entry_apply_vocal_tract_val, reloq_jajp61_apply_vocal_tract_val)
RELOQ61_ENTRY2(entry_apply_head_size_val, reloq_jajp61_apply_head_size_val)
RELOQ61_ENTRY2(entry_apply_pitch_baseline_val, reloq_jajp61_apply_pitch_baseline_val)
RELOQ61_ENTRY2(entry_apply_pitch_fluct_val, reloq_jajp61_apply_pitch_fluct_val)
RELOQ61_ENTRY2(entry_apply_roughness_val, reloq_jajp61_apply_roughness_val)
RELOQ61_ENTRY2(entry_apply_breathiness_val, reloq_jajp61_apply_breathiness_val)
RELOQ61_ENTRY2(entry_apply_volume_val, reloq_jajp61_apply_volume_val)
RELOQ61_ENTRY2(entry_apply_speed_val, reloq_jajp61_apply_speed_val)
RELOQ61_ENTRY3(entry_insert_voice_change_pause, reloq_jajp61_insert_voice_change_pause)
RELOQ61_ENTRY4(entry_spr_output_anno, reloq_jajp61_spr_output_anno)
RELOQ61_ENTRY4(entry_spellout_mode_anno, reloq_jajp61_spellout_mode_anno)
RELOQ61_ENTRY4(entry_year_mode_anno, reloq_jajp61_year_mode_anno)
RELOQ61_ENTRY3(entry_user_rootdict, reloq_jajp61_user_rootdict)
RELOQ61_ENTRY4(entry_user_dict_error, reloq_jajp61_user_dict_error)
RELOQ61_ENTRY3(entry_isLegalWord, reloq_jajp61_isLegalWord)
RELOQ61_ENTRY3(entry_insert_userdict_root, reloq_jajp61_insert_userdict_root)
RELOQ61_ENTRY3(entry_internal_dict_lookup, reloq_jajp61_internal_dict_lookup)
RELOQ61_ENTRY3(entry_user_abbr_dict, reloq_jajp61_user_abbr_dict)
RELOQ61_ENTRY1(entry_mark_word_categories, reloq_jajp61_mark_word_categories)
RELOQ61_ENTRY2(entry_count_input, reloq_jajp61_count_input)
RELOQ61_ENTRY1(entry_count_normalized_inp, reloq_jajp61_count_normalized_inp)
RELOQ61_ENTRY3(entry_count_unnormalized_inp, reloq_jajp61_count_unnormalized_inp)
RELOQ61_ENTRY2(entry_forced_synthesis, reloq_jajp61_forced_synthesis)
RELOQ61_ENTRY6(entry_single_chars_forced_syn, reloq_jajp61_single_chars_forced_syn)
RELOQ61_ENTRY1(entry_get_input, reloq_jajp61_get_input)
RELOQ61_ENTRY2(entry_no_readin, reloq_jajp61_no_readin)
RELOQ61_ENTRY1(entry_get_tok, reloq_jajp61_get_tok)
RELOQ61_ENTRY1(entry_gettok, reloq_jajp61_gettok)
RELOQ61_ENTRY1(entry_insert_char_count, reloq_jajp61_insert_char_count)
RELOQ61_ENTRY3(entry_build_phrases, reloq_jajp61_build_phrases)
RELOQ61_ENTRY3(entry_delimit_sentence, reloq_jajp61_delimit_sentence)
RELOQ61_ENTRY2(entry_delimit_major_phrase, reloq_jajp61_delimit_major_phrase)
RELOQ61_ENTRY3(entry_insert_major_phrase, reloq_jajp61_insert_major_phrase)
RELOQ61_ENTRY3(entry_post_comma_phrase, reloq_jajp61_post_comma_phrase)
RELOQ61_ENTRY4(entry_num_tokens_in_range, reloq_jajp61_num_tokens_in_range)
RELOQ61_ENTRY4(entry_insert_unpuncted_phrases, reloq_jajp61_insert_unpuncted_phrases)
RELOQ61_ENTRY4(entry_set_min_token_vals, reloq_jajp61_set_min_token_vals)
RELOQ61_ENTRY3(entry_insert_unpuncted_phrase, reloq_jajp61_insert_unpuncted_phrase)
RELOQ61_ENTRY3(entry_assign_major_phr_features, reloq_jajp61_assign_major_phr_features)
RELOQ61_ENTRY5(entry_insert_phrase, reloq_jajp61_insert_phrase)
RELOQ61_ENTRY3(entry_period_phrase, reloq_jajp61_period_phrase)
RELOQ61_ENTRY1(entry_comma_phrase, reloq_jajp61_comma_phrase)
RELOQ61_ENTRY3(entry_question_phrase, reloq_jajp61_question_phrase)
RELOQ61_ENTRY1(entry_semicolon_phrase, reloq_jajp61_semicolon_phrase)
RELOQ61_ENTRY1(entry_assign_sent_final_features, reloq_jajp61_assign_sent_final_features)
RELOQ61_ENTRY2(entry_assign_currPhr_vals, reloq_jajp61_assign_currPhr_vals)
RELOQ61_ENTRY1(entry_assign_phr_final_anno_features, reloq_jajp61_assign_phr_final_anno_features)
RELOQ61_ENTRY1(entry_assign_wh_quest_features, reloq_jajp61_assign_wh_quest_features)
RELOQ61_ENTRY1(entry_assign_alt_quest_features, reloq_jajp61_assign_alt_quest_features)
RELOQ61_ENTRY1(entry_assign_default_quest_features, reloq_jajp61_assign_default_quest_features)
RELOQ61_ENTRY1(entry_set_window_size, reloq_jajp61_set_window_size)
RELOQ61_ENTRY3(entry_adjust_words_in_phrase, reloq_jajp61_adjust_words_in_phrase)
RELOQ61_ENTRY3(entry_adjust_word_stress, reloq_jajp61_adjust_word_stress)
RELOQ61_ENTRY5(entry_get_nuclear_accent, reloq_jajp61_get_nuclear_accent)
RELOQ61_ENTRY5(entry_adjust_accents, reloq_jajp61_adjust_accents)
RELOQ61_ENTRY5(entry_mark_nuclear_accent, reloq_jajp61_mark_nuclear_accent)
RELOQ61_ENTRY3(entry_insert_phrase_pause, reloq_jajp61_insert_phrase_pause)
RELOQ61_ENTRY1(entry_reset_phrase_vars, reloq_jajp61_reset_phrase_vars)
RELOQ61_ENTRY3(entry_assign_unpuncted_phr_vals, reloq_jajp61_assign_unpuncted_phr_vals)
RELOQ61_ENTRY3(entry_insert_comma_phrase, reloq_jajp61_insert_comma_phrase)
RELOQ61_ENTRY3(entry_adjust_prev_phr_pause, reloq_jajp61_adjust_prev_phr_pause)
RELOQ61_ENTRY1(entry_DeltaProc_main, reloq_jajp61_DeltaProc_main)
RELOQ61_ENTRY1(entry_DeltaProc_start, reloq_jajp61_DeltaProc_start)
RELOQ61_ENTRY1(entry_DeltaProc_process_remaining, reloq_jajp61_DeltaProc_process_remaining)
RELOQ61_ENTRY2(entry_process_input, reloq_jajp61_process_input)
RELOQ61_ENTRY1(entry_DeltaProc_process_sentences, reloq_jajp61_DeltaProc_process_sentences)
RELOQ61_ENTRY1(entry_no_words_in_delta, reloq_jajp61_no_words_in_delta)
RELOQ61_ENTRY1(entry_apply_rules, reloq_jajp61_apply_rules)
RELOQ61_ENTRY2(entry_text_index, reloq_jajp61_text_index)
RELOQ61_ENTRY1(entry_clear_delta, reloq_jajp61_clear_delta)
RELOQ61_ENTRY1(entry_run_text_module, reloq_jajp61_run_text_module)
RELOQ61_ENTRY1(entry_run_speech_module, reloq_jajp61_run_speech_module)
RELOQ61_ENTRY1(entry_printouts, reloq_jajp61_printouts)
RELOQ61_ENTRY1(entry_init_delta, reloq_jajp61_init_delta)
RELOQ61_ENTRY6(entry_get_indices, reloq_jajp61_get_indices)
RELOQ61_ENTRY2(entry_do_user_index_callbacks, reloq_jajp61_do_user_index_callbacks)
RELOQ61_ENTRY4(entry_count_user_indices, reloq_jajp61_count_user_indices)
RELOQ61_ENTRY1(entry_delete_2_more, reloq_jajp61_delete_2_more)
RELOQ61_ENTRY1(entry_DeltaProc_end, reloq_jajp61_DeltaProc_end)
RELOQ61_ENTRY1(entry_DeltaProc_getInputCharCount, reloq_jajp61_DeltaProc_getInputCharCount)
RELOQ61_ENTRY3(entry_normalize_text, reloq_jajp61_normalize_text)
RELOQ61_ENTRY2(entry_handle_end_of_delta, reloq_jajp61_handle_end_of_delta)
RELOQ61_ENTRY2(entry_reset_token_vars, reloq_jajp61_reset_token_vars)
RELOQ61_ENTRY2(entry_merge_adjacent_spaces, reloq_jajp61_merge_adjacent_spaces)
RELOQ61_ENTRY3(entry_space, reloq_jajp61_space)
RELOQ61_ENTRY3(entry_punctuation, reloq_jajp61_punctuation)
RELOQ61_ENTRY3(entry_apostrophe, reloq_jajp61_apostrophe)
RELOQ61_ENTRY3(entry_letter_sequence, reloq_jajp61_letter_sequence)
RELOQ61_ENTRY3(entry_misc_chars, reloq_jajp61_misc_chars)
RELOQ61_ENTRY3(entry_init_ptr_End_reproc_string, reloq_jajp61_init_ptr_End_reproc_string)
RELOQ61_ENTRY3(entry_create_final_sync, reloq_jajp61_create_final_sync)
RELOQ61_ENTRY3(entry_interpret_single_char_modes, reloq_jajp61_interpret_single_char_modes)
RELOQ61_ENTRY3(entry_single_chars_token, reloq_jajp61_single_chars_token)
RELOQ61_ENTRY3(entry_single_chars, reloq_jajp61_single_chars)
RELOQ61_ENTRY3(entry_spell_out_alphanum, reloq_jajp61_spell_out_alphanum)
RELOQ61_ENTRY3(entry_single_letter, reloq_jajp61_single_letter)
RELOQ61_ENTRY3(entry_spell_out_sequence, reloq_jajp61_spell_out_sequence)
RELOQ61_ENTRY2(entry_separate_tokens, reloq_jajp61_separate_tokens)
RELOQ61_ENTRY4(entry_process_trailing_period, reloq_jajp61_process_trailing_period)
RELOQ61_ENTRY3(entry_read_punct_by_name, reloq_jajp61_read_punct_by_name)
RELOQ61_ENTRY2(entry_end_of_sentence, reloq_jajp61_end_of_sentence)
RELOQ61_ENTRY2(entry_skip_punct_and_delimiters, reloq_jajp61_skip_punct_and_delimiters)
RELOQ61_ENTRY2(entry_build_phrase_final_structure, reloq_jajp61_build_phrase_final_structure)
RELOQ61_ENTRY2(entry_end_of_word, reloq_jajp61_end_of_word)
RELOQ61_ENTRY3(entry_in_quotes, reloq_jajp61_in_quotes)
RELOQ61_ENTRY3(entry_in_brackets, reloq_jajp61_in_brackets)
RELOQ61_ENTRY2(entry_beg_of_word, reloq_jajp61_beg_of_word)
RELOQ61_ENTRY2(entry_potential_sent_end, reloq_jajp61_potential_sent_end)
RELOQ61_ENTRY2(entry_end_of_token, reloq_jajp61_end_of_token)
RELOQ61_ENTRY2(entry_create_sentence, reloq_jajp61_create_sentence)
RELOQ61_ENTRY3(entry_project_word_boundary_sync, reloq_jajp61_project_word_boundary_sync)
RELOQ61_ENTRY2(entry_divide_char_count_token, reloq_jajp61_divide_char_count_token)
RELOQ61_ENTRY3(entry_insert_non_phrase_pause, reloq_jajp61_insert_non_phrase_pause)
RELOQ61_ENTRY3(entry_process_final_parenthetical, reloq_jajp61_process_final_parenthetical)
RELOQ61_ENTRY4(entry_find_next_token, reloq_jajp61_find_next_token)
RELOQ61_ENTRY4(entry_find_next_word, reloq_jajp61_find_next_word)
RELOQ61_ENTRY4(entry_find_previous_word, reloq_jajp61_find_previous_word)
RELOQ61_ENTRY4(entry_find_prev_word_optional_comma, reloq_jajp61_find_prev_word_optional_comma)
RELOQ61_ENTRY3(entry_precedes_number, reloq_jajp61_precedes_number)
RELOQ61_ENTRY2(entry_abbr_before_num, reloq_jajp61_abbr_before_num)
RELOQ61_ENTRY3(entry_spell_out_token, reloq_jajp61_spell_out_token)
RELOQ61_ENTRY3(entry_single_chars_to_punct, reloq_jajp61_single_chars_to_punct)
RELOQ61_ENTRY5(entry_single_chars_count_inp, reloq_jajp61_single_chars_count_inp)
RELOQ61_ENTRY2(entry_delete_comma_sync, reloq_jajp61_delete_comma_sync)
RELOQ61_ENTRY2(entry_is_hyphenated_word, reloq_jajp61_is_hyphenated_word)
RELOQ61_ENTRY3(entry_followed_by_hyphen, reloq_jajp61_followed_by_hyphen)
RELOQ61_ENTRY3(entry_mark_uppercase, reloq_jajp61_mark_uppercase)
RELOQ61_ENTRY3(entry_mark_non_letter, reloq_jajp61_mark_non_letter)
RELOQ61_ENTRY2(entry_inside_orig_inp_token, reloq_jajp61_inside_orig_inp_token)
RELOQ61_ENTRY3(entry_lowercase_and_normalize, reloq_jajp61_lowercase_and_normalize)
RELOQ61_ENTRY3(entry_convert_to_lowercase, reloq_jajp61_convert_to_lowercase)
RELOQ61_ENTRY1(entry_print_spr, reloq_jajp61_print_spr)
RELOQ61_ENTRY3(entry_print_between_word_chars, reloq_jajp61_print_between_word_chars)
RELOQ61_ENTRY2(entry_print_word_stress, reloq_jajp61_print_word_stress)
RELOQ61_ENTRY3(entry_print_after_word_chars, reloq_jajp61_print_after_word_chars)
RELOQ61_ENTRY5(entry_handle_SPR, reloq_jajp61_handle_SPR)
RELOQ61_ENTRY3(entry_delete_invalid_SPR, reloq_jajp61_delete_invalid_SPR)
RELOQ61_ENTRY4(entry_find_pair_of_single_quotes, reloq_jajp61_find_pair_of_single_quotes)
RELOQ61_ENTRY6(entry_mark_spr_syll_vals, reloq_jajp61_mark_spr_syll_vals)
RELOQ61_ENTRY3(entry_assign_user_stress, reloq_jajp61_assign_user_stress)
RELOQ61_ENTRY3(entry_word_level_rules, reloq_jajp61_word_level_rules)
RELOQ61_ENTRY3(entry_phrase_level_rules, reloq_jajp61_phrase_level_rules)
RELOQ61_ENTRY3(entry_mark_h, reloq_jajp61_mark_h)
RELOQ61_ENTRY3(entry_copy_stress_to_phones, reloq_jajp61_copy_stress_to_phones)
RELOQ61_ENTRY3(entry_build_words, reloq_jajp61_build_words)
RELOQ61_ENTRY3(entry_insert_word, reloq_jajp61_insert_word)
RELOQ61_ENTRY2(entry_reset_word_vals, reloq_jajp61_reset_word_vals)
RELOQ61_ENTRY1(entry_init_word, reloq_jajp61_init_word)
RELOQ61_ENTRY4(entry_estimate_nsylls, reloq_jajp61_estimate_nsylls)
RELOQ61_ENTRY3(entry_token_to_number, reloq_jajp61_token_to_number)
RELOQ61_ENTRY4(entry_string_to_short, reloq_jajp61_string_to_short)
RELOQ61_ENTRY3(entry_token_to_digit, reloq_jajp61_token_to_digit)
RELOQ61_ENTRY4(entry_string_to_number, reloq_jajp61_string_to_number)
RELOQ61_ENTRY3(entry_absval, reloq_jajp61_absval)
RELOQ61_ENTRY4(entry_find_last_syll, reloq_jajp61_find_last_syll)
RELOQ61_ENTRY5(entry_get_numRootSylls, reloq_jajp61_get_numRootSylls)
RELOQ61_ENTRY4(entry_mark_syll_stress, reloq_jajp61_mark_syll_stress)
RELOQ61_ENTRY4(entry_insert_stress, reloq_jajp61_insert_stress)
RELOQ61_ENTRY3(entry_insert_penult_stress, reloq_jajp61_insert_penult_stress)
RELOQ61_ENTRY3(entry_insert_final_stress, reloq_jajp61_insert_final_stress)
RELOQ61_ENTRY3(entry_insert_initial_stress, reloq_jajp61_insert_initial_stress)
RELOQ61_ENTRY3(entry_insert_root_final_stress, reloq_jajp61_insert_root_final_stress)
RELOQ61_ENTRY3(entry_insert_zero_sylls, reloq_jajp61_insert_zero_sylls)
RELOQ61_ENTRY3(entry_test_stress_annot, reloq_jajp61_test_stress_annot)
RELOQ61_ENTRY3(entry_mark_word_stress, reloq_jajp61_mark_word_stress)
RELOQ61_ENTRY3(entry_demote_word, reloq_jajp61_demote_word)
RELOQ61_ENTRY3(entry_promote_word, reloq_jajp61_promote_word)
RELOQ61_ENTRY2(entry_insert_space, reloq_jajp61_insert_space)
RELOQ61_ENTRY3(entry_delete_inp_from_left, reloq_jajp61_delete_inp_from_left)
RELOQ61_ENTRY3(entry_delete_inp_from_right, reloq_jajp61_delete_inp_from_right)
RELOQ61_ENTRY3(entry_delete_phone_from_left, reloq_jajp61_delete_phone_from_left)
RELOQ61_ENTRY3(entry_delete_phone_from_right, reloq_jajp61_delete_phone_from_right)
RELOQ61_ENTRY3(entry_delete_morph_from_left, reloq_jajp61_delete_morph_from_left)
RELOQ61_ENTRY3(entry_delete_morph_from_right, reloq_jajp61_delete_morph_from_right)
RELOQ61_ENTRY3(entry_delete_syll_from_left, reloq_jajp61_delete_syll_from_left)
RELOQ61_ENTRY3(entry_delete_syll_from_right, reloq_jajp61_delete_syll_from_right)
RELOQ61_ENTRY3(entry_strip_last_digit, reloq_jajp61_strip_last_digit)
RELOQ61_ENTRY4(entry_modulus, reloq_jajp61_modulus)
RELOQ61_ENTRY5(entry_copy_string, reloq_jajp61_copy_string)
RELOQ61_ENTRY5(entry_copy_token, reloq_jajp61_copy_token)
RELOQ61_ENTRY5(entry_same_inp, reloq_jajp61_same_inp)
RELOQ61_ENTRY4(entry_num_words_in_range, reloq_jajp61_num_words_in_range)
RELOQ61_ENTRY3(entry_mark_word_str_annot, reloq_jajp61_mark_word_str_annot)
RELOQ61_ENTRY3(entry_is_in_parentheses, reloq_jajp61_is_in_parentheses)
RELOQ61_ENTRY4(entry_count_sylls, reloq_jajp61_count_sylls)
RELOQ61_ENTRY4(entry_count_chars, reloq_jajp61_count_chars)
RELOQ61_ENTRY4(entry_count_cumulative_chars, reloq_jajp61_count_cumulative_chars)
RELOQ61_ENTRY4(entry_reset_endptr, reloq_jajp61_reset_endptr)
RELOQ61_ENTRY5(entry_reset_endptr_bounded, reloq_jajp61_reset_endptr_bounded)
RELOQ61_ENTRY4(entry_in_syllable, reloq_jajp61_in_syllable)
RELOQ61_ENTRY3(entry_is_spr, reloq_jajp61_is_spr)
RELOQ61_ENTRY2(entry_first_root_vow, reloq_jajp61_first_root_vow)
RELOQ61_ENTRY2(entry_is_after_h, reloq_jajp61_is_after_h)
RELOQ61_ENTRY1(entry_set_jap_global_constants, reloq_jajp61_set_jap_global_constants)
RELOQ61_ENTRY1(entry_init_jap_vars, reloq_jajp61_init_jap_vars)
RELOQ61_ENTRY1(entry_reset_jap_sent_vars, reloq_jajp61_reset_jap_sent_vars)
RELOQ61_ENTRY1(entry_init_jap_symbolic_vars, reloq_jajp61_init_jap_symbolic_vars)
RELOQ61_ENTRY1(entry_init_jap_accphr_inton_vars, reloq_jajp61_init_jap_accphr_inton_vars)
RELOQ61_ENTRY3(entry_register_jap_phones, reloq_jajp61_register_jap_phones)
RELOQ61_ENTRY3(entry_jap_char_name, reloq_jajp61_jap_char_name)
RELOQ61_ENTRY3(entry_jap_cons_durs, reloq_jajp61_jap_cons_durs)
RELOQ61_ENTRY1(entry_jap_ph_p_dur, reloq_jajp61_jap_ph_p_dur)
RELOQ61_ENTRY1(entry_jap_ph_t_dur, reloq_jajp61_jap_ph_t_dur)
RELOQ61_ENTRY1(entry_jap_ph_k_dur, reloq_jajp61_jap_ph_k_dur)
RELOQ61_ENTRY1(entry_jap_ph_b_dur, reloq_jajp61_jap_ph_b_dur)
RELOQ61_ENTRY1(entry_jap_ph_d_dur, reloq_jajp61_jap_ph_d_dur)
RELOQ61_ENTRY1(entry_jap_ph_g_dur, reloq_jajp61_jap_ph_g_dur)
RELOQ61_ENTRY1(entry_jap_ph_f_dur, reloq_jajp61_jap_ph_f_dur)
RELOQ61_ENTRY1(entry_jap_ph_h_dur, reloq_jajp61_jap_ph_h_dur)
RELOQ61_ENTRY1(entry_jap_ph_c_dur, reloq_jajp61_jap_ph_c_dur)
RELOQ61_ENTRY1(entry_jap_ph_s_dur, reloq_jajp61_jap_ph_s_dur)
RELOQ61_ENTRY1(entry_jap_ph_S_dur, reloq_jajp61_jap_ph_S_dur)
RELOQ61_ENTRY1(entry_jap_ph_z_dur, reloq_jajp61_jap_ph_z_dur)
RELOQ61_ENTRY1(entry_jap_ph_Z_dur, reloq_jajp61_jap_ph_Z_dur)
RELOQ61_ENTRY1(entry_jap_ph_m_dur, reloq_jajp61_jap_ph_m_dur)
RELOQ61_ENTRY1(entry_jap_ph_n_dur, reloq_jajp61_jap_ph_n_dur)
RELOQ61_ENTRY1(entry_jap_ph_N_dur, reloq_jajp61_jap_ph_N_dur)
RELOQ61_ENTRY1(entry_jap_ph_w_dur, reloq_jajp61_jap_ph_w_dur)
RELOQ61_ENTRY1(entry_jap_ph_y_dur, reloq_jajp61_jap_ph_y_dur)
RELOQ61_ENTRY1(entry_jap_ph_r_dur, reloq_jajp61_jap_ph_r_dur)
RELOQ61_ENTRY1(entry_jap_cons_dur_adjust, reloq_jajp61_jap_cons_dur_adjust)
RELOQ61_ENTRY1(entry_jpn_init_syll_inton_vars, reloq_jajp61_jpn_init_syll_inton_vars)
RELOQ61_ENTRY1(entry_is_jap_target_syll, reloq_jajp61_is_jap_target_syll)
RELOQ61_ENTRY1(entry_jap_phr_initial_F0, reloq_jajp61_jap_phr_initial_F0)
RELOQ61_ENTRY1(entry_jap_nucl_high_tone, reloq_jajp61_jap_nucl_high_tone)
RELOQ61_ENTRY1(entry_jap_boundary_tone, reloq_jajp61_jap_boundary_tone)
RELOQ61_ENTRY1(entry_jap_position_t_ptrs, reloq_jajp61_jap_position_t_ptrs)
RELOQ61_ENTRY1(entry_jap_first_postnucl_accent, reloq_jajp61_jap_first_postnucl_accent)
RELOQ61_ENTRY1(entry_jap_insert_phrase_final_tones, reloq_jajp61_jap_insert_phrase_final_tones)
RELOQ61_ENTRY2(entry_is_accented_phrase, reloq_jajp61_is_accented_phrase)
RELOQ61_ENTRY3(entry_what_syll_accented_in_phrase, reloq_jajp61_what_syll_accented_in_phrase)
RELOQ61_ENTRY1(entry_jap_trans_features, reloq_jajp61_jap_trans_features)
RELOQ61_ENTRY2(entry_get_jap_trans_dur, reloq_jajp61_get_jap_trans_dur)
RELOQ61_ENTRY2(entry_get_jap_vow_trans_dur, reloq_jajp61_get_jap_vow_trans_dur)
RELOQ61_ENTRY2(entry_get_jap_nas_trans_dur, reloq_jajp61_get_jap_nas_trans_dur)
RELOQ61_ENTRY2(entry_get_jap_fric_trans_dur, reloq_jajp61_get_jap_fric_trans_dur)
RELOQ61_ENTRY2(entry_get_jap_stop_trans_dur, reloq_jajp61_get_jap_stop_trans_dur)
RELOQ61_ENTRY2(entry_get_jap_son_trans_dur, reloq_jajp61_get_jap_son_trans_dur)
RELOQ61_ENTRY4(entry_get_jap_h_dur, reloq_jajp61_get_jap_h_dur)
RELOQ61_ENTRY1(entry_find_jap_nucleus, reloq_jajp61_find_jap_nucleus)
RELOQ61_ENTRY1(entry_assign_jap_nuc_durs, reloq_jajp61_assign_jap_nuc_durs)
RELOQ61_ENTRY1(entry_start_dur, reloq_jajp61_start_dur)
RELOQ61_ENTRY1(entry_phrase_initial_dur, reloq_jajp61_phrase_initial_dur)
RELOQ61_ENTRY1(entry_phrase_medial_dur, reloq_jajp61_phrase_medial_dur)
RELOQ61_ENTRY1(entry_phrase_final_dur, reloq_jajp61_phrase_final_dur)
RELOQ61_ENTRY1(entry_accent_adjustments, reloq_jajp61_accent_adjustments)
RELOQ61_ENTRY1(entry_dur_adjustments, reloq_jajp61_dur_adjustments)
RELOQ61_ENTRY1(entry_devoiced_vowel_dur, reloq_jajp61_devoiced_vowel_dur)
RELOQ61_ENTRY2(entry_ident_to_left, reloq_jajp61_ident_to_left)
RELOQ61_ENTRY1(entry_pre_voice_lengthening, reloq_jajp61_pre_voice_lengthening)
RELOQ61_ENTRY1(entry_pre_con_shortening_rules, reloq_jajp61_pre_con_shortening_rules)
RELOQ61_ENTRY1(entry_vow_to_vow_adjustments, reloq_jajp61_vow_to_vow_adjustments)
RELOQ61_ENTRY1(entry_word_final_lengthening, reloq_jajp61_word_final_lengthening)
RELOQ61_ENTRY2(entry_ident_to_right, reloq_jajp61_ident_to_right)
RELOQ61_ENTRY1(entry_exceptions_to_pre_cons_shortening, reloq_jajp61_exceptions_to_pre_cons_shortening)
RELOQ61_ENTRY2(entry_reduce_nucleus, reloq_jajp61_reduce_nucleus)
RELOQ61_ENTRY1(entry_pf_word_final_syll, reloq_jajp61_pf_word_final_syll)
RELOQ61_ENTRY1(entry_pf_non_final_accented_syll, reloq_jajp61_pf_non_final_accented_syll)
RELOQ61_ENTRY1(entry_jap_con_vals, reloq_jajp61_jap_con_vals)
RELOQ61_ENTRY1(entry_jap_ph_b, reloq_jajp61_jap_ph_b)
RELOQ61_ENTRY1(entry_jap_ph_c, reloq_jajp61_jap_ph_c)
RELOQ61_ENTRY1(entry_jap_ph_d, reloq_jajp61_jap_ph_d)
RELOQ61_ENTRY1(entry_jap_ph_f, reloq_jajp61_jap_ph_f)
RELOQ61_ENTRY1(entry_jap_ph_g, reloq_jajp61_jap_ph_g)
RELOQ61_ENTRY1(entry_jap_ph_p, reloq_jajp61_jap_ph_p)
RELOQ61_ENTRY1(entry_jap_ph_t, reloq_jajp61_jap_ph_t)
RELOQ61_ENTRY1(entry_jap_ph_k, reloq_jajp61_jap_ph_k)
RELOQ61_ENTRY1(entry_jap_ph_m, reloq_jajp61_jap_ph_m)
RELOQ61_ENTRY1(entry_jap_ph_n, reloq_jajp61_jap_ph_n)
RELOQ61_ENTRY1(entry_jap_ph_N, reloq_jajp61_jap_ph_N)
RELOQ61_ENTRY1(entry_jap_ph_z, reloq_jajp61_jap_ph_z)
RELOQ61_ENTRY1(entry_jap_ph_s, reloq_jajp61_jap_ph_s)
RELOQ61_ENTRY1(entry_jap_ph_S, reloq_jajp61_jap_ph_S)
RELOQ61_ENTRY1(entry_jap_ph_Z, reloq_jajp61_jap_ph_Z)
RELOQ61_ENTRY1(entry_jap_ph_w, reloq_jajp61_jap_ph_w)
RELOQ61_ENTRY1(entry_jap_ph_y, reloq_jajp61_jap_ph_y)
RELOQ61_ENTRY1(entry_jap_ph_r, reloq_jajp61_jap_ph_r)
RELOQ61_ENTRY2(entry_add_tilt, reloq_jajp61_add_tilt)
RELOQ61_ENTRY1(entry_is_retroflex_d, reloq_jajp61_is_retroflex_d)
RELOQ61_ENTRY2(entry_do_jap_h_amps, reloq_jajp61_do_jap_h_amps)
RELOQ61_ENTRY1(entry_adjust_female_breathiness, reloq_jajp61_adjust_female_breathiness)
RELOQ61_ENTRY1(entry_adjust_nuc_AV_for_unstr, reloq_jajp61_adjust_nuc_AV_for_unstr)
RELOQ61_ENTRY1(entry_stanjap_vals, reloq_jajp61_stanjap_vals)
RELOQ61_ENTRY1(entry_jap_ph_a, reloq_jajp61_jap_ph_a)
RELOQ61_ENTRY1(entry_jap_ph_e, reloq_jajp61_jap_ph_e)
RELOQ61_ENTRY1(entry_jap_ph_o, reloq_jajp61_jap_ph_o)
RELOQ61_ENTRY1(entry_jap_ph_i, reloq_jajp61_jap_ph_i)
RELOQ61_ENTRY1(entry_jap_ph_u, reloq_jajp61_jap_ph_u)
RELOQ61_ENTRY1(entry_nasalize_jap_vow, reloq_jajp61_nasalize_jap_vow)
RELOQ61_ENTRY4(entry_reduce_long_vowel_coarticulation, reloq_jajp61_reduce_long_vowel_coarticulation)
RELOQ61_ENTRY1(entry_devoice_jap_vowel, reloq_jajp61_devoice_jap_vowel)
RELOQ61_ENTRY1(entry_get_stanjap_nuc_AV, reloq_jajp61_get_stanjap_nuc_AV)
RELOQ61_ENTRY1(entry_adjust_AV_for_misc_segments, reloq_jajp61_adjust_AV_for_misc_segments)
RELOQ61_ENTRY3(entry_jap_phrase_level_rules, reloq_jajp61_jap_phrase_level_rules)
RELOQ61_ENTRY3(entry_high_vowel_devoicing, reloq_jajp61_high_vowel_devoicing)
RELOQ61_ENTRY3(entry_generate_diaphones, reloq_jajp61_generate_diaphones)
RELOQ61_ENTRY1(entry_jap_phrase_type, reloq_jajp61_jap_phrase_type)
RELOQ61_ENTRY1(entry_insert_acc_phrases, reloq_jajp61_insert_acc_phrases)
RELOQ61_ENTRY3(entry_insert_jap_spr_phone, reloq_jajp61_insert_jap_spr_phone)
RELOQ61_ENTRY3(entry_print_jap_SPR, reloq_jajp61_print_jap_SPR)
RELOQ61_ENTRY3(entry_create_syllables, reloq_jajp61_create_syllables)
RELOQ61_ENTRY2(entry_assign_syll_coda, reloq_jajp61_assign_syll_coda)
RELOQ61_ENTRY3(entry_assign_accent_val, reloq_jajp61_assign_accent_val)
RELOQ61_ENTRY4(entry_estimate_jap_nsylls, reloq_jajp61_estimate_jap_nsylls)
RELOQ61_ENTRY3(entry_jap_symbolic_rules, reloq_jajp61_jap_symbolic_rules)
RELOQ61_ENTRY1(entry_init_platform, reloq_jajp61_init_platform)
RELOQ61_ENTRY2(entry_set_phrase_defaults, reloq_jajp61_set_phrase_defaults)
const delta_rule jajp61_delta_rules[] = {
    { "char_name", entry_char_name },
    { "normalize_letters", entry_normalize_letters },
    { "insert_spr_phone", entry_insert_spr_phone },
    { "breakup_spr_phones", entry_breakup_spr_phones },
    { "lang_specific_SPR", entry_lang_specific_SPR },
    { "apply_lang_comma_rules", entry_apply_lang_comma_rules },
    { "estimate_lang_nsylls", entry_estimate_lang_nsylls },
    { "lang_phrase_level_rules", entry_lang_phrase_level_rules },
    { "get_nuc_AV", entry_get_nuc_AV },
    { "con_vals", entry_con_vals },
    { "nucleus_vals", entry_nucleus_vals },
    { "do_h_amps", entry_do_h_amps },
    { "find_nucleus", entry_find_nucleus },
    { "assign_cons_durs", entry_assign_cons_durs },
    { "assign_nuc_durs", entry_assign_nuc_durs },
    { "get_trans_dur", entry_get_trans_dur },
    { "init_accphr_inton_vars", entry_init_accphr_inton_vars },
    { "phr_initial_F0", entry_phr_initial_F0 },
    { "position_t_ptrs", entry_position_t_ptrs },
    { "nucl_high_tone", entry_nucl_high_tone },
    { "phrase_tone", entry_phrase_tone },
    { "boundary_tone", entry_boundary_tone },
    { "init_lang_vars", entry_init_lang_vars },
    { "reset_lang_sent_vars", entry_reset_lang_sent_vars },
    { "set_lang_global_constants", entry_set_lang_global_constants },
    { "DeltaProc_flush", entry_DeltaProc_flush },
    { "read_end", entry_read_end },
    { "close_platform", entry_close_platform },
    { "print_prompt", entry_print_prompt },
    { "PhonemeIndex", entry_PhonemeIndex },
    { "DeltaProc_registerLanguages", entry_DeltaProc_registerLanguages },
    { "register_SAPI_voices", entry_register_SAPI_voices },
    { "setup_lang_reg", entry_setup_lang_reg },
    { "register_voices", entry_register_voices },
    { "init_vars", entry_init_vars },
    { "set_global_constants", entry_set_global_constants },
    { "set_voice_param_constants", entry_set_voice_param_constants },
    { "reset_sent_vars", entry_reset_sent_vars },
    { "increment_output_file_number", entry_increment_output_file_number },
    { "init_symbolic_vars", entry_init_symbolic_vars },
    { "init_tm_vars", entry_init_tm_vars },
    { "init_word_ptrs", entry_init_word_ptrs },
    { "init_current_word_ptrs", entry_init_current_word_ptrs },
    { "init_word_vars", entry_init_word_vars },
    { "init_phrase_vars", entry_init_phrase_vars },
    { "set_syll_stream_word_ptrs", entry_set_syll_stream_word_ptrs },
    { "init_phr_feature_vars", entry_init_phr_feature_vars },
    { "init_syll_vars", entry_init_syll_vars },
    { "init_accphr_vars", entry_init_accphr_vars },
    { "generate_durations", entry_generate_durations },
    { "insert_transitions", entry_insert_transitions },
    { "insert_pause_durs", entry_insert_pause_durs },
    { "find_acoustic_nuc", entry_find_acoustic_nuc },
    { "assign_phone_durs", entry_assign_phone_durs },
    { "set_syll_offset_vals", entry_set_syll_offset_vals },
    { "get_h_dur", entry_get_h_dur },
    { "insert_dur", entry_insert_dur },
    { "get_pause_trans_dur", entry_get_pause_trans_dur },
    { "stretch_focus", entry_stretch_focus },
    { "apply_speed_anno", entry_apply_speed_anno },
    { "insert_burst_dur", entry_insert_burst_dur },
    { "scale_word_final_trans", entry_scale_word_final_trans },
    { "scale_transdur", entry_scale_transdur },
    { "distribute_nucdur", entry_distribute_nucdur },
    { "apply_min_durs", entry_apply_min_durs },
    { "setdur", entry_setdur },
    { "delete_voice_change_pause", entry_delete_voice_change_pause },
    { "set_end_of_pause_trans", entry_set_end_of_pause_trans },
    { "run_filters", entry_run_filters },
    { "size_filter", entry_size_filter },
    { "filter_by_range", entry_filter_by_range },
    { "eightkHz_filter", entry_eightkHz_filter },
    { "set_word_default_acoustic_vals", entry_set_word_default_acoustic_vals },
    { "adjust_AH_AF_for_TLv_w", entry_adjust_AH_AF_for_TLv_w },
    { "modify_breathiness_for_femV", entry_modify_breathiness_for_femV },
    { "nasalize_vow", entry_nasalize_vow },
    { "adjust_AV_for_fem_voice", entry_adjust_AV_for_fem_voice },
    { "insert_acoustic_vals", entry_insert_acoustic_vals },
    { "add_filtered_val", entry_add_filtered_val },
    { "doAV", entry_doAV },
    { "addStreamArraySsVal", entry_addStreamArraySsVal },
    { "addStreamArrayPtVal", entry_addStreamArrayPtVal },
    { "doFNP", entry_doFNP },
    { "doFNZ", entry_doFNZ },
    { "doAF", entry_doAF },
    { "extend_vals", entry_extend_vals },
    { "modulate_noise", entry_modulate_noise },
    { "modulate_nas_AV", entry_modulate_nas_AV },
    { "synthesizeRange", entry_synthesizeRange },
    { "set_klattparms", entry_set_klattparms },
    { "set_num_formants", entry_set_num_formants },
    { "generate_intonation", entry_generate_intonation },
    { "add_f0_for_new_voice", entry_add_f0_for_new_voice },
    { "init_phrase_inton_vars", entry_init_phrase_inton_vars },
    { "init_word_inton_vars", entry_init_word_inton_vars },
    { "init_syll_inton_vars", entry_init_syll_inton_vars },
    { "is_target_syll", entry_is_target_syll },
    { "nucl_f0", entry_nucl_f0 },
    { "first_postnucl_accent", entry_first_postnucl_accent },
    { "insert_target_t3", entry_insert_target_t3 },
    { "insert_phrase_final_tones", entry_insert_phrase_final_tones },
    { "add_f0_for_next_voice", entry_add_f0_for_next_voice },
    { "init_voice_vars", entry_init_voice_vars },
    { "init_nuclwrd_vars", entry_init_nuclwrd_vars },
    { "set_f0_wind_and_base", entry_set_f0_wind_and_base },
    { "f0_step", entry_f0_step },
    { "realize_prenucl_tone", entry_realize_prenucl_tone },
    { "modulate_pwind", entry_modulate_pwind },
    { "insert_f0", entry_insert_f0 },
    { "set_f0_range_ptrs", entry_set_f0_range_ptrs },
    { "valid_f0_posn", entry_valid_f0_posn },
    { "shift_tone", entry_shift_tone },
    { "insert_preset_phrase_tone", entry_insert_preset_phrase_tone },
    { "find_most_stressed_syll", entry_find_most_stressed_syll },
    { "doF", entry_doF },
    { "do_son_amps", entry_do_son_amps },
    { "do_fric_amps", entry_do_fric_amps },
    { "set_seg_default_acoustic_vals", entry_set_seg_default_acoustic_vals },
    { "final_voiced_release", entry_final_voiced_release },
    { "do_FTZ_FTP", entry_do_FTZ_FTP },
    { "do_non_son_amps", entry_do_non_son_amps },
    { "do_pt_voicing", entry_do_pt_voicing },
    { "do_release", entry_do_release },
    { "do_voice_bar", entry_do_voice_bar },
    { "generate_acoustic_vals", entry_generate_acoustic_vals },
    { "handle_pause", entry_handle_pause },
    { "init_syll_acoustic_vars", entry_init_syll_acoustic_vars },
    { "insert_syll_level_vals", entry_insert_syll_level_vals },
    { "set_phone_offset_vals", entry_set_phone_offset_vals },
    { "word_initial_h", entry_word_initial_h },
    { "assign_acoustic_vals", entry_assign_acoustic_vals },
    { "define_all_params_for_silence", entry_define_all_params_for_silence },
    { "print_F_trans", entry_print_F_trans },
    { "callInternalSynthesizer", entry_callInternalSynthesizer },
    { "print_streamarrays_from_delta", entry_print_streamarrays_from_delta },
    { "process_annotation", entry_process_annotation },
    { "is_afterslash_char", entry_is_afterslash_char },
    { "SPR_anno", entry_SPR_anno },
    { "word_cat_anno", entry_word_cat_anno },
    { "emphasis_anno", entry_emphasis_anno },
    { "phr_final_anno", entry_phr_final_anno },
    { "phr_prediction_anno", entry_phr_prediction_anno },
    { "pause_anno", entry_pause_anno },
    { "textnorm_anno", entry_textnorm_anno },
    { "environment_anno", entry_environment_anno },
    { "user_index_anno", entry_user_index_anno },
    { "phrase_accent_anno", entry_phrase_accent_anno },
    { "voice_anno", entry_voice_anno },
    { "word_accent_anno", entry_word_accent_anno },
    { "dict_anno", entry_dict_anno },
    { "handle_invalid_anno", entry_handle_invalid_anno },
    { "legal_end_of_anno", entry_legal_end_of_anno },
    { "prestored_voice_anno", entry_prestored_voice_anno },
    { "vocal_tract_anno", entry_vocal_tract_anno },
    { "voice_feature_anno", entry_voice_feature_anno },
    { "wpm_anno", entry_wpm_anno },
    { "assign_prestored_voice_vals", entry_assign_prestored_voice_vals },
    { "get_formant_voice_vals", entry_get_formant_voice_vals },
    { "apply_vocal_tract_val", entry_apply_vocal_tract_val },
    { "apply_head_size_val", entry_apply_head_size_val },
    { "apply_pitch_baseline_val", entry_apply_pitch_baseline_val },
    { "apply_pitch_fluct_val", entry_apply_pitch_fluct_val },
    { "apply_roughness_val", entry_apply_roughness_val },
    { "apply_breathiness_val", entry_apply_breathiness_val },
    { "apply_volume_val", entry_apply_volume_val },
    { "apply_speed_val", entry_apply_speed_val },
    { "insert_voice_change_pause", entry_insert_voice_change_pause },
    { "spr_output_anno", entry_spr_output_anno },
    { "spellout_mode_anno", entry_spellout_mode_anno },
    { "year_mode_anno", entry_year_mode_anno },
    { "user_rootdict", entry_user_rootdict },
    { "user_dict_error", entry_user_dict_error },
    { "isLegalWord", entry_isLegalWord },
    { "insert_userdict_root", entry_insert_userdict_root },
    { "internal_dict_lookup", entry_internal_dict_lookup },
    { "user_abbr_dict", entry_user_abbr_dict },
    { "mark_word_categories", entry_mark_word_categories },
    { "count_input", entry_count_input },
    { "count_normalized_inp", entry_count_normalized_inp },
    { "count_unnormalized_inp", entry_count_unnormalized_inp },
    { "forced_synthesis", entry_forced_synthesis },
    { "single_chars_forced_syn", entry_single_chars_forced_syn },
    { "get_input", entry_get_input },
    { "no_readin", entry_no_readin },
    { "get_tok", entry_get_tok },
    { "gettok", entry_gettok },
    { "insert_char_count", entry_insert_char_count },
    { "build_phrases", entry_build_phrases },
    { "delimit_sentence", entry_delimit_sentence },
    { "delimit_major_phrase", entry_delimit_major_phrase },
    { "insert_major_phrase", entry_insert_major_phrase },
    { "post_comma_phrase", entry_post_comma_phrase },
    { "num_tokens_in_range", entry_num_tokens_in_range },
    { "insert_unpuncted_phrases", entry_insert_unpuncted_phrases },
    { "set_min_token_vals", entry_set_min_token_vals },
    { "insert_unpuncted_phrase", entry_insert_unpuncted_phrase },
    { "assign_major_phr_features", entry_assign_major_phr_features },
    { "insert_phrase", entry_insert_phrase },
    { "period_phrase", entry_period_phrase },
    { "comma_phrase", entry_comma_phrase },
    { "question_phrase", entry_question_phrase },
    { "semicolon_phrase", entry_semicolon_phrase },
    { "assign_sent_final_features", entry_assign_sent_final_features },
    { "assign_currPhr_vals", entry_assign_currPhr_vals },
    { "assign_phr_final_anno_features", entry_assign_phr_final_anno_features },
    { "assign_wh_quest_features", entry_assign_wh_quest_features },
    { "assign_alt_quest_features", entry_assign_alt_quest_features },
    { "assign_default_quest_features", entry_assign_default_quest_features },
    { "set_window_size", entry_set_window_size },
    { "adjust_words_in_phrase", entry_adjust_words_in_phrase },
    { "adjust_word_stress", entry_adjust_word_stress },
    { "get_nuclear_accent", entry_get_nuclear_accent },
    { "adjust_accents", entry_adjust_accents },
    { "mark_nuclear_accent", entry_mark_nuclear_accent },
    { "insert_phrase_pause", entry_insert_phrase_pause },
    { "reset_phrase_vars", entry_reset_phrase_vars },
    { "assign_unpuncted_phr_vals", entry_assign_unpuncted_phr_vals },
    { "insert_comma_phrase", entry_insert_comma_phrase },
    { "adjust_prev_phr_pause", entry_adjust_prev_phr_pause },
    { "DeltaProc_main", entry_DeltaProc_main },
    { "DeltaProc_start", entry_DeltaProc_start },
    { "DeltaProc_process_remaining", entry_DeltaProc_process_remaining },
    { "process_input", entry_process_input },
    { "DeltaProc_process_sentences", entry_DeltaProc_process_sentences },
    { "no_words_in_delta", entry_no_words_in_delta },
    { "apply_rules", entry_apply_rules },
    { "text_index", entry_text_index },
    { "clear_delta", entry_clear_delta },
    { "run_text_module", entry_run_text_module },
    { "run_speech_module", entry_run_speech_module },
    { "printouts", entry_printouts },
    { "init_delta", entry_init_delta },
    { "get_indices", entry_get_indices },
    { "do_user_index_callbacks", entry_do_user_index_callbacks },
    { "count_user_indices", entry_count_user_indices },
    { "delete_2_more", entry_delete_2_more },
    { "DeltaProc_end", entry_DeltaProc_end },
    { "DeltaProc_getInputCharCount", entry_DeltaProc_getInputCharCount },
    { "normalize_text", entry_normalize_text },
    { "handle_end_of_delta", entry_handle_end_of_delta },
    { "reset_token_vars", entry_reset_token_vars },
    { "merge_adjacent_spaces", entry_merge_adjacent_spaces },
    { "space", entry_space },
    { "punctuation", entry_punctuation },
    { "apostrophe", entry_apostrophe },
    { "letter_sequence", entry_letter_sequence },
    { "misc_chars", entry_misc_chars },
    { "init_ptr_End_reproc_string", entry_init_ptr_End_reproc_string },
    { "create_final_sync", entry_create_final_sync },
    { "interpret_single_char_modes", entry_interpret_single_char_modes },
    { "single_chars_token", entry_single_chars_token },
    { "single_chars", entry_single_chars },
    { "spell_out_alphanum", entry_spell_out_alphanum },
    { "single_letter", entry_single_letter },
    { "spell_out_sequence", entry_spell_out_sequence },
    { "separate_tokens", entry_separate_tokens },
    { "process_trailing_period", entry_process_trailing_period },
    { "read_punct_by_name", entry_read_punct_by_name },
    { "end_of_sentence", entry_end_of_sentence },
    { "skip_punct_and_delimiters", entry_skip_punct_and_delimiters },
    { "build_phrase_final_structure", entry_build_phrase_final_structure },
    { "end_of_word", entry_end_of_word },
    { "in_quotes", entry_in_quotes },
    { "in_brackets", entry_in_brackets },
    { "beg_of_word", entry_beg_of_word },
    { "potential_sent_end", entry_potential_sent_end },
    { "end_of_token", entry_end_of_token },
    { "create_sentence", entry_create_sentence },
    { "project_word_boundary_sync", entry_project_word_boundary_sync },
    { "divide_char_count_token", entry_divide_char_count_token },
    { "insert_non_phrase_pause", entry_insert_non_phrase_pause },
    { "process_final_parenthetical", entry_process_final_parenthetical },
    { "find_next_token", entry_find_next_token },
    { "find_next_word", entry_find_next_word },
    { "find_previous_word", entry_find_previous_word },
    { "find_prev_word_optional_comma", entry_find_prev_word_optional_comma },
    { "precedes_number", entry_precedes_number },
    { "abbr_before_num", entry_abbr_before_num },
    { "spell_out_token", entry_spell_out_token },
    { "single_chars_to_punct", entry_single_chars_to_punct },
    { "single_chars_count_inp", entry_single_chars_count_inp },
    { "delete_comma_sync", entry_delete_comma_sync },
    { "is_hyphenated_word", entry_is_hyphenated_word },
    { "followed_by_hyphen", entry_followed_by_hyphen },
    { "mark_uppercase", entry_mark_uppercase },
    { "mark_non_letter", entry_mark_non_letter },
    { "inside_orig_inp_token", entry_inside_orig_inp_token },
    { "lowercase_and_normalize", entry_lowercase_and_normalize },
    { "convert_to_lowercase", entry_convert_to_lowercase },
    { "print_spr", entry_print_spr },
    { "print_between_word_chars", entry_print_between_word_chars },
    { "print_word_stress", entry_print_word_stress },
    { "print_after_word_chars", entry_print_after_word_chars },
    { "handle_SPR", entry_handle_SPR },
    { "delete_invalid_SPR", entry_delete_invalid_SPR },
    { "find_pair_of_single_quotes", entry_find_pair_of_single_quotes },
    { "mark_spr_syll_vals", entry_mark_spr_syll_vals },
    { "assign_user_stress", entry_assign_user_stress },
    { "word_level_rules", entry_word_level_rules },
    { "phrase_level_rules", entry_phrase_level_rules },
    { "mark_h", entry_mark_h },
    { "copy_stress_to_phones", entry_copy_stress_to_phones },
    { "build_words", entry_build_words },
    { "insert_word", entry_insert_word },
    { "reset_word_vals", entry_reset_word_vals },
    { "init_word", entry_init_word },
    { "estimate_nsylls", entry_estimate_nsylls },
    { "token_to_number", entry_token_to_number },
    { "string_to_short", entry_string_to_short },
    { "token_to_digit", entry_token_to_digit },
    { "string_to_number", entry_string_to_number },
    { "absval", entry_absval },
    { "find_last_syll", entry_find_last_syll },
    { "get_numRootSylls", entry_get_numRootSylls },
    { "mark_syll_stress", entry_mark_syll_stress },
    { "insert_stress", entry_insert_stress },
    { "insert_penult_stress", entry_insert_penult_stress },
    { "insert_final_stress", entry_insert_final_stress },
    { "insert_initial_stress", entry_insert_initial_stress },
    { "insert_root_final_stress", entry_insert_root_final_stress },
    { "insert_zero_sylls", entry_insert_zero_sylls },
    { "test_stress_annot", entry_test_stress_annot },
    { "mark_word_stress", entry_mark_word_stress },
    { "demote_word", entry_demote_word },
    { "promote_word", entry_promote_word },
    { "insert_space", entry_insert_space },
    { "delete_inp_from_left", entry_delete_inp_from_left },
    { "delete_inp_from_right", entry_delete_inp_from_right },
    { "delete_phone_from_left", entry_delete_phone_from_left },
    { "delete_phone_from_right", entry_delete_phone_from_right },
    { "delete_morph_from_left", entry_delete_morph_from_left },
    { "delete_morph_from_right", entry_delete_morph_from_right },
    { "delete_syll_from_left", entry_delete_syll_from_left },
    { "delete_syll_from_right", entry_delete_syll_from_right },
    { "strip_last_digit", entry_strip_last_digit },
    { "modulus", entry_modulus },
    { "copy_string", entry_copy_string },
    { "copy_token", entry_copy_token },
    { "same_inp", entry_same_inp },
    { "num_words_in_range", entry_num_words_in_range },
    { "mark_word_str_annot", entry_mark_word_str_annot },
    { "is_in_parentheses", entry_is_in_parentheses },
    { "count_sylls", entry_count_sylls },
    { "count_chars", entry_count_chars },
    { "count_cumulative_chars", entry_count_cumulative_chars },
    { "reset_endptr", entry_reset_endptr },
    { "reset_endptr_bounded", entry_reset_endptr_bounded },
    { "in_syllable", entry_in_syllable },
    { "is_spr", entry_is_spr },
    { "first_root_vow", entry_first_root_vow },
    { "is_after_h", entry_is_after_h },
    { "set_jap_global_constants", entry_set_jap_global_constants },
    { "init_jap_vars", entry_init_jap_vars },
    { "reset_jap_sent_vars", entry_reset_jap_sent_vars },
    { "init_jap_symbolic_vars", entry_init_jap_symbolic_vars },
    { "init_jap_accphr_inton_vars", entry_init_jap_accphr_inton_vars },
    { "register_jap_phones", entry_register_jap_phones },
    { "jap_char_name", entry_jap_char_name },
    { "jap_cons_durs", entry_jap_cons_durs },
    { "jap_ph_p_dur", entry_jap_ph_p_dur },
    { "jap_ph_t_dur", entry_jap_ph_t_dur },
    { "jap_ph_k_dur", entry_jap_ph_k_dur },
    { "jap_ph_b_dur", entry_jap_ph_b_dur },
    { "jap_ph_d_dur", entry_jap_ph_d_dur },
    { "jap_ph_g_dur", entry_jap_ph_g_dur },
    { "jap_ph_f_dur", entry_jap_ph_f_dur },
    { "jap_ph_h_dur", entry_jap_ph_h_dur },
    { "jap_ph_c_dur", entry_jap_ph_c_dur },
    { "jap_ph_s_dur", entry_jap_ph_s_dur },
    { "jap_ph_S_dur", entry_jap_ph_S_dur },
    { "jap_ph_z_dur", entry_jap_ph_z_dur },
    { "jap_ph_Z_dur", entry_jap_ph_Z_dur },
    { "jap_ph_m_dur", entry_jap_ph_m_dur },
    { "jap_ph_n_dur", entry_jap_ph_n_dur },
    { "jap_ph_N_dur", entry_jap_ph_N_dur },
    { "jap_ph_w_dur", entry_jap_ph_w_dur },
    { "jap_ph_y_dur", entry_jap_ph_y_dur },
    { "jap_ph_r_dur", entry_jap_ph_r_dur },
    { "jap_cons_dur_adjust", entry_jap_cons_dur_adjust },
    { "jpn_init_syll_inton_vars", entry_jpn_init_syll_inton_vars },
    { "is_jap_target_syll", entry_is_jap_target_syll },
    { "jap_phr_initial_F0", entry_jap_phr_initial_F0 },
    { "jap_nucl_high_tone", entry_jap_nucl_high_tone },
    { "jap_boundary_tone", entry_jap_boundary_tone },
    { "jap_position_t_ptrs", entry_jap_position_t_ptrs },
    { "jap_first_postnucl_accent", entry_jap_first_postnucl_accent },
    { "jap_insert_phrase_final_tones", entry_jap_insert_phrase_final_tones },
    { "is_accented_phrase", entry_is_accented_phrase },
    { "what_syll_accented_in_phrase", entry_what_syll_accented_in_phrase },
    { "jap_trans_features", entry_jap_trans_features },
    { "get_jap_trans_dur", entry_get_jap_trans_dur },
    { "get_jap_vow_trans_dur", entry_get_jap_vow_trans_dur },
    { "get_jap_nas_trans_dur", entry_get_jap_nas_trans_dur },
    { "get_jap_fric_trans_dur", entry_get_jap_fric_trans_dur },
    { "get_jap_stop_trans_dur", entry_get_jap_stop_trans_dur },
    { "get_jap_son_trans_dur", entry_get_jap_son_trans_dur },
    { "get_jap_h_dur", entry_get_jap_h_dur },
    { "find_jap_nucleus", entry_find_jap_nucleus },
    { "assign_jap_nuc_durs", entry_assign_jap_nuc_durs },
    { "start_dur", entry_start_dur },
    { "phrase_initial_dur", entry_phrase_initial_dur },
    { "phrase_medial_dur", entry_phrase_medial_dur },
    { "phrase_final_dur", entry_phrase_final_dur },
    { "accent_adjustments", entry_accent_adjustments },
    { "dur_adjustments", entry_dur_adjustments },
    { "devoiced_vowel_dur", entry_devoiced_vowel_dur },
    { "ident_to_left", entry_ident_to_left },
    { "pre_voice_lengthening", entry_pre_voice_lengthening },
    { "pre_con_shortening_rules", entry_pre_con_shortening_rules },
    { "vow_to_vow_adjustments", entry_vow_to_vow_adjustments },
    { "word_final_lengthening", entry_word_final_lengthening },
    { "ident_to_right", entry_ident_to_right },
    { "exceptions_to_pre_cons_shortening", entry_exceptions_to_pre_cons_shortening },
    { "reduce_nucleus", entry_reduce_nucleus },
    { "pf_word_final_syll", entry_pf_word_final_syll },
    { "pf_non_final_accented_syll", entry_pf_non_final_accented_syll },
    { "jap_con_vals", entry_jap_con_vals },
    { "jap_ph_b", entry_jap_ph_b },
    { "jap_ph_c", entry_jap_ph_c },
    { "jap_ph_d", entry_jap_ph_d },
    { "jap_ph_f", entry_jap_ph_f },
    { "jap_ph_g", entry_jap_ph_g },
    { "jap_ph_p", entry_jap_ph_p },
    { "jap_ph_t", entry_jap_ph_t },
    { "jap_ph_k", entry_jap_ph_k },
    { "jap_ph_m", entry_jap_ph_m },
    { "jap_ph_n", entry_jap_ph_n },
    { "jap_ph_N", entry_jap_ph_N },
    { "jap_ph_z", entry_jap_ph_z },
    { "jap_ph_s", entry_jap_ph_s },
    { "jap_ph_S", entry_jap_ph_S },
    { "jap_ph_Z", entry_jap_ph_Z },
    { "jap_ph_w", entry_jap_ph_w },
    { "jap_ph_y", entry_jap_ph_y },
    { "jap_ph_r", entry_jap_ph_r },
    { "add_tilt", entry_add_tilt },
    { "is_retroflex_d", entry_is_retroflex_d },
    { "do_jap_h_amps", entry_do_jap_h_amps },
    { "adjust_female_breathiness", entry_adjust_female_breathiness },
    { "adjust_nuc_AV_for_unstr", entry_adjust_nuc_AV_for_unstr },
    { "stanjap_vals", entry_stanjap_vals },
    { "jap_ph_a", entry_jap_ph_a },
    { "jap_ph_e", entry_jap_ph_e },
    { "jap_ph_o", entry_jap_ph_o },
    { "jap_ph_i", entry_jap_ph_i },
    { "jap_ph_u", entry_jap_ph_u },
    { "nasalize_jap_vow", entry_nasalize_jap_vow },
    { "reduce_long_vowel_coarticulation", entry_reduce_long_vowel_coarticulation },
    { "devoice_jap_vowel", entry_devoice_jap_vowel },
    { "get_stanjap_nuc_AV", entry_get_stanjap_nuc_AV },
    { "adjust_AV_for_misc_segments", entry_adjust_AV_for_misc_segments },
    { "jap_phrase_level_rules", entry_jap_phrase_level_rules },
    { "high_vowel_devoicing", entry_high_vowel_devoicing },
    { "generate_diaphones", entry_generate_diaphones },
    { "jap_phrase_type", entry_jap_phrase_type },
    { "insert_acc_phrases", entry_insert_acc_phrases },
    { "insert_jap_spr_phone", entry_insert_jap_spr_phone },
    { "print_jap_SPR", entry_print_jap_SPR },
    { "create_syllables", entry_create_syllables },
    { "assign_syll_coda", entry_assign_syll_coda },
    { "assign_accent_val", entry_assign_accent_val },
    { "estimate_jap_nsylls", entry_estimate_jap_nsylls },
    { "jap_symbolic_rules", entry_jap_symbolic_rules },
    { "init_platform", entry_init_platform },
    { "set_phrase_defaults", entry_set_phrase_defaults },
};
#endif
