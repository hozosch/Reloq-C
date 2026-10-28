#include "reloq_german_generic_timing.h"

enum vowel_context {
    RIGHT_NONE = 0,
    RIGHT_SHORT,
    RIGHT_FRONT_SHORT,
    RIGHT_HIGH_BACK_SHORT,
    RIGHT_LONG,
    RIGHT_FRONT_LONG,
    RIGHT_HIGH_BACK_LONG
};

static enum vowel_context right_vowel(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_AT: case RELOQ_DE_PHONE_R:
    case RELOQ_DE_PHONE_Q_aj: case RELOQ_DE_PHONE_Q_aw: case RELOQ_DE_PHONE_Q_oj:
    case RELOQ_DE_PHONE_a: return RIGHT_SHORT;
    case RELOQ_DE_PHONE_E: case RELOQ_DE_PHONE_I:
    case RELOQ_DE_PHONE_Q_OE: case RELOQ_DE_PHONE_Q_E_COLON:
    case RELOQ_DE_PHONE_Y: return RIGHT_FRONT_SHORT;
    case RELOQ_DE_PHONE_U: return RIGHT_HIGH_BACK_SHORT;
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
    case RELOQ_DE_PHONE_Q_oe:
    case RELOQ_DE_PHONE_y: return RIGHT_FRONT_LONG;
    case RELOQ_DE_PHONE_o: return RIGHT_LONG;
    case RELOQ_DE_PHONE_u: return RIGHT_HIGH_BACK_LONG;
    default: return RIGHT_NONE;
    }
}

static int32_t pre_stop_vowel(enum reloq_german_phone phone,
                              unsigned stress)
{
    /* Short and long phone classes have distinct unstressed compression.
     * Values are nominal 1-ms articulatory intervals, not held audio or
     * word-specific curves. Other phones keep their existing model. */
    switch (phone) {
    case RELOQ_DE_PHONE_A: return stress ? 81 : 74;
    case RELOQ_DE_PHONE_E: return stress ? 89 : 74;
    case RELOQ_DE_PHONE_Q_OE: return stress ? 89 : 74;
    case RELOQ_DE_PHONE_R: return stress ? 83 : 69;
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_Y: return stress ? 83 : 69;
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_AT:
        return stress ? 71 : 59;
    case RELOQ_DE_PHONE_a: return stress ? 167 : 74;
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_o:
    case RELOQ_DE_PHONE_Q_oe:
        return stress ? 142 : 89;
    case RELOQ_DE_PHONE_Q_E_COLON: return stress ? 143 : 74;
    case RELOQ_DE_PHONE_i: case RELOQ_DE_PHONE_u:
        return stress ? 131 : 79;
    case RELOQ_DE_PHONE_y: return stress ? 131 : 89;
    default: return -1;
    }
}

int reloq_german_generic_pre_stop_vowel_ticks(
    const struct reloq_german_phone_node *node, int32_t *out)
{
    if (!node || !out) return -1;
    if (!node->same_word_right || node->same_syllable_right ||
        (node->right_phone != RELOQ_DE_PHONE_p &&
         node->right_phone != RELOQ_DE_PHONE_t &&
         node->right_phone != RELOQ_DE_PHONE_k) ||
        node->stress > 1) return 0;
    const int32_t ticks = pre_stop_vowel(node->phone, node->stress);
    if (ticks < 0) return 0;
    *out = ticks;
    return 1;
}

int reloq_german_generic_stop_onset_timing(
    const struct reloq_german_phone_node *node,
    struct reloq_german_generic_stop_timing *out)
{
    if (!node || !out) return -1;
    if ((node->phone != RELOQ_DE_PHONE_p &&
         node->phone != RELOQ_DE_PHONE_t &&
         node->phone != RELOQ_DE_PHONE_k) ||
        !node->same_syllable_right || node->stress > 1) return 0;
    enum vowel_context right = right_vowel(node->right_phone);
    if (right == RIGHT_NONE) return 0;
    /* Consonant clusters and codas have a separate closure geometry. */
    const int internal = node->same_word_left;
    if (internal && (node->same_syllable_left ||
        pre_stop_vowel(node->left_phone, 1) < 0)) return 0;
    const int primary = node->stress == 1;
    const int front = right == RIGHT_FRONT_SHORT ||
                      right == RIGHT_FRONT_LONG;
    const int long_vowel = right == RIGHT_LONG ||
                           right == RIGHT_FRONT_LONG ||
                           right == RIGHT_HIGH_BACK_LONG;
    int32_t closure, burst, aspiration;
    if (!internal) {
        closure = 5;
        if (node->phone == RELOQ_DE_PHONE_p) {
            burst = 10;
            if (primary) aspiration = right == RIGHT_HIGH_BACK_SHORT ? 35 :
                right == RIGHT_HIGH_BACK_LONG ? 44 : long_vowel ? 63 : 50;
            else aspiration = long_vowel ? 25 : 19;
        } else if (node->phone == RELOQ_DE_PHONE_t) {
            burst = front ? 18 : 12;
            if (primary) aspiration = right == RIGHT_HIGH_BACK_SHORT ? 50 :
                right == RIGHT_HIGH_BACK_LONG ? 63 : long_vowel ? 90 : 72;
            else aspiration = long_vowel ? 36 : 28;
        } else {
            burst = 12;
            aspiration = primary ? (long_vowel ? 76 : 61) : 50;
        }
    } else if (node->phone == RELOQ_DE_PHONE_p) {
        closure = primary ||
            pre_stop_vowel(node->left_phone, 1) < 130 ? 90 : 75;
        burst = 10;
        aspiration = !primary ? 19 :
            right == RIGHT_HIGH_BACK_SHORT ? 35 :
            right == RIGHT_HIGH_BACK_LONG ? 44 : long_vowel ? 63 : 50;
    } else if (node->phone == RELOQ_DE_PHONE_t) {
        closure = node->right_phone==RELOQ_DE_PHONE_AT ||
            node->right_phone==RELOQ_DE_PHONE_R?34:57;
        burst = front ? 18 : 12;
        aspiration = !primary ? 28 :
            right == RIGHT_HIGH_BACK_SHORT ? 50 :
            right == RIGHT_HIGH_BACK_LONG ? 63 : long_vowel ? 90 : 72;
    } else {
        closure = primary ? 75 : node->left_phone==RELOQ_DE_PHONE_AT?60:70;
        burst = 12;
        aspiration = !primary ? 50 : long_vowel ? 76 : 61;
    }
    *out = (struct reloq_german_generic_stop_timing){
        closure + burst + aspiration, closure, closure + burst
    };
    return 1;
}

static int stop_place(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p ? 0 :
           phone == RELOQ_DE_PHONE_t ? 1 :
           phone == RELOQ_DE_PHONE_k ? 2 : -1;
}

int reloq_german_generic_internal_stop_pair(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t coda_index,
    struct reloq_german_generic_internal_stop_pair *out)
{
    if (!phones || !out) return -1;
    if (!coda_index || coda_index >= phone_count ||
        phone_count - coda_index < 3) return 0;
    const struct reloq_german_phone_node *first = phones + coda_index - 1;
    const struct reloq_german_phone_node *coda = phones + coda_index;
    const struct reloq_german_phone_node *onset = phones + coda_index + 1;
    const struct reloq_german_phone_node *second = phones + coda_index + 2;
    const int left = stop_place(coda->phone);
    const int right = stop_place(onset->phone);
    if (left < 0 || right < 0 ||
        (first->phone != RELOQ_DE_PHONE_A &&
         first->phone != RELOQ_DE_PHONE_E &&
         first->phone != RELOQ_DE_PHONE_U) ||
        (second->phone != RELOQ_DE_PHONE_A &&
         second->phone != RELOQ_DE_PHONE_I) ||
        first->word_index != coda->word_index ||
        coda->word_index != onset->word_index ||
        onset->word_index != second->word_index ||
        !coda->same_word_left || !coda->same_syllable_left ||
        !coda->same_word_right || coda->same_syllable_right ||
        !onset->same_word_left || onset->same_syllable_left ||
        !onset->same_syllable_right || !onset->same_word_right ||
        !second->same_syllable_left ||
        !first->stress_explicit || !coda->stress_explicit ||
        !onset->stress_explicit || !second->stress_explicit ||
        coda->stress > 1 || onset->stress > 1 ||
        first->stress != coda->stress ||
        onset->stress != second->stress ||
        (second->phone == RELOQ_DE_PHONE_A && onset->stress != 0))
        return 0;

    /* The C1 closure/release depends on both stop places. The second C2
     * burst, aspiration and vowel entrance depend on place, following vowel
     * and stress. Keep the whole V.CCV gesture separate from word-final VCC
     * and cross-word connected stop timing. */
    static const int32_t coda_ticks[3][3] = {
        {40, 100, 100}, {42, 30, 42}, {60, 60, 52}
    };
    const int primary = onset->stress == 1;
    const int front = second->phone == RELOQ_DE_PHONE_I;
    const int32_t coda_span = left == 2 && right == 2 && primary ?
        65 : coda_ticks[left][right];
    int32_t burst, aspiration, next_vowel;
    if (right == 0) {
        burst = primary ? 91 : 76;
        aspiration = burst + 10;
        next_vowel = primary ? 152 : 106;
    } else if (right == 1) {
        burst = left == 0 ? 53 : left == 1 ? 58 : 92;
        aspiration = burst + (front ? 18 : 12);
        next_vowel = aspiration + (primary ? left == 2 ? 44 : 73 : 29);
    } else {
        burst = left == 1 ? primary ? 105 : 100 :
                primary ? 56 : 71;
        aspiration = burst + 12;
        next_vowel = aspiration + (primary ? 62 : 51);
    }
    *out = (struct reloq_german_generic_internal_stop_pair){
        coda_span, coda_span - (left == 0 ? 10 : left == 1 ? 8 : 12),
        left == right,
        {next_vowel - 1, burst, aspiration}
    };
    return 1;
}

int reloq_german_generic_voiced_onset_timing(
    const struct reloq_german_phone_node *node,
    struct reloq_german_generic_voiced_onset_timing *out)
{
    if (!node || !out) return -1;
    const enum reloq_german_phone r=node->right_phone;
    const int is_vowel=(r>=RELOQ_DE_PHONE_a && r<=RELOQ_DE_PHONE_AT) ||
        r==RELOQ_DE_PHONE_R || r>=RELOQ_DE_PHONE_Q_E_COLON;
    const enum reloq_german_phone l=node->left_phone;
    const int left_vowel=(l>=RELOQ_DE_PHONE_a && l<=RELOQ_DE_PHONE_AT) ||
        l==RELOQ_DE_PHONE_R || l>=RELOQ_DE_PHONE_Q_E_COLON;
    if (!node->same_syllable_right || node->stress > 1 || !is_vowel ||
        (node->same_word_left && (node->same_syllable_left || !left_vowel)))
        return 0;
    const int internal=node->same_word_left;
    int32_t span, release;
    if (node->phone == RELOQ_DE_PHONE_b) {
        if (r==RELOQ_DE_PHONE_R) span=39;
        else if (!node->stress) span = 71;
        else {
            switch (node->right_phone) {
            case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_a:
            case RELOQ_DE_PHONE_Q_aj: case RELOQ_DE_PHONE_Q_aw:
            case RELOQ_DE_PHONE_Q_a_TILDE:
                span = 81; break;
            case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_U:
            case RELOQ_DE_PHONE_i: case RELOQ_DE_PHONE_u:
            case RELOQ_DE_PHONE_y: case RELOQ_DE_PHONE_Y:
                span = 106; break;
            case RELOQ_DE_PHONE_AT: span=69;break;
            default: span = 93; break;
            }
        }
        release=11;
    } else if (node->phone == RELOQ_DE_PHONE_d) {
        const int closed_front=r==RELOQ_DE_PHONE_e || r==RELOQ_DE_PHONE_i ||
            r==RELOQ_DE_PHONE_y || r==RELOQ_DE_PHONE_Q_oe;
        release=r==RELOQ_DE_PHONE_R?7:node->stress && closed_front?15:10;
        if (!internal && !node->stress) span=40+release;
        else if (r==RELOQ_DE_PHONE_AT || r==RELOQ_DE_PHONE_R) span=20+release;
        else span=(node->stress?90:45)+release;
    } else if (node->phone == RELOQ_DE_PHONE_g) {
        const int closed_long=r==RELOQ_DE_PHONE_e || r==RELOQ_DE_PHONE_i ||
            r==RELOQ_DE_PHONE_y || r==RELOQ_DE_PHONE_Q_oe ||
            r==RELOQ_DE_PHONE_o || r==RELOQ_DE_PHONE_u ||
            r==RELOQ_DE_PHONE_Q_o_TILDE || r==RELOQ_DE_PHONE_Q_oe_TILDE;
        release=node->stress?20:10;
        span=(!internal?70:!node->stress?50:closed_long?70:42)+release;
    } else return 0;
    *out = (struct reloq_german_generic_voiced_onset_timing){span, span-release};
    return 1;
}
