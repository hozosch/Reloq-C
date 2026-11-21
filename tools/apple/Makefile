# Build the shared ECI core as an Apple XCFramework.
# Based on the Apple build approach in ReLoq Apple by Quinton Williams
# (https://github.com/Quinton1110/ReLoq-Apple, GPL-3.0-or-later).
# The source list intentionally matches eci/Makefile, including the Persian
# acoustics object that the native rule-provider archive already supplies.
.DEFAULT_GOAL := xcframework
SELF := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOT := $(abspath $(dir $(SELF))/../..)
OUT ?= $(ROOT)/build/apple
OUT := $(abspath $(OUT))
PLATFORM ?= macos
OPT ?= -O2 -flto=thin
RULE_OPT ?= -Os

ifeq ($(PLATFORM),ios)
SDK := iphoneos
TARGET := arm64-apple-ios16.0
else ifeq ($(PLATFORM),ios-sim)
SDK := iphonesimulator
TARGET := arm64-apple-ios16.0-simulator
else ifeq ($(PLATFORM),macos)
SDK := macosx
TARGET := arm64-apple-macos11.0
else
$(error PLATFORM must be ios, ios-sim or macos)
endif

CC := $(shell xcrun --sdk $(SDK) --find clang) -isysroot $(shell xcrun --sdk $(SDK) --show-sdk-path) -target $(TARGET)
AR := $(shell xcrun --find ar)
BUILD := $(OUT)/$(PLATFORM)
RULES := $(BUILD)/rules/libreloq61.a
SOURCE := $(filter-out $(ROOT)/src/reloq_host.c $(ROOT)/src/reloq_voice_filter.c \
    $(ROOT)/src/reloq_orthography.c $(ROOT)/src/reloq_german_word_edges.c \
    $(ROOT)/src/reloq_persian_acoustics.c $(ROOT)/src/reloq_english%.c, \
    $(wildcard $(ROOT)/src/*.c)) $(ROOT)/src/reloq_english_spr.c
OBJECTS := $(patsubst $(ROOT)/src/%.c,$(BUILD)/core/%.o,$(SOURCE))
PUBLIC_HEADERS := eci.h reloq_eci.h reloq_persian_plan.h reloq_persian_numbers.h \
    reloq_persian_phones.h reloq_persian_us_proxy.h
CFLAGS := $(OPT) -std=c11 -Wall -Wextra -pedantic -pthread -ffunction-sections \
    -fdata-sections -ffp-contract=off -fvisibility=hidden -DECI_STATIC -MMD -MP -I$(ROOT)/include

.PHONY: lib rules xcframework clean
lib: $(BUILD)/libreloq.a

rules:
	$(MAKE) -C $(ROOT)/vendor/openevv61 -f Makefile.native BUILD='$(BUILD)/rules' \
	    CC='$(CC)' AR='$(AR)' OPT='$(OPT)' RULE_OPT='$(RULE_OPT)' PORT=posix
$(RULES): rules

$(BUILD)/core/%.o: $(ROOT)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
-include $(OBJECTS:.o=.d)

# Prelink the rule-provider archive and core once. The consuming application
# keeps its own app and extension signing separate from this static library.
$(BUILD)/reloq.o: $(OBJECTS) $(RULES)
	@mkdir -p $(BUILD)
	cp $(RULES) $(BUILD)/objects.a
	$(AR) rs $(BUILD)/objects.a $(OBJECTS) >/dev/null
	printf '_eci*\n_reloq_eci_*\n' > $(BUILD)/exports.txt
	$(CC) -r -nostdlib $(filter -flto%,$(OPT)) -Wl,-all_load \
	    -Wl,-exported_symbols_list,$(BUILD)/exports.txt $(BUILD)/objects.a -o $@
	rm -f $(BUILD)/objects.a

$(BUILD)/libreloq.a: $(BUILD)/reloq.o
	rm -f $@
	$(AR) rcs $@ $<
	@mkdir -p $(BUILD)/Headers
	cp $(addprefix $(ROOT)/include/,$(PUBLIC_HEADERS)) $(BUILD)/Headers/

xcframework:
	$(MAKE) -f $(SELF) PLATFORM=ios OUT=$(OUT) lib
	$(MAKE) -f $(SELF) PLATFORM=ios-sim OUT=$(OUT) lib
	$(MAKE) -f $(SELF) PLATFORM=macos OUT=$(OUT) lib
	rm -rf $(OUT)/ReLoq.xcframework
	xcodebuild -create-xcframework \
	    -library $(OUT)/ios/libreloq.a -headers $(OUT)/ios/Headers \
	    -library $(OUT)/ios-sim/libreloq.a -headers $(OUT)/ios-sim/Headers \
	    -library $(OUT)/macos/libreloq.a -headers $(OUT)/macos/Headers \
	    -output $(OUT)/ReLoq.xcframework

clean:
	rm -rf $(OUT)
