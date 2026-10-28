#include "reloq_german_reduced_source.h"

#include <limits.h>

struct vowel_source {
    enum reloq_german_phone phone;
    int32_t av, k_initial_af, p_initial_ab;
    enum reloq_target_stream k_initial_noise;
    int32_t k_onsetless_af, k_onsetless_noise_gain;
    enum reloq_target_stream k_onsetless_noise;
};

/* Place of the palatal/velar frication follows vowel quality rather than
 * orthography. In an onsetless weak syllable its coda burst overlaps schwa;
 * an intervening /t/ instead cuts that burst off before its own release. */
static const struct vowel_source vowels[] = {
    {RELOQ_DE_PHONE_A, 51, 60, 64, RELOQ_TARGET_A2F,
        65, 70, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_E, 52, 54, 64, RELOQ_TARGET_A2F,
        60, 60, RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_I, 53, 64, 64, RELOQ_TARGET_A3F,
        60, 60, RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_O, 53, 62, 66, RELOQ_TARGET_A2F,
        63, 70, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_U, 54, 62, 66, RELOQ_TARGET_A2F,
        63, 70, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_a, 50, 60, 64, RELOQ_TARGET_A2F,
        65, 70, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_e, 51, 64, 64, RELOQ_TARGET_A3F,
        60, 60, RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_i, 53, 64, 64, RELOQ_TARGET_A3F,
        60, 60, RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_o, 54, 62, 66, RELOQ_TARGET_A2F,
        63, 70, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_u, 55, 62, 66, RELOQ_TARGET_A2F,
        63, 70, RELOQ_TARGET_A2F}
};

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_reduced_source_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *landmarks,
    struct reloq_target_builder *builder)
{
    if (!phones || !syllables || !landmarks || !builder) return -1;
    /* The timing planner is also the strict structural gate. */
    struct reloq_german_reduced_landmarks checked;
    const int class_status = reloq_german_reduced_landmarks_plan(
        phones, phone_count, syllables, syllable_count, &checked);
    if (class_status) return class_status;
    const struct reloq_german_reduced_landmarks *l = landmarks;
    const int has_t = syllables[1].onset_count == 1;
    const int has_n = syllables[1].coda_count == 1;
    if (l->first_burst_end < 5 ||
        l->first_vowel_onset <= l->first_burst_end ||
        l->first_vowel_onset >= l->first_vowel_hold ||
        l->first_vowel_hold >= l->first_closure ||
        l->first_closure >= l->first_coda_burst_start ||
        l->first_coda_burst_start >= l->first_coda_release ||
        (has_t ? l->first_coda_release >= l->second_vowel_onset :
                 l->first_coda_release > l->second_vowel_onset) ||
        (has_t ? l->second_vowel_onset != l->second_schwa_target :
                 l->second_vowel_onset >= l->second_schwa_target) ||
        l->second_schwa_target > l->second_closure ||
        l->second_closure >= l->planned_ticks ||
        l->second_closure > INT64_MAX - 60 ||
        l->planned_ticks < l->second_closure + 60) return -1;
    const int first_p = phones[2].phone == RELOQ_DE_PHONE_p;
    const struct vowel_source *v = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[1].phone == vowels[i].phone) v = &vowels[i];
    if (!v ||
        (has_t && (l->second_burst_start <= l->first_coda_release ||
                   l->second_burst_start >= l->second_burst_end ||
                   l->second_burst_end >= l->second_vowel_onset)) ||
        (has_n && (l->second_nasal_onset <= l->second_vowel_onset ||
                   l->second_nasal_onset >= l->second_nasal_mid ||
                   l->second_nasal_mid >= l->second_closure)) ||
        (!has_n && l->second_vowel_hold != l->second_closure) ||
        l->second_noise_fade < l->first_coda_release ||
        l->second_noise_fade > l->second_closure) return -1;

    const enum reloq_german_phone initial = phones[0].phone;
    const int32_t initial_ah = initial == RELOQ_DE_PHONE_p ? 49 :
        initial == RELOQ_DE_PHONE_t ? 47 : 45;
    const int32_t initial_af = initial == RELOQ_DE_PHONE_p ? 60 :
        initial == RELOQ_DE_PHONE_t ? 59 : v->k_initial_af;
    const enum reloq_target_stream initial_noise =
        initial == RELOQ_DE_PHONE_p ? RELOQ_TARGET_AB :
        initial == RELOQ_DE_PHONE_t ? RELOQ_TARGET_A5F :
        v->k_initial_noise;
    const int32_t initial_gain = initial == RELOQ_DE_PHONE_p ?
        v->p_initial_ab : initial == RELOQ_DE_PHONE_t ? 56 :
        initial_noise == RELOQ_TARGET_A3F ? 60 : 70;
    const enum reloq_target_stream first_noise = first_p ? RELOQ_TARGET_AB :
        has_t ? RELOQ_TARGET_A2F : v->k_onsetless_noise;
    const int32_t coda_af = first_p ? 60 : has_t ? 54 :
        v->k_onsetless_af;
    const int32_t coda_gain = first_p ? has_t ? 60 : 54 :
        has_t ? 60 : v->k_onsetless_noise_gain;
    const int64_t first_noise_end = has_t ?
        l->first_coda_release + (first_p ? 1 : 20) :
        l->second_noise_fade;
    const int64_t first_vowel_span =
        l->first_closure - l->first_vowel_onset;
    const int64_t f0_middle = l->first_vowel_onset +
        (first_vowel_span / 100) * 35 +
        ((first_vowel_span % 100) * 35) / 100;
    if (first_noise_end <= l->first_coda_release ||
        (has_t && first_noise_end >= l->second_burst_start) ||
        (has_n && (l->second_nasal_onset >= l->second_closure ||
                   l->second_closure > INT64_MAX - 30))) return -1;

    /* Sparse gesture targets are sufficient: redundant zero holds from the
     * reference target topology are not needed for frame-equivalent values. */
    struct reloq_target_event events[160];
    size_t n = 0;
#define PUT(stream, tick, value) put(events, &n, stream, tick, value)
    PUT(RELOQ_TARGET_F0, initial == RELOQ_DE_PHONE_t &&
        l->first_burst_end == 23 ? 2 : 1, 1046);
    PUT(RELOQ_TARGET_F0, f0_middle, 1296);
    PUT(RELOQ_TARGET_F0, l->second_closure -
        (has_n ? has_t ? 51 : 53 : has_t ? 25 : 27), 793);
    PUT(RELOQ_TARGET_F0, l->second_closure - (has_n ? 2 : 1), 740);

    PUT(RELOQ_TARGET_AV, 0, 0);
    PUT(RELOQ_TARGET_AV, l->first_vowel_onset, 0);
    PUT(RELOQ_TARGET_AV, l->first_vowel_onset + 1, v->av);
    PUT(RELOQ_TARGET_AV, l->first_closure, v->av - 2);
    PUT(RELOQ_TARGET_AV, l->first_closure, 0);
    PUT(RELOQ_TARGET_AV, l->second_vowel_onset, 0);
    PUT(RELOQ_TARGET_AV, l->second_vowel_onset + 1,
        has_n ? 47 : 50);
    if (has_n) {
        PUT(RELOQ_TARGET_AV, l->second_nasal_onset, 45);
        PUT(RELOQ_TARGET_AV, l->second_nasal_mid, 40);
        PUT(RELOQ_TARGET_AV, l->second_closure, 38);
        PUT(RELOQ_TARGET_AV, l->second_closure + 15, 38);
        PUT(RELOQ_TARGET_AV, l->second_closure + 15, 0);
    } else {
        PUT(RELOQ_TARGET_AV, l->second_closure, 48);
        PUT(RELOQ_TARGET_AV, l->second_closure + 60, 0);
    }
    PUT(RELOQ_TARGET_AV, l->planned_ticks, 0);

    PUT(RELOQ_TARGET_AH, 0, 0);
    PUT(RELOQ_TARGET_AH, l->first_burst_end, 0);
    PUT(RELOQ_TARGET_AH, l->first_burst_end, initial_ah);
    PUT(RELOQ_TARGET_AH, l->first_vowel_onset, initial_ah);
    PUT(RELOQ_TARGET_AH, l->first_vowel_onset + 1, 34);
    PUT(RELOQ_TARGET_AH, l->first_closure, 34);
    PUT(RELOQ_TARGET_AH, l->first_closure, 0);
    if (has_t && !first_p) {
        PUT(RELOQ_TARGET_AH, l->first_coda_release, 0);
        PUT(RELOQ_TARGET_AH, l->first_coda_release, 50);
        PUT(RELOQ_TARGET_AH, first_noise_end, 50);
        PUT(RELOQ_TARGET_AH, first_noise_end, 0);
    }
    if (has_t) {
        PUT(RELOQ_TARGET_AH, l->second_burst_end, 0);
        PUT(RELOQ_TARGET_AH, l->second_burst_end, 47);
        PUT(RELOQ_TARGET_AH, l->second_vowel_onset, 47);
    } else {
        PUT(RELOQ_TARGET_AH, l->second_vowel_onset, 0);
    }
    PUT(RELOQ_TARGET_AH, l->second_vowel_onset + 1, 34);
    if (has_n) {
        PUT(RELOQ_TARGET_AH, l->second_nasal_onset, 34);
        PUT(RELOQ_TARGET_AH, l->second_nasal_onset, 0);
        PUT(RELOQ_TARGET_AH, l->second_closure, 0);
        PUT(RELOQ_TARGET_AH, l->second_closure, 36);
        PUT(RELOQ_TARGET_AH, l->second_closure + 30, 36);
        PUT(RELOQ_TARGET_AH, l->second_closure + 30, 0);
    } else {
        PUT(RELOQ_TARGET_AH, l->second_closure, 34);
        PUT(RELOQ_TARGET_AH, l->second_closure, 35);
        PUT(RELOQ_TARGET_AH, l->second_closure + 60, 35);
        PUT(RELOQ_TARGET_AH, l->second_closure + 60, 0);
    }
    PUT(RELOQ_TARGET_AH, l->planned_ticks, 0);

    PUT(RELOQ_TARGET_AF, 0, 0);
    PUT(RELOQ_TARGET_AF, 5, 0);
    PUT(RELOQ_TARGET_AF, 5, initial_af);
    PUT(RELOQ_TARGET_AF, l->first_burst_end, initial_af);
    PUT(RELOQ_TARGET_AF, l->first_vowel_onset, 0);
    PUT(RELOQ_TARGET_AF, l->first_coda_burst_start, 0);
    PUT(RELOQ_TARGET_AF, l->first_coda_burst_start, coda_af);
    PUT(RELOQ_TARGET_AF, l->first_coda_release, coda_af);
    PUT(RELOQ_TARGET_AF, first_noise_end, 0);
    if (has_t) {
        PUT(RELOQ_TARGET_AF, l->second_burst_start, 0);
        PUT(RELOQ_TARGET_AF, l->second_burst_start, 56);
        PUT(RELOQ_TARGET_AF, l->second_burst_end, 56);
        PUT(RELOQ_TARGET_AF, l->second_vowel_onset, 0);
    }
    PUT(RELOQ_TARGET_AF, l->planned_ticks, 0);

    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        PUT(s, 0, 0);
        if (s == initial_noise) {
            PUT(s, 5, 0);
            PUT(s, 5, initial_gain);
            PUT(s, l->first_burst_end, initial_gain);
        }
        PUT(s, l->first_vowel_onset, 0);
        if (s == first_noise) {
            PUT(s, l->first_coda_burst_start, 0);
            PUT(s, l->first_coda_burst_start, coda_gain);
            PUT(s, l->first_coda_release, coda_gain);
        }
        PUT(s, first_noise_end, 0);
        if (has_t && s == RELOQ_TARGET_A5F) {
            PUT(s, l->second_burst_start, 0);
            PUT(s, l->second_burst_start, 56);
            PUT(s, l->second_burst_end, 56);
            PUT(s, l->second_vowel_onset, 0);
        }
        PUT(s, l->planned_ticks, 0);
    }
#undef PUT
    if (n > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder, 0, events, n)) return -1;
    return 0;
}
