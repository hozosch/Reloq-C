#ifndef RELOQ_SILENCE_CUT_H
#define RELOQ_SILENCE_CUT_H
#include "reloq_session.h"

/* Half-open quiet intervals on the original sample clock. Only their final
 * run of exact PCM zeros may be removed. Audible filter decay and zero
 * crossings inside it stay byte-for-byte intact. No PCM is retained. */
struct reloq_silence_span { size_t begin,end,minimum; };
struct reloq_silence_cut {
    const struct reloq_silence_span *spans;
    size_t count,index,input_samples,output_samples,pending_zeros;
    reloq_session_pcm_sink sink;
    void *context;
};
int reloq_silence_cut_begin(struct reloq_silence_cut *,
    const struct reloq_silence_span *,size_t,reloq_session_pcm_sink,void *);
int reloq_silence_cut_pcm(void *,const int16_t *,size_t);
#endif
