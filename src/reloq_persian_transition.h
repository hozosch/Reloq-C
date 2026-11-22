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

/* Shape remaining voiced joins, leaving closures and excluded edges intact.
 * Buffer and cumulative-offset conventions match the continuation bridge. */
int reloq_persian_bridge_remaining(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed);

/* Replace an inherited vowel at a stop-to-nasal join with the same nasal
 * onset from the continuous English context, then return to the Persian
 * nasal. The previously spoken stop and its release are never modified. */
int reloq_persian_match_stop_nasal(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary);

/* Use a matching US /z/ onset at an English-to-Persian boundary. Only the
 * frication source and bands are copied; pitch, voicing, timings and source
 * indexes remain the Persian segment's own. Return 1 when applied, 0 when a
 * matching onset cannot be verified, and -1 for invalid input. */
int reloq_persian_match_us_z_onset(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary);

/* Retain the actual H-to-Z voicing trajectory when the same English Z is
 * present in the Persian context. The US frication, timing and pitch stay
 * untouched. Refuse unrelated or unaligned context captures. */
int reloq_persian_match_h_to_us_z(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary);

/* Adopt the actual US source transition into a verified common unvoiced
 * consonant without shortening the Persian consonant or moving indexes. */
int reloq_persian_match_us_unvoiced_onset(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary,size_t reference_end,unsigned char phone);

/* Smooth only a discontinuous formant at an already identified final-R
 * language join. Do not alter the R's source, timing or steady state. */
int reloq_persian_smooth_final_r_join(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end);

/* Plan and compact voiceless onsets in-place. onsets contains an ASCII phone
 * (f, s, S, t, p, k or h), zero for a skipped segment, and bit 7 when the
 * continuous English reference is to be checked before removing its lead.
 * reference_edges has segment_count+1 frame offsets when reference is present.
 * A matching source transition is retained. If the reference has no actual
 * consonant, a bounded, source-continuous native-shaped transition can also
 * be retained; an incompatible carrier is removed. Counts and elapsed
 * removals describe the compacted segments. */
int reloq_persian_plan_unvoiced_onsets(struct reloq_fa_frame *frames,size_t count,
    const size_t *ends,size_t segments,const unsigned char *onsets,
    const struct reloq_fa_frame *reference,size_t reference_count,
    const size_t *reference_edges,size_t *counts,double *removed,size_t *new_count);

#endif
