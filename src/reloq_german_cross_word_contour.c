#include "reloq_german_cross_word_contour.h"

#include <limits.h>

struct contour_values {
    int32_t first_start, first_hold;
    int32_t final[4];
    int32_t first_hold_percent, last_rise_percent;
};

/* Measured phrase-role coefficients for the default 11,025-Hz German
 * voice. They are independent of word spelling and of p/t/k place. */
static int contour_values(enum reloq_german_terminal_mark mark,
                          struct contour_values *out)
{
    switch (reloq_german_terminal_contour_mark(mark)) {
    case RELOQ_DE_TERMINAL_IMPLICIT:
    case RELOQ_DE_TERMINAL_PERIOD:
    case RELOQ_DE_TERMINAL_COLON:
        *out = (struct contour_values){1153, 1224,
            {1043, 1293, 793, 740}, 75, 15};
        return 0;
    case RELOQ_DE_TERMINAL_QUESTION:
        *out = (struct contour_values){939, 868,
            {1043, 1293, 1150, 1293}, 75, 15};
        return 0;
    case RELOQ_DE_TERMINAL_EXCLAMATION:
        *out = (struct contour_values){868, 1403,
            {1043, 1221, 829, 740}, 90, 15};
        return 0;
    case RELOQ_DE_TERMINAL_SEMICOLON:
    case RELOQ_DE_TERMINAL_COMMA:
        *out = (struct contour_values){1153, 1224,
            {1043, 1293, 936, 865}, 75, 15};
        return 0;
    case RELOQ_DE_TERMINAL_ELLIPSIS:
        *out = (struct contour_values){1153, 1224,
            {1043, 1293, 954, 989}, 75, 10};
        return 0;
    default:
        return 1;
    }
}

static int source_gain(enum reloq_german_phone vowel)
{
    switch (vowel) {
    case RELOQ_DE_PHONE_A: return 51;
    case RELOQ_DE_PHONE_E: return 52;
    case RELOQ_DE_PHONE_I: return 53;
    case RELOQ_DE_PHONE_O: return 53;
    case RELOQ_DE_PHONE_U: return 54;
    case RELOQ_DE_PHONE_a: return 50;
    case RELOQ_DE_PHONE_e: return 51;
    case RELOQ_DE_PHONE_i: return 53;
    case RELOQ_DE_PHONE_o: return 54;
    case RELOQ_DE_PHONE_u: return 55;
    default: return -1;
    }
}

static int valid_coda(enum reloq_german_phone coda)
{
    return coda == RELOQ_DE_PHONE_NONE || coda == RELOQ_DE_PHONE_p ||
           coda == RELOQ_DE_PHONE_t || coda == RELOQ_DE_PHONE_k;
}

static int32_t terminal_aspiration(
    const struct reloq_german_cross_word_word *last)
{
    if (last->coda == RELOQ_DE_PHONE_NONE ||
        last->coda == RELOQ_DE_PHONE_p) return 35;
    if (last->coda == RELOQ_DE_PHONE_t) return 48;
    if (last->vowel == RELOQ_DE_PHONE_I) return 53;
    if (last->vowel == RELOQ_DE_PHONE_e ||
        last->vowel == RELOQ_DE_PHONE_i) return 51;
    return 48;
}

static int64_t percentage(int64_t start, int64_t span, int percent)
{
    return start + (span / 100) * percent +
           ((span % 100) * percent) / 100;
}

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_cross_word_contour_append(
    const struct reloq_german_cross_word_word *words, size_t word_count,
    enum reloq_german_terminal_mark mark, int64_t planned_ticks,
    struct reloq_target_builder *builder)
{
    if (!words || !builder) return -1;
    if (word_count < 2 || word_count > 20) return 1;
    struct contour_values values;
    if (contour_values(mark, &values)) return 1;
    if (planned_ticks <= 0) return -1;
    for (size_t i = 0; i < word_count; ++i) {
        const struct reloq_german_cross_word_word *w = &words[i];
        if ((w->initial != RELOQ_DE_PHONE_p &&
             w->initial != RELOQ_DE_PHONE_t &&
             w->initial != RELOQ_DE_PHONE_k) ||
            source_gain(w->vowel) < 0 || !valid_coda(w->coda)) return 1;
        if (w->vowel_onset < 0 || w->vowel_closure <= w->vowel_onset ||
            w->f0_early_tick < 0 ||
            w->f0_early_tick >= w->vowel_onset ||
            w->vowel_closure >= planned_ticks ||
            (i && (w->vowel_onset <= words[i - 1].vowel_closure ||
                   w->f0_early_tick <= words[i - 1].vowel_closure)))
            return -1;
    }
    const struct reloq_german_cross_word_word *last =
        &words[word_count - 1];
    if (last->vowel_closure > INT64_MAX - 60 ||
        planned_ticks < last->vowel_closure + 60) return -1;

    struct reloq_target_event events[512];
    size_t count = 0;
#define PUT(stream, tick, value) put(events, &count, stream, tick, value)
    for (size_t i = 0; i < word_count; ++i) {
        const struct reloq_german_cross_word_word *w = &words[i];
        const int64_t span = w->vowel_closure - w->vowel_onset;
        if (i == 0) {
            PUT(RELOQ_TARGET_F0, w->f0_early_tick,
                values.first_start);
            PUT(RELOQ_TARGET_F0,
                percentage(w->vowel_onset, span,
                           values.first_hold_percent), values.first_hold);
        } else if (i + 1 < word_count) {
            const int32_t first = i == 1 &&
                (reloq_german_terminal_contour_mark(mark) == RELOQ_DE_TERMINAL_QUESTION ||
                 reloq_german_terminal_contour_mark(mark) == RELOQ_DE_TERMINAL_EXCLAMATION) ? 1043 :
                1132 - 3 * (int32_t)(i - 1);
            PUT(RELOQ_TARGET_F0, w->f0_early_tick, first);
            PUT(RELOQ_TARGET_F0,
                percentage(w->vowel_onset, span, 75),
                1221 - 3 * (int32_t)(i - 1));
        } else {
            const int32_t downstep = 3 * (int32_t)(word_count - 2);
            PUT(RELOQ_TARGET_F0, w->f0_early_tick,
                values.final[0] - downstep);
            PUT(RELOQ_TARGET_F0,
                percentage(w->vowel_onset, span,
                           values.last_rise_percent),
                values.final[1] - downstep);
            PUT(RELOQ_TARGET_F0,
                percentage(w->vowel_onset, span, 70),
                values.final[2] - downstep);
            PUT(RELOQ_TARGET_F0,
                percentage(w->vowel_onset, span, 99),
                values.final[3] - downstep);
        }
    }
    PUT(RELOQ_TARGET_AV, 0, 0);
    for (size_t i = 0; i < word_count; ++i) {
        const struct reloq_german_cross_word_word *w = &words[i];
        const int32_t gain = source_gain(w->vowel);
        PUT(RELOQ_TARGET_AV, w->vowel_onset, 0);
        PUT(RELOQ_TARGET_AV, w->vowel_onset + 1, gain);
        PUT(RELOQ_TARGET_AV, w->vowel_closure,
            gain - (i + 1 == word_count ? 5 : 2));
        if (i + 1 < word_count || w->coda != RELOQ_DE_PHONE_NONE) {
            PUT(RELOQ_TARGET_AV, w->vowel_closure, 0);
        } else {
            PUT(RELOQ_TARGET_AV, w->vowel_closure + 60, 0);
        }
    }
    PUT(RELOQ_TARGET_AV, planned_ticks, 0);

    PUT(RELOQ_TARGET_AH, 0, 0);
    for (size_t i = 0; i < word_count; ++i) {
        const struct reloq_german_cross_word_word *w = &words[i];
        const int32_t initial_ah = w->initial == RELOQ_DE_PHONE_p ? 49 :
            w->initial == RELOQ_DE_PHONE_t ? 47 : 45;
        PUT(RELOQ_TARGET_AH, w->burst_end, 0);
        PUT(RELOQ_TARGET_AH, w->burst_end, initial_ah);
        PUT(RELOQ_TARGET_AH, w->vowel_onset, initial_ah);
        PUT(RELOQ_TARGET_AH, w->vowel_onset + 1, 34);
        PUT(RELOQ_TARGET_AH, w->vowel_closure, 34);
        if (i + 1 < word_count || w->coda != RELOQ_DE_PHONE_NONE) {
            PUT(RELOQ_TARGET_AH, w->vowel_closure, 0);
        } else {
            PUT(RELOQ_TARGET_AH, w->vowel_closure, 35);
        }
        if (i + 1 < word_count) {
            const enum reloq_german_phone next = words[i + 1].initial;
            int32_t coda_ah = 0;
            int64_t coda_ticks = 0;
            if (w->coda == RELOQ_DE_PHONE_k &&
                next == RELOQ_DE_PHONE_t) {
                coda_ah = 50;
                coda_ticks = 20;
            } else if (w->coda == RELOQ_DE_PHONE_t &&
                       next != RELOQ_DE_PHONE_t) {
                coda_ah = 51;
                coda_ticks = next == RELOQ_DE_PHONE_p ? 1 : 30;
            }
            if (coda_ah) {
                PUT(RELOQ_TARGET_AH, w->boundary_release, 0);
                PUT(RELOQ_TARGET_AH, w->boundary_release, coda_ah);
                PUT(RELOQ_TARGET_AH,
                    w->boundary_release + coda_ticks, coda_ah);
                PUT(RELOQ_TARGET_AH,
                    w->boundary_release + coda_ticks, 0);
            }
        } else {
            const int64_t release = w->coda == RELOQ_DE_PHONE_NONE ?
                w->vowel_closure : w->boundary_release;
            const int32_t aspiration = terminal_aspiration(w);
            if (w->coda != RELOQ_DE_PHONE_NONE) {
                PUT(RELOQ_TARGET_AH, release, 0);
                PUT(RELOQ_TARGET_AH, release, aspiration);
            }
            PUT(RELOQ_TARGET_AH, release + 60, aspiration);
            PUT(RELOQ_TARGET_AH, release + 60, 0);
        }
    }
    PUT(RELOQ_TARGET_AH, planned_ticks, 0);
#undef PUT
    if (count > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder, 0, events, count)) return -1;
    return 0;
}
