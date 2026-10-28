#include "reloq_german_generic_diphthong.h"

static int stop(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_p || p == RELOQ_DE_PHONE_t ||
           p == RELOQ_DE_PHONE_k || p == RELOQ_DE_PHONE_b ||
           p == RELOQ_DE_PHONE_d || p == RELOQ_DE_PHONE_g;
}

static int fricative(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_s || p == RELOQ_DE_PHONE_S ||
           p == RELOQ_DE_PHONE_x || p == RELOQ_DE_PHONE_X ||
           p == RELOQ_DE_PHONE_f || p == RELOQ_DE_PHONE_z ||
           p == RELOQ_DE_PHONE_Z || p == RELOQ_DE_PHONE_v;
}

int reloq_german_generic_diphthong_plan(
    const struct reloq_german_phone_node *node,
    int connected_word,
    struct reloq_german_generic_diphthong *out)
{
    if (!node || !out || (connected_word != 0 && connected_word != 1))
        return -1;
    const enum reloq_german_phone p = node->phone;
    struct reloq_german_generic_diphthong gesture = {{0}, 0, 0, 20, 0, 0};
    if (p == RELOQ_DE_PHONE_Q_aj) {
        gesture.endpoint[0] = 420;
        gesture.endpoint[1] = 1900;
        gesture.endpoint[2] = 2450;
        gesture.glide_ticks = 75;
        gesture.final_av = node->stress == 0 ? 42 : 45;
    } else if (p == RELOQ_DE_PHONE_Q_aw) {
        gesture.endpoint[0] = 400;
        gesture.endpoint[1] = 940;
        gesture.endpoint[2] = 2260;
        gesture.glide_ticks = 75;
        gesture.final_av = node->stress == 0 ? 45 : 48;
    } else if (p == RELOQ_DE_PHONE_Q_oj) {
        gesture.endpoint[0] = 350;
        gesture.endpoint[1] = 1900;
        gesture.endpoint[2] = 2450;
        gesture.glide_ticks = 45;
        gesture.final_av = node->stress == 0 ? 48 : 51;
    } else return 0;

    /* A complete utterance leaves the nucleus long. Within connected text,
     * a following syllable compresses it, whereas a coda reserves distinct
     * space for its anticipatory formant movement. This one structural rule
     * applies to every word with the same phonetic neighbors. */
    if (!node->same_syllable_right) {
        gesture.duration_ticks = node->right_phone == RELOQ_DE_PHONE_NONE ?
            (node->stress == 1 ? 232 :
             p == RELOQ_DE_PHONE_Q_aj ? 203 : 188) :
            node->same_word_right ? 159 : 174;
    } else {
        const enum reloq_german_phone c = node->right_phone;
        if (stop(c)) {
            gesture.duration_ticks = node->stress == 2 ? 174 : 188;
            gesture.before_coda_ticks = 15;
            if (p == RELOQ_DE_PHONE_Q_aw &&
                (c == RELOQ_DE_PHONE_t || c == RELOQ_DE_PHONE_d))
                gesture.before_coda_ticks = 40;
        } else if (fricative(c)) {
            gesture.duration_ticks = node->stress == 2 ?
                (p == RELOQ_DE_PHONE_Q_aw ? 174 : 191) :
                node->stress == 0 ? 188 :
                p == RELOQ_DE_PHONE_Q_aw ? 188 : 207;
            gesture.before_coda_ticks =
                p == RELOQ_DE_PHONE_Q_aw &&
                (c == RELOQ_DE_PHONE_s || c == RELOQ_DE_PHONE_z ||
                 c == RELOQ_DE_PHONE_X) ?
                40 : c == RELOQ_DE_PHONE_X ? 32 : 15;
        } else if (c == RELOQ_DE_PHONE_n || c == RELOQ_DE_PHONE_m ||
                   c == RELOQ_DE_PHONE_G) {
            gesture.duration_ticks = node->stress == 1 ? 203 :
                node->stress == 2 ? 174 : 188;
            gesture.before_coda_ticks =
                p == RELOQ_DE_PHONE_Q_aw && c == RELOQ_DE_PHONE_n ?
                20 : 15;
            if (p == RELOQ_DE_PHONE_Q_aw && c == RELOQ_DE_PHONE_m)
                gesture.endpoint[1] = 975;
            if (p == RELOQ_DE_PHONE_Q_oj && c == RELOQ_DE_PHONE_m) {
                gesture.endpoint[1] = 1780;
                gesture.endpoint[2] = 2400;
            }
        } else {
            gesture.duration_ticks = node->stress == 2 ? 174 :
                node->stress == 1 ? 203 : 188;
            gesture.before_coda_ticks = 15;
        }
    }
    /* The closing j/w has its own anticipatory tract target. Select it
     * from the already adjusted next phone, including retained voiced
     * closures; spelling or an earlier unvoiced substitute is insufficient. */
    if(node->same_word_right) {
        const enum reloq_german_phone c=node->right_phone;
        if(p==RELOQ_DE_PHONE_Q_aw) {
            if(c==RELOQ_DE_PHONE_p || c==RELOQ_DE_PHONE_b || c==RELOQ_DE_PHONE_f ||
               c==RELOQ_DE_PHONE_v || c==RELOQ_DE_PHONE_m || c==RELOQ_DE_PHONE_w)
                gesture.endpoint[1]=975;
        } else if(c==RELOQ_DE_PHONE_b || c==RELOQ_DE_PHONE_m) {
            gesture.endpoint[1]=1780;gesture.endpoint[2]=2400;
        } else if(c==RELOQ_DE_PHONE_k || c==RELOQ_DE_PHONE_g || c==RELOQ_DE_PHONE_X)
            gesture.endpoint[1]=1950;
        else if(c==RELOQ_DE_PHONE_f || c==RELOQ_DE_PHONE_v ||
                (p==RELOQ_DE_PHONE_Q_oj && c==RELOQ_DE_PHONE_S))
            gesture.endpoint[1]=1800;
    }
    /* The same final coda is shorter when speech continues into an
     * unpunctuated next word. Keep this measured connected-speech gesture
     * on primary-stressed syllables; the other stress classes need their
     * own contextual timing observations. */
    if (connected_word && node->stress == 1 &&
        node->same_syllable_right) {
        gesture.duration_ticks =
            fricative(node->right_phone) &&
            p != RELOQ_DE_PHONE_Q_aw ? 159 : 145;
    }
    *out = gesture;
    return 1;
}
