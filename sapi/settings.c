#include "settings.h"
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include "catalogue.inc"
size_t sapi_language_count(void){return sizeof sapi_languages/sizeof *sapi_languages;}
const WCHAR *sapi_profile_name(unsigned i){return i<8?sapi_profiles[i]:L"";}
int sapi_sample_rate(unsigned i){return i<7?sapi_rates[i]:0;}
const struct sapi_language *sapi_language_at(size_t i){return i<sapi_language_count()?sapi_languages+i:NULL;}
int sapi_language_index(int dialect)
{for(size_t i=0;i<sapi_language_count();++i)if(sapi_languages[i].dialect==dialect)return (int)i;return -1;}
void sapi_settings_profile(struct sapi_settings *s,unsigned profile)
{
    if(profile>=8)profile=0;
    s->profile=profile;s->gender=sapi_anatomy[profile][0];s->head=sapi_anatomy[profile][1];
    s->pitch=sapi_anatomy[profile][2];s->inflection=sapi_anatomy[profile][3];
    s->roughness=sapi_anatomy[profile][4];s->breathiness=sapi_anatomy[profile][5];
}
void sapi_settings_default(struct sapi_settings *s)
{static const DWORD frequencies[SAPI_EQ_FILTER_COUNT]={125,500,1200,3000,6000};
 memset(s,0,sizeof *s);s->schema=1;sapi_settings_profile(s,0);s->rate=1;s->abbreviations=1;s->pitch_hz=150;
 s->speed_value=60;s->volume_value=100;s->speed_wpm=180;s->eq_low=s->eq_mid=s->eq_high=12;
 for(int i=0;i<SAPI_EQ_FILTER_COUNT;++i){s->eq_filters[i].frequency=frequencies[i];
     s->eq_filters[i].gain=12;s->eq_filters[i].width=100;}}
int sapi_module_path(WCHAR *path,size_t capacity,const WCHAR *name)
{
    DWORD n=GetModuleFileNameW(sapi_module,path,(DWORD)capacity);
    if(!n||n>=capacity)return 0;
    WCHAR *p=wcsrchr(path,L'\\');if(!p)return 0;
    size_t left=capacity-(size_t)(p-path)-1;if(wcslen(name)>=left)return 0;
    wcscpy(p+1,name);
    if(!wcscmp(name,L"dictionaries")) {
        WIN32_FILE_ATTRIBUTE_DATA attributes;
        if(!GetFileAttributesExW(path,GetFileExInfoStandard,&attributes)||!(attributes.dwFileAttributes&16)) {
            /* The unified installer keeps one dictionary folder beside
             * x64 and above x86. Legacy adjacent folders retain priority. */
            *p=0;WCHAR *parent=wcsrchr(path,L'\\');
            if(parent&&wcslen(name)<capacity-(size_t)(parent-path)-1) {
                WCHAR saved[2048];if(wcslen(path)<2048){wcscpy(saved,path);wcscpy(parent+1,name);
                    if(GetFileAttributesExW(path,GetFileExInfoStandard,&attributes)&&(attributes.dwFileAttributes&16))return 1;
                    wcscpy(path,saved);}
            }
            wcscpy(p+1,name);
        }
    }
    return 1;
}
static void key_name(int dialect,WCHAR key[128])
{swprintf(key,128,L"Software\\ReLoq C\\SAPI5\\Voices\\%08X",(unsigned)dialect);}
static int valid(const struct sapi_settings *s)
{
    for(int i=0;i<SAPI_EQ_FILTER_COUNT;++i){const struct sapi_eq_filter *f=s->eq_filters+i;
        if(f->enabled>1||f->frequency<20||f->frequency>24000||f->gain>24||f->width<10||f->width>400)return 0;}
    return s->schema==1&&s->profile<8&&s->pitch<=100&&s->inflection<=100&&s->head<=100&&
        s->roughness<=100&&s->breathiness<=100&&s->gender<=1&&s->rate<7&&s->pauses<3&&
        s->tags<2&&s->abbreviations<2&&s->prediction<2&&s->dictionaries<2&&s->boost<2&&s->original_questions<2&&s->mellow_mode<2&&
        s->pitch_hz>=1&&s->pitch_hz<=24000&&s->pitch_hz_enabled<2&&
        s->speed_override<2&&s->speed_value<=250&&s->pitch_override<2&&
        s->volume_override<2&&s->volume_value<=100&&
        s->speed_wpm_enabled<2&&s->speed_wpm>=1&&s->speed_wpm<=6000&&
        s->eq_enabled<2&&s->eq_preset<5&&s->eq_low<=24&&s->eq_mid<=24&&s->eq_high<=24&&
        wmemchr(s->directory,0,sizeof s->directory/sizeof *s->directory)!=NULL;
}
static const WCHAR root[]=L"Software\\ReLoq C\\SAPI5";
static HRESULT read_binary(const WCHAR *key,const WCHAR *name,struct sapi_settings *out)
{
    struct sapi_settings input;sapi_settings_default(&input);DWORD size=sizeof input,type=0;
    LSTATUS rc=RegGetValueW(HKEY_CURRENT_USER,key,name,RRF_RT_REG_BINARY,&type,&input,&size);
    if(rc==ERROR_FILE_NOT_FOUND)return S_FALSE;
    if(rc!=ERROR_SUCCESS)return HRESULT_FROM_WIN32(rc);
    /* Older versions end before one of the appended extensions. Preserve
     * their settings and use current defaults for any missing fields. */
    if((size!=sizeof input&&size!=offsetof(struct sapi_settings,original_questions)&&
        size!=offsetof(struct sapi_settings,mellow_mode)&&size!=offsetof(struct sapi_settings,pitch_hz)&&
        size!=offsetof(struct sapi_settings,speed_override)&&
        size!=offsetof(struct sapi_settings,speed_wpm_enabled)&&
        size!=offsetof(struct sapi_settings,eq_enabled)&&
        size!=offsetof(struct sapi_settings,eq_filters))||!valid(&input))
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    *out=input;return S_OK;
}
static HRESULT write_value(const WCHAR *key,const WCHAR *name,DWORD type,const void *data,DWORD bytes)
{
    HKEY handle;
    LSTATUS rc=RegCreateKeyExW(HKEY_CURRENT_USER,key,0,NULL,0,KEY_SET_VALUE,NULL,&handle,NULL);
    if(rc==ERROR_SUCCESS){rc=RegSetValueExW(handle,name,0,type,data,bytes);RegCloseKey(handle);}
    return rc==ERROR_SUCCESS?S_OK:HRESULT_FROM_WIN32(rc);
}
HRESULT sapi_settings_shared(BOOL *enabled)
{
    if(!enabled)return E_POINTER;
    DWORD value=0,bytes=sizeof value;
    LSTATUS rc=RegGetValueW(HKEY_CURRENT_USER,root,L"SharedEnabled",RRF_RT_REG_DWORD,NULL,&value,&bytes);
    if(rc!=ERROR_SUCCESS&&rc!=ERROR_FILE_NOT_FOUND)return HRESULT_FROM_WIN32(rc);
    if(value>1)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    *enabled=value!=0;return S_OK;
}
static DWORD output_rate(void)
{
    DWORD rate=1,bytes=sizeof rate;
    if(RegGetValueW(HKEY_CURRENT_USER,root,L"OutputRate",RRF_RT_REG_DWORD,NULL,&rate,&bytes)==ERROR_SUCCESS&&rate<7)return rate;
    /* A fresh installation has no old per-language settings to migrate.
     * Avoid opening every missing language key for each speech request. */
    HKEY voices;LSTATUS rc=RegOpenKeyExW(HKEY_CURRENT_USER,L"Software\\ReLoq C\\SAPI5\\Voices",0,KEY_READ,&voices);
    if(rc==ERROR_FILE_NOT_FOUND||rc==ERROR_PATH_NOT_FOUND)return 1;
    if(rc==ERROR_SUCCESS)RegCloseKey(voices);
    /* Legacy settings migration: German, US English, then catalogue order. */
    for(size_t i=0;i<sapi_language_count()+2;++i){
        int dialect=i==0?0x40000:i==1?0x10000:sapi_language_at(i-2)->dialect;
        WCHAR key[128];key_name(dialect,key);struct sapi_settings old;
        if(read_binary(key,L"Settings",&old)==S_OK)return old.rate;
    }
    return 1;
}
HRESULT sapi_settings_read(int dialect,struct sapi_settings *settings)
{
    if(!settings||sapi_language_index(dialect)<0)return E_INVALIDARG;
    sapi_settings_default(settings);BOOL shared;HRESULT hr=sapi_settings_shared(&shared);if(FAILED(hr))return hr;
    WCHAR key[128];key_name(dialect,key);
    hr=read_binary(shared?root:key,shared?L"SharedSettings":L"Settings",settings);
    if(FAILED(hr))return hr;
    settings->rate=output_rate();return S_OK;
}
HRESULT sapi_settings_write(int dialect,const struct sapi_settings *settings)
{
    if(sapi_language_index(dialect)<0||!settings||!valid(settings))return E_INVALIDARG;
    BOOL shared;HRESULT hr=sapi_settings_shared(&shared);if(FAILED(hr))return hr;
    WCHAR key[128];key_name(dialect,key);
    hr=write_value(shared?root:key,shared?L"SharedSettings":L"Settings",REG_BINARY,settings,sizeof *settings);
    if(FAILED(hr))return hr;
    return write_value(root,L"OutputRate",REG_DWORD,&settings->rate,sizeof settings->rate);
}
HRESULT sapi_settings_set_shared(int dialect,BOOL enabled)
{
    if(enabled!=FALSE&&enabled!=TRUE)return E_INVALIDARG;
    struct sapi_settings current;HRESULT hr=sapi_settings_read(dialect,&current);if(FAILED(hr))return hr;
    if(enabled){hr=write_value(root,L"SharedSettings",REG_BINARY,&current,sizeof current);if(FAILED(hr))return hr;}
    DWORD value=enabled;
    return write_value(root,L"SharedEnabled",REG_DWORD,&value,sizeof value);
}
