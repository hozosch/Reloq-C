#ifndef RELOQ_GERMAN_VOICE_DEFAULTS_H
#define RELOQ_GERMAN_VOICE_DEFAULTS_H

#include "reloq_parameter_stream.h"
#include "reloq_session.h"
#include "reloq_target_builder.h"

#include <stdint.h>

/* Measured default German Apple ECI 6.1 voice at 11,025 Hz. These scalar
 * controls and target-field positions were observed at the public numeric
 * synthesis boundary and cross-checked across independent German utterances;
 * no reference executable or table is linked into this implementation.
 *
 * The original array object's field 0 default is 5 ms, but an actual
 * sendArrayParameters call replaces it with its 1-ms step. This initializer
 * returns the effective frame control (1.0f), ready for the C curve renderer.
 * It does not supply pronunciation, source targets or prosody. A caller must
 * render with five active oral formants and use one target tick per frame.
 * Higher sample rates and other voices require separately measured profiles.
 *
 * Returns 0, or -1 for NULL. On error all outputs remain unchanged. */
enum {
    RELOQ_GERMAN_VOICE_SAMPLE_RATE = 11025,
    RELOQ_GERMAN_VOICE_FRAME_TICKS = 1,
    RELOQ_GERMAN_VOICE_FORMANTS = 5
};

int reloq_german_voice_defaults(
    float controls[RELOQ_PARAMETER_TRACKS],
    uint8_t field_for_stream[RELOQ_TARGET_STREAM_COUNT],
    struct reloq_session_config *config);

#endif
