#include "reloq_german_best_effort_text.h"

#include "reloq_german_annotation.h"
#include "reloq_german_text.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t annotation_groups(const unsigned char *text)
{
    struct reloq_german_annotation_cursor cursor = {0};
    struct reloq_german_annotation_event event;
    size_t groups = 0;
    int state;
    while ((state = reloq_german_annotation_next(
                text, strlen((const char *)text), &cursor, 0, &event)) == 1)
        if (event.kind == RELOQ_DE_ANNOTATION_SPR &&
            event.spr.kind == RELOQ_DE_SPR_WORD_OPEN) ++groups;
    assert(state == 0);
    return groups;
}

static void check_annotation(const unsigned char *text)
{
    assert(annotation_groups(text) > 0);
}

static int cancel_now(void *ignored)
{
    (void)ignored;
    return 1;
}

int main(void)
{
    const unsigned char *simple = (const unsigned char *)"Paket.";
    unsigned char *experimental = NULL;
    size_t length = 0, guessed = 999;
    assert(reloq_german_best_effort_annotate(
        simple, strlen((const char *)simple), NULL,
        &experimental, &length, &guessed, NULL) == 0);
    assert(guessed == 0);
    unsigned char strict[128];
    size_t strict_length = 0;
    assert(reloq_german_text_annotate(
        simple, strlen((const char *)simple), strict, sizeof strict,
        &strict_length, NULL) == 0);
    assert(length == strict_length && !memcmp(experimental, strict, length));
    free(experimental);

    /* Public reference pronunciations: spelling is not ordinary G2P of
     * pseudo-words such as Ce/En. Single names and expanded sequences must
     * share the same phones, including the secondary stress in Eszett. */
    static const struct { const char *text, *spr; } letters[] = {
        {"C", "`[.1Te]"}, {"F", "`[.1Ef]"}, {"l", "`[.1El]"},
        {"m", "`[.1Em]"}, {"N", "`[.1En]"}, {"r", "`[.1Er]"},
        {"\xc3\x9f", "`[.1Es.2TEt]"},
        {"CD", "`[.1Te] `2 `[.1de]"},
        {"NVDA", "`[.1En] `[.1f'aw'] `[.1de] `2 `[.1a]"},
        {"HTML", "`[.1ha] `[.1te] `[.1Em] `2 `[.1El]"},
        {"F L M N R C", "`[.1Ef] `[.1El] `[.1Em] `[.1En] `[.1Er] `2 `[.1Te]"}
    };
    for (size_t i = 0; i < sizeof letters / sizeof *letters; ++i) {
        assert(reloq_german_best_effort_annotate(
            (const unsigned char *)letters[i].text, strlen(letters[i].text),
            NULL, &experimental, &length, &guessed, NULL) == 0);
        assert(!strcmp((const char *)experimental, letters[i].spr));
        check_annotation(experimental);
        free(experimental);
    }

    /* Natural-text output passes through the native phrase-stress model.
     * The strict isolated-word lexicon still carries its lexical .1, while
     * the same article in a sentence is weak. */
    const unsigned char *article = (const unsigned char *)"Die Katze.";
    assert(reloq_german_best_effort_annotate(
        article, strlen((const char *)article), NULL,
        &experimental, &length, &guessed, NULL) == 0);
    assert(strcmp((const char *)experimental,
                  "`00 `[.0di] `2 `[.1kA.0T@].") == 0);
    free(experimental);

    /* Inserted weak-word controls must not leave native ECI indexes at
     * pre-insertion annotation offsets. The first mark remains before the
     * first word; the others advance monotonically with the groups. */
    struct reloq_german_best_effort_boundary weak_indices[] = {
        {0, 999}, {1, 999}, {3, 999}, {4, 999},
        {5, 999}, {9, 999}, {10, 999}
    };
    assert(reloq_german_best_effort_annotate_indexed_checked(
        article, strlen((const char *)article), NULL,
        &experimental, &length, &guessed, NULL,
        weak_indices, sizeof weak_indices / sizeof *weak_indices,
        NULL, NULL) == 0);
    assert(strcmp((const char *)experimental,
                  "`00 `[.0di] `2 `[.1kA.0T@].") == 0);
    static const size_t expected_weak_offsets[] = {
        0, 11, 11, 12, 26, 26, 27
    };
    for (size_t i = 0;
         i < sizeof weak_indices / sizeof *weak_indices; ++i)
        assert(weak_indices[i].annotation_offset ==
               expected_weak_offsets[i]);
    assert(weak_indices[6].annotation_offset == length);
    free(experimental);

    const unsigned char *phrase = (const unsigned char *)
        "\xe2\x80\x9ePeter\xe2\x80\x9c packt f\xc3\xbcnf Pakete.";
    size_t bad = 999;
    assert(reloq_german_best_effort_annotate(
        phrase, strlen((const char *)phrase), NULL,
        &experimental, &length, &guessed, &bad) == 0);
    assert(guessed > 0 && length > 20);
    check_annotation(experimental);
    free(experimental);

    /* Map source marks during the same frontend transaction, including a
     * multibyte UTF-8 umlaut, both sides of a separator, and an interior
     * letter offset. A mark within a token follows its completed word. */
    const unsigned char *indexed = (const unsigned char *)
        "F\xc3\xbcnf Pakete.";
    struct reloq_german_best_effort_boundary boundaries[] = {
        {0, 999}, {1, 999}, {4, 999}, {4, 999},
        {5, 999}, {6, 999}, {12, 999}
    };
    assert(reloq_german_best_effort_annotate_indexed_checked(
        indexed, strlen((const char *)indexed), NULL,
        &experimental, &length, &guessed, &bad,
        boundaries, sizeof boundaries / sizeof *boundaries,
        NULL, NULL) == 0);
    assert(boundaries[0].annotation_offset == 0);
    assert(boundaries[1].annotation_offset ==
           boundaries[2].annotation_offset);
    assert(boundaries[2].annotation_offset ==
           boundaries[3].annotation_offset);
    assert(boundaries[4].annotation_offset ==
           boundaries[3].annotation_offset + 1);
    assert(boundaries[5].annotation_offset >
           boundaries[4].annotation_offset);
    assert(boundaries[6].annotation_offset == length);
    free(experimental);

    /* These natural forms exercise productive token classes as one complete
     * annotation: an ordinal-like period, thousands and decimal separators,
     * letter names, an email address, symbols and a Latin loan diacritic. */
    const unsigned char *mixed = (const unsigned char *)
        "E-Mail-Adresse: Karl@example.de; Caf\xc3\xa9 kostet 1.234,56 \xe2\x82\xac + 42%.";
    assert(reloq_german_best_effort_annotate(
        mixed, strlen((const char *)mixed), NULL,
        &experimental, &length, &guessed, &bad) == 0);
    assert(length > 100 && guessed >= 8);
    check_annotation(experimental);
    /* None of these operators may disappear as silent TEXT tokens. */
    assert(strchr((const char *)experimental, '+') == NULL);
    assert(strchr((const char *)experimental, '%') == NULL);
    free(experimental);

    /* A document symbol is a spoken token in the approximate voice. One
     * unsupported CP1252 scalar, including a Latin loan letter, must not
     * reject all earlier and later words in the same NVDA utterance. */
    static const char *const symbols[] = {
        "Hallo \xc2\xa7 1.", "Das Bild ist \xc2\xa9 2026.",
        "Die Temperatur ist 21 \xc2\xb0" "C.",
        "Die Formel ist 3 \xc3\x97 4.",
        "Die Formel enth\xc3\xa4lt x\xc2\xb2 und Schriftzeichen.",
        "Der Pfad C:\\Users\\Karl.",
        "Sein Name ist S\xc3\xb8ren.",
        "Was steht in {geschweiften} Klammern?"
    };
    for (size_t i = 0; i < sizeof symbols / sizeof *symbols; ++i) {
        const char *sentence = symbols[i];
        assert(reloq_german_best_effort_annotate(
            (const unsigned char *)sentence, strlen(sentence), NULL,
            &experimental, &length, &guessed, &bad) == 0);
        assert(guessed && length > 20);
        check_annotation(experimental);
        free(experimental);
    }

    /* A whole word missed by G2P can still be one pronounceable compound
     * when its constituent words are covered. It must not be rendered as a
     * sequence of individually spoken letter names in the surrounding
     * sentence. */
    const char *compound = "Tonqualit\xc3\xa4t";
    assert(reloq_german_best_effort_annotate(
        (const unsigned char *)compound, strlen(compound), NULL,
        &experimental, &length, &guessed, &bad) == 0);
    assert(guessed == 1 && annotation_groups(experimental) == 1);
    free(experimental);

    /* UTF-8 -> CP1252 must retain German umlauts before folding accents
     * in loanwords. A decomposed diaeresis is the same native vowel, while
     * French é still takes the intentional one-byte loan approximation. */
    static const struct {
        const char *input;
        const char *spr;
    } vowel_cases[] = {
        { "M\xc3\xa4" "dchen", "`[.1m'E:'t.0X@n]" },
        { "Ma\xcc\x88" "dchen", "`[.1m'E:'t.0X@n]" },
        { "B\xc3\xa4" "ume", "`[.1b'oj'.0m@]" },
        { "B\xc3\xb6" "tten", "`[.1b'OE'.0t@n]" },
        { "F\xc3\xbc" "sschen", "`[.1fYs.0X@n]" },
        { "Caf\xc3\xa9", "`[.1ka.0f@]" }
    };
    for (size_t i = 0; i < sizeof vowel_cases / sizeof vowel_cases[0]; ++i) {
        const char *word = vowel_cases[i].input;
        assert(reloq_german_best_effort_annotate(
            (const unsigned char *)word, strlen(word), NULL,
            &experimental, &length, &guessed, &bad) == 0);
        assert(strcmp((const char *)experimental, vowel_cases[i].spr) == 0);
        free(experimental);
    }

    unsigned char large_digits[137];
    memset(large_digits, '9', sizeof large_digits - 1);
    large_digits[sizeof large_digits - 1] = 0;
    assert(reloq_german_best_effort_annotate(
        large_digits, sizeof large_digits - 1, NULL,
        &experimental, &length, &guessed, &bad) == 0);
    assert(guessed > 0);
    check_annotation(experimental);
    free(experimental);

    experimental = (unsigned char *)phrase;
    length = 77;
    guessed = 88;
    const unsigned char *invalid = (const unsigned char *)"Peter ` A.";
    assert(reloq_german_best_effort_annotate(
        invalid, strlen((const char *)invalid), NULL,
        &experimental, &length, &guessed, &bad) == -1);
    assert(experimental == phrase && length == 77 && guessed == 88);
    assert(bad == 6);
    assert(reloq_german_best_effort_annotate_checked(
        mixed, strlen((const char *)mixed), NULL,
        &experimental, &length, &guessed, &bad,
        cancel_now, NULL) == 2);
    assert(experimental == phrase && length == 77 && guessed == 88);
    puts("Experimental German best-effort text adapter: OK");
    return 0;
}
