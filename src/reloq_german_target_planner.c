#include "reloq_german_target_planner.h"

#include <limits.h>

static int valid_bool_condition(int8_t value)
{
    return value >= -1 && value <= 1;
}

static int valid_rule(const struct reloq_german_target_rule *rule)
{
    if (!rule->condition_count || !rule->conditions ||
        rule->utterance_tag < -1 || rule->advance_ticks < 0 ||
        (rule->event_count && !rule->events) ||
        (!rule->advance_ticks && !rule->event_count)) return 0;

    int center_phone = 0;
    for (size_t i = 0; i < rule->condition_count; ++i) {
        const struct reloq_german_target_condition *condition =
            &rule->conditions[i];
        if (condition->phone < -1 ||
            condition->phone > RELOQ_DE_PHONE_Q_oe_TILDE ||
            condition->stress < -1 || condition->stress > 2 ||
            !valid_bool_condition(condition->stress_explicit) ||
            !valid_bool_condition(condition->same_word) ||
            !valid_bool_condition(condition->same_syllable)) return 0;
        if (!condition->relative_phone && condition->phone > 0)
            center_phone = 1;
    }
    if (!center_phone) return 0;
    for (size_t i = 0; i < rule->event_count; ++i)
        if (rule->events[i].stream < 0 ||
            rule->events[i].stream >= RELOQ_TARGET_STREAM_COUNT) return 0;
    return 1;
}

static int condition_matches(
    const struct reloq_german_target_condition *condition,
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t center)
{
    int64_t relative = condition->relative_phone;
    size_t index = center;
    int edge = 0;
    if (relative < 0) {
        uint64_t distance = (uint64_t)(-relative);
        if (distance > center) edge = 1;
        else index -= (size_t)distance;
    } else if ((uint64_t)relative >= phone_count - center) {
        edge = 1;
    } else {
        index += (size_t)relative;
    }
    if (!edge && phones[index].word_index != phones[center].word_index)
        edge = 1;

    int phone = edge ? RELOQ_DE_PHONE_NONE : phones[index].phone;
    int stress = edge ? 0 : phones[index].stress;
    int explicit_stress = edge ? 0 : phones[index].stress_explicit;
    int same_word = !edge &&
        phones[index].word_index == phones[center].word_index;
    int same_syllable = !edge &&
        phones[index].syllable_index == phones[center].syllable_index;
    return (condition->phone == -1 || condition->phone == phone) &&
           (condition->stress == -1 || condition->stress == stress) &&
           (condition->stress_explicit == -1 ||
            condition->stress_explicit == explicit_stress) &&
           (condition->same_word == -1 ||
            condition->same_word == same_word) &&
           (condition->same_syllable == -1 ||
            condition->same_syllable == same_syllable);
}

static int rule_matches(const struct reloq_german_target_rule *rule,
                        const struct reloq_german_phone_node *phones,
                        size_t phone_count, size_t center,
                        int16_t utterance_tag)
{
    if (rule->utterance_tag != -1 &&
        rule->utterance_tag != utterance_tag) return 0;
    for (size_t i = 0; i < rule->condition_count; ++i)
        if (!condition_matches(&rule->conditions[i], phones, phone_count,
                               center)) return 0;
    return 1;
}

static int add_ticks(int64_t left, int64_t right, int64_t *result)
{
    if ((right > 0 && left > INT64_MAX - right) ||
        (right < 0 && left < INT64_MIN - right)) return -1;
    *result = left + right;
    return 0;
}

/* Stable merge: the original traversal order wins at equal target ticks.
 * All indexes are bounded by count; mid avoids addition overflow. */
static int sort_by_tick(struct reloq_target_event *events,
                        struct reloq_target_event *work,
                        size_t first, size_t end,
                        reloq_german_cancel_check cancelled,
                        void *cancel_context)
{
    if (cancelled && end - first >= 256 && cancelled(cancel_context))
        return 1;
    if (end - first < 2) return 0;
    size_t mid = first + (end - first) / 2;
    if (sort_by_tick(events, work, first, mid, cancelled, cancel_context) ||
        sort_by_tick(events, work, mid, end, cancelled, cancel_context))
        return 1;
    size_t left = first, right = mid;
    for (size_t out = first; out < end; ++out) {
        if (right == end ||
            (left < mid && events[left].offset <= events[right].offset))
            work[out] = events[left++];
        else
            work[out] = events[right++];
    }
    for (size_t i = first; i < end; ++i) events[i] = work[i];
    return 0;
}

int reloq_german_target_plan_checked(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int16_t utterance_tag, const struct reloq_german_target_rule *rules,
    size_t rule_count, struct reloq_target_builder *builder,
    struct reloq_target_event *scratch, size_t scratch_capacity,
    struct reloq_target_event *sort_work, size_t sort_work_capacity,
    struct reloq_german_target_plan_info *info,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    if ((phone_count && !phones) || (rule_count && !rules) ||
        (scratch_capacity && !scratch) || utterance_tag < 0 ||
        (sort_work_capacity && !sort_work) ||
        !builder || !info) return -1;
    for (size_t i = 0; i < phone_count; ++i)
    {
        if (cancelled && (i & 255) == 0 && cancelled(cancel_context))
            return 1;
        if (phones[i].phone <= RELOQ_DE_PHONE_NONE ||
            phones[i].phone > RELOQ_DE_PHONE_Q_oe_TILDE ||
            phones[i].stress > 2 || phones[i].stress_explicit > 1)
            return -1;
    }
    for (size_t i = 0; i < rule_count; ++i)
    {
        if (cancelled && (i & 63) == 0 && cancelled(cancel_context))
            return 1;
        if (!valid_rule(&rules[i])) return -1;
    }

    int64_t origin = 0;
    size_t used = 0;
    for (size_t center = 0; center < phone_count; ++center) {
        if (cancelled && cancelled(cancel_context)) return 1;
        size_t duration_rule = SIZE_MAX;
        size_t selected[RELOQ_TARGET_STREAM_COUNT];
        for (size_t stream = 0; stream < RELOQ_TARGET_STREAM_COUNT; ++stream)
            selected[stream] = SIZE_MAX;

        for (size_t r = 0; r < rule_count; ++r) {
            if (cancelled && (r & 63) == 0 && cancelled(cancel_context))
                return 1;
            const struct reloq_german_target_rule *rule = &rules[r];
            if (!rule_matches(rule, phones, phone_count, center,
                              utterance_tag)) continue;
            if (rule->advance_ticks) {
                if (duration_rule != SIZE_MAX &&
                    rules[duration_rule].priority == rule->priority)
                    return -1;
                if (duration_rule == SIZE_MAX ||
                    rules[duration_rule].priority < rule->priority)
                    duration_rule = r;
            }
            for (size_t e = 0; e < rule->event_count; ++e) {
                size_t stream = (size_t)rule->events[e].stream;
                if (selected[stream] == r) continue;
                if (selected[stream] != SIZE_MAX &&
                    rules[selected[stream]].priority == rule->priority)
                    return -1;
                if (selected[stream] == SIZE_MAX ||
                    rules[selected[stream]].priority < rule->priority)
                    selected[stream] = r;
            }
        }
        if (duration_rule == SIZE_MAX) return -1;

        /* Traverse rules and their events in authored order. A later phone
         * can insert a point before an earlier phone's end. Sorting below
         * resolves this while preserving duplicate-tick insertion order. */
        for (size_t r = 0; r < rule_count; ++r) {
            if (cancelled && (r & 63) == 0 && cancelled(cancel_context))
                return 1;
            const struct reloq_german_target_rule *rule = &rules[r];
            for (size_t e = 0; e < rule->event_count; ++e) {
                if (cancelled && (e & 255) == 0 && cancelled(cancel_context))
                    return 1;
                const struct reloq_target_event *event = &rule->events[e];
                if (selected[(size_t)event->stream] != r) continue;
                if (used == scratch_capacity ||
                    add_ticks(origin, event->offset,
                              &scratch[used].offset)) return -1;
                scratch[used].stream = event->stream;
                scratch[used].value = event->value;
                ++used;
            }
        }
        if (add_ticks(origin, rules[duration_rule].advance_ticks,
                      &origin)) return -1;
    }
    if (used > sort_work_capacity) return -1;
    if (sort_by_tick(scratch, sort_work, 0, used,
                     cancelled, cancel_context)) return 1;
    if (cancelled && cancelled(cancel_context)) return 1;
    if (reloq_target_builder_append(builder, 0, scratch, used)) return -1;
    *info = (struct reloq_german_target_plan_info){origin, used};
    return 0;
}

int reloq_german_target_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int16_t utterance_tag, const struct reloq_german_target_rule *rules,
    size_t rule_count, struct reloq_target_builder *builder,
    struct reloq_target_event *scratch, size_t scratch_capacity,
    struct reloq_target_event *sort_work, size_t sort_work_capacity,
    struct reloq_german_target_plan_info *info)
{
    return reloq_german_target_plan_checked(
        phones, phone_count, utterance_tag, rules, rule_count, builder,
        scratch, scratch_capacity, sort_work, sort_work_capacity, info,
        NULL, NULL);
}
