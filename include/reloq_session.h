#ifndef RELOQ_SESSION_H
#define RELOQ_SESSION_H

#include "reloq_core.h"

#include <stddef.h>
#include <stdint.h>

/* A numeric-frame-to-PCM session for a future audio host. The language
 * front end must supply fully scheduled Klatt frames. This interface does
 * not accept text or implement pronunciation, an ECI API, or NVDA itself.
 */
struct reloq_session;

struct reloq_session_config {
    float sample_rate;
    float duration_scale;
    float output_gain;
    int32_t initial_seed;
    long source_offset;
    long av_trim;
    long ah_trim;
    long af_trim;
    /* Maximum number of samples in one numeric frame. This is a scratch
     * allocation limit, not a limit on utterance or text length. */
    size_t max_frame_samples;
    /* Optional physical reference for rescaling a sampled source filter. */
    float glottal_reference_rate;
    unsigned reference_formants;
    float frication_reference_rate;
};

enum reloq_session_result {
    RELOQ_SESSION_OK = 0,
    RELOQ_SESSION_CANCELLED = 1,
    RELOQ_SESSION_INVALID = -1,
    RELOQ_SESSION_MEMORY = -2,
    RELOQ_SESSION_RENDER = -3,
    RELOQ_SESSION_SINK = -4
};

/* Called synchronously once for each completed frame. The PCM pointer is
 * valid only during this call; a playback queue must copy samples before
 * returning. Return zero to accept the frame, nonzero to stop rendering.
 * The callback may call reloq_session_cancel(), but not reset/destroy or
 * recursively render the same session. */
typedef int (*reloq_session_pcm_sink)(void *context, const int16_t *pcm,
                                      size_t samples);

/* Returns NULL on invalid config or allocation failure. The calling thread
 * owns reset, render and destroy. Only cancel and cancelled may concurrently
 * run on another thread. destroy requires the owner to join that thread or
 * otherwise guarantee that no cancel call remains in progress. */
struct reloq_session *reloq_session_create(
    const struct reloq_session_config *config);
void reloq_session_destroy(struct reloq_session *session);
/* Immutable output clock, available to text/index planners. */
float reloq_session_sample_rate(const struct reloq_session *session);
/* Update absolute source levels for a word-scoped voice model. Histories,
 * phase and configured reset defaults stay intact. Serialized render only. */
int reloq_session_source_levels(struct reloq_session *session,long source,
    long av,long ah,long af,unsigned reference_formants);

/* Begin another numeric utterance with fresh synthesis histories and seeds.
 * Scratch buffers remain allocated for low-latency reuse. A reset is needed
 * after cancellation or sink/render failure before a new utterance starts.
 * A concurrent cancel increments the generation; reset never clears it.
 */
int reloq_session_reset(struct reloq_session *session);
void reloq_session_cancel(struct reloq_session *session);
int reloq_session_cancelled(const struct reloq_session *session);

/* Render each frame and deliver PCM to sink. Check cancellation between
 * frames and before delivery. An in-flight numeric frame cannot be
 * interrupted inside the core renderer; a concurrently cancelled frame may
 * therefore finish internally without being delivered. frames_completed
 * counts only frames already accepted by sink, and may be NULL. A callback
 * can race a cancel immediately after the pre-delivery check; hosts should
 * discard obsolete buffers using their own utterance generation as well.
 */
int reloq_session_render_frames(struct reloq_session *session,
    const struct reloq_core_voiced_frame *frames, size_t count,
    reloq_session_pcm_sink sink, void *context, size_t *frames_completed);

#endif
