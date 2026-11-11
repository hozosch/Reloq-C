#define _POSIX_C_SOURCE 200809L
#define main other_backend_tests
#include "test_backend.c"
#undef main
#include <unistd.h>

static struct site render(struct sapi_backend *b,struct sapi_settings *s,const WCHAR *value,ULONGLONG events)
{
    struct site site=site_new();site.interest=events;
    SPVTEXTFRAG f=text(value,13);WAVEFORMATEX out=format(11025);
    assert(SUCCEEDED(sapi_backend_speak(b,s,0,&out,&f,&site.iface)));assert(site.count>0);return site;
}
static void sound_matches(struct sapi_backend *b,struct sapi_settings *s,const WCHAR *actual,const WCHAR *expected,ULONGLONG events)
{
    struct site a=render(b,s,actual,events),e=render(b,s,expected,events);same(&a,&e);
    if(events){assert(a.event_count>=2);assert(a.events[1].lParam==13);assert(a.events[1].wParam==wcslen(actual));}
    free(a.pcm);free(e.pcm);
}
int main(int argc,char **argv)
{
    assert(argc==2);sapi_test_library=argv[1];char folder[]="/tmp/reloq-nvda-sapi-XXXXXX";assert(mkdtemp(folder));
    char marker[256],file[256],scoped[256],british[256],native[256];
    snprintf(marker,sizeof marker,"%s/nvda.ready",folder);
    snprintf(file,sizeof file,"%s/nvda-000-00010000-0.nvda",folder);
    snprintf(scoped,sizeof scoped,"%s/nvda-001-00010000-2.nvda",folder);
    snprintf(british,sizeof british,"%s/nvda-002-00010001-0.nvda",folder);
    snprintf(native,sizeof native,"%s/ENUmain.dic",folder);
    FILE *f=fopen(marker,"wb");assert(f&&!fclose(f));
    f=fopen(file,"wb");assert(f);assert(fputs("\xef\xbb\xbf# NVDA\nyoutube\tbanana\t0\t0\n",f)>=0&&!fclose(f));
    f=fopen(scoped,"wb");assert(f);assert(fputs("foo([0-9]+)\tbar\\1\t1\t1\n",f)>=0&&!fclose(f));
    f=fopen(british,"wb");assert(f);assert(fputs("fileswap\tbanana\t0\t0\n",f)>=0&&!fclose(f));
    f=fopen(native,"wb");assert(f);assert(fputs("nativetest\tpeach\n",f)>=0&&!fclose(f));
    struct sapi_settings s=settings();s.dictionaries=1;
    for(size_t i=0;i<=strlen(folder);++i)s.directory[i]=(unsigned char)folder[i];
    struct sapi_backend *b=sapi_backend_new(0x10000);assert(b);
    ULONGLONG events=((ULONGLONG)1<<SPEI_WORD_BOUNDARY)|((ULONGLONG)1<<SPEI_SENTENCE_BOUNDARY);
    for(unsigned seg=0;seg<2;++seg)for(unsigned camel=0;camel<2;++camel){
        s.word_segmentation=seg;s.camel_case=camel;
        sound_matches(b,&s,L"youtube",L"banana",0);
        sound_matches(b,&s,L"youtube",L"banana",events);
    }
    /* Ordinary SAPI text can be fragmented in the middle of a match. */
    SPVTEXTFRAG left=text(L"you",13),right=text(L"tube",16);left.pNext=&right;
    struct site split=site_new(),expected=render(b,&s,L"banana",events);WAVEFORMATEX out=format(11025);
    split.interest=events;assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&left,&split.iface)));
    same(&split,&expected);assert(split.events[1].lParam==13&&split.events[1].wParam==7);
    free(split.pcm);free(expected.pcm);
    s.profile=1;s.word_segmentation=0;s.camel_case=0;
    sound_matches(b,&s,L"foo42",L"bar42",events);
    sound_matches(b,&s,L"nativetest",L"peach",events);
    s.profile=0;struct site a=render(b,&s,L"foo42",0),e=render(b,&s,L"bar42",0);
    assert(a.count!=e.count||memcmp(a.pcm,e.pcm,a.count*sizeof *a.pcm));free(a.pcm);free(e.pcm);
    struct sapi_backend *gb=sapi_backend_new(0x10001);assert(gb);
    s.word_segmentation=1;s.camel_case=1;sound_matches(gb,&s,L"fileswap",L"banana",events);
    sapi_backend_delete(gb);sapi_backend_delete(b);
    assert(!unlink(marker)&&!unlink(file)&&!unlink(scoped)&&!unlink(british)&&!unlink(native)&&!rmdir(folder));
    puts("SAPI host: imported NVDA speech dictionaries, voice scope, segmentation and source events passed");
    return 0;
}
