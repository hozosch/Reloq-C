#include "reloq_german_voice_defaults.h"

#include <string.h>

/* From distinct German utterances at the observed 62-control Klatt input.
 * Fields 11/12, 21-26 and 44-61 have no named target stream, but their
 * defaults still affect the native renderer. */
static const float german_controls[RELOQ_PARAMETER_TRACKS] = {
    1, 1000, 60, 50, 0, 0, 0, 0, 0, 500,
    60, 0, 0, 1500, 90, 2500, 150, 3250, 200, 3700,
    200, 5000, 500, 6300, 500, 7500, 600, 280, 90, 280,
    90, 250, 90, 250, 90, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 80, 200, 350, 500, 500, 1000,
    1500, 1500, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0
};

static const uint8_t german_field_map[RELOQ_TARGET_STREAM_COUNT] = {
    [RELOQ_TARGET_F0] = 1,
    [RELOQ_TARGET_F1] = 9,
    [RELOQ_TARGET_F2] = 13,
    [RELOQ_TARGET_F3] = 15,
    [RELOQ_TARGET_F4] = 17,
    [RELOQ_TARGET_F5] = 19,
    [RELOQ_TARGET_B1] = 10,
    [RELOQ_TARGET_B2] = 14,
    [RELOQ_TARGET_B3] = 16,
    [RELOQ_TARGET_B4] = 18,
    [RELOQ_TARGET_B5] = 20,
    [RELOQ_TARGET_FNP] = 27,
    [RELOQ_TARGET_FNZ] = 29,
    [RELOQ_TARGET_FTP] = 31,
    [RELOQ_TARGET_FTZ] = 33,
    [RELOQ_TARGET_TL] = 4,
    [RELOQ_TARGET_OQ] = 3,
    [RELOQ_TARGET_FL] = 5,
    [RELOQ_TARGET_DI] = 6,
    [RELOQ_TARGET_AV] = 2,
    [RELOQ_TARGET_AH] = 7,
    [RELOQ_TARGET_AF] = 8,
    [RELOQ_TARGET_A1F] = 35,
    [RELOQ_TARGET_A2F] = 36,
    [RELOQ_TARGET_A3F] = 37,
    [RELOQ_TARGET_A4F] = 38,
    [RELOQ_TARGET_A5F] = 39,
    [RELOQ_TARGET_AB] = 43
};

int reloq_german_voice_defaults(
    float controls[RELOQ_PARAMETER_TRACKS],
    uint8_t field_for_stream[RELOQ_TARGET_STREAM_COUNT],
    struct reloq_session_config *config)
{
    if (!controls || !field_for_stream || !config) return -1;
    const struct reloq_session_config german_config = {
        .sample_rate = RELOQ_GERMAN_VOICE_SAMPLE_RATE,
        .duration_scale = 1.0f,
        .output_gain = 2.02f,
        .initial_seed = 8,
        .source_offset = 4,
        .av_trim = -5,
        .ah_trim = 6,
        .af_trim = 0,
        .max_frame_samples = 64
    };
    memcpy(controls, german_controls, sizeof german_controls);
    memcpy(field_for_stream, german_field_map, sizeof german_field_map);
    *config = german_config;
    return 0;
}
