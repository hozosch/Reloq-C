#include "reloq_german_utterance.h"

#include "reloq_curve_scheduler.h"
#include "reloq_german_text.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int text_result(int result)
{
    if (result == RELOQ_DE_TEXT_CANCELLED)
        return RELOQ_DE_UTTERANCE_CANCELLED;
    if (result == RELOQ_DE_TEXT_UNCOVERED)
        return RELOQ_DE_UTTERANCE_UNCOVERED;
    if (result == RELOQ_DE_TEXT_NO_MEMORY)
        return RELOQ_DE_UTTERANCE_MEMORY;
    return RELOQ_DE_UTTERANCE_INVALID;
}

static int cancelled(void *context)
{
    return reloq_session_cancelled((const struct reloq_session *)context);
}

static int render_result(int result)
{
    switch (result) {
    case RELOQ_SESSION_OK: return RELOQ_DE_UTTERANCE_OK;
    case RELOQ_SESSION_CANCELLED: return RELOQ_DE_UTTERANCE_CANCELLED;
    case RELOQ_SESSION_MEMORY: return RELOQ_DE_UTTERANCE_MEMORY;
    case RELOQ_SESSION_SINK: return RELOQ_DE_UTTERANCE_SINK;
    default: return RELOQ_DE_UTTERANCE_RENDER;
    }
}

/* Capacity bounds count every authored event for every phone. The planner
 * selects a subset, but must have enough scratch space before it can decide.
 * Do not impose an arbitrary utterance-length limit. */
static int event_capacities(const struct reloq_german_utterance_model *model,
                            size_t phones,
                            size_t per_stream[RELOQ_TARGET_STREAM_COUNT],
                            size_t *total)
{
    size_t per_phone[RELOQ_TARGET_STREAM_COUNT] = {0};
    size_t sum = 0;
    for (size_t r = 0; r < model->rule_count; ++r) {
        const struct reloq_german_target_rule *rule = &model->rules[r];
        if (rule->event_count && !rule->events) return -1;
        for (size_t e = 0; e < rule->event_count; ++e) {
            int stream = rule->events[e].stream;
            if (stream < 0 || stream >= RELOQ_TARGET_STREAM_COUNT ||
                per_phone[(size_t)stream] == SIZE_MAX) return -1;
            ++per_phone[(size_t)stream];
        }
    }
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        if (phones && per_phone[s] > SIZE_MAX / phones) return -1;
        per_stream[s] = per_phone[s] * phones;
        if (per_stream[s] > SIZE_MAX - sum) return -1;
        sum += per_stream[s];
    }
    if (sum > SIZE_MAX / sizeof(struct reloq_acoustic_point) ||
        sum > SIZE_MAX / sizeof(struct reloq_target_event)) return -1;
    *total = sum;
    return 0;
}

int reloq_german_utterance_render(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_german_utterance_model *model,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info)
{
    struct reloq_german_utterance_info progress = {0};
    unsigned char *annotation = NULL;
    struct reloq_german_phone_node *phones = NULL;
    struct reloq_acoustic_point *points = NULL;
    struct reloq_target_event *scratch = NULL, *sort_work = NULL;
    int result = RELOQ_DE_UTTERANCE_INVALID;
    size_t annotation_bytes = 0, error_offset = 0;

    if (info) *info = progress;
    if (!model || !session || !sink || !info ||
        (utf8_length && !utf8) || !model->frame_ticks ||
        model->trailing_ticks < 0 || model->utterance_tag < 0 ||
        model->highest_formant < 1 || model->highest_formant > 8 ||
        (model->rule_count && !model->rules)) return result;
    /* Validate the model's field map and defaults before parsing any text.
     * All bound point arrays are empty here, so export cannot read events. */
    struct reloq_target_builder builder;
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS];
    unsigned char mapped[RELOQ_PARAMETER_TRACKS];
    if (reloq_target_builder_begin(&builder, storage) ||
        reloq_target_builder_export(&builder, model->field_for_stream,
                                    model->defaults, curves, mapped))
        return RELOQ_DE_UTTERANCE_MODEL;
    if (reloq_session_cancelled(session))
        return RELOQ_DE_UTTERANCE_CANCELLED;
    if (!utf8_length) return RELOQ_DE_UTTERANCE_OK;

    int text_status = reloq_german_text_annotate_checked(
        utf8, utf8_length, NULL, 0, &annotation_bytes, &error_offset,
        model->lexicon, cancelled, session);
    if (text_status) {
        progress.error_offset = error_offset;
        result = text_result(text_status);
        goto done;
    }
    if (reloq_session_cancelled(session)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done;
    }
    if (annotation_bytes == SIZE_MAX) {
        result = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    annotation = malloc(annotation_bytes + 1);
    if (!annotation) {
        result = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    size_t written = annotation_bytes;
    text_status = reloq_german_text_annotate_checked(
        utf8, utf8_length, annotation, annotation_bytes + 1, &written,
        &error_offset, model->lexicon, cancelled, session);
    if (text_status || written != annotation_bytes) {
        progress.error_offset = error_offset;
        result = text_status ? text_result(text_status) :
                 RELOQ_DE_UTTERANCE_INVALID;
        goto done;
    }
    progress.annotation_bytes = annotation_bytes;
    if (reloq_session_cancelled(session)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done;
    }
    if (annotation_bytes > SIZE_MAX / sizeof *phones) {
        result = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    phones = malloc((annotation_bytes ? annotation_bytes : 1) *
                    sizeof *phones);
    if (!phones) {
        result = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    struct reloq_german_phone_sequence_info sequence = {0};
    int parsed = reloq_german_phone_sequence_build_checked(
        annotation, annotation_bytes, 0, phones, annotation_bytes,
        &sequence, cancelled, session);
    if (parsed) {
        result = parsed == 1 ? RELOQ_DE_UTTERANCE_CANCELLED :
                 parsed == -2 ? RELOQ_DE_UTTERANCE_MEMORY :
                 RELOQ_DE_UTTERANCE_INVALID;
        goto done;
    }
    progress.phone_count = sequence.phone_count;
    if (reloq_session_cancelled(session)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done;
    }
    if (!sequence.phone_count) {
        result = RELOQ_DE_UTTERANCE_OK;
        goto done;
    }
    if (!model->rule_count) {
        result = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }

    size_t per_stream[RELOQ_TARGET_STREAM_COUNT] = {0}, total = 0;
    if (event_capacities(model, sequence.phone_count, per_stream, &total)) {
        result = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    if (!total) {
        result = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    points = malloc(total * sizeof *points);
    scratch = malloc(total * sizeof *scratch);
    sort_work = malloc(total * sizeof *sort_work);
    if (!points || !scratch || !sort_work) {
        result = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    size_t start = 0;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        storage[s] = (struct reloq_target_buffer){
            per_stream[s] ? points + start : NULL, per_stream[s], 0};
        start += per_stream[s];
    }
    if (reloq_target_builder_begin(&builder, storage)) {
        result = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    struct reloq_german_target_plan_info plan = {0};
    int planned = reloq_german_target_plan_checked(
        phones, sequence.phone_count, model->utterance_tag, model->rules,
        model->rule_count, &builder, scratch, total, sort_work, total,
        &plan, cancelled, session);
    if (planned) {
        result = planned == 1 ? RELOQ_DE_UTTERANCE_CANCELLED :
                 RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    progress.target_count = plan.point_count;
    if (plan.duration_ticks > INT64_MAX - model->trailing_ticks ||
        !plan.point_count) {
        result = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    progress.planned_ticks = plan.duration_ticks + model->trailing_ticks;
    if (reloq_session_cancelled(session)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done;
    }
    if (reloq_target_builder_export(&builder, model->field_for_stream,
                                    model->defaults, curves, mapped)) {
        result = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    struct reloq_curve_scheduler scheduler;
    struct reloq_curve_schedule schedule;
    if (reloq_curve_scheduler_begin(&scheduler, 0, model->frame_ticks,
                                    mapped) ||
        reloq_curve_scheduler_plan(&scheduler, curves,
                                   progress.planned_ticks, 1, &schedule)) {
        result = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    int rendered = reloq_curve_pipeline_render(
        &schedule, model->highest_formant, session, sink, context,
        &progress.frames_completed);
    result = render_result(rendered);
    if (reloq_curve_scheduler_commit(&scheduler, progress.frames_completed) &&
        result == RELOQ_DE_UTTERANCE_OK)
        result = RELOQ_DE_UTTERANCE_RENDER;

done:
    free(sort_work);
    free(scratch);
    free(points);
    free(phones);
    free(annotation);
    *info = progress;
    return result;
}
