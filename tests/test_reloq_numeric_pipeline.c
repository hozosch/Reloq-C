#include "reloq_numeric_pipeline.h"

#include <stdint.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

struct captured_pcm {
    int16_t samples[128];
    size_t count;
    size_t calls;
    struct reloq_session *cancel_after_first;
};

static int capture(void *context, const int16_t *samples, size_t count)
{
    struct captured_pcm *result = context;
    if (count > 128 - result->count) return -1;
    memcpy(result->samples + result->count, samples,
           count * sizeof *samples);
    result->count += count;
    ++result->calls;
    if (result->cancel_after_first && result->calls == 1)
        reloq_session_cancel(result->cancel_after_first);
    return 0;
}

int main(void)
{
    /* The first frame has independent expected PCM from the trace-WAV
     * example at 16 kHz, seed 100 and unity output gain. The second frame
     * changes pitch via a numeric knot. Tick width deliberately differs
     * from frame duration, which is supplied by control 0. */
    static const int16_t expected_pcm[80] = {
        0, 0, 0, 2, 11, 36, 95, 208, 393, 647,
        951, 1267, 1555, 1788, 1955, 2061, 2116, 2127, 2100, 2046,
        1982, 1925, 1888, 1872, 1872, 1880, 1884, 1881, 1869, 1848,
        1816, 1772, 1712, 1639, 1554, 1462, 1367, 1267, 1158, 1035,
        895, 739, 578, 424, 283, 155, 31, -102, -261, -454,
        -686, -955, -1257, -1587, -1938, -2302, -2667, -3021, -3359,
        -3677, -3975, -4255, -4521, -4780, -5038, -5301, -5573,
        -5852, -6127, -6352, -6409, -6081, -5091, -3248, -645, 2240,
        4685, 6058, 6151, 5255
    };
    static const struct reloq_parameter_knot pitch[] = {{1, 1350}};
    struct reloq_parameter_schedule schedule = {0};
    schedule.first_tick = 0;
    schedule.end_tick = 2;
    schedule.frame_ticks = 1;
    schedule.tracks[0].baseline = 2.5f;
    schedule.tracks[1] = (struct reloq_parameter_track){1200, pitch, 1};
    schedule.tracks[2].baseline = 50;
    schedule.tracks[3].baseline = 50;
    schedule.tracks[4].baseline = 7;
    schedule.tracks[7].baseline = 18;
    schedule.tracks[8].baseline = 25;
    static const unsigned centers[8] = {9, 13, 15, 17, 19, 21, 23, 25};
    static const float frequencies[8] = {
        500, 900, 1300, 1800, 2300, 2700, 3100, 3500
    };
    for (size_t i = 0; i < 8; ++i) {
        schedule.tracks[centers[i]].baseline = frequencies[i];
        schedule.tracks[centers[i] + 1].baseline = 70 + (float)(i ? i + 1 : 0) * 10;
        schedule.tracks[35 + i].baseline = 25 - (float)i;
        schedule.tracks[44 + i].baseline = 80 + (float)i * 5;
    }
    schedule.tracks[27].baseline = 260;
    schedule.tracks[28].baseline = 75;
    schedule.tracks[29].baseline = 360;
    schedule.tracks[30].baseline = 90;
    schedule.tracks[31].baseline = 2100;
    schedule.tracks[32].baseline = 140;
    schedule.tracks[33].baseline = 2000;
    schedule.tracks[34].baseline = 150;
    schedule.tracks[43].baseline = 35.5f;

    struct reloq_session_config config = {
        .sample_rate = 16000,
        .duration_scale = 1,
        .output_gain = 1,
        .initial_seed = 100,
        .max_frame_samples = 128
    };
    struct reloq_session *session = reloq_session_create(&config);
    CHECK(session);
    struct captured_pcm result = {0};
    size_t delivered = 99;
    CHECK(reloq_numeric_pipeline_render(&schedule, 8, session, capture,
                                         &result, &delivered) ==
          RELOQ_SESSION_OK);
    CHECK(delivered == 2 && result.calls == 2 && result.count == 80);
    CHECK(!memcmp(result.samples, expected_pcm, sizeof expected_pcm));

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct captured_pcm interrupted = {.cancel_after_first = session};
    CHECK(reloq_numeric_pipeline_render(&schedule, 8, session, capture,
                                         &interrupted, &delivered) ==
          RELOQ_SESSION_CANCELLED);
    CHECK(delivered == 1 && interrupted.calls == 1 &&
          interrupted.count == 40 &&
          !memcmp(interrupted.samples, expected_pcm, 40 * sizeof(int16_t)));

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    schedule.tracks[2].baseline = 50.5f;
    struct captured_pcm invalid = {0};
    CHECK(reloq_numeric_pipeline_render(&schedule, 8, session, capture,
                                         &invalid, &delivered) ==
          RELOQ_SESSION_INVALID);
    CHECK(delivered == 0 && invalid.calls == 0);
    schedule.tracks[2].baseline = 50;
    schedule.first_tick = schedule.end_tick;
    CHECK(reloq_numeric_pipeline_render(&schedule, 8, session, capture,
                                         &invalid, &delivered) ==
          RELOQ_SESSION_OK && delivered == 0 && invalid.calls == 0);
    reloq_session_destroy(session);
    return 0;
}
