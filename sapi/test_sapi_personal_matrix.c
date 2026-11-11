#define wmain unused_standard_sapi_main
#include "../sapi/test_windows.c"
#undef wmain

/* Only the test's settings functions use settings.c. */
HMODULE sapi_module;

static int put_files(HKEY root,const WCHAR *main_path,const WCHAR *root_path,int dialect)
{
    WCHAR entries[2300]={0};size_t at=0;
    int n=swprintf(entries+at,2300-at,L"%08X\t0\t%ls",(unsigned)dialect,main_path);
    if(n<0)return 0;at+=(size_t)n+1;
    n=swprintf(entries+at,2300-at,L"%08X\t1\t%ls",(unsigned)dialect,root_path);
    if(n<0)return 0;at+=(size_t)n+1;
    return RegSetValueExW(root,L"DictionaryFiles",0,REG_MULTI_SZ,(BYTE *)entries,(DWORD)((at+1)*sizeof(WCHAR)))==ERROR_SUCCESS;
}
static int make_file(const WCHAR *path,const char *content)
{
    FILE *f=_wfopen(path,L"wb");if(!f)return 0;
    size_t size=strlen(content);int ok=fwrite(content,1,size,f)==size;
    if(fclose(f))ok=0;return ok;
}
static HRESULT expect_same(ISpVoice *voice,IStream *memory,const WCHAR *test,const WCHAR *reference)
{
    ULONGLONG expected_bytes=0,actual_bytes=0;uint64_t expected=0,actual=0;
    HRESULT hr=speak_hash(voice,memory,reference,&expected_bytes,&expected);
    if(SUCCEEDED(hr))hr=speak_hash(voice,memory,test,&actual_bytes,&actual);
    if(SUCCEEDED(hr)&&(expected_bytes!=actual_bytes||expected!=actual))hr=E_FAIL;
    wprintf(L"  %ls compared with %ls: %08lX\n",test,reference,(unsigned long)hr);fflush(stdout);
    return hr;
}
int wmain(void)
{
    setvbuf(stdout,NULL,_IONBF,0);
    WCHAR temp[MAX_PATH],root[MAX_PATH],source[MAX_PATH],main_path[MAX_PATH],root_path[MAX_PATH],keyname[128];
    if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"rqt",0,root)||
       !DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 1;
    if(!SetEnvironmentVariableW(L"LOCALAPPDATA",root))return 2;
    swprintf(keyname,128,L"Software\\ReLoq-SapiMatrix-%lu",(unsigned long)GetCurrentProcessId());
    HKEY isolated=NULL,key=NULL;
    if(RegCreateKeyExW(HKEY_CURRENT_USER,keyname,0,NULL,0,KEY_ALL_ACCESS,NULL,&isolated,NULL)!=ERROR_SUCCESS)return 3;
    if(RegOverridePredefKey(HKEY_CURRENT_USER,isolated)!=ERROR_SUCCESS)return 4;
    if(RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\ReLoq C\\SAPI5",0,NULL,0,KEY_ALL_ACCESS,NULL,&key,NULL)!=ERROR_SUCCESS)return 5;
    swprintf(source,MAX_PATH,L"%ls\\Pers\u00f6nlich",root);if(!CreateDirectoryW(source,NULL))return 6;
    swprintf(main_path,MAX_PATH,L"%ls\\custom-main.dic",source);
    swprintf(root_path,MAX_PATH,L"%ls\\ENURoot.dic",source);
    if(!make_file(main_path,"mainsource\tbanana\r\n")||!make_file(root_path,
      "youtube\t`[.1tEst]\r\nhumanware\t`[.1tEst]\r\nsnapdragon\t`[.1tEst]\r\n"))return 7;
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);int ready=SUCCEEDED(hr);
    for(unsigned dialect_index=0;dialect_index<2&&SUCCEEDED(hr);++dialect_index){
        int dialect=dialect_index?0x10001:0x10000;
        if(!put_files(key,main_path,root_path,dialect)){hr=E_FAIL;break;}
        struct sapi_settings settings;sapi_settings_default(&settings);
        if(FAILED(sapi_settings_write(dialect,&settings))){hr=E_FAIL;break;}
        size_t index=0;while(index<sizeof sapi_languages/sizeof *sapi_languages&&sapi_languages[index].dialect!=dialect)++index;
        if(index==sizeof sapi_languages/sizeof *sapi_languages){hr=E_FAIL;break;}
        WCHAR id[512];swprintf(id,512,L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\ReLoqC_%02u_%08X",(unsigned)index,(unsigned)dialect);
        ISpVoice *voice=NULL;ISpObjectToken *token=NULL;ISpStream *audio=NULL;IStream *memory=NULL;
        hr=CoCreateInstance(&CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,&IID_ISpVoice,(void **)&voice);
        if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpObjectToken,NULL,CLSCTX_INPROC_SERVER,&IID_ISpObjectToken,(void **)&token);
        if(SUCCEEDED(hr))hr=ISpObjectToken_SetId(token,NULL,id,FALSE);
        if(SUCCEEDED(hr))hr=ISpVoice_SetVoice(voice,token);
        if(SUCCEEDED(hr))hr=CreateStreamOnHGlobal(NULL,TRUE,&memory);
        if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpStream,NULL,CLSCTX_INPROC_SERVER,&IID_ISpStream,(void **)&audio);
        WAVEFORMATEX format={WAVE_FORMAT_PCM,1,11025,22050,2,16,0};
        if(SUCCEEDED(hr))hr=ISpStream_SetBaseStream(audio,memory,&SPDFID_WaveFormatEx,&format);
        if(SUCCEEDED(hr))hr=ISpVoice_SetOutput(voice,(IUnknown *)audio,FALSE);
        for(unsigned n=0;n<3&&SUCCEEDED(hr);++n)for(unsigned mode=0;mode<4&&SUCCEEDED(hr);++mode){
            settings.dictionaries=n==1;settings.word_segmentation=mode&1;settings.camel_case=(mode>>1)&1;
            if(FAILED(sapi_settings_write(dialect,&settings))){hr=E_FAIL;break;}
            wprintf(L"English dialect %08X, community %ls, word %lu, camel %lu:\n",(unsigned)dialect,
                settings.dictionaries?L"on":L"off",(unsigned long)settings.word_segmentation,(unsigned long)settings.camel_case);
            const WCHAR *words[]={L"youtube",L"HumanWare",L"snapdragon"};
            for(unsigned i=0;i<3&&SUCCEEDED(hr);++i)hr=expect_same(voice,memory,words[i],L"test");
            if(SUCCEEDED(hr))hr=expect_same(voice,memory,L"mainsource",L"banana");
            if(SUCCEEDED(hr))hr=expect_same(voice,memory,L"before youtube after.",L"before test after.");
        }
        if(voice)ISpVoice_Release(voice);if(audio)ISpStream_Release(audio);
        if(memory)IStream_Release(memory);if(token)ISpObjectToken_Release(token);
    }
    if(ready)CoUninitialize();if(key)RegCloseKey(key);
    RegOverridePredefKey(HKEY_CURRENT_USER,NULL);RegCloseKey(isolated);RegDeleteTreeW(HKEY_CURRENT_USER,keyname);
    wprintf(L"SAPI matrix final HRESULT: %08lX\n",(unsigned long)hr);return FAILED(hr)?1:0;
}
