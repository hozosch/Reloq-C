#include "reloq_english_intonation.h"
#include <limits.h>
#include <math.h>

/* Authored from the connected eng_nucl_high_tone / eng_nucl_low_tone,
 * eng_phrase_tone / insert_f0 / set_f0_wind_and_base control flow.
 * The other nuclear tone categories and continuation accent overrides
 * remain outside this scalar contract. */
int reloq_english_nuclear_tone(const struct reloq_english_nuclear_context *c,
                             struct reloq_english_tone_point out[2])
{
    if(!c || !out || (unsigned)c->tone>RELOQ_EN_TONE_LOW || c->first_word>1 ||
       c->last_word>1 || c->last_syllable>1 || c->first_accent>1)return -1;
    struct reloq_english_tone_point p[2]={{10,50},{40,100}};
    if(c->tone==RELOQ_EN_TONE_LOW) {
        p[1]=(struct reloq_english_tone_point){20,25};
        if(c->first_word)p[1].percent=c->last_word?60:15;
    } else {
        if(c->first_word)p[1].percent=c->last_word?35:60;
        else if(c->last_word)p[1].percent=c->last_syllable?15:50;
        if(c->first_accent)p[1].level=p[1].level*120/100;
    }
    out[0]=p[0];out[1]=p[1];return 0;
}

int reloq_english_final_tone(int rising,int continuing,int final_word,int weak,
                           struct reloq_english_tone_point out[2])
{
    if(!out || (unsigned)rising>1 || (unsigned)continuing>1 ||
       (unsigned)final_word>1 || (unsigned)weak>1)return -1;
    const int percent=final_word?70:weak?55:85;
    out[0]=(struct reloq_english_tone_point){percent,rising?100:continuing?20:-20};
    out[1]=(struct reloq_english_tone_point){99,rising?150:continuing?0:-35};
    return 0;
}

int reloq_english_tone_tick(int64_t begin,int64_t end,int percent,int64_t *tick)
{
    if(!tick || begin<0 || end<begin || percent<0 || percent>100)return -1;
    if(percent==0)percent=3;
    else if(percent==100)percent=97;
    const int64_t d=end-begin;
    /* Quotient/remainder form keeps the original integer truncation
     * without overflowing on a caller's long time axis. */
    *tick=begin+(d/100)*percent+(d%100)*percent/100;return 0;
}

int reloq_english_tone_value(double pitch,int inflection,size_t count,
                            double position,int level,int *value)
{
    if(!value || !isfinite(pitch) || pitch<=0 || pitch>INT_MAX/4 ||
       inflection<0 || inflection>100 || !count || !isfinite(position) ||
       position< -1 || position>(double)count+1 || level< -100 || level>300)return -1;
    double f0=pitch;
    if(inflection) {
        const double window=pitch*inflection/100.0,base=pitch-window*.9;
        const double declination=log(trunc(base)/fmax(10,trunc(base)-3.0*count))/count;
        f0=base*exp(-declination*position)+window*level/100.0;
    }
    f0=fmax(trunc(pitch*.5),trunc(f0));
    if(!isfinite(f0) || f0>INT_MAX)return -1;
    *value=(int)f0;return 0;
}
