#ifndef RELOQ_CURVE_SCHEDULER_H
#define RELOQ_CURVE_SCHEDULER_H

#include "reloq_curve_pipeline.h"

/* Plans consecutive numeric frame batches from append-only target curves.
 * mapped[i] identifies a mutable target stream; an unmapped field must have
 * no points and uses its curve's fixed default. Every mapped stream must
 * acquire a target later than a frame tick before an ordinary batch can
 * include that tick.
 * This exclusive endpoint matches the measured target/send boundary. An
 * empty mapped stream therefore blocks ordinary output. final=1 promises
 * no more target insertions and releases the requested tail, including
 * tracks which never received a point.
 *
 * The caller owns every point array. Existing points and their order may
 * not change; new points may be appended in nondecreasing tick order only
 * between plan/commit pairs. Keep a planned schedule's arrays alive and
 * immutable until its rendering and commit have finished. A scheduler is
 * single-threaded, and its public fields should be changed only by these
 * functions. No language, voice or reference-engine types are involved.
 */
struct reloq_curve_scheduler {
    int64_t next_tick;
    int64_t requested_end;
    uint32_t frame_ticks;
    unsigned char mapped[RELOQ_PARAMETER_TRACKS];
    size_t planned_frames;
    int pending;
    int terminal;
    int begun;
};

int reloq_curve_scheduler_begin(
    struct reloq_curve_scheduler *scheduler, int64_t first_tick,
    uint32_t frame_ticks,
    const unsigned char mapped[RELOQ_PARAMETER_TRACKS]);

/* Request an exclusive end tick, which must never decrease. On success,
 * schedule describes the consecutive ready prefix starting at next_tick.
 * The prefix can be empty: schedule.first_tick == schedule.end_tick. A
 * nonempty result must be committed before another plan. A final request
 * may be repeated with the same end after a partial commit, but may not be
 * extended. A zero-frame final request closes the scheduler immediately.
 * Return -1 without changing scheduler or schedule on invalid input.
 */
int reloq_curve_scheduler_plan(
    struct reloq_curve_scheduler *scheduler,
    const struct reloq_acoustic_curve tracks[RELOQ_PARAMETER_TRACKS],
    int64_t requested_end, int final,
    struct reloq_curve_schedule *schedule);

/* Record the number of frames actually consumed from the last plan.
 * Passing fewer than planned permits retrying the remaining prefix after
 * a stopped render. The existing curve pipeline's frames_completed is a
 * suitable count. The next sample tick retains the original frame stride.
 * Return -1 and leave state unchanged for an invalid count/no plan.
 */
int reloq_curve_scheduler_commit(struct reloq_curve_scheduler *scheduler,
                                 size_t frames_completed);

#endif
