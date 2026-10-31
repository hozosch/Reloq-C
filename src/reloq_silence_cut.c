#include "reloq_silence_cut.h"
#include <stdint.h>

int reloq_silence_cut_begin(struct reloq_silence_cut *cut,
    const struct reloq_silence_span *spans,size_t count,
    reloq_session_pcm_sink sink,void *context)
{
    if(!cut||!sink||(count&&!spans))return -1;
    for(size_t i=0;i<count;++i)
        if(spans[i].end<spans[i].begin ||
           spans[i].minimum>spans[i].end-spans[i].begin ||
           (i&&spans[i].begin<spans[i-1].end))return -1;
    *cut=(struct reloq_silence_cut){.spans=spans,.count=count,.sink=sink,.context=context};
    return 0;
}
static int send(struct reloq_silence_cut *cut,const int16_t *pcm,size_t count)
{
    if(!count)return 0;
    if(count>SIZE_MAX-cut->output_samples||cut->sink(cut->context,pcm,count))return -1;
    cut->output_samples+=count;return 0;
}
static int zeros(struct reloq_silence_cut *cut,size_t count)
{
    static const int16_t zero[256]={0};
    while(count){size_t n=count>256?256:count;if(send(cut,zero,n))return -1;count-=n;}
    return 0;
}
int reloq_silence_cut_pcm(void *context,const int16_t *pcm,size_t samples)
{
    struct reloq_silence_cut *cut=context;
    if(!cut||(!pcm&&samples)||samples>SIZE_MAX-cut->input_samples)return -1;
    while(samples) {
        const struct reloq_silence_span *span=cut->index<cut->count?cut->spans+cut->index:NULL;
        if(span&&cut->input_samples>=span->end) {
            size_t keep=cut->pending_zeros<span->minimum?cut->pending_zeros:span->minimum;
            if(zeros(cut,keep))return -1;
            cut->pending_zeros=0;++cut->index;continue;
        }
        size_t n=samples;
        if(!span||cut->input_samples<span->begin) {
            if(span&&n>span->begin-cut->input_samples)n=span->begin-cut->input_samples;
            if(send(cut,pcm,n))return -1;
        } else {
            if(n>span->end-cut->input_samples)n=span->end-cut->input_samples;
            size_t at=0;
            while(at<n){
                size_t end=at;
                if(!pcm[at]){
                    do{++end;}while(end<n&&!pcm[end]);
                    cut->pending_zeros+=end-at;
                }else{
                    size_t keep=cut->pending_zeros<span->minimum?cut->pending_zeros:span->minimum;
                    if(zeros(cut,keep))return -1;
                    cut->pending_zeros=0;
                    /* Nonzero samples retain their original values. */
                    do{++end;}while(end<n&&pcm[end]);
                    if(send(cut,pcm+at,end-at))return -1;
                }
                at=end;
            }
        }
        cut->input_samples+=n;pcm+=n;samples-=n;
    }
    /* An index callback may follow this frame immediately, including EOF.
     * Resolve the quiet interval before reporting its compressed position. */
    while(cut->index<cut->count&&cut->input_samples==cut->spans[cut->index].end) {
        size_t keep=cut->pending_zeros<cut->spans[cut->index].minimum?
            cut->pending_zeros:cut->spans[cut->index].minimum;
        if(zeros(cut,keep))return -1;
        cut->pending_zeros=0;++cut->index;
    }
    return 0;
}
