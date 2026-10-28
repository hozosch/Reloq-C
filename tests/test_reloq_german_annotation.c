#include "reloq_german_annotation.h"

#include <string.h>

#define CHECK(value) do { if (!(value)) return __LINE__; } while (0)

static int check_reference(const unsigned char *input, size_t length,
                           size_t expected_groups, size_t expected_phones,
                           size_t expected_controls, size_t expected_stresses)
{
    struct reloq_german_annotation_cursor cursor = {0};
    struct reloq_german_annotation_event event = {0};
    size_t source_offset = 0;
    size_t opens = 0, closes = 0, phones = 0, controls = 0, stresses = 0;
    size_t callback_nuls = 0;
    int result;
    while ((result = reloq_german_annotation_next(input, length, &cursor,
                                                   1000, &event)) == 1) {
        CHECK(event.offset == source_offset);
        CHECK(event.length <= length - source_offset);
        if (event.length) {
            /* Every nonimplicit event is one consecutive source span. */
            source_offset += event.length;
        }
        if (event.kind == RELOQ_DE_ANNOTATION_SPR) {
            CHECK(event.offset == event.spr.offset &&
                  event.length == event.spr.length);
            opens += event.spr.kind == RELOQ_DE_SPR_WORD_OPEN;
            closes += event.spr.kind == RELOQ_DE_SPR_WORD_CLOSE;
            phones += event.spr.kind == RELOQ_DE_SPR_PHONE;
            stresses += event.spr.kind == RELOQ_DE_SPR_STRESS;
        } else if (event.kind == RELOQ_DE_ANNOTATION_CONTROL) {
            ++controls;
        } else if (event.kind == RELOQ_DE_ANNOTATION_CALLBACK_NUL) {
            CHECK(input[event.offset] == 0 && event.length == 1);
            ++callback_nuls;
        }
    }
    CHECK(result == 0 && source_offset == length &&
          opens == expected_groups && closes == expected_groups &&
          phones == expected_phones && controls == expected_controls &&
          stresses == expected_stresses && callback_nuls == 1);
    return 0;
}

int main(void)
{
    /* Exact German Apple phoneme callback bytes, including the NUL. */
    const unsigned char hallo[] = " `[.1hA.0lo] `2 `[.1vElt].";
    CHECK(check_reference(hallo, sizeof hallo, 2, 8, 1, 3) == 0);
    const unsigned char susi[] =
        " `[.1zu.0zi] `0 `[.1kOxt] `2 `[.0kA.1fe].";
    CHECK(check_reference(susi, sizeof susi, 3, 12, 2, 5) == 0);

    const unsigned char extra[] = "`[.1'aw's] `p40 ";
    struct reloq_german_annotation_cursor cursor = {0};
    struct reloq_german_annotation_event event = {0};
    int found_diphthong = 0, found_pause = 0;
    while (reloq_german_annotation_next(extra, sizeof extra - 1,
                                         &cursor, 50, &event) == 1) {
        if (event.kind == RELOQ_DE_ANNOTATION_SPR &&
            event.spr.phone == RELOQ_DE_PHONE_Q_aw)
            found_diphthong = 1;
        if (event.kind == RELOQ_DE_ANNOTATION_PAUSE) {
            CHECK(event.number == 40 && event.length == 4);
            found_pause = 1;
        }
    }
    CHECK(found_diphthong && found_pause &&
          cursor.outer.offset == sizeof extra - 1);

    /* An incomplete group must fail before emitting even its opener. */
    const unsigned char incomplete[] = " `[.1hA";
    cursor = (struct reloq_german_annotation_cursor){0};
    CHECK(reloq_german_annotation_next(incomplete, sizeof incomplete - 1,
                                       &cursor, 100, &event) == 1);
    struct reloq_german_annotation_cursor before = cursor;
    struct reloq_german_annotation_event saved = event;
    CHECK(reloq_german_annotation_next(incomplete, sizeof incomplete - 1,
                                       &cursor, 100, &event) == -1 &&
          !memcmp(&cursor, &before, sizeof cursor) &&
          !memcmp(&event, &saved, sizeof event));

    const unsigned char invalid[] = "`[.1a.'bad']";
    cursor = (struct reloq_german_annotation_cursor){0};
    int status;
    do {
        before = cursor;
        saved = event;
        status = reloq_german_annotation_next(invalid, sizeof invalid - 1,
                                               &cursor, 100, &event);
    } while (status == 1);
    CHECK(status == -1 && !memcmp(&cursor, &before, sizeof cursor) &&
          !memcmp(&event, &saved, sizeof event));

    cursor = (struct reloq_german_annotation_cursor){0};
    const unsigned char over_pause[] = "`p101 ";
    CHECK(reloq_german_annotation_next(over_pause, sizeof over_pause - 1,
                                       &cursor, 100, &event) == -1 &&
          cursor.outer.offset == 0);
    CHECK(reloq_german_annotation_next(NULL, 0, &cursor, 100, &event) == -1);
    return 0;
}
