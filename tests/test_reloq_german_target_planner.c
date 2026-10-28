#include "reloq_german_target_planner.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) return __LINE__; } while (0)
#define CENTER(phone_id) {0, phone_id, -1, -1, -1, -1}
#define NEIGHBOR(distance, phone_id) {distance, phone_id, -1, -1, -1, -1}

struct cancel_probe { size_t calls, after; };

static int poll_cancel(void *context)
{
    struct cancel_probe *probe = context;
    return ++probe->calls >= probe->after;
}

/* Small invented controls verify selection/scheduling, not voice fidelity. */
static const struct reloq_german_target_condition h[] = {
    CENTER(RELOQ_DE_PHONE_h)
};
static const struct reloq_german_target_condition k[] = {
    CENTER(RELOQ_DE_PHONE_k)
};
static const struct reloq_german_target_condition a[] = {
    CENTER(RELOQ_DE_PHONE_A)
};
static const struct reloq_german_target_condition a_k[] = {
    CENTER(RELOQ_DE_PHONE_A), NEIGHBOR(1, RELOQ_DE_PHONE_k)
};
static const struct reloq_german_target_condition a_k_a[] = {
    CENTER(RELOQ_DE_PHONE_A), NEIGHBOR(1, RELOQ_DE_PHONE_k),
    NEIGHBOR(2, RELOQ_DE_PHONE_A)
};
static const struct reloq_german_target_condition a_h_same_word[] = {
    CENTER(RELOQ_DE_PHONE_A), {-1, RELOQ_DE_PHONE_h, -1, -1, 1, -1}
};
static const struct reloq_german_target_condition a_primary[] = {
    {0, RELOQ_DE_PHONE_A, 1, 1, -1, -1}
};
static const struct reloq_german_target_condition a_unstressed[] = {
    {0, RELOQ_DE_PHONE_A, 0, 1, -1, -1}
};
static const struct reloq_target_event h_points[] = {
    {RELOQ_TARGET_F1, 20, 400}
};
static const struct reloq_target_event k_points[] = {
    {RELOQ_TARGET_AF, -5, 5},
    {RELOQ_TARGET_F1, -50, 350}
};
static const struct reloq_target_event a_points[] = {
    {RELOQ_TARGET_F1, 0, 500}, {RELOQ_TARGET_F2, 0, 1100}
};
static const struct reloq_target_event a_k_points[] = {
    {RELOQ_TARGET_F1, 0, 650}
};
static const struct reloq_target_event a_k_a_points[] = {
    {RELOQ_TARGET_F1, 0, 700}
};
static const struct reloq_target_event a_h_points[] = {
    {RELOQ_TARGET_F2, 0, 1250}
};
static const struct reloq_target_event primary_points[] = {
    {RELOQ_TARGET_AV, 0, 45}
};
static const struct reloq_target_event unstressed_points[] = {
    {RELOQ_TARGET_AV, 0, 42}
};
static const struct reloq_target_event question_points[] = {
    {RELOQ_TARGET_F0, -2, 900}
};
#define RULE(conditions, tag, priority_value, advance, points) \
    {conditions, sizeof conditions / sizeof conditions[0], tag, \
     priority_value, advance, points, sizeof points / sizeof points[0]}

static const struct reloq_german_target_rule rules[] = {
    RULE(h, -1, 0, 20, h_points),
    RULE(k, -1, 0, 30, k_points),
    RULE(a, -1, 0, 100, a_points),
    RULE(a_k, -1, 2, 70, a_k_points),
    RULE(a_k_a, -1, 3, 45, a_k_a_points),
    RULE(a_h_same_word, -1, 2, 0, a_h_points),
    RULE(a_primary, -1, 1, 0, primary_points),
    RULE(a_unstressed, -1, 1, 0, unstressed_points),
    RULE(a, 2, 1, 0, question_points)
};

struct fixture {
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][16];
    struct reloq_target_builder builder;
    struct reloq_target_event scratch[32];
    struct reloq_target_event sort_work[32];
};

static int begin(struct fixture *fixture)
{
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    memset(fixture, 0, sizeof *fixture);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){fixture->points[i], 16, 0};
    return reloq_target_builder_begin(&fixture->builder, buffers);
}

static int parse(const char *annotation,
                 struct reloq_german_phone_node phones[8], size_t *count)
{
    struct reloq_german_phone_sequence_info info = {0};
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            phones, 8, &info)) return -1;
    *count = info.phone_count;
    return 0;
}

static int contextual_points(void)
{
    struct fixture fixture;
    struct reloq_german_phone_node phones[8];
    struct reloq_german_target_plan_info info;
    size_t count;
    CHECK(begin(&fixture) == 0);
    CHECK(parse("`[.1A].", phones, &count) == 0);
    CHECK(reloq_german_target_plan(
        phones, count, 1, rules, sizeof rules / sizeof rules[0],
        &fixture.builder, fixture.scratch, 32,
        fixture.sort_work, 32, &info) == 0);
    CHECK(info.duration_ticks == 100 && info.point_count == 3);
    CHECK(fixture.points[RELOQ_TARGET_F1][0].tick == 0 &&
          fixture.points[RELOQ_TARGET_F1][0].value == 500 &&
          fixture.points[RELOQ_TARGET_F2][0].value == 1100 &&
          fixture.points[RELOQ_TARGET_AV][0].value == 45 &&
          fixture.builder.stream[RELOQ_TARGET_F0].count == 0);

    CHECK(begin(&fixture) == 0);
    CHECK(parse("`[.1Ak].", phones, &count) == 0);
    CHECK(reloq_german_target_plan(
        phones, count, 1, rules, sizeof rules / sizeof rules[0],
        &fixture.builder, fixture.scratch, 32,
        fixture.sort_work, 32, &info) == 0);
    CHECK(info.duration_ticks == 100 && info.point_count == 5);
    CHECK(fixture.points[RELOQ_TARGET_F1][0].value == 650 &&
          fixture.points[RELOQ_TARGET_F1][1].tick == 20 &&
          fixture.points[RELOQ_TARGET_F1][1].value == 350 &&
          fixture.points[RELOQ_TARGET_AF][0].tick == 65 &&
          fixture.points[RELOQ_TARGET_AF][0].value == 5);

    CHECK(begin(&fixture) == 0);
    CHECK(parse("`[.1hAk.0A].", phones, &count) == 0);
    CHECK(reloq_german_target_plan(
        phones, count, 2, rules, sizeof rules / sizeof rules[0],
        &fixture.builder, fixture.scratch, 32,
        fixture.sort_work, 32, &info) == 0);
    CHECK(info.duration_ticks == 195 && info.point_count == 11);
    CHECK(fixture.builder.stream[RELOQ_TARGET_F1].count == 4);
    /* k's anticipatory F1 point at tick 15 sorts before h's tick 20;
     * h then A preserve their authored order at their shared tick. */
    CHECK(fixture.points[RELOQ_TARGET_F1][0].tick == 15 &&
          fixture.points[RELOQ_TARGET_F1][0].value == 350 &&
          fixture.points[RELOQ_TARGET_F1][1].tick == 20 &&
          fixture.points[RELOQ_TARGET_F1][1].value == 400 &&
          fixture.points[RELOQ_TARGET_F1][2].tick == 20 &&
          fixture.points[RELOQ_TARGET_F1][2].value == 700 &&
          fixture.points[RELOQ_TARGET_F1][3].tick == 95 &&
          fixture.points[RELOQ_TARGET_F1][3].value == 500);
    CHECK(fixture.points[RELOQ_TARGET_F2][0].value == 1250 &&
          fixture.points[RELOQ_TARGET_F2][1].value == 1100 &&
          fixture.points[RELOQ_TARGET_AF][0].tick == 60 &&
          fixture.points[RELOQ_TARGET_AV][0].value == 45 &&
          fixture.points[RELOQ_TARGET_AV][1].value == 42 &&
          fixture.points[RELOQ_TARGET_F0][0].tick == 18);

    /* A cross-word h does not satisfy the same-word F2 rule. */
    CHECK(begin(&fixture) == 0);
    CHECK(parse("`[.1h] `[.1A].", phones, &count) == 0);
    CHECK(reloq_german_target_plan(
        phones, count, 1, rules, sizeof rules / sizeof rules[0],
        &fixture.builder, fixture.scratch, 32,
        fixture.sort_work, 32, &info) == 0);
    CHECK(fixture.points[RELOQ_TARGET_F2][0].value == 1100);
    return 0;
}

static int transaction_and_ambiguity(void)
{
    struct fixture fixture;
    struct reloq_german_phone_node phones[8];
    struct reloq_german_target_plan_info info = {333, 444};
    size_t count;
    CHECK(begin(&fixture) == 0);
    CHECK(parse("`[.1Ak].", phones, &count) == 0);
    CHECK(reloq_german_target_plan(
        phones, count, 1, rules, sizeof rules / sizeof rules[0],
        &fixture.builder, fixture.scratch, 1,
        fixture.sort_work, 32, &info) == -1);
    CHECK(info.duration_ticks == 333 && info.point_count == 444 &&
          fixture.builder.stream[RELOQ_TARGET_F1].count == 0);

    /* Equal priority F1 candidates must be resolved in the authored model. */
    const struct reloq_german_target_rule ambiguous[] = {
        RULE(a, -1, 0, 100, a_points),
        RULE(a, -1, 0, 0, a_k_points)
    };
    CHECK(parse("`[.1A]", phones, &count) == 0);
    CHECK(reloq_german_target_plan(
        phones, count, 1, ambiguous, 2, &fixture.builder,
        fixture.scratch, 32, fixture.sort_work, 32, &info) == -1);
    CHECK(info.duration_ticks == 333 &&
          fixture.builder.stream[RELOQ_TARGET_F1].count == 0);

    CHECK(reloq_german_target_plan(
        phones, count, 1, rules, sizeof rules / sizeof rules[0],
        &fixture.builder, fixture.scratch, 32,
        fixture.sort_work, 32, &info) == 0);
    struct reloq_target_buffer previous =
        fixture.builder.stream[RELOQ_TARGET_F1];
    struct reloq_acoustic_point first = fixture.points[RELOQ_TARGET_F1][0];
    info = (struct reloq_german_target_plan_info){333, 444};
    /* The existing F1 point at tick 0 makes a fresh negative-tick plan fail
     * in the target builder, after the planner has selected its rules. */
    const struct reloq_target_event early_point[] = {
        {RELOQ_TARGET_F1, -1, 12}
    };
    const struct reloq_german_target_rule early_rule =
        {a, 1, -1, 0, 100, early_point, 1};
    CHECK(reloq_german_target_plan(phones, count, 1, &early_rule, 1,
                                   &fixture.builder, fixture.scratch, 32,
                                   fixture.sort_work, 32, &info) == -1);
    CHECK(info.duration_ticks == 333 && info.point_count == 444 &&
          fixture.builder.stream[RELOQ_TARGET_F1].count == previous.count &&
          fixture.points[RELOQ_TARGET_F1][0].tick == first.tick &&
          fixture.points[RELOQ_TARGET_F1][0].value == first.value);

    CHECK(parse("`[.1p]", phones, &count) == 0);
    CHECK(reloq_german_target_plan(
        phones, count, 1, rules, sizeof rules / sizeof rules[0],
        &fixture.builder, fixture.scratch, 32,
        fixture.sort_work, 32, &info) == -1 &&
          info.duration_ticks == 333);

    CHECK(begin(&fixture) == 0);
    CHECK(parse("`[.1AA]", phones, &count) == 0);
    const struct reloq_german_target_rule overflow =
        {a, 1, -1, 0, INT64_MAX, a_points, 2};
    CHECK(reloq_german_target_plan(phones, count, 1, &overflow, 1,
                                   &fixture.builder, fixture.scratch, 32,
                                   fixture.sort_work, 32, &info) == -1 &&
          fixture.builder.stream[RELOQ_TARGET_F1].count == 0);
    return 0;
}

static int long_out_of_order_plan(void)
{
    enum { HALF = 8000, COUNT = 2 * HALF };
    char *annotation = malloc(COUNT + 6);
    struct reloq_german_phone_node *phones =
        malloc(COUNT * sizeof *phones);
    struct reloq_acoustic_point *points =
        malloc(COUNT * sizeof *points);
    struct reloq_target_event *scratch =
        malloc(COUNT * sizeof *scratch);
    struct reloq_target_event *work =
        malloc(COUNT * sizeof *work);
    CHECK(annotation && phones && points && scratch && work);
    memcpy(annotation, "`[.1", 4);
    memset(annotation + 4, 'A', HALF);
    memset(annotation + 4 + HALF, 'k', HALF);
    annotation[4 + COUNT] = ']';
    annotation[5 + COUNT] = '\0';

    struct reloq_german_phone_sequence_info sequence = {0};
    CHECK(reloq_german_phone_sequence_build(
        (const unsigned char *)annotation, COUNT + 5, 0,
        phones, COUNT, &sequence) == 0 && sequence.phone_count == COUNT);

    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    buffers[RELOQ_TARGET_F1] =
        (struct reloq_target_buffer){points, COUNT, 0};
    struct reloq_target_builder builder;
    CHECK(reloq_target_builder_begin(&builder, buffers) == 0);
    static const struct reloq_german_target_condition a_long[] = {
        CENTER(RELOQ_DE_PHONE_A)
    };
    static const struct reloq_german_target_condition k_long[] = {
        CENTER(RELOQ_DE_PHONE_k)
    };
    static const struct reloq_target_event early[] = {
        {RELOQ_TARGET_F1, COUNT, 111}
    };
    static const struct reloq_target_event late[] = {
        {RELOQ_TARGET_F1, -COUNT, 222}
    };
    const struct reloq_german_target_rule long_rules[] = {
        RULE(a_long, -1, 0, 1, early),
        RULE(k_long, -1, 0, 1, late)
    };
    struct reloq_german_target_plan_info info = {777, 888};
    /* The caller cannot provide a complete sort allocation: no point is
     * appended and no reported duration/count changes. */
    CHECK(reloq_german_target_plan(
        phones, COUNT, 0, long_rules, 2, &builder, scratch, COUNT,
        work, COUNT - 1, &info) == -1);
    CHECK(builder.stream[RELOQ_TARGET_F1].count == 0 &&
          info.duration_ticks == 777 && info.point_count == 888);
    CHECK(reloq_german_target_plan(
        phones, COUNT, 0, long_rules, 2, &builder, scratch, COUNT,
        NULL, 0, &info) == -1);
    CHECK(builder.stream[RELOQ_TARGET_F1].count == 0 &&
          info.duration_ticks == 777);

    struct cancel_probe cancelled = {0, 8000};
    CHECK(reloq_german_target_plan_checked(
        phones, COUNT, 0, long_rules, 2, &builder, scratch, COUNT,
        work, COUNT, &info, poll_cancel, &cancelled) == 1);
    CHECK(cancelled.calls == cancelled.after &&
          builder.stream[RELOQ_TARGET_F1].count == 0 &&
          info.duration_ticks == 777 && info.point_count == 888);

    /* The second half of the phones contributes earlier targets than the
     * entire first half. Sorting this pattern by insertion is quadratic. */
    CHECK(reloq_german_target_plan(
        phones, COUNT, 0, long_rules, 2, &builder, scratch, COUNT,
        work, COUNT, &info) == 0);
    CHECK(info.duration_ticks == COUNT && info.point_count == COUNT &&
          builder.stream[RELOQ_TARGET_F1].count == COUNT &&
          points[0].tick == -HALF && points[0].value == 222 &&
          points[HALF - 1].tick == -1 &&
          points[HALF].tick == COUNT && points[HALF].value == 111 &&
          points[COUNT - 1].tick == COUNT + HALF - 1);
    free(work);
    free(scratch);
    free(points);
    free(phones);
    free(annotation);
    return 0;
}

int main(void)
{
    int failure = contextual_points();
    if (failure) return failure;
    failure = transaction_and_ambiguity();
    return failure ? failure : long_out_of_order_plan();
}
