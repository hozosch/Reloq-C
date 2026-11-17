#ifndef RELOQ_FA_DIRECT_CHARACTERS_H
#define RELOQ_FA_DIRECT_CHARACTERS_H

#include <stddef.h>
#include <stdint.h>

/* Write the Persian phoneme-tag spelling of one Unicode scalar. Return 1
 * on success, 0 for NUL or an invalid scalar, and -1 if the destination is
 * absent or too small. On failure, the destination is unchanged. */
int reloq_fa_direct_character(uint32_t codepoint, char *output, size_t capacity);

#endif
