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
{memset(s,0,sizeof *s);s->schema=1;sapi_settings_profile(s,0);s->rate=1;s->abbreviations=1;}
int sapi_module_path(WCHAR *path,size_t capacity,const WCHAR *name)
{
    DWORD n=GetModuleFileNameW(sapi_module,path,(DWORD)capacity);
    if(!n||n>=capacity)return 0;
    WCHAR *p=wcsrchr(path,L'\\');if(!p)return 0;
    size_t left=capacity-(size_t)(p-path)-1;if(wcslen(name)>=left)return 0;
    wcscpy(p+1,name);return 1;
}
static void key_name(int dialect,WCHAR key[128])
{swprintf(key,128,L"Software\\ReLoq C\\SAPI5\\Voices\\%08X",(unsigned)dialect);}
static int valid(const struct sapi_settings *s)
{
    return s->schema==1&&s->profile<8&&s->pitch<=100&&s->inflection<=100&&s->head<=100&&
        s->roughness<=100&&s->breathiness<=100&&s->gender<=1&&s->rate<7&&s->pauses<3&&
        s->tags<2&&s->abbreviations<2&&s->prediction<2&&s->dictionaries<2&&s->boost<2&&
        wmemchr(s->directory,0,sizeof s->directory/sizeof *s->directory)!=NULL;
}
HRESULT sapi_settings_read(int dialect,struct sapi_settings *settings)
{
    if(!settings||sapi_language_index(dialect)<0)return E_INVALIDARG;
    sapi_settings_default(settings);WCHAR key[128];key_name(dialect,key);
    struct sapi_settings input;DWORD size=sizeof input,type=0;
    LSTATUS rc=RegGetValueW(HKEY_CURRENT_USER,key,L"Settings",RRF_RT_REG_BINARY,&type,&input,&size);
    if(rc==ERROR_FILE_NOT_FOUND)return S_OK;
    if(rc!=ERROR_SUCCESS)return HRESULT_FROM_WIN32(rc);
    if(size!=sizeof input||!valid(&input))return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    *settings=input;return S_OK;
}
HRESULT sapi_settings_write(int dialect,const struct sapi_settings *settings)
{
    if(sapi_language_index(dialect)<0||!settings||!valid(settings))return E_INVALIDARG;
    WCHAR key[128];HKEY handle;key_name(dialect,key);
    LSTATUS rc=RegCreateKeyExW(HKEY_CURRENT_USER,key,0,NULL,0,KEY_SET_VALUE,NULL,&handle,NULL);
    if(rc==ERROR_SUCCESS){rc=RegSetValueExW(handle,L"Settings",0,REG_BINARY,(const BYTE *)settings,sizeof *settings);RegCloseKey(handle);}
    return rc==ERROR_SUCCESS?S_OK:HRESULT_FROM_WIN32(rc);
}
