#include "reloq_german_reduced_tract.h"

#include <limits.h>
#include <stddef.h>

static int is_first_vowel(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_A || phone == RELOQ_DE_PHONE_E ||
        phone == RELOQ_DE_PHONE_I || phone == RELOQ_DE_PHONE_O ||
        phone == RELOQ_DE_PHONE_U || phone == RELOQ_DE_PHONE_a ||
        phone == RELOQ_DE_PHONE_e || phone == RELOQ_DE_PHONE_i ||
        phone == RELOQ_DE_PHONE_o || phone == RELOQ_DE_PHONE_u;
}

static int valid_shape(const struct reloq_german_phone_node *p,
                       size_t phone_count,
                       const struct reloq_german_syllable *s)
{
    const size_t second = s[1].first_phone;
    const int first_open = second == 2;
    const int has_onset = s[1].onset_count == 1;
    const int nasal = s[1].coda_count == 1;
    if ((second != 2 && second != 3) ||
        (first_open && !has_onset) ||
        (s[1].onset_count != 0 && !has_onset) ||
        phone_count != second + 1U + (size_t)has_onset + (size_t)nasal ||
        (p[0].phone != RELOQ_DE_PHONE_p &&
         p[0].phone != RELOQ_DE_PHONE_t &&
         p[0].phone != RELOQ_DE_PHONE_k) ||
        !is_first_vowel(p[1].phone) ||
        (!first_open && p[2].phone != RELOQ_DE_PHONE_p &&
                        p[2].phone != RELOQ_DE_PHONE_k) ||
        (first_open ?
          (p[second].phone != RELOQ_DE_PHONE_p &&
           p[second].phone != RELOQ_DE_PHONE_t &&
           p[second].phone != RELOQ_DE_PHONE_k) :
          has_onset && p[second].phone != RELOQ_DE_PHONE_t) ||
        p[second + (size_t)has_onset].phone != RELOQ_DE_PHONE_AT ||
        (nasal && p[second + (size_t)has_onset + 1].phone != RELOQ_DE_PHONE_n) ||
        s[0].first_phone != 0 || s[0].phone_count != second ||
        s[0].onset_count != 1 || s[0].nucleus_count != 1 ||
        s[0].coda_count != (size_t)!first_open ||
        s[1].phone_count != 1U + (size_t)has_onset + (size_t)nasal ||
        s[1].nucleus_count != 1 ||
        s[1].coda_count != (nasal ? 1U : 0U) ||
        !s[0].nucleus_contiguous || !s[1].nucleus_contiguous ||
        s[0].word_index != s[1].word_index ||
        s[0].syllable_in_word != 0 || s[1].syllable_in_word != 1 ||
        s[0].stress != 1 || s[1].stress != 0 ||
        !s[0].stress_explicit || !s[1].stress_explicit)
        return 0;
    for (size_t i = 0; i < phone_count; ++i)
        if (p[i].word_index != s[0].word_index ||
            p[i].syllable_index != p[i < second ? 0 : second].syllable_index ||
            p[i].syllable_in_word != (i < second ? 0U : 1U) ||
            p[i].stress != (i < second ? 1U : 0U) ||
            !p[i].stress_explicit) return 0;
    return 1;
}

static int valid_timing(const struct reloq_german_reduced_landmarks *t,
                        int nasal, int first_open, int has_onset)
{
    if (!has_onset) {
        if (t->second_onset_origin != 0 || t->second_burst_start != 0 ||
            t->second_burst_end != 0 ||
            t->second_vowel_onset != t->first_coda_release ||
            t->second_schwa_target <= t->second_vowel_onset ||
            t->second_vowel_hold < t->second_schwa_target ||
            t->second_closure > INT64_MAX - 60 ||
            t->planned_ticks <= t->second_closure + 60) return 0;
        return nasal ? t->second_vowel_hold > t->second_schwa_target &&
            t->second_nasal_onset > t->second_vowel_hold &&
            t->second_closure > t->second_nasal_onset + 55 :
            t->second_nasal_onset == 0 &&
            t->second_vowel_hold == t->second_closure;
    }
    if ((first_open ?
            t->second_onset_origin != t->first_closure ||
            t->first_coda_release != t->second_burst_end :
            t->second_onset_origin <= t->first_coda_release) ||
        t->second_burst_start <= t->second_onset_origin ||
        t->second_burst_end <= t->second_burst_start ||
        t->second_schwa_target <= t->second_burst_end ||
        t->second_closure <= t->second_schwa_target ||
        t->second_closure > INT64_MAX - 60 ||
        t->planned_ticks <= t->second_closure + 60) return 0;
    if (nasal) return t->second_vowel_hold > t->second_schwa_target &&
        t->second_nasal_onset > t->second_vowel_hold &&
        t->second_closure > t->second_nasal_onset + 55;
    return t->second_nasal_onset == 0 &&
        t->second_vowel_hold == t->second_closure;
}

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick,
                int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream,tick,value};
}

/* An onsetless weak syllable starts as the first coda releases. The
 * following F1/F2 schwa gesture has a separate, later target; there is no
 * invented second burst or voiceless interval. */
static int append_onsetless(const struct reloq_german_phone_node *phones,
                            const struct reloq_german_reduced_landmarks *t,
                            int nasal, struct reloq_target_builder *builder)
{
    const int k_coda = phones[2].phone == RELOQ_DE_PHONE_k;
    const int64_t release = t->second_vowel_onset;
    const int64_t schwa = t->second_schwa_target;
    const int64_t hold = t->second_vowel_hold;
    const int64_t end = t->second_closure;
    const int64_t fade = end + (nasal ? 30 : 60);
    struct reloq_target_event events[256];
    size_t n = 0;
    for (size_t f = 0; f < 3; ++f) {
        enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)f);
        const int32_t onset[] = {300,k_coda ? 1600 : 1000,
                                k_coda ? 2100 : 2500};
        const int32_t target[] = {
            k_coda || nasal ? 450 : 420,
            k_coda ? (nasal ? 1600 : 1700) : (nasal ? 1540 : 1270),
            k_coda ? 2260 : 2500};
        const int32_t n_value[] = {200,1700,2600};
        put(events,&n,stream,release,onset[f]);
        put(events,&n,stream,schwa,target[f]);
        put(events,&n,stream,hold,target[f]);
        if (nasal) {
            put(events,&n,stream,t->second_nasal_onset,n_value[f]);
            put(events,&n,stream,end,n_value[f]);
        }
    }
    for (size_t f = 0; f < 2; ++f) {
        enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_F4 + (int)f);
        const int32_t normal = f ? 3900 : 3600;
        put(events,&n,stream,release,normal);
        put(events,&n,stream,schwa,normal);
        put(events,&n,stream,hold,normal);
        if (nasal) {
            put(events,&n,stream,t->second_nasal_onset,
                f ? 3800 : 3350);
            put(events,&n,stream,end,f ? 3800 : 3350);
        }
        put(events,&n,stream,fade,normal);
        put(events,&n,stream,t->planned_ticks,normal);
    }
    const int32_t widths[] = {120,100,150,330,260};
    for (size_t f = 0; f < 5; ++f) {
        enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)f);
        put(events,&n,stream,release,widths[f]);
        put(events,&n,stream,schwa,nasal && f == 0 ? 270 : widths[f]);
        put(events,&n,stream,hold,nasal && f == 0 ? 270 : widths[f]);
        if (nasal) {
            const int32_t n_width = f == 1 ? 200 : f == 2 ? 80 : widths[f];
            put(events,&n,stream,t->second_nasal_onset,n_width);
            put(events,&n,stream,end,n_width);
        }
        put(events,&n,stream,fade,widths[f]);
        put(events,&n,stream,t->planned_ticks,widths[f]);
    }
    for (enum reloq_target_stream stream = RELOQ_TARGET_FNP;
         stream <= RELOQ_TARGET_FNZ; ++stream) {
        put(events,&n,stream,release,200);
        put(events,&n,stream,schwa,200);
        put(events,&n,stream,hold,200);
        if (nasal) {
            if (stream == RELOQ_TARGET_FNP) {
                put(events,&n,stream,t->second_nasal_onset,200);
                put(events,&n,stream,end,200);
            } else put(events,&n,stream,end - 55,350);
        }
        put(events,&n,stream,fade,200);
        put(events,&n,stream,t->planned_ticks,200);
    }
    for (enum reloq_target_stream stream = RELOQ_TARGET_FTP;
         stream <= RELOQ_TARGET_FTZ; ++stream) {
        put(events,&n,stream,release,250);
        put(events,&n,stream,schwa,250);
        put(events,&n,stream,end,250);
    }
    put(events,&n,RELOQ_TARGET_FL,release,0);
    put(events,&n,RELOQ_TARGET_FL,schwa,0);
    put(events,&n,RELOQ_TARGET_FL,end,0);
    const enum reloq_target_stream quiet_streams[] = {
        RELOQ_TARGET_OQ,RELOQ_TARGET_DI};
    for (size_t q = 0; q < sizeof quiet_streams / sizeof quiet_streams[0];
         ++q) {
        const enum reloq_target_stream stream = quiet_streams[q];
        const int32_t value = stream == RELOQ_TARGET_OQ ? 56 : 0;
        put(events,&n,stream,schwa,value);
        if (nasal) put(events,&n,stream,hold,value);
        if (nasal) put(events,&n,stream,t->second_nasal_onset,value);
        put(events,&n,stream,end,value);
        put(events,&n,stream,fade,value);
        put(events,&n,stream,t->planned_ticks,value);
    }
    put(events,&n,RELOQ_TARGET_TL,release,0);
    put(events,&n,RELOQ_TARGET_TL,release + 1,0);
    if (nasal) {
        put(events,&n,RELOQ_TARGET_TL,t->second_nasal_onset,0);
        put(events,&n,RELOQ_TARGET_TL,t->second_nasal_onset,0);
        put(events,&n,RELOQ_TARGET_TL,end,0);
        put(events,&n,RELOQ_TARGET_TL,end,0);
        put(events,&n,RELOQ_TARGET_TL,end + 15,0);
        put(events,&n,RELOQ_TARGET_TL,end + 15,0);
        put(events,&n,RELOQ_TARGET_TL,fade,0);
        put(events,&n,RELOQ_TARGET_TL,fade,0);
    } else {
        put(events,&n,RELOQ_TARGET_TL,end,0);
        put(events,&n,RELOQ_TARGET_TL,fade,0);
    }
    put(events,&n,RELOQ_TARGET_TL,t->planned_ticks,0);
    return n > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder,0,events,n) ? -1 : 0;
}

int reloq_german_reduced_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *t,
    struct reloq_target_builder *builder)
{
    if (!phones || !syllables || !t || !builder) return -1;
    if (syllable_count != 2 ||
        !valid_shape(phones, phone_count, syllables)) return 1;
    const int nasal = syllables[1].coda_count == 1;
    const int first_open = syllables[1].first_phone == 2;
    const int has_onset = syllables[1].onset_count == 1;
    if (!valid_timing(t, nasal, first_open, has_onset)) return -1;
    if (!has_onset) return append_onsetless(phones,t,nasal,builder);
    const enum reloq_german_phone second_stop =
        phones[syllables[1].first_phone].phone;
    struct reloq_target_event events[256];
    size_t n = 0;
    const int64_t end = t->second_closure;
    const int64_t fade_end = end + (nasal ? 30 : 60);
    const int64_t oral[] = {
        t->second_onset_origin, t->second_burst_end,
        t->second_schwa_target,
        nasal ? t->second_vowel_hold : end,
        t->second_nasal_onset, end
    };
    const int32_t open_f2[][10] = {
        {800,1000,1000,700,600,800,1500,1500,600,600}, /* p */
        {1600,1800,1800,1400,1300,1600,1900,1900,1300,1300}, /* t */
        {1750,1900,1900,1500,1000,1750,1900,2200,960,1000} /* k */
    };
    const int32_t open_f3[][10] = {
        {2500,2200,2200,2500,2100,2500,2500,2500,2100,2100},
        {2500,2500,2700,2500,2050,2500,2700,2700,2050,2050},
        {2200,2300,2300,2300,2300,2200,2500,2400,2300,2300}
    };
    const enum reloq_german_phone first_vowels[] = {
        RELOQ_DE_PHONE_A,RELOQ_DE_PHONE_E,RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O,RELOQ_DE_PHONE_U,RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_e,RELOQ_DE_PHONE_i,RELOQ_DE_PHONE_o,
        RELOQ_DE_PHONE_u
    };
    size_t first_vowel = 0;
    if (first_open) {
        for (; first_vowel < sizeof first_vowels / sizeof first_vowels[0];
             ++first_vowel)
            if (phones[1].phone == first_vowels[first_vowel]) break;
        if (first_vowel == sizeof first_vowels / sizeof first_vowels[0])
            return 1;
    }
    const size_t place = second_stop == RELOQ_DE_PHONE_p ? 0 :
        second_stop == RELOQ_DE_PHONE_t ? 1 : 2;
    const int32_t burst_f2[] = {1000,1600,1600};
    const int32_t burst_f3[] = {2500,2500,2100};
    const int32_t schwa_f1[] = {nasal ? 450 : 420,450,450};
    const int32_t schwa_f2[] = {nasal ? 1540 : 1270,
        nasal ? 1600 : 1540,nasal ? 1600 : 1700};
    const int32_t schwa_f3[] = {2500,2500,2260};
    for (size_t f = 0; f < 3; ++f) {
        const enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)f);
        const int32_t prefix[] = {300,burst_f2[place],burst_f3[place]};
        const int32_t schwa[] = {
            schwa_f1[place],schwa_f2[place],schwa_f3[place]};
        const int32_t nasal_value[] = {200,1700,2600};
        const int32_t at_closure = first_open && f == 1 ?
            open_f2[place][first_vowel] : first_open && f == 2 ?
            open_f3[place][first_vowel] : prefix[f];
        put(events,&n,stream,oral[0],at_closure);
        put(events,&n,stream,oral[1],prefix[f]);
        put(events,&n,stream,oral[2],schwa[f]);
        put(events,&n,stream,oral[3],schwa[f]);
        if (nasal) {
            put(events,&n,stream,oral[4],nasal_value[f]);
            put(events,&n,stream,oral[5],nasal_value[f]);
        }
    }
    const int64_t upper[] = {
        t->second_onset_origin, t->second_burst_end,
        t->second_schwa_target,
        nasal ? t->second_vowel_hold : end,
        t->second_nasal_onset, end, fade_end, t->planned_ticks
    };
    const size_t count = nasal ? 8 : 6;
    for (size_t f = 0; f < 2; ++f) {
        enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_F4 + (int)f);
        for (size_t i = 0; i < count; ++i) {
            const size_t index = nasal ? i : i < 4 ? i : i + 2;
            put(events,&n,stream,upper[index],
                i < 2 ? (place == 1 ? (f ? 4200 : 3500) :
                                      (f ? 3900 : 3600)) :
                nasal && (i == 4 || i == 5) ?
                    (f ? 3800 : 3350) : (f ? 3900 : 3600));
        }
    }
    const int32_t widths[] = {120,100,150,330,260};
    for (size_t f = 0; f < 5; ++f) {
        enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)f);
        for (size_t i = 0; i < count; ++i) {
            const size_t index = nasal ? i : i < 4 ? i : i + 2;
            int32_t value = widths[f];
            if (nasal && f == 0 && (i == 2 || i == 3)) value = 270;
            if (nasal && f == 1 && (i == 4 || i == 5)) value = 200;
            if (nasal && f == 2 && (i == 4 || i == 5)) value = 80;
            put(events,&n,stream,upper[index],value);
        }
    }
    for (enum reloq_target_stream stream = RELOQ_TARGET_FNP;
         stream <= RELOQ_TARGET_FNZ; ++stream) {
        for (size_t i = 0; i < count; ++i) {
            if (nasal && stream == RELOQ_TARGET_FNZ && i == 4)
                continue; /* resonator rise is delayed within nasal /n/ */
            const size_t index = nasal ? i : i < 4 ? i : i + 2;
            int32_t value = 200;
            int64_t tick = upper[index];
            if (nasal && stream == RELOQ_TARGET_FNZ && i == 5) {
                tick = end - 55; value = 350;
            }
            put(events,&n,stream,tick,value);
        }
    }
    for (enum reloq_target_stream stream = RELOQ_TARGET_FTP;
         stream <= RELOQ_TARGET_FTZ; ++stream) {
        put(events,&n,stream,t->second_onset_origin,250);
        put(events,&n,stream,end,250);
    }
    put(events,&n,RELOQ_TARGET_FL,t->second_onset_origin,0);
    put(events,&n,RELOQ_TARGET_FL,end,0);

    const int64_t oq[] = {
        t->second_onset_origin,t->second_burst_start,
        t->second_schwa_target,
        nasal ? t->second_vowel_hold : end,
        t->second_nasal_onset,end,fade_end,t->planned_ticks
    };
    for (size_t i = 0; i < count; ++i) {
        size_t j = nasal ? i : i < 4 ? i : i + 2;
        put(events,&n,RELOQ_TARGET_OQ,oq[j],56);
        put(events,&n,RELOQ_TARGET_DI,oq[j],0);
    }
    put(events,&n,RELOQ_TARGET_TL,t->second_onset_origin,0);
    if (first_open)
        put(events,&n,RELOQ_TARGET_TL,t->second_onset_origin,0);
    put(events,&n,RELOQ_TARGET_TL,t->second_burst_start,0);
    put(events,&n,RELOQ_TARGET_TL,t->second_burst_start,0);
    put(events,&n,RELOQ_TARGET_TL,t->second_burst_end,0);
    put(events,&n,RELOQ_TARGET_TL,t->second_schwa_target + 1,0);
    if (nasal) {
        put(events,&n,RELOQ_TARGET_TL,t->second_nasal_onset,0);
        put(events,&n,RELOQ_TARGET_TL,t->second_nasal_onset,0);
        put(events,&n,RELOQ_TARGET_TL,end,0);
        put(events,&n,RELOQ_TARGET_TL,end,0);
        put(events,&n,RELOQ_TARGET_TL,end + 15,0);
        put(events,&n,RELOQ_TARGET_TL,end + 15,0);
        put(events,&n,RELOQ_TARGET_TL,fade_end,0);
        put(events,&n,RELOQ_TARGET_TL,fade_end,0);
    } else {
        put(events,&n,RELOQ_TARGET_TL,end,0);
        put(events,&n,RELOQ_TARGET_TL,fade_end,0);
    }
    put(events,&n,RELOQ_TARGET_TL,t->planned_ticks,0);
    if (n > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder,0,events,n)) return -1;
    return 0;
}
