#include "reloq_german_generic_stop_connection.h"

static int stop(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_p || p == RELOQ_DE_PHONE_t ||
           p == RELOQ_DE_PHONE_k;
}

static int vowel(enum reloq_german_phone p)
{
    switch (p) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
    case RELOQ_DE_PHONE_y: case RELOQ_DE_PHONE_Y:
    case RELOQ_DE_PHONE_AT: case RELOQ_DE_PHONE_Q_E_COLON:
    case RELOQ_DE_PHONE_Q_oe: case RELOQ_DE_PHONE_Q_OE:
    case RELOQ_DE_PHONE_Q_aj: case RELOQ_DE_PHONE_Q_aw:
    case RELOQ_DE_PHONE_Q_oj:
    case RELOQ_DE_PHONE_Q_a_TILDE:
    case RELOQ_DE_PHONE_Q_E_TILDE:
    case RELOQ_DE_PHONE_Q_o_TILDE:
    case RELOQ_DE_PHONE_Q_oe_TILDE: return 1;
    default: return 0;
    }
}

static int index(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_p ? 0 :
           p == RELOQ_DE_PHONE_t ? 1 :
           p == RELOQ_DE_PHONE_k ? 2 : -1;
}

int reloq_german_generic_connected_coda(
    const struct reloq_german_phone_node *node,
    struct reloq_german_generic_connected_coda *out)
{
    if (!node || !out) return -1;
    if (!stop(node->phone) || !stop(node->right_phone) ||
        !node->same_syllable_left || !vowel(node->left_phone) ||
        node->same_word_right || node->stress != 1) return 0;
    /* Observed two-word classes: a nonfinal coda releases earlier or later
     * according to the next stop. Values are source/phone timing, never a
     * word lookup; other syllable contexts retain their generic timings. */
    static const int32_t spans[3][3] = {
        {40, 100, 100}, {58, 30, 58}, {52, 52, 65}
    };
    const int left = index(node->phone), right = index(node->right_phone);
    *out = (struct reloq_german_generic_connected_coda){
        spans[left][right], spans[left][right] -
            (left == 0 ? 10 : 12), left == right
    };
    return 1;
}

int reloq_german_generic_connected_onset(
    const struct reloq_german_phone_node *node,
    int32_t *burst_at_ticks)
{
    if (!node || !burst_at_ticks) return -1;
    if (!stop(node->phone) || node->same_word_left ||
        node->left_phone == RELOQ_DE_PHONE_NONE ||
        !node->same_syllable_right || !vowel(node->right_phone) ||
        node->stress != 1) return 0;
    static const int32_t after_stop[3][3] = {
        {91, 69, 56}, {91, 69, 105}, {91, 88, 56}
    };
    const int left = index(node->left_phone), right = index(node->phone);
    /* Vowel-final first words have a different, shorter coda than a
     * consonant-final first word. The previous word's measured C vowel
     * schedule also differs from this generic planner; these durations
     * account for that broad shape without injecting a silence token. */
    *burst_at_ticks = left >= 0 ? after_stop[left][right] :
        vowel(node->left_phone) ?
            (right == 0 ? 80 : right == 1 ? 58 : 65) :
            (right == 0 ? 72 : right == 1 ? 64 : 60);
    return 1;
}

static int vowel_class(enum reloq_german_phone p)
{
    switch (p) {
    case RELOQ_DE_PHONE_E: case RELOQ_DE_PHONE_I:
    case RELOQ_DE_PHONE_Y: case RELOQ_DE_PHONE_e:
    case RELOQ_DE_PHONE_i: return 1;
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_o:
    case RELOQ_DE_PHONE_u: return 2;
    case RELOQ_DE_PHONE_a: return 3;
    default: return 0;
    }
}

int reloq_german_generic_cluster_vowel_ticks(
    const struct reloq_german_phone_node *node, int32_t *out)
{
    if (!node || !out) return -1;
    if (node->stress != 1 || !node->same_syllable_right ||
        !node->same_word_right || !stop(node->right_phone)) return 0;
    int32_t span;
    switch (node->phone) {
    case RELOQ_DE_PHONE_A: span = 87; break;
    case RELOQ_DE_PHONE_E: span = 94; break;
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_Y: span = 88; break;
    case RELOQ_DE_PHONE_U: span = 75; break;
    case RELOQ_DE_PHONE_a: span = 151; break;
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_o: span = 163; break;
    case RELOQ_DE_PHONE_i: case RELOQ_DE_PHONE_u: span = 129; break;
    default: return 0;
    }
    *out = span;
    return 1;
}

int reloq_german_generic_cluster_stop(
    const struct reloq_german_phone_node *node, int first,
    struct reloq_german_generic_cluster_stop *out)
{
    if (!node || !out || (first != 0 && first != 1)) return -1;
    if (node->stress != 1 || !node->same_word_left ||
        !node->same_syllable_left || !stop(node->phone)) return 0;
    const int phone = index(node->phone);
    struct reloq_german_generic_cluster_stop result = {0};
    if (first) {
        const int left = vowel_class(node->left_phone);
        const int following = index(node->right_phone);
        if (!left || following < 0 || !node->same_word_right ||
            !node->same_syllable_right) return 0;
        /* Once the preceding vowel's source ends, the closure and release
         * interval depends on the two stop places, but not on the tested
         * vowel class.  This is distinct from the pre-Klatt send-window
         * duration, whose start overlaps the preceding voiced vowel. */
        static const int32_t spans[3][3] = {
            {40, 100, 100}, {42, 30, 42}, {73, 73, 65}
        };
        result.span_ticks = spans[phone][following];
        result.release_formants[0] = 300;
        result.release_formants[1] = phone == 0 ?
            (following == 1 ? 1300 : 1000) :
            phone == 1 ? 1600 : following == 1 ? 1800 : 1500;
        result.release_formants[2] = phone == 2 ? 2300 : 2500;
        result.merge_with_next = phone == following;
        if (!result.merge_with_next) {
            result.burst_ticks = phone == 0 ? 10 : 8;
            if (phone == 0) {
                result.ab_level = following == 2 ? 54 : 60;
                result.af_level = 60;
            } else if (phone == 1) {
                result.af_level = 70;
                if (following == 2) {
                    result.a2f_level = 50;
                    result.a5f_level = 40;
                    result.carry_aspiration_ticks = 30;
                    result.carry_aspiration_level = 51;
                }
            } else {
                result.a2f_level = 60;
                result.af_level = following == 1 ? 54 : 65;
                if (following == 1) {
                    result.carry_aspiration_ticks = 20;
                    result.carry_aspiration_level = 50;
                }
            }
        }
    } else {
        const int previous = index(node->left_phone);
        if (previous < 0 || node->same_word_right) return 0;
        static const int32_t spans[3][3] = {
            {126, 61, 123}, {126, 66, 152}, {126, 100, 123}
        };
        result.span_ticks = spans[previous][phone];
        result.release_formants[0] = 300;
        result.release_formants[1] = phone == 0 ? 1000 :
            phone == 1 ? 1600 : 1500;
        result.release_formants[2] = phone == 2 ? 2300 : 2500;
        result.burst_ticks = phone == 0 ? 10 : phone == 1 ? 8 : 12;
        result.af_level = phone == 0 ? 60 : phone == 1 ? 48 : 62;
        result.ab_level = phone == 0 ? 64 : 0;
        result.a5f_level = phone == 1 ? 60 : 0;
        result.a2f_level = phone == 2 ? 60 : 0;
        result.following_aspiration_level = phone == 0 ? 35 : 48;
        /* A following initial stop reshapes an alveolar final release:
         * /t+p/ is diffuse, /t+k/ distributes its burst between lower and
         * upper resonators, and /t+t/ merges without a middle release. */
        if (phone == 1 && !node->same_word_right &&
            node->right_phone != RELOQ_DE_PHONE_NONE) {
            const enum reloq_german_phone right_phone = node->right_phone;
            const int right = index(right_phone);
            if (right >= 0 || right_phone == RELOQ_DE_PHONE_b) {
                result.span_ticks = right == 1 ?
                    (previous == 2 ? 92 : 53) : spans[previous][phone];
                result.merge_with_next = right == 1;
                result.af_level = result.merge_with_next ? 0 : 70;
                result.a5f_level = right == 2 ? 40 : 0;
                result.a2f_level = right == 2 ? 50 : 0;
                result.following_aspiration_level =
                    result.merge_with_next ? 0 : 51;
            } else if (right_phone == RELOQ_DE_PHONE_d) {
                result.span_ticks = previous == 2 ? 90 : 71;
                result.merge_with_next = 1;
                result.af_level = result.a5f_level = 0;
                result.following_aspiration_level = 0;
            } else if (right_phone == RELOQ_DE_PHONE_g) {
                result.af_level = 70;
                result.a2f_level = 50;
                result.a5f_level = 40;
                result.following_aspiration_level = 0;
            } else if (right_phone == RELOQ_DE_PHONE_m) {
                result.af_level = 54;
                result.a5f_level = 51;
                result.following_aspiration_level = 51;
            } else if (right_phone == RELOQ_DE_PHONE_s) {
                result.span_ticks = previous == 2 ? 73 : 54;
                result.af_level = 66;
                result.a5f_level = 53;
                result.following_aspiration_level = 0;
            } else if (vowel(right_phone)) {
                result.af_level = result.a5f_level = 56;
                result.following_aspiration_level = 0;
            }
        }
    }
    *out = result;
    return 1;
}
