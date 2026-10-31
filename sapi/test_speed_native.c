/* Actual SAPI transport at every host rate, XML extremes and optional boost.
 * Python compares these RAM hashes to direct native ECI controls. */
#define main other_backend_tests
#include "test_backend.c"
#undef main
#include "speed.h"
static void record(struct sapi_backend *b,int language,LONG speed,LONG adjustment,int boost,int *count)
{
    struct sapi_settings s=settings();s.boost=boost;struct site output=site_new();output.speed=speed;
    SPVTEXTFRAG f=text(language==0x40000?L"Ein kurzer Test.":L"A short test.",0);f.State.RateAdj=adjustment;
    WAVEFORMATEX out=format(11025);assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&output.iface)));
    assert(output.count>20);
    printf("%s{\"language\":%d,\"sapi_rate\":%d,\"xml_adjustment\":%d,\"boost\":%d,\"native_speed\":%d,\"samples\":%zu,\"hash\":\"%016llx\"}",
        (*count)++?",\n":"",language,(int)speed,(int)adjustment,boost,sapi_native_speed(speed,adjustment,boost),output.count,(unsigned long long)hash(output.pcm,output.count));free(output.pcm);
}
int main(int argc,char **argv)
{
    assert(argc==2);sapi_test_library=argv[1];int count=0;puts("{\"no_audio_files\":true,\"cases\":[");
    for(int i=0;i<2;++i){int language=i?0x40000:0x10000;struct sapi_backend *b=sapi_backend_new(language);assert(b);
        for(LONG speed=-10;speed<=10;++speed)record(b,language,speed,0,0,&count);
        record(b,language,10,5,0,&count);record(b,language,0,INT32_MAX,0,&count);
        record(b,language,0,INT32_MIN,0,&count);record(b,language,0,0,1,&count);record(b,language,10,0,1,&count);
        sapi_backend_delete(b);
    }
    printf("],\"case_count\":%d}\n",count);return 0;
}
