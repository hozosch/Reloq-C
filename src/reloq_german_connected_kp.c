#include "reloq_german_connected_kp.h"

#include <limits.h>
#include <stddef.h>

struct vowel_tract {
    enum reloq_german_phone phone;
    int16_t k_f1, k_f2, k_f3;
    int16_t p_vowel_onset, p_release_f2, p_noise;
};

/* The nonfinal velar closure retains the first vowel's tract, while the
 * following labial closure has its own place target. The second p's
 * postburst duration is the isolated p duration, measured relative to its
 * common 15-tick burst. These are reusable phone coefficients. */
static const struct vowel_tract vowels[] = {
    {RELOQ_DE_PHONE_A, 500, 1750, 2200, 65, 800, 64},
    {RELOQ_DE_PHONE_E, 300, 1900, 2300, 65, 1300, 64},
    {RELOQ_DE_PHONE_I, 300, 1900, 2300, 65, 1500, 64},
    {RELOQ_DE_PHONE_O, 300, 1500, 2300, 65, 700, 66},
    {RELOQ_DE_PHONE_U, 300, 1000, 2300, 50, 600, 66},
    {RELOQ_DE_PHONE_a, 500, 1750, 2200, 65, 800, 64},
    {RELOQ_DE_PHONE_e, 300, 1900, 2500, 78, 1500, 64},
    {RELOQ_DE_PHONE_i, 300, 2200, 2400, 78, 1500, 64},
    {RELOQ_DE_PHONE_o, 300, 960, 2300, 78, 600, 66},
    {RELOQ_DE_PHONE_u, 300, 1000, 2300, 59, 600, 66}
};

static const struct vowel_tract *tract(enum reloq_german_phone phone)
{
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (vowels[i].phone == phone) return &vowels[i];
    return NULL;
}

static void target(struct reloq_target_event *events, size_t *used,
                   enum reloq_target_stream stream, int64_t tick,
                   int32_t value)
{
    events[(*used)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_connected_kp_coda_append(
    const struct reloq_german_connected_pair *pair,
    int64_t first_closure_tick, struct reloq_target_builder *builder,
    int64_t *second_onset_origin)
{
    if (!pair || !builder || !second_onset_origin ||
        first_closure_tick < 0 || first_closure_tick > INT64_MAX - 52)
        return -1;
    if (pair->first_coda != RELOQ_DE_PHONE_k ||
        pair->second_initial != RELOQ_DE_PHONE_p) return 1;
    const struct vowel_tract *v = tract(pair->first_vowel);
    if (!v) return 1;
    const int64_t closure = first_closure_tick;
    const int64_t release = closure + 52;
    const int64_t burst = release - 12;
    const int32_t from[5] = {v->k_f1,v->k_f2,v->k_f3,3600,3900};
    const int32_t to[5] = {300,1500,2300,3600,3900};
    const int32_t width[5] = {120,100,150,330,260};
    struct reloq_target_event events[90];
    size_t used = 0;
    for (size_t i = 0; i < 5; ++i) {
        enum reloq_target_stream s =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)i);
        target(events,&used,s,closure,from[i]);
        target(events,&used,s,release,to[i]);
        s = (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)i);
        target(events,&used,s,closure,width[i]);
        target(events,&used,s,release,width[i]);
    }
    target(events,&used,RELOQ_TARGET_FNP,closure,200);
    target(events,&used,RELOQ_TARGET_FNP,release,200);
    target(events,&used,RELOQ_TARGET_FNZ,closure,200);
    target(events,&used,RELOQ_TARGET_FNZ,release,200);
    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        const int32_t strength = s == RELOQ_TARGET_A2F ? 70 : 0;
        target(events,&used,s,closure,0);
        target(events,&used,s,burst,0);
        target(events,&used,s,burst,strength);
        target(events,&used,s,release,strength);
    }
    target(events,&used,RELOQ_TARGET_AF,closure,0);
    target(events,&used,RELOQ_TARGET_AF,burst,0);
    target(events,&used,RELOQ_TARGET_AF,burst,50);
    target(events,&used,RELOQ_TARGET_AF,release,50);
    for (enum reloq_target_stream s = RELOQ_TARGET_AV;
         s <= RELOQ_TARGET_AH; ++s) {
        target(events,&used,s,closure,0);
        target(events,&used,s,burst,0);
        target(events,&used,s,burst,0);
        target(events,&used,s,release,0);
    }
    target(events,&used,RELOQ_TARGET_TL,closure,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,release,0);
    target(events,&used,RELOQ_TARGET_OQ,closure,56);
    target(events,&used,RELOQ_TARGET_OQ,burst,56);
    target(events,&used,RELOQ_TARGET_DI,closure,0);
    target(events,&used,RELOQ_TARGET_DI,burst,0);
    if (reloq_target_builder_append(builder,0,events,used)) return -1;
    *second_onset_origin = release + 1;
    return 0;
}

int reloq_german_connected_kp_onset_landmarks(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick,
    int64_t *first_send_end, int64_t *second_vowel_onset)
{
    if (!pair || !first_send_end || !second_vowel_onset ||
        first_coda_release_tick < 0 ||
        first_coda_release_tick > INT64_MAX - 164) return -1;
    if (pair->first_coda != RELOQ_DE_PHONE_k ||
        pair->second_initial != RELOQ_DE_PHONE_p) return 1;
    const struct vowel_tract *v = tract(second_vowel);
    if (!v) return 1;
    *first_send_end = first_coda_release_tick + 101;
    *second_vowel_onset = *first_send_end + v->p_vowel_onset - 15;
    return 0;
}

int reloq_german_connected_kp_onset_append(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick, int64_t second_final_release_tick,
    struct reloq_target_builder *builder,
    int64_t *first_send_end, int64_t *second_vowel_onset)
{
    if (!builder || !first_send_end || !second_vowel_onset) return -1;
    int64_t burst_end, vowel;
    int status = reloq_german_connected_kp_onset_landmarks(
        pair,second_vowel,first_coda_release_tick,&burst_end,&vowel);
    if (status) return status;
    if (second_final_release_tick <= vowel) return -1;
    const struct vowel_tract *v = tract(second_vowel);
    if (!v) return -1;
    const int64_t origin = first_coda_release_tick + 1;
    const int64_t burst = first_coda_release_tick + 91;
    const int32_t first_f[5] = {300,1000,2500,3600,3900};
    const int32_t burst_f[5] = {400,v->p_release_f2,2500,3600,3900};
    const int32_t width[5] = {120,100,150,330,260};
    struct reloq_target_event events[100];
    size_t used = 0;
    for (size_t i = 0; i < 5; ++i) {
        enum reloq_target_stream s =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)i);
        target(events,&used,s,origin,first_f[i]);
        target(events,&used,s,burst_end,burst_f[i]);
        s = (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)i);
        target(events,&used,s,origin,width[i]);
        target(events,&used,s,burst_end,width[i]);
    }
    target(events,&used,RELOQ_TARGET_FNP,origin,200);
    target(events,&used,RELOQ_TARGET_FNP,burst_end,200);
    target(events,&used,RELOQ_TARGET_FNZ,origin,200);
    target(events,&used,RELOQ_TARGET_FNZ,burst_end,200);
    target(events,&used,RELOQ_TARGET_FL,origin,0);
    target(events,&used,RELOQ_TARGET_FL,second_final_release_tick,0);
    target(events,&used,RELOQ_TARGET_FTP,origin,250);
    target(events,&used,RELOQ_TARGET_FTP,second_final_release_tick,250);
    target(events,&used,RELOQ_TARGET_FTZ,origin,250);
    target(events,&used,RELOQ_TARGET_FTZ,second_final_release_tick,250);
    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        const int32_t gain = s == RELOQ_TARGET_AB ? v->p_noise : 0;
        target(events,&used,s,origin,0);
        target(events,&used,s,burst,0);
        target(events,&used,s,burst,gain);
        target(events,&used,s,burst_end,gain);
    }
    target(events,&used,RELOQ_TARGET_AF,origin,0);
    target(events,&used,RELOQ_TARGET_AF,burst,0);
    target(events,&used,RELOQ_TARGET_AF,burst,60);
    target(events,&used,RELOQ_TARGET_AF,burst_end,60);
    target(events,&used,RELOQ_TARGET_AV,origin,0);
    target(events,&used,RELOQ_TARGET_AV,burst,0);
    target(events,&used,RELOQ_TARGET_AV,burst,0);
    target(events,&used,RELOQ_TARGET_AV,burst_end,0);
    target(events,&used,RELOQ_TARGET_AV,burst_end,0);
    target(events,&used,RELOQ_TARGET_AV,vowel,0);
    target(events,&used,RELOQ_TARGET_AH,origin,0);
    target(events,&used,RELOQ_TARGET_AH,burst,0);
    target(events,&used,RELOQ_TARGET_AH,burst,0);
    target(events,&used,RELOQ_TARGET_AH,burst_end,0);
    target(events,&used,RELOQ_TARGET_AH,burst_end,49);
    target(events,&used,RELOQ_TARGET_AH,vowel,49);
    target(events,&used,RELOQ_TARGET_TL,origin,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,burst_end,0);
    target(events,&used,RELOQ_TARGET_OQ,origin,56);
    target(events,&used,RELOQ_TARGET_OQ,burst,56);
    target(events,&used,RELOQ_TARGET_DI,origin,0);
    target(events,&used,RELOQ_TARGET_DI,burst,0);
    if (reloq_target_builder_append(builder,0,events,used)) return -1;
    *first_send_end = burst_end;
    *second_vowel_onset = vowel;
    return 0;
}
