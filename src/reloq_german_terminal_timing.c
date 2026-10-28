#include "reloq_german_terminal_timing.h"

int reloq_german_terminal_timing(
    enum reloq_german_terminal_mark mark,
    struct reloq_german_terminal_timing *result)
{
    if (!result) return -1;
    int32_t window;
    switch (mark) {
    case RELOQ_DE_TERMINAL_CONTROL: window=110;break;
    case RELOQ_DE_TERMINAL_IMPLICIT:
    case RELOQ_DE_TERMINAL_PERIOD:
    case RELOQ_DE_TERMINAL_QUESTION:
    case RELOQ_DE_TERMINAL_EXCLAMATION:
    case RELOQ_DE_TERMINAL_COLON:
        window = 460;
        break;
    case RELOQ_DE_TERMINAL_SEMICOLON:
        window = 310;
        break;
    case RELOQ_DE_TERMINAL_COMMA:
    case RELOQ_DE_TERMINAL_QUESTION_CONTINUE:
    case RELOQ_DE_TERMINAL_EXCLAMATION_CONTINUE:
    case RELOQ_DE_TERMINAL_ELLIPSIS:
        window = 210;
        break;
    default:
        return -1;
    }
    struct reloq_german_terminal_timing next = {
        window, 61, window - 61
    };
    *result = next;
    return 0;
}
