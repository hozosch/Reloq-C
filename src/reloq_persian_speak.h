#ifndef RELOQ_PERSIAN_SPEAK_H
#define RELOQ_PERSIAN_SPEAK_H

#include <stddef.h>
#include "reloq_eci.h"
#include "reloq_persian_backend.h"

/* Reusable request-local preparation for the native sentence renderer.
 * Source positions are mapped to acoustic-plan byte offsets; the caller
 * releases the plan after capture or rendering, even after a failed job. */
struct reloq_fa_prepared {
    struct reloq_fa_backend_plan backend;
    size_t *positions;
    size_t index_count;
};
int reloq_fa_prepare_native(const char *text,size_t bytes,int variant,
    const size_t *source_offsets,size_t index_count,size_t *error_offset,
    struct reloq_fa_prepared *result);
int reloq_fa_prepare_native_runs(const char *text,size_t bytes,int variant,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,size_t index_count,size_t *error_offset,
    struct reloq_fa_prepared *result,char **donor_text,int validate_modes);
void reloq_fa_prepared_clear(struct reloq_fa_prepared *prepared);

/* Speak one already prepared acoustic plan. Positions refer to bytes in the
 * prepared plan, not the original source. Optional pause flags add the same
 * boundary space used by the existing sentence planner after a word mark. */
int reloq_fa_speak_prepared(ECIHand voice,const struct reloq_fa_prepared *prepared,
    const size_t *positions,const int *index_values,const unsigned char *pause,
    size_t index_count);

/* Development-only shared C route. The caller supplies an idle, otherwise
 * empty 0x90000 ECI handle with its callback, output buffer and desired voice
 * configured. It works without NVDA or eSpeak. This initial route still uses
 * the accepted acoustic backend and has not yet adopted the NVDA sentence
 * contour, every inline control, marked onsets across an English run, or
 * cancellation during preparation. A Persian acoustic scope must not already
 * be active on this thread.
 * Do not select it as the production Persian voice until those match.
 *
 * Ordered source offsets are UTF-8 byte positions. On success their index
 * values are emitted by the handle's normal ECI callback. Unknown words use
 * the independent orthographic completion rules. Input backticks are not
 * supported by this pilot, so user controls cannot be mistaken for text.
 *
 * Return 0 on success, 1 for an unsupported pronunciation/route, -1 for
 * invalid input or a bad handle, -2 for memory failure, -3 for synthesis or
 * callback failure. The caller owns its handle and all input buffers. */
int reloq_fa_speak_native(ECIHand voice,const char *text,size_t bytes,int variant,
    const size_t *source_offsets,const int *index_values,size_t index_count,
    size_t *error_offset);

#endif
