#include "reloq_german_generic_voice.h"
#include "reloq_german_lexicon_file.h"
#include "reloq_german_voice_defaults.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct capture {
    size_t samples, active, clipped, quiet_run, longest_quiet;
    size_t first_active;
    int peak;
    uint64_t energy, fingerprint;
    FILE *raw;
};

static int collect(void *ctx, const int16_t *pcm, size_t samples)
{
    struct capture *c = ctx;
    if (c->raw && fwrite(pcm, sizeof *pcm, samples, c->raw) != samples)
        return -1;
    for (size_t i = 0; i < samples; ++i) {
        int value = pcm[i] < 0 ? -(int)pcm[i] : pcm[i];
        c->energy += (uint64_t)value * (uint64_t)value;
        c->fingerprint = (c->fingerprint ^ (uint16_t)pcm[i]) *
                         UINT64_C(1099511628211);
        if (value > c->peak) c->peak = value;
        if (value >= 32760) ++c->clipped;
        if (value > 8) {
            if (!c->active) c->first_active = c->samples + i;
            ++c->active;
            if (c->quiet_run > c->longest_quiet)
                c->longest_quiet = c->quiet_run;
            c->quiet_run = 0;
        } else ++c->quiet_run;
    }
    c->samples += samples;
    return 0;
}

static struct capture render_controlled(
    struct reloq_session *session, const char *annotation,
    const struct reloq_german_generic_settings *settings, int expected_status)
{
    struct capture c = {.fingerprint = UINT64_C(14695981039346656037)};
    struct reloq_german_utterance_info info = {0};
    assert(reloq_session_reset(session) == RELOQ_SESSION_OK);
    int status = reloq_german_generic_render_annotation_with_settings(
        (const unsigned char *)annotation, strlen(annotation), settings,
        session, collect, &c, &info);
    assert(status == expected_status);
    if (!status) {
        assert(info.frames_completed == (size_t)info.planned_ticks);
        assert(c.samples > 0 && c.clipped == 0);
    } else assert(c.samples == 0);
    return c;
}

static void test_native_controls(struct reloq_session *session)
{
    const char *annotation = "`[.1pAk] `2 `[.1sAt].";
    const struct reloq_german_generic_settings original = {50, 50, 100, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings faster = {80, 50, 100, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings slower = {20, 50, 100, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings higher = {50, 80, 100, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings softer = {50, 50, 50, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings muted = {50, 50, 0, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings extreme_fast = {100, 100, 100, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings extreme_slow = {0, 0, 100, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings invalid = {101, 50, 100, 0, 0, 0,0,0,0,0,0};
    struct capture baseline = render_controlled(
        session, annotation, NULL, RELOQ_DE_UTTERANCE_OK);
    struct capture defaults = render_controlled(
        session, annotation, &original, RELOQ_DE_UTTERANCE_OK);
    struct capture quick = render_controlled(
        session, annotation, &faster, RELOQ_DE_UTTERANCE_OK);
    struct capture slow = render_controlled(
        session, annotation, &slower, RELOQ_DE_UTTERANCE_OK);
    struct capture high = render_controlled(
        session, annotation, &higher, RELOQ_DE_UTTERANCE_OK);
    struct capture soft = render_controlled(
        session, annotation, &softer, RELOQ_DE_UTTERANCE_OK);
    struct capture silent = render_controlled(
        session, annotation, &muted, RELOQ_DE_UTTERANCE_OK);
    struct capture fastest = render_controlled(
        session, annotation, &extreme_fast, RELOQ_DE_UTTERANCE_OK);
    struct capture slowest = render_controlled(
        session, annotation, &extreme_slow, RELOQ_DE_UTTERANCE_OK);
    (void)render_controlled(session, annotation, &invalid,
                            RELOQ_DE_UTTERANCE_INVALID);
    assert(baseline.samples == defaults.samples &&
           baseline.fingerprint == defaults.fingerprint);
    assert(quick.samples < baseline.samples &&
           slow.samples > baseline.samples &&
           quick.active > 0 && slow.active > 0);
    assert(fastest.samples < quick.samples &&
           slowest.samples > slow.samples &&
           fastest.active > 0 && slowest.active > 0);
    assert(high.samples == baseline.samples &&
           high.fingerprint != baseline.fingerprint && high.active > 0);
    assert(soft.samples == baseline.samples && soft.peak < baseline.peak &&
           soft.energy < baseline.energy && soft.active > 0);
    assert(silent.samples == baseline.samples && silent.peak == 0);
    printf("generic controls: samples slow/default/fast %zu/%zu/%zu, "
           "pitch changed PCM, peak default/half/mute %d/%d/%d\n",
           slow.samples, baseline.samples, quick.samples, baseline.peak,
           soft.peak, silent.peak);
}

static void test_boundary_duration(struct reloq_session *session)
{
    const char *joined = "`[.1pAk] `2 `[.1tIp].";
    const char *comma = "`[.1pAk], `[.1tIp].";
    const char *semicolon = "`[.1pAk]; `[.1tIp].";
    const char *sentences = "`[.1pAk]. `[.1tIp].";
    struct capture a = render_controlled(session, joined, NULL,
                                         RELOQ_DE_UTTERANCE_OK);
    struct capture b = render_controlled(session, comma, NULL,
                                         RELOQ_DE_UTTERANCE_OK);
    struct capture c = render_controlled(session, semicolon, NULL,
                                         RELOQ_DE_UTTERANCE_OK);
    struct capture d = render_controlled(session, sentences, NULL,
                                         RELOQ_DE_UTTERANCE_OK);
    /* A comma's explicit pause replaces the connected stop closure rather
     * than being added on top of it. The connected pair remains shorter;
     * punctuation tiers preserve their relative durations. */
    /* In connected k#t, k owns its 20-tick aspiration, followed by the
     * t body's 86 ticks and 43-tick release. After a real pause the t
     * intervals are 23 and 72; k no longer contributes a carried release. */
    /* A continuing comma retains the observed one-millisecond endpoint
     * before the next phone (approximately eleven PCM samples). */
    assert(b.samples > a.samples + 2417 && b.samples < a.samples + 2567);
    assert(c.samples > b.samples + 1000 && c.samples < b.samples + 1200);
    assert(d.samples > c.samples + 1600 && d.samples < c.samples + 1700);
}

static void test_terminal_duration(struct reloq_session *session)
{
    struct capture period = render_controlled(
        session, "`[.1pAk].", NULL, RELOQ_DE_UTTERANCE_OK);
    struct capture question = render_controlled(
        session, "`[.1pAk]?", NULL, RELOQ_DE_UTTERANCE_OK);
    struct capture semicolon = render_controlled(
        session, "`[.1pAk];", NULL, RELOQ_DE_UTTERANCE_OK);
    struct capture comma = render_controlled(
        session, "`[.1pAk],", NULL, RELOQ_DE_UTTERANCE_OK);
    /* These marks use the same word and phone durations. Their different
     * final windows are native PCM, so they also survive separate ECI jobs
     * where NVDA supplies no following text in this synthesis request. */
    assert(period.samples == question.samples);
    assert(period.samples > semicolon.samples + 1600 &&
           period.samples < semicolon.samples + 1700);
    assert(semicolon.samples > comma.samples + 1000 &&
           semicolon.samples < comma.samples + 1150);
    assert(period.quiet_run > 11025 / 3);
    assert(comma.quiet_run < 11025 / 4);
}

static void test_separate_jobs_match_internal_punctuation(
    struct reloq_session *session)
{
    struct capture next = render_controlled(
        session, "`[.1tIp].", NULL, RELOQ_DE_UTTERANCE_OK);
    /* NVDA's consecutive synth requests must retain the same full-sentence
     * pause already present inside one synthesis job, without inserting a
     * driver delay or changing the mid-phrase punctuation rule. Check
     * source-release classes and three different punctuation durations. */
    static const char *const final_phones[] = {
        "A", "As", "Ak", "An", "Al"
    };
    static const char *const marks[] = {".", "?", ",", ";"};
    for (size_t p = 0; p < sizeof final_phones / sizeof final_phones[0]; ++p) {
        for (size_t m = 0; m < sizeof marks / sizeof marks[0]; ++m) {
            char first_text[64], joined_text[96];
            int n = snprintf(first_text, sizeof first_text,
                             "`[.1%s]%s", final_phones[p], marks[m]);
            int j = snprintf(joined_text, sizeof joined_text,
                             "`[.1%s]%s `[.1tIp].",
                             final_phones[p], marks[m]);
            assert(n > 0 && (size_t)n < sizeof first_text);
            assert(j > 0 && (size_t)j < sizeof joined_text);
            struct capture first = render_controlled(
                session, first_text, NULL, RELOQ_DE_UTTERANCE_OK);
            struct capture one_job = render_controlled(
                session, joined_text, NULL, RELOQ_DE_UTTERANCE_OK);
            size_t separated_quiet = first.quiet_run + next.first_active;
            assert(separated_quiet > 11025 / 10);
            assert(one_job.longest_quiet > 11025 / 10);
            assert(separated_quiet >= one_job.longest_quiet - 110 &&
                   separated_quiet <= one_job.longest_quiet + 110);
        }
    }
}

static void test_phrase(struct reloq_session *session,
                        const char *text, const char *raw_name)
{
    struct capture capture = {0};
    struct reloq_german_utterance_info info;
    if (raw_name) capture.raw = fopen(raw_name, "wb");
    assert(!raw_name || capture.raw);
    assert(reloq_session_reset(session) == RELOQ_SESSION_OK);
    int result = reloq_german_generic_render(
        (const unsigned char *)text, strlen(text), NULL, session,
        collect, &capture, &info);
    if (capture.raw) assert(fclose(capture.raw) == 0);
    if (result) fprintf(stderr, "generic render %s: %d (byte %zu)\n",
                        text, result, info.error_offset);
    assert(result == RELOQ_DE_UTTERANCE_OK);
    assert(info.phone_count > 3);
    assert(info.planned_ticks > 350);
    assert(info.frames_completed == (size_t)info.planned_ticks);
    assert(capture.samples > 3000);
    assert(capture.active > 500);
    assert(capture.peak > 100);
    assert(capture.clipped == 0);
    assert(capture.quiet_run > 11025 / 3);
    assert(capture.longest_quiet < 11025 / 4);
    printf("%s: %zu phones, %zu samples, peak=%d, active=%zu, "
           "longest internal quiet=%zu samples\n", text, info.phone_count,
           capture.samples, capture.peak, capture.active,
           capture.longest_quiet);
}

static void test_complete_phone_inventory(struct reloq_session *session)
{
    static const char *const vowels[] = {
        "a", "A", "e", "E", "i", "I", "o", "O", "u", "U",
        "y", "Y", "@", "'E:'", "'oe'", "'OE'", "'aj'", "'aw'",
        "'oj'", "'a~'", "'E~'", "'o~'", "'oe~'"
    };
    static const char *const consonants[] = {
        "b", "C", "d", "f", "g", "G", "h", "j", "J", "k",
        "l", "m", "n", "p", "P", "r", "R", "s", "S", "t",
        "T", "v", "w", "x", "X", "z", "Z"
    };
    char annotation[64];
    for (size_t cls = 0; cls < 2; ++cls) {
        const char *const *symbols = cls ? consonants : vowels;
        size_t count = cls ? sizeof consonants / sizeof consonants[0] :
                             sizeof vowels / sizeof vowels[0];
        for (size_t i = 0; i < count; ++i) {
            int n = snprintf(annotation, sizeof annotation,
                             cls ? "`[.1%sAt]." : "`[.1p%st].", symbols[i]);
            assert(n > 0 && (size_t)n < sizeof annotation);
            assert(reloq_session_reset(session) == RELOQ_SESSION_OK);
            struct capture capture = {0};
            struct reloq_german_utterance_info info;
            int result = reloq_german_generic_render(
                (const unsigned char *)annotation, (size_t)n, NULL,
                session, collect, &capture, &info);
            if (result) fprintf(stderr, "phone %s: %d\n", annotation, result);
            assert(result == RELOQ_DE_UTTERANCE_OK);
            assert(capture.samples > 1000 && capture.active > 100);
            assert(capture.clipped == 0);
        }
    }
}

int main(int argc, char **argv)
{
    float controls[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    assert(!reloq_german_voice_defaults(controls, fields, &config));
    struct reloq_session *session = reloq_session_create(&config);
    assert(session);
    if (argc >= 4 && !strcmp(argv[1], "--probe")) {
        struct reloq_german_lexicon_file lexicon = {0};
        if (strcmp(argv[2], "-") &&
            reloq_german_lexicon_file_load_path(&lexicon, argv[2]))
            return 2;
        struct capture capture = {0};
        struct reloq_german_utterance_info info = {0};
        int status = reloq_german_generic_render(
            (const unsigned char *)argv[3], strlen(argv[3]),
            lexicon.rows ? &lexicon.view : NULL, session,
            collect, &capture, &info);
        printf("status=%d byte=%zu phones=%zu frames=%zu samples=%zu "
               "peak=%d clipped=%zu active=%zu longestQuiet=%zu\n",
               status, info.error_offset, info.phone_count,
               info.frames_completed, capture.samples, capture.peak,
               capture.clipped, capture.active, capture.longest_quiet);
        reloq_german_lexicon_file_clear(&lexicon);
        reloq_session_destroy(session);
        return status == RELOQ_DE_UTTERANCE_OK ? 0 : 1;
    }
    test_phrase(session, "`[.0pAk.1tIp].", argc > 1 ? argv[1] : NULL);
    test_phrase(session, "Paket.", argc > 2 ? argv[2] : NULL);
    test_phrase(session, " `[.0pAk.1tIp].", NULL);
    /* The current natural-text frontend does not know this word yet. The
     * explicit SPR still exercises three syllables, weak secondary stress,
     * and an internal t+d boundary without a lexical word exception. */
    test_phrase(session, "`[.1bEt.2dE.0k@].", NULL);
    test_phrase(session, "Pack Tipp.", NULL);
    test_complete_phone_inventory(session);
    test_boundary_duration(session);
    test_terminal_duration(session);
    test_separate_jobs_match_internal_punctuation(session);
    test_native_controls(session);
    reloq_session_destroy(session);
    return 0;
}
