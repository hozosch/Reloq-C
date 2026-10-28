#ifndef RELOQ_GERMAN_VOICE_H
#define RELOQ_GERMAN_VOICE_H

#include "reloq_german_utterance.h"
#include "reloq_german_input.h"

/* A narrow, native German text-to-PCM voice prototype for one isolated,
 * explicitly primary-stressed syllable [h|p|t|k] V [p|t|k], or
 * [h] V [s|f], with V = AEIOUaeiou. These isolated forms support implicit
 * endings and . ? ! : ; , ... after the word.
 * Plain UTF-8 German spelling and valid German ECI
 * phonetic input are accepted through the same conservative frontend.
 * Examples: "Hieb.", "Hack.", "App.", "Hass.", or "`[.1hAt].".
 *
 * The voice uses dynamic C acoustic gestures and the existing C Klatt
 * renderer. It has no recorded word contours or Apple module execution.
 * Complete sentences, such as "Pack. Tipp.", are also supported as a
 * sequence. The target timeline restarts for each sentence while the
 * Klatt/noise session continues without reset. One connected two-word
 * sentence is covered when both stressed syllables have the shape
 * [optional h|p|t|k]V k + t V [p|t|k] and the text frontend has verified its
 * complete natural SPR callback; an explicit `2 phonetic annotation may
 * specify the same two stresses. This connected form uses one target
 * timeline, distinct from two separate sentences. Other word-boundary
 * consonant pairs and other variable stress shapes remain outside this
 * voice model. Three explicit two-syllable phone classes have C composers:
 * weak .0[p/t/k]V[p/k] followed by stressed .1tV[p/t/k], stressed open
 * .1[p/t/k]V followed by weak .0[p/t/k]@{,n}, and stressed closed
 * .1[p/t/k]V[p/k] followed by .0(@|@n|t@|t@n). Natural spellings including
 * Pappe, Packen, Tappen, Packte and Packten enter only when the frontend
 * supplies the exact corresponding SPR structure.
 * Rates other than nominal 11,025 Hz and other voices are not measured.
 * Create and reset a session using reloq_german_voice_defaults(). A caller
 * must never interpret an UNCOVERED result as successful silent speech. */

/* 1 = supported, 0 = well-formed but not modeled, -1 = invalid input.
 * bad_offset is set to the UTF-8 offset of an invalid or uncovered spelling
 * where one is known, or zero for a well-formed unsupported phone shape. */
int reloq_german_voice_supports(const unsigned char *utf8, size_t length,
                                size_t *bad_offset);

/* Expose complete sentence boundaries for hosts with ECI index callbacks.
 * This preflights ALL sentences before any synthesis: 1 = covered, 0 =
 * uncovered, -1 = invalid, -2 = output capacity too small. On full success,
 * count is at least 1 and spans refer to
 * byte ranges in the original UTF-8 text, including terminal punctuation
 * but excluding intersentence space. A connected two-word sentence is
 * returned as one span; this API does not locate an index between its words.
 * A host can render each complete span with the same
 * session (NO reset) and emit a callback between spans. On failure no span
 * is published. With insufficient capacity, *count is the required number
 * after all spans have been validated; NULL spans and capacity zero query
 * that number without publishing spans. */
struct reloq_german_voice_span {
    size_t offset;
    size_t length;
};
int reloq_german_voice_sentence_spans(
    const unsigned char *utf8, size_t length,
    struct reloq_german_voice_span *spans, size_t capacity, size_t *count,
    size_t *bad_offset);

/* Cooperative preflight for hosts that can cancel a long queued request.
 * The predicate is polled while locating sentence boundaries, decoding
 * UTF-8 and checking each word. -3 means cancelled, -4 means allocation
 * failure. Both leave count and spans unchanged; no audio is produced.
 * The unchecked API above is this function with a NULL predicate. */
int reloq_german_voice_sentence_spans_checked(
    const unsigned char *utf8, size_t length,
    struct reloq_german_voice_span *spans, size_t capacity, size_t *count,
    size_t *bad_offset, reloq_german_cancel_check cancelled,
    void *cancel_context);

/* Locate the first word boundary in one fully modeled connected sentence.
 * The byte offset is the end of the first written word (the separating
 * space follows it). The sample offset is the reference-measured ECI index
 * position on this phrase's 11,025-Hz PCM timeline. Neither result is
 * published if the whole connected gesture cannot be composed. Only
 * natural pairs with separately checked public ECI callback alignment are
 * admitted; an otherwise speakable new pair may still lack this alignment.
 * Return 1 for a measured pair, 0 for an isolated/unverified sentence,
 * -1 for invalid input, -3 for cancellation or -4 for memory failure. */
int reloq_german_voice_connected_index_sample(
    const unsigned char *utf8, size_t length,
    size_t *word_byte_offset, size_t *sample_offset,
    reloq_german_cancel_check cancelled, void *cancel_context);

/* The same result and progress conventions as reloq_german_utterance_render.
 * A text/model failure emits no PCM. A sink failure or cancellation may
 * follow accepted frames. Only cancellation may be called concurrently. */
int reloq_german_voice_render(const unsigned char *utf8, size_t length,
                              struct reloq_session *session,
                              reloq_session_pcm_sink sink, void *context,
                              struct reloq_german_utterance_info *info);

#endif
