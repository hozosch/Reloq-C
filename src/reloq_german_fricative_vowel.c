#include "reloq_german_fricative_vowel.h"

#include <stdint.h>

struct vowel {
    enum reloq_german_phone phone;
    int32_t f1, f2, f3, av;
    int64_t closure, first_s, first_f;
};

/* Shared vowel nucleus and two coda-dependent transitions. The first send
 * end is measured independently from the actual vowel closure. */
static const struct vowel vowels[] = {
    {RELOQ_DE_PHONE_A, 780, 1160, 2500, 51, 107, 67, 82},
    {RELOQ_DE_PHONE_E, 550, 1800, 2600, 52, 115, 90, 90},
    {RELOQ_DE_PHONE_I, 380, 2000, 2800, 53, 107, 82, 82},
    {RELOQ_DE_PHONE_O, 570, 900, 2500, 53, 107, 57, 82},
    {RELOQ_DE_PHONE_U, 420, 930, 2400, 54, 92, 52, 67},
    {RELOQ_DE_PHONE_a, 800, 1220, 2500, 50, 184, 134, 159},
    {RELOQ_DE_PHONE_e, 370, 2150, 2700, 51, 200, 175, 175},
    {RELOQ_DE_PHONE_i, 265, 2150, 2900, 53, 184, 159, 159},
    {RELOQ_DE_PHONE_o, 390, 735, 2400, 54, 200, 160, 170},
    {RELOQ_DE_PHONE_u, 250, 750, 2400, 55, 184, 144, 154}
};

int reloq_german_fricative_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t onset_tick,
    struct reloq_target_builder *builder, int64_t *closure_tick,
    int64_t *first_send_end_tick)
{
    if (!phones || !builder || !closure_tick || !first_send_end_tick ||
        !phone_count || vowel_index >= phone_count || onset_tick < 0)
        return -1;
    if ((phone_count != 2 && phone_count != 3) ||
        (vowel_index != 0 && vowel_index != 1) ||
        vowel_index + 2 != phone_count ||
        (vowel_index && phones[0].phone != RELOQ_DE_PHONE_h) ||
        onset_tick != (vowel_index ? 80 : 0)) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index || phones[i].syllable_index ||
            phones[i].stress != 1 || !phones[i].stress_explicit)
            return 1;
    const enum reloq_german_phone coda = phones[phone_count - 1].phone;
    if (coda != RELOQ_DE_PHONE_s && coda != RELOQ_DE_PHONE_f) return 1;
    const struct vowel *voice = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[vowel_index].phone == vowels[i].phone) {
            voice = &vowels[i];
            break;
        }
    if (!voice) return 1;
    if (onset_tick > INT64_MAX - voice->closure) return -1;
    const int s = coda == RELOQ_DE_PHONE_s;
    const int32_t f1 = s && voice->phone == RELOQ_DE_PHONE_u ? 320 : voice->f1;
    const int32_t av = s && voice->phone == RELOQ_DE_PHONE_U ? 58 : voice->av;
    struct reloq_target_event events[8];
    size_t count = 0;
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_F1, 0, f1};
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_F2, 0, voice->f2};
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_F3, 0, voice->f3};
    /* Short /u/ anticipates /s/ by lifting F2 during the vowel's final
     * scheduler window. /uf/ retains the 750-Hz vowel target here. */
    if (s && voice->phone == RELOQ_DE_PHONE_u)
        events[count++] = (struct reloq_target_event){
            RELOQ_TARGET_F2, voice->first_s, 820};
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_AV, 1, av};
    events[count++] = (struct reloq_target_event){
        RELOQ_TARGET_AV, voice->closure, av - 2};
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_AH, 1, 34};
    events[count++] = (struct reloq_target_event){
        RELOQ_TARGET_AH, voice->closure, 34};
    if (reloq_target_builder_append(builder, onset_tick, events, count))
        return -1;
    *closure_tick = onset_tick + voice->closure;
    *first_send_end_tick = onset_tick + (s ? voice->first_s : voice->first_f);
    return 0;
}
