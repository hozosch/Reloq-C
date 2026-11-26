/* Context-sensitive English phonetic corrections, shared by both dialects. */
#ifndef RELOQ61_ENGLISH_PHONETICS_H
#define RELOQ61_ENGLISH_PHONETICS_H
#include <stdint.h>

/* Restore the ordinary /S/ noise target for the exceptional /t.S.r/ path. */
void reloq61_english_shr_noise(void *state);

/* Give adjacent schwas next to /w/ their own neutral oral-tract target. */
void reloq61_english_schwa_timbre(void *state);

#endif
