#ifndef RELOQ_PERSIAN_FRAME_INTERNAL_H
#define RELOQ_PERSIAN_FRAME_INTERNAL_H

#include <stddef.h>
#include <stdint.h>
#include "reloq_persian_frames.h"

struct reloq_fa_tuning_filter;
void reloq_fa_tuning_prepare_revision(struct reloq_fa_tuning_filter *,const float controls[62],int rate);

/* Capture storage is thread-local and released after each request. */
int reloq_fa_capture_begin(size_t maximum);
const struct reloq_fa_frame *reloq_fa_capture_end(size_t *count, int *failed);
const struct reloq_fa_frame *reloq_fa_capture_current(size_t *count, int *failed);
void reloq_fa_capture_release(void);
void reloq_fa_capture_syllable(void *context,int32_t from,int32_t to);
const struct reloq_fa_syllable *reloq_fa_capture_syllables(size_t *count,int *failed);
int reloq_fa_capture_set_controls_only(int enabled);
int reloq_fa_capture_set_pcm_storage(int enabled);
int reloq_fa_capture_pcm_storage(void);
int reloq_fa_capture_append_pcm(const int16_t *samples,size_t count);
const int16_t *reloq_fa_capture_pcm(size_t *count);
int64_t reloq_fa_last_audible_sample(const unsigned char *pcm,size_t bytes);
int reloq_fa_frame_ticks(const struct reloq_fa_frame *rows,size_t count,int64_t *ticks);
int reloq_fa_frame_timeline(const struct reloq_fa_frame *rows,size_t count,double *times);
int reloq_fa_donor_span(const struct reloq_fa_frame *rows,size_t count,
    size_t leading,size_t syllables,size_t trailing,size_t *first,size_t *last);
void reloq_fa_recompute_frication(struct reloq_fa_frame *rows,size_t count);
int reloq_fa_bridge_continuation(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed);
int reloq_fa_dense_curve(const struct reloq_fa_frame *rows,size_t count,
    unsigned column,float *out,size_t length);
int reloq_fa_fit_pitch(struct reloq_fa_frame *rows,size_t count,const float *pilot,
    size_t length,int64_t start,int64_t end,int64_t a,int64_t b);
int reloq_fa_fit_pitch_anchors(struct reloq_fa_frame *rows,size_t count,const float *pilot,
    size_t length,int64_t start,int64_t end,int64_t a,int64_t b,
    const int64_t *target,const int64_t *source,size_t anchors);
int reloq_fa_transfer_us_words(struct reloq_fa_frame *target,size_t target_count,
    const struct reloq_fa_frame *donor,size_t donor_count,const int64_t *target_bounds,
    const int64_t *donor_bounds,size_t word_count,const struct reloq_fa_syllable *target_syllables,
    size_t target_syllable_count,const struct reloq_fa_syllable *donor_syllables,
    size_t donor_syllable_count,const uint32_t *syllables_per_word);
int reloq_fa_fit_pitch_edges(struct reloq_fa_frame *rows,size_t count,const float *pilot,
    size_t length,const float *original,int64_t start,int64_t end,int64_t a,int64_t b,
    unsigned edges,int64_t window);
int reloq_fa_transfer_korean_pitch(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_frame *donor,size_t donor_count,unsigned edges,int64_t window);
int reloq_fa_transfer_korean_aligned(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_frame *donor,size_t donor_count,const int64_t *word_ticks,
    const uint32_t *syllables,size_t word_count);
int reloq_fa_capture_controls_only(void);
int reloq_fa_capture_active(void);
int reloq_fa_capture_advance_samples(size_t count);
size_t reloq_fa_capture_sample_count(void);
int reloq_fa_capture_append(int64_t tick, const float controls[62],
    float duration, float gain, int formants, int head, int gender,
    int s_frication, int limit_frication);

#endif
