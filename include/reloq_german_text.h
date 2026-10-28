#ifndef RELOQ_GERMAN_TEXT_H
#define RELOQ_GERMAN_TEXT_H

#include <stddef.h>
#include "reloq_german_input.h"

struct reloq_lexicon;

enum reloq_german_text_status {
    RELOQ_DE_TEXT_OK = 0,
    RELOQ_DE_TEXT_CANCELLED = 1,
    RELOQ_DE_TEXT_INVALID = -1,
    RELOQ_DE_TEXT_UNCOVERED = -2,
    RELOQ_DE_TEXT_CAPACITY = -3,
    RELOQ_DE_TEXT_NO_MEMORY = -4
};

/* Convert bounded UTF-8 free text to a NUL-terminated German SPR annotation.
 * Words consist of ASCII letters, German umlauts, and sharp s. Each complete
 * word must be covered by reloq_german_g2p_word; otherwise the whole call
 * fails with UNCOVERED. The only nonword input accepted is ASCII whitespace
 * (space, tab, CR, LF), the punctuation . , ! ? ; : - ( ) ' ", and CP1252
 * typographic quotes, dashes, and ellipsis. These bytes are preserved in
 * scanner TEXT spans. Digits, ECI markup, arbitrary symbols and other
 * letters are rejected. No number expansion or prosody is inferred.
 *
 * The output can be passed to reloq_german_phone_sequence_build. Its phone
 * source offsets refer to this annotation, so retain the annotation while
 * using the nodes. A NULL output with zero capacity queries the required
 * byte count. Otherwise capacity includes the terminating NUL. Input and
 * output may overlap. Input must stay immutable during the call.
 *
 * On success, *byte_length is the byte count excluding NUL. CAPACITY leaves
 * output unchanged and sets *byte_length to the required count. INVALID and
 * UNCOVERED leave output and *byte_length unchanged; if error_offset is
 * provided, it receives the offending UTF-8 byte offset (the first byte of
 * an uncovered word). Invalid arguments and NO_MEMORY leave error_offset
 * unchanged. Success and CAPACITY leave error_offset unchanged.
 */
int reloq_german_text_annotate(const unsigned char *input,
                               size_t input_length,
                               unsigned char *output, size_t capacity,
                               size_t *byte_length, size_t *error_offset);

/* The same transactional text conversion with an optional caller-owned
 * lexicon bound by reloq_lexicon_bind. For each CP1252 word, a validated
 * explicit SPR override takes precedence over the restricted G2P rules;
 * a missing override falls back to those rules. Opaque action references
 * never trigger pronunciation. The original entry point is this function
 * with a NULL lexicon. Keep the view and all its records immutable for the
 * duration of the call, including both validation and output passes. */
int reloq_german_text_annotate_with_lexicon(
    const unsigned char *input, size_t input_length,
    unsigned char *output, size_t capacity,
    size_t *byte_length, size_t *error_offset,
    const struct reloq_lexicon *lexicon);

/* Add cooperative cancellation to the same transactional conversion.
 * The predicate is polled during UTF-8 scanning, between words and while
 * validating the annotation. On cancellation return TEXT_CANCELLED, leave
 * output, byte_length and error_offset unchanged, and retain no callback.
 * A single lexicon override or G2P word is processed atomically; the
 * fallback G2P word grammar rejects spellings of 128 bytes or longer. */
int reloq_german_text_annotate_checked(
    const unsigned char *input, size_t input_length,
    unsigned char *output, size_t capacity,
    size_t *byte_length, size_t *error_offset,
    const struct reloq_lexicon *lexicon,
    reloq_german_cancel_check cancelled, void *cancel_context);

#endif
