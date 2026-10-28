#ifndef RELOQ_GERMAN_HV_TIMING_H
#define RELOQ_GERMAN_HV_TIMING_H

#include "reloq_german_phone_sequence.h"

#include <stdint.h>

/* Predict the first vowel FORMANT target's acoustic anchor for one isolated,
 * explicitly stressed German SPR syllable hV or hV[p/t/k], whose h origin is
 * tick zero. V ranges over A/E/I/O/U/a/e/i/o/u. The default 11,025-Hz
 * German reference has a nominal 1 ms tick. The first AV-positive target is
 * one tick later. This is an observed acoustic landmark, not a validated
 * linguistic phone duration or a general word scheduler.
 *
 * All nodes must come from one word and syllable; punctuation, pauses and
 * controls outside the SPR group are excluded by the caller. Returns 0 on a
 * covered pattern, 1 on an uncovered valid pattern, and -1 on invalid input.
 * On 1 or -1, onset_tick is unchanged. */
int reloq_german_hv_acoustic_anchor(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *onset_tick);

/* For the same isolated hV context with a final p, t or k, return the first
 * F1 target whose value differs from the vowel's initial F1 value. That is
 * a coda-transition landmark, not the onset of closure, a stop burst or an
 * internally established phone boundary. Supported stress is explicit
 * 0/1/2; the window/target onset is independently measured and does not
 * use sendArrayParameters boundaries. Same return/unchanged conventions. */
int reloq_german_hvc_f1_target_anchor(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *coda_f1_tick);

/* The matching isolated V[p/t/k] shapes have the same relative F1/AV=0
 * target offset, with the vowel's initial F1 target at tick zero. This
 * variant needs no h gesture and returns that coda target tick directly. */
int reloq_german_vc_f1_target_anchor(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *coda_f1_tick);

/* Common first vowel-send hold lead before a final p/t/k. Independently
 * checked for isolated Vc, hVc and stop-initial CVc at primary stress.
 * Return 0 on covered phones, 1 if uncovered, -1 for a NULL output. */
int reloq_german_coda_hold_lead(
    enum reloq_german_phone vowel, enum reloq_german_phone stop,
    int64_t *lead_ticks);

/* In the isolated hV[p/t/k] reference shape, predict the end of the first
 * sendArrayParameters range. This is a numeric scheduler endpoint, not the
 * vowel or stop's linguistic duration. Derived independently of punctuation
 * targets and checked at explicit stress 0/1/2 for final . , ? ! : ; ... .
 * The caller must use precisely one syllable with no intervening controls.
 * Return/unchanged semantics match the other functions. */
int reloq_german_hvc_first_send_end(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *send_end_tick);

/* The analogous isolated V[p/t/k] first numeric send ends at its closure
 * landmark minus the same vowel/place dependent lead. There is no h onset;
 * the first vowel-formant target is tick zero. Restricted to the same one
 * explicit-stress syllable; this remains a numeric-window landmark. */
int reloq_german_vc_first_send_end(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *send_end_tick);

#endif
