#include "reloq_german_generic_prosody.h"
#include "reloq_german_phone_sequence.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cancel(void *context)
{
    (void)context;
    return 1;
}

static void stress(const char *text, const char *before, const char *expected)
{
    unsigned char *result = NULL;
    size_t length = 0;
    assert(reloq_german_generic_prosody_reaccent(
        (const unsigned char *)text, strlen(text),
        (const unsigned char *)before, strlen(before), &result, &length,
        NULL, NULL) == 0);
    if (length!=strlen(expected) || strcmp((const char *)result,expected))
        fprintf(stderr,"%s\nactual: %s\nexpected: %s\n",text,result,expected);
    assert(length == strlen(expected));
    assert(!memcmp(result, expected, length + 1));
    free(result);
}

static void check(const char *annotation, size_t length,
                  const enum reloq_german_terminal_mark *expected,
                  size_t count)
{
    enum reloq_german_terminal_mark result[8];
    assert(count <= sizeof result / sizeof result[0]);
    for (size_t i = 0; i < 8; ++i)
        result[i] = RELOQ_DE_TERMINAL_EXCLAMATION;
    assert(reloq_german_generic_prosody_marks(
        (const unsigned char *)annotation, length, count,
        result, 8) == 0);
    for (size_t i = 0; i < count; ++i) assert(result[i] == expected[i]);
    for (size_t i = count; i < 8; ++i)
        assert(result[i] == RELOQ_DE_TERMINAL_EXCLAMATION);
}

int main(void)
{
    static const char connected[] = " `[.1pAk] `2 `[.1tIp].";
    static const enum reloq_german_terminal_mark c[] = {
        RELOQ_DE_TERMINAL_IMPLICIT, RELOQ_DE_TERMINAL_PERIOD};
    check(connected, sizeof connected - 1, c, 2);

    static const char mixed[] = " `[.1pAk], `[.1tIp]; "
                                "`[.1hAk]... `[.1pAk]?";
    static const enum reloq_german_terminal_mark m[] = {
        RELOQ_DE_TERMINAL_COMMA, RELOQ_DE_TERMINAL_SEMICOLON,
        RELOQ_DE_TERMINAL_ELLIPSIS, RELOQ_DE_TERMINAL_QUESTION};
    check(mixed, sizeof mixed - 1, m, 4);

    static const char separate[] = " `[.1pAk].\0 `[.1tIp]\0";
    static const enum reloq_german_terminal_mark s[] = {
        RELOQ_DE_TERMINAL_PERIOD, RELOQ_DE_TERMINAL_PERIOD};
    check(separate, sizeof separate - 1, s, 2);

    enum reloq_german_terminal_mark unchanged[2] = {
        RELOQ_DE_TERMINAL_QUESTION, RELOQ_DE_TERMINAL_EXCLAMATION};
    assert(reloq_german_generic_prosody_marks(
        (const unsigned char *)connected, sizeof connected - 1, 3,
        unchanged, 2) == -1);
    assert(unchanged[0] == RELOQ_DE_TERMINAL_QUESTION &&
           unchanged[1] == RELOQ_DE_TERMINAL_EXCLAMATION);
    int32_t ticks = -1;
    assert(reloq_german_generic_prosody_boundary_ticks(c[0], &ticks) == 0 &&
           ticks == 0);
    assert(reloq_german_generic_prosody_boundary_ticks(m[0], &ticks) == 0 &&
           ticks == 210);
    assert(reloq_german_generic_prosody_boundary_ticks(m[1], &ticks) == 0 &&
           ticks == 310);
    assert(reloq_german_generic_prosody_boundary_ticks(s[0], &ticks) == 0 &&
           ticks == 460);
    assert(reloq_german_generic_prosody_boundary_ticks(99, &ticks) == -1 &&
           ticks == 460);
    stress("Der Hund bellt.", "`[.1der] `[.1hUnt] `[.1bElt].",
           "`00 `[.0der] `[.1hUnt] `2 `[.1bElt].");
    stress("Wir sehen den Mann.",
           "`[.1vir] `[.1ze.0@n] `[.1den] `[.1mAn].",
           "`00 `[.0vir] `0 `[.1ze.0@n] `00 `[.0den] `2 `[.1mAn].");
    stress("Hast du das schon gesehen?",
           "`[.1hAst] `[.1du] `[.1dAs] `[.1Son] `[.0g@.1ze.0@n]?",
           "`00 `[.0hAst] `00 `[.0du] `00 `[.0dAs] "
           "`00 `[.0Son] `2 `[.0g@.1ze.0@n]?");
    stress("Was willst du?", "`[.1vAs] `[.1vIlst] `[.1du]?",
           "`00 `[.0vAs] `2 `[.1vIlst] `00 `[.0du]?");
    stress("Ein Mann sitzt.", "`[.1'aj'n] `[.1mAn] `[.1zITt].",
           "`00 `[.0'aj'n] `[.1mAn] `2 `[.1zITt].");
    stress("Die Katze", "`[.0di] `[.1kA.0T@]",
           "`00 `[.0di] `2 `[.1kA.0T@]");
    stress("Die Katze", "`00 `[.0di] `[.1kA.0T@]",
           "`00 `[.0di] `[.1kA.0T@]");
    stress("Die Katze", "`1 `[.0di] `[.1kA.0T@]",
           "`1 `[.0di] `[.1kA.0T@]");
    stress("Peter", "`[.1pe.0tR].", "`[.1pe.0tR].");
    stress("12", "`[.1tsv'aj'] `[.1tse].",
           "`[.1tsv'aj'] `[.1tse].");
    stress("Bahnsteig bleibt.", "`[.1ban.2St'aj'k] `[.1bl'aj'pt].",
           "`2 `[.1ban.2St'aj'k] `0 `[.1bl'aj'pt].");
    stress("Ich sehe ihn.","`[.1IX] `[.1ze.0@] `[.1in].",
           "`00 `[.0IX] `2 `[.1ze.0@] `00 `[.0in].");
    stress("Meine Stimme spricht.","`[.1m'aj'.0n@] `[.1StI.0m@] `[.1SprIXt].",
           "`00 `[.1m'aj'.0n@] `[.1StI.0m@] `2 `[.1SprIXt].");
    stress("Ich komme auch.","`[.1IX] `[.1kO.0m@] `[.1'aw'x].",
           "`00 `[.0IX] `0 `[.1kO.0m@] `2 `[.1'aw'x].");
    stress("Ich sehe gerade.","`[.1IX] `[.1ze.0@] `[.0g@.1ra.0d@].",
           "`00 `[.0IX] `2 `[.1ze.0@] `0 `[.0g@.1ra.0d@].");
    struct reloq_german_phone_node weak = {
        .phone = RELOQ_DE_PHONE_i, .weak_word_control = 1, .stress = 0};
    int32_t weak_ticks = -1;
    assert(reloq_german_generic_prosody_weak_vowel_ticks(
               &weak, 165, 1, &weak_ticks) == 1 && weak_ticks == 61);
    weak.phone = RELOQ_DE_PHONE_E;
    assert(reloq_german_generic_prosody_weak_vowel_ticks(
               &weak, 104, 1, &weak_ticks) == 1 && weak_ticks == 31);
    weak.phone = RELOQ_DE_PHONE_e;
    weak.same_word_right = weak.same_syllable_right = 1;
    weak.right_phone = RELOQ_DE_PHONE_r;
    assert(reloq_german_generic_prosody_weak_vowel_ticks(
               &weak, 170, 1, &weak_ticks) == 1 && weak_ticks == 35);
    weak.weak_word_control = 0;
    assert(reloq_german_generic_prosody_weak_vowel_ticks(
               &weak, 170, 1, &weak_ticks) == 0 && weak_ticks == 35);
    weak.weak_word_control = 1;
    assert(reloq_german_generic_prosody_weak_vowel_ticks(
               &weak, 170, 0, &weak_ticks) == 0 && weak_ticks == 35);
    weak.phone = RELOQ_DE_PHONE_p;
    assert(reloq_german_generic_prosody_weak_vowel_ticks(
               &weak, 100, 1, &weak_ticks) == 0 && weak_ticks == 35);
    unsigned char *cancelled_result = NULL;
    size_t cancelled_length = 77;
    assert(reloq_german_generic_prosody_reaccent(
        (const unsigned char *)"Die Katze", 9,
        (const unsigned char *)"`[.1di] `[.1kA.0T@]", 20,
        &cancelled_result, &cancelled_length, cancel, NULL) == 1);
    assert(cancelled_result == NULL && cancelled_length == 77);
    puts("generic word/phrase boundary marks: connected, comma, semicolon, "
         "ellipsis, full sentences; grammatical stress classes");
    return 0;
}
