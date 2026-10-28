#ifndef RELOQ_GREEK_H
#define RELOQ_GREEK_H
#include <stddef.h>
typedef int (*reloq_greek_english)(void *,const char *,char *,size_t);
int reloq_greek_text(const char *,int,reloq_greek_english,void *,char **,size_t **);
#endif
