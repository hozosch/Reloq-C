#include "reloq_cp1252.h"

#include <stdint.h>
#include <string.h>

/* The discontinuous 0x80..0x9f range from the Unicode Consortium's
 * CP1252 mapping. Zero entries represent undefined bytes. */
static const uint16_t cp1252_c1[32] = {
    0x20ac, 0,      0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0,      0x017d, 0,
    0,      0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0,      0x017e, 0x0178
};

static uint32_t scalar_for_byte(unsigned char byte)
{
    if (byte >= 0x80 && byte <= 0x9f)
        return cp1252_c1[byte - 0x80];
    return byte;
}

static size_t utf8_width(uint32_t scalar)
{
    if (scalar < 0x80) return 1;
    if (scalar < 0x800) return 2;
    return 3;
}

static size_t write_utf8(unsigned char *output, uint32_t scalar)
{
    if (scalar < 0x80) {
        output[0] = (unsigned char)scalar;
        return 1;
    }
    if (scalar < 0x800) {
        output[0] = (unsigned char)(0xc0u | (scalar >> 6));
        output[1] = (unsigned char)(0x80u | (scalar & 0x3fu));
        return 2;
    }
    output[0] = (unsigned char)(0xe0u | (scalar >> 12));
    output[1] = (unsigned char)(0x80u | ((scalar >> 6) & 0x3fu));
    output[2] = (unsigned char)(0x80u | (scalar & 0x3fu));
    return 3;
}

int reloq_cp1252_decode(const unsigned char *input, size_t input_length,
                        unsigned char *output, size_t capacity,
                        size_t *utf8_length, size_t *error_offset)
{
    if (!input || !utf8_length || (!output && capacity))
        return RELOQ_CP1252_INVALID;

    size_t required = 0;
    for (size_t at = 0; at < input_length; ++at) {
        uint32_t scalar = scalar_for_byte(input[at]);
        if (scalar == 0) {
            if (error_offset) *error_offset = at;
            return RELOQ_CP1252_INVALID;
        }
        size_t width = utf8_width(scalar);
        /* One more byte is needed for the trailing NUL when writing. */
        if (width > SIZE_MAX - 1 - required)
            return RELOQ_CP1252_INVALID;
        required += width;
    }
    if (!output) {
        *utf8_length = required;
        return RELOQ_CP1252_OK;
    }
    if (capacity <= required) {
        *utf8_length = required;
        return RELOQ_CP1252_CAPACITY;
    }
    size_t used = 0;
    for (size_t at = 0; at < input_length; ++at)
        used += write_utf8(output + used, scalar_for_byte(input[at]));
    output[used] = 0;
    *utf8_length = used;
    return RELOQ_CP1252_OK;
}
