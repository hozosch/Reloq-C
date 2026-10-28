#include "reloq_german_voice.h"

#include "reloq_curve_pipeline.h"
#include "reloq_curve_scheduler.h"
#include "reloq_german_hv_timing.h"
#include "reloq_german_connected_pair.h"
#include "reloq_german_boundary_profile.h"
#include "reloq_german_connected_words.h"
#include "reloq_german_connected_cvc_index.h"
#include "reloq_german_initial_h.h"
#include "reloq_german_initial_stop.h"
#include "reloq_german_initial_voiced_stop.h"
#include "reloq_german_initial_voiced_stop_envelope.h"
#include "reloq_german_initial_nasal.h"
#include "reloq_german_initial_nasal_envelope.h"
#include "reloq_german_fricative_vowel.h"
#include "reloq_german_stop_vowel.h"
#include "reloq_german_stop_fricative_vowel.h"
#include "reloq_german_voiced_stop_vowel.h"
#include "reloq_german_voiced_stop_tract.h"
#include "reloq_german_nasal_vowel.h"
#include "reloq_german_phrase_f0.h"
#include "reloq_german_phone_sequence.h"
#include "reloq_german_syllable_plan.h"
#include "reloq_german_reduced_closed_composer.h"
#include "reloq_german_reduced_open_composer.h"
#include "reloq_german_within_word_composer.h"
#include "reloq_german_sentence_boundary.h"
#include "reloq_german_terminal_source.h"
#include "reloq_german_terminal_fricative.h"
#include "reloq_german_terminal_stop_tract.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_german_text.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_german_vowel_nucleus.h"
#include "reloq_target_stage_merge.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Separate gesture builders allow a vowel nucleus and a final-stop tract to
 * be authored independently, then stably merged before curve evaluation.
 * Same-tick order within a gesture must be retained (especially AV/AH). */
enum { VOICE_STAGES = 12, STAGE_POINTS = 64,
       FINAL_POINTS = VOICE_STAGES * STAGE_POINTS };
enum { INITIAL_H, VOWEL, VOWEL_TRACT, STOP_TRACT, STOP_BURST,
       RELEASE, PHRASE_F0 };
enum { PAIR_FIRST_INITIAL, PAIR_FIRST_VOWEL, PAIR_FIRST_TRACT,
       PAIR_FIRST_CODA, PAIR_SECOND_ONSET, PAIR_SECOND_VOWEL,
       PAIR_SECOND_TRACT, PAIR_SECOND_CODA_TRACT, PAIR_SECOND_BURST,
       PAIR_SECOND_RELEASE, PAIR_F0 };

struct gesture_stage {
    struct reloq_target_builder builder;
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][STAGE_POINTS];
};

struct composition {
    struct gesture_stage stage[VOICE_STAGES];
    struct reloq_target_builder combined;
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][FINAL_POINTS];
};

struct prepared {
    struct reloq_german_phone_node phones[3];
    size_t phone_count;
    size_t vowel_index;
    size_t annotation_bytes;
    size_t bad_offset;
    enum reloq_german_terminal_mark mark;
};

struct prepared_within_word {
    struct reloq_german_phone_node phones[6];
    struct reloq_german_syllable syllables[2];
    size_t phone_count;
    uint8_t model;
    size_t annotation_bytes;
    size_t bad_offset;
};

enum two_syllable_model {
    TWO_SYLLABLE_WEAK_FIRST = 0,
    TWO_SYLLABLE_REDUCED_OPEN = 1,
    TWO_SYLLABLE_REDUCED_CLOSED = 2
};

struct syllable_timing {
    int64_t first_window;
    int64_t hold_window;
    int64_t vowel_onset;
    int64_t vowel_closure;
    int64_t coda_release;
};

static int prepare(const unsigned char *text, size_t length,
                   struct prepared *out,
                   reloq_german_cancel_check cancelled,
                   void *cancel_context)
{
    unsigned char *annotation = NULL;
    size_t bytes = 0, bad = 0;
    int result = RELOQ_DE_UTTERANCE_INVALID;
    if (!out) return result;
    memset(out, 0, sizeof *out);
    if (!text || !length) return result;
    if (cancelled && cancelled(cancel_context))
        return RELOQ_DE_UTTERANCE_CANCELLED;
    if (text[0] == '`') {
        if (length == SIZE_MAX) return RELOQ_DE_UTTERANCE_MEMORY;
        bytes = length;
        annotation = malloc(bytes + 1);
        if (!annotation) return RELOQ_DE_UTTERANCE_MEMORY;
        memcpy(annotation, text, bytes);
        annotation[bytes] = 0;
    } else {
        int status = reloq_german_text_annotate_checked(
            text, length, NULL, 0, &bytes, &bad, NULL,
            cancelled, cancel_context);
        if (status) {
            out->bad_offset = bad;
            return status == RELOQ_DE_TEXT_UNCOVERED ?
                RELOQ_DE_UTTERANCE_UNCOVERED :
                status == RELOQ_DE_TEXT_CANCELLED ?
                RELOQ_DE_UTTERANCE_CANCELLED :
                status == RELOQ_DE_TEXT_NO_MEMORY ?
                RELOQ_DE_UTTERANCE_MEMORY : RELOQ_DE_UTTERANCE_INVALID;
        }
        if (bytes == SIZE_MAX) return RELOQ_DE_UTTERANCE_MEMORY;
        annotation = malloc(bytes + 1);
        if (!annotation) return RELOQ_DE_UTTERANCE_MEMORY;
        size_t written = bytes;
        status = reloq_german_text_annotate_checked(
            text, length, annotation, bytes + 1, &written, &bad,
            NULL, cancelled, cancel_context);
        if (status || written != bytes) {
            out->bad_offset = bad;
            result = status == RELOQ_DE_TEXT_UNCOVERED ?
                RELOQ_DE_UTTERANCE_UNCOVERED :
                status == RELOQ_DE_TEXT_CANCELLED ?
                RELOQ_DE_UTTERANCE_CANCELLED :
                status == RELOQ_DE_TEXT_NO_MEMORY ?
                RELOQ_DE_UTTERANCE_MEMORY : RELOQ_DE_UTTERANCE_INVALID;
            goto done;
        }
    }
    if (cancelled && cancelled(cancel_context)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done;
    }
    /* One phonetic group and one isolated terminal mark. A connected
     * control, another word, or stray punctuation must not be silently
     * discarded by the phone-sequence scanner. */
    if (bytes < 7 || annotation[0] != '`' || annotation[1] != '[') {
        result = RELOQ_DE_UTTERANCE_UNCOVERED;
        goto done;
    }
    const unsigned char *close = memchr(annotation + 2, ']', bytes - 2);
    if (!close) {
        result = RELOQ_DE_UTTERANCE_UNCOVERED;
        goto done;
    }
    const size_t trailing = bytes - (size_t)(close - annotation) - 1;
    enum reloq_german_terminal_mark mark;
    if (!trailing) mark = RELOQ_DE_TERMINAL_IMPLICIT;
    else if (trailing == 1) {
        switch (close[1]) {
        case '.': mark = RELOQ_DE_TERMINAL_PERIOD; break;
        case '?': mark = RELOQ_DE_TERMINAL_QUESTION; break;
        case '!': mark = RELOQ_DE_TERMINAL_EXCLAMATION; break;
        case ':': mark = RELOQ_DE_TERMINAL_COLON; break;
        case ';': mark = RELOQ_DE_TERMINAL_SEMICOLON; break;
        case ',': mark = RELOQ_DE_TERMINAL_COMMA; break;
        default:
            result = RELOQ_DE_UTTERANCE_UNCOVERED;
            goto done;
        }
    } else if (trailing == 3 && close[1] == '.' &&
               close[2] == '.' && close[3] == '.')
        mark = RELOQ_DE_TERMINAL_ELLIPSIS;
    else {
        result = RELOQ_DE_UTTERANCE_UNCOVERED;
        goto done;
    }
    struct reloq_german_phone_sequence_info sequence = {0};
    if (reloq_german_phone_sequence_build(annotation, bytes, 0, out->phones,
                                           3, &sequence)) {
        result = RELOQ_DE_UTTERANCE_UNCOVERED;
        goto done;
    }
    if (cancelled && cancelled(cancel_context)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done;
    }
    const enum reloq_german_phone initial = out->phones[0].phone;
    const int voiced_stop = initial == RELOQ_DE_PHONE_b ||
        initial == RELOQ_DE_PHONE_d || initial == RELOQ_DE_PHONE_g;
    const int nasal_onset = initial == RELOQ_DE_PHONE_m ||
        initial == RELOQ_DE_PHONE_n;
    const size_t vowel_index = initial == RELOQ_DE_PHONE_h ||
        initial == RELOQ_DE_PHONE_p || initial == RELOQ_DE_PHONE_t ||
        initial == RELOQ_DE_PHONE_k || voiced_stop || nasal_onset;
    if ((sequence.phone_count != 2 && sequence.phone_count != 3) ||
        sequence.phone_count != vowel_index + 2 ||
        sequence.word_count != 1 || sequence.syllable_count != 1 ||
        (out->phones[vowel_index + 1].phone != RELOQ_DE_PHONE_p &&
         out->phones[vowel_index + 1].phone != RELOQ_DE_PHONE_t &&
         out->phones[vowel_index + 1].phone != RELOQ_DE_PHONE_k &&
         out->phones[vowel_index + 1].phone != RELOQ_DE_PHONE_s &&
         out->phones[vowel_index + 1].phone != RELOQ_DE_PHONE_f) ||
        ((voiced_stop || nasal_onset) &&
         (mark != RELOQ_DE_TERMINAL_PERIOD ||
          (out->phones[vowel_index + 1].phone != RELOQ_DE_PHONE_p &&
           out->phones[vowel_index + 1].phone != RELOQ_DE_PHONE_t &&
           out->phones[vowel_index + 1].phone != RELOQ_DE_PHONE_k)))) {
        result = RELOQ_DE_UTTERANCE_UNCOVERED;
        goto done;
    }
    for (size_t i = 0; i < sequence.phone_count; ++i)
        if (out->phones[i].stress != 1 ||
            !out->phones[i].stress_explicit ||
            out->phones[i].word_index != 0 ||
            out->phones[i].syllable_in_word != 0) {
            result = RELOQ_DE_UTTERANCE_UNCOVERED;
            goto done;
        }
    switch (out->phones[vowel_index].phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
        result = RELOQ_DE_UTTERANCE_OK;
        out->phone_count = sequence.phone_count;
        out->vowel_index = vowel_index;
        out->annotation_bytes = bytes;
        out->mark = mark;
        break;
    default:
        result = RELOQ_DE_UTTERANCE_UNCOVERED;
        break;
    }
done:
    free(annotation);
    return result;
}

/* Complete explicit two-syllable word, validated by one of three measured
 * phone-class models. Natural spelling reaches a model only
 * if the text frontend supplies exactly the same two-syllable structure. */
static int prepare_within_word(
    const unsigned char *text, size_t length,
    struct prepared_within_word *out,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    if (!text || !length || !out) return RELOQ_DE_UTTERANCE_INVALID;
    memset(out, 0, sizeof *out);
    if (cancelled && cancelled(cancel_context))
        return RELOQ_DE_UTTERANCE_CANCELLED;
    unsigned char *annotation = NULL;
    size_t bytes = 0, bad = 0;
    int result = RELOQ_DE_UTTERANCE_UNCOVERED;
    if (text[0] == '`' || (length > 1 && text[0] == ' ' && text[1] == '`')) {
        bytes = length;
        if (memchr(text, 0, bytes) || bytes == SIZE_MAX)
            return RELOQ_DE_UTTERANCE_INVALID;
        annotation = malloc(bytes + 1);
        if (!annotation) return RELOQ_DE_UTTERANCE_MEMORY;
        memcpy(annotation, text, bytes);
        annotation[bytes] = 0;
    } else {
        int status = reloq_german_text_annotate_checked(
            text, length, NULL, 0, &bytes, &bad, NULL,
            cancelled, cancel_context);
        if (status) {
            out->bad_offset = bad;
            return status == RELOQ_DE_TEXT_UNCOVERED ?
                RELOQ_DE_UTTERANCE_UNCOVERED :
                status == RELOQ_DE_TEXT_CANCELLED ?
                RELOQ_DE_UTTERANCE_CANCELLED :
                status == RELOQ_DE_TEXT_NO_MEMORY ?
                RELOQ_DE_UTTERANCE_MEMORY : RELOQ_DE_UTTERANCE_INVALID;
        }
        if (bytes == SIZE_MAX) return RELOQ_DE_UTTERANCE_MEMORY;
        annotation = malloc(bytes + 1);
        if (!annotation) return RELOQ_DE_UTTERANCE_MEMORY;
        size_t written = bytes;
        status = reloq_german_text_annotate_checked(
            text, length, annotation, bytes + 1, &written, &bad, NULL,
            cancelled, cancel_context);
        if (status || written != bytes) {
            out->bad_offset = bad;
            result = status == RELOQ_DE_TEXT_UNCOVERED ?
                RELOQ_DE_UTTERANCE_UNCOVERED :
                status == RELOQ_DE_TEXT_CANCELLED ?
                RELOQ_DE_UTTERANCE_CANCELLED :
                status == RELOQ_DE_TEXT_NO_MEMORY ?
                RELOQ_DE_UTTERANCE_MEMORY : RELOQ_DE_UTTERANCE_INVALID;
            goto done_within;
        }
    }
    if (cancelled && cancelled(cancel_context)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done_within;
    }
    size_t lead = annotation[0] == ' ' ? 1 : 0;
    if (bytes < lead + 10 || annotation[lead] != '`' ||
        annotation[lead + 1] != '[' ||
        annotation[bytes - 2] != ']' || annotation[bytes - 1] != '.')
        goto done_within;
    const unsigned char *close = memchr(annotation + lead + 2, ']',
                                        bytes - lead - 2);
    if (close != annotation + bytes - 2) goto done_within;
    struct reloq_german_phone_sequence_info sequence = {0};
    if (reloq_german_phone_sequence_build(
            annotation, bytes, 0, out->phones, 6, &sequence) ||
        (sequence.phone_count != 4 && sequence.phone_count != 5 &&
         sequence.phone_count != 6) || sequence.word_count != 1 ||
        sequence.syllable_count != 2) goto done_within;
    size_t syllable_count = 0;
    if (reloq_german_syllable_plan_build(
            out->phones, sequence.phone_count, out->syllables, 2,
            &syllable_count) ||
        syllable_count != 2) goto done_within;
    int matched = 0;
    if (sequence.phone_count == 6) {
        struct reloq_german_within_word_landmarks landmarks;
        if (!reloq_german_within_word_landmarks_plan(
                out->phones, 6, out->syllables, 2, &landmarks)) {
            out->model = TWO_SYLLABLE_WEAK_FIRST;
            matched = 1;
        }
    }
    if (!matched) {
        struct reloq_german_reduced_landmarks landmarks;
        if (reloq_german_reduced_landmarks_plan(
                out->phones, sequence.phone_count,
                out->syllables, 2, &landmarks))
            goto done_within;
        if (out->syllables[0].phone_count == 2 &&
            out->syllables[1].onset_count == 1)
            out->model = TWO_SYLLABLE_REDUCED_OPEN;
        else if (out->syllables[0].phone_count == 3)
            out->model = TWO_SYLLABLE_REDUCED_CLOSED;
        else goto done_within;
    }
    out->phone_count = sequence.phone_count;
    out->annotation_bytes = bytes;
    result = RELOQ_DE_UTTERANCE_OK;
done_within:
    free(annotation);
    return result;
}

/* The connected frontend validates the complete SPR structure. Natural
 * spelling is admitted only when its entire public phoneme callback was
 * measured to have the same two primary stresses and the internal `2
 * control. This is a data-driven coverage gate, independent of the
 * reusable acoustic k-to-t transition below. */
static int prepare_connected(const unsigned char *text, size_t length,
                             struct reloq_german_connected_plan *plan,
                             size_t *bad_offset,
                             reloq_german_cancel_check cancelled,
                             void *cancel_context)
{
    if (bad_offset) *bad_offset = 0;
    if (!text || !length || !plan) return RELOQ_DE_UTTERANCE_INVALID;
    if (cancelled && cancelled(cancel_context))
        return RELOQ_DE_UTTERANCE_CANCELLED;
    int status = reloq_german_connected_annotation_prepare(
        text, length, plan);
    if (status != 1) {
        size_t needed = 0, bad = 0;
        status = reloq_german_connected_words_prepare(
            text, length, NULL, 0, &needed, &bad, NULL);
        if (status != 1) {
            if (bad_offset) *bad_offset = bad;
            return status == -3 ? RELOQ_DE_UTTERANCE_MEMORY :
                status == -1 ? RELOQ_DE_UTTERANCE_INVALID :
                RELOQ_DE_UTTERANCE_UNCOVERED;
        }
        if (needed == SIZE_MAX) return RELOQ_DE_UTTERANCE_MEMORY;
        unsigned char *annotation = malloc(needed + 1);
        if (!annotation) return RELOQ_DE_UTTERANCE_MEMORY;
        size_t written = needed;
        status = reloq_german_connected_words_prepare(
            text, length, annotation, needed + 1, &written, &bad, plan);
        free(annotation);
        if (status != 1 || written != needed) {
            if (bad_offset) *bad_offset = bad;
            return status == -3 ? RELOQ_DE_UTTERANCE_MEMORY :
                status == 0 ? RELOQ_DE_UTTERANCE_UNCOVERED :
                RELOQ_DE_UTTERANCE_INVALID;
        }
        if (!plan->reference_spr_equivalent)
            return RELOQ_DE_UTTERANCE_UNCOVERED;
    }
    if (cancelled && cancelled(cancel_context))
        return RELOQ_DE_UTTERANCE_CANCELLED;
    const size_t second = plan->word_phone_count[0];
    if (plan->word_phone_count[0] < 2 ||
        plan->word_phone_count[0] > 3 ||
        plan->word_phone_count[1] != 3 ||
        plan->phone_count != second + 3 ||
        !reloq_german_boundary_profile_find(
            plan->phones[second - 1].phone, plan->phones[second].phone) ||
        (plan->phones[0].phone != RELOQ_DE_PHONE_h &&
         plan->phones[0].phone != RELOQ_DE_PHONE_p &&
         plan->phones[0].phone != RELOQ_DE_PHONE_t &&
         plan->phones[0].phone != RELOQ_DE_PHONE_k &&
         plan->word_phone_count[0] != 2) ||
        (plan->phones[second + 2].phone != RELOQ_DE_PHONE_p &&
         plan->phones[second + 2].phone != RELOQ_DE_PHONE_t &&
         plan->phones[second + 2].phone != RELOQ_DE_PHONE_k))
        return RELOQ_DE_UTTERANCE_UNCOVERED;
    return RELOQ_DE_UTTERANCE_OK;
}

static void prepared_word(const struct reloq_german_connected_plan *plan,
                          size_t word, struct prepared *out)
{
    memset(out, 0, sizeof *out);
    const size_t start = word ? plan->word_phone_count[0] : 0;
    out->phone_count = plan->word_phone_count[word];
    out->vowel_index = out->phone_count - 2;
    out->mark = RELOQ_DE_TERMINAL_PERIOD;
    for (size_t i = 0; i < out->phone_count; ++i) {
        out->phones[i] = plan->phones[start + i];
        out->phones[i].word_index = 0;
        out->phones[i].syllable_index = 0;
        out->phones[i].syllable_in_word = 0;
    }
}

static int begin_stage(struct gesture_stage *stage)
{
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        stage->buffers[i].points = stage->points[i];
        stage->buffers[i].capacity = STAGE_POINTS;
    }
    return reloq_target_builder_begin(&stage->builder, stage->buffers);
}

/* A shared steady-state oral tract, extending the vowel's three first
 * formants to five and supplying measured bandwidth, nasal, and source
 * controls. Its second landmarks keep vowel resonance steady until the
 * independently predicted first scheduler window. The final-stop C tract
 * owns subsequent constriction and release. */
static int vowel_tract_append(struct composition *part,
                              int64_t anchor, int64_t first_window,
                              int64_t closure, int has_h)
{
    struct reloq_target_event events[64];
    size_t n = 0;
#define PUT(s, t, v) do { events[n++] = (struct reloq_target_event){s, t, v}; } while (0)
    const int oral[5] = {0, 0, 0, 3600, 3900};
    const int widths[5] = {120, 100, 150, 330, 260};
    for (size_t i = 0; i < 5; ++i) {
        enum reloq_target_stream s = (enum reloq_target_stream)
            (RELOQ_TARGET_F1 + (int)i);
        int value = i < 3 ?
            part->stage[VOWEL].buffers[s].points[0].value : oral[i];
        if (i < 3) {
            const struct reloq_target_buffer *vowel =
                &part->stage[VOWEL].builder.stream[s];
            for (size_t j = 1; j < vowel->count; ++j)
                if (vowel->points[j].tick == first_window)
                    value = vowel->points[j].value;
        }
        if (i >= 3) PUT(s, anchor, value);
        PUT(s, first_window, value);
        s = (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)i);
        PUT(s, anchor, widths[i]);
        PUT(s, first_window, widths[i]);
    }
    const enum reloq_target_stream nasal[] = {
        RELOQ_TARGET_FNP, RELOQ_TARGET_FNZ};
    for (size_t i = 0; i < 2; ++i) {
        PUT(nasal[i], anchor, 200);
        PUT(nasal[i], first_window, 200);
    }
    PUT(RELOQ_TARGET_FTP, anchor, 250);
    PUT(RELOQ_TARGET_FTZ, anchor, 250);
    PUT(RELOQ_TARGET_FL, anchor, 0);
    PUT(RELOQ_TARGET_TL, anchor + 1, 0);
    PUT(RELOQ_TARGET_TL, closure, 0);
    PUT(RELOQ_TARGET_OQ, anchor, has_h ? 56 : 18);
    if (!has_h) PUT(RELOQ_TARGET_OQ, 21, 56);
    PUT(RELOQ_TARGET_OQ, first_window, 56);
    PUT(RELOQ_TARGET_DI, anchor, has_h ? 0 : 100);
    if (!has_h) PUT(RELOQ_TARGET_DI, 21, 0);
    PUT(RELOQ_TARGET_DI, first_window, 0);
    PUT(RELOQ_TARGET_AF, first_window, 0);
    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        PUT(s, anchor, 0);
        PUT(s, first_window, 0);
    }
#undef PUT
    return reloq_target_builder_append(&part->stage[VOWEL_TRACT].builder,
                                        0, events, n);
}

/* Initial voiceless stops have two lead-in windows: a place burst, then
 * aspiration and formant movement toward the vowel. The three lower
 * coarticulating formants and vowel AV/AH belong to the stop-vowel model;
 * the remaining steady-state tract controls use this common shape. */
static int stop_vowel_tract_append(struct composition *part,
                                   enum reloq_german_phone initial,
                                   int64_t burst_end, int64_t vowel_onset,
                                   int64_t second_window, int64_t closure)
{
    if (burst_end < 5 || vowel_onset <= burst_end ||
        second_window < vowel_onset || closure <= second_window) return -1;
    struct reloq_target_event events[100];
    size_t n = 0;
#define PUT(s, t, v) do { events[n++] = (struct reloq_target_event){s, t, v}; } while (0)
    const int widths[5] = {120, 100, 150, 330, 260};
    const int top[2] = {3600, 3900};
    const int t_top[2] = {3500, 4200};
    const int64_t windows[] = {0, burst_end, vowel_onset, second_window};
    for (size_t w = 0; w < sizeof windows / sizeof windows[0]; ++w) {
        for (size_t f = 0; f < 2; ++f)
            PUT((enum reloq_target_stream)(RELOQ_TARGET_F4 + (int)f),
                windows[w], w < 2 && initial == RELOQ_DE_PHONE_t ?
                t_top[f] : top[f]);
        for (size_t b = 0; b < 5; ++b)
            PUT((enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)b),
                windows[w], widths[b]);
        PUT(RELOQ_TARGET_FNP, windows[w], 200);
        PUT(RELOQ_TARGET_FNZ, windows[w], 200);
    }
    PUT(RELOQ_TARGET_FTP, 0, 250);
    PUT(RELOQ_TARGET_FTZ, 0, 250);
    PUT(RELOQ_TARGET_FL, 0, 0);
    PUT(RELOQ_TARGET_TL, 0, 0);
    PUT(RELOQ_TARGET_TL, 5, 0);
    PUT(RELOQ_TARGET_TL, vowel_onset + 1, 0);
    PUT(RELOQ_TARGET_TL, closure, 0);
    for (size_t w = 0; w < sizeof windows / sizeof windows[0]; ++w) {
        PUT(RELOQ_TARGET_OQ, windows[w], 56);
        PUT(RELOQ_TARGET_DI, windows[w], 0);
    }
    enum reloq_target_stream active = RELOQ_TARGET_STREAM_COUNT;
    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        const struct reloq_target_buffer *src =
            &part->stage[INITIAL_H].builder.stream[s];
        int nonzero = 0;
        for (size_t j = 0; j < src->count; ++j)
            nonzero |= src->points[j].value > 0;
        if (nonzero) {
            if (active != RELOQ_TARGET_STREAM_COUNT) return -1;
            active = s;
            PUT(s, second_window, 0);
        } else {
            PUT(s, 0, 0);
            PUT(s, 5, 0);
            PUT(s, vowel_onset, 0);
            PUT(s, second_window, 0);
        }
    }
    if (active == RELOQ_TARGET_STREAM_COUNT) return -1;
    PUT(RELOQ_TARGET_AF, second_window, 0);
#undef PUT
    return reloq_target_builder_append(&part->stage[VOWEL_TRACT].builder,
                                        0, events, n);
}

/* The coda's frication remains zero through the closure. Its short burst
 * then rises at a same-tick 0 -> peak transition preburst milliseconds
 * before the final release target. Without this hold, a full-utterance
 * curve would anticipate the terminal-source peak from the first vowel
 * window and turn a stop into a prolonged hiss. */
static int stop_burst_append(struct composition *part,
                              enum reloq_german_phone coda,
                              int64_t closure, int64_t release)
{
    int64_t preburst;
    if (coda == RELOQ_DE_PHONE_p) preburst = 10;
    else if (coda == RELOQ_DE_PHONE_t) preburst = 8;
    else if (coda == RELOQ_DE_PHONE_k) preburst = 12;
    else return -1;
    if (release < closure + preburst) return -1;
    enum reloq_target_stream noise = RELOQ_TARGET_STREAM_COUNT;
    int32_t strength = 0;
    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        const struct reloq_target_buffer *source =
            &part->stage[RELEASE].builder.stream[s];
        if (source->count && source->points[0].value > 0) {
            if (noise != RELOQ_TARGET_STREAM_COUNT) return -1;
            noise = s;
            strength = source->points[0].value;
        }
    }
    if (noise == RELOQ_TARGET_STREAM_COUNT) return -1;
    int32_t af = part->stage[RELEASE].builder.stream[RELOQ_TARGET_AF]
                     .points[0].value;
    struct reloq_target_event points[] = {
        {RELOQ_TARGET_AF, closure, 0},
        {RELOQ_TARGET_AF, release - preburst, 0},
        {RELOQ_TARGET_AF, release - preburst, af},
        {RELOQ_TARGET_AF, release, af},
        {noise, closure, 0},
        {noise, release - preburst, 0},
        {noise, release - preburst, strength},
        {noise, release, strength}
    };
    return reloq_target_builder_append(&part->stage[STOP_BURST].builder,
                                        0, points, sizeof points / sizeof points[0]);
}

/* Terminal release returns the upper oral resonators to the shared
 * steady-state vowel tract during the 60-tick source fade. This transition
 * is audible after /t/, whose closure moves F4/F5 to 3500/4200. For /p/
 * and /k/ the same return values are unchanged. */
static int terminal_tract_return(struct composition *part, int64_t release)
{
    if (release > INT64_MAX - 60) return -1;
    const struct reloq_target_event events[] = {
        {RELOQ_TARGET_F4, release + 60, 3600},
        {RELOQ_TARGET_F5, release + 60, 3900}
    };
    return reloq_target_builder_append(&part->stage[VOWEL_TRACT].builder,
                                        0, events,
                                        sizeof events / sizeof events[0]);
}

static int combine(struct composition *part,
                   const float defaults[RELOQ_PARAMETER_TRACKS],
                   const uint8_t fields[RELOQ_TARGET_STREAM_COUNT],
                   struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS],
                   unsigned char mapped[RELOQ_PARAMETER_TRACKS],
                   size_t *point_count)
{
    struct reloq_target_stage stages[VOICE_STAGES];
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        part->buffers[s] = (struct reloq_target_buffer){
            part->points[s], FINAL_POINTS, 0};
    }
    for (size_t i = 0; i < VOICE_STAGES; ++i)
        stages[i] = (struct reloq_target_stage){
            &part->stage[i].builder, 0, RELOQ_TARGET_STAGE_ALL_STREAMS};
    if (reloq_target_builder_begin(&part->combined, part->buffers) ||
        reloq_target_stage_merge(&part->combined, stages,
                                 VOICE_STAGES, NULL, point_count) ||
        reloq_target_builder_export(&part->combined, fields, defaults,
                                    curves, mapped)) return -1;
    return 0;
}

enum pair_copy_kind {
    PAIR_COPY_FIRST_INITIAL,
    PAIR_COPY_FIRST_VOWEL,
    PAIR_COPY_FIRST_TRACT,
    PAIR_COPY_SECOND_VOWEL,
    PAIR_COPY_SECOND_TRACT,
    PAIR_COPY_SECOND_CODA
};

struct pair_copy_timing {
    int64_t old_hold, new_hold;
    int64_t old_closure, new_closure;
    int64_t second_old_onset, second_delta;
};

/* Transfer the reusable isolated phone coefficients into one connected
 * target timeline. The first vowel changes only its nonfinal duration and
 * coda-hold endpoint. The second vowel/coda retain their measured isolated
 * shape relative to the vowel anchor; its positive closure AV drops by 3.
 * Onset and word-boundary gestures come from connected_pair instead. */
static int copy_pair_stage(const struct gesture_stage *source,
                           struct gesture_stage *destination,
                           enum pair_copy_kind kind,
                           const struct pair_copy_timing *timing)
{
    if (!source || !destination || !timing) return -1;
    for (enum reloq_target_stream stream = RELOQ_TARGET_F0;
         stream < RELOQ_TARGET_STREAM_COUNT; ++stream) {
        const struct reloq_target_buffer *input =
            &source->builder.stream[stream];
        struct reloq_target_event events[STAGE_POINTS];
        size_t used = 0;
        for (size_t i = 0; i < input->count; ++i) {
            const struct reloq_acoustic_point p = input->points[i];
            int64_t tick = p.tick;
            int32_t value = p.value;
            if (stream == RELOQ_TARGET_F0) continue;
            if (kind == PAIR_COPY_FIRST_TRACT &&
                tick > timing->old_closure) continue;
            if (kind == PAIR_COPY_SECOND_VOWEL &&
                stream >= RELOQ_TARGET_F1 &&
                stream <= RELOQ_TARGET_F3 &&
                tick < timing->second_old_onset) continue;
            if (kind == PAIR_COPY_SECOND_TRACT &&
                tick < timing->second_old_onset) continue;
            if (kind == PAIR_COPY_FIRST_VOWEL ||
                kind == PAIR_COPY_FIRST_TRACT) {
                if (tick == timing->old_hold) tick = timing->new_hold;
                else if (tick == timing->old_closure)
                    tick = timing->new_closure;
            } else if (kind == PAIR_COPY_SECOND_VOWEL ||
                       kind == PAIR_COPY_SECOND_TRACT ||
                       kind == PAIR_COPY_SECOND_CODA) {
                if (tick > INT64_MAX - timing->second_delta)
                    return -1;
                if (kind == PAIR_COPY_SECOND_VOWEL &&
                    stream == RELOQ_TARGET_AV &&
                    tick == timing->old_closure && value > 0)
                    value -= 3;
                tick += timing->second_delta;
            }
            if (used == STAGE_POINTS) return -1;
            events[used++] = (struct reloq_target_event){
                stream, tick, value};
        }
        if (used && reloq_target_builder_append(
                        &destination->builder, 0, events, used)) return -1;
    }
    return 0;
}

static int render_result(int result)
{
    switch (result) {
    case RELOQ_SESSION_OK: return RELOQ_DE_UTTERANCE_OK;
    case RELOQ_SESSION_CANCELLED: return RELOQ_DE_UTTERANCE_CANCELLED;
    case RELOQ_SESSION_MEMORY: return RELOQ_DE_UTTERANCE_MEMORY;
    case RELOQ_SESSION_SINK: return RELOQ_DE_UTTERANCE_SINK;
    default: return RELOQ_DE_UTTERANCE_RENDER;
    }
}

static int session_cancelled(void *context)
{
    return reloq_session_cancelled((const struct reloq_session *)context);
}

static int compose_one(const struct prepared *prepared,
                       struct composition *part,
                       struct reloq_german_utterance_info *progress,
                       struct syllable_timing *timing);

static int compose_connected(
    const struct reloq_german_connected_plan *plan,
    struct composition *phrase,
    struct reloq_german_utterance_info *progress,
    int64_t *first_boundary_tick)
{
    if (!plan || !phrase || !progress) return -1;
    struct composition *isolated_first = calloc(1, sizeof *isolated_first);
    struct composition *isolated_second = calloc(1, sizeof *isolated_second);
    if (!isolated_first || !isolated_second) {
        free(isolated_first);
        free(isolated_second);
        return -2;
    }
    int result = -1;
    struct prepared first, second;
    struct syllable_timing first_time, second_time;
    struct reloq_german_utterance_info ignored = {0};
    prepared_word(plan, 0, &first);
    prepared_word(plan, 1, &second);
    if (compose_one(&first, isolated_first, &ignored, &first_time) ||
        compose_one(&second, isolated_second, &ignored, &second_time))
        goto done;
    for (size_t i = 0; i < VOICE_STAGES; ++i)
        if (begin_stage(&phrase->stage[i])) goto done;

    struct reloq_german_connected_pair pair = {
        first.vowel_index ? first.phones[0].phone : RELOQ_DE_PHONE_NONE,
        first.phones[first.vowel_index].phone,
        first.phones[first.vowel_index + 1].phone,
        second.phones[0].phone
    };
    int64_t lead;
    if (reloq_german_coda_hold_lead(
            pair.first_vowel, pair.first_coda, &lead) || lead <= 0)
        goto done;
    int64_t initial_end = first.vowel_index ?
        first_time.first_window : -1;
    if (pair.first_initial == RELOQ_DE_PHONE_h) {
        int64_t duration;
        if (reloq_german_connected_first_duration(&pair, &duration) ||
            first_time.vowel_onset > INT64_MAX - duration ||
            first_time.vowel_onset + duration <= lead)
            goto done;
        initial_end = first_time.vowel_onset + duration - lead;
    }
    int64_t first_closure, first_release;
    if (reloq_german_connected_first_closure(
            &pair, first_time.vowel_onset, initial_end,
            &first_closure, &first_release)) goto done;
    const struct reloq_german_boundary_profile *profile =
        reloq_german_boundary_profile_find(pair.first_coda,
                                            pair.second_initial);
    if (!profile) goto done;
    int64_t second_burst_end, second_onset;
    if (profile->onset_landmarks(
            &pair, second.phones[second.vowel_index].phone,
            first_release, &second_burst_end, &second_onset)) goto done;
    if (second_onset < second_time.vowel_onset) goto done;
    const int64_t delta = second_onset - second_time.vowel_onset;
    if (second_time.coda_release > INT64_MAX - delta ||
        second_time.vowel_closure > INT64_MAX - delta) goto done;
    const int64_t second_closure = second_time.vowel_closure + delta;
    const int64_t second_release = second_time.coda_release + delta;
    if (first_closure <= lead) goto done;
    const struct pair_copy_timing first_copy = {
        first_time.hold_window, first_closure - lead,
        first_time.vowel_closure, first_closure, 0, 0};
    const struct pair_copy_timing second_copy = {
        0, 0, second_time.vowel_closure, 0,
        second_time.vowel_onset, delta};
    if (copy_pair_stage(&isolated_first->stage[INITIAL_H],
                        &phrase->stage[PAIR_FIRST_INITIAL],
                        PAIR_COPY_FIRST_INITIAL, &first_copy) ||
        copy_pair_stage(&isolated_first->stage[VOWEL],
                        &phrase->stage[PAIR_FIRST_VOWEL],
                        PAIR_COPY_FIRST_VOWEL, &first_copy) ||
        copy_pair_stage(&isolated_first->stage[VOWEL_TRACT],
                        &phrase->stage[PAIR_FIRST_TRACT],
                        PAIR_COPY_FIRST_TRACT, &first_copy)) goto done;

    int64_t next_origin;
    if (profile->coda_append(
            &pair, first_closure,
            &phrase->stage[PAIR_FIRST_CODA].builder, &next_origin) ||
        next_origin != first_release + profile->next_origin_offset)
        goto done;
    const struct reloq_target_event first_end[] = {
        {RELOQ_TARGET_FL, first_release, 0},
        {RELOQ_TARGET_FTP, first_release, 250},
        {RELOQ_TARGET_FTZ, first_release, 250}
    };
    if (reloq_target_builder_append(
            &phrase->stage[PAIR_FIRST_CODA].builder, 0,
            first_end, sizeof first_end / sizeof first_end[0])) goto done;
    int64_t observed_burst, observed_onset;
    if (profile->onset_append(
            &pair, second.phones[second.vowel_index].phone,
            first_release, second_release,
            &phrase->stage[PAIR_SECOND_ONSET].builder,
            &observed_burst, &observed_onset) ||
        observed_burst != second_burst_end ||
        observed_onset != second_onset) goto done;
    /* The initial /t/ frication ends at the vowel anchor. The onset helper
     * owns the burst, while the isolated stop model supplies this shared
     * noise-to-vowel endpoint. Holding the burst until the vowel's later
     * scheduler window would spread its AF/A5F energy into the vowel. */
    const struct reloq_target_event onset_fade[] = {
        {RELOQ_TARGET_AF, second_onset, 0},
        {profile->onset_fade_stream, second_onset, 0}
    };
    if (reloq_target_builder_append(
            &phrase->stage[PAIR_SECOND_ONSET].builder, 0,
            onset_fade, sizeof onset_fade / sizeof onset_fade[0]))
        goto done;
    if (copy_pair_stage(&isolated_second->stage[VOWEL],
                        &phrase->stage[PAIR_SECOND_VOWEL],
                        PAIR_COPY_SECOND_VOWEL, &second_copy) ||
        copy_pair_stage(&isolated_second->stage[VOWEL_TRACT],
                        &phrase->stage[PAIR_SECOND_TRACT],
                        PAIR_COPY_SECOND_TRACT, &second_copy) ||
        copy_pair_stage(&isolated_second->stage[STOP_TRACT],
                        &phrase->stage[PAIR_SECOND_CODA_TRACT],
                        PAIR_COPY_SECOND_CODA, &second_copy) ||
        copy_pair_stage(&isolated_second->stage[STOP_BURST],
                        &phrase->stage[PAIR_SECOND_BURST],
                        PAIR_COPY_SECOND_CODA, &second_copy) ||
        copy_pair_stage(&isolated_second->stage[RELEASE],
                        &phrase->stage[PAIR_SECOND_RELEASE],
                        PAIR_COPY_SECOND_CODA, &second_copy)) goto done;
    struct reloq_german_connected_landmarks landmarks;
    if (reloq_german_connected_pair_f0_append(
            &pair, first_time.vowel_onset, initial_end,
            second_onset, second_closure,
            &phrase->stage[PAIR_F0].builder, &landmarks) ||
        landmarks.first_closure_tick != first_closure ||
        landmarks.first_coda_send_end != first_release ||
        second_release > INT64_MAX - 460) goto done;
    progress->phone_count = first.phone_count + second.phone_count;
    progress->planned_ticks = second_release + 460;
    if (first_boundary_tick) *first_boundary_tick = first_release;
    result = 0;
done:
    free(isolated_first);
    free(isolated_second);
    return result;
}

static int render_connected(const unsigned char *utf8, size_t length,
                            struct reloq_session *session,
                            reloq_session_pcm_sink sink, void *context,
                            struct reloq_german_utterance_info *info)
{
    if (!info || !session || !sink) return RELOQ_DE_UTTERANCE_INVALID;
    *info = (struct reloq_german_utterance_info){0};
    struct reloq_german_connected_plan plan;
    size_t bad = 0;
    int status = prepare_connected(utf8, length, &plan, &bad,
                                   session_cancelled, session);
    info->error_offset = bad;
    if (status) return status;
    struct composition *phrase = calloc(1, sizeof *phrase);
    if (!phrase) return RELOQ_DE_UTTERANCE_MEMORY;
    struct reloq_german_utterance_info progress = {0};
    int result = RELOQ_DE_UTTERANCE_MODEL;
    float defaults[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config voice_config;
    if (reloq_german_voice_defaults(defaults, fields, &voice_config))
        goto done;
    int composed = compose_connected(&plan, phrase, &progress, NULL);
    if (composed) {
        if (composed == -2) result = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    if (plan.group_offset[1] >
        SIZE_MAX - plan.group_length[1] - 1) goto done;
    progress.annotation_bytes =
        plan.group_offset[1] + plan.group_length[1] + 1;
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS];
    unsigned char mapped[RELOQ_PARAMETER_TRACKS];
    if (combine(phrase, defaults, fields, curves, mapped,
                &progress.target_count)) goto done;
    if (reloq_session_cancelled(session)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done;
    }
    struct reloq_curve_scheduler scheduler;
    struct reloq_curve_schedule schedule;
    if (reloq_curve_scheduler_begin(&scheduler, 0, 1, mapped) ||
        reloq_curve_scheduler_plan(&scheduler, curves,
                                   progress.planned_ticks, 1, &schedule))
        goto done;
    result = render_result(reloq_curve_pipeline_render(
        &schedule, RELOQ_GERMAN_VOICE_FORMANTS, session, sink, context,
        &progress.frames_completed));
    if (reloq_curve_scheduler_commit(&scheduler, progress.frames_completed) &&
        result == RELOQ_DE_UTTERANCE_OK)
        result = RELOQ_DE_UTTERANCE_RENDER;
done:
    *info = progress;
    free(phrase);
    return result;
}

static int word_is(const unsigned char *word, size_t length,
                   const char *spelling)
{
    size_t candidate = strlen(spelling);
    return length == candidate && !memcmp(word, spelling, candidate);
}

/* Apple's public ECI index is delivered 1, 2 or 3 PCM samples after the
 * connected coda send endpoint, depending on its first word. This is an
 * independent finite callback observation: the 50 natural k-coda first
 * words with the Pack carrier, 122 Pack+second-word contexts, 61 App+T
 * contexts, and the individually verified k->t/p and p->t pairs. Do not
 * infer a timing offset for a newly added voice pair from its phonemes.
 * Returning zero keeps the ECI preflight transactional for that phrase. */
static int connected_index_tail(const struct reloq_german_connected_plan *plan,
                                const unsigned char *utf8)
{
    if (!plan->word_phone_count[0] ||
        plan->word_phone_count[0] >= plan->phone_count) return 0;
    const enum reloq_german_phone coda =
        plan->phones[plan->word_phone_count[0] - 1].phone;
    const enum reloq_german_phone onset =
        plan->phones[plan->word_phone_count[0]].phone;
    const unsigned char *first = utf8 + plan->word_input_offset[0];
    size_t length = plan->word_input_length[0];
#define FIRST(spelling) word_is(first, length, (spelling))
    if (coda == RELOQ_DE_PHONE_k &&
        (onset == RELOQ_DE_PHONE_p || onset == RELOQ_DE_PHONE_t)) {
        if (FIRST("Tick") || FIRST("Kick")) return 1;
        if (reloq_german_connected_cvc_reference_pair(
                first, length, (const unsigned char *)"Pack", 4) != 1)
            return 0;
        if (FIRST("Hack") || FIRST("hack") ||
            FIRST("Pack") || FIRST("pack") ||
            FIRST("Pik") || FIRST("pik") ||
            FIRST("Puk") || FIRST("puk") ||
            FIRST("Kik") || FIRST("kik") ||
            FIRST("Kuk") || FIRST("kuk") ||
            FIRST("Tik") || FIRST("tik") ||
            FIRST("Tuk") || FIRST("tuk")) return 1;
        return 2;
    }
    if (coda == RELOQ_DE_PHONE_p && onset == RELOQ_DE_PHONE_t) {
        if (FIRST("App") || FIRST("app") || FIRST("Papp") ||
            FIRST("Piep") || FIRST("Hopp") || FIRST("Pup") ||
            FIRST("Kapp") || FIRST("Pip") || FIRST("Hipp")) return 3;
        if (FIRST("Tipp") || FIRST("Topp") || FIRST("Popp")) return 2;
    }
#undef FIRST
    return 0;
}

int reloq_german_voice_connected_index_sample(
    const unsigned char *utf8, size_t length,
    size_t *word_byte_offset, size_t *sample_offset,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    if (!utf8 || !length || !word_byte_offset || !sample_offset) return -1;
    if (cancelled && cancelled(cancel_context)) return -3;
    struct reloq_german_connected_plan plan;
    size_t bad = 0;
    int status = prepare_connected(utf8, length, &plan, &bad,
                                   cancelled, cancel_context);
    if (status == RELOQ_DE_UTTERANCE_UNCOVERED) return 0;
    if (status == RELOQ_DE_UTTERANCE_CANCELLED) return -3;
    if (status == RELOQ_DE_UTTERANCE_MEMORY) return -4;
    if (status != RELOQ_DE_UTTERANCE_OK) return -1;
    /* Explicit SPR has a different byte-offset and ECI command grammar.
     * The public reference establishes this index only for natural text. */
    if (plan.stress_explicitly_specified) return 0;
    int tail = connected_index_tail(&plan, utf8);
    if (!tail) return 0;
    struct composition *phrase = calloc(1, sizeof *phrase);
    if (!phrase) return -4;
    struct reloq_german_utterance_info progress = {0};
    int64_t boundary_tick = -1;
    int composed = compose_connected(&plan, phrase, &progress,
                                     &boundary_tick);
    free(phrase);
    if (composed) return composed == -2 ? -4 : 0;
    if (cancelled && cancelled(cancel_context)) return -3;
    if (boundary_tick < 0 ||
        (uint64_t)boundary_tick > (SIZE_MAX - (size_t)tail) / 11) return 0;
    const size_t word_end = plan.word_input_offset[0] +
                            plan.word_input_length[0];
    if (word_end >= length || utf8[word_end] != ' ') return 0;
    *word_byte_offset = word_end;
    *sample_offset = (size_t)boundary_tick * 11 + (size_t)tail;
    return 1;
}

static int compose_one(const struct prepared *prepared,
                       struct composition *part,
                       struct reloq_german_utterance_info *progress,
                       struct syllable_timing *timing)
{
    for (size_t i = 0; i < VOICE_STAGES; ++i)
        if (begin_stage(&part->stage[i])) return -1;

    int64_t first_window = 0, second_window = 0, formant_target = 0;
    int64_t anchor = 0, anchor_from_h = 0;
    int64_t closure_offset = 0, release_tract = 0, release_source = 0;
    int64_t fricative_end = 0;
    const int has_h = prepared->vowel_index != 0;
    const size_t vowel = prepared->vowel_index;
    const int has_initial_stop = has_h &&
        prepared->phones[0].phone != RELOQ_DE_PHONE_h;
    const int has_voiced_stop = has_initial_stop &&
        (prepared->phones[0].phone == RELOQ_DE_PHONE_b ||
         prepared->phones[0].phone == RELOQ_DE_PHONE_d ||
         prepared->phones[0].phone == RELOQ_DE_PHONE_g);
    const int has_nasal_onset = has_initial_stop &&
        (prepared->phones[0].phone == RELOQ_DE_PHONE_m ||
         prepared->phones[0].phone == RELOQ_DE_PHONE_n);
    const enum reloq_german_phone coda = prepared->phones[vowel + 1].phone;
    const int has_fricative_coda = coda == RELOQ_DE_PHONE_s ||
        coda == RELOQ_DE_PHONE_f;
    if (has_nasal_onset) {
        int64_t target, hold, absolute_closure;
        if (reloq_german_initial_nasal_append(
                prepared->phones, prepared->phone_count, 0,
                &part->stage[INITIAL_H].builder, &anchor) ||
            reloq_german_nasal_vowel_append(
                prepared->phones, prepared->phone_count, 0,
                &part->stage[VOWEL].builder, &target, &hold,
                &absolute_closure) ||
            anchor >= target || target > hold || hold >= absolute_closure)
            return -1;
        first_window = anchor;
        second_window = hold;
        closure_offset = absolute_closure - anchor;
    } else if (has_voiced_stop) {
        int64_t burst = 0;
        if (reloq_german_initial_voiced_stop_append(
                prepared->phones, prepared->phone_count, 0,
                &part->stage[INITIAL_H].builder, &anchor, &burst) ||
            reloq_german_voiced_stop_vowel_append(
                prepared->phones, prepared->phone_count, anchor,
                &part->stage[VOWEL].builder, &closure_offset,
                &first_window, &second_window, &formant_target) ||
            burst != first_window) return -1;
    } else if (has_initial_stop) {
        if (reloq_german_initial_stop_append(
                prepared->phones, prepared->phone_count, 0,
                &part->stage[INITIAL_H].builder, &anchor) ||
            (has_fricative_coda ?
             reloq_german_stop_fricative_vowel_append(
                 prepared->phones, prepared->phone_count, anchor,
                 &part->stage[VOWEL].builder, &closure_offset,
                 &first_window, &second_window) :
             reloq_german_stop_vowel_append(
                 prepared->phones, prepared->phone_count, anchor,
                 &part->stage[VOWEL].builder, &closure_offset,
                 &first_window, &second_window))) return -1;
    } else if (has_fricative_coda) {
        int64_t absolute_closure;
        if ((has_h && reloq_german_hv_acoustic_anchor(
                prepared->phones, prepared->phone_count, &anchor)) ||
            reloq_german_fricative_vowel_append(
                prepared->phones, prepared->phone_count, vowel, anchor,
                &part->stage[VOWEL].builder, &absolute_closure,
                &first_window)) return -1;
        closure_offset = absolute_closure - anchor;
        if (has_h) {
            int32_t vowel_av =
                part->stage[VOWEL].builder.stream[RELOQ_TARGET_AV]
                    .points[0].value;
            if (reloq_german_initial_h_append(
                    prepared->phones, prepared->phone_count, 0, vowel_av, -1,
                    &part->stage[INITIAL_H].builder, &anchor_from_h) ||
                anchor_from_h != anchor) return -1;
        }
    } else {
        if ((has_h ?
              reloq_german_hvc_first_send_end(prepared->phones,
                    prepared->phone_count, &first_window) :
              reloq_german_vc_first_send_end(prepared->phones,
                    prepared->phone_count, &first_window)) ||
            (has_h && reloq_german_hv_acoustic_anchor(
                prepared->phones, prepared->phone_count, &anchor)) ||
            reloq_german_vowel_nucleus_append(
                prepared->phones, prepared->phone_count, vowel, anchor, 0,
                &part->stage[VOWEL].builder, &closure_offset)) return -1;
        if (has_h) {
            int32_t vowel_av =
                part->stage[VOWEL].builder.stream[RELOQ_TARGET_AV]
                    .points[0].value;
            if (reloq_german_initial_h_append(
                    prepared->phones, prepared->phone_count, 0, vowel_av, -1,
                    &part->stage[INITIAL_H].builder, &anchor_from_h) ||
                anchor_from_h != anchor) return -1;
        }
    }
    int64_t closure = anchor + closure_offset;
    if ((has_nasal_onset ? 0 :
         has_voiced_stop ?
         reloq_german_voiced_stop_tract_append(
             prepared->phones, prepared->phone_count, anchor,
             formant_target, second_window, closure,
             &part->stage[VOWEL_TRACT].builder) :
         has_initial_stop ?
         (first_window >= anchor || second_window < anchor ||
          second_window >= closure ||
          stop_vowel_tract_append(part, prepared->phones[0].phone,
                                  first_window, anchor, second_window,
                                  closure)) :
         (first_window < anchor || first_window >= closure ||
          vowel_tract_append(part, anchor, first_window, closure, has_h))))
        return -1;
    if (has_fricative_coda) {
        if (reloq_german_terminal_fricative_append_marked(
                prepared->phones, prepared->phone_count, vowel, closure,
                prepared->mark, &part->stage[RELEASE].builder,
                &release_source, &fricative_end)) return -1;
    } else if (reloq_german_terminal_stop_tract_append(
                   prepared->phones, prepared->phone_count, vowel, closure,
                   &part->stage[STOP_TRACT].builder, &release_tract) ||
               reloq_german_terminal_source_append(
                   prepared->phones, prepared->phone_count, vowel, closure,
                   &part->stage[RELEASE].builder, &release_source) ||
               release_source != release_tract ||
               stop_burst_append(part, coda, closure, release_source) ||
               terminal_tract_return(part, release_source)) return -1;

    if (has_nasal_onset) {
        if (release_source > INT64_MAX - 460 ||
            reloq_german_initial_nasal_envelope_append(
                prepared->phones, prepared->phone_count, prepared->mark,
                0, anchor, closure, release_source,
                &part->stage[INITIAL_H].builder)) return -1;
        progress->planned_ticks = release_source + 460;
    } else if (has_voiced_stop) {
        if (release_source > INT64_MAX - 460 ||
            reloq_german_initial_voiced_stop_envelope_append(
                prepared->phones, prepared->phone_count, prepared->mark,
                0, first_window, anchor, closure, release_source,
                &part->stage[INITIAL_H].builder)) return -1;
        progress->planned_ticks = release_source + 460;
    } else if (reloq_german_phrase_f0_append(
                   prepared->phones, prepared->phone_count,
                   prepared->mark, has_h ? first_window : -1,
                   anchor, closure, release_source,
                   &part->stage[PHRASE_F0].builder,
                   &progress->planned_ticks)) return -1;
    if (has_fricative_coda && progress->planned_ticks != fricative_end)
        return -1;
    if (timing) *timing = (struct syllable_timing){
        first_window, has_initial_stop ? second_window : first_window,
        anchor, closure, release_source};
    return 0;
}

static int render_one(const unsigned char *utf8, size_t length,
                      struct reloq_session *session,
                      reloq_session_pcm_sink sink, void *context,
                      struct reloq_german_utterance_info *info)
{
    struct reloq_german_utterance_info progress = {0};
    if (!info || !session || !sink) return RELOQ_DE_UTTERANCE_INVALID;
    *info = progress;
    if (reloq_session_cancelled(session)) return RELOQ_DE_UTTERANCE_CANCELLED;
    struct prepared prepared;
    int parsed = prepare(utf8, length, &prepared, session_cancelled,
                         session);
    progress.error_offset = prepared.bad_offset;
    if (parsed) {
        *info = progress;
        return parsed;
    }
    progress.annotation_bytes = prepared.annotation_bytes;
    progress.phone_count = prepared.phone_count;
    if (reloq_session_cancelled(session)) return RELOQ_DE_UTTERANCE_CANCELLED;

    struct composition *part = calloc(1, sizeof *part);
    if (!part) return RELOQ_DE_UTTERANCE_MEMORY;
    int result = RELOQ_DE_UTTERANCE_MODEL;
    float defaults[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config voice_config;
    if (reloq_german_voice_defaults(defaults, fields, &voice_config)) goto done;
    if (compose_one(&prepared, part, &progress, NULL)) goto done;
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS];
    unsigned char mapped[RELOQ_PARAMETER_TRACKS];
    if (combine(part, defaults, fields, curves, mapped,
                &progress.target_count)) goto done;
    if (reloq_session_cancelled(session)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done;
    }
    struct reloq_curve_scheduler scheduler;
    struct reloq_curve_schedule schedule;
    if (reloq_curve_scheduler_begin(&scheduler, 0, 1, mapped) ||
        reloq_curve_scheduler_plan(&scheduler, curves,
                                   progress.planned_ticks, 1, &schedule))
        goto done;
    result = render_result(reloq_curve_pipeline_render(
        &schedule, RELOQ_GERMAN_VOICE_FORMANTS, session, sink, context,
        &progress.frames_completed));
    if (reloq_curve_scheduler_commit(&scheduler, progress.frames_completed) &&
        result == RELOQ_DE_UTTERANCE_OK)
        result = RELOQ_DE_UTTERANCE_RENDER;
done:
    *info = progress;
    free(part);
    return result;
}

static int render_within_word(const unsigned char *utf8, size_t length,
                              struct reloq_session *session,
                              reloq_session_pcm_sink sink, void *context,
                              struct reloq_german_utterance_info *info)
{
    if (!info || !session || !sink) return RELOQ_DE_UTTERANCE_INVALID;
    *info = (struct reloq_german_utterance_info){0};
    struct prepared_within_word prepared;
    int parsed = prepare_within_word(utf8, length, &prepared,
                                     session_cancelled, session);
    info->error_offset = prepared.bad_offset;
    if (parsed) return parsed;

    struct composition *part = calloc(1, sizeof *part);
    if (!part) return RELOQ_DE_UTTERANCE_MEMORY;
    struct reloq_german_utterance_info progress = {0};
    progress.annotation_bytes = prepared.annotation_bytes;
    progress.phone_count = prepared.phone_count;
    int result = RELOQ_DE_UTTERANCE_MODEL;
    float defaults[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    if (reloq_german_voice_defaults(defaults, fields, &config) ||
        begin_stage(&part->stage[0])) goto done_within_render;
    int composed;
    if (prepared.model == TWO_SYLLABLE_REDUCED_OPEN ||
        prepared.model == TWO_SYLLABLE_REDUCED_CLOSED) {
        struct reloq_german_reduced_landmarks landmarks;
        composed = prepared.model == TWO_SYLLABLE_REDUCED_OPEN ?
            reloq_german_reduced_open_compose(
                prepared.phones, prepared.phone_count,
                prepared.syllables, 2, &part->stage[0].builder,
                &landmarks) :
            reloq_german_reduced_closed_compose(
                prepared.phones, prepared.phone_count,
                prepared.syllables, 2, &part->stage[0].builder,
                &landmarks);
        if (!composed) progress.planned_ticks = landmarks.planned_ticks;
    } else {
        struct reloq_german_within_word_landmarks landmarks;
        composed = reloq_german_within_word_compose(
            prepared.phones, prepared.phone_count, prepared.syllables, 2,
            &part->stage[0].builder, &landmarks);
        if (!composed) progress.planned_ticks = landmarks.planned_ticks;
    }
    if (composed) {
        if (composed == 1) result = RELOQ_DE_UTTERANCE_UNCOVERED;
        goto done_within_render;
    }
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS];
    unsigned char mapped[RELOQ_PARAMETER_TRACKS];
    if (combine(part, defaults, fields, curves, mapped,
                &progress.target_count)) goto done_within_render;
    if (reloq_session_cancelled(session)) {
        result = RELOQ_DE_UTTERANCE_CANCELLED;
        goto done_within_render;
    }
    struct reloq_curve_scheduler scheduler;
    struct reloq_curve_schedule schedule;
    if (reloq_curve_scheduler_begin(&scheduler, 0, 1, mapped) ||
        reloq_curve_scheduler_plan(&scheduler, curves,
                                   progress.planned_ticks, 1, &schedule))
        goto done_within_render;
    result = render_result(reloq_curve_pipeline_render(
        &schedule, RELOQ_GERMAN_VOICE_FORMANTS, session, sink, context,
        &progress.frames_completed));
    if (reloq_curve_scheduler_commit(&scheduler, progress.frames_completed) &&
        result == RELOQ_DE_UTTERANCE_OK)
        result = RELOQ_DE_UTTERANCE_RENDER;
done_within_render:
    *info = progress;
    free(part);
    return result;
}

int reloq_german_voice_sentence_spans_checked(
    const unsigned char *utf8, size_t length,
    struct reloq_german_voice_span *spans, size_t capacity, size_t *count,
    size_t *bad_offset, reloq_german_cancel_check cancelled,
    void *cancel_context)
{
    if (bad_offset) *bad_offset = 0;
    if (!utf8 || !length || !count || (!spans && capacity)) return -1;
    if (cancelled && cancelled(cancel_context)) return -3;

    size_t needed = 0;
    int syntax = reloq_german_sentence_spans_checked(
        utf8, length, NULL, 0, &needed, cancelled, cancel_context);
    if (syntax < 0) return syntax;
    if (!syntax) {
        struct prepared single;
        int valid = prepare(utf8, length, &single, cancelled,
                            cancel_context);
        size_t connected_bad = 0;
        if (valid) {
            struct prepared_within_word compound;
            int within = prepare_within_word(
                utf8, length, &compound, cancelled, cancel_context);
            if (within == RELOQ_DE_UTTERANCE_OK)
                valid = RELOQ_DE_UTTERANCE_OK;
            else if (within == RELOQ_DE_UTTERANCE_CANCELLED ||
                     within == RELOQ_DE_UTTERANCE_MEMORY)
                valid = within;
        }
        if (valid != RELOQ_DE_UTTERANCE_OK &&
            valid != RELOQ_DE_UTTERANCE_CANCELLED &&
            valid != RELOQ_DE_UTTERANCE_MEMORY) {
            struct reloq_german_connected_plan pair;
            int pair_status = prepare_connected(
                utf8, length, &pair, &connected_bad,
                cancelled, cancel_context);
            if (pair_status != RELOQ_DE_UTTERANCE_UNCOVERED ||
                valid == RELOQ_DE_UTTERANCE_UNCOVERED)
                valid = pair_status;
        }
        if (bad_offset) *bad_offset = connected_bad ? connected_bad :
            single.bad_offset;
        if (valid) return valid == RELOQ_DE_UTTERANCE_UNCOVERED ? 0 :
            valid == RELOQ_DE_UTTERANCE_CANCELLED ? -3 :
            valid == RELOQ_DE_UTTERANCE_MEMORY ? -4 : -1;
        if (capacity < 1) {
            *count = 1;
            return -2;
        }
        if (cancelled && cancelled(cancel_context)) return -3;
        spans[0] = (struct reloq_german_voice_span){0, length};
        *count = 1;
        return 1;
    }

    if (needed > SIZE_MAX / sizeof(struct reloq_german_sentence_span))
        return -4;
    struct reloq_german_sentence_span *parts =
        malloc(needed * sizeof *parts);
    if (!parts) return -4;
    size_t filled = 0;
    syntax = reloq_german_sentence_spans_checked(
        utf8, length, parts, needed, &filled, cancelled, cancel_context);
    if (syntax != 1 || filled != needed) {
        free(parts);
        return syntax == 1 ? -1 : syntax;
    }
    for (size_t i = 0; i < needed; ++i) {
        struct prepared current;
        int valid = prepare(utf8 + parts[i].offset, parts[i].length,
                            &current, cancelled, cancel_context);
        if (valid) {
            struct prepared_within_word compound;
            int within = prepare_within_word(
                utf8 + parts[i].offset, parts[i].length, &compound,
                cancelled, cancel_context);
            if (within == RELOQ_DE_UTTERANCE_OK)
                valid = RELOQ_DE_UTTERANCE_OK;
            else if (within == RELOQ_DE_UTTERANCE_CANCELLED ||
                     within == RELOQ_DE_UTTERANCE_MEMORY)
                valid = within;
        }
        if (valid) {
            size_t connected_bad = 0;
            struct reloq_german_connected_plan pair;
            int pair_status = prepare_connected(
                utf8 + parts[i].offset, parts[i].length, &pair,
                &connected_bad, cancelled, cancel_context);
            int use_connected_error =
                pair_status != RELOQ_DE_UTTERANCE_UNCOVERED &&
                valid != RELOQ_DE_UTTERANCE_CANCELLED &&
                valid != RELOQ_DE_UTTERANCE_MEMORY;
            if (use_connected_error)
                valid = pair_status;
            if (bad_offset) *bad_offset = parts[i].offset +
                (use_connected_error ? connected_bad : current.bad_offset);
        }
        if (valid) {
            free(parts);
            return valid == RELOQ_DE_UTTERANCE_UNCOVERED ? 0 :
                valid == RELOQ_DE_UTTERANCE_CANCELLED ? -3 :
                valid == RELOQ_DE_UTTERANCE_MEMORY ? -4 : -1;
        }
    }
    if (cancelled && cancelled(cancel_context)) {
        free(parts);
        return -3;
    }
    if (capacity < needed) {
        *count = needed;
        free(parts);
        return -2;
    }
    for (size_t i = 0; i < needed; ++i)
        spans[i] = (struct reloq_german_voice_span){
            parts[i].offset, parts[i].length};
    *count = needed;
    free(parts);
    return 1;
}

int reloq_german_voice_sentence_spans(
    const unsigned char *utf8, size_t length,
    struct reloq_german_voice_span *spans, size_t capacity, size_t *count,
    size_t *bad_offset)
{
    return reloq_german_voice_sentence_spans_checked(
        utf8, length, spans, capacity, count, bad_offset, NULL, NULL);
}

int reloq_german_voice_supports(const unsigned char *utf8, size_t length,
                                size_t *bad_offset)
{
    size_t count = 0;
    int status = reloq_german_voice_sentence_spans(
        utf8, length, NULL, 0, &count, bad_offset);
    return status == -2 ? 1 : status;
}

int reloq_german_voice_render(const unsigned char *utf8, size_t length,
                              struct reloq_session *session,
                              reloq_session_pcm_sink sink, void *context,
                              struct reloq_german_utterance_info *info)
{
    if (!info || !session || !sink) return RELOQ_DE_UTTERANCE_INVALID;
    *info = (struct reloq_german_utterance_info){0};
    if (reloq_session_cancelled(session)) return RELOQ_DE_UTTERANCE_CANCELLED;

    size_t needed = 0, bad = 0;
    int valid = reloq_german_voice_sentence_spans_checked(
        utf8, length, NULL, 0, &needed, &bad, session_cancelled,
        session);
    if (valid != -2) {
        info->error_offset = bad;
        return valid == 0 ? RELOQ_DE_UTTERANCE_UNCOVERED :
            valid == -3 ? RELOQ_DE_UTTERANCE_CANCELLED :
            valid == -4 ? RELOQ_DE_UTTERANCE_MEMORY :
            RELOQ_DE_UTTERANCE_INVALID;
    }
    if (needed > SIZE_MAX / sizeof(struct reloq_german_voice_span))
        return RELOQ_DE_UTTERANCE_MEMORY;
    struct reloq_german_voice_span *parts = malloc(needed * sizeof *parts);
    if (!parts) return RELOQ_DE_UTTERANCE_MEMORY;
    size_t filled = 0;
    valid = reloq_german_voice_sentence_spans_checked(
        utf8, length, parts, needed, &filled, &bad, session_cancelled,
        session);
    if (valid != 1 || filled != needed) {
        info->error_offset = bad;
        free(parts);
        return valid == 0 ? RELOQ_DE_UTTERANCE_UNCOVERED :
            valid == -3 ? RELOQ_DE_UTTERANCE_CANCELLED :
            valid == -4 ? RELOQ_DE_UTTERANCE_MEMORY :
            RELOQ_DE_UTTERANCE_INVALID;
    }

    struct reloq_german_utterance_info progress = {0};
    int result = RELOQ_DE_UTTERANCE_OK;
    for (size_t i = 0; i < needed; ++i) {
        if (reloq_session_cancelled(session)) {
            result = RELOQ_DE_UTTERANCE_CANCELLED;
            break;
        }
        struct reloq_german_utterance_info fragment = {0};
        struct prepared single;
        int kind = prepare(utf8 + parts[i].offset, parts[i].length,
                           &single, session_cancelled, session);
        if (kind == RELOQ_DE_UTTERANCE_OK)
            result = render_one(utf8 + parts[i].offset, parts[i].length,
                                session, sink, context, &fragment);
        else {
            struct prepared_within_word compound;
            int within = prepare_within_word(
                utf8 + parts[i].offset, parts[i].length,
                &compound, session_cancelled, session);
            result = within == RELOQ_DE_UTTERANCE_OK ?
                render_within_word(utf8 + parts[i].offset, parts[i].length,
                                   session, sink, context, &fragment) :
                render_connected(utf8 + parts[i].offset, parts[i].length,
                                 session, sink, context, &fragment);
        }
        progress.annotation_bytes += fragment.annotation_bytes;
        progress.phone_count += fragment.phone_count;
        progress.target_count += fragment.target_count;
        progress.planned_ticks += fragment.planned_ticks;
        progress.frames_completed += fragment.frames_completed;
        if (result != RELOQ_DE_UTTERANCE_OK) {
            progress.error_offset = parts[i].offset + fragment.error_offset;
            break;
        }
    }
    *info = progress;
    free(parts);
    return result;
}
