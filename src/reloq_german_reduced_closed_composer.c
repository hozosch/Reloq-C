#include "reloq_german_reduced_closed_composer.h"

#include "reloq_german_reduced_closed_first_tract.h"
#include "reloq_german_reduced_source.h"
#include "reloq_german_reduced_tract.h"

#include <stdint.h>
#include <stdlib.h>

enum { POINTS_PER_STREAM = 48 };

struct staging {
    struct reloq_target_builder builder;
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT]
                                       [POINTS_PER_STREAM];
};

int reloq_german_reduced_closed_compose(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_target_builder *builder,
    struct reloq_german_reduced_landmarks *landmarks)
{
    if (!phones || !syllables || !builder || !landmarks) return -1;
    struct reloq_german_reduced_landmarks plan;
    int status = reloq_german_reduced_landmarks_plan(
        phones,phone_count,syllables,syllable_count,&plan);
    if (status) return status;
    if (syllables[0].phone_count!=3 ||
        syllables[1].first_phone!=3) return 1;

    struct staging *stage=calloc(1,sizeof *stage);
    if (!stage) return -1;
    for (size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i) {
        stage->buffers[i].points=stage->points[i];
        stage->buffers[i].capacity=POINTS_PER_STREAM;
    }
    status=-1;
    if (reloq_target_builder_begin(&stage->builder,stage->buffers))
        goto done;
    status=reloq_german_reduced_closed_first_tract_append(
        phones,phone_count,syllables,syllable_count,
        &plan,&stage->builder);
    if (status) goto done;
    status=reloq_german_reduced_tract_append(
        phones,phone_count,syllables,syllable_count,
        &plan,&stage->builder);
    if (status) goto done;
    status=reloq_german_reduced_source_append(
        phones,phone_count,syllables,syllable_count,
        &plan,&stage->builder);
    if (status) goto done;

    size_t total=0;
    for (size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i) {
        if (stage->builder.stream[i].count>SIZE_MAX-total) {
            status=-1; goto done;
        }
        total+=stage->builder.stream[i].count;
    }
    if (total>SIZE_MAX/sizeof(struct reloq_target_event)) {
        status=-1; goto done;
    }
    struct reloq_target_event *events=malloc(total*sizeof *events);
    if (!events) {status=-1;goto done;}
    size_t used=0;
    for (size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i) {
        const struct reloq_target_buffer *track=&stage->builder.stream[i];
        for (size_t j=0;j<track->count;++j)
            events[used++]=(struct reloq_target_event){
                (enum reloq_target_stream)i,
                track->points[j].tick,track->points[j].value};
    }
    status=reloq_target_builder_append(builder,0,events,used);
    free(events);
    if (!status) *landmarks=plan;
done:
    free(stage);
    return status;
}
