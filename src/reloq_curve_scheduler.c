#include "reloq_curve_scheduler.h"

#include <limits.h>
#include <string.h>

int reloq_curve_scheduler_begin(
    struct reloq_curve_scheduler *scheduler, int64_t first_tick,
    uint32_t frame_ticks,
    const unsigned char mapped[RELOQ_PARAMETER_TRACKS])
{
    if (!scheduler || !mapped || !frame_ticks) return -1;
    for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i)
        if (mapped[i] > 1) return -1;
    struct reloq_curve_scheduler initial = {0};
    initial.next_tick = first_tick;
    initial.requested_end = first_tick;
    initial.frame_ticks = frame_ticks;
    memcpy(initial.mapped, mapped, sizeof initial.mapped);
    initial.begun = 1;
    *scheduler = initial;
    return 0;
}

int reloq_curve_scheduler_plan(
    struct reloq_curve_scheduler *scheduler,
    const struct reloq_acoustic_curve tracks[RELOQ_PARAMETER_TRACKS],
    int64_t requested_end, int final,
    struct reloq_curve_schedule *schedule)
{
    if (!scheduler || !scheduler->begun || !scheduler->frame_ticks ||
        scheduler->pending || !tracks || !schedule ||
        requested_end < scheduler->requested_end ||
        (final != 0 && final != 1) ||
        (scheduler->terminal && (!final || requested_end != scheduler->requested_end)))
        return -1;

    int64_t ready_end = requested_end;
    for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i) {
        struct reloq_acoustic_cursor cursor;
        if (reloq_acoustic_begin(&tracks[i], &cursor)) return -1;
        if (!scheduler->mapped[i] && tracks[i].point_count) return -1;
        if (!final && scheduler->mapped[i]) {
            if (!tracks[i].point_count) {
                ready_end = scheduler->next_tick;
            } else {
                int64_t last = tracks[i].points[tracks[i].point_count - 1].tick;
                if (last < ready_end) ready_end = last;
            }
        }
    }
    if (ready_end < scheduler->next_tick)
        ready_end = scheduler->next_tick;
    uint64_t span = (uint64_t)ready_end - (uint64_t)scheduler->next_tick;
    uint64_t count = span ? (span - 1) / scheduler->frame_ticks + 1 : 0;
    if (count > SIZE_MAX) return -1;

    struct reloq_curve_schedule ready = {0};
    ready.first_tick = scheduler->next_tick;
    ready.end_tick = ready_end;
    ready.frame_ticks = scheduler->frame_ticks;
    memcpy(ready.tracks, tracks, sizeof ready.tracks);
    *schedule = ready;
    scheduler->requested_end = requested_end;
    if (final) scheduler->terminal = 1;
    scheduler->pending = count != 0;
    scheduler->planned_frames = (size_t)count;
    return 0;
}

int reloq_curve_scheduler_commit(struct reloq_curve_scheduler *scheduler,
                                 size_t frames_completed)
{
    if (!scheduler || !scheduler->begun || !scheduler->pending ||
        frames_completed > scheduler->planned_frames) return -1;
    /* If the next sample lies beyond the signed timebase, INT64_MAX is a
     * terminal cursor: no exclusive end_tick can contain another frame. */
    uint64_t room = (uint64_t)INT64_MAX - (uint64_t)scheduler->next_tick;
    if (frames_completed > room / scheduler->frame_ticks) {
        scheduler->next_tick = INT64_MAX;
    } else {
        uint64_t advance = (uint64_t)frames_completed * scheduler->frame_ticks;
        while (advance > INT64_MAX) {
            scheduler->next_tick += INT64_MAX;
            advance -= INT64_MAX;
        }
        scheduler->next_tick += (int64_t)advance;
    }
    scheduler->planned_frames = 0;
    scheduler->pending = 0;
    return 0;
}
