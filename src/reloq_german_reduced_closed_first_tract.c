#include "reloq_german_reduced_closed_first_tract.h"

#include "reloq_german_stop_vowel.h"

#include <stddef.h>
#include <stdint.h>

struct coda_shape {
    enum reloq_german_phone vowel;
    int32_t p[3], k[3];
};

/* Coarticulation from a primary vowel into the first coda. These rows are
 * phone-place coefficients, shared across the first stop and weak follower. */
static const struct coda_shape constrictions[] = {
    {RELOQ_DE_PHONE_A,{500,900,2500},{500,1750,2200}},
    {RELOQ_DE_PHONE_E,{300,1000,2200},{300,1900,2300}},
    {RELOQ_DE_PHONE_I,{300,1000,2200},{300,1900,2300}},
    {RELOQ_DE_PHONE_O,{300,700,2500},{300,1500,2300}},
    {RELOQ_DE_PHONE_U,{300,600,2100},{300,1000,2300}},
    {RELOQ_DE_PHONE_a,{500,900,2500},{500,1750,2200}},
    {RELOQ_DE_PHONE_e,{300,1500,2500},{300,1900,2500}},
    {RELOQ_DE_PHONE_i,{300,1500,2500},{300,2200,2400}},
    {RELOQ_DE_PHONE_o,{300,600,2100},{300,960,2300}},
    {RELOQ_DE_PHONE_u,{300,600,2100},{300,1000,2300}}
};

static void put(struct reloq_target_event *events, size_t *n,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*n)++] = (struct reloq_target_event){stream,tick,value};
}

int reloq_german_reduced_closed_first_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *t,
    struct reloq_target_builder *builder)
{
    if (!phones || !syllables || !t || !builder) return -1;
    struct reloq_german_reduced_landmarks check;
    const int status = reloq_german_reduced_landmarks_plan(
        phones,phone_count,syllables,syllable_count,&check);
    if (status) return status;
    if (syllables[0].phone_count != 3 ||
        syllables[1].first_phone != 3 ||
        t->first_burst_end != check.first_burst_end ||
        t->first_vowel_onset != check.first_vowel_onset ||
        t->first_vowel_hold != check.first_vowel_hold ||
        t->first_closure != check.first_closure ||
        t->first_coda_burst_start != check.first_coda_burst_start ||
        t->first_coda_release != check.first_coda_release) return 1;
    const int has_onset = syllables[1].onset_count == 1;
    const int p_coda = phones[2].phone == RELOQ_DE_PHONE_p;
    const struct coda_shape *shape = NULL;
    for (size_t i = 0;
         i < sizeof constrictions / sizeof constrictions[0]; ++i)
        if (phones[1].phone == constrictions[i].vowel) {
            shape = &constrictions[i];
            break;
        }
    if (!shape) return 1;

    /* Reuse the vowel model's contextual initial-stop and nucleus targets.
     * Its original final vowel duration is discarded in favor of the shared
     * reduced-word landmarks. This keeps both vowel classes consistent. */
    struct reloq_german_phone_node local[3] = {
        phones[0],phones[1],phones[2]};
    for (size_t i = 0; i < 3; ++i) {
        local[i].word_index = 0;
        local[i].syllable_index = 0;
        local[i].syllable_in_word = 0;
        local[i].stress = 1;
        local[i].stress_explicit = 1;
    }
    struct reloq_target_builder oracle = {0};
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    struct reloq_acoustic_point controls[RELOQ_TARGET_STREAM_COUNT][8] = {0};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        buffers[i].points = controls[i];
        buffers[i].capacity = 8;
    }
    if (reloq_target_builder_begin(&oracle,buffers)) return -1;
    int64_t ignored_closure, burst_end, ignored_hold;
    const int found = reloq_german_stop_vowel_append(
        local,3,t->first_vowel_onset,&oracle,
        &ignored_closure,&burst_end,&ignored_hold);
    if (found) return found;
    if (burst_end != t->first_burst_end) return 1;

    struct reloq_target_event events[180];
    size_t n = 0;
    const int64_t ticks[] = {0,t->first_burst_end,t->first_vowel_onset,
        t->first_vowel_hold,t->first_closure,t->first_coda_release};
    const size_t formant_count = has_onset ? 6 : 5;
    const int32_t *const closure = p_coda ? shape->p : shape->k;
    for (size_t f = 0; f < 3; ++f) {
        const enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)f);
        if (oracle.stream[stream].count != 4) return -1;
        for (size_t i = 0; i < formant_count; ++i) {
            const int32_t value = i < 4 ? controls[stream][i].value :
                i == 4 ? closure[f] :
                f == 0 ? 300 : f == 1 ? (p_coda ? 1300 : 1800) :
                (p_coda ? 2500 : 2300);
            put(events,&n,stream,ticks[i],value);
        }
    }
    for (size_t f = 0; f < 2; ++f) {
        const enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_F4 + (int)f);
        const int32_t prefix = phones[0].phone == RELOQ_DE_PHONE_t ?
            (f ? 4200 : 3500) : (f ? 3900 : 3600);
        const int32_t vowel = f ? 3900 : 3600;
        for (size_t i = 0; i < formant_count; ++i)
            put(events,&n,stream,ticks[i],i < 2 ? prefix : vowel);
    }
    const int32_t widths[] = {120,100,150,330,260};
    for (size_t f = 0; f < 5; ++f) {
        const enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)f);
        for (size_t i = 0; i < formant_count; ++i)
            put(events,&n,stream,ticks[i],widths[f]);
    }
    for (enum reloq_target_stream stream = RELOQ_TARGET_FNP;
         stream <= RELOQ_TARGET_FNZ; ++stream)
        for (size_t i = 0; i < formant_count; ++i)
            put(events,&n,stream,ticks[i],200);
    for (enum reloq_target_stream stream = RELOQ_TARGET_FTP;
         stream <= RELOQ_TARGET_FTZ; ++stream) {
        put(events,&n,stream,0,250);
        if (has_onset) put(events,&n,stream,t->first_coda_release,250);
    }
    put(events,&n,RELOQ_TARGET_FL,0,0);
    if (has_onset) put(events,&n,RELOQ_TARGET_FL,t->first_coda_release,0);
    put(events,&n,RELOQ_TARGET_TL,0,0);
    put(events,&n,RELOQ_TARGET_TL,5,0);
    put(events,&n,RELOQ_TARGET_TL,5,0);
    put(events,&n,RELOQ_TARGET_TL,t->first_burst_end,0);
    put(events,&n,RELOQ_TARGET_TL,t->first_vowel_onset+1,0);
    put(events,&n,RELOQ_TARGET_TL,t->first_closure,0);
    put(events,&n,RELOQ_TARGET_TL,t->first_closure,0);
    put(events,&n,RELOQ_TARGET_TL,t->first_coda_burst_start,0);
    put(events,&n,RELOQ_TARGET_TL,t->first_coda_burst_start,0);
    if (has_onset) put(events,&n,RELOQ_TARGET_TL,t->first_coda_release,0);
    const int64_t source_ticks[] = {0,5,t->first_vowel_onset,
        t->first_vowel_hold,t->first_closure,t->first_coda_burst_start};
    for (size_t i = 0; i < 6; ++i) {
        put(events,&n,RELOQ_TARGET_OQ,source_ticks[i],56);
        put(events,&n,RELOQ_TARGET_DI,source_ticks[i],0);
    }
    if (n > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder,0,events,n)) return -1;
    return 0;
}
