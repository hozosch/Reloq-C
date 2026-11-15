#ifndef RELOQ_FA_DIRECT_LETTERS_H
#define RELOQ_FA_DIRECT_LETTERS_H
#include <stdint.h>

/* A supported standalone letter name, or NULL. The pointer is static. */
const char *reloq_fa_direct_letter(uint32_t codepoint);

#endif
