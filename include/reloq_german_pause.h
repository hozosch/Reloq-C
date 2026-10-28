#ifndef RELOQ_GERMAN_PAUSE_H
#define RELOQ_GERMAN_PAUSE_H

#include "reloq_german_phone_sequence.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_target_builder.h"

/* The release belongs to the preceding phone; the quiet body belongs to
 * punctuation. Negative source values mean no new target, not silence. */
struct reloq_german_pause {
    int32_t release_ticks, quiet_ticks;
    int32_t av, ah, voiced_ticks;
};

int reloq_german_pause_plan(const struct reloq_german_phone_node *phone,
    int nucleus_carry, enum reloq_german_terminal_mark mark, int continuing,
    struct reloq_german_pause *out);

typedef int (*reloq_german_pause_emit)(void *, enum reloq_target_stream,
                                      int64_t, int32_t, size_t);
/* Emits release source gestures only. A pause contributes no F1/F2/F3
 * knots: those tracks connect the two neighboring phone targets directly. */
int reloq_german_pause_release(const struct reloq_german_pause *plan,
    int64_t begin, size_t phone, reloq_german_pause_emit emit, void *context);

#endif
