#ifndef RELOQ61_H
#define RELOQ61_H
#include <stddef.h>
#include <stdint.h>
/* Adjacent sparse endpoints. Evaluate val_l + (at-at_l)*(val_r-val_l) /
 * (at_r-at_l) in single precision, then truncate the complete value. */
struct reloq61_curve { int32_t at_l, val_l, at_r, val_r; };
typedef int (*reloq61_span_sink)(void *,int32_t from,int32_t to,
    const struct reloq61_curve curves[62],int32_t volume);
struct reloq61_mark { size_t offset; int32_t index, pitch; };
typedef int (*reloq61_frame_sink)(void *, const int32_t frame[62], int32_t volume);
typedef void (*reloq61_index_sink)(void *, int32_t index, int64_t tick);
struct reloq61_request {
    int language, annotations, phonemes_only;
    int voice[8];
    const char *text;
    const struct reloq61_mark *marks;
    size_t mark_count;
    reloq61_frame_sink frame;
    reloq61_index_sink index;
    void *context;
    char *phonemes;
    size_t phonemes_capacity;
    /* Optional compact 1-ms runs; frame remains the general fallback. */
    reloq61_span_sink span;
    /* Acoustic rule profile, independent of the host's PCM clock. Zero is
     * the established 11025-Hz profile; 8000 selects the narrow-band model. */
    int sample_rate;
    /* Physical host clock for independently requested source balancing;
     * it does not select a different original acoustic-rule profile. */
    int output_rate;
};
/* Lossless German onset facts collected during the existing phoneme pass.
 * The caller owns this scoped sink; no word or synthesis result is retained. */
struct reloq61_german_word_facts { uint64_t glottal_syllables; unsigned syllables; };
struct reloq61_german_capture {
    struct reloq61_german_word_facts *words;
    size_t count,capacity;
    int overflow;
};
struct reloq61_german_capture *reloq61_german_capture_scope(struct reloq61_german_capture *);
void reloq61_german_word_begin(void);
void reloq61_german_syllable_begin(void);
struct reloq61_result { int unsupported, error; size_t frames, phonemes_length; int64_t ticks; };
/* CP1252 input; synchronous and independent of the host audio device.
 * 0 success, 1 unsupported plan (discard all results), -1 failure. */
int reloq61_generate(const struct reloq61_request *, struct reloq61_result *);
/* Optional owner-scoped worker state; no speech-result reuse. The ECI owner serializes requests,
 * restores the preceding scope after each utterance, and destroys the worker
 * after all callbacks and cancellation have joined. No process-global engine. */
void *reloq61_worker_new(int language);
void reloq61_worker_free(void *);
void *reloq61_worker_scope(void *);
int reloq61_worker_dictionary(void *,const char *);
size_t reloq61_dictionary_match(const unsigned char *,size_t);
uint64_t reloq61_dictionary_hits(void);
int reloq61_labiodental_gain(void *state);
int reloq61_language_supported(int language);
#endif
