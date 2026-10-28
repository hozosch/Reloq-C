#include "reloq_curve_scheduler.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

static void defaults(struct reloq_acoustic_curve tracks[RELOQ_PARAMETER_TRACKS])
{
    for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i)
        tracks[i] = (struct reloq_acoustic_curve){0.0f, NULL, 0};
    tracks[0].default_value = 1.0f;
}

static int value_at(const struct reloq_curve_schedule *schedule,
                    size_t track, int64_t tick, float expected)
{
    struct reloq_acoustic_cursor cursor;
    float result = NAN;
    return reloq_acoustic_begin(&schedule->tracks[track], &cursor) == 0 &&
           reloq_acoustic_value(&cursor, tick, &result) == 0 &&
           result == expected;
}

int main(void)
{
    struct reloq_acoustic_curve tracks[RELOQ_PARAMETER_TRACKS];
    unsigned char mapped[RELOQ_PARAMETER_TRACKS] = {0};
    mapped[1] = mapped[2] = 1;
    defaults(tracks);
    static const struct reloq_acoustic_point pitch_initial[] = {
        {0, 100}, {2, 110}
    };
    static const struct reloq_acoustic_point pitch_extended[] = {
        {0, 100}, {2, 110}, {6, 90}
    };
    static const struct reloq_acoustic_point voice_initial[] = {
        {0, 1}, {2, 1}
    };
    static const struct reloq_acoustic_point voice_extended[] = {
        {0, 1}, {2, 1}, {2, 10}, {2, 20}, {6, 30}
    };
    tracks[1] = (struct reloq_acoustic_curve){0, pitch_initial, 2};
    struct reloq_curve_scheduler stream;
    struct reloq_curve_schedule batch;
    CHECK(reloq_curve_scheduler_begin(&stream, 0, 1, mapped) == 0);

    /* A mutable stream with no first target can still change all earlier
     * frames. A terminal request is the only way to flush it empty. */
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 8, 0, &batch) == 0);
    CHECK(batch.first_tick == 0 && batch.end_tick == 0);
    CHECK(reloq_curve_scheduler_commit(&stream, 0) == -1);
    tracks[2] = (struct reloq_acoustic_curve){0, voice_initial, 2};
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 8, 0, &batch) == 0);
    CHECK(batch.first_tick == 0 && batch.end_tick == 2);
    CHECK(value_at(&batch, 1, 1, 105));
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 8, 0, &batch) == -1);
    CHECK(reloq_curve_scheduler_commit(&stream, 2) == 0);
    CHECK(stream.next_tick == 2);

    /* The first of the equal-time AV targets wins at tick 2, and the last
     * supplies the interpolation left endpoint at tick 3. A strict gate
     * waits for tick 6 before releasing the tick-2 frame. */
    tracks[1] = (struct reloq_acoustic_curve){0, pitch_extended, 3};
    tracks[2] = (struct reloq_acoustic_curve){0, voice_extended, 5};
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 8, 0, &batch) == 0);
    CHECK(batch.first_tick == 2 && batch.end_tick == 6);
    CHECK(value_at(&batch, 2, 2, 1));
    CHECK(value_at(&batch, 2, 3, 22));
    CHECK(value_at(&batch, 1, 3, 105));
    CHECK(reloq_curve_scheduler_commit(&stream, 2) == 0);
    CHECK(stream.next_tick == 4);
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 8, 0, &batch) == 0);
    CHECK(batch.first_tick == 4 && batch.end_tick == 6);
    CHECK(reloq_curve_scheduler_commit(&stream, 3) == -1);
    CHECK(stream.next_tick == 4);
    CHECK(reloq_curve_scheduler_commit(&stream, 2) == 0);

    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 10, 1, &batch) == 0);
    CHECK(batch.first_tick == 6 && batch.end_tick == 10);
    CHECK(value_at(&batch, 1, 9, 90));
    CHECK(value_at(&batch, 2, 9, 30));
    CHECK(reloq_curve_scheduler_commit(&stream, 4) == 0);
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 11, 1, &batch) == -1);
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 10, 0, &batch) == -1);
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 10, 1, &batch) == 0);
    CHECK(batch.first_tick == batch.end_tick && batch.first_tick == 10);

    /* A stride crossing a send boundary must remain anchored to the
     * original sample grid. An empty track is valid on the final flush. */
    defaults(tracks);
    mapped[2] = 0;
    tracks[1] = (struct reloq_acoustic_curve){0, pitch_extended, 3};
    CHECK(reloq_curve_scheduler_begin(&stream, 0, 4, mapped) == 0);
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 5, 0, &batch) == 0);
    CHECK(batch.first_tick == 0 && batch.end_tick == 5);
    CHECK(reloq_curve_scheduler_commit(&stream, 2) == 0);
    CHECK(stream.next_tick == 8);
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 7, 0, &batch) == 0);
    CHECK(batch.first_tick == 8 && batch.end_tick == 8);
    tracks[1] = (struct reloq_acoustic_curve){17, NULL, 0};
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 13, 1, &batch) == 0);
    CHECK(batch.first_tick == 8 && batch.end_tick == 13);
    CHECK(value_at(&batch, 1, 8, 17));
    CHECK(reloq_curve_scheduler_commit(&stream, 2) == 0);
    CHECK(stream.next_tick == 16);

    /* Validation must not destroy a previously usable scheduler. */
    defaults(tracks);
    mapped[1] = 0;
    CHECK(reloq_curve_scheduler_begin(&stream, -3, 1, mapped) == 0);
    static const struct reloq_acoustic_point descending[] = {{1, 0}, {0, 1}};
    tracks[1] = (struct reloq_acoustic_curve){0, descending, 2};
    struct reloq_curve_scheduler before = stream;
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 2, 0, &batch) == -1);
    CHECK(memcmp(&before, &stream, sizeof stream) == 0);
    tracks[1] = (struct reloq_acoustic_curve){0, pitch_initial, 2};
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 2, 0, &batch) == -1);
    CHECK(memcmp(&before, &stream, sizeof stream) == 0);
    defaults(tracks);
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, 2, 0, &batch) == 0);
    CHECK(batch.first_tick == -3 && batch.end_tick == 2);
    CHECK(reloq_curve_scheduler_commit(&stream, 5) == 0);
    CHECK(reloq_curve_scheduler_begin(&stream, 0, 0, mapped) == -1);
    mapped[0] = 2;
    CHECK(reloq_curve_scheduler_begin(&stream, 0, 1, mapped) == -1);

    /* A final request near the end of the signed range never overflows
     * when the next stride cannot be represented. */
    mapped[0] = 0;
    CHECK(reloq_curve_scheduler_begin(&stream, INT64_MAX - 2, 4, mapped) == 0);
    CHECK(reloq_curve_scheduler_plan(&stream, tracks, INT64_MAX, 1, &batch) == 0);
    CHECK(batch.first_tick == INT64_MAX - 2 && batch.end_tick == INT64_MAX);
    CHECK(reloq_curve_scheduler_commit(&stream, 1) == 0);
    CHECK(stream.next_tick == INT64_MAX);
    return 0;
}
