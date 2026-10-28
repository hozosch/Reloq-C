#include "reloq_german_reduced_open_first_tract.h"

#include "reloq_german_stop_vowel.h"

#include <stddef.h>
#include <stdint.h>

static void put(struct reloq_target_event *events, size_t *n,
                enum reloq_target_stream s, int64_t tick, int32_t value)
{
    events[(*n)++] = (struct reloq_target_event){s,tick,value};
}

int reloq_german_reduced_open_first_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *t,
    struct reloq_target_builder *builder)
{
    if (!phones || !syllables || !t || !builder) return -1;
    /* The timing planner owns the full shape validation. This stage also
     * rejects a closed first syllable, which uses a different first tract. */
    struct reloq_german_reduced_landmarks check;
    const int status = reloq_german_reduced_landmarks_plan(
        phones,phone_count,syllables,syllable_count,&check);
    if (status) return status;
    if (syllables[0].phone_count != 2 ||
        syllables[1].first_phone != 2 ||
        t->first_coda_release != check.first_coda_release ||
        t->first_vowel_onset != check.first_vowel_onset ||
        t->first_vowel_hold != check.first_vowel_hold ||
        t->first_closure != check.first_closure ||
        t->first_burst_end != check.first_burst_end) return 1;

    /* The existing stop-vowel coefficient model includes anticipatory
     * coarticulation from the following /p,t,k/. Give it a virtual coda
     * with the same following place, then place its first four formant
     * values at the nonfinal timings chosen by the reduced planner.
     * Neither word text nor a recorded trajectory is consulted. */
    struct reloq_german_phone_node local[3] = {
        phones[0],phones[1],phones[2]
    };
    for (size_t i=0;i<3;++i) {
        local[i].word_index=0;
        local[i].syllable_index=0;
        local[i].syllable_in_word=0;
        local[i].stress=1;
        local[i].stress_explicit=1;
    }
    struct reloq_target_builder oracle={0};
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT]={{0}};
    struct reloq_acoustic_point controls[RELOQ_TARGET_STREAM_COUNT][8]={0};
    for (size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i) {
        buffers[i].points=controls[i];
        buffers[i].capacity=8;
    }
    if (reloq_target_builder_begin(&oracle,buffers)) return -1;
    int64_t ignored_closure,ignored_burst,ignored_hold;
    const int found = reloq_german_stop_vowel_append(
        local,3,t->first_vowel_onset,&oracle,
        &ignored_closure,&ignored_burst,&ignored_hold);
    if (found) return found;
    if (ignored_burst != t->first_burst_end) return 1;

    struct reloq_target_event events[128];
    size_t n=0;
    const int64_t ticks[]={0,t->first_burst_end,
        t->first_vowel_onset,t->first_vowel_hold};
    for (enum reloq_target_stream s=RELOQ_TARGET_F1;
         s<=RELOQ_TARGET_F3;++s) {
        if (oracle.stream[s].count!=4) return -1;
        for (size_t j=0;j<4;++j)
            put(events,&n,s,ticks[j],controls[s][j].value);
    }
    for (enum reloq_target_stream s=RELOQ_TARGET_F4;
         s<=RELOQ_TARGET_F5;++s) {
        const int32_t prefix=phones[0].phone==RELOQ_DE_PHONE_t ?
            (s==RELOQ_TARGET_F4 ? 3500 : 4200) :
            (s==RELOQ_TARGET_F4 ? 3600 : 3900);
        const int32_t vowel=s==RELOQ_TARGET_F4 ? 3600 : 3900;
        for (size_t j=0;j<4;++j)
            put(events,&n,s,ticks[j],j<2 ? prefix : vowel);
    }
    const int32_t widths[]={120,100,150,330,260};
    for (size_t j=0;j<5;++j) {
        const enum reloq_target_stream s=
            (enum reloq_target_stream)(RELOQ_TARGET_B1+(int)j);
        for (size_t k=0;k<4;++k)
            put(events,&n,s,ticks[k],widths[j]);
    }
    for (enum reloq_target_stream s=RELOQ_TARGET_FNP;
         s<=RELOQ_TARGET_FNZ;++s)
        for (size_t j=0;j<4;++j) put(events,&n,s,ticks[j],200);
    for (enum reloq_target_stream s=RELOQ_TARGET_FTP;
         s<=RELOQ_TARGET_FTZ;++s) {
        put(events,&n,s,0,250);
        put(events,&n,s,t->first_vowel_hold,250);
    }
    put(events,&n,RELOQ_TARGET_FL,0,0);
    put(events,&n,RELOQ_TARGET_FL,t->first_vowel_hold,0);
    put(events,&n,RELOQ_TARGET_TL,0,0);
    put(events,&n,RELOQ_TARGET_TL,5,0);
    put(events,&n,RELOQ_TARGET_TL,5,0);
    put(events,&n,RELOQ_TARGET_TL,t->first_burst_end,0);
    put(events,&n,RELOQ_TARGET_TL,t->first_vowel_onset+1,0);
    const int64_t source_ticks[]={0,5,t->first_vowel_onset,t->first_vowel_hold};
    for (size_t i=0;i<4;++i) {
        put(events,&n,RELOQ_TARGET_OQ,source_ticks[i],56);
        put(events,&n,RELOQ_TARGET_DI,source_ticks[i],0);
    }
    if (n>sizeof events/sizeof events[0] ||
        reloq_target_builder_append(builder,0,events,n)) return -1;
    return 0;
}
