#include "reloq_german_generic_coarticulation.h"
#include "reloq_german_transition_duration.h"

/* Formant classes are authored from controlled onset/coda observations.
 * Low/back and front vowels share gestures by tract position. Durations are
 * The independent duration rules supply supported German CV/VC intervals.
 * Earlier estimates remain fallback values for unmodeled vowel classes. */
static int vowel(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_A || p == RELOQ_DE_PHONE_E ||
        p == RELOQ_DE_PHONE_I || p == RELOQ_DE_PHONE_O ||
        p == RELOQ_DE_PHONE_U || p == RELOQ_DE_PHONE_a ||
        p == RELOQ_DE_PHONE_e || p == RELOQ_DE_PHONE_i ||
        p == RELOQ_DE_PHONE_o || p == RELOQ_DE_PHONE_u ||
        p == RELOQ_DE_PHONE_y || p == RELOQ_DE_PHONE_Y ||
        p == RELOQ_DE_PHONE_AT || p == RELOQ_DE_PHONE_R ||
        (p >= RELOQ_DE_PHONE_Q_E_COLON &&
         p <= RELOQ_DE_PHONE_Q_oe_TILDE);
}

static void formants(int32_t target[3], int32_t f1, int32_t f2, int32_t f3)
{
    target[0] = f1;
    target[1] = f2;
    target[2] = f3;
}

int reloq_german_generic_cv_gesture(
    const struct reloq_german_phone_node *node, const int32_t v[3],
    struct reloq_german_generic_cv_gesture *out)
{
    if (!node || !v || !out || v[0] <= 0 || v[1] <= 0 || v[2] <= 0)
        return -1;
    if (!node->same_syllable_right || !vowel(node->right_phone) ||
        vowel(node->phone)) return 0;
    struct reloq_german_generic_cv_gesture p = {{0}, {0}, 0, 1, 0};
    switch (node->phone) {
    case RELOQ_DE_PHONE_p:
        formants(p.first_formant, 350,
                 v[1] >= 1900 ? 1250 : v[1] >= 1500 ? 1150 :
                 v[1] >= 1000 ? 900 : v[0] >= 500 ? 850 : 800,
                 2500);
        p.anticipate_during_consonant = 1;
        break;
    case RELOQ_DE_PHONE_t:
        formants(p.first_formant, 300, 1600, 2500);
        p.anticipate_during_consonant = 1;
        break;
    case RELOQ_DE_PHONE_k:
        formants(p.first_formant, 300, 1500, 2300);
        p.anticipate_during_consonant = 1;
        break;
    case RELOQ_DE_PHONE_b:
        formants(p.first_formant, 200, 1000, 2500);
        p.vowel_glide_ticks = v[1] >= 1850 ? 25 : 15;
        break;
    case RELOQ_DE_PHONE_d:
        formants(p.first_formant, 200, 1600, 2600);
        p.vowel_glide_ticks = v[1] >= 1550 ? 25 : 45;
        break;
    case RELOQ_DE_PHONE_g:
        formants(p.first_formant, 300, 1500, 1890);
        p.vowel_glide_ticks = v[1] >= 1100 && v[1] < 1500 ? 35 : 25;
        break;
    case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_v:
        /* The voiced labiodental onset is more constricted than /f/:
         * keep its 300-Hz F1 during the consonant, then glide into the
         * following vowel.  The noise-source profile already uses 300. */
        formants(p.first_formant,
                 node->phone == RELOQ_DE_PHONE_v ? 300 : 400,
                 v[1] >= 1900 ? 1400 : v[1] >= 1550 ? 1000 :
                 v[1] >= 1050 ? 920 : 820,
                 v[1] >= 1900 ? 2500 : 2300);
        p.vowel_glide_ticks = 25;
        break;
    case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_z:
        formants(p.first_formant, 300, v[1] >= 1900 ? 1760 : 1500,
                 2600);
        p.vowel_glide_ticks = v[1] < 900 ? 40 : 30;
        break;
    case RELOQ_DE_PHONE_S: case RELOQ_DE_PHONE_Z:
        formants(p.first_formant, 300,
                 v[1] < 1000 && v[0] < 500 ? 1400 : 1800,
                 v[1] < 1000 && v[0] < 500 ? 2200 : 2400);
        p.vowel_glide_ticks = v[1] < 1000 && v[0] < 500 ? 40 : 30;
        break;
    case RELOQ_DE_PHONE_h:
        formants(p.first_formant, v[0], v[1], v[2]);
        p.anticipate_during_consonant = 1;
        break;
    case RELOQ_DE_PHONE_m: case RELOQ_DE_PHONE_G:
        p.replace_consonant_tract = 0;
        p.vowel_glide_ticks = 15;
        break;
    case RELOQ_DE_PHONE_n:
        p.replace_consonant_tract = 0;
        p.vowel_glide_ticks = v[1] < 1100 ? 50 : v[1] >= 1550 ? 30 : 25;
        break;
    case RELOQ_DE_PHONE_l:
        p.replace_consonant_tract = 0;
        p.vowel_glide_ticks = v[1] >= 1550 ? 30 : 20;
        break;
    case RELOQ_DE_PHONE_r: case RELOQ_DE_PHONE_R:
        p.replace_consonant_tract = 0;
        p.vowel_glide_ticks = v[1] >= 1800 ? 60 : 45;
        break;
    case RELOQ_DE_PHONE_j:
        p.replace_consonant_tract = 0;
        p.vowel_glide_ticks = 20;
        break;
    default: return 0;
    }
    if (p.replace_consonant_tract) {
        formants(p.release_formant, p.first_formant[0],
                 p.first_formant[1], p.first_formant[2]);
        if (node->phone == RELOQ_DE_PHONE_t && v[1] >= 1550) {
            p.release_formant[1] += 100;
            p.release_formant[2] += 200;
        } else if (node->phone == RELOQ_DE_PHONE_k) {
            p.release_formant[1] = v[1] >= 1900 ? 2200 :
                v[1] >= 1550 ? 2150 : v[1] >= 1050 ? 1600 :
                v[0] >= 500 ? 1400 : 1250;
            p.release_formant[2] = v[1] >= 1900 ? 2700 :
                v[1] >= 1550 ? 2600 : v[1] >= 1050 ? 2100 : 2000;
        }
    }
    int32_t ticks;
    if (p.vowel_glide_ticks > 0 &&
        reloq_german_transition_cv_ticks(node, &ticks) == 1)
        p.vowel_glide_ticks = ticks;
    *out = p;
    return 1;
}

int reloq_german_generic_vc_gesture(
    const struct reloq_german_phone_node *node,
    const struct reloq_german_phone_node *right,
    const int32_t v[3], const int32_t c[3],
    struct reloq_german_generic_vc_gesture *out)
{
    if (!node || !right || !v || !c || !out || v[0] <= 0 || v[1] <= 0 ||
        v[2] <= 0 || c[0] <= 0 || c[1] <= 0 || c[2] <= 0) return -1;
    if (!vowel(node->phone) || !node->same_syllable_right ||
        vowel(right->phone) || node->right_phone != right->phone) return 0;
    struct reloq_german_generic_vc_gesture p = {{c[0], c[1], c[2]}, 0};
    switch (right->phone) {
    case RELOQ_DE_PHONE_p:
        formants(p.coda_formant, v[0] >= 500 ? 500 : 300,
                 v[1] >= 1750 ? 1000 : 900, 2500);
        p.anticipation_ticks = v[0] >= 500 ? 30 : 25;
        break;
    case RELOQ_DE_PHONE_t:
        formants(p.coda_formant, 300, v[1] >= 1750 ? 1800 : 1600,
                 2500);
        p.anticipation_ticks = v[0] >= 500 ? 40 : 25;
        break;
    case RELOQ_DE_PHONE_k:
        formants(p.coda_formant, v[0] >= 500 ? 500 : 300,
                 v[1] >= 1750 ? 1900 : 1750, 2300);
        p.anticipation_ticks = v[0] >= 500 ? 40 : 25;
        break;
    case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_v:
        formants(p.coda_formant, 400,
                 v[1] >= 1750 ? 1450 : 1000,
                 v[1] >= 1750 ? 2500 : 2300);
        p.anticipation_ticks = 25;
        break;
    case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_z:
        formants(p.coda_formant, 300, 1500, 2600);
        p.anticipation_ticks = v[0] >= 500 ? 40 : 25;
        break;
    case RELOQ_DE_PHONE_S: case RELOQ_DE_PHONE_Z:
        formants(p.coda_formant, 300, 1800, 2400);
        p.anticipation_ticks = 30;
        break;
    case RELOQ_DE_PHONE_x: case RELOQ_DE_PHONE_X: {
        /* /x/ and /X/ carry their own oral place, despite sharing the
         * following broad-band frication route. Without an anticipatory
         * target, the generic planner simply copies the preceding vowel's
         * formants through the entire consonant, making German ch sound
         * like a noisy continuation of that vowel. The tongue-place class
         * also determines how early the vowel moves into the coda. */
        const int back = node->phone == RELOQ_DE_PHONE_O ||
            node->phone == RELOQ_DE_PHONE_U ||
            node->phone == RELOQ_DE_PHONE_o ||
            node->phone == RELOQ_DE_PHONE_u;
        const int low = node->phone == RELOQ_DE_PHONE_A ||
            node->phone == RELOQ_DE_PHONE_a;
        p.anticipation_ticks = node->phone == RELOQ_DE_PHONE_AT ? 20 :
            back ? right->phone == RELOQ_DE_PHONE_x ? 40 : 30 :
            low ? 30 : 25;
        if (right->phone == RELOQ_DE_PHONE_x) {
            int f2 = low ? 1220 :
                node->phone == RELOQ_DE_PHONE_O ||
                node->phone == RELOQ_DE_PHONE_U ? 900 :
                node->phone == RELOQ_DE_PHONE_o ? 620 :
                node->phone == RELOQ_DE_PHONE_u ? 750 : 1350;
            formants(p.coda_formant, 300, f2, v[2]);
        } else {
            int f2 = node->phone == RELOQ_DE_PHONE_E ? 1880 :
                node->phone == RELOQ_DE_PHONE_I ? 2200 :
                node->phone == RELOQ_DE_PHONE_e ? 2430 :
                node->phone == RELOQ_DE_PHONE_i ? 2500 :
                node->phone == RELOQ_DE_PHONE_y ? 2050 :
                node->phone == RELOQ_DE_PHONE_Y ? 1740 : 2150;
            int f3 = node->phone == RELOQ_DE_PHONE_I ||
                node->phone == RELOQ_DE_PHONE_e ||
                node->phone == RELOQ_DE_PHONE_i ? 3100 :
                node->phone == RELOQ_DE_PHONE_Y ? 2350 : 2900;
            formants(p.coda_formant, 300, f2, f3);
        }
        break;
    }
    case RELOQ_DE_PHONE_m: case RELOQ_DE_PHONE_n:
    case RELOQ_DE_PHONE_G:
        p.anticipation_ticks = v[1] >= 1700 ? 30 : 40;
        break;
    case RELOQ_DE_PHONE_l:
        p.anticipation_ticks = v[1] >= 1700 ? 30 : 60;
        break;
    case RELOQ_DE_PHONE_r: case RELOQ_DE_PHONE_R:
        p.anticipation_ticks = 40;
        break;
    default: return 0;
    }
    int32_t ticks;
    if (reloq_german_transition_vc_ticks(node, right, &ticks) == 1)
        p.anticipation_ticks = ticks;
    *out = p;
    return 1;
}

int reloq_german_generic_vowel_center(
    const struct reloq_german_phone_node *node, int32_t onset_f2,
    struct reloq_german_generic_vowel_center *out)
{
    if (!node || !out || onset_f2 <= 0) return -1;
    if (!node->same_syllable_left ||
        (node->left_phone != RELOQ_DE_PHONE_p &&
         node->left_phone != RELOQ_DE_PHONE_t &&
         node->left_phone != RELOQ_DE_PHONE_k)) return 0;
    const int32_t neutral = node->phone == RELOQ_DE_PHONE_a ? 1220 :
                            node->phone == RELOQ_DE_PHONE_e ? 2150 : 0;
    if (!neutral || onset_f2 == neutral) return 0;
    if (node->same_word_right && !node->same_syllable_right) return 0;
    int32_t lead = 0;
    if (node->same_syllable_right && !vowel(node->right_phone)) {
        switch (node->right_phone) {
        case RELOQ_DE_PHONE_p: case RELOQ_DE_PHONE_t:
        case RELOQ_DE_PHONE_k: case RELOQ_DE_PHONE_s:
            lead = node->phone == RELOQ_DE_PHONE_a ? 50 : 25;
            break;
        case RELOQ_DE_PHONE_f:
            lead = 25;
            break;
        case RELOQ_DE_PHONE_m: case RELOQ_DE_PHONE_n:
        case RELOQ_DE_PHONE_G: case RELOQ_DE_PHONE_x:
        case RELOQ_DE_PHONE_X:
            lead = 30;
            break;
        default:
            /* Unmeasured coda classes retain the old gesture rather than
             * inventing a medial target in the wrong order. */
            return 0;
        }
    }
    *out = (struct reloq_german_generic_vowel_center){neutral, lead};
    return 1;
}
