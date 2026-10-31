.DELETE_ON_ERROR:
CC = x86_64-w64-mingw32-gcc
CFLAGS = -std=c11 -Os -Wall -Wextra -Werror -Wno-misleading-indentation -ffunction-sections -fdata-sections -DUNICODE -D_UNICODE -I. -I../include
LDFLAGS = -static-libgcc -Wl,--gc-sections,--strip-all,--no-insert-timestamp
LIBS = -lole32 -luuid -ladvapi32 -lcomctl32 -lshell32 -lgdi32 -lm
BUILD ?= build
SOURCES = engine.c backend.c settings.c dialog.c register.c guids.c ../src/reloq_voice_controls.c
all: $(BUILD)/ReLoqSapi.dll $(BUILD)/ReLoqSettings.exe $(BUILD)/Install-SAPI.exe $(BUILD)/Test-SAPI.exe
$(BUILD):
	mkdir -p $@
$(BUILD)/ReLoqSapi.dll: $(SOURCES) sapi_ddk.h settings.h catalogue.inc | $(BUILD)
	$(CC) $(CFLAGS) -shared $(SOURCES) $(LDFLAGS) $(LIBS) -o $@
$(BUILD)/ReLoqSettings.exe: settings_main.c | $(BUILD)
	$(CC) $(CFLAGS) -municode -mwindows $< $(LDFLAGS) -luser32 -o $@
$(BUILD)/Install-SAPI.exe: install.c | $(BUILD)
	$(CC) $(CFLAGS) -municode -mwindows $< $(LDFLAGS) $(LIBS) -o $@
$(BUILD)/Test-SAPI.exe: test_windows.c guids.c sapi_ddk.h catalogue.inc | $(BUILD)
	$(CC) $(CFLAGS) -municode test_windows.c guids.c $(LDFLAGS) $(LIBS) -o $@

.PHONY: test
HOST_CC ?= cc
test: | $(BUILD)
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -DRELOQ_SAPI_TEST_HOST -I. -I../include backend.c test_host.c test_backend.c ../src/reloq_voice_controls.c -ldl -lpthread -lm -o $(BUILD)/test_backend
	$(BUILD)/test_backend $(abspath ../eci/libreloq_eci.so) > $(BUILD)/backend-results.json
	python3 test_reference.py --library $(abspath ../eci/libreloq_eci.so) --report $(BUILD)/backend-results.json
