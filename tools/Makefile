CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror -pedantic
CPPFLAGS ?=
PYTHON ?= python3

.PHONY: all test clean
all: reloq_trace_wav

reloq_trace_wav: reloq_trace_wav.c ../src/reloq_core.c \
                 ../src/reloq_glottal_profiles.inc ../include/reloq_core.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -ffp-contract=off -I../include \
	    reloq_trace_wav.c ../src/reloq_core.c -lm -o $@

test: reloq_trace_wav
	$(PYTHON) test_trace_wav.py ./reloq_trace_wav example.trace

clean:
	rm -f reloq_trace_wav
