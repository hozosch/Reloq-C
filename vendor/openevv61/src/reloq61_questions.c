/* Product intonation policy. Read the completed grammatical phrase class;
 * leave its words, focus, phonemes and duration analysis intact. This runs
 * once at the beginning of the F0 realization stage. */
#include "delta.h"

static int is_question(void *state,int32_t phrase)
{
    if(!phrase)return 0;
    delta_state *d=state;
    int32_t token=vgetsc(d,0,0,phrase,5);
    if(!token || (*EVV_AT(unsigned char *,token)&2))return 0;
    void *fields=TFLDS(EVV_AT(void *,token));
    unsigned kind=*(unsigned char *)delta_record_field(&vstmtbl[5], 1, fields);
    /* Both yes/no classes, alternative questions and WH questions. These
     * are grammatical labels, independent of the spelling of a question. */
    return kind==3 || kind==4 || kind==7 || kind==8;
}

void reloq61_english_question_tones(void *state,int32_t phrase,int16_t high,
    int16_t *phrase_tone,int16_t *boundary_tone)
{
    if(is_question(state,phrase)) {
        *phrase_tone=high;
        *boundary_tone=high;
    }
}

int reloq61_english_question_time(void *state,int32_t phrase,
    int16_t position,int16_t tone,int32_t previous,int32_t *when)
{
    /* At compressed rates the terminal target can share the preceding
     * target's millisecond. Retain a one-millisecond rising segment on
     * the F0 clock; no phone or pause is lengthened. Ordinary accepted
     * targets retain their native positions. */
    if(position!=99 || tone<100 || previous<*when ||
        previous>INT32_MAX-10000 || !is_question(state,phrase))return 0;
    *when=previous+10000;
    return 1;
}
