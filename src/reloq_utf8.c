#include "reloq_utf8.h"

int reloq_utf8_next(const unsigned char *input, size_t length,
                    size_t *offset, uint32_t *scalar)
{
    if (!input || !offset || !scalar || *offset > length)
        return -1;
    size_t at = *offset;
    if (at == length)
        return 0;
    unsigned char first = input[at];
    if (first < 0x80) {
        *scalar = first;
        *offset = at + 1;
        return 1;
    }
    size_t width;
    uint32_t value, minimum;
    if (first >= 0xc2 && first <= 0xdf) {
        width = 2;
        value = first & 0x1fu;
        minimum = 0x80u;
    } else if (first >= 0xe0 && first <= 0xef) {
        width = 3;
        value = first & 0x0fu;
        minimum = 0x800u;
    } else if (first >= 0xf0 && first <= 0xf4) {
        width = 4;
        value = first & 0x07u;
        minimum = 0x10000u;
    } else {
        return -1;
    }
    if (width > length - at)
        return -1;
    for (size_t i = 1; i < width; ++i) {
        unsigned char byte = input[at + i];
        if ((byte & 0xc0u) != 0x80u)
            return -1;
        value = (value << 6) | (uint32_t)(byte & 0x3fu);
    }
    if (value < minimum || (value >= 0xd800u && value <= 0xdfffu) ||
        value > 0x10ffffu)
        return -1;
    *scalar = value;
    *offset = at + width;
    return 1;
}
