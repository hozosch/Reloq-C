#ifndef RELOQ_UTF8_H
#define RELOQ_UTF8_H

#include <stddef.h>
#include <stdint.h>

/* Decode one Unicode scalar from a bounded UTF-8 span. The caller retains
 * ownership of the input. On success, *offset advances to the next scalar
 * and *scalar receives its value. End of input returns 0. An incomplete or
 * malformed sequence returns -1 without changing *offset. No locale or
 * platform conversion is involved. */
int reloq_utf8_next(const unsigned char *input, size_t length,
                    size_t *offset, uint32_t *scalar);

#endif
