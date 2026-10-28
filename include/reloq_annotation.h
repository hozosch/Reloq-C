#ifndef RELOQ_ANNOTATION_H
#define RELOQ_ANNOTATION_H

#include <stddef.h>
#include <stdint.h>

/* A bounded scanner for an explicit pause inside an annotation stream.
 * Everything else is returned unchanged as a text span; this does not
 * decode phones, words, stress, language rules, or prosody. An escaped
 * backtick (preceded by a backslash) belongs to the text span.
 *
 * Recognized spelling: backtick, 'p', one or more decimal digits, followed
 * by ASCII whitespace or the end of input. The requested duration is kept
 * verbatim: the scheduler must handle contextual pause timing separately.
 */
enum reloq_annotation_kind {
    RELOQ_ANNOTATION_TEXT = 1,
    RELOQ_ANNOTATION_EXPLICIT_PAUSE = 2
};

struct reloq_annotation_event {
    enum reloq_annotation_kind kind;
    size_t offset;
    size_t length;
    uint32_t requested_ms; /* meaningful only for EXPLICIT_PAUSE */
};

struct reloq_annotation_cursor {
    size_t offset;
    int in_pronunciation;
};

/* Start with an all-zero cursor; return 1 for an event, 0 at end, -1 for invalid
 * input, overflow, or a pause exceeding max_pause_ms. On error, neither
 * cursor nor event changes. The caller retains the input for text spans. */
int reloq_annotation_next(const unsigned char *input, size_t input_length,
                          struct reloq_annotation_cursor *cursor,
                          uint32_t max_pause_ms,
                          struct reloq_annotation_event *event);

#endif
