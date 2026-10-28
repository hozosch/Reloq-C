#ifndef RELOQ_PHONETIC_H
#define RELOQ_PHONETIC_H

#include <stddef.h>
#include <stdint.h>

/* Lossless lexical view of the text returned by eciGeneratePhonemes.
 *
 * The output is an annotation, not a sequence of Klatt frames. Within a
 * pronunciation group, period, decimal digits, apostrophe, and colon are
 * exposed separately. A PHONE_RUN may contain more than one phone: the
 * alphabet and variable-width phone names belong to a language module.
 * In particular, digits are only candidate stress markers and a colon is
 * only a candidate length marker here. No timing or pronunciation semantics
 * are inferred by this scanner. A backslash and the next byte remain in the
 * same literal span, so an escaped delimiter has no structural meaning.
 */
enum reloq_phonetic_kind {
    RELOQ_PHONETIC_TEXT = 1,
    RELOQ_PHONETIC_GROUP_OPEN,
    RELOQ_PHONETIC_GROUP_CLOSE,
    RELOQ_PHONETIC_SEGMENT_DOT,
    RELOQ_PHONETIC_MODIFIER_DIGITS,
    RELOQ_PHONETIC_PHONE_RUN,
    RELOQ_PHONETIC_PHONE_QUOTE,
    RELOQ_PHONETIC_LENGTH_COLON,
    RELOQ_PHONETIC_CONTROL_DIGITS,
    RELOQ_PHONETIC_EXPLICIT_PAUSE,
    RELOQ_PHONETIC_CALLBACK_NUL
};

struct reloq_phonetic_token {
    enum reloq_phonetic_kind kind;
    size_t offset;
    size_t length;
    uint32_t number; /* CONTROL_DIGITS / EXPLICIT_PAUSE only */
};

struct reloq_phonetic_cursor {
    size_t offset;
    int in_group;
};

/* Use an all-zero cursor. Input is length-bounded, not required to be NUL
 * terminated. NUL bytes separating callback fragments are returned as
 * CALLBACK_NUL, including a final NUL; a NUL inside a pronunciation group
 * is invalid. On -1 (invalid arguments, malformed pause, overflow,
 * unterminated group), neither cursor nor token changes. On success the
 * token's span is always nonempty and points into the original input.
 * max_pause_ms constrains only an explicit `pNNN request. A `NNN marker is
 * an opaque control number; both forms retain their exact source spelling.
 */
int reloq_phonetic_next(const unsigned char *input, size_t input_length,
                        struct reloq_phonetic_cursor *cursor,
                        uint32_t max_pause_ms,
                        struct reloq_phonetic_token *token);

#endif
