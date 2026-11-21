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
int reloq_fa_pinglish_word_complete(const char *word,size_t bytes,char *output,size_t capacity);
int reloq_fa_pinglish_project(const char *text,size_t bytes,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
int reloq_fa_pinglish_project_complete(const char *text,size_t bytes,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
int reloq_fa_project_runs(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
int reloq_fa_project_runs_complete(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
int reloq_fa_spell_for_plan(const char *text,size_t bytes,char **output,
    size_t **mapping,size_t *error);
#endif
