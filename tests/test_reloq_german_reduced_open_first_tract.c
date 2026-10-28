#include "reloq_german_reduced_open_first_tract.h"
#include "reloq_german_phone_sequence.h"
#include "reloq_german_syllable_plan.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

struct output {
    struct reloq_target_builder builder;
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][12];
    struct reloq_german_reduced_landmarks plan;
};

static int run(const char *annotation, struct output *out, size_t capacity)
{
    struct reloq_german_phone_node phones[6]={{0}};
    struct reloq_german_phone_sequence_info info={0};
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation,strlen(annotation),0,
            phones,6,&info)) return -2;
    struct reloq_german_syllable syllables[2]={{0}};
    size_t count=0;
    if (reloq_german_syllable_plan_build(
            phones,info.phone_count,syllables,2,&count)) return -2;
    const int plan=reloq_german_reduced_landmarks_plan(
        phones,info.phone_count,syllables,count,&out->plan);
    if (plan) return plan;
    for (size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i) {
        out->buffers[i].points=out->points[i];
        out->buffers[i].capacity=capacity;
    }
    if (reloq_target_builder_begin(&out->builder,out->buffers)) return -2;
    return reloq_german_reduced_open_first_tract_append(
        phones,info.phone_count,syllables,count,&out->plan,&out->builder);
}

static int basic(void)
{
    struct output a={0};
    CHECK(run(" `[.1pA.0p@].",&a,12)==0);
    CHECK(a.plan.first_vowel_hold==109);
    CHECK(a.builder.stream[RELOQ_TARGET_F1].count==4);
    CHECK(a.builder.stream[RELOQ_TARGET_F1].points[2].value==780);
    CHECK(a.builder.stream[RELOQ_TARGET_F2].points[2].value==1150);
    CHECK(a.builder.stream[RELOQ_TARGET_TL].count==5);
    struct output b={0};
    CHECK(run(" `[.1tE.0k@n].",&b,12)==0);
    CHECK(b.builder.stream[RELOQ_TARGET_F3].count==4);
    struct output full={0};
    CHECK(run(" `[.1pA.0p@].",&full,1)==-1);
    for (size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i)
        CHECK(full.builder.stream[i].count==0);
    struct output bad={0};
    CHECK(run(" `[.0pA.1p@].",&bad,12)==1);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc==3 && !strcmp(argv[1],"--query")) {
        struct output out={0};
        if (run(argv[2],&out,12)) return 1;
        printf("hold %lld\n",(long long)out.plan.first_vowel_hold);
        for (size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i)
            for (size_t j=0;j<out.builder.stream[i].count;++j)
                printf("%s %lld %d\n",
                    reloq_target_stream_name((enum reloq_target_stream)i),
                    (long long)out.builder.stream[i].points[j].tick,
                    out.builder.stream[i].points[j].value);
        return 0;
    }
    if (argc!=1) return 2;
    int line=basic();
    if (line) fprintf(stderr,"open first tract failed at %d\n",line);
    return !!line;
}
