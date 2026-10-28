#include "reloq_german_terminal_fricative.h"

#include <stdint.h>

/* Only /s/ in the short front-vowel class changes its F2 constriction;
 * /f/ retains three broad vowel classes. The duration and noise resonator
 * are properties of the final consonant, not an utterance recording. */
static int vowel_class(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_O: return 0;
    case RELOQ_DE_PHONE_E: case RELOQ_DE_PHONE_I:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i: return 1;
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_o:
    case RELOQ_DE_PHONE_u: return 2;
    default: return -1;
    }
}

static void point(struct reloq_target_event *events, size_t *count,
                  enum reloq_target_stream stream, int64_t offset,
                  int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, offset, value};
}

int reloq_german_terminal_fricative_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t closure_tick,
    struct reloq_target_builder *builder, int64_t *release_tick)
{
    if (!phones || !builder || !release_tick || closure_tick < 0 ||
        !phone_count || vowel_index >= phone_count) return -1;
    if ((phone_count != 2 && phone_count != 3) ||
        (vowel_index != 0 && vowel_index != 1) ||
        vowel_index + 2 != phone_count ||
        (vowel_index && phones[0].phone != RELOQ_DE_PHONE_h &&
         phones[0].phone != RELOQ_DE_PHONE_p &&
         phones[0].phone != RELOQ_DE_PHONE_t &&
         phones[0].phone != RELOQ_DE_PHONE_k)) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index || phones[i].syllable_index ||
            phones[i].stress != 1 || !phones[i].stress_explicit)
            return 1;
    const int vowel = vowel_class(phones[vowel_index].phone);
    if (vowel < 0) return 1;
    const enum reloq_german_phone coda = phones[phone_count - 1].phone;
    if (coda != RELOQ_DE_PHONE_s && coda != RELOQ_DE_PHONE_f) return 1;
    const int s = coda == RELOQ_DE_PHONE_s;
    const int64_t duration = s ? 155 : 150;
    if (closure_tick > INT64_MAX - duration - 30) return -1;

    const int32_t oral[5] = {
        s ? 300 : 400,
        s ? (phones[vowel_index].phone == RELOQ_DE_PHONE_e ||
             phones[vowel_index].phone == RELOQ_DE_PHONE_i ? 1800 : 1500) :
            (vowel == 0 ? 1000 : vowel == 1 ? 1450 : 750),
        s ? 2600 : vowel == 1 ? 2500 : 2300,
        s ? 3400 : 3600,
        s ? 4400 : 3900
    };
    const int32_t bandwidth[5] = {120, 100, 150, 330, 260};
    struct reloq_target_event events[80];
    size_t count = 0;
    for (size_t i = 0; i < 5; ++i) {
        enum reloq_target_stream formant =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)i);
        enum reloq_target_stream width =
            (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)i);
        point(events, &count, formant, 0, oral[i]);
        point(events, &count, formant, duration, oral[i]);
        if (i >= 3)
            point(events, &count, formant, duration + 30,
                  i == 3 ? 3600 : 3900);
        point(events, &count, width, 0, bandwidth[i]);
        point(events, &count, width, duration, bandwidth[i]);
        point(events, &count, width, duration + 30, bandwidth[i]);
    }
    for (enum reloq_target_stream stream = RELOQ_TARGET_FNP;
         stream <= RELOQ_TARGET_FNZ; ++stream) {
        point(events, &count, stream, 0, 200);
        point(events, &count, stream, duration, 200);
        point(events, &count, stream, duration + 30, 200);
    }
    point(events, &count, RELOQ_TARGET_FTP, duration, 250);
    point(events, &count, RELOQ_TARGET_FTZ, duration, 250);
    point(events, &count, RELOQ_TARGET_FL, duration, 0);
    for (enum reloq_target_stream stream = RELOQ_TARGET_TL;
         stream <= RELOQ_TARGET_AH; ++stream) {
        if (stream == RELOQ_TARGET_FL) continue;
        int32_t level = stream == RELOQ_TARGET_OQ ? 56 : 0;
        point(events, &count, stream, 0, level);
        point(events, &count, stream, duration, level);
        point(events, &count, stream, duration + 30, level);
    }
    for (enum reloq_target_stream stream = RELOQ_TARGET_AF;
         stream <= RELOQ_TARGET_AB; ++stream) {
        int32_t level = stream == RELOQ_TARGET_AF ? (s ? 62 : 58) :
            stream == (s ? RELOQ_TARGET_A5F : RELOQ_TARGET_AB) ?
                (s ? 50 : 60) : 0;
        point(events, &count, stream, 0, level);
        point(events, &count, stream, duration, level);
        point(events, &count, stream, duration + 30, 0);
    }
    if (count > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder, closure_tick, events, count))
        return -1;
    *release_tick = closure_tick + duration;
    return 0;
}

int reloq_german_terminal_fricative_append_marked(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t closure_tick,
    enum reloq_german_terminal_mark mark,
    struct reloq_target_builder *builder, int64_t *release_tick,
    int64_t *end_tick)
{
    if (!phones || !builder || !release_tick || !end_tick ||
        closure_tick < 0) return -1;
    struct reloq_german_terminal_timing terminal;
    if (reloq_german_terminal_timing(mark, &terminal)) return -1;
    const int32_t window = terminal.window_ticks - 30;
    const enum reloq_german_phone coda = phone_count && phone_count <= 3 ?
        phones[phone_count - 1].phone : RELOQ_DE_PHONE_NONE;
    const int64_t duration = coda == RELOQ_DE_PHONE_s ? 155 :
        coda == RELOQ_DE_PHONE_f ? 150 : 0;
    /* Reject overflow before the target builder is touched, including the
     * quiet continuation beyond the coda's 30-tick active fade. */
    if (duration && closure_tick > INT64_MAX - duration - window) return -1;
    int64_t release;
    const int result = reloq_german_terminal_fricative_append(
        phones, phone_count, vowel_index, closure_tick, builder, &release);
    if (result) return result;
    *release_tick = release;
    *end_tick = release + window;
    return 0;
}
