#include "reloq_german_nucleus_acoustics.h"

#include <stddef.h>

struct nucleus_base {
    enum reloq_german_phone phone;
    int16_t f[3];
    uint8_t end_mask;
    uint8_t av;
};

/* Phonetic observations, checked against the separate original. The table
 * is a compact inventory of scalar acoustic properties, not source code or
 * a recorded utterance. Neighbor-dependent gestures are formulated below. */
static const struct nucleus_base bases[] = {
    {RELOQ_DE_PHONE_a, {800, 1220, 2500}, 2, 50},
    {RELOQ_DE_PHONE_A, {700, 1160, 2500}, 0, 51},
    {RELOQ_DE_PHONE_e, {370, 2150, 2700}, 2, 51},
    {RELOQ_DE_PHONE_E, {550, 1800, 2600}, 0, 52},
    {RELOQ_DE_PHONE_i, {265, 2150, 2900}, 7, 53},
    {RELOQ_DE_PHONE_I, {380, 2000, 2520}, 0, 53},
    {RELOQ_DE_PHONE_o, {390, 735, 2400}, 6, 54},
    {RELOQ_DE_PHONE_O, {570, 900, 2500}, 0, 53},
    {RELOQ_DE_PHONE_u, {250, 750, 2400}, 2, 55},
    {RELOQ_DE_PHONE_U, {420, 930, 2400}, 0, 54},
    {RELOQ_DE_PHONE_y, {300, 1820, 2300}, 0, 57},
    {RELOQ_DE_PHONE_Y, {400, 1480, 2100}, 2, 57},
    {RELOQ_DE_PHONE_AT, {450, 1540, 2500}, 0, 53},
    {RELOQ_DE_PHONE_Q_E_COLON, {500, 2005, 2600}, 6, 51},
    {RELOQ_DE_PHONE_Q_oe, {380, 1480, 2010}, 6, 57},
    {RELOQ_DE_PHONE_Q_OE, {540, 1350, 2140}, 0, 53},
    {RELOQ_DE_PHONE_R, {600, 1360, 2280}, 2, 48},
    /* User-selected eloquence_64 nasal-midrange a~; the other nasal
     * vowel classes retain the independently observed original targets. */
    {RELOQ_DE_PHONE_Q_a_TILDE, {624, 960, 2880}, 0, 52},
    {RELOQ_DE_PHONE_Q_E_TILDE, {570, 1850, 2550}, 31, 57},
    {RELOQ_DE_PHONE_Q_o_TILDE, {450, 650, 2800}, 0, 55},
    {RELOQ_DE_PHONE_Q_oe_TILDE, {610, 1350, 2700}, 0, 57}
};

static int labial(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_p || p == RELOQ_DE_PHONE_b ||
           p == RELOQ_DE_PHONE_f || p == RELOQ_DE_PHONE_v ||
           p == RELOQ_DE_PHONE_m || p == RELOQ_DE_PHONE_w;
}

static int alveolar_stop(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_t || p == RELOQ_DE_PHONE_d;
}

static int velar_stop(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_k || p == RELOQ_DE_PHONE_g;
}

static int nasal(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_m || p == RELOQ_DE_PHONE_n ||
           p == RELOQ_DE_PHONE_G;
}

static void departure(struct reloq_german_nucleus_acoustics *p,
                      unsigned formant, int32_t value)
{
    p->departing[formant] = value;
    p->end_mask |= (uint8_t)(1u << formant);
}

static void clear_departure(struct reloq_german_nucleus_acoustics *p,
                            unsigned formant)
{
    p->end_mask &= (uint8_t)~(1u << formant);
}

int reloq_german_nucleus_acoustics_plan(
    enum reloq_german_phone phone, enum reloq_german_phone left,
    enum reloq_german_phone right, uint8_t stress, int weak_final,
    struct reloq_german_nucleus_acoustics *out)
{
    if (!out || phone < RELOQ_DE_PHONE_NONE ||
        phone > RELOQ_DE_PHONE_Q_oe_TILDE ||
        left < RELOQ_DE_PHONE_NONE || left > RELOQ_DE_PHONE_Q_oe_TILDE ||
        right < RELOQ_DE_PHONE_NONE || right > RELOQ_DE_PHONE_Q_oe_TILDE ||
        stress > 2 || (weak_final != 0 && weak_final != 1)) return -1;
    const struct nucleus_base *base = NULL;
    for (size_t i = 0; i < sizeof bases / sizeof bases[0]; ++i)
        if (bases[i].phone == phone) { base = bases + i; break; }
    if (!base) return 0;
    struct reloq_german_nucleus_acoustics p = {
        {base->f[0], base->f[1], base->f[2], 3600, 3900},
        {base->f[0], base->f[1], base->f[2], 3600, 3900},
        {120, 100, 150, 330, 260}, base->av, 0, base->end_mask, 200, 200
    };
    if (phone>=RELOQ_DE_PHONE_Q_a_TILDE) {
        switch (phone) {
        case RELOQ_DE_PHONE_Q_a_TILDE:
            p.bandwidth[0]=180;p.fnp=280;p.fnz=350;break;
        case RELOQ_DE_PHONE_Q_E_TILDE:
            p.entering[3]=p.departing[3]=3500;
            p.entering[4]=p.departing[4]=4000;
            p.fnp=250;p.fnz=350;break;
        case RELOQ_DE_PHONE_Q_o_TILDE:
            p.bandwidth[0]=p.bandwidth[1]=200;break;
        case RELOQ_DE_PHONE_Q_oe_TILDE:
            p.bandwidth[0]=200;p.bandwidth[2]=400;break;
        default:break;
        }
        p.av_end=p.av-2;
        if (weak_final) {p.av-=3;p.av_end-=3;}
        *out=p;return 1;
    }
    if (phone == RELOQ_DE_PHONE_I && stress == 1) {
        p.entering[2] = 2800;
        departure(&p, 2, 2800);
    }
    if (phone == RELOQ_DE_PHONE_Q_oe) departure(&p, 1, 1370);
    if (phone == RELOQ_DE_PHONE_R) departure(&p, 1, 1300);

    /* Entering and departing tongue gestures are separate. This avoids a
     * single whole-vowel correction erasing either consonant's influence. */
    switch (phone) {
    case RELOQ_DE_PHONE_a:
        if (labial(left)) p.entering[1] = 1100;
        else if (alveolar_stop(left) || left == RELOQ_DE_PHONE_z)
            p.entering[1] = 1250;
        else if (velar_stop(left)) p.entering[1] = 1280;
        if (right == RELOQ_DE_PHONE_n) departure(&p, 1, 1275);
        else if (right == RELOQ_DE_PHONE_j) {
            p.entering[0] = 820; clear_departure(&p, 1);
        } else if (right == RELOQ_DE_PHONE_w) {
            p.entering[0] = 780; p.entering[1] = 1160;
            clear_departure(&p, 1);
        }
        if (right == RELOQ_DE_PHONE_j || right == RELOQ_DE_PHONE_w) {
            if (labial(left)) p.entering[1] = 1150;
            else if (alveolar_stop(left)) p.entering[1] = 1300;
            else if (velar_stop(left)) {
                p.entering[1] = 1250; p.entering[2] = 2400;
            } else if (left == RELOQ_DE_PHONE_r) {
                p.entering[1] = 1200; p.entering[2] = 2300;
            } else if (left == RELOQ_DE_PHONE_z && right == RELOQ_DE_PHONE_j)
                p.entering[1] = 1220;
            if (right == RELOQ_DE_PHONE_j && left == RELOQ_DE_PHONE_v)
                p.entering[0] = 800;
        }
        break;
    case RELOQ_DE_PHONE_A:
        if (labial(left)) p.entering[1] = 1150;
        else if (alveolar_stop(left)) p.entering[1] = 1300;
        else if (velar_stop(left)) {
            p.entering[1] = 1250; p.entering[2] = 2400;
        } else if (left == RELOQ_DE_PHONE_r) {
            p.entering[1] = 1200; p.entering[2] = 2300;
        }
        if (right != RELOQ_DE_PHONE_NONE && right != RELOQ_DE_PHONE_h) {
            p.entering[0] = right == RELOQ_DE_PHONE_x ? 800 : 780;
            if (alveolar_stop(right) && !alveolar_stop(left) &&
                !velar_stop(left) && !labial(left) &&
                left != RELOQ_DE_PHONE_r) p.entering[1] = 1300;
        }
        if (right == RELOQ_DE_PHONE_j) {
            p.entering[0] = left == RELOQ_DE_PHONE_v ? 800 : 820;
            if (!labial(left) && !alveolar_stop(left) &&
                !velar_stop(left) && left != RELOQ_DE_PHONE_r)
                p.entering[1] = 1220;
        }
        break;
    case RELOQ_DE_PHONE_e:
        if (left == RELOQ_DE_PHONE_p || left == RELOQ_DE_PHONE_b ||
            left == RELOQ_DE_PHONE_r) p.entering[1] = 2000;
        if (right == RELOQ_DE_PHONE_r) {
            p.entering[0] = 400;
            if (labial(left)) {
                p.entering[1] = 2100; departure(&p, 1, 1960);
            } else if (left == RELOQ_DE_PHONE_h) {
                p.entering[1] = 2050; departure(&p, 1, 2050);
            }
        }
        break;
    case RELOQ_DE_PHONE_E:
        if (left == RELOQ_DE_PHONE_b) {
            p.entering[0] = 600; p.entering[1] = 1640;
        } else if (left == RELOQ_DE_PHONE_g) p.entering[1] = 1850;
        else if (left == RELOQ_DE_PHONE_z) p.entering[1] = 1700;
        if (left == RELOQ_DE_PHONE_f && right == RELOQ_DE_PHONE_r &&
            stress == 0) p.entering[1] = 1550;
        if (right == RELOQ_DE_PHONE_X) departure(&p, 2, 2610);
        break;
    case RELOQ_DE_PHONE_i:
        if (left == RELOQ_DE_PHONE_r) p.entering[1] = 2000;
        if (right == RELOQ_DE_PHONE_r) {
            departure(&p, 1, 1980); departure(&p, 2, 2680);
        } else if (right == RELOQ_DE_PHONE_l) departure(&p, 1, 2000);
        break;
    case RELOQ_DE_PHONE_I:
        if (velar_stop(left)) p.entering[0] = 350;
        else if (left == RELOQ_DE_PHONE_v) p.entering[1] = 1800;
        else if (left == RELOQ_DE_PHONE_l) p.entering[1] = 1900;
        else if (left == RELOQ_DE_PHONE_r) p.entering[1] = 1850;
        else if (left == RELOQ_DE_PHONE_R) {
            p.entering[0] = 420; p.entering[2] = 2520;
            clear_departure(&p, 2);
        }
        if (right == RELOQ_DE_PHONE_r) p.entering[0] = 280;
        else if (right == RELOQ_DE_PHONE_X)
            p.entering[0] = 380;
        break;
    case RELOQ_DE_PHONE_o:
        if (left == RELOQ_DE_PHONE_S) {
            p.entering[0] = 400; p.entering[1] = 760;
        } else if (left == RELOQ_DE_PHONE_l) {
            p.entering[1] = 765;
            if (right == RELOQ_DE_PHONE_n || right == RELOQ_DE_PHONE_r)
                departure(&p, 1, 765);
        }
        if (right == RELOQ_DE_PHONE_j) {
            p.entering[0] = 550; p.entering[1] = 900;
            p.entering[2] = 2500;
            clear_departure(&p, 1); clear_departure(&p, 2);
        }
        if ((left == RELOQ_DE_PHONE_l || left == RELOQ_DE_PHONE_S) &&
            (right == RELOQ_DE_PHONE_n || right == RELOQ_DE_PHONE_r ||
             right == RELOQ_DE_PHONE_w)) departure(&p, 1, p.entering[1]);
        if (right == RELOQ_DE_PHONE_A || right == RELOQ_DE_PHONE_a)
            departure(&p, 1, 800);
        break;
    case RELOQ_DE_PHONE_O:
        if (left == RELOQ_DE_PHONE_f) p.entering[1] = 890;
        if (right == RELOQ_DE_PHONE_l) p.entering[2] = 2400;
        if (right == RELOQ_DE_PHONE_l &&
            (left == RELOQ_DE_PHONE_p || left == RELOQ_DE_PHONE_b ||
             left == RELOQ_DE_PHONE_v || left == RELOQ_DE_PHONE_m))
            p.entering[0] = 500;
        if (right == RELOQ_DE_PHONE_l && left == RELOQ_DE_PHONE_w)
            p.entering[0] = 500;
        if (right == RELOQ_DE_PHONE_j) {
            p.entering[0] = 550; p.entering[1] = 900;
        }
        break;
    case RELOQ_DE_PHONE_u:
        if (left == RELOQ_DE_PHONE_m) p.entering[2] = 2600;
        if (right == RELOQ_DE_PHONE_s) {
            p.entering[0] = 320; departure(&p, 1, 820);
        } else if (right == RELOQ_DE_PHONE_n) {
            p.entering[0] = 280; departure(&p, 1, 780);
            if (alveolar_stop(left) || left == RELOQ_DE_PHONE_s ||
                left == RELOQ_DE_PHONE_z || left == RELOQ_DE_PHONE_n ||
                left == RELOQ_DE_PHONE_l) p.entering[1] = 780;
        } else if (right == RELOQ_DE_PHONE_l) departure(&p, 1, 950);
        else if (right == RELOQ_DE_PHONE_r) p.entering[0] = 300;
        break;
    case RELOQ_DE_PHONE_U:
        if (labial(left) || left == RELOQ_DE_PHONE_r) p.entering[1] = 900;
        else if (alveolar_stop(left) || left == RELOQ_DE_PHONE_s ||
                 left == RELOQ_DE_PHONE_z || left == RELOQ_DE_PHONE_X ||
                 left == RELOQ_DE_PHONE_l || left == RELOQ_DE_PHONE_j ||
                 left == RELOQ_DE_PHONE_n) p.entering[1] = 1000;
        else if (velar_stop(left) || left == RELOQ_DE_PHONE_x ||
                 left == RELOQ_DE_PHONE_G) p.entering[2] = 2060;
        if (right == RELOQ_DE_PHONE_m || right == RELOQ_DE_PHONE_n)
            p.entering[0] = 390;
        /* The preceding place gesture takes priority over this nasal
         * anticipation. Its F1/F2/F3 modifications are not additive. */
        if (right == RELOQ_DE_PHONE_m || right == RELOQ_DE_PHONE_n) {
            if (labial(left) || left == RELOQ_DE_PHONE_r) p.entering[0] = 420;
            if (alveolar_stop(left) || left == RELOQ_DE_PHONE_s ||
                left == RELOQ_DE_PHONE_z || left == RELOQ_DE_PHONE_X ||
                left == RELOQ_DE_PHONE_l || left == RELOQ_DE_PHONE_j ||
                left == RELOQ_DE_PHONE_n) p.entering[1] = 930;
            if (velar_stop(left) || left == RELOQ_DE_PHONE_x ||
                left == RELOQ_DE_PHONE_G) p.entering[2] = 2400;
        }
        if (right == RELOQ_DE_PHONE_G) departure(&p, 1, 800);
        if (nasal(left)) --p.av;
        break;
    case RELOQ_DE_PHONE_y:
        if (left == RELOQ_DE_PHONE_r) {
            p.entering[0] = 320; p.entering[1] = 1750;
        }
        break;
    case RELOQ_DE_PHONE_Y:
        if (left == RELOQ_DE_PHONE_l) {
            p.entering[1] = 1310;
            if (right == RELOQ_DE_PHONE_r || right == RELOQ_DE_PHONE_j ||
                right == RELOQ_DE_PHONE_w) departure(&p, 1, 1310);
        }
        if (right == RELOQ_DE_PHONE_X) departure(&p, 1, 1690);
        if (right == RELOQ_DE_PHONE_n) p.av -= 2;
        break;
    case RELOQ_DE_PHONE_AT:
        if (labial(left)) {
            p.entering[0] = 420; p.entering[1] = 1270;
        } else if (velar_stop(left)) {
            p.entering[1] = 1700; p.entering[2] = 2260;
        } else if (left == RELOQ_DE_PHONE_r) p.entering[1] = 1420;
        if (right != RELOQ_DE_PHONE_NONE && right != RELOQ_DE_PHONE_r &&
            right != RELOQ_DE_PHONE_h) {
            if (labial(left)) {
                p.entering[0] = 450; p.entering[1] = 1540;
            } else if (alveolar_stop(left) || velar_stop(left))
                p.entering[1] = 1600;
            else if (left == RELOQ_DE_PHONE_r) p.entering[1] = 1540;
        }
        if (labial(right)) departure(&p, 1, 1440);
        if (stress == 1 && alveolar_stop(right)) p.entering[1] = 1600;
        if (right == RELOQ_DE_PHONE_n) p.av -= 3;
        break;
    case RELOQ_DE_PHONE_Q_E_COLON:
        if (labial(left) || left == RELOQ_DE_PHONE_l ||
            left == RELOQ_DE_PHONE_r) p.entering[1] = 2000;
        if (right == RELOQ_DE_PHONE_r) departure(&p, 1, 1900);
        break;
    case RELOQ_DE_PHONE_Q_oe:
        if (left == RELOQ_DE_PHONE_s || left == RELOQ_DE_PHONE_z)
            p.entering[2] = 2250;
        if (alveolar_stop(right)) {
            departure(&p, 1, 1500); departure(&p, 2, 2100);
        } else if (velar_stop(right)) departure(&p, 2, 2150);
        break;
    case RELOQ_DE_PHONE_Q_OE:
        if (left == RELOQ_DE_PHONE_r) p.entering[1] = 1200;
        if (right == RELOQ_DE_PHONE_X && left != RELOQ_DE_PHONE_r)
            p.entering[1] = 1600;
        break;
    case RELOQ_DE_PHONE_R:
        if (left == RELOQ_DE_PHONE_m) {
            p.entering[0] = 700; p.entering[1] = 1350;
        } else if (alveolar_stop(left) || left == RELOQ_DE_PHONE_s ||
                   left == RELOQ_DE_PHONE_z || left == RELOQ_DE_PHONE_n)
            p.entering[1] = 1500;
        else if (left == RELOQ_DE_PHONE_l) {
            p.entering[1] = 1500; departure(&p, 1, 1400);
        } else if (left == RELOQ_DE_PHONE_S || left == RELOQ_DE_PHONE_Z) {
            p.entering[1] = 1450; departure(&p, 1, 1400);
        } else if (left == RELOQ_DE_PHONE_X) {
            p.entering[1] = 1400; p.entering[2] = 2310;
            departure(&p, 1, 1400);
        } else if (velar_stop(left) || left == RELOQ_DE_PHONE_x ||
                   left == RELOQ_DE_PHONE_G || left == RELOQ_DE_PHONE_r) {
            p.entering[1] = 1400; p.entering[2] = 2400;
        } else if (labial(left) && left != RELOQ_DE_PHONE_w)
            p.entering[1] = 1400;
        if (right != RELOQ_DE_PHONE_NONE && right != RELOQ_DE_PHONE_h)
            clear_departure(&p, 1);
        if (left != RELOQ_DE_PHONE_NONE && left != RELOQ_DE_PHONE_h &&
            left != RELOQ_DE_PHONE_j && left != RELOQ_DE_PHONE_w &&
            (right == RELOQ_DE_PHONE_p || right == RELOQ_DE_PHONE_b ||
             right == RELOQ_DE_PHONE_m || right == RELOQ_DE_PHONE_f ||
             right == RELOQ_DE_PHONE_v || right == RELOQ_DE_PHONE_w)) {
            p.entering[0] = 600; p.entering[1] = 1250;
            p.entering[2] = 2420;
        }
        if (right == RELOQ_DE_PHONE_f || right == RELOQ_DE_PHONE_v)
            p.entering[2] = 2200;
        else if (right == RELOQ_DE_PHONE_X) departure(&p, 1, 1710);
        else if (right == RELOQ_DE_PHONE_l) departure(&p, 1, 1210);
        else if (right == RELOQ_DE_PHONE_n) {
            if (left == RELOQ_DE_PHONE_NONE || left == RELOQ_DE_PHONE_h ||
                left == RELOQ_DE_PHONE_j || left == RELOQ_DE_PHONE_w) {
                p.entering[0] = 550; p.entering[1] = 1330;
            }
            departure(&p, 1, 1200);
        }
        break;
    default: break;
    }

    if (nasal(right)) {
        if (phone == RELOQ_DE_PHONE_a || phone == RELOQ_DE_PHONE_A ||
            phone == RELOQ_DE_PHONE_AT) p.bandwidth[0] = 270;
        else if (phone == RELOQ_DE_PHONE_i) {
            p.bandwidth[0] = 270; p.bandwidth[2] = 300;
        } else if (phone == RELOQ_DE_PHONE_o || phone == RELOQ_DE_PHONE_O ||
                   phone == RELOQ_DE_PHONE_u || phone == RELOQ_DE_PHONE_U ||
                   phone == RELOQ_DE_PHONE_R) {
            p.bandwidth[0] = 320; p.bandwidth[1] = 200;
            p.bandwidth[3] = 280; p.bandwidth[4] = 210;
        } else {
            p.bandwidth[0] = 220; p.bandwidth[4] = 310;
        }
    }
    if ((phone == RELOQ_DE_PHONE_i || phone == RELOQ_DE_PHONE_I) &&
        (right == RELOQ_DE_PHONE_j || right == RELOQ_DE_PHONE_w ||
         (phone == RELOQ_DE_PHONE_i && right == RELOQ_DE_PHONE_l))) p.av += 4;
    if ((phone == RELOQ_DE_PHONE_i || phone == RELOQ_DE_PHONE_I) &&
        right == RELOQ_DE_PHONE_r) --p.av;
    if (phone == RELOQ_DE_PHONE_y && right == RELOQ_DE_PHONE_r) p.av -= 3;
    if (phone == RELOQ_DE_PHONE_U && !nasal(left) &&
        (right == RELOQ_DE_PHONE_s ||
        right == RELOQ_DE_PHONE_z || right == RELOQ_DE_PHONE_n)) p.av += 4;
    if (phone == RELOQ_DE_PHONE_A &&
        (right == RELOQ_DE_PHONE_j || right == RELOQ_DE_PHONE_w)) --p.av;
    p.av_end = p.av - 2;
    if (right == RELOQ_DE_PHONE_l) p.av_end = p.av - 5;
    if (phone == RELOQ_DE_PHONE_A && right == RELOQ_DE_PHONE_j)
        p.av_end = p.av - 5;
    if (right == RELOQ_DE_PHONE_n || right == RELOQ_DE_PHONE_G) {
        p.av_end = phone == RELOQ_DE_PHONE_a ? p.av - 2 :
               phone == RELOQ_DE_PHONE_u ? p.av - 3 : 48;
    }
    if (weak_final) { p.av -= 3; p.av_end -= 3; }
    if (phone == RELOQ_DE_PHONE_a && stress == 2 &&
        right != RELOQ_DE_PHONE_l && right != RELOQ_DE_PHONE_r &&
        right != RELOQ_DE_PHONE_j && right != RELOQ_DE_PHONE_w) {
        p.av -= 3; p.av_end -= 3;
    }
    *out = p;
    return 1;
}
