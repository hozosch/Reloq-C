/* Exercise cancellation at the native phoneme sink, before any PCM exists.
 * A controlled provider callback makes this independent of thread timing. */
#include "reloq_rules61.h"
#include "reloq_german_generic_prosody.h"
#include "reloq_lexicon.h"
#include "reloq61.h"
#include "evv_arena.h"
#include "evv_land.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int stopped, provider_mode, provider_calls, sink_calls;
static int cancelled(void *context)
{return *(const int *)context;}

int __real_reloq61_generate_phonemes(const struct reloq61_request *,
    struct reloq61_result *,reloq61_phoneme_sink,void *);
int __wrap_reloq61_generate_phonemes(const struct reloq61_request *q,
    struct reloq61_result *r,reloq61_phoneme_sink sink,void *context)
{
    ++provider_calls;
    if(!provider_mode)return __real_reloq61_generate_phonemes(q,r,sink,context);
    memset(r,0,sizeof *r);
    if(provider_mode==1) {
        static const char first[]="`[.1StI.0m@]";
        assert(!sink(context,first,sizeof first-1));++sink_calls;
        stopped=1;
        /* The second bounded chunk must be refused as soon as the owner
         * requests cancellation. Waiting for the final grammar result is
         * the long-text handoff regression this test guards. */
        assert(sink(context," ",1));++sink_calls;
    }
    r->error=1;
    return -1;
}

static int analyze(unsigned char **output,size_t *length)
{
    static const unsigned char word[]="Stimme";
    static const unsigned char annotation[]="`[.1StI.0m@].";
    const struct reloq_german_prosody_unit unit={.text=word,.length=sizeof word-1};
    const struct reloq_lexicon lexicon={0};
    return reloq_rules61_german_context_features(&unit,1,&lexicon,
        annotation,sizeof annotation-1,output,length,NULL,cancelled,&stopped,NULL);
}

struct native_output {char bytes[65536];size_t used;};
static int native_take(void *context,const char *data,size_t length)
{
    struct native_output *out=context;
    assert(length<=sizeof out->bytes-out->used);
    memcpy(out->bytes+out->used,data,length);out->used+=length;return 0;
}
static int stop_in_grammar(void *context)
{return ++*(unsigned *)context>=16;}
static void native_cancellation(void)
{
    /* Warm the real provider so the requested stop happens inside native
     * rule evaluation, independently of output-buffer or thread timing. */
    static const char unit[]="Die Stimme spricht einen langen Text. Danach liest sie weitere kurze Nachrichten. ";
    char text[4096];size_t used=0;
    for(unsigned i=0;i<32;++i) {
        assert(sizeof unit<=sizeof text-used);
        memcpy(text+used,unit,sizeof unit);used+=sizeof unit-1;
    }
    const struct reloq61_request q={.language=0x40000,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,92},
        .text=text};
    struct reloq61_result r;struct native_output before={0},partial={0},after={0};
    void *worker=reloq61_worker_new(q.language);assert(worker);
    void *previous_worker=reloq61_worker_scope(worker);
    assert(!__real_reloq61_generate_phonemes(&q,&r,native_take,&before)&&before.used);
    size_t frame_mark=evv_frame_mark();
    unsigned checked=0;
    struct reloq61_cancel_check previous=reloq61_cancel_scope(
        (struct reloq61_cancel_check){stop_in_grammar,&checked});
    assert(__real_reloq61_generate_phonemes(&q,&r,native_take,&partial));
    reloq61_cancel_scope(previous);
    assert(checked==16&&partial.used<before.used);
    assert(evv_frame_mark()==frame_mark);
    assert(!__real_reloq61_generate_phonemes(&q,&r,native_take,&after));
    assert(after.used==before.used&&!memcmp(after.bytes,before.bytes,before.used));
    reloq61_worker_scope(previous_worker);reloq61_worker_free(worker);
}

struct nested_output {uint64_t hash;size_t frames;int enter,returned;void *child;};
static void child_request(struct reloq61_request *q,char *text,size_t capacity)
{
    static const char unit[]="The voice reads a longer text with several complete sentences. ";
    size_t used=0;
    for(unsigned i=0;i<32;++i) {
        assert(sizeof unit<=capacity-used);
        memcpy(text+used,unit,sizeof unit);used+=sizeof unit-1;
    }
    *q=(struct reloq61_request){.language=0x10000,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,92},.text=text};
}
static int parent_frame(void *context,const int32_t frame[62],int32_t volume)
{
    struct nested_output *out=context;
    for(unsigned i=0;i<62;++i)out->hash=(out->hash^(uint32_t)frame[i])*UINT64_C(1099511628211);
    out->hash=(out->hash^(uint32_t)volume)*UINT64_C(1099511628211);
    ++out->frames;
    if(out->enter&&!out->returned) {
        struct reloq61_request q;char text[4096];child_request(&q,text,sizeof text);
        struct reloq61_result r;struct native_output partial={0};unsigned checked=0;
        void *previous_worker=reloq61_worker_scope(out->child);
        size_t frame_mark=evv_frame_mark();
        uintptr_t landing=evv_land_current_outermost();assert(landing);
        struct reloq61_cancel_check previous=reloq61_cancel_scope(
            (struct reloq61_cancel_check){stop_in_grammar,&checked});
        assert(__real_reloq61_generate_phonemes(&q,&r,native_take,&partial));
        reloq61_cancel_scope(previous);reloq61_worker_scope(previous_worker);
        assert(checked==16&&evv_frame_mark()==frame_mark);
        assert(evv_land_current_outermost()==landing);
        out->returned=1;
    }
    return 0;
}
static void nested_cancellation(void)
{
    void *parent=reloq61_worker_new(0x40000),*child=reloq61_worker_new(0x10000);
    assert(parent&&child);
    struct reloq61_request q;char text[4096];child_request(&q,text,sizeof text);
    struct reloq61_result r;struct native_output warm={0};
    void *previous=reloq61_worker_scope(child);
    assert(!__real_reloq61_generate_phonemes(&q,&r,native_take,&warm));
    reloq61_worker_scope(parent);
    struct nested_output before={.hash=UINT64_C(14695981039346656037)},
        after={.hash=UINT64_C(14695981039346656037),.enter=1,.child=child};
    q=(struct reloq61_request){.language=0x40000,.annotations=1,
        .voice={0,50,65,30,0,0,50,92},.text="Die Stimme liest einen Text.",
        .frame=parent_frame,.context=&before};
    assert(!reloq61_generate(&q,&r)&&before.frames);
    q.context=&after;
    assert(!reloq61_generate(&q,&r));
    assert(after.returned&&after.frames==before.frames&&after.hash==before.hash);
    reloq61_worker_scope(previous);reloq61_worker_free(child);reloq61_worker_free(parent);
}

int main(void)
{
    unsigned char *output=NULL;size_t length=0;
    provider_mode=1;
    assert(analyze(&output,&length)==2);
    assert(provider_calls==1&&sink_calls==2&&!output&&!length);
    /* Already-cancelled analysis must not enter the provider at all. */
    assert(analyze(&output,&length)==2&&provider_calls==1);
    stopped=0;provider_mode=2;
    assert(analyze(&output,&length)==-1);
    assert(!output&&!length); /* Real memory failures keep their status. */
    stopped=0;provider_mode=0;
    assert(analyze(&output,&length)==1&&output&&length);
    free(output);
    native_cancellation();
    nested_cancellation();
    puts("German phoneme analysis: native rule cancellation before output, chunk cancellation, status, memory failure and exact recovery passed");
    return 0;
}
