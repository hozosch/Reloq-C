#include "reloq_cp1252.h"
#include "reloq_utf8.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const uint16_t expected_c1[32] = {
    0x20ac, 0,      0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0,      0x017d, 0,
    0,      0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0,      0x017e, 0x0178
};

static void all_bytes(void)
{
    for (unsigned byte = 1; byte <= 255; ++byte) {
        unsigned char input[] = {(unsigned char)byte};
        unsigned char output[8];
        memset(output, 0xa5, sizeof output);
        size_t length = 123, bad = 123;
        int status = reloq_cp1252_decode(input, 1, output, sizeof output,
                                          &length, &bad);
        uint32_t expected = byte;
        if (byte >= 0x80 && byte <= 0x9f)
            expected = expected_c1[byte - 0x80];
        if (!expected) {
            assert(status == RELOQ_CP1252_INVALID);
            assert(bad == 0 && length == 123);
            for (size_t at = 0; at < sizeof output; ++at)
                assert(output[at] == 0xa5);
            continue;
        }
        assert(status == RELOQ_CP1252_OK);
        assert(bad == 123);
        assert(length >= 1 && length <= 3);
        assert(output[length] == 0);
        assert(output[length + 1] == 0xa5);
        size_t offset = 0;
        uint32_t scalar = 0;
        assert(reloq_utf8_next(output, length, &offset, &scalar) == 1);
        assert(offset == length && scalar == expected);
    }
}

static void spans_and_errors(void)
{
    const unsigned char text[] = {'G','r',0xfc,0xdf,'e', ' ',0x80};
    const unsigned char expected[] = {'G','r',0xc3,0xbc,0xc3,0x9f,'e',
                                      ' ',0xe2,0x82,0xac,0};
    unsigned char result[24];
    memset(result, 0xa5, sizeof result);
    size_t length = 999, bad = 999;
    assert(reloq_cp1252_decode(text, sizeof text, NULL, 0,
                                &length, &bad) == RELOQ_CP1252_OK);
    assert(length == sizeof expected - 1 && bad == 999);
    assert(reloq_cp1252_decode(text, sizeof text, result, length,
                                &length, &bad) == RELOQ_CP1252_CAPACITY);
    assert(length == sizeof expected - 1 && result[0] == 0xa5);
    assert(reloq_cp1252_decode(text, sizeof text, result, sizeof result,
                                &length, &bad) == RELOQ_CP1252_OK);
    assert(length == sizeof expected - 1);
    assert(memcmp(result, expected, sizeof expected) == 0);
    assert(result[sizeof expected] == 0xa5);

    const unsigned char with_nul[] = {'x', 0, 'z'};
    memset(result, 0xa5, sizeof result);
    length = 777;
    assert(reloq_cp1252_decode(with_nul, sizeof with_nul, result,
                                sizeof result, &length, &bad) ==
           RELOQ_CP1252_INVALID);
    assert(bad == 1 && length == 777 && result[0] == 0xa5);
    assert(reloq_cp1252_decode(with_nul, sizeof with_nul, NULL, 0,
                                &length, &bad) == RELOQ_CP1252_INVALID);

    const unsigned char undefined[] = {'x', 0x9d, 'y'};
    assert(reloq_cp1252_decode(undefined, sizeof undefined, result,
                                sizeof result, &length, &bad) ==
           RELOQ_CP1252_INVALID);
    assert(bad == 1 && length == 777 && result[0] == 0xa5);

    const unsigned char empty[] = {0};
    assert(reloq_cp1252_decode(empty, 0, result, sizeof result,
                                &length, &bad) == RELOQ_CP1252_OK);
    assert(length == 0 && result[0] == 0);
    assert(reloq_cp1252_decode(empty, 0, NULL, 0,
                                &length, &bad) == RELOQ_CP1252_OK);
    assert(length == 0);

    assert(reloq_cp1252_decode(NULL, 0, result, sizeof result,
                                &length, &bad) == RELOQ_CP1252_INVALID);
    assert(reloq_cp1252_decode(empty, 0, NULL, 1,
                                &length, &bad) == RELOQ_CP1252_INVALID);
    assert(reloq_cp1252_decode(empty, 0, result, sizeof result,
                                NULL, &bad) == RELOQ_CP1252_INVALID);
}

int main(void)
{
    all_bytes();
    spans_and_errors();
    puts("CP1252-to-UTF-8 boundary: 250 defined bytes and checked errors OK");
    return 0;
}
