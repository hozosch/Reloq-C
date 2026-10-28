#ifndef RELOQ_ECI_H
#define RELOQ_ECI_H

#include <stdint.h>
#include <stddef.h>

/* A direct, native ECI ABI around the independent C renderer and a deliberately
 * narrow German developmental voice. German input is Windows-1252. Text
 * submitted through successive eciAddText calls is combined before phonetic
 * analysis; complete covered sentences are rendered in one continuing Klatt
 * session. Indices at sentence boundaries and at the measured word boundary
 * of a covered natural connected pair are delivered at their exact PCM
 * position. Other indices inside a sentence fail before output, as does
 * uncovered text. An explicitly selected experimental generic mode renders
 * natural German through a best-effort spelling frontend and independent C
 * Klatt approximation at 11,025 Hz. In that mode, indices can occur within
 * a phrase and are delivered after accepted PCM at the next acoustic word
 * boundary; indices inside a spelling token follow that token's word. No
 * index splits a phrase into separate synthesis jobs. Neither mode advertises
 * a complete language. The private numeric
 * entry below tests the same PCM callback without a host.
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
typedef int (ECICALL *ECICallback)(ECIHand, int message, int param,
                                   void *data);

enum {
    RELOQ_ECI_WAVEFORM_BUFFER = 0,
    RELOQ_ECI_INDEX_REPLY = 2,
    RELOQ_ECI_SYNTH_MODE = 0,
    RELOQ_ECI_INPUT_TYPE = 1,
    RELOQ_ECI_SAMPLE_RATE = 5,
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
     * semantics. Speed uses native ECI 40..250 (NVDA 0..100 -> 40..156,
     * optional boost to 250). Pitch uses the native 0..100 voice scale.
     * The last setter for rate or pitch selects that control route; merely
     * getting either parameter does not alter the legacy developmental path.
     * They do not pretend to implement the full eciSetVoiceParam ABI. */
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
 * All functions other than eciStop must run on the owner thread, outside a
 * callback. eciStop may run on another thread or from the callback. This
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
ECIAPI int ECICALL eciSynchronize(ECIHand handle);
ECIAPI int ECICALL eciStop(ECIHand handle);
ECIAPI int ECICALL eciSpeaking(ECIHand handle);
ECIAPI int ECICALL eciClearInput(ECIHand handle);
ECIAPI int ECICALL eciGetParam(ECIHand handle, int parameter);
ECIAPI int ECICALL eciSetParam(ECIHand handle, int parameter, int value);

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
 * message 0 with its sample count. The callback must return 1 to accept it;
 * 0 or 2 stops with RELOQ_ECI_CALLBACK. Already delivered PCM remains valid.
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

#ifdef __cplusplus
}
#endif

#endif
