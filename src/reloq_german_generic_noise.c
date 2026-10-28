#include "reloq_german_generic_noise.h"

#include <stddef.h>

/* Only phonetic classes and adjacent phones select these controls.  The
 * source strengths and active resonator routes are calibrated against
 * Apple ECI's German 11,025-Hz voiced/unvoiced consonant contrasts; temporal
 * trajectories remain the generic planner's responsibility. */
static int front(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_E || p == RELOQ_DE_PHONE_I ||
           p == RELOQ_DE_PHONE_e || p == RELOQ_DE_PHONE_i ||
           p == RELOQ_DE_PHONE_y || p == RELOQ_DE_PHONE_Y ||
           p == RELOQ_DE_PHONE_Q_E_COLON ||
           p == RELOQ_DE_PHONE_Q_oe || p == RELOQ_DE_PHONE_Q_OE;
}

static int back(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_O || p == RELOQ_DE_PHONE_U ||
           p == RELOQ_DE_PHONE_o || p == RELOQ_DE_PHONE_u ||
           p == RELOQ_DE_PHONE_Q_oj;
}

static int fricative(enum reloq_german_phone p)
{
    return p==RELOQ_DE_PHONE_f || p==RELOQ_DE_PHONE_v ||
        p==RELOQ_DE_PHONE_s || p==RELOQ_DE_PHONE_z ||
        p==RELOQ_DE_PHONE_S || p==RELOQ_DE_PHONE_Z ||
        p==RELOQ_DE_PHONE_x || p==RELOQ_DE_PHONE_X;
}

static int vowel_source(enum reloq_german_phone p)
{
    switch (p) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_e: return 51;
    case RELOQ_DE_PHONE_E: return 52;
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_i: return 53;
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_o: return 54;
    case RELOQ_DE_PHONE_a: return 50;
    case RELOQ_DE_PHONE_u: return 55;
    default: return 52;
    }
}

static void oral(int32_t target[RELOQ_TARGET_STREAM_COUNT],
                 int f1, int f2, int f3, int f4, int f5)
{
    target[RELOQ_TARGET_F1] = f1;
    target[RELOQ_TARGET_F2] = f2;
    target[RELOQ_TARGET_F3] = f3;
    target[RELOQ_TARGET_F4] = f4;
    target[RELOQ_TARGET_F5] = f5;
}

int reloq_german_s_nasal_source(const struct reloq_german_phone_node *node)
{
    return node && !node->same_word_left &&
        (node->right_phone==RELOQ_DE_PHONE_m || node->right_phone==RELOQ_DE_PHONE_n)?62:50;
}

int reloq_german_generic_fricative_noise(
    const struct reloq_german_phone_node *node,
    int32_t target[RELOQ_TARGET_STREAM_COUNT])
{
    if (!node || !target) return -1;
    const enum reloq_german_phone p = node->phone;
    const enum reloq_german_phone near = node->same_syllable_right ?
        node->right_phone : node->left_phone;
    const int onset = node->same_syllable_right &&
        (!node->same_syllable_left || node->same_syllable_right);
    switch (p) {
    case RELOQ_DE_PHONE_f:
    case RELOQ_DE_PHONE_v:
        oral(target, p == RELOQ_DE_PHONE_v && onset ? 300 : 400,
             front(near) ? 1450 : back(near) ? 750 :
             onset && p == RELOQ_DE_PHONE_f ? 920 : 1000,
             front(near) ? 2500 : 2300, 3600, 3900);
        target[RELOQ_TARGET_AF] = p == RELOQ_DE_PHONE_v && onset ? 60 : 58;
        target[RELOQ_TARGET_AB] = 60;
        target[RELOQ_TARGET_AV] = p == RELOQ_DE_PHONE_v && onset ? 30 : 0;
        break;
    case RELOQ_DE_PHONE_s:
    case RELOQ_DE_PHONE_z:
        oral(target, 300, front(near) ? 1800 :
             p == RELOQ_DE_PHONE_z && onset ? 1390 : 1500,
             2600, p == RELOQ_DE_PHONE_z && onset ? 3700 : 3400,
             p == RELOQ_DE_PHONE_z && onset ? 4200 :
             back(near) ? 4300 : 4400);
        /* A word-final s that runs on into the next word is louder. */
        target[RELOQ_TARGET_AF] = p == RELOQ_DE_PHONE_z && onset ? 66 :
            p == RELOQ_DE_PHONE_s && !onset && !node->same_word_right &&
            node->right_phone != RELOQ_DE_PHONE_NONE ? 65 : 62;
        /* Requested departure: an initial /sm, sn/ uses ordinary /s/
         * excitation. Preserve the original reduction inside a word and
         * before /ng/, including the original transition timing. */
        if (p==RELOQ_DE_PHONE_s && node->same_syllable_right &&
            (node->right_phone==RELOQ_DE_PHONE_m || node->right_phone==RELOQ_DE_PHONE_n ||
             node->right_phone==RELOQ_DE_PHONE_G)) target[RELOQ_TARGET_AF]=reloq_german_s_nasal_source(node);
        target[RELOQ_TARGET_A5F] = 50;
        if (p == RELOQ_DE_PHONE_z && onset) {
            target[RELOQ_TARGET_A4F] = 50;
            target[RELOQ_TARGET_AV] = 20;
            const enum reloq_german_phone l=node->left_phone;
            target[RELOQ_TARGET_TL] = l==RELOQ_DE_PHONE_p || l==RELOQ_DE_PHONE_t ||
                l==RELOQ_DE_PHONE_k || l==RELOQ_DE_PHONE_s || l==RELOQ_DE_PHONE_z ||
                (!node->same_word_left && fricative(l))?0:35;
        }
        break;
    case RELOQ_DE_PHONE_S:
    case RELOQ_DE_PHONE_Z:
        oral(target, 300, p == RELOQ_DE_PHONE_Z ? 1700 : 1800,
             p == RELOQ_DE_PHONE_Z ? 2300 : 2400, 3600, 4000);
        target[RELOQ_TARGET_AF] = p == RELOQ_DE_PHONE_Z ?
            (node->right_phone!=RELOQ_DE_PHONE_NONE ? 57 : 54) : 56;
        if (p == RELOQ_DE_PHONE_S) target[RELOQ_TARGET_A2F] = 56;
        target[RELOQ_TARGET_A3F] = 62;
        target[RELOQ_TARGET_A4F] = p == RELOQ_DE_PHONE_Z ? 59 : 62;
        target[RELOQ_TARGET_AV] = p == RELOQ_DE_PHONE_Z && onset ? 40 : 0;
        break;
    case RELOQ_DE_PHONE_x:
        oral(target, 300, 1220, 2500, 3600, 3900);
        /* ger_ph_x/Z lower AF only before the pause phone. A syllable
         * coda followed by another phone retains the connected level. */
        target[RELOQ_TARGET_AF] = node->right_phone!=RELOQ_DE_PHONE_NONE ? 55 : 52;
        target[RELOQ_TARGET_A2F] = 70;
        target[RELOQ_TARGET_A4F] = 55;
        break;
    case RELOQ_DE_PHONE_X:
        oral(target, 300, 1700, 2900, 3500, 4500);
        target[RELOQ_TARGET_AF] = node->right_phone==RELOQ_DE_PHONE_NONE ||
            (!node->same_word_right && fricative(node->right_phone)) ? 57 : 62;
        /* A2F reads the preceding acoustic phone even across a syllable
         * or word boundary (ger_ph_X), not the following vowel. */
        target[RELOQ_TARGET_A2F] =
            (node->left_phone == RELOQ_DE_PHONE_e || node->left_phone == RELOQ_DE_PHONE_i ||
             node->left_phone == RELOQ_DE_PHONE_I || node->left_phone == RELOQ_DE_PHONE_y ||
             node->left_phone == RELOQ_DE_PHONE_j ||
             node->left_phone == RELOQ_DE_PHONE_Q_aj || node->left_phone == RELOQ_DE_PHONE_Q_oj) ?
            48 : 52;
        target[RELOQ_TARGET_A3F] = 56;
        break;
    case RELOQ_DE_PHONE_h:
        /* /h/ has no AF or parallel noise; its aspiration follows the
         * neighboring vowel class and the oral tract remains permissive. */
        target[RELOQ_TARGET_AH] = vowel_source(node->right_phone) - 9;
        break;
    case RELOQ_DE_PHONE_w:
        /* German SPR /w/ is the voiced approximant, not the /v/ noise
         * route. It is particularly common in everyday German sentences. */
        oral(target, 400, 940, 2260, 3600, 3900);
        target[RELOQ_TARGET_AV] = node->same_syllable_left ?
            vowel_source(node->left_phone) : 50;
        target[RELOQ_TARGET_AH] = 34;
        break;
    default:
        return 0;
    }
    return 1;
}

int reloq_german_generic_voiced_stop_closure(
    enum reloq_german_phone phone,
    int32_t target[RELOQ_TARGET_STREAM_COUNT])
{
    if (!target) return -1;
    if (phone != RELOQ_DE_PHONE_b && phone != RELOQ_DE_PHONE_d &&
        phone != RELOQ_DE_PHONE_g && phone != RELOQ_DE_PHONE_J)
        return 0;
    target[RELOQ_TARGET_AV] = phone == RELOQ_DE_PHONE_J ? 35 : 20;
    target[RELOQ_TARGET_TL] = 35;
    if (phone == RELOQ_DE_PHONE_b) oral(target, 200, 1000, 2500, 3600, 4000);
    else if (phone == RELOQ_DE_PHONE_d)
        oral(target, 200, 1600, 2600, 4000, 4300);
    else if (phone == RELOQ_DE_PHONE_g)
        oral(target, 300, 1500, 1890, 3600, 3900);
    else oral(target, 200, 1600, 2600, 4000, 4300);
    return 1;
}

int reloq_german_generic_stop_noise(
    const struct reloq_german_phone_node *node, int syllable_onset,
    enum reloq_german_generic_noise_phase phase,
    int32_t target[RELOQ_TARGET_STREAM_COUNT])
{
    if (!node || !target || phase < RELOQ_DE_GENERIC_BURST ||
        phase > RELOQ_DE_GENERIC_FRICATION) return -1;
    const enum reloq_german_phone p = node->phone;
    const enum reloq_german_phone neighbor = syllable_onset ?
        node->right_phone : node->left_phone;
    const int front_velar = front(neighbor) &&
        (!syllable_onset || neighbor != RELOQ_DE_PHONE_E);
    const int affricate = p == RELOQ_DE_PHONE_P ||
        p == RELOQ_DE_PHONE_T || p == RELOQ_DE_PHONE_C ||
        p == RELOQ_DE_PHONE_J;
    if (phase == RELOQ_DE_GENERIC_FRICATION && !affricate) return 0;
    if (phase == RELOQ_DE_GENERIC_ASPIRATION && affricate) return 0;
    if (phase == RELOQ_DE_GENERIC_FRICATION) {
        switch (p) {
        case RELOQ_DE_PHONE_P:
            oral(target, 400, 1000, 2300, 3600, 3900);
            target[RELOQ_TARGET_AF] = 60;
            target[RELOQ_TARGET_AB] = 60;
            break;
        case RELOQ_DE_PHONE_T:
            oral(target, 300, 1500, 2600, 3400, 4400);
            target[RELOQ_TARGET_AF] = 65;
            target[RELOQ_TARGET_A5F] = 55;
            break;
        case RELOQ_DE_PHONE_C:
            oral(target, 300, 1800, 2400, 3600, 4000);
            target[RELOQ_TARGET_AF] = 56;
            target[RELOQ_TARGET_A2F] = 56;
            target[RELOQ_TARGET_A3F] = 62;
            target[RELOQ_TARGET_A4F] = 62;
            break;
        case RELOQ_DE_PHONE_J:
            oral(target, 300, 1700, 2300, 3600, 4000);
            target[RELOQ_TARGET_AF] = 57;
            target[RELOQ_TARGET_A3F] = 62;
            target[RELOQ_TARGET_A4F] = 59;
            target[RELOQ_TARGET_AV] = 40;
            break;
        default: return 0;
        }
        return 1;
    }
    if (phase == RELOQ_DE_GENERIC_ASPIRATION) {
        target[RELOQ_TARGET_AH] = p == RELOQ_DE_PHONE_p ? 49 :
            p == RELOQ_DE_PHONE_t ? 47 :
            p == RELOQ_DE_PHONE_k ? syllable_onset && !node->stress?48:45 : 0;
        return 1;
    }
    switch (p) {
    case RELOQ_DE_PHONE_p:
    case RELOQ_DE_PHONE_b:
        oral(target, p == RELOQ_DE_PHONE_b ? 200 : 350,
             1000, 2500, 3600, p == RELOQ_DE_PHONE_b ? 4000 : 3900);
        target[RELOQ_TARGET_AF] = p == RELOQ_DE_PHONE_b && syllable_onset ?
            56 : 60;
        target[RELOQ_TARGET_AB] = syllable_onset ?
            p == RELOQ_DE_PHONE_p && node->same_word_left ? 54 :
            p == RELOQ_DE_PHONE_p && back(neighbor) ? 66 : 64 :
            front(neighbor) ? 60 : 64;
        break;
    case RELOQ_DE_PHONE_t:
    case RELOQ_DE_PHONE_d:
        oral(target, 200, 1600, 2600, p == RELOQ_DE_PHONE_d ? 4000 : 3500,
             p == RELOQ_DE_PHONE_d ? 4300 : 4200);
        target[RELOQ_TARGET_AF] = !syllable_onset ? 48 :
            p == RELOQ_DE_PHONE_t ? 59 :
            neighbor == RELOQ_DE_PHONE_A ||
            neighbor == RELOQ_DE_PHONE_a ? 53 :
            neighbor == RELOQ_DE_PHONE_e ||
            neighbor == RELOQ_DE_PHONE_i ? 52 : 50;
        target[RELOQ_TARGET_A5F] = syllable_onset ?
            p == RELOQ_DE_PHONE_t ? 56 : 55 : 60;
        if (p==RELOQ_DE_PHONE_t && syllable_onset && neighbor==RELOQ_DE_PHONE_AT)
            target[RELOQ_TARGET_AF]=56;
        if (p==RELOQ_DE_PHONE_d && syllable_onset) {
            if (neighbor==RELOQ_DE_PHONE_AT) target[RELOQ_TARGET_AF]=48;
            else if (neighbor==RELOQ_DE_PHONE_R ||
                neighbor==RELOQ_DE_PHONE_Q_aj || neighbor==RELOQ_DE_PHONE_Q_aw ||
                neighbor==RELOQ_DE_PHONE_Q_a_TILDE) target[RELOQ_TARGET_AF]=53;
        }
        break;
    case RELOQ_DE_PHONE_k:
    case RELOQ_DE_PHONE_g:
        oral(target, 300, 1500, 2300, 3600, 3900);
        target[RELOQ_TARGET_AF] = !syllable_onset ? 62 :
            p == RELOQ_DE_PHONE_k ?
            front_velar ? 64 : back(neighbor) ? 62 :
            neighbor == RELOQ_DE_PHONE_E ? 54 : 60 :
            front_velar || neighbor == RELOQ_DE_PHONE_E ? 63 : 56;
        target[front_velar ? RELOQ_TARGET_A3F : RELOQ_TARGET_A2F] =
            syllable_onset && !front_velar ?
            p == RELOQ_DE_PHONE_g && neighbor == RELOQ_DE_PHONE_E ?
            65 : 70 : 60;
        if (p==RELOQ_DE_PHONE_k && syllable_onset) {
            const int high_front=neighbor==RELOQ_DE_PHONE_I ||
                neighbor==RELOQ_DE_PHONE_e || neighbor==RELOQ_DE_PHONE_i ||
                neighbor==RELOQ_DE_PHONE_y || neighbor==RELOQ_DE_PHONE_Y ||
                neighbor==RELOQ_DE_PHONE_Q_oe || neighbor==RELOQ_DE_PHONE_AT;
            target[RELOQ_TARGET_A2F]=high_front?0:70;
            target[RELOQ_TARGET_A3F]=high_front?60:0;
            target[RELOQ_TARGET_AF]=high_front?64:
                neighbor==RELOQ_DE_PHONE_E || neighbor==RELOQ_DE_PHONE_Q_OE?54:
                back(neighbor) || neighbor==RELOQ_DE_PHONE_Q_E_COLON?62:60;
        }
        if (p==RELOQ_DE_PHONE_g && syllable_onset) {
            /* The voiced velar's noise route has its own vowel classes.
             * In particular schwa uses the front route, while oe/OE and
             * open long E retain the back route. */
            const int high_front=neighbor==RELOQ_DE_PHONE_I ||
                neighbor==RELOQ_DE_PHONE_e || neighbor==RELOQ_DE_PHONE_i ||
                neighbor==RELOQ_DE_PHONE_y || neighbor==RELOQ_DE_PHONE_Y ||
                neighbor==RELOQ_DE_PHONE_AT;
            target[RELOQ_TARGET_AF]=high_front || neighbor==RELOQ_DE_PHONE_E?63:56;
            target[RELOQ_TARGET_A2F]=high_front?0:neighbor==RELOQ_DE_PHONE_E?65:70;
            target[RELOQ_TARGET_A3F]=high_front?60:0;
        }
        break;
    case RELOQ_DE_PHONE_P:
        target[RELOQ_TARGET_AF] = 60;
        target[RELOQ_TARGET_AB] = 67;
        break;
    case RELOQ_DE_PHONE_T:
    case RELOQ_DE_PHONE_C:
        target[RELOQ_TARGET_AF] = 66;
        target[RELOQ_TARGET_A5F] = p == RELOQ_DE_PHONE_C ? 50 : 53;
        break;
    case RELOQ_DE_PHONE_J:
        /* German final obstruents devoice: /J/ takes the /C/ coda burst. */
        target[RELOQ_TARGET_AF] = syllable_onset ? 51 : 66;
        target[RELOQ_TARGET_A5F] = syllable_onset ? 55 : 50;
        break;
    default:
        return 0;
    }
    return 1;
}
