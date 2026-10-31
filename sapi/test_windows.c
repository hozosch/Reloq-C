/* A real Windows/SAPI activation test. The stream lives in HGLOBAL memory;
 * the test never opens an audio device or creates an audio file. */
#include "settings.h"
#include <stdio.h>
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
int wmain(int argc,wchar_t **argv)
{
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 1;
    static const DWORD rates[]={8000,48000,11025,44100,16000,32000,22050,11025};
    const int all_rates=argc==2&&!wcscmp(argv[1],L"--all-native-rates");
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
    unsigned passed=0,expected=112*(all_rates?7:1);ISpVoice *voice=NULL;
    for(unsigned native=0;native<(all_rates?7u:1u);++native){
    if(all_rates&&RegSetValueExW(settings,L"OutputRate",0,REG_DWORD,(const BYTE *)&native,sizeof native)!=ERROR_SUCCESS){hr=E_FAIL;break;}
    hr=CoCreateInstance(&CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,&IID_ISpVoice,(void **)&voice);
    if(FAILED(hr))break;
    for(size_t i=0;i<sizeof sapi_languages/sizeof *sapi_languages;++i){
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
        for(unsigned r=0;r<sizeof rates/sizeof *rates;++r){
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
