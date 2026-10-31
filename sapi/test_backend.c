/* Exercise production ECI calls and direct SAPI delivery
 * against the real C core. PCM stays in memory and is never written. */
#include "settings.h"
#include "format.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "catalogue.inc"
struct site {
    ISpTTSEngineSite iface;short *pcm;size_t count,capacity,abort_after;
    SPEVENT events[4096];size_t event_count;ULONG partial;int fail_after,writes;
    LONG speed;USHORT volume;struct sapi_backend *nested;int nested_done;
    ULONGLONG interest;
    struct sapi_settings *settings;
};
static HRESULT events(ISpTTSEngineSite *p,const SPEVENT *e,ULONG count)
{struct site *s=(struct site *)p;assert(s->event_count+count<4096);
    for(ULONG i=0;i<count;++i){assert(e[i].ullAudioStreamOffset==s->count*2);
    if(e[i].eEventId==SPEI_TTS_BOOKMARK)assert(wcslen((const WCHAR *)e[i].lParam)>0);
    if(s->event_count)assert(e[i].ullAudioStreamOffset>=s->events[s->event_count-1].ullAudioStreamOffset);
    s->events[s->event_count++]=e[i];}return S_OK;}
static HRESULT interest(ISpTTSEngineSite *p,ULONGLONG *out){*out=((struct site *)p)->interest;return S_OK;}
static DWORD actions(ISpTTSEngineSite *p){struct site *s=(struct site *)p;return s->abort_after&&s->count>=s->abort_after?SPVES_ABORT:0;}
static HRESULT rate(ISpTTSEngineSite *p,LONG *out){*out=((struct site *)p)->speed;return S_OK;}
static HRESULT volume(ISpTTSEngineSite *p,USHORT *out){*out=((struct site *)p)->volume;return S_OK;}
static const ISpTTSEngineSiteVtbl vtable;
static SPVTEXTFRAG text(const WCHAR *value,ULONG offset)
{SPVTEXTFRAG f={0};f.pTextStart=value;f.ulTextLen=(ULONG)wcslen(value);f.ulTextSrcOffset=offset;f.State.Volume=100;return f;}
static WAVEFORMATEX format(unsigned hz){return (WAVEFORMATEX){1,1,hz,hz*2,2,16,0};}
static struct site site_new(void){return (struct site){.iface={&vtable},.volume=100,.interest=~(ULONGLONG)0};}
static HRESULT write_pcm(ISpTTSEngineSite *p,const void *data,ULONG bytes,ULONG *written)
{
    struct site *s=(struct site *)p;assert(!(bytes&1));
    if(s->nested&&!s->nested_done){s->nested_done=1;struct site child=site_new();
        SPVTEXTFRAG f=text(L"This priority message must not deadlock.",0);WAVEFORMATEX out=format(11025);
        assert(SUCCEEDED(sapi_backend_speak(s->nested,s->settings,0,&out,&f,&child.iface)));assert(child.count>0);free(child.pcm);}
    ++s->writes;
    if(s->fail_after&&s->writes>=s->fail_after){*written=0;return E_FAIL;}
    if(s->partial&&bytes>s->partial)bytes=s->partial;*written=bytes;
    size_t count=bytes/2;if(s->count+count>s->capacity){s->capacity=(s->count+count)*2;
        void *mem=realloc(s->pcm,s->capacity*sizeof *s->pcm);assert(mem);s->pcm=mem;}
    memcpy(s->pcm+s->count,data,bytes);s->count+=count;return S_OK;
}
static HRESULT skip(ISpTTSEngineSite *p,SPVSKIPTYPE *type,LONG *n){(void)p;*type=SPVST_SENTENCE;*n=1;return S_OK;}
static HRESULT complete(ISpTTSEngineSite *p,LONG n){(void)p;(void)n;return S_OK;}
static const ISpTTSEngineSiteVtbl vtable={NULL,NULL,NULL,events,interest,actions,write_pcm,rate,volume,skip,complete};
static struct sapi_settings settings(void)
{struct sapi_settings s={.schema=1,.profile=0,.head=50,.pitch=65,.inflection=30,.rate=1,.abbreviations=1};return s;}
static uint64_t hash(const short *pcm,size_t count)
{uint64_t h=1469598103934665603ULL;for(size_t i=0;i<count;++i){h^=(uint16_t)pcm[i];h*=1099511628211ULL;}return h;}
static void same(const struct site *a,const struct site *b)
{assert(a->count==b->count);assert(!memcmp(a->pcm,b->pcm,a->count*2));}
int main(int argc,char **argv)
{
    assert(argc==2);sapi_test_library=argv[1];struct sapi_settings s=settings();unsigned cases=0;
    struct {int language,rate;size_t samples;uint64_t hash;} comparisons[98];unsigned comparison_count=0,format_changes=0;
    for(size_t i=0;i<sizeof sapi_languages/sizeof *sapi_languages;++i){
        struct sapi_backend *b=sapi_backend_new(sapi_languages[i].dialect);assert(b);
        for(unsigned r=0;r<7;++r){struct site output=site_new();SPVTEXTFRAG f=text(sapi_languages[i].sample,0);
            WAVEFORMATEX out=format((unsigned)sapi_rates[r]);
            HRESULT hr=sapi_backend_speak(b,&s,0,&out,&f,&output.iface);
            if(FAILED(hr)){fprintf(stderr,"language %ls, rate %d: %08x\n",sapi_languages[i].name,sapi_rates[r],(unsigned)hr);abort();}
            assert(output.count>100);assert(output.event_count>0);
            comparisons[comparison_count].language=sapi_languages[i].dialect;
            comparisons[comparison_count].rate=(int)r;comparisons[comparison_count].samples=output.count;
            comparisons[comparison_count++].hash=hash(output.pcm,output.count);
            free(output.pcm);++cases;
        }
        if(sapi_languages[i].dialect==0x10000||sapi_languages[i].dialect==0x40000)
            for(unsigned preferred=0;preferred<7;++preferred)for(unsigned next=0;next<7;++next){
                /* Requests go through the production format policy. The
                 * engine supplies its selected native signal to SAPI;
                 * Windows, not this backend, converts to the target. */
                unsigned r=preferred&1?6-next:next,advertised=0;s.rate=preferred;
                struct site output=site_new();SPVTEXTFRAG f=text(sapi_languages[i].sample,0);
                WAVEFORMATEX actual=format(sapi_format_offer(&advertised,(unsigned)sapi_rates[preferred],(unsigned)sapi_rates[r]));
                assert(sapi_format_accepts(advertised,actual.nSamplesPerSec));
                assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&actual,&f,&output.iface)));
                assert(output.count==comparisons[i*7+preferred].samples);
                assert(hash(output.pcm,output.count)==comparisons[i*7+preferred].hash);
                free(output.pcm);++cases;++format_changes;
            }
        s.rate=1;sapi_backend_delete(b);
    }
    struct sapi_backend *b=sapi_backend_new(0x10000);assert(b);WAVEFORMATEX out=format(11025);
    SPVTEXTFRAG f=text(L"One computer. The next sentence works.",0);struct site base=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&base.iface)));
    assert(base.events[0].eEventId==SPEI_SENTENCE_BOUNDARY&&base.events[0].wParam==13&&base.events[0].lParam==0);
    assert(base.events[1].eEventId==SPEI_WORD_BOUNDARY&&base.events[1].wParam==3&&base.events[1].lParam==0);
    SPVTEXTFRAG a=text(L"One comp",0),c=text(L"uter. The next sentence works.",8);a.pNext=&c;
    struct site fragmented=site_new();fragmented.partial=38;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&a,&fragmented.iface)));same(&base,&fragmented);free(fragmented.pcm);++cases;
    struct site uninterested=site_new();uninterested.interest=0;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&uninterested.iface)));
    assert(!uninterested.event_count);same(&base,&uninterested);
    /* A plain stream is delivered in large blocks after its immediate start,
     * rather than one small Write per native callback. */
    assert(uninterested.writes>1&&uninterested.writes<=(int)(uninterested.count/4096)+2);
    free(uninterested.pcm);++cases;
    struct site half=site_new();half.volume=50;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&half.iface)));assert(half.count==base.count);
    for(size_t i=0;i<base.count;++i)assert(half.pcm[i]==base.pcm[i]/2);free(half.pcm);++cases;
    SPVTEXTFRAG xml=f;xml.State.Volume=50;struct site soft=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&xml,&soft.iface)));assert(soft.count==base.count);
    for(size_t i=0;i<base.count;++i)assert(soft.pcm[i]==base.pcm[i]/2);free(soft.pcm);++cases;
    xml=f;xml.State.RateAdj=3;struct site fast=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&xml,&fast.iface)));assert(fast.count<base.count);free(fast.pcm);++cases;
    xml=f;xml.State.PitchAdj.MiddleAdj=2;struct site high=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&xml,&high.iface)));
    assert(high.count==base.count&&hash(high.pcm,high.count)!=hash(base.pcm,base.count));free(high.pcm);++cases;
    struct site cancel=site_new();cancel.abort_after=512;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&cancel.iface)));assert(cancel.count<base.count);free(cancel.pcm);++cases;
    struct site after=site_new();assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&after.iface)));same(&base,&after);free(after.pcm);++cases;
    struct site failure=site_new();failure.fail_after=2;
    assert(FAILED(sapi_backend_speak(b,&s,0,&out,&f,&failure.iface)));free(failure.pcm);++cases;
    struct sapi_settings female=s;female.gender=1;female.head=22;female.pitch=93;
    struct site changed=site_new();assert(SUCCEEDED(sapi_backend_speak(b,&female,0,&out,&f,&changed.iface)));
    assert(hash(changed.pcm,changed.count)!=hash(base.pcm,base.count));free(changed.pcm);++cases;
    struct site nested=site_new();nested.nested=sapi_backend_new(0x10000);assert(nested.nested);nested.settings=&s;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&nested.iface)));assert(nested.nested_done);same(&base,&nested);
    sapi_backend_delete(nested.nested);free(nested.pcm);++cases;
    WCHAR invalid[]={0xdc00,0};SPVTEXTFRAG bad=text(invalid,0);struct site rejected=site_new();
    assert(FAILED(sapi_backend_speak(b,&s,0,&out,&bad,&rejected.iface)));assert(!rejected.count);free(rejected.pcm);++cases;
    SPVTEXTFRAG mark=text(L"7",0);mark.State.eAction=SPVA_Bookmark;struct site bookmark=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&mark,&bookmark.iface)));
    assert(!bookmark.count&&bookmark.event_count==1&&bookmark.events[0].wParam==7);free(bookmark.pcm);++cases;
    WCHAR bounded_name[]={L'8',L'9',L'X'};mark=text(L"",0);mark.pTextStart=bounded_name;mark.ulTextLen=2;mark.State.eAction=SPVA_Bookmark;
    struct site bounded=site_new();assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&mark,&bounded.iface)));
    assert(bounded.event_count==1&&bounded.events[0].wParam==89);free(bounded.pcm);++cases;
    SPVTEXTFRAG spell=text(L"AB",5);spell.State.eAction=SPVA_SpellOut;struct site letters=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&spell,&letters.iface)));assert(letters.count>0);free(letters.pcm);++cases;
    assert(sapi_test_dictionary_calls==0);++cases;
    printf("{\"backend_cases\":%u,\"format_negotiation_cases\":%u,\"host_tests_convert_audio\":false,\"languages\":14,\"rates\":7,\"reference_samples\":%zu,\"reference_hash\":\"%016llx\",\"profile_names\":8,\"last_profile\":\"%ls\",\"comparisons\":[",
        cases,format_changes,base.count,(unsigned long long)hash(base.pcm,base.count),sapi_profiles[7]);
    for(unsigned i=0;i<comparison_count;++i)printf("%s{\"language\":%d,\"rate\":%d,\"samples\":%zu,\"hash\":\"%016llx\"}",
        i?",":"",comparisons[i].language,comparisons[i].rate,comparisons[i].samples,(unsigned long long)comparisons[i].hash);
    puts("]}");
    (void)sapi_anatomy;free(base.pcm);sapi_backend_delete(b);return 0;
}
