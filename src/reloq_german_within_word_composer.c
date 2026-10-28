#include "reloq_german_within_word_composer.h"

#include "reloq_german_within_word_f0.h"
#include "reloq_german_within_word_source.h"
#include "reloq_german_within_word_tract.h"

#include <stdint.h>
#include <stdlib.h>

/* This staging capacity is per control, not per utterance. The measured
 * two-syllable class has at most 30 points on one control. Separate source,
 * tract and F0 builders share this scratch arena because their control sets
 * do not overlap. To admit longer words, replace the bounded arena with a
 * syllable-count-sized allocation; the same transaction remains valid. */
enum { POINTS_PER_STREAM = 40 };

struct staging {
    struct reloq_target_builder builder;
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT]
                                       [POINTS_PER_STREAM];
};

int reloq_german_within_word_compose(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_target_builder *builder,
    struct reloq_german_within_word_landmarks *landmarks)
{
    if (!phones || !syllables || !builder || !landmarks) return -1;
    struct reloq_german_within_word_landmarks plan;
    int status = reloq_german_within_word_landmarks_plan(
        phones, phone_count, syllables, syllable_count, &plan);
    if (status) return status;

    struct staging *stage = calloc(1, sizeof *stage);
    if (!stage) return -1;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        stage->buffers[i].points = stage->points[i];
        stage->buffers[i].capacity = POINTS_PER_STREAM;
    }
    status = -1;
    if (reloq_target_builder_begin(&stage->builder, stage->buffers))
        goto done;

    status = reloq_german_within_word_tract_append(
        phones, phone_count, syllables, syllable_count,
        &plan, &stage->builder);
    if (status) goto done;
    status = reloq_german_within_word_source_append(
        phones, phone_count, syllables, syllable_count,
        &plan, &stage->builder);
    if (status) goto done;
    status = reloq_german_within_word_f0_append(
        phones, phone_count, syllables, syllable_count,
        plan.first_coda_release, plan.second_vowel_onset,
        plan.second_closure, &stage->builder);
    if (status) goto done;

    size_t total = 0;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        if (stage->builder.stream[i].count >
            SIZE_MAX - total) goto done;
        total += stage->builder.stream[i].count;
    }
    if (total > SIZE_MAX / sizeof(struct reloq_target_event)) goto done;
    struct reloq_target_event *events =
        malloc(total * sizeof *events);
    if (!events) goto done;
    size_t used = 0;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        const struct reloq_target_buffer *track =
            &stage->builder.stream[i];
        for (size_t j = 0; j < track->count; ++j)
            events[used++] = (struct reloq_target_event){
                (enum reloq_target_stream)i,
                track->points[j].tick,
                track->points[j].value};
    }
    status = reloq_target_builder_append(builder, 0, events, used);
    free(events);
    if (!status) *landmarks = plan;
done:
    free(stage);
    return status;
}
