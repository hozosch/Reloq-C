#include "reloq_german_phone_sequence.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) return __LINE__; } while (0)

struct interrupt {
    unsigned calls, after;
};

static int cancelled(void *context)
{
    struct interrupt *state = context;
    return ++state->calls >= state->after;
}

static int cancelled_in_second_pass(void)
{
    enum { WORDS = 2048 };
    const unsigned char group[] = "`[.1ma] ";
    size_t length = WORDS * (sizeof group - 1);
    unsigned char *annotation = malloc(length + 1);
    struct reloq_german_phone_node *nodes =
        malloc(WORDS * 2 * sizeof *nodes);
    struct reloq_german_phone_node *saved =
        malloc(WORDS * 2 * sizeof *nodes);
    CHECK(annotation && nodes && saved);
    for (size_t i = 0; i < WORDS; ++i)
        memcpy(annotation + i * (sizeof group - 1), group,
               sizeof group - 1);
    annotation[length] = 0;
    struct reloq_german_phone_sequence_info info = {0};
    struct interrupt measure = {0, UINT_MAX};
    CHECK(reloq_german_phone_sequence_build_checked(
        annotation, length, 0, nodes, WORDS * 2, &info,
        cancelled, &measure) == 0 &&
        info.phone_count == WORDS * 2 && info.word_count == WORDS);
    unsigned mid_second_pass = measure.calls * 3 / 4;
    CHECK(mid_second_pass > measure.calls / 2 &&
          mid_second_pass < measure.calls);

    memset(nodes, 0xa5, WORDS * 2 * sizeof *nodes);
    memcpy(saved, nodes, WORDS * 2 * sizeof *nodes);
    info = (struct reloq_german_phone_sequence_info){99, 88, 77};
    struct interrupt stop = {0, mid_second_pass};
    CHECK(reloq_german_phone_sequence_build_checked(
        annotation, length, 0, nodes, WORDS * 2, &info,
        cancelled, &stop) == 1 && stop.calls == mid_second_pass);
    CHECK(!memcmp(nodes, saved, WORDS * 2 * sizeof *nodes) &&
          info.phone_count == 99 && info.word_count == 88 &&
          info.syllable_count == 77);

    free(saved);
    free(nodes);
    free(annotation);
    return 0;
}

static int reference_with_controls(void)
{
    const unsigned char input[] =
        " `[.1hA.0lo] `2 `[.1vElt].`p40\0 `[.1zu.0zi]";
    struct reloq_german_phone_node nodes[12] = {{0}};
    struct reloq_german_phone_sequence_info info = {0};
    CHECK(reloq_german_phone_sequence_build(
              input, sizeof input - 1, 50, nodes, 12, &info) == 0);
    CHECK(info.phone_count == 12 && info.word_count == 3 &&
          info.syllable_count == 5);
    CHECK(nodes[0].phone == RELOQ_DE_PHONE_h &&
          nodes[0].left_phone == RELOQ_DE_PHONE_NONE &&
          nodes[0].right_phone == RELOQ_DE_PHONE_A &&
          nodes[0].stress == 1 && nodes[0].stress_explicit == 1 &&
          nodes[0].word_index == 0 && nodes[0].syllable_index == 0 &&
          nodes[0].same_word_right && nodes[0].same_syllable_right);
    CHECK(nodes[1].phone == RELOQ_DE_PHONE_A &&
          nodes[1].same_syllable_left && !nodes[1].same_syllable_right &&
          nodes[1].same_word_right);
    CHECK(nodes[2].phone == RELOQ_DE_PHONE_l &&
          nodes[2].stress == 0 && nodes[2].stress_explicit &&
          nodes[2].syllable_in_word == 1 && nodes[2].syllable_index == 1);
    CHECK(nodes[3].phone == RELOQ_DE_PHONE_o &&
          nodes[3].right_phone == RELOQ_DE_PHONE_v &&
          !nodes[3].same_word_right && !nodes[3].same_syllable_right);
    CHECK(nodes[4].phone == RELOQ_DE_PHONE_v &&
          nodes[4].word_index == 1 && nodes[4].syllable_index == 2 &&
          nodes[4].left_phone == RELOQ_DE_PHONE_o &&
          !nodes[4].same_word_left && nodes[4].stress == 1 &&
          nodes[4].prosody_control == RELOQ_DE_WORD_CONTROL_2);
    CHECK(nodes[8].phone == RELOQ_DE_PHONE_z &&
          nodes[8].word_index == 2 && nodes[8].syllable_index == 3);
    CHECK(nodes[11].phone == RELOQ_DE_PHONE_i &&
          nodes[11].word_index == 2 && nodes[11].syllable_index == 4 &&
          nodes[11].right_phone == RELOQ_DE_PHONE_NONE &&
          !nodes[11].same_word_right);
    for (size_t i = 0; i < info.phone_count; ++i) {
        CHECK(nodes[i].source_length &&
              nodes[i].source_offset < sizeof input - 1 &&
              nodes[i].source_length <= sizeof input - 1 -
                                        nodes[i].source_offset);
        CHECK((i == 0 || nodes[i].left_phone == nodes[i - 1].phone) &&
              (i == 11 || nodes[i].right_phone == nodes[i + 1].phone));
    }
    return 0;
}

static int delayed_stress_and_quoted_phone(void)
{
    const unsigned char after_phone[] = "`[.a1b.0o]";
    struct reloq_german_phone_node nodes[3] = {{0}};
    struct reloq_german_phone_sequence_info info = {0};
    CHECK(reloq_german_phone_sequence_build(
              after_phone, sizeof after_phone - 1, 0, nodes, 3,
              &info) == 0);
    CHECK(info.phone_count == 3 && info.syllable_count == 2 &&
          nodes[0].stress == 1 && nodes[0].stress_explicit &&
          nodes[1].stress == 1 && nodes[1].stress_explicit &&
          nodes[2].stress == 0 && nodes[2].stress_explicit);

    const unsigned char quoted[] = "`[.1'aw's]";
    CHECK(reloq_german_phone_sequence_build(
              quoted, sizeof quoted - 1, 0, nodes, 3, &info) == 0);
    CHECK(info.phone_count == 2 && info.word_count == 1 &&
          nodes[0].phone == RELOQ_DE_PHONE_Q_aw &&
          nodes[0].source_length == 4 &&
          nodes[1].phone == RELOQ_DE_PHONE_s);

    const unsigned char implicit[] = "`[ao]";
    CHECK(reloq_german_phone_sequence_build(
              implicit, sizeof implicit - 1, 0, nodes, 3, &info) == 0);
    CHECK(info.phone_count == 2 && info.syllable_count == 1 &&
          !nodes[0].stress_explicit && !nodes[1].stress_explicit);
    return 0;
}

static int exact_weak_word_control(void)
{
    const unsigned char input[] =
        "`00 `[.0di] `1 `[.0di] `000 `[.0di] "
        "`00 , `[.0di] `00 `[.1di]";
    struct reloq_german_phone_node nodes[10] = {{0}};
    struct reloq_german_phone_sequence_info info = {0};
    CHECK(reloq_german_phone_sequence_build(
              input, sizeof input - 1, 0, nodes, 10, &info) == 0);
    CHECK(info.word_count == 5 && info.phone_count == 10);
    for (size_t i = 0; i < info.phone_count; ++i)
        CHECK(nodes[i].weak_word_control == (i < 2 || i >= 8));
    CHECK(nodes[8].stress == 1 && nodes[8].weak_word_control);
    CHECK(nodes[0].prosody_control == RELOQ_DE_WORD_CONTROL_00 &&
          nodes[2].prosody_control == RELOQ_DE_WORD_CONTROL_1 &&
          nodes[4].prosody_control == RELOQ_DE_WORD_CONTROL_NONE &&
          nodes[6].prosody_control == RELOQ_DE_WORD_CONTROL_NONE &&
          nodes[8].prosody_control == RELOQ_DE_WORD_CONTROL_00);
    return 0;
}

static int exact_deaccent_control(void)
{
    const unsigned char input[] =
        "`0 `[.1ma] `1 `[.1ma] `2 `[.1ma] `0 : `[.1ma]";
    struct reloq_german_phone_node nodes[8] = {{0}};
    struct reloq_german_phone_sequence_info info = {0};
    CHECK(reloq_german_phone_sequence_build(
              input, sizeof input - 1, 0, nodes, 8, &info) == 0);
    CHECK(info.word_count == 4 && info.phone_count == 8);
    CHECK(nodes[0].prosody_control == RELOQ_DE_WORD_CONTROL_0 &&
          nodes[1].prosody_control == RELOQ_DE_WORD_CONTROL_0 &&
          nodes[2].prosody_control == RELOQ_DE_WORD_CONTROL_1 &&
          nodes[4].prosody_control == RELOQ_DE_WORD_CONTROL_2 &&
          nodes[6].prosody_control == RELOQ_DE_WORD_CONTROL_NONE);
    return 0;
}

static int validation_before_write(void)
{
    const unsigned char input[] = "`[.1hA] `p40";
    struct reloq_german_phone_node nodes[2];
    struct reloq_german_phone_node saved[2];
    memset(nodes, 0xA5, sizeof nodes);
    memcpy(saved, nodes, sizeof saved);
    struct reloq_german_phone_sequence_info info = {9, 8, 7};
    struct reloq_german_phone_sequence_info previous = info;
    CHECK(reloq_german_phone_sequence_build(input, sizeof input - 1,
                                             39, nodes, 2, &info) == -1 &&
          !memcmp(nodes, saved, sizeof nodes) &&
          !memcmp(&info, &previous, sizeof info));
    CHECK(reloq_german_phone_sequence_build(input, sizeof input - 1,
                                             50, nodes, 1, &info) == -1 &&
          !memcmp(nodes, saved, sizeof nodes) &&
          !memcmp(&info, &previous, sizeof info));
    const unsigned char malformed[] = "`[.1hA] `[.1'bad']";
    CHECK(reloq_german_phone_sequence_build(
              malformed, sizeof malformed - 1, 50, nodes, 2,
              &info) == -1 && !memcmp(nodes, saved, sizeof nodes) &&
          !memcmp(&info, &previous, sizeof info));
    CHECK(reloq_german_phone_sequence_build(NULL, 0, 50, nodes, 2,
                                             &info) == -1 &&
          !memcmp(nodes, saved, sizeof nodes));
    const unsigned char empty[] = ".";
    CHECK(reloq_german_phone_sequence_build(empty, sizeof empty - 1,
                                             50, NULL, 0, &info) == 0 &&
          info.phone_count == 0 && info.word_count == 0 &&
          info.syllable_count == 0);
    return 0;
}

static int allocated_sequence(void)
{
    const unsigned char text[]="`[.1kva.0t@] `00 `[.0Unt] `[.1tS'aw']";
    struct reloq_german_phone_node fixed[32],*owned=NULL;
    struct reloq_german_phone_sequence_info a={0},b={0};size_t capacity=99;
    struct interrupt measure={0,UINT_MAX};
    CHECK(!reloq_german_phone_sequence_build_checked(text,sizeof text-1,0,
        fixed,32,&a,cancelled,&measure));
    CHECK(!reloq_german_phone_sequence_allocate_checked(text,sizeof text-1,0,
        &owned,&capacity,&b,NULL,NULL));
    CHECK(capacity==2*a.phone_count&&b.phone_count==a.phone_count&&
        a.word_count==b.word_count&&a.syllable_count==b.syllable_count);
    for(size_t i=0;i<a.phone_count;++i) {
        CHECK(owned[i].phone==fixed[i].phone&&owned[i].source_offset==fixed[i].source_offset&&
            owned[i].word_index==fixed[i].word_index&&owned[i].syllable_index==fixed[i].syllable_index&&
            owned[i].stress==fixed[i].stress&&owned[i].left_phone==fixed[i].left_phone&&
            owned[i].right_phone==fixed[i].right_phone&&owned[i].primary_ahead==fixed[i].primary_ahead);
    }
    free(owned);owned=NULL;capacity=99;b=(struct reloq_german_phone_sequence_info){99,88,77};
    struct interrupt stop={0,measure.calls*3/4};
    CHECK(reloq_german_phone_sequence_allocate_checked(text,sizeof text-1,0,
        &owned,&capacity,&b,cancelled,&stop)==1);
    CHECK(!owned&&capacity==99&&b.phone_count==99&&b.word_count==88);
    CHECK(reloq_german_phone_sequence_allocate_checked((const unsigned char *)"`[bad",5,0,
        &owned,&capacity,&b,NULL,NULL)==-1&&!owned&&capacity==99);
    CHECK(!reloq_german_phone_sequence_allocate_checked((const unsigned char *)".",1,0,
        &owned,&capacity,&b,NULL,NULL)&&!b.phone_count&&capacity==1);
    free(owned);return 0;
}

int main(void)
{
    int result = allocated_sequence();
    if (result) return result;
    result = cancelled_in_second_pass();
    if (result) return result;
    result = reference_with_controls();
    if (result) return result;
    result = delayed_stress_and_quoted_phone();
    if (result) return result;
    result = exact_weak_word_control();
    if (result) return result;
    result = exact_deaccent_control();
    if (result) return result;
    return validation_before_write();
}
