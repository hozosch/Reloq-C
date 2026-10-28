#ifndef RELOQ_ENGLISH_H
#define RELOQ_ENGLISH_H
#include "reloq_session.h"
#include <stddef.h>

/* Independent development front end. The finite lexicon contains public
 * English readings; unobserved spellings return UNCOVERED rather than
 * falling through German rules. Sentence syntax is not complete. */
enum reloq_english_result {
    RELOQ_ENGLISH_OK=0, RELOQ_ENGLISH_UNCOVERED=1,
    RELOQ_ENGLISH_INVALID=-1, RELOQ_ENGLISH_MEMORY=-2
};
int reloq_english_word(const unsigned char *word,size_t length,
    const char **isolated,const char **before_vowel,const char **before_consonant);
size_t reloq_english_lexicon_size(void);
/* Principal lexical inventory, excluding exploratory supplemental readings.
 * Used to license a morphological stem rather than treating every observed
 * letter run as a verb. Same result and ownership contract as word(). */
int reloq_english_primary_word(const unsigned char *,size_t,
    const char **,const char **,const char **);

struct reloq_english_phone {
    unsigned char phone,stress,syllabic,weak;
    signed char accent; /* -1: weak function word; 0/1/2: word prominence. */
    size_t word,syllable,source_offset;
};
/* Parse the English public SPR inventory, including syllabic N/M. The
 * returned vector is caller-owned. Unknown tokens fail transactionally. */
int reloq_english_parse(const unsigned char *annotation,size_t length,
    struct reloq_english_phone **phones,size_t *count,size_t *error_offset);

/* Development acoustic implementation: isolated, stressed English vowel nuclei
 * and Y/W/O diphthongs with continuous tract/source gestures,
 * at the observed male/default 11.025-kHz profile. Returns UNCOVERED for
 * other phones. This is not yet a connected-text voice or NVDA language. */
int reloq_english_render_nucleus(unsigned char phone,
    struct reloq_session *session,reloq_session_pcm_sink sink,void *context,
    size_t *frames_completed);
/* Development CV composer: m/n/p/t/k/b/d/g/f/v/s/z/S/Z plus one of 14 simple
 * stressed vowels. Contact, releases, antiresonance and source/F0 are English data;
 * unsupported combinations are reported explicitly. Same profile as above. */
int reloq_english_render_cv(unsigned char onset,unsigned char vowel,
    struct reloq_session *session,reloq_session_pcm_sink sink,void *context,
    size_t *frames_completed);
/* Development closed-syllable composer, using English vowel-to-stop
 * gestures and one shared nucleus budget. onset may be zero (VC).
 * The coda inventory is p/t/k/b/d/g; the CV restrictions above still apply. */
int reloq_english_render_cvc(unsigned char onset,unsigned char vowel,unsigned char coda,
    struct reloq_session *session,reloq_session_pcm_sink sink,void *context,
    size_t *frames_completed);
#endif
