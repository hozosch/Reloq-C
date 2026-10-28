#ifndef RELOQ_ENGLISH_VOICE_H
#define RELOQ_ENGLISH_VOICE_H
#include "reloq_english.h"
#include "reloq_german_generic_voice.h"

/* First connected English preview. Uses the observed English dictionary,
 * weak forms, stem-based inflection and English pair gestures. Remaining
 * unknown words are spelled with English letter names. No German text or
 * phoneme planner is called.
 * Return codes and host control settings are shared with the ECI adapter. */
int reloq_english_render_text(const unsigned char *text,size_t length,
    int process_tags,int native_volume,const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink sink,
    reloq_german_generic_index_sink index_sink,void *context);
/* Diagnostic text analysis. Allocated SPR belongs to the caller. */
int reloq_english_text_annotation(const unsigned char *text,size_t length,
    unsigned char **annotation,size_t *written,size_t *spelled_words);
#endif
