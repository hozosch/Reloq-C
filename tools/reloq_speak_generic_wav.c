/* CLI hearing probe for the opt-in, approximate German C voice.
 * This executable never loads Apple/OpenEVV code. Its --lexicon file is a
 * verified word-to-SPR text format, not an acoustic recording or bytecode. */
#define _POSIX_C_SOURCE 200809L

#include "reloq_german_generic_voice.h"
#include "reloq_german_best_effort_text.h"
#include "reloq_german_lexicon_file.h"
#include "reloq_german_voice_defaults.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct sink {
    FILE *file;
    uint32_t samples;
    int error;
};

static int u16(FILE *f, uint16_t value)
{
    return fputc((int)(value & 255), f) == EOF ||
           fputc((int)((value >> 8) & 255), f) == EOF ? -1 : 0;
}

static int u32(FILE *f, uint32_t value)
{
    return u16(f, (uint16_t)value) || u16(f, (uint16_t)(value >> 16));
}

static int header(FILE *f, uint32_t samples)
{
    uint32_t bytes = samples * 2;
    return fwrite("RIFF", 1, 4, f) != 4 || u32(f, bytes + 36) ||
           fwrite("WAVEfmt ", 1, 8, f) != 8 || u32(f, 16) ||
           u16(f, 1) || u16(f, 1) || u32(f, 11025) ||
           u32(f, 22050) || u16(f, 2) || u16(f, 16) ||
           fwrite("data", 1, 4, f) != 4 || u32(f, bytes) ? -1 : 0;
}

static int receive(void *context, const int16_t *pcm, size_t count)
{
    struct sink *out = context;
    if (count > (UINT32_MAX - 36u) / 2u - out->samples) {
        out->error = 1;
        return -1;
    }
    for (size_t i = 0; i < count; ++i)
        if (u16(out->file, (uint16_t)pcm[i])) {
            out->error = 1;
            return -1;
        }
    out->samples += (uint32_t)count;
    return 0;
}

int main(int argc, char **argv)
{
    const char *lexicon_path = NULL;
    int best_effort = 0;
    int arg = 1;
    while (arg < argc) {
        if (!strcmp(argv[arg], "--best-effort")) {
            best_effort = 1;
            ++arg;
        } else if (!strcmp(argv[arg], "--lexicon") && arg + 1 < argc) {
            lexicon_path = argv[arg + 1];
            arg += 2;
        } else break;
    }
    if (argc - arg != 2) {
        fprintf(stderr, "usage: %s [--lexicon path.rlex] [--best-effort] 'German text or `[...]' output.wav\n",
                argv[0]);
        return 2;
    }
    const char *text = argv[arg], *path = argv[arg + 1];
    struct reloq_german_lexicon_file lexicon = {0};
    if (lexicon_path &&
        reloq_german_lexicon_file_load_path(&lexicon, lexicon_path)) {
        fprintf(stderr, "invalid or unreadable lexicon: %s\n", lexicon_path);
        return 1;
    }
    unsigned char *best_annotation = NULL;
    size_t annotation_length = 0, approximate_words = 0;
    if (best_effort) {
        size_t first = 0;
        while (text[first] == ' ' || text[first] == '\t' ||
               text[first] == '\r' || text[first] == '\n') ++first;
        if (text[first] != '`') {
            size_t bad = 0;
            int converted = reloq_german_best_effort_annotate(
                (const unsigned char *)text, strlen(text),
                lexicon.rows ? &lexicon.view : NULL,
                &best_annotation, &annotation_length,
                &approximate_words, &bad);
            if (converted) {
                fprintf(stderr, "experimental best-effort text: status %d at encoded byte %zu\n",
                        converted, bad);
                reloq_german_lexicon_file_clear(&lexicon);
                return 1;
            }
        }
    }
    float defaults[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    if (reloq_german_voice_defaults(defaults, fields, &config)) {
        free(best_annotation);
        reloq_german_lexicon_file_clear(&lexicon);
        return 1;
    }
    struct reloq_session *session = reloq_session_create(&config);
    if (!session || reloq_session_reset(session) != RELOQ_SESSION_OK) {
        fprintf(stderr, "could not create German C Klatt session\n");
        reloq_session_destroy(session);
        free(best_annotation);
        reloq_german_lexicon_file_clear(&lexicon);
        return 1;
    }
    size_t length = strlen(path);
    static const char suffix[] = ".tmp.XXXXXX";
    if (length > SIZE_MAX - sizeof suffix) {
        reloq_session_destroy(session);
        free(best_annotation);
        reloq_german_lexicon_file_clear(&lexicon);
        return 1;
    }
    char *temporary = malloc(length + sizeof suffix);
    if (!temporary) {
        reloq_session_destroy(session);
        reloq_german_lexicon_file_clear(&lexicon);
        return 1;
    }
    memcpy(temporary, path, length);
    memcpy(temporary + length, suffix, sizeof suffix);
    int fd = mkstemp(temporary);
    if (fd < 0) {
        fprintf(stderr, "could not create output: %s\n", strerror(errno));
        free(temporary);
        reloq_session_destroy(session);
        free(best_annotation);
        reloq_german_lexicon_file_clear(&lexicon);
        return 1;
    }
    FILE *file = fdopen(fd, "wb+");
    if (!file) {
        fprintf(stderr, "could not open output: %s\n", strerror(errno));
        close(fd);
        unlink(temporary);
        free(temporary);
        reloq_session_destroy(session);
        free(best_annotation);
        reloq_german_lexicon_file_clear(&lexicon);
        return 1;
    }
    struct sink output = {file, 0, 0};
    struct reloq_german_utterance_info progress = {0};
    int status = header(file, 0);
    if (!status && best_annotation)
        status = reloq_german_generic_render_annotation(
            best_annotation, annotation_length, session,
            receive, &output, &progress);
    else if (!status)
        status = reloq_german_generic_render(
            (const unsigned char *)text, strlen(text),
            lexicon.rows ? &lexicon.view : NULL, session,
            receive, &output, &progress);
    if (!status && (fseek(file, 0, SEEK_SET) ||
                    header(file, output.samples) || fflush(file)))
        status = RELOQ_DE_UTTERANCE_SINK;
    if (fclose(file)) status = RELOQ_DE_UTTERANCE_SINK;
    if (!status && rename(temporary, path)) status = RELOQ_DE_UTTERANCE_SINK;
    if (status) {
        unlink(temporary);
        fprintf(stderr, "experimental voice: status %d, byte %zu, phones %zu, "
                "frames %zu%s\n", status, progress.error_offset,
                progress.phone_count, progress.frames_completed,
                output.error ? ", WAV size or write failed" : "");
    } else {
        fprintf(stderr, "experimental voice: %zu phones, %zu frames, "
                "%u samples at 11025 Hz; %zu guessed words%s\n",
                progress.phone_count, progress.frames_completed,
                output.samples, approximate_words,
                approximate_words ? " (pronunciation may be wrong)" : "");
    }
    free(temporary);
    reloq_session_destroy(session);
    free(best_annotation);
    reloq_german_lexicon_file_clear(&lexicon);
    return status ? 1 : 0;
}
