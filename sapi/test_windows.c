/* A real Windows/SAPI activation test. The stream lives in HGLOBAL memory;
 * the test never opens an audio device or creates an audio file. */
#include "settings.h"
#include <stdio.h>
#include <wchar.h>
#include "catalogue.inc"
int wmain(void)
{
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 1;
    unsigned passed=0;ISpVoice *voice=NULL;
    hr=CoCreateInstance(&CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,&IID_ISpVoice,(void **)&voice);
    if(FAILED(hr))goto done;
    for(size_t i=0;i<sizeof sapi_languages/sizeof *sapi_languages;++i){
        ISpObjectToken *token=NULL;ISpStream *speech=NULL;IStream *memory=NULL;
        WCHAR key[512];swprintf(key,512,L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\ReLoqC_%02u_%08X",
            (unsigned)i,(unsigned)sapi_languages[i].dialect);
        hr=CoCreateInstance(&CLSID_SpObjectToken,NULL,CLSCTX_INPROC_SERVER,&IID_ISpObjectToken,(void **)&token);
        if(SUCCEEDED(hr))hr=ISpObjectToken_SetId(token,NULL,key,FALSE);
        if(SUCCEEDED(hr))hr=ISpVoice_SetVoice(voice,token);
        if(SUCCEEDED(hr))hr=CreateStreamOnHGlobal(NULL,TRUE,&memory);
        if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpStream,NULL,CLSCTX_INPROC_SERVER,&IID_ISpStream,(void **)&speech);
        WAVEFORMATEX format={WAVE_FORMAT_PCM,1,11025,22050,2,16,0};
        if(SUCCEEDED(hr))hr=ISpStream_SetBaseStream(speech,memory,&SPDFID_WaveFormatEx,&format);
        if(SUCCEEDED(hr))hr=ISpVoice_SetOutput(voice,(IUnknown *)speech,TRUE);
        if(SUCCEEDED(hr))hr=ISpVoice_Speak(voice,sapi_languages[i].sample,SPF_IS_NOT_XML,NULL);
        STATSTG stat={0};if(SUCCEEDED(hr))hr=IStream_Stat(memory,&stat,STATFLAG_NONAME);
        if(SUCCEEDED(hr)&&!stat.cbSize.QuadPart)hr=E_FAIL;
        wprintf(L"%ls: HRESULT=%08lX, PCM bytes=%llu\n",sapi_languages[i].name,(unsigned long)hr,(unsigned long long)stat.cbSize.QuadPart);
        if(speech)ISpStream_Release(speech);if(memory)IStream_Release(memory);if(token)ISpObjectToken_Release(token);
        if(FAILED(hr))break;++passed;
    }
done:
    if(voice)ISpVoice_Release(voice);CoUninitialize();
    wprintf(L"SAPI activation: %u/14 languages passed (HRESULT=%08lX).\n",passed,(unsigned long)hr);
    (void)sapi_rates;(void)sapi_profiles;(void)sapi_anatomy;return passed==14?0:1;
}
