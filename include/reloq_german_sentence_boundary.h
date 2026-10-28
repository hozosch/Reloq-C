#ifndef RELOQ_GERMAN_SENTENCE_BOUNDARY_H
#define RELOQ_GERMAN_SENTENCE_BOUNDARY_H

#include <stddef.h>
#include "reloq_german_input.h"

/* Conservative span planner for two or more single-word German phrase
 * segments separated by exactly one ASCII space. Every span includes its
 * terminal punctuation and excludes the separator. Independent fragments
 * may end in period, colon or semicolon; question/exclamation form a
 * boundary only before an uppercase next word. Comma and ellipsis are
 * allowed only at the end of the entire request: before another word Apple
 * joins them into one prosodic phrase. Each span refers to the original
 * UTF-8 input. The voice validates and models every span before synthesis;
 * this scanner does not claim lexical coverage or punctuation prosody.
 *
 * Apple reference controls for the accepted boundary class concatenate
 * isolated segment controls exactly. Feed spans to the SAME Klatt session;
 * do not reset or insert a second pause. All words and the final mark must
 * have supported acoustic coverage. Markup, quotes, comma/ellipsis before
 * another word, ?/! before lowercase and unpunctuated connected words
 * require different prosody and return 0.
 *
 * Return 1 on a matching chain, 0 on other well-formed or unsupported text,
 * -1 on invalid arguments, -2 on insufficient capacity. A NULL spans and
 * zero capacity queries the required count: return 1 and set *count.
 * On -2, set *count to the required count without writing spans. On 0/-1,
 * leave *count and spans unchanged. Keep input immutable throughout the call;
 * spans must not overlap the input. No arbitrary sentence-count limit.
 */
struct reloq_german_sentence_span {
    size_t offset;
    size_t length;
};

int reloq_german_sentence_spans(const unsigned char *utf8, size_t length,
                                struct reloq_german_sentence_span *spans,
                                size_t capacity, size_t *count);

/* Cooperative variant: poll cancellation on entry/exit and at least every
 * 256 input bytes during both passes. Return -3 on cancellation and -4 on
 * scratch allocation failure. On either result leave *count and spans
 * unchanged. All other results have the same contract as the base API. */
int reloq_german_sentence_spans_checked(
    const unsigned char *utf8, size_t length,
    struct reloq_german_sentence_span *spans, size_t capacity, size_t *count,
    reloq_german_cancel_check cancelled, void *cancel_context);

#endif
