#include "reloq_numeric_pipeline.h"

#include "reloq_frame_adapter.h"

#include <stdint.h>

int reloq_numeric_pipeline_render(
    const struct reloq_parameter_schedule *schedule, long highest_formant,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    size_t *frames_completed)
{
    if (frames_completed) *frames_completed = 0;
    if (!session || !sink) return RELOQ_SESSION_INVALID;
    struct reloq_parameter_cursor cursor;
    if (reloq_parameter_begin(schedule, &cursor))
        return RELOQ_SESSION_INVALID;
    size_t completed = 0;
    for (;;) {
        struct reloq_parameter_frame numeric;
        int has_frame = reloq_parameter_next(&cursor, &numeric);
        if (has_frame < 0) return RELOQ_SESSION_INVALID;
        if (!has_frame) break;
        struct reloq_core_voiced_frame typed;
        if (reloq_frame_adapter_map(numeric.parameters, highest_formant,
                                    &typed))
            return RELOQ_SESSION_INVALID;
        size_t delivered = 0;
        int result = reloq_session_render_frames(session, &typed, 1, sink,
                                                  context, &delivered);
        if (delivered) {
            if (completed == SIZE_MAX) return RELOQ_SESSION_INVALID;
            completed += delivered;
            if (frames_completed) *frames_completed = completed;
        }
        if (result != RELOQ_SESSION_OK) return result;
    }
    return RELOQ_SESSION_OK;
}
