#include "reloq_german_voiced_stop_vowel.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

struct fixture {
    struct reloq_german_phone_node phones[4];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][4];
    struct reloq_target_builder builder;
    size_t count;
};

static void begin(struct fixture *f, const char *annotation, size_t capacity)
{
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_german_phone_sequence_info info = {0};
    memset(f, 0, sizeof *f);
    assert(reloq_german_phone_sequence_build(
        (const unsigned char *)annotation, strlen(annotation), 0,
        f->phones, 4, &info) == 0);
    f->count = info.phone_count;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        buffers[s] = (struct reloq_target_buffer){
            f->points[s], capacity, 0};
    assert(reloq_target_builder_begin(&f->builder, buffers) == 0);
}

/* Independent accepted-target captures for the ten vowels, three initial
 * places and each of three final places. T and H are absolute formant
 * landmarks; F1/F2/F3 are the first vowel targets, not stop targets. */
struct observation {
    char initial, vowel;
    int onset, burst, closure;
    int target[3], hold[3]; /* coda p/t/k */
    int f1, f2, f3, held_f2, av;
};

static const struct observation apple[] = {
    {'b','A',81,70,97,{96,96,96},{148,138,138},780,1150,2500,1150,51},
    {'b','E',93,82,105,{118,118,118},{173,173,173},600,1640,2600,1640,52},
    {'b','I',106,95,98,{131,131,131},{179,179,179},380,2000,2800,2000,53},
    {'b','O',93,82,98,{108,108,108},{161,141,151},570,900,2500,900,53},
    {'b','U',106,95,84,{121,121,121},{160,150,150},420,900,2400,900,54},
    {'b','a',81,70,168,{96,96,96},{219,199,229},800,1100,2500,1220,50},
    {'b','e',93,82,182,{118,118,118},{250,250,250},370,2000,2700,2150,51},
    {'b','i',106,95,168,{131,131,131},{249,249,249},265,2150,2900,2150,53},
    {'b','o',93,82,182,{108,108,108},{245,235,235},390,735,2400,735,54},
    {'b','u',106,95,168,{121,121,121},{244,234,234},250,750,2400,750,55},
    {'d','A',100,90,97,{145,145,145},{167,157,157},780,1300,2500,1300,51},
    {'d','E',100,90,105,{125,125,125},{180,180,180},550,1800,2600,1800,52},
    {'d','I',100,90,98,{125,125,125},{173,173,173},380,2000,2800,2000,53},
    {'d','O',100,90,98,{145,145,145},{168,148,158},570,900,2500,900,53},
    {'d','U',100,90,84,{145,142,142},{154,145,145},420,1000,2400,1000,54},
    {'d','a',100,90,168,{145,145,145},{238,218,248},800,1250,2500,1220,50},
    {'d','e',105,90,182,{130,130,130},{262,262,262},370,2150,2700,2150,51},
    {'d','i',105,90,168,{130,130,130},{248,248,248},265,2150,2900,2150,53},
    {'d','o',100,90,182,{145,145,145},{252,242,242},390,735,2400,735,54},
    {'d','u',100,90,168,{145,145,145},{238,228,228},250,750,2400,750,55},
    {'g','A',90,70,97,{125,125,125},{157,147,147},780,1250,2400,1250,51},
    {'g','E',90,70,105,{115,115,115},{170,170,170},550,1850,2600,1850,52},
    {'g','I',90,70,98,{115,115,115},{163,163,163},350,2000,2800,2000,53},
    {'g','O',90,70,98,{115,115,115},{158,138,148},570,900,2500,900,53},
    {'g','U',90,70,84,{115,115,115},{144,134,134},420,930,2060,930,54},
    {'g','a',90,70,168,{125,125,125},{228,208,238},800,1280,2500,1220,50},
    {'g','e',90,70,182,{115,115,115},{247,247,247},370,2150,2700,2150,51},
    {'g','i',90,70,168,{115,115,115},{233,233,233},265,2150,2900,2150,53},
    {'g','o',90,70,182,{115,115,115},{242,232,232},390,735,2400,735,54},
    {'g','u',90,70,168,{115,115,115},{228,218,218},250,750,2400,750,55}
};

static void grid(void)
{
    struct fixture f;
    for (size_t row = 0; row < sizeof apple / sizeof apple[0]; ++row) {
        const struct observation *o = &apple[row];
        for (size_t k = 0; k < 3; ++k) {
            char annotation[16];
            assert(snprintf(annotation, sizeof annotation, "`[.1%c%c%c].",
                            o->initial, o->vowel, "ptk"[k]) > 0);
            begin(&f, annotation, 4);
            int64_t closure = -1, burst = -1, hold = -1, target = -1;
            assert(reloq_german_voiced_stop_vowel_append(
                f.phones, f.count, o->onset, &f.builder,
                &closure, &burst, &hold, &target) == 0);
            assert(closure == o->closure && burst == o->burst &&
                   hold == o->hold[k] && target == o->target[k]);
            const int first[] = {o->f1, o->f2, o->f3};
            for (size_t i = 0; i < 3; ++i) {
                const struct reloq_target_buffer *stream =
                    &f.builder.stream[RELOQ_TARGET_F1 + i];
                assert(stream->count == 2);
                assert(stream->points[0].tick == o->target[k] &&
                       stream->points[0].value == first[i]);
                assert(stream->points[1].tick == o->hold[k] &&
                       stream->points[1].value ==
                           (i == 1 ? o->held_f2 : first[i]));
            }
            const struct reloq_target_buffer *av =
                &f.builder.stream[RELOQ_TARGET_AV];
            const struct reloq_target_buffer *ah =
                &f.builder.stream[RELOQ_TARGET_AH];
            assert(av->count == 3 && ah->count == 3);
            assert(av->points[0].tick == o->onset + 1 &&
                   av->points[0].value == o->av);
            assert(av->points[1].tick == o->onset + o->closure &&
                   av->points[1].value == o->av - 2);
            assert(av->points[2].tick == o->onset + o->closure &&
                   av->points[2].value == 0);
            assert(ah->points[0].tick == o->onset + 1 &&
                   ah->points[0].value == 34);
            assert(ah->points[1].tick == o->onset + o->closure &&
                   ah->points[1].value == 34);
            assert(ah->points[2].tick == o->onset + o->closure &&
                   ah->points[2].value == 0);
        }
    }
}

static void errors(void)
{
    struct fixture f;
    int64_t closure = -17, burst = -17, hold = -17, target = -17;
    begin(&f, "`[.1hAp].", 4);
    assert(reloq_german_voiced_stop_vowel_append(
        f.phones, f.count, 81, &f.builder,
        &closure, &burst, &hold, &target) == 1);
    begin(&f, "`[.1bA].", 4);
    assert(reloq_german_voiced_stop_vowel_append(
        f.phones, f.count, 81, &f.builder,
        &closure, &burst, &hold, &target) == 1);
    begin(&f, "`[.0bAp].", 4);
    assert(reloq_german_voiced_stop_vowel_append(
        f.phones, f.count, 81, &f.builder,
        &closure, &burst, &hold, &target) == 1);
    begin(&f, "`[.1bAp].", 4);
    assert(reloq_german_voiced_stop_vowel_append(
        f.phones, f.count, INT64_MAX, &f.builder,
        &closure, &burst, &hold, &target) == -1);
    begin(&f, "`[.1bAp].", 1);
    assert(reloq_german_voiced_stop_vowel_append(
        f.phones, f.count, 81, &f.builder,
        &closure, &burst, &hold, &target) == -1);
    assert(f.builder.stream[RELOQ_TARGET_F1].count == 0 &&
           f.builder.stream[RELOQ_TARGET_AV].count == 0);
    assert(closure == -17 && burst == -17 && hold == -17 && target == -17);
}

int main(void)
{
    grid();
    errors();
    return 0;
}
