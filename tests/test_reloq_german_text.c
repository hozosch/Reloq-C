#include "reloq_german_text.h"
#include "reloq_german_phone_sequence.h"

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

static int checked_cancellation(void)
{
    const unsigned char input[] = "Maus, Schnee! B\xc3\xa4ume?";
    unsigned observed = 0, completed = 0;
    for (unsigned after = 1; after < 150; ++after) {
        struct interrupt state = {0, after};
        unsigned char output[100];
        memset(output, 0xa5, sizeof output);
        size_t needed = 999, bad = 888;
        int result = reloq_german_text_annotate_checked(
            input, sizeof input - 1, output, sizeof output,
            &needed, &bad, NULL, cancelled, &state);
        if (result == RELOQ_DE_TEXT_OK) {
            CHECK(after > 5 && needed > 0);
            ++completed;
            break;
        }
        CHECK(result == RELOQ_DE_TEXT_CANCELLED &&
              needed == 999 && bad == 888);
        for (size_t i = 0; i < sizeof output; ++i)
            CHECK(output[i] == 0xa5);
        ++observed;
    }
    CHECK(observed > 5 && completed == 1);
    return 0;
}

static int covered_sentence(void)
{
    const unsigned char input[] =
        "Maus, Schnee! B\xc3\xa4ume?";
    const unsigned char expected[] =
        "`[.1m'aw's], `[.1Sne]! `[.1b'oj'.0m@]?";
    unsigned char output[128] = {0};
    size_t needed = 123, bad = 456;
    CHECK(reloq_german_text_annotate(input, sizeof input - 1,
                                      NULL, 0, &needed, &bad) ==
          RELOQ_DE_TEXT_OK);
    CHECK(needed == sizeof expected - 1 && bad == 456);
    CHECK(reloq_german_text_annotate(input, sizeof input - 1, output,
                                      sizeof output, &needed, &bad) ==
          RELOQ_DE_TEXT_OK);
    CHECK(needed == sizeof expected - 1 &&
          memcmp(output, expected, sizeof expected) == 0 && bad == 456);

    struct reloq_german_phone_node nodes[10] = {{0}};
    struct reloq_german_phone_sequence_info info = {0};
    CHECK(reloq_german_phone_sequence_build(output, needed, 0,
                                             nodes, 10, &info) == 0);
    CHECK(info.word_count == 3 && info.syllable_count == 4 &&
          info.phone_count == 10);
    CHECK(nodes[0].phone == RELOQ_DE_PHONE_m &&
          nodes[0].source_offset == 4 && nodes[0].word_index == 0);
    CHECK(nodes[2].phone == RELOQ_DE_PHONE_s &&
          nodes[3].phone == RELOQ_DE_PHONE_S &&
          !nodes[2].same_word_right && !nodes[3].same_word_left &&
          nodes[2].right_phone == RELOQ_DE_PHONE_S);
    CHECK(nodes[6].word_index == 2 && nodes[6].stress == 1 &&
          nodes[9].word_index == 2 && nodes[9].stress == 0);
    return 0;
}

static int punctuation_and_composition(void)
{
    const unsigned char input[] =
        "(Ma\xcc\x88use) \xe2\x80\x94 \xe2\x80\x9eSchnee\xe2\x80\x9c\xe2\x80\xa6";
    /* The decomposed umlaut takes the same spelling as precomposed ä. */
    const unsigned char expected[] =
        "(`[.1m'oj'.0z@]) \x97 \x84`[.1Sne]\x93\x85";
    unsigned char output[128] = {0};
    size_t length = 0;
    CHECK(reloq_german_text_annotate(input, sizeof input - 1,
                                      output, sizeof output, &length,
                                      NULL) == RELOQ_DE_TEXT_OK);
    CHECK(length == sizeof expected - 1 &&
          memcmp(output, expected, sizeof expected) == 0);
    return 0;
}

static int productive_nominal_plurals(void)
{
    const unsigned char input[] =
        "L\xc3\xb6sungen, Freiheiten, Sauberkeiten.";
    const unsigned char expected[] =
        "`[.1l'oe'.0zUG.0@n], `[.1fr'aj'.0h'aj'.0t@n], "
        "`[.1z'aw'.0bR.0k'aj'.0t@n].";
    unsigned char output[160] = {0};
    size_t bytes = 0;
    CHECK(reloq_german_text_annotate(input, sizeof input - 1,
                                      output, sizeof output, &bytes,
                                      NULL) == RELOQ_DE_TEXT_OK);
    CHECK(bytes == sizeof expected - 1 &&
          memcmp(output, expected, sizeof expected) == 0);
    return 0;
}

static int transactional_failures(void)
{
    const unsigned char uncovered[] = "Maus, Bach!";
    const unsigned char decomposed[] =
        "Ba\xcc\x88ume, Bach";
    const unsigned char malformed[] = {'M', 'a', 'u', 's', ' ', 0xc0, 0xaf};
    const unsigned char unmappable[] = "Maus \xf0\x9f\x98\x80";
    const unsigned char digit[] = "Maus 2";
    const unsigned char control[] = "Maus `p40";
    const unsigned char foreign[] = "Maus \xe2\x82\xac";
    const struct {
        const unsigned char *input;
        size_t length;
        int status;
        size_t offset;
    } cases[] = {
        {uncovered, sizeof uncovered - 1, RELOQ_DE_TEXT_UNCOVERED, 6},
        {decomposed, sizeof decomposed - 1, RELOQ_DE_TEXT_UNCOVERED, 9},
        {malformed, sizeof malformed, RELOQ_DE_TEXT_INVALID, 5},
        {unmappable, sizeof unmappable - 1, RELOQ_DE_TEXT_INVALID, 5},
        {digit, sizeof digit - 1, RELOQ_DE_TEXT_INVALID, 5},
        {control, sizeof control - 1, RELOQ_DE_TEXT_INVALID, 5},
        {foreign, sizeof foreign - 1, RELOQ_DE_TEXT_INVALID, 5}
    };
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        unsigned char output[] = "untouched";
        size_t length = 999, bad = 999;
        CHECK(reloq_german_text_annotate(cases[i].input, cases[i].length,
                                          output, sizeof output,
                                          &length, &bad) == cases[i].status);
        CHECK(length == 999 && bad == cases[i].offset &&
              memcmp(output, "untouched", sizeof output) == 0);
    }
    return 0;
}

static int capacity_and_edges(void)
{
    const unsigned char input[] = "Maus";
    unsigned char output[32];
    memset(output, 0xa5, sizeof output);
    size_t needed = 111, bad = 222;
    CHECK(reloq_german_text_annotate(input, sizeof input - 1,
                                      output, strlen("`[.1m'aw's]"),
                                      &needed, &bad) ==
          RELOQ_DE_TEXT_CAPACITY);
    CHECK(needed == strlen("`[.1m'aw's]") && bad == 222);
    for (size_t i = 0; i < sizeof output; ++i)
        CHECK(output[i] == 0xa5);

    unsigned char inplace[32] = "Maus";
    CHECK(reloq_german_text_annotate(inplace, 4, inplace,
                                      sizeof inplace, &needed, &bad) ==
          RELOQ_DE_TEXT_OK);
    CHECK(strcmp((const char *)inplace, "`[.1m'aw's]") == 0);

    const unsigned char empty[] = "";
    CHECK(reloq_german_text_annotate(empty, 0, output,
                                      sizeof output, &needed, NULL) ==
          RELOQ_DE_TEXT_OK && needed == 0 && output[0] == 0);
    CHECK(reloq_german_text_annotate(NULL, 0, output,
                                      sizeof output, &needed, &bad) ==
          RELOQ_DE_TEXT_INVALID && needed == 0 && bad == 222);
    return 0;
}

int main(void)
{
    int result = checked_cancellation();
    if (result) return result;
    result = covered_sentence();
    if (result) return result;
    result = punctuation_and_composition();
    if (result) return result;
    result = productive_nominal_plurals();
    if (result) return result;
    result = transactional_failures();
    if (result) return result;
    return capacity_and_edges();
}
