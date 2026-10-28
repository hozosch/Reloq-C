#include "reloq_german_nucleus_prosody.h"

#include <math.h>

typedef enum reloq_german_phone Phone;

static int closed(Phone p)
{
    return p == RELOQ_DE_PHONE_i || p == RELOQ_DE_PHONE_u ||
        p == RELOQ_DE_PHONE_y || p == RELOQ_DE_PHONE_e ||
        p == RELOQ_DE_PHONE_o || p == RELOQ_DE_PHONE_Q_oe ||
        p == RELOQ_DE_PHONE_Q_o_TILDE || p == RELOQ_DE_PHONE_Q_oe_TILDE;
}

static int high(Phone p)
{
    return p == RELOQ_DE_PHONE_i || p == RELOQ_DE_PHONE_I ||
        p == RELOQ_DE_PHONE_u || p == RELOQ_DE_PHONE_U ||
        p == RELOQ_DE_PHONE_y || p == RELOQ_DE_PHONE_Y;
}

static int nasal(Phone p)
{
    return p == RELOQ_DE_PHONE_m || p == RELOQ_DE_PHONE_n ||
        p == RELOQ_DE_PHONE_G;
}

static int low(Phone p)
{
    return p == RELOQ_DE_PHONE_a || p == RELOQ_DE_PHONE_A ||
        p == RELOQ_DE_PHONE_R || p == RELOQ_DE_PHONE_Q_a_TILDE;
}

static int consonant(Phone p)
{
    return p >= RELOQ_DE_PHONE_b && p <= RELOQ_DE_PHONE_Z &&
        p != RELOQ_DE_PHONE_R;
}

static Phone at(const struct reloq_german_nucleus_context *c, size_t i)
{
    return i < c->phone_count ? c->phones[i].phone : RELOQ_DE_PHONE_NONE;
}

static int nucleus_base(const struct reloq_german_nucleus_context *c,
                        struct reloq_german_nucleus_duration *out)
{
    const size_t count = c->nucleus_end - c->nucleus_begin;
    if (count > 3) return 0;
    const Phone head = at(c, c->nucleus_begin);
    /* Prosody inspects separate members of a diphthong. */
    if (head >= RELOQ_DE_PHONE_Q_aj && head <= RELOQ_DE_PHONE_Q_oj) return 0;
    for (size_t i = c->nucleus_begin; i < c->nucleus_end; ++i)
        if (c->phones[i].stress > 2 || c->phones[i].short_quantity > 1 ||
            c->phones[i].voiceless > 1 || c->phones[i].vocalized_r > 1)
            return -1;
    const int shortened = c->phones[c->nucleus_begin].short_quantity &&
        (closed(head) || (head >= RELOQ_DE_PHONE_Q_a_TILDE &&
                         head <= RELOQ_DE_PHONE_Q_oe_TILDE));
    return reloq_german_nucleus_duration_base(head,
        count > 1 ? at(c, c->nucleus_begin + 1) : RELOQ_DE_PHONE_NONE,
        count > 2 ? at(c, c->nucleus_begin + 2) : RELOQ_DE_PHONE_NONE,
        shortened, out);
}

static int valid(const struct reloq_german_nucleus_context *c,
                 double input, double *out)
{
    return c && c->phones && out && isfinite(input) && input >= 0 &&
        input <= 32767 && c->word_begin <= c->syllable_begin &&
        c->syllable_begin <= c->nucleus_begin &&
        c->nucleus_begin < c->nucleus_end &&
        c->nucleus_end <= c->syllable_end &&
        c->syllable_end <= c->word_end && c->word_end <= c->phone_count &&
        c->word_syllables && c->syllable_in_word < c->word_syllables &&
        c->stress <= 2 && c->prominence >= -1 && c->prominence <= 2 &&
        c->final_word_in_phrase <= 1 && c->final_syllable_in_phrase <= 1 &&
        c->incoming_transition_left_owned <= 1 &&
        c->following_unstressed_syllables <= c->following_syllables;
}

static int final_unstressed(const struct reloq_german_nucleus_context *c)
{
    const Phone head = at(c, c->nucleus_begin);
    const Phone tail = at(c, c->nucleus_end - 1);
    const size_t members = c->nucleus_end - c->nucleus_begin;
    const int open_word = c->nucleus_end == c->word_end;
    if (open_word) {
        if (c->final_syllable_in_phrase && members == 2 &&
            head == RELOQ_DE_PHONE_a &&
            (tail == RELOQ_DE_PHONE_l || tail == RELOQ_DE_PHONE_r))
            return 17;
        if (members == 1 &&
            (head == RELOQ_DE_PHONE_AT || head == RELOQ_DE_PHONE_R))
            return 15;
        if (members == 2 && head == RELOQ_DE_PHONE_A &&
            tail == RELOQ_DE_PHONE_j) return 14;
        return 13;
    }
    if (head == RELOQ_DE_PHONE_R && c->final_syllable_in_phrase) return 16;
    if (nasal(at(c, c->nucleus_end))) {
        if (tail == RELOQ_DE_PHONE_A) return 14;
        return !c->final_syllable_in_phrase || tail == RELOQ_DE_PHONE_AT ? 11 : 13;
    }
    if (c->word_syllables == 1) return 13;
    if (members >= 2) return c->final_syllable_in_phrase ? 14 : 12;
    return c->final_syllable_in_phrase ? 11 : 10;
}

static int final_stressed(const struct reloq_german_nucleus_context *c)
{
    const Phone head = at(c, c->nucleus_begin);
    const Phone tail = at(c, c->nucleus_end - 1);
    const size_t members = c->nucleus_end - c->nucleus_begin;
    const int open_word = c->nucleus_end == c->word_end;
    const int last_syllable = c->syllable_in_word + 1 == c->word_syllables;
    if (members == 1) {
        if (!c->phones[c->nucleus_begin].stress && !open_word) return 10;
        if (closed(head)) {
            if (!last_syllable) return 11;
            if ((head == RELOQ_DE_PHONE_i || head == RELOQ_DE_PHONE_u) &&
                c->nucleus_end + 1 < c->word_end &&
                consonant(at(c, c->nucleus_end)) &&
                !nasal(at(c, c->nucleus_end)) &&
                consonant(at(c, c->nucleus_end + 1))) return 12;
            return open_word ? 16 : 14;
        }
        if (head == RELOQ_DE_PHONE_a) return open_word ? 15 : 12;
        if (head == RELOQ_DE_PHONE_Q_E_COLON) return 12;
        if (head == RELOQ_DE_PHONE_A) return last_syllable ? 13 : 11;
        return last_syllable ? 14 : 12;
    }
    if (members == 2 &&
        (tail == RELOQ_DE_PHONE_j || tail == RELOQ_DE_PHONE_w)) {
        if (c->final_syllable_in_phrase && c->stress == 2)
            return open_word ? 14 : 12;
        if (open_word) return 16;
        if (c->nucleus_end + 1 == c->word_end &&
            nasal(at(c, c->nucleus_end))) return 14;
        return last_syllable ? 13 : 11;
    }
    if (members == 2 &&
        (tail == RELOQ_DE_PHONE_r || tail == RELOQ_DE_PHONE_l)) {
        if (open_word) {
            if (closed(head)) return 15;
            return head == RELOQ_DE_PHONE_a ? 14 : 12;
        }
        if (head == RELOQ_DE_PHONE_A && tail == RELOQ_DE_PHONE_l) return 11;
        if (last_syllable)
            return tail == RELOQ_DE_PHONE_r && high(head) ? 12 : 14;
        return head == RELOQ_DE_PHONE_A && tail == RELOQ_DE_PHONE_r ? 13 : 12;
    }
    return members == 3 ? 13 : 10;
}

int reloq_german_nucleus_phrase_final(
    const struct reloq_german_nucleus_context *c, double input, double *out)
{
    if (!valid(c, input, out)) return -1;
    struct reloq_german_nucleus_duration base;
    const int supported = nucleus_base(c, &base);
    if (supported != 1) return supported;
    const int tenths = c->stress ? final_stressed(c) : final_unstressed(c);
    double duration = input * tenths / 10.0;
    if (c->stress && c->following_unstressed_syllables >= 2)
        duration = duration * 90.0 / 100.0;
    *out = duration;
    return 1;
}

/* Return the numerator and denominator separately: the reference behavior
 * rounds neither these budgets nor their intermediate products. */
static int medial_percent(const struct reloq_german_nucleus_context *c,
                          int *denominator)
{
    const Phone head = at(c, c->nucleus_begin);
    const Phone tail = at(c, c->nucleus_end - 1);
    const Phone after = at(c, c->nucleus_end);
    const Phone before = c->nucleus_begin ? at(c, c->nucleus_begin - 1) :
        RELOQ_DE_PHONE_NONE;
    const size_t members = c->nucleus_end - c->nucleus_begin;
    const int open_word = c->nucleus_end == c->word_end;
    const int last_syllable = c->syllable_in_word + 1 == c->word_syllables;
    *denominator = 100;
    if (c->prominence < 0 && c->word_syllables == 1) {
        const Phone next = at(c, c->nucleus_begin + 1);
        if ((head == RELOQ_DE_PHONE_A || head == RELOQ_DE_PHONE_E ||
             head == RELOQ_DE_PHONE_U) && nasal(next)) return 100;
        if (closed(head) && head != RELOQ_DE_PHONE_e && nasal(next)) return 90;
        const Phone penultimate = c->nucleus_end >= 2 ?
            at(c, c->nucleus_end - 2) : RELOQ_DE_PHONE_NONE;
        if ((tail == RELOQ_DE_PHONE_r || tail == RELOQ_DE_PHONE_R) &&
            (penultimate == RELOQ_DE_PHONE_i ||
             penultimate == RELOQ_DE_PHONE_u ||
             penultimate == RELOQ_DE_PHONE_e)) return 95;
        if ((tail == RELOQ_DE_PHONE_l || tail == RELOQ_DE_PHONE_r ||
             tail == RELOQ_DE_PHONE_j || tail == RELOQ_DE_PHONE_w) &&
            !closed(penultimate) && !consonant(penultimate) &&
            penultimate != RELOQ_DE_PHONE_NONE) return 90;
        return 80;
    }
    if (tail == RELOQ_DE_PHONE_AT) {
        if (open_word) { *denominator = 10; return 12; }
        if (c->prominence < 0 && consonant(after) &&
            c->nucleus_end + 1 < c->syllable_end &&
            consonant(at(c, c->nucleus_end + 1))) return 80;
        if ((before == RELOQ_DE_PHONE_d || before == RELOQ_DE_PHONE_t) &&
            c->nucleus_end < c->syllable_end && after == RELOQ_DE_PHONE_n)
            return 65;
        return 100;
    }
    if (!c->stress && !last_syllable) {
        if (low(tail) || closed(tail)) return 85;
        if (tail == RELOQ_DE_PHONE_I) return 90;
    }
    if (head == RELOQ_DE_PHONE_R && members == 1 && open_word) {
        *denominator = 10;
        return c->prominence < 0 && c->word_syllables > 1 ? 12 : 14;
    }
    if (last_syllable && c->stress) {
        if ((closed(head) || head == RELOQ_DE_PHONE_a) && !open_word)
            return 100;
        if (!closed(head) && members >= 2 && !open_word) return 100;
        if ((low(tail) || consonant(tail)) &&
            c->nucleus_end + 2 == c->syllable_end && consonant(after) &&
            consonant(at(c, c->nucleus_end + 1))) return 100;
        *denominator = 10;
        if (c->prominence == 0) return 11;
        if (head == RELOQ_DE_PHONE_a && members == 1 && open_word) return 14;
        if (!closed(tail) && !consonant(tail) &&
            !c->phones[c->nucleus_end - 1].short_quantity) return 11;
        return 12;
    }
    if (!c->final_word_in_phrase && c->prominence == 2 &&
        c->stress == 1 && c->word_syllables == 2) {
        *denominator = 10;
        return 11;
    }
    return 100;
}

int reloq_german_nucleus_phrase_medial(
    const struct reloq_german_nucleus_context *c, double input, double *out)
{
    if (!valid(c, input, out)) return -1;
    struct reloq_german_nucleus_duration base;
    const int supported = nucleus_base(c, &base);
    if (supported != 1) return supported;
    int denominator;
    const int numerator = medial_percent(c, &denominator);
    *out = input * numerator / denominator;
    return 1;
}

static int fricative(Phone p)
{
    return p == RELOQ_DE_PHONE_f || p == RELOQ_DE_PHONE_v ||
        p == RELOQ_DE_PHONE_s || p == RELOQ_DE_PHONE_z ||
        p == RELOQ_DE_PHONE_S || p == RELOQ_DE_PHONE_Z ||
        p == RELOQ_DE_PHONE_x || p == RELOQ_DE_PHONE_X;
}

static int obstruent(Phone p)
{
    return fricative(p) || p == RELOQ_DE_PHONE_p || p == RELOQ_DE_PHONE_b ||
        p == RELOQ_DE_PHONE_t || p == RELOQ_DE_PHONE_d ||
        p == RELOQ_DE_PHONE_k || p == RELOQ_DE_PHONE_g;
}

static int glide(const struct reloq_german_duration_phone *p)
{
    return p->phone == RELOQ_DE_PHONE_j || p->phone == RELOQ_DE_PHONE_w ||
        (p->phone == RELOQ_DE_PHONE_r && p->vocalized_r);
}

static int vowel_phone(Phone p)
{
    return (p >= RELOQ_DE_PHONE_a && p <= RELOQ_DE_PHONE_AT) ||
        p == RELOQ_DE_PHONE_R || p == RELOQ_DE_PHONE_Q_E_COLON ||
        p == RELOQ_DE_PHONE_Q_oe || p == RELOQ_DE_PHONE_Q_OE ||
        (p >= RELOQ_DE_PHONE_Q_a_TILDE && p <= RELOQ_DE_PHONE_Q_oe_TILDE);
}

static int adjustment_ratio(const struct reloq_german_nucleus_context *c,
                            int *denominator)
{
    const Phone head = at(c, c->nucleus_begin);
    const Phone tail = at(c, c->nucleus_end - 1);
    const Phone after = c->nucleus_end < c->word_end ?
        at(c, c->nucleus_end) : RELOQ_DE_PHONE_NONE;
    const size_t members = c->nucleus_end - c->nucleus_begin;
    const int last_syllable = c->syllable_in_word + 1 == c->word_syllables;
    const int after_voiced = consonant(after) &&
        !c->phones[c->nucleus_end].voiceless;
    const int tail_long = !c->phones[c->nucleus_end - 1].short_quantity;
    const size_t following = c->word_syllables - c->syllable_in_word - 1;
    *denominator = 100;
    if (!c->stress) {
        if (tail == RELOQ_DE_PHONE_U && nasal(after)) {
            *denominator = 10; return 13;
        }
        if (head == RELOQ_DE_PHONE_R &&
            c->nucleus_begin + 1 < c->word_end &&
            consonant(at(c, c->nucleus_begin + 1)) &&
            !c->phones[c->nucleus_begin + 1].voiceless) {
            *denominator = 10; return 12;
        }
        if ((tail == RELOQ_DE_PHONE_l || tail == RELOQ_DE_PHONE_r) &&
            members >= 2) return head == RELOQ_DE_PHONE_E &&
                tail == RELOQ_DE_PHONE_r ? 70 : 75;
        if (!last_syllable && members == 2 &&
            glide(c->phones + c->nucleus_end - 1)) return 85;
        if (tail == RELOQ_DE_PHONE_a) return 90;
        if (tail == RELOQ_DE_PHONE_o && after == RELOQ_DE_PHONE_r &&
            !c->phones[c->nucleus_end].vocalized_r) return 75;
        if (members == 1 && (closed(head) || head == RELOQ_DE_PHONE_A) &&
            following >= 2) return head == RELOQ_DE_PHONE_A ? 80 : 95;
        return 100;
    }
    if (following >= 2) {
        if (low(tail) && tail_long) return 80;
        if (closed(tail)) return 95;
        return 100;
    }
    if (c->stress == 1) {
        if (tail == RELOQ_DE_PHONE_Q_E_COLON && vowel_phone(after)) {
            *denominator = 10; return 13;
        }
        if (!c->final_syllable_in_phrase && members == 2 &&
            head == RELOQ_DE_PHONE_A &&
            (tail == RELOQ_DE_PHONE_j || tail == RELOQ_DE_PHONE_w) &&
            c->nucleus_end < c->syllable_end &&
            c->nucleus_end + 1 < c->word_end && consonant(after) &&
            consonant(at(c, c->nucleus_end + 1))) return 90;
        if (members == 1 && after_voiced) {
            if (tail == RELOQ_DE_PHONE_a) return 100;
            *denominator = 10;
            if (tail_long || tail == RELOQ_DE_PHONE_A) return 11;
            return c->nucleus_end == c->syllable_end ? 12 : 11;
        }
        if (vowel_phone(after) && !c->phones[c->nucleus_end].stress) {
            *denominator = 10; return 11;
        }
        return 100;
    }
    if (!c->final_syllable_in_phrase && members == 2 && !closed(head) &&
        (tail == RELOQ_DE_PHONE_r || tail == RELOQ_DE_PHONE_l)) return 90;
    if (members == 1 && after_voiced && !closed(head) && !tail_long) {
        *denominator = 10; return 11;
    }
    return 100;
}

int reloq_german_nucleus_adjust(
    const struct reloq_german_nucleus_context *c, double input, double *out)
{
    if (!valid(c, input, out)) return -1;
    const size_t members = c->nucleus_end - c->nucleus_begin;
    struct reloq_german_nucleus_duration base;
    const int supported = nucleus_base(c, &base);
    if (supported != 1) return supported;
    const Phone head = at(c, c->nucleus_begin);
    const Phone tail = at(c, c->nucleus_end - 1);
    const Phone after = c->nucleus_end < c->word_end ?
        at(c, c->nucleus_end) : RELOQ_DE_PHONE_NONE;
    double duration = input;
    if (c->prominence <= 0 && !c->stress) duration = duration * 90.0 / 100.0;
    if (c->prominence < 0 && c->stress == 1 && !c->final_word_in_phrase) {
        if (members == 1 && (closed(tail) || low(tail)))
            duration = duration * 85.0 / 100.0;
        if (members == 2 && head == RELOQ_DE_PHONE_A)
            duration = duration * 90.0 / 100.0;
    }
    const int clustered_fricative = c->nucleus_end + 1 < c->syllable_end &&
        obstruent(at(c, c->nucleus_end + 1));
    const int aw = members == 2 && head == RELOQ_DE_PHONE_A &&
        tail == RELOQ_DE_PHONE_w;
    if (c->stress && fricative(after) && !clustered_fricative && !aw)
        duration = duration * 11.0 / 10.0;
    if (c->prominence >= 0) {
        if (c->incoming_transition_left_owned && c->nucleus_begin > c->word_begin &&
            tail != RELOQ_DE_PHONE_r && tail != RELOQ_DE_PHONE_R &&
            tail != RELOQ_DE_PHONE_l) duration = duration * 90.0 / 100.0;
        int denominator;
        const int numerator = adjustment_ratio(c, &denominator);
        if (numerator != denominator)
            duration = duration * numerator / denominator;
    }
    *out = duration;
    return 1;
}

int reloq_german_nucleus_budget(const struct reloq_german_nucleus_context *c,
                               struct reloq_german_nucleus_budget *out)
{
    double unused;
    if (!out || !valid(c, 0.0, &unused)) return -1;
    struct reloq_german_nucleus_budget result = {0};
    const int status = nucleus_base(c, &result.base);
    if (status != 1) return status;
    result.phrase_ticks = result.base.base_ticks;
    result.region = RELOQ_DE_NUCLEUS_BEFORE_LATER_STRESS;
    if (!c->final_word_in_phrase) {
        result.region = RELOQ_DE_NUCLEUS_MEDIAL;
        if (reloq_german_nucleus_phrase_medial(c, result.phrase_ticks,
                                             &result.phrase_ticks) != 1) return 0;
    } else if (c->word_syllables - c->syllable_in_word - 1 <=
               c->following_unstressed_syllables) {
        result.region = RELOQ_DE_NUCLEUS_FINAL;
        if (reloq_german_nucleus_phrase_final(c, result.phrase_ticks,
                                            &result.phrase_ticks) != 1) return 0;
    }
    if (reloq_german_nucleus_adjust(c, result.phrase_ticks,
                                   &result.adjusted_ticks) != 1) return 0;
    *out = result;
    return 1;
}
