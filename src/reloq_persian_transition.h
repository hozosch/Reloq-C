/* Cross-platform planning of the selected Persian/English frame transitions.
 * The caller owns the input, output and all request-local timing data. */
#ifndef RELOQ_PERSIAN_TRANSITION_H
#define RELOQ_PERSIAN_TRANSITION_H

#include <stddef.h>
#include <stdint.h>
#include "reloq_persian_frames.h"

/* ends contains one cumulative frame count per segment, times contains
 * frame_count+1 cumulative times, and skip/changed contain segment_count
 * bytes. The caller preselects the eligible English-to-Persian boundaries.
 * Input and output must not overlap. Output is populated only when at least
 * one boundary changes. No results survive this call. */
int reloq_persian_bridge_continuation(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output, const double *times, const size_t *ends,
    const unsigned char *skip, size_t frame_count, size_t segment_count,
    double window_ms, unsigned char *changed);

/* Smooth a voiced Persian onset after a verified English stop closure.
 * Eligibility is selected by the caller from the actual phonemes. */
int reloq_persian_bridge_stop_onsets(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed);

/* Replace an inherited vowel at a stop-to-nasal join with the same nasal
 * onset from the continuous English context, then return to the Persian
 * nasal. The previously spoken stop and its release are never modified. */
int reloq_persian_match_stop_nasal(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary);

#endif
