#ifndef RELOQ_GERMAN_SPR_H
#define RELOQ_GERMAN_SPR_H

#include <stddef.h>
#include <stdint.h>

/* IBM's published German SPR alphabet. Identifiers retain the exact case of
 * one-byte symbols; Q_ names denote one quoted, multi-byte symbol. These are
 * phonetic symbols, not acoustic targets or durations. */
enum reloq_german_phone {
    RELOQ_DE_PHONE_NONE = 0,
    RELOQ_DE_PHONE_a, RELOQ_DE_PHONE_A,
    RELOQ_DE_PHONE_e, RELOQ_DE_PHONE_E,
    RELOQ_DE_PHONE_i, RELOQ_DE_PHONE_I,
    RELOQ_DE_PHONE_o, RELOQ_DE_PHONE_O,
    RELOQ_DE_PHONE_u, RELOQ_DE_PHONE_U,
    RELOQ_DE_PHONE_y, RELOQ_DE_PHONE_Y,
    RELOQ_DE_PHONE_AT,
    RELOQ_DE_PHONE_b, RELOQ_DE_PHONE_C, RELOQ_DE_PHONE_d,
    RELOQ_DE_PHONE_f, RELOQ_DE_PHONE_g, RELOQ_DE_PHONE_G,
    RELOQ_DE_PHONE_h, RELOQ_DE_PHONE_j, RELOQ_DE_PHONE_J,
    RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_l, RELOQ_DE_PHONE_m,
    RELOQ_DE_PHONE_n, RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_P,
    RELOQ_DE_PHONE_r, RELOQ_DE_PHONE_R, RELOQ_DE_PHONE_s,
    RELOQ_DE_PHONE_S, RELOQ_DE_PHONE_t, RELOQ_DE_PHONE_T,
    RELOQ_DE_PHONE_v, RELOQ_DE_PHONE_w, RELOQ_DE_PHONE_x,
    RELOQ_DE_PHONE_X, RELOQ_DE_PHONE_z, RELOQ_DE_PHONE_Z,
    RELOQ_DE_PHONE_Q_E_COLON, RELOQ_DE_PHONE_Q_oe,
    RELOQ_DE_PHONE_Q_OE, RELOQ_DE_PHONE_Q_aj,
    RELOQ_DE_PHONE_Q_aw, RELOQ_DE_PHONE_Q_oj,
    RELOQ_DE_PHONE_Q_a_TILDE, RELOQ_DE_PHONE_Q_E_TILDE,
    RELOQ_DE_PHONE_Q_o_TILDE, RELOQ_DE_PHONE_Q_oe_TILDE
};

enum reloq_german_spr_kind {
    RELOQ_DE_SPR_WORD_OPEN = 1,
    RELOQ_DE_SPR_SYLLABLE,
    RELOQ_DE_SPR_STRESS,
    RELOQ_DE_SPR_PHONE,
    RELOQ_DE_SPR_WORD_CLOSE,
    RELOQ_DE_SPR_FEATURES
};

struct reloq_german_spr_event {
    enum reloq_german_spr_kind kind;
    size_t offset; /* exact byte span in input */
    size_t length; /* 0 only for an implicit initial syllable boundary */
    size_t syllable; /* zero-based; meaningful inside a word */
    enum reloq_german_phone phone; /* PHONE only */
    uint8_t stress; /* STRESS only: 0, 1, or 2 */
    /* Optional independent-engine lossless header, before the first
     * syllable: !count,glottal,full-quantity,final-a; (hexadecimal masks).
     * This is an extension, not a claim about the original SPR alphabet. */
    unsigned feature_syllables;
    uint64_t glottal_syllables, full_quantity_syllables, full_a_syllables;
};

struct reloq_german_spr_cursor {
    size_t offset;
    size_t syllable_count;
    unsigned char phase; /* zero-initialized; do not modify during iteration */
    unsigned char has_phone;
    unsigned char has_stress;
    unsigned char primary_count;
    unsigned char has_features;
    unsigned char feature_syllables;
};

/* Parse exactly one German SPR group: `[.1hA.0lo] (or [.1hA.0lo]).
 * Use the byte scanner to locate groups in a complete ECI annotation before
 * passing each group separately. An all-zero cursor begins iteration.
 * Returns 1/event, 0 after the closing bracket, or -1 for malformed input;
 * on -1 neither cursor nor event is changed, and cursor->offset identifies
 * the failing byte (or the input length for an incomplete group).
 *
 * Quoted multi-byte symbols are indivisible; unquoted symbols are exactly
 * one byte. A stress event preserves the marker's position within a syllable
 * rather than assigning it to a phone or guessing its timing. Explicit dots
 * have a one-byte span. When no first dot is present, an implicit SYLLABLE
 * event has a zero-byte span. The parser checks symbol spelling and requires
 * at most one primary stress. Original German oi forms can contain only
 * unaccented syllables, so a multi-syllable group need not contain .1.
 * No phone duration, syllable timing, prosody or synthesis target is implied.
 */
int reloq_german_spr_next(const unsigned char *input, size_t length,
                          struct reloq_german_spr_cursor *cursor,
                          struct reloq_german_spr_event *event);

#endif
