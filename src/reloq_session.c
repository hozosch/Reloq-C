#include "reloq_session.h"
#include "reloq_core_internal.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#if defined(__SSE2__) || defined(_M_X64)
#include <xmmintrin.h>
#define RELOQ_SESSION_SSE_CONTROL 1
#endif

/* MSVC's C11 mode does not supply <stdatomic.h>. Keep cancellation atomic
 * on that compiler using its 32-bit Interlocked intrinsics. MinGW, Clang,
 * GCC and other C11 compilers use the standard atomics below. No Windows
 * header or platform type enters the public session interface. */
#if defined(_MSC_VER)
#include <intrin.h>
typedef volatile long reloq_epoch_atomic;
static void epoch_init(reloq_epoch_atomic *value)
{
    *value = 0; /* initialization precedes publication of the session */
}
static uint32_t epoch_load(const reloq_epoch_atomic *value)
{
    return (uint32_t)_InterlockedCompareExchange(
        (volatile long *)value, 0, 0);
}
static void epoch_store(reloq_epoch_atomic *value, uint32_t next)
{
    (void)_InterlockedExchange(value, (long)next);
}
static void epoch_increment(reloq_epoch_atomic *value)
{
    (void)_InterlockedIncrement(value);
}
#else
#include <stdatomic.h>
typedef atomic_uint reloq_epoch_atomic;
static void epoch_init(reloq_epoch_atomic *value)
{
    atomic_init(value, 0);
}
static uint32_t epoch_load(const reloq_epoch_atomic *value)
{
    return (uint32_t)atomic_load_explicit(value, memory_order_relaxed);
}
static void epoch_store(reloq_epoch_atomic *value, uint32_t next)
{
    atomic_store_explicit(value, next, memory_order_relaxed);
}
static void epoch_increment(reloq_epoch_atomic *value)
{
    (void)atomic_fetch_add_explicit(value, 1, memory_order_relaxed);
}
#endif

enum { RELOQ_PCM_CONVERSION_BLOCK = 4096 };

struct reloq_session_buffers {
    float *output;
    float *raw_af;
    float *scratch;
    uint8_t *closed_mask;
    int16_t *pcm;
    size_t capacity;
};

struct reloq_session {
    struct reloq_session_config config;
    struct reloq_core_voiced_frame_state frame_state;
    struct reloq_session_buffers buffers;
    reloq_epoch_atomic cancel_epoch;
    reloq_epoch_atomic active_epoch;
    unsigned high_rate_profile;
    int text_loudness;
    float linear_gain;
    double text_peak_gain;
};

float reloq_session_sample_rate(const struct reloq_session *s)
{ return s ? s->config.sample_rate : 0; }
int reloq_session_float_volume(const struct reloq_session *s)
{return s&&(s->text_loudness||(s->high_rate_profile&&s->config.sample_rate>=16000&&s->config.glottal_reference_rate>0));}
int reloq_session_text_loudness(struct reloq_session *s,int enabled)
{
    if(!s||(enabled!=0&&enabled!=1))return RELOQ_SESSION_INVALID;
    s->text_loudness=enabled;return RELOQ_SESSION_OK;
}
int reloq_session_linear_gain(struct reloq_session *s,float gain)
{
    if(!s||!isfinite(gain)||gain<0||gain>16)return RELOQ_SESSION_INVALID;
    s->linear_gain=gain;return RELOQ_SESSION_OK;
}

int reloq_session_high_rate_profile(struct reloq_session *s,unsigned profile)
{
    if(!s||profile>3)return RELOQ_SESSION_INVALID;
    s->high_rate_profile=profile;return RELOQ_SESSION_OK;
}

int reloq_session_source_levels(struct reloq_session *s,long source,
    long av,long ah,long af,unsigned reference_formants)
{
    if(!s || source < -120 || source>120 || av < -120 || av>120 ||
       ah < -120 || ah>120 || af < -120 || af>120 || reference_formants>8)
        return RELOQ_SESSION_INVALID;
    s->frame_state.noise.source_offset=source;
    s->frame_state.noise.ah_trim=ah;
    s->frame_state.noise.af_trim=af;
    s->frame_state.av_trim=av;
    s->frame_state.reference_formants=reference_formants;
    return RELOQ_SESSION_OK;
}

static void free_buffers(struct reloq_session_buffers *buffers)
{
    free(buffers->output);
    free(buffers->raw_af);
    free(buffers->scratch);
    free(buffers->closed_mask);
    free(buffers->pcm);
    memset(buffers, 0, sizeof *buffers);
}

static int reserve_buffers(struct reloq_session_buffers *buffers, size_t count)
{
    if (count <= buffers->capacity) return RELOQ_SESSION_OK;
    if (count > SIZE_MAX / sizeof(float) ||
        count > SIZE_MAX / sizeof(int16_t)) return RELOQ_SESSION_INVALID;
    struct reloq_session_buffers next = {0};
    next.output = malloc(count * sizeof(float));
    next.raw_af = malloc(count * sizeof(float));
    next.scratch = malloc(count * sizeof(float));
    next.closed_mask = malloc(count);
    next.pcm = malloc(count * sizeof(int16_t));
    if (!next.output || !next.raw_af || !next.scratch ||
        !next.closed_mask || !next.pcm) {
        free_buffers(&next);
        return RELOQ_SESSION_MEMORY;
    }
    free_buffers(buffers);
    next.capacity = count;
    *buffers = next;
    return RELOQ_SESSION_OK;
}

static int output_pcm(struct reloq_session *session, size_t count)
{
    const float *output = session->buffers.output;
    int16_t *pcm = session->buffers.pcm;
    float gain = session->config.output_gain;
#ifndef RELOQ_SESSION_FLOAT_DIAGNOSTIC
    /* Restore the measured 100-vs-92 ECI loudness ratio. Do this while
     * samples are still wide so transient protection can prevent clipping. */
    if(session->text_loudness)gain*=2768.533f/2066.394f;
#endif
#ifdef RELOQ_SESSION_FLOAT_DIAGNOSTIC
    gain*=.015625f;
#endif
    int64_t block[RELOQ_PCM_CONVERSION_BLOCK];
    for (size_t at = 0; at < count; ) {
        size_t n = count - at;
        if (n > RELOQ_PCM_CONVERSION_BLOCK) n = RELOQ_PCM_CONVERSION_BLOCK;
        for (size_t i = 0; i < n; ++i) {
            float sample = output[at + i]*session->linear_gain;
            if (!isfinite(sample) || sample < -0x1p63f ||
                sample >= 0x1p63f) return RELOQ_SESSION_RENDER;
            block[i] = (int64_t)sample;
            float scaled = (float)block[i] * gain;
            if (!isfinite(scaled) || scaled < -0x1p63f ||
                scaled >= 0x1p63f) return RELOQ_SESSION_RENDER;
        }
        reloq_core_scale_output(block, (long)n, gain);
        /* Keep the existing final protection while the remaining model
         * headroom cases are investigated. This is not evidence that the
         * unbounded synthesis fits PCM: raw-source tests and observations
         * must measure before this knee. */
#ifndef RELOQ_SESSION_FLOAT_DIAGNOSTIC
        if(session->text_loudness) {
            /* Short source onsets can be conspicuous without reaching the
             * PCM rails. A continuous knee and an 8 ms gain recovery tame
             * them before PCM conversion without an extra audio buffer. */
            const double recovery=-expm1(-1.0/(session->config.sample_rate*.008));
            float *anticipated=session->buffers.scratch+at;
            if(session->config.sample_rate>=22050) {
                /* The whole native frame has already been synthesized.
                 * Anticipate peaks within that frame, easing the gain down
                 * before the peak rather than changing it abruptly on a
                 * single high-frequency sample. This adds no audio buffer
                 * or samples and leaves the accepted 8/11/16-kHz path. */
                const double attack=exp(-1.0/(session->config.sample_rate*.002));
                double future=1;
                for(size_t i=n;i>0;--i) {
                    double amplitude=fabs((double)block[i-1]);
                    double desired=1;
                    if(amplitude>22000) {
                        double bounded=22000-6000*expm1(-(amplitude-22000)/6000);
                        desired=bounded/amplitude;
                    }
                    double approaching=1-(1-future)*attack;
                    future=desired<approaching?desired:approaching;
                    anticipated[i-1]=(float)future;
                }
            }
            for(size_t i=0;i<n;++i) {
                double amplitude=fabs((double)block[i]);
                double desired=1;
                if(session->config.sample_rate>=22050)desired=anticipated[i];
                else if(amplitude>22000) {
                    double bounded=22000-6000*expm1(-(amplitude-22000)/6000);
                    desired=bounded/amplitude;
                }
                if(desired<session->text_peak_gain)session->text_peak_gain=desired;
                else {
                    session->text_peak_gain+=(1-session->text_peak_gain)*recovery;
                    if(session->text_peak_gain>desired)session->text_peak_gain=desired;
                }
                block[i]=(int64_t)((double)block[i]*session->text_peak_gain);
            }
        }
        else if(session->high_rate_profile && session->config.sample_rate>=16000)
            for(size_t i=0;i<n;++i) {
                double amplitude=fabs((double)block[i]);
                if(amplitude>30000) {
                    double bounded=30000-2600*expm1(-(amplitude-30000)/2600);
                    block[i]=(int64_t)copysign(bounded,(double)block[i]);
                }
            }
#endif
        /* The sample sink has a finite range. A model/filter transient
         * must saturate there instead of wrapping across the sign bit;
         * retain the core's low-word conversion for numeric reference use. */
        for (size_t i = 0; i < n; ++i) {
            if (block[i] > INT16_MAX) block[i] = INT16_MAX;
            else if (block[i] < INT16_MIN) block[i] = INT16_MIN;
        }
        reloq_core_pcm16_from_i64(block, pcm + at, n);
        at += n;
    }
    return RELOQ_SESSION_OK;
}

/* Wideband filter tails can enter the subnormal range long after their
 * PCM output has become zero. Keep these inaudible values off the slow
 * arithmetic path. The mode is local to this numeric call on this thread;
 * caller rounding, exception masks and the callback environment survive.
 * Standalone core operations retain their strict floating-point contract. */
static int render_owned(struct reloq_session *session,
    const struct reloq_core_voiced_frame *frame, size_t *written)
{
    struct reloq_session_buffers *b = &session->buffers;
#ifdef RELOQ_SESSION_SSE_CONTROL
    unsigned before = 0;
    int change = 0;
    if (session->config.sample_rate >= 16000.0f) {
        before = _mm_getcsr();
        change = (before & 0x8040u) != 0x8040u;
        if (change) _mm_setcsr(before | 0x8040u); /* FTZ and DAZ */
    }
#endif
    int status = reloq_core_voiced_frame_render_owned(
        &session->frame_state,frame,b->output,b->raw_af,
        b->scratch,b->closed_mask,NULL,b->capacity,written);
#ifdef RELOQ_SESSION_SSE_CONTROL
    if (change) {
        unsigned raised = _mm_getcsr() & 0x3fu;
        _mm_setcsr(before | raised);
    }
#endif
    return status;
}

struct reloq_session *reloq_session_create(
    const struct reloq_session_config *config)
{
    if (!config || !isfinite(config->sample_rate) ||
        config->sample_rate <= 0.0f || !isfinite(config->glottal_reference_rate) ||
        !isfinite(config->frication_reference_rate) || config->frication_reference_rate<0 ||
        config->glottal_reference_rate<0 || !isfinite(config->duration_scale) ||
        config->duration_scale <= 0.0f || !isfinite(config->output_gain) ||
        config->output_gain < 0.0f || !config->max_frame_samples ||
        config->max_frame_samples > SIZE_MAX / sizeof(float)) return NULL;
    struct reloq_session *session = calloc(1, sizeof *session);
    if (!session) return NULL;
    session->config = *config;
    epoch_init(&session->cancel_epoch);
    epoch_init(&session->active_epoch);
    if (reloq_session_reset(session) != RELOQ_SESSION_OK) {
        free(session);
        return NULL;
    }
    return session;
}

void reloq_session_destroy(struct reloq_session *session)
{
    if (!session) return;
    free_buffers(&session->buffers);
    free(session);
}

int reloq_session_reset(struct reloq_session *session)
{
    if (!session) return RELOQ_SESSION_INVALID;
    uint32_t generation = epoch_load(&session->cancel_epoch);
    struct reloq_core_voiced_frame_state next;
    const struct reloq_session_config *c = &session->config;
    if (reloq_core_voiced_frame_initialize(
            &next, c->sample_rate, c->initial_seed, c->source_offset,
            c->ah_trim, c->af_trim) ||
        reloq_core_voiced_frame_set_av_trim(&next, c->av_trim) ||
        reloq_core_noise_frame_set_duration_scale(&next.noise,
                                                   c->duration_scale))
        return RELOQ_SESSION_INVALID;
    next.glottal_reference_rate=c->glottal_reference_rate;
    next.reference_formants=c->reference_formants;
    next.frication_reference_rate=c->frication_reference_rate;
    /* B preserves the accepted A voice and Classic consonants. Optional C
     * applies a small fixed periodic contour on this same voice from 22 kHz;
     * it does not change the glottal source, aspiration or AF noise. */
    unsigned classic=session->high_rate_profile==3?1:session->high_rate_profile;
    next.rate_transfer.trial_profile=c->sample_rate<16000?classic:0;
    if(session->high_rate_profile==3&&c->glottal_reference_rate>0)
        next.rate_transfer.body_amount=c->sample_rate>=48000?.41666667f:
            c->sample_rate>=44100?.36666667f:c->sample_rate>=32000?.26666667f:
            c->sample_rate>=22050?.15f:0;
    session->linear_gain=1;
    session->text_peak_gain=1;
    next.noise_transfer.trial_profile=classic;
    session->frame_state = next;
    epoch_store(&session->active_epoch, generation);
    return RELOQ_SESSION_OK;
}

void reloq_session_cancel(struct reloq_session *session)
{
    if (session) epoch_increment(&session->cancel_epoch);
}

int reloq_session_cancelled(const struct reloq_session *session)
{
    return !session ||
        epoch_load(&session->active_epoch) !=
        epoch_load(&session->cancel_epoch);
}

int reloq_session_render_frames(struct reloq_session *session,
    const struct reloq_core_voiced_frame *frames, size_t count,
    reloq_session_pcm_sink sink, void *context, size_t *frames_completed)
{
    if (frames_completed) *frames_completed = 0;
    if (!session || (count && (!frames || !sink)))
        return RELOQ_SESSION_INVALID;
    for (size_t i = 0; i < count; ++i) {
        if (reloq_session_cancelled(session)) return RELOQ_SESSION_CANCELLED;
        long samples = reloq_core_frame_sample_count(
            frames[i].noise.duration_ms, session->config.duration_scale,
            session->frame_state.noise.clock.samples_per_millisecond);
        if (samples <= 0 || (uintmax_t)samples >
            session->config.max_frame_samples) return RELOQ_SESSION_INVALID;
        int reserved = reserve_buffers(&session->buffers, (size_t)samples);
        if (reserved != RELOQ_SESSION_OK) return reserved;
        size_t written = 0;
        struct reloq_session_buffers *b = &session->buffers;
        /* The session owns the state exclusively and requires a reset
         * after failure. Keep its histories in place across numeric
         * frames; the public core entry retains transactional copies. */
        if (render_owned(session, &frames[i], &written) ||
            written != (size_t)samples) return RELOQ_SESSION_RENDER;
        if (reloq_session_cancelled(session)) return RELOQ_SESSION_CANCELLED;
        int converted = output_pcm(session, written);
        if (converted != RELOQ_SESSION_OK) return converted;
        if (reloq_session_cancelled(session)) return RELOQ_SESSION_CANCELLED;
        int delivered = sink(context, b->pcm, written);
        if (delivered) return RELOQ_SESSION_SINK;
        if (frames_completed) ++*frames_completed;
        if (reloq_session_cancelled(session)) return RELOQ_SESSION_CANCELLED;
    }
    return RELOQ_SESSION_OK;
}
