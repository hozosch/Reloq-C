#ifndef RELOQ_GERMAN_GENERIC_VOICE_H
#define RELOQ_GERMAN_GENERIC_VOICE_H

#include "reloq_german_word_features.h"

#include "reloq_german_utterance.h"
#include "reloq_german_syllable_plan.h"

/* Native controls for the explicitly enabled experimental voice. All values
 * are integers in 0..100. The reference-compatible existing entry points
 * use rate=50, pitch=50, volume=100. Rate changes the target time axis,
 * pitch changes F0, and volume changes all excitation sources together.
 * They never modify the separately matched exact German voice. Optional
 * native_speed (40..250) and native_pitch_plus_one (1..101, for pitch 0..100)
 * select the separately measured Apple ECI curve. Zero leaves the legacy
 * developmental control unchanged. All existing callers initialize the
 * optional fields to zero. native_inflection_plus_one (1..101) similarly
 * selects inflection 0..100; the German melody planner owns that contour. */
struct reloq_german_generic_settings {
    int rate;
    int pitch;
    int volume;
    int native_speed;
    int native_pitch_plus_one;
    int native_inflection_plus_one;
    /* 0: original pauses, 1: shorten only the final automatic pause,
     * 2: shorten every automatic pause. Explicit timed pauses are retained. */
    int pause_mode;
    /* Zero head_size_plus_one keeps the historical 50. Other values 1..101
     * select native head size 0..100. Source controls use native 0..100. */
    int head_size_plus_one;
    int roughness;
    int breathiness;
    int gender; /* 0 male, 1 female acoustic model */
};

/* Experimental, independently authored 11,025-Hz fallback. It maps the
 * complete German SPR alphabet to broad vowel, sonorant, fricative and stop
 * gestures. It is deliberately separate from the Apple-matched narrow
 * voice. The output is intelligibility research, NOT reference-exact speech.
 * The host must explicitly select approximate speech; the opt-in ECI route
 * may accept a complete SPR annotation directly. No proprietary runtime or
 * captured word contours are used. */

/* A caller-owned builder must have at least (6*phone_count+8) points per
 * stream. Return 0 on success, 1 if cancelled, -1 for invalid phone sequence
 * or exhausted builder storage. A complete, syllabified SPR sequence is
 * required; punctuation is not included in this input. On failure, the
 * builder may contain a partial plan, and the caller must discard it. */
int reloq_german_generic_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_target_builder *builder, int64_t *end_tick,
    reloq_german_cancel_check cancelled, void *cancel_context);

int reloq_german_generic_plan_with_settings(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_generic_settings *settings,
    struct reloq_target_builder *builder, int64_t *end_tick,
    reloq_german_cancel_check cancelled, void *cancel_context);

/* UTF-8 natural text uses the conservative German frontend and an optional
 * validated pronunciation lexicon; input beginning with ` is interpreted
 * as ECI SPR annotation. Output uses the existing C Klatt session and the
 * same result convention as reloq_german_utterance_render. The caller resets
 * the session before a new utterance. Cancellation may leave accepted PCM,
 * like the exact voice. No hard-coded maximum length or word list exists. */
int reloq_german_generic_render(
    const unsigned char *utf8, size_t length,
    const struct reloq_lexicon *lexicon,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info);

int reloq_german_generic_render_with_settings(
    const unsigned char *utf8, size_t length,
    const struct reloq_lexicon *lexicon,
    const struct reloq_german_generic_settings *settings,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info);

/* Direct annotation entry point for the optional CLI or native ECI adapter.
 * This bypasses conservative text-to-SPR after a caller has prepared and
 * validated its own complete annotation. */
int reloq_german_generic_render_annotation(
    const unsigned char *annotation, size_t length,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info);

int reloq_german_generic_render_annotation_with_settings(
    const unsigned char *annotation, size_t length,
    const struct reloq_german_generic_settings *settings,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info);

/* Native ECI indices refer to byte positions in the already converted SPR
 * annotation. They must be supplied in nondecreasing order. The renderer
 * resolves them to the next acoustic word boundary, reports an index only
 * after every preceding PCM sample has been accepted, and preserves exactly
 * the same PCM as a render without indices. An index within an SPR word is
 * delivered when that word ends; an index at the annotation EOF follows the
 * final PCM callback. The index sink returns zero to accept or nonzero to
 * stop rendering, like the PCM sink. */
struct reloq_german_generic_index {
    size_t annotation_offset;
};
/* Ordered absolute native-pitch marks at annotation offsets. The current
 * generic voice applies each mark beginning with the next acoustic word.
 * Marks inside a word follow that word, matching native ECI index mapping. */
struct reloq_german_generic_pitch_mark {
    size_t annotation_offset;
    int native_pitch;
};
typedef int (*reloq_german_generic_index_sink)(void *context,
                                                 size_t ordinal);

int reloq_german_generic_render_annotation_indexed_with_settings(
    const unsigned char *annotation, size_t length,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices, size_t index_count,
    struct reloq_session *session, reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink, void *context,
    struct reloq_german_utterance_info *info);

int reloq_german_generic_render_annotation_indexed_with_settings_and_pitch(
    const unsigned char *annotation, size_t length,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices, size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches, size_t pitch_count,
    struct reloq_session *session, reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink, void *context,
    struct reloq_german_utterance_info *info);

/* Text frontends can preserve onset distinctions absent from public SPR. */
int reloq_german_generic_render_text_plan(
    const unsigned char *annotation,size_t length,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_word_features *features,size_t feature_count,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink,void *context,
    struct reloq_german_utterance_info *info);

int reloq_german_generic_render_tagged_text_plan(
    const unsigned char *annotation,size_t length,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_word_features *features,size_t feature_count,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink,void *context,
    struct reloq_german_utterance_info *info,int native_volume);

#endif
