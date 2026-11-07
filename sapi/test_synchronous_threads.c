/* Each SAPI caller owns its ECI context and receives callbacks directly.
 * Thread changes and thread teardown must not corrupt subsequent speech. */
#define main other_backend_tests
#include "test_backend.c"
#undef main

struct request {
    struct sapi_backend *backend;
    struct sapi_settings *settings;
    struct site result;
};
static void *speak_on_thread(void *value)
{
    struct request *request=value;
    SPVTEXTFRAG f=text(L"This is a short message.",0);
    WAVEFORMATEX out=format(11025);
    request->result=site_new();
    request->result.caller=pthread_self();request->result.require_caller=1;
    assert(SUCCEEDED(sapi_backend_speak(request->backend,request->settings,0,&out,&f,&request->result.iface)));
    assert(request->result.count&&request->result.event_count);
    return NULL;
}
int main(int argc,char **argv)
{
    assert(argc==2);sapi_test_library=argv[1];struct sapi_settings settings_value=settings();
    struct sapi_backend *b=sapi_backend_new_prepared(0x10000,&settings_value);assert(b);
    struct request reference={.backend=b,.settings=&settings_value};
    speak_on_thread(&reference);
    for(unsigned i=0;i<70;++i){
        pthread_t thread;struct request next={.backend=b,.settings=&settings_value};
        assert(!pthread_create(&thread,NULL,speak_on_thread,&next));
        assert(!pthread_join(thread,NULL));
        same(&reference.result,&next.result);
        assert(reference.result.event_count==next.result.event_count);
        free(next.result.pcm);
    }
    /* Separate voices may also render concurrently on separate callers. */
    struct sapi_backend *other=sapi_backend_new(0x10000);assert(other);
    struct request a={.backend=b,.settings=&settings_value},c={.backend=other,.settings=&settings_value};
    pthread_t first,second;
    assert(!pthread_create(&first,NULL,speak_on_thread,&a));
    assert(!pthread_create(&second,NULL,speak_on_thread,&c));
    assert(!pthread_join(first,NULL)&&!pthread_join(second,NULL));
    same(&reference.result,&a.result);same(&reference.result,&c.result);
    free(a.result.pcm);free(c.result.pcm);free(reference.result.pcm);
    sapi_backend_delete(other);sapi_backend_delete(b);
    puts("70 caller-thread changes, cleanup and simultaneous SAPI contexts passed");
    return 0;
}
