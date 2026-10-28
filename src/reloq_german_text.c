#include "reloq_german_text.h"

#include "reloq_german_annotation.h"
#include "reloq_german_lexicon.h"
#include "reloq_german_input.h"
#include "reloq_utf8.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int word_letter(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           c == 0xc4 || c == 0xd6 || c == 0xdc || c == 0xe4 ||
           c == 0xf6 || c == 0xfc || c == 0xdf;
}

/* These are literal TEXT to the existing annotation scanner. Excluding the
 * backtick prevents a text input from introducing an ECI control or group. */
static int literal(unsigned char c)
{
    switch (c) {
    case ' ': case '\t': case '\r': case '\n':
    case '.': case ',': case '!': case '?': case ';': case ':':
    case '-': case '(': case ')': case '\'': case '"':
    case 0x84: /* low double quote */
    case 0x85: /* ellipsis */
    case 0x91: case 0x92: case 0x93: case 0x94: /* quotes */
    case 0x96: case 0x97: /* dashes */
        return 1;
    default:
        return 0;
    }
}

/* An encoded byte is exactly one Unicode scalar, except a German base vowel
 * and U+0308 together produce one byte. This maps lexical errors back to the
 * original UTF-8 input without storing a per-byte offset table. */
static size_t utf8_offset(const unsigned char *input, size_t input_length,
                          size_t encoded_offset,
                          reloq_german_cancel_check cancelled,
                          void *cancel_context)
{
    size_t pos = 0;
    for (size_t index = 0; pos < input_length; ++index) {
        if (cancelled && (index & 255) == 0 && cancelled(cancel_context))
            return SIZE_MAX;
        size_t start = pos;
        uint32_t scalar;
        if (reloq_utf8_next(input, input_length, &pos, &scalar) != 1)
            return input_length; /* already validated by input_encode */
        if (index == encoded_offset) return start;
        if ((scalar == 'A' || scalar == 'O' || scalar == 'U' ||
             scalar == 'a' || scalar == 'o' || scalar == 'u') &&
            pos < input_length) {
            size_t next = pos;
            uint32_t mark;
            if (reloq_utf8_next(input, input_length, &next, &mark) == 1 &&
                mark == 0x0308)
                pos = next;
        }
    }
    return input_length;
}

static int one_group(const unsigned char *word, size_t word_length,
                     unsigned char *fixed, size_t fixed_capacity,
                     unsigned char **group, size_t *group_length,
                     const struct reloq_lexicon *lexicon)
{
    size_t length = 0;
    int state = reloq_german_lexicon_word(lexicon, word, word_length,
                                          fixed, fixed_capacity, &length);
    *group = fixed;
    if (state == -2) {
        if (length == SIZE_MAX) return RELOQ_DE_TEXT_NO_MEMORY;
        *group = (unsigned char *)malloc(length + 1);
        if (!*group) return RELOQ_DE_TEXT_NO_MEMORY;
        state = reloq_german_lexicon_word(lexicon, word, word_length,
                                          *group, length + 1, &length);
    }
    if (state != 1) {
        if (*group != fixed) free(*group);
        return RELOQ_DE_TEXT_UNCOVERED;
    }

    /* A covered spelling must actually be a complete, valid SPR word. */
    struct reloq_german_spr_cursor cursor = {0};
    struct reloq_german_spr_event event;
    while ((state = reloq_german_spr_next(*group, length,
                                          &cursor, &event)) == 1) {}
    if (state != 0) {
        if (*group != fixed) free(*group);
        return RELOQ_DE_TEXT_UNCOVERED;
    }
    *group_length = length;
    return RELOQ_DE_TEXT_OK;
}

/* The first pass computes the exact result without touching caller storage.
 * The second writes to private memory after all lexical validation passed. */
static int convert(const unsigned char *encoded, size_t encoded_length,
                   unsigned char *annotation, size_t *length,
                   size_t *bad_offset,
                   const struct reloq_lexicon *lexicon,
                   reloq_german_cancel_check cancelled,
                   void *cancel_context)
{
    size_t at = 0, used = 0;
    while (at < encoded_length) {
        if (cancelled && cancelled(cancel_context))
            return RELOQ_DE_TEXT_CANCELLED;
        if (literal(encoded[at])) {
            if (used >= SIZE_MAX - 1) return RELOQ_DE_TEXT_NO_MEMORY;
            if (annotation) annotation[used] = encoded[at];
            ++used;
            ++at;
            continue;
        }
        if (!word_letter(encoded[at])) {
            *bad_offset = at;
            return RELOQ_DE_TEXT_INVALID;
        }
        size_t start = at;
        while (at < encoded_length && word_letter(encoded[at])) ++at;
        unsigned char fixed[256];
        unsigned char *group;
        size_t group_length;
        int status = one_group(encoded + start, at - start, fixed,
                               sizeof fixed, &group, &group_length, lexicon);
        if (status != RELOQ_DE_TEXT_OK) {
            if (status == RELOQ_DE_TEXT_UNCOVERED) *bad_offset = start;
            return status;
        }
        /* Reserve the extra byte required for the final NUL as well. */
        if (group_length >= SIZE_MAX - used) {
            if (group != fixed) free(group);
            return RELOQ_DE_TEXT_NO_MEMORY;
        }
        if (annotation) memcpy(annotation + used, group, group_length);
        used += group_length;
        if (group != fixed) free(group);
    }
    if (cancelled && cancelled(cancel_context))
        return RELOQ_DE_TEXT_CANCELLED;
    *length = used;
    return RELOQ_DE_TEXT_OK;
}

int reloq_german_text_annotate_checked(
    const unsigned char *input, size_t input_length,
    unsigned char *output, size_t capacity,
    size_t *byte_length, size_t *error_offset,
    const struct reloq_lexicon *lexicon,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    if (!input || !byte_length || (!output && capacity))
        return RELOQ_DE_TEXT_INVALID;

    size_t encoded_length = 0, bad = 0;
    int status = reloq_german_input_encode_checked(
        input, input_length, NULL, 0, &encoded_length, &bad,
        cancelled, cancel_context);
    if (status == 1) return RELOQ_DE_TEXT_CANCELLED;
    if (status != 0) {
        if (error_offset) *error_offset = bad;
        return RELOQ_DE_TEXT_INVALID;
    }
    if (encoded_length == SIZE_MAX) return RELOQ_DE_TEXT_NO_MEMORY;
    unsigned char *encoded = (unsigned char *)malloc(encoded_length + 1);
    if (!encoded) return RELOQ_DE_TEXT_NO_MEMORY;
    status = reloq_german_input_encode_checked(
        input, input_length, encoded, encoded_length + 1, &encoded_length,
        NULL, cancelled, cancel_context);
    if (status != 0) {
        free(encoded);
        if (status == 1) return RELOQ_DE_TEXT_CANCELLED;
        if (status == -3) return RELOQ_DE_TEXT_NO_MEMORY;
        return RELOQ_DE_TEXT_INVALID; /* input must remain immutable */
    }

    size_t needed = 0;
    status = convert(encoded, encoded_length, NULL, &needed, &bad,
                     lexicon, cancelled, cancel_context);
    if (status == RELOQ_DE_TEXT_INVALID ||
        status == RELOQ_DE_TEXT_UNCOVERED) {
        if (error_offset) {
            size_t offset = utf8_offset(input, input_length, bad,
                                        cancelled, cancel_context);
            if (offset == SIZE_MAX) status = RELOQ_DE_TEXT_CANCELLED;
            else *error_offset = offset;
        }
    }
    if (status != RELOQ_DE_TEXT_OK) {
        free(encoded);
        return status;
    }
    if (!output) {
        if (cancelled && cancelled(cancel_context)) {
            free(encoded);
            return RELOQ_DE_TEXT_CANCELLED;
        }
        *byte_length = needed;
        free(encoded);
        return RELOQ_DE_TEXT_OK;
    }
    if (capacity <= needed) {
        if (cancelled && cancelled(cancel_context)) {
            free(encoded);
            return RELOQ_DE_TEXT_CANCELLED;
        }
        *byte_length = needed;
        free(encoded);
        return RELOQ_DE_TEXT_CAPACITY;
    }

    unsigned char *annotation = (unsigned char *)malloc(needed + 1);
    if (!annotation) {
        free(encoded);
        return RELOQ_DE_TEXT_NO_MEMORY;
    }
    size_t written = 0;
    status = convert(encoded, encoded_length, annotation, &written, &bad,
                     lexicon, cancelled, cancel_context);
    free(encoded);
    if (status != RELOQ_DE_TEXT_OK || written != needed) {
        free(annotation);
        return status == RELOQ_DE_TEXT_OK ? RELOQ_DE_TEXT_INVALID : status;
    }
    annotation[written] = 0;

    struct reloq_german_annotation_cursor cursor = {0};
    struct reloq_german_annotation_event event;
    int parsed;
    while ((parsed = reloq_german_annotation_next(annotation, written,
                                                   &cursor, 0, &event)) == 1) {
        if (cancelled && cancelled(cancel_context)) {
            free(annotation);
            return RELOQ_DE_TEXT_CANCELLED;
        }
    }
    if (parsed != 0) {
        free(annotation);
        return RELOQ_DE_TEXT_INVALID;
    }
    if (cancelled && cancelled(cancel_context)) {
        free(annotation);
        return RELOQ_DE_TEXT_CANCELLED;
    }
    memmove(output, annotation, needed + 1);
    *byte_length = needed;
    free(annotation);
    return RELOQ_DE_TEXT_OK;
}

int reloq_german_text_annotate_with_lexicon(
    const unsigned char *input, size_t input_length,
    unsigned char *output, size_t capacity,
    size_t *byte_length, size_t *error_offset,
    const struct reloq_lexicon *lexicon)
{
    return reloq_german_text_annotate_checked(
        input, input_length, output, capacity, byte_length, error_offset,
        lexicon, NULL, NULL);
}

int reloq_german_text_annotate(const unsigned char *input,
                               size_t input_length,
                               unsigned char *output, size_t capacity,
                               size_t *byte_length, size_t *error_offset)
{
    return reloq_german_text_annotate_with_lexicon(
        input, input_length, output, capacity, byte_length, error_offset,
        NULL);
}
