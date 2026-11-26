#ifndef RELOQ_GREEK_H
#define RELOQ_GREEK_H
#include <stddef.h>
#include <stdint.h>
typedef int (*reloq_greek_english)(void *,const char *,char *,size_t);
/* Return 1 with a projected text and map, 0 when unchanged, -1 on error. */
int reloq_greek_symbol_text(const char *,int,int (*)(void *,uint32_t),void *,
                            int (*)(void *,const char *,size_t),void *,char **,size_t **);
int reloq_greek_text(const char *,int,reloq_greek_english,void *,char **,size_t **);
#endif
