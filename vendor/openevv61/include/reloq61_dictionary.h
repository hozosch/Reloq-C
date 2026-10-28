#ifndef RELOQ61_DICTIONARY_H
#define RELOQ61_DICTIONARY_H
#include <stdint.h>
/* Data participates at the same complete-word stage as built-in entries. */
int32_t reloq61_dictionary_word_lookup(void *,int32_t,int32_t,int32_t,
                                      int32_t (*)(void *,int32_t,int32_t,int32_t));
#endif
