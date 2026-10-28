#include "reloq_german_generic_noise.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static enum reloq_german_phone phone(char c)
{
    switch (c) {
    case 'A': return RELOQ_DE_PHONE_A;
    case 'E': return RELOQ_DE_PHONE_E;
    case 'I': return RELOQ_DE_PHONE_I;
    case 'O': return RELOQ_DE_PHONE_O;
    case 'U': return RELOQ_DE_PHONE_U;
    case 'a': return RELOQ_DE_PHONE_a;
    case 'e': return RELOQ_DE_PHONE_e;
    case 'i': return RELOQ_DE_PHONE_i;
    case 'o': return RELOQ_DE_PHONE_o;
    case 'u': return RELOQ_DE_PHONE_u;
    case 'p': return RELOQ_DE_PHONE_p;
    case 't': return RELOQ_DE_PHONE_t;
    case 'k': return RELOQ_DE_PHONE_k;
    case 'b': return RELOQ_DE_PHONE_b;
    case 'd': return RELOQ_DE_PHONE_d;
    case 'g': return RELOQ_DE_PHONE_g;
    case 'f': return RELOQ_DE_PHONE_f;
    case 'v': return RELOQ_DE_PHONE_v;
    case 'w': return RELOQ_DE_PHONE_w;
    case 's': return RELOQ_DE_PHONE_s;
    case 'z': return RELOQ_DE_PHONE_z;
    case 'S': return RELOQ_DE_PHONE_S;
    case 'Z': return RELOQ_DE_PHONE_Z;
    case 'x': return RELOQ_DE_PHONE_x;
    case 'X': return RELOQ_DE_PHONE_X;
    case 'h': return RELOQ_DE_PHONE_h;
    case 'C': return RELOQ_DE_PHONE_C;
    case 'T': return RELOQ_DE_PHONE_T;
    case 'P': return RELOQ_DE_PHONE_P;
    case 'J': return RELOQ_DE_PHONE_J;
    default: return RELOQ_DE_PHONE_NONE;
    }
}

static void profile(char consonant, char vowel, int initial, char phase,
                    int32_t values[RELOQ_TARGET_STREAM_COUNT])
{
    struct reloq_german_phone_node node = {0};
    node.phone = phone(consonant);
    node.left_phone = initial == 1 ? RELOQ_DE_PHONE_NONE :
        initial == 2 ? RELOQ_DE_PHONE_A : phone(vowel);
    node.right_phone = initial ? phone(vowel) : RELOQ_DE_PHONE_NONE;
    node.same_word_left = initial == 2;
    node.same_syllable_left = !initial;
    node.same_syllable_right = !!initial;
    assert(node.phone && phone(vowel));
    memset(values, 0, sizeof(int32_t) * RELOQ_TARGET_STREAM_COUNT);
    int result = phase == 'f' ?
        reloq_german_generic_fricative_noise(&node, values) :
        reloq_german_generic_stop_noise(
            &node, !!initial, phase == 'a' ? RELOQ_DE_GENERIC_ASPIRATION :
            phase == 'r' ? RELOQ_DE_GENERIC_FRICATION :
                           RELOQ_DE_GENERIC_BURST, values);
    assert(result == 1);
}

static void tests(void)
{
    int32_t x[RELOQ_TARGET_STREAM_COUNT];
    profile('s', 'A', 1, 'f', x);
    assert(x[RELOQ_TARGET_AF] == 62 && x[RELOQ_TARGET_A5F] == 50 &&
           !x[RELOQ_TARGET_AB] && x[RELOQ_TARGET_F4] == 3400);
    profile('S', 'i', 1, 'f', x);
    assert(x[RELOQ_TARGET_A2F] == 56 && x[RELOQ_TARGET_A3F] == 62 &&
           x[RELOQ_TARGET_A4F] == 62 && !x[RELOQ_TARGET_A5F]);
    profile('f', 'e', 0, 'f', x);
    assert(x[RELOQ_TARGET_AF] == 58 && x[RELOQ_TARGET_AB] == 60 &&
           x[RELOQ_TARGET_F2] == 1450);
    profile('x', 'u', 0, 'f', x);
    assert(x[RELOQ_TARGET_AF] == 52 && x[RELOQ_TARGET_A2F] == 70 &&
           x[RELOQ_TARGET_A4F] == 55);
    profile('w', 'A', 1, 'f', x);
    assert(!x[RELOQ_TARGET_AF] && !x[RELOQ_TARGET_AB] &&
           x[RELOQ_TARGET_AV] == 50);
    profile('z', 'i', 1, 'f', x);
    assert(x[RELOQ_TARGET_AF] == 66 && x[RELOQ_TARGET_A4F] == 50 &&
           x[RELOQ_TARGET_A5F] == 50 && !x[RELOQ_TARGET_AB]);
    profile('z', 'i', 0, 'f', x);
    assert(x[RELOQ_TARGET_AF] == 62 && !x[RELOQ_TARGET_A4F] &&
           !x[RELOQ_TARGET_AV]);
    profile('k', 'i', 1, 'b', x);
    assert(x[RELOQ_TARGET_AF] == 64 && x[RELOQ_TARGET_A3F] == 60 &&
           !x[RELOQ_TARGET_A2F]);
    profile('k', 'u', 1, 'b', x);
    assert(x[RELOQ_TARGET_AF] == 62 && x[RELOQ_TARGET_A2F] == 70 &&
           !x[RELOQ_TARGET_A3F]);
    profile('p', 'u', 1, 'a', x);
    assert(x[RELOQ_TARGET_AH] == 49 && !x[RELOQ_TARGET_AF] &&
           !x[RELOQ_TARGET_AB]);
    struct reloq_german_phone_node medial = {0};
    medial.phone = RELOQ_DE_PHONE_p;
    medial.left_phone = RELOQ_DE_PHONE_A;
    medial.right_phone = RELOQ_DE_PHONE_i;
    medial.same_word_left = medial.same_syllable_right = 1;
    memset(x, 0, sizeof x);
    assert(reloq_german_generic_stop_noise(&medial, 1,
           RELOQ_DE_GENERIC_BURST, x) == 1);
    assert(x[RELOQ_TARGET_AF] == 60 && x[RELOQ_TARGET_AB] == 54);
    profile('C', 'A', 1, 'r', x);
    assert(x[RELOQ_TARGET_AF] == 56 && x[RELOQ_TARGET_A2F] == 56 &&
           x[RELOQ_TARGET_A3F] == 62 && x[RELOQ_TARGET_A4F] == 62);
    assert(reloq_german_generic_fricative_noise(NULL, x) == -1);
    assert(reloq_german_generic_voiced_stop_closure(RELOQ_DE_PHONE_b, x) == 1);
    assert(x[RELOQ_TARGET_AV] == 20 && x[RELOQ_TARGET_TL] == 35);
}

int main(int argc, char **argv)
{
    if (argc == 1) { tests(); puts("generic noise routes: OK"); return 0; }
    if (argc != 5 || strcmp(argv[1], "--query") ||
        strlen(argv[2]) != 1 || strlen(argv[3]) != 1 ||
        strlen(argv[4]) != 2 || (argv[4][0] != 'i' &&
                                 argv[4][0] != 'c' && argv[4][0] != 'm'))
        return 2;
    int32_t x[RELOQ_TARGET_STREAM_COUNT];
    profile(argv[2][0], argv[3][0], argv[4][0] == 'i' ? 1 :
            argv[4][0] == 'm' ? 2 : 0, argv[4][1], x);
    for (int i = RELOQ_TARGET_F1; i <= RELOQ_TARGET_F5; ++i)
        printf("%s=%d ", reloq_target_stream_name(i), (int)x[i]);
    for (int i = RELOQ_TARGET_AV; i <= RELOQ_TARGET_AB; ++i)
        printf("%s=%d%c", reloq_target_stream_name(i), (int)x[i],
               i == RELOQ_TARGET_AB ? '\n' : ' ');
    return 0;
}
