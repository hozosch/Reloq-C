/* Numeric Klatt frame trace -> little-endian PCM16 WAV.
 * This executable uses only the authored C11 core. See README.md for the
 * source-index mapping and text format. */
#include "reloq_core.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { FRAME_FIELDS = 64, WAV_HEADER_BYTES = 44, PCM_CHUNK = 4096 };

struct scanner {
    FILE *input;
    char *token;
    size_t capacity;
    size_t line;
    size_t token_line;
};

struct sample_buffers {
    float *output;
    float *raw_af;
    float *scratch;
    uint8_t *closed_mask;
    int16_t *pcm;
    size_t capacity;
};

/* Return 1 for a token, 0 for EOF, -1 for I/O, size, or allocation error.
 * A token (and a physical line) can grow without a fixed-size input limit. */
static int next_token(struct scanner *s)
{
    int c;
    size_t used = 0;
    for (;;) {
        c = fgetc(s->input);
        if (c == EOF) return ferror(s->input) ? -1 : 0;
        if (c == '\n') {
            if (s->line == SIZE_MAX) return -1;
            ++s->line;
        }
        if (isspace((unsigned char)c)) continue;
        if (c == '#') {
            while ((c = fgetc(s->input)) != EOF && c != '\n') { }
            if (c == '\n') {
                if (s->line == SIZE_MAX) return -1;
                ++s->line;
            } else if (ferror(s->input)) return -1;
            continue;
        }
        break;
    }
    s->token_line = s->line;
    for (;;) {
        if (c == 0 || used == SIZE_MAX - 1) return -1;
        if (used + 1 >= s->capacity) {
            size_t next = s->capacity ? s->capacity * 2 : 64;
            if (next <= s->capacity || next <= used + 1) return -1;
            char *new_token = realloc(s->token, next);
            if (!new_token) return -1;
            s->token = new_token;
            s->capacity = next;
        }
        s->token[used++] = (char)c;
        c = fgetc(s->input);
        if (c == EOF) {
            if (ferror(s->input)) return -1;
            break;
        }
        if (isspace((unsigned char)c) || c == '#') {
            if (c == '#') {
                while ((c = fgetc(s->input)) != EOF && c != '\n') { }
                if (c == EOF && ferror(s->input)) return -1;
            }
            if (c == '\n') {
                if (s->line == SIZE_MAX) return -1;
                ++s->line;
            }
            break;
        }
    }
    s->token[used] = '\0';
    return 1;
}

static int fail_token(const struct scanner *s, const char *message)
{
    fprintf(stderr, "trace line %zu: %s%s%s\n", s->token_line,
            message, s->token ? ": " : "", s->token ? s->token : "");
    return -1;
}

static int expect(struct scanner *s, const char *literal)
{
    int status = next_token(s);
    if (status != 1) {
        fprintf(stderr, "trace line %zu: expected %s%s\n", s->line,
                literal, status < 0 ? " (read error)" : " (end of file)");
        return -1;
    }
    return strcmp(s->token, literal) == 0 ? 0 :
           fail_token(s, "unexpected token");
}

static int number(struct scanner *s, double *value)
{
    int status = next_token(s);
    if (status != 1) {
        fprintf(stderr, "trace line %zu: expected numeric value%s\n",
                s->line, status < 0 ? " (read error)" : " (end of file)");
        return -1;
    }
    char *end;
    errno = 0;
    double parsed = strtod(s->token, &end);
    if (end == s->token || *end || errno == ERANGE || !isfinite(parsed) ||
        parsed > FLT_MAX || parsed < -FLT_MAX)
        return fail_token(s, "invalid finite float");
    *value = parsed;
    return 0;
}

static int integer_value(const struct scanner *s, double value,
                         long minimum, long maximum, long *out)
{
    if (trunc(value) != value || (long double)value < minimum ||
        (long double)value > maximum)
        return fail_token(s, "integer outside allowed range");
    *out = (long)value;
    return 0;
}

static int header_integer(struct scanner *s, const char *key,
                          long minimum, long maximum, long *out)
{
    double value;
    return expect(s, key) || number(s, &value) ||
           integer_value(s, value, minimum, maximum, out) ? -1 : 0;
}

static int header_float(struct scanner *s, const char *key, float *out)
{
    double value;
    if (expect(s, key) || number(s, &value)) return -1;
    if (value <= 0.0 || (float)value <= 0.0f)
        return fail_token(s, "expected positive float");
    *out = (float)value;
    return 0;
}

static int frame_integer(struct scanner *s, const double raw[FRAME_FIELDS],
                         size_t index, long minimum, long maximum, long *out)
{
    /* The index is always printed, even when the last scanned token was a
     * different field on another line. */
    if (trunc(raw[index]) != raw[index] ||
        (long double)raw[index] < minimum ||
        (long double)raw[index] > maximum) {
        fprintf(stderr, "trace frame field %zu: expected integer in [%ld,%ld]\n",
                index, minimum, maximum);
        return -1;
    }
    *out = (long)raw[index];
    (void)s;
    return 0;
}

static int map_frame(struct scanner *s, const double raw[FRAME_FIELDS],
                     long highest, struct reloq_core_voiced_frame *frame)
{
    static const int oral_center[8] = {9, 13, 15, 17, 19, 21, 23, 25};
    memset(frame, 0, sizeof *frame);
    frame->noise.duration_ms = (float)raw[0];
    frame->pitch_tenths = (float)raw[1];
    frame->open_percent = (float)raw[3];
    frame->flutter = (float)raw[5];
    frame->diplophonia = (float)raw[6];
    frame->noise.bypass_level = (float)raw[43];
    frame->noise.tract.highest_formant = highest;
    if (raw[0] <= 0.0 || (float)raw[0] <= 0.0f ||
        raw[5] < 0.0 || raw[6] < 0.0 || raw[6] > 100.0 ||
        frame_integer(s, raw, 2, 0, 400, &frame->av_level) ||
        frame_integer(s, raw, 4, 0, 100, &frame->glottal_profile) ||
        frame_integer(s, raw, 7, -400, 400, &frame->noise.ah_level) ||
        frame_integer(s, raw, 8, 0, 400, &frame->noise.af_level)) {
        fprintf(stderr, "trace: invalid duration/FL/DI or source level\n");
        return -1;
    }
    if(frame->glottal_profile>35)frame->glottal_profile=35;
    if (frame->av_level > 0 &&
        (frame->pitch_tenths <= 0.0f || frame->open_percent <= 0.0f ||
         frame->open_percent > 100.0f)) {
        fprintf(stderr, "trace: voiced F0 and OQ must be positive; OQ <= 100\n");
        return -1;
    }
    struct reloq_core_voiced_formants *tract = &frame->noise.tract;
    tract->nasal_pole = (struct reloq_core_formant){(float)raw[27], (float)raw[28]};
    tract->nasal_zero = (struct reloq_core_formant){(float)raw[29], (float)raw[30]};
    tract->tracheal_pole = (struct reloq_core_formant){(float)raw[31], (float)raw[32]};
    tract->tracheal_zero = (struct reloq_core_formant){(float)raw[33], (float)raw[34]};
    for (int i = 0; i < 8; ++i) {
        int c = oral_center[i];
        tract->oral[i] = (struct reloq_core_formant){
            (float)raw[c], (float)raw[c + 1]};
        frame->noise.af_bandwidths[i] = (float)raw[44 + i];
        if (frame_integer(s, raw, (size_t)(35 + i), 0, 400,
                          &frame->noise.af_levels[i])) return -1;
        if (raw[c] < 0.0 || raw[c + 1] < 0.0 || raw[44 + i] < 0.0) {
            fprintf(stderr, "trace: negative F%d frequency/bandwidth\n", i + 1);
            return -1;
        }
    }
    for (int i = 27; i <= 34; ++i)
        if (raw[i] < 0.0) {
            fprintf(stderr, "trace: negative nasal/tracheal frequency/bandwidth at %d\n", i);
            return -1;
        }
    return 0;
}

static void free_buffers(struct sample_buffers *b)
{
    free(b->output);
    free(b->raw_af);
    free(b->scratch);
    free(b->closed_mask);
    free(b->pcm);
    memset(b, 0, sizeof *b);
}

static int ensure_buffers(struct sample_buffers *b, size_t count)
{
    if (count <= b->capacity) return 0;
    if (count > SIZE_MAX / sizeof(float) ||
        count > SIZE_MAX / sizeof(int16_t)) return -1;
    struct sample_buffers next = {0};
    next.output = malloc(count * sizeof(float));
    next.raw_af = malloc(count * sizeof(float));
    next.scratch = malloc(count * sizeof(float));
    next.closed_mask = malloc(count);
    next.pcm = malloc(count * sizeof(int16_t));
    if (!next.output || !next.raw_af || !next.scratch ||
        !next.closed_mask || !next.pcm) {
        free_buffers(&next);
        return -1;
    }
    free_buffers(b);
    next.capacity = count;
    *b = next;
    return 0;
}

static void le16(unsigned char *at, uint16_t value)
{
    at[0] = (unsigned char)value;
    at[1] = (unsigned char)(value >> 8);
}

static void le32(unsigned char *at, uint32_t value)
{
    at[0] = (unsigned char)value;
    at[1] = (unsigned char)(value >> 8);
    at[2] = (unsigned char)(value >> 16);
    at[3] = (unsigned char)(value >> 24);
}

static int wav_header(FILE *out, uint32_t rate, uint32_t data_bytes)
{
    unsigned char h[WAV_HEADER_BYTES] = {0};
    memcpy(h, "RIFF", 4);
    le32(h + 4, 36u + data_bytes);
    memcpy(h + 8, "WAVEfmt ", 8);
    le32(h + 16, 16);  /* PCM fmt chunk */
    le16(h + 20, 1);   /* PCM */
    le16(h + 22, 1);   /* mono */
    le32(h + 24, rate);
    le32(h + 28, rate * 2);
    le16(h + 32, 2);
    le16(h + 34, 16);
    memcpy(h + 36, "data", 4);
    le32(h + 40, data_bytes);
    return fwrite(h, 1, sizeof h, out) == sizeof h ? 0 : -1;
}

static int write_samples(FILE *out, const int16_t *pcm, size_t count)
{
    unsigned char bytes[PCM_CHUNK * 2];
    for (size_t at = 0; at < count; ) {
        size_t n = count - at < PCM_CHUNK ? count - at : PCM_CHUNK;
        for (size_t i = 0; i < n; ++i)
            le16(bytes + 2 * i, (uint16_t)pcm[at + i]);
        if (fwrite(bytes, 2, n, out) != n) return -1;
        at += n;
    }
    return 0;
}

/* The original output path converts the unscaled float signal to int64,
 * scales that intermediate, and only then writes its low 16 bits. Scaling
 * already-wrapped PCM would lose information on louder frames. */
static int apply_output_gain(const float *output, int16_t *pcm,
                             size_t count, float gain)
{
    int64_t block[PCM_CHUNK];
    for (size_t at = 0; at < count; ) {
        size_t n = count - at < PCM_CHUNK ? count - at : PCM_CHUNK;
        for (size_t i = 0; i < n; ++i) {
            float sample = output[at + i];
            if (!isfinite(sample) || sample < -0x1p63f ||
                sample >= 0x1p63f) return -1;
            block[i] = (int64_t)sample;
            float scaled = (float)block[i] * gain;
            if (!isfinite(scaled) || scaled < -0x1p63f ||
                scaled >= 0x1p63f) return -1;
        }
        reloq_core_scale_output(block, (long)n, gain);
        reloq_core_pcm16_from_i64(block, pcm + at, n);
        at += n;
    }
    return 0;
}

static int copy_file(FILE *src, const char *destination)
{
    unsigned char block[65536];
    FILE *out = fopen(destination, "wb");
    if (!out) { perror(destination); return -1; }
    int result = 0;
    size_t n;
    while ((n = fread(block, 1, sizeof block, src)) != 0)
        if (fwrite(block, 1, n, out) != n) { result = -1; break; }
    if (ferror(src)) result = -1;
    if (fclose(out) != 0) result = -1;
    if (result) {
        fprintf(stderr, "could not write completed WAV: %s\n", destination);
        remove(destination);
    }
    return result;
}

static int run(const char *trace_path, const char *wav_path)
{
    FILE *input = fopen(trace_path, "rb");
    if (!input) { perror(trace_path); return -1; }
    FILE *temp = tmpfile();
    if (!temp) { perror("tmpfile"); fclose(input); return -1; }
    struct scanner scan = {.input = input, .line = 1, .token_line = 1};
    struct sample_buffers buffers = {0};
    struct reloq_core_voiced_frame_state state;
    size_t total = 0, frames = 0, reserved_nonzero = 0;
    long rate = 0, seed = 0, source = 0, av_trim = 0;
    long ah_trim = 0, af_trim = 0;
    long highest = 0;
    float duration_scale = 0.0f, output_gain = 0.0f;
    int result = -1;
    if (expect(&scan, "reloq-klatt-trace") || expect(&scan, "1") ||
        header_integer(&scan, "rate", 1, UINT32_MAX / 2, &rate) ||
        header_float(&scan, "duration_scale", &duration_scale) ||
        header_integer(&scan, "seed", 0, UINT16_MAX, &seed) ||
        header_integer(&scan, "source_offset", -400, 400, &source) ||
        header_integer(&scan, "av_trim", -400, 400, &av_trim) ||
        header_integer(&scan, "ah_trim", -400, 400, &ah_trim) ||
        header_integer(&scan, "af_trim", -400, 400, &af_trim) ||
        header_integer(&scan, "fhighest", 1, 8, &highest) ||
        header_float(&scan, "output_gain", &output_gain)) goto cleanup;
    if (rate != 8000 && rate != 11025 && rate != 16000 &&
        rate != 22050 && rate != 32000 && rate != 48000) {
        fprintf(stderr, "trace: unsupported rate %ld\n", rate);
        goto cleanup;
    }
    if (reloq_core_voiced_frame_initialize(
            &state, (float)rate, (int32_t)seed, source, ah_trim, af_trim) ||
        reloq_core_voiced_frame_set_av_trim(&state, av_trim) ||
        reloq_core_noise_frame_set_duration_scale(&state.noise,
                                                   duration_scale) ||
        wav_header(temp, (uint32_t)rate, 0)) {
        fprintf(stderr, "trace: unable to initialize renderer or WAV\n");
        goto cleanup;
    }
    for (;;) {
        int status = next_token(&scan);
        if (status == 0) break;
        if (status < 0) { fprintf(stderr, "trace: read error\n"); goto cleanup; }
        if (strcmp(scan.token, "frame") != 0) {
            fail_token(&scan, "expected frame or end of file");
            goto cleanup;
        }
        double raw[FRAME_FIELDS];
        for (size_t i = 0; i < FRAME_FIELDS; ++i) {
            if (number(&scan, &raw[i])) {
                fprintf(stderr, "trace: frame %zu field %zu\n", frames + 1, i);
                goto cleanup;
            }
            if ((i == 11 || i == 12 || i >= 52) &&
                raw[i] != 0.0 && reserved_nonzero < SIZE_MAX)
                ++reserved_nonzero;
        }
        struct reloq_core_voiced_frame frame;
        if (map_frame(&scan, raw, highest, &frame)) {
            fprintf(stderr, "trace: frame %zu\n", frames + 1);
            goto cleanup;
        }
        long count = reloq_core_frame_sample_count(
            frame.noise.duration_ms, duration_scale,
            state.noise.clock.samples_per_millisecond);
        if (count <= 0 || (uintmax_t)count >
            (UINT32_MAX - 36u) / 2u - total) {
            fprintf(stderr, "trace: frame %zu exceeds PCM/WAV capacity\n",
                    frames + 1);
            goto cleanup;
        }
        if (ensure_buffers(&buffers, (size_t)count)) {
            fprintf(stderr, "trace: frame %zu scratch allocation failed\n",
                    frames + 1);
            goto cleanup;
        }
        size_t written = 0;
        if (reloq_core_voiced_frame_render(
                &state, &frame, buffers.output, buffers.raw_af,
                buffers.scratch, buffers.closed_mask, buffers.pcm,
                buffers.capacity, &written)) {
            fprintf(stderr, "trace: frame %zu core renderer rejected parameters\n",
                    frames + 1);
            goto cleanup;
        }
        if (written != (size_t)count ||
            apply_output_gain(buffers.output, buffers.pcm, written,
                              output_gain)) {
            fprintf(stderr, "trace: frame %zu output sample count/range invalid\n",
                    frames + 1);
            goto cleanup;
        }
        if (write_samples(temp, buffers.pcm, written)) {
            fprintf(stderr, "trace: frame %zu temporary WAV write failed\n",
                    frames + 1);
            goto cleanup;
        }
        total += written;
        ++frames;
    }
    if (!frames) { fprintf(stderr, "trace: no frames\n"); goto cleanup; }
    if (fseek(temp, 0, SEEK_SET) ||
        wav_header(temp, (uint32_t)rate, (uint32_t)(total * 2)) ||
        fflush(temp) || fseek(temp, 0, SEEK_SET)) {
        fprintf(stderr, "trace: WAV header patch failed\n");
        goto cleanup;
    }
    if (fclose(input) != 0) {
        input = NULL;
        fprintf(stderr, "trace: close failed\n");
        goto cleanup;
    }
    input = NULL;
    if (copy_file(temp, wav_path)) goto cleanup;
    printf("%zu frames, %zu samples, %ld Hz -> %s\n",
           frames, total, rate, wav_path);
    if (reserved_nonzero)
        fprintf(stderr, "trace: %zu nonzero reserved field values ignored "
                        "(11,12,52..63)\n", reserved_nonzero);
    result = 0;
cleanup:
    free(scan.token);
    free_buffers(&buffers);
    if (input) fclose(input);
    if (fclose(temp) != 0) {
        fprintf(stderr, "trace: temporary WAV close failed\n");
        result = -1;
    }
    return result;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s trace.txt output.wav\n", argv[0]);
        return 2;
    }
    if (strcmp(argv[1], argv[2]) == 0) {
        fprintf(stderr, "input and output paths must differ\n");
        return 2;
    }
    return run(argv[1], argv[2]) ? 1 : 0;
}
