/* A blocked SAPI output must not block synthesis of a short announcement.
 * Once the producer finishes, cancelling the buffered output must discard it
 * and leave the next announcement unchanged. No audio device is opened. */
#define _POSIX_C_SOURCE 200809L
#define main other_backend_tests
#include "test_backend.c"
#undef main
#include <time.h>

static unsigned before_synthesis;
static HRESULT blocked_write(ISpTTSEngineSite *p,const void *data,ULONG bytes,ULONG *written)
{
    struct site *s=(struct site *)p;
    HRESULT result=write_pcm(p,data,bytes,written);
    struct timespec delay={0,1000000};
    unsigned waited=0;
    while(__atomic_load_n(&sapi_test_synth_complete,__ATOMIC_SEQ_CST)==before_synthesis&&waited++<10000)
        nanosleep(&delay,NULL);
    assert(__atomic_load_n(&sapi_test_synth_complete,__ATOMIC_SEQ_CST)>before_synthesis);
    s->abort_after=s->count;
    return result;
}
static const ISpTTSEngineSiteVtbl blocked_site=
    {NULL,NULL,NULL,events,interest,actions,blocked_write,rate,volume,skip,complete};

int main(int argc,char **argv)
{
    assert(argc==2);sapi_test_library=argv[1];struct sapi_settings s=settings();
    const unsigned rates[]={11025,48000};
    for(size_t i=0;i<sizeof rates/sizeof *rates;++i){
        struct sapi_backend *b=sapi_backend_new(0x40000);assert(b);
        WAVEFORMATEX out=format(rates[i]);SPVTEXTFRAG text_fragment=text(L"Desktop",0);
        struct site reference=site_new();reference.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&text_fragment,&reference.iface)));
        assert(reference.count>rates[i]/20);
        struct site cancelled=site_new();cancelled.interest=0;cancelled.iface.lpVtbl=&blocked_site;
        before_synthesis=__atomic_load_n(&sapi_test_synth_complete,__ATOMIC_SEQ_CST);
        assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&text_fragment,&cancelled.iface)));
        assert(cancelled.writes==1&&cancelled.count==cancelled.abort_after&&cancelled.count<reference.count);
        struct site recovered=site_new();recovered.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&text_fragment,&recovered.iface)));
        same(&reference,&recovered);
        free(reference.pcm);free(cancelled.pcm);free(recovered.pcm);sapi_backend_delete(b);
    }
    puts("{\"buffered_cancel_cases\":2,\"producer_finishes_while_output_waits\":true,\"next_output_identical\":true}");
    return 0;
}
