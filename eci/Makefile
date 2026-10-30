.DELETE_ON_ERROR:
.DEFAULT_GOAL := all
CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror -pedantic
SECTIONS := -ffunction-sections -fdata-sections
CPPFLAGS ?=
WINDOWS_CC ?= x86_64-w64-mingw32-gcc
WINDOWS_OBJDUMP ?= x86_64-w64-mingw32-objdump
WINDOWS_CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror -pedantic
WINDOWS_LDFLAGS ?= -static -Wl,--strip-all,--no-insert-timestamp
ANDROID_CC ?= aarch64-linux-android26-clang
ANDROID_AR ?= llvm-ar
ANDROID_BUILD ?= build/android-arm64
# NDK r30's complex.h expands the standard I macro to an imaginary literal.
ANDROID_CFLAGS := $(CFLAGS) -fsigned-char -Wno-c2y-extensions
# Keep component-test archives inexpensive; optimize the shipped provider
# across translation units, including its many small C rule primitives.
PROVIDER_OPT ?= -O3
PROVIDER_RULE_OPT ?= -Os
PROVIDER_LTO ?= -flto=4
# Our objects enforce -Werror at compilation. Keep upstream LTO diagnostics
# visible without turning its existing backtrack-flow warnings into errors.
PROVIDER_LINK := $(PROVIDER_OPT) $(PROVIDER_LTO) -fno-strict-aliasing -ffp-contract=off
RULES61 := ../vendor/openevv61/build/release/libreloq61.a
RULES61_WIN := ../vendor/openevv61/build/windows-release/libreloq61.a
RULES61_ANDROID := ../vendor/openevv61/build/android-arm64/libreloq61.a
RULES61_SOURCES := $(wildcard ../vendor/openevv61/src/*.c ../vendor/openevv61/src/*/*.c ../vendor/openevv61/src/*/*/*.c ../vendor/openevv61/src/*/*.h ../vendor/openevv61/src/*/*/*.h ../vendor/openevv61/include/*.h ../vendor/openevv61/lang/*/*.c ../vendor/openevv61/lang/*/*.h) ../vendor/openevv61/Makefile.native
RULES61_SOURCES += $(wildcard ../vendor/openevv61/rom/*/*.c ../vendor/openevv61/rom/*/*.h ../vendor/openevv61/rom/*/*.inc)
RULES61_SOURCES += ../src/reloq_voice_filter.c ../include/reloq_voice_filter.h
RULES61_SOURCES += ../include/reloq_voice_controls.h
RULES61_SOURCES += ../src/reloq_orthography.c ../include/reloq_orthography.h
RULES61_SOURCES += ../src/reloq_german_word_edges.c ../include/reloq_german_word_edges.h
RULES61_SOURCES += ../vendor/openevv61/src/reloq61_german_alphabet.inc
RULES61_SOURCES += ../src/reloq_english_word_edges.c ../include/reloq_english_word_edges.h
RULES61_SOURCES += ../vendor/openevv61/src/reloq61_english_alphabet.inc
RULES61_SOURCES += ../vendor/openevv61/src/reloq61_latin_data.inc
RULES61_SOURCES += ../vendor/openevv61/src/reloq61_text_case.inc
RULES61_SOURCES += ../vendor/openevv61/src/reloq61_dictionary.inc
RULES61_SOURCES += ../vendor/openevv61/lang/kokr61/foreign_words_lexicon_kokr61.inc ../vendor/openevv61/lang/kokr61/foreign_units_kokr61.inc ../vendor/openevv61/lang/kokr61/foreign_expansions_kokr61.inc ../vendor/openevv61/lang/kokr61/foreign_vowels_kokr61.inc
$(RULES61): $(RULES61_SOURCES)
	$(MAKE) -C ../vendor/openevv61 -f Makefile.native BUILD=build/release OPT='$(PROVIDER_OPT) $(PROVIDER_LTO)' RULE_OPT='$(PROVIDER_RULE_OPT)'
$(RULES61_WIN): $(RULES61_SOURCES)
	$(MAKE) -C ../vendor/openevv61 -f Makefile.native BUILD=build/windows-release OPT='$(PROVIDER_OPT) $(PROVIDER_LTO)' RULE_OPT='$(PROVIDER_RULE_OPT)' PORT=win32 CC=$(WINDOWS_CC) AR=$(patsubst %gcc-posix,%gcc-ar-posix,$(patsubst %gcc,%gcc-ar,$(WINDOWS_CC)))
$(RULES61_ANDROID): $(RULES61_SOURCES)
	$(MAKE) -C ../vendor/openevv61 -f Makefile.native BUILD='$(abspath $(dir $(RULES61_ANDROID)))' OPT='-O2 -fsigned-char' RULE_OPT='$(PROVIDER_RULE_OPT)' PORT=posix CC=$(ANDROID_CC) AR=$(ANDROID_AR)
libreloq_eci.so test_reloq_eci: $(RULES61)
eci.dll: $(RULES61_WIN)
libreloq_eci.so test_reloq_eci eci.dll: Makefile

# The ECI interface is compiled directly into the DLL with the independent
# numeric C engine. Neither the development host ABI nor an external helper
# process is needed by this target.
# The scalar endpoint filter is compiled once, with its provider adapter,
# so ordinary LTO can remove the temporary language-cell boundary as well.
SOURCE := $(filter-out ../src/reloq_host.c ../src/reloq_voice_filter.c ../src/reloq_orthography.c ../src/reloq_german_word_edges.c ../src/reloq_english%.c,$(wildcard ../src/*.c))
SOURCE += ../src/reloq_english_spr.c
HEADERS := $(wildcard ../src/reloq_english*.inc ../src/reloq_english*.h) ../src/reloq_unicode_data.inc
HEADERS += ../src/reloq_character_data.inc
HEADERS += ../src/reloq_german_inline_controls.inc ../src/reloq_german_roots85.inc
HEADERS += $(wildcard ../include/*.h) ../src/reloq_german_lexical_quantity.inc
HEADERS += ../src/reloq_core_internal.h
HEADERS += $(wildcard ../src/*.inc)
HEADERS += ../src/reloq_german_ung_roles.inc ../src/reloq_german_el_roles.inc
HEADERS += ../src/reloq_german_t_quantity.inc ../src/reloq_german_stop_edges.inc
HEADERS += ../src/reloq_german_en_roles.inc ../src/reloq_german_en_head_roles.inc ../src/reloq_german_weak_ending_roles.inc
HEADERS += ../src/reloq_german_contextual_stems.inc
HEADERS += ../src/reloq_german_compound_classes.inc
HEADERS += ../src/reloq_german_compound_boundaries.inc
HEADERS += ../src/reloq_german_g_french74.inc ../src/reloq_german_g_affricate74.inc
HEADERS += ../src/reloq_german_prefix_protected74.inc ../src/reloq_german_compound_accent74.inc
HEADERS += ../src/reloq_german_feminine_protected75.inc ../src/reloq_german_predicative_modifiers75.inc ../src/reloq_german_da_focus75.inc

# Compile the public host separately. LTO belongs to the private language
# rules and Delta primitives, not to the acoustic composer or legacy ECI ABI.
OBJECTS := $(patsubst ../src/%.c,build/posix/%.o,$(SOURCE))
WINDOWS_OBJECTS := $(patsubst ../src/%.c,build/windows/%.o,$(SOURCE))
ANDROID_OBJECTS := $(patsubst ../src/%.c,$(ANDROID_BUILD)/%.o,$(SOURCE))
$(ANDROID_BUILD)/%.o: ../src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(ANDROID_CC) $(CPPFLAGS) $(ANDROID_CFLAGS) $(SECTIONS) -ffp-contract=off -fPIC -fvisibility=hidden -I../include -c $< -o $@
$(ANDROID_BUILD)/reloq_jni.o: ../android/native/reloq_jni.c ../include/reloq_eci.h
	@mkdir -p $(dir $@)
	$(ANDROID_CC) $(ANDROID_CFLAGS) $(SECTIONS) -fPIC -fvisibility=hidden -I../include -c $< -o $@
libreloq_android.so: $(ANDROID_OBJECTS) $(ANDROID_BUILD)/reloq_jni.o $(RULES61_ANDROID)
	$(ANDROID_CC) -shared $^ -Wl,--gc-sections,--no-undefined,-z,max-page-size=16384 -lm -o $@
build/posix/%.o: ../src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SECTIONS) -ffp-contract=off -fPIC -fvisibility=hidden -I../include -c $< -o $@
build/windows/%.o: ../src/%.c $(HEADERS) | check-windows-toolchain
	@mkdir -p $(dir $@)
	$(WINDOWS_CC) $(CPPFLAGS) $(WINDOWS_CFLAGS) $(SECTIONS) -ffp-contract=off -fvisibility=hidden -I../include -c $< -o $@

.PHONY: all test windows-x64 check-windows-toolchain clean
all: libreloq_eci.so

libreloq_eci.so: $(SOURCE) $(HEADERS) ../src/reloq_glottal_profiles.inc ../src/reloq_german_root_roles.inc ../src/reloq_german_weak_verb_roles.inc ../src/reloq_german_inflection_roles.inc ../src/reloq_german_glottal_lexicon.inc ../src/reloq_german_number_names.inc ../src/reloq_german_date_names.inc ../src/reloq_german_ie_roles.inc ../src/reloq_german_root_stress_roles.inc ../src/reloq_german_phrase_metadata.inc
	$(CC) $(filter-out -Werror,$(CFLAGS)) $(PROVIDER_LINK) -shared $(OBJECTS) $(RULES61) -pthread -Wl,--gc-sections,--no-undefined -lm -o $@

libreloq_eci.so: $(OBJECTS)

test_reloq_eci: ../tests/test_reloq_eci.c $(SOURCE) $(HEADERS) ../src/reloq_glottal_profiles.inc ../src/reloq_german_root_roles.inc ../src/reloq_german_weak_verb_roles.inc ../src/reloq_german_inflection_roles.inc ../src/reloq_german_glottal_lexicon.inc ../src/reloq_german_number_names.inc ../src/reloq_german_date_names.inc ../src/reloq_german_ie_roles.inc ../src/reloq_german_root_stress_roles.inc ../src/reloq_german_phrase_metadata.inc
	$(CC) $(CPPFLAGS) $(filter-out -Werror,$(CFLAGS)) $(PROVIDER_LINK) -I../include ../tests/test_reloq_eci.c $(OBJECTS) $(RULES61) -pthread -Wl,--gc-sections -lm -o $@

test_reloq_eci: $(OBJECTS)

test_reloq_hash_delete: ../tests/test_reloq_hash_delete.c $(RULES61)
	$(CC) $(filter-out -Werror,$(CFLAGS)) $(PROVIDER_LINK) -DEVV_ARENA=1 -I../vendor/openevv61/src/port $< $(RULES61) -pthread -lm -o $@

test: test_reloq_eci test_reloq_hash_delete libreloq_eci.so
	./test_reloq_eci
	./test_reloq_hash_delete
	python3 test_public_abi.py libreloq_eci.so --header ../include/reloq_eci.h
	python3 test_diagnostics.py libreloq_eci.so
	python3 -B ../tools/update_chinese_traditional.py --check
	python3 -B ../tools/test_native_text_input.py
	python3 -B ../nvda/test_eci_text_input_native.py
	python3 -B ../nvda/test_eci_analysis_options.py
	python3 -B ../tools/german_generic_eci_regression.py --native-eci libreloq_eci.so --quiet
	python3 -B ../nvda/test_eci_numeric_native.py
	python3 -B ../nvda/test_eci_email_native.py
	python3 -B ../nvda/test_eci_letters_native.py
	python3 -B ../nvda/test_eci_tags_native.py
	python3 -B ../nvda/test_eci_pause_modes_native.py
	python3 -B ../nvda/test_eci_voice_models_native.py
	python3 -B ../nvda/test_eci_sample_rates_native.py
	python3 -B ../nvda/test_eci_sample_rates_transport.py
	python3 -B ../nvda/test_eci_transport.py
	python3 -B ../nvda/test_eci_dictionary_transport.py
	python3 -B ../nvda/test_eci_playback_queue.py
	python3 -B ../nvda/test_eci_player_watchdog.py
	python3 -B ../nvda/test_eci_english_native.py
	python3 -B ../nvda/test_eci_additional_languages_native.py
	python3 -B ../nvda/test_eci_western_pauses_native.py
	python3 -B ../nvda/test_eci_finnish_letters_native.py
	python3 -B ../nvda/test_eci_asian_greek_native.py

check-windows-toolchain:
	@command -v $(WINDOWS_CC) >/dev/null 2>&1 || { echo "missing Windows x64 compiler: $(WINDOWS_CC)" >&2; exit 1; }
	@command -v $(WINDOWS_OBJDUMP) >/dev/null 2>&1 || { echo "missing PE inspection tool: $(WINDOWS_OBJDUMP)" >&2; exit 1; }

eci.dll: $(SOURCE) $(HEADERS) ../src/reloq_glottal_profiles.inc ../src/reloq_german_root_roles.inc ../src/reloq_german_weak_verb_roles.inc ../src/reloq_german_inflection_roles.inc ../src/reloq_german_glottal_lexicon.inc ../src/reloq_german_number_names.inc ../src/reloq_german_date_names.inc ../src/reloq_german_ie_roles.inc ../src/reloq_german_root_stress_roles.inc ../src/reloq_german_phrase_metadata.inc | check-windows-toolchain
	$(WINDOWS_CC) $(filter-out -Werror,$(WINDOWS_CFLAGS)) $(PROVIDER_LINK) -shared $(WINDOWS_OBJECTS) $(RULES61_WIN) -static-libgcc -lm $(WINDOWS_LDFLAGS) -Wl,--gc-sections -Wl,--no-undefined -Wl,--out-implib,libeci.dll.a -o $@

eci.dll: $(WINDOWS_OBJECTS)

eci_windows_x64_abi_probe.exe: ../reference-apple/eci_windows_x64_abi_probe.c | check-windows-toolchain
	$(WINDOWS_CC) $(CPPFLAGS) $(WINDOWS_CFLAGS) $< -static-libgcc -o $@

eci_windows_x64_runtime_smoke.exe: eci_windows_x64_runtime_smoke.c ../include/reloq_eci.h | check-windows-toolchain
	$(WINDOWS_CC) $(CPPFLAGS) $(WINDOWS_CFLAGS) -I../include $< -static-libgcc -o $@

windows-x64: eci.dll eci_windows_x64_abi_probe.exe eci_windows_x64_runtime_smoke.exe
	python3 verify_pe_exports.py eci.dll --objdump $(WINDOWS_OBJDUMP) --header ../include/reloq_eci.h

clean:
	rm -f libreloq_eci.so test_reloq_eci test_reloq_hash_delete eci.dll libeci.dll.a eci_windows_x64_abi_probe.exe eci_windows_x64_runtime_smoke.exe
	rm -rf build
.DELETE_ON_ERROR:
