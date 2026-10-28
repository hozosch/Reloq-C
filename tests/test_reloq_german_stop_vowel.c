#include "reloq_german_stop_vowel.h"

#include <assert.h>
#include <limits.h>
#include <string.h>

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
    struct reloq_target_builder builder;
    size_t count;
};

static void begin(struct fixture *f, const char *annotation)
{
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_german_phone_sequence_info info = {0};
    memset(f, 0, sizeof *f);
    assert(reloq_german_phone_sequence_build(
        (const unsigned char *)annotation, strlen(annotation), 0,
        f->phones, 8, &info) == 0);
    f->count = info.phone_count;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        buffers[s] = (struct reloq_target_buffer){f->points[s], 8, 0};
    assert(reloq_target_builder_begin(&f->builder, buffers) == 0);
}

static void reference_cases(void)
{
    static const struct {
        const char *annotation;
        int64_t onset, closure, burst, hold;
        int32_t prefix_f[3], burst_f[3], vowel_f[3], av;
        int32_t hold_f2;
    } cases[] = {
        {"`[.1pA].",65,87,15,152,
         {350,900,2500},{350,900,2500},{700,1150,2500},51,0},
        {"`[.1pAt].",65,87,15,112,
         {350,900,2500},{350,900,2500},{780,1150,2500},51,0},
        {"`[.1pE].",65,94,15,159,
         {350,1150,2500},{350,1150,2500},{550,1800,2600},52,0},
        {"`[.1puk].",59,151,15,170,
         {350,800,2500},{350,800,2500},{250,750,2400},55,0},
        {"`[.1tA].",89,87,17,176,
         {300,1600,2500},{300,1600,2500},{700,1300,2500},51,0},
        {"`[.1tEk].",95,94,23,164,
         {300,1600,2500},{300,1700,2700},{550,1800,2600},52,0},
        {"`[.1ti].",113,172,23,285,
         {300,1600,2500},{300,1700,2700},{265,2150,2900},53,0},
        {"`[.1tu].",80,172,17,252,
         {300,1600,2500},{280,1600,2500},{250,750,2400},55,0},
        {"`[.1kAk].",78,87,17,125,
         {300,1500,2300},{300,1600,2100},{780,1250,2400},51,0},
        {"`[.1kE].",78,94,17,172,
         {300,1500,2300},{300,2150,2600},{550,1800,2600},52,0},
        {"`[.1kIk].",78,88,17,141,
         {300,1500,2300},{300,2200,2700},{350,2000,2800},53,0},
        {"`[.1kUp].",78,75,17,123,
         {300,1500,2300},{300,1250,2000},{420,930,2060},54,0},
        {"`[.1ka].",78,189,17,267,
         {300,1500,2300},{300,1600,2100},{800,1280,2500},50,1220},
        {"`[.1ke].",93,187,17,280,
         {300,1500,2300},{300,2200,2850},{370,2150,2700},51,0},
        {"`[.1pak].",65,151,15,196,
         {350,900,2500},{350,900,2500},{800,1100,2500},50,1220},
        {"`[.1tak].",89,151,17,220,
         {300,1600,2500},{300,1600,2500},{800,1250,2500},50,1220},
        {"`[.1pek].",78,163,15,216,
         {350,1250,2500},{350,1250,2500},{370,2000,2700},51,2150}
    };
    struct fixture f;
    for (size_t c = 0; c < sizeof cases / sizeof cases[0]; ++c) {
        begin(&f, cases[c].annotation);
        int64_t closure = -1, first = -1, second = -1;
        assert(reloq_german_stop_vowel_append(
            f.phones, f.count, cases[c].onset, &f.builder,
            &closure, &first, &second) == 0);
        assert(closure == cases[c].closure && first == cases[c].burst &&
               second == cases[c].hold);
        for (size_t i = 0; i < 3; ++i) {
            const struct reloq_acoustic_point *p = f.points[RELOQ_TARGET_F1+i];
            assert(f.builder.stream[RELOQ_TARGET_F1+i].count == 4);
            assert(p[0].tick == 0 && p[0].value == cases[c].prefix_f[i]);
            assert(p[1].tick == first && p[1].value == cases[c].burst_f[i]);
            assert(p[2].tick == cases[c].onset &&
                   p[2].value == cases[c].vowel_f[i]);
            assert(p[3].tick == second &&
                   p[3].value == (i == 1 && cases[c].hold_f2 ?
                                  cases[c].hold_f2 : p[2].value));
        }
        const struct reloq_acoustic_point *av = f.points[RELOQ_TARGET_AV];
        assert(av[0].tick == cases[c].onset + 1 &&
               av[0].value == cases[c].av);
        assert(av[1].tick == cases[c].onset + closure &&
               av[1].value == cases[c].av - 2);
        assert(av[2].tick == cases[c].onset + closure +
               (f.count == 2 ? 60 : 0) && av[2].value == 0);
    }
}

static void error_cases(void)
{
    struct fixture f;
    int64_t closure = -17, first = -17, second = -17;
    begin(&f, "`[.1hAk].");
    assert(reloq_german_stop_vowel_append(
        f.phones, f.count, 65, &f.builder,
        &closure, &first, &second) == 1);
    assert(closure == -17 && first == -17 && second == -17);
    begin(&f, "`[.0pAk].");
    assert(reloq_german_stop_vowel_append(
        f.phones, f.count, 65, &f.builder,
        &closure, &first, &second) == 1);
    begin(&f, "`[.1pAk].");
    assert(reloq_german_stop_vowel_append(
        f.phones, f.count, INT64_MAX, &f.builder,
        &closure, &first, &second) == -1);
    assert(f.builder.stream[RELOQ_TARGET_F1].count == 0 &&
           closure == -17 && first == -17 && second == -17);
}

int main(void)
{
    reference_cases();
    error_cases();
    return 0;
}
