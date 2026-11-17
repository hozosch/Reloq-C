/* Shared ECI host adapter for the existing direct Persian implementation. */
#ifndef RELOQ_PERSIAN_DIRECT_H
#define RELOQ_PERSIAN_DIRECT_H
#include "../experiments/persian/direct_fa_text.h"
#include "../experiments/persian/direct_fa_lexicon.h"
#include "../experiments/persian/direct_fa_normalize.h"
#include "../experiments/persian/direct_fa_numbers.h"
#include "../experiments/persian/direct_fa_letters.h"
#include "../experiments/persian/direct_fa_characters.h"
int reloq_fa_pinglish_word(const char *word,size_t bytes,char *output,size_t capacity);
#endif
