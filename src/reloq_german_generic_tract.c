#include "reloq_german_generic_tract.h"

enum vowel_region {
    REGION_NONE, REGION_LOW, REGION_MID_FRONT, REGION_HIGH_FRONT,
    REGION_BACK, REGION_LONG_BACK, REGION_ROUNDED_FRONT, REGION_CENTRAL
};

static enum vowel_region region(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_Q_aj: case RELOQ_DE_PHONE_Q_aw:
        return REGION_LOW;
    case RELOQ_DE_PHONE_E: return REGION_MID_FRONT;
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_e:
    case RELOQ_DE_PHONE_i: return REGION_HIGH_FRONT;
    case RELOQ_DE_PHONE_O: case RELOQ_DE_PHONE_U:
        return REGION_BACK;
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
    case RELOQ_DE_PHONE_Q_oj: return REGION_LONG_BACK;
    case RELOQ_DE_PHONE_y: case RELOQ_DE_PHONE_Y:
    case RELOQ_DE_PHONE_Q_oe: case RELOQ_DE_PHONE_Q_OE:
        return REGION_ROUNDED_FRONT;
    case RELOQ_DE_PHONE_AT: case RELOQ_DE_PHONE_R: return REGION_CENTRAL;
    default: return REGION_NONE;
    }
}

static int stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

int reloq_german_generic_vowel_tract(
    const struct reloq_german_phone_node *node,
    int32_t base_f1, int32_t base_f2, int32_t base_f3, int32_t base_av,
    struct reloq_german_generic_tract *out)
{
    if (!node || !out || base_f1 <= 0 || base_f2 <= 0 ||
        base_f3 <= 0 || base_av < 0 ||
        (region(node->phone) == REGION_NONE &&
         (node->phone < RELOQ_DE_PHONE_Q_E_COLON ||
          node->phone > RELOQ_DE_PHONE_Q_oe_TILDE))) return -1;
    struct reloq_german_generic_tract p = {
        base_f1, base_f2, base_f3, 120, 100, 200, 200, base_av, 34
    };
    /* Isolated central, rounded and nasal vowel classes needed substantially
     * different oral/nasal target locations from the provisional voice.
     * Ordinary A/E/I/O/U and a/e/i/o/u already match their isolated centres. */
    switch (node->phone) {
    case RELOQ_DE_PHONE_AT:
        p.f1 = 450; p.f2 = 1540; p.av = 53; break;
    case RELOQ_DE_PHONE_y:
        p.f2 = 1820; p.f3 = 2300; p.av = 57; break;
    case RELOQ_DE_PHONE_Y:
        p.f1 = 400; p.f2 = 1480; p.f3 = 2100; p.av = 57; break;
    case RELOQ_DE_PHONE_Q_E_COLON:
        p.f1 = 500; p.f2 = 2005; p.f3 = 2600; p.av = 51; break;
    case RELOQ_DE_PHONE_Q_oe:
        p.f2 = 1480; p.f3 = 2010; p.av = 57; break;
    case RELOQ_DE_PHONE_Q_OE:
        p.f1 = 540; p.f2 = 1350; p.f3 = 2140; p.av = 53; break;
    case RELOQ_DE_PHONE_Q_aj:
        p.f1 = 820; p.f2 = 1220; p.av = 50; break;
    case RELOQ_DE_PHONE_Q_aw:
        p.f1 = 780; p.f2 = 1160; p.av = 50; break;
    case RELOQ_DE_PHONE_Q_oj:
        p.f1 = 550; p.f2 = 900; p.f3 = 2500; p.av = 53; break;
    case RELOQ_DE_PHONE_Q_a_TILDE:
        p.f1 = 624; p.f2 = 960; p.f3 = 2880;
        p.fnp = 280; p.fnz = 350; p.b1 = 180; p.av = 52; break;
    case RELOQ_DE_PHONE_Q_E_TILDE:
        p.f1 = 570; p.f2 = 1850; p.f3 = 2550;
        p.fnp = 250; p.fnz = 350; p.av = 57; break;
    case RELOQ_DE_PHONE_Q_o_TILDE:
        p.f1 = 450; p.f2 = 650; p.f3 = 2800;
        p.b1 = 200; p.b2 = 200; p.av = 55; break;
    case RELOQ_DE_PHONE_Q_oe_TILDE:
        p.f1 = 610; p.f2 = 1350; p.f3 = 2700;
        p.b1 = 200; p.av = 57; break;
    default: break;
    }
    enum reloq_german_phone left = node->same_syllable_left ?
        node->left_phone : RELOQ_DE_PHONE_NONE;
    enum reloq_german_phone right = node->same_syllable_right ?
        node->right_phone : RELOQ_DE_PHONE_NONE;
    /* A short /A/ is shaped toward the next phone from its first target,
     * including across a syllable boundary inside one word. A following h
     * is neutral; palatal j and velar x need a more open centre. A word
     * boundary excludes this anticipatory F1 coloring. This is a phone
     * context, independent of spelling and lexical entries. */
    const enum reloq_german_phone word_right = node->same_word_right ?
        node->right_phone : RELOQ_DE_PHONE_NONE;
    /* A right t raises isolated short A's F2, except when a stop onset
     * already sets its own F2 transition. */
    if (node->phone == RELOQ_DE_PHONE_A) {
        if (word_right != RELOQ_DE_PHONE_NONE &&
            word_right != RELOQ_DE_PHONE_h)
            p.f1 += word_right == RELOQ_DE_PHONE_x ? 100 :
                    word_right == RELOQ_DE_PHONE_j ? 120 : 80;
        if (right == RELOQ_DE_PHONE_t && !stop(left)) p.f2 += 140;
        if (left == RELOQ_DE_PHONE_p) p.f2 -= 10;
        else if (left == RELOQ_DE_PHONE_t) p.f2 += 140;
        else if (left == RELOQ_DE_PHONE_k) {
            p.f2 += 90; p.f3 -= 100;
        } else if (left == RELOQ_DE_PHONE_m) p.f2 -= 10;
        else if (left == RELOQ_DE_PHONE_r) {
            p.f2 = 1200; p.f3 = 2300;
        }
    } else if (node->phone == RELOQ_DE_PHONE_U) {
        if (left == RELOQ_DE_PHONE_p) p.f2 -= 30;
        else if (left == RELOQ_DE_PHONE_t) p.f2 += 70;
        else if (left == RELOQ_DE_PHONE_k) p.f3 -= 340;
        else if (left == RELOQ_DE_PHONE_m) p.f2 -= 30;
    } else if (node->phone == RELOQ_DE_PHONE_I) {
        if (left == RELOQ_DE_PHONE_k) p.f1 -= 30;
        else if (left == RELOQ_DE_PHONE_l) p.f2 -= 100;
        else if (left == RELOQ_DE_PHONE_r) p.f2 -= 150;
        else if (left == RELOQ_DE_PHONE_R) {
            p.f1 = 420; p.f3 = 2520; p.av -= 3;
        }
    } else if (node->phone == RELOQ_DE_PHONE_a) {
        if (left == RELOQ_DE_PHONE_p || left == RELOQ_DE_PHONE_m)
            p.f2 -= 120;
        else if (left == RELOQ_DE_PHONE_t) p.f2 += 30;
        else if (left == RELOQ_DE_PHONE_k) p.f2 += 60;
        else if (left == RELOQ_DE_PHONE_R) {
            p.f1 = 700; p.f2 = 1160; p.av = 48;
        }
    } else if (node->phone == RELOQ_DE_PHONE_Q_aj ||
               node->phone == RELOQ_DE_PHONE_Q_aw) {
        /* Both low-onset diphthongs share the tongue gesture left by
         * a place consonant. /oj/ starts at its own rounded-back centre. */
        switch (left) {
        case RELOQ_DE_PHONE_p: case RELOQ_DE_PHONE_b:
        case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_v:
        case RELOQ_DE_PHONE_m: p.f2 = 1150; break;
        case RELOQ_DE_PHONE_t: case RELOQ_DE_PHONE_d:
            p.f2 = 1300; break;
        case RELOQ_DE_PHONE_k: case RELOQ_DE_PHONE_g:
            p.f2 = 1250; p.f3 = 2400; break;
        case RELOQ_DE_PHONE_r:
            p.f2 = 1200; p.f3 = 2300; break;
        default: break;
        }
    } else if (node->phone == RELOQ_DE_PHONE_e &&
               left == RELOQ_DE_PHONE_p) p.f2 -= 150;
    else if (node->phone == RELOQ_DE_PHONE_AT) {
        if (left == RELOQ_DE_PHONE_m || left == RELOQ_DE_PHONE_p) {
            p.f1 -= 30; p.f2 -= 270;
        } else if (left == RELOQ_DE_PHONE_k) {
            p.f2 += 160; p.f3 -= 240;
        } else if (left == RELOQ_DE_PHONE_r) p.f2 -= 120;
        if (right == RELOQ_DE_PHONE_t) p.f2 += 60;
    }
    *out = p;
    return 0;
}

int reloq_german_generic_sonorant_tract(
    const struct reloq_german_phone_node *node,
    struct reloq_german_generic_tract *out)
{
    if (!node || !out) return -1;
    const enum vowel_region following = node->same_syllable_right ?
        region(node->right_phone) : REGION_NONE;
    const enum vowel_region preceding = node->same_syllable_left ?
        region(node->left_phone) : REGION_NONE;
    /* The same consonant makes an onset gesture toward a following vowel
     * or a coda gesture away from a preceding vowel. The compact vowel
     * regions encode tongue height/rounding, not lexical items. */
    struct reloq_german_generic_tract p = {
        400, 1320, 2100, 120, 100, 200, 200, 0, 27
    };
    switch (node->phone) {
    case RELOQ_DE_PHONE_m:
        p = (struct reloq_german_generic_tract){
            following ? 300 : 200, 1000, 2500, 120, 150, 200, 350,
            following ? 51 : 45, 0};
        if (following == REGION_MID_FRONT || following == REGION_HIGH_FRONT ||
            following == REGION_ROUNDED_FRONT) p.f2 = 1300;
        else if (following == REGION_BACK ||
                 following == REGION_LONG_BACK) {
            p.f2 = 700; p.f3 = 2300;
        } else if (!following) {
            p.f2 = preceding == REGION_LOW ? 750 :
                   preceding == REGION_BACK || preceding == REGION_LONG_BACK ?
                   700 : preceding == REGION_HIGH_FRONT ? 1300 : 1000;
            if (preceding == REGION_BACK ||
                preceding == REGION_LONG_BACK) p.f3 = 2300;
            if (preceding == REGION_LOW) p.av = 42;
        }
        break;
    case RELOQ_DE_PHONE_n:
        p = (struct reloq_german_generic_tract){
            200, 1600, 2500, 120, 200, 200, 350,
            following ? 48 : 42, 0};
        if (following == REGION_MID_FRONT ||
            following == REGION_ROUNDED_FRONT) {
            p.f2 = 1700; p.f3 = 2600;
        } else if (following == REGION_HIGH_FRONT) {
            p.f2 = 1850; p.f3 = 2800;
        } else if (following == REGION_BACK) p.f2 = 1500;
        else if (following == REGION_LONG_BACK) p.f2 = 1400;
        else if (!following) {
            p.f2 = preceding == REGION_BACK ? 1450 :
                   preceding == REGION_LONG_BACK ? 1450 :
                   preceding == REGION_MID_FRONT ? 1700 : 1600;
            p.f3 = preceding == REGION_HIGH_FRONT ? 2800 :
                   preceding == REGION_MID_FRONT ? 2600 : 2500;
        }
        break;
    case RELOQ_DE_PHONE_G:
        p = (struct reloq_german_generic_tract){
            300, 1600, 2600, 108, 150, 200, 400, 49, 0};
        if (following == REGION_MID_FRONT ||
            following == REGION_CENTRAL ||
            following == REGION_ROUNDED_FRONT) {
            p.f2 = 1900; p.f3 = 2300;
        } else if (following == REGION_HIGH_FRONT) {
            p.f2 = 2100; p.f3 = 2500;
        } else if (!following) {
            p.av = 42;
            p.f2 = preceding == REGION_BACK ||
                   preceding == REGION_LONG_BACK ? 800 :
                   preceding == REGION_LOW ? 1750 : 2100;
            p.f3 = preceding == REGION_BACK ||
                   preceding == REGION_LONG_BACK ? 2100 :
                   preceding == REGION_LOW ? 2200 : 2600;
        }
        break;
    case RELOQ_DE_PHONE_l:
        p = (struct reloq_german_generic_tract){
            400, 1450, 2900, 120, 100, 200, 200, 46, 34};
        if (following == REGION_LOW) p.f2 = 1230;
        else if (following == REGION_HIGH_FRONT) {
            p.f1 = 300; p.f2 = 1550;
        } else if (following == REGION_BACK) p.f2 = 1400;
        else if (following == REGION_LONG_BACK) p.f2 = 1300;
        else if (!following) {
            p.f1 = 350; p.f2 = 1500;
            p.f3 = preceding == REGION_BACK ||
                   preceding == REGION_LONG_BACK ? 2400 : 2800;
        }
        break;
    case RELOQ_DE_PHONE_r:
        if (following == REGION_MID_FRONT ||
            following == REGION_HIGH_FRONT) {
            p.f2 = following == REGION_MID_FRONT ? 1450 : 1670;
            p.f3 = following == REGION_MID_FRONT ? 2200 : 2300;
        } else if (following == REGION_BACK) {
            p.f2 = 1200; p.f3 = 2000;
        } else if (following == REGION_LONG_BACK) {
            p.f2 = 840; p.f3 = 2000;
        } else if (following == REGION_ROUNDED_FRONT) {
            p.f2 = 1200; p.f3 = 2000;
        } else if (!following) {
            p.f1 = 600; p.f2 = 1320; p.f3 = 2300;
            p.av = 45; p.ah = 35;
            if (preceding == REGION_BACK ||
                preceding == REGION_LONG_BACK) {
                p.f2 = 1050; p.f3 = 2100;
            }
        }
        break;
    case RELOQ_DE_PHONE_R:
        p = (struct reloq_german_generic_tract){
            600, 1360, 2280, 120, 100, 200, 200, 48, 34};
        if (!following) {
            p.f1 = preceding == REGION_LOW ? 570 : 600;
            p.f2 = preceding == REGION_LOW ? 1190 : 1360;
            p.f3 = preceding == REGION_LOW ? 2320 : 2280;
            p.av = 45;
        }
        break;
    case RELOQ_DE_PHONE_j:
        p = (struct reloq_german_generic_tract){
            300, 2100, 2900, 120, 250, 200, 200, 48, 34};
        if (following == REGION_HIGH_FRONT ||
            following == REGION_ROUNDED_FRONT) p.av = 52;
        else if (following == REGION_BACK ||
                 following == REGION_LONG_BACK) {
            p.f2 = 1900; p.f3 = 2500;
            if (node->right_phone == RELOQ_DE_PHONE_U ||
                node->right_phone == RELOQ_DE_PHONE_u) p.av = 52;
        } else if (following == REGION_CENTRAL) {
            p.f2 = 1940; p.f3 = 2700; p.av = 53;
        } else if (!following) {
            p.f1 = preceding == REGION_LOW ? 420 : 300;
            p.f2 = preceding == REGION_MID_FRONT ||
                   preceding == REGION_HIGH_FRONT ? 2150 : 1900;
            p.f3 = preceding == REGION_MID_FRONT ||
                   preceding == REGION_HIGH_FRONT ? 2900 : 2450;
            p.b2 = 100; p.av = 45; p.ah = 35;
        }
        break;
    default: return -1;
    }
    *out = p;
    return 0;
}
