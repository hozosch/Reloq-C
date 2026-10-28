cmake_minimum_required(VERSION 3.16)
project(eloquence_c LANGUAGES C)

add_library(reloq_core STATIC src/reloq_core.c src/reloq_annotation.c
    src/reloq_german_stop_phase.c
    src/reloq_duration_span.c
    src/reloq_german_transition_duration.c
    src/reloq_parameter_stream.c src/reloq_utf8.c src/reloq_cp1252.c src/reloq_german_input.c
    src/reloq_phonetic.c
    src/reloq_frame_adapter.c src/reloq_session.c
    src/reloq_numeric_pipeline.c src/reloq_acoustic_curve.c
    src/reloq_german_spr.c src/reloq_german_annotation.c
    src/reloq_curve_pipeline.c src/reloq_curve_scheduler.c
    src/reloq_target_builder.c src/reloq_target_stage_merge.c
    src/reloq_target_stage_router.c
    src/reloq_german_phone_sequence.c
    src/reloq_german_syllable_plan.c
    src/reloq_german_within_word_f0.c
    src/reloq_german_within_word_landmarks.c
    src/reloq_german_within_word_tract.c
    src/reloq_german_within_word_source.c
    src/reloq_german_within_word_composer.c
    src/reloq_german_reduced_landmarks.c
    src/reloq_german_reduced_source.c
    src/reloq_german_reduced_tract.c
    src/reloq_german_reduced_open_first_tract.c
    src/reloq_german_reduced_open_source.c
    src/reloq_german_reduced_open_composer.c
    src/reloq_german_reduced_closed_first_tract.c
    src/reloq_german_reduced_closed_composer.c
    src/reloq_german_cross_word_landmarks.c
    src/reloq_german_cross_word_contour.c
    src/reloq_german_stop_vowel_interaction.c
    src/reloq_german_g2p.c src/reloq_german_compound_recovery.c src/reloq_lexicon.c src/reloq_german_cvc_lexicon.c
    src/reloq_german_lexicon.c src/reloq_german_lexicon_file.c src/reloq_german_text.c
    src/reloq_german_target_planner.c
    src/reloq_german_vowel_nucleus.c
    src/reloq_german_hv_timing.c
    src/reloq_german_initial_stop.c
    src/reloq_german_initial_voiced_stop.c
    src/reloq_german_initial_voiced_stop_envelope.c
    src/reloq_german_initial_nasal.c
    src/reloq_german_initial_nasal_envelope.c
    src/reloq_german_nasal_vowel.c
    src/reloq_german_initial_h.c
    src/reloq_german_terminal_source.c
    src/reloq_german_terminal_stop_tract.c
    src/reloq_german_voice_defaults.c
    src/reloq_german_phrase_f0.c
    src/reloq_german_stop_vowel.c
    src/reloq_german_stop_fricative_vowel.c
    src/reloq_german_voiced_stop_vowel.c
    src/reloq_german_voiced_stop_tract.c
    src/reloq_german_fricative_vowel.c
    src/reloq_german_terminal_fricative.c
    src/reloq_german_sentence_boundary.c
    src/reloq_german_voice.c
    src/reloq_german_utterance.c
    src/reloq_german_generic_voice.c
    src/reloq_german_generic_point_budget.c
    src/reloq_german_generic_noise.c
    src/reloq_german_generic_prosody.c
    src/reloq_german_generic_tract.c
    src/reloq_german_generic_nasal_coda.c
    src/reloq_german_generic_transition.c
    src/reloq_german_generic_melody.c
    src/reloq_german_generic_coarticulation.c
    src/reloq_german_generic_timing.c
    src/reloq_german_generic_index.c
    src/reloq_german_generic_diphthong.c
    src/reloq_german_generic_stop_connection.c
    src/reloq_voice_controls.c
    src/reloq_german_best_effort_text.c
    src/reloq_german_terminal_timing.c)
target_sources(reloq_core PRIVATE
    src/reloq_german_connected_pair.c
    src/reloq_german_connected_kp.c
    src/reloq_german_connected_pt.c
    src/reloq_german_boundary_profile.c
    src/reloq_german_connected_cvc_index.c
    src/reloq_german_connected_words.c)
target_sources(reloq_core PRIVATE
    src/reloq_noise_profile.c
    src/reloq_rate_transfer.c
    src/reloq_german_voice_model.c
    src/reloq_german_coda_timing.c
    src/reloq_english.c
    src/reloq_english_nucleus.c
    src/reloq_english_text.c
    src/reloq_english_voice.c
    src/reloq_german_affricate.c
    src/reloq_german_glottal.c
    src/reloq_german_liquid_tract.c
    src/reloq_german_morphology.c
    src/reloq_german_nasal_tract.c
    src/reloq_german_nucleus_acoustics.c
    src/reloq_german_nucleus_duration.c
    src/reloq_german_nucleus_prosody.c
    src/reloq_german_nucleus_schedule.c
    src/reloq_german_numbers.c
    src/reloq_german_obstruent_tract.c
    src/reloq_german_onset_cluster.c
    src/reloq_german_pause.c
    src/reloq_german_punctuation.c
    src/reloq_german_root_plan.c
    src/reloq_german_source_envelope.c
    src/reloq_german_word_class.c
    src/reloq_german_word_features.c
)
target_compile_features(reloq_core PUBLIC c_std_11)
target_include_directories(reloq_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(UNIX)
    target_link_libraries(reloq_core PUBLIC m)
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(reloq_core PRIVATE -ffp-contract=off)
elseif(MSVC)
    target_compile_options(reloq_core PRIVATE /fp:strict)
endif()

if(UNIX)
    add_executable(reloq_speak_wav tools/reloq_speak_wav.c)
    target_link_libraries(reloq_speak_wav PRIVATE reloq_core)
    add_executable(reloq_speak_generic_wav tools/reloq_speak_generic_wav.c)
    target_link_libraries(reloq_speak_generic_wav PRIVATE reloq_core)
endif()

option(RELOQ_BUILD_HOST "Build the numeric PCM host library" OFF)
option(RELOQ_BUILD_ECI "Build the direct native ECI library" OFF)
if(RELOQ_BUILD_HOST OR RELOQ_BUILD_ECI)
    set_target_properties(reloq_core PROPERTIES POSITION_INDEPENDENT_CODE ON
        C_VISIBILITY_PRESET hidden)
endif()
if(RELOQ_BUILD_HOST)
    add_library(reloq_host SHARED src/reloq_host.c)
    target_compile_features(reloq_host PRIVATE c_std_11)
    target_include_directories(reloq_host PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(reloq_host PRIVATE reloq_core)
    set_target_properties(reloq_host PROPERTIES C_VISIBILITY_PRESET hidden)
endif()
if(RELOQ_BUILD_ECI)
    add_library(reloq_eci SHARED src/reloq_eci.c)
    target_compile_features(reloq_eci PRIVATE c_std_11)
    target_include_directories(reloq_eci PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(reloq_eci PRIVATE reloq_core)
    set_target_properties(reloq_eci PROPERTIES C_VISIBILITY_PRESET hidden)
    if(WIN32)
        set_target_properties(reloq_eci PROPERTIES OUTPUT_NAME eci PREFIX "")
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(reloq_german_transition_duration_test tests/test_reloq_german_transition_duration.c)
    target_link_libraries(reloq_german_transition_duration_test PRIVATE reloq_core)
    add_test(NAME reloq_german_transition_duration COMMAND reloq_german_transition_duration_test)
    add_executable(reloq_duration_span_test tests/test_reloq_duration_span.c)
    target_link_libraries(reloq_duration_span_test PRIVATE reloq_core)
    add_test(NAME reloq_duration_span COMMAND reloq_duration_span_test)
    add_executable(reloq_core_test tests/test_reloq_core.c)
    target_link_libraries(reloq_core_test PRIVATE reloq_core)
    add_test(NAME reloq_core COMMAND reloq_core_test)
    add_executable(reloq_annotation_test tests/test_reloq_annotation.c)
    target_link_libraries(reloq_annotation_test PRIVATE reloq_core)
    add_test(NAME reloq_annotation COMMAND reloq_annotation_test)
    add_executable(reloq_parameter_stream_test tests/test_reloq_parameter_stream.c)
    target_link_libraries(reloq_parameter_stream_test PRIVATE reloq_core)
    add_test(NAME reloq_parameter_stream COMMAND reloq_parameter_stream_test)
    add_executable(reloq_utf8_test tests/test_reloq_utf8.c)
    target_link_libraries(reloq_utf8_test PRIVATE reloq_core)
    add_test(NAME reloq_utf8 COMMAND reloq_utf8_test)
    add_executable(reloq_cp1252_test tests/test_reloq_cp1252.c)
    target_link_libraries(reloq_cp1252_test PRIVATE reloq_core)
    add_test(NAME reloq_cp1252 COMMAND reloq_cp1252_test)
    add_executable(reloq_german_input_test tests/test_reloq_german_input.c)
    target_link_libraries(reloq_german_input_test PRIVATE reloq_core)
    add_test(NAME reloq_german_input COMMAND reloq_german_input_test)
    add_executable(reloq_phonetic_test tests/test_reloq_phonetic.c)
    target_link_libraries(reloq_phonetic_test PRIVATE reloq_core)
    add_test(NAME reloq_phonetic COMMAND reloq_phonetic_test)
    add_executable(reloq_frame_adapter_test tests/test_reloq_frame_adapter.c)
    target_link_libraries(reloq_frame_adapter_test PRIVATE reloq_core)
    add_test(NAME reloq_frame_adapter COMMAND reloq_frame_adapter_test)
    add_executable(reloq_session_test tests/test_reloq_session.c)
    target_link_libraries(reloq_session_test PRIVATE reloq_core)
    add_test(NAME reloq_session COMMAND reloq_session_test)
    add_executable(reloq_numeric_pipeline_test tests/test_reloq_numeric_pipeline.c)
    target_link_libraries(reloq_numeric_pipeline_test PRIVATE reloq_core)
    add_test(NAME reloq_numeric_pipeline COMMAND reloq_numeric_pipeline_test)
    add_executable(reloq_acoustic_curve_test tests/test_reloq_acoustic_curve.c)
    target_link_libraries(reloq_acoustic_curve_test PRIVATE reloq_core)
    add_test(NAME reloq_acoustic_curve COMMAND reloq_acoustic_curve_test)
    add_executable(reloq_german_spr_test tests/test_reloq_german_spr.c)
    target_link_libraries(reloq_german_spr_test PRIVATE reloq_core)
    add_test(NAME reloq_german_spr COMMAND reloq_german_spr_test)
    add_executable(reloq_german_annotation_test tests/test_reloq_german_annotation.c)
    target_link_libraries(reloq_german_annotation_test PRIVATE reloq_core)
    add_test(NAME reloq_german_annotation COMMAND reloq_german_annotation_test)
    add_executable(reloq_curve_pipeline_test tests/test_reloq_curve_pipeline.c)
    target_link_libraries(reloq_curve_pipeline_test PRIVATE reloq_core)
    add_test(NAME reloq_curve_pipeline COMMAND reloq_curve_pipeline_test)
    add_executable(reloq_curve_scheduler_test tests/test_reloq_curve_scheduler.c)
    target_link_libraries(reloq_curve_scheduler_test PRIVATE reloq_core)
    add_test(NAME reloq_curve_scheduler COMMAND reloq_curve_scheduler_test)
    add_executable(reloq_target_builder_test tests/test_reloq_target_builder.c)
    target_link_libraries(reloq_target_builder_test PRIVATE reloq_core)
    add_test(NAME reloq_target_builder COMMAND reloq_target_builder_test)
    add_executable(reloq_target_stage_merge_test tests/test_reloq_target_stage_merge.c)
    target_link_libraries(reloq_target_stage_merge_test PRIVATE reloq_core)
    add_test(NAME reloq_target_stage_merge COMMAND reloq_target_stage_merge_test)
    add_executable(reloq_target_stage_router_test tests/test_reloq_target_stage_router.c)
    target_link_libraries(reloq_target_stage_router_test PRIVATE reloq_core)
    add_test(NAME reloq_target_stage_router COMMAND reloq_target_stage_router_test)
    add_executable(reloq_german_phone_sequence_test tests/test_reloq_german_phone_sequence.c)
    target_link_libraries(reloq_german_phone_sequence_test PRIVATE reloq_core)
    add_test(NAME reloq_german_phone_sequence COMMAND reloq_german_phone_sequence_test)
    add_executable(reloq_german_syllable_plan_test tests/test_reloq_german_syllable_plan.c)
    target_link_libraries(reloq_german_syllable_plan_test PRIVATE reloq_core)
    add_test(NAME reloq_german_syllable_plan COMMAND reloq_german_syllable_plan_test)
    add_executable(reloq_german_within_word_f0_test tests/test_reloq_german_within_word_f0.c)
    target_link_libraries(reloq_german_within_word_f0_test PRIVATE reloq_core)
    add_test(NAME reloq_german_within_word_f0 COMMAND reloq_german_within_word_f0_test)
    add_executable(reloq_german_within_word_landmarks_test tests/test_reloq_german_within_word_landmarks.c)
    target_link_libraries(reloq_german_within_word_landmarks_test PRIVATE reloq_core)
    add_test(NAME reloq_german_within_word_landmarks COMMAND reloq_german_within_word_landmarks_test)
    add_executable(reloq_german_within_word_tract_test tests/test_reloq_german_within_word_tract.c)
    target_link_libraries(reloq_german_within_word_tract_test PRIVATE reloq_core)
    add_test(NAME reloq_german_within_word_tract COMMAND reloq_german_within_word_tract_test)
    add_executable(reloq_german_within_word_source_test tests/test_reloq_german_within_word_source.c)
    target_link_libraries(reloq_german_within_word_source_test PRIVATE reloq_core)
    add_test(NAME reloq_german_within_word_source COMMAND reloq_german_within_word_source_test)
    add_executable(reloq_german_within_word_composer_test tests/test_reloq_german_within_word_composer.c)
    target_link_libraries(reloq_german_within_word_composer_test PRIVATE reloq_core)
    add_test(NAME reloq_german_within_word_composer COMMAND reloq_german_within_word_composer_test)
    add_executable(reloq_german_generic_voice_test tests/test_reloq_german_generic_voice.c)
    target_link_libraries(reloq_german_generic_voice_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_voice COMMAND reloq_german_generic_voice_test)
    add_executable(reloq_german_generic_point_budget_test tests/test_reloq_german_generic_point_budget.c)
    target_link_libraries(reloq_german_generic_point_budget_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_point_budget COMMAND reloq_german_generic_point_budget_test)
    add_executable(reloq_german_generic_oq_test tests/test_reloq_german_generic_oq.c)
    target_link_libraries(reloq_german_generic_oq_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_oq COMMAND reloq_german_generic_oq_test)
    add_executable(reloq_german_generic_tract_test tests/test_reloq_german_generic_tract.c)
    target_link_libraries(reloq_german_generic_tract_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_tract COMMAND reloq_german_generic_tract_test)
    add_executable(reloq_german_generic_nasal_coda_test tests/test_reloq_german_generic_nasal_coda.c)
    target_link_libraries(reloq_german_generic_nasal_coda_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_nasal_coda COMMAND reloq_german_generic_nasal_coda_test)
    add_executable(reloq_german_generic_prosody_test tests/test_reloq_german_generic_prosody.c)
    target_link_libraries(reloq_german_generic_prosody_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_prosody COMMAND reloq_german_generic_prosody_test)
    add_executable(reloq_german_generic_noise_test tests/test_reloq_german_generic_noise.c)
    target_link_libraries(reloq_german_generic_noise_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_noise COMMAND reloq_german_generic_noise_test)
    add_executable(reloq_voice_controls_test tests/test_reloq_voice_controls.c)
    target_link_libraries(reloq_voice_controls_test PRIVATE reloq_core)
    add_test(NAME reloq_voice_controls COMMAND reloq_voice_controls_test)
    add_executable(reloq_german_generic_transition_test tests/test_reloq_german_generic_transition.c)
    target_link_libraries(reloq_german_generic_transition_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_transition COMMAND reloq_german_generic_transition_test)
    add_executable(reloq_german_generic_melody_test tests/test_reloq_german_generic_melody.c)
    target_link_libraries(reloq_german_generic_melody_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_melody COMMAND reloq_german_generic_melody_test)
    add_executable(reloq_german_generic_coarticulation_test tests/test_reloq_german_generic_coarticulation.c)
    target_link_libraries(reloq_german_generic_coarticulation_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_coarticulation COMMAND reloq_german_generic_coarticulation_test)
    add_executable(reloq_german_generic_timing_test tests/test_reloq_german_generic_timing.c)
    target_link_libraries(reloq_german_generic_timing_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_timing COMMAND reloq_german_generic_timing_test)
    add_executable(reloq_german_generic_index_test tests/test_reloq_german_generic_index.c)
    target_link_libraries(reloq_german_generic_index_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_index COMMAND reloq_german_generic_index_test)
    add_executable(reloq_german_generic_diphthong_test tests/test_reloq_german_generic_diphthong.c)
    target_link_libraries(reloq_german_generic_diphthong_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_diphthong COMMAND reloq_german_generic_diphthong_test)
    add_executable(reloq_german_generic_stop_connection_test tests/test_reloq_german_generic_stop_connection.c)
    target_link_libraries(reloq_german_generic_stop_connection_test PRIVATE reloq_core)
    add_test(NAME reloq_german_generic_stop_connection COMMAND reloq_german_generic_stop_connection_test)
    add_executable(reloq_german_best_effort_text_test tests/test_reloq_german_best_effort_text.c)
    target_link_libraries(reloq_german_best_effort_text_test PRIVATE reloq_core)
    add_test(NAME reloq_german_best_effort_text COMMAND reloq_german_best_effort_text_test)
    add_executable(reloq_german_reduced_landmarks_test tests/test_reloq_german_reduced_landmarks.c)
    target_link_libraries(reloq_german_reduced_landmarks_test PRIVATE reloq_core)
    add_test(NAME reloq_german_reduced_landmarks COMMAND reloq_german_reduced_landmarks_test)
    add_executable(reloq_german_reduced_source_test tests/test_reloq_german_reduced_source.c)
    target_link_libraries(reloq_german_reduced_source_test PRIVATE reloq_core)
    add_test(NAME reloq_german_reduced_source COMMAND reloq_german_reduced_source_test)
    add_executable(reloq_german_reduced_tract_test tests/test_reloq_german_reduced_tract.c)
    target_link_libraries(reloq_german_reduced_tract_test PRIVATE reloq_core)
    add_test(NAME reloq_german_reduced_tract COMMAND reloq_german_reduced_tract_test)
    add_executable(reloq_german_reduced_open_first_tract_test tests/test_reloq_german_reduced_open_first_tract.c)
    target_link_libraries(reloq_german_reduced_open_first_tract_test PRIVATE reloq_core)
    add_test(NAME reloq_german_reduced_open_first_tract COMMAND reloq_german_reduced_open_first_tract_test)
    add_executable(reloq_german_reduced_open_source_test tests/test_reloq_german_reduced_open_source.c)
    target_link_libraries(reloq_german_reduced_open_source_test PRIVATE reloq_core)
    add_test(NAME reloq_german_reduced_open_source COMMAND reloq_german_reduced_open_source_test)
    add_executable(reloq_german_reduced_open_composer_test tests/test_reloq_german_reduced_open_composer.c)
    target_link_libraries(reloq_german_reduced_open_composer_test PRIVATE reloq_core)
    add_test(NAME reloq_german_reduced_open_composer COMMAND reloq_german_reduced_open_composer_test)
    add_executable(reloq_german_reduced_closed_first_tract_test tests/test_reloq_german_reduced_closed_first_tract.c)
    target_link_libraries(reloq_german_reduced_closed_first_tract_test PRIVATE reloq_core)
    add_test(NAME reloq_german_reduced_closed_first_tract COMMAND reloq_german_reduced_closed_first_tract_test)
    add_executable(reloq_german_reduced_closed_composer_test tests/test_reloq_german_reduced_closed_composer.c)
    target_link_libraries(reloq_german_reduced_closed_composer_test PRIVATE reloq_core)
    add_test(NAME reloq_german_reduced_closed_composer COMMAND reloq_german_reduced_closed_composer_test)
    add_executable(reloq_german_cross_word_landmarks_test tests/test_reloq_german_cross_word_landmarks.c)
    target_link_libraries(reloq_german_cross_word_landmarks_test PRIVATE reloq_core)
    add_test(NAME reloq_german_cross_word_landmarks COMMAND reloq_german_cross_word_landmarks_test)
    add_executable(reloq_german_cross_word_contour_test tests/test_reloq_german_cross_word_contour.c)
    target_link_libraries(reloq_german_cross_word_contour_test PRIVATE reloq_core)
    add_test(NAME reloq_german_cross_word_contour COMMAND reloq_german_cross_word_contour_test)
    add_executable(reloq_german_stop_vowel_interaction_test tests/test_reloq_german_stop_vowel_interaction.c)
    target_link_libraries(reloq_german_stop_vowel_interaction_test PRIVATE reloq_core)
    add_test(NAME reloq_german_stop_vowel_interaction COMMAND reloq_german_stop_vowel_interaction_test)
    add_executable(reloq_german_g2p_test tests/test_reloq_german_g2p.c)
    target_link_libraries(reloq_german_g2p_test PRIVATE reloq_core)
    add_test(NAME reloq_german_g2p COMMAND reloq_german_g2p_test)
    add_executable(reloq_german_compound_recovery_test tests/test_reloq_german_compound_recovery.c)
    target_link_libraries(reloq_german_compound_recovery_test PRIVATE reloq_core)
    add_test(NAME reloq_german_compound_recovery COMMAND reloq_german_compound_recovery_test)
    add_executable(reloq_lexicon_test tests/test_reloq_lexicon.c)
    target_link_libraries(reloq_lexicon_test PRIVATE reloq_core)
    add_test(NAME reloq_lexicon COMMAND reloq_lexicon_test)
    add_executable(reloq_german_lexicon_file_test tests/test_reloq_german_lexicon_file.c)
    target_link_libraries(reloq_german_lexicon_file_test PRIVATE reloq_core)
    add_test(NAME reloq_german_lexicon_file COMMAND reloq_german_lexicon_file_test)
    set_tests_properties(reloq_german_lexicon_file PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_executable(reloq_german_cvc_lexicon_test tests/test_reloq_german_cvc_lexicon.c)
    target_link_libraries(reloq_german_cvc_lexicon_test PRIVATE reloq_core)
    add_test(NAME reloq_german_cvc_lexicon COMMAND reloq_german_cvc_lexicon_test)
    add_executable(reloq_german_text_test tests/test_reloq_german_text.c)
    target_link_libraries(reloq_german_text_test PRIVATE reloq_core)
    add_test(NAME reloq_german_text COMMAND reloq_german_text_test)
    add_executable(reloq_german_target_planner_test tests/test_reloq_german_target_planner.c)
    target_link_libraries(reloq_german_target_planner_test PRIVATE reloq_core)
    add_test(NAME reloq_german_target_planner COMMAND reloq_german_target_planner_test)
    add_executable(reloq_german_vowel_nucleus_test tests/test_reloq_german_vowel_nucleus.c)
    target_link_libraries(reloq_german_vowel_nucleus_test PRIVATE reloq_core)
    add_test(NAME reloq_german_vowel_nucleus COMMAND reloq_german_vowel_nucleus_test)
    add_executable(reloq_german_hv_timing_test tests/test_reloq_german_hv_timing.c)
    target_link_libraries(reloq_german_hv_timing_test PRIVATE reloq_core)
    add_test(NAME reloq_german_hv_timing COMMAND reloq_german_hv_timing_test)
    add_executable(reloq_german_initial_stop_test tests/test_reloq_german_initial_stop.c)
    target_link_libraries(reloq_german_initial_stop_test PRIVATE reloq_core)
    add_test(NAME reloq_german_initial_stop COMMAND reloq_german_initial_stop_test)
    add_executable(reloq_german_initial_voiced_stop_test tests/test_reloq_german_initial_voiced_stop.c)
    target_link_libraries(reloq_german_initial_voiced_stop_test PRIVATE reloq_core)
    add_test(NAME reloq_german_initial_voiced_stop COMMAND reloq_german_initial_voiced_stop_test)
    add_executable(reloq_german_initial_voiced_stop_envelope_test tests/test_reloq_german_initial_voiced_stop_envelope.c)
    target_link_libraries(reloq_german_initial_voiced_stop_envelope_test PRIVATE reloq_core)
    add_test(NAME reloq_german_initial_voiced_stop_envelope COMMAND reloq_german_initial_voiced_stop_envelope_test)
    add_executable(reloq_german_initial_nasal_test tests/test_reloq_german_initial_nasal.c)
    target_link_libraries(reloq_german_initial_nasal_test PRIVATE reloq_core)
    add_test(NAME reloq_german_initial_nasal COMMAND reloq_german_initial_nasal_test)
    add_executable(reloq_german_initial_nasal_envelope_test tests/test_reloq_german_initial_nasal_envelope.c)
    target_link_libraries(reloq_german_initial_nasal_envelope_test PRIVATE reloq_core)
    add_test(NAME reloq_german_initial_nasal_envelope COMMAND reloq_german_initial_nasal_envelope_test)
    add_executable(reloq_german_nasal_vowel_test tests/test_reloq_german_nasal_vowel.c)
    target_link_libraries(reloq_german_nasal_vowel_test PRIVATE reloq_core)
    add_test(NAME reloq_german_nasal_vowel COMMAND reloq_german_nasal_vowel_test)
    add_executable(reloq_german_initial_h_test tests/test_reloq_german_initial_h.c)
    target_link_libraries(reloq_german_initial_h_test PRIVATE reloq_core)
    add_test(NAME reloq_german_initial_h COMMAND reloq_german_initial_h_test)
    add_executable(reloq_german_terminal_source_test tests/test_reloq_german_terminal_source.c)
    target_link_libraries(reloq_german_terminal_source_test PRIVATE reloq_core)
    add_test(NAME reloq_german_terminal_source COMMAND reloq_german_terminal_source_test)
    add_executable(reloq_german_terminal_stop_tract_test tests/test_reloq_german_terminal_stop_tract.c)
    target_link_libraries(reloq_german_terminal_stop_tract_test PRIVATE reloq_core)
    add_test(NAME reloq_german_terminal_stop_tract COMMAND reloq_german_terminal_stop_tract_test)
    add_executable(reloq_german_voice_defaults_test tests/test_reloq_german_voice_defaults.c)
    target_link_libraries(reloq_german_voice_defaults_test PRIVATE reloq_core)
    add_test(NAME reloq_german_voice_defaults COMMAND reloq_german_voice_defaults_test)
    add_executable(reloq_german_phrase_f0_test tests/test_reloq_german_phrase_f0.c)
    target_link_libraries(reloq_german_phrase_f0_test PRIVATE reloq_core)
    add_test(NAME reloq_german_phrase_f0 COMMAND reloq_german_phrase_f0_test)
    add_executable(reloq_german_stop_vowel_test tests/test_reloq_german_stop_vowel.c)
    target_link_libraries(reloq_german_stop_vowel_test PRIVATE reloq_core)
    add_test(NAME reloq_german_stop_vowel COMMAND reloq_german_stop_vowel_test)
    add_executable(reloq_german_stop_fricative_vowel_test tests/test_reloq_german_stop_fricative_vowel.c)
    target_link_libraries(reloq_german_stop_fricative_vowel_test PRIVATE reloq_core)
    add_test(NAME reloq_german_stop_fricative_vowel COMMAND reloq_german_stop_fricative_vowel_test)
    add_executable(reloq_german_voiced_stop_vowel_test tests/test_reloq_german_voiced_stop_vowel.c)
    target_link_libraries(reloq_german_voiced_stop_vowel_test PRIVATE reloq_core)
    add_test(NAME reloq_german_voiced_stop_vowel COMMAND reloq_german_voiced_stop_vowel_test)
    add_executable(reloq_german_voiced_stop_tract_test tests/test_reloq_german_voiced_stop_tract.c)
    target_link_libraries(reloq_german_voiced_stop_tract_test PRIVATE reloq_core)
    add_test(NAME reloq_german_voiced_stop_tract COMMAND reloq_german_voiced_stop_tract_test)
    add_executable(reloq_german_fricative_vowel_test tests/test_reloq_german_fricative_vowel.c)
    target_link_libraries(reloq_german_fricative_vowel_test PRIVATE reloq_core)
    add_test(NAME reloq_german_fricative_vowel COMMAND reloq_german_fricative_vowel_test)
    add_executable(reloq_german_terminal_fricative_test tests/test_reloq_german_terminal_fricative.c)
    target_link_libraries(reloq_german_terminal_fricative_test PRIVATE reloq_core)
    add_test(NAME reloq_german_terminal_fricative COMMAND reloq_german_terminal_fricative_test)
    add_executable(reloq_german_sentence_boundary_test tests/test_reloq_german_sentence_boundary.c)
    target_link_libraries(reloq_german_sentence_boundary_test PRIVATE reloq_core)
    add_test(NAME reloq_german_sentence_boundary COMMAND reloq_german_sentence_boundary_test)
    add_executable(reloq_german_voice_test tests/test_reloq_german_voice.c)
    target_link_libraries(reloq_german_voice_test PRIVATE reloq_core)
    add_test(NAME reloq_german_voice COMMAND reloq_german_voice_test)
    add_executable(reloq_german_connected_pair_test tests/test_reloq_german_connected_pair.c)
    target_link_libraries(reloq_german_connected_pair_test PRIVATE reloq_core)
    add_test(NAME reloq_german_connected_pair COMMAND reloq_german_connected_pair_test)
    add_executable(reloq_german_connected_pt_test tests/test_reloq_german_connected_pt.c)
    target_link_libraries(reloq_german_connected_pt_test PRIVATE reloq_core)
    add_test(NAME reloq_german_connected_pt COMMAND reloq_german_connected_pt_test)
    add_executable(reloq_german_connected_words_test tests/test_reloq_german_connected_words.c)
    target_link_libraries(reloq_german_connected_words_test PRIVATE reloq_core)
    add_test(NAME reloq_german_connected_words COMMAND reloq_german_connected_words_test)
    add_executable(reloq_german_hvc_composition_test tests/test_reloq_german_hvc_composition.c)
    target_link_libraries(reloq_german_hvc_composition_test PRIVATE reloq_core)
    add_test(NAME reloq_german_hvc_composition COMMAND reloq_german_hvc_composition_test --self-test)
    add_executable(reloq_german_utterance_test tests/test_reloq_german_utterance.c)
    target_link_libraries(reloq_german_utterance_test PRIVATE reloq_core)
    add_test(NAME reloq_german_utterance COMMAND reloq_german_utterance_test)
    add_executable(reloq_german_terminal_timing_test tests/test_reloq_german_terminal_timing.c)
    target_link_libraries(reloq_german_terminal_timing_test PRIVATE reloq_core)
    add_test(NAME reloq_german_terminal_timing COMMAND reloq_german_terminal_timing_test)
    if(RELOQ_BUILD_HOST)
        add_executable(reloq_host_test host/test_reloq_host.c)
        target_link_libraries(reloq_host_test PRIVATE reloq_host)
        add_test(NAME reloq_host COMMAND reloq_host_test)
    endif()
    if(RELOQ_BUILD_ECI)
        # The controlled PCM fixture is intentionally not part of the
        # exported ECI ABI, so this test links the facade source directly.
        add_executable(reloq_eci_test tests/test_reloq_eci.c src/reloq_eci.c)
        target_compile_definitions(reloq_eci_test PRIVATE RELOQ_ECI_STATIC)
        target_link_libraries(reloq_eci_test PRIVATE reloq_core)
        add_test(NAME reloq_eci COMMAND reloq_eci_test)
    endif()
endif()
