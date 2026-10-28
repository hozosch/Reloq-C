#ifndef RELOQ_GERMAN_INPUT_H
#define RELOQ_GERMAN_INPUT_H

#include <stddef.h>

typedef int (*reloq_german_cancel_check)(void *context);

/* Encode UTF-8 text as the Windows-1252 bytes accepted by the observed
 * German ECI text input. ASCII and representable Windows-1252 characters
 * retain their spelling; A/a, O/o and U/u followed by U+0308 combine into
 * the corresponding German umlaut. No pronunciation, case folding, word
 * segmentation, or transliteration is performed.
 *
 * The resulting buffer is NUL terminated for the byte-oriented text path;
 * an embedded U+0000, malformed UTF-8, and characters without a direct
 * mapping are rejected. A NULL output with zero capacity queries the byte
 * length (excluding the NUL). The input and output may be the same buffer,
 * but must otherwise not overlap. Return 0 on success, -1 on invalid input
 * (with *error_offset set to the offending UTF-8 byte when provided), or -2
 * when capacity is too small (with *byte_length set to the required length).
 * On error the destination is unchanged. No ECI library is needed.
 */
int reloq_german_input_encode(const unsigned char *input, size_t input_length,
                              unsigned char *output, size_t capacity,
                              size_t *byte_length, size_t *error_offset);

/* Same conversion with a caller-owned cancellation predicate, checked while
 * scanning UTF-8. Return 1 when cancelled, leaving output, byte_length and
 * error_offset unchanged. Return -3 if scratch allocation fails. No
 * callback is retained after return. A NULL predicate gives the original
 * conversion behavior. */
int reloq_german_input_encode_checked(
    const unsigned char *input, size_t input_length,
    unsigned char *output, size_t capacity, size_t *byte_length,
    size_t *error_offset, reloq_german_cancel_check cancelled,
    void *cancel_context);

#endif
