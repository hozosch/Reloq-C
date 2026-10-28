#include "reloq_german_connected_words.h"

#include "reloq_german_connected_cvc_index.h"
#include "reloq_german_text.h"
#include "reloq_utf8.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int letter(uint32_t scalar)
{
    return (scalar >= 'A' && scalar <= 'Z') ||
           (scalar >= 'a' && scalar <= 'z') ||
           scalar == 0xc4 || scalar == 0xd6 || scalar == 0xdc ||
           scalar == 0xe4 || scalar == 0xf6 || scalar == 0xfc ||
           scalar == 0xdf;
}

static int vowel_base(uint32_t scalar)
{
    return scalar == 'A' || scalar == 'O' || scalar == 'U' ||
           scalar == 'a' || scalar == 'o' || scalar == 'u';
}

/* Return 1 for a word, 0 if the next scalar is not a word letter. Input
 * UTF-8 has already been checked over its full bounded span. A base vowel
 * followed by combining diaeresis is part of the same word, as in the
 * existing text adapter. */
static int word_span(const unsigned char *utf8, size_t length,
                     size_t *offset, size_t *word_length)
{
    size_t at = *offset;
    while (at < length) {
        size_t next = at;
        uint32_t scalar;
        if (reloq_utf8_next(utf8, length, &next, &scalar) != 1 ||
            !letter(scalar)) break;
        at = next;
        if (vowel_base(scalar) && at < length) {
            next = at;
            uint32_t mark;
            if (reloq_utf8_next(utf8, length, &next, &mark) == 1 &&
                mark == 0x0308)
                at = next;
        }
    }
    *word_length = at - *offset;
    *offset = at;
    return *word_length != 0;
}

static int modeled(const struct reloq_german_phone_node *phones,
                   const struct reloq_german_phone_sequence_info *info)
{
    if (info->word_count != 1 || info->syllable_count != 1 ||
        (info->phone_count != 2 && info->phone_count != 3)) return 0;
    enum reloq_german_phone first = phones[0].phone;
    size_t vowel = first == RELOQ_DE_PHONE_h ||
                   first == RELOQ_DE_PHONE_p ||
                   first == RELOQ_DE_PHONE_t ||
                   first == RELOQ_DE_PHONE_k;
    if (info->phone_count != vowel + 2) return 0;
    enum reloq_german_phone last = phones[vowel + 1].phone;
    if (last != RELOQ_DE_PHONE_p && last != RELOQ_DE_PHONE_t &&
        last != RELOQ_DE_PHONE_k && last != RELOQ_DE_PHONE_s &&
        last != RELOQ_DE_PHONE_f) return 0;
    if (vowel && first != RELOQ_DE_PHONE_h &&
        (last == RELOQ_DE_PHONE_s || last == RELOQ_DE_PHONE_f)) return 0;
    switch (phones[vowel].phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
        break;
    default: return 0;
    }
    for (size_t i = 0; i < info->phone_count; ++i)
        if (phones[i].stress != 1 || !phones[i].stress_explicit ||
            phones[i].word_index != 0 || phones[i].syllable_in_word != 0)
            return 0;
    return 1;
}

static int group_for_word(const unsigned char *word, size_t length,
                          unsigned char **group, size_t *group_length)
{
    size_t needed = 0, error = 0;
    int status = reloq_german_text_annotate(word, length, NULL, 0,
                                              &needed, &error);
    if (status == RELOQ_DE_TEXT_UNCOVERED) return 0;
    if (status == RELOQ_DE_TEXT_NO_MEMORY) return -3;
    if (status != RELOQ_DE_TEXT_OK) return -1;
    if (needed == SIZE_MAX) return -3;
    unsigned char *temporary = malloc(needed + 1);
    if (!temporary) return -3;
    size_t written = needed;
    status = reloq_german_text_annotate(word, length, temporary,
                                          needed + 1, &written, &error);
    if (status != RELOQ_DE_TEXT_OK || needed != written) {
        free(temporary);
        return status == RELOQ_DE_TEXT_NO_MEMORY ? -3 : -1;
    }
    struct reloq_german_phone_node nodes[3];
    struct reloq_german_phone_sequence_info info = {0};
    if (needed < 3 || temporary[0] != '`' || temporary[1] != '[' ||
        temporary[needed - 1] != ']' ||
        memchr(temporary + 2, ']', needed - 3) ||
        reloq_german_phone_sequence_build(temporary, needed, 0,
                                            nodes, 3, &info) ||
        !modeled(nodes, &info)) {
        free(temporary);
        return 0;
    }
    *group = temporary;
    *group_length = needed;
    return 1;
}

int reloq_german_connected_words_prepare(
    const unsigned char *utf8, size_t length,
    unsigned char *annotation, size_t capacity,
    size_t *byte_length, size_t *error_offset,
    struct reloq_german_connected_plan *plan)
{
    if (!utf8 || !byte_length || (capacity && !annotation)) return -1;
    size_t at = 0, word_offset[2], word_length[2];
    while (at < length) {
        size_t next = at;
        uint32_t scalar;
        if (reloq_utf8_next(utf8, length, &next, &scalar) != 1) {
            if (error_offset) *error_offset = at;
            return -1;
        }
        at = next;
    }
    at = 0;
    word_offset[0] = at;
    if (!word_span(utf8, length, &at, &word_length[0]) ||
        at >= length || utf8[at] != ' ') {
        if (error_offset) *error_offset = at;
        return 0;
    }
    ++at;
    word_offset[1] = at;
    if (!word_span(utf8, length, &at, &word_length[1]) ||
        at + 1 != length || utf8[at] != '.') {
        if (error_offset) *error_offset = at;
        return 0;
    }

    unsigned char *groups[2] = {NULL, NULL};
    size_t group_length[2] = {0, 0};
    int status = 0;
    for (size_t i = 0; i < 2; ++i) {
        status = group_for_word(utf8 + word_offset[i], word_length[i],
                                &groups[i], &group_length[i]);
        if (status != 1) {
            if (error_offset && status == 0) *error_offset = word_offset[i];
            goto done;
        }
    }
    /* prefix space + two groups + " `2 " + final period. Reserve a NUL. */
    if (group_length[1] > SIZE_MAX - 7 ||
        group_length[0] > SIZE_MAX - group_length[1] - 7) {
        status = -3;
        goto done;
    }
    size_t needed = group_length[0] + group_length[1] + 6;
    unsigned char *constructed = malloc(needed + 1);
    if (!constructed) {
        status = -3;
        goto done;
    }
    size_t second = 1 + group_length[0] + 4;
    constructed[0] = ' ';
    memcpy(constructed + 1, groups[0], group_length[0]);
    memcpy(constructed + 1 + group_length[0], " `2 ", 4);
    memcpy(constructed + second, groups[1], group_length[1]);
    constructed[needed - 1] = '.';
    constructed[needed] = 0;
    struct reloq_german_connected_plan candidate = {0};
    struct reloq_german_phone_sequence_info info = {0};
    if (reloq_german_phone_sequence_build(constructed, needed, 0,
                                           candidate.phones, 6, &info) ||
        info.word_count != 2 || info.syllable_count != 2 ||
        info.phone_count < 4 || info.phone_count > 6) {
        free(constructed);
        status = 0;
        goto done;
    }
    candidate.phone_count = info.phone_count;
    for (size_t i = 0; i < info.phone_count; ++i) {
        if (candidate.phones[i].word_index > 1) {
            status = 0;
            break;
        }
        ++candidate.word_phone_count[candidate.phones[i].word_index];
    }
    if (status != 1 || !candidate.word_phone_count[0] ||
        !candidate.word_phone_count[1]) {
        free(constructed);
        status = 0;
        goto done;
    }
    for (size_t i = 0; i < 2; ++i) {
        candidate.word_input_offset[i] = word_offset[i];
        candidate.word_input_length[i] = word_length[i];
        candidate.group_length[i] = group_length[i];
    }
    candidate.group_offset[0] = 1;
    candidate.group_offset[1] = second;
    candidate.boundary_offset = 2 + group_length[0];
    /* Phrase-dependent lexical stress is a separate missing model. */
    candidate.stress_explicitly_specified = 0;
    candidate.reference_spr_equivalent =
        reloq_german_connected_cvc_reference_pair(
            utf8 + word_offset[0], word_length[0],
            utf8 + word_offset[1], word_length[1]) == 1;
    if (!annotation) {
        *byte_length = needed;
    } else if (capacity <= needed) {
        *byte_length = needed;
        status = -2;
    } else {
        memmove(annotation, constructed, needed + 1);
        *byte_length = needed;
        if (plan) *plan = candidate;
    }
    free(constructed);
done:
    free(groups[0]);
    free(groups[1]);
    return status;
}

int reloq_german_connected_annotation_prepare(
    const unsigned char *annotation, size_t length,
    struct reloq_german_connected_plan *plan)
{
    if (!annotation || !plan) return -1;
    struct reloq_german_connected_plan candidate = {0};
    size_t at = length && annotation[0] == ' ' ? 1 : 0;
    for (size_t word = 0; word < 2; ++word) {
        size_t start = at;
        if (length - at < 4 || annotation[at] != '`' ||
            annotation[at + 1] != '[') return 0;
        at += 2;
        while (at < length && annotation[at] != ']') {
            if (annotation[at] == '`') return 0;
            ++at;
        }
        if (at == length) return 0;
        ++at;
        struct reloq_german_phone_node phones[3];
        struct reloq_german_phone_sequence_info info = {0};
        if (reloq_german_phone_sequence_build(annotation + start,
                                                at - start, 0, phones, 3,
                                                &info) ||
            !modeled(phones, &info)) return 0;
        candidate.word_phone_count[word] = info.phone_count;
        candidate.group_offset[word] = start;
        candidate.group_length[word] = at - start;
        candidate.word_input_offset[word] = start;
        candidate.word_input_length[word] = at - start;
        if (word == 0) {
            if (length - at < 4 || memcmp(annotation + at, " `2 ", 4))
                return 0;
            candidate.boundary_offset = at + 1;
            at += 4;
        }
    }
    if (at >= length || length - at != 1 || annotation[at] != '.') return 0;
    struct reloq_german_phone_sequence_info info = {0};
    if (reloq_german_phone_sequence_build(annotation, length, 0,
                                           candidate.phones, 6, &info) ||
        info.word_count != 2 || info.syllable_count != 2 ||
        info.phone_count != candidate.word_phone_count[0] +
                            candidate.word_phone_count[1]) return 0;
    candidate.phone_count = info.phone_count;
    candidate.stress_explicitly_specified = 1;
    candidate.reference_spr_equivalent = 0;
    *plan = candidate;
    return 1;
}
