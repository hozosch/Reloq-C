#include "reloq_german_hv_timing.h"

static int tested_vowel(enum reloq_german_phone phone)
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

static int valid_nodes(const struct reloq_german_phone_node *phones,
                       size_t phone_count)
{
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].phone <= RELOQ_DE_PHONE_NONE ||
            phones[i].phone > RELOQ_DE_PHONE_Q_oe_TILDE ||
            phones[i].stress > 2 || phones[i].stress_explicit > 1)
            return 0;
    return 1;
}

static int tested_coda(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

static int tested_fricative(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_s || phone == RELOQ_DE_PHONE_f;
}

static int coda_offset(enum reloq_german_phone vowel, unsigned stress,
                       int64_t *offset)
{
    /* Secondary stress uses the stressed coda offset even though its hV
     * vowel landmark is five ticks earlier than primary. */
    static const struct {
        enum reloq_german_phone vowel;
        int32_t unstressed;
        int32_t stressed;
    } offsets[] = {
        {RELOQ_DE_PHONE_A, 97, 97},
        {RELOQ_DE_PHONE_E, 97, 105},
        {RELOQ_DE_PHONE_I, 91, 98},
        {RELOQ_DE_PHONE_O, 91, 98},
        {RELOQ_DE_PHONE_U, 78, 84},
        {RELOQ_DE_PHONE_a, 163, 168},
        {RELOQ_DE_PHONE_e, 169, 182},
        {RELOQ_DE_PHONE_i, 156, 168},
        {RELOQ_DE_PHONE_o, 169, 182},
        {RELOQ_DE_PHONE_u, 156, 168}
    };
    for (size_t i = 0; i < sizeof offsets / sizeof offsets[0]; ++i) {
        if (vowel == offsets[i].vowel) {
            *offset = stress == 0 ? offsets[i].unstressed :
                                  offsets[i].stressed;
            return 0;
        }
    }
    return 1;
}

int reloq_german_hv_acoustic_anchor(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *onset_tick)
{
    if (!phones || !onset_tick || !phone_count) return -1;
    if (!valid_nodes(phones, phone_count)) return -1;
    if ((phone_count != 2 && phone_count != 3) ||
        phones[0].phone != RELOQ_DE_PHONE_h ||
        !tested_vowel(phones[1].phone)) return 1;
    if (phone_count == 3 && !tested_coda(phones[2].phone) &&
        !tested_fricative(phones[2].phone)) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != phones[0].word_index ||
            phones[i].syllable_index != phones[0].syllable_index ||
            phones[i].stress != phones[0].stress ||
            !phones[i].stress_explicit) return 1;

    *onset_tick = phones[0].stress == 1 ? 80 : 75;
    return 0;
}

int reloq_german_hvc_f1_target_anchor(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *coda_f1_tick)
{
    if (!phones || !coda_f1_tick || !phone_count) return -1;
    int64_t vowel_tick;
    int status = reloq_german_hv_acoustic_anchor(
        phones, phone_count, &vowel_tick);
    if (status != 0) return status;
    if (phone_count != 3 || !tested_coda(phones[2].phone)) return 1;

    int64_t offset;
    if (coda_offset(phones[1].phone, phones[0].stress, &offset)) return 1;
    *coda_f1_tick = vowel_tick + offset;
    return 0;
}

int reloq_german_vc_f1_target_anchor(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *coda_f1_tick)
{
    if (!phones || !coda_f1_tick || !phone_count) return -1;
    if (!valid_nodes(phones, phone_count)) return -1;
    if (phone_count != 2 || !tested_vowel(phones[0].phone) ||
        !tested_coda(phones[1].phone) ||
        phones[0].word_index != phones[1].word_index ||
        phones[0].syllable_index != phones[1].syllable_index ||
        phones[0].stress != phones[1].stress ||
        !phones[0].stress_explicit || !phones[1].stress_explicit) return 1;
    int64_t offset;
    if (coda_offset(phones[0].phone, phones[0].stress, &offset)) return 1;
    *coda_f1_tick = offset;
    return 0;
}

static int64_t first_send_lead(enum reloq_german_phone vowel,
                               enum reloq_german_phone stop)
{
    if (vowel == RELOQ_DE_PHONE_E || vowel == RELOQ_DE_PHONE_I ||
        vowel == RELOQ_DE_PHONE_e || vowel == RELOQ_DE_PHONE_i)
        return 25;
    else if (stop == RELOQ_DE_PHONE_p)
        return 30;
    else if (stop == RELOQ_DE_PHONE_t &&
             (vowel == RELOQ_DE_PHONE_O || vowel == RELOQ_DE_PHONE_a))
        return 50;
    else if (stop == RELOQ_DE_PHONE_k && vowel == RELOQ_DE_PHONE_a)
        return 20;
    else
        return 40;
}

int reloq_german_coda_hold_lead(
    enum reloq_german_phone vowel, enum reloq_german_phone stop,
    int64_t *lead_ticks)
{
    if (!lead_ticks) return -1;
    if (!tested_vowel(vowel) || !tested_coda(stop)) return 1;
    *lead_ticks = first_send_lead(vowel, stop);
    return 0;
}

int reloq_german_hvc_first_send_end(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *send_end_tick)
{
    if (!phones || !send_end_tick || !phone_count) return -1;
    int64_t closure;
    int status = reloq_german_hvc_f1_target_anchor(
        phones, phone_count, &closure);
    if (status != 0) return status;
    if (phone_count != 3) return 1;

    *send_end_tick = closure -
        first_send_lead(phones[1].phone, phones[2].phone);
    return 0;
}

int reloq_german_vc_first_send_end(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t *send_end_tick)
{
    if (!phones || !send_end_tick || !phone_count) return -1;
    int64_t closure;
    int status = reloq_german_vc_f1_target_anchor(
        phones, phone_count, &closure);
    if (status != 0) return status;
    if (phone_count != 2) return 1;

    *send_end_tick = closure -
        first_send_lead(phones[0].phone, phones[1].phone);
    return 0;
}
