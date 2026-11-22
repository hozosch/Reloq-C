#ifndef RELOQ_PERSIAN_CLOCK_H
#define RELOQ_PERSIAN_CLOCK_H

#include <stddef.h>
#include <stdint.h>
#include "reloq_persian_frames.h"
#include "reloq_persian_plan.h"

#ifdef __cplusplus
extern "C" {
#endif

/* One language-owned syllable and its measured vowel nucleus. All times use
 * the provider's 100 ns clock; word numbers count native pronunciation groups,
 * not source words. A doubled affricate can cross a backend syllable boundary
 * without changing the linguistic plan or the vowel anchor. When an
 * identified phone extends into a gap around a provider syllable,
 * syllable_extended records that the returned bounds include that phone. */
struct reloq_fa_nucleus_clock {
    int32_t syllable_from,syllable_to,nucleus_from,nucleus_to;
    uint32_t word,syllable,stress,weak,nucleus,bridged_j,syllable_extended;
};

/* Experimental alignment of a Persian event plan with independently traced
 * native syllable and phone clocks and their private provider slots. It does
 * not generate or change sound. Traced slots must be from the same capture as
 * the phone clocks; the helper checks identities before selecting a vowel.
 * NULL/zero output queries the required count. The output array is unchanged
 * on every failure. Unsupported or ambiguous inputs return 1; malformed
 * arguments return -1, insufficient capacity -2, allocation failure -3. */
int reloq_fa_align_nucleus_clocks(const struct reloq_fa_plan_event *events,size_t event_count,
    const struct reloq_fa_syllable *syllables,size_t syllable_count,
    const struct reloq_fa_syllable *phones,size_t phone_count,
    const uint32_t *provider_slots,size_t slot_count,
    struct reloq_fa_nucleus_clock *output,size_t capacity,size_t *written);

/* A linguistic syllable paired with its early provider vowel interval.
 * This deliberately does not claim provider syllable or source boundaries. */
struct reloq_fa_early_nucleus {
    int32_t from,to;
    uint32_t word,syllable,stress,weak,nucleus;
};

/* A request-local cursor validates the linguistic plan once, before audio,
 * then matches each early provider vowel as the syllable is synthesized.
 * Its records are private; release every successfully created cursor.
 * A rejected input leaves the cursor and result unchanged. */
struct reloq_fa_early_cursor;
int reloq_fa_early_cursor_create(const struct reloq_fa_plan_event *events,size_t event_count,
    struct reloq_fa_early_cursor **result);
int reloq_fa_early_cursor_next(struct reloq_fa_early_cursor *cursor,
    const struct reloq_fa_syllable *nucleus,uint32_t identity,
    struct reloq_fa_early_nucleus *output);
int reloq_fa_early_cursor_finish(const struct reloq_fa_early_cursor *cursor);
void reloq_fa_early_cursor_release(struct reloq_fa_early_cursor *cursor);

/* Match early, pre-intonation vowel clocks with the owned Persian plan.
 * identities are the paired private provider vowel slots and q-release
 * counts emitted in nucleus trace mode. This is an experimental building
 * block, not a complete voice or proof of source-word boundary alignment.
 * The failure, count-query and no-partial-output contracts match the
 * independently traced alignment above. */
int reloq_fa_align_early_nuclei(const struct reloq_fa_plan_event *events,size_t event_count,
    const struct reloq_fa_syllable *nuclei,size_t nucleus_count,
    const uint32_t *identities,size_t identity_count,
    struct reloq_fa_early_nucleus *output,size_t capacity,size_t *written);

#ifdef __cplusplus
}
#endif
#endif
