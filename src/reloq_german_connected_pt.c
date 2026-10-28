#include "reloq_german_connected_pt.h"

#include <limits.h>
#include <stddef.h>

struct vowel_class {
    enum reloq_german_phone phone;
    int16_t p_f1, p_f2, p_f3;
    int16_t t_onset;
    int8_t t_front;
};

/* First-vowel closure place and second-/t/ isolated postburst aspiration
 * are vowel-class coefficients. Only the boundary changes their timing. */
static const struct vowel_class vowels[] = {
    {RELOQ_DE_PHONE_A, 500, 900, 2500,  89, 0},
    {RELOQ_DE_PHONE_E, 300,1000, 2200,  95, 1},
    {RELOQ_DE_PHONE_I, 300,1000, 2200,  95, 1},
    {RELOQ_DE_PHONE_O, 300, 700, 2500,  89, 0},
    {RELOQ_DE_PHONE_U, 300, 600, 2100,  67, 0},
    {RELOQ_DE_PHONE_a, 500, 900, 2500,  89, 0},
    {RELOQ_DE_PHONE_e, 300,1500, 2500, 113, 1},
    {RELOQ_DE_PHONE_i, 300,1500, 2500, 113, 1},
    {RELOQ_DE_PHONE_o, 300, 600, 2100, 107, 0},
    {RELOQ_DE_PHONE_u, 300, 600, 2100,  80, 0}
};

static const struct vowel_class *vowel(enum reloq_german_phone phone)
{
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phone == vowels[i].phone) return &vowels[i];
    return NULL;
}

static void target(struct reloq_target_event *events, size_t *used,
                   enum reloq_target_stream stream, int64_t tick,
                   int32_t value)
{
    events[(*used)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_connected_pt_coda_append(
    const struct reloq_german_connected_pair *pair,
    int64_t first_closure_tick, struct reloq_target_builder *builder,
    int64_t *second_onset_origin)
{
    if (!pair || !builder || !second_onset_origin ||
        first_closure_tick < 0 || first_closure_tick > INT64_MAX - 101)
        return -1;
    if (pair->first_coda != RELOQ_DE_PHONE_p ||
        pair->second_initial != RELOQ_DE_PHONE_t) return 1;
    const struct vowel_class *v = vowel(pair->first_vowel);
    if (!v) return 1;

    const int64_t closure = first_closure_tick;
    const int64_t release = closure + 100;
    const int64_t burst = release - 10;
    const int32_t from[5] = {v->p_f1,v->p_f2,v->p_f3,3600,3900};
    const int32_t to[5] = {300,1300,2500,3600,3900};
    const int32_t width[5] = {120,100,150,330,260};
    struct reloq_target_event events[68];
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
        const int32_t gain = s == RELOQ_TARGET_AB ? 60 : 0;
        target(events,&used,s,closure,0);
        target(events,&used,s,burst,0);
        target(events,&used,s,burst,gain);
        target(events,&used,s,release,gain);
    }
    target(events,&used,RELOQ_TARGET_AF,closure,0);
    target(events,&used,RELOQ_TARGET_AF,burst,0);
    target(events,&used,RELOQ_TARGET_AF,burst,60);
    target(events,&used,RELOQ_TARGET_AF,release,60);
    /* The isolated terminal /p/ has an additional post-release AV/AH
     * continuation. The next /t/ replaces it at the connected boundary. */
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
    if (used != sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder,0,events,used)) return -1;
    *second_onset_origin = release + 1;
    return 0;
}

int reloq_german_connected_pt_onset_landmarks(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick,
    int64_t *first_send_end, int64_t *second_vowel_onset)
{
    if (!pair || !first_send_end || !second_vowel_onset ||
        first_coda_release_tick < 0 ||
        first_coda_release_tick > INT64_MAX - 177) return -1;
    if (pair->first_coda != RELOQ_DE_PHONE_p ||
        pair->second_initial != RELOQ_DE_PHONE_t) return 1;
    const struct vowel_class *v = vowel(second_vowel);
    if (!v) return 1;
    const int64_t burst = v->t_front ? 23 : 17;
    *first_send_end = first_coda_release_tick + 64 + burst;
    *second_vowel_onset = first_coda_release_tick + 64 + v->t_onset;
    return 0;
}

int reloq_german_connected_pt_onset_append(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick, int64_t second_final_release_tick,
    struct reloq_target_builder *builder,
    int64_t *first_send_end, int64_t *second_vowel_onset)
{
    if (!builder || !first_send_end || !second_vowel_onset) return -1;
    int64_t burst_end, anchor;
    int status = reloq_german_connected_pt_onset_landmarks(
        pair, second_vowel, first_coda_release_tick, &burst_end, &anchor);
    if (status) return status;
    if (second_final_release_tick <= anchor) return -1;
    const struct vowel_class *v = vowel(second_vowel);
    if (!v) return -1;

    const int64_t origin = first_coda_release_tick + 1;
    const int64_t burst = first_coda_release_tick + 69;
    const int32_t initial_f[5] = {300,1600,2500,3500,4200};
    const int32_t burst_f[5] = {
        second_vowel == RELOQ_DE_PHONE_u ? 280 : 300,
        v->t_front ? 1700 : 1600, v->t_front ? 2700 : 2500,
        3500,4200};
    const int32_t width[5] = {120,100,150,330,260};
    struct reloq_target_event events[78];
    size_t used = 0;
    for (size_t i = 0; i < 5; ++i) {
        enum reloq_target_stream s =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)i);
        target(events,&used,s,origin,initial_f[i]);
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
        const int32_t gain = s == RELOQ_TARGET_A5F ? 56 : 0;
        target(events,&used,s,origin,0);
        target(events,&used,s,burst,0);
        target(events,&used,s,burst,gain);
        target(events,&used,s,burst_end,gain);
    }
    target(events,&used,RELOQ_TARGET_AF,origin,0);
    target(events,&used,RELOQ_TARGET_AF,burst,0);
    target(events,&used,RELOQ_TARGET_AF,burst,59);
    target(events,&used,RELOQ_TARGET_AF,burst_end,59);
    target(events,&used,RELOQ_TARGET_AV,origin,0);
    target(events,&used,RELOQ_TARGET_AV,burst,0);
    target(events,&used,RELOQ_TARGET_AV,burst,0);
    target(events,&used,RELOQ_TARGET_AV,burst_end,0);
    target(events,&used,RELOQ_TARGET_AV,burst_end,0);
    target(events,&used,RELOQ_TARGET_AV,anchor,0);
    target(events,&used,RELOQ_TARGET_AH,origin,0);
    target(events,&used,RELOQ_TARGET_AH,burst,0);
    target(events,&used,RELOQ_TARGET_AH,burst,0);
    target(events,&used,RELOQ_TARGET_AH,burst_end,0);
    target(events,&used,RELOQ_TARGET_AH,burst_end,47);
    target(events,&used,RELOQ_TARGET_AH,anchor,47);
    target(events,&used,RELOQ_TARGET_TL,origin,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,burst_end,0);
    target(events,&used,RELOQ_TARGET_OQ,origin,56);
    target(events,&used,RELOQ_TARGET_OQ,burst,56);
    target(events,&used,RELOQ_TARGET_DI,origin,0);
    target(events,&used,RELOQ_TARGET_DI,burst,0);
    if (used != sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder,0,events,used)) return -1;
    *first_send_end = burst_end;
    *second_vowel_onset = anchor;
    return 0;
}
