#ifndef RELOQ61_ENGLISH_CLOCK_H
#define RELOQ61_ENGLISH_CLOCK_H
#include <stddef.h>
/* Native lexical projection, with an original-byte-boundary map. Ordinary
 * HH:MM remains in the existing clock converter. No allocation without a
 * valid seconds field; the caller owns returned text and map. */
int reloq61_english_clock_text(const char *,int,int,char **,size_t **);
#endif
