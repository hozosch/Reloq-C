#ifndef RELOQ_GERMAN_INITIAL_NASAL_H
#define RELOQ_GERMAN_INITIAL_NASAL_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* The initial /m,n/ tract and source gesture for one isolated, explicitly
 * primary-stressed German SPR syllable m/n V [p|t|k], or m/n V. V is one of
 * A/E/I/O/U/a/e/i/o/u. The two members of each constant stream mark the
 * consonant start and first array-send end; FNZ has its nasal peak halfway
 * through that interval. These are 47 reusable targets across the two
 * consonants, ten following vowels, and three final-stop places.
 *
 * The caller must independently author the vowel's future tract and FNZ=200
 * targets, its AV/AH transition one tick after the returned first-send end,
 * phrase F0, and final FL/FTP/FTZ release targets before evaluating even the
 * first 62-control window. Nasal FNZ interpolates to that future target.
 * The vowel tract timing depends on later context; notably nOt transitions
 * earlier than nOp/nOk. It is not predicted by this initial-only gesture.
 * This function neither synthesizes a complete word nor claims PCM parity.
 * It is scoped to the default German 11,025-Hz reference voice.
 *
 * Returns 0 on success, 1 for an uncovered phone/stress shape, -1 for
 * invalid input or builder failure. On failure, the builder and *first_send_offset
 * remain unchanged. */
int reloq_german_initial_nasal_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t origin_tick, struct reloq_target_builder *builder,
    int64_t *first_send_offset);

#endif
