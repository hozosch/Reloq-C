#ifndef RELOQ_GERMAN_UTTERANCE_H
#define RELOQ_GERMAN_UTTERANCE_H

#include "reloq_german_target_planner.h"
#include "reloq_lexicon.h"
#include "reloq_session.h"

/* Composition boundary for a caller-authored German voice model. The model
 * supplies all acoustic defaults, stream locations, timing and context rules;
 * this library does not contain a complete Eloquence voice or dictionary.
 * Keep the model, its rules and the optional lexicon immutable during the
 * call. The session must have been reset by its owning thread before a new
 * utterance. Only reloq_session_cancel may run concurrently. */
struct reloq_german_utterance_model {
    const struct reloq_lexicon *lexicon;
    const struct reloq_german_target_rule *rules;
    size_t rule_count;
    uint8_t field_for_stream[RELOQ_TARGET_STREAM_COUNT];
    float defaults[RELOQ_PARAMETER_TRACKS];
    uint32_t frame_ticks;
    int64_t trailing_ticks;
    int16_t utterance_tag;
    long highest_formant;
};

enum reloq_german_utterance_result {
    RELOQ_DE_UTTERANCE_OK = 0,
    RELOQ_DE_UTTERANCE_CANCELLED = 1,
    RELOQ_DE_UTTERANCE_INVALID = -1,
    RELOQ_DE_UTTERANCE_UNCOVERED = -2,
    RELOQ_DE_UTTERANCE_MEMORY = -3,
    RELOQ_DE_UTTERANCE_MODEL = -4,
    RELOQ_DE_UTTERANCE_RENDER = -5,
    RELOQ_DE_UTTERANCE_SINK = -6
};

struct reloq_german_utterance_info {
    size_t annotation_bytes;
    size_t phone_count;
    size_t target_count;
    int64_t planned_ticks;
    size_t frames_completed;
    size_t error_offset; /* UTF-8 byte offset on INVALID/UNCOVERED text */
};

/* Compose UTF-8 text, German SPR, phone context, caller-authored target
 * rules, acoustic curves and C PCM rendering. No Apple/OpenEVV runtime is
 * called. Target/phone storage grows with the input and model rather than
 * truncating the utterance. A text or model failure emits no PCM. A render,
 * sink or cancellation result may follow accepted frames, whose count is
 * reported. Punctuation is preserved through annotation but this interface
 * cannot yet infer phrase-boundary timing: the supplied model is responsible
 * for its own prosody and trailing ticks. Empty text emits no PCM.
 *
 * The PCM callback is synchronous. Its pointer is valid only until return.
 * Returns an enum reloq_german_utterance_result, sets *info to the work done,
 * and releases all temporary storage before returning. */
int reloq_german_utterance_render(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_german_utterance_model *model,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info);

#endif
