#include "reloq_phonetic.h"

#include <stdint.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

struct expected {
    enum reloq_phonetic_kind kind;
    const char *spelling;
    uint32_t number;
};

static int check_tokens(const unsigned char *source, size_t length,
                        const struct expected *expected, size_t count)
{
    struct reloq_phonetic_cursor cursor = {0};
    struct reloq_phonetic_token token = {0};
    size_t at = 0;
    for (size_t i = 0; i < count; ++i) {
        const size_t len = strlen(expected[i].spelling);
        if (reloq_phonetic_next(source, length, &cursor, 1000, &token) != 1 ||
            token.offset != at || token.kind != expected[i].kind ||
            (token.kind != RELOQ_PHONETIC_CALLBACK_NUL &&
             (token.length != len ||
              memcmp(source + at, expected[i].spelling, len) != 0)) ||
            (token.kind == RELOQ_PHONETIC_CALLBACK_NUL && token.length != 1) ||
            token.number != expected[i].number)
            return -1;
        at += token.length;
    }
    return at == length &&
           reloq_phonetic_next(source, length, &cursor, 1000, &token) == 0
               ? 0 : -1;
}

int main(void)
{
    /* Natural Apple ECI annotation, German. Digits may be medial. */
    const unsigned char german[] = " `[.1In.0tR.0nEt] `00 `[.0r0x].";
    const struct expected german_tokens[] = {
        {RELOQ_PHONETIC_TEXT, " ", 0},
        {RELOQ_PHONETIC_GROUP_OPEN, "`[", 0},
        {RELOQ_PHONETIC_SEGMENT_DOT, ".", 0},
        {RELOQ_PHONETIC_MODIFIER_DIGITS, "1", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "In", 0},
        {RELOQ_PHONETIC_SEGMENT_DOT, ".", 0},
        {RELOQ_PHONETIC_MODIFIER_DIGITS, "0", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "tR", 0},
        {RELOQ_PHONETIC_SEGMENT_DOT, ".", 0},
        {RELOQ_PHONETIC_MODIFIER_DIGITS, "0", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "nEt", 0},
        {RELOQ_PHONETIC_GROUP_CLOSE, "]", 0},
        {RELOQ_PHONETIC_TEXT, " ", 0},
        {RELOQ_PHONETIC_CONTROL_DIGITS, "`00", 0},
        {RELOQ_PHONETIC_TEXT, " ", 0},
        {RELOQ_PHONETIC_GROUP_OPEN, "`[", 0},
        {RELOQ_PHONETIC_SEGMENT_DOT, ".", 0},
        {RELOQ_PHONETIC_MODIFIER_DIGITS, "0", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "r", 0},
        {RELOQ_PHONETIC_MODIFIER_DIGITS, "0", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "x", 0},
        {RELOQ_PHONETIC_GROUP_CLOSE, "]", 0},
        {RELOQ_PHONETIC_TEXT, ".", 0},
        {RELOQ_PHONETIC_CALLBACK_NUL, "", 0}
    };
    CHECK(check_tokens(german, sizeof german, german_tokens,
                       sizeof german_tokens / sizeof german_tokens[0]) == 0);

    /* French names include quoted phones and a colon. These punctuation
     * bytes are exposed; the language's phone table owns interpretation. */
    const unsigned char french[] = "`[.l1'i:'.'v:'r] `p40 ";
    const struct expected french_tokens[] = {
        {RELOQ_PHONETIC_GROUP_OPEN, "`[", 0},
        {RELOQ_PHONETIC_SEGMENT_DOT, ".", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "l", 0},
        {RELOQ_PHONETIC_MODIFIER_DIGITS, "1", 0},
        {RELOQ_PHONETIC_PHONE_QUOTE, "'", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "i", 0},
        {RELOQ_PHONETIC_LENGTH_COLON, ":", 0},
        {RELOQ_PHONETIC_PHONE_QUOTE, "'", 0},
        {RELOQ_PHONETIC_SEGMENT_DOT, ".", 0},
        {RELOQ_PHONETIC_PHONE_QUOTE, "'", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "v", 0},
        {RELOQ_PHONETIC_LENGTH_COLON, ":", 0},
        {RELOQ_PHONETIC_PHONE_QUOTE, "'", 0},
        {RELOQ_PHONETIC_PHONE_RUN, "r", 0},
        {RELOQ_PHONETIC_GROUP_CLOSE, "]", 0},
        {RELOQ_PHONETIC_TEXT, " ", 0},
        {RELOQ_PHONETIC_EXPLICIT_PAUSE, "`p40", 40},
        {RELOQ_PHONETIC_TEXT, " ", 0},
        {RELOQ_PHONETIC_CALLBACK_NUL, "", 0}
    };
    CHECK(check_tokens(french, sizeof french, french_tokens,
                       sizeof french_tokens / sizeof french_tokens[0]) == 0);

    /* Korean output commonly has no stress digits at all. */
    const unsigned char korean[] = "`[.an.nyE.'ng'ha.se.yo]";
    struct reloq_phonetic_cursor cursor = {0};
    struct reloq_phonetic_token token = {0};
    int count = 0;
    while (reloq_phonetic_next(korean, sizeof korean - 1, &cursor,
                               1000, &token) == 1) {
        CHECK(token.kind != RELOQ_PHONETIC_MODIFIER_DIGITS &&
              token.length > 0 && ++count < 32);
    }
    CHECK(cursor.offset == sizeof korean - 1 && !cursor.in_group);

    /* ECI callback captures may concatenate sentence fragments, each with
     * its own NUL. A NUL between complete groups is a boundary, not EOF. */
    const unsigned char fragments[] = "`[.1ja].\0 `[.0ne].\0";
    cursor = (struct reloq_phonetic_cursor){0};
    int nul_count = 0;
    while (reloq_phonetic_next(fragments, sizeof fragments - 1, &cursor,
                               1000, &token) == 1) {
        if (token.kind == RELOQ_PHONETIC_CALLBACK_NUL)
            ++nul_count;
    }
    CHECK(nul_count == 2 && cursor.offset == sizeof fragments - 1);

    const unsigned char escaped[] = "a\\`p99 b`2 `[.1x]";
    cursor = (struct reloq_phonetic_cursor){0};
    CHECK(reloq_phonetic_next(escaped, sizeof escaped - 1,
                             &cursor, 50, &token) == 1 &&
          token.kind == RELOQ_PHONETIC_TEXT && token.length == 8);
    CHECK(reloq_phonetic_next(escaped, sizeof escaped - 1,
                             &cursor, 50, &token) == 1 &&
          token.kind == RELOQ_PHONETIC_CONTROL_DIGITS && token.number == 2);
    const unsigned char escaped_group[] = "`[a\\]b\\.c]";
    cursor = (struct reloq_phonetic_cursor){0};
    CHECK(reloq_phonetic_next(escaped_group, sizeof escaped_group - 1,
                             &cursor, 50, &token) == 1 &&
          token.kind == RELOQ_PHONETIC_GROUP_OPEN);
    CHECK(reloq_phonetic_next(escaped_group, sizeof escaped_group - 1,
                             &cursor, 50, &token) == 1 &&
          token.kind == RELOQ_PHONETIC_PHONE_RUN && token.length == 7);
    CHECK(reloq_phonetic_next(escaped_group, sizeof escaped_group - 1,
                             &cursor, 50, &token) == 1 &&
          token.kind == RELOQ_PHONETIC_GROUP_CLOSE);

    const unsigned char oversized[] = "`p4294967296 ";
    const unsigned char ambiguous[] = "`p40x ";
    cursor = (struct reloq_phonetic_cursor){0};
    token = (struct reloq_phonetic_token){RELOQ_PHONETIC_TEXT, 23, 5, 7};
    CHECK(reloq_phonetic_next(oversized, sizeof oversized - 1,
                             &cursor, UINT32_MAX, &token) == -1 &&
          cursor.offset == 0 && token.offset == 23);
    CHECK(reloq_phonetic_next(ambiguous, sizeof ambiguous - 1,
                             &cursor, 50, &token) == -1 &&
          cursor.offset == 0 && token.offset == 23);
    const unsigned char incomplete[] = "`[.1x";
    cursor = (struct reloq_phonetic_cursor){0};
    while (reloq_phonetic_next(incomplete, sizeof incomplete - 1,
                               &cursor, 50, &token) == 1) {}
    CHECK(cursor.in_group && cursor.offset == sizeof incomplete - 1);
    const unsigned char embedded_nul[] = {'a', 0, 'b'};
    cursor = (struct reloq_phonetic_cursor){0};
    CHECK(reloq_phonetic_next(embedded_nul, 3, &cursor, 50, &token) == 1 &&
          token.kind == RELOQ_PHONETIC_TEXT);
    CHECK(reloq_phonetic_next(embedded_nul, 3, &cursor, 50, &token) == 1 &&
          token.kind == RELOQ_PHONETIC_CALLBACK_NUL && cursor.offset == 2);
    CHECK(reloq_phonetic_next(embedded_nul, 3, &cursor, 50, &token) == 1 &&
          token.kind == RELOQ_PHONETIC_TEXT && cursor.offset == 3);
    CHECK(reloq_phonetic_next(NULL, 0, &cursor, 50, &token) == -1);
    return 0;
}
