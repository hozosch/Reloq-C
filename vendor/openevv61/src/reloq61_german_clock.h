#ifndef RELOQ61_GERMAN_CLOCK_H
#define RELOQ61_GERMAN_CLOCK_H
#include <stddef.h>
/* Normalize clock fields and grouped numbers while preserving source positions. */
int reloq61_german_clock_text(const char *,int,int,char **,size_t **);
#endif
