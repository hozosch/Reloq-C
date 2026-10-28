#include "reloq_german_connected_pair.h"

#include <limits.h>

/* One common nonfinal duration rule across the checked first-word vowel
 * classes. A preceding oral stop applies the same 90% initial modifier as
 * its isolated CV context; the connected coda replaces the isolated coda
 * shortening. These are voice coefficients, not utterance recordings. */
struct nonfinal_vowel {
    enum reloq_german_phone phone;
    int16_t base_ticks;
};

static const struct nonfinal_vowel vowels[] = {
    {RELOQ_DE_PHONE_A, 90}, {RELOQ_DE_PHONE_E, 90},
    {RELOQ_DE_PHONE_I, 84}, {RELOQ_DE_PHONE_O, 84},
    {RELOQ_DE_PHONE_U, 72}, {RELOQ_DE_PHONE_a, 140},
    {RELOQ_DE_PHONE_e, 130}, {RELOQ_DE_PHONE_i, 120},
    {RELOQ_DE_PHONE_o, 130}, {RELOQ_DE_PHONE_u, 120}
};

/* Rows: first coda p,t,k. Columns: following onset p,t,k,h. The coda send
 * endpoint and next phrase F0 lead are independently observed at the word
 * boundary. Same-place stops share the second onset's burst. */
static const int16_t coda_send_offset[3][4] = {
    {40, 100, 100, 100}, {58, 30, 58, 65}, {52, 52, 65, 87}
};
static const int16_t next_f0_lead[3][4] = {
    {11, 9, 7, 13}, {11, 9, 38, 13}, {11, 28, 7, 13}
};

static int coda_index(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_p: return 0;
    case RELOQ_DE_PHONE_t: return 1;
    case RELOQ_DE_PHONE_k: return 2;
    default: return -1;
    }
}

static int onset_index(enum reloq_german_phone phone)
{
    int stop = coda_index(phone);
    return stop >= 0 ? stop : phone == RELOQ_DE_PHONE_h ? 3 : -1;
}

static int vowel_duration(enum reloq_german_phone phone,
                          int oral_initial, int64_t *ticks)
{
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (vowels[i].phone == phone) {
            *ticks = oral_initial ? vowels[i].base_ticks * 9 / 10 :
                vowels[i].base_ticks;
            return 0;
        }
    return 1;
}

int reloq_german_connected_first_duration(
    const struct reloq_german_connected_pair *pair,
    int64_t *first_vowel_ticks)
{
    if (!pair || !first_vowel_ticks) return -1;
    int coda = coda_index(pair->first_coda);
    int next = onset_index(pair->second_initial);
    int initial = coda_index(pair->first_initial);
    if (coda < 0 || next < 0 ||
        (initial < 0 && pair->first_initial != RELOQ_DE_PHONE_h &&
         pair->first_initial != RELOQ_DE_PHONE_NONE)) return 1;
    return vowel_duration(pair->first_vowel, initial >= 0,
                          first_vowel_ticks);
}

int reloq_german_connected_first_closure(
    const struct reloq_german_connected_pair *pair,
    int64_t first_vowel_onset, int64_t first_send_end,
    int64_t *first_closure_tick, int64_t *first_coda_send_end)
{
    if (!pair || !first_closure_tick || !first_coda_send_end ||
        first_vowel_onset < 0)
        return -1;
    int64_t duration = 0;
    int status = reloq_german_connected_first_duration(pair, &duration);
    if (status) return status;
    int coda = coda_index(pair->first_coda);
    int next = onset_index(pair->second_initial);
    if (first_vowel_onset > INT64_MAX - duration - 100)
        return -1;
    int64_t closure = first_vowel_onset + duration;
    if (pair->first_initial == RELOQ_DE_PHONE_NONE ?
          (first_vowel_onset != 0 || first_send_end != -1) :
        pair->first_initial == RELOQ_DE_PHONE_h ?
          (first_vowel_onset == 0 ||
           first_send_end < first_vowel_onset ||
           first_send_end >= closure) :
          (first_vowel_onset == 0 || first_send_end < 0 ||
           first_send_end >= first_vowel_onset))
        return -1;
    int64_t end = closure + coda_send_offset[coda][next];
    *first_closure_tick = closure;
    *first_coda_send_end = end;
    return 0;
}

int reloq_german_connected_pair_landmarks(
    const struct reloq_german_connected_pair *pair,
    int64_t first_vowel_onset, int64_t first_send_end,
    int64_t second_vowel_onset,
    struct reloq_german_connected_landmarks *out)
{
    if (!pair || !out || second_vowel_onset < 0) return -1;
    int64_t closure, end, duration;
    int status = reloq_german_connected_first_closure(
        pair, first_vowel_onset, first_send_end, &closure, &end);
    if (status) return status;
    int coda = coda_index(pair->first_coda);
    int next = onset_index(pair->second_initial);
    if (vowel_duration(pair->first_vowel,
                       coda_index(pair->first_initial) >= 0,
                       &duration)) return -1;
    if (end > INT64_MAX - next_f0_lead[coda][next] ||
        second_vowel_onset <= end + next_f0_lead[coda][next])
        return -1;

    *out = (struct reloq_german_connected_landmarks){
        duration, closure, end, end + next_f0_lead[coda][next],
        pair->first_coda == pair->second_initial};
    return 0;
}

int reloq_german_connected_pair_f0_append(
    const struct reloq_german_connected_pair *pair,
    int64_t first_vowel_onset, int64_t first_send_end,
    int64_t second_vowel_onset, int64_t second_vowel_closure,
    struct reloq_target_builder *builder,
    struct reloq_german_connected_landmarks *out)
{
    if (!builder || !out) return -1;
    struct reloq_german_connected_landmarks planned;
    int result = reloq_german_connected_pair_landmarks(
        pair, first_vowel_onset, first_send_end, second_vowel_onset,
        &planned);
    if (result) return result;
    if (second_vowel_closure <= second_vowel_onset ||
        second_vowel_closure - second_vowel_onset > INT32_MAX)
        return -1;
    int64_t second_span = second_vowel_closure - second_vowel_onset;
    int64_t first_f0 = pair->first_initial == RELOQ_DE_PHONE_NONE ?
        planned.first_vowel_ticks * 5 / 100 : first_send_end / 20;
    int64_t first_late = first_vowel_onset +
        planned.first_vowel_ticks * 75 / 100;
    int64_t second_15 = second_vowel_onset + second_span * 15 / 100;
    int64_t second_70 = second_vowel_onset + second_span * 70 / 100;
    int64_t second_99 = second_vowel_onset + second_span * 99 / 100;
    if (first_f0 >= first_late ||
        first_late >= planned.second_early_f0_tick ||
        planned.second_early_f0_tick >= second_15 ||
        second_15 >= second_70 || second_70 >= second_99 ||
        second_99 >= second_vowel_closure) return -1;

    const struct reloq_target_event events[] = {
        {RELOQ_TARGET_F0, first_f0,
         pair->first_initial == RELOQ_DE_PHONE_h ? 1156 : 1153},
        {RELOQ_TARGET_F0, first_late, 1224},
        {RELOQ_TARGET_F0, planned.second_early_f0_tick,
         pair->second_initial == RELOQ_DE_PHONE_h ? 1046 : 1043},
        {RELOQ_TARGET_F0, second_15, 1293},
        {RELOQ_TARGET_F0, second_70, 793},
        {RELOQ_TARGET_F0, second_99, 740}
    };
    if (reloq_target_builder_append(builder, 0, events,
                                    sizeof events / sizeof events[0]))
        return -1;
    *out = planned;
    return 0;
}

/* Closure formants for /k/ depend on the first vowel's place, as in the
 * independently authored final-stop tract. The connected /k/ -> /t/
 * release instead moves to the following alveolar gesture. */
struct kt_vowel_tract {
    enum reloq_german_phone phone;
    int16_t f1, f2, f3;
};

static const struct kt_vowel_tract kt_vowels[] = {
    {RELOQ_DE_PHONE_A, 500, 1750, 2200},
    {RELOQ_DE_PHONE_E, 300, 1900, 2300},
    {RELOQ_DE_PHONE_I, 300, 1900, 2300},
    /* The following alveolar /t/ pulls back rounded-vowel F1 to 300 at
     * this nonfinal /k/ closure; an isolated final /k/ retains 500/400. */
    {RELOQ_DE_PHONE_O, 300, 1500, 2300},
    {RELOQ_DE_PHONE_U, 300, 1000, 2300},
    {RELOQ_DE_PHONE_a, 500, 1750, 2200},
    {RELOQ_DE_PHONE_e, 300, 1900, 2500},
    {RELOQ_DE_PHONE_i, 300, 2200, 2400},
    {RELOQ_DE_PHONE_o, 300, 960, 2300},
    {RELOQ_DE_PHONE_u, 300, 1000, 2300}
};

static void target(struct reloq_target_event *events, size_t *used,
                   enum reloq_target_stream stream, int64_t tick,
                   int32_t value)
{
    events[(*used)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_connected_kt_coda_append(
    const struct reloq_german_connected_pair *pair,
    int64_t first_closure_tick, struct reloq_target_builder *builder,
    int64_t *second_onset_origin)
{
    if (!pair || !builder || !second_onset_origin ||
        first_closure_tick < 0 || first_closure_tick > INT64_MAX - 72)
        return -1;
    if (pair->first_coda != RELOQ_DE_PHONE_k ||
        pair->second_initial != RELOQ_DE_PHONE_t) return 1;
    const struct kt_vowel_tract *v = NULL;
    for (size_t i = 0; i < sizeof kt_vowels / sizeof kt_vowels[0]; ++i)
        if (pair->first_vowel == kt_vowels[i].phone) {
            v = &kt_vowels[i];
            break;
        }
    if (!v) return 1;

    const int64_t closure = first_closure_tick;
    const int64_t release = closure + 52;
    const int64_t burst = release - 12;
    const int32_t from[5] = {v->f1,v->f2,v->f3,3600,3900};
    const int32_t to[5] = {300,1800,2300,3600,3900};
    const int32_t width[5] = {120,100,150,330,260};
    struct reloq_target_event events[100];
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

    /* The six parallel noise bands have the same burst topology. Only A2F
     * is energized for this place transition; the other zero holds still
     * matter because the first vowel and next onset share one timeline. */
    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        int32_t strength = s == RELOQ_TARGET_A2F ? 60 : 0;
        target(events,&used,s,closure,0);
        target(events,&used,s,burst,0);
        target(events,&used,s,burst,strength);
        target(events,&used,s,release,strength);
    }
    target(events,&used,RELOQ_TARGET_AF,closure,0);
    target(events,&used,RELOQ_TARGET_AF,burst,0);
    target(events,&used,RELOQ_TARGET_AF,burst,54);
    target(events,&used,RELOQ_TARGET_AF,release,54);
    target(events,&used,RELOQ_TARGET_AV,closure,0);
    target(events,&used,RELOQ_TARGET_AV,burst,0);
    target(events,&used,RELOQ_TARGET_AV,burst,0);
    target(events,&used,RELOQ_TARGET_AV,release,0);
    target(events,&used,RELOQ_TARGET_AV,release,0);
    target(events,&used,RELOQ_TARGET_AV,release+20,0);
    target(events,&used,RELOQ_TARGET_AH,closure,0);
    target(events,&used,RELOQ_TARGET_AH,burst,0);
    target(events,&used,RELOQ_TARGET_AH,burst,0);
    target(events,&used,RELOQ_TARGET_AH,release,0);
    target(events,&used,RELOQ_TARGET_AH,release,50);
    target(events,&used,RELOQ_TARGET_AH,release+20,50);
    target(events,&used,RELOQ_TARGET_TL,closure,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,release,0);
    target(events,&used,RELOQ_TARGET_OQ,closure,56);
    target(events,&used,RELOQ_TARGET_OQ,burst,56);
    target(events,&used,RELOQ_TARGET_DI,closure,0);
    target(events,&used,RELOQ_TARGET_DI,burst,0);
    if (reloq_target_builder_append(builder, 0, events, used)) return -1;
    *second_onset_origin = release + 20;
    return 0;
}

/* The isolated /t/ onset's feature values are reused as phone-class
 * coefficients. A connected onset has its own timing: the /k/ release
 * anchors a short closure, the burst moves near the first send's end, and
 * the following vowel begins at a separately translated acoustic anchor.
 * Both short and long vowel durations use the same onset by quality. */
struct t_following_vowel {
    enum reloq_german_phone phone;
    int16_t onset;
    int8_t front;
};

static const struct t_following_vowel t_vowels[] = {
    {RELOQ_DE_PHONE_A, 89, 0}, {RELOQ_DE_PHONE_E, 95, 1},
    {RELOQ_DE_PHONE_I, 95, 1}, {RELOQ_DE_PHONE_O, 89, 0},
    {RELOQ_DE_PHONE_U, 67, 0}, {RELOQ_DE_PHONE_a, 89, 0},
    {RELOQ_DE_PHONE_e, 113, 1}, {RELOQ_DE_PHONE_i, 113, 1},
    {RELOQ_DE_PHONE_o, 107, 0}, {RELOQ_DE_PHONE_u, 80, 0}
};

int reloq_german_connected_kt_onset_landmarks(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick,
    int64_t *first_send_end, int64_t *second_vowel_onset)
{
    if (!pair || !first_send_end || !second_vowel_onset ||
        first_coda_release_tick < 0 ||
        first_coda_release_tick > INT64_MAX - 256)
        return -1;
    if (pair->first_coda != RELOQ_DE_PHONE_k ||
        pair->second_initial != RELOQ_DE_PHONE_t) return 1;
    const struct t_following_vowel *v = NULL;
    for (size_t i = 0; i < sizeof t_vowels / sizeof t_vowels[0]; ++i)
        if (t_vowels[i].phone == second_vowel) {
            v = &t_vowels[i];
            break;
        }
    if (!v) return 1;
    const int64_t release = first_coda_release_tick;
    const int64_t isolated_burst_end = v->front ? 23 : 17;
    const int64_t burst_end = release + 83 + isolated_burst_end;
    /* Connected /t/ shortens its post-burst aspiration to three fifths of
     * the isolated interval. This single ratio covers U/e/i/o/u as well as
     * the initial A/E/I/O/a examples; fixed +54 happened to fit only the
     * latter group. Integer truncation is part of the observed timing. */
    const int64_t vowel = burst_end +
        3 * (v->onset - isolated_burst_end) / 5;
    if (release + 88 >= burst_end || burst_end >= vowel) return -1;
    *first_send_end = burst_end;
    *second_vowel_onset = vowel;
    return 0;
}

int reloq_german_connected_kt_onset_append(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick, int64_t second_final_release_tick,
    struct reloq_target_builder *builder,
    int64_t *first_send_end, int64_t *second_vowel_onset)
{
    if (!builder || !first_send_end || !second_vowel_onset) return -1;
    int64_t burst_end, vowel;
    int status = reloq_german_connected_kt_onset_landmarks(
        pair, second_vowel, first_coda_release_tick, &burst_end, &vowel);
    if (status) return status;
    if (second_final_release_tick <= vowel) return -1;
    const struct t_following_vowel *v = NULL;
    for (size_t i = 0; i < sizeof t_vowels / sizeof t_vowels[0]; ++i)
        if (t_vowels[i].phone == second_vowel) {
            v = &t_vowels[i];
            break;
        }
    /* The landmarks query has already validated this vowel class. */
    if (!v) return -1;
    const int64_t release = first_coda_release_tick;
    const int64_t origin = release + 20;
    const int64_t burst = release + 88;

    struct reloq_target_event events[100];
    size_t used = 0;
    const int32_t initial_f[] = {300,1600,2500,3500,4200};
    const int32_t burst_f[] = {
        second_vowel == RELOQ_DE_PHONE_u ? 280 : 300,
        v->front ? 1700 : 1600, v->front ? 2700 : 2500,
        3500,4200};
    const int32_t widths[] = {120,100,150,330,260};
    for (size_t i = 0; i < 5; ++i) {
        enum reloq_target_stream s =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)i);
        target(events,&used,s,origin,initial_f[i]);
        target(events,&used,s,burst_end,burst_f[i]);
        s = (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)i);
        target(events,&used,s,origin,widths[i]);
        target(events,&used,s,burst_end,widths[i]);
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
        int32_t gain = s == RELOQ_TARGET_A5F ? 56 : 0;
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
    target(events,&used,RELOQ_TARGET_AV,vowel,0);
    target(events,&used,RELOQ_TARGET_AH,origin,0);
    target(events,&used,RELOQ_TARGET_AH,burst,0);
    target(events,&used,RELOQ_TARGET_AH,burst,0);
    target(events,&used,RELOQ_TARGET_AH,burst_end,0);
    target(events,&used,RELOQ_TARGET_AH,burst_end,47);
    target(events,&used,RELOQ_TARGET_AH,vowel,47);
    target(events,&used,RELOQ_TARGET_TL,origin,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,burst,0);
    target(events,&used,RELOQ_TARGET_TL,burst_end,0);
    target(events,&used,RELOQ_TARGET_OQ,origin,56);
    target(events,&used,RELOQ_TARGET_OQ,burst,56);
    target(events,&used,RELOQ_TARGET_DI,origin,0);
    target(events,&used,RELOQ_TARGET_DI,burst,0);
    if (reloq_target_builder_append(builder, 0, events, used)) return -1;
    *first_send_end = burst_end;
    *second_vowel_onset = vowel;
    return 0;
}
