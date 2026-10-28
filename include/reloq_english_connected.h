#ifndef RELOQ_ENGLISH_CONNECTED_H
#define RELOQ_ENGLISH_CONNECTED_H
#include <stddef.h>

/* Apply connected-speech allophones to a complete native SPR string.
 * The writable buffer includes its terminating NUL. Optional source
 * positions are compacted with deleted phones, including the EOF entry.
 * No allocation, word dictionary, reference runtime or spelling is used. */
void reloq_english_connected_rules(unsigned char *text,size_t *length,
                                  size_t *source_positions);
#endif
