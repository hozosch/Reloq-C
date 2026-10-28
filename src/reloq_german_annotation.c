#include "reloq_german_annotation.h"

int reloq_german_annotation_next(
    const unsigned char *input, size_t input_length,
    struct reloq_german_annotation_cursor *cursor, uint32_t max_pause_ms,
    struct reloq_german_annotation_event *event)
{
    if (!input || !cursor || !event || cursor->in_group < 0 ||
        cursor->in_group > 1 ||
        (cursor->in_group &&
         (cursor->group_offset > input_length ||
          cursor->group_length > input_length - cursor->group_offset)))
        return -1;
    struct reloq_german_annotation_cursor next = *cursor;
    struct reloq_german_annotation_event result = {0};
    if (next.in_group) {
        int state = reloq_german_spr_next(input + next.group_offset,
                                          next.group_length, &next.spr,
                                          &result.spr);
        if (state < 0) return -1;
        if (state > 0) {
            result.kind = RELOQ_DE_ANNOTATION_SPR;
            result.offset = next.group_offset + result.spr.offset;
            result.length = result.spr.length;
            result.spr.offset = result.offset;
            *cursor = next;
            *event = result;
            return 1;
        }
        next.in_group = 0;
    }

    struct reloq_phonetic_token token;
    int state = reloq_phonetic_next(input, input_length, &next.outer,
                                    max_pause_ms, &token);
    if (state <= 0) {
        if (state == 0) *cursor = next;
        return state;
    }
    if (token.kind == RELOQ_PHONETIC_GROUP_OPEN) {
        /* The outer scanner validates the complete group boundary before
         * emitting any German event. Its cursor is copied so failure leaves
         * the caller's state intact. */
        struct reloq_phonetic_cursor probe = next.outer;
        struct reloq_phonetic_token part;
        do {
            state = reloq_phonetic_next(input, input_length, &probe,
                                        max_pause_ms, &part);
            if (state != 1) return -1;
        } while (part.kind != RELOQ_PHONETIC_GROUP_CLOSE);
        next.group_offset = token.offset;
        next.group_length = probe.offset - token.offset;
        next.outer = probe;
        next.spr = (struct reloq_german_spr_cursor){0};
        next.in_group = 1;
        state = reloq_german_spr_next(input + next.group_offset,
                                      next.group_length, &next.spr,
                                      &result.spr);
        if (state != 1) return -1;
        result.kind = RELOQ_DE_ANNOTATION_SPR;
        result.offset = next.group_offset + result.spr.offset;
        result.length = result.spr.length;
        result.spr.offset = result.offset;
    } else {
        switch (token.kind) {
        case RELOQ_PHONETIC_TEXT:
            result.kind = RELOQ_DE_ANNOTATION_TEXT;
            break;
        case RELOQ_PHONETIC_CONTROL_DIGITS:
            result.kind = RELOQ_DE_ANNOTATION_CONTROL;
            break;
        case RELOQ_PHONETIC_EXPLICIT_PAUSE:
            result.kind = RELOQ_DE_ANNOTATION_PAUSE;
            break;
        case RELOQ_PHONETIC_CALLBACK_NUL:
            result.kind = RELOQ_DE_ANNOTATION_CALLBACK_NUL;
            break;
        default:
            return -1;
        }
        result.offset = token.offset;
        result.length = token.length;
        result.number = token.number;
    }
    *cursor = next;
    *event = result;
    return 1;
}
