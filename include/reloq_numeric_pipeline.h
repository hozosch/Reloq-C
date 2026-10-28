#ifndef RELOQ_NUMERIC_PIPELINE_H
#define RELOQ_NUMERIC_PIPELINE_H

#include "reloq_parameter_stream.h"
#include "reloq_session.h"

/* Render a fully specified 62-track control schedule through the typed
 * frame adapter and numeric PCM session. The caller owns the schedule,
 * session, and sink. This does not derive a schedule from text or phones.
 * Reset the session before a new utterance; the function itself preserves
 * filter state, which also permits consecutive schedule segments.
 *
 * The schedule's tick spacing determines evaluation points only. Control 0
 * determines each rendered frame's duration in milliseconds. A schedule
 * with no frames succeeds without invoking the sink. Return a
 * RELOQ_SESSION_* result and count only frames accepted by the sink.
 */
int reloq_numeric_pipeline_render(
    const struct reloq_parameter_schedule *schedule, long highest_formant,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    size_t *frames_completed);

#endif
