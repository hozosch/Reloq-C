#include "reloq_german_nasal_vowel.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "test_reloq_german_nasal_vowel.c:%d: %s\n", \
            __LINE__, #test); return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][4];
    struct reloq_target_builder builder;
    size_t count;
};

static int begin(struct fixture *f, const char *annotation)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &info)) return -1;
    f->count = info.phone_count;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], 4, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

/* Independently observed Apple run-1 landmarks for all 60 explicit CVC
 * captures. Arrays are in p,t,k order; no natural word is a model key. */
static const struct {
    char initial, vowel;
    int target[3], hold[3], closure[3];
    int f1, f2[3], f3, held_f2, av;
} cases[] = {
    {'m','A',{100,100,100},{152,142,142},{182,182,182},
        780,{1150,1150,1150},2500,1150,51},
    {'m','E',{100,100,100},{165,165,165},{190,190,190},
        550,{1800,1800,1800},2600,1800,52},
    {'m','I',{100,100,100},{158,158,158},{183,183,183},
        380,{2000,2000,2000},2800,2000,53},
    {'m','O',{100,100,100},{153,133,143},{183,183,183},
        570,{900,900,900},2500,900,53},
    {'m','U',{100,100,100},{139,129,129},{169,169,169},
        420,{900,900,900},2400,900,53},
    {'m','a',{100,100,100},{223,203,233},{253,253,253},
        800,{1100,1100,1100},2500,1220,50},
    {'m','e',{100,100,100},{242,242,242},{267,267,267},
        370,{2150,2150,2150},2700,2150,51},
    {'m','i',{100,100,100},{228,228,228},{253,253,253},
        265,{2150,2150,2150},2900,2150,53},
    {'m','o',{100,100,100},{237,227,227},{267,267,267},
        390,{735,735,735},2400,735,54},
    {'m','u',{100,100,100},{223,213,213},{253,253,253},
        250,{750,750,750},2600,750,55},
    {'n','A',{95,95,95},{137,127,127},{167,167,167},
        780,{1160,1300,1160},2500,0,51},
    {'n','E',{100,100,100},{150,150,150},{175,175,175},
        550,{1800,1800,1800},2600,1800,52},
    {'n','I',{100,100,100},{143,143,143},{168,168,168},
        380,{2000,2000,2000},2800,2000,53},
    {'n','O',{120,118,120},{138,121,128},{168,169,168},
        570,{900,900,900},2500,900,53},
    {'n','U',{110,110,110},{124,114,114},{154,154,154},
        420,{1000,1000,1000},2400,1000,53},
    {'n','a',{95,95,95},{208,188,218},{238,238,238},
        800,{1220,1220,1220},2500,1220,50},
    {'n','e',{100,100,100},{227,227,227},{252,252,252},
        370,{2150,2150,2150},2700,2150,51},
    {'n','i',{100,100,100},{213,213,213},{238,238,238},
        265,{2150,2150,2150},2900,2150,53},
    {'n','o',{110,110,110},{222,212,212},{252,252,252},
        390,{735,735,735},2400,735,54},
    {'n','u',{110,110,110},{208,198,198},{238,238,238},
        250,{750,750,750},2400,750,55}
};

static int self_test(void)
{
    struct fixture f;
    const char codas[] = "ptk";
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        for (size_t c = 0; c < 3; ++c) {
            char annotation[] = "`[.1mAp].";
            annotation[4] = cases[i].initial;
            annotation[5] = cases[i].vowel;
            annotation[6] = codas[c];
            CHECK(begin(&f, annotation) == 0);
            int64_t target = -7, hold = -7, closure = -7;
            CHECK(reloq_german_nasal_vowel_append(
                f.phones, f.count, 31, &f.builder,
                &target, &hold, &closure) == 0);
            CHECK(target == cases[i].target[c]);
            CHECK(hold == cases[i].hold[c]);
            CHECK(closure == cases[i].closure[c]);
            const int32_t values[] = {
                cases[i].f1, cases[i].f2[c], cases[i].f3, 3600, 3900,
                120, 100, 150, 330, 260, 200, 200
            };
            size_t total = 0;
            for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
                const int expected = s == RELOQ_TARGET_AV ||
                    s == RELOQ_TARGET_AH ? 3 :
                    s == RELOQ_TARGET_F0 ||
                    s == RELOQ_TARGET_FTP || s == RELOQ_TARGET_FTZ ||
                    s == RELOQ_TARGET_FL ? 0 : 2;
                CHECK(f.builder.stream[s].count == (size_t)expected);
                total += (size_t)expected;
                if (!expected) continue;
                const struct reloq_acoustic_point *p = f.points[s];
                const int source = s == RELOQ_TARGET_AV ||
                    s == RELOQ_TARGET_TL || s == RELOQ_TARGET_AH;
                CHECK(p[0].tick == 31 + (source ?
                    (cases[i].initial == 'm' ? 86 : 71) : target));
                CHECK(p[1].tick == 31 + (source ? closure : hold));
                int32_t value = s >= RELOQ_TARGET_F1 &&
                    s <= RELOQ_TARGET_FNZ ?
                    values[s - RELOQ_TARGET_F1] :
                    s == RELOQ_TARGET_OQ ? 56 :
                    s == RELOQ_TARGET_AH ? 34 :
                    s == RELOQ_TARGET_AV ? cases[i].av : 0;
                CHECK(p[0].value == value);
                CHECK(p[1].value == (s == RELOQ_TARGET_F2 ?
                    (cases[i].held_f2 ? cases[i].held_f2 : value) :
                    s == RELOQ_TARGET_AV ? value - 2 : value));
                if (expected == 3) {
                    CHECK(p[2].tick == 31 + closure);
                    CHECK(p[2].value == 0);
                }
            }
            CHECK(total == 50);
        }
    }

    const char *uncovered[] = {
        "`[.1mA].", "`[.1nA].", "`[.1hAp].", "`[.1mAs].",
        "`[.1nAf].", "`[.1mYp].", "`[.0mAp].", "`[.1mAp.0nA]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i]) == 0);
        int64_t target = -7, hold = -7, closure = -7;
        CHECK(reloq_german_nasal_vowel_append(
            f.phones, f.count, 0, &f.builder,
            &target, &hold, &closure) == 1);
        CHECK(target == -7 && hold == -7 && closure == -7);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1mAp].") == 0);
    int64_t target = -7, hold = -7, closure = -7;
    CHECK(reloq_german_nasal_vowel_append(
        f.phones, f.count, -1, &f.builder,
        &target, &hold, &closure) == -1);
    CHECK(reloq_german_nasal_vowel_append(
        f.phones, f.count, INT64_MAX, &f.builder,
        &target, &hold, &closure) == -1);
    CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 0);
    f.builder.stream[RELOQ_TARGET_AV].capacity = 1;
    CHECK(reloq_german_nasal_vowel_append(
        f.phones, f.count, 0, &f.builder,
        &target, &hold, &closure) == -1);
    CHECK(target == -7 && hold == -7 && closure == -7);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    return 0;
}

static int dump(const char *annotation)
{
    struct fixture f;
    int64_t target, hold, closure;
    if (begin(&f, annotation) || reloq_german_nasal_vowel_append(
            f.phones, f.count, 0, &f.builder,
            &target, &hold, &closure)) return -1;
    printf("target %" PRId64 " hold %" PRId64 " closure %" PRId64 "\n",
           target, hold, closure);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        for (size_t i = 0; i < f.builder.stream[s].count; ++i)
            printf("%s %" PRId64 " %" PRId32 "\n",
                   reloq_target_stream_name((enum reloq_target_stream)s),
                   f.points[s][i].tick, f.points[s][i].value);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return self_test() ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 3 && strcmp(argv[1], "--points") == 0)
        return dump(argv[2]) ? EXIT_FAILURE : EXIT_SUCCESS;
    fprintf(stderr, "usage: %s [--points ANNOTATION]\n", argv[0]);
    return EXIT_FAILURE;
}
