#ifndef RELOQ_CP1252_H
#define RELOQ_CP1252_H

#include <stddef.h>

enum reloq_cp1252_status {
    RELOQ_CP1252_OK = 0,
    RELOQ_CP1252_INVALID = -1,
    RELOQ_CP1252_CAPACITY = -2
};

/* Decode a bounded Windows-1252 text span into UTF-8. This is a byte
 * boundary conversion only; it performs no pronunciation or Unicode
 * normalization. Embedded NUL and the five undefined CP1252 bytes (81, 8d,
 * 8f, 90, 9d) are rejected. An empty span is valid.
 *
 * output == NULL with capacity == 0 queries the UTF-8 byte count, excluding
 * the terminating NUL. A successful write always adds that NUL. The input
 * and output must not overlap. On INVALID, *error_offset identifies the bad
 * input byte when error_offset is supplied. On CAPACITY, *utf8_length gives
 * the required byte count. On any error the output buffer is untouched; on
 * INVALID the caller's length is also untouched. The caller retains all
 * buffers. No locale, system code page, or platform API is consulted. */
int reloq_cp1252_decode(const unsigned char *input, size_t input_length,
                        unsigned char *output, size_t capacity,
                        size_t *utf8_length, size_t *error_offset);

#endif
