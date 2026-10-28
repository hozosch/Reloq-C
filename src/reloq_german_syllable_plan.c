#include "reloq_german_syllable_plan.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int is_nucleus(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_a: case RELOQ_DE_PHONE_A:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_i: case RELOQ_DE_PHONE_I:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_u: case RELOQ_DE_PHONE_U:
    case RELOQ_DE_PHONE_y: case RELOQ_DE_PHONE_Y:
    case RELOQ_DE_PHONE_AT:
    case RELOQ_DE_PHONE_Q_E_COLON:
    case RELOQ_DE_PHONE_Q_oe: case RELOQ_DE_PHONE_Q_OE:
    case RELOQ_DE_PHONE_Q_aj: case RELOQ_DE_PHONE_Q_aw:
    case RELOQ_DE_PHONE_Q_oj:
    case RELOQ_DE_PHONE_Q_a_TILDE: case RELOQ_DE_PHONE_Q_E_TILDE:
    case RELOQ_DE_PHONE_Q_o_TILDE: case RELOQ_DE_PHONE_Q_oe_TILDE:
        return 1;
    default:
        return 0;
    }
}

static int scan(const struct reloq_german_phone_node *phones,
                size_t phone_count, struct reloq_german_syllable *out,
                size_t *count, reloq_german_cancel_check cancelled,
                void *context)
{
    size_t used = 0, pos = 0;
    if (cancelled && cancelled(context)) return 1;
    while (pos < phone_count) {
        if (cancelled && cancelled(context)) return 1;
        const struct reloq_german_phone_node *first = phones + pos;
        if (first->phone <= RELOQ_DE_PHONE_NONE ||
            first->phone > RELOQ_DE_PHONE_Q_oe_TILDE ||
            first->stress > 2 || first->stress_explicit > 1 ||
            first->syllable_index != used ||
            (pos == 0 ? (first->word_index != 0 ||
                         first->syllable_in_word != 0) :
             (first->word_index == phones[pos - 1].word_index ?
              first->syllable_in_word !=
                  phones[pos - 1].syllable_in_word + 1 :
              first->word_index != phones[pos - 1].word_index + 1 ||
                  first->syllable_in_word != 0))) return -1;

        size_t begin = pos, first_nucleus = SIZE_MAX, last_nucleus = 0;
        uint8_t contiguous = 1;
        while (pos < phone_count &&
               phones[pos].syllable_index == first->syllable_index) {
            const struct reloq_german_phone_node *node = &phones[pos];
            if ((pos & 255) == 0 && cancelled && cancelled(context))
                return 1;
            if (node->word_index != first->word_index ||
                node->syllable_in_word != first->syllable_in_word ||
                node->stress != first->stress ||
                node->stress_explicit != first->stress_explicit ||
                node->phone <= RELOQ_DE_PHONE_NONE ||
                node->phone > RELOQ_DE_PHONE_Q_oe_TILDE)
                return -1;
            if (is_nucleus(node->phone)) {
                if (first_nucleus == SIZE_MAX) first_nucleus = pos;
                else if (last_nucleus != pos) contiguous = 0;
                last_nucleus = pos + 1;
            }
            ++pos;
        }
        enum reloq_german_syllable_boundary boundary =
            pos == phone_count ? RELOQ_DE_BOUNDARY_UTTERANCE :
            phones[pos].word_index != first->word_index ?
            RELOQ_DE_BOUNDARY_WORD : RELOQ_DE_BOUNDARY_SYLLABLE;
        if (pos < phone_count &&
            (phones[pos].syllable_index != used + 1 ||
             phones[pos].word_index < first->word_index ||
             phones[pos].word_index > first->word_index + 1 ||
             (boundary == RELOQ_DE_BOUNDARY_WORD &&
              phones[pos].syllable_in_word != 0))) return -1;
        if (out) {
            struct reloq_german_syllable *result = &out[used];
            result->first_phone = begin;
            result->phone_count = pos - begin;
            result->word_index = first->word_index;
            result->syllable_in_word = first->syllable_in_word;
            result->onset_count = first_nucleus == SIZE_MAX ?
                pos - begin : first_nucleus - begin;
            result->nucleus_count = first_nucleus == SIZE_MAX ?
                0 : last_nucleus - first_nucleus;
            result->coda_count = first_nucleus == SIZE_MAX ?
                0 : pos - last_nucleus;
            result->nucleus_contiguous = contiguous;
            result->stress = first->stress;
            result->stress_explicit = first->stress_explicit;
            result->boundary_after = boundary;
        }
        ++used;
    }
    if (cancelled && cancelled(context)) return 1;
    *count = used;
    return 0;
}

int reloq_german_syllable_plan_build_checked(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    struct reloq_german_syllable *syllables, size_t capacity, size_t *count,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    if ((phone_count && !phones) || !count || (capacity && !syllables))
        return -1;
    size_t needed = 0;
    int result = scan(phones, phone_count, NULL, &needed,
                      cancelled, cancel_context);
    if (result) return result;
    if (!syllables || capacity < needed) {
        *count = needed;
        return needed ? -2 : 0;
    }
    if (cancelled && cancelled(cancel_context)) return 1;
    size_t written = 0;
    if (cancelled && needed) {
        if (needed > SIZE_MAX / sizeof *syllables) return -3;
        struct reloq_german_syllable *staged =
            malloc(needed * sizeof *staged);
        if (!staged) return -3;
        result = scan(phones, phone_count, staged, &written,
                      cancelled, cancel_context);
        if (!result) memcpy(syllables, staged, written * sizeof *staged);
        free(staged);
        if (result) return result;
    } else {
        result = scan(phones, phone_count, syllables, &written,
                      NULL, NULL);
        if (result) return result; /* immutable input after first pass */
    }
    *count = written;
    return 0;
}

int reloq_german_syllable_plan_build(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    struct reloq_german_syllable *syllables, size_t capacity, size_t *count)
{
    return reloq_german_syllable_plan_build_checked(
        phones, phone_count, syllables, capacity, count, NULL, NULL);
}
