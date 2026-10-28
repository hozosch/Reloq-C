#include "reloq_german_utterance.h"
#include "reloq_frame_adapter.h"

#include <string.h>

#define CHECK(test) do { if (!(test)) return __LINE__; } while (0)
#define CENTER(phone_id) {0, phone_id, -1, -1, -1, -1}

/* Invented three-phone voice, useful for checking that every C stage is
 * connected. Its PCM must not be mistaken for an Apple German voice. */
static const struct reloq_german_target_condition m[] = {
    CENTER(RELOQ_DE_PHONE_m)
};
static const struct reloq_german_target_condition aw[] = {
    CENTER(RELOQ_DE_PHONE_Q_aw)
};
static const struct reloq_german_target_condition s[] = {
    CENTER(RELOQ_DE_PHONE_s)
};
static const struct reloq_target_event m_events[] = {
    {RELOQ_TARGET_F1, 0, 500}, {RELOQ_TARGET_AV, 0, 50},
    {RELOQ_TARGET_AF, 0, 0}
};
static const struct reloq_target_event aw_events[] = {
    {RELOQ_TARGET_F1, 0, 600}, {RELOQ_TARGET_AV, 0, 45}
};
static const struct reloq_target_event s_events[] = {
    {RELOQ_TARGET_AV, 0, 0}, {RELOQ_TARGET_AF, 0, 35}
};
static const struct reloq_german_target_rule rules[] = {
    {m, 1, -1, 0, 10, m_events, 3},
    {aw, 1, -1, 0, 10, aw_events, 2},
    {s, 1, -1, 0, 10, s_events, 2}
};
static const uint8_t field_for_stream[RELOQ_TARGET_STREAM_COUNT] = {
    1, 9, 13, 15, 17, 19, 10, 14, 16, 18, 20, 27, 29, 31, 33,
    4, 3, 5, 6, 2, 7, 8, 35, 36, 37, 38, 39, 43
};

struct capture {
    int16_t pcm[160];
    size_t samples, calls, cancel_after;
    struct reloq_session *session;
    int reject;
};

static int receive_pcm(void *context, const int16_t *pcm, size_t count)
{
    struct capture *capture = context;
    if (capture->reject || count > 160 - capture->samples) return -1;
    memcpy(capture->pcm + capture->samples, pcm, count * sizeof *pcm);
    capture->samples += count;
    ++capture->calls;
    if (capture->session && capture->calls == capture->cancel_after)
        reloq_session_cancel(capture->session);
    return 0;
}

static void defaults(float controls[RELOQ_PARAMETER_TRACKS])
{
    memset(controls, 0, RELOQ_PARAMETER_TRACKS * sizeof *controls);
    controls[0] = 2.5f;
    controls[1] = 1200;
    controls[3] = 50;
    controls[4] = 7;
    const size_t centers[8] = {9, 13, 15, 17, 19, 21, 23, 25};
    for (size_t i = 0; i < 8; ++i) {
        controls[centers[i]] = (float)(500 + i * 400);
        controls[centers[i] + 1] = (float)(70 + i * 10);
        controls[35 + i] = (float)(25 - i);
        controls[44 + i] = (float)(80 + i * 5);
    }
    const float nasal[8] = {260, 75, 360, 90, 2100, 140, 2000, 150};
    memcpy(controls + 27, nasal, sizeof nasal);
    controls[43] = 35.5f;
}

static void model(struct reloq_german_utterance_model *model)
{
    memset(model, 0, sizeof *model);
    model->rules = rules;
    model->rule_count = sizeof rules / sizeof rules[0];
    memcpy(model->field_for_stream, field_for_stream, sizeof field_for_stream);
    defaults(model->defaults);
    model->frame_ticks = 10;
    model->utterance_tag = 1;
    model->highest_formant = 8;
}

int main(void)
{
    const struct reloq_session_config config = {
        .sample_rate = 16000,
        .duration_scale = 1,
        .output_gain = 1,
        .initial_seed = 100,
        .max_frame_samples = 128
    };
    struct reloq_german_utterance_model voice;
    model(&voice);
    struct reloq_session *session = reloq_session_create(&config);
    struct reloq_session *direct = reloq_session_create(&config);
    CHECK(session && direct);
    struct capture actual = {0}, expected = {0};
    struct reloq_german_utterance_info info = {0};
    static const unsigned char word[] = "Maus";
    CHECK(reloq_german_utterance_render(word, sizeof word - 1,
          &voice, session, receive_pcm, &actual, &info) ==
          RELOQ_DE_UTTERANCE_OK);
    CHECK(info.annotation_bytes == strlen("`[.1m'aw's]") &&
          info.phone_count == 3 && info.target_count == 7 &&
          info.planned_ticks == 30 && info.frames_completed == 3 &&
          actual.calls == 3 && actual.samples == 120);

    /* Independent numeric-frame construction checks both event placement
     * and exact PCM, with the same explicitly authored synthetic controls. */
    struct reloq_core_voiced_frame frames[3];
    float controls[RELOQ_PARAMETER_TRACKS];
    defaults(controls);
    controls[9] = 500;
    controls[2] = 50;
    CHECK(reloq_frame_adapter_map(controls, 8, &frames[0]) == 0);
    controls[9] = 600;
    controls[2] = 45;
    /* AF interpolates halfway toward the final fricative target. */
    controls[8] = 17;
    CHECK(reloq_frame_adapter_map(controls, 8, &frames[1]) == 0);
    controls[2] = 0;
    controls[8] = 35;
    CHECK(reloq_frame_adapter_map(controls, 8, &frames[2]) == 0);
    size_t completed = 0;
    CHECK(reloq_session_render_frames(direct, frames, 3, receive_pcm,
                                      &expected, &completed) ==
          RELOQ_SESSION_OK);
    CHECK(completed == 3 && expected.samples == actual.samples &&
          memcmp(expected.pcm, actual.pcm,
                 expected.samples * sizeof(int16_t)) == 0);

    /* No partial PCM can escape a text or model failure. */
    static const unsigned char unknown[] = "Maus Bach";
    struct capture empty = {0};
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    CHECK(reloq_german_utterance_render(unknown, sizeof unknown - 1,
          &voice, session, receive_pcm, &empty, &info) ==
          RELOQ_DE_UTTERANCE_UNCOVERED);
    CHECK(info.error_offset == 5 && !empty.samples);
    voice.field_for_stream[RELOQ_TARGET_AV] =
        voice.field_for_stream[RELOQ_TARGET_F1];
    CHECK(reloq_german_utterance_render(word, sizeof word - 1,
          &voice, session, receive_pcm, &empty, &info) ==
          RELOQ_DE_UTTERANCE_MODEL && !empty.samples);
    model(&voice);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture cancelled = {.session = session, .cancel_after = 1};
    CHECK(reloq_german_utterance_render(word, sizeof word - 1,
          &voice, session, receive_pcm, &cancelled, &info) ==
          RELOQ_DE_UTTERANCE_CANCELLED);
    CHECK(info.frames_completed == 1 && cancelled.samples == 40 &&
          !memcmp(cancelled.pcm, expected.pcm, 40 * sizeof(int16_t)));
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture rejected = {.reject = 1};
    CHECK(reloq_german_utterance_render(word, sizeof word - 1,
          &voice, session, receive_pcm, &rejected, &info) ==
          RELOQ_DE_UTTERANCE_SINK && info.frames_completed == 0);

    reloq_session_destroy(direct);
    reloq_session_destroy(session);
    return 0;
}
