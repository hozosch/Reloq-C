#define _POSIX_C_SOURCE 200809L
#define main other_backend_tests
#include "test_backend.c"
#undef main
#include <unistd.h>
#include <sys/stat.h>
static void entry(const char *path,const char *pronunciation)
{FILE *f=fopen(path,"wb");assert(f);assert(fprintf(f,"computer\t%s\r\n",pronunciation)>0);assert(!fclose(f));}
static uint64_t speak_hash(struct sapi_backend *b,struct sapi_settings *s)
{
    SPVTEXTFRAG f=text(L"computer",0);struct site site=site_new();WAVEFORMATEX w=format(11025);
    assert(SUCCEEDED(sapi_backend_speak(b,s,0,&w,&f,&site.iface)));
    uint64_t result=hash(site.pcm,site.count);free(site.pcm);return result;
}
int main(int argc,char **argv)
{
    assert(argc==2);sapi_test_library=argv[1];struct sapi_backend *b=sapi_backend_new(0x10000);assert(b);
    struct sapi_settings s=settings();uint64_t plain=speak_hash(b,&s);
    assert(sapi_test_dictionary_calls==0&&speak_hash(b,&s)==plain&&sapi_test_dictionary_calls==0);
    char folder[]="/tmp/reloq-sapi-dictionary-XXXXXX";assert(mkdtemp(folder));
    char file[256];snprintf(file,sizeof file,"%s/ENUmain.dic",folder);entry(file,"hello");
    for(size_t i=0;i<=strlen(folder);++i)s.directory[i]=(unsigned char)folder[i];s.dictionaries=1;
    uint64_t first=speak_hash(b,&s);assert(first!=plain&&sapi_test_dictionary_calls==1);
    unsigned checks=sapi_test_attribute_calls;
    assert(speak_hash(b,&s)==first&&sapi_test_dictionary_calls==1);
    assert(sapi_test_attribute_calls==checks+1); /* only the directory */
    struct site cancelled=site_new();cancelled.abort_after=512;
    SPVTEXTFRAG long_text=text(L"computer computer computer computer",0);WAVEFORMATEX out=format(11025);
    assert(SUCCEEDED(sapi_backend_speak(b,&s,0,&out,&long_text,&cancelled.iface)));free(cancelled.pcm);
    assert(speak_hash(b,&s)==first&&sapi_test_dictionary_calls==1);
    /* Both file edits and editor-style replacement are observed without
     * resetting on unchanged utterances or unrelated parameter edits. */
    char replacement[256];snprintf(replacement,sizeof replacement,"%s/new.dic",folder);
    entry(replacement,"world");assert(!rename(replacement,file));
    uint64_t second=speak_hash(b,&s);assert(second!=first&&sapi_test_dictionary_calls==2);
    ++s.pitch;speak_hash(b,&s);assert(sapi_test_dictionary_calls==2);--s.pitch;
    assert(!unlink(file));assert(speak_hash(b,&s)==plain&&sapi_test_dictionary_calls==3);
    entry(file,"hello");assert(speak_hash(b,&s)==first&&sapi_test_dictionary_calls==4);
    entry(file,"world");assert(speak_hash(b,&s)==second&&sapi_test_dictionary_calls==5);
    assert(!unlink(file)&&!rmdir(folder));assert(!mkdir(folder,0700));entry(file,"hello");
    assert(speak_hash(b,&s)==first&&sapi_test_dictionary_calls==6);
    s.dictionaries=0;assert(speak_hash(b,&s)==plain&&sapi_test_dictionary_calls==7);
    assert(speak_hash(b,&s)==plain&&sapi_test_dictionary_calls==7);
    sapi_backend_delete(b);assert(!unlink(file)&&!rmdir(folder));
    puts("{\"reloads\":7,\"unchanged_language_retained\":true,\"one_unchanged_directory_check\":true,\"file_edit_create_delete\":true,\"directory_recreation\":true,\"same_size_replacement\":true,\"abort_reuse\":true,\"no_audio_files\":true}");return 0;
}
