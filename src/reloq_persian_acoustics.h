#ifndef RELOQ_PERSIAN_ACOUSTICS_H
#define RELOQ_PERSIAN_ACOUSTICS_H

#include <stddef.h>
#include <stdint.h>

int reloq_fa_acoustic_scope(int active);
/* Query the calling thread without changing its acoustic scope. */
int reloq_fa_acoustic_active(void);
int reloq_fa_native_onsets(const uint32_t *words, size_t count);
int reloq_fa_native_geminates(const uint32_t *pairs, size_t count);

#endif
