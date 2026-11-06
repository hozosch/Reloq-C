/* The speculative voice preparation must never change or output a request. */
#define _POSIX_C_SOURCE 200809L
#define main other_backend_tests
#include "test_backend.c"
#undef main
#include <time.h>

int main(int argc,char **argv)
{
    assert(argc==2);sapi_test_library=argv[1];unsigned cases=0;
    for(size_t i=0;i<sizeof sapi_languages/sizeof *sapi_languages;++i)
    for(unsigned choice=0;choice<2;++choice){
        struct sapi_settings config=settings();config.rate=choice?6:1;
        WAVEFORMATEX output=format(choice?48000:11025);
        SPVTEXTFRAG fragment=text(sapi_languages[i].sample,0);
        struct sapi_backend *normal=sapi_backend_new(sapi_languages[i].dialect);assert(normal);
        struct site expected=site_new();expected.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(normal,&config,0,&output,&fragment,&expected.iface)));
        assert(expected.count);sapi_backend_delete(normal);
        for(unsigned immediate=0;immediate<2;++immediate){
            unsigned before=__atomic_load_n(&sapi_test_synth_complete,__ATOMIC_SEQ_CST);
            struct sapi_backend *prepared=sapi_backend_new_prepared(sapi_languages[i].dialect,&config);assert(prepared);
            if(!immediate){
                struct timespec delay={0,1000000};unsigned waits=0;
                while(__atomic_load_n(&sapi_test_synth_complete,__ATOMIC_SEQ_CST)==before&&waits++<30000)nanosleep(&delay,NULL);
                assert(__atomic_load_n(&sapi_test_synth_complete,__ATOMIC_SEQ_CST)>before);
            }
            struct site actual=site_new();actual.interest=0;
            assert(SUCCEEDED(sapi_backend_speak(prepared,&config,0,&output,&fragment,&actual.iface)));
            same(&expected,&actual);assert(!actual.event_count);
            free(actual.pcm);sapi_backend_delete(prepared);++cases;
        }
        free(expected.pcm);
    }
    printf("Prepared and immediate SAPI startup: %u matching audio cases passed\n",cases);
    return 0;
}
