#include "reloq_german_reduced_open_composer.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_curve_scheduler.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

enum { POINTS = 40 };
struct result {
    struct reloq_target_builder builder;
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][POINTS];
    struct reloq_german_reduced_landmarks plan;
};

static int run(const char *annotation, struct result *out,
               size_t destination_capacity)
{
    struct reloq_german_phone_node phones[8] = {{0}};
    struct reloq_german_phone_sequence_info info = {0};
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            phones, 8, &info) || (info.phone_count != 4 && info.phone_count != 5)) return -2;
    struct reloq_german_syllable syllables[2] = {{0}};
    size_t count = 0;
    if (reloq_german_syllable_plan_build(
            phones, info.phone_count, syllables, 2, &count) || count != 2) return -2;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        out->buffers[i].points = out->points[i];
        out->buffers[i].capacity = destination_capacity;
    }
    if (reloq_target_builder_begin(&out->builder, out->buffers)) return -2;
    return reloq_german_reduced_open_compose(
        phones, info.phone_count, syllables, 2, &out->builder, &out->plan);
}

static int normal_test(void)
{
    struct result a = {0};
    CHECK(run(" `[.1pA.0p@].", &a, POINTS) == 0);
    CHECK(a.plan.first_vowel_hold == 109);
    CHECK(a.plan.second_schwa_target == 258);
    CHECK(a.builder.stream[RELOQ_TARGET_F1].points[2].value == 780);
    CHECK(a.builder.stream[RELOQ_TARGET_F0].count >= 2);
    struct result b = {0};
    CHECK(run(" `[.1tA.0k@n].", &b, POINTS) == 0);
    CHECK(b.builder.stream[RELOQ_TARGET_F1].count >= 8);
    struct result full = {0};
    full.plan.planned_ticks = 12345;
    CHECK(run(" `[.1pA.0p@].", &full, 1) == -1);
    CHECK(full.plan.planned_ticks == 12345);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        CHECK(full.builder.stream[i].count == 0);
    struct result bad = {0};
    CHECK(run(" `[.0pA.1p@].", &bad, POINTS) == 1);
    return 0;
}

static int query(const char *annotation)
{
    struct result result = {0};
    if (run(annotation, &result, POINTS)) return 1;
    printf("planned %lld\n", (long long)result.plan.planned_ticks);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        const struct reloq_target_buffer *track = &result.builder.stream[i];
        for (size_t j = 0; j < track->count; ++j)
            printf("%s %lld %d\n",
                   reloq_target_stream_name((enum reloq_target_stream)i),
                   (long long)track->points[j].tick,
                   track->points[j].value);
    }
    return 0;
}

struct pcm_output { FILE *file; size_t samples; };
static int write_pcm(void *context, const int16_t *pcm, size_t count)
{
    struct pcm_output *out = context;
    if (fwrite(pcm, sizeof *pcm, count, out->file) != count) return -1;
    out->samples += count;
    return 0;
}

/* A direct, independent C render of the class model. This does not go
 * through voice.c or the NVDA text frontend. Controls and PCM can be
 * compared to the reference observer before public coverage is widened. */
static int render(const char *annotation, const char *pcm_path,
                  const char *frames_path)
{
    struct result result = {0};
    if (run(annotation, &result, POINTS)) return 1;
    float defaults[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    if (reloq_german_voice_defaults(defaults, fields, &config)) return 1;
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS];
    unsigned char mapped[RELOQ_PARAMETER_TRACKS];
    if (reloq_target_builder_export(&result.builder, fields, defaults,
                                    curves, mapped)) return 1;
    struct reloq_curve_scheduler scheduler;
    struct reloq_curve_schedule schedule;
    if (reloq_curve_scheduler_begin(&scheduler, 0, 1, mapped) ||
        reloq_curve_scheduler_plan(&scheduler, curves,
            result.plan.planned_ticks, 1, &schedule)) return 1;
    FILE *frames = fopen(frames_path, "wb");
    if (!frames) return 1;
    struct reloq_acoustic_cursor cursors[RELOQ_PARAMETER_TRACKS];
    for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i)
        if (reloq_acoustic_begin(&schedule.tracks[i], &cursors[i])) {
            fclose(frames); return 1;
        }
    for (int64_t tick = schedule.first_tick; tick < schedule.end_tick;
         tick += schedule.frame_ticks) {
        fprintf(frames, "frame %lld", (long long)tick);
        for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i) {
            float value;
            if (reloq_acoustic_value(&cursors[i], tick, &value)) {
                fclose(frames); return 1;
            }
            fprintf(frames, " %a", (double)value);
        }
        fputc('\n', frames);
    }
    if (fclose(frames)) return 1;
    FILE *pcm = fopen(pcm_path, "wb");
    if (!pcm) return 1;
    struct reloq_session *session = reloq_session_create(&config);
    if (!session) { fclose(pcm); return 1; }
    struct pcm_output out = {pcm, 0};
    size_t completed = 0;
    int status = reloq_curve_pipeline_render(&schedule,
        RELOQ_GERMAN_VOICE_FORMANTS, session, write_pcm, &out, &completed);
    reloq_session_destroy(session);
    int io = fclose(pcm);
    if (status != RELOQ_SESSION_OK || io ||
        completed != (size_t)result.plan.planned_ticks ||
        out.samples != completed * 11) return 1;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--query"))
        return query(argv[2]);
    if (argc == 5 && !strcmp(argv[1], "--render"))
        return render(argv[2], argv[3], argv[4]);
    if (argc != 1) return 2;
    int status = normal_test();
    if (status) fprintf(stderr, "reduced-open composer test line %d\n", status);
    else puts("German open-schwa target composer: OK");
    return status ? 1 : 0;
}
