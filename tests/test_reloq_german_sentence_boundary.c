#include "reloq_german_sentence_boundary.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct cancel_count {
    size_t calls;
    size_t at;
};

static int cancelled(void *context)
{
    struct cancel_count *c = context;
    ++c->calls;
    return c->at && c->calls >= c->at;
}

static void chain(const char *text, size_t expected)
{
    size_t count = 0;
    assert(reloq_german_sentence_spans((const unsigned char *)text,
                                       strlen(text), NULL, 0, &count) == 1);
    assert(count == expected);
    struct reloq_german_sentence_span *spans =
        malloc(count * sizeof *spans);
    assert(spans);
    for (size_t i = 0; i < count; ++i)
        spans[i] = (struct reloq_german_sentence_span){666, 777};
    size_t needed = 0;
    assert(reloq_german_sentence_spans((const unsigned char *)text,
                                       strlen(text), spans, count - 1,
                                       &needed) == -2);
    assert(needed == count);
    for (size_t i = 0; i < count; ++i)
        assert(spans[i].offset == 666 && spans[i].length == 777);
    assert(reloq_german_sentence_spans((const unsigned char *)text,
                                       strlen(text), spans, count,
                                       &needed) == 1);
    assert(needed == count);
    size_t cursor = 0;
    for (size_t i = 0; i < count; ++i) {
        assert(spans[i].offset == cursor);
        assert(spans[i].length >= 2);
        const char last = text[spans[i].offset + spans[i].length - 1];
        assert(last == '.' || last == '?' || last == '!' ||
               last == ':' || last == ';' || last == ',');
        cursor += spans[i].length;
        if (i + 1 < count) assert(text[cursor++] == ' ');
    }
    assert(cursor == strlen(text));
    free(spans);
}

int main(void)
{
    chain("Pack. Tipp.", 2);
    chain("Hack. Pack.", 2);
    chain("Pack. Tipp. Hack.", 3);
    chain("H\xc3\xbc" "ck. \xc3\x84" "rger.", 2);
    chain("Pack? Tipp!", 2);
    chain("Pack! Tipp,", 2);
    chain("Pack: Tipp; Hack...", 3);
    chain("Pack; tipp.", 2);
    chain("Pack: tipp.", 2);
    chain("Pack. tipp.", 2);
    chain("H\xc3\xbc" "ck? \xc3\x84" "rger!", 2);
    chain("Pack. Tipp...", 2);
    chain("Hass? Tipp; Hiss! Pack.", 4);
    /* Byte offsets are the ECI index boundaries, before and after the one
     * separator byte. A mark after the first phrase fires before Tipp PCM. */
    const char *mixed = "Hass? Tipp! Pack; Hack: Hut,";
    const struct reloq_german_sentence_span expected[] = {
        {0, 5}, {6, 5}, {12, 5}, {18, 5}, {24, 4}
    };
    struct reloq_german_sentence_span boundaries[5];
    size_t boundary_count = 0;
    assert(reloq_german_sentence_spans(
        (const unsigned char *)mixed, strlen(mixed), boundaries, 5,
        &boundary_count) == 1);
    assert(boundary_count == 5);
    for (size_t i = 0; i < 5; ++i) {
        assert(boundaries[i].offset == expected[i].offset);
        assert(boundaries[i].length == expected[i].length);
    }
    char many[4000];
    /* Query 500 periods without imposing a hardcoded span count. */
    size_t at = 0;
    for (size_t i = 0; i < 500; ++i) {
        memcpy(many + at, "Pa.", 3); at += 3;
        if (i + 1 < 500) many[at++] = ' ';
    }
    many[at] = 0;
    chain(many, 500);
    struct cancel_count query = {0};
    size_t needed = 0;
    assert(reloq_german_sentence_spans_checked(
               (const unsigned char *)many, at, NULL, 0, &needed,
               cancelled, &query) == 1);
    assert(needed == 500 && query.calls >= 2 + at / 256);
    struct reloq_german_sentence_span marks[500];
    for (size_t i = 0; i < 500; ++i)
        marks[i] = (struct reloq_german_sentence_span){666, 777};
    struct cancel_count during_second = {0, query.calls + 2};
    needed = 9;
    assert(reloq_german_sentence_spans_checked(
               (const unsigned char *)many, at, marks, 500, &needed,
               cancelled, &during_second) == -3);
    assert(needed == 9);
    for (size_t i = 0; i < 500; ++i)
        assert(marks[i].offset == 666 && marks[i].length == 777);
    struct cancel_count on_entry = {0, 1};
    assert(reloq_german_sentence_spans_checked(
               (const unsigned char *)many, at, marks, 500, &needed,
               cancelled, &on_entry) == -3);
    assert(needed == 9);
    struct cancel_count no_cancel = {0};
    assert(reloq_german_sentence_spans_checked(
               (const unsigned char *)many, at, marks, 500, &needed,
               cancelled, &no_cancel) == 1);
    assert(needed == 500);
    assert(marks[499].offset == at - 3 && marks[499].length == 3);

    const char *other[] = {
        "", "Pack.", "Pack Tipp.", "Pack, Tipp.",
        "Pack... Tipp.", "Pack? tipp.", "Pack! tipp.",
        "Pack.  Tipp.", "Pack.\tTipp.", "Pack.Tipp.",
        "Pack. Tipp. ", "Pack? Tipp! tipp.",
        "Pack?! Tipp.", "Pack.... Tipp.", "Pack., Tipp.",
        "`[.1pAk]. `[.1tIp]."
    };
    for (size_t i = 0; i < sizeof other / sizeof other[0]; ++i) {
        size_t unchanged = 123;
        assert(reloq_german_sentence_spans(
                   (const unsigned char *)other[i], strlen(other[i]),
                   NULL, 0, &unchanged) == 0);
        assert(unchanged == 123);
    }
    chain("Dr. Pack.", 2); /* classifier only; the voice rejects Dr. */
    size_t count = 9;
    assert(reloq_german_sentence_spans(NULL, 1, NULL, 0, &count) == -1);
    assert(count == 9);
    assert(reloq_german_sentence_spans((const unsigned char *)"A. B.",
                                       5, NULL, 1, &count) == -1);
    assert(count == 9);
    return 0;
}
