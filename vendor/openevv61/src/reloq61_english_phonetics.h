/* Context-sensitive English phonetic corrections, shared by both dialects. */
#ifndef RELOQ61_ENGLISH_PHONETICS_H
#define RELOQ61_ENGLISH_PHONETICS_H
#include <stdint.h>

/* Restore the ordinary /S/ noise target for the exceptional /t.S.r/ path. */
void reloq61_english_shr_noise(void *state);

/* Give adjacent schwas next to /w/ their own neutral oral-tract target. */
void reloq61_english_schwa_timbre(void *state);

/* Keep the ordinary s/z noise colour before the native r transition. */
void reloq61_english_s_z_r(void *state);
struct reloq61_english_sr_scope {
    void *state;
    int32_t following_r;
    uint16_t streams;
};
struct reloq61_english_sr_scope reloq61_english_sr_scope(
    struct reloq61_english_sr_scope next);
/* Shape post-s t/p releases from the corresponding ordinary stops. */
void reloq61_english_tp_tract(void *state);

/* Pending English release, scoped to a synthesis transaction. */
struct reloq61_english_stop_scope {
    void *state;
    int32_t stop, following, anchor, aspiration_begin;
    int32_t s_node, s_end, s_cut;
    int32_t closure_begin, closure_end, closure_cut;
    int32_t tract_start, tract_earliest;
    int16_t release_level;
    uint16_t frication_seen, closure_valid, tract_enabled, tract_held, tract_followed;
    int16_t tract_values[10];
};
struct reloq61_english_stop_scope reloq61_english_stop_scope(
    struct reloq61_english_stop_scope next);

struct reloq61_english_release_guard {
    int32_t end;
    int16_t level;
    int active;
};
void reloq61_english_stop_prepare(void *state);
void reloq61_english_stop_watch(void *state);
void reloq61_english_stop_release_begin(void *state,
    struct reloq61_english_release_guard *guard);
void reloq61_english_stop_release_end(void *state,
    const struct reloq61_english_release_guard *guard, int32_t result);
void reloq61_english_stop_interval(void *state,int16_t stream,int16_t value,
    int32_t *begin,int32_t *end);

#endif
