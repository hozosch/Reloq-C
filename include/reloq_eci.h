#ifndef RELOQ_ECI_H
#define RELOQ_ECI_H

#include <stdint.h>
#include <stddef.h>

#if defined(ECI_STATIC) && !defined(RELOQ_ECI_STATIC)
#define RELOQ_ECI_STATIC
#endif

/* Native ECI ABI for the built-in C speech languages. Instances use the
 * ordinary text engine by default; the old narrow German numeric comparison
 * path remains available through the explicit VOICE_MODE extension.
 * eciAddText accepts the selected language's byte code page, or UTF-16LE
 * when bit 0x800 is set in LANGUAGE_DIALECT. eciAddTextUtf8 remains available.
 * Voice slots 1..8 are presets, 9..16 are editable copies, and 0 is active.
 */
#if defined(_WIN32)
#define ECICALL __stdcall
#if defined(RELOQ_ECI_STATIC)
#define ECIAPI
#elif defined(RELOQ_ECI_BUILD)
#define ECIAPI __declspec(dllexport)
#else
#define ECIAPI __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define ECICALL
#if defined(RELOQ_ECI_BUILD)
#define ECIAPI __attribute__((visibility("default")))
#else
#define ECIAPI
#endif
#else
#define ECICALL
#define ECIAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void *ECIHand;
typedef void *ECIDictHand;
typedef void *ECIFilterHand;
#define NULL_ECI_HAND ((ECIHand)0)
#define NULL_DICT_HAND ((ECIDictHand)0)
typedef int ECIMessage;
typedef int ECICallbackReturn;
enum {eciWaveformBuffer=0,eciPhonemeBuffer=1,eciIndexReply=2,
    eciPhonemeIndexReply=3,eciWordIndexReply=4,eciStringIndexReply=5,eciAudioIndexReply=6};
enum {eciDataNotProcessed=0,eciDataProcessed=1,eciDataAbort=2};
typedef enum {eciSynthMode=0,eciInputType=1,eciTextMode=2,eciDictionary=3,
    eciSampleRate=5,eciWantPhonemeIndices=7,eciRealWorldUnits=8,
    eciLanguageDialect=9,eciNumberMode=10,eciRomanizer=12,
    eciAudioFormatA=13,eciAudioFormatB=14,eciAudioFormatC=15,
    eciAudioFormatD=16,eciNumParams=18} ECIParam;
typedef int (ECICALL *ECICallback)(ECIHand, int message, int param,
                                   void *data);

enum {
    RELOQ_ECI_WAVEFORM_BUFFER = 0,
    RELOQ_ECI_INDEX_REPLY = 2,
    RELOQ_ECI_SYNTH_MODE = 0,
    RELOQ_ECI_INPUT_TYPE = 1,
    /* ECI dictionary polarity: 0 enables abbreviations, 1 disables them.
     * Main/Root pronunciation volumes stay active independently. */
    RELOQ_ECI_DICTIONARY = 3,
    /* Per-instance counterpart of the native phrase-prediction setting
     * and inline pp control. 0 off, 1 on; default remains off. */
    RELOQ_ECI_PHRASE_PREDICTION = 11,
    RELOQ_ECI_SAMPLE_RATE = 5,
    RELOQ_ECI_REAL_WORLD_UNITS = 8,
    RELOQ_ECI_LANGUAGE_DIALECT = 9,
    /* Read-only development diagnostics, queried with eciGetParam after
     * eciSynthesize. They survive eciClearInput (the adapter calls it on a
     * failed job) until the next synthesis or eciReset. These are local
     * extensions, not standard ECI values. */
    RELOQ_ECI_LAST_ERROR = 0x1000,
    RELOQ_ECI_FAILURE_UTF8_OFFSET = 0x1001,
    RELOQ_ECI_FAILURE_STAGE = 0x1002,
    /* Opt-in developmental German voice selection. 0 keeps the exact
     * reference-matched, narrowly covered path; 1 selects the independently
     * authored, approximate generic C voice with best-effort G2P. */
    RELOQ_ECI_VOICE_MODE = 0x1003,
    RELOQ_ECI_VOICE_EXACT = 0,
    RELOQ_ECI_VOICE_GENERIC = 1,
    /* Native controls for the experimental generic C voice only. Each is
     * an integer in 0..100; defaults are 50 / 50 / 100 respectively. */
    RELOQ_ECI_GENERIC_RATE = 0x1004,
    RELOQ_ECI_GENERIC_PITCH = 0x1005,
    RELOQ_ECI_GENERIC_VOLUME = 0x1006,
    /* Independent C voice extensions for the OpenEVV/NVDA percentage
     * semantics. Speed uses native ECI 0..250 (NVDA 0..100 -> 40..156,
     * optional boost to 250). Pitch uses the native 0..100 voice scale.
     * The last setter for rate or pitch selects that control route; merely
     * getting either parameter does not alter the legacy developmental path.
     * These aliases share state with standard eciSetVoiceParam on slot 0. */
    RELOQ_ECI_NATIVE_SPEED = 0x1007,
    RELOQ_ECI_NATIVE_PITCH = 0x1008,
    /* Native F0 contour span; 0 is flat, 30 is the Apple voice default. */
    RELOQ_ECI_NATIVE_INFLECTION = 0x1009,
    /* Gain after the generic Klatt source, matching ECI 0..100. */
    RELOQ_ECI_NATIVE_VOLUME = 0x100a,
    RELOQ_ECI_PROCESS_TAGS = 0x100b,
    /* Native pause policy: 0 never, 1 utterance end only, 2 always. */
    RELOQ_ECI_PAUSE_MODE = 0x100c,
    RELOQ_ECI_HEAD_SIZE = 0x100d,
    RELOQ_ECI_ROUGHNESS = 0x100e,
    RELOQ_ECI_BREATHINESS = 0x100f,
    RELOQ_ECI_GENDER = 0x1010,
    /* Legacy ReLoq setting. Values 0..3 remain readable/writable; every
     * value renders the accepted C tuning in ECI speech. */
    RELOQ_ECI_HIGH_RATE_PROFILE = 0x1011,
    /* German /Ej/ coda colour, 0..100; defaults 85, 45 and 45 respectively.
     * Higher opening opens F1; backness lowers F2/F3; softness lowers F3. */
    RELOQ_ECI_GERMAN_GLIDE_OPENING = 0x1012,
    RELOQ_ECI_GERMAN_GLIDE_BACKNESS = 0x1013,
    RELOQ_ECI_GERMAN_GLIDE_SOFTNESS = 0x1014,
    /* 0: ReLoq's consistently rising questions; 1: native question melody. */
    RELOQ_ECI_ORIGINAL_QUESTIONS = 0x1015,
    /* 0: standard voice; 1: reduce periodic presence from 16 kHz. */
    RELOQ_ECI_MELLOW_MODE = 0x1016,
    /* Default native VWPM timing. 0 uses the ordinary ECI speed; otherwise
     * 1..6000 sets WPM without enabling user-supplied inline tags. */
    RELOQ_ECI_NATIVE_WPM = 0x1017,
    /* Optional output EQ; values 0..24 represent -12..+12 dB. 12 is flat. */
    RELOQ_ECI_EQ_LOW = 0x1018,
    RELOQ_ECI_EQ_MID = 0x1019,
    RELOQ_ECI_EQ_HIGH = 0x101a,
    /* 0: 1-ms controls; 1: original 5-ms clock; 2: choose per language.
     * Automatic uses 5 ms for Castilian Spanish, Greek and Aragonese, 1 ms otherwise.
     * Native VS and VWPM duration scales are unchanged in every mode. */
    RELOQ_ECI_ORIGINAL_ACOUSTIC_CLOCK = 0x101b,
    /* Optional shared text analysis controls; disabled by default. */
    RELOQ_ECI_WORD_SEGMENTATION = 0x101c,
    RELOQ_ECI_CAMEL_CASE = 0x101d,
    /* Read-only UTF-8 input mode bits supported by eciAddTextUtf8. */
    RELOQ_ECI_UTF8_INPUT_MODES = 0x101e,
    /* Five optional peaking filters, each with four consecutive parameters:
     * enabled 0/1, frequency 1..24000 Hz, gain 0..24 (-12..+12 dB),
     * and bandwidth 10..400 percent of an octave. */
    RELOQ_ECI_EQ_FILTER_BASE = 0x1020,
    RELOQ_ECI_ENGLISH = 0x00010000,
    RELOQ_ECI_ENGLISH_BRITAIN = 0x00010001,
    RELOQ_ECI_SPANISH_SPAIN = 0x00020000,
    RELOQ_ECI_SPANISH_MEXICO = 0x00020001,
    RELOQ_ECI_FRENCH_FRANCE = 0x00030000,
    RELOQ_ECI_FRENCH_CANADA = 0x00030001,
    RELOQ_ECI_GERMAN = 0x00040000,
    RELOQ_ECI_ITALIAN = 0x00050000,
    RELOQ_ECI_CHINESE_CHINA = 0x00060000,
    RELOQ_ECI_CHINESE_TAIWAN = 0x00060001,
    RELOQ_ECI_PORTUGUESE_BRAZIL = 0x00070000,
    RELOQ_ECI_JAPANESE = 0x00080000,
    RELOQ_ECI_FINNISH = 0x00090000,
    RELOQ_ECI_KOREAN = 0x000a0000,
    RELOQ_ECI_GREEK = 0x00120000,
    RELOQ_ECI_ARAGONESE = 0x00130000
};
#define RELOQ_ECI_EQ_FILTER_COUNT 5
#define RELOQ_ECI_EQ_FILTER_STRIDE 4

/* Standard ECI voice/dictionary parameter numbers and return codes. */
typedef enum {eciGender=0,eciHeadSize=1,eciPitchBaseline=2,eciPitchFluctuation=3,
    eciRoughness=4,eciBreathiness=5,eciSpeed=6,eciVolume=7,eciNumVoiceParams=8} ECIVoiceParam;
typedef enum {eciMainDict=0,eciRootDict=1,eciAbbvDict=2,eciMainDictExt=3,eciNumDictVolumes=4} ECIDictVolume;
typedef int ECIPartOfSpeech;
typedef enum {eciDictNoError=0,eciDictFileNotFound=1,eciDictOutOfMemory=2,
    eciDictInternalError=3,eciDictNoEntry=4,eciDictErrLookUpKey=5,
    eciDictAccessError=6,eciDictInvalidVolume=7} ECIDictError;
typedef enum {eciGeneralAmericanEnglish=0x10000,eciBritishEnglish=0x10001,
    eciCastilianSpanish=0x20000,eciMexicanSpanish=0x20001,
    eciStandardFrench=0x30000,eciCanadianFrench=0x30001,
    eciStandardGerman=0x40000,eciStandardItalian=0x50000,
    eciMandarinChinese=0x60000,eciTaiwaneseMandarin=0x60001,
    eciBrazilianPortuguese=0x70000,eciStandardJapanese=0x80000,
    eciStandardFinnish=0x90000,eciStandardKorean=0xa0000,
    eciUnicodeCodeSet=0x800} ECILanguageDialect;
enum {ECI_VOICE_NAME_LENGTH=30};

enum reloq_eci_failure_stage {
    RELOQ_ECI_STAGE_NONE = 0,
    RELOQ_ECI_STAGE_PROFILE = 1,
    RELOQ_ECI_STAGE_TEXT = 2,
    RELOQ_ECI_STAGE_ACOUSTIC = 3,
    RELOQ_ECI_STAGE_INDEX = 4
};

/* FAILURE_UTF8_OFFSET is -1 when no text location is known. Otherwise it
 * refers to bytes of the complete CP1252-decoded UTF-8 phrase, independent
 * of eciAddText fragmentation. When the phrase has no terminal punctuation,
 * ECI appends an implicit period; its position is clamped to the caller's
 * original end. A stage of TEXT means spelling, symbol or lexicon coverage;
 * ACOUSTIC means the complete spelling was recognized but the current sound
 * and prosody composition does not cover its phone context; its offset is
 * rounded back to the beginning of the implicated word. INDEX refers to
 * a marker without a verified audio boundary. Other errors may have no
 * stage or location. These diagnostics do not change synthesis or callbacks.
 */

/* ECI exports. SetParam returns the previous value or -1 on refusal.
 * Mutating calls run on the owner thread, outside a callback. eciStop may
 * run on another thread or from the callback, as may eciPause. eciSpeaking and eciGetIndex
 * read atomic state and may also be queried during synthesis. This
 * developmental eciSynthesize runs synchronously; eciSynchronize reports
 * its result. Delete requires that any concurrent Stop has completed. */
ECIAPI ECIHand ECICALL eciNew(void);
ECIAPI ECIHand ECICALL eciNewEx(int language);
ECIAPI ECIHand ECICALL eciDelete(ECIHand handle);
ECIAPI int ECICALL eciReset(ECIHand handle);
ECIAPI void ECICALL eciVersion(char *buffer);
ECIAPI int ECICALL eciGetAvailableLanguages(unsigned int *languages,
                                            int *count);
ECIAPI void ECICALL eciRegisterCallback(ECIHand handle,
                                        ECICallback callback, void *data);
ECIAPI int ECICALL eciSetOutputBuffer(ECIHand handle, int samples,
                                      short *buffer);
ECIAPI int ECICALL eciAddText(ECIHand handle, const void *text);
ECIAPI int ECICALL eciInsertIndex(ECIHand handle, int index);
/* Local extension for the experimental generic voice: queue an absolute
 * native 0..100 pitch change at the current text offset. It takes effect at
 * the following acoustic word boundary, without ending the utterance. In
 * ordinary text, a mark inside a source token follows that entire token,
 * including an initialism expanded into separately spoken letter names. A
 * Set RELOQ_ECI_NATIVE_PITCH for the utterance's baseline when using this
 * extension; a later mark can then restore that absolute native value.
 * All marks disappear
 * with the current input queue after synthesis, reset or eciClearInput. A
 * stop before synthesis only signals cancellation; the caller must invoke
 * eciClearInput to discard text, indices and pitch marks already queued.
 * This is not an emulation of the proprietary inline ECI syntax. */
ECIAPI int ECICALL eciInsertPitch(ECIHand handle, int native_pitch);
ECIAPI int ECICALL eciSynthesize(ECIHand handle);
/* Native phoneme bytes arrive through message eciPhonemeBuffer. The caller
 * owns room bytes; a callback and eciSynthMode=1 are required. */
ECIAPI int ECICALL eciGeneratePhonemes(ECIHand handle,int room,void *buffer);
ECIAPI int ECICALL eciSynchronize(ECIHand handle);
ECIAPI int ECICALL eciStop(ECIHand handle);
ECIAPI int ECICALL eciPause(ECIHand handle,int pause);
ECIAPI int ECICALL eciSpeaking(ECIHand handle);
ECIAPI int ECICALL eciGetIndex(ECIHand handle);
ECIAPI int ECICALL eciProgStatus(ECIHand handle);
ECIAPI void ECICALL eciClearErrors(ECIHand handle);
/* The SDK caller supplies at least 100 bytes, or 100 UTF-16 code units. */
ECIAPI void ECICALL eciErrorMessage(ECIHand handle,void *buffer);
ECIAPI int ECICALL eciIsBeingReentered(ECIHand handle);
ECIAPI int ECICALL eciClearInput(ECIHand handle);
ECIAPI int ECICALL eciGetParam(ECIHand handle, int parameter);
ECIAPI int ECICALL eciSetParam(ECIHand handle, int parameter, int value);
ECIAPI int ECICALL eciGetDefaultParam(int parameter);
ECIAPI int ECICALL eciSetDefaultParam(int parameter,int value);
ECIAPI int ECICALL eciGetVoiceParam(ECIHand handle,int voice,int parameter);
ECIAPI int ECICALL eciSetVoiceParam(ECIHand handle,int voice,int parameter,int value);
ECIAPI int ECICALL eciCopyVoice(ECIHand handle,int from,int to);
ECIAPI int ECICALL eciGetVoiceName(ECIHand handle,int voice,void *name);
ECIAPI int ECICALL eciSetVoiceName(ECIHand handle,int voice,const void *name);
ECIAPI ECIDictHand ECICALL eciNewDict(ECIHand handle);
ECIAPI ECIDictHand ECICALL eciGetDict(ECIHand handle);
/* Dictionary selection returns eciDictNoError (0), not a Boolean. */
ECIAPI int ECICALL eciSetDict(ECIHand handle,ECIDictHand dictionary);
ECIAPI ECIDictHand ECICALL eciDeleteDict(ECIHand handle,ECIDictHand dictionary);
ECIAPI int ECICALL eciLoadDict(ECIHand handle,ECIDictHand dictionary,int volume,const void *filename);
ECIAPI int ECICALL eciSaveDict(ECIHand handle,ECIDictHand dictionary,int volume,const void *filename);
ECIAPI const char *ECICALL eciDictLookup(ECIHand handle,ECIDictHand dictionary,int volume,const void *key);
ECIAPI int ECICALL eciDictLookupA(ECIHand handle,ECIDictHand dictionary,int volume,const void *key,const char **translation,ECIPartOfSpeech *part);
ECIAPI int ECICALL eciDictFindFirst(ECIHand handle,ECIDictHand dictionary,int volume,const char **key,const char **translation);
ECIAPI int ECICALL eciDictFindNext(ECIHand handle,ECIDictHand dictionary,int volume,const char **key,const char **translation);
ECIAPI int ECICALL eciDictFindFirstA(ECIHand handle,ECIDictHand dictionary,int volume,const char **key,const char **translation,ECIPartOfSpeech *part);
ECIAPI int ECICALL eciDictFindNextA(ECIHand handle,ECIDictHand dictionary,int volume,const char **key,const char **translation,ECIPartOfSpeech *part);
ECIAPI int ECICALL eciUpdateDict(ECIHand handle,ECIDictHand dictionary,int volume,const void *key,const void *translation);
ECIAPI int ECICALL eciUpdateDictA(ECIHand handle,ECIDictHand dictionary,int volume,const void *key,const void *translation,ECIPartOfSpeech part);

/* Optional RELOQ vendor extension. A UTF-8 path names a bounded, validated
 * RELOQ-DE-SPR-1 word-to-phonemes companion file. The engine owns its copy
 * until a subsequent successful replacement, explicit NULL unload or Delete.
 * Existing data survives a failed load. Call between utterances on the owner
 * thread, never during a callback. 1 success, 0 failure. It only affects the
 * experimental generic text frontend; the exact voice remains unchanged. */
/* Length-aware native Unicode input. Mode 1 names characters in the selected
 * language; mode 0 reads text. Successive fragments are analyzed together.
 * If the complete queued UTF-8 text is one Unicode scalar, mode 0 also
 * names that character unless it is dictionary-protected; indexes and voice
 * controls do not make it text. */
/* Mode bit 0 requests character spelling; bit 1 protects the text from the
 * optional word/CamelCase segmenter (for host-side dictionary substitutions).
 * Bit 2 says this text belongs to a larger utterance, even if a host language
 * switch requires a separate synthesis call. It suppresses only the implicit
 * single-character request; explicit spelling and emoji handling still apply. */
ECIAPI int ECICALL eciAddTextUtf8(ECIHand handle, const void *text, size_t bytes, int character_mode);

/* Project directly covered Persian UTF-8 text and ordered source-byte
 * positions to internal phoneme-tag boundaries. For each event, is_index is
 * 1 for a text index and 0 for another control. Returns 0 on success, 1 for
 * uncovered input, 2 for unsupported number syntax, 3 for an unsafe event,
 * -1 for invalid input, and -2 on allocation failure. The output must be
 * released with eciPersianRelease; mapped offsets are written only on
 * success. This does not select a Persian dialect or synthesize speech. */
ECIAPI int ECICALL eciPersianProjectUtf8(const char *text,size_t bytes,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
/* Complete Persian projection with general native rules for uncovered
 * words. Mixed-language routing remains the host's responsibility. */
ECIAPI int ECICALL eciPersianProjectCompleteUtf8(const char *text,size_t bytes,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
ECIAPI void ECICALL eciPersianRelease(void *pointer);
/* True for presentation-only characters ignored in ordinary running text;
 * explicit character spelling can still request their Unicode names. */
ECIAPI int ECICALL eciPersianIgnorableTextCharacter(uint32_t codepoint);
/* Recognize Unicode/CLDR emoji components without requiring a host library. */
ECIAPI int ECICALL eciUnicodeIsEmoji(uint32_t codepoint);

/* Shared Persian text frontend. mode 0 reads Persian script; mode 1 accepts
 * Persian script and Pinglish. Both use the same lexical/stress rules and
 * source-event projection. Other modes are invalid. Ownership and status are
 * the same as eciPersianProjectUtf8; uncovered input returns 1. */
ECIAPI int ECICALL eciPersianProjectTextUtf8(const char *text,size_t bytes,int mode,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
/* Use the same projection and optional Pinglish mode with the native
 * orthographic fallback for uncovered words. */
ECIAPI int ECICALL eciPersianProjectTextCompleteUtf8(const char *text,size_t bytes,int mode,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
/* Project contiguous normal-text (run mode 0), explicit character-name
 * (run mode 1) and phonetic-input (run mode 2) spans together. A malformed
 * phonetic group is named as Persian characters, never passed as raw text to
 * the acoustic backend. run_ends are exclusive UTF-8 byte offsets; adjacent
 * text or character runs with the same mode are joined before analysis. The final
 * run ends at bytes; empty input has no runs. Names requiring an English
 * voice return 1 for the caller to route as a separate language event.
 * Ownership, event mapping and status are as for eciPersianProjectUtf8. */
ECIAPI int ECICALL eciPersianProjectRunsUtf8(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
ECIAPI int ECICALL eciPersianProjectRunsCompleteUtf8(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);
/* Analyze the same input into language-owned phone and boundary events.
 * Normal text also accepts explicit phonetic groups and `l20.0 / `l20.1
 * switches between the current experimental input variants. A malformed
 * phonetic group is spelled with Persian character names; unfamiliar names
 * use a Persian reading of the Unicode code. Source marks map to event
 * boundaries; event storage is released with eciPersianRelease.
 * This is experimental analysis only: it neither registers the dialects
 * with standard ECI voice selection nor synthesizes audio. */
#include "reloq_persian_plan.h"
ECIAPI int ECICALL eciPersianAnalyzeRunsUtf8(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    struct reloq_fa_plan_event **events,size_t *count,size_t *target_events,size_t *error_offset);
/* Like eciPersianAnalyzeRunsUtf8, but analyze uncovered Persian words with
 * general orthographic rules. This is analysis, not speech rendering. */
ECIAPI int ECICALL eciPersianAnalyzeRunsCompleteUtf8(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    struct reloq_fa_plan_event **events,size_t *count,size_t *target_events,size_t *error_offset);
/* Experimental C-only speech route using an idle, empty Finnish-backed ECI
 * handle and the same native Persian text plan. Does not need NVDA or
 * eSpeak. The US sentence contour, spelling and user inline controls are
 * not yet implemented here; cancellation during preparation is incomplete.
 * It must not replace the current Persian voice or run inside an existing
 * Persian acoustic scope.
 * Source offsets are ordered UTF-8 byte positions and values are delivered
 * as ECI indices. mode: 0 Persian, 1 Pinglish. The caller owns all buffers.
 * Returns 0 on success, 1 unsupported, -1 invalid, -2 memory failure,
 * -3 synthesis failure. Unknown words use orthographic inference. */
ECIAPI int ECICALL eciPersianSpeakNativeUtf8(ECIHand voice,const char *text,size_t bytes,int mode,
    const size_t *source_offsets,const int *index_values,size_t index_count,size_t *error_offset);
/* Experimental incremental version for normal text, character-name and
 * unambiguous phonetic runs, using the same run format as the analyzer.
 * This uses the Finnish-based contour and is not the selected US-melody
 * voice. It must not replace the current Persian voice. English-only names,
 * unsupported controls and ambiguous phonetics are declined before output.
 * An English text run can also be declined by the acoustic backend.
 * A stop observed during rendering returns -4. Like the earlier native
 * route, cancellation racing preparation or the initial render is not yet
 * guaranteed. Other return values match eciPersianSpeakNativeUtf8. */
ECIAPI int ECICALL eciPersianSpeakNativeRunsUtf8(ECIHand voice,const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const int *index_values,size_t index_count,size_t *error_offset);
/* Prepare the silent US contour text on an idle, empty US ECI handle.
 * Word boundary offsets and the result allocation are described in
 * reloq_persian_us_proxy.h. This does not alter Persian pronunciation. */
struct reloq_fa_us_proxy;
ECIAPI int ECICALL eciPersianUsProxyUtf8(ECIHand pilot,const char *text,size_t bytes,
    struct reloq_fa_us_proxy *result);
ECIAPI void ECICALL eciPersianUsProxyRelease(struct reloq_fa_us_proxy *result);
/* Backward-compatible Pinglish-mode entry point. */
ECIAPI int ECICALL eciPersianPinglishProjectUtf8(const char *text,size_t bytes,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset);

/* Direct text, number, character and Pinglish building blocks. Text and
 * offset arrays are caller-owned and must be released with PersianRelease.
 * Uncovered input is reported to the caller for an appropriate fallback. */
ECIAPI int ECICALL eciPersianTranslateUtf8(const char *text,size_t bytes,
    char **output,size_t **source_map,size_t *error_offset);
ECIAPI int ECICALL eciPersianStressUtf8(const char *word,size_t bytes,char *tag,size_t capacity);
ECIAPI int ECICALL eciPersianNumber(const char *text,size_t bytes,size_t *consumed,
    char *output,size_t capacity);
/* Structured Persian number analysis; events contain no printable ECI tags.
 * Accepts ASCII, Persian or Arabic-Indic digits, decimal/group separators,
 * dotted versions and an optional sign. A NULL events pointer and capacity
 * zero queries the required count. Returns 0 on success, 2 on unsupported
 * syntax, -1 on invalid input, or -2 for insufficient capacity. On failure,
 * the array is unchanged and count and consumed are set to zero. */
#include "reloq_persian_numbers.h"
ECIAPI int ECICALL eciPersianNumberEvents(const char *text,size_t bytes,size_t *consumed,
    struct reloq_fa_number_event *events,size_t capacity,size_t *count);
ECIAPI const char *ECICALL eciPersianLetter(uint32_t codepoint);
/* Write the native Persian spelling of one Unicode scalar. Returns 1 on
 * success, 0 for an invalid scalar, and -1 for an inadequate destination. */
ECIAPI int ECICALL eciPersianCharacter(uint32_t codepoint,char *output,size_t capacity);
/* Resolve a character and its language without inline language controls.
 * On success, kind is 0 for Persian or 1 for a plain English Unicode name. */
ECIAPI int ECICALL eciPersianCharacterResolve(uint32_t codepoint,char *output,size_t capacity,int *kind);
ECIAPI int ECICALL eciPersianPinglishWord(const char *word,size_t bytes,
    char *output,size_t capacity);

/* Experimental language-independent phone identities for Iranian Persian.
 * The legacy adapters preserve the existing acoustic realization and are
 * not a public dictionary syntax. They accept exactly one phone group. */
#include "reloq_persian_phones.h"
ECIAPI const struct reloq_fa_phone_info *ECICALL eciPersianPhoneInventory(size_t *count);
ECIAPI int ECICALL eciPersianPhonesFromLegacy(const char *text,size_t bytes,
    struct reloq_fa_phone *phones,size_t capacity,size_t *count,size_t *error_offset);
ECIAPI int ECICALL eciPersianPhonesToLegacy(const struct reloq_fa_phone *phones,size_t count,
    char *output,size_t capacity,size_t *bytes,size_t *error_index);
/* One explicitly supplied Persian phonetic group; q=uvular, x=Ach, ?=glottal.
 * It preserves legacy input and the existing sound backend. Do not use for
 * ordinary punctuation or for another language. See reloq_persian_phones.h. */
ECIAPI int ECICALL eciPersianInputToLegacy(const char *text,size_t bytes,char *output,
    size_t capacity,size_t *written,size_t *error_offset);
/* Validate one Persian phonetic group using Persian rules, then adapt it to
 * the existing acoustics. Invalid or unsupported groups return 1. */
ECIAPI int ECICALL eciPersianInputChecked(const char *text,size_t bytes,char *output,
    size_t capacity,size_t *written,size_t *error_offset);
/* Convert one directly covered word to the same identities. pinglish must be
 * 0 (Persian script) or 1 (Latin transcription). No dictionary handle or
 * fallback is used. Returns 0 success, 1 uncovered/unsupported, -1 invalid
 * input, -2 insufficient capacity/allocation failure. A NULL phones pointer
 * and zero capacity queries the required number. On failure the caller's
 * array remains unchanged. error_offset is a source UTF-8 byte offset when
 * known, otherwise zero. This function does not select or synthesize a voice. */
ECIAPI int ECICALL eciPersianWordPhonesUtf8(const char *word,size_t bytes,int pinglish,
    struct reloq_fa_phone *phones,size_t capacity,size_t *count,size_t *error_offset);

/* Select the approved Finnish-based Persian acoustic rules for the current
 * synthesis thread. Restore the returned previous state on every exit.
 * Word and affricate ordinals are request-local; clear both lists after use.
 * These low-level functions do not register a Persian voice or handle text. */
ECIAPI int ECICALL eciPersianAcousticScope(int active);
/* Temporarily bypass the saved Persian sound tuning for a comparison on the
 * current synthesis thread. Returns the previous state; restore it on exit. */
ECIAPI int ECICALL eciPersianTuningScope(int active);
ECIAPI int ECICALL eciPersianNativeOnsets(const uint32_t *words,size_t count);
ECIAPI int ECICALL eciPersianNativeGeminates(const uint32_t *pairs,size_t count);

/* Request-local frame capture/replay for a future native mixed-language
 * planner. Frame definitions are in reloq_persian_frames.h. Begin, capture,
 * end and release must run on the same synthesis thread. The returned rows
 * remain valid only until the next begin or release; no text is retained.
 * Capture applies to the next synthesis on that thread, not to an ECI
 * handle globally. The caller must release it on both success and failure. */
struct reloq_fa_frame;
struct reloq_fa_frame_mark;
struct reloq_fa_syllable;
struct reloq_fa_glide_candidate;
/* Owned, silent capture for the experimental native Persian renderer.
 * Frame and syllable layouts are defined in reloq_persian_frames.h. The
 * mark array contains one sample-clock position for each source offset in
 * the order supplied by the caller. No sound or index is sent to the
 * handle's normal callback while capturing. */
struct reloq_fa_owned_capture {
    struct reloq_fa_frame *frames;
    size_t frame_count;
    struct reloq_fa_syllable *syllables;
    size_t syllable_count;
    uint64_t *mark_samples;
    size_t mark_count;
    uint64_t sample_count;
};
/* Experimental C-only planning step, not yet a registered Persian voice.
 * Requires an idle, empty Finnish-backed handle reserved by the caller for
 * this request. Temporarily replaces and then restores its callback and
 * output buffer; the caller may stop it from another thread. mode is 0 for
 * Persian and 1 for Pinglish. Ordered offsets are UTF-8 byte positions.
 * maximum_frames is 1..120000; index_count is at most 4096. Release a
 * successful result with eciPersianReleaseOwnedCapture before reusing it.
 * Returns 0 success, 1 unsupported input, -1 invalid/nested request,
 * -2 allocation failure, -3 synthesis/capture failure, -4 cancellation.
 * Unknown written words still use the experimental orthographic inference. */
ECIAPI int ECICALL eciPersianCaptureNativeUtf8(ECIHand voice,const char *text,size_t bytes,int mode,
    const size_t *source_offsets,size_t index_count,size_t maximum_frames,
    struct reloq_fa_owned_capture *result,size_t *error_offset);
/* Capture the matching US prosody pilot from a Persian acoustic projection.
 * The caller supplies a dedicated idle, empty US-English handle. mark_samples
 * contains the word boundaries before every word after the first. The result
 * and return codes follow eciPersianCaptureNativeUtf8; invalid/unsupported
 * projection returns 1. This still does not register a Persian voice. */
ECIAPI int ECICALL eciPersianCaptureUsProxyUtf8(ECIHand pilot,const char *tagged,size_t bytes,
    size_t maximum_frames,struct reloq_fa_owned_capture *result);
ECIAPI void ECICALL eciPersianReleaseOwnedCapture(struct reloq_fa_owned_capture *result);
ECIAPI int ECICALL eciPersianCaptureBegin(size_t maximum);
ECIAPI const struct reloq_fa_frame *ECICALL eciPersianCaptureEnd(size_t *count,int *failed);
ECIAPI const struct reloq_fa_frame *ECICALL eciPersianCaptureCurrent(size_t *count,int *failed);
ECIAPI const struct reloq_fa_syllable *ECICALL eciPersianCaptureSyllables(size_t *count,int *failed);
ECIAPI void ECICALL eciPersianCaptureRelease(void);
ECIAPI int ECICALL eciPersianCaptureSetControlsOnly(int enabled);
/* Select phone timing instead of syllable timing for an active capture.
 * This optional diagnostic is request-local and does not alter speech. */
ECIAPI int ECICALL eciPersianCaptureSetPhoneTrace(int enabled);
/* Select experimental provider nucleus timing captured before intonation.
 * Retrieve times with CaptureSyllables and private encoded vowel identities
 * with CapturePhoneIdentities: its low byte is the private provider vowel
 * slot and its next byte counts marked q releases before that vowel. These
 * are diagnostics, not validated speech controls; the three timing modes
 * are exclusive. */
ECIAPI int ECICALL eciPersianCaptureSetNucleusTrace(int enabled);
/* Private-provider identities for phone or nucleus trace clocks. These are
 * diagnostics for native alignment, not public Persian phone identities. */
ECIAPI const uint32_t *ECICALL eciPersianCapturePhoneIdentities(size_t *count,int *failed);
/* Experimental clock adapter. Layout and failure contract are defined in
 * reloq_persian_clock.h; this does not itself select or synthesize a voice. */
struct reloq_fa_nucleus_clock;
ECIAPI int ECICALL eciPersianAlignNucleusClocks(const struct reloq_fa_plan_event *events,size_t event_count,
    const struct reloq_fa_syllable *syllables,size_t syllable_count,
    const struct reloq_fa_syllable *phones,size_t phone_count,
    const uint32_t *provider_slots,size_t slot_count,
    struct reloq_fa_nucleus_clock *output,size_t capacity,size_t *written);
struct reloq_fa_early_nucleus;
struct reloq_fa_early_cursor;
ECIAPI int ECICALL eciPersianEarlyCursorCreate(const struct reloq_fa_plan_event *events,size_t event_count,
    struct reloq_fa_early_cursor **result);
ECIAPI int ECICALL eciPersianEarlyCursorNext(struct reloq_fa_early_cursor *cursor,
    const struct reloq_fa_syllable *nucleus,uint32_t identity,struct reloq_fa_early_nucleus *output);
ECIAPI int ECICALL eciPersianEarlyCursorFinish(const struct reloq_fa_early_cursor *cursor);
ECIAPI void ECICALL eciPersianEarlyCursorRelease(struct reloq_fa_early_cursor *cursor);
ECIAPI int ECICALL eciPersianAlignEarlyNuclei(const struct reloq_fa_plan_event *events,size_t event_count,
    const struct reloq_fa_syllable *nuclei,size_t nucleus_count,
    const uint32_t *identities,size_t identity_count,
    struct reloq_fa_early_nucleus *output,size_t capacity,size_t *written);
/* Apply verified U-to-A word-boundary transitions to request-local frames.
 * Returns 0 on success, -1 for invalid inputs, -2 for allocation failure. */
ECIAPI int ECICALL eciPersianApplyGlides(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_glide_candidate *candidates,size_t candidate_count,
    int pitch,size_t *applied);
ECIAPI size_t ECICALL eciPersianCaptureSampleCount(void);
/* Optional request-local PCM capture avoids callbacks for each tiny frame.
 * Retrieve the samples after CaptureEnd and before CaptureRelease. */
ECIAPI int ECICALL eciPersianCaptureSetPcmStorage(int enabled);
/* During an active capture this pointer is valid only until synthesis
 * resumes; copy it in the owning callback before returning. */
ECIAPI const int16_t *ECICALL eciPersianCapturePcm(size_t *count);
ECIAPI int64_t ECICALL eciPersianLastAudibleSample(const unsigned char *pcm,size_t bytes);
ECIAPI int ECICALL eciPersianFrameTicks(const struct reloq_fa_frame *rows,size_t count,int64_t *ticks);
ECIAPI int ECICALL eciPersianFrameTimeline(const struct reloq_fa_frame *rows,size_t count,double *times);
/* Find the contextual Korean donor interval without changing frame data. */
ECIAPI int ECICALL eciPersianDonorSpan(const struct reloq_fa_frame *rows,size_t count,
    size_t leading,size_t syllables,size_t trailing,size_t *first,size_t *last);
ECIAPI void ECICALL eciPersianRecomputeFrication(struct reloq_fa_frame *rows,size_t count);
ECIAPI int ECICALL eciPersianBridgeContinuation(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed);
ECIAPI int ECICALL eciPersianBridgeStopOnsets(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed);
ECIAPI int ECICALL eciPersianBridgeRemaining(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed);
ECIAPI int ECICALL eciPersianMatchStopNasal(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary);
ECIAPI int ECICALL eciPersianPlanUnvoicedOnsets(struct reloq_fa_frame *frames,size_t count,
    const size_t *ends,size_t segments,const unsigned char *onsets,
    const struct reloq_fa_frame *reference,size_t reference_count,
    const size_t *reference_edges,size_t *counts,double *removed,size_t *new_count);
ECIAPI int ECICALL eciPersianDenseCurve(const struct reloq_fa_frame *rows,size_t count,
    unsigned column,float *out,size_t length);
ECIAPI int ECICALL eciPersianFitPitch(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,int64_t start,int64_t end,int64_t a,int64_t b);
ECIAPI int ECICALL eciPersianFitPitchAnchors(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,int64_t start,int64_t end,int64_t a,int64_t b,
    const int64_t *target,const int64_t *source,size_t anchors);
/* Fit a captured US contour to Persian word and syllable boundaries. This
 * transfers only pitch and leaves frame timing and other controls untouched.
 * Returns 0 on success, 1 if alignment is unavailable, -1 on invalid input. */
ECIAPI int ECICALL eciPersianTransferUsWords(struct reloq_fa_frame *target,size_t target_count,
    const struct reloq_fa_frame *donor,size_t donor_count,const int64_t *target_bounds,
    const int64_t *donor_bounds,size_t word_count,const struct reloq_fa_syllable *target_syllables,
    size_t target_syllable_count,const struct reloq_fa_syllable *donor_syllables,
    size_t donor_syllable_count,const uint32_t *syllables_per_word);
/* As above, with word bounds in captured audio samples. Both arrays contain
 * word_count+1 positions, including the beginning and end of the phrase.
 * Conversion and last-frame clamping are performed in the common C core. */
ECIAPI int ECICALL eciPersianTransferUsSamples(struct reloq_fa_frame *target,size_t target_count,
    const struct reloq_fa_frame *donor,size_t donor_count,const uint64_t *target_samples,
    const uint64_t *donor_samples,size_t word_count,unsigned sample_rate,
    const struct reloq_fa_syllable *target_syllables,size_t target_syllable_count,
    const struct reloq_fa_syllable *donor_syllables,size_t donor_syllable_count,
    const uint32_t *syllables_per_word);
/* Experimental orchestration for two owned captures. Both must use the
 * same sample rate as voice and contain matching word-boundary marks.
 * Fits the US contour to a private copy of target and replays it through
 * voice, emitting the caller's optional frame marks. Neither capture is
 * modified. If individual word alignment fails, retry the complete phrase;
 * entirely unvoiced targets need no pitch fit. The caller owns an idle,
 * otherwise empty handle and must retain both captures until return.
 * Returns 0 success, 1 insufficient contour, -1 invalid input, -2 memory,
 * -3 replay failure, -4 cancellation. This is not yet a registered voice. */
ECIAPI int ECICALL eciPersianReplayUsCapture(ECIHand voice,
    const struct reloq_fa_owned_capture *target,const struct reloq_fa_owned_capture *donor,
    const uint32_t *syllables_per_word,const struct reloq_fa_frame_mark *marks,size_t mark_count);
/* Experimental end-to-end C renderer for plain Persian or Pinglish text.
 * A configured Finnish-backed acoustic handle and a separate US pilot must be
 * idle, empty and set to the same sample rate. The renderer prepares the
 * Persian plan, captures both contours, fits the US melody and replays it
 * through voice. Ordered source byte offsets and non-negative index values
 * are delivered through voice's callback. The pilot is silent. No NVDA or
 * eSpeak component is required.
 * This initial route rejects mixed English and inline controls and does not
 * yet include all of the established NVDA sentence/transition rules or
 * incremental playback; it is not the registered Persian voice. Unknown
 * Persian words still use experimental orthographic inference. Stop on voice
 * is checked between stages; also stop pilot to interrupt a pilot capture.
 * Return 0 success, 1 unsupported input/contour, -1 invalid input,
 * -2 allocation failure, -3 synthesis failure, -4 cancellation. */
ECIAPI int ECICALL eciPersianSpeakUsNativeUtf8(ECIHand voice,ECIHand pilot,
    const char *text,size_t bytes,int mode,const size_t *source_offsets,
    const int *index_values,size_t index_count,size_t maximum_frames,size_t *error_offset);
/* Experimental extended sentence route. run_ends and run_modes have the
 * eciPersianAnalyzeRunsCompleteUtf8 contract: normal text (0), character
 * names (1), or one explicit Persian phonetic group (2). Normal runs also
 * accept `l20.0 and `l20.1 input-variant selectors. A name requiring a
 * separate English voice, mixed English prose, unsupported controls and
 * phonetic groups whose multiple nuclei lack explicit syllable markers
 * return 1 without output; invalid arrays return -1. This uses the same accepted
 * acoustic bridge and US contour as eciPersianSpeakUsNativeUtf8, and is not
 * a registered voice. Unknown words still require linguistic validation. */
ECIAPI int ECICALL eciPersianSpeakUsNativeRunsUtf8(ECIHand voice,ECIHand pilot,
    const char *text,size_t bytes,int mode,const size_t *run_ends,
    const unsigned char *run_modes,size_t run_count,const size_t *source_offsets,
    const int *index_values,size_t index_count,size_t maximum_frames,size_t *error_offset);
/* Keep the original pitch except near language-switch edges. Bit 0 selects
 * the beginning and bit 1 the end; window is in the frame clock's ms. */
ECIAPI int ECICALL eciPersianFitPitchEdges(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,const float *original,int64_t start,int64_t end,
    int64_t a,int64_t b,unsigned edges,int64_t window);
/* Transfer a voiced donor contour, preserving the target's median pitch.
 * Edge bits fade the transfer near an adjoining language switch. Returns 1
 * when voiced material is insufficient, without changing the frames. */
ECIAPI int ECICALL eciPersianTransferKoreanPitch(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_frame *donor,size_t donor_count,unsigned edges,int64_t window);
/* Use the donor's absolute pitch and align its word breaks to Persian word
 * ticks. The optional word_ticks contains word_count-1 increasing ticks;
 * syllables contains a positive syllable count for each word. */
ECIAPI int ECICALL eciPersianTransferKoreanAligned(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_frame *donor,size_t donor_count,const int64_t *word_ticks,
    const uint32_t *syllables,size_t word_count);
/* Replay uses the handle's output callbacks and stop epoch. An incremental
 * stream owns the handle until End and must always be ended on its owner
 * thread. It returns the normal RELOQ_ECI_* status codes. */
ECIAPI uint32_t ECICALL eciPersianReplayEpoch(ECIHand handle);
ECIAPI int ECICALL eciPersianReplayBegin(ECIHand handle,uint32_t epoch,void **stream);
ECIAPI int ECICALL eciPersianReplayAppend(void *stream,const struct reloq_fa_frame *frames,
    size_t frame_count,const struct reloq_fa_frame_mark *marks,size_t mark_count);
ECIAPI int ECICALL eciPersianReplayEnd(void *stream,int flush);
ECIAPI int ECICALL eciPersianReplayFrames(ECIHand handle,uint32_t epoch,
    const struct reloq_fa_frame *frames,size_t frame_count,
    const struct reloq_fa_frame_mark *marks,size_t mark_count);

ECIAPI int ECICALL eciLoadLexicon(ECIHand handle, const char *utf8_path);
/* Optional native main/root/abbreviation volumes. NULL disables them.
 * The UTF-8 directory is immutable while active; changes commit atomically. */
ECIAPI int ECICALL eciSetDictionaryPath(ECIHand handle,const char *utf8_directory);

/* Private C-only test interface, intentionally not exported from eci.dll.
 * controls contains frame_count consecutive 62-float Klatt control frames.
 * Resets numeric state at the beginning of each call and synchronously
 * copies every PCM chunk into the caller-owned output buffer before invoking
 * message 0 with its sample count. Return 1 to accept it, 0 to retry the
 * same buffer after a short wait, or 2 to abort. Stop cancels retry waits.
 * A concurrent eciStop (also permitted from the callback) cancels the call.
 */
enum reloq_eci_result {
    RELOQ_ECI_OK = 0,
    RELOQ_ECI_CANCELLED = 1,
    RELOQ_ECI_INVALID = -1,
    RELOQ_ECI_UNSUPPORTED_TEXT = -2,
    RELOQ_ECI_BUSY = -3,
    RELOQ_ECI_OUTPUT = -4,
    RELOQ_ECI_CALLBACK = -5,
    RELOQ_ECI_RENDER = -6,
    RELOQ_ECI_MEMORY = -7
};
int reloq_eci_render_controls(ECIHand handle, const float *controls,
                               uint32_t frame_count, uint32_t highest_formant,
                               uint32_t *frames_completed);
int reloq_eci_last_error(ECIHand handle);
/* Numeric/reference tests only: keep settings, clear input and audio state. */
int reloq_eci_reset_audio(ECIHand handle);

#ifdef __cplusplus
}
#endif

#endif
