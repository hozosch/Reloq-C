#include "reloq_parameter_stream.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

int main(void)
{
    /* Hand-tabulated sequence: the first curve starts before frame zero,
     * crosses zero with truncation toward zero and then holds its endpoint.
     * The last frame has only one tick. */
    static const struct reloq_parameter_knot rise[] = {
        {-2, -9}, {2, 0}, {7, 11}
    };
    static const struct reloq_parameter_knot fall[] = {
        {0, 1500}, {8, 1200}
    };
    struct reloq_parameter_schedule schedule = {0};
    schedule.first_tick = -3;
    schedule.end_tick = 13;
    schedule.frame_ticks = 3;
    schedule.tracks[0] = (struct reloq_parameter_track){77, rise, 3};
    schedule.tracks[1] = (struct reloq_parameter_track){900, fall, 2};
    schedule.tracks[61].baseline = 0.75f;

    struct reloq_parameter_cursor cursor = {0};
    struct reloq_parameter_frame frame = {0};
    CHECK(reloq_parameter_begin(&schedule, &cursor) == 0);
    static const struct {
        int64_t tick;
        uint32_t duration;
        float rise, fall;
    } trace[] = {
        {-3, 3, 77, 900},
        { 0, 3, -4, 1500}, /* -9 + 9 * 2/4 = -4.5 -> -4 */
        { 3, 3, 2, 1387},  /* 1500 - 300 * 3/8 = 1387.5 -> 1387 */
        { 6, 3, 8, 1275},
        { 9, 3, 11, 1200},
        {12, 1, 11, 1200}
    };
    for (size_t i = 0; i < sizeof trace / sizeof trace[0]; ++i) {
        CHECK(reloq_parameter_next(&cursor, &frame) == 1);
        CHECK(frame.tick == trace[i].tick &&
              frame.duration_ticks == trace[i].duration &&
              frame.parameters[0] == trace[i].rise &&
              frame.parameters[1] == trace[i].fall &&
              frame.parameters[61] == 0.75f);
        for (size_t p = 2; p < 61; ++p)
            CHECK(frame.parameters[p] == 0.0f);
    }
    CHECK(reloq_parameter_next(&cursor, &frame) == 0);

    /* Original Apple ECI 6.1 German `Hallo Welt.` at 11,025 Hz, captured by
     * reference-apple/natural_trace_capture.c at the pre-Klatt frame hook.
     * Each UI is one tick. Indices 9 and 13 of the reference's numeric frame
     * are two formant tracks: the plateau endpoints at 122 and 162 are the
     * only knots supplied here; all 39 interior answers are held-out Apple
     * observations, including the alternating half-integer truncations.
     * This checks interpolation for these tracks, not a phonetic scheduler. */
    static const struct reloq_parameter_knot apple_first[] = {
        {122, 780}, {162, 400}
    };
    static const struct reloq_parameter_knot apple_second[] = {
        {122, 1160}, {162, 1300}
    };
    static const int16_t apple_trace[][2] = {
        {780, 1160}, {780, 1160}, {780, 1160}, {770, 1163},
        {761, 1167}, {751, 1170}, {742, 1174}, {732, 1177},
        {723, 1181}, {713, 1184}, {704, 1188}, {694, 1191},
        {685, 1195}, {675, 1198}, {666, 1202}, {656, 1205},
        {647, 1209}, {637, 1212}, {628, 1216}, {618, 1219},
        {609, 1223}, {599, 1226}, {590, 1230}, {580, 1233},
        {571, 1237}, {561, 1240}, {552, 1244}, {542, 1247},
        {533, 1251}, {523, 1254}, {514, 1258}, {504, 1261},
        {495, 1265}, {485, 1268}, {476, 1272}, {466, 1275},
        {457, 1279}, {447, 1282}, {438, 1286}, {428, 1289},
        {419, 1293}, {409, 1296}, {400, 1300}, {400, 1300},
        {400, 1300}
    };
    schedule.first_tick = 120;
    schedule.end_tick = 165;
    schedule.frame_ticks = 1;
    schedule.tracks[0] = (struct reloq_parameter_track){
        780.0f, apple_first, 2
    };
    schedule.tracks[1] = (struct reloq_parameter_track){
        1160.0f, apple_second, 2
    };
    CHECK(reloq_parameter_begin(&schedule, &cursor) == 0);
    for (size_t i = 0; i < sizeof apple_trace / sizeof apple_trace[0]; ++i) {
        CHECK(reloq_parameter_next(&cursor, &frame) == 1);
        CHECK(frame.tick == 120 + (int64_t)i && frame.duration_ticks == 1 &&
              frame.parameters[0] == apple_trace[i][0] &&
              frame.parameters[1] == apple_trace[i][1]);
    }
    CHECK(reloq_parameter_next(&cursor, &frame) == 0);

    /* A failed conversion must not consume the frame or partially replace
     * the output. The float endpoint 2^31 cannot become a signed int32. */
    static const struct reloq_parameter_knot extreme[] = {
        {0, INT32_MAX}, {2, INT32_MAX - 1}
    };
    schedule.first_tick = 0;
    schedule.end_tick = 2;
    schedule.frame_ticks = 1;
    schedule.tracks[0] = (struct reloq_parameter_track){0, extreme, 2};
    CHECK(reloq_parameter_begin(&schedule, &cursor) == 0);
    struct reloq_parameter_cursor before_cursor = cursor;
    struct reloq_parameter_frame before_frame = frame;
    CHECK(reloq_parameter_next(&cursor, &frame) == -1 &&
          !memcmp(&cursor, &before_cursor, sizeof cursor) &&
          !memcmp(&frame, &before_frame, sizeof frame));

    static const struct reloq_parameter_knot duplicate[] = {
        {0, 1}, {0, 2}
    };
    schedule.tracks[0] = (struct reloq_parameter_track){0, duplicate, 2};
    CHECK(reloq_parameter_begin(&schedule, &cursor) == -1 &&
          !memcmp(&cursor, &before_cursor, sizeof cursor));
    static const struct reloq_parameter_knot over_delta[] = {
        {0, INT32_MIN}, {1, INT32_MAX}
    };
    schedule.tracks[0] = (struct reloq_parameter_track){0, over_delta, 2};
    CHECK(reloq_parameter_begin(&schedule, &cursor) == -1);
    schedule.tracks[0] = (struct reloq_parameter_track){NAN, NULL, 0};
    CHECK(reloq_parameter_begin(&schedule, &cursor) == -1);
    schedule.tracks[0] = (struct reloq_parameter_track){0, NULL, 1};
    CHECK(reloq_parameter_begin(&schedule, &cursor) == -1);
    schedule.tracks[0] = (struct reloq_parameter_track){0, NULL, 0};
    schedule.frame_ticks = 0;
    CHECK(reloq_parameter_begin(&schedule, &cursor) == -1);
    schedule.frame_ticks = 1;
    schedule.first_tick = 3;
    CHECK(reloq_parameter_begin(&schedule, &cursor) == -1);
    schedule.end_tick = 3;
    CHECK(reloq_parameter_begin(&schedule, &cursor) == 0 &&
          reloq_parameter_next(&cursor, &frame) == 0);
    CHECK(reloq_parameter_begin(NULL, &cursor) == -1);
    CHECK(reloq_parameter_begin(&schedule, NULL) == -1);
    CHECK(reloq_parameter_next(NULL, &frame) == -1);
    CHECK(reloq_parameter_next(&cursor, NULL) == -1);

    /* Crossing INT64_MAX with an overly long nominal frame clips safely. */
    schedule.first_tick = INT64_MAX - 2;
    schedule.end_tick = INT64_MAX;
    schedule.frame_ticks = UINT32_MAX;
    CHECK(reloq_parameter_begin(&schedule, &cursor) == 0);
    CHECK(reloq_parameter_next(&cursor, &frame) == 1 &&
          frame.duration_ticks == 2 && cursor.next_tick == INT64_MAX);
    CHECK(reloq_parameter_next(&cursor, &frame) == 0);

    /* An unrepresentable span is refused before any cursor change. */
    static const struct reloq_parameter_knot huge_span[] = {
        {INT64_MIN, 0}, {INT64_MAX, 100}
    };
    schedule.first_tick = 0;
    schedule.end_tick = 1;
    schedule.frame_ticks = 1;
    schedule.tracks[0] = (struct reloq_parameter_track){0, huge_span, 2};
    before_cursor = cursor;
    CHECK(reloq_parameter_begin(&schedule, &cursor) == -1 &&
          !memcmp(&cursor, &before_cursor, sizeof cursor));
    return 0;
}
