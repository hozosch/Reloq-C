.DELETE_ON_ERROR:
CC = $(if $(filter x86,$(ARCH)),i686,x86_64)-w64-mingw32-gcc
CFLAGS = -std=c11 -Os -Wall -Wextra -Werror -Wno-misleading-indentation -ffunction-sections -fdata-sections -DUNICODE -D_UNICODE -I. -I../include
LDFLAGS = -static-libgcc -Wl,--gc-sections,--strip-all,--no-insert-timestamp
LIBS = -lole32 -luuid -ladvapi32 -lcomctl32 -lshell32 -lgdi32 -lwinhttp -lm
BUILD ?= $(if $(filter x86,$(ARCH)),build/x86,build)
ARCH ?= x64
SOURCES = engine.c backend.c settings.c dictionary.c dictionary_diff.c dialog.c register.c guids.c ../src/reloq_voice_controls.c
ifeq ($(ARCH),x86)
CFLAGS += -msse2 -mfpmath=sse
LDFLAGS += -Wl,--kill-at
endif
all: $(BUILD)/ReLoqSapi.dll $(BUILD)/ReLoqSettings.exe $(BUILD)/Test-SAPI.exe $(BUILD)/Test-Dictionary.exe
$(BUILD):
	mkdir -p $@
$(BUILD)/ReLoqSapi.dll: $(SOURCES) sapi_ddk.h settings.h speed.h format.h dictionary_diff.h catalogue.inc ui.inc | $(BUILD)
	$(CC) $(CFLAGS) -shared $(SOURCES) $(LDFLAGS) $(LIBS) -o $@
$(BUILD)/ReLoqSettings.exe: settings_main.c | $(BUILD)
	$(CC) $(CFLAGS) -municode -mwindows $< $(LDFLAGS) -luser32 -o $@
$(BUILD)/Test-SAPI.exe: test_windows.c guids.c sapi_ddk.h catalogue.inc | $(BUILD)
	$(CC) $(CFLAGS) -municode test_windows.c guids.c $(LDFLAGS) $(LIBS) -o $@

$(BUILD)/Test-Dictionary.exe: test_dictionary_download.c dictionary.c dictionary_diff.c dictionary_diff.h settings.h | $(BUILD)
	$(CC) $(CFLAGS) test_dictionary_download.c dictionary.c dictionary_diff.c $(LDFLAGS) -lwinhttp -ladvapi32 -o $@

.PHONY: test
HOST_CC ?= cc
test: | $(BUILD)
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -I. dictionary_diff.c test_dictionary_diff.c -o $(BUILD)/test_dictionary_diff
	$(BUILD)/test_dictionary_diff
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -I. test_format.c -o $(BUILD)/test_format
	$(BUILD)/test_format > $(BUILD)/format-results.json
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -I. test_speed.c -o $(BUILD)/test_speed
	$(BUILD)/test_speed > $(BUILD)/speed-results.json
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -DRELOQ_SAPI_TEST_HOST -DRELOQ_SAPI_SETTINGS_TEST -I. settings.c test_settings.c -o $(BUILD)/test_settings
	$(BUILD)/test_settings > $(BUILD)/settings-results.json
	python3 generate_ui.py --check
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -DRELOQ_SAPI_TEST_HOST -I. test_ui.c -o $(BUILD)/test_ui
	$(BUILD)/test_ui > $(BUILD)/ui-results.json
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -DRELOQ_SAPI_TEST_HOST -I. -I../include backend.c test_host.c test_backend.c ../src/reloq_voice_controls.c -ldl -lpthread -lm -o $(BUILD)/test_backend
	$(BUILD)/test_backend $(abspath ../eci/libreloq_eci.so) > $(BUILD)/backend-results.json
	python3 test_reference.py --library $(abspath ../eci/libreloq_eci.so) --report $(BUILD)/backend-results.json
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -DRELOQ_SAPI_TEST_HOST -I. -I../include backend.c test_host.c test_buffered_cancel.c ../src/reloq_voice_controls.c -ldl -lpthread -lm -o $(BUILD)/test_buffered_cancel
	$(BUILD)/test_buffered_cancel $(abspath ../eci/libreloq_eci.so) > $(BUILD)/buffered-cancel-results.json
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -DRELOQ_SAPI_TEST_HOST -I. -I../include backend.c test_host.c test_cancel_recovery.c ../src/reloq_voice_controls.c -ldl -lpthread -lm -o $(BUILD)/test_cancel_recovery
	$(BUILD)/test_cancel_recovery $(abspath ../eci/libreloq_eci.so) > $(BUILD)/cancel-recovery-results.txt
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -DRELOQ_SAPI_TEST_HOST -I. -I../include backend.c test_host.c test_dictionary.c ../src/reloq_voice_controls.c -ldl -lpthread -lm -o $(BUILD)/test_dictionary
	$(BUILD)/test_dictionary $(abspath ../eci/libreloq_eci.so) > $(BUILD)/dictionary-results.json
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -DRELOQ_SAPI_TEST_HOST -I. -I../include backend.c test_host.c test_speed_native.c ../src/reloq_voice_controls.c -ldl -lpthread -lm -o $(BUILD)/test_speed_native
	$(BUILD)/test_speed_native $(abspath ../eci/libreloq_eci.so) > $(BUILD)/speed-native-results.json
	python3 test_speed_reference.py --library $(abspath ../eci/libreloq_eci.so) --report $(BUILD)/speed-native-results.json
