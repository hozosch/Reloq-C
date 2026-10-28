/* Render the narrow native German voice prototype to a PCM16 WAV file.
 * It covers stressed one-syllable German words ending in a period, including
 * several complete sentences in one call, for example "Hack. Hieb.".
 * No Apple code,
 * pre-recorded audio, or substitute PCM is used. */
#define _POSIX_C_SOURCE 200809L

#include "reloq_german_voice.h"
#include "reloq_german_voice_defaults.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { SAMPLE_RATE = 11025, PCM_BUFFER_SAMPLES = 2048 };

struct wav_sink {
    FILE *file;
    size_t samples;
    size_t nonzero;
    int io_error;
    int io_errno;
    int size_error;
};

static int u16le(FILE *file, uint16_t value)
{
    return fputc(value & 255u, file) == EOF ||
           fputc((value >> 8) & 255u, file) == EOF ? -1 : 0;
}

static int u32le(FILE *file, uint32_t value)
{
    return u16le(file, (uint16_t)value) ||
           u16le(file, (uint16_t)(value >> 16)) ? -1 : 0;
}

static int wav_header(FILE *file, uint32_t samples)
{
    uint32_t bytes = samples * 2u;
    return fwrite("RIFF", 1, 4, file) != 4 ||
           u32le(file, 36u + bytes) ||
           fwrite("WAVEfmt ", 1, 8, file) != 8 ||
           u32le(file, 16) || u16le(file, 1) || u16le(file, 1) ||
           u32le(file, SAMPLE_RATE) || u32le(file, SAMPLE_RATE * 2u) ||
           u16le(file, 2) || u16le(file, 16) ||
           fwrite("data", 1, 4, file) != 4 || u32le(file, bytes) ? -1 : 0;
}

static int receive_pcm(void *context, const int16_t *pcm, size_t count)
{
    struct wav_sink *sink = context;
    /* The RIFF chunk size is 36 + 2 * samples. Reject overflow before
     * writing any of the offending frame; the temporary file is discarded. */
    if (count > (UINT32_MAX - 36u) / 2u - sink->samples) {
        sink->size_error = 1;
        return -1;
    }
    unsigned char bytes[PCM_BUFFER_SAMPLES * 2];
    for (size_t offset = 0; offset < count;) {
        size_t n = count - offset;
        if (n > PCM_BUFFER_SAMPLES) n = PCM_BUFFER_SAMPLES;
        for (size_t i = 0; i < n; ++i) {
            uint16_t value = (uint16_t)pcm[offset + i];
            bytes[i * 2] = (unsigned char)value;
            bytes[i * 2 + 1] = (unsigned char)(value >> 8);
            sink->nonzero += value != 0;
        }
        if (fwrite(bytes, 2, n, sink->file) != n) {
            sink->io_error = 1;
            sink->io_errno = errno;
            return -1;
        }
        offset += n;
    }
    sink->samples += count;
    return 0;
}

static void explain_render_error(int status, const struct wav_sink *sink,
                                 const struct reloq_german_utterance_info *info)
{
    switch (status) {
    case RELOQ_DE_UTTERANCE_INVALID:
        fprintf(stderr, "invalid UTF-8 or text at byte %zu\n", info->error_offset);
        break;
    case RELOQ_DE_UTTERANCE_UNCOVERED:
        fprintf(stderr, "text is outside the current German voice coverage"
                        " (byte %zu); try complete simple sentences"
                        " with periods, such as Hack. Hieb.\n",
                info->error_offset);
        break;
    case RELOQ_DE_UTTERANCE_SINK:
        if (sink->size_error)
            fprintf(stderr, "WAV exceeds the RIFF 32-bit size limit\n");
        else if (sink->io_error)
            fprintf(stderr, "could not write temporary WAV: %s\n",
                    strerror(sink->io_errno));
        else
            fprintf(stderr, "PCM output was rejected\n");
        break;
    case RELOQ_DE_UTTERANCE_MEMORY:
        fprintf(stderr, "out of memory\n");
        break;
    case RELOQ_DE_UTTERANCE_CANCELLED:
        fprintf(stderr, "render cancelled\n");
        break;
    default:
        fprintf(stderr, "voice render failed (status %d)\n", status);
        break;
    }
}

static int render(const char *text, const char *path)
{
    int result = 1, fd = -1;
    FILE *file = NULL;
    char *temporary = NULL;
    struct reloq_session *session = NULL;
    size_t path_bytes = strlen(path);
    static const char suffix[] = ".tmp.XXXXXX";

    if (path_bytes > SIZE_MAX - sizeof suffix) {
        fprintf(stderr, "output path is too long\n");
        return result;
    }
    temporary = malloc(path_bytes + sizeof suffix);
    if (!temporary) {
        fprintf(stderr, "out of memory\n");
        return result;
    }
    memcpy(temporary, path, path_bytes);
    memcpy(temporary + path_bytes, suffix, sizeof suffix);

    float controls[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    if (reloq_german_voice_defaults(controls, fields, &config) ||
        config.sample_rate != (float)SAMPLE_RATE ||
        !(session = reloq_session_create(&config)) ||
        reloq_session_reset(session) != RELOQ_SESSION_OK) {
        fprintf(stderr, "could not initialize the 11025 Hz German voice\n");
        goto cleanup;
    }

    fd = mkstemp(temporary);
    if (fd < 0) {
        fprintf(stderr, "could not create temporary WAV near %s: %s\n",
                path, strerror(errno));
        goto cleanup;
    }
    file = fdopen(fd, "wb+");
    if (!file) {
        fprintf(stderr, "could not open temporary WAV: %s\n", strerror(errno));
        goto cleanup;
    }
    fd = -1; /* FILE now owns the descriptor. */
    if (wav_header(file, 0)) {
        fprintf(stderr, "could not write WAV header: %s\n", strerror(errno));
        goto cleanup;
    }

    struct wav_sink sink = {.file = file};
    struct reloq_german_utterance_info info;
    int status = reloq_german_voice_render((const unsigned char *)text,
                                           strlen(text), session,
                                           receive_pcm, &sink, &info);
    if (status != RELOQ_DE_UTTERANCE_OK) {
        explain_render_error(status, &sink, &info);
        goto cleanup;
    }
    if (sink.samples == 0 || sink.nonzero == 0 ||
        info.frames_completed == 0) {
        fprintf(stderr, "voice produced no audible PCM\n");
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) || wav_header(file, (uint32_t)sink.samples) ||
        fflush(file) || fsync(fileno(file))) {
        fprintf(stderr, "could not finalize WAV: %s\n", strerror(errno));
        goto cleanup;
    }
    if (fclose(file)) {
        file = NULL;
        fprintf(stderr, "could not close WAV: %s\n", strerror(errno));
        goto cleanup;
    }
    file = NULL;
    if (rename(temporary, path)) {
        fprintf(stderr, "could not replace %s: %s\n", path, strerror(errno));
        goto cleanup;
    }
    printf("%zu frames, %zu samples, %zu nonzero, %d Hz -> %s\n",
           info.frames_completed, sink.samples, sink.nonzero,
           SAMPLE_RATE, path);
    result = 0;

cleanup:
    if (file && fclose(file)) fprintf(stderr, "temporary WAV close failed\n");
    if (fd >= 0) close(fd);
    if (temporary) {
        if (result) unlink(temporary);
        free(temporary);
    }
    reloq_session_destroy(session);
    return result;
}

int main(int argc, char **argv)
{
    if (argc != 3 || !argv[1][0] || !argv[2][0]) {
        fprintf(stderr, "usage: %s 'Hack. Hieb.' output.wav\n", argv[0]);
        return 2;
    }
    return render(argv[1], argv[2]);
}
