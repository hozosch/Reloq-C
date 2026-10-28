#include "reloq_german_voice.h"
#include "reloq_german_voice_defaults.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

struct capture {
    size_t samples;
    size_t nonzero;
    size_t frames;
    uint64_t checksum;
    struct reloq_session *cancel_session;
    size_t cancel_at;
    int reject;
};

struct cancel_probe {
    size_t polls;
    size_t stop_at;
};

static int stop_preflight(void *context)
{
    struct cancel_probe *probe = context;
    return ++probe->polls >= probe->stop_at;
}

static int receive(void *context, const int16_t *pcm, size_t count)
{
    struct capture *capture = context;
    if (capture->reject) return -1;
    for (size_t i = 0; i < count; ++i) {
        capture->checksum = capture->checksum * UINT64_C(1315423911) +
            (uint16_t)pcm[i];
        capture->nonzero += pcm[i] != 0;
    }
    capture->samples += count;
    ++capture->frames;
    if (capture->cancel_session && capture->frames == capture->cancel_at)
        reloq_session_cancel(capture->cancel_session);
    return 0;
}

static int discard_pcm(void *context, const int16_t *pcm, size_t count)
{
    (void)context;
    (void)pcm;
    (void)count;
    return 0;
}

int main(void)
{
    const unsigned char hack[] = "Hack.";
    const unsigned char hieb[] = "Hieb.";
    const unsigned char app[] = "App.";
    const unsigned char ass[] = "Ass.";
    const unsigned char hass[] = "Hass.";
    const unsigned char hass_question[] = "Hass?";
    const unsigned char hiss[] = "Hiss.";
    const unsigned char hiss_ellipsis[] = "Hiss...";
    const unsigned char ass_comma[] = "Ass,";
    const unsigned char ass_implicit[] = "Ass";
    const unsigned char pack[] = "Pack.";
    const unsigned char tipp[] = "Tipp.";
    const unsigned char two[] = "Pack. Tipp.";
    const unsigned char three[] = "Hack. Pack. Tipp.";
    const unsigned char bad_second[] = "Pack. Maus.";
    const unsigned char connected[] = "Pack Tipp.";
    const unsigned char connected_kp[] = "Pack Pack.";
    const unsigned char connected_kp_annotation[] =
        " `[.1pAk] `2 `[.1pAk].";
    const unsigned char connected_twice[] = "Pack Tipp. Hack Tipp.";
    const unsigned char connected_mixed[] =
        "Hiss! Pack Tipp. Hack Tipp.";
    const unsigned char connected_bad_later[] = "Pack Tipp. Maus.";
    const unsigned char connected_annotation[] =
        " `[.1pAk] `2 `[.1tIp].";
    const unsigned char reduced_stress[] = "Hat Tipp.";
    const unsigned char unmeasured_pair[] = "Pek Tep.";
    const unsigned char fricative_after_stop[] = "Pass.";
    const unsigned char stop_question[] = "Hack?";
    const unsigned char notation[] = "`[.1hAt].";
    const unsigned char compound[] = "`[.0pAp.1tIp].";
    const unsigned char compound_back[] = "`[.0kIk.1tIp].";
    const unsigned char compound_wrong_stress[] = "`[.1pAp.0tIp].";
    const unsigned char pappe[] = "Pappe.";
    const unsigned char packen[] = "Packen.";
    const unsigned char tappen[] = "Tappen.";
    const unsigned char pappe_spr[] = "`[.1pA.0p@].";
    const unsigned char closed_schwa[] = "`[.1pAp.0@].";
    const unsigned char closed_schwa_n[] = "`[.1pAk.0t@n].";
    const unsigned char closed_schwa_t[] = "`[.1pAk.0t@].";
    const unsigned char packte[] = "Packte.";
    const unsigned char packten[] = "Packten.";
    const unsigned char unknown[] = "Maus.";
    size_t bad = 987;
    CHECK(reloq_german_voice_supports(hack, sizeof hack - 1, &bad) == 1 &&
          bad == 0);
    CHECK(reloq_german_voice_supports(hieb, sizeof hieb - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(app, sizeof app - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(ass, sizeof ass - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(hass, sizeof hass - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(hass_question,
                                      sizeof hass_question - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(hiss, sizeof hiss - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(hiss_ellipsis,
                                      sizeof hiss_ellipsis - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(ass_comma,
                                      sizeof ass_comma - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(ass_implicit,
                                      sizeof ass_implicit - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(pack, sizeof pack - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(two, sizeof two - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(three, sizeof three - 1,
                                      &bad) == 1);
    CHECK(reloq_german_voice_supports(connected, sizeof connected - 1,
                                      &bad) == 1);
    CHECK(reloq_german_voice_supports(connected_kp,
              sizeof connected_kp - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(connected_kp_annotation,
              sizeof connected_kp_annotation - 1, &bad) == 1);
    size_t word_boundary = 999, sample_boundary = 999;
    CHECK(reloq_german_voice_connected_index_sample(
              connected, sizeof connected - 1,
              &word_boundary, &sample_boundary, NULL, NULL) == 1 &&
          word_boundary == 4 && sample_boundary == 2179);
    CHECK(reloq_german_voice_connected_index_sample(
              connected_kp, sizeof connected_kp - 1,
              &word_boundary, &sample_boundary, NULL, NULL) == 1 &&
          word_boundary == 4 && sample_boundary == 2179);
    CHECK(reloq_german_voice_connected_index_sample(
              connected_annotation, sizeof connected_annotation - 1,
              &word_boundary, &sample_boundary, NULL, NULL) == 0 &&
          word_boundary == 4 && sample_boundary == 2179);
    CHECK(reloq_german_voice_connected_index_sample(
              pack, sizeof pack - 1,
              &word_boundary, &sample_boundary, NULL, NULL) == 0 &&
          word_boundary == 4 && sample_boundary == 2179);
    CHECK(reloq_german_voice_supports(connected_twice,
              sizeof connected_twice - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(connected_mixed,
              sizeof connected_mixed - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(connected_bad_later,
              sizeof connected_bad_later - 1, &bad) == 0 && bad == 11);
    CHECK(reloq_german_voice_supports(connected_annotation,
              sizeof connected_annotation - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(reduced_stress,
              sizeof reduced_stress - 1, &bad) == 0);
    CHECK(reloq_german_voice_supports(unmeasured_pair,
              sizeof unmeasured_pair - 1, &bad) == 0);
    CHECK(reloq_german_voice_supports(fricative_after_stop,
              sizeof fricative_after_stop - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(stop_question,
              sizeof stop_question - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(bad_second, sizeof bad_second - 1,
                                      &bad) == 0 && bad == 6);
    CHECK(reloq_german_voice_supports(notation, sizeof notation - 1,
                                      &bad) == 1);
    CHECK(reloq_german_voice_supports(compound,
              sizeof compound - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(compound_back,
              sizeof compound_back - 1, &bad) == 1);
    CHECK(reloq_german_voice_supports(compound_wrong_stress,
              sizeof compound_wrong_stress - 1, &bad) == 0);
    CHECK(reloq_german_voice_supports(unknown, sizeof unknown - 1,
                                      &bad) == 0);
    CHECK(reloq_german_voice_supports(NULL, 0, &bad) == -1 && bad == 0);

    struct reloq_german_voice_span spans[3] = {{0}};
    size_t span_count = 0;
    CHECK(reloq_german_voice_sentence_spans(two, sizeof two - 1,
              NULL, 0, &span_count, &bad) == -2 && span_count == 2);
    CHECK(reloq_german_voice_sentence_spans(two, sizeof two - 1,
              spans, 1, &span_count, &bad) == -2 && span_count == 2);
    CHECK(reloq_german_voice_sentence_spans(two, sizeof two - 1,
              spans, 2, &span_count, &bad) == 1 && span_count == 2 &&
          spans[0].offset == 0 && spans[0].length == 5 &&
          spans[1].offset == 6 && spans[1].length == 5);
    CHECK(reloq_german_voice_sentence_spans(three, sizeof three - 1,
              spans, 3, &span_count, &bad) == 1 && span_count == 3 &&
          spans[0].offset == 0 && spans[1].offset == 6 &&
          spans[2].offset == 12);
    CHECK(reloq_german_voice_sentence_spans(connected,
              sizeof connected - 1, spans, 1, &span_count, &bad) == 1 &&
          span_count == 1 && spans[0].offset == 0 &&
          spans[0].length == sizeof connected - 1);
    spans[0].length = 4321;
    span_count = 1234;
    CHECK(reloq_german_voice_sentence_spans(bad_second,
              sizeof bad_second - 1, spans, 3, &span_count, &bad) == 0 &&
          bad == 6 && span_count == 1234 && spans[0].length == 4321);
    struct cancel_probe stop = {.stop_at = 5};
    CHECK(reloq_german_voice_sentence_spans_checked(
              three, sizeof three - 1, spans, 3, &span_count, &bad,
              stop_preflight, &stop) == -3 && stop.polls >= 5 &&
          span_count == 1234 && spans[0].length == 4321);

    float controls[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    CHECK(reloq_german_voice_defaults(controls, fields, &config) == 0);
    struct reloq_session *session = reloq_session_create(&config);
    CHECK(session);
    struct reloq_german_utterance_info info;
    struct capture a = {0};
    CHECK(reloq_german_voice_render(hack, sizeof hack - 1, session,
                                    receive, &a, &info) ==
          RELOQ_DE_UTTERANCE_OK);
    CHECK(info.phone_count == 3 && info.target_count > 100 &&
          info.planned_ticks == 759 && info.frames_completed == 759 &&
          a.frames == 759 && a.samples > 8000 && a.nonzero > 1000);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture replay = {0};
    CHECK(reloq_german_voice_render(hack, sizeof hack - 1, session,
                                    receive, &replay, &info) ==
          RELOQ_DE_UTTERANCE_OK);
    CHECK(replay.samples == a.samples && replay.nonzero == a.nonzero &&
          replay.checksum == a.checksum);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture empty = {0};
    struct capture compound_audio = {0};
    CHECK(reloq_german_voice_render(compound,
              sizeof compound - 1, session,
              receive, &compound_audio, &info) == RELOQ_DE_UTTERANCE_OK &&
          info.phone_count == 6 && info.planned_ticks == 1017 &&
          info.frames_completed == 1017 &&
          compound_audio.samples == 11187 && compound_audio.nonzero > 1000);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture compound_other = {0};
    CHECK(reloq_german_voice_render(compound_back,
              sizeof compound_back - 1, session,
              receive, &compound_other, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 6 &&
          info.planned_ticks == 1016 && compound_other.samples == 11176);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture compound_rejected = {0};
    CHECK(reloq_german_voice_render(compound_wrong_stress,
              sizeof compound_wrong_stress - 1, session,
              receive, &compound_rejected, &info) ==
          RELOQ_DE_UTTERANCE_UNCOVERED && compound_rejected.samples == 0);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture schwa = {0};
    CHECK(reloq_german_voice_render(pappe,sizeof pappe - 1,session,
              receive,&schwa,&info) == RELOQ_DE_UTTERANCE_OK &&
          info.phone_count == 4 && info.planned_ticks == 799 &&
          schwa.samples == 8789 && schwa.nonzero > 1000);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture schwa_spr = {0};
    CHECK(reloq_german_voice_render(pappe_spr,sizeof pappe_spr - 1,session,
              receive,&schwa_spr,&info) == RELOQ_DE_UTTERANCE_OK &&
          schwa_spr.samples == schwa.samples &&
          schwa_spr.checksum == schwa.checksum);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    schwa = (struct capture){0};
    CHECK(reloq_german_voice_render(packen,sizeof packen - 1,session,
              receive,&schwa,&info) == RELOQ_DE_UTTERANCE_OK &&
          info.phone_count == 5 && info.planned_ticks == 870 &&
          schwa.samples == 9570);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    schwa = (struct capture){0};
    CHECK(reloq_german_voice_render(tappen,sizeof tappen - 1,session,
              receive,&schwa,&info) == RELOQ_DE_UTTERANCE_OK &&
          info.phone_count == 5 && info.planned_ticks == 881 &&
          schwa.samples == 9691);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    schwa = (struct capture){0};
    CHECK(reloq_german_voice_render(closed_schwa,
              sizeof closed_schwa - 1,session,receive,&schwa,&info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 4 &&
          info.planned_ticks == 789 && schwa.samples == 8679);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    schwa = (struct capture){0};
    CHECK(reloq_german_voice_render(closed_schwa_n,
              sizeof closed_schwa_n - 1,session,receive,&schwa,&info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 6 &&
          info.planned_ticks == 930 && schwa.samples == 10230);
    struct capture closed_n_reference = schwa;
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    schwa = (struct capture){0};
    CHECK(reloq_german_voice_render(packten,sizeof packten - 1,session,
              receive,&schwa,&info) == RELOQ_DE_UTTERANCE_OK &&
          info.phone_count == 6 && info.planned_ticks == 930 &&
          schwa.samples == closed_n_reference.samples &&
          schwa.checksum == closed_n_reference.checksum);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    schwa = (struct capture){0};
    CHECK(reloq_german_voice_render(closed_schwa_t,
              sizeof closed_schwa_t - 1,session,receive,&schwa,&info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 5 &&
          info.planned_ticks == 872 && schwa.samples == 9592);
    struct capture closed_t_reference = schwa;
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    schwa = (struct capture){0};
    CHECK(reloq_german_voice_render(packte,sizeof packte - 1,session,
              receive,&schwa,&info) == RELOQ_DE_UTTERANCE_OK &&
          info.phone_count == 5 && info.planned_ticks == 872 &&
          schwa.samples == closed_t_reference.samples &&
          schwa.checksum == closed_t_reference.checksum);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    CHECK(reloq_german_voice_render(unknown, sizeof unknown - 1, session,
                                    receive, &empty, &info) ==
          RELOQ_DE_UTTERANCE_UNCOVERED && !empty.frames);
    CHECK(reloq_german_voice_render(hieb, sizeof hieb - 1, session,
                                    receive, &empty, &info) ==
          RELOQ_DE_UTTERANCE_OK && empty.nonzero > 0);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture bare = {0};
    CHECK(reloq_german_voice_render(app, sizeof app - 1, session,
                                    receive, &bare, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 2 &&
          info.planned_ticks == 682 && info.frames_completed == 682 &&
          bare.nonzero > 1000);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture fricative = {0};
    CHECK(reloq_german_voice_render(ass, sizeof ass - 1, session,
                                    receive, &fricative, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 2 &&
          info.planned_ticks == 692 && fricative.samples == 7612);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    fricative = (struct capture){0};
    CHECK(reloq_german_voice_render(hass, sizeof hass - 1, session,
                                    receive, &fricative, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 3 &&
          info.planned_ticks == 772 && fricative.samples == 8492);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    fricative = (struct capture){0};
    CHECK(reloq_german_voice_render(hiss, sizeof hiss - 1, session,
                                    receive, &fricative, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 3 &&
          info.planned_ticks == 772 && fricative.samples == 8492);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    fricative = (struct capture){0};
    CHECK(reloq_german_voice_render(hass_question,
                                    sizeof hass_question - 1, session,
                                    receive, &fricative, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.planned_ticks == 772 &&
          fricative.samples == 8492);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    fricative = (struct capture){0};
    CHECK(reloq_german_voice_render(ass_comma, sizeof ass_comma - 1,
                                    session, receive, &fricative, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.planned_ticks == 442 &&
          fricative.samples == 4862);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    fricative = (struct capture){0};
    CHECK(reloq_german_voice_render(hiss_ellipsis,
                                    sizeof hiss_ellipsis - 1, session,
                                    receive, &fricative, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.planned_ticks == 522 &&
          fricative.samples == 5742);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture initial_stop = {0};
    CHECK(reloq_german_voice_render(pack, sizeof pack - 1, session,
                                    receive, &initial_stop, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 3 &&
          info.planned_ticks == 734 && info.frames_completed == 734 &&
          initial_stop.nonzero > 1000);

    /* Each natural voiced-stop CVC must take the same complete render path
     * as its independently observed Apple SPR replay. */
    static const struct {
        const char *written, *spr;
        size_t frames;
    } voiced_words[] = {
        {"Bett.", "`[.1bEt].", 766},
        {"Deck.", "`[.1dEk].", 787},
        {"Guck.", "`[.1gUk].", 756},
        {"Bad.", "`[.1bat].", 817}
    };
    for (size_t i = 0; i < sizeof voiced_words / sizeof voiced_words[0];
         ++i) {
        const size_t written_length = strlen(voiced_words[i].written);
        const size_t spr_length = strlen(voiced_words[i].spr);
        CHECK(reloq_german_voice_supports(
            (const unsigned char *)voiced_words[i].written,
            written_length, &bad) == 1);
        CHECK(reloq_german_voice_supports(
            (const unsigned char *)voiced_words[i].spr,
            spr_length, &bad) == 1);
        CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
        struct capture natural = {0};
        CHECK(reloq_german_voice_render(
            (const unsigned char *)voiced_words[i].written,
            written_length, session, receive, &natural, &info) ==
            RELOQ_DE_UTTERANCE_OK && info.phone_count == 3 &&
            info.frames_completed == voiced_words[i].frames &&
            natural.samples == voiced_words[i].frames * 11 &&
            natural.nonzero > 1000);
        CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
        struct capture phonetic = {0};
        CHECK(reloq_german_voice_render(
            (const unsigned char *)voiced_words[i].spr, spr_length,
            session, receive, &phonetic, &info) ==
            RELOQ_DE_UTTERANCE_OK &&
            phonetic.samples == natural.samples &&
            phonetic.checksum == natural.checksum);
    }

    /* Apple public natural-text callbacks for these seven spellings are
     * exactly the corresponding isolated, primary-stressed SPR groups.
     * Native text and explicit SPR must both reach the measured PCM path. */
    static const struct {
        const char *written, *spr;
        size_t frames;
    } nasal_words[] = {
        {"Mett.", "`[.1mEt].", 758},
        {"Muck.", "`[.1mUk].", 751},
        {"Mut.", "`[.1mut].", 821},
        {"Nett.", "`[.1nEt].", 743},
        {"Nipp.", "`[.1nIp].", 753},
        {"Nock.", "`[.1nOk].", 750},
        {"Not.", "`[.1not].", 820}
    };
    for (size_t i = 0; i < sizeof nasal_words / sizeof nasal_words[0];
         ++i) {
        const size_t natural_length = strlen(nasal_words[i].written);
        const size_t spr_length = strlen(nasal_words[i].spr);
        CHECK(reloq_german_voice_supports(
            (const unsigned char *)nasal_words[i].written,
            natural_length, &bad) == 1 && bad == 0);
        CHECK(reloq_german_voice_supports(
            (const unsigned char *)nasal_words[i].spr,
            spr_length, &bad) == 1 && bad == 0);
        CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
        struct capture natural = {0};
        CHECK(reloq_german_voice_render(
            (const unsigned char *)nasal_words[i].written,
            natural_length, session, receive, &natural, &info) ==
            RELOQ_DE_UTTERANCE_OK && info.phone_count == 3 &&
            info.annotation_bytes == spr_length &&
            info.planned_ticks == (int64_t)nasal_words[i].frames &&
            info.frames_completed == nasal_words[i].frames &&
            natural.samples == nasal_words[i].frames * 11 &&
            natural.nonzero > 1000);
        CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
        struct capture phonetic = {0};
        CHECK(reloq_german_voice_render(
            (const unsigned char *)nasal_words[i].spr,
            spr_length, session, receive, &phonetic, &info) ==
            RELOQ_DE_UTTERANCE_OK &&
            phonetic.samples == natural.samples &&
            phonetic.checksum == natural.checksum);
    }

    /* An isolated nasal onset does not license a medial unstressed nasal:
     * its onset, duration and prosody are unmeasured VCV contexts. */
    const char *nasal_vcv[] = {
        "`[.1A.0mAp].", "`[.1A.0nAt]."
    };
    for (size_t i = 0; i < sizeof nasal_vcv / sizeof nasal_vcv[0]; ++i) {
        const size_t length = strlen(nasal_vcv[i]);
        CHECK(reloq_german_voice_supports(
            (const unsigned char *)nasal_vcv[i], length, &bad) == 0);
        CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
        struct capture rejected_nasal_vcv = {0};
        CHECK(reloq_german_voice_render(
            (const unsigned char *)nasal_vcv[i], length, session,
            receive, &rejected_nasal_vcv, &info) ==
            RELOQ_DE_UTTERANCE_UNCOVERED &&
            rejected_nasal_vcv.frames == 0 &&
            rejected_nasal_vcv.samples == 0);
    }

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture connected_audio = {0};
    CHECK(reloq_german_voice_render(connected, sizeof connected - 1,
                                    session, receive, &connected_audio,
                                    &info) == RELOQ_DE_UTTERANCE_OK &&
          info.phone_count == 6 && info.target_count > 300 &&
          info.annotation_bytes ==
              sizeof connected_annotation - 1 &&
          info.planned_ticks == 1020 && info.frames_completed == 1020 &&
          connected_audio.frames == 1020 &&
          connected_audio.samples == 11220 &&
          connected_audio.nonzero > 1000);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture connected_phonetic = {0};
    CHECK(reloq_german_voice_render(connected_annotation,
                                    sizeof connected_annotation - 1,
                                    session, receive, &connected_phonetic,
                                    &info) == RELOQ_DE_UTTERANCE_OK &&
          connected_phonetic.samples == connected_audio.samples &&
          connected_phonetic.checksum == connected_audio.checksum);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture labial = {0};
    CHECK(reloq_german_voice_render(connected_kp, sizeof connected_kp - 1,
                                    session, receive, &labial, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.frames_completed == 1018 &&
          labial.samples == 11198);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture labial_explicit = {0};
    CHECK(reloq_german_voice_render(connected_kp_annotation,
                                    sizeof connected_kp_annotation - 1,
                                    session, receive, &labial_explicit,
                                    &info) == RELOQ_DE_UTTERANCE_OK &&
          labial_explicit.samples == labial.samples &&
          labial_explicit.checksum == labial.checksum);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture connected_sequence = {0};
    CHECK(reloq_german_voice_render(connected_twice,
                                    sizeof connected_twice - 1,
                                    session, receive, &connected_sequence,
                                    &info) == RELOQ_DE_UTTERANCE_OK &&
          info.frames_completed == 2064 &&
          connected_sequence.samples == 22704);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture connected_rejected_later = {0};
    CHECK(reloq_german_voice_render(connected_bad_later,
                                    sizeof connected_bad_later - 1,
                                    session, receive, &connected_rejected_later,
                                    &info) == RELOQ_DE_UTTERANCE_UNCOVERED &&
          info.error_offset == 11 && !connected_rejected_later.frames);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture rejected_pair = {0};
    CHECK(reloq_german_voice_render(reduced_stress,
                                    sizeof reduced_stress - 1, session,
                                    receive, &rejected_pair, &info) ==
          RELOQ_DE_UTTERANCE_UNCOVERED && !rejected_pair.frames);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture joint = {0};
    CHECK(reloq_german_voice_render(two, sizeof two - 1, session,
                                    receive, &joint, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 6 &&
          info.planned_ticks == 1502 && info.frames_completed == 1502 &&
          joint.frames == 1502 && joint.samples == 16522);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture explicit_sentences = {0};
    CHECK(reloq_german_voice_render(pack, sizeof pack - 1, session,
                                    receive, &explicit_sentences, &info) ==
          RELOQ_DE_UTTERANCE_OK);
    CHECK(reloq_german_voice_render(tipp, sizeof tipp - 1, session,
                                    receive, &explicit_sentences, &info) ==
          RELOQ_DE_UTTERANCE_OK);
    CHECK(explicit_sentences.samples == joint.samples &&
          explicit_sentences.checksum == joint.checksum);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture all_three = {0};
    CHECK(reloq_german_voice_render(three, sizeof three - 1, session,
                                    receive, &all_three, &info) ==
          RELOQ_DE_UTTERANCE_OK && info.phone_count == 9 &&
          info.planned_ticks == 2261 && info.frames_completed == 2261 &&
          all_three.frames == 2261 && all_three.samples == 24871);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture atomic = {0};
    CHECK(reloq_german_voice_render(bad_second,
                                    sizeof bad_second - 1, session,
                                    receive, &atomic, &info) ==
          RELOQ_DE_UTTERANCE_UNCOVERED && !atomic.frames &&
          info.error_offset == 6);

    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture cancel = {.cancel_session = session, .cancel_at = 20};
    CHECK(reloq_german_voice_render(notation, sizeof notation - 1, session,
                                    receive, &cancel, &info) ==
          RELOQ_DE_UTTERANCE_CANCELLED && info.frames_completed == 20 &&
          cancel.frames == 20);
    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
    struct capture rejected = {.reject = 1};
    CHECK(reloq_german_voice_render(notation, sizeof notation - 1, session,
                                    receive, &rejected, &info) ==
          RELOQ_DE_UTTERANCE_SINK && !info.frames_completed);

    /* Every structurally accepted k→t phonetic context must reach PCM,
     * including long vowels and all initial/coda places. A support result
     * followed by MODEL would strand an ECI request after preflight. */
    const char *initials[] = {"", "h", "p", "t", "k"};
    const char *vowels = "AEIOUaeiou";
    const char *codas = "ptk";
    char pair[64];
    for (size_t a = 0; a < 5; ++a)
        for (size_t v = 0; v < 10; ++v)
            for (size_t w = 0; w < 10; ++w)
                for (size_t c = 0; c < 3; ++c) {
                    int length = snprintf(pair, sizeof pair,
                        "`[.1%s%ck] `2 `[.1t%c%c].", initials[a],
                        vowels[v], vowels[w], codas[c]);
                    CHECK(length > 0 && (size_t)length < sizeof pair);
                    CHECK(reloq_german_voice_supports(
                        (const unsigned char *)pair, (size_t)length,
                        &bad) == 1);
                    CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
                    CHECK(reloq_german_voice_render(
                        (const unsigned char *)pair, (size_t)length,
                        session, discard_pcm, NULL, &info) ==
                        RELOQ_DE_UTTERANCE_OK &&
                        info.frames_completed > 0 &&
                        (int64_t)info.frames_completed ==
                        info.planned_ticks);
                }
    /* Acoustic support must be symmetric with preflight for the newly
     * covered labial onset over the entire vowel/coda matrix. */
    for (size_t v = 0; v < 10; ++v)
        for (size_t w = 0; w < 10; ++w)
            for (size_t c = 0; c < 3; ++c) {
                int length = snprintf(pair, sizeof pair,
                    "`[.1p%ck] `2 `[.1p%c%c].", vowels[v],
                    vowels[w], codas[c]);
                CHECK(length > 0 && (size_t)length < sizeof pair);
                CHECK(reloq_german_voice_supports(
                    (const unsigned char *)pair, (size_t)length, &bad) == 1);
                CHECK(reloq_session_reset(session) == RELOQ_SESSION_OK);
                CHECK(reloq_german_voice_render(
                    (const unsigned char *)pair, (size_t)length,
                    session, discard_pcm, NULL, &info) ==
                    RELOQ_DE_UTTERANCE_OK &&
                    (int64_t)info.frames_completed == info.planned_ticks);
            }
    reloq_session_destroy(session);
    return 0;
}
