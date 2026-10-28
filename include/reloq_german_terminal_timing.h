#ifndef RELOQ_GERMAN_TERMINAL_TIMING_H
#define RELOQ_GERMAN_TERMINAL_TIMING_H

#include <stdint.h>

/* A phrase-final punctuation decision, supplied by a separate text/prosody
 * analyser. These names do not classify punctuation within a phrase. */
enum reloq_german_terminal_mark {
    RELOQ_DE_TERMINAL_IMPLICIT = 0,
    RELOQ_DE_TERMINAL_PERIOD,
    RELOQ_DE_TERMINAL_QUESTION,
    RELOQ_DE_TERMINAL_EXCLAMATION,
    RELOQ_DE_TERMINAL_COLON,
    RELOQ_DE_TERMINAL_SEMICOLON,
    RELOQ_DE_TERMINAL_COMMA,
    RELOQ_DE_TERMINAL_ELLIPSIS,
    RELOQ_DE_TERMINAL_QUESTION_CONTINUE,
    RELOQ_DE_TERMINAL_EXCLAMATION_CONTINUE,
    RELOQ_DE_TERMINAL_CONTROL
};

static inline int reloq_german_terminal_continues(enum reloq_german_terminal_mark m)
{
    return m==RELOQ_DE_TERMINAL_CONTROL || m==RELOQ_DE_TERMINAL_COMMA || m==RELOQ_DE_TERMINAL_ELLIPSIS ||
        m==RELOQ_DE_TERMINAL_QUESTION_CONTINUE || m==RELOQ_DE_TERMINAL_EXCLAMATION_CONTINUE;
}
static inline enum reloq_german_terminal_mark reloq_german_terminal_contour_mark(enum reloq_german_terminal_mark m)
{
    return m==RELOQ_DE_TERMINAL_QUESTION_CONTINUE?RELOQ_DE_TERMINAL_QUESTION:
        m==RELOQ_DE_TERMINAL_EXCLAMATION_CONTINUE?RELOQ_DE_TERMINAL_EXCLAMATION:m;
}

/* One nominal tick is 1 ms for the observed default German 11,025-Hz
 * reference voice. The source-release part includes the transition down to
 * zero excitation; zero_source_ticks then counts frames with every observed
 * source amplitude control at zero. These values only determine a final
 * scheduling window. They do not author the F0 contour, source fade, PCM,
 * phoneme duration or a mid-phrase pause. */
struct reloq_german_terminal_timing {
    int32_t window_ticks;
    int32_t source_release_ticks;
    int32_t zero_source_ticks;
};

/* Return 0 for a recognized phrase-final mark. Invalid inputs return -1 and
 * leave the result unchanged. The caller decides whether a phrase boundary
 * exists before calling this function. */
int reloq_german_terminal_timing(
    enum reloq_german_terminal_mark mark,
    struct reloq_german_terminal_timing *result);

#endif
