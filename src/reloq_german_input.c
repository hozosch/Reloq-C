#include "reloq_german_input.h"

#include "reloq_utf8.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* The discontinuous part of the published Windows-1252 character mapping.
 * Undefined bytes 81, 8d, 8f, 90, and 9d have no Unicode inverse. */
static int cp1252_byte(uint32_t scalar, unsigned char *byte)
{
    if (scalar > 0 && scalar < 0x80) {
        *byte = (unsigned char)scalar;
        return 0;
    }
    if (scalar >= 0xa0 && scalar <= 0xff) {
        *byte = (unsigned char)scalar;
        return 0;
    }
    switch (scalar) {
    case 0x20ac: *byte = 0x80; break;
    case 0x201a: *byte = 0x82; break;
    case 0x0192: *byte = 0x83; break;
    case 0x201e: *byte = 0x84; break;
    case 0x2026: *byte = 0x85; break;
    case 0x2020: *byte = 0x86; break;
    case 0x2021: *byte = 0x87; break;
    case 0x02c6: *byte = 0x88; break;
    case 0x2030: *byte = 0x89; break;
    case 0x0160: *byte = 0x8a; break;
    case 0x2039: *byte = 0x8b; break;
    case 0x0152: *byte = 0x8c; break;
    case 0x017d: *byte = 0x8e; break;
    case 0x2018: *byte = 0x91; break;
    case 0x2019: *byte = 0x92; break;
    case 0x201c: *byte = 0x93; break;
    case 0x201d: *byte = 0x94; break;
    case 0x2022: *byte = 0x95; break;
    case 0x2013: *byte = 0x96; break;
    case 0x2014: *byte = 0x97; break;
    case 0x02dc: *byte = 0x98; break;
    case 0x2122: *byte = 0x99; break;
    case 0x0161: *byte = 0x9a; break;
    case 0x203a: *byte = 0x9b; break;
    case 0x0153: *byte = 0x9c; break;
    case 0x017e: *byte = 0x9e; break;
    case 0x0178: *byte = 0x9f; break;
    default: return -1;
    }
    return 0;
}

static uint32_t umlaut(uint32_t scalar)
{
    switch (scalar) {
    case 'A': return 0xc4;
    case 'O': return 0xd6;
    case 'U': return 0xdc;
    case 'a': return 0xe4;
    case 'o': return 0xf6;
    case 'u': return 0xfc;
    default: return 0;
    }
}

static int scan(const unsigned char *input, size_t length,
                unsigned char *output, size_t *count, size_t *error_offset,
                reloq_german_cancel_check cancelled, void *cancel_context)
{
    size_t offset = 0, used = 0;
    while (offset < length) {
        if (cancelled && (used & 255) == 0 && cancelled(cancel_context))
            return 1;
        size_t start = offset;
        uint32_t scalar;
        if (reloq_utf8_next(input, length, &offset, &scalar) != 1) {
            if (error_offset) *error_offset = start;
            return -1;
        }
        uint32_t combined = umlaut(scalar);
        if (combined && offset < length) {
            size_t next = offset;
            uint32_t mark;
            /* Decode the lookahead only to recognize a combining diaeresis.
             * An invalid following sequence is diagnosed on the next turn. */
            if (reloq_utf8_next(input, length, &next, &mark) == 1 &&
                mark == 0x0308) {
                scalar = combined;
                offset = next;
            }
        }
        unsigned char byte;
        if (cp1252_byte(scalar, &byte)) {
            if (error_offset) *error_offset = start;
            return -1;
        }
        if (output) output[used] = byte;
        ++used;
    }
    if (cancelled && cancelled(cancel_context)) return 1;
    *count = used;
    return 0;
}

int reloq_german_input_encode_checked(
    const unsigned char *input, size_t input_length,
    unsigned char *output, size_t capacity, size_t *byte_length,
    size_t *error_offset, reloq_german_cancel_check cancelled,
    void *cancel_context)
{
    if (!input || !byte_length || (!output && capacity)) return -1;
    size_t needed = 0;
    size_t bad = 0;
    int status = scan(input, input_length, NULL, &needed, &bad,
                      cancelled, cancel_context);
    if (status == 1) return 1;
    if (status != 0) {
        if (error_offset) *error_offset = bad;
        return -1;
    }
    if (!output) {
        *byte_length = needed;
        return 0;
    }
    if (capacity <= needed) {
        *byte_length = needed;
        return -2;
    }
    size_t written = 0;
    /* A checked second pass can be interrupted, so commit it only after
     * completion. The original NULL-predicate path retains its in-place
     * conversion and allocation behavior. */
    unsigned char *temporary = NULL;
    unsigned char *target = output;
    if (cancelled) {
        if (needed == SIZE_MAX) return -1;
        temporary = malloc(needed + 1);
        if (!temporary) return -3;
        target = temporary;
    }
    status = scan(input, input_length, target, &written, NULL,
                  cancelled, cancel_context);
    if (status != 0) {
        free(temporary);
        return status;
    }
    target[written] = 0;
    if (temporary) {
        memmove(output, temporary, written + 1);
        free(temporary);
    }
    *byte_length = written;
    return 0;
}

int reloq_german_input_encode(const unsigned char *input, size_t input_length,
                              unsigned char *output, size_t capacity,
                              size_t *byte_length, size_t *error_offset)
{
    return reloq_german_input_encode_checked(
        input, input_length, output, capacity, byte_length, error_offset,
        NULL, NULL);
}
