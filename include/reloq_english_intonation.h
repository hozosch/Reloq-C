#ifndef RELOQ_ENGLISH_INTONATION_H
#define RELOQ_ENGLISH_INTONATION_H
#include <stddef.h>
#include <stdint.h>

enum reloq_english_nuclear_tone { RELOQ_EN_TONE_HIGH, RELOQ_EN_TONE_LOW };
struct reloq_english_tone_point { int percent,level; };
struct reloq_english_nuclear_context {
    enum reloq_english_nuclear_tone tone;
    unsigned first_word,last_word,last_syllable,first_accent;
};
/* Native nuclear high/low tone scalar decisions. Incoming and nuclear
 * ranges are independent; percentages do not refer to whole sentences. */
int reloq_english_nuclear_tone(const struct reloq_english_nuclear_context *,
                             struct reloq_english_tone_point out[2]);

/* The final phrase tone uses the last voiced nucleus, including its
 * closing sonorants. A weak final word is a postaccentual tail. */
int reloq_english_final_tone(int rising,int continuing,int final_nuclear_word,
                           int weak_tail,struct reloq_english_tone_point out[2]);
int reloq_english_tone_tick(int64_t begin,int64_t end,int percent,int64_t *tick);
int reloq_english_tone_value(double selected_pitch,int inflection,
                            size_t syllables,double position,int level,int *value);
#endif
