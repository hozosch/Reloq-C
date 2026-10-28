#include "reloq_german_transition_duration.h"

enum vowel_trait {
    FRONT = 1u << 0, BACK = 1u << 1, HIGH = 1u << 2,
    LOW = 1u << 3, ROUND = 1u << 4, CLOSED_LONG = 1u << 5,
    SCHWA = 1u << 6, OPEN_LONG_E = 1u << 7, DIPHTHONG = 1u << 8,
    VOWEL = 1u << 9
};

static unsigned traits(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_Q_a_TILDE:
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_a:
        return VOWEL | LOW;
    case RELOQ_DE_PHONE_Q_E_TILDE:
    case RELOQ_DE_PHONE_E: return VOWEL | FRONT;
    case RELOQ_DE_PHONE_e: return VOWEL | FRONT | CLOSED_LONG;
    case RELOQ_DE_PHONE_I: return VOWEL | FRONT | HIGH;
    case RELOQ_DE_PHONE_i: return VOWEL | FRONT | HIGH | CLOSED_LONG;
    case RELOQ_DE_PHONE_Q_o_TILDE: case RELOQ_DE_PHONE_Q_oe_TILDE:
    case RELOQ_DE_PHONE_O: return VOWEL | BACK | ROUND;
    case RELOQ_DE_PHONE_o: return VOWEL | BACK | ROUND | CLOSED_LONG;
    case RELOQ_DE_PHONE_U: return VOWEL | BACK | ROUND | HIGH;
    case RELOQ_DE_PHONE_u: return VOWEL | BACK | ROUND | HIGH | CLOSED_LONG;
    case RELOQ_DE_PHONE_Y: return VOWEL | FRONT | ROUND | HIGH;
    case RELOQ_DE_PHONE_y: return VOWEL | FRONT | ROUND | HIGH | CLOSED_LONG;
    case RELOQ_DE_PHONE_AT: return VOWEL | SCHWA;
    case RELOQ_DE_PHONE_R: return VOWEL;
    case RELOQ_DE_PHONE_Q_E_COLON: return VOWEL | FRONT | OPEN_LONG_E;
    case RELOQ_DE_PHONE_Q_OE: return VOWEL | FRONT | ROUND;
    case RELOQ_DE_PHONE_Q_oe: return VOWEL | FRONT | ROUND | CLOSED_LONG;
    case RELOQ_DE_PHONE_Q_aj: case RELOQ_DE_PHONE_Q_aw:
        return VOWEL | LOW | DIPHTHONG;
    case RELOQ_DE_PHONE_Q_oj: return VOWEL | BACK | ROUND | DIPHTHONG;
    default: return 0;
    }
}

int reloq_german_transition_cc_ticks(
    const struct reloq_german_phone_node *left,
    const struct reloq_german_phone_node *right,int32_t *out)
{
    if (!left || !right || !out) return -1;
    const enum reloq_german_phone a=left->phone,b=right->phone;
    if (traits(a) || traits(b) || a==RELOQ_DE_PHONE_NONE ||
        b==RELOQ_DE_PHONE_NONE || b==RELOQ_DE_PHONE_h ||
        a>=RELOQ_DE_PHONE_Q_a_TILDE || b>=RELOQ_DE_PHONE_Q_a_TILDE ||
        left->affricate_role || right->affricate_role) return 0;
    int32_t t=1;
    const int nasal=b==RELOQ_DE_PHONE_m || b==RELOQ_DE_PHONE_n || b==RELOQ_DE_PHONE_G;
    if (b==RELOQ_DE_PHONE_w) {
        /* Closing r/l already share the vowel's scheduled contact. */
        if ((a==RELOQ_DE_PHONE_r || a==RELOQ_DE_PHONE_l) &&
            left->same_syllable_left) return 0;
        switch (a) {
        case RELOQ_DE_PHONE_m: *out=15;return 1;
        case RELOQ_DE_PHONE_n: case RELOQ_DE_PHONE_G:
        case RELOQ_DE_PHONE_l: *out=20;return 1;
        case RELOQ_DE_PHONE_r: *out=45;return 1;
        case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_v:
        case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_z:
        case RELOQ_DE_PHONE_S: case RELOQ_DE_PHONE_Z:
        case RELOQ_DE_PHONE_x: case RELOQ_DE_PHONE_X: *out=30;return 1;
        default:break;
        }
    }
    switch (a) {
    case RELOQ_DE_PHONE_m:
        if (b==RELOQ_DE_PHONE_f || b==RELOQ_DE_PHONE_l || b==RELOQ_DE_PHONE_j) t=15;
        break;
    case RELOQ_DE_PHONE_n:
        if (b==RELOQ_DE_PHONE_f || b==RELOQ_DE_PHONE_j) t=30;
        else if (b==RELOQ_DE_PHONE_l) t=4;
        break;
    case RELOQ_DE_PHONE_G:
        if (b==RELOQ_DE_PHONE_t || b==RELOQ_DE_PHONE_d || b==RELOQ_DE_PHONE_f ||
            b==RELOQ_DE_PHONE_s || b==RELOQ_DE_PHONE_z || b==RELOQ_DE_PHONE_n ||
            b==RELOQ_DE_PHONE_l || b==RELOQ_DE_PHONE_j) t=30;
        else if (b==RELOQ_DE_PHONE_v) t=4;
        break;
    case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_v:
    case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_z:
    case RELOQ_DE_PHONE_S: case RELOQ_DE_PHONE_Z:
    case RELOQ_DE_PHONE_x: case RELOQ_DE_PHONE_X:
        if (nasal) {
            if (a==RELOQ_DE_PHONE_S) t=16;
            else if (!left->same_syllable_right)
                t=(a==RELOQ_DE_PHONE_s || a==RELOQ_DE_PHONE_z) && b==RELOQ_DE_PHONE_n?25:45;
        } else if (b==RELOQ_DE_PHONE_l) {
            if (a==RELOQ_DE_PHONE_S || a==RELOQ_DE_PHONE_Z ||
                a==RELOQ_DE_PHONE_x || a==RELOQ_DE_PHONE_X ||
                (a==RELOQ_DE_PHONE_f && left->same_syllable_right)) t=30;
        } else if (b==RELOQ_DE_PHONE_r && left->same_syllable_right) {
            if (a==RELOQ_DE_PHONE_f) t=35;
            else if (a!=RELOQ_DE_PHONE_v && a!=RELOQ_DE_PHONE_z) t=60;
        } else if (b==RELOQ_DE_PHONE_j && a!=RELOQ_DE_PHONE_X) t=30;
        else if (b==RELOQ_DE_PHONE_S &&
                 (a==RELOQ_DE_PHONE_s || a==RELOQ_DE_PHONE_z || a==RELOQ_DE_PHONE_S)) t=25;
        break;
    default:return 0;
    }
    *out=t;return 1;
}

int reloq_german_transition_vv_ticks(
    const struct reloq_german_phone_node *left,
    const struct reloq_german_phone_node *right,int32_t *out)
{
    if (!left || !right || !out) return -1;
    /* A combined SPR diphthong enters through A/O and leaves through
     * j/w. The incoming vowel transition belongs to its nucleus budget
     * even when an unstressed syllable has no glottal gesture. */
    struct reloq_german_phone_node entry=*right;
    if(entry.phone==RELOQ_DE_PHONE_Q_aj || entry.phone==RELOQ_DE_PHONE_Q_aw)
        entry.phone=RELOQ_DE_PHONE_A;
    else if(entry.phone==RELOQ_DE_PHONE_Q_oj)entry.phone=RELOQ_DE_PHONE_O;
    right=&entry;
    const unsigned a=traits(left->phone),b=traits(right->phone);
    if (right->same_syllable_left ||
        (!a && left->phone!=RELOQ_DE_PHONE_R) ||
        (!b && right->phone!=RELOQ_DE_PHONE_R)) return 0;
    int32_t t=65;
    if (left->phone==RELOQ_DE_PHONE_Q_aj || left->phone==RELOQ_DE_PHONE_Q_oj)
        t=right->phone==RELOQ_DE_PHONE_R?35:50;
    else if (left->phone==RELOQ_DE_PHONE_Q_aw)
        t=right->phone==RELOQ_DE_PHONE_e?60:40;
    else if (right->phone==RELOQ_DE_PHONE_AT && right->same_word_left) t=55;
    else if (right->phone==RELOQ_DE_PHONE_R)
        t=left->phone==RELOQ_DE_PHONE_I?54:
          left->phone==RELOQ_DE_PHONE_O || left->phone==RELOQ_DE_PHONE_U?25:40;
    else if ((a&FRONT) && !right->stress)
        t=b&LOW?50:(b&(FRONT|HIGH|SCHWA))?30:65;
    *out=t;return 1;
}

int reloq_german_transition_cv_ticks(
    const struct reloq_german_phone_node *node, int32_t *out)
{
    if (!node || !out) return -1;
    const unsigned v = traits(node->right_phone);
    if (!node->same_word_right || !node->same_syllable_right ||
        !v || node->stress > 2) return 0;
    const int stressed = node->stress != 0;
    const enum reloq_german_phone next = node->right_phone;
    int32_t ticks;
    switch (node->phone) {
    case RELOQ_DE_PHONE_b: ticks = v & (FRONT | SCHWA) ? 25 : 15; break;
    case RELOQ_DE_PHONE_d:
        ticks = v & FRONT ? stressed ? 25 : 15 : v & SCHWA ? 35 : 45;
        break;
    case RELOQ_DE_PHONE_g:
        ticks = v & (LOW | SCHWA) ? 35 : stressed ? 25 : 15;
        break;
    case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_v: ticks = 25; break;
    case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_z:
        ticks = next == RELOQ_DE_PHONE_i ? stressed ? 70 : 25 :
            (v & BACK) && !(v & HIGH) ? 40 : 30;
        break;
    case RELOQ_DE_PHONE_S:
        ticks = (v & BACK) && (v & (HIGH | CLOSED_LONG)) ? 40 : 30;
        break;
    case RELOQ_DE_PHONE_Z: ticks = 40; break;
    case RELOQ_DE_PHONE_x: case RELOQ_DE_PHONE_X: ticks = 20; break;
    case RELOQ_DE_PHONE_w: ticks = next == RELOQ_DE_PHONE_e ? 60 : 40; break;
    case RELOQ_DE_PHONE_j: ticks = 50; break;
    case RELOQ_DE_PHONE_m: ticks = 15; break;
    case RELOQ_DE_PHONE_n:
        ticks = next>=RELOQ_DE_PHONE_Q_a_TILDE?30:v & SCHWA ? 10 : v & FRONT ? 30 :
            next == RELOQ_DE_PHONE_O || next == RELOQ_DE_PHONE_Q_oj ? 50 :
            (v & BACK) || next == RELOQ_DE_PHONE_R ? 40 : 25;
        break;
    case RELOQ_DE_PHONE_G: ticks = v & SCHWA ? 10 : 30; break;
    case RELOQ_DE_PHONE_l:
        if (!node->same_word_left)
            ticks = next == RELOQ_DE_PHONE_i ? 10 : v & FRONT ? 30 : 20;
        else
            ticks = next == RELOQ_DE_PHONE_i ? 20 :
                v & (FRONT | SCHWA) ? 10 :
                (v & BACK) && (v & CLOSED_LONG) ? 45 : 30;
        break;
    case RELOQ_DE_PHONE_r:
        if (node->same_syllable_left &&
            (node->left_phone==RELOQ_DE_PHONE_p || node->left_phone==RELOQ_DE_PHONE_t ||
             node->left_phone==RELOQ_DE_PHONE_k || node->left_phone==RELOQ_DE_PHONE_b ||
             node->left_phone==RELOQ_DE_PHONE_d || node->left_phone==RELOQ_DE_PHONE_g ||
             node->left_phone==RELOQ_DE_PHONE_f || node->left_phone==RELOQ_DE_PHONE_s ||
             node->left_phone==RELOQ_DE_PHONE_S)) {
            ticks=next==RELOQ_DE_PHONE_e || next==RELOQ_DE_PHONE_i?35:
                (v&(FRONT|SCHWA)) || next==RELOQ_DE_PHONE_R?25:10;
            break;
        }
        ticks = v & SCHWA || next == RELOQ_DE_PHONE_R ||
            next == RELOQ_DE_PHONE_O || next == RELOQ_DE_PHONE_Q_oj ? 25 :
            next == RELOQ_DE_PHONE_e || next == RELOQ_DE_PHONE_i ? 35 :
            v & FRONT ? 60 : 45;
        break;
    default: return 0;
    }
    *out = ticks;
    return 1;
}

int reloq_german_transition_vc_ticks(
    const struct reloq_german_phone_node *node,
    const struct reloq_german_phone_node *right, int32_t *out)
{
    if (!node || !right || !out) return -1;
    const int short_nucleus = node->stress == 2 ||
        (!node->stress && (right->same_word_right || node->syllable_in_word));
    enum reloq_german_phone source = node->phone;
    /* Secondary a and open long E are shortened before transitions. The
     * public annotation remains intact; only the interval's vowel class
     * follows that independently described normalization. */
    if (short_nucleus) {
        if (source == RELOQ_DE_PHONE_a) source = RELOQ_DE_PHONE_A;
        if (source == RELOQ_DE_PHONE_Q_E_COLON) source = RELOQ_DE_PHONE_E;
    }
    const unsigned v = traits(source);
    if (!node->same_word_right || !node->same_syllable_right ||
        !v || v & DIPHTHONG || node->stress > 2 ||
        node->right_phone != right->phone) return 0;
    int32_t ticks;
    /* Schwa-to-closure timing is 15 inside the word and 20 across a
     * word boundary. Expanded affricates use their closure here too;
     * looking only for a vowel immediately after the stop missed them. */
    const int closure=right->phone==RELOQ_DE_PHONE_p ||
        right->phone==RELOQ_DE_PHONE_b || right->phone==RELOQ_DE_PHONE_t ||
        right->phone==RELOQ_DE_PHONE_d || right->phone==RELOQ_DE_PHONE_k ||
        right->phone==RELOQ_DE_PHONE_g;
    const int affricate=right->affricate_role>=1 && right->affricate_role<=7 &&
        (right->affricate_role&1);
    if((v&SCHWA) && closure && !right->same_syllable_left &&
       right->same_syllable_right && right->same_word_right &&
       (traits(right->right_phone) || affricate)) {
        *out=right->same_word_left?15:20;return 1;
    }
    if(right->same_syllable_left && right->same_syllable_right &&
       right->right_phone==RELOQ_DE_PHONE_n && closure) {
        if(right->phone==RELOQ_DE_PHONE_b) {*out=v&FRONT?25:20;return 1;}
        if(right->phone==RELOQ_DE_PHONE_d) {
            *out=v&SCHWA?20:v&FRONT?25:
                source==RELOQ_DE_PHONE_a||source==RELOQ_DE_PHONE_O?50:40;return 1;
        }
        if(right->phone==RELOQ_DE_PHONE_g) {
            *out=v&SCHWA?20:v&FRONT?25:v&LOW?45:40;return 1;
        }
    }
    /* A voiced stop beginning the next syllable retains a vowel-to-
     * closure transition, separate from its closure and noise release. */
    if (!right->same_syllable_left && right->same_syllable_right &&
        right->same_word_right && traits(right->right_phone)) {
        if (right->phone==RELOQ_DE_PHONE_k) {
            /* Syllable-initial k owns a short entering contact, also
             * before v/r/l/n. A coda's 40 ticks would consume the body of
             * short O/U and obscure their stable vowel colour. */
            *out=v&SCHWA?15:v&FRONT?25:20;return 1;
        }
        if (right->phone==RELOQ_DE_PHONE_b) {
            *out=v&SCHWA?15:v&FRONT?25:20;return 1;
        }
        if (right->phone==RELOQ_DE_PHONE_d) {
            *out=v&SCHWA?15:v&FRONT?25:
                source==RELOQ_DE_PHONE_a || source==RELOQ_DE_PHONE_O?50:40;return 1;
        }
        if (right->phone==RELOQ_DE_PHONE_g) {
            *out=v&SCHWA?15:v&FRONT?25:
                (v&BACK) && (v&CLOSED_LONG)?20:45;return 1;
        }
        if (right->phone==RELOQ_DE_PHONE_z) {
            /* A voiced s onset keeps a longer back/central-vowel
             * contact than the voiceless dental path. */
            *out=v&SCHWA?20:v&FRONT?25:50;return 1;
        }
        if (right->phone==RELOQ_DE_PHONE_r) {
            *out=(v&(CLOSED_LONG|OPEN_LONG_E)) ||
                (source==RELOQ_DE_PHONE_a && !short_nucleus)?40:20;
            return 1;
        }
        if (right->phone==RELOQ_DE_PHONE_l) {
            *out=source==RELOQ_DE_PHONE_O ||
                (source==RELOQ_DE_PHONE_a && !short_nucleus)?50:40;
            return 1;
        }
    }
    switch (right->phone) {
    case RELOQ_DE_PHONE_w:
        if (right->same_syllable_left) return 0;
        ticks=right->same_word_left?75:60;break;
    case RELOQ_DE_PHONE_j:
        /* A syllabic onset j has its own long entering glide. Postvocalic
         * diphthong members are scheduled by the nucleus carrier instead. */
        if (right->same_syllable_left) return 0;
        ticks=75;break;
    case RELOQ_DE_PHONE_p: case RELOQ_DE_PHONE_b: case RELOQ_DE_PHONE_P:
        ticks = v & SCHWA ? 20 : v & FRONT ? 25 : 30;
        break;
    case RELOQ_DE_PHONE_t: case RELOQ_DE_PHONE_d:
    case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_z:
    case RELOQ_DE_PHONE_C: case RELOQ_DE_PHONE_T: case RELOQ_DE_PHONE_J:
        ticks = v & SCHWA ? 20 : v & FRONT ? 25 :
            node->phone == RELOQ_DE_PHONE_O ||
            (source == RELOQ_DE_PHONE_a && !short_nucleus) ? 50 : 40;
        break;
    case RELOQ_DE_PHONE_k: case RELOQ_DE_PHONE_g:
        ticks = v & SCHWA ? 20 : v & FRONT ? 25 :
            source == RELOQ_DE_PHONE_a && !short_nucleus ? 20 : 40;
        break;
    case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_v:
        ticks = v & SCHWA ? 20 :
            (v & BACK) && (v & CLOSED_LONG) ? 30 : 25;
        break;
    case RELOQ_DE_PHONE_S: case RELOQ_DE_PHONE_Z: case RELOQ_DE_PHONE_X:
        ticks = v & SCHWA ? 20 : v & FRONT ? 25 : 30;
        break;
    case RELOQ_DE_PHONE_x:
        ticks = v & SCHWA ? 20 : v & FRONT ? 25 : v & BACK ? 40 : 30;
        break;
    case RELOQ_DE_PHONE_m: ticks = v & SCHWA ? 15 : v & FRONT ? 20 : 30; break;
    case RELOQ_DE_PHONE_n: case RELOQ_DE_PHONE_G:
        ticks = v & SCHWA ? 20 : 40;
        break;
    case RELOQ_DE_PHONE_l:
        ticks = v & SCHWA ? 10 :
            (v & FRONT) && !(v & ROUND) &&
            (!(v & OPEN_LONG_E) || short_nucleus) ? 30 : 60;
        break;
    case RELOQ_DE_PHONE_r: case RELOQ_DE_PHONE_R:
        if (v & SCHWA) return 0;
        ticks = node->phone == RELOQ_DE_PHONE_I ? 54 :
            node->phone == RELOQ_DE_PHONE_O || node->phone == RELOQ_DE_PHONE_U ? 25 :
            node->phone == RELOQ_DE_PHONE_e && short_nucleus ? 24 : 40;
        break;
    default: return 0;
    }
    *out = ticks;
    return 1;
}
