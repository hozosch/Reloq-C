/* Context-sensitive English phonetic corrections, shared by both dialects. */
#ifndef RELOQ61_ENGLISH_PHONETICS_H
#define RELOQ61_ENGLISH_PHONETICS_H
#include <stdint.h>

/* Restore the ordinary /S/ noise target for the exceptional /t.S.r/ path. */
void reloq61_english_shr_noise(void *state);

/* Give adjacent schwas next to /w/ their own neutral oral-tract target. */
void reloq61_english_schwa_timbre(void *state);

/* Shorten the native closure of /p t k/ following /s/, before insertion. */
void reloq61_english_stop_duration(void *state);

/* Pending English release, scoped to a synthesis transaction. */
struct reloq61_english_stop_scope {
    void *state;
    int32_t stop, following, anchor, aspiration_begin;
    int16_t release_level, blended;
};
struct reloq61_english_stop_scope reloq61_english_stop_scope(
    struct reloq61_english_stop_scope next);

struct reloq61_english_release_guard {
    int32_t end;
    int16_t level;
    int active;
};
void reloq61_english_stop_prepare(void *state);
void reloq61_english_stop_release_begin(void *state,
    struct reloq61_english_release_guard *guard);
void reloq61_english_stop_release_end(void *state,
    const struct reloq61_english_release_guard *guard, int32_t result);
void reloq61_english_stop_interval(void *state,int16_t stream,int16_t value,
    int32_t *begin,int32_t *end);

#endif
