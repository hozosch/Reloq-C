#include "reloq_session.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#if defined(__SSE2__) || defined(_M_X64)
#include <xmmintrin.h>
#endif

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

struct capture {
    int16_t pcm[1024];
    size_t samples;
    size_t calls;
    struct reloq_session *cancel_session;
    size_t cancel_after;
    int fail;
};

static int receive_pcm(void *context, const int16_t *pcm, size_t samples)
{
    struct capture *capture = context;
    if (capture->fail || samples > 1024 - capture->samples) return -1;
    memcpy(capture->pcm + capture->samples, pcm,
           samples * sizeof *pcm);
    capture->samples += samples;
    ++capture->calls;
    if (capture->cancel_session &&
        capture->calls == capture->cancel_after)
        reloq_session_cancel(capture->cancel_session);
    return 0;
}

static struct reloq_core_voiced_frame frame_for_test(void)
{
    struct reloq_core_voiced_frame frame = {0};
    frame.noise.duration_ms = 2.5f;
    frame.pitch_tenths = 1200.0f;
    frame.open_percent = 50.0f;
    frame.av_level = 50;
    frame.glottal_profile = 7;
    frame.noise.ah_level = 18;
    frame.noise.af_level = 25;
    frame.noise.bypass_level = 35.5f;
    frame.noise.tract.nasal_pole =
        (struct reloq_core_formant){260.0f, 75.0f};
    frame.noise.tract.nasal_zero =
        (struct reloq_core_formant){360.0f, 90.0f};
    frame.noise.tract.tracheal_pole =
        (struct reloq_core_formant){2100.0f, 140.0f};
    frame.noise.tract.tracheal_zero =
        (struct reloq_core_formant){2000.0f, 150.0f};
    frame.noise.tract.highest_formant = 8;
    for (int i = 0; i < 8; ++i) {
        frame.noise.tract.oral[i] = (struct reloq_core_formant){
            500.0f + 400.0f * (float)i,
            70.0f + 10.0f * (float)i};
        frame.noise.af_bandwidths[i] = 80.0f + 5.0f * (float)i;
        frame.noise.af_levels[i] = 25 - i;
    }
    return frame;
}

static void example_trace_frames(struct reloq_core_voiced_frame frames[3])
{
    static const float centers[8] = {
        500, 900, 1300, 1800, 2300, 2700, 3100, 3500
    };
    static const float widths[8] = {
        70, 90, 100, 110, 120, 130, 140, 150
    };
    frames[0] = frame_for_test();
    for (int i = 0; i < 8; ++i) {
        frames[0].noise.tract.oral[i].frequency_hz = centers[i];
        frames[0].noise.tract.oral[i].bandwidth_hz = widths[i];
    }
    frames[1] = frames[0];
    frames[1].pitch_tenths = 1350;
    frames[1].av_level = 46;
    frames[1].open_percent = 45;
    frames[1].glottal_profile = 12;
    frames[1].flutter = 3;
    frames[1].diplophonia = 8;
    frames[1].noise.ah_level = 0;
    frames[1].noise.af_level = 0;
    for (int i = 0; i < 8; ++i) {
        frames[1].noise.tract.oral[i].frequency_hz += 20;
        frames[1].noise.tract.oral[i].bandwidth_hz += 2;
    }
    frames[1].noise.tract.nasal_pole =
        (struct reloq_core_formant){270, 80};
    frames[1].noise.tract.nasal_zero =
        (struct reloq_core_formant){370, 95};
    frames[1].noise.tract.tracheal_pole =
        (struct reloq_core_formant){2120, 142};
    frames[1].noise.tract.tracheal_zero =
        (struct reloq_core_formant){2020, 152};
    frames[2] = frames[1];
    frames[2].av_level = 0;
    frames[2].flutter = 0;
    frames[2].diplophonia = 0;
}

#if defined(__SSE2__) || defined(_M_X64)
struct fp_capture { struct capture pcm; unsigned control; };
static int receive_fp(void *context,const int16_t *pcm,size_t n)
{
    struct fp_capture *c=context;
    if((_mm_getcsr() & ~0x3fu)!=c->control)return -1;
    return receive_pcm(&c->pcm,pcm,n);
}
static int check_fp_scope(void)
{
    unsigned saved=_mm_getcsr();
    const unsigned modes[]={0,0x8040,0x6000};
    const float rates[]={8000,11025,16000,22050,32000,44100,48000};
    for(unsigned rate=0;rate<7;++rate)for(unsigned mode=0;mode<3;++mode) {
        unsigned requested=(saved & ~(0x8040u|0x6000u|0x3fu))|modes[mode];
        _mm_setcsr(requested);
        struct reloq_session_config config={.sample_rate=rates[rate],
            .duration_scale=1,.output_gain=1,.initial_seed=100,.max_frame_samples=256};
        struct reloq_session *s=reloq_session_create(&config);CHECK(s);
        struct reloq_core_voiced_frame f=frame_for_test();
        struct fp_capture a={.control=requested & ~0x3fu},b={.control=requested & ~0x3fu};
        size_t complete=99;
        CHECK(reloq_session_render_frames(s,&f,1,receive_fp,&a,&complete)==RELOQ_SESSION_OK && complete==1);
        CHECK((_mm_getcsr() & ~0x3fu)==a.control);
        f.glottal_profile=36;
        CHECK(reloq_session_render_frames(s,&f,1,receive_fp,&a,&complete)==RELOQ_SESSION_RENDER && !complete && a.pcm.calls==1);
        CHECK((_mm_getcsr() & ~0x3fu)==a.control);
        CHECK(reloq_session_reset(s)==RELOQ_SESSION_OK);
        f=frame_for_test();
        CHECK(reloq_session_render_frames(s,&f,1,receive_fp,&b,&complete)==RELOQ_SESSION_OK && complete==1);
        CHECK(a.pcm.samples==b.pcm.samples && !memcmp(a.pcm.pcm,b.pcm.pcm,a.pcm.samples*sizeof(int16_t)));
        CHECK(reloq_session_reset(s)==RELOQ_SESSION_OK);
        b.pcm.fail=1;
        CHECK(reloq_session_render_frames(s,&f,1,receive_fp,&b,&complete)==RELOQ_SESSION_SINK && !complete);
        CHECK((_mm_getcsr() & ~0x3fu)==a.control);
        reloq_session_destroy(s);
    }
    _mm_setcsr(saved);
    return 0;
}
#endif

int main(void)
{
    struct reloq_session_config config = {
        .sample_rate = 16000.0f,
        .duration_scale = 1.0f,
        .output_gain = 1.25f,
        .initial_seed = 100,
        .max_frame_samples = 256
    };
    CHECK(reloq_session_create(NULL) == NULL);
    config.duration_scale = 0;
    CHECK(reloq_session_create(&config) == NULL);
    config.duration_scale = 1;
    config.output_gain = NAN;
    CHECK(reloq_session_create(&config) == NULL);
    config.output_gain = 1.25f;
    config.max_frame_samples = 0;
    CHECK(reloq_session_create(&config) == NULL);
    config.max_frame_samples = 256;
    struct reloq_session *session = reloq_session_create(&config);
    CHECK(session && !reloq_session_cancelled(session));

    struct reloq_core_voiced_frame frames[2] = {
        frame_for_test(), frame_for_test()
    };
    frames[1].pitch_tenths = 1350.0f;
    frames[1].glottal_profile = 12;
    struct capture capture = {0};
    size_t completed = 99;
    CHECK(reloq_session_render_frames(session, frames, 2, receive_pcm,
                                      &capture, &completed) ==
          RELOQ_SESSION_OK);
    CHECK(completed == 2 && capture.calls == 2 && capture.samples == 80);

    /* The wrapper's exact output gain order is checked against the core's
     * unscaled float frames and its integer PCM operations. */
    struct reloq_core_voiced_frame_state direct;
    CHECK(reloq_core_voiced_frame_initialize(&direct, 16000.0f, 100,
                                              0, 0, 0) == 0);
    float output[40], af[40], scratch[40];
    uint8_t mask[40];
    int16_t expected[80];
    for (size_t f = 0; f < 2; ++f) {
        size_t written = 0;
        int64_t intermediate[40];
        CHECK(reloq_core_voiced_frame_render(
                  &direct, &frames[f], output, af, scratch, mask, NULL,
                  40, &written) == 0 && written == 40);
        for (size_t i = 0; i < written; ++i)
            intermediate[i] = (int64_t)output[i];
        reloq_core_scale_output(intermediate, (long)written, 1.25f);
        reloq_core_pcm16_from_i64(intermediate, expected + 40 * f,
                                  written);
    }
    CHECK(memcmp(capture.pcm, expected, sizeof expected) == 0);

    /* Cancel on the callback boundary: accepted frame remains counted,
     * pending frames do not reach the sink. Reset restores deterministic
     * seeds and filter histories for the next numeric utterance. */
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture interrupted = {.cancel_session = session,
                                  .cancel_after = 1};
    CHECK(reloq_session_render_frames(session, frames, 2, receive_pcm,
                                      &interrupted, &completed) ==
          RELOQ_SESSION_CANCELLED);
    CHECK(completed == 1 && interrupted.calls == 1 &&
          interrupted.samples == 40 &&
          memcmp(interrupted.pcm, expected, 40 * sizeof(int16_t)) == 0);
    CHECK(reloq_session_cancelled(session));
    CHECK(reloq_session_render_frames(session, frames, 1, receive_pcm,
                                      &interrupted, &completed) ==
          RELOQ_SESSION_CANCELLED && completed == 0);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK &&
          !reloq_session_cancelled(session));
    struct capture restarted = {0};
    CHECK(reloq_session_render_frames(session, frames, 1, receive_pcm,
                                      &restarted, &completed) ==
          RELOQ_SESSION_OK && completed == 1 &&
          memcmp(restarted.pcm, expected, 40 * sizeof(int16_t)) == 0);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture rejected = {.fail = 1};
    CHECK(reloq_session_render_frames(session, frames, 1, receive_pcm,
                                      &rejected, &completed) ==
          RELOQ_SESSION_SINK && completed == 0);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    frames[0].noise.duration_ms = 13.0f;
    struct capture longer = {0};
    CHECK(reloq_session_render_frames(session, frames, 1, receive_pcm,
                                      &longer, &completed) ==
          RELOQ_SESSION_OK && longer.samples == 208 &&
          completed == 1);
    frames[0].noise.duration_ms = 2.5f;
    frames[0].glottal_profile = 36;
    CHECK(reloq_session_render_frames(session, frames, 1, receive_pcm,
                                      &longer, &completed) ==
          RELOQ_SESSION_RENDER && completed == 0 && longer.calls == 1);
    reloq_session_destroy(session);

    config.max_frame_samples = 39;
    session = reloq_session_create(&config);
    CHECK(session);
    frames[0].glottal_profile = 7;
    CHECK(reloq_session_render_frames(session, frames, 1, receive_pcm,
                                      &capture, &completed) ==
          RELOQ_SESSION_INVALID && completed == 0);
    reloq_session_destroy(session);

    /* Independent CLI integration vector: tools/example.trace rendered by
     * reloq_trace_wav produces these 120 PCM samples. The session accepts
     * the same three scheduled numeric frames and must deliver byte-exact
     * PCM in order, including the source-off release in the third frame. */
    static const int16_t example_pcm[120] = {
        0, 0, 0, 2, 11, 36, 95, 208, 393, 647,
        951, 1267, 1555, 1788, 1955, 2061, 2116, 2127, 2100, 2046,
        1982, 1925, 1888, 1872, 1872, 1880, 1884, 1881, 1869, 1848,
        1816, 1772, 1712, 1639, 1554, 1462, 1367, 1267, 1158, 1035,
        894, 738, 576, 420, 277, 143, 6, -150, -341, -571,
        -835, -1128, -1445, -1782, -2135, -2497, -2855, -3197,
        -3516, -3813,
        -4090, -4353, -4605, -4852, -5097, -5347, -5604, -5869,
        -6129, -6326, -6319, -5849, -4601, -2385, 622, 3801,
        6281, 7382, 6984, 5540, 3737, 2059, 628, -639, -1744,
        -2494, -2640, -2107, -1100, -1, 846, 1306, 1457, 1481,
        1514, 1581, 1612, 1539, 1375, 1217, 1185, 1322, 1549,
        1700, 1626, 1312, 917, 707, 903, 1536, 2385, 3076,
        3259, 2796, 1806, 556, -703, -1859, -2893, -3782
    };
    struct reloq_core_voiced_frame example[3];
    example_trace_frames(example);
    config.max_frame_samples = 256;
    config.output_gain = 1.0f;
    session = reloq_session_create(&config);
    CHECK(session);
    struct capture example_capture = {0};
    CHECK(reloq_session_render_frames(session, example, 3, receive_pcm,
                                      &example_capture, &completed) ==
          RELOQ_SESSION_OK);
    if (example_capture.samples == 120)
        for (size_t i = 0; i < 120; ++i)
            if (example_capture.pcm[i] != example_pcm[i]) {
                fprintf(stderr, "trace PCM mismatch at %zu: %d != %d\n",
                        i, example_capture.pcm[i], example_pcm[i]);
                break;
            }
    CHECK(completed == 3 && example_capture.calls == 3 &&
          example_capture.samples == 120 &&
          memcmp(example_capture.pcm, example_pcm, sizeof example_pcm) == 0);
    reloq_session_destroy(session);
    /* Filter transients at the PCM limit must retain their sign. These
     * positions use the independent numeric example above; ordinary quiet
     * samples must retain exactly the configured gain. */
    config.output_gain = 16.0f;
    session = reloq_session_create(&config);
    CHECK(session);
    struct capture loud = {0};
    CHECK(reloq_session_render_frames(session, example, 3, receive_pcm,
                                      &loud, &completed) == RELOQ_SESSION_OK);
    CHECK(loud.samples == 120 && loud.pcm[3] == 32 &&
          loud.pcm[16] == INT16_MAX && loud.pcm[70] == INT16_MIN &&
          loud.pcm[76] == INT16_MAX);
    reloq_session_destroy(session);
    #if defined(__SSE2__) || defined(_M_X64)
    CHECK(check_fp_scope()==0);
    #endif
    return 0;
}
