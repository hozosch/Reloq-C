#ifndef RELOQ_PINYIN_H
#define RELOQ_PINYIN_H
#include <stddef.h>
/* Bytes consumed by the original pinyin-prefix grammar; zero for foreign
 * text. This is lexical classification, not a language-switch shim. */
size_t reloq_pinyin_prefix(const unsigned char *text,size_t length);
#endif
