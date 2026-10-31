#include "reloq_silence_cut.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
struct output {int16_t data[256];size_t count;int reject;};
static int sink(void *context,const int16_t *pcm,size_t count)
{
    struct output *out=context;if(out->reject)return 1;
    assert(out->count+count<=256);memcpy(out->data+out->count,pcm,count*sizeof *pcm);
    out->count+=count;return 0;
}
int main(void)
{
    const int16_t input[]={10,0,20,5,0,0,-2,0,0,0,0,0,0,0,9,0,8};
    const int16_t expected[]={10,0,20,5,0,0,-2,0,0,9,0,8};
    const struct reloq_silence_span span={3,14,2};
    for(size_t block=1;block<=20;++block) {
        struct output output={0};struct reloq_silence_cut cut;
        assert(!reloq_silence_cut_begin(&cut,&span,1,sink,&output));
        for(size_t p=0;p<sizeof input/sizeof *input;) {
            size_t n=sizeof input/sizeof *input-p;if(n>block)n=block;
            assert(!reloq_silence_cut_pcm(&cut,input+p,n));p+=n;
        }
        assert(output.count==sizeof expected/sizeof *expected);
        assert(!memcmp(output.data,expected,sizeof expected));
        assert(cut.input_samples==17&&cut.output_samples==12&&!cut.pending_zeros&&cut.index==1);
    }
    /* A late one-LSB filter residual must not keep the intervening quiet
     * body. Every nonzero sample and both short zero crossings survive. */
    const int16_t residual[]={5,0,0,0,0,0,0,0,1,0,0,-1,0,0,0,0};
    const int16_t compact[]={5,0,0,1,0,0,-1,0,0};
    const struct reloq_silence_span quiet={0,16,2};
    for(size_t block=1;block<=20;++block){
        struct output o={0};struct reloq_silence_cut c;
        assert(!reloq_silence_cut_begin(&c,&quiet,1,sink,&o));
        for(size_t at=0;at<16;){size_t n=16-at;if(n>block)n=block;
            assert(!reloq_silence_cut_pcm(&c,residual+at,n));at+=n;}
        assert(o.count==sizeof compact/sizeof *compact&&!memcmp(o.data,compact,sizeof compact));
    }
    struct output output={0};struct reloq_silence_cut cut;
    const struct reloq_silence_span tail={0,3,0};const int16_t zero[3]={0};
    assert(!reloq_silence_cut_begin(&cut,&tail,1,sink,&output));
    assert(!reloq_silence_cut_pcm(&cut,zero,3)&&!output.count&&cut.index==1);
    const struct reloq_silence_span overlap[]={{0,4,0},{3,5,0}};
    assert(reloq_silence_cut_begin(&cut,overlap,2,sink,&output));
    output.reject=1;assert(!reloq_silence_cut_begin(&cut,NULL,0,sink,&output));
    assert(reloq_silence_cut_pcm(&cut,input,1));
    puts("Silence cut: exact decay, internal zero crossings, EOF, chunk boundaries, rejection and bounds passed");
    return 0;
}
