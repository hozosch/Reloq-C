#include "reloq_frame_adapter.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

/* A complete 62-control frame from the numeric example independently
 * rendered by the trace CLI. Its first 40 PCM values below are a held-out
 * integration result, so an index shift produces audible sample differences
 * even when a mistaken field happens to remain within the valid range. */
static const float reference_controls[RELOQ_PARAMETER_TRACKS] = {
    [0] = 2.5f, [1] = 1200, [2] = 50, [3] = 50, [4] = 7,
    [7] = 18, [8] = 25,
    [9] = 500, [10] = 70, [13] = 900, [14] = 90,
    [15] = 1300, [16] = 100, [17] = 1800, [18] = 110,
    [19] = 2300, [20] = 120, [21] = 2700, [22] = 130,
    [23] = 3100, [24] = 140, [25] = 3500, [26] = 150,
    [27] = 260, [28] = 75, [29] = 360, [30] = 90,
    [31] = 2100, [32] = 140, [33] = 2000, [34] = 150,
    [35] = 25, [36] = 24, [37] = 23, [38] = 22,
    [39] = 21, [40] = 20, [41] = 19, [42] = 18,
    [43] = 35.5f,
    [44] = 80, [45] = 85, [46] = 90, [47] = 95,
    [48] = 100, [49] = 105, [50] = 110, [51] = 115
};

static const int16_t reference_pcm[] = {
    0, 0, 0, 2, 11, 36, 95, 208, 393, 647,
    951, 1267, 1555, 1788, 1955, 2061, 2116, 2127, 2100, 2046,
    1982, 1925, 1888, 1872, 1872, 1880, 1884, 1881, 1869, 1848,
    1816, 1772, 1712, 1639, 1554, 1462, 1367, 1267, 1158, 1035
};

/* First one-millisecond frame of Apple ECI German `Hallo Welt.` at 11,025 Hz,
 * captured before its Klatt renderer. The PCM below is the separately
 * captured Apple callback, after that voice's calibrated output gain. This
 * is an actual language-scheduled frame, with nonzero AH and zero AV/AF. */
static const float natural_controls[RELOQ_PARAMETER_TRACKS] = {
    [0] = 1, [1] = 1156, [3] = 56, [7] = 42,
    [9] = 780, [10] = 120, [13] = 1160, [14] = 100,
    [15] = 2500, [16] = 150, [17] = 3600, [18] = 330,
    [19] = 3900, [20] = 260, [21] = 5000, [22] = 500,
    [23] = 6300, [24] = 500, [25] = 7500, [26] = 600,
    [27] = 200, [28] = 90, [29] = 200, [30] = 90,
    [31] = 250, [32] = 90, [33] = 250, [34] = 90,
    [44] = 80, [45] = 200, [46] = 350, [47] = 500,
    [48] = 500, [49] = 1000, [50] = 1500, [51] = 1500
};
static const int16_t natural_pcm[11] = {
    -228, -420, -169, 52, -14, 16, 161, 115, 117, 401, 456
};

/* First Apple frame of `Susi kocht Kaffee.` exercises simultaneous AV and
 * AF, parallel branches four and five, and glottal source profile 35. */
static const float natural_mixed_controls[RELOQ_PARAMETER_TRACKS] = {
    [0] = 1, [1] = 1153, [2] = 20, [3] = 56, [4] = 35, [8] = 66,
    [9] = 300, [10] = 120, [13] = 1210, [14] = 100,
    [15] = 2600, [16] = 150, [17] = 3700, [18] = 330,
    [19] = 4200, [20] = 260, [21] = 5000, [22] = 500,
    [23] = 6300, [24] = 500, [25] = 7500, [26] = 600,
    [27] = 200, [28] = 90, [29] = 200, [30] = 90,
    [31] = 250, [32] = 90, [33] = 250, [34] = 90,
    [38] = 50, [39] = 50,
    [44] = 80, [45] = 200, [46] = 350, [47] = 500,
    [48] = 500, [49] = 1000, [50] = 1500, [51] = 1500
};
static const int16_t natural_mixed_pcm[11] = {
    -60, 224, -335, 220, 38, -202, 208, -115, 153, -165, 64
};

static int unchanged_on_error(const float input[RELOQ_PARAMETER_TRACKS],
                              long highest)
{
    struct reloq_core_voiced_frame output;
    memset(&output, 0xa5, sizeof output);
    struct reloq_core_voiced_frame saved = output;
    return reloq_frame_adapter_map(input, highest, &output) == -1 &&
           memcmp(&output, &saved, sizeof output) == 0;
}

int main(void)
{
    struct reloq_parameter_frame schedule_output = {0};
    schedule_output.tick = 1234;
    schedule_output.duration_ticks = 77; /* not a duration in milliseconds */
    memcpy(schedule_output.parameters, reference_controls,
           sizeof reference_controls);
    struct reloq_core_voiced_frame frame;
    CHECK(reloq_frame_adapter_map(schedule_output.parameters, 8, &frame) == 0);
    CHECK(frame.noise.duration_ms == 2.5f &&
          frame.noise.tract.highest_formant == 8 &&
          frame.noise.tract.oral[0].frequency_hz == 500.0f &&
          frame.noise.tract.oral[1].frequency_hz == 900.0f &&
          frame.noise.tract.oral[7].frequency_hz == 3500.0f &&
          frame.noise.af_levels[7] == 18 &&
          frame.noise.bypass_level == 35.5f);

    struct reloq_core_voiced_frame_state synth;
    float output[40], af[40], scratch[40];
    uint8_t closed[40];
    int16_t pcm[40];
    size_t written = 0;
    CHECK(reloq_core_voiced_frame_initialize(&synth, 16000.0f, 100, 0, 0, 0) == 0);
    CHECK(reloq_core_voiced_frame_render(&synth, &frame, output, af,
                                         scratch, closed, pcm, 40,
                                         &written) == 0);
    CHECK(written == 40 &&
          !memcmp(pcm, reference_pcm, sizeof reference_pcm));

    CHECK(reloq_frame_adapter_map(natural_controls, 5, &frame) == 0);
    CHECK(reloq_core_voiced_frame_initialize(&synth, 11025.0f,
                                             8, 4, 6, 0) == 0);
    CHECK(reloq_core_voiced_frame_set_av_trim(&synth, -5) == 0);
    CHECK(reloq_core_voiced_frame_render(&synth, &frame, output, af,
                                         scratch, closed, pcm, 40,
                                         &written) == 0);
    CHECK(written == 11);
    int64_t scaled[11];
    for (size_t i = 0; i < written; ++i)
        scaled[i] = (int64_t)output[i];
    reloq_core_scale_output(scaled, (long)written, 2.02f);
    reloq_core_pcm16_from_i64(scaled, pcm, written);
    CHECK(!memcmp(pcm, natural_pcm, sizeof natural_pcm));

    CHECK(reloq_frame_adapter_map(natural_mixed_controls, 5, &frame) == 0);
    CHECK(reloq_core_voiced_frame_initialize(&synth, 11025.0f,
                                             8, 4, 6, 0) == 0);
    CHECK(reloq_core_voiced_frame_set_av_trim(&synth, -5) == 0);
    CHECK(reloq_core_voiced_frame_render(&synth, &frame, output, af,
                                         scratch, closed, pcm, 40,
                                         &written) == 0);
    CHECK(written == 11);
    for (size_t i = 0; i < written; ++i)
        scaled[i] = (int64_t)output[i];
    reloq_core_scale_output(scaled, (long)written, 2.02f);
    reloq_core_pcm16_from_i64(scaled, pcm, written);
    CHECK(!memcmp(pcm, natural_mixed_pcm, sizeof natural_mixed_pcm));

    float invalid[RELOQ_PARAMETER_TRACKS];
    memcpy(invalid, reference_controls, sizeof invalid);
    CHECK(unchanged_on_error(invalid, 0) && unchanged_on_error(invalid, 9));
    CHECK(reloq_frame_adapter_map(NULL, 8, &frame) == -1);
    CHECK(reloq_frame_adapter_map(invalid, 8, NULL) == -1);
    invalid[61] = NAN; /* unmapped controls must still be finite */
    CHECK(unchanged_on_error(invalid, 8));
    invalid[61] = 0;
    invalid[2] = 50.5f;
    CHECK(unchanged_on_error(invalid, 8));
    invalid[2] = 50;
    invalid[0] = 0;
    CHECK(unchanged_on_error(invalid, 8));
    invalid[0] = 2.5f;
    invalid[4] = 35.5f;
    CHECK(unchanged_on_error(invalid, 8));
    invalid[4] = 54;
    CHECK(reloq_frame_adapter_map(invalid, 8, &frame) == 0 &&
          frame.glottal_profile == 35);
    invalid[4] = 101;
    CHECK(unchanged_on_error(invalid, 8));
    invalid[4] = 7;
    invalid[1] = 0;
    CHECK(unchanged_on_error(invalid, 8));
    invalid[1] = 1200;
    invalid[3] = 101;
    CHECK(unchanged_on_error(invalid, 8));
    invalid[3] = 50;
    invalid[6] = 100;
    CHECK(reloq_frame_adapter_map(invalid, 8, &frame) == 0 &&
          frame.diplophonia == 100.0f);
    invalid[6] = nextafterf(100.0f, INFINITY);
    CHECK(unchanged_on_error(invalid, 8));
    invalid[6] = 0;
    invalid[28] = -1;
    CHECK(unchanged_on_error(invalid, 8));
    invalid[28] = 75;
    invalid[44] = -1;
    CHECK(unchanged_on_error(invalid, 8));
    invalid[44] = 80;
    invalid[43] = 1e10f;
    CHECK(unchanged_on_error(invalid, 8));

    memcpy(invalid, reference_controls, sizeof invalid);
    invalid[2] = 0;
    invalid[1] = 0;
    invalid[3] = 0;
    CHECK(reloq_frame_adapter_map(invalid, 3, &frame) == 0 &&
          frame.av_level == 0 && frame.noise.tract.highest_formant == 3);
    return 0;
}
