#include "reloq_german_generic_transition.h"

static int nasal(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_m || phone == RELOQ_DE_PHONE_n ||
           phone == RELOQ_DE_PHONE_G;
}

static int nasalized_vowel(enum reloq_german_phone phone)
{
    return phone >= RELOQ_DE_PHONE_Q_a_TILDE &&
           phone <= RELOQ_DE_PHONE_Q_oe_TILDE;
}

static int vowel(enum reloq_german_phone phone);

int reloq_german_generic_prenasal_bandwidth(
    const struct reloq_german_phone_node *vowel,
    int32_t targets[RELOQ_TARGET_STREAM_COUNT])
{
    if (!vowel || !targets) return -1;
    /* The adjacent nasal also colours the preceding vowel when it starts
     * the next syllable of the same word.  Do not carry this anticipation
     * across an independent word boundary. */
    if ((!vowel->same_syllable_right && !vowel->same_word_right) ||
        !nasal(vowel->right_phone)) return 0;
    int32_t b1;
    int32_t b2 = 100;
    switch (vowel->phone) {
    case RELOQ_DE_PHONE_E: case RELOQ_DE_PHONE_I:
    case RELOQ_DE_PHONE_e:
        b1 = 220;
        break;
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_i: case RELOQ_DE_PHONE_AT:
        b1 = 270;
        break;
    case RELOQ_DE_PHONE_O: case RELOQ_DE_PHONE_U:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
        b1 = 320;
        b2 = 200;
        break;
    default:
        return 0;
    }
    targets[RELOQ_TARGET_B1] = b1;
    targets[RELOQ_TARGET_B2] = b2;
    return 1;
}

int reloq_german_generic_nasal_transition(
    const struct reloq_german_phone_node *node, int32_t phone_ticks,
    struct reloq_german_generic_nasal_transition *out)
{
    if (!node || !out) return -1;
    if (!nasal(node->phone)) return 0;
    if (phone_ticks <= 16) return -1;
    if (node->same_syllable_left && nasalized_vowel(node->left_phone))
        return 0;
    /* The same midpoint rule applies to the short inter-syllabic /n/.
     * Its 15-ms peak belongs to a 30-ms interval, not a 68-ms interval. */
    int32_t rise = phone_ticks / 2;
    if (rise < 8) rise = 8; /* Caller places the oral start at tick + 7. */
    *out = (struct reloq_german_generic_nasal_transition){
        200, node->phone == RELOQ_DE_PHONE_G ? 400 : 350, rise
    };
    return 1;
}

static int front_high_or_reduced(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_I || phone == RELOQ_DE_PHONE_U ||
           phone == RELOQ_DE_PHONE_e || phone == RELOQ_DE_PHONE_i ||
           phone == RELOQ_DE_PHONE_u || phone == RELOQ_DE_PHONE_y ||
           phone == RELOQ_DE_PHONE_Y || phone == RELOQ_DE_PHONE_AT;
}

static int vowel(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_A || phone == RELOQ_DE_PHONE_E ||
           phone == RELOQ_DE_PHONE_I || phone == RELOQ_DE_PHONE_O ||
           phone == RELOQ_DE_PHONE_U || phone == RELOQ_DE_PHONE_a ||
           phone == RELOQ_DE_PHONE_e || phone == RELOQ_DE_PHONE_i ||
           phone == RELOQ_DE_PHONE_o || phone == RELOQ_DE_PHONE_u ||
           phone == RELOQ_DE_PHONE_y || phone == RELOQ_DE_PHONE_Y ||
           phone == RELOQ_DE_PHONE_AT || phone == RELOQ_DE_PHONE_R ||
           (phone >= RELOQ_DE_PHONE_Q_E_COLON &&
            phone <= RELOQ_DE_PHONE_Q_oe_TILDE);
}

static int sibilant_neighbor(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_s || phone == RELOQ_DE_PHONE_S ||
           phone == RELOQ_DE_PHONE_z || phone == RELOQ_DE_PHONE_Z ||
           phone == RELOQ_DE_PHONE_f || phone == RELOQ_DE_PHONE_v ||
           phone == RELOQ_DE_PHONE_x || phone == RELOQ_DE_PHONE_X;
}

int reloq_german_generic_fricative_ticks(
    const struct reloq_german_phone_node *node, int phrase_final,
    int following_onset_cluster,
    int32_t *out_ticks)
{
    if (!node || !out_ticks || (phrase_final != 0 && phrase_final != 1) ||
        (following_onset_cluster != 0 && following_onset_cluster != 1))
        return -1;
    /* Terminal Z retains its oral/noise identity, but has no voiced
     * plateau. Its 99-ms body also controls the preceding vowel budget. */
    if (node->phone==RELOQ_DE_PHONE_Z && !node->same_word_right) {
        *out_ticks=99;
        return 1;
    }
    if (node->phone != RELOQ_DE_PHONE_s &&
        node->phone != RELOQ_DE_PHONE_f &&
        node->phone != RELOQ_DE_PHONE_x &&
        node->phone != RELOQ_DE_PHONE_S &&
        node->phone != RELOQ_DE_PHONE_X) return 0;
    const int onset = node->same_syllable_right &&
        vowel(node->right_phone) &&
        !node->same_syllable_left;
    const int coda = node->same_syllable_left &&
        vowel(node->left_phone) &&
        !node->same_syllable_right;
    const int stop_s = node->phone==RELOQ_DE_PHONE_s &&
        node->same_syllable_left && !node->same_syllable_right &&
        (node->left_phone==RELOQ_DE_PHONE_p || node->left_phone==RELOQ_DE_PHONE_k ||
         node->left_phone==RELOQ_DE_PHONE_t);
    const int clustered_s=node->phone==RELOQ_DE_PHONE_s &&
        node->same_syllable_left && !node->same_syllable_right;
    const int nasal_s=clustered_s && nasal(node->left_phone);
    const int fricative_s=clustered_s && sibilant_neighbor(node->left_phone);
    const int lateral_s=clustered_s && node->left_phone==RELOQ_DE_PHONE_l;
    /* A coda cluster keeps its own s gesture and the separate 30-ms
     * phrase release. These classes also apply to unfamiliar spellings. */
    if (onset) {
        switch (node->phone) {
        case RELOQ_DE_PHONE_s:
            *out_ticks = node->stress<=1 ? (node->stress?125:85) :
                100;
            if(node->same_word_left &&
                (node->left_phone==RELOQ_DE_PHONE_s || node->left_phone==RELOQ_DE_PHONE_z))
                *out_ticks=70;
            break;
        case RELOQ_DE_PHONE_f:
            /* The preceding excitation class takes priority over accent.
             * ger_ph_f_dur: fricative=95, nasal=105, including word edges. */
            *out_ticks = sibilant_neighbor(node->left_phone) ? 95 :
                node->left_phone==RELOQ_DE_PHONE_m ||
                node->left_phone==RELOQ_DE_PHONE_n ||
                node->left_phone==RELOQ_DE_PHONE_G ? 105 :
                node->stress ? 118 : 95;
            break;
        case RELOQ_DE_PHONE_x:
            *out_ticks = node->right_phone == RELOQ_DE_PHONE_AT ? 80 : 95;
            break;
        case RELOQ_DE_PHONE_S:
            *out_ticks = node->same_word_left ?
                (node->right_phone == RELOQ_DE_PHONE_AT ? 95 : 125) :
                (front_high_or_reduced(node->right_phone) ||
                 nasalized_vowel(node->right_phone) ? 87 : 110);
            break;
        case RELOQ_DE_PHONE_X: *out_ticks = 90; break;
        default: return 0;
        }
        return 1;
    }
    if (!coda && !stop_s && !nasal_s && !fricative_s && !lateral_s) return 0;
    if (phrase_final) {
        *out_ticks = nasal_s ? (node->left_phone==RELOQ_DE_PHONE_n?120:150) :
                     fricative_s ? 135 : stop_s || lateral_s ? 125 : node->phone == RELOQ_DE_PHONE_s ||
                     node->phone == RELOQ_DE_PHONE_x ? 155 :
                     node->phone == RELOQ_DE_PHONE_f ? 150 : 160;
        return 1;
    }
    if (fricative_s) {*out_ticks=70;return 1;}
    if(stop_s && node->left_phone==RELOQ_DE_PHONE_t) {
        const enum reloq_german_phone next=node->right_phone;
        *out_ticks=!node->same_word_right &&
            (vowel(next)||next==RELOQ_DE_PHONE_h)?96:
            !node->same_word_right && (next==RELOQ_DE_PHONE_m ||
             next==RELOQ_DE_PHONE_n || next==RELOQ_DE_PHONE_G)?
                (node->stress?102:85):80;
        return 1;
    }
    const int connected_word = !node->same_word_right;
    switch (node->phone) {
    case RELOQ_DE_PHONE_s:
        *out_ticks = !connected_word ? 64 :
            stop_s && following_onset_cluster ? 95 :
            (stop_s || lateral_s || nasal_s) && !vowel(node->right_phone) &&
                node->right_phone!=RELOQ_DE_PHONE_h &&
                node->right_phone!=RELOQ_DE_PHONE_m &&
                node->right_phone!=RELOQ_DE_PHONE_n &&
                node->right_phone!=RELOQ_DE_PHONE_G ? (nasal_s?76:66) :
            sibilant_neighbor(node->right_phone) ? 85 :
            node->right_phone == RELOQ_DE_PHONE_m ||
            node->right_phone == RELOQ_DE_PHONE_n ||
            node->right_phone == RELOQ_DE_PHONE_G ?
                node->stress == 0 ? 85 : 102 :
            node->right_phone == RELOQ_DE_PHONE_h ||
            node->right_phone == RELOQ_DE_PHONE_R ||
            vowel(node->right_phone) ? 96 :
            node->stress == 0 ? 75 : 96;
        break;
    case RELOQ_DE_PHONE_f:
        *out_ticks = !connected_word ? 95 :
            following_onset_cluster ? 60 :
            node->right_phone == RELOQ_DE_PHONE_l ? 110 :
            node->right_phone == RELOQ_DE_PHONE_r ? 95 : 118;
        break;
    case RELOQ_DE_PHONE_x:
        *out_ticks = !connected_word ?
            sibilant_neighbor(node->right_phone) ? 80 :
            node->right_phone == RELOQ_DE_PHONE_m ||
            node->right_phone == RELOQ_DE_PHONE_n ||
            node->right_phone == RELOQ_DE_PHONE_G ||
            node->right_phone == RELOQ_DE_PHONE_l ||
            node->right_phone == RELOQ_DE_PHONE_r ? 78 : 60 :
            node->right_phone == RELOQ_DE_PHONE_l ||
            node->right_phone == RELOQ_DE_PHONE_m ||
            node->right_phone == RELOQ_DE_PHONE_n ||
            node->right_phone == RELOQ_DE_PHONE_r ?
                node->stress == 0 ? 90 : 128 :
            node->right_phone == RELOQ_DE_PHONE_h ||
            sibilant_neighbor(node->right_phone) ? 80 :
            node->right_phone == RELOQ_DE_PHONE_R ||
            vowel(node->right_phone) ? 95 : 60;
        break;
    case RELOQ_DE_PHONE_S:
        *out_ticks = !connected_word ? 80 :
            node->right_phone == RELOQ_DE_PHONE_S ? 70 :
            node->right_phone == RELOQ_DE_PHONE_h ||
            node->right_phone == RELOQ_DE_PHONE_R ||
            vowel(node->right_phone) ? 125 : 110;
        break;
    case RELOQ_DE_PHONE_X:
        *out_ticks = !connected_word ? 90 :
            node->right_phone == RELOQ_DE_PHONE_S ? 85 :
            node->right_phone == RELOQ_DE_PHONE_s ||
            node->right_phone == RELOQ_DE_PHONE_f ||
            node->right_phone == RELOQ_DE_PHONE_x ? 70 : 90;
        break;
    default: return 0;
    }
    return 1;
}
