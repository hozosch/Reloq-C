#include "reloq_curve_pipeline.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

/* Held-out PCM from tools/example.trace, rendered by the independent trace
 * WAV path: three 2.5 ms frames at 16 kHz, seed 100, unity gain. Frame two
 * changes the source and tract; frame three turns AV off while its filters
 * retain history. The curve schedule below must reproduce these samples. */
static const int16_t expected_pcm[120] = {
    0, 0, 0, 2, 11, 36, 95, 208, 393, 647,
    951, 1267, 1555, 1788, 1955, 2061, 2116, 2127, 2100, 2046,
    1982, 1925, 1888, 1872, 1872, 1880, 1884, 1881, 1869, 1848,
    1816, 1772, 1712, 1639, 1554, 1462, 1367, 1267, 1158, 1035,
    894, 738, 576, 420, 277, 143, 6, -150, -341, -571,
    -835, -1128, -1445, -1782, -2135, -2497, -2855, -3197,
    -3516, -3813,
    -4090, -4353, -4605, -4852, -5097, -5347, -5604, -5869,
    -6129, -6326, -6319, -5849, -4601, -2385, 622, 3801,
    6281, 7382, 6984, 5540, 3737, 2059, 628, -639, -1744,
    -2494, -2640, -2107, -1100, -1, 846, 1306, 1457, 1481,
    1514, 1581, 1612, 1539, 1375, 1217, 1185, 1322, 1549,
    1700, 1626, 1312, 917, 707, 903, 1536, 2385, 3076,
    3259, 2796, 1806, 556, -703, -1859, -2893, -3782
};

struct capture {
    int16_t pcm[120];
    size_t samples;
    size_t calls;
    size_t cancel_after;
    struct reloq_session *session;
    int reject;
};

static int receive_pcm(void *context, const int16_t *pcm, size_t count)
{
    struct capture *capture = context;
    if (capture->reject || count > 120 - capture->samples) return -1;
    memcpy(capture->pcm + capture->samples, pcm, count * sizeof *pcm);
    capture->samples += count;
    ++capture->calls;
    if (capture->session && capture->calls == capture->cancel_after)
        reloq_session_cancel(capture->session);
    return 0;
}

static void change(struct reloq_curve_schedule *schedule,
                   struct reloq_acoustic_point points[RELOQ_PARAMETER_TRACKS][3],
                   size_t track, int32_t value)
{
    points[track][0] = (struct reloq_acoustic_point){0,
        (int32_t)schedule->tracks[track].default_value};
    points[track][1] = (struct reloq_acoustic_point){10, value};
    schedule->tracks[track].points = points[track];
    schedule->tracks[track].point_count = 2;
}

static void prepare(struct reloq_curve_schedule *schedule,
                    struct reloq_acoustic_point points[RELOQ_PARAMETER_TRACKS][3],
                    struct reloq_acoustic_point av_points[4])
{
    memset(schedule, 0, sizeof *schedule);
    schedule->first_tick = 0;
    schedule->end_tick = 30;
    schedule->frame_ticks = 10; /* Not 10 ms: control 0 stays 2.5 ms. */
    schedule->tracks[0].default_value = 2.5f;
    schedule->tracks[1].default_value = 1200;
    schedule->tracks[2].default_value = 50;
    schedule->tracks[3].default_value = 50;
    schedule->tracks[4].default_value = 7;
    schedule->tracks[7].default_value = 18;
    schedule->tracks[8].default_value = 25;
    static const unsigned centers[8] = {9, 13, 15, 17, 19, 21, 23, 25};
    static const int32_t frequencies[8] = {
        500, 900, 1300, 1800, 2300, 2700, 3100, 3500
    };
    static const int32_t widths[8] = {
        70, 90, 100, 110, 120, 130, 140, 150
    };
    for (size_t i = 0; i < 8; ++i) {
        schedule->tracks[centers[i]].default_value = (float)frequencies[i];
        schedule->tracks[centers[i] + 1].default_value = (float)widths[i];
        schedule->tracks[35 + i].default_value = (float)(25 - (int32_t)i);
        schedule->tracks[44 + i].default_value = (float)(80 + 5 * i);
        change(schedule, points, centers[i], frequencies[i] + 20);
        change(schedule, points, centers[i] + 1, widths[i] + 2);
    }
    static const int32_t nasal_tracheal[8] = {
        260, 75, 360, 90, 2100, 140, 2000, 150
    };
    static const int32_t changed_nasal_tracheal[8] = {
        270, 80, 370, 95, 2120, 142, 2020, 152
    };
    for (size_t i = 0; i < 8; ++i) {
        schedule->tracks[27 + i].default_value = (float)nasal_tracheal[i];
        change(schedule, points, 27 + i, changed_nasal_tracheal[i]);
    }
    schedule->tracks[43].default_value = 35.5f;
    change(schedule, points, 1, 1350);
    change(schedule, points, 3, 45);
    change(schedule, points, 4, 12);
    change(schedule, points, 5, 3);
    change(schedule, points, 6, 8);
    change(schedule, points, 7, 0);
    change(schedule, points, 8, 0);
    points[5][2] = (struct reloq_acoustic_point){20, 0};
    points[6][2] = (struct reloq_acoustic_point){20, 0};
    schedule->tracks[5].point_count = 3;
    schedule->tracks[6].point_count = 3;

    /* At tick 10 the first 46 wins. At tick 20 the last target at tick 10
     * has taken effect, so AV is zero despite the later tick-30 target. */
    av_points[0] = (struct reloq_acoustic_point){0, 50};
    av_points[1] = (struct reloq_acoustic_point){10, 46};
    av_points[2] = (struct reloq_acoustic_point){10, 0};
    av_points[3] = (struct reloq_acoustic_point){30, 0};
    schedule->tracks[2].points = av_points;
    schedule->tracks[2].point_count = 4;
}

int main(void)
{
    struct reloq_curve_schedule schedule;
    struct reloq_acoustic_point points[RELOQ_PARAMETER_TRACKS][3] = {{{0}}};
    struct reloq_acoustic_point av_points[4];
    prepare(&schedule, points, av_points);
    const struct reloq_session_config config = {
        .sample_rate = 16000,
        .duration_scale = 1,
        .output_gain = 1,
        .initial_seed = 100,
        .max_frame_samples = 128
    };
    struct reloq_session *session = reloq_session_create(&config);
    CHECK(session);
    struct capture actual = {0};
    size_t completed = 99;
    CHECK(reloq_curve_pipeline_render(&schedule, 8, session, receive_pcm,
                                      &actual, &completed) == RELOQ_SESSION_OK);
    CHECK(completed == 3 && actual.calls == 3 && actual.samples == 120);
    CHECK(!memcmp(actual.pcm, expected_pcm, sizeof expected_pcm));
    CHECK(!memcmp(actual.pcm + 40, expected_pcm + 40,
                  40 * sizeof(int16_t)));
    CHECK(!memcmp(actual.pcm + 80, expected_pcm + 80,
                  40 * sizeof(int16_t)));

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture cancelled = {.cancel_after = 2, .session = session};
    CHECK(reloq_curve_pipeline_render(&schedule, 8, session, receive_pcm,
                                      &cancelled, &completed) ==
          RELOQ_SESSION_CANCELLED);
    CHECK(completed == 2 && cancelled.calls == 2 && cancelled.samples == 80 &&
          !memcmp(cancelled.pcm, expected_pcm, 80 * sizeof(int16_t)));
    CHECK(reloq_session_cancelled(session));

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture invalid = {0};
    av_points[2].tick = 9; /* Descending, rejected before the first PCM. */
    CHECK(reloq_curve_pipeline_render(&schedule, 8, session, receive_pcm,
                                      &invalid, &completed) ==
          RELOQ_SESSION_INVALID);
    CHECK(completed == 0 && invalid.calls == 0);
    av_points[2].tick = 10;
    schedule.tracks[0].default_value = 0;
    CHECK(reloq_curve_pipeline_render(&schedule, 8, session, receive_pcm,
                                      &invalid, &completed) ==
          RELOQ_SESSION_INVALID);
    CHECK(completed == 0 && invalid.calls == 0);
    schedule.tracks[0].default_value = 2.5f;

    /* A clipped final tick stride never changes the PCM duration, even
     * when the end of the signed tick domain is reached. */
    struct reloq_curve_schedule edge = {0};
    edge.first_tick = INT64_MAX - 1;
    edge.end_tick = INT64_MAX;
    edge.frame_ticks = UINT32_MAX;
    for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i)
        edge.tracks[i].default_value = schedule.tracks[i].default_value;
    struct capture final_tick = {0};
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    CHECK(reloq_curve_pipeline_render(&edge, 8, session, receive_pcm,
                                      &final_tick, &completed) ==
          RELOQ_SESSION_OK);
    CHECK(completed == 1 && final_tick.calls == 1 && final_tick.samples == 40 &&
          !memcmp(final_tick.pcm, expected_pcm, 40 * sizeof(int16_t)));

    edge.first_tick = edge.end_tick;
    struct capture empty = {0};
    CHECK(reloq_curve_pipeline_render(&edge, 8, session, receive_pcm,
                                      &empty, &completed) ==
          RELOQ_SESSION_OK && completed == 0 && empty.calls == 0);
    reloq_session_destroy(session);
    return 0;
}
