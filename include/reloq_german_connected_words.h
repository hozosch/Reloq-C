#ifndef RELOQ_GERMAN_CONNECTED_WORDS_H
#define RELOQ_GERMAN_CONNECTED_WORDS_H

#include "reloq_german_phone_sequence.h"

/* A deliberately narrow, structural candidate for one German sentence of
 * exactly two independently covered, primary-stressed monosyllables. This
 * identifies the phrase syntax, not its acoustic target curves. In natural
 * ECI output the words are separated by the numeric `2 control. Some words
 * change stress in a phrase (e.g. auxiliary "hat"), and those contexts must
 * be verified separately before a speech renderer claims parity. */
struct reloq_german_connected_plan {
    struct reloq_german_phone_node phones[6];
    size_t phone_count;
    size_t word_phone_count[2];
    size_t word_input_offset[2];
    size_t word_input_length[2];
    size_t group_offset[2]; /* byte offset into output annotation */
    size_t group_length[2];
    size_t boundary_offset; /* the backtick in the literal `2 control */
    /* 1 only when the caller explicitly supplied both phonetic groups and
     * the intervening control; natural spelling leaves this zero because
     * phrase context may reduce stress. No value asserts acoustic parity. */
    int stress_explicitly_specified;
    /* 1 only for a naturally spelled pair with a positive whole-callback
     * comparison in the finite, public ECI CVC carrier matrix. This certifies
     * the SPR and control spelling, never the acoustic target curve. */
    int reference_spr_equivalent;
};

/* 1 = syntactically modeled candidate, 0 = unsupported text/phones,
 * -1 = invalid arguments or malformed UTF-8, -2 = output too small,
 * -3 = out of memory. Accepts precisely <word> SP <word> '.' in UTF-8;
 * words may use German umlauts and sharp s. Successful annotation includes
 * the Apple callback's leading ASCII space and is NUL-terminated:
 * " `<group1> `2 `<group2>." where each group already starts with `[.
 * For example, "Pack Tipp." gives " `[.1pAk] `2 `[.1tIp].".
 * byte_length excludes the final NUL. The caller may query the required
 * length with annotation=NULL/capacity=0; plan is published only on fully
 * successful output. On -2 byte_length is the required length, and output
 * and plan are unchanged. On 0/-1/-3 all outputs except a supplied
 * error_offset remain unchanged. The error offset refers to UTF-8 input.
 * Caller must keep input immutable throughout the call. */
int reloq_german_connected_words_prepare(
    const unsigned char *utf8, size_t length,
    unsigned char *annotation, size_t capacity,
    size_t *byte_length, size_t *error_offset,
    struct reloq_german_connected_plan *plan);

/* Validate an explicit two-word annotation of the same narrow phone class:
 * optional leading ASCII space, <SPR group> " `2 " <SPR group> period.
 * It may be used with ECI phonetic InputType=1 to test connected acoustics
 * without guessing lexical stress from spelling. 1 = supported shape, 0 =
 * unsupported shape/phones, -1 = invalid argument. On success plan nodes
 * refer to input byte offsets; word_input_* are the SPR group spans, not
 * orthographic word positions. On failure plan is unchanged. The supplied
 * annotation must stay immutable while these offsets are used. */
int reloq_german_connected_annotation_prepare(
    const unsigned char *annotation, size_t length,
    struct reloq_german_connected_plan *plan);

#endif
