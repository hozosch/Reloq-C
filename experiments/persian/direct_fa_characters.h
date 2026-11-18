#ifndef RELOQ_FA_DIRECT_CHARACTERS_H
#define RELOQ_FA_DIRECT_CHARACTERS_H

#include <stddef.h>
#include <stdint.h>

/* Write a Persian phoneme-tag spelling or a named English fallback for one
 * Unicode scalar, using internal ECI language tags where necessary. Return 1
 * on success, 0 for NUL or an invalid scalar, and -1 if the destination is
 * absent or too small. On failure, the destination is unchanged. */
int reloq_fa_direct_character(uint32_t codepoint, char *output, size_t capacity);

/* Resolve a character without embedding language-switch controls in text.
 * On success, kind is 0 for Persian pronunciation and 1 for an English
 * Unicode name. Output and kind are unchanged on failure. */
int reloq_fa_character_resolve(uint32_t codepoint, char *output, size_t capacity, int *kind);

#endif
