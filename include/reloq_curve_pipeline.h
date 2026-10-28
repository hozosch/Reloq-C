#ifndef RELOQ_CURVE_PIPELINE_H
#define RELOQ_CURVE_PIPELINE_H

#include "reloq_acoustic_curve.h"
#include "reloq_parameter_stream.h"
#include "reloq_session.h"

/* A caller-scheduled set of 62 acoustic target curves. Frames are evaluated
 * at first_tick, first_tick + frame_ticks, ... before end_tick (exclusive).
 * The tick unit and stride select control values only: track 0 supplies the
 * rendered frame duration in milliseconds, independently of frame_ticks.
 * Curves and their points must remain immutable and alive during render.
 */
struct reloq_curve_schedule {
    int64_t first_tick;
    int64_t end_tick;
    uint32_t frame_ticks;
    struct reloq_acoustic_curve tracks[RELOQ_PARAMETER_TRACKS];
    /* Independent extension control, never stored in the original 62 slots.
     * Values are centidB of AB high-frequency attenuation. */
    const struct reloq_acoustic_curve *bypass_attenuation;
    const struct reloq_acoustic_curve *s_frication; /* step-valued s/z/t ownership */
    int s_frication_profile; /* zero: current, 1: accepted German 0.2.84 blend */
};

/* Evaluate each track at the same frame tick, map the 62 controls to a typed
 * Klatt frame, then pass it to a reusable PCM session. Duplicate curve points
 * retain reloq_acoustic_value's first-at-equality, last-on-following-tick
 * semantics. The caller resets the session before a new utterance; consecutive
 * calls may instead append segments while retaining synthesis history.
 *
 * Return a RELOQ_SESSION_* result. frames_completed, if non-NULL, counts
 * frames accepted by the sink, including one whose callback cancelled the
 * session. On a failure or cancellation, pending frames are not delivered.
 * A zero-frame schedule succeeds without calling the sink. No text, phoneme
 * scheduling, Apple library or host audio API is involved.
 */
int reloq_curve_pipeline_render(
    const struct reloq_curve_schedule *schedule, long highest_formant,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    size_t *frames_completed);

/* Profile hook for explicitly extended sample-rate voices. No time-axis change. */
typedef void (*reloq_frame_profile)(void *context,float controls[RELOQ_PARAMETER_TRACKS]);
typedef long (*reloq_frame_formants)(void *context);
/* A word-scoped anatomy change can alter the active cascade count without
 * resetting oscillator/filter history or splitting the utterance. */
int reloq_curve_pipeline_render_models(
    const struct reloq_curve_schedule *schedule,long highest_formant,
    struct reloq_session *session,reloq_session_pcm_sink sink,void *context,
    size_t *frames_completed,reloq_frame_profile profile,void *profile_context,
    reloq_frame_formants formants);
int reloq_curve_pipeline_render_profile(
    const struct reloq_curve_schedule *schedule,long highest_formant,
    struct reloq_session *session,reloq_session_pcm_sink sink,void *context,
    size_t *frames_completed,reloq_frame_profile profile,void *profile_context);
#endif
