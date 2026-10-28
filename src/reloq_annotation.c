#include "reloq_annotation.h"

static int ascii_space(unsigned char c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

static int ascii_digit(unsigned char c)
{
    return c >= '0' && c <= '9';
}

static int pause_start(const unsigned char *input, size_t length, size_t at)
{
    return length - at >= 3 && input[at] == '`' &&
           input[at + 1] == 'p' && ascii_digit(input[at + 2]);
}

int reloq_annotation_next(const unsigned char *input, size_t input_length,
                          struct reloq_annotation_cursor *cursor,
                          uint32_t max_pause_ms,
                          struct reloq_annotation_event *event)
{
    if (!input || !cursor || !event || cursor->offset > input_length ||
        (cursor->in_pronunciation != 0 && cursor->in_pronunciation != 1))
        return -1;
    size_t start = cursor->offset;
    if (start == input_length)
        return 0;

    if (!cursor->in_pronunciation && pause_start(input, input_length, start)) {
        uint32_t value = 0;
        size_t at = start + 2;
        while (at < input_length && ascii_digit(input[at])) {
            unsigned digit = (unsigned)(input[at] - '0');
            if (digit > max_pause_ms ||
                value > (max_pause_ms - digit) / 10u)
                return -1;
            value = value * 10u + digit;
            ++at;
        }
        if (at < input_length && !ascii_space(input[at]))
            return -1;
        const struct reloq_annotation_event result = {
            RELOQ_ANNOTATION_EXPLICIT_PAUSE, start, at - start, value
        };
        *event = result;
        cursor->offset = at;
        return 1;
    }

    size_t at = start;
    int in_pronunciation = cursor->in_pronunciation;
    while (at < input_length) {
        if (input[at] == '\\' && at + 1 < input_length) {
            at += 2;
            continue;
        }
        if (!in_pronunciation && pause_start(input, input_length, at))
            break;
        if (input[at] == '[')
            in_pronunciation = 1;
        else if (input[at] == ']')
            in_pronunciation = 0;
        ++at;
    }
    const struct reloq_annotation_event result = {
        RELOQ_ANNOTATION_TEXT, start, at - start, 0
    };
    *event = result;
    cursor->offset = at;
    cursor->in_pronunciation = in_pronunciation;
    return 1;
}
