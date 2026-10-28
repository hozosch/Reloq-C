#include "reloq_german_input.h"

#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

static int check_bytes(const unsigned char *input, size_t input_length,
                       const unsigned char *expected, size_t expected_length)
{
    unsigned char output[128];
    size_t length = 999;
    CHECK(reloq_german_input_encode(input, input_length, NULL, 0,
                                    &length, NULL) == 0);
    CHECK(length == expected_length);
    memset(output, 0xa5, sizeof output);
    CHECK(reloq_german_input_encode(input, input_length, output,
                                    sizeof output, &length, NULL) == 0);
    CHECK(length == expected_length);
    CHECK(memcmp(output, expected, length) == 0 && output[length] == 0);
    CHECK(output[length + 1] == 0xa5);
    return 0;
}

struct interrupt {
    unsigned calls, after;
};

static int cancelled(void *context)
{
    struct interrupt *state = context;
    return ++state->calls >= state->after;
}

static int checked_cancellation(void)
{
    const unsigned char text[] = "B\xc3\xa4r, B\xc3\xa4r, B\xc3\xa4r";
    unsigned observed = 0;
    for (unsigned after = 1; after < 20; ++after) {
        struct interrupt state = {0, after};
        unsigned char output[64];
        memset(output, 0xa5, sizeof output);
        size_t length = 999, bad = 888;
        int result = reloq_german_input_encode_checked(
            text, sizeof text - 1, output, sizeof output,
            &length, &bad, cancelled, &state);
        if (result == 0) {
            CHECK(after > 2 && length == sizeof "B\xe4r, B\xe4r, B\xe4r" - 1);
            break;
        }
        CHECK(result == 1 && length == 999 && bad == 888);
        for (size_t i = 0; i < sizeof output; ++i)
            CHECK(output[i] == 0xa5);
        ++observed;
    }
    CHECK(observed > 2);
    return 0;
}

int main(void)
{
    /* German letters remain one byte each at the observed ECI boundary. */
    const unsigned char utf8[] =
        "B\xc3\xa4r, \xc3\x96l, \xc3\x9c" "bung und Fu\xc3\x9f.";
    const unsigned char cp1252[] =
        "B\xe4r, \xd6l, \xdc" "bung und Fu\xdf.";
    CHECK(check_bytes(utf8, sizeof utf8 - 1,
                      cp1252, sizeof cp1252 - 1) == 0);

    /* Canonically decomposed German umlauts have the same byte spelling. */
    const unsigned char decomposed[] =
        "A\xcc\x88 a\xcc\x88 O\xcc\x88 o\xcc\x88 U\xcc\x88 u\xcc\x88";
    const unsigned char combined[] = "\xc4 \xe4 \xd6 \xf6 \xdc \xfc";
    CHECK(check_bytes(decomposed, sizeof decomposed - 1,
                      combined, sizeof combined - 1) == 0);

    /* Windows-1252 punctuation and ligatures have direct byte mappings. */
    const unsigned char rich[] =
        "\xe2\x82\xac \xe2\x80\x99 \xe2\x80\x93 \xc5\x92 \xc5\xb8";
    const unsigned char rich_bytes[] = "\x80 \x92 \x96 \x8c \x9f";
    CHECK(check_bytes(rich, sizeof rich - 1,
                      rich_bytes, sizeof rich_bytes - 1) == 0);

    unsigned char inplace[sizeof utf8];
    memcpy(inplace, utf8, sizeof utf8);
    size_t length = 999;
    CHECK(reloq_german_input_encode(inplace, sizeof utf8 - 1,
                                    inplace, sizeof inplace,
                                    &length, NULL) == 0);
    CHECK(length == sizeof cp1252 - 1 &&
          memcmp(inplace, cp1252, sizeof cp1252) == 0);

    unsigned char short_buffer[sizeof cp1252];
    memset(short_buffer, 0x5a, sizeof short_buffer);
    length = 999;
    CHECK(reloq_german_input_encode(utf8, sizeof utf8 - 1, short_buffer,
                                    sizeof cp1252 - 1, &length, NULL) == -2);
    CHECK(length == sizeof cp1252 - 1);
    for (size_t i = 0; i < sizeof short_buffer; ++i)
        CHECK(short_buffer[i] == 0x5a);

    /* Validation is transactional even when an invalid scalar follows text. */
    const unsigned char malformed[] = {'H', 'i', 0xc0, 0xaf};
    const unsigned char unmappable[] = "Hi\xf0\x9f\x98\x80";
    const unsigned char embedded_nul[] = {'H', 0, 'i'};
    const unsigned char stray_mark[] = {'a', 0xcc, 0x88, 0xcc, 0x88};
    const struct {
        const unsigned char *input;
        size_t length;
        size_t bad_offset;
    } rejects[] = {
        {malformed, sizeof malformed, 2},
        {unmappable, sizeof unmappable - 1, 2},
        {embedded_nul, sizeof embedded_nul, 1},
        {stray_mark, sizeof stray_mark, 3}
    };
    for (size_t i = 0; i < sizeof rejects / sizeof rejects[0]; ++i) {
        unsigned char output[] = "unchanged";
        size_t bad_offset = 999;
        length = 999;
        CHECK(reloq_german_input_encode(rejects[i].input,
                                        rejects[i].length, output,
                                        sizeof output, &length,
                                        &bad_offset) == -1);
        CHECK(length == 999 && bad_offset == rejects[i].bad_offset);
        CHECK(memcmp(output, "unchanged", sizeof output) == 0);
    }
    return checked_cancellation();
}
