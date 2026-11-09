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
    RELOQ_ECI_GREEK = 0x00120000
};

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
 * language; mode 0 reads text. Successive fragments are analyzed together. */
ECIAPI int ECICALL eciAddTextUtf8(ECIHand handle, const void *text, size_t bytes, int character_mode);

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
