#ifndef RELOQ_GERMAN_ANNOTATION_H
#define RELOQ_GERMAN_ANNOTATION_H

#include "reloq_german_spr.h"
#include "reloq_phonetic.h"

/* A lossless, bounded view of a complete German ECI phoneme callback. The
 * outer scanner retains literal text, numeric controls, explicit pauses and
 * callback NULs. Each SPR group is expanded to typed German word, syllable,
 * stress and phone events. No word-to-SPR or SPR-to-duration rule is implied.
 */
enum reloq_german_annotation_kind {
    RELOQ_DE_ANNOTATION_TEXT = 1,
    RELOQ_DE_ANNOTATION_CONTROL,
    RELOQ_DE_ANNOTATION_PAUSE,
    RELOQ_DE_ANNOTATION_CALLBACK_NUL,
    RELOQ_DE_ANNOTATION_SPR
};

struct reloq_german_annotation_event {
    enum reloq_german_annotation_kind kind;
    size_t offset; /* absolute source byte offset */
    size_t length; /* zero only for an implicit first syllable */
    uint32_t number; /* CONTROL/PAUSE only */
    struct reloq_german_spr_event spr; /* SPR only, absolute source span */
};

struct reloq_german_annotation_cursor {
    struct reloq_phonetic_cursor outer;
    struct reloq_german_spr_cursor spr;
    size_t group_offset;
    size_t group_length;
    int in_group;
};

/* Start with an all-zero cursor. Input may contain NUL-separated fragments
 * and need not be terminated. On 1, return one event; 0 means end; -1 means
 * invalid input, a malformed German SPR group or an over-limit pause. On
 * -1, neither cursor nor event changes. The source span is exactly the
 * original spelling, except an implicit syllable boundary has length zero.
 */
int reloq_german_annotation_next(
    const unsigned char *input, size_t input_length,
    struct reloq_german_annotation_cursor *cursor, uint32_t max_pause_ms,
    struct reloq_german_annotation_event *event);

#endif
