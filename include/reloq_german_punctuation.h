#ifndef RELOQ_GERMAN_PUNCTUATION_H
#define RELOQ_GERMAN_PUNCTUATION_H
#include <stddef.h>
int reloq_german_silent_separator(unsigned char c);
int reloq_german_apostrophe(unsigned char c);
/* Private annotation text markers. Input controls are never copied into this
 * representation. They retain contour type and a continuing phrase boundary. */
#define RELOQ_DE_TEXT_QUESTION_CONTINUE 0x1e
#define RELOQ_DE_TEXT_EXCLAMATION_CONTINUE 0x1f
enum reloq_german_punctuation_action { RELOQ_DE_PUNCT_UNKNOWN,
 RELOQ_DE_PUNCT_SILENT,RELOQ_DE_PUNCT_BOUNDARY,RELOQ_DE_PUNCT_SPOKEN };
struct reloq_german_punctuation {enum reloq_german_punctuation_action action;
 size_t consumed; unsigned char mark; const char *spoken; int weak;};
int reloq_german_punctuation_read(const unsigned char *,size_t,size_t,int,
 struct reloq_german_punctuation *);
#endif
