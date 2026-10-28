#include "reloq_german_initial_voiced_stop_envelope.h"

#include <limits.h>

static int is_vowel(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
        return 1;
    default:
        return 0;
    }
}

static int is_coda(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

static enum reloq_target_stream active_noise(
    enum reloq_german_phone initial, enum reloq_german_phone vowel)
{
    if (initial == RELOQ_DE_PHONE_b) return RELOQ_TARGET_AB;
    if (initial == RELOQ_DE_PHONE_d) return RELOQ_TARGET_A5F;
    if (vowel == RELOQ_DE_PHONE_I || vowel == RELOQ_DE_PHONE_e ||
        vowel == RELOQ_DE_PHONE_i) return RELOQ_TARGET_A3F;
    return RELOQ_TARGET_A2F;
}

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_initial_voiced_stop_envelope_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    enum reloq_german_terminal_mark mark,
    int64_t origin_tick, int64_t burst_tick, int64_t onset_tick,
    int64_t closure_tick, int64_t release_tick,
    struct reloq_target_builder *builder)
{
    if (!phones || !builder || !phone_count || origin_tick < 0 ||
        burst_tick < origin_tick || onset_tick <= burst_tick ||
        closure_tick <= onset_tick || release_tick < closure_tick)
        return -1;
    if (mark != RELOQ_DE_TERMINAL_PERIOD ||
        (phone_count != 2 && phone_count != 3) ||
        (phones[0].phone != RELOQ_DE_PHONE_b &&
         phones[0].phone != RELOQ_DE_PHONE_d &&
         phones[0].phone != RELOQ_DE_PHONE_g) ||
        !is_vowel(phones[1].phone) ||
        (phone_count == 2 && release_tick != closure_tick) ||
        (phone_count == 3 && (!is_coda(phones[2].phone) ||
                              release_tick == closure_tick)))
        return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index || phones[i].syllable_index ||
            phones[i].syllable_in_word || phones[i].stress != 1 ||
            !phones[i].stress_explicit) return 1;
    const int64_t s = onset_tick - origin_tick;
    const int64_t b = burst_tick - origin_tick;
    const int64_t c = closure_tick - onset_tick;
    const int64_t r = release_tick - origin_tick;
    if (s < 10 || s >= 800 || b < 5 || c <= 0 ||
        c > INT32_MAX / 99 || release_tick > INT64_MAX - 460)
        return -1;
    const int64_t pitch[] = {s / 10,
                             s + 35 * c / 100,
                             s + 70 * c / 100,
                             s + 99 * c / 100};
    if (pitch[0] >= pitch[1] || pitch[1] >= pitch[2] ||
        pitch[2] >= pitch[3] || pitch[3] >= s + c) return -1;

    struct reloq_target_event events[52];
    size_t count = 0;
    const int32_t pitch_value[] = {1046, 1296, 796, 743};
    for (size_t i = 0; i < 4; ++i)
        put(events, &count, RELOQ_TARGET_F0, pitch[i], pitch_value[i]);
    put(events, &count, RELOQ_TARGET_FL, 0, 0);
    put(events, &count, RELOQ_TARGET_FL, r, 0);
    put(events, &count, RELOQ_TARGET_FTP, 0, 250);
    put(events, &count, RELOQ_TARGET_FTP, r, 250);
    put(events, &count, RELOQ_TARGET_FTZ, 0, 250);
    put(events, &count, RELOQ_TARGET_FTZ, r, 250);
    for (enum reloq_target_stream stream = RELOQ_TARGET_F4;
         stream <= RELOQ_TARGET_F5; ++stream) {
        const int32_t value = stream == RELOQ_TARGET_F4 ?
            (phones[0].phone == RELOQ_DE_PHONE_d ? 4000 : 3600) :
            phones[0].phone == RELOQ_DE_PHONE_d ? 4300 :
            phones[0].phone == RELOQ_DE_PHONE_b ? 4000 : 3900;
        put(events, &count, stream, 0, value);
        put(events, &count, stream, s, value);
    }
    const int32_t widths[] = {120, 100, 150, 330, 260};
    for (size_t i = 0; i < 5; ++i) {
        const enum reloq_target_stream stream =
            (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)i);
        put(events, &count, stream, 0, widths[i]);
        put(events, &count, stream, s, widths[i]);
    }
    const enum reloq_target_stream nasals[] = {
        RELOQ_TARGET_FNP, RELOQ_TARGET_FNZ};
    for (size_t i = 0; i < 2; ++i) {
        put(events, &count, nasals[i], 0, 200);
        put(events, &count, nasals[i], s, 200);
    }
    const enum reloq_target_stream active =
        active_noise(phones[0].phone, phones[1].phone);
    for (enum reloq_target_stream stream = RELOQ_TARGET_A1F;
         stream <= RELOQ_TARGET_AB; ++stream) {
        if (stream == active) continue;
        put(events, &count, stream, 0, 0);
        put(events, &count, stream, b, 0);
        put(events, &count, stream, b, 0);
        put(events, &count, stream, s, 0);
    }
    put(events, &count, RELOQ_TARGET_OQ, 0, 56);
    put(events, &count, RELOQ_TARGET_OQ, b, 56);
    put(events, &count, RELOQ_TARGET_DI, 0, 0);
    put(events, &count, RELOQ_TARGET_DI, b, 0);
    if (count != sizeof events / sizeof events[0]) return -1;
    return reloq_target_builder_append(builder, origin_tick, events, count) ?
        -1 : 0;
}
