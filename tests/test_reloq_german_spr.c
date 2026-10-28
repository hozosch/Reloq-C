#include "reloq_german_spr.h"

#include <string.h>

#define CHECK(test) do { if (!(test)) return __LINE__; } while (0)

struct expected {
    enum reloq_german_spr_kind kind;
    const char *span;
    size_t syllable;
    enum reloq_german_phone phone;
    uint8_t stress;
};

static int check_group(const unsigned char *source, size_t length,
                       const struct expected *expected, size_t count)
{
    struct reloq_german_spr_cursor cursor = {0};
    struct reloq_german_spr_event event = {0};
    size_t offset = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t width = strlen(expected[i].span);
        CHECK(reloq_german_spr_next(source, length, &cursor, &event) == 1);
        CHECK(event.kind == expected[i].kind && event.offset == offset &&
              event.length == width && event.syllable == expected[i].syllable &&
              event.phone == expected[i].phone &&
              event.stress == expected[i].stress);
        CHECK(memcmp(source + offset, expected[i].span, width) == 0);
        offset += width;
    }
    CHECK(offset == length && cursor.offset == length);
    CHECK(reloq_german_spr_next(source, length, &cursor, &event) == 0);
    return 0;
}

static int rejects_at(const char *source, size_t failing_offset)
{
    struct reloq_german_spr_cursor cursor = {0};
    struct reloq_german_spr_event event = {0};
    size_t length = strlen(source);
    int result;
    do {
        struct reloq_german_spr_cursor before = cursor;
        event.kind = RELOQ_DE_SPR_PHONE;
        event.offset = 812;
        result = reloq_german_spr_next((const unsigned char *)source,
                                       length, &cursor, &event);
        if (result < 0) {
            CHECK(memcmp(&before, &cursor, sizeof cursor) == 0);
            CHECK(event.kind == RELOQ_DE_SPR_PHONE && event.offset == 812);
            CHECK(cursor.offset == failing_offset);
        }
    } while (result > 0);
    CHECK(result == -1);
    return 0;
}

static int accepts(const char *source)
{
    struct reloq_german_spr_cursor cursor={0};
    struct reloq_german_spr_event event={0};
    int result;
    do { result=reloq_german_spr_next((const unsigned char *)source,
        strlen(source),&cursor,&event); } while(result>0);
    CHECK(result==0 && cursor.offset==strlen(source));
    return 0;
}

int main(void)
{
    const unsigned char compound[] = "`[.1Sprax.2'aw's.2ga.0b@]";
    const struct expected events[] = {
        {RELOQ_DE_SPR_WORD_OPEN, "`[", 0, 0, 0},
        {RELOQ_DE_SPR_SYLLABLE, ".", 0, 0, 0},
        {RELOQ_DE_SPR_STRESS, "1", 0, 0, 1},
        {RELOQ_DE_SPR_PHONE, "S", 0, RELOQ_DE_PHONE_S, 0},
        {RELOQ_DE_SPR_PHONE, "p", 0, RELOQ_DE_PHONE_p, 0},
        {RELOQ_DE_SPR_PHONE, "r", 0, RELOQ_DE_PHONE_r, 0},
        {RELOQ_DE_SPR_PHONE, "a", 0, RELOQ_DE_PHONE_a, 0},
        {RELOQ_DE_SPR_PHONE, "x", 0, RELOQ_DE_PHONE_x, 0},
        {RELOQ_DE_SPR_SYLLABLE, ".", 1, 0, 0},
        {RELOQ_DE_SPR_STRESS, "2", 1, 0, 2},
        {RELOQ_DE_SPR_PHONE, "'aw'", 1, RELOQ_DE_PHONE_Q_aw, 0},
        {RELOQ_DE_SPR_PHONE, "s", 1, RELOQ_DE_PHONE_s, 0},
        {RELOQ_DE_SPR_SYLLABLE, ".", 2, 0, 0},
        {RELOQ_DE_SPR_STRESS, "2", 2, 0, 2},
        {RELOQ_DE_SPR_PHONE, "g", 2, RELOQ_DE_PHONE_g, 0},
        {RELOQ_DE_SPR_PHONE, "a", 2, RELOQ_DE_PHONE_a, 0},
        {RELOQ_DE_SPR_SYLLABLE, ".", 3, 0, 0},
        {RELOQ_DE_SPR_STRESS, "0", 3, 0, 0},
        {RELOQ_DE_SPR_PHONE, "b", 3, RELOQ_DE_PHONE_b, 0},
        {RELOQ_DE_SPR_PHONE, "@", 3, RELOQ_DE_PHONE_AT, 0},
        {RELOQ_DE_SPR_WORD_CLOSE, "]", 0, 0, 0}
    };
    CHECK(check_group(compound, sizeof compound - 1, events,
                      sizeof events / sizeof events[0]) == 0);

    /* A stress marker may follow an onset phone, and R may carry a
     * syllable without one of the documented vowel symbols. */
    const unsigned char medial[] = "`[.1kR.0sR]";
    struct reloq_german_spr_cursor cursor = {0};
    struct reloq_german_spr_event event = {0};
    int phones = 0, stresses = 0, syllables = 0;
    while (reloq_german_spr_next(medial, sizeof medial - 1,
                                 &cursor, &event) == 1) {
        phones += event.kind == RELOQ_DE_SPR_PHONE;
        stresses += event.kind == RELOQ_DE_SPR_STRESS;
        syllables += event.kind == RELOQ_DE_SPR_SYLLABLE;
    }
    CHECK(phones == 4 && stresses == 2 && syllables == 2);

    const unsigned char mid_stress[] = "[.r0x]";
    cursor = (struct reloq_german_spr_cursor){0};
    int saw_medial = 0;
    while (reloq_german_spr_next(mid_stress, sizeof mid_stress - 1,
                                 &cursor, &event) == 1) {
        if (event.kind == RELOQ_DE_SPR_STRESS) {
            CHECK(event.offset == 3 && event.stress == 0);
            ++saw_medial;
        }
    }
    CHECK(saw_medial == 1);

    const unsigned char no_dot[] = "[1'oe~'x]";
    cursor = (struct reloq_german_spr_cursor){0};
    CHECK(reloq_german_spr_next(no_dot, sizeof no_dot - 1,
                                &cursor, &event) == 1 &&
          event.kind == RELOQ_DE_SPR_WORD_OPEN);
    CHECK(reloq_german_spr_next(no_dot, sizeof no_dot - 1,
                                &cursor, &event) == 1 &&
          event.kind == RELOQ_DE_SPR_SYLLABLE && event.length == 0 &&
          event.offset == 1 && cursor.offset == 1);
    CHECK(reloq_german_spr_next(no_dot, sizeof no_dot - 1,
                                &cursor, &event) == 1 &&
          event.kind == RELOQ_DE_SPR_STRESS && event.stress == 1);
    CHECK(reloq_german_spr_next(no_dot, sizeof no_dot - 1,
                                &cursor, &event) == 1 &&
          event.kind == RELOQ_DE_SPR_PHONE &&
          event.phone == RELOQ_DE_PHONE_Q_oe_TILDE && event.length == 5);

    CHECK(rejects_at("`[.1a.'bad']", 6) == 0);
    CHECK(rejects_at("`[.1a.'aw", 6) == 0);
    CHECK(rejects_at("`[.1a.0]", 7) == 0);
    CHECK(accepts("`[.0a.2b]") == 0); /* optional primary stress */
    CHECK(rejects_at("`[.1a.1b]", 6) == 0); /* repeated primary stress */
    CHECK(accepts("`[.0b'oj'.0lR]") == 0);
    CHECK(rejects_at("`[.1a0b]", 5) == 0); /* two marks in one syllable */
    CHECK(rejects_at("`[.1a]x", 6) == 0); /* exactly one group */
    CHECK(rejects_at("`[.1a3]", 5) == 0);
    CHECK(rejects_at("`[.1a..0b]", 6) == 0);
    CHECK(rejects_at("`[.1a", 5) == 0);
    CHECK(rejects_at("oops", 0) == 0);
    CHECK(reloq_german_spr_next(NULL, 0, &cursor, &event) == -1);
    return 0;
}
