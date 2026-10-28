#include "reloq_german_generic_melody.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct fixture {
    struct reloq_target_builder builder;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][32];
};

static void reset(struct fixture *f)
{
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], 32, 0};
    assert(!reloq_target_builder_begin(&f->builder, buffers));
}

static void expected(const struct fixture *f, size_t index,
                     int64_t tick, int32_t value)
{
    assert(index < f->builder.stream[RELOQ_TARGET_F0].count);
    const struct reloq_acoustic_point point =
        f->builder.stream[RELOQ_TARGET_F0].points[index];
    if (point.tick != tick || point.value != value)
        fprintf(stderr, "F0[%zu] got %lld:%d expected %lld:%d\n", index,
                (long long)point.tick, point.value,
                (long long)tick, value);
    assert(point.tick == tick && point.value == value);
}

static void question_rise_is_terminal(void)
{
    struct reloq_german_generic_melody_syllable syllables[7];
    enum reloq_german_terminal_mark periods[7],questions[7];
    for(size_t i=0;i<7;++i) {
        int64_t at=(int64_t)i*400;
        syllables[i]=(struct reloq_german_generic_melody_syllable){
            .start_tick=at,.onset_begin_tick=at,.nucleus_tick=at+80,
            .closure_tick=at+220,.end_tick=at+220,.onset_end_tick=at+80,
            .word_index=i,.first_phone=RELOQ_DE_PHONE_m,.stress=1};
        periods[i]=questions[i]=RELOQ_DE_TERMINAL_IMPLICIT;
    }
    periods[6]=RELOQ_DE_TERMINAL_PERIOD;
    questions[6]=RELOQ_DE_TERMINAL_QUESTION;
    syllables[6].prosody_control=RELOQ_DE_WORD_CONTROL_2;
    struct fixture declarative,question;
    reset(&declarative);reset(&question);
    assert(!reloq_german_generic_melody_append(syllables,7,periods,7,30,1,1,&declarative.builder));
    assert(!reloq_german_generic_melody_append(syllables,7,questions,7,30,1,1,&question.builder));
    const struct reloq_target_buffer *a=declarative.builder.stream+RELOQ_TARGET_F0;
    const struct reloq_target_buffer *b=question.builder.stream+RELOQ_TARGET_F0;
    size_t prehead=0;
    while(prehead<a->count && a->points[prehead].tick<syllables[6].start_tick) {
        assert(prehead<b->count);
        assert(a->points[prehead].tick==b->points[prehead].tick);
        assert(a->points[prehead].value==b->points[prehead].value);
        ++prehead;
    }
    assert(prehead>=12); /* ordinary earlier accents survive in questions */
    assert(b->points[b->count-2].tick>=syllables[6].nucleus_tick);
    assert(b->points[b->count-1].value>b->points[b->count-2].value);
    assert(a->points[a->count-1].value<a->points[a->count-2].value);

    /* An explicit earlier focus retains the original nuclear anchor. */
    syllables[0].prosody_control=RELOQ_DE_WORD_CONTROL_2;
    syllables[6].prosody_control=RELOQ_DE_WORD_CONTROL_NONE;
    reset(&question);
    assert(!reloq_german_generic_melody_append(syllables,7,questions,7,30,1,1,&question.builder));
    assert(b->points[1].tick<syllables[0].closure_tick);
    assert(b->points[2].tick>=syllables[1].nucleus_tick);
    assert(b->points[2].tick<syllables[1].closure_tick);
    assert(b->points[b->count-1].value>b->points[b->count-2].value);

    /* The observed first postnuclear tone follows the selected focus,
     * including a weak continuation; no substitute later nucleus appears. */
    for(size_t i=1;i<7;++i){syllables[i].stress=0;syllables[i].prosody_control=RELOQ_DE_WORD_CONTROL_00;}
    reset(&question);
    assert(!reloq_german_generic_melody_append(syllables,7,questions,7,30,1,1,&question.builder));
    assert(b->points[1].tick<syllables[0].closure_tick);
    assert(b->points[2].tick>=syllables[1].nucleus_tick);
    assert(b->points[2].tick<syllables[1].closure_tick);
    assert(b->points[2].value>b->points[1].value);

    /* A wholly deaccented prehead must not stretch the terminal gesture
     * back to its first syllable. Zero inflection still flattens everything. */
    for(size_t i=0;i<7;++i){syllables[i].stress=0;syllables[i].prosody_control=RELOQ_DE_WORD_CONTROL_00;}
    reset(&question);
    assert(!reloq_german_generic_melody_append(syllables,7,questions,7,30,1,1,&question.builder));
    for(size_t i=1;i<b->count;++i)assert(b->points[i].tick>=syllables[6].start_tick);
    reset(&question);
    assert(!reloq_german_generic_melody_append(syllables,7,questions,7,0,1.2,.5,&question.builder));
    for(size_t i=0;i<b->count;++i)assert(b->points[i].value==1426);
}

static void question_prehead_returns_follow_original(void)
{
    struct reloq_german_generic_melody_syllable s[6];
    enum reloq_german_terminal_mark marks[3]={0,0,RELOQ_DE_TERMINAL_QUESTION};
    for(size_t i=0;i<6;++i){int64_t at=(int64_t)i*400;
        s[i]=(struct reloq_german_generic_melody_syllable){
            .start_tick=at,.onset_begin_tick=at,.onset_end_tick=at+80,
            .nucleus_tick=at+80,.closure_tick=at+220,.end_tick=at+340,
            .coda_begin_tick=at+220,.coda_body_end_tick=at+340,
            .word_index=i/2,.first_phone=RELOQ_DE_PHONE_m,.stress=i%2?0:1,
            .prosody_control=i>=4?RELOQ_DE_WORD_CONTROL_2:RELOQ_DE_WORD_CONTROL_NONE};}
    struct fixture f;reset(&f);
    assert(!reloq_german_generic_melody_append(s,6,marks,3,30,1,1,&f.builder));
    const struct reloq_target_buffer *q=f.builder.stream+RELOQ_TARGET_F0;
    int earlier=0,final=0;
    for(size_t i=0;i<q->count;++i){earlier|=q->points[i].tick==316;final|=q->points[i].tick==1116;}
    assert(earlier&&!final);
    marks[2]=RELOQ_DE_TERMINAL_PERIOD;reset(&f);
    assert(!reloq_german_generic_melody_append(s,6,marks,3,30,1,1,&f.builder));
    earlier=final=0;
    for(size_t i=0;i<q->count;++i){earlier|=q->points[i].tick==316;final|=q->points[i].tick==1116;}
    assert(earlier&&final);
}

int main(void)
{
    struct fixture fixture;
    const struct reloq_german_generic_melody_syllable isolated =
        {0, 85, 182, 307, 0, RELOQ_DE_PHONE_m, 1, 0, 0, 0, 0, 0, 0, 0, 0, 85, 0, 0};
    const enum reloq_german_terminal_mark period[] = {
        RELOQ_DE_TERMINAL_PERIOD};
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        &isolated, 1, period, 1, 30, 1.0, 1.0, &fixture.builder));
    assert(fixture.builder.stream[RELOQ_TARGET_F0].count == 4);
    /* These four development anchors were independently observed from
     * Apple on the authored `.1mAp.` annotation; no word is keyed here. */
    expected(&fixture, 0, 8, 1046);
    expected(&fixture, 1, 118, 1296);
    expected(&fixture, 2, 152, 796);
    expected(&fixture, 3, 181, 743);
    for (size_t i = RELOQ_TARGET_F1; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        assert(fixture.builder.stream[i].count == 0);

    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        &isolated, 1, period, 1, 0, 1.0, 1.0, &fixture.builder));
    for (size_t i = 0; i < 4; ++i)
        assert(fixture.builder.stream[RELOQ_TARGET_F0].points[i].value == 1189);

    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        &isolated, 1, period, 1, 100, 1.0, 1.0, &fixture.builder));
    expected(&fixture, 1, 118, 1545);
    expected(&fixture, 2, 152, 594);
    expected(&fixture, 3, 181, 594);

    const enum reloq_german_terminal_mark question[] = {
        RELOQ_DE_TERMINAL_QUESTION};
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        &isolated, 1, question, 1, 30, 1.0, 1.0, &fixture.builder));
    expected(&fixture, 0, 8, 1046);
    expected(&fixture, 1, 143, 957);
    expected(&fixture, 2, 172, 1224);
    expected(&fixture, 3, 181, 1403);

    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        &isolated, 1, period, 1, 30, 1.2, 0.5, &fixture.builder));
    expected(&fixture, 0, 4, 1255);
    expected(&fixture, 1, 59, 1555);

    const struct reloq_german_generic_melody_syllable weak[] = {
        {0, 85, 182, 182, 0, RELOQ_DE_PHONE_m, 1, 0, 0, 0, 0, 0, 0, 0, 0, 85, 0, 0},
        {182, 271, 351, 351, 0, RELOQ_DE_PHONE_t, 0, 0, 0, 0, 0, 0, 0, 182, 182, 271, 0, 0}
    };
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        weak, 2, period, 1, 30, 1.0, 1.0, &fixture.builder));
    expected(&fixture, 0, 8, 1046);
    expected(&fixture, 1, 118, 1296);
    expected(&fixture, 2, 327, 793);
    expected(&fixture, 3, 350, 740);

    const struct reloq_german_generic_melody_syllable two_words[] = {
        {0, 85, 175, 145, 0, RELOQ_DE_PHONE_m, 1, 0, 0, 0, 0, 0, 0, 0, 0, 85, 0, 0},
        {145, 261, 358, 358, 1, RELOQ_DE_PHONE_m, 1, 0, 0, 0, 0, 0, 0, 145, 145, 261, 0, 0}
    };
    /* Gesture closure can outlast a send window, but a syllable's planner
     * `end_tick` must still encompass its nucleus. */
    reset(&fixture);
    assert(reloq_german_generic_melody_append(
        two_words, 2, NULL, 0, 30, 1.0, 1.0, &fixture.builder) == -1);

    const struct reloq_german_generic_melody_syllable split[] = {
        {0, 68, 168, 168, 0, RELOQ_DE_PHONE_m, 1, 0, 0, 0, 0, 0, 0, 0, 0, 68, 0, 0},
        {168, 236, 336, 336, 1, RELOQ_DE_PHONE_m, 1, 0, 0, 0, 0, 0, 0, 168, 168, 236, 0, 0}
    };
    const enum reloq_german_terminal_mark linked[] = {
        RELOQ_DE_TERMINAL_IMPLICIT, RELOQ_DE_TERMINAL_PERIOD};
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        split, 2, linked, 2, 30, 1.0, 1.0, &fixture.builder));
    assert(fixture.builder.stream[RELOQ_TARGET_F0].count == 6);
    for (size_t i = 1; i < 6; ++i)
        assert(fixture.builder.stream[RELOQ_TARGET_F0].points[i].tick >=
               fixture.builder.stream[RELOQ_TARGET_F0].points[i - 1].tick);
    assert(reloq_german_generic_melody_append(
        split, 2, linked, 2, 30, 1.0, 1.0, &fixture.builder) == -1);

    /* The marker changes the F0 for a lexically stressed, nonfinal word,
     * without flattening the next word's accent or its terminal cadence. */
    const struct reloq_german_generic_melody_syllable deaccent[] = {
        {0, 68, 168, 168, 0, RELOQ_DE_PHONE_m, 1,
         RELOQ_DE_WORD_CONTROL_0, 0, 0, 0, 0, 0, 0, 0, 68, 0, 0},
        {168, 236, 336, 336, 1, RELOQ_DE_PHONE_m, 1,
         RELOQ_DE_WORD_CONTROL_2, 0, 0, 0, 0, 0, 168, 168, 236, 0, 0}
    };
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        deaccent, 2, linked, 2, 30, 1.0, 1.0, &fixture.builder));
    expected(&fixture, 0, 3, 1117);
    expected(&fixture, 1, 174, 1043);
    assert(fixture.builder.stream[RELOQ_TARGET_F0].count == 5);
    expected(&fixture, 2, 251, 1293);
    expected(&fixture, 4, 335, 740);

    /* The native inflection and rate operate on this gesture as on all
     * other phrase targets; the zero-inflection contour is globally flat. */
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        deaccent, 2, linked, 2, 0, 1.0, 1.0, &fixture.builder));
    for (size_t i = 0; i < fixture.builder.stream[RELOQ_TARGET_F0].count; ++i)
        assert(fixture.builder.stream[RELOQ_TARGET_F0].points[i].value == 1189);
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        deaccent, 2, linked, 2, 30, 1.2, .5, &fixture.builder));
    /* Pitch scales the physical baseline before integer targets are made. */
    expected(&fixture, 0, 2, 1341);
    expected(&fixture, 1, 87, 1252);

    /* Observed Mann: its voiced n coda belongs to the rime. Ending the
     * contour at the vowel would move the fall to ticks 159/190. */
    const struct reloq_german_generic_melody_syllable mann =
        {0, 85, 312, 312, 0, RELOQ_DE_PHONE_m, 1, 0, 85, 152, 85, 0, 0, 0, 0, 85, 0, 0};
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        &mann, 1, period, 1, 30, 1.0, 1.0, &fixture.builder));
    expected(&fixture, 0, 8, 1046);
    expected(&fixture, 1, 164, 1296);
    expected(&fixture, 2, 243, 796);
    expected(&fixture, 3, 309, 743);

    /* Ja. Mann. starts a fresh original phrase curve at tick 715. */
    const struct reloq_german_generic_melody_syllable after_pause[] = {
        {0, 45, 255, 255, 0, RELOQ_DE_PHONE_j, 1, 0, 45, 255, 45, 0, 0, 0, 0, 45, 0, 0},
        {715, 800, 1027, 1027, 1, RELOQ_DE_PHONE_m, 1, 0, 800, 867, 800, 0, 0, 715, 715, 800, 0, 0}
    };
    const enum reloq_german_terminal_mark two_periods[] = {
        RELOQ_DE_TERMINAL_PERIOD, RELOQ_DE_TERMINAL_PERIOD};
    reset(&fixture);
    assert(!reloq_german_generic_melody_append(
        after_pause, 2, two_periods, 2, 30, 1.0, 1.0, &fixture.builder));
    expected(&fixture, 3, 252, 743);
    expected(&fixture, 4, 714, 743);
    expected(&fixture, 5, 715, 1046);
    expected(&fixture, 6, 723, 1046);
    expected(&fixture, 7, 879, 1296);
    expected(&fixture, 9, 1024, 743);
    /* The completed first phrase holds its final pitch throughout the
     * release and silence. The next phrase cannot bend that old tail. */
    struct reloq_acoustic_curve curve={0,
        fixture.builder.stream[RELOQ_TARGET_F0].points,
        fixture.builder.stream[RELOQ_TARGET_F0].count};
    struct reloq_acoustic_cursor cursor;
    assert(!reloq_acoustic_begin(&curve,&cursor));
    for (int64_t tick=252;tick<715;++tick) {
        float value;
        assert(!reloq_acoustic_value(&cursor,tick,&value) && value==743);
    }

    /* Invalid landmark ordering cannot leak partly built contours. */
    struct reloq_german_generic_melody_syllable invalid = isolated;
    invalid.end_tick = 1;
    reset(&fixture);
    assert(reloq_german_generic_melody_append(
        &invalid, 1, period, 1, 30, 1.0, 1.0, &fixture.builder) == -1);
    question_rise_is_terminal();
    question_prehead_returns_follow_original();
    puts("generic melody targets, punctuation, native inflection and terminal question rise passed");
    return 0;
}
