#include "reloq_phonetic.h"

#include <stdint.h>

static int ascii_digit(unsigned char c)
{
    return c >= '0' && c <= '9';
}

static int ascii_space(unsigned char c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

static int pause_start(const unsigned char *input, size_t length, size_t at)
{
    return length - at >= 3 && input[at] == '`' &&
           input[at + 1] == 'p' && ascii_digit(input[at + 2]);
}

static int control_start(const unsigned char *input, size_t length, size_t at)
{
    return length - at >= 2 && input[at] == '`' &&
           ascii_digit(input[at + 1]);
}

static int special_start(const unsigned char *input, size_t length, size_t at)
{
    return length - at >= 2 && input[at] == '`' &&
           (input[at + 1] == '[' || ascii_digit(input[at + 1]) ||
            pause_start(input, length, at));
}

static int decimal_number(const unsigned char *input, size_t length,
                          size_t at, uint32_t maximum,
                          size_t *end, uint32_t *number)
{
    uint32_t result = 0;
    size_t pos = at;
    if (pos >= length || !ascii_digit(input[pos]))
        return -1;
    do {
        uint32_t digit = (uint32_t)(input[pos] - '0');
        if (digit > maximum || result > (maximum - digit) / 10u)
            return -1;
        result = result * 10u + digit;
        ++pos;
    } while (pos < length && ascii_digit(input[pos]));
    *end = pos;
    *number = result;
    return 0;
}

int reloq_phonetic_next(const unsigned char *input, size_t input_length,
                        struct reloq_phonetic_cursor *cursor,
                        uint32_t max_pause_ms,
                        struct reloq_phonetic_token *token)
{
    if (!input || !cursor || !token || cursor->offset > input_length ||
        (cursor->in_group != 0 && cursor->in_group != 1))
        return -1;
    const size_t start = cursor->offset;
    if (start == input_length)
        return cursor->in_group ? -1 : 0;

    struct reloq_phonetic_token result = {0};
    result.offset = start;
    if (input[start] == 0) {
        if (cursor->in_group)
            return -1;
        result.kind = RELOQ_PHONETIC_CALLBACK_NUL;
        result.length = 1;
    } else if (cursor->in_group) {
        switch (input[start]) {
        case ']':
            result.kind = RELOQ_PHONETIC_GROUP_CLOSE;
            result.length = 1;
            break;
        case '.':
            result.kind = RELOQ_PHONETIC_SEGMENT_DOT;
            result.length = 1;
            break;
        case '\'':
            result.kind = RELOQ_PHONETIC_PHONE_QUOTE;
            result.length = 1;
            break;
        case ':':
            result.kind = RELOQ_PHONETIC_LENGTH_COLON;
            result.length = 1;
            break;
        default: {
            if (ascii_digit(input[start])) {
                size_t at = start;
                while (at < input_length && ascii_digit(input[at]))
                    ++at;
                result.kind = RELOQ_PHONETIC_MODIFIER_DIGITS;
                result.length = at - start;
            } else {
                size_t at = start;
                while (at < input_length && input[at] != 0) {
                    if (input[at] == '\\' && at + 1 < input_length &&
                        input[at + 1] != 0) {
                        at += 2;
                        continue;
                    }
                    if (input[at] == ']' || input[at] == '.' ||
                        input[at] == '\'' || input[at] == ':' ||
                        ascii_digit(input[at]))
                        break;
                    ++at;
                }
                result.kind = RELOQ_PHONETIC_PHONE_RUN;
                result.length = at - start;
            }
            break;
        }
        }
    } else if (special_start(input, input_length, start) &&
               input[start + 1] == '[') {
        result.kind = RELOQ_PHONETIC_GROUP_OPEN;
        result.length = 2;
    } else if (pause_start(input, input_length, start)) {
        size_t at;
        if (decimal_number(input, input_length, start + 2,
                           max_pause_ms, &at, &result.number) != 0 ||
            (at < input_length && input[at] != 0 &&
             !ascii_space(input[at])))
            return -1;
        result.kind = RELOQ_PHONETIC_EXPLICIT_PAUSE;
        result.length = at - start;
    } else if (control_start(input, input_length, start)) {
        size_t at;
        if (decimal_number(input, input_length, start + 1,
                           UINT32_MAX, &at, &result.number) != 0)
            return -1;
        result.kind = RELOQ_PHONETIC_CONTROL_DIGITS;
        result.length = at - start;
    } else {
        size_t at = start;
        while (at < input_length && input[at] != 0) {
            if (input[at] == '\\' && at + 1 < input_length &&
                input[at + 1] != 0) {
                at += 2;
                continue;
            }
            if (special_start(input, input_length, at))
                break;
            ++at;
        }
        result.kind = RELOQ_PHONETIC_TEXT;
        result.length = at - start;
    }

    if (!result.length)
        return -1;
    *token = result;
    cursor->offset = start + result.length;
    if (result.kind == RELOQ_PHONETIC_GROUP_OPEN)
        cursor->in_group = 1;
    else if (result.kind == RELOQ_PHONETIC_GROUP_CLOSE)
        cursor->in_group = 0;
    return 1;
}
