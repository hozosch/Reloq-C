#ifndef RELOQ_CHARACTER_H
#define RELOQ_CHARACTER_H
#include <stddef.h>
#include <stdint.h>
#include "reloq_german_input.h"
/* A terminal character reading uses phonetic groups, never ordinary-word
 * spellings. Original Latin readings and extensions share this interface. */
struct reloq_character_reading {
    /* Immutable group bodies; each ends at ']', not at a NUL byte. */
    const char *groups[8];
    size_t count;
    const char *original_spr;
};
int reloq_character_read(uint32_t scalar,int english,struct reloq_character_reading *);
/* Explicit character navigation: native punctuation/digits as well as letters. */
int reloq_character_read_native(uint32_t scalar,int english,struct reloq_character_reading *);
/* Extend a missing native character with its base and local accent names. */
struct reloq_character_extension {
    uint32_t base;
    const char *accents[3];
    size_t count;
};
int reloq_character_extension_read(uint32_t scalar,int language,struct reloq_character_extension *);
int reloq_character_native_available_language(uint32_t scalar,int language);
int reloq_character_locale(int language);
/* Strict original alphabet coverage, before Unicode extension fallback. */
int reloq_character_native_available(uint32_t scalar,int english);
int reloq_character_spelling1(uint32_t scalar);
int reloq_character_latin_base(uint32_t scalar,unsigned char *base);
/* One text-scanner byte per Latin scalar. Optional scalars retain the
 * original identity for single_chars -> char_name; words use the base in
 * the existing loan-letter path. The strict CP1252 codec stays separate. */
int reloq_character_encode_text(const unsigned char *utf8,size_t length,
    unsigned char *output,uint32_t *scalars,size_t capacity,size_t *written,
    reloq_german_cancel_check cancelled,void *context);
#endif
