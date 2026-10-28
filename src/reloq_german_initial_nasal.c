#include "reloq_german_initial_nasal.h"

/* Rounded vowel-frontness classes and place-specific nasal tract values.
 * These are phonetic coefficients; the table has no word or final-stop key. */
static int vowel_profile(enum reloq_german_phone vowel, int is_m,
                         int32_t *f2, int32_t *f3, int32_t *f4)
{
    if (is_m) {
        *f4 = 3600;
        switch (vowel) {
        case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_a:
            *f2 = 1000; *f3 = 2500; return 1;
        case RELOQ_DE_PHONE_E: case RELOQ_DE_PHONE_I:
        case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
            *f2 = 1300; *f3 = 2500; return 1;
        case RELOQ_DE_PHONE_O: case RELOQ_DE_PHONE_U:
        case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
            *f2 = 700; *f3 = 2300; return 1;
        default: return 0;
        }
    }
    *f3 = 2500;
    *f4 = 3350;
    switch (vowel) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_a:
        *f2 = 1600; return 1;
    case RELOQ_DE_PHONE_E:
        *f2 = 1700; *f3 = 2600; return 1;
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_e:
    case RELOQ_DE_PHONE_i:
        *f2 = 1850; *f3 = 2800; *f4 = 3400;
        return 1;
    case RELOQ_DE_PHONE_O:
        *f2 = 1500; return 1;
    case RELOQ_DE_PHONE_U:
        *f2 = 1500; return 1;
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
        *f2 = 1400; return 1;
    default: return 0;
    }
}

int reloq_german_initial_nasal_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t origin_tick, struct reloq_target_builder *builder,
    int64_t *first_send_offset)
{
    if (!phones || !builder || !first_send_offset || !phone_count ||
        origin_tick < 0)
        return -1;
    if (phone_count != 2 && phone_count != 3) return 1;
    const int is_m = phones[0].phone == RELOQ_DE_PHONE_m;
    if (!is_m && phones[0].phone != RELOQ_DE_PHONE_n) return 1;
    if (phone_count == 3 && phones[2].phone != RELOQ_DE_PHONE_p &&
        phones[2].phone != RELOQ_DE_PHONE_t &&
        phones[2].phone != RELOQ_DE_PHONE_k) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != 0 || phones[i].syllable_index !=
                phones[0].syllable_index ||
            phones[i].syllable_in_word != 0 || phones[i].stress != 1 ||
            !phones[i].stress_explicit) return 1;

    int32_t f2, f3, f4;
    if (!vowel_profile(phones[1].phone, is_m,
                       &f2, &f3, &f4)) return 1;
    const int64_t end = is_m ? 85 : 70;
    const int32_t f1 = is_m ? 300 : 200;
    const int32_t f5 = is_m ? 3900 : 3800;
    const int32_t b2 = is_m ? 150 : 200;
    const int32_t b3 = is_m ? 105 : 80;
    const int32_t av = is_m ? 51 : 48;
    struct reloq_target_event events[47];
    size_t n = 0;
#define HOLD(stream, value) do { \
    events[n++] = (struct reloq_target_event){stream, 0, value}; \
    events[n++] = (struct reloq_target_event){stream, end, value}; \
} while (0)
    HOLD(RELOQ_TARGET_F1, f1);
    HOLD(RELOQ_TARGET_F2, f2);
    HOLD(RELOQ_TARGET_F3, f3);
    HOLD(RELOQ_TARGET_F4, f4);
    HOLD(RELOQ_TARGET_F5, f5);
    HOLD(RELOQ_TARGET_B1, 120);
    HOLD(RELOQ_TARGET_B2, b2);
    HOLD(RELOQ_TARGET_B3, b3);
    HOLD(RELOQ_TARGET_B4, 330);
    HOLD(RELOQ_TARGET_B5, 260);
    HOLD(RELOQ_TARGET_FNP, 200);
    events[n++] = (struct reloq_target_event){RELOQ_TARGET_FNZ, end / 2, 350};
    HOLD(RELOQ_TARGET_TL, 0);
    HOLD(RELOQ_TARGET_OQ, 56);
    HOLD(RELOQ_TARGET_DI, 0);
    HOLD(RELOQ_TARGET_AV, av);
    HOLD(RELOQ_TARGET_AH, 0);
    HOLD(RELOQ_TARGET_AF, 0);
    for (enum reloq_target_stream stream = RELOQ_TARGET_A1F;
         stream <= RELOQ_TARGET_AB; ++stream)
        HOLD(stream, 0);
#undef HOLD
    if (n != sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder, origin_tick, events, n))
        return -1;
    *first_send_offset = end;
    return 0;
}
