#ifndef RELOQ61_LATIN_H
#define RELOQ61_LATIN_H
#include <stdint.h>
/* Latin input for the existing US character alphabet. CP1252 accents retain
 * their native identity; additional Latin letters use the shared base entry. */
int reloq61_latin_byte(uint32_t scalar);
int reloq61_latin_letter_byte(unsigned byte);
#endif
