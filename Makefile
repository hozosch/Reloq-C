.DELETE_ON_ERROR:
.DEFAULT_GOAL := all

test_reloq_owned_render: tests/test_reloq_owned_render.c src/reloq_core_internal.h libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude $< libreloq_core.a -lm -o $@

test: test-owned-render
.PHONY: test-owned-render
test-owned-render: test_reloq_owned_render
	./test_reloq_owned_render

CC ?= cc
AR ?= ar
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror -pedantic
CPPFLAGS ?=
CFLAGS += -pthread
RULES61 := vendor/openevv61/build/native/libreloq61.a
RULES61_SOURCES := $(wildcard vendor/openevv61/src/*.c vendor/openevv61/src/*/*.c vendor/openevv61/src/*/*/*.c vendor/openevv61/src/*/*.h vendor/openevv61/src/*/*/*.h vendor/openevv61/include/*.h vendor/openevv61/lang/*/*.c vendor/openevv61/lang/*/*.h) vendor/openevv61/Makefile.native
RULES61_SOURCES += src/reloq_voice_filter.c include/reloq_voice_filter.h
RULES61_SOURCES += src/reloq_orthography.c include/reloq_orthography.h
RULES61_SOURCES += src/reloq_german_word_edges.c include/reloq_german_word_edges.h
RULES61_SOURCES += vendor/openevv61/src/reloq61_german_alphabet.inc
RULES61_SOURCES += src/reloq_english_word_edges.c include/reloq_english_word_edges.h
RULES61_SOURCES += vendor/openevv61/src/reloq61_english_alphabet.inc
RULES61_SOURCES += vendor/openevv61/src/reloq61_text_case.inc
RULES61_SOURCES += vendor/openevv61/lang/kokr61/foreign_words_lexicon_kokr61.inc
$(RULES61): $(RULES61_SOURCES)
	$(MAKE) -C vendor/openevv61 -f Makefile.native

# Keep transitive C header dependencies current. Handwritten rules list the
# immediate public header only; a nested phone-structure change otherwise
# leaves stale objects with incompatible field offsets in the static archive.
CORE_OBJECTS := $(patsubst src/%.c,%.o,$(filter-out \
    src/reloq_eci.c src/reloq_host.c,$(wildcard src/*.c)))
CORE_DEPFILES := $(CORE_OBJECTS:.o=.d)
$(CORE_OBJECTS): private CFLAGS += -MMD -MP
-include $(CORE_DEPFILES)


test_reloq_german_morphology: tests/test_reloq_german_morphology.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_morphology

test_reloq_german_feature_roundtrip: tests/test_reloq_german_feature_roundtrip.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_feature_roundtrip

test_reloq_german_coda_timing: tests/test_reloq_german_coda_timing.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_coda_timing

test_reloq_german_source_handoff: tests/test_reloq_german_source_handoff.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_source_handoff

test_reloq_german_source_envelope: tests/test_reloq_german_source_envelope.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_source_envelope

.PHONY: all test host-test eci-test eci-windows-x64 clean
all: libreloq_core.a

libreloq_core.a: reloq_english.o reloq_english_spr.o reloq_english_nucleus.o reloq_english_nucleus_duration.o reloq_english_source.o reloq_english_connected.o reloq_english_intonation.o reloq_english_text.o reloq_english_voice.o
libreloq_core.a: reloq_character.o
libreloq_core.a: reloq_english_tract.o
libreloq_core.a: reloq_english_transition.o
reloq_english_transition.o: src/reloq_english_transition.c include/reloq_english_transition.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
reloq_english_tract.o: src/reloq_english_tract.c include/reloq_english_tract.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
test_reloq_english_tract: tests/test_reloq_english_tract.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
.PHONY: test-english-tract
test: test-english-tract
test-english-tract: test_reloq_english_tract
	./test_reloq_english_tract
reloq_character.o: src/reloq_character.c include/reloq_character.h src/reloq_character_data.inc src/reloq_character_native.inc
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
test_reloq_character: tests/test_reloq_character.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
.PHONY: test-character
test: test-character
test-character: test_reloq_character
	./test_reloq_character
reloq_english_intonation.o: src/reloq_english_intonation.c include/reloq_english_intonation.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@
reloq_english_connected.o: src/reloq_english_connected.c include/reloq_english_connected.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
reloq_english_source.o: src/reloq_english_source.c include/reloq_english_source.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@
reloq_english_nucleus_duration.o: src/reloq_english_nucleus_duration.c include/reloq_english_nucleus_duration.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@
reloq_english_text.o: src/reloq_english_text.c include/reloq_english_voice.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
reloq_english_voice.o: src/reloq_english_voice.c include/reloq_english_voice.h src/reloq_english_gestures.inc
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@
reloq_english.o: src/reloq_english.c include/reloq_english.h src/reloq_english_lexicon.inc
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
reloq_english_spr.o: src/reloq_english_spr.c include/reloq_english.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
reloq_english_nucleus.o: src/reloq_english_nucleus.c include/reloq_english.h src/reloq_english_nuclei.inc src/reloq_english_coda_targets.inc
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@
test_reloq_english: tests/test_reloq_english.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
test: test-english
.PHONY: test-english
test-english: test_reloq_english
	./test_reloq_english

test_reloq_english_nucleus_duration: tests/test_reloq_english_nucleus_duration.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
.PHONY: test-english-nucleus-duration
test: test-english-nucleus-duration
test-english-nucleus-duration: test_reloq_english_nucleus_duration
	./test_reloq_english_nucleus_duration

test_reloq_english_source: tests/test_reloq_english_source.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
test: test-english-source
.PHONY: test-english-source
test-english-source: test_reloq_english_source
	./test_reloq_english_source

test_reloq_english_connected: tests/test_reloq_english_connected.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
test_reloq_english_intonation: tests/test_reloq_english_intonation.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
.PHONY: test-english-connected test-english-intonation
test: test-english-connected test-english-intonation
test-english-connected: test_reloq_english_connected
	./test_reloq_english_connected
test-english-intonation: test_reloq_english_intonation
	./test_reloq_english_intonation

libreloq_core.a: reloq_duration_span.o reloq_german_transition_duration.o
libreloq_core.a: reloq_german_nucleus_acoustics.o
libreloq_core.a: reloq_german_nucleus_duration.o
libreloq_core.a: reloq_german_nucleus_prosody.o
libreloq_core.a: reloq_german_nucleus_schedule.o
libreloq_core.a: reloq_german_morphology.o
reloq_german_morphology.o: src/reloq_german_ung_roles.inc src/reloq_german_el_roles.inc
reloq_german_morphology.o: src/reloq_german_en_roles.inc src/reloq_german_en_head_roles.inc src/reloq_german_weak_ending_roles.inc
reloq_german_morphology.o: src/reloq_german_contextual_stems.inc
libreloq_core.a: reloq_german_source_envelope.o
libreloq_core.a: reloq_german_pause.o
libreloq_core.a: reloq_german_word_class.o
libreloq_core.a: reloq_german_nasal_tract.o
libreloq_core.a: reloq_german_obstruent_tract.o

libreloq_core.a: reloq_german_liquid_tract.o reloq_german_onset_cluster.o
libreloq_core.a: reloq_german_stop_phase.o
libreloq_core.a: reloq_german_coda_timing.o

reloq_german_coda_timing.o: src/reloq_german_coda_timing.c include/reloq_german_coda_timing.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_stop_phase.o: src/reloq_german_stop_phase.c src/reloq_german_stop_edges.inc include/reloq_german_stop_phase.h include/reloq_german_onset_cluster.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
libreloq_core.a: reloq_german_affricate.o

reloq_german_affricate.o: src/reloq_german_affricate.c include/reloq_german_affricate.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

test_reloq_german_affricate: tests/test_reloq_german_affricate.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_affricate

reloq_german_liquid_tract.o: src/reloq_german_liquid_tract.c include/reloq_german_liquid_tract.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_obstruent_tract.o: src/reloq_german_obstruent_tract.c include/reloq_german_obstruent_tract.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_nasal_tract.o: src/reloq_german_nasal_tract.c include/reloq_german_nasal_tract.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_word_class.o: src/reloq_german_word_class.c include/reloq_german_word_class.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_pause.o: src/reloq_german_pause.c include/reloq_german_pause.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_source_envelope.o: src/reloq_german_source_envelope.c include/reloq_german_source_envelope.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_morphology.o: src/reloq_german_morphology.c src/reloq_german_root_roles.inc src/reloq_german_weak_verb_roles.inc src/reloq_german_inflection_roles.inc include/reloq_german_morphology.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_nucleus_schedule.o: src/reloq_german_nucleus_schedule.c include/reloq_german_nucleus_schedule.h include/reloq_german_stop_phase.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

test_reloq_german_nucleus_schedule: tests/test_reloq_german_nucleus_schedule.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_nucleus_schedule

reloq_german_nucleus_prosody.o: src/reloq_german_nucleus_prosody.c include/reloq_german_nucleus_prosody.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

test_reloq_german_nucleus_prosody: tests/test_reloq_german_nucleus_prosody.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_nucleus_prosody

reloq_german_nucleus_duration.o: src/reloq_german_nucleus_duration.c include/reloq_german_nucleus_duration.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

test_reloq_german_nucleus_duration: tests/test_reloq_german_nucleus_duration.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_nucleus_duration

reloq_german_nucleus_acoustics.o: src/reloq_german_nucleus_acoustics.c include/reloq_german_nucleus_acoustics.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

test_reloq_german_nucleus_acoustics: tests/test_reloq_german_nucleus_acoustics.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

reloq_duration_span.o: src/reloq_duration_span.c include/reloq_duration_span.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

test_reloq_duration_span: tests/test_reloq_duration_span.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

reloq_german_transition_duration.o: src/reloq_german_transition_duration.c include/reloq_german_transition_duration.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

test_reloq_german_transition_duration: tests/test_reloq_german_transition_duration.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_duration_span test_reloq_german_transition_duration

host-test:
	$(MAKE) -C host test

eci-test:
	$(MAKE) -C eci test

eci-windows-x64:
	$(MAKE) -C eci windows-x64

reloq_core.o: src/reloq_core.c src/reloq_glottal_profiles.inc include/reloq_core.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

reloq_annotation.o: src/reloq_annotation.c include/reloq_annotation.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_parameter_stream.o: src/reloq_parameter_stream.c include/reloq_parameter_stream.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

reloq_utf8.o: src/reloq_utf8.c include/reloq_utf8.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_cp1252.o: src/reloq_cp1252.c include/reloq_cp1252.h include/reloq_utf8.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_input.o: src/reloq_german_input.c include/reloq_german_input.h include/reloq_utf8.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_phonetic.o: src/reloq_phonetic.c include/reloq_phonetic.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_frame_adapter.o: src/reloq_frame_adapter.c include/reloq_frame_adapter.h include/reloq_core.h include/reloq_parameter_stream.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_session.o: src/reloq_session.c include/reloq_session.h include/reloq_core.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

reloq_numeric_pipeline.o: src/reloq_numeric_pipeline.c include/reloq_numeric_pipeline.h include/reloq_parameter_stream.h include/reloq_frame_adapter.h include/reloq_session.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_acoustic_curve.o: src/reloq_acoustic_curve.c include/reloq_acoustic_curve.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

reloq_german_spr.o: src/reloq_german_spr.c include/reloq_german_spr.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_annotation.o: src/reloq_german_annotation.c include/reloq_german_annotation.h include/reloq_german_spr.h include/reloq_phonetic.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_curve_pipeline.o: src/reloq_curve_pipeline.c include/reloq_curve_pipeline.h include/reloq_acoustic_curve.h include/reloq_frame_adapter.h include/reloq_session.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

reloq_curve_scheduler.o: src/reloq_curve_scheduler.c include/reloq_curve_scheduler.h include/reloq_curve_pipeline.h include/reloq_acoustic_curve.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_target_builder.o: src/reloq_target_builder.c include/reloq_target_builder.h include/reloq_acoustic_curve.h include/reloq_parameter_stream.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_target_stage_merge.o: src/reloq_target_stage_merge.c include/reloq_target_stage_merge.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_target_stage_router.o: src/reloq_target_stage_router.c include/reloq_target_stage_router.h include/reloq_target_stage_merge.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_phone_sequence.o: src/reloq_german_phone_sequence.c include/reloq_german_phone_sequence.h include/reloq_german_annotation.h include/reloq_german_input.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_syllable_plan.o: src/reloq_german_syllable_plan.c include/reloq_german_syllable_plan.h include/reloq_german_phone_sequence.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_within_word_f0.o: src/reloq_german_within_word_f0.c include/reloq_german_within_word_f0.h include/reloq_german_syllable_plan.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_within_word_landmarks.o: src/reloq_german_within_word_landmarks.c include/reloq_german_within_word_landmarks.h include/reloq_german_syllable_plan.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_within_word_tract.o: src/reloq_german_within_word_tract.c include/reloq_german_within_word_tract.h include/reloq_german_within_word_landmarks.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_within_word_source.o: src/reloq_german_within_word_source.c include/reloq_german_within_word_source.h include/reloq_german_within_word_landmarks.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_within_word_composer.o: src/reloq_german_within_word_composer.c include/reloq_german_within_word_composer.h include/reloq_german_within_word_landmarks.h include/reloq_german_within_word_tract.h include/reloq_german_within_word_source.h include/reloq_german_within_word_f0.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_reduced_landmarks.o: src/reloq_german_reduced_landmarks.c include/reloq_german_reduced_landmarks.h include/reloq_german_syllable_plan.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_reduced_source.o: src/reloq_german_reduced_source.c include/reloq_german_reduced_source.h include/reloq_german_reduced_landmarks.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_reduced_tract.o: src/reloq_german_reduced_tract.c include/reloq_german_reduced_tract.h include/reloq_german_reduced_landmarks.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_reduced_open_first_tract.o: src/reloq_german_reduced_open_first_tract.c include/reloq_german_reduced_open_first_tract.h include/reloq_german_reduced_landmarks.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_reduced_open_source.o: src/reloq_german_reduced_open_source.c include/reloq_german_reduced_open_source.h include/reloq_german_reduced_landmarks.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_reduced_open_composer.o: src/reloq_german_reduced_open_composer.c include/reloq_german_reduced_open_composer.h include/reloq_german_reduced_open_source.h include/reloq_german_reduced_open_first_tract.h include/reloq_german_reduced_tract.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_reduced_closed_first_tract.o: src/reloq_german_reduced_closed_first_tract.c include/reloq_german_reduced_closed_first_tract.h include/reloq_german_reduced_landmarks.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_reduced_closed_composer.o: src/reloq_german_reduced_closed_composer.c include/reloq_german_reduced_closed_composer.h include/reloq_german_reduced_closed_first_tract.h include/reloq_german_reduced_source.h include/reloq_german_reduced_tract.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_cross_word_landmarks.o: src/reloq_german_cross_word_landmarks.c include/reloq_german_cross_word_landmarks.h include/reloq_german_syllable_plan.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_cross_word_contour.o: src/reloq_german_cross_word_contour.c include/reloq_german_cross_word_contour.h include/reloq_german_cross_word_landmarks.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_stop_vowel_interaction.o: src/reloq_german_stop_vowel_interaction.c include/reloq_german_stop_vowel_interaction.h include/reloq_german_phone_sequence.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_g2p.o: src/reloq_german_g2p.c src/reloq_german_t_quantity.inc include/reloq_german_g2p.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_compound_recovery.o: src/reloq_german_compound_recovery.c include/reloq_german_compound_recovery.h include/reloq_german_g2p.h include/reloq_german_lexicon.h include/reloq_german_spr.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_lexicon.o: src/reloq_lexicon.c include/reloq_lexicon.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_cvc_lexicon.o: src/reloq_german_cvc_lexicon.c include/reloq_german_cvc_lexicon.h include/reloq_lexicon.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_lexicon.o: src/reloq_german_lexicon.c src/reloq_german_roots85.inc include/reloq_german_lexicon.h include/reloq_german_cvc_lexicon.h include/reloq_lexicon.h include/reloq_german_g2p.h include/reloq_german_spr.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_lexicon_file.o: src/reloq_german_lexicon_file.c include/reloq_german_lexicon_file.h include/reloq_german_lexicon.h include/reloq_lexicon.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_text.o: src/reloq_german_text.c include/reloq_german_text.h include/reloq_german_lexicon.h include/reloq_lexicon.h include/reloq_german_input.h include/reloq_german_annotation.h include/reloq_utf8.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_target_planner.o: src/reloq_german_target_planner.c include/reloq_german_target_planner.h include/reloq_german_phone_sequence.h include/reloq_german_input.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_vowel_nucleus.o: src/reloq_german_vowel_nucleus.c include/reloq_german_vowel_nucleus.h include/reloq_german_target_planner.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_hv_timing.o: src/reloq_german_hv_timing.c include/reloq_german_hv_timing.h include/reloq_german_phone_sequence.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_initial_stop.o: src/reloq_german_initial_stop.c include/reloq_german_initial_stop.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_initial_voiced_stop.o: src/reloq_german_initial_voiced_stop.c include/reloq_german_initial_voiced_stop.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_initial_voiced_stop_envelope.o: src/reloq_german_initial_voiced_stop_envelope.c include/reloq_german_initial_voiced_stop_envelope.h include/reloq_german_terminal_timing.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_initial_nasal.o: src/reloq_german_initial_nasal.c include/reloq_german_initial_nasal.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_initial_nasal_envelope.o: src/reloq_german_initial_nasal_envelope.c include/reloq_german_initial_nasal_envelope.h include/reloq_german_terminal_timing.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_nasal_vowel.o: src/reloq_german_nasal_vowel.c include/reloq_german_nasal_vowel.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_initial_h.o: src/reloq_german_initial_h.c include/reloq_german_initial_h.h include/reloq_german_hv_timing.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_terminal_source.o: src/reloq_german_terminal_source.c include/reloq_german_terminal_source.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_terminal_stop_tract.o: src/reloq_german_terminal_stop_tract.c include/reloq_german_terminal_stop_tract.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_voice_defaults.o: src/reloq_german_voice_defaults.c include/reloq_german_voice_defaults.h include/reloq_session.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_phrase_f0.o: src/reloq_german_phrase_f0.c include/reloq_german_phrase_f0.h include/reloq_german_terminal_timing.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_stop_vowel.o: src/reloq_german_stop_vowel.c include/reloq_german_stop_vowel.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_stop_fricative_vowel.o: src/reloq_german_stop_fricative_vowel.c include/reloq_german_stop_fricative_vowel.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_voiced_stop_vowel.o: src/reloq_german_voiced_stop_vowel.c include/reloq_german_voiced_stop_vowel.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_voiced_stop_tract.o: src/reloq_german_voiced_stop_tract.c include/reloq_german_voiced_stop_tract.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_fricative_vowel.o: src/reloq_german_fricative_vowel.c include/reloq_german_fricative_vowel.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_terminal_fricative.o: src/reloq_german_terminal_fricative.c include/reloq_german_terminal_fricative.h include/reloq_german_phone_sequence.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_sentence_boundary.o: src/reloq_german_sentence_boundary.c include/reloq_german_sentence_boundary.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_voice.o: src/reloq_german_voice.c include/reloq_german_voice.h include/reloq_german_voice_defaults.h include/reloq_german_phrase_f0.h include/reloq_german_boundary_profile.h include/reloq_german_initial_voiced_stop.h include/reloq_german_initial_voiced_stop_envelope.h include/reloq_german_initial_nasal.h include/reloq_german_initial_nasal_envelope.h include/reloq_german_nasal_vowel.h include/reloq_german_voiced_stop_vowel.h include/reloq_german_voiced_stop_tract.h include/reloq_german_stop_fricative_vowel.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_utterance.o: src/reloq_german_utterance.c include/reloq_german_utterance.h include/reloq_german_target_planner.h include/reloq_german_text.h include/reloq_german_phone_sequence.h include/reloq_curve_scheduler.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_voice.o: src/reloq_german_generic_voice.c include/reloq_german_generic_voice.h include/reloq_german_utterance.h include/reloq_german_syllable_plan.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_point_budget.o: src/reloq_german_generic_point_budget.c include/reloq_german_generic_point_budget.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_noise.o: src/reloq_german_generic_noise.c include/reloq_german_generic_noise.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_prosody.o: src/reloq_german_generic_prosody.c include/reloq_german_generic_prosody.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_tract.o: src/reloq_german_generic_tract.c include/reloq_german_generic_tract.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_nasal_coda.o: src/reloq_german_generic_nasal_coda.c include/reloq_german_generic_nasal_coda.h include/reloq_german_generic_tract.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_voice_model.o: src/reloq_german_voice_model.c include/reloq_german_voice_model.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_voice_controls.o: src/reloq_voice_controls.c include/reloq_voice_controls.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_transition.o: src/reloq_german_generic_transition.c include/reloq_german_generic_transition.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_melody.o: src/reloq_german_generic_melody.c include/reloq_german_generic_melody.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_coarticulation.o: src/reloq_german_generic_coarticulation.c include/reloq_german_generic_coarticulation.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_timing.o: src/reloq_german_generic_timing.c include/reloq_german_generic_timing.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_index.o: src/reloq_german_generic_index.c include/reloq_german_generic_index.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_diphthong.o: src/reloq_german_generic_diphthong.c include/reloq_german_generic_diphthong.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_generic_stop_connection.o: src/reloq_german_generic_stop_connection.c include/reloq_german_generic_stop_connection.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_best_effort_text.o: src/reloq_german_best_effort_text.c include/reloq_german_best_effort_text.h include/reloq_german_g2p.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_terminal_timing.o: src/reloq_german_terminal_timing.c include/reloq_german_terminal_timing.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_connected_pair.o: src/reloq_german_connected_pair.c include/reloq_german_connected_pair.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_connected_kp.o: src/reloq_german_connected_kp.c include/reloq_german_connected_kp.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_connected_pt.o: src/reloq_german_connected_pt.c include/reloq_german_connected_pt.h include/reloq_target_builder.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_boundary_profile.o: src/reloq_german_boundary_profile.c include/reloq_german_boundary_profile.h include/reloq_german_connected_pair.h include/reloq_german_connected_kp.h include/reloq_german_connected_pt.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_connected_cvc_index.o: src/reloq_german_connected_cvc_index.c include/reloq_german_connected_cvc_index.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

reloq_german_connected_words.o: src/reloq_german_connected_words.c include/reloq_german_connected_words.h include/reloq_german_connected_cvc_index.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

libreloq_core.a: reloq_core.o reloq_annotation.o reloq_parameter_stream.o reloq_utf8.o reloq_cp1252.o reloq_german_input.o reloq_phonetic.o reloq_frame_adapter.o reloq_session.o reloq_numeric_pipeline.o reloq_acoustic_curve.o reloq_german_spr.o reloq_german_annotation.o reloq_curve_pipeline.o reloq_curve_scheduler.o reloq_target_builder.o reloq_target_stage_merge.o reloq_target_stage_router.o reloq_german_phone_sequence.o reloq_german_syllable_plan.o reloq_german_g2p.o reloq_german_compound_recovery.o reloq_lexicon.o reloq_german_cvc_lexicon.o reloq_german_lexicon.o reloq_german_lexicon_file.o reloq_german_text.o reloq_german_target_planner.o reloq_german_vowel_nucleus.o reloq_german_hv_timing.o reloq_german_initial_stop.o reloq_german_initial_voiced_stop.o reloq_german_initial_voiced_stop_envelope.o reloq_german_initial_nasal.o reloq_german_initial_nasal_envelope.o reloq_german_nasal_vowel.o reloq_german_initial_h.o reloq_german_terminal_source.o reloq_german_terminal_stop_tract.o reloq_german_voice_defaults.o reloq_german_phrase_f0.o reloq_german_stop_vowel.o reloq_german_stop_fricative_vowel.o reloq_german_voiced_stop_vowel.o reloq_german_voiced_stop_tract.o reloq_german_fricative_vowel.o reloq_german_terminal_fricative.o reloq_german_sentence_boundary.o reloq_german_voice.o reloq_german_utterance.o reloq_german_terminal_timing.o reloq_german_connected_pair.o reloq_german_connected_kp.o reloq_german_connected_pt.o reloq_german_boundary_profile.o reloq_german_connected_cvc_index.o reloq_german_connected_words.o
	rm -f $@
	$(AR) rcs $@ $(filter-out $(RULES61),$^)
	printf 'open %s\naddlib %s\nsave\nend\n' '$@' '$(RULES61)' | $(AR) -M

libreloq_core.a: reloq_german_within_word_f0.o reloq_german_within_word_landmarks.o reloq_german_within_word_tract.o reloq_german_within_word_source.o reloq_german_within_word_composer.o reloq_german_stop_vowel_interaction.o
libreloq_core.a: reloq_rules61.o reloq_german_rules61.o $(RULES61)
reloq_german_rules61.o: src/reloq_german_rules61.c include/reloq_rules61.h include/reloq61.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@
reloq_rules61.o: src/reloq_rules61.c include/reloq_rules61.h include/reloq61.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
libreloq_core.a: reloq_german_generic_voice.o reloq_german_generic_point_budget.o reloq_german_generic_noise.o reloq_german_generic_prosody.o reloq_german_generic_tract.o reloq_german_generic_nasal_coda.o reloq_german_generic_transition.o reloq_german_generic_melody.o reloq_german_generic_coarticulation.o reloq_german_generic_timing.o reloq_german_generic_index.o reloq_german_generic_diphthong.o reloq_german_generic_stop_connection.o reloq_german_voice_model.o reloq_voice_controls.o reloq_german_best_effort_text.o
libreloq_core.a: reloq_german_reduced_landmarks.o reloq_german_reduced_source.o reloq_german_reduced_tract.o reloq_german_reduced_open_first_tract.o reloq_german_reduced_open_source.o reloq_german_reduced_open_composer.o reloq_german_reduced_closed_first_tract.o reloq_german_reduced_closed_composer.o
libreloq_core.a: reloq_german_cross_word_landmarks.o reloq_german_cross_word_contour.o

test_reloq_core: tests/test_reloq_core.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_annotation: tests/test_reloq_annotation.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_parameter_stream: tests/test_reloq_parameter_stream.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_utf8: tests/test_reloq_utf8.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_cp1252: tests/test_reloq_cp1252.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_input: tests/test_reloq_german_input.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_phonetic: tests/test_reloq_phonetic.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_frame_adapter: tests/test_reloq_frame_adapter.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_session: tests/test_reloq_session.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_source_continuity: tests/test_reloq_source_continuity.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_source_continuity

test_reloq_numeric_pipeline: tests/test_reloq_numeric_pipeline.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_acoustic_curve: tests/test_reloq_acoustic_curve.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_spr: tests/test_reloq_german_spr.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_annotation: tests/test_reloq_german_annotation.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_curve_pipeline: tests/test_reloq_curve_pipeline.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_curve_scheduler: tests/test_reloq_curve_scheduler.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_target_builder: tests/test_reloq_target_builder.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_target_stage_merge: tests/test_reloq_target_stage_merge.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_target_stage_router: tests/test_reloq_target_stage_router.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_phone_sequence: tests/test_reloq_german_phone_sequence.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_syllable_plan: tests/test_reloq_german_syllable_plan.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_within_word_f0: tests/test_reloq_german_within_word_f0.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_within_word_landmarks: tests/test_reloq_german_within_word_landmarks.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_within_word_tract: tests/test_reloq_german_within_word_tract.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_within_word_source: tests/test_reloq_german_within_word_source.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_within_word_composer: tests/test_reloq_german_within_word_composer.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_voice: tests/test_reloq_german_generic_voice.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_point_budget: tests/test_reloq_german_generic_point_budget.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_oq: tests/test_reloq_german_generic_oq.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_tract: tests/test_reloq_german_generic_tract.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_nasal_coda: tests/test_reloq_german_generic_nasal_coda.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_prosody: tests/test_reloq_german_generic_prosody.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_noise: tests/test_reloq_german_generic_noise.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_voice_controls: tests/test_reloq_voice_controls.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_transition: tests/test_reloq_german_generic_transition.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_melody: tests/test_reloq_german_generic_melody.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_coarticulation: tests/test_reloq_german_generic_coarticulation.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_timing: tests/test_reloq_german_generic_timing.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_index: tests/test_reloq_german_generic_index.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_diphthong: tests/test_reloq_german_generic_diphthong.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_generic_stop_connection: tests/test_reloq_german_generic_stop_connection.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_best_effort_text: tests/test_reloq_german_best_effort_text.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_compound_recovery: tests/test_reloq_german_compound_recovery.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_reduced_landmarks: tests/test_reloq_german_reduced_landmarks.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_reduced_source: tests/test_reloq_german_reduced_source.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_reduced_tract: tests/test_reloq_german_reduced_tract.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_reduced_open_first_tract: tests/test_reloq_german_reduced_open_first_tract.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_reduced_open_source: tests/test_reloq_german_reduced_open_source.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_reduced_open_composer: tests/test_reloq_german_reduced_open_composer.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_reduced_closed_first_tract: tests/test_reloq_german_reduced_closed_first_tract.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_reduced_closed_composer: tests/test_reloq_german_reduced_closed_composer.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_cross_word_landmarks: tests/test_reloq_german_cross_word_landmarks.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_cross_word_contour: tests/test_reloq_german_cross_word_contour.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_stop_vowel_interaction: tests/test_reloq_german_stop_vowel_interaction.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_g2p: tests/test_reloq_german_g2p.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_lexicon: tests/test_reloq_lexicon.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_lexicon_file: tests/test_reloq_german_lexicon_file.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_cvc_lexicon: tests/test_reloq_german_cvc_lexicon.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_text: tests/test_reloq_german_text.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_target_planner: tests/test_reloq_german_target_planner.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_vowel_nucleus: tests/test_reloq_german_vowel_nucleus.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_hv_timing: tests/test_reloq_german_hv_timing.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_initial_stop: tests/test_reloq_german_initial_stop.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_initial_voiced_stop: tests/test_reloq_german_initial_voiced_stop.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_initial_voiced_stop_envelope: tests/test_reloq_german_initial_voiced_stop_envelope.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_initial_nasal: tests/test_reloq_german_initial_nasal.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_initial_nasal_envelope: tests/test_reloq_german_initial_nasal_envelope.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_nasal_vowel: tests/test_reloq_german_nasal_vowel.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_initial_h: tests/test_reloq_german_initial_h.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_terminal_source: tests/test_reloq_german_terminal_source.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_terminal_stop_tract: tests/test_reloq_german_terminal_stop_tract.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_voice_defaults: tests/test_reloq_german_voice_defaults.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_phrase_f0: tests/test_reloq_german_phrase_f0.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_stop_vowel: tests/test_reloq_german_stop_vowel.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_stop_fricative_vowel: tests/test_reloq_german_stop_fricative_vowel.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_voiced_stop_vowel: tests/test_reloq_german_voiced_stop_vowel.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_voiced_stop_tract: tests/test_reloq_german_voiced_stop_tract.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_fricative_vowel: tests/test_reloq_german_fricative_vowel.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_terminal_fricative: tests/test_reloq_german_terminal_fricative.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_sentence_boundary: tests/test_reloq_german_sentence_boundary.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_voice: tests/test_reloq_german_voice.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_connected_pair: tests/test_reloq_german_connected_pair.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_connected_pt: tests/test_reloq_german_connected_pt.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_connected_words: tests/test_reloq_german_connected_words.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

# POSIX development CLI for the genuinely covered, narrow text voice.
reloq_speak_wav: tools/reloq_speak_wav.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

reloq_speak_generic_wav: tools/reloq_speak_generic_wav.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_hvc_composition: tests/test_reloq_german_hvc_composition.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_utterance: tests/test_reloq_german_utterance.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test_reloq_german_terminal_timing: tests/test_reloq_german_terminal_timing.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_compound_recovery test_reloq_german_nucleus_acoustics
test: test_reloq_core test_reloq_annotation test_reloq_parameter_stream test_reloq_utf8 test_reloq_cp1252 test_reloq_german_input test_reloq_phonetic test_reloq_frame_adapter test_reloq_session test_reloq_numeric_pipeline test_reloq_acoustic_curve test_reloq_german_spr test_reloq_german_annotation test_reloq_curve_pipeline test_reloq_curve_scheduler test_reloq_target_builder test_reloq_target_stage_merge test_reloq_target_stage_router test_reloq_german_phone_sequence test_reloq_german_syllable_plan test_reloq_german_g2p test_reloq_lexicon test_reloq_german_lexicon_file test_reloq_german_cvc_lexicon test_reloq_german_text test_reloq_german_target_planner test_reloq_german_vowel_nucleus test_reloq_german_hv_timing test_reloq_german_initial_stop test_reloq_german_initial_voiced_stop test_reloq_german_initial_voiced_stop_envelope test_reloq_german_initial_nasal test_reloq_german_initial_nasal_envelope test_reloq_german_nasal_vowel test_reloq_german_initial_h test_reloq_german_terminal_source test_reloq_german_terminal_stop_tract test_reloq_german_voice_defaults test_reloq_german_phrase_f0 test_reloq_german_stop_vowel test_reloq_german_stop_fricative_vowel test_reloq_german_voiced_stop_vowel test_reloq_german_voiced_stop_tract test_reloq_german_fricative_vowel test_reloq_german_terminal_fricative test_reloq_german_sentence_boundary test_reloq_german_voice test_reloq_german_connected_pair test_reloq_german_connected_pt test_reloq_german_connected_words test_reloq_german_hvc_composition test_reloq_german_utterance test_reloq_german_terminal_timing

test: test_reloq_german_within_word_f0 test_reloq_german_within_word_landmarks test_reloq_german_within_word_tract test_reloq_german_within_word_source test_reloq_german_within_word_composer test_reloq_german_generic_voice test_reloq_german_generic_point_budget test_reloq_german_generic_oq test_reloq_german_generic_tract test_reloq_german_generic_nasal_coda test_reloq_german_generic_prosody test_reloq_german_generic_noise test_reloq_german_generic_transition test_reloq_german_generic_melody test_reloq_german_generic_coarticulation test_reloq_german_generic_timing test_reloq_german_generic_index test_reloq_german_generic_diphthong test_reloq_german_generic_stop_connection test_reloq_voice_controls test_reloq_german_best_effort_text test_reloq_german_reduced_landmarks test_reloq_german_reduced_source test_reloq_german_reduced_tract test_reloq_german_reduced_open_first_tract test_reloq_german_reduced_open_source test_reloq_german_reduced_open_composer test_reloq_german_reduced_closed_first_tract test_reloq_german_reduced_closed_composer test_reloq_german_cross_word_landmarks test_reloq_german_cross_word_contour test_reloq_german_stop_vowel_interaction
	./test_reloq_core
	./test_reloq_duration_span
	./test_reloq_german_transition_duration
	./test_reloq_annotation
	./test_reloq_parameter_stream
	./test_reloq_utf8
	./test_reloq_cp1252
	./test_reloq_german_input
	./test_reloq_phonetic
	./test_reloq_frame_adapter
	./test_reloq_session
	./test_reloq_source_continuity
	./test_reloq_numeric_pipeline
	./test_reloq_acoustic_curve
	./test_reloq_german_spr
	./test_reloq_german_feature_roundtrip
	./test_reloq_german_annotation
	./test_reloq_curve_pipeline
	./test_reloq_curve_scheduler
	./test_reloq_target_builder
	./test_reloq_target_stage_merge
	./test_reloq_target_stage_router
	./test_reloq_german_phone_sequence
	./test_reloq_german_syllable_plan
	./test_reloq_german_within_word_f0
	./test_reloq_german_within_word_landmarks
	./test_reloq_german_within_word_tract
	./test_reloq_german_within_word_source
	./test_reloq_german_within_word_composer
	./test_reloq_german_generic_voice
	./test_reloq_german_generic_point_budget
	./test_reloq_german_generic_oq
	./test_reloq_german_nucleus_acoustics
	./test_reloq_german_nucleus_duration
	./test_reloq_german_nucleus_prosody
	./test_reloq_german_nucleus_schedule
	./test_reloq_german_generic_tract
	./test_reloq_german_generic_nasal_coda
	./test_reloq_german_generic_prosody
	./test_reloq_german_generic_noise
	./test_reloq_german_generic_transition
	./test_reloq_german_source_envelope
	./test_reloq_german_generic_melody
	./test_reloq_german_morphology
	./test_reloq_german_coda_timing
	./test_reloq_german_root_plan
	./test_reloq_german_glottal
	./test_reloq_german_numbers
	./test_reloq_german_source_handoff
	./test_reloq_german_affricate
	./test_reloq_german_generic_coarticulation
	./test_reloq_german_generic_timing
	./test_reloq_german_generic_index
	./test_reloq_german_generic_diphthong
	./test_reloq_german_generic_stop_connection
	./test_reloq_voice_controls
	./test_reloq_german_best_effort_text
	./test_reloq_german_compound_recovery
	./test_reloq_german_reduced_landmarks
	./test_reloq_german_reduced_source
	./test_reloq_german_reduced_tract
	./test_reloq_german_reduced_open_first_tract
	./test_reloq_german_reduced_open_source
	./test_reloq_german_reduced_open_composer
	./test_reloq_german_reduced_closed_first_tract
	./test_reloq_german_reduced_closed_composer
	./test_reloq_german_cross_word_landmarks
	./test_reloq_german_cross_word_contour
	./test_reloq_german_stop_vowel_interaction
	./test_reloq_german_g2p
	./test_reloq_lexicon
	./test_reloq_german_lexicon_file
	./test_reloq_german_cvc_lexicon
	./test_reloq_german_text
	./test_reloq_german_target_planner
	./test_reloq_german_vowel_nucleus
	./test_reloq_german_hv_timing
	./test_reloq_german_initial_stop
	./test_reloq_german_initial_voiced_stop
	./test_reloq_german_initial_voiced_stop_envelope
	./test_reloq_german_initial_nasal
	./test_reloq_german_initial_nasal_envelope
	./test_reloq_german_nasal_vowel
	./test_reloq_german_initial_h
	./test_reloq_german_terminal_source
	./test_reloq_german_terminal_stop_tract
	./test_reloq_german_voice_defaults
	./test_reloq_german_phrase_f0
	./test_reloq_german_stop_vowel
	./test_reloq_german_stop_fricative_vowel
	./test_reloq_german_voiced_stop_vowel
	./test_reloq_german_voiced_stop_tract
	./test_reloq_german_fricative_vowel
	./test_reloq_german_terminal_fricative
	./test_reloq_german_sentence_boundary
	./test_reloq_german_voice
	./test_reloq_german_connected_pair
	./test_reloq_german_connected_pt
	./test_reloq_german_connected_words
	./test_reloq_german_hvc_composition --self-test
	./test_reloq_german_utterance
	./test_reloq_german_terminal_timing
	./test_reloq_wideband
	./test_reloq_presence
	./test_reloq_field_offsets

clean:
	rm -f test_reloq_owned_render test_reloq_rate_math test_reloq_rate_transform
	rm -f test_reloq_field_offsets
	rm -f test_reloq_presence
	rm -f reloq_english_spr.o reloq_english_spr.d
	rm -f reloq_character.o reloq_character.d test_reloq_character
	rm -f reloq_english_connected.o reloq_english_connected.d reloq_english_intonation.o reloq_english_intonation.d test_reloq_english_connected test_reloq_english_intonation
	rm -f reloq_english.o reloq_english.d reloq_english_nucleus.o reloq_english_nucleus.d reloq_english_nucleus_duration.o reloq_english_nucleus_duration.d reloq_english_text.o reloq_english_text.d reloq_english_voice.o reloq_english_voice.d test_reloq_english test_reloq_english_nucleus_duration
	rm -f reloq_german_stop_phase.o reloq_german_stop_phase.d
	rm -f reloq_german_onset_cluster.o
	rm -f reloq_german_affricate.o reloq_german_affricate.d test_reloq_german_affricate
	rm -f reloq_german_root_plan.o reloq_german_root_plan.d reloq_german_punctuation.o reloq_german_punctuation.d test_reloq_german_root_plan
	rm -f reloq_german_source_envelope.o reloq_german_source_envelope.d test_reloq_german_source_envelope
	rm -f reloq_german_morphology.o reloq_german_morphology.d test_reloq_german_morphology test_reloq_german_source_handoff
	rm -f reloq_german_nucleus_schedule.o reloq_german_nucleus_schedule.d test_reloq_german_nucleus_schedule
	rm -f reloq_german_nucleus_prosody.o reloq_german_nucleus_prosody.d test_reloq_german_nucleus_prosody
	rm -f reloq_german_nucleus_duration.o reloq_german_nucleus_duration.d test_reloq_german_nucleus_duration
	rm -f reloq_german_nucleus_acoustics.o reloq_german_nucleus_acoustics.d test_reloq_german_nucleus_acoustics
	rm -f reloq_german_transition_duration.o reloq_german_transition_duration.d test_reloq_german_transition_duration
	rm -f reloq_duration_span.o reloq_duration_span.d test_reloq_duration_span
	rm -f reloq_core.o reloq_annotation.o reloq_parameter_stream.o reloq_utf8.o reloq_cp1252.o \
	    reloq_german_input.o \
	    reloq_phonetic.o reloq_frame_adapter.o reloq_session.o \
	    reloq_numeric_pipeline.o reloq_acoustic_curve.o \
	    reloq_german_spr.o reloq_german_annotation.o reloq_curve_pipeline.o \
	    reloq_curve_scheduler.o reloq_target_builder.o reloq_target_stage_merge.o reloq_target_stage_router.o \
	    reloq_german_phone_sequence.o reloq_german_syllable_plan.o reloq_german_within_word_f0.o reloq_german_within_word_landmarks.o reloq_german_within_word_tract.o reloq_german_within_word_source.o reloq_german_within_word_composer.o reloq_german_reduced_landmarks.o reloq_german_reduced_source.o reloq_german_reduced_tract.o reloq_german_reduced_open_first_tract.o reloq_german_reduced_open_source.o reloq_german_reduced_open_composer.o reloq_german_reduced_closed_first_tract.o reloq_german_reduced_closed_composer.o reloq_german_cross_word_landmarks.o reloq_german_cross_word_contour.o reloq_german_stop_vowel_interaction.o reloq_german_g2p.o reloq_lexicon.o reloq_german_cvc_lexicon.o \
	    reloq_german_lexicon.o reloq_german_lexicon_file.o reloq_german_text.o \
	    reloq_german_target_planner.o reloq_german_vowel_nucleus.o \
	    reloq_german_hv_timing.o \
	    reloq_german_initial_stop.o reloq_german_initial_voiced_stop.o reloq_german_initial_voiced_stop_envelope.o reloq_german_initial_nasal.o reloq_german_initial_nasal_envelope.o reloq_german_nasal_vowel.o \
	    reloq_german_initial_h.o \
	    reloq_german_terminal_source.o reloq_german_terminal_stop_tract.o \
	    reloq_german_voice_defaults.o reloq_german_phrase_f0.o reloq_german_stop_vowel.o reloq_german_stop_fricative_vowel.o reloq_german_voiced_stop_vowel.o reloq_german_voiced_stop_tract.o reloq_german_fricative_vowel.o reloq_german_terminal_fricative.o reloq_german_sentence_boundary.o reloq_german_voice.o \
	    reloq_german_utterance.o reloq_german_generic_voice.o reloq_german_generic_point_budget.o reloq_german_generic_noise.o reloq_german_generic_prosody.o reloq_german_generic_tract.o reloq_german_generic_nasal_coda.o reloq_german_generic_transition.o reloq_german_generic_melody.o reloq_german_generic_coarticulation.o reloq_german_generic_timing.o reloq_german_generic_index.o reloq_german_generic_diphthong.o reloq_german_generic_stop_connection.o reloq_german_voice_model.o reloq_voice_controls.o reloq_german_best_effort_text.o reloq_german_terminal_timing.o \
	    reloq_german_connected_pair.o reloq_german_connected_kp.o reloq_german_connected_pt.o reloq_german_boundary_profile.o reloq_german_connected_cvc_index.o reloq_german_connected_words.o \
	    libreloq_core.a $(CORE_DEPFILES) test_reloq_core test_reloq_annotation \
	    test_reloq_parameter_stream test_reloq_utf8 test_reloq_cp1252 test_reloq_german_input \
	    test_reloq_phonetic \
	    test_reloq_frame_adapter test_reloq_session test_reloq_source_continuity \
	    test_reloq_numeric_pipeline test_reloq_acoustic_curve \
	    test_reloq_german_spr test_reloq_german_annotation \
	    test_reloq_curve_pipeline test_reloq_curve_scheduler \
	    test_reloq_target_builder test_reloq_target_stage_merge test_reloq_target_stage_router test_reloq_german_phone_sequence \
	    test_reloq_german_syllable_plan \
	    test_reloq_german_within_word_f0 \
	    test_reloq_german_within_word_landmarks test_reloq_german_within_word_tract test_reloq_german_within_word_source test_reloq_german_within_word_composer \
	    test_reloq_german_generic_voice test_reloq_german_generic_point_budget test_reloq_german_generic_oq test_reloq_german_generic_tract test_reloq_german_generic_nasal_coda test_reloq_german_generic_prosody test_reloq_german_generic_noise test_reloq_german_generic_transition test_reloq_german_generic_melody test_reloq_german_generic_coarticulation test_reloq_german_generic_timing test_reloq_german_generic_index test_reloq_german_generic_diphthong test_reloq_german_generic_stop_connection test_reloq_voice_controls test_reloq_german_best_effort_text reloq_speak_generic_wav \
	    test_reloq_german_reduced_landmarks test_reloq_german_reduced_source test_reloq_german_reduced_tract \
	    test_reloq_german_reduced_open_first_tract \
	    test_reloq_german_reduced_open_source \
	    test_reloq_german_reduced_open_composer \
	    test_reloq_german_reduced_closed_first_tract \
	    test_reloq_german_reduced_closed_composer \
	    test_reloq_german_cross_word_landmarks \
	    test_reloq_german_cross_word_contour \
	    test_reloq_german_stop_vowel_interaction \
	    test_reloq_german_g2p test_reloq_lexicon test_reloq_german_lexicon_file test_reloq_german_cvc_lexicon test_reloq_german_text \
	    test_reloq_german_target_planner test_reloq_german_vowel_nucleus \
	    test_reloq_german_hv_timing \
	    test_reloq_german_initial_stop test_reloq_german_initial_voiced_stop test_reloq_german_initial_voiced_stop_envelope test_reloq_german_initial_nasal test_reloq_german_initial_nasal_envelope test_reloq_german_nasal_vowel \
	    test_reloq_german_initial_h \
	    test_reloq_german_terminal_source test_reloq_german_terminal_stop_tract \
	    test_reloq_german_voice_defaults test_reloq_german_phrase_f0 test_reloq_german_stop_vowel test_reloq_german_stop_fricative_vowel test_reloq_german_voiced_stop_vowel test_reloq_german_voiced_stop_tract test_reloq_german_fricative_vowel test_reloq_german_terminal_fricative test_reloq_german_sentence_boundary test_reloq_german_voice \
	    reloq_speak_wav test_reloq_german_connected_pair test_reloq_german_connected_pt test_reloq_german_connected_words test_reloq_german_hvc_composition \
	    test_reloq_german_utterance test_reloq_german_terminal_timing
	rm -f reloq_german_compound_recovery.o test_reloq_german_compound_recovery

libreloq_core.a: reloq_german_word_features.o reloq_german_glottal.o
reloq_german_word_features.o: src/reloq_german_word_features.c src/reloq_german_glottal_lexicon.inc include/reloq_german_word_features.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
reloq_german_glottal.o: src/reloq_german_glottal.c include/reloq_german_glottal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

libreloq_core.a: reloq_german_numbers.o
reloq_german_numbers.o: src/reloq_german_numbers.c src/reloq_german_number_names.inc src/reloq_german_date_names.inc include/reloq_german_numbers.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

test_reloq_german_glottal: tests/test_reloq_german_glottal.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
test_reloq_german_numbers: tests/test_reloq_german_numbers.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
test: test_reloq_german_glottal test_reloq_german_numbers

libreloq_core.a: reloq_german_root_plan.o
reloq_german_root_plan.o: src/reloq_german_root_plan.c src/reloq_german_ie_roles.inc src/reloq_german_root_stress_roles.inc include/reloq_german_root_plan.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

libreloq_core.a: reloq_german_punctuation.o
reloq_german_punctuation.o: src/reloq_german_punctuation.c include/reloq_german_punctuation.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

test_reloq_german_root_plan: tests/test_reloq_german_root_plan.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
test: test_reloq_german_root_plan

reloq_german_onset_cluster.o: src/reloq_german_onset_cluster.c include/reloq_german_onset_cluster.h include/reloq_german_stop_phase.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@

# Reference-measured natural-text abbreviations (no Apple runtime required).
test_reloq_german_original_abbreviations: tests/test_reloq_german_original_abbreviations.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_original_abbreviations

# Original public-reference character and diacritic names.
test_reloq_german_original_character_names: tests/test_reloq_german_original_character_names.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_german_original_character_names

reloq_german_generic_voice.o: src/reloq_german_inline_controls.inc include/reloq_inline_tags.h


test_reloq_wideband: tests/test_reloq_wideband.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
test: test_reloq_wideband

test_reloq_presence: tests/test_reloq_presence.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@

test: test_reloq_presence

test_reloq_field_offsets: tests/test_reloq_field_offsets.c tests/test_reloq_field_offsets_expected.h libreloq_core.a
	$(CC) $(CPPFLAGS) $(filter-out -std=c11 -pedantic,$(CFLAGS)) -std=gnu11 -DEVV_ARENA=1 -Iinclude -Ivendor/openevv61/include -Ivendor/openevv61/src/delta -Ivendor/openevv61/src/port $< libreloq_core.a -lm -o $@

test: test_reloq_field_offsets

libreloq_core.a: reloq_rate_transfer.o
libreloq_core.a: reloq_noise_profile.o
reloq_noise_profile.o: src/reloq_noise_profile.c include/reloq_noise_profile.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
reloq_rate_transfer.o: src/reloq_rate_transfer.c include/reloq_rate_transfer.h include/reloq_core.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude -c $< -o $@

reloq_english_nucleus.o: src/reloq_english_continuant_targets.inc

libreloq_core.a: reloq_unicode.o
reloq_unicode.o: src/reloq_unicode.c src/reloq_unicode_data.inc include/reloq_unicode.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
test_reloq_unicode: tests/test_reloq_unicode.c libreloq_core.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude $< libreloq_core.a -lm -o $@
test: test-unicode
.PHONY: test-unicode
test-unicode: test_reloq_unicode
	./test_reloq_unicode

libreloq_core.a: reloq_english_g2p.o
reloq_english_g2p.o: src/reloq_english_g2p.c src/reloq_english_g2p83.inc
	$(CC) $(CPPFLAGS) $(CFLAGS) -Iinclude -c $< -o $@
reloq_english.o: src/reloq_english_vocabulary82.inc src/reloq_english_vocabulary83.inc

test_reloq_rate_math: tests/test_reloq_rate_math.c src/reloq_rate_transfer.c include/reloq_rate_transfer.h include/reloq_core.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude $< -lm -o $@

.PHONY: test-rate-math
test-rate-math: test_reloq_rate_math
	./test_reloq_rate_math
test: test-rate-math

test_reloq_rate_transform: tests/test_reloq_rate_transform.c src/reloq_rate_transfer.c include/reloq_rate_transfer.h include/reloq_core.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -Iinclude $< -lm -o $@

.PHONY: test-rate-transform
test-rate-transform: test_reloq_rate_transform
	./test_reloq_rate_transform
test: test-rate-transform
