/* Direct Persian cardinal/decimal experiment, GPL-3.0-or-later. */
#ifndef RELOQ_DIRECT_FA_NUMBERS_H
#define RELOQ_DIRECT_FA_NUMBERS_H
#include <stddef.h>

/* Read a number at the beginning of already-normalized UTF-8 text. Persian
 * and Arabic-Indic digits have become ASCII; U+066B/U+066C stay distinct.
 * Returns 0 on success, 2 for malformed/unsupported numeric syntax,
 * -1 for invalid arguments, -2 for an insufficient output buffer. */
int reloq_fa_direct_number(const char *text,size_t length,size_t *consumed,
                           char *output,size_t capacity);
#endif
