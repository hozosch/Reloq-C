#include "reloq_german_cross_word_landmarks.h"

#include "reloq_german_hv_timing.h"

#include <limits.h>
#include <stddef.h>

struct vowel_timing {
    enum reloq_german_phone phone;
    int16_t p_anchor, t_anchor, k_anchor;
    int16_t nonfinal_closed, nonfinal_open;
    int16_t final_closed, final_open;
};

/* A nonfinal stressed vowel differs from an isolated final one. An open
 * first vowel also differs from a vowel constrained by its own final stop.
 * These are phone-class durations on the reference voice's 1-ms clock. */
static const struct vowel_timing vowel_timing[] = {
    {RELOQ_DE_PHONE_A, 65,  89, 78,  81,  81,  87,  87},
    {RELOQ_DE_PHONE_E, 65,  95, 78,  81,  81,  94,  94},
    {RELOQ_DE_PHONE_I, 65,  95, 78,  75,  75,  88,  88},
    {RELOQ_DE_PHONE_O, 65,  89, 78,  75,  75,  88,  88},
    {RELOQ_DE_PHONE_U, 50,  67, 78,  64,  64,  75,  75},
    {RELOQ_DE_PHONE_a, 65,  89, 78, 126, 176, 151, 189},
    {RELOQ_DE_PHONE_e, 78, 113, 93, 117, 140, 163, 187},
    {RELOQ_DE_PHONE_i, 78, 113, 93, 108, 129, 151, 172},
    {RELOQ_DE_PHONE_o, 78, 107, 93, 117, 140, 163, 187},
    {RELOQ_DE_PHONE_u, 59,  80, 93, 108, 129, 151, 172}
};

static int stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

static int front(enum reloq_german_phone vowel)
{
    return vowel == RELOQ_DE_PHONE_E || vowel == RELOQ_DE_PHONE_I ||
           vowel == RELOQ_DE_PHONE_e || vowel == RELOQ_DE_PHONE_i;
}

static const struct vowel_timing *vowel(enum reloq_german_phone phone)
{
    for (size_t i = 0; i < sizeof vowel_timing / sizeof vowel_timing[0];
         ++i)
        if (vowel_timing[i].phone == phone) return &vowel_timing[i];
    return NULL;
}

static int validate(const struct reloq_german_phone_node *phones,
                    size_t phone_count,
                    const struct reloq_german_syllable *syllables,
                    size_t count)
{
    if (count < 2 || count > SIZE_MAX / 2 ||
        phone_count < count * 2) return 1;
    size_t next = 0;
    for (size_t i = 0; i < count; ++i) {
        const struct reloq_german_syllable *s = syllables + i;
        if (s->first_phone != next || s->word_index != i ||
            s->syllable_in_word || s->onset_count != 1 ||
            s->nucleus_count != 1 || s->coda_count > 1 ||
            s->phone_count != 2 + s->coda_count ||
            !s->nucleus_contiguous || s->stress != 1 ||
            !s->stress_explicit ||
            s->boundary_after != (i + 1 == count ?
                RELOQ_DE_BOUNDARY_UTTERANCE : RELOQ_DE_BOUNDARY_WORD) ||
            next > phone_count || s->phone_count > phone_count - next ||
            !stop(phones[next].phone) || !vowel(phones[next + 1].phone) ||
            (s->coda_count && !stop(phones[next + 2].phone))) return 1;
        for (size_t p = 0; p < s->phone_count; ++p)
            if (phones[next + p].word_index != i ||
                phones[next + p].syllable_index != i ||
                phones[next + p].syllable_in_word != 0 ||
                phones[next + p].stress != 1 ||
                !phones[next + p].stress_explicit) return 1;
        next += s->phone_count;
    }
    return next == phone_count ? 0 : 1;
}

/* Release of the preceding word, measured from its vowel closure. The
 * open-CV boundary includes the following stop's burst; a CVC boundary
 * ends its own coda before the next onset. */
static int64_t boundary_offset(enum reloq_german_phone coda,
                               enum reloq_german_phone next,
                               int next_front)
{
    if (coda == RELOQ_DE_PHONE_NONE)
        return next == RELOQ_DE_PHONE_p ? 100 :
            next == RELOQ_DE_PHONE_t ? 80 + (next_front ? 6 : 0) : 87;
    if (coda == RELOQ_DE_PHONE_p)
        return next == RELOQ_DE_PHONE_p ? 40 : 100;
    if (coda == RELOQ_DE_PHONE_t)
        return next == RELOQ_DE_PHONE_p ? 58 :
            next == RELOQ_DE_PHONE_t ? 30 : 58;
    return next == RELOQ_DE_PHONE_k ? 65 : 52;
}

static int64_t early_lead(enum reloq_german_phone coda,
                          enum reloq_german_phone next)
{
    if (coda == RELOQ_DE_PHONE_NONE)
        return next == RELOQ_DE_PHONE_p ? 10 : 8;
    if (coda == RELOQ_DE_PHONE_t && next == RELOQ_DE_PHONE_k)
        return 38;
    if (coda == RELOQ_DE_PHONE_k && next == RELOQ_DE_PHONE_t)
        return 28;
    return next == RELOQ_DE_PHONE_p ? 11 :
           next == RELOQ_DE_PHONE_t ? 9 : 7;
}

static int64_t next_burst_offset(enum reloq_german_phone coda,
                                 enum reloq_german_phone next,
                                 int next_front)
{
    if (coda == RELOQ_DE_PHONE_NONE) return 0;
    if (next == RELOQ_DE_PHONE_p) return 101;
    if (next == RELOQ_DE_PHONE_k)
        return coda == RELOQ_DE_PHONE_t ? 117 : 68;
    return (coda == RELOQ_DE_PHONE_k ? 100 : 81) +
           (next_front ? 6 : 0);
}

static int64_t next_vowel_offset(enum reloq_german_phone coda,
                                 enum reloq_german_phone next,
                                 const struct vowel_timing *v,
                                 int next_front)
{
    int64_t isolated = next == RELOQ_DE_PHONE_p ? v->p_anchor - 15 :
        next == RELOQ_DE_PHONE_t ? v->t_anchor -
                                  (next_front ? 23 : 17) :
                                  v->k_anchor - 17;
    /* Only the cross-word /k/→/t/ transition shortens post-burst air. */
    return coda == RELOQ_DE_PHONE_k && next == RELOQ_DE_PHONE_t ?
        isolated * 3 / 5 : isolated;
}

static int64_t final_release_offset(enum reloq_german_phone coda)
{
    return coda == RELOQ_DE_PHONE_p ? 125 :
        coda == RELOQ_DE_PHONE_t ? 108 :
        coda == RELOQ_DE_PHONE_k ? 122 : 0;
}

static int plan_one(const struct reloq_german_phone_node *phones,
                    const struct reloq_german_syllable *syllables,
                    size_t count, size_t index,
                    enum reloq_german_terminal_mark mark,
                    const struct reloq_german_cross_word_word *previous,
                    struct reloq_german_cross_word_word *out)
{
    const struct reloq_german_syllable *s = syllables + index;
    const size_t begin = s->first_phone;
    const enum reloq_german_phone initial = phones[begin].phone;
    const struct vowel_timing *v = vowel(phones[begin + 1].phone);
    const enum reloq_german_phone coda = s->coda_count ?
        phones[begin + 2].phone : RELOQ_DE_PHONE_NONE;
    const int last = index + 1 == count, is_front = front(v->phone);
    if (previous && previous->boundary_release > INT64_MAX - 1024)
        return 1;
    struct reloq_german_cross_word_word result = {0};
    result.initial = initial;
    result.vowel = v->phone;
    result.coda = coda;
    if (!index) {
        result.onset_origin = 0;
        result.burst_start = 5;
        result.burst_end = initial == RELOQ_DE_PHONE_p ? 15 :
            initial == RELOQ_DE_PHONE_t && is_front ? 23 : 17;
        result.vowel_onset = initial == RELOQ_DE_PHONE_p ? v->p_anchor :
            initial == RELOQ_DE_PHONE_t ? v->t_anchor : v->k_anchor;
        result.f0_early_tick = reloq_german_terminal_contour_mark(mark) == RELOQ_DE_TERMINAL_EXCLAMATION ?
            3 * result.burst_end / 5 : result.burst_end / 20 +
            (reloq_german_terminal_contour_mark(mark) == RELOQ_DE_TERMINAL_QUESTION ? 1 : 0);
    } else {
        result.onset_origin = previous->coda == RELOQ_DE_PHONE_NONE ?
            previous->vowel_closure : previous->boundary_release +
            (previous->coda == RELOQ_DE_PHONE_k &&
             initial == RELOQ_DE_PHONE_t ? 20 :
             previous->coda == RELOQ_DE_PHONE_t &&
             initial == RELOQ_DE_PHONE_k ? 30 : 1);
        result.burst_end = previous->boundary_release +
            next_burst_offset(previous->coda, initial, is_front);
        result.burst_start = result.burst_end -
            (initial == RELOQ_DE_PHONE_p ? 10 :
             initial == RELOQ_DE_PHONE_k ? 12 : is_front ? 18 : 12);
        result.vowel_onset = result.burst_end +
            next_vowel_offset(previous->coda, initial, v, is_front);
        result.f0_early_tick =
            (previous->coda == RELOQ_DE_PHONE_NONE ?
             previous->vowel_closure : previous->boundary_release) +
            early_lead(previous->coda, initial);
        if (!last) result.f0_early_tick +=
            initial == RELOQ_DE_PHONE_p ? 40 :
            initial == RELOQ_DE_PHONE_t ? is_front ? 35 : 32 :
            previous->coda == RELOQ_DE_PHONE_p ||
            previous->coda == RELOQ_DE_PHONE_k ? 27 : 35;
    }
    if (result.vowel_onset > INT64_MAX - 1024) return 1;
    result.vowel_closure = result.vowel_onset +
        (last ? coda == RELOQ_DE_PHONE_NONE ? v->final_open :
                                          v->final_closed :
         coda == RELOQ_DE_PHONE_NONE ? v->nonfinal_open :
                                       v->nonfinal_closed);
    if (coda != RELOQ_DE_PHONE_NONE || !last) {
        enum reloq_german_phone hold_stop = coda != RELOQ_DE_PHONE_NONE ?
            coda : phones[syllables[index + 1].first_phone].phone;
        int64_t lead = 0;
        if (reloq_german_coda_hold_lead(v->phone, hold_stop, &lead))
            return 1;
        if (coda == RELOQ_DE_PHONE_NONE &&
            hold_stop == RELOQ_DE_PHONE_k && !is_front)
            lead = 20;
        result.vowel_hold = result.vowel_closure - lead;
    } else result.vowel_hold = result.vowel_closure;
    if (!last) {
        const enum reloq_german_phone next =
            phones[syllables[index + 1].first_phone].phone;
        const int next_front = front(
            phones[syllables[index + 1].first_phone + 1].phone);
        if (result.vowel_closure > INT64_MAX - 1024) return 1;
        result.boundary_release = result.vowel_closure +
            boundary_offset(coda, next, next_front);
        if (coda != RELOQ_DE_PHONE_NONE && coda != next)
            result.coda_burst_start = result.boundary_release -
                (coda == RELOQ_DE_PHONE_t ? 8 :
                 coda == RELOQ_DE_PHONE_k ? 12 : 10);
    } else {
        if (result.vowel_closure > INT64_MAX - 1024) return 1;
        result.boundary_release = result.vowel_closure +
            final_release_offset(coda);
        if (coda != RELOQ_DE_PHONE_NONE)
            result.coda_burst_start = result.boundary_release -
                (coda == RELOQ_DE_PHONE_t ? 8 :
                 coda == RELOQ_DE_PHONE_k ? 12 : 10);
    }
    if (result.onset_origin > result.burst_start ||
        result.burst_start >= result.burst_end ||
        result.burst_end >= result.vowel_onset ||
        result.vowel_onset >= result.vowel_closure ||
        result.vowel_hold <= result.vowel_onset ||
        result.vowel_hold > result.vowel_closure ||
        result.boundary_release < result.vowel_closure ||
        result.boundary_release > INT64_MAX - 1024 ||
        (result.coda_burst_start &&
         result.coda_burst_start <= result.vowel_closure)) return 1;
    *out = result;
    return 0;
}

int reloq_german_cross_word_landmarks_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    enum reloq_german_terminal_mark mark,
    struct reloq_german_cross_word_word *words, size_t word_capacity,
    int64_t *planned_ticks)
{
    if (!phones || !syllables || !words || !planned_ticks ||
        word_capacity < syllable_count) return -1;
    struct reloq_german_terminal_timing tail;
    if (reloq_german_terminal_timing(mark, &tail)) return -1;
    if (validate(phones, phone_count, syllables, syllable_count)) return 1;

    /* Dry run prevents any partial caller output on uncovered input. */
    struct reloq_german_cross_word_word prev = {0}, next;
    for (size_t i = 0; i < syllable_count; ++i) {
        if (plan_one(phones, syllables, syllable_count, i, mark,
                     i ? &prev : NULL, &next)) return 1;
        prev = next;
    }
    if (prev.boundary_release > INT64_MAX - tail.window_ticks) return 1;
    const int64_t total = prev.boundary_release + tail.window_ticks;
    prev = (struct reloq_german_cross_word_word){0};
    for (size_t i = 0; i < syllable_count; ++i) {
        (void)plan_one(phones, syllables, syllable_count, i, mark,
                       i ? &prev : NULL, &next);
        words[i] = next;
        prev = next;
    }
    *planned_ticks = total;
    return 0;
}
