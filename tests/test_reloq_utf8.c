#include "reloq_utf8.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static void valid(const unsigned char *s, size_t n,
                  const uint32_t *expected, size_t count)
{
    size_t at = 0;
    uint32_t value = 0;
    for (size_t i = 0; i < count; ++i) {
        assert(reloq_utf8_next(s, n, &at, &value) == 1);
        assert(value == expected[i]);
    }
    assert(at == n);
    assert(reloq_utf8_next(s, n, &at, &value) == 0);
}

static void invalid(const unsigned char *s, size_t n)
{
    size_t at = 0;
    uint32_t value = 123;
    assert(reloq_utf8_next(s, n, &at, &value) == -1);
    assert(at == 0);
    assert(value == 123);
}

int main(void)
{
    static const unsigned char german[] = {
        'F', 'u', 0xc3, 0x9f, ' ', 0xc3, 0xa4, 0xc3, 0xb6, 0xc3, 0xbc
    };
    static const uint32_t german_expected[] = {
        'F', 'u', 0xdf, ' ', 0xe4, 0xf6, 0xfc
    };
    valid(german, sizeof german, german_expected,
          sizeof german_expected / sizeof german_expected[0]);
    static const unsigned char other[] = {
        0xce, 0xb1, 0xe6, 0x97, 0xa5, 0xed, 0x95, 0x9c,
        0xf0, 0x9f, 0x98, 0x80, 0x00
    };
    static const uint32_t other_expected[] = {
        0x3b1, 0x65e5, 0xd55c, 0x1f600, 0
    };
    valid(other, sizeof other, other_expected,
          sizeof other_expected / sizeof other_expected[0]);
    static const unsigned char malformed[][4] = {
        { 0x80, 0, 0, 0 }, { 0xc0, 0x80, 0, 0 },
        { 0xc1, 0xbf, 0, 0 }, { 0xe0, 0x80, 0x80, 0 },
        { 0xed, 0xa0, 0x80, 0 }, { 0xf0, 0x80, 0x80, 0x80 },
        { 0xf4, 0x90, 0x80, 0x80 }, { 0xf5, 0x80, 0x80, 0x80 },
        { 0xc3, 'X', 0, 0 }, { 0xe2, 0x82, 0, 0 }
    };
    static const size_t malformed_lengths[] = { 1, 2, 2, 3, 3, 4, 4, 4, 2, 2 };
    for (size_t i = 0; i < sizeof malformed / sizeof malformed[0]; ++i)
        invalid(malformed[i], malformed_lengths[i]);
    static const unsigned char incomplete[] = { 0xe2, 0x82 };
    invalid(incomplete, sizeof incomplete);
    size_t at = 1;
    uint32_t scalar = 99;
    assert(reloq_utf8_next(incomplete, sizeof incomplete, &at, &scalar) == -1);
    assert(reloq_utf8_next(NULL, 0, &at, &scalar) == -1);
    assert(reloq_utf8_next(incomplete, 1, NULL, &scalar) == -1);
    assert(reloq_utf8_next(incomplete, 1, &at, NULL) == -1);
    puts("UTF-8 scalar decoder passed");
    return 0;
}
