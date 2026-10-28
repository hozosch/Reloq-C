#include "reloq_german_terminal_source.h"

#include <limits.h>

struct terminal_vowel {
    enum reloq_german_phone phone;
    int32_t bare_av;
    int32_t p_ab;
    int32_t k_ah;
    int8_t k_front;
};

/* Phone-class release levels. The release timing and stream topology are
 * shared gestures; these small vowel coefficients do not encode utterances. */
static const struct terminal_vowel vowels[] = {
    {RELOQ_DE_PHONE_A, 49, 64, 48, 0},
    {RELOQ_DE_PHONE_E, 50, 60, 48, 1},
    {RELOQ_DE_PHONE_I, 51, 60, 53, 1},
    {RELOQ_DE_PHONE_O, 51, 64, 48, 0},
    {RELOQ_DE_PHONE_U, 52, 64, 48, 0},
    {RELOQ_DE_PHONE_a, 48, 64, 48, 0},
    {RELOQ_DE_PHONE_e, 49, 60, 51, 1},
    {RELOQ_DE_PHONE_i, 51, 60, 51, 1},
    {RELOQ_DE_PHONE_o, 52, 64, 48, 0},
    {RELOQ_DE_PHONE_u, 53, 64, 48, 0}
};

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_terminal_source_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t vowel_closure_tick,
    struct reloq_target_builder *builder, int64_t *release_start_tick)
{
    if (!phones || !builder || !release_start_tick || !phone_count ||
        vowel_index >= phone_count || vowel_closure_tick < 0) return -1;
    if (phone_count > 3 || vowel_index > 1 ||
        phone_count - vowel_index > 2 ||
        (vowel_index && phones[0].phone != RELOQ_DE_PHONE_h &&
         phones[0].phone != RELOQ_DE_PHONE_p &&
         phones[0].phone != RELOQ_DE_PHONE_t &&
         phones[0].phone != RELOQ_DE_PHONE_k &&
         phones[0].phone != RELOQ_DE_PHONE_b &&
         phones[0].phone != RELOQ_DE_PHONE_d &&
         phones[0].phone != RELOQ_DE_PHONE_g &&
         phones[0].phone != RELOQ_DE_PHONE_m &&
         phones[0].phone != RELOQ_DE_PHONE_n)) return 1;

    const struct reloq_german_phone_node *vowel = &phones[vowel_index];
    if (vowel->word_index != 0 || vowel->syllable_in_word != 0 ||
        vowel->stress != 1 || !vowel->stress_explicit) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != vowel->word_index ||
            phones[i].syllable_index != vowel->syllable_index ||
            phones[i].stress != 1 || !phones[i].stress_explicit) return 1;
    const struct terminal_vowel *voice = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (vowels[i].phone == vowel->phone) {
            voice = &vowels[i];
            break;
        }
    if (!voice) return 1;

    enum reloq_german_phone coda = phone_count - vowel_index == 2 ?
        phones[vowel_index + 1].phone : RELOQ_DE_PHONE_NONE;
    int64_t coda_to_release = 0;
    int32_t ah = 35, af = 0, av = 0, active_gain = 0;
    enum reloq_target_stream active = RELOQ_TARGET_AB;
    if (coda == RELOQ_DE_PHONE_NONE) {
        av = voice->bare_av;
    } else if (coda == RELOQ_DE_PHONE_p) {
        coda_to_release = 125;
        af = 60;
        active = RELOQ_TARGET_AB;
        active_gain = voice->p_ab;
    } else if (coda == RELOQ_DE_PHONE_t) {
        coda_to_release = 108;
        ah = 48;
        af = 48;
        active = RELOQ_TARGET_A5F;
        active_gain = 60;
    } else if (coda == RELOQ_DE_PHONE_k) {
        coda_to_release = 122;
        ah = voice->k_ah;
        af = 62;
        active = voice->k_front ? RELOQ_TARGET_A3F : RELOQ_TARGET_A2F;
        active_gain = 60;
    } else return 1;
    if (vowel_closure_tick > INT64_MAX - coda_to_release) return -1;
    const int64_t origin = vowel_closure_tick + coda_to_release;

    struct reloq_target_event events[20];
    size_t count = 0;
    put(events, &count, RELOQ_TARGET_AV, 0, av);
    put(events, &count, RELOQ_TARGET_AV, 60, 0);
    put(events, &count, RELOQ_TARGET_AH, 0, coda == RELOQ_DE_PHONE_NONE ?
        34 : 0);
    put(events, &count, RELOQ_TARGET_AH, 0, ah);
    put(events, &count, RELOQ_TARGET_AH, 60, ah);
    put(events, &count, RELOQ_TARGET_AH, 60, 0);
    put(events, &count, RELOQ_TARGET_AF, 0, af);
    put(events, &count, RELOQ_TARGET_AF, 60, 0);
    for (enum reloq_target_stream stream = RELOQ_TARGET_A1F;
         stream <= RELOQ_TARGET_AB; ++stream) {
        put(events, &count, stream, 0,
            coda != RELOQ_DE_PHONE_NONE && stream == active ?
            active_gain : 0);
        put(events, &count, stream, 60, 0);
    }
    if (reloq_target_builder_append(builder, origin, events, count))
        return -1;
    *release_start_tick = origin;
    return 0;
}
