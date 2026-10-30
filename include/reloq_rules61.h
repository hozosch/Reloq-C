#ifndef RELOQ_RULES61_H
#define RELOQ_RULES61_H
#include "reloq_english_voice.h"
/* Complete native 6.1 acoustic rules, with the ReLoq PCM renderer. */
int reloq_language_rules_render(int language,const unsigned char *,size_t,int,int,
    const struct reloq_german_generic_settings *,
    const struct reloq_german_generic_index *,size_t,
    const struct reloq_german_generic_pitch_mark *,size_t,
    struct reloq_session *,reloq_session_pcm_sink,
    reloq_german_generic_index_sink,void *);
int reloq_english_rules_render(const unsigned char *,size_t,int,int,
    const struct reloq_german_generic_settings *,
    const struct reloq_german_generic_index *,size_t,
    const struct reloq_german_generic_pitch_mark *,size_t,
    struct reloq_session *,reloq_session_pcm_sink,
    reloq_german_generic_index_sink,void *);
/* Transactional word-to-SPR analysis; every call analyzes anew. 1 success, 0 uncovered,
 * -2 insufficient output capacity, -1 invalid input. */
int reloq_rules61_word(int,const unsigned char *,size_t,
    unsigned char *,size_t,size_t *);
struct reloq_german_prosody_unit;
/* Contextual sentence analysis, aligned to the existing spoken-word plan.
 * 1 accepted, 0 incompatible (keep the established frontend), -1 allocation
 * failure, 2 cancelled. Explicit pronunciations remain caller-owned. */
int reloq_rules61_german_context(const struct reloq_german_prosody_unit *,size_t,
    const struct reloq_lexicon *,
    const unsigned char *,size_t,unsigned char **,size_t *,
    int (*cancelled)(void *),void *);
struct reloq61_german_word_facts;
/* Immutable admitted source buffers share offsets. Retaining actual source
 * whitespace avoids feeding annotation-formatting blanks into the grammar. */
struct reloq_german_context_source {
    const unsigned char *encoded,*original,*words;
    size_t length;
};
int reloq_rules61_german_context_features(const struct reloq_german_prosody_unit *,size_t,
    const struct reloq_lexicon *,const unsigned char *,size_t,unsigned char **,size_t *,
    struct reloq61_german_word_facts *,int (*cancelled)(void *),void *,
    const struct reloq_german_context_source *);
#endif
