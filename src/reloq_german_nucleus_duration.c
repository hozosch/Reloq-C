#include "reloq_german_nucleus_duration.h"

#include <limits.h>
#include <math.h>

int reloq_german_nucleus_shortened(const struct reloq_german_phone_node *n)
{
    /* The text phonetic adjustment shortens pretonic secondary vowels,
     * except before a long primary nucleus with a consonant-cluster coda.
     * See behavior-0.2.60/pretonic observations, including extended words.
     * Public SPR incorrectly also shortens posttonic secondary vowels;
     * both input modes deliberately share the text behavior here. */
    if (!n || n->stress==1 ||
        (n->stress==2&&(!n->primary_ahead||n->primary_long_closed)) || n->same_syllable_right ||
        (!n->syllable_in_word && !n->same_word_right)) return 0;
    return n->phone==RELOQ_DE_PHONE_e || n->phone==RELOQ_DE_PHONE_i ||
        n->phone==RELOQ_DE_PHONE_o || n->phone==RELOQ_DE_PHONE_u ||
        n->phone==RELOQ_DE_PHONE_y || n->phone==RELOQ_DE_PHONE_Q_oe;
}

static int single_vowel(enum reloq_german_phone p)
{
    switch (p) {
    case RELOQ_DE_PHONE_R:
    case RELOQ_DE_PHONE_a: case RELOQ_DE_PHONE_A:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_i: case RELOQ_DE_PHONE_I:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_u: case RELOQ_DE_PHONE_U:
    case RELOQ_DE_PHONE_y: case RELOQ_DE_PHONE_Y:
    case RELOQ_DE_PHONE_AT: case RELOQ_DE_PHONE_Q_E_COLON:
    case RELOQ_DE_PHONE_Q_oe: case RELOQ_DE_PHONE_Q_OE:
    case RELOQ_DE_PHONE_Q_a_TILDE: case RELOQ_DE_PHONE_Q_E_TILDE:
    case RELOQ_DE_PHONE_Q_o_TILDE: case RELOQ_DE_PHONE_Q_oe_TILDE:
        return 1;
    default: return 0;
    }
}

static int diphthong(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_Q_aj || p == RELOQ_DE_PHONE_Q_aw ||
           p == RELOQ_DE_PHONE_Q_oj;
}

int reloq_german_nucleus_find(const struct reloq_german_phone_node *phones,
    size_t count, size_t *first, size_t *length)
{
    if ((!phones && count) || !first || !length || first == length) return -1;
    for (size_t i = 0; i < count; ++i)
        if (phones[i].phone <= RELOQ_DE_PHONE_NONE ||
            phones[i].phone > RELOQ_DE_PHONE_Q_oe_TILDE ||
            phones[i].syllable_index != phones[0].syllable_index ||
            phones[i].word_index != phones[0].word_index) return -1;
    size_t start = 0;
    while (start < count && !single_vowel(phones[start].phone) &&
           !diphthong(phones[start].phone)) ++start;
    if (start == count) return 0;
    size_t end = start + 1;
    if (!diphthong(phones[start].phone) && end < count &&
        (phones[end].phone == RELOQ_DE_PHONE_r ||
         phones[end].phone == RELOQ_DE_PHONE_j ||
         phones[end].phone == RELOQ_DE_PHONE_w)) ++end;
    if (end < count && phones[end].phone == RELOQ_DE_PHONE_l) ++end;
    *first = start;
    *length = end - start;
    return 1;
}

int reloq_german_nucleus_duration_base(enum reloq_german_phone head,
    enum reloq_german_phone closing, enum reloq_german_phone third, int shortened,
    struct reloq_german_nucleus_duration *out)
{
    if (!out || (shortened != 0 && shortened != 1) ||
        head <= RELOQ_DE_PHONE_NONE || head > RELOQ_DE_PHONE_Q_oe_TILDE ||
        closing < RELOQ_DE_PHONE_NONE || closing > RELOQ_DE_PHONE_Q_oe_TILDE ||
        third < RELOQ_DE_PHONE_NONE || third > RELOQ_DE_PHONE_Q_oe_TILDE)
        return -1;
    if (!single_vowel(head) && !diphthong(head)) return 0;
    struct reloq_german_nucleus_duration d = {0,0,0};
    if (third != RELOQ_DE_PHONE_NONE) {
        if (third != RELOQ_DE_PHONE_l || diphthong(head) ||
            (closing != RELOQ_DE_PHONE_r && closing != RELOQ_DE_PHONE_j &&
             closing != RELOQ_DE_PHONE_w)) return 0;
        d = (struct reloq_german_nucleus_duration){190,40,20};
    } else if (diphthong(head)) {
        if (closing != RELOQ_DE_PHONE_NONE && closing != RELOQ_DE_PHONE_l)
            return 0;
        d = closing == RELOQ_DE_PHONE_l ?
            (struct reloq_german_nucleus_duration){190,40,20} :
            (struct reloq_german_nucleus_duration){145,20,0};
    } else if (closing == RELOQ_DE_PHONE_r) {
        d.base_ticks = head == RELOQ_DE_PHONE_AT ? 100 :
            head == RELOQ_DE_PHONE_U || head == RELOQ_DE_PHONE_O ? 115 :
            head == RELOQ_DE_PHONE_Y || head == RELOQ_DE_PHONE_Q_OE ? 130 :
            head == RELOQ_DE_PHONE_E ? 145 :
            head == RELOQ_DE_PHONE_A ? 140 :
            head == RELOQ_DE_PHONE_i ? 175 : 155;
        d.closing_ticks = 25;
    } else if (closing == RELOQ_DE_PHONE_l) {
        d.base_ticks = head == RELOQ_DE_PHONE_AT ? 110 :
            head == RELOQ_DE_PHONE_A ? 145 :
            shortened ? (head == RELOQ_DE_PHONE_Q_a_TILDE ||
                         head == RELOQ_DE_PHONE_Q_E_TILDE ? 140 : 150) :
            head == RELOQ_DE_PHONE_E || head == RELOQ_DE_PHONE_I ||
            head == RELOQ_DE_PHONE_O || head == RELOQ_DE_PHONE_U ||
            head == RELOQ_DE_PHONE_Y || head == RELOQ_DE_PHONE_Q_OE ||
            head == RELOQ_DE_PHONE_R ? 140 :
            head == RELOQ_DE_PHONE_i || head == RELOQ_DE_PHONE_u ||
            head == RELOQ_DE_PHONE_y ? 175 : 185;
        /* Long open a has a separate longer VL nucleus. */
        if (head == RELOQ_DE_PHONE_a && !shortened) d.base_ticks = 200;
        d.closing_ticks = 40;
    } else if (closing == RELOQ_DE_PHONE_j || closing == RELOQ_DE_PHONE_w) {
        /* A closing glide forms one timed nucleus with every vowel,
         * including E+j in loanwords. It is not restricted to aj/oj/aw. */
        d = (struct reloq_german_nucleus_duration){145,20,0};
    } else if (closing != RELOQ_DE_PHONE_NONE) {
        return 0;
    } else {
        switch (head) {
        case RELOQ_DE_PHONE_Q_a_TILDE: case RELOQ_DE_PHONE_Q_E_TILDE:
        case RELOQ_DE_PHONE_Q_o_TILDE: case RELOQ_DE_PHONE_Q_oe_TILDE:
            d.base_ticks = 150; break;
        case RELOQ_DE_PHONE_AT: case RELOQ_DE_PHONE_U:
            d.base_ticks = 60; break;
        case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
        case RELOQ_DE_PHONE_Y: case RELOQ_DE_PHONE_R:
            d.base_ticks = 70; break;
        case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E: case RELOQ_DE_PHONE_Q_OE:
            d.base_ticks = 75; break;
        case RELOQ_DE_PHONE_i: case RELOQ_DE_PHONE_u:
            d.base_ticks = shortened ? 80 : 120; break;
        case RELOQ_DE_PHONE_y: d.base_ticks = shortened ? 90 : 120; break;
        case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_Q_oe:
            d.base_ticks = shortened ? 90 : 130; break;
        case RELOQ_DE_PHONE_Q_E_COLON: d.base_ticks = 120; break;
        case RELOQ_DE_PHONE_a: d.base_ticks = 140; break;
        default: return 0;
        }
    }
    *out = d;
    return 1;
}

int reloq_german_nucleus_distribute(
    const struct reloq_nucleus_interval *fixed, size_t count,
    double requested_ticks, int32_t vowel_minimum,
    int32_t *fixed_out, int32_t *vowel_out)
{
    if ((count && (!fixed || !fixed_out)) || !vowel_out || count > INT16_MAX ||
        !isfinite(requested_ticks) || requested_ticks < 0 ||
        requested_ticks > INT16_MAX || vowel_minimum < 0 ||
        vowel_minimum > INT16_MAX) return -1;
    int32_t total = 0;
    for (size_t i = 0; i < count; ++i) {
        if (fixed[i].ticks < 0 || fixed[i].ticks > INT16_MAX ||
            fixed[i].minimum < 0 || fixed[i].minimum > INT16_MAX ||
            total > INT16_MAX - fixed[i].ticks) return -1;
        total += fixed[i].ticks;
    }
    double first_factor = 1.0, second_factor = 1.0;
    int32_t free_ticks;
    if (requested_ticks < total) {
        const double percent = requested_ticks * 100.0 / total;
        const int32_t target = (int32_t)((double)total * percent / 100.0);
        first_factor = (double)target / total;
        int32_t rounded_total = 0;
        for (size_t i = 0; i < count; ++i)
            rounded_total += (int32_t)(fixed[i].ticks * first_factor + 0.5);
        /* The normal synthesis domain is nonnegative. Reserve at most the
         * available total; final interval minima are applied afterwards. */
        int32_t remainder = rounded_total - vowel_minimum;
        if (remainder < 0) remainder = 0;
        second_factor = rounded_total ? (double)remainder / rounded_total : 0.0;
        free_ticks = vowel_minimum;
    } else {
        free_ticks = (int32_t)(requested_ticks - total);
        if (free_ticks < vowel_minimum) free_ticks = vowel_minimum;
    }
    for (size_t i = 0; i < count; ++i) {
        int32_t value = (int32_t)(fixed[i].ticks * first_factor + 0.5);
        value = (int32_t)(value * second_factor + 0.5);
        fixed_out[i] = value < fixed[i].minimum ? fixed[i].minimum : value;
    }
    *vowel_out = free_ticks;
    return 0;
}
