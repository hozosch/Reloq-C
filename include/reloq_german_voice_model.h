#ifndef RELOQ_GERMAN_VOICE_MODEL_H
#define RELOQ_GERMAN_VOICE_MODEL_H
#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

struct reloq_german_voice_model {int head_size,roughness,breathiness,gender; float sample_rate;};
unsigned reloq_german_voice_model_formants(const struct reloq_german_voice_model *model);
/* Scalar controls are applied once, to a target with its phonetic owner,
 * before interpolation. The neutral male model is an identity operation. */
int32_t reloq_german_voice_model_target(const struct reloq_german_voice_model *model,
    const struct reloq_german_phone_node *phone,
    const struct reloq_german_phone_node *right,
    enum reloq_target_stream stream,int32_t value);
void reloq_german_voice_model_frame(void *model,float controls[62]);
#endif
