/* ReLoq C output module for Speech Dispatcher.
 *
 * Speech Dispatcher owns audio playback. This executable receives the module
 * protocol on standard input, synthesizes with the ReLoq C engine linked into
 * it, and returns 16-bit mono PCM, index marks and events to the server.
 *
 * eciSynthesize runs synchronously in ReLoq C, so the server's STOP and
 * PAUSE requests are read from the audio callback while an utterance is being
 * rendered, as spd_module_main.h describes for module_process(fd, 0).
 *
 * The SSML reader and the punctuation filter for punctuation mode "none" are
 * adapted from OpenEVV's Speech Dispatcher module (speechd/openevv.c,
 * https://github.com/Mudb0y/openevv), MIT License,
 * Copyright (c) 2026 Stanislaw Przedzinkowski. See
 * vendor/openevv61/LICENSE for the full MIT terms.
 *
 * ReLoq C
 * Copyright (C) 2026 Karl Eick and ReLoq C contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <spd_module_main.h>

#include "reloq_eci.h"

enum { PRESET_COUNT = 8 };

typedef struct {
    int id;
    const char *locale;
    const char *name;
} Language;

/* The fifteen language variants offered by the NVDA add-on. Taiwanese
 * Mandarin (0x60001) is an alias of the Mandarin module in the engine; a
 * request for zh-TW selects Mandarin through the language root. */
static const Language languages[] = {
    { 0x00010000, "en-US", "American English" },
    { 0x00010001, "en-GB", "British English" },
    { 0x00020000, "es-ES", "Castilian Spanish" },
    { 0x00020001, "es-MX", "Latin American Spanish" },
    { 0x00030000, "fr-FR", "French" },
    { 0x00030001, "fr-CA", "French Canadian" },
    { 0x00040000, "de-DE", "German" },
    { 0x00050000, "it-IT", "Italian" },
    { 0x00060000, "zh-CN", "Mandarin Chinese" },
    { 0x00070000, "pt-BR", "Brazilian Portuguese" },
    { 0x00080000, "ja-JP", "Japanese" },
    { 0x00090000, "fi-FI", "Finnish" },
    { 0x000a0000, "ko-KR", "Korean" },
    { 0x00120000, "el-GR", "Greek" },
    { 0x00130000, "an-ES", "Aragonese" },
};
#define LANGUAGE_COUNT (sizeof(languages) / sizeof(languages[0]))

/* The NVDA add-on's eight shared presets (nvda/_reloq_languages.py). */
typedef struct {
    const char *name;
    int gender, headSize, pitch, inflection, roughness, breathiness, volume;
} Preset;

static const Preset presets[PRESET_COUNT] = {
    { "Reed",    0, 50, 65, 30,  0,  0, 100 },
    { "Shelly",  1, 50, 81, 30,  0, 50, 100 },
    { "Bobby",   1, 22, 93, 35,  0,  0, 100 },
    { "Rocko",   0, 86, 56, 47,  0,  0, 100 },
    { "Glen",    0, 50, 69, 34,  0,  0, 100 },
    { "Sandy",   1, 56, 89, 35,  0, 40, 100 },
    { "Grandma", 1, 45, 68, 30,  3, 40, 100 },
    { "Grandpa", 0, 30, 61, 44, 18, 20, 100 },
};

/* ECI sample-rate codes as the NVDA add-on numbers them. */
static const struct { int hertz, code; } sampleRates[] = {
    { 8000, 0 }, { 11025, 1 }, { 16000, 3 }, { 22050, 2 },
    { 32000, 4 }, { 44100, 5 }, { 48000, 6 },
};

typedef struct {
    int rate;
    int pitch;
    int pitchRange;
    int volume;
    SPDPunctuation punctuation;
    SPDSpelling spelling;
    SPDCapitalLetters capitals;
    SPDVoiceType voiceType;
    char *language;
    char *synthesisVoice;
} ModuleSettings;

typedef struct {
    int sampleRate;
    int sampleRateCode;
    int pauseMode;
    int abbreviations;
    int phrasePrediction;
    int originalQuestions;
    int mellowMode;
    int acousticClock;
    int wordSegmentation;
    int camelCase;
    int capitalPitchChange;
    char *dictionaryDirectory;
} ModuleConfig;

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} TextBuffer;

static ECIHand engines[LANGUAGE_COUNT];
static int available[LANGUAGE_COUNT];
static size_t currentLanguage = SIZE_MAX;
static short *audioFrame;
static int audioFrameSamples;
static SPDVoice **speechdVoices;
static char **markNames;
static size_t markCount;
static char *pendingIcon;
static int queuedText;
static int synthesizing;
static volatile int stopRequested;
static volatile int pauseRequested;
static volatile int pauseIndexReached;
static int debugEnabled;
static int logLevel;
static FILE *debugFile;

static ModuleConfig config = {
    .sampleRate = 11025,
    .sampleRateCode = 1,
    .pauseMode = 0,
    .abbreviations = 1,
    .phrasePrediction = 0,
    .originalQuestions = 0,
    .mellowMode = 0,
    .acousticClock = 2,
    .wordSegmentation = 0,
    .camelCase = 0,
    .capitalPitchChange = 20,
    .dictionaryDirectory = NULL,
};

static ModuleSettings settings = {
    .rate = 0,
    .pitch = 0,
    .pitchRange = 0,
    .volume = 0,
    .punctuation = SPD_PUNCT_NONE,
    .spelling = SPD_SPELL_OFF,
    .capitals = SPD_CAP_NONE,
    .voiceType = SPD_MALE1,
};

static void debug_log(const char *format, const char *value)
{
    FILE *out;

    if (!debugEnabled)
        return;
    out = debugFile ? debugFile : stderr;
    fputs("ReLoq C: ", out);
    fprintf(out, format, value ? value : "");
    fputc('\n', out);
    fflush(out);
}

static void warn(const char *format, const char *value)
{
    fputs("ReLoq C: ", stderr);
    fprintf(stderr, format, value ? value : "");
    fputc('\n', stderr);
}

static char *copy_string(const char *value)
{
    char *copy;

    if (!value || strcmp(value, "NULL") == 0)
        return NULL;
    copy = malloc(strlen(value) + 1);
    if (copy)
        strcpy(copy, value);
    return copy;
}

static int parse_number(const char *value, int minimum, int maximum, int *out)
{
    char *end;
    long number;

    errno = 0;
    number = strtol(value, &end, 10);
    if (errno || end == value || *end || number < minimum || number > maximum)
        return -1;
    *out = (int)number;
    return 0;
}

/* ---------------------------------------------------------------------- */
/* Languages and voices                                                    */

/* Whether two locale spellings are equal, treating '_' as '-' and ignoring
 * case: Speech Dispatcher clients send en, en-us, en_US and EN-US alike. */
static int locale_equal(const char *a, const char *b, size_t length)
{
    size_t i;

    for (i = 0; i < length; i++) {
        char x = a[i] == '_' ? '-' : a[i];
        char y = b[i] == '_' ? '-' : b[i];

        if (tolower((unsigned char)x) != tolower((unsigned char)y))
            return 0;
        if (!x)
            return 1;
    }
    return 1;
}

static size_t language_for(const char *requested)
{
    size_t i, root;

    if (!requested || !*requested)
        return SIZE_MAX;
    for (i = 0; i < LANGUAGE_COUNT; i++)
        if (available[i] && locale_equal(requested, languages[i].locale,
                                         strlen(languages[i].locale) + 1))
            return i;
    root = strcspn(requested, "-_");
    if (root == 0)
        return SIZE_MAX;
    /* A regional request with no exact variant, or a bare language, keeps
     * the current variant of that language when there is one. */
    if (currentLanguage < LANGUAGE_COUNT
        && strcspn(languages[currentLanguage].locale, "-") == root
        && locale_equal(requested, languages[currentLanguage].locale, root))
        return currentLanguage;
    for (i = 0; i < LANGUAGE_COUNT; i++)
        if (available[i] && strcspn(languages[i].locale, "-") == root
            && locale_equal(requested, languages[i].locale, root))
            return i;
    return SIZE_MAX;
}

static size_t default_language(void)
{
    size_t i;

    for (i = 0; i < LANGUAGE_COUNT; i++)
        if (available[i])
            return i;
    return SIZE_MAX;
}

/* A voice is named "<Preset> (<Language>)", for example "Reed (German)".
 * The name is unique across languages and reads well in a voice list. */
static int voice_name_matches(const char *requested, size_t language,
                              size_t preset)
{
    size_t nameLength = strlen(presets[preset].name);
    const char *rest;

    if (strncasecmp(requested, presets[preset].name, nameLength))
        return 0;
    rest = requested + nameLength;
    if (rest[0] != ' ' || rest[1] != '(')
        return 0;
    rest += 2;
    nameLength = strlen(languages[language].name);
    return !strncasecmp(rest, languages[language].name, nameLength)
        && rest[nameLength] == ')' && rest[nameLength + 1] == 0;
}

static int find_voice(const char *requested, size_t *language, size_t *preset)
{
    size_t i, j;

    if (!requested)
        return 0;
    for (i = 0; i < LANGUAGE_COUNT; i++) {
        if (!available[i])
            continue;
        for (j = 0; j < PRESET_COUNT; j++)
            if (voice_name_matches(requested, i, j)) {
                *language = i;
                *preset = j;
                return 1;
            }
    }
    return 0;
}

static size_t requested_language(void)
{
    size_t language, preset;

    if (find_voice(settings.synthesisVoice, &language, &preset))
        return language;
    language = language_for(settings.language);
    if (language < LANGUAGE_COUNT)
        return language;
    if (currentLanguage < LANGUAGE_COUNT)
        return currentLanguage;
    return default_language();
}

static size_t requested_preset(void)
{
    size_t language, preset;

    if (find_voice(settings.synthesisVoice, &language, &preset))
        return preset;
    if (settings.synthesisVoice) {
        /* A bare preset name selects it in the requested language. */
        for (preset = 0; preset < PRESET_COUNT; preset++)
            if (!strcasecmp(settings.synthesisVoice, presets[preset].name))
                return preset;
    }
    switch (settings.voiceType) {
    case SPD_MALE2: return 3;          /* Rocko */
    case SPD_MALE3: return 4;          /* Glen */
    case SPD_FEMALE1: return 1;        /* Shelly */
    case SPD_FEMALE2: return 5;        /* Sandy */
    case SPD_FEMALE3: return 6;        /* Grandma */
    case SPD_CHILD_MALE:
    case SPD_CHILD_FEMALE: return 2;   /* Bobby */
    case SPD_MALE1:
    default: return 0;                 /* Reed */
    }
}

/* Speech Dispatcher sends -100..100 with 0 as the voice's normal value. */
static int relative_value(int requested, int baseline, int maximum)
{
    if (requested < 0)
        return (requested + 100) * baseline / 100;
    return baseline + requested * (maximum - baseline) / 100;
}

/* -100..0 covers ECI speed 0..50 and 0..100 covers 50..250, the same upper
 * limit as the NVDA add-on's rate boost. 0 is the engine's normal speed. */
static int native_speed(int requested)
{
    return requested < 0 ? 50 + requested / 2 : 50 + requested * 2;
}

static int set_param(ECIHand handle, int parameter, int value)
{
    return eciSetParam(handle, parameter, value) >= 0;
}

/* ---------------------------------------------------------------------- */
/* Engine callback                                                         */

static int ECICALL on_message(ECIHand handle, int message, int parameter,
                              void *data)
{
    (void)handle;
    (void)data;

    /* Synthesis is synchronous: let the server tell us to stop or pause. */
    if (!stopRequested)
        module_process(STDIN_FILENO, 0);
    if (stopRequested || (pauseRequested && pauseIndexReached))
        return eciDataAbort;
    if (message == eciWaveformBuffer && parameter > 0) {
        const uint16_t endianProbe = 1;
        AudioTrack track = {
            .bits = 16,
            .num_channels = 1,
            .sample_rate = config.sampleRate,
            .num_samples = parameter,
            .samples = audioFrame,
        };

        module_tts_output_server(&track,
            *(const unsigned char *)&endianProbe ? SPD_AUDIO_LE
                                                 : SPD_AUDIO_BE);
    } else if (message == eciIndexReply && parameter > 0
               && (size_t)parameter <= markCount) {
        const char *name = markNames[parameter - 1];

        if (!name) {
            module_report_icon("capital");
        } else {
            module_report_index_mark(name);
            if (pauseRequested && !strncmp(name, "__spd_", 6)) {
                pauseIndexReached = 1;
                return eciDataAbort;
            }
        }
    }
    return eciDataProcessed;
}

static ECIHand open_engine(size_t language)
{
    ECIHand handle;

    if (engines[language])
        return engines[language];
    handle = eciNewEx(languages[language].id);
    if (!handle)
        return NULL;
    eciRegisterCallback(handle, on_message, NULL);
    if (eciSetOutputBuffer(handle, audioFrameSamples, audioFrame) <= 0
        || !set_param(handle, RELOQ_ECI_VOICE_MODE, RELOQ_ECI_VOICE_GENERIC)
        || !set_param(handle, RELOQ_ECI_SAMPLE_RATE, config.sampleRateCode)
        || !set_param(handle, RELOQ_ECI_PROCESS_TAGS, 0)
        || !set_param(handle, RELOQ_ECI_DICTIONARY, config.abbreviations ? 0 : 1)
        || !set_param(handle, RELOQ_ECI_PHRASE_PREDICTION, config.phrasePrediction)
        || !set_param(handle, RELOQ_ECI_ORIGINAL_QUESTIONS, config.originalQuestions)
        || !set_param(handle, RELOQ_ECI_MELLOW_MODE, config.mellowMode)
        || !set_param(handle, RELOQ_ECI_PAUSE_MODE, config.pauseMode)
        || !set_param(handle, RELOQ_ECI_ORIGINAL_ACOUSTIC_CLOCK, config.acousticClock)
        || !set_param(handle, RELOQ_ECI_WORD_SEGMENTATION, config.wordSegmentation)
        || !set_param(handle, RELOQ_ECI_CAMEL_CASE, config.camelCase)) {
        eciDelete(handle);
        return NULL;
    }
    if (config.dictionaryDirectory
        && !eciSetDictionaryPath(handle, config.dictionaryDirectory))
        warn("dictionaries in %s could not be loaded; speaking without them",
             config.dictionaryDirectory);
    engines[language] = handle;
    debug_log("opened %s", languages[language].locale);
    return handle;
}

static ECIHand configure_engine(int capitalLetter)
{
    size_t language = requested_language();
    const Preset *preset = &presets[requested_preset()];
    ECIHand handle;
    int pitch;

    if (language >= LANGUAGE_COUNT || !(handle = open_engine(language)))
        return NULL;
    currentLanguage = language;
    pitch = relative_value(settings.pitch, preset->pitch, 100);
    if (capitalLetter && settings.capitals == SPD_CAP_SPELL)
        pitch = pitch + config.capitalPitchChange > 100
              ? 100 : pitch + config.capitalPitchChange;
    if (!set_param(handle, RELOQ_ECI_GENDER, preset->gender)
        || !set_param(handle, RELOQ_ECI_HEAD_SIZE, preset->headSize)
        || !set_param(handle, RELOQ_ECI_ROUGHNESS, preset->roughness)
        || !set_param(handle, RELOQ_ECI_BREATHINESS, preset->breathiness)
        || !set_param(handle, RELOQ_ECI_NATIVE_PITCH, pitch)
        || !set_param(handle, RELOQ_ECI_NATIVE_INFLECTION,
                      relative_value(settings.pitchRange, preset->inflection, 100))
        || !set_param(handle, RELOQ_ECI_NATIVE_VOLUME,
                      relative_value(settings.volume, preset->volume, 100))
        || !set_param(handle, RELOQ_ECI_NATIVE_SPEED, native_speed(settings.rate)))
        return NULL;
    return handle;
}

/* ---------------------------------------------------------------------- */
/* Text                                                                    */

static void clear_marks(void)
{
    size_t i;

    for (i = 0; i < markCount; i++)
        free(markNames[i]);
    free(markNames);
    markNames = NULL;
    markCount = 0;
}

static int add_mark(ECIHand handle, const char *name, size_t length)
{
    char **names;
    char *copy = NULL;

    if (markCount >= (size_t)INT32_MAX)
        return -1;
    if (name) {
        copy = malloc(length + 1);
        if (!copy)
            return -1;
        memcpy(copy, name, length);
        copy[length] = 0;
    }
    names = realloc(markNames, (markCount + 1) * sizeof(*markNames));
    if (!names) {
        free(copy);
        return -1;
    }
    markNames = names;
    markNames[markCount++] = copy;
    return eciInsertIndex(handle, (int)markCount) ? 0 : -1;
}

static int buffer_append(TextBuffer *buffer, const char *text, size_t length)
{
    size_t needed = buffer->length + length + 1;

    if (needed > buffer->capacity) {
        size_t capacity = buffer->capacity ? buffer->capacity : 128;
        char *data;

        while (capacity < needed)
            capacity *= 2;
        data = realloc(buffer->data, capacity);
        if (!data)
            return -1;
        buffer->data = data;
        buffer->capacity = capacity;
    }
    memcpy(buffer->data + buffer->length, text, length);
    buffer->length += length;
    buffer->data[buffer->length] = 0;
    return 0;
}

/* Decode one UTF-8 scalar. Malformed input yields U+FFFD and one byte. */
static uint32_t utf8_next(const unsigned char *at, size_t *length)
{
    if (at[0] < 0x80) {
        *length = 1;
        return at[0];
    }
    if ((at[0] & 0xe0) == 0xc0 && (at[1] & 0xc0) == 0x80 && at[0] >= 0xc2) {
        *length = 2;
        return ((uint32_t)(at[0] & 0x1f) << 6) | (at[1] & 0x3f);
    }
    if ((at[0] & 0xf0) == 0xe0 && (at[1] & 0xc0) == 0x80
        && (at[2] & 0xc0) == 0x80) {
        *length = 3;
        return ((uint32_t)(at[0] & 0x0f) << 12)
             | ((uint32_t)(at[1] & 0x3f) << 6) | (at[2] & 0x3f);
    }
    if ((at[0] & 0xf8) == 0xf0 && (at[1] & 0xc0) == 0x80
        && (at[2] & 0xc0) == 0x80 && (at[3] & 0xc0) == 0x80) {
        *length = 4;
        return ((uint32_t)(at[0] & 0x07) << 18)
             | ((uint32_t)(at[1] & 0x3f) << 12)
             | ((uint32_t)(at[2] & 0x3f) << 6) | (at[3] & 0x3f);
    }
    *length = 1;
    return 0xfffd;
}

static int utf8_encode(uint32_t codepoint, char *out)
{
    if (codepoint < 0x80) {
        out[0] = (char)codepoint;
        return 1;
    }
    if (codepoint < 0x800) {
        out[0] = (char)(0xc0 | (codepoint >> 6));
        out[1] = (char)(0x80 | (codepoint & 0x3f));
        return 2;
    }
    if (codepoint < 0x10000) {
        if (codepoint >= 0xd800 && codepoint <= 0xdfff)
            return 0;
        out[0] = (char)(0xe0 | (codepoint >> 12));
        out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        out[2] = (char)(0x80 | (codepoint & 0x3f));
        return 3;
    }
    if (codepoint < 0x110000) {
        out[0] = (char)(0xf0 | (codepoint >> 18));
        out[1] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
        out[2] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        out[3] = (char)(0x80 | (codepoint & 0x3f));
        return 4;
    }
    return 0;
}

/* Upper-case letters of the scripts the engine reads. */
static int is_upper(uint32_t c)
{
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c >= 0xc0 && c <= 0xde && c != 0xd7)
        return 1;
    /* Latin Extended-A alternates capital and small letters; the pairing
     * shifts by one after U+0138 and again after U+0148 and U+0178. */
    if ((c >= 0x100 && c <= 0x137) || (c >= 0x14a && c <= 0x177))
        return !(c & 1);
    if ((c >= 0x139 && c <= 0x148) || (c >= 0x179 && c <= 0x17e))
        return c & 1;
    if (c == 0x178)
        return 1;
    if ((c >= 0x391 && c <= 0x3a9) || (c >= 0x388 && c <= 0x38f) || c == 0x386)
        return 1;
    if (c >= 0x400 && c <= 0x42f)
        return 1;
    return 0;
}

static int is_letter_or_digit(uint32_t c)
{
    return c >= 0x80 || isalnum((int)c);
}

static int single_scalar(const char *text, uint32_t *scalar)
{
    size_t length;

    if (!*text)
        return 0;
    *scalar = utf8_next((const unsigned char *)text, &length);
    return text[length] == 0;
}

/* Punctuation mode "none". With server-side symbol preprocessing Speech
 * Dispatcher has already inserted the names it wants spoken and keeps some
 * punctuation for prosody; the engine would otherwise name many of those
 * retained symbols. Sentence punctuation, dashes, currency, apostrophes
 * inside words and brackets enclosing an aside stay. Only ASCII bytes are
 * judged; every byte of a multi-byte UTF-8 character is kept. */
static int word_byte(unsigned char value)
{
    return value >= 0x80 || isalnum(value);
}

static int opening_bracket(unsigned char value)
{
    return value == '(' || value == '[' || value == '{' || value == '<';
}

static int closing_bracket(unsigned char value)
{
    return value == ')' || value == ']' || value == '}' || value == '>';
}

static int bracket_kept(const unsigned char *text, const unsigned char *at)
{
    unsigned char value = *at;
    const unsigned char *scan;
    int depth = 0;

    if (opening_bracket(value)) {
        if (at[1] == 0 || at[1] == ' ' || (at > text && word_byte(at[-1])))
            return 0;
        if (value != '<')
            return 1;
        for (scan = at + 1; *scan; scan++) {
            if (*scan == '<')
                depth++;
            else if (*scan == '>' && depth-- == 0)
                return scan[-1] != ' ';
        }
        return 0;
    }
    if (at == text || at[-1] == ' ' || word_byte(at[1]))
        return 0;
    if (value != '>')
        return 1;
    for (scan = at; scan > text; scan--) {
        if (scan[-1] == '>')
            depth++;
        else if (scan[-1] == '<' && depth-- == 0)
            return scan[0] != ' ';
    }
    return 0;
}

static void suppress_spoken_punctuation(char *text)
{
    unsigned char *at;
    int changed;

    for (at = (unsigned char *)text; *at; at++) {
        unsigned char value = *at;

        if (value >= 0x80 || !ispunct(value))
            continue;
        if (value == '\'' && at > (unsigned char *)text
            && word_byte(at[-1]) && word_byte(at[1]))
            continue;
        if (value == '-' || value == '$' || opening_bracket(value)
            || closing_bracket(value) || value == ',' || value == '.'
            || value == ';' || value == ':' || value == '?' || value == '!')
            continue;
        *at = ' ';
    }
    /* Brackets last, until nothing more goes: removing one can leave its
     * partner standing on its own. */
    do {
        changed = 0;
        for (at = (unsigned char *)text; *at; at++)
            if ((opening_bracket(*at) || closing_bracket(*at))
                && !bracket_kept((unsigned char *)text, at)) {
                *at = ' ';
                changed = 1;
            }
    } while (changed);
}

static int add_text(ECIHand handle, const char *text, size_t length,
                    int characterMode)
{
    if (!length)
        return 0;
    queuedText = 1;
    return eciAddTextUtf8(handle, text, length, characterMode) ? 0 : -1;
}

/* Queue text, reporting the capital sound icon before each word that starts
 * with a capital when the client asked for icons. */
static int queue_text(ECIHand handle, char *text, int characterMode,
                      int suppressPunctuation)
{
    const unsigned char *at = (const unsigned char *)text;
    const char *segment = text;
    uint32_t previous = ' ';

    if (suppressPunctuation)
        suppress_spoken_punctuation(text);
    if (settings.capitals != SPD_CAP_ICON)
        return add_text(handle, text, strlen(text), characterMode);
    while (*at) {
        size_t length;
        uint32_t scalar = utf8_next(at, &length);

        if (is_upper(scalar) && (characterMode || !is_letter_or_digit(previous))) {
            if (add_text(handle, segment, (size_t)((const char *)at - segment),
                         characterMode) != 0
                || add_mark(handle, NULL, 0) != 0)
                return -1;
            segment = (const char *)at;
        }
        previous = scalar;
        at += length;
    }
    return add_text(handle, segment, strlen(segment), characterMode);
}

static int flush_text(ECIHand handle, TextBuffer *buffer, int characterMode,
                      int suppressPunctuation)
{
    int result;

    if (!buffer->length)
        return 0;
    result = queue_text(handle, buffer->data, characterMode, suppressPunctuation);
    buffer->length = 0;
    buffer->data[0] = 0;
    return result;
}

static int append_entity(TextBuffer *buffer, const char *entity, size_t length)
{
    static const struct { const char *name; const char *value; } entities[] = {
        { "amp", "&" }, { "lt", "<" }, { "gt", ">" },
        { "quot", "\"" }, { "apos", "'" }, { "nbsp", " " },
    };
    size_t i;

    if (length > 1 && entity[0] == '#') {
        char digits[16];
        char encoded[4];
        char *end;
        unsigned long value;
        int base = 10;
        size_t start = 1;
        int bytes;

        if (entity[1] == 'x' || entity[1] == 'X') {
            base = 16;
            start = 2;
        }
        if (length - start == 0 || length - start >= sizeof(digits))
            return buffer_append(buffer, " ", 1);
        memcpy(digits, entity + start, length - start);
        digits[length - start] = 0;
        value = strtoul(digits, &end, base);
        bytes = *end || value == 0 ? 0 : utf8_encode((uint32_t)value, encoded);
        return bytes ? buffer_append(buffer, encoded, (size_t)bytes)
                     : buffer_append(buffer, " ", 1);
    }
    for (i = 0; i < sizeof(entities) / sizeof(entities[0]); i++)
        if (strlen(entities[i].name) == length
            && !strncmp(entity, entities[i].name, length))
            return buffer_append(buffer, entities[i].value,
                                 strlen(entities[i].value));
    return buffer_append(buffer, " ", 1);
}

static const char *find_attribute(const char *tag, size_t length,
                                  const char *attribute, size_t *valueLength)
{
    const char *at = tag;
    const char *end = tag + length;
    size_t nameLength = strlen(attribute);

    while (at + nameLength < end) {
        if (at > tag && isspace((unsigned char)at[-1])
            && !strncasecmp(at, attribute, nameLength)
            && (at[nameLength] == '=' || isspace((unsigned char)at[nameLength]))) {
            const char *value = at + nameLength;
            const char *close;
            char quote;

            while (value < end && isspace((unsigned char)*value))
                value++;
            if (value >= end || *value != '=') {
                at++;
                continue;
            }
            value++;
            while (value < end && isspace((unsigned char)*value))
                value++;
            if (value >= end || (*value != '\'' && *value != '"')) {
                at++;
                continue;
            }
            quote = *value++;
            close = memchr(value, quote, (size_t)(end - value));
            if (!close)
                return NULL;
            *valueLength = (size_t)(close - value);
            return value;
        }
        at++;
    }
    return NULL;
}

static int tag_matches(const char *tag, size_t length, const char *name)
{
    size_t nameLength = strlen(name);
    char next;

    if (length < nameLength + 2 || tag[0] != '<' || tag[1] == '/')
        return 0;
    if (strncasecmp(tag + 1, name, nameLength))
        return 0;
    next = tag[nameLength + 1];
    return next == '>' || next == '/' || isspace((unsigned char)next);
}

static int contains_tag(const char *text, const char *name)
{
    const char *tag = text;

    while ((tag = strchr(tag, '<')) != NULL) {
        const char *end = strchr(tag, '>');

        if (!end)
            return 0;
        if (tag_matches(tag, (size_t)(end - tag + 1), name))
            return 1;
        tag = end + 1;
    }
    return 0;
}

static int queue_ssml(ECIHand handle, const char *data, int characterMode,
                      int suppressPunctuation)
{
    TextBuffer text = { 0 };
    const char *at = data;
    int result = 0;

    while (*at && result == 0) {
        if (*at == '<') {
            const char *end = strchr(at, '>');
            size_t length;

            if (!end) {
                result = buffer_append(&text, at, strlen(at));
                break;
            }
            length = (size_t)(end - at + 1);
            if (tag_matches(at, length, "mark")) {
                size_t nameLength;
                const char *name = find_attribute(at, length, "name",
                                                  &nameLength);

                if (name) {
                    result = flush_text(handle, &text, characterMode,
                                        suppressPunctuation);
                    if (result == 0)
                        result = add_mark(handle, name, nameLength);
                }
            } else if (tag_matches(at, length, "break")) {
                if (!characterMode)
                    result = buffer_append(&text, ", ", 2);
            } else if (!characterMode) {
                /* Other elements such as speak, s, p and voice keep words
                 * apart; their content is spoken. In character mode an
                 * added space would itself be named. */
                result = buffer_append(&text, " ", 1);
            }
            at = end + 1;
        } else if (*at == '&') {
            const char *end = strchr(at + 1, ';');

            if (end && end - at <= 12) {
                result = append_entity(&text, at + 1, (size_t)(end - at - 1));
                at = end + 1;
            } else {
                result = buffer_append(&text, at++, 1);
            }
        } else {
            size_t length = strcspn(at, "<&");

            result = buffer_append(&text, at, length);
            at += length;
        }
    }
    if (result == 0)
        result = flush_text(handle, &text, characterMode, suppressPunctuation);
    free(text.data);
    return result;
}

/* Key names such as "shift_a", "KP_Enter" or "control-alt-delete". */
static char *key_text(const char *data)
{
    TextBuffer text = { 0 };
    const char *at = data;

    if (!strncmp(at, "KP_", 3)) {
        if (buffer_append(&text, "keypad ", 7) != 0)
            goto failed;
        at += 3;
    }
    while (*at) {
        char value = *at == '_' || *at == '-' ? ' ' : *at;

        if (buffer_append(&text, &value, 1) != 0)
            goto failed;
        at++;
    }
    if (!text.data && buffer_append(&text, "", 0) != 0)
        goto failed;
    return text.data;

failed:
    free(text.data);
    return NULL;
}

/* ---------------------------------------------------------------------- */
/* Module interface                                                        */

static int config_flag(const char *key, const char *value, int *out)
{
    if (parse_number(value, 0, 1, out) == 0)
        return 0;
    warn("ignoring the invalid %s line; use 0 or 1", key);
    return -1;
}

int module_config(const char *configFile)
{
    FILE *file;
    char line[1024];

    if (!configFile || !*configFile)
        return 0;
    file = fopen(configFile, "r");
    if (!file)
        return 0;
    while (fgets(line, sizeof(line), file)) {
        char *key = line;
        char *value;
        int number;

        while (isspace((unsigned char)*key))
            key++;
        if (!*key || *key == '#')
            continue;
        value = key + strcspn(key, " \t\r\n");
        if (*value)
            *value++ = 0;
        while (isspace((unsigned char)*value))
            value++;
        value[strcspn(value, "\r\n")] = 0;
        while (*value && isspace((unsigned char)value[strlen(value) - 1]))
            value[strlen(value) - 1] = 0;
        if (*value == '"' && strlen(value) >= 2 && value[strlen(value) - 1] == '"') {
            value[strlen(value) - 1] = 0;
            value++;
        }
        /* An invalid line is reported and ignored: refusing the whole file
         * would leave the listener with no voice over one setting. */
        if (!strcasecmp(key, "Debug")) {
            config_flag(key, value, &debugEnabled);
        } else if (!strcasecmp(key, "SampleRate")) {
            size_t i;
            int found = 0;

            if (parse_number(value, 1, 192000, &number) == 0)
                for (i = 0; i < sizeof(sampleRates) / sizeof(sampleRates[0]); i++)
                    if (sampleRates[i].hertz == number) {
                        config.sampleRate = number;
                        config.sampleRateCode = sampleRates[i].code;
                        found = 1;
                    }
            if (!found)
                warn("ignoring SampleRate %s; use 8000, 11025, 16000, 22050, "
                     "32000, 44100 or 48000", value);
        } else if (!strcasecmp(key, "PauseMode")) {
            if (parse_number(value, 0, 2, &number) == 0)
                config.pauseMode = number;
            else
                warn("ignoring PauseMode %s; use 0, 1 or 2", value);
        } else if (!strcasecmp(key, "AbbreviationDictionary")) {
            config_flag(key, value, &config.abbreviations);
        } else if (!strcasecmp(key, "PhrasePrediction")) {
            config_flag(key, value, &config.phrasePrediction);
        } else if (!strcasecmp(key, "OriginalQuestions")) {
            config_flag(key, value, &config.originalQuestions);
        } else if (!strcasecmp(key, "MellowMode")) {
            config_flag(key, value, &config.mellowMode);
        } else if (!strcasecmp(key, "WordSegmentation")) {
            config_flag(key, value, &config.wordSegmentation);
        } else if (!strcasecmp(key, "CamelCase")) {
            config_flag(key, value, &config.camelCase);
        } else if (!strcasecmp(key, "AcousticTiming")) {
            if (!strcasecmp(value, "auto"))
                config.acousticClock = 2;
            else if (!strcmp(value, "1"))
                config.acousticClock = 0;
            else if (!strcmp(value, "5"))
                config.acousticClock = 1;
            else
                warn("ignoring AcousticTiming %s; use auto, 1 or 5", value);
        } else if (!strcasecmp(key, "CapitalPitchChange")) {
            if (parse_number(value, 0, 100, &number) == 0)
                config.capitalPitchChange = number;
            else
                warn("ignoring CapitalPitchChange %s; use 0 to 100", value);
        } else if (!strcasecmp(key, "DictionaryDirectory")) {
            free(config.dictionaryDirectory);
            config.dictionaryDirectory = *value ? copy_string(value) : NULL;
        } else {
            warn("ignoring the unknown setting %s", key);
        }
    }
    fclose(file);
    return 0;
}

static void free_voice_list(void)
{
    size_t i;

    if (!speechdVoices)
        return;
    for (i = 0; speechdVoices[i]; i++) {
        free(speechdVoices[i]->name);
        free(speechdVoices[i]->language);
        free(speechdVoices[i]->variant);
        free(speechdVoices[i]);
    }
    free(speechdVoices);
    speechdVoices = NULL;
}

static int allocate_voice_list(void)
{
    size_t at = 0;
    size_t language, preset;

    speechdVoices = calloc(LANGUAGE_COUNT * PRESET_COUNT + 1,
                           sizeof(*speechdVoices));
    if (!speechdVoices)
        return -1;
    for (language = 0; language < LANGUAGE_COUNT; language++) {
        if (!available[language])
            continue;
        for (preset = 0; preset < PRESET_COUNT; preset++) {
            size_t length = strlen(presets[preset].name)
                          + strlen(languages[language].name) + 4;
            SPDVoice *voice = calloc(1, sizeof(*voice));

            if (!voice)
                return -1;
            speechdVoices[at++] = voice;
            voice->name = malloc(length);
            voice->language = copy_string(languages[language].locale);
            voice->variant = copy_string("none");
            if (!voice->name || !voice->language || !voice->variant)
                return -1;
            snprintf(voice->name, length, "%s (%s)", presets[preset].name,
                     languages[language].name);
        }
    }
    return 0;
}

int module_init(char **statusInfo)
{
    unsigned int ids[64];
    int count = (int)(sizeof(ids) / sizeof(ids[0]));
    char version[64] = { 0 };
    char status[128];
    size_t i;
    int j;

    module_audio_set_server();
    /* About 50 ms of audio per callback, so STOP is read promptly. */
    audioFrameSamples = config.sampleRate / 20;
    audioFrame = calloc((size_t)audioFrameSamples, sizeof(*audioFrame));
    if (!audioFrame) {
        *statusInfo = copy_string("ReLoq C is out of memory");
        return -1;
    }
    if (eciGetAvailableLanguages(ids, &count) != 0 || count < 1) {
        *statusInfo = copy_string("ReLoq C reports no languages");
        return -1;
    }
    for (j = 0; j < count; j++)
        for (i = 0; i < LANGUAGE_COUNT; i++)
            if ((unsigned int)languages[i].id == ids[j])
                available[i] = 1;
    i = language_for("en-US");
    if (i >= LANGUAGE_COUNT)
        i = default_language();
    if (allocate_voice_list() != 0 || i >= LANGUAGE_COUNT || !open_engine(i)) {
        *statusInfo = copy_string("ReLoq C could not start its engine");
        return -1;
    }
    currentLanguage = i;
    eciVersion(version);
    version[sizeof(version) - 1] = 0;
    snprintf(status, sizeof(status), "ReLoq C %s initialized", version);
    *statusInfo = copy_string(status);
    debug_log("%s", status);
    return 0;
}

SPDVoice **module_list_voices(void)
{
    return speechdVoices;
}

static int queue_message(ECIHand handle, const char *data,
                         SPDMessageType messageType)
{
    char *prepared = NULL;
    const char *message = data;
    int characterMode = 0;
    int suppressPunctuation;
    int result;
    uint32_t scalar;

    switch (messageType) {
    case SPD_MSGTYPE_SOUND_ICON:
        /* The server plays the icon; nothing is spoken. */
        pendingIcon = copy_string(data);
        return pendingIcon ? 0 : -1;
    case SPD_MSGTYPE_KEY:
        if (single_scalar(data, &scalar)) {
            characterMode = 1;
        } else {
            prepared = key_text(data);
            if (!prepared)
                return -1;
            message = prepared;
        }
        break;
    case SPD_MSGTYPE_CHAR:
    case SPD_MSGTYPE_SPELL:
        characterMode = 1;
        break;
    case SPD_MSGTYPE_TEXT:
    default:
        if (settings.spelling == SPD_SPELL_ON)
            characterMode = 1;
        break;
    }
    suppressPunctuation = !characterMode && messageType == SPD_MSGTYPE_TEXT
                       && settings.punctuation == SPD_PUNCT_NONE;
    if (contains_tag(message, "speak") || contains_tag(message, "mark")
        || contains_tag(message, "break")) {
        result = queue_ssml(handle, message, characterMode, suppressPunctuation);
    } else {
        /* queue_text may rewrite punctuation in place. */
        size_t length = strlen(message) + 1;
        char *copy = malloc(length);

        if (copy) {
            memcpy(copy, message, length);
            result = queue_text(handle, copy, characterMode, suppressPunctuation);
        } else {
            result = -1;
        }
        free(copy);
    }
    free(prepared);
    return result;
}

static int capital_character(const char *data, SPDMessageType messageType)
{
    uint32_t scalar;

    return (messageType == SPD_MSGTYPE_CHAR || messageType == SPD_MSGTYPE_KEY)
        && single_scalar(data, &scalar) && is_upper(scalar);
}

void module_speak_sync(const char *data, size_t bytes,
                       SPDMessageType messageType)
{
    ECIHand handle;
    char *message;
    int synthesized = 1;

    stopRequested = 0;
    pauseRequested = 0;
    pauseIndexReached = 0;
    queuedText = 0;
    clear_marks();
    free(pendingIcon);
    pendingIcon = NULL;
    if (bytes == SIZE_MAX || !(message = malloc(bytes + 1))) {
        module_speak_error();
        return;
    }
    memcpy(message, data, bytes);
    message[bytes] = 0;
    handle = configure_engine(capital_character(message, messageType));
    if (!handle || queue_message(handle, message, messageType) != 0) {
        if (handle)
            eciClearInput(handle);
        free(message);
        module_speak_error();
        return;
    }
    free(message);
    module_speak_ok();
    module_report_event_begin();
    if (pendingIcon)
        module_report_icon(pendingIcon);
    if (queuedText) {
        synthesizing = 1;
        synthesized = eciSynthesize(handle) > 0;
        if (!eciSynchronize(handle))
            synthesized = 0;
        synthesizing = 0;
        if (!synthesized || stopRequested || pauseIndexReached)
            eciClearInput(handle);
    }
    /* A pause takes effect at Speech Dispatcher's next own mark, from where
     * the server resumes. A message that ended first simply ended. */
    if (pauseIndexReached)
        module_report_event_pause();
    else if (stopRequested || !synthesized)
        module_report_event_stop();
    else
        module_report_event_end();
}

int module_stop(void)
{
    stopRequested = 1;
    return 0;
}

size_t module_pause(void)
{
    pauseRequested = 1;
    return 0;
}

int module_close(void)
{
    size_t i;

    /* QUIT can be read from the audio callback. The process exits next;
     * the engine must not be deleted underneath its own synthesis. */
    if (synthesizing) {
        stopRequested = 1;
        return 0;
    }
    for (i = 0; i < LANGUAGE_COUNT; i++)
        if (engines[i]) {
            eciDelete(engines[i]);
            engines[i] = NULL;
        }
    clear_marks();
    free(pendingIcon);
    pendingIcon = NULL;
    free(audioFrame);
    audioFrame = NULL;
    free_voice_list();
    free(settings.language);
    settings.language = NULL;
    free(settings.synthesisVoice);
    settings.synthesisVoice = NULL;
    free(config.dictionaryDirectory);
    config.dictionaryDirectory = NULL;
    if (debugFile) {
        fclose(debugFile);
        debugFile = NULL;
    }
    return 0;
}

static int set_string(char **target, const char *value)
{
    char *copy = copy_string(value);

    if (strcmp(value, "NULL") && !copy)
        return -1;
    free(*target);
    *target = copy;
    return 0;
}

int module_set(const char *variable, const char *value)
{
    int parsed;

    if (!strcmp(variable, "rate"))
        return parse_number(value, -100, 100, &settings.rate);
    if (!strcmp(variable, "pitch"))
        return parse_number(value, -100, 100, &settings.pitch);
    if (!strcmp(variable, "pitch_range"))
        return parse_number(value, -100, 100, &settings.pitchRange);
    if (!strcmp(variable, "volume"))
        return parse_number(value, -100, 100, &settings.volume);
    if (!strcmp(variable, "punctuation_mode")) {
        if (!strcmp(value, "all")) settings.punctuation = SPD_PUNCT_ALL;
        else if (!strcmp(value, "none")) settings.punctuation = SPD_PUNCT_NONE;
        else if (!strcmp(value, "some")) settings.punctuation = SPD_PUNCT_SOME;
        else if (!strcmp(value, "most")) settings.punctuation = SPD_PUNCT_MOST;
        else return -1;
        return 0;
    }
    if (!strcmp(variable, "spelling_mode")) {
        if (!strcmp(value, "on")) settings.spelling = SPD_SPELL_ON;
        else if (!strcmp(value, "off")) settings.spelling = SPD_SPELL_OFF;
        else return -1;
        return 0;
    }
    if (!strcmp(variable, "cap_let_recogn")) {
        if (!strcmp(value, "none")) settings.capitals = SPD_CAP_NONE;
        else if (!strcmp(value, "spell")) settings.capitals = SPD_CAP_SPELL;
        else if (!strcmp(value, "icon")) settings.capitals = SPD_CAP_ICON;
        else return -1;
        return 0;
    }
    if (!strcmp(variable, "voice")) {
        static const char *const names[] = {
            "male1", "male2", "male3", "female1", "female2", "female3",
            "child_male", "child_female"
        };

        for (parsed = 0; parsed < 8; parsed++)
            if (!strcmp(value, names[parsed])) {
                settings.voiceType = (SPDVoiceType)(parsed + 1);
                return 0;
            }
        return -1;
    }
    if (!strcmp(variable, "language"))
        return set_string(&settings.language, value);
    if (!strcmp(variable, "synthesis_voice"))
        return set_string(&settings.synthesisVoice, value);
    return -1;
}

int module_audio_set(const char *variable, const char *value)
{
    return !strcmp(variable, "audio_output_method")
        && !strcmp(value, "server") ? 0 : -1;
}

int module_audio_init(char **statusInfo)
{
    (void)statusInfo;
    return 0;
}

int module_loglevel_set(const char *variable, const char *value)
{
    if (strcmp(variable, "log_level"))
        return -1;
    return parse_number(value, 0, 5, &logLevel);
}

int module_debug(int enable, const char *filename)
{
    FILE *file = NULL;

    if (enable) {
        if (!filename)
            return -1;
        file = fopen(filename, "w");
        if (!file)
            return -1;
    }
    if (debugFile)
        fclose(debugFile);
    debugFile = file;
    debugEnabled = enable;
    return 0;
}

int module_loop(void)
{
    return module_process(STDIN_FILENO, 1);
}
