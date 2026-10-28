#include "reloq_german_vowel_nucleus.h"

/* These are voice-model coefficients, not recordings or a copied sequence of
 * reference targets. The context model uses a base vowel, a following-stop
 * shortening for long vowels, an anticipatory A transition, and one shared
 * source closure/release gesture. Tested only in the header's closed scope. */
struct nucleus_voice {
    enum reloq_german_phone phone;
    int32_t f1, f2, f3;
    int32_t av;
    int64_t open_ticks;
    int64_t stop_shortening;
};

static const struct nucleus_voice nuclei[] = {
    {RELOQ_DE_PHONE_A, 700, 1160, 2500, 51, 97, 0},
    {RELOQ_DE_PHONE_E, 550, 1800, 2600, 52, 105, 0},
    {RELOQ_DE_PHONE_I, 380, 2000, 2800, 53, 98, 0},
    {RELOQ_DE_PHONE_O, 570, 900, 2500, 53, 98, 0},
    {RELOQ_DE_PHONE_U, 420, 930, 2400, 54, 84, 0},
    {RELOQ_DE_PHONE_a, 800, 1220, 2500, 50, 210, 42},
    {RELOQ_DE_PHONE_e, 370, 2150, 2700, 51, 208, 26},
    {RELOQ_DE_PHONE_i, 265, 2150, 2900, 53, 192, 24},
    {RELOQ_DE_PHONE_o, 390, 735, 2400, 54, 208, 26},
    {RELOQ_DE_PHONE_u, 250, 750, 2400, 55, 192, 24}
};

int reloq_german_vowel_nucleus_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t onset_tick, int terminal_period,
    struct reloq_target_builder *builder, int64_t *closure_offset)
{
    if (!phones || !builder || !closure_offset || !phone_count ||
        vowel_index >= phone_count || onset_tick < 0 ||
        (terminal_period != 0 && terminal_period != 1)) return -1;
    if (phone_count > 3 || vowel_index > 1 ||
        phone_count - vowel_index > 2) return 1;
    if (vowel_index && phones[0].phone != RELOQ_DE_PHONE_h) return 1;

    const struct reloq_german_phone_node *vowel = &phones[vowel_index];
    if (vowel->word_index != 0 || vowel->syllable_in_word != 0 ||
        vowel->stress != 1 || !vowel->stress_explicit) return 1;
    for (size_t i = 0; i < phone_count; ++i) {
        if (phones[i].word_index != vowel->word_index ||
            phones[i].syllable_index != vowel->syllable_index ||
            phones[i].stress != 1 || !phones[i].stress_explicit) return 1;
    }

    const struct nucleus_voice *voice = NULL;
    for (size_t i = 0; i < sizeof nuclei / sizeof nuclei[0]; ++i) {
        if (nuclei[i].phone == vowel->phone) {
            voice = &nuclei[i];
            break;
        }
    }
    if (!voice) return 1;

    enum reloq_german_phone right = phone_count - vowel_index == 2 ?
        phones[vowel_index + 1].phone : RELOQ_DE_PHONE_NONE;
    if (right != RELOQ_DE_PHONE_NONE && right != RELOQ_DE_PHONE_p &&
        right != RELOQ_DE_PHONE_t && right != RELOQ_DE_PHONE_k) return 1;
    const int stop = right != RELOQ_DE_PHONE_NONE;
    const int64_t closure = voice->open_ticks -
        (stop ? voice->stop_shortening : 0);
    int32_t f1 = voice->f1, f2 = voice->f2;
    if (voice->phone == RELOQ_DE_PHONE_A && stop) {
        f1 += 80;
        if (right == RELOQ_DE_PHONE_t) f2 += 140;
    }

    const struct reloq_target_event points[] = {
        {RELOQ_TARGET_F1, 0, f1},
        {RELOQ_TARGET_F2, 0, f2},
        {RELOQ_TARGET_F3, 0, voice->f3},
        {RELOQ_TARGET_AV, 1, voice->av},
        {RELOQ_TARGET_AV, closure, voice->av - 2},
        {RELOQ_TARGET_AV, stop ? closure : closure + 60, 0},
        {RELOQ_TARGET_AH, 1, 34},
        {RELOQ_TARGET_AH, closure, 34},
        {RELOQ_TARGET_AH, closure, stop ? 0 : 35},
        {RELOQ_TARGET_AH, closure + 60, 0}
    };
    struct reloq_target_event batch[14];
    size_t count = stop ? 9 : 10;
    for (size_t i = 0; i < count; ++i) batch[i] = points[i];
    if (terminal_period) {
        /* Four contour anchors scale with this model's source interval.
         * A preceding h supplies its own first F0 target before onset. */
        if (!vowel_index)
            batch[count++] = (struct reloq_target_event){
                RELOQ_TARGET_F0, closure * 2 / 100, 1217};
        batch[count++] = (struct reloq_target_event){
            RELOQ_TARGET_F0, closure * 35 / 100, 1296};
        batch[count++] = (struct reloq_target_event){
            RELOQ_TARGET_F0, closure * 70 / 100, 796};
        batch[count++] = (struct reloq_target_event){
            RELOQ_TARGET_F0, closure * 99 / 100, 743};
    }
    if (reloq_target_builder_append(builder, onset_tick, batch, count))
        return -1;
    *closure_offset = closure;
    return 0;
}
