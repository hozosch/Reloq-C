#include "reloq_german_initial_h.h"

#include "reloq_german_hv_timing.h"

int reloq_german_initial_h_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t h_origin_tick, int32_t vowel_av,
    int64_t first_window_ticks, struct reloq_target_builder *builder,
    int64_t *vowel_anchor_offset)
{
    if (!phones || !builder || !vowel_anchor_offset || !phone_count ||
        h_origin_tick < 0 || vowel_av < 50 || vowel_av > 58 ||
        (first_window_ticks != -1 &&
         (first_window_ticks < 10 || first_window_ticks >= 800))) return -1;
    int64_t onset;
    int shape = reloq_german_hv_acoustic_anchor(
        phones, phone_count, &onset);
    if (shape) return shape;
    if (onset != 80) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != 0 || phones[i].syllable_in_word != 0 ||
            phones[i].stress != 1 || !phones[i].stress_explicit)
            return 1;

    struct reloq_target_event events[7];
    size_t count = 0;
    if (first_window_ticks != -1)
        events[count++] = (struct reloq_target_event){
            RELOQ_TARGET_F0, first_window_ticks / 10, 1049};
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_AV, 0, 0};
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_AV, onset, 0};
    events[count++] = (struct reloq_target_event){
        RELOQ_TARGET_AH, 0, vowel_av - 9};
    events[count++] = (struct reloq_target_event){
        RELOQ_TARGET_AH, onset, vowel_av - 9};
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_AF, 0, 0};
    events[count++] = (struct reloq_target_event){RELOQ_TARGET_AF, onset, 0};
    if (reloq_target_builder_append(builder, h_origin_tick, events, count))
        return -1;
    *vowel_anchor_offset = onset;
    return 0;
}
