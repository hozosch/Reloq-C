#ifndef RELOQ_HOST_H
#define RELOQ_HOST_H

#include <stdint.h>

/* Stable, fixed-width boundary for a future Windows NVDA host or another
 * audio client. No compiler-specific C structs from the synthesizer cross
 * this boundary. The ABI accepts fully scheduled 62-control frames; a text
 * and language frontend is still required to turn arbitrary text into them.
 */
#if defined(_WIN32)
#define RELOQ_HOST_CALL __cdecl
#if defined(RELOQ_HOST_BUILD)
#define RELOQ_HOST_API __declspec(dllexport)
#else
#define RELOQ_HOST_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define RELOQ_HOST_CALL
#define RELOQ_HOST_API __attribute__((visibility("default")))
#else
#define RELOQ_HOST_CALL
#define RELOQ_HOST_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum { RELOQ_HOST_ABI_VERSION = 1, RELOQ_HOST_FRAME_CONTROLS = 62 };

enum reloq_host_result {
    RELOQ_HOST_OK = 0,
    RELOQ_HOST_CANCELLED = 1,
    RELOQ_HOST_INVALID = -1,
    RELOQ_HOST_MEMORY = -2,
    RELOQ_HOST_RENDER = -3,
    RELOQ_HOST_SINK = -4,
    RELOQ_HOST_TEXT_INVALID = -5,
    RELOQ_HOST_TEXT_UNCOVERED = -6,
    RELOQ_HOST_CAPACITY = -7
};

struct reloq_host;

/* struct_size must be sizeof(struct reloq_host_config), abi_version must be
 * RELOQ_HOST_ABI_VERSION. max_frame_samples limits only one control frame's
 * scratch allocation, not an utterance's length. The other settings map
 * directly to the independently measured C numeric renderer. */
struct reloq_host_config {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t sample_rate_hz;
    uint32_t max_frame_samples;
    float duration_scale;
    float output_gain;
    int32_t initial_seed;
    int32_t source_offset;
    int32_t av_trim;
    int32_t ah_trim;
    int32_t af_trim;
};

/* Called synchronously once per rendered frame. Copy PCM inside the callback
 * before returning; the pointer becomes invalid afterward. Returning a
 * nonzero value stops with RELOQ_HOST_SINK. The calling thread owns create,
 * reset, render and destroy; only cancel may run on another thread. The
 * caller must join that thread before destroying the host. A callback may
 * cancel, but must not reset, recursively render, or destroy its host. */
typedef int (RELOQ_HOST_CALL *reloq_host_pcm_sink)(
    void *context, const int16_t *pcm, uint32_t samples);

RELOQ_HOST_API uint32_t RELOQ_HOST_CALL reloq_host_abi_version(void);
RELOQ_HOST_API int RELOQ_HOST_CALL reloq_host_create(
    const struct reloq_host_config *config, struct reloq_host **out);
RELOQ_HOST_API void RELOQ_HOST_CALL reloq_host_destroy(
    struct reloq_host *host);
/* Reset before every new utterance. Render calls within one utterance retain
 * the numeric synthesis history; cancel marks it stale until reset. */
RELOQ_HOST_API int RELOQ_HOST_CALL reloq_host_reset(
    struct reloq_host *host);
RELOQ_HOST_API void RELOQ_HOST_CALL reloq_host_cancel(
    struct reloq_host *host);

/* controls is frame_count consecutive arrays of 62 float32 values in the
 * same index order as reloq_frame_adapter. No text, phoneme, or target-curve
 * generation takes place. fhighest selects F1..F8. A frame is counted once
 * its PCM callback accepts it, including one followed by cancellation.
 * Failure preserves already accepted frames but leaves the session's state
 * unspecified; reset before trying another utterance. pcm_sink must copy
 * data and return promptly. This call never queues samples on its own. */
RELOQ_HOST_API int RELOQ_HOST_CALL reloq_host_render_controls(
    struct reloq_host *host, const float *controls, uint32_t frame_count,
    uint32_t fhighest, reloq_host_pcm_sink pcm_sink, void *context,
    uint32_t *frames_completed);

/* ABI-stable German text analysis. Every field is uint32_t, including the
 * phone identifier from reloq_german_spr.h. Source spans index the returned
 * CP1252/SPR annotation (not the UTF-8 input). Neighbor flags describe
 * linguistic adjacency, including word and syllable boundaries; they do
 * not prescribe timing, prosody, acoustic targets or PCM. */
struct reloq_host_phone {
    uint32_t phone, left_phone, right_phone;
    uint32_t source_offset, source_length;
    uint32_t word_index, syllable_index, syllable_in_word;
    uint32_t stress, stress_explicit;
    uint32_t same_word_left, same_word_right;
    uint32_t same_syllable_left, same_syllable_right;
};

struct reloq_host_text_info {
    uint32_t struct_size; /* caller sets sizeof(struct reloq_host_text_info) */
    uint32_t annotation_bytes; /* excludes NUL; capacity includes NUL */
    uint32_t phone_count;
    uint32_t word_count, syllable_count;
    uint32_t error_offset; /* UTF-8 byte index on lexical failure, else UINT32_MAX */
};

/* This does not synthesize audio from text. Input is a bounded UTF-8 byte
 * span; the current restricted German rules reject uncovered words and
 * unsupported punctuation. On success, writes a NUL-terminated annotation
 * and exactly phone_count fixed-width phone records. NULL output pointers
 * with zero capacities query exact requirements, returning CAPACITY (the
 * annotation always needs at least a NUL byte). CAPACITY leaves outputs
 * untouched and fills
 * info with required counts. Invalid text returns TEXT_INVALID or
 * TEXT_UNCOVERED and records an error_offset; no partial output is exposed.
 * Only cancel may be called concurrently. Cancellation is observed during
 * text scanning, between words, and between phone parser events. A single
 * lexicon/G2P word or one scanner event is not yet preemptible. Reset clears a cancelled
 * state. Input and output buffers must remain alive and immutable during the
 * call, except that output buffers are written at successful completion.
 * Output annotation, phones and info must not overlap each other. */
RELOQ_HOST_API int RELOQ_HOST_CALL reloq_host_analyze_german(
    struct reloq_host *host, const unsigned char *utf8,
    uint32_t utf8_bytes, unsigned char *annotation,
    uint32_t annotation_capacity, struct reloq_host_phone *phones,
    uint32_t phone_capacity, struct reloq_host_text_info *info);

#ifdef __cplusplus
}
#endif

#endif
