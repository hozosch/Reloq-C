/* A real Windows/SAPI activation test. The stream lives in HGLOBAL memory;
 * the test never opens an audio device or creates an audio file. */
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include "catalogue.inc"
static HRESULT native_format(ISpObjectToken *token,DWORD target,DWORD *rate)
{
    ISpTTSEngine *engine=NULL;ISpObjectWithToken *bound=NULL;WAVEFORMATEX *output=NULL;GUID id={0};
    HRESULT hr=CoCreateInstance(&CLSID_ReLoqEngine,NULL,CLSCTX_INPROC_SERVER,&IID_ISpTTSEngine,(void **)&engine);
    if(SUCCEEDED(hr))hr=engine->lpVtbl->QueryInterface(engine,&IID_ISpObjectWithToken,(void **)&bound);
    if(SUCCEEDED(hr))hr=ISpObjectWithToken_SetObjectToken(bound,token);
    WAVEFORMATEX requested={WAVE_FORMAT_PCM,1,target,target*2,2,16,0};
    if(SUCCEEDED(hr))hr=engine->lpVtbl->GetOutputFormat(engine,target?&SPDFID_WaveFormatEx:NULL,
        target?&requested:NULL,&id,&output);
    if(SUCCEEDED(hr)&&(!output||!IsEqualGUID(&id,&SPDFID_WaveFormatEx)||output->nChannels!=1||
        output->wBitsPerSample!=16||output->nBlockAlign!=2||output->nAvgBytesPerSec!=output->nSamplesPerSec*2))hr=E_FAIL;
    if(SUCCEEDED(hr))*rate=output->nSamplesPerSec;
    CoTaskMemFree(output);if(bound)ISpObjectWithToken_Release(bound);if(engine)engine->lpVtbl->Release(engine);return hr;
}
static int run_one(int argc,wchar_t **argv)
{
    /* Preserve the last completed case if a native adapter terminates. */
    setvbuf(stdout,NULL,_IONBF,0);
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 1;
    static const DWORD rates[]={8000,48000,11025,44100,16000,32000,22050,11025};
    const int first_case=argc==2&&!wcscmp(argv[1],L"--first-case");
    const int all_rates=first_case||(argc==2&&!wcscmp(argv[1],L"--all-native-rates"));
    HKEY settings=NULL;DWORD saved=0,saved_bytes=sizeof saved,saved_type=0;
    LSTATUS had_saved=ERROR_FILE_NOT_FOUND;
    if(all_rates){
        LSTATUS rc=RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\ReLoq C\\SAPI5",0,NULL,0,
            KEY_QUERY_VALUE|KEY_SET_VALUE,NULL,&settings,NULL);
        if(rc!=ERROR_SUCCESS){CoUninitialize();return 1;}
        had_saved=RegQueryValueExW(settings,L"OutputRate",NULL,&saved_type,(BYTE *)&saved,&saved_bytes);
        if(had_saved!=ERROR_FILE_NOT_FOUND&&(had_saved!=ERROR_SUCCESS||saved_type!=REG_DWORD||saved_bytes!=sizeof saved)){
            RegCloseKey(settings);CoUninitialize();return 1;}
    }
    unsigned passed=0,expected=first_case?1:112*(all_rates?7:1);ISpVoice *voice=NULL;
    for(unsigned native=0;native<(first_case?1u:all_rates?7u:1u);++native){
    if(all_rates&&RegSetValueExW(settings,L"OutputRate",0,REG_DWORD,(const BYTE *)&native,sizeof native)!=ERROR_SUCCESS){hr=E_FAIL;break;}
    hr=CoCreateInstance(&CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,&IID_ISpVoice,(void **)&voice);
    if(FAILED(hr))break;
    for(size_t i=0;i<(first_case?1:sizeof sapi_languages/sizeof *sapi_languages);++i){
        ISpObjectToken *token=NULL;
        WCHAR key[512];swprintf(key,512,L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\ReLoqC_%02u_%08X",
            (unsigned)i,(unsigned)sapi_languages[i].dialect);
        hr=CoCreateInstance(&CLSID_SpObjectToken,NULL,CLSCTX_INPROC_SERVER,&IID_ISpObjectToken,(void **)&token);
        if(SUCCEEDED(hr))hr=ISpObjectToken_SetId(token,NULL,key,FALSE);
        if(SUCCEEDED(hr))hr=ISpVoice_SetVoice(voice,token);
        DWORD selected=0;if(SUCCEEDED(hr))hr=native_format(token,0,&selected);
        if(all_rates&&SUCCEEDED(hr)&&selected!=(DWORD)sapi_rates[native])hr=E_FAIL;
        if(FAILED(hr)){if(token)ISpObjectToken_Release(token);break;}
        ULONGLONG reference_us=0;
        for(unsigned r=0;r<(first_case?1:sizeof rates/sizeof *rates);++r){
        fwprintf(stdout,L"Checking %ls, native=%lu Hz, output=%lu Hz\n",sapi_languages[i].name,
            (unsigned long)selected,(unsigned long)rates[r]);
        DWORD offered=0;hr=native_format(token,rates[r],&offered);
        if(SUCCEEDED(hr)&&offered!=selected)hr=E_FAIL;
        ISpStream *speech=NULL;IStream *memory=NULL;
        if(SUCCEEDED(hr))hr=CreateStreamOnHGlobal(NULL,TRUE,&memory);
        if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpStream,NULL,CLSCTX_INPROC_SERVER,&IID_ISpStream,(void **)&speech);
        WAVEFORMATEX format={WAVE_FORMAT_PCM,1,rates[r],rates[r]*2,2,16,0};
        if(SUCCEEDED(hr))hr=ISpStream_SetBaseStream(speech,memory,&SPDFID_WaveFormatEx,&format);
        /* FALSE pins the application's clock, even when the saved voice
         * preference differs. TRUE could hide a negotiation regression. */
        if(SUCCEEDED(hr))hr=ISpVoice_SetOutput(voice,(IUnknown *)speech,FALSE);
        if(SUCCEEDED(hr))hr=ISpVoice_Speak(voice,sapi_languages[i].sample,SPF_IS_NOT_XML,NULL);
        STATSTG stat={0};if(SUCCEEDED(hr))hr=IStream_Stat(memory,&stat,STATFLAG_NONAME);
        if(SUCCEEDED(hr)&&(!stat.cbSize.QuadPart||(stat.cbSize.QuadPart&1)))hr=E_FAIL;
        ULONGLONG duration_us=stat.cbSize.QuadPart*1000000/(rates[r]*2);
        if(SUCCEEDED(hr)&&r){ULONGLONG difference=duration_us>reference_us?duration_us-reference_us:reference_us-duration_us;
            if(difference>50000)hr=E_FAIL;}
        if(!r)reference_us=duration_us;
        GUID id={0};WAVEFORMATEX *negotiated=NULL;
        if(SUCCEEDED(hr))hr=ISpStream_GetFormat(speech,&id,&negotiated);
        if(SUCCEEDED(hr)&&(!IsEqualGUID(&id,&SPDFID_WaveFormatEx)||!negotiated||
            negotiated->nSamplesPerSec!=rates[r]||negotiated->nChannels!=1||negotiated->wBitsPerSample!=16))hr=E_FAIL;
        CoTaskMemFree(negotiated);
        wprintf(L"%ls, native=%lu Hz, SAPI=%lu Hz: HRESULT=%08lX, PCM bytes=%llu\n",sapi_languages[i].name,(unsigned long)offered,(unsigned long)rates[r],(unsigned long)hr,(unsigned long long)stat.cbSize.QuadPart);
        if(speech)ISpStream_Release(speech);if(memory)IStream_Release(memory);
        if(FAILED(hr))break;++passed;
        }
        if(token)ISpObjectToken_Release(token);if(FAILED(hr))break;
    }
    if(voice){ISpVoice_Release(voice);voice=NULL;}if(FAILED(hr))break;
    }
    if(settings){
        if(had_saved==ERROR_SUCCESS)RegSetValueExW(settings,L"OutputRate",0,REG_DWORD,(const BYTE *)&saved,sizeof saved);
        else RegDeleteValueW(settings,L"OutputRate");
        RegCloseKey(settings);
    }
    CoUninitialize();
    wprintf(L"SAPI activation: %u/%u language/format cases passed (HRESULT=%08lX).\n",passed,expected,(unsigned long)hr);
    (void)sapi_rates;(void)sapi_profiles;(void)sapi_anatomy;return passed==expected?0:1;
}

static DWORD WINAPI run_thread(void *unused)
{
    (void)unused;
    wchar_t *args[]={L"Test-SAPI",L"--first-case"};
    return (DWORD)run_one(2,args);
}

static HRESULT clear_memory(IStream *stream)
{
    LARGE_INTEGER zero={0};ULARGE_INTEGER empty={0};
    HRESULT hr=IStream_Seek(stream,zero,STREAM_SEEK_SET,NULL);
    return SUCCEEDED(hr)?IStream_SetSize(stream,empty):hr;
}

static HRESULT memory_hash(IStream *stream,ULONGLONG *bytes,uint64_t *hash)
{
    STATSTG stat={0};HGLOBAL storage=NULL;
    HRESULT hr=IStream_Stat(stream,&stat,STATFLAG_NONAME);
    if(SUCCEEDED(hr))hr=GetHGlobalFromStream(stream,&storage);
    if(FAILED(hr)||!stat.cbSize.QuadPart||(ULONGLONG)(size_t)stat.cbSize.QuadPart!=stat.cbSize.QuadPart)return E_FAIL;
    const unsigned char *data=GlobalLock(storage);if(!data)return E_FAIL;
    uint64_t value=1469598103934665603ULL;
    for(size_t i=0;i<(size_t)stat.cbSize.QuadPart;++i)value=(value^data[i])*1099511628211ULL;
    GlobalUnlock(storage);*bytes=stat.cbSize.QuadPart;*hash=value;return S_OK;
}

static HRESULT speak_hash(ISpVoice *voice,IStream *memory,const WCHAR *text,ULONGLONG *bytes,uint64_t *hash)
{
    HRESULT hr=clear_memory(memory);
    if(SUCCEEDED(hr))hr=ISpVoice_Speak(voice,text,SPF_IS_NOT_XML,NULL);
    if(SUCCEEDED(hr))hr=memory_hash(memory,bytes,hash);
    return hr;
}

/* Exercise the registered SAPI engine, its real per-user dictionary snapshot,
 * and ECI together. All registry and files live under disposable test roots. */
static int run_personal_dictionary_case(void)
{
    WCHAR temp[MAX_PATH],root[MAX_PATH],source[MAX_PATH],keyname[128],old_appdata[2048];
    if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"rqs",0,root)||
       !DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 1;
    DWORD old=GetEnvironmentVariableW(L"LOCALAPPDATA",old_appdata,2048);
    if(!old||old>=2048||!SetEnvironmentVariableW(L"LOCALAPPDATA",root))return 2;
    swprintf(keyname,128,L"Software\\ReLoq-SapiIntegrationTest-%lu",(unsigned long)GetCurrentProcessId());
    HKEY isolated=NULL,key=NULL;
    if(RegCreateKeyExW(HKEY_CURRENT_USER,keyname,0,NULL,0,KEY_ALL_ACCESS,NULL,&isolated,NULL)!=ERROR_SUCCESS)return 3;
    if(RegOverridePredefKey(HKEY_CURRENT_USER,isolated)!=ERROR_SUCCESS)return 4;
    swprintf(source,MAX_PATH,L"%ls\\personal.dic",root);
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    int com_ready=SUCCEEDED(hr);
    ISpVoice *voice=NULL;ISpObjectToken *token=NULL;ISpStream *audio=NULL;IStream *memory=NULL;
    WCHAR token_key[512];size_t english=0;
    const size_t languages=sizeof sapi_languages/sizeof *sapi_languages;
    while(english<languages&&sapi_languages[english].dialect!=0x10000)++english;
    if(english==languages)hr=E_FAIL;
    swprintf(token_key,512,L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\ReLoqC_%02u_%08X",
        (unsigned)english,0x10000u);
    if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,&IID_ISpVoice,(void **)&voice);
    if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpObjectToken,NULL,CLSCTX_INPROC_SERVER,&IID_ISpObjectToken,(void **)&token);
    if(SUCCEEDED(hr))hr=ISpObjectToken_SetId(token,NULL,token_key,FALSE);
    if(SUCCEEDED(hr))hr=ISpVoice_SetVoice(voice,token);
    if(SUCCEEDED(hr))hr=CreateStreamOnHGlobal(NULL,TRUE,&memory);
    if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpStream,NULL,CLSCTX_INPROC_SERVER,&IID_ISpStream,(void **)&audio);
    WAVEFORMATEX format={WAVE_FORMAT_PCM,1,11025,22050,2,16,0};
    if(SUCCEEDED(hr))hr=ISpStream_SetBaseStream(audio,memory,&SPDFID_WaveFormatEx,&format);
    if(SUCCEEDED(hr))hr=ISpVoice_SetOutput(voice,(IUnknown *)audio,FALSE);
    ULONGLONG before_bytes=0,expected_bytes=0,actual_bytes=0;uint64_t before=0,expected=0,actual=0;
    if(SUCCEEDED(hr))hr=speak_hash(voice,memory,L"youtube",&before_bytes,&before);
    if(SUCCEEDED(hr))hr=speak_hash(voice,memory,L"banana",&expected_bytes,&expected);
    if(SUCCEEDED(hr)&&before_bytes==expected_bytes&&before==expected)hr=E_FAIL;
    FILE *file=NULL;
    if(SUCCEEDED(hr)){file=_wfopen(source,L"wb");if(!file)hr=E_FAIL;}
    if(file){const char *data="youtube\tbanana\nHumanWare\tbanana\n";size_t n=strlen(data);
        if(fwrite(data,1,n,file)!=n||fclose(file)!=0)hr=E_FAIL;}
    if(SUCCEEDED(hr)&&RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\ReLoq C\\SAPI5",0,NULL,0,KEY_SET_VALUE,NULL,&key,NULL)!=ERROR_SUCCESS)hr=E_FAIL;
    WCHAR list[1200]={0};
    if(SUCCEEDED(hr)){
        int chars=swprintf(list,1200,L"%08X\t0\t%ls",0x10000u,source);
        if(chars<0||RegSetValueExW(key,L"DictionaryFiles",0,REG_MULTI_SZ,(BYTE *)list,(DWORD)((chars+2)*sizeof(WCHAR)))!=ERROR_SUCCESS)hr=E_FAIL;
    }
    const WCHAR *words[]={L"youtube",L"HumanWare"};
    for(unsigned i=0;i<2&&SUCCEEDED(hr);++i){
        hr=speak_hash(voice,memory,words[i],&actual_bytes,&actual);
        wprintf(L"Personal dictionary %ls: HRESULT=%08lX, matched=%d\n",words[i],(unsigned long)hr,
            actual_bytes==expected_bytes&&actual==expected);
        if(SUCCEEDED(hr)&&(actual_bytes!=expected_bytes||actual!=expected))hr=E_FAIL;
    }
    if(SUCCEEDED(hr)){
        list[0]=list[1]=0;
        if(RegSetValueExW(key,L"DictionaryFiles",0,REG_MULTI_SZ,(BYTE *)list,2*sizeof(WCHAR))!=ERROR_SUCCESS)hr=E_FAIL;
    }
    if(SUCCEEDED(hr))hr=speak_hash(voice,memory,L"youtube",&actual_bytes,&actual);
    if(SUCCEEDED(hr)&&(actual_bytes!=before_bytes||actual!=before))hr=E_FAIL;
    if(key)RegCloseKey(key);if(voice)ISpVoice_Release(voice);if(audio)ISpStream_Release(audio);
    if(memory)IStream_Release(memory);if(token)ISpObjectToken_Release(token);if(com_ready)CoUninitialize();
    RegOverridePredefKey(HKEY_CURRENT_USER,NULL);RegCloseKey(isolated);RegDeleteTreeW(HKEY_CURRENT_USER,keyname);
    SetEnvironmentVariableW(L"LOCALAPPDATA",old_appdata);DeleteFileW(source);
    wprintf(L"SAPI personal dictionary integration: HRESULT=%08lX\n",(unsigned long)hr);
    return FAILED(hr)?1:0;
}

static int run_abort_case(void)
{
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 1;
    ISpVoice *voice=NULL;ISpObjectToken *token=NULL;ISpStream *audio=NULL;IStream *memory=NULL;
    WCHAR key[512];size_t german=0;
    const size_t languages=sizeof sapi_languages/sizeof *sapi_languages;
    while(german<languages&&sapi_languages[german].dialect!=0x40000)++german;
    if(german==languages){hr=E_FAIL;goto done;}
    swprintf(key,512,L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\ReLoqC_%02u_%08X",
        (unsigned)german,0x40000u);
    hr=CoCreateInstance(&CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,&IID_ISpVoice,(void **)&voice);
    if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpObjectToken,NULL,CLSCTX_INPROC_SERVER,&IID_ISpObjectToken,(void **)&token);
    if(SUCCEEDED(hr))hr=ISpObjectToken_SetId(token,NULL,key,FALSE);
    if(SUCCEEDED(hr))hr=ISpVoice_SetVoice(voice,token);
    if(SUCCEEDED(hr))hr=CreateStreamOnHGlobal(NULL,TRUE,&memory);
    if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpStream,NULL,CLSCTX_INPROC_SERVER,&IID_ISpStream,(void **)&audio);
    WAVEFORMATEX format={WAVE_FORMAT_PCM,1,11025,22050,2,16,0};
    if(SUCCEEDED(hr))hr=ISpStream_SetBaseStream(audio,memory,&SPDFID_WaveFormatEx,&format);
    if(SUCCEEDED(hr))hr=ISpVoice_SetOutput(voice,(IUnknown *)audio,FALSE);
    const WCHAR *short_text=L"Dies ist eine kurze Ansage.";
    if(SUCCEEDED(hr))hr=ISpVoice_Speak(voice,short_text,SPF_IS_NOT_XML,NULL);
    ULONGLONG bytes=0;uint64_t expected=0;
    if(SUCCEEDED(hr))hr=memory_hash(memory,&bytes,&expected);
    WCHAR long_text[8192]=L"";
    for(unsigned i=0;i<180;++i)wcscat(long_text,L"Eine längere Ansage zum Unterbrechen. ");
    for(unsigned i=0;i<12&&SUCCEEDED(hr);++i){
        hr=clear_memory(memory);
        if(SUCCEEDED(hr))hr=ISpVoice_Speak(voice,long_text,SPF_ASYNC|SPF_IS_NOT_XML,NULL);
        /* Wait until the asynchronous utterance has actually produced PCM;
         * this exercises interruption during synthesis, rather than merely
         * removing a still-pending utterance from SAPI's queue. */
        STATSTG progress={0};
        for(unsigned wait=0;SUCCEEDED(hr)&&!progress.cbSize.QuadPart&&wait<400;++wait){
            Sleep(5);hr=IStream_Stat(memory,&progress,STATFLAG_NONAME);
        }
        if(SUCCEEDED(hr)&&!progress.cbSize.QuadPart)hr=E_FAIL;
        if(SUCCEEDED(hr))hr=ISpVoice_Speak(voice,NULL,SPF_PURGEBEFORESPEAK,NULL);
        if(SUCCEEDED(hr))hr=clear_memory(memory);
        if(SUCCEEDED(hr))hr=ISpVoice_Speak(voice,short_text,SPF_IS_NOT_XML,NULL);
        ULONGLONG got_bytes=0;uint64_t got=0;
        if(SUCCEEDED(hr))hr=memory_hash(memory,&got_bytes,&got);
        if(SUCCEEDED(hr)&&(got_bytes!=bytes||got!=expected))hr=E_FAIL;
        wprintf(L"SAPI abort/recovery %u: HRESULT=%08lX\n",i,(unsigned long)hr);
    }
done:
    if(voice)ISpVoice_Release(voice);if(audio)ISpStream_Release(audio);
    if(memory)IStream_Release(memory);if(token)ISpObjectToken_Release(token);
    CoUninitialize();return FAILED(hr)?1:0;
}

int wmain(int argc,wchar_t **argv)
{
    if(argc==2&&!wcscmp(argv[1],L"--dictionarycase"))return run_personal_dictionary_case();
    if(argc==2&&!wcscmp(argv[1],L"--abortcase"))return run_abort_case();
    if(argc!=2||wcscmp(argv[1],L"--threadcase"))return run_one(argc,argv);
    /* A fresh caller must be able to end and free its cached contexts, even
     * when SAPI creates and releases its voices on many different threads. */
    for(unsigned i=0;i<12;++i){
        HANDLE thread=CreateThread(NULL,0,run_thread,NULL,0,NULL);
        if(!thread)return 2;
        DWORD wait=WaitForSingleObject(thread,20000),result=1;
        if(wait==WAIT_OBJECT_0)GetExitCodeThread(thread,&result);
        CloseHandle(thread);
        wprintf(L"Caller thread %u: wait=%lu result=%lu\n",i,(unsigned long)wait,(unsigned long)result);
        if(wait!=WAIT_OBJECT_0||result)return 3;
    }
    return 0;
}
