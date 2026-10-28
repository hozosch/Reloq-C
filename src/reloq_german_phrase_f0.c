#include "reloq_german_phrase_f0.h"

#include <limits.h>

/* These are one default voice's shared phrase-contour coefficients, not
 * recorded target sequences per word or a reference binary data table.
 * Percentages interpolate from the vowel acoustic anchor to AV closure. */
struct phrase_contour {
    int32_t first_without_h, first_with_h;
    int32_t second_percent, second_value;
    int32_t third_percent, third_value;
    int32_t fourth_value;
};

static int contour_for_mark(enum reloq_german_terminal_mark mark,
                            struct phrase_contour *c)
{
    switch (reloq_german_terminal_contour_mark(mark)) {
    case RELOQ_DE_TERMINAL_IMPLICIT:
    case RELOQ_DE_TERMINAL_PERIOD:
    case RELOQ_DE_TERMINAL_COLON:
        *c = (struct phrase_contour){1217, 1049, 35, 1296, 70, 796, 743};
        return 0;
    case RELOQ_DE_TERMINAL_QUESTION:
        *c = (struct phrase_contour){1046, 1049, 60, 957, 90, 1224, 1403};
        return 0;
    case RELOQ_DE_TERMINAL_EXCLAMATION:
        *c = (struct phrase_contour){1046, 1013, 15, 1331, 70, 832, 743};
        return 0;
    case RELOQ_DE_TERMINAL_SEMICOLON:
    case RELOQ_DE_TERMINAL_COMMA:
        *c = (struct phrase_contour){1217, 1049, 35, 1296, 70, 939, 868};
        return 0;
    case RELOQ_DE_TERMINAL_ELLIPSIS:
        *c = (struct phrase_contour){1217, 1049, 10, 1210, 70, 957, 992};
        return 0;
    default:
        return -1;
    }
}

static int is_initial_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

static int is_vowel(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
        return 1;
    default:
        return 0;
    }
}

int reloq_german_phrase_f0_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    enum reloq_german_terminal_mark mark, int64_t first_window_ticks,
    int64_t vowel_onset_tick, int64_t vowel_closure_tick,
    int64_t release_start_tick, struct reloq_target_builder *builder,
    int64_t *total_ticks)
{
    if (!phones || !builder || !total_ticks || !phone_count ||
        phone_count > 3) return -1;
    struct phrase_contour c;
    struct reloq_german_terminal_timing terminal;
    if (contour_for_mark(mark, &c) ||
        reloq_german_terminal_timing(mark, &terminal)) return -1;

    const int has_h = phones[0].phone == RELOQ_DE_PHONE_h;
    const int has_stop = is_initial_stop(phones[0].phone);
    const size_t vowel_index = has_h || has_stop ? 1 : 0;
    if (vowel_index >= phone_count ||
        phone_count - vowel_index > 2 ||
        !is_vowel(phones[vowel_index].phone)) return 1;
    int has_fricative_coda = 0;
    if (phone_count - vowel_index == 2) {
        const enum reloq_german_phone coda = phones[vowel_index + 1].phone;
        has_fricative_coda = coda == RELOQ_DE_PHONE_s ||
                             coda == RELOQ_DE_PHONE_f;
        if (!has_fricative_coda && coda != RELOQ_DE_PHONE_p &&
                   coda != RELOQ_DE_PHONE_t &&
                   coda != RELOQ_DE_PHONE_k) return 1;
    }
    for (size_t i = 0; i < phone_count; ++i) {
        if (phones[i].word_index != 0 || phones[i].syllable_index != 0 ||
            phones[i].stress != 1 || !phones[i].stress_explicit)
            return 1;
    }

    /* A final s/f keeps its 30-tick active fade. Its quiet continuation is
     * 30 ticks shorter than the corresponding vowel/stop terminal window
     * for every measured mark (430/280/180 instead of 460/310/210). */
    const int32_t final_window = has_fricative_coda ?
        terminal.window_ticks - 30 : terminal.window_ticks;
    if ((has_h && vowel_onset_tick != 80) ||
        (has_stop && (vowel_onset_tick <= 10 ||
                      vowel_onset_tick >= 800 ||
                      first_window_ticks >= vowel_onset_tick)) ||
        (!has_h && !has_stop && vowel_onset_tick != 0) ||
        vowel_closure_tick <= vowel_onset_tick ||
        release_start_tick < vowel_closure_tick ||
        release_start_tick > INT64_MAX - final_window ||
        vowel_closure_tick - vowel_onset_tick > INT32_MAX ||
        (has_h || has_stop ? (first_window_ticks < 10 ||
                             first_window_ticks >= 800) :
                            (first_window_ticks != -1)))
        return -1;

    const int64_t span = vowel_closure_tick - vowel_onset_tick;
    const int64_t first = has_h || has_stop ?
        first_window_ticks / 10 : span * 2 / 100;
    const int64_t second = vowel_onset_tick +
        span * c.second_percent / 100;
    const int64_t third = vowel_onset_tick + span * c.third_percent / 100;
    const int64_t fourth = vowel_onset_tick + span * 99 / 100;
    if (first >= second || second >= third || third >= fourth ||
        fourth >= vowel_closure_tick) return -1;

    if (has_stop) {
        c.first_with_h = reloq_german_terminal_contour_mark(mark) == RELOQ_DE_TERMINAL_EXCLAMATION ? 1010 : 1046;
        if (mark == RELOQ_DE_TERMINAL_ELLIPSIS) c.second_value = 1296;
    }
    const struct reloq_target_event f0[] = {
        {RELOQ_TARGET_F0, first,
         has_h || has_stop ? c.first_with_h : c.first_without_h},
        {RELOQ_TARGET_F0, second, c.second_value},
        {RELOQ_TARGET_F0, third, c.third_value},
        {RELOQ_TARGET_F0, fourth, c.fourth_value}
    };
    if (reloq_target_builder_append(builder, 0, f0,
                                    sizeof f0 / sizeof f0[0])) return -1;
    *total_ticks = release_start_tick + final_window;
    return 0;
}
