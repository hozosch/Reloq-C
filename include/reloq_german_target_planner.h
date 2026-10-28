#ifndef RELOQ_GERMAN_TARGET_PLANNER_H
#define RELOQ_GERMAN_TARGET_PLANNER_H

#include "reloq_german_phone_sequence.h"
#include "reloq_german_input.h"
#include "reloq_target_builder.h"

/* A model-owned condition on a phone relative to the phone being planned.
 * -1 means "any" for every field except relative_phone. Phone NONE matches
 * an edge of the center SPR word. Relative matching stops at word boundaries:
 * phone_sequence omits intervening punctuation, pauses and controls, so a
 * phone in the next group is not a known uninterrupted acoustic neighbor.
 * Word/syllable relations are to the center phone; at an edge both are
 * false. A rule must name its center phone explicitly. */
struct reloq_german_target_condition {
    int32_t relative_phone;
    int16_t phone; /* -1 or enum reloq_german_phone */
    int8_t stress; /* -1, 0, 1, 2 */
    int8_t stress_explicit; /* -1, 0, 1 */
    int8_t same_word; /* -1, 0, 1 */
    int8_t same_syllable; /* -1, 0, 1 */
};

/* One independently authored gesture. A positive advance_ticks proposes
 * the next phone's nominal origin; zero makes a spectral/source-only rule.
 * Events are offsets relative to this phone's origin, including negative
 * anticipatory offsets. A matching rule with the highest priority wins
 * separately for each stream it mentions, and for the advance. Equal top
 * priorities are ambiguous and cause failure. Separate stress, context,
 * intonation and timing rules can therefore contribute different streams.
 * The opaque utterance_tag comes from the caller's prosody/punctuation
 * analysis; -1 matches any tag. Interword pause/control timing needs a
 * separate boundary-aware planning input. These are authored model ticks,
 * not the reference's array-send intervals or measured phone durations. */
struct reloq_german_target_rule {
    const struct reloq_german_target_condition *conditions;
    size_t condition_count;
    int16_t utterance_tag; /* -1 or a caller-defined nonnegative tag */
    int32_t priority;
    int64_t advance_ticks; /* zero or positive */
    const struct reloq_target_event *events;
    size_t event_count;
};

struct reloq_german_target_plan_info {
    int64_t duration_ticks;
    size_t point_count;
};

/* Select rules for every phone, place their events on a common timeline and
 * append the entire utterance to the caller's target builder. The scratch
 * and sort_work arrays each need room for all selected events. Sorting is
 * stable O(n log n) using caller-owned space, so no hidden allocation occurs.
 * An input/rule/capacity/timing/builder error leaves the builder and info
 * unchanged; the two work arrays may have changed. They must not overlap
 * each other, phone/rule storage, or any builder point array. On success,
 * equal-tick points for a stream retain
 * phone order, then rule order, then event order. The planner owns no voice
 * table and cannot fill streams for which the supplied model has no rules.
 * For anticipatory gestures, plan a complete utterance before exporting or
 * rendering its target curves. Empty phone sequences are allowed. */
int reloq_german_target_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int16_t utterance_tag, const struct reloq_german_target_rule *rules,
    size_t rule_count, struct reloq_target_builder *builder,
    struct reloq_target_event *scratch, size_t scratch_capacity,
    struct reloq_target_event *sort_work, size_t sort_work_capacity,
    struct reloq_german_target_plan_info *info);

/* The same transactional planner with a cooperative cancellation predicate.
 * Return 1 if cancelled, leaving the builder and info untouched; the two
 * work arrays may contain partially sorted candidates. Polls between phones,
 * rule/event groups and during large sorts. No callback is retained. */
int reloq_german_target_plan_checked(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int16_t utterance_tag, const struct reloq_german_target_rule *rules,
    size_t rule_count, struct reloq_target_builder *builder,
    struct reloq_target_event *scratch, size_t scratch_capacity,
    struct reloq_target_event *sort_work, size_t sort_work_capacity,
    struct reloq_german_target_plan_info *info,
    reloq_german_cancel_check cancelled, void *cancel_context);

#endif
