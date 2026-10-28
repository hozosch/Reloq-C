/* A lexical acoustic bank owns only the current word's temporary targets. */
#ifndef RELOQ61_ENGLISH_BANK_H
#define RELOQ61_ENGLISH_BANK_H
#include "reloq61.h"
#include <stddef.h>
#include <stdint.h>
struct reloq61_bank_phone {
 char name[4];int16_t stress,syllable_stress;uint16_t word,syllable;
 uint8_t traits[6],transition,ghost,after;int16_t duration;
};
struct reloq61_bank_word {unsigned char fields[16];};
struct reloq61_bank_span {struct reloq61_curve curve[62];int32_t from,to,volume,step;};
struct reloq61_bank_atom {
 struct reloq61_bank_phone phone;int32_t from,to,group;
};
struct reloq61_bank_syllable {
 int32_t from,to;size_t begin,end;int16_t stress;uint16_t word;
};
struct reloq61_bank_pitch {int32_t at,value;};
struct reloq61_english_bank {
 struct reloq61_bank_phone phones[255];struct reloq61_bank_word words[255],early[255];
 int32_t last_word,last_syllable;unsigned word_cursor,syllable_cursor;
 size_t phone_count,word_count;uint8_t early_present[255];
 int16_t pitch_word_stress[255];
 struct reloq61_bank_span *spans;size_t span_count,span_capacity;int failed,emitted;
 struct reloq61_bank_atom *atoms;size_t atom_count,atom_capacity;
 struct reloq61_bank_syllable *syllables;size_t syllable_count,syllable_capacity;
 struct reloq61_bank_pitch *pitch;size_t pitch_count;
 int32_t word_points[255];int32_t host_left,host_right;int64_t pause_from,pause_to;int trim_pause;
 char text[256];int rendering,nonfinal,neutral_inflection;
};
void *reloq61_english_bank_scope(struct reloq61_english_bank *);
int reloq61_english_bank_active(void);
void reloq61_english_bank_words(void *,int32_t,int32_t);
void reloq61_english_bank_phones(void *,int32_t,int32_t);
void reloq61_english_bank_timing(void *);
int reloq61_english_bank_prepare(void *,const struct reloq61_english_bank *,int32_t);
int reloq61_english_bank_pitch(void *,struct reloq61_english_bank *,int32_t,int32_t);
struct reloq61_english_bank *reloq61_english_bank_build(void *,const char *);
int reloq61_english_bank_emit(void *,const struct reloq61_english_bank *,int32_t,int32_t);
int reloq61_english_bank_render(void *,struct reloq61_english_bank *,int);
void reloq61_english_bank_duration_context(void *);
unsigned reloq61_english_bank_attach(void *,struct reloq61_english_bank *);
struct reloq61_english_bank *reloq61_english_bank_at(void *,unsigned);
struct reloq61_english_bank *reloq61_english_bank_for_word(void *,int32_t);
void reloq61_english_bank_free(struct reloq61_english_bank *);
void reloq61_english_banks_reset(void *);
#endif
