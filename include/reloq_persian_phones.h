#ifndef RELOQ_PERSIAN_PHONES_H
#define RELOQ_PERSIAN_PHONES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Iranian Persian phoneme identities. These numbers are independent of any
 * donor language's phone slots. A phoneme identity specifies a linguistic
 * category; duration, stress and context are separate. The approved voice's
 * concrete acoustic realizations remain subject to listening tests. */
enum reloq_fa_phone_id {
    RELOQ_FA_PHONE_AE = 1, RELOQ_FA_PHONE_BACK_A, RELOQ_FA_PHONE_E,
    RELOQ_FA_PHONE_I, RELOQ_FA_PHONE_O, RELOQ_FA_PHONE_U,
    RELOQ_FA_PHONE_P, RELOQ_FA_PHONE_B, RELOQ_FA_PHONE_T, RELOQ_FA_PHONE_D,
    RELOQ_FA_PHONE_K, RELOQ_FA_PHONE_G, RELOQ_FA_PHONE_UVULAR,
    RELOQ_FA_PHONE_GLOTTAL_STOP,
    RELOQ_FA_PHONE_F, RELOQ_FA_PHONE_V, RELOQ_FA_PHONE_S, RELOQ_FA_PHONE_Z,
    RELOQ_FA_PHONE_SH, RELOQ_FA_PHONE_ZH, RELOQ_FA_PHONE_KH, RELOQ_FA_PHONE_H,
    RELOQ_FA_PHONE_CH, RELOQ_FA_PHONE_J, RELOQ_FA_PHONE_M, RELOQ_FA_PHONE_N,
    RELOQ_FA_PHONE_L, RELOQ_FA_PHONE_R, RELOQ_FA_PHONE_Y,
    RELOQ_FA_PHONE_COUNT = RELOQ_FA_PHONE_Y
};

enum reloq_fa_phone_category {
    RELOQ_FA_PHONE_VOWEL = 1, RELOQ_FA_PHONE_CONSONANT = 2
};

/* A long marker describes the existing pronunciation's relative duration,
 * not a promise that all vowels of the same category have the same length. */
#define RELOQ_FA_PHONE_LONG 1u

/* One phone in one word. Syllables are numbered consecutively from zero;
 * every phone in a syllable carries that syllable's stress (0, 1, or 2).
 * Repeated consonant events stay repeated: an affricate remains one phone,
 * and gemination is not confused with a two-character spelling. */
struct reloq_fa_phone {
    uint32_t id;
    uint32_t flags;
    uint32_t syllable;
    uint32_t stress;
};

/* Static inventory names are for inspection, not a finalized dictionary
 * input syntax. IPA lists linguistic approximations, not measured synthesis. */
struct reloq_fa_phone_info {
    uint32_t id;
    uint32_t category;
    const char *name;
    const char *ipa;
};

const struct reloq_fa_phone_info *reloq_fa_phone_inventory(size_t *count);

/* Transitional adapter for one existing Finnish-based Persian phone group.
 * A generated final H carrier is decoded as H, not a separate glottal stop.
 * The target array is unchanged on every failure; NULL/zero queries its size.
 * Returns 0 on success, 1 for an unrepresentable sequence, -1 for malformed
 * arguments or syntax, -2 for insufficient capacity. The error byte offset
 * is relative to the legacy group; this is not a source-text position. */
int reloq_fa_phones_from_legacy(const char *text,size_t bytes,
    struct reloq_fa_phone *phones,size_t capacity,size_t *count,size_t *error_offset);

/* Serialize one structured word for the temporary approved acoustic backend.
 * Output is unchanged on failure; NULL/zero queries bytes excluding NUL.
 * An actual output buffer must also have room for the trailing NUL. The
 * bridge accepts groups of at most 8192 bytes.
 * A final H receives its carrier internally. H immediately followed by an
 * explicit glottal stop cannot yet be expressed unambiguously and returns 1.
 * Success 0; unrepresentable 1; malformed -1; insufficient capacity -2. */
int reloq_fa_phones_to_legacy(const struct reloq_fa_phone *phones,size_t count,
    char *output,size_t capacity,size_t *bytes,size_t *error_index);

#ifdef __cplusplus
}
#endif
#endif
