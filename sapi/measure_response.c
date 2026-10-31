/* Host transport timing, distinct from Windows/device audio latency. */
#define _POSIX_C_SOURCE 200809L
#define main other_backend_tests
#include "test_backend.c"
#undef main
#include <time.h>
static double start,first;
static double now(void)
{struct timespec t;assert(!clock_gettime(CLOCK_MONOTONIC,&t));return t.tv_sec+t.tv_nsec*1e-9;}
static HRESULT timed_write(ISpTTSEngineSite *p,const void *data,ULONG bytes,ULONG *written)
{if(!first)first=now()-start;return write_pcm(p,data,bytes,written);}
static const ISpTTSEngineSiteVtbl timing_vtable={NULL,NULL,NULL,events,interest,actions,timed_write,rate,volume,skip,complete};
int main(int argc,char **argv)
{
    assert(argc==3);sapi_test_library=argv[1];puts("{\"host_transport_only\":true,\"no_audio_files\":true,\"records\":[");int record=0;
    for(int lang=0;lang<2;++lang)for(int enabled=0;enabled<2;++enabled)for(int hz=0;hz<2;++hz){
        struct sapi_backend *b=sapi_backend_new(lang?0x40000:0x10000);assert(b);struct sapi_settings s=settings();s.dictionaries=enabled;
        assert(strlen(argv[2])<1024);for(size_t i=0;i<=strlen(argv[2]);++i)s.directory[i]=(unsigned char)argv[2][i];
        WAVEFORMATEX out=format(hz?48000:11025);
        for(int i=0;i<13;++i){WCHAR words[128];swprintf(words,128,lang?L"Dies ist die kurze Nachricht Nummer %d.":L"This is short message number %d.",i+1);
            SPVTEXTFRAG f=text(words,0);struct site result=site_new();result.iface.lpVtbl=&timing_vtable;
            first=0;start=now();HRESULT hr=sapi_backend_speak(b,&s,0,&out,&f,&result.iface);double total=now()-start;
            assert(SUCCEEDED(hr)&&result.count>0&&first>0);
            printf("%s{\"language\":%d,\"dictionaries\":%d,\"hz\":%u,\"iteration\":%d,\"first_write_ms\":%.6f,\"total_ms\":%.6f,\"samples\":%zu,\"hash\":\"%016llx\"}",
                record++?",\n":"",lang?0x40000:0x10000,enabled,(unsigned)out.nSamplesPerSec,i,first*1000,total*1000,result.count,(unsigned long long)hash(result.pcm,result.count));free(result.pcm);
        }
        sapi_backend_delete(b);
    }
    printf("],\"dictionary_reload_calls\":%u}\n",sapi_test_dictionary_calls);return 0;
}
