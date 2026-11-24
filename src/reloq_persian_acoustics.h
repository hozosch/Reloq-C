#ifndef RELOQ_PERSIAN_ACOUSTICS_H
#define RELOQ_PERSIAN_ACOUSTICS_H

#include <stddef.h>
#include <stdint.h>

int reloq_fa_acoustic_scope(int active);
/* Query the calling thread without changing its acoustic scope. */
int reloq_fa_acoustic_active(void);
/* Listening variant of the Persian sound, shared by all threads: 0 is the
 * previous sound and the default, 1 to 3 are candidates (see the source). The setter returns the previous
 * value, or -1 for an unknown variant. */
int reloq_fa_set_sound_variant(int variant);
int reloq_fa_sound_variant(void);
int reloq_fa_native_onsets(const uint32_t *words, size_t count);
int reloq_fa_native_geminates(const uint32_t *pairs, size_t count);

#endif
