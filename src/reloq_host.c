#define RELOQ_HOST_BUILD
#include "reloq_host.h"

#include "reloq_frame_adapter.h"
#include "reloq_german_annotation.h"
#include "reloq_german_phone_sequence.h"
#include "reloq_german_text.h"
#include "reloq_session.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct reloq_host {
    struct reloq_session *session;
};

struct callback_bridge {
    reloq_host_pcm_sink sink;
    void *context;
};

static int forward_pcm(void *context, const int16_t *pcm, size_t count)
{
    struct callback_bridge *bridge = context;
    if (count > UINT32_MAX) return -1;
    return bridge->sink(bridge->context, pcm, (uint32_t)count);
}

uint32_t RELOQ_HOST_CALL reloq_host_abi_version(void)
{
    return RELOQ_HOST_ABI_VERSION;
}

int RELOQ_HOST_CALL reloq_host_create(
    const struct reloq_host_config *config, struct reloq_host **out)
{
    if (!out) return RELOQ_HOST_INVALID;
    *out = NULL;
    if (!config || config->struct_size != sizeof *config ||
        config->abi_version != RELOQ_HOST_ABI_VERSION ||
        config->sample_rate_hz == 0 || config->max_frame_samples == 0)
        return RELOQ_HOST_INVALID;
    struct reloq_host *host = malloc(sizeof *host);
    if (!host) return RELOQ_HOST_MEMORY;
    struct reloq_session_config native = {
        .sample_rate = (float)config->sample_rate_hz,
        .duration_scale = config->duration_scale,
        .output_gain = config->output_gain,
        .initial_seed = config->initial_seed,
        .source_offset = config->source_offset,
        .av_trim = config->av_trim,
        .ah_trim = config->ah_trim,
        .af_trim = config->af_trim,
        .max_frame_samples = config->max_frame_samples
    };
    host->session = reloq_session_create(&native);
    if (!host->session) {
        free(host);
        return RELOQ_HOST_INVALID;
    }
    *out = host;
    return RELOQ_HOST_OK;
}

void RELOQ_HOST_CALL reloq_host_destroy(struct reloq_host *host)
{
    if (!host) return;
    reloq_session_destroy(host->session);
    free(host);
}

int RELOQ_HOST_CALL reloq_host_reset(struct reloq_host *host)
{
    return host ? reloq_session_reset(host->session) : RELOQ_HOST_INVALID;
}

void RELOQ_HOST_CALL reloq_host_cancel(struct reloq_host *host)
{
    if (host) reloq_session_cancel(host->session);
}

int RELOQ_HOST_CALL reloq_host_render_controls(
    struct reloq_host *host, const float *controls, uint32_t frame_count,
    uint32_t fhighest, reloq_host_pcm_sink pcm_sink, void *context,
    uint32_t *frames_completed)
{
    if (frames_completed) *frames_completed = 0;
    if (!host || fhighest < 1 || fhighest > 8 ||
        (frame_count && (!controls || !pcm_sink)))
        return RELOQ_HOST_INVALID;
#if SIZE_MAX < UINT64_MAX
    if ((size_t)frame_count > SIZE_MAX / RELOQ_HOST_FRAME_CONTROLS)
        return RELOQ_HOST_INVALID;
#endif
    struct callback_bridge bridge = {pcm_sink, context};
    for (uint32_t i = 0; i < frame_count; ++i) {
        struct reloq_core_voiced_frame frame;
        if (reloq_frame_adapter_map(
                controls + (size_t)i * RELOQ_HOST_FRAME_CONTROLS,
                (long)fhighest, &frame)) return RELOQ_HOST_INVALID;
        size_t done = 0;
        int result = reloq_session_render_frames(
            host->session, &frame, 1, forward_pcm, &bridge, &done);
        if (frames_completed) *frames_completed += (uint32_t)done;
        if (result != RELOQ_SESSION_OK) return result;
    }
    return RELOQ_HOST_OK;
}

static int text_result(int result)
{
    switch (result) {
    case RELOQ_DE_TEXT_CANCELLED: return RELOQ_HOST_CANCELLED;
    case RELOQ_DE_TEXT_INVALID: return RELOQ_HOST_TEXT_INVALID;
    case RELOQ_DE_TEXT_UNCOVERED: return RELOQ_HOST_TEXT_UNCOVERED;
    case RELOQ_DE_TEXT_CAPACITY: return RELOQ_HOST_CAPACITY;
    case RELOQ_DE_TEXT_NO_MEMORY: return RELOQ_HOST_MEMORY;
    default: return RELOQ_HOST_INVALID;
    }
}

static int host_cancelled(void *context)
{
    const struct reloq_host *host = context;
    return reloq_session_cancelled(host->session);
}

static struct reloq_host_phone pack_phone(
    const struct reloq_german_phone_node *node)
{
    struct reloq_host_phone result = {
        (uint32_t)node->phone,
        (uint32_t)node->left_phone,
        (uint32_t)node->right_phone,
        (uint32_t)node->source_offset,
        (uint32_t)node->source_length,
        (uint32_t)node->word_index,
        (uint32_t)node->syllable_index,
        (uint32_t)node->syllable_in_word,
        node->stress,
        node->stress_explicit,
        node->same_word_left,
        node->same_word_right,
        node->same_syllable_left,
        node->same_syllable_right
    };
    return result;
}

int RELOQ_HOST_CALL reloq_host_analyze_german(
    struct reloq_host *host, const unsigned char *utf8,
    uint32_t utf8_bytes, unsigned char *annotation,
    uint32_t annotation_capacity, struct reloq_host_phone *phones,
    uint32_t phone_capacity, struct reloq_host_text_info *info)
{
    if (!host || !utf8 || !info || info->struct_size != sizeof *info ||
        (!annotation && annotation_capacity) || (!phones && phone_capacity))
        return RELOQ_HOST_INVALID;

    struct reloq_host_text_info result = {
        (uint32_t)sizeof *info, 0, 0, 0, 0, UINT32_MAX
    };
    if (reloq_session_cancelled(host->session)) return RELOQ_HOST_CANCELLED;

    size_t length = 0, error = 0;
    int status = reloq_german_text_annotate_checked(
        utf8, utf8_bytes, NULL, 0, &length, &error,
        NULL, host_cancelled, host);
    if (status != RELOQ_DE_TEXT_OK) {
        if ((status == RELOQ_DE_TEXT_INVALID ||
             status == RELOQ_DE_TEXT_UNCOVERED) && error <= UINT32_MAX) {
            result.error_offset = (uint32_t)error;
            *info = result;
        }
        return text_result(status);
    }
    if (length >= UINT32_MAX) return RELOQ_HOST_INVALID;
    if (reloq_session_cancelled(host->session)) return RELOQ_HOST_CANCELLED;

    unsigned char *spr = malloc(length + 1);
    if (!spr) return RELOQ_HOST_MEMORY;
    size_t written = 0;
    status = reloq_german_text_annotate_checked(
        utf8, utf8_bytes, spr, length + 1, &written, NULL,
        NULL, host_cancelled, host);
    if (status != RELOQ_DE_TEXT_OK || written != length) {
        free(spr);
        return status == RELOQ_DE_TEXT_OK ? RELOQ_HOST_INVALID :
               text_result(status);
    }
    if (reloq_session_cancelled(host->session)) {
        free(spr);
        return RELOQ_HOST_CANCELLED;
    }

    struct reloq_german_annotation_cursor cursor = {0};
    struct reloq_german_annotation_event event;
    uint32_t count = 0, words = 0, syllables = 0;
    int parsed;
    while ((parsed = reloq_german_annotation_next(
                spr, length, &cursor, 0, &event)) == 1) {
        if (reloq_session_cancelled(host->session)) {
            free(spr);
            return RELOQ_HOST_CANCELLED;
        }
        if (event.kind == RELOQ_DE_ANNOTATION_SPR) {
            uint32_t *counter = NULL;
            if (event.spr.kind == RELOQ_DE_SPR_PHONE)
                counter = &count;
            else if (event.spr.kind == RELOQ_DE_SPR_WORD_OPEN)
                counter = &words;
            else if (event.spr.kind == RELOQ_DE_SPR_SYLLABLE)
                counter = &syllables;
            if (counter && *counter == UINT32_MAX) {
                free(spr);
                return RELOQ_HOST_INVALID;
            }
            if (counter) ++*counter;
        }
    }
    if (parsed != 0) {
        free(spr);
        return RELOQ_HOST_INVALID;
    }
    result.annotation_bytes = (uint32_t)length;
    result.phone_count = count;
    result.word_count = words;
    result.syllable_count = syllables;
    if (reloq_session_cancelled(host->session)) {
        free(spr);
        return RELOQ_HOST_CANCELLED;
    }
    if (!annotation || annotation_capacity <= length ||
        (count && (!phones || phone_capacity < count))) {
        *info = result;
        free(spr);
        return RELOQ_HOST_CAPACITY;
    }
#if SIZE_MAX < UINT64_MAX
    if ((size_t)count > SIZE_MAX / sizeof(struct reloq_german_phone_node)) {
        free(spr);
        return RELOQ_HOST_INVALID;
    }
#endif

    struct reloq_german_phone_node *nodes = NULL;
    if (count) {
        nodes = malloc((size_t)count * sizeof *nodes);
        if (!nodes) {
            free(spr);
            return RELOQ_HOST_MEMORY;
        }
    }
    struct reloq_german_phone_sequence_info counts = {0};
    int built = reloq_german_phone_sequence_build_checked(
        spr, length, 0, nodes, count, &counts, host_cancelled, host);
    if (built != 0 || counts.phone_count != count ||
        counts.word_count != words || counts.syllable_count != syllables) {
        free(nodes);
        free(spr);
        return built == 1 ? RELOQ_HOST_CANCELLED :
               built == -2 ? RELOQ_HOST_MEMORY : RELOQ_HOST_INVALID;
    }
    if (reloq_session_cancelled(host->session)) {
        free(nodes);
        free(spr);
        return RELOQ_HOST_CANCELLED;
    }
    /* All validation is complete before publishing either output. An
     * in-flight cancel after this check is handled by the caller's normal
     * generation check, as it is for PCM callbacks. */
    memcpy(annotation, spr, length + 1);
    for (uint32_t i = 0; i < count; ++i)
        phones[i] = pack_phone(nodes + i);
    *info = result;
    free(nodes);
    free(spr);
    return RELOQ_HOST_OK;
}
