/* Exercise production ECI calls and direct SAPI delivery
 * against the real C core. PCM stays in memory and is never written. */
#include "settings.h"
#include "format.h"
#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <pthread.h>
#include "catalogue.inc"
struct site {
    ISpTTSEngineSite iface;short *pcm;size_t count,capacity,abort_after;
    size_t playback_threshold,first_playable_samples,first_write_samples,batched_after;
    int post_start_writes;
    SPEVENT events[4096];size_t event_count;ULONG partial;int fail_after,writes;
    LONG speed;USHORT volume;struct sapi_backend *nested;int nested_done;
    size_t volume_change_after;int volume_changed;
    pthread_t caller;int require_caller;
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
static DWORD actions(ISpTTSEngineSite *p)
{struct site *s=(struct site *)p;
    if(s->abort_after&&s->count>=s->abort_after)return SPVES_ABORT;
    if(s->volume_change_after&&!s->volume_changed&&s->count>=s->volume_change_after){s->volume=50;s->volume_changed=1;return SPVES_VOLUME;}
    return 0;}
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
    if(s->require_caller)assert(pthread_equal(s->caller,pthread_self()));
    if(s->nested&&!s->nested_done){s->nested_done=1;struct site child=site_new();
        SPVTEXTFRAG f=text(L"This priority message must not deadlock.",0);WAVEFORMATEX out=format(11025);
        assert(SUCCEEDED(sapi_backend_speak(s->nested,s->settings,0,&out,&f,&child.iface)));assert(child.count>0);free(child.pcm);}
    ++s->writes;
    if(s->batched_after&&s->count>=s->batched_after)++s->post_start_writes;
    if(s->fail_after&&s->writes>=s->fail_after){*written=0;return E_FAIL;}
    if(s->partial&&bytes>s->partial)bytes=s->partial;*written=bytes;
    size_t count=bytes/2;if(!s->first_write_samples)s->first_write_samples=count;
    if(s->count+count>s->capacity){s->capacity=(s->count+count)*2;
        void *mem=realloc(s->pcm,s->capacity*sizeof *s->pcm);assert(mem);s->pcm=mem;}
    memcpy(s->pcm+s->count,data,bytes);s->count+=count;
    if(s->playback_threshold&&!s->first_playable_samples&&s->count>=s->playback_threshold)
        s->first_playable_samples=s->count;
    return S_OK;
}
static HRESULT skip(ISpTTSEngineSite *p,SPVSKIPTYPE *type,LONG *n){(void)p;*type=SPVST_SENTENCE;*n=1;return S_OK;}
static HRESULT complete(ISpTTSEngineSite *p,LONG n){(void)p;(void)n;return S_OK;}
static const ISpTTSEngineSiteVtbl vtable={NULL,NULL,NULL,events,interest,actions,write_pcm,rate,volume,skip,complete};
static struct sapi_settings settings(void)
{struct sapi_settings s={.schema=1,.profile=0,.head=50,.pitch=65,.inflection=30,.rate=1,.abbreviations=1,
    .eq_low=12,.eq_mid=12,.eq_high=12,
    .eq_filters={{0,125,12,100},{0,500,12,100},{0,1200,12,100},{0,3000,12,100},{0,6000,12,100}}};return s;}
static uint64_t hash(const short *pcm,size_t count)
{uint64_t h=1469598103934665603ULL;for(size_t i=0;i<count;++i){h^=(uint16_t)pcm[i];h*=1099511628211ULL;}return h;}
static void same(const struct site *a,const struct site *b)
{assert(a->count==b->count);assert(!memcmp(a->pcm,b->pcm,a->count*2));}
int main(int argc,char **argv)
{
    assert(argc==2);sapi_test_library=argv[1];struct sapi_settings s=settings();unsigned cases=0;
    struct {int language,rate;size_t samples;uint64_t hash;unsigned event_count;
        ULONG event_offsets[128];uint64_t event_audio_offsets[128];}
        comparisons[(sizeof sapi_languages/sizeof *sapi_languages)*7];
    unsigned comparison_count=0,format_changes=0;
    size_t hz_samples[3]={0};uint64_t hz_hashes[3]={0};
    size_t long_marked=0,long_plain=0,long_events=0;uint64_t long_marked_hash=0,long_plain_hash=0,long_event_hash=0;
    for(size_t i=0;i<sizeof sapi_languages/sizeof *sapi_languages;++i){
        struct sapi_backend *b=sapi_backend_new(sapi_languages[i].dialect);assert(b);
        for(unsigned r=0;r<7;++r){struct site output=site_new();SPVTEXTFRAG f=text(sapi_languages[i].sample,0);
            WAVEFORMATEX out=format((unsigned)sapi_rates[r]);
            HRESULT hr=sapi_backend_speak(b,&s,0,&out,&f,&output.iface);
            if(FAILED(hr)){fprintf(stderr,"language %ls, rate %d: %08x\n",sapi_languages[i].name,sapi_rates[r],(unsigned)hr);abort();}
            assert(output.count>100);assert(output.event_count>0);
            comparisons[comparison_count].language=sapi_languages[i].dialect;
            comparisons[comparison_count].rate=(int)r;comparisons[comparison_count].samples=output.count;
            comparisons[comparison_count].hash=hash(output.pcm,output.count);
            assert(output.event_count<=128);
            comparisons[comparison_count].event_count=(unsigned)output.event_count;
            for(size_t k=0;k<output.event_count;++k){
                assert(output.events[k].eEventId==SPEI_WORD_BOUNDARY||output.events[k].eEventId==SPEI_SENTENCE_BOUNDARY);
                assert((uint64_t)output.events[k].lParam<=f.ulTextLen);
                comparisons[comparison_count].event_offsets[k]=(ULONG)output.events[k].lParam;
                comparisons[comparison_count].event_audio_offsets[k]=output.events[k].ullAudioStreamOffset;
            }
            ++comparison_count;
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
#ifndef RELOQ_SAPI_REFERENCE_BASELINE
    /* Japanese text must not gain pauses merely because a host requests
     * events or divides the same input into many SAPI text fragments. */
    {
        static const WCHAR *const phrases[]={
            L"これは音声のテストです。Windowsとcomputer。",
            L"こんにちは、今日は良い天気です。次の文章です。",
            L"NVDAの音声テストです。私は日本語を話します。",
            L"こんにちは 世界 こんにちは 世界。",
            L"Windows 11 を使います。"};
        static const unsigned rates[]={11025,16000,48000};
        char original_locale[128];const char *locale=setlocale(LC_CTYPE,NULL);
        assert(locale&&strlen(locale)<sizeof original_locale);strcpy(original_locale,locale);
        assert(setlocale(LC_CTYPE,"C.UTF-8"));
        struct sapi_backend *ja=sapi_backend_new(0x80000);assert(ja);
        for(unsigned pause=0;pause<3;++pause)for(unsigned r=0;r<3;++r){
            s.pauses=pause;WAVEFORMATEX fmt=format(rates[r]);
            for(size_t k=0;k<sizeof phrases/sizeof *phrases;++k){
                const WCHAR *value=phrases[k];size_t length=wcslen(value);
                SPVTEXTFRAG whole=text(value,0),*pieces=calloc(length,sizeof *pieces);assert(pieces);
                for(size_t i=0;i<length;++i){pieces[i]=text(value+i,(ULONG)i);pieces[i].ulTextLen=1;
                    pieces[i].pNext=i+1<length?pieces+i+1:NULL;}
                struct site marked=site_new(),plain=site_new(),split=site_new();plain.interest=0;
                assert(SUCCEEDED(sapi_backend_speak(ja,&s,0,&fmt,&whole,&marked.iface)));
                assert(SUCCEEDED(sapi_backend_speak(ja,&s,0,&fmt,&whole,&plain.iface)));
                assert(SUCCEEDED(sapi_backend_speak(ja,&s,0,&fmt,pieces,&split.iface)));
                assert(marked.count&&marked.event_count&&!plain.event_count);
                if(marked.count!=plain.count||marked.count!=split.count)
                    fprintf(stderr,"Japanese pause=%u rate=%u text=%zu: marked=%zu plain=%zu split=%zu\n",
                        pause,rates[r],k,marked.count,plain.count,split.count);
                same(&marked,&plain);same(&marked,&split);
                free(pieces);free(marked.pcm);free(plain.pcm);free(split.pcm);++cases;
            }
        }
        sapi_backend_delete(ja);s.pauses=0;assert(setlocale(LC_CTYPE,original_locale));
    }
#endif
    struct site uninterested=site_new();uninterested.interest=0;
    uninterested.batched_after=(out.nSamplesPerSec*500+999)/1000;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&uninterested.iface)));
    assert(!uninterested.event_count);same(&base,&uninterested);
    /* Delivery remains synchronous on the caller, with bounded blocks. */
    assert(uninterested.writes>1);
    assert(uninterested.first_write_samples<=(out.nSamplesPerSec+19)/20);
    free(uninterested.pcm);++cases;
    /* Start immediately, even when the consumer needs to collect 50 ms
     * before playback; bound how much later audio is buffered. */
    static const unsigned startup_rates[]={8000,11025,16000,22050,32000,44100,48000};
    for(size_t k=0;k<sizeof startup_rates/sizeof *startup_rates;++k){
        WAVEFORMATEX early_format=format(startup_rates[k]);
        struct site early=site_new();early.interest=0;
        early.caller=pthread_self();early.require_caller=1;
        early.playback_threshold=(startup_rates[k]+19)/20;
        SPVTEXTFRAG early_text=text(L"A clear sentence starts playing promptly.",0);
        assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&early_format,&early_text,&early.iface)));
        assert(early.first_playable_samples>=early.playback_threshold);
#ifndef RELOQ_SAPI_REFERENCE_BASELINE
        assert(early.first_write_samples>0);
        assert(early.first_write_samples==early.playback_threshold);
        assert(early.first_playable_samples==early.playback_threshold);
        struct site boosted=site_new();boosted.interest=0;
        boosted.playback_threshold=(startup_rates[k]*300+999)/1000;
        assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&early_format,&early_text,&boosted.iface)));
        assert(boosted.first_playable_samples>=boosted.playback_threshold);
        assert(boosted.first_playable_samples<boosted.playback_threshold+(startup_rates[k]+49)/50);
        free(boosted.pcm);++cases;
#endif
        free(early.pcm);++cases;
    }
    /* Capture long-text PCM and ordered events with and without interest.
     * A separate baseline comparison verifies that each remains unchanged. */
    {
        WCHAR long_text[4096]=L"";
        for(unsigned i=0;i<50;++i)wcscat(long_text,L"Dies ist ein langer Text mit mehreren Worten, ");
        wcscat(long_text,L"Ende.");
        SPVTEXTFRAG long_fragment=text(long_text,0);
        struct sapi_backend *german=sapi_backend_new(0x40000);assert(german);
        struct site marked=site_new(),plain=site_new();plain.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(german,&s,0,&out,&long_fragment,&marked.iface)));
        assert(SUCCEEDED(sapi_backend_speak(german,&s,0,&out,&long_fragment,&plain.iface)));
        assert(marked.count>0&&plain.count>0&&!plain.event_count);
        long_marked=marked.count;long_plain=plain.count;long_events=marked.event_count;
        long_marked_hash=hash(marked.pcm,marked.count);long_plain_hash=hash(plain.pcm,plain.count);
        long_event_hash=1469598103934665603ULL;
        for(size_t i=0;i<marked.event_count;++i){const SPEVENT *e=&marked.events[i];
            long_event_hash=(long_event_hash^e->eEventId)*1099511628211ULL;
            long_event_hash=(long_event_hash^e->wParam)*1099511628211ULL;
            long_event_hash=(long_event_hash^(uint64_t)e->lParam)*1099511628211ULL;
            long_event_hash=(long_event_hash^e->ullAudioStreamOffset)*1099511628211ULL;}
        size_t words=0;ULONG previous=0;
        for(size_t i=0;i<marked.event_count;++i)if(marked.events[i].eEventId==SPEI_WORD_BOUNDARY){
            ULONG position=(ULONG)marked.events[i].lParam;
            assert(marked.events[i].wParam>0&&position<long_fragment.ulTextLen);
            assert(!words||position>=previous);previous=position;++words;
        }
        assert(words>300);
        free(marked.pcm);free(plain.pcm);sapi_backend_delete(german);++cases;
    }
    struct site half=site_new();half.volume=50;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&half.iface)));assert(half.count==base.count);
    for(size_t i=0;i<base.count;++i)assert(half.pcm[i]==base.pcm[i]/2);free(half.pcm);++cases;
    struct site changed_volume=site_new();changed_volume.interest=0;changed_volume.volume_change_after=1;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&changed_volume.iface)));
    assert(changed_volume.volume_changed&&changed_volume.count==base.count);
    for(size_t i=0;i<base.count;++i)
        assert(changed_volume.pcm[i]==(i<changed_volume.first_write_samples?base.pcm[i]:base.pcm[i]/2));
    free(changed_volume.pcm);++cases;
    SPVTEXTFRAG xml=f;xml.State.Volume=50;struct site soft=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&xml,&soft.iface)));assert(soft.count==base.count);
    for(size_t i=0;i<base.count;++i)assert(soft.pcm[i]==base.pcm[i]/2);free(soft.pcm);++cases;
    xml=f;xml.State.RateAdj=3;struct site fast=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&xml,&fast.iface)));assert(fast.count<base.count);free(fast.pcm);++cases;
    xml=f;xml.State.PitchAdj.MiddleAdj=2;struct site high=site_new();
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&xml,&high.iface)));
    assert(high.count==base.count&&hash(high.pcm,high.count)!=hash(base.pcm,base.count));free(high.pcm);++cases;
    #ifndef RELOQ_SAPI_REFERENCE_BASELINE
    {
        /* The adapter must forward both simple and freely tuned filters,
         * and switching off must restore bit-identical ordinary output. */
        struct sapi_settings eq=s;eq.eq_enabled=1;eq.eq_preset=1;
        struct site warm=site_new();warm.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(b,&eq,0,&out,&f,&warm.iface)));
        assert(warm.count==base.count&&hash(warm.pcm,warm.count)!=hash(base.pcm,base.count));
        eq.eq_preset=0;eq.eq_filters[0]=(struct sapi_eq_filter){1,777,18,50};
        struct site custom=site_new();custom.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(b,&eq,0,&out,&f,&custom.iface)));
        assert(custom.count==base.count&&hash(custom.pcm,custom.count)!=hash(base.pcm,base.count));
        eq.eq_enabled=0;struct site restored=site_new();restored.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(b,&eq,0,&out,&f,&restored.iface)));same(&base,&restored);
        free(warm.pcm);free(custom.pcm);free(restored.pcm);cases+=3;
    }
    /* Per-engine overrides ignore SAPI and XML values while remaining
     * independent, including mid-output application volume changes. */
    {
        struct sapi_settings own=s;own.speed_override=1;own.speed_value=95;
        own.pitch_override=1;own.volume_override=1;own.volume_value=40;
        SPVTEXTFRAG a=f,bfrag=f;
        a.State.RateAdj=-5;a.State.PitchAdj.MiddleAdj=-7;a.State.Volume=10;
        bfrag.State.RateAdj=7;bfrag.State.PitchAdj.MiddleAdj=9;bfrag.State.Volume=80;
        struct site left=site_new(),right=site_new();left.speed=-10;left.volume=15;
        right.speed=10;right.volume=95;right.volume_change_after=1;
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&a,&left.iface)));
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&bfrag,&right.iface)));
        assert(left.count>0&&right.volume_changed);same(&left,&right);
        own.boost=1;struct site boosted=site_new();boosted.speed=7;boosted.volume=1;
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&a,&boosted.iface)));same(&left,&boosted);
        own.speed_override=0;own.boost=0;struct site different_rate=site_new();different_rate.speed=10;
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&bfrag,&different_rate.iface)));
        assert(different_rate.count!=left.count);
        own.speed_override=1;own.pitch_override=0;struct site different_pitch=site_new();
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&bfrag,&different_pitch.iface)));
        assert(different_pitch.count==left.count&&hash(different_pitch.pcm,different_pitch.count)!=hash(left.pcm,left.count));
        own.pitch_override=1;own.volume_value=0;struct site silent=site_new();
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&a,&silent.iface)));
        assert(silent.count==left.count);for(size_t i=0;i<silent.count;++i)assert(silent.pcm[i]==0);
        free(left.pcm);free(right.pcm);free(boosted.pcm);free(different_rate.pcm);free(different_pitch.pcm);free(silent.pcm);cases+=6;
    }
    {
        /* A WPM override has the same native timing as a VWPM tag, even
         * when the application-facing ECI tag option is switched off. */
        struct sapi_settings tagged=s;tagged.tags=1;
        SPVTEXTFRAG tagged_f=text(L"`vwpm150 One computer. The next sentence works.",0);
        struct site expected=site_new();expected.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(b,&tagged,0,&out,&tagged_f,&expected.iface)));
        struct sapi_settings own=s;own.speed_override=1;own.speed_wpm_enabled=1;own.speed_wpm=150;
        struct site actual=site_new();actual.interest=0;actual.speed=-10;
        SPVTEXTFRAG shifted=f;shifted.State.RateAdj=10;
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&shifted,&actual.iface)));same(&expected,&actual);
        struct site marked=site_new();
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&shifted,&marked.iface)));same(&actual,&marked);
        assert(marked.event_count==base.event_count);
        for(size_t i=0;i<base.event_count;++i)
            assert(marked.events[i].eEventId==base.events[i].eEventId&&marked.events[i].wParam==base.events[i].wParam&&marked.events[i].lParam==base.events[i].lParam);
        own.speed_wpm_enabled=0;own.speed_value=95;
        struct site eci=site_new();eci.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(b,&own,0,&out,&f,&eci.iface)));
        assert(eci.count!=actual.count);
        struct site restored=site_new();restored.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&restored.iface)));same(&base,&restored);
        free(expected.pcm);free(actual.pcm);free(marked.pcm);free(eci.pcm);free(restored.pcm);cases+=4;
    }
    /* Exact Hertz stays the base for SAPI's neutral 50. Relative pitch
     * offsets and interior fragments keep the same continuous ECI stream. */
    {
        struct sapi_backend *voice=sapi_backend_new(0x40000);assert(voice);
        struct sapi_settings h=settings();h.rate=3;h.inflection=0;h.pitch_hz=151;h.pitch_hz_enabled=1;
        WAVEFORMATEX fmt=format(16000);
        SPVTEXTFRAG phrase=text(L"Hallo Welt.",0),raised=phrase;
        raised.State.PitchAdj.MiddleAdj=10;
        SPVTEXTFRAG left=text(L"Hallo ",0),right=text(L"Welt.",6);
        left.pNext=&right;right.State.PitchAdj.MiddleAdj=10;
        SPVTEXTFRAG *fragments[]={&phrase,&raised,&left};
        for(unsigned i=0;i<3;++i){
            struct site audible=site_new();audible.interest=0;
            assert(SUCCEEDED(sapi_backend_speak(voice,&h,0,&fmt,fragments[i],&audible.iface)));
            hz_samples[i]=audible.count;hz_hashes[i]=hash(audible.pcm,audible.count);
            free(audible.pcm);
        }
        assert(hz_samples[0]==hz_samples[1]&&hz_hashes[0]!=hz_hashes[1]);
        h.pitch_override=1;
        struct site fixed_hz=site_new();fixed_hz.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(voice,&h,0,&fmt,&raised,&fixed_hz.iface)));
        assert(fixed_hz.count==hz_samples[0]&&hash(fixed_hz.pcm,fixed_hz.count)==hz_hashes[0]);
        free(fixed_hz.pcm);h.pitch_override=0;
        struct site restored=site_new();restored.interest=0;
        assert(SUCCEEDED(sapi_backend_speak(voice,&h,0,&fmt,&phrase,&restored.iface)));
        assert(restored.count==hz_samples[0]&&hash(restored.pcm,restored.count)==hz_hashes[0]);
        free(restored.pcm);sapi_backend_delete(voice);cases+=5;
    }
    #endif
    struct site cancel=site_new();cancel.abort_after=512;
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&cancel.iface)));assert(cancel.count<base.count);free(cancel.pcm);++cases;
    struct site after=site_new();assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&f,&after.iface)));same(&base,&after);free(after.pcm);++cases;
    struct site failure=site_new();failure.fail_after=2;
    assert(FAILED(sapi_backend_speak(b,&s,0,&out,&f,&failure.iface)));free(failure.pcm);++cases;
    /* Mellow mode follows the stored setting on successive SAPI requests,
     * without changing the duration or the audio/index delivery contract. */
    {
        struct sapi_backend *voice=sapi_backend_new(0x40000);assert(voice);
        struct sapi_settings m=s; m.rate=3;
        WAVEFORMATEX wide=format(16000);
        SPVTEXTFRAG phrase=text(L"Eine weiche Stimme zum Vergleichen.",0);
        struct site regular=site_new(),mellow=site_new(),restored=site_new();
        assert(SUCCEEDED(sapi_backend_speak(voice,&m,0,&wide,&phrase,&regular.iface)));
        m.mellow_mode=1;assert(SUCCEEDED(sapi_backend_speak(voice,&m,0,&wide,&phrase,&mellow.iface)));
        assert(regular.count==mellow.count&&hash(regular.pcm,regular.count)!=hash(mellow.pcm,mellow.count));
        assert(regular.event_count==mellow.event_count);
        for(size_t i=0;i<regular.event_count;++i)
            assert(regular.events[i].ullAudioStreamOffset==mellow.events[i].ullAudioStreamOffset);
        m.mellow_mode=0;assert(SUCCEEDED(sapi_backend_speak(voice,&m,0,&wide,&phrase,&restored.iface)));
        same(&regular,&restored);
        free(regular.pcm);free(mellow.pcm);free(restored.pcm);sapi_backend_delete(voice);++cases;
    }
    /* The exposed question setting must reach the same native core on the
     * next SAPI request without changing ordinary declarative sentences. */
    for(unsigned i=0;i<2;++i){
        int dialect=i?0x40000:0x10000;
        struct sapi_backend *question_voice=sapi_backend_new(dialect);assert(question_voice);
        SPVTEXTFRAG question=text(i?L"Was möchtest du machen?":L"What would you like to do?",0);
        SPVTEXTFRAG statement=text(i?L"Das ist ein Satz.":L"This is a sentence.",0);
        struct site rising=site_new(),original=site_new(),plain=site_new(),plain_original=site_new();
        s.original_questions=0;assert(SUCCEEDED(sapi_backend_speak(question_voice,&s,0,&out,&question,&rising.iface)));
        assert(SUCCEEDED(sapi_backend_speak(question_voice,&s,0,&out,&statement,&plain.iface)));
        s.original_questions=1;assert(SUCCEEDED(sapi_backend_speak(question_voice,&s,0,&out,&question,&original.iface)));
        assert(SUCCEEDED(sapi_backend_speak(question_voice,&s,0,&out,&statement,&plain_original.iface)));
        assert(rising.count==original.count&&hash(rising.pcm,rising.count)!=hash(original.pcm,original.count));
        same(&plain,&plain_original);
        free(rising.pcm);free(original.pcm);free(plain.pcm);free(plain_original.pcm);
        sapi_backend_delete(question_voice);++cases;
    }
    s.original_questions=0;
    {
        struct sapi_backend *voice=sapi_backend_new(0x10000);assert(voice);
        struct sapi_settings timing=s;timing.speed_override=1;timing.speed_value=106;
        SPVTEXTFRAG phrase=text(L"adictive drums",0);
        struct site fine=site_new(),original=site_new(),restored=site_new(),automatic=site_new();
        timing.original_acoustic_clock=0;assert(SUCCEEDED(sapi_backend_speak(voice,&timing,0,&out,&phrase,&fine.iface)));
        timing.original_acoustic_clock=1;assert(SUCCEEDED(sapi_backend_speak(voice,&timing,0,&out,&phrase,&original.iface)));
        assert(fine.count==original.count&&hash(fine.pcm,fine.count)!=hash(original.pcm,original.count));
        timing.original_acoustic_clock=0;assert(SUCCEEDED(sapi_backend_speak(voice,&timing,0,&out,&phrase,&restored.iface)));
        same(&fine,&restored);
        timing.original_acoustic_clock=2;assert(SUCCEEDED(sapi_backend_speak(voice,&timing,0,&out,&phrase,&automatic.iface)));
        same(&fine,&automatic);
        free(fine.pcm);free(original.pcm);free(restored.pcm);free(automatic.pcm);sapi_backend_delete(voice);++cases;
    }
    for(int i=0;i<2;++i){
        int dialect=i?0x120000:0x20000;struct sapi_backend *voice=sapi_backend_new(dialect);assert(voice);
        struct sapi_settings timing=s;timing.speed_override=1;timing.speed_value=106;
        SPVTEXTFRAG phrase=text(i?L"Αυτή είναι μια δοκιμή.":L"Esta es una prueba.",0);
        struct site automatic=site_new(),original=site_new(),fine=site_new();
        timing.original_acoustic_clock=2;assert(SUCCEEDED(sapi_backend_speak(voice,&timing,0,&out,&phrase,&automatic.iface)));
        timing.original_acoustic_clock=1;assert(SUCCEEDED(sapi_backend_speak(voice,&timing,0,&out,&phrase,&original.iface)));
        timing.original_acoustic_clock=0;assert(SUCCEEDED(sapi_backend_speak(voice,&timing,0,&out,&phrase,&fine.iface)));
        same(&automatic,&original);assert(hash(fine.pcm,fine.count)!=hash(original.pcm,original.count));
        free(automatic.pcm);free(original.pcm);free(fine.pcm);sapi_backend_delete(voice);++cases;
    }
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
    printf("{\"backend_cases\":%u,\"format_negotiation_cases\":%u,\"host_tests_convert_audio\":false,\"languages\":%zu,\"rates\":7,\"reference_samples\":%zu,\"reference_hash\":\"%016llx\",\"profile_names\":8,\"last_profile\":\"%ls\","
        "\"pitch_hz\":[{\"samples\":%zu,\"hash\":\"%016llx\"},{\"samples\":%zu,\"hash\":\"%016llx\"},{\"samples\":%zu,\"hash\":\"%016llx\"}],"
        "\"long_german\":{\"marked_samples\":%zu,\"marked_hash\":\"%016llx\",\"plain_samples\":%zu,\"plain_hash\":\"%016llx\",\"events\":%zu,\"event_hash\":\"%016llx\"},\"comparisons\":[",
        cases,format_changes,sizeof sapi_languages/sizeof *sapi_languages,base.count,(unsigned long long)hash(base.pcm,base.count),sapi_profiles[7],
        hz_samples[0],(unsigned long long)hz_hashes[0],hz_samples[1],(unsigned long long)hz_hashes[1],
        hz_samples[2],(unsigned long long)hz_hashes[2],
        long_marked,(unsigned long long)long_marked_hash,long_plain,(unsigned long long)long_plain_hash,
        long_events,(unsigned long long)long_event_hash);
    for(unsigned i=0;i<comparison_count;++i){
        printf("%s{\"language\":%d,\"rate\":%d,\"samples\":%zu,\"hash\":\"%016llx\",\"event_offsets\":[",
            i?",":"",comparisons[i].language,comparisons[i].rate,comparisons[i].samples,(unsigned long long)comparisons[i].hash);
        for(unsigned j=0;j<comparisons[i].event_count;++j)printf("%s%u",j?",":"",(unsigned)comparisons[i].event_offsets[j]);
        fputs("],\"event_audio_offsets\":[",stdout);
        for(unsigned j=0;j<comparisons[i].event_count;++j)
            printf("%s%llu",j?",":"",(unsigned long long)comparisons[i].event_audio_offsets[j]);
        fputs("]}",stdout);
    }
    puts("]}");
    (void)sapi_anatomy;free(base.pcm);sapi_backend_delete(b);return 0;
}
