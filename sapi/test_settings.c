/* Exercise the production registry logic with a process-local registry.
 * No installed user settings or speech files are touched. */
#include "settings.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
HMODULE sapi_module;
struct key {WCHAR path[128];} keys[40];static size_t key_count;
struct value {HKEY key;WCHAR name[32];DWORD type,bytes;BYTE data[sizeof(struct sapi_settings)];} values[80];
static size_t value_count;static unsigned value_reads,key_opens;static LSTATUS open_error;
static HKEY find(LPCWSTR path)
{for(size_t i=0;i<key_count;++i)if(!wcscmp(path,keys[i].path))return keys+i;return NULL;}
LSTATUS RegGetValueW(HKEY base,LPCWSTR path,LPCWSTR name,DWORD flags,DWORD *type,void *data,DWORD *bytes)
{
    ++value_reads;assert(base==HKEY_CURRENT_USER);HKEY key=find(path);
    for(size_t i=0;i<value_count;++i)if(values[i].key==key&&!wcscmp(name,values[i].name)){
        struct value *v=values+i;
        if((v->type==REG_BINARY&&flags!=RRF_RT_REG_BINARY)||(v->type==REG_DWORD&&flags!=RRF_RT_REG_DWORD))return ERROR_INVALID_DATA;
        if(*bytes<v->bytes)return 234;memcpy(data,v->data,v->bytes);*bytes=v->bytes;if(type)*type=v->type;return 0;}
    return ERROR_FILE_NOT_FOUND;
}
LSTATUS RegOpenKeyExW(HKEY base,LPCWSTR path,DWORD options,DWORD access,HKEY *out)
{
    ++key_opens;assert(base==HKEY_CURRENT_USER&&!options&&access==KEY_READ);
    if(open_error)return open_error;
    size_t n=wcslen(path);
    for(size_t i=0;i<key_count;++i)
        if(!wcsncmp(path,keys[i].path,n)&&(keys[i].path[n]==0||keys[i].path[n]==L'\\')){
            *out=keys+i;return ERROR_SUCCESS;}
    return ERROR_FILE_NOT_FOUND;
}
LSTATUS RegCreateKeyExW(HKEY base,LPCWSTR path,DWORD a,void *b,DWORD c,DWORD access,void *d,HKEY *out,DWORD *disp)
{
    (void)a;(void)b;(void)c;(void)d;(void)disp;assert(base==HKEY_CURRENT_USER&&access==KEY_SET_VALUE);
    *out=find(path);if(!*out){assert(key_count<40&&wcslen(path)<128);wcscpy(keys[key_count].path,path);*out=keys+key_count++;}return 0;
}
LSTATUS RegSetValueExW(HKEY key,LPCWSTR name,DWORD reserved,DWORD type,const BYTE *data,DWORD bytes)
{
    (void)reserved;size_t i;for(i=0;i<value_count;++i)if(values[i].key==key&&!wcscmp(values[i].name,name))break;
    if(i==value_count){assert(i<80&&wcslen(name)<32);++value_count;values[i].key=key;wcscpy(values[i].name,name);}
    assert(bytes<=sizeof values[i].data);values[i].type=type;values[i].bytes=bytes;memcpy(values[i].data,data,bytes);return 0;
}
LSTATUS RegCloseKey(HKEY key){(void)key;return 0;}
static const WCHAR *module_path=L"";static int adjacent_dictionary;
DWORD GetModuleFileNameW(HMODULE m,WCHAR *p,DWORD cap)
{(void)m;size_t n=wcslen(module_path);if(!n||n>=cap)return 0;wcscpy(p,module_path);return (DWORD)n;}
BOOL GetFileAttributesExW(LPCWSTR path,int kind,WIN32_FILE_ATTRIBUTE_DATA *out)
{(void)kind;memset(out,0,sizeof *out);
 if(!wcscmp(path,L"C:\\ReLoq\\dictionaries")||(adjacent_dictionary&&!wcscmp(path,L"C:\\ReLoq\\x86\\dictionaries"))){out->dwFileAttributes=16;return TRUE;}return FALSE;}
static void same_parameters(const struct sapi_settings *a,const struct sapi_settings *b)
{assert(!memcmp(a,b,sizeof *a));}
int main(void)
{
    unsigned checks=0;struct sapi_settings de,en,s;BOOL shared;
    WCHAR path[2048];module_path=L"C:\\ReLoq\\x86\\ReLoqSapi.dll";
    assert(sapi_module_path(path,2048,L"eci.dll")&&!wcscmp(path,L"C:\\ReLoq\\x86\\eci.dll"));++checks;
    assert(sapi_module_path(path,2048,L"dictionaries")&&!wcscmp(path,L"C:\\ReLoq\\dictionaries"));++checks;
    adjacent_dictionary=1;
    assert(sapi_module_path(path,2048,L"dictionaries")&&!wcscmp(path,L"C:\\ReLoq\\x86\\dictionaries"));++checks;
    module_path=L"";
    assert(sapi_settings_read(0x40000,&de)==S_OK&&de.rate==1);
    assert(!de.speed_override&&de.speed_value==60&&!de.pitch_override&&
           !de.volume_override&&de.volume_value==100&&!de.speed_wpm_enabled&&de.speed_wpm==180);
    assert(!de.eq_enabled&&de.eq_preset==0&&de.eq_low==12&&de.eq_filters[0].frequency==125&&
           !de.eq_filters[0].enabled&&de.eq_filters[0].gain==12&&de.eq_filters[0].width==100);
    assert(value_reads==3&&key_opens==1);++checks;
    assert(sapi_settings_shared(&shared)==S_OK&&!shared);++checks;
    de.pitch=72;de.rate=6;de.pitch_hz=151;de.pitch_hz_enabled=1;
    de.speed_override=1;de.speed_value=87;de.pitch_override=1;de.volume_override=1;de.volume_value=42;
    de.speed_wpm_enabled=1;de.speed_wpm=210;
    de.eq_enabled=1;de.eq_preset=4;de.eq_low=16;de.eq_mid=10;de.eq_high=15;
    de.eq_filters[0]=(struct sapi_eq_filter){1,777,18,50};
    wcscpy(de.directory,L"C:\\My dictionaries");de.dictionaries=1;
    assert(sapi_settings_write(0x40000,&de)==S_OK);
    assert(sapi_settings_read(0x10000,&en)==S_OK&&en.rate==6&&en.pitch==65&&!en.dictionaries&&
           !en.speed_override&&!en.pitch_override&&!en.volume_override);++checks;
    en.pitch=43;en.rate=4;assert(sapi_settings_write(0x10000,&en)==S_OK);
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.rate==4&&s.pitch==72&&s.pitch_hz==151&&s.pitch_hz_enabled&&
           s.speed_override&&s.speed_value==87&&s.pitch_override&&s.volume_override&&s.volume_value==42&&
           s.speed_wpm_enabled&&s.speed_wpm==210&&s.eq_enabled&&s.eq_preset==4&&s.eq_low==16&&
           s.eq_filters[0].enabled&&s.eq_filters[0].frequency==777&&s.eq_filters[0].gain==18&&s.eq_filters[0].width==50);++checks;
    for(size_t i=0;i<sapi_language_count();++i){assert(sapi_settings_read(sapi_language_at(i)->dialect,&s)==S_OK&&s.rate==4);++checks;}
    assert(sapi_settings_set_shared(0x40000,TRUE)==S_OK);assert(sapi_settings_shared(&shared)==S_OK&&shared);++checks;
    de.rate=4;
    for(size_t i=0;i<sapi_language_count();++i){assert(sapi_settings_read(sapi_language_at(i)->dialect,&s)==S_OK);same_parameters(&de,&s);++checks;}
    s.head=22;s.gender=1;s.rate=2;assert(sapi_settings_write(0xa0000,&s)==S_OK);
    assert(sapi_settings_read(0x40000,&de)==S_OK&&de.head==22&&de.gender==1&&de.rate==2);++checks;
    assert(sapi_settings_set_shared(0x40000,FALSE)==S_OK);
    assert(sapi_settings_read(0x10000,&s)==S_OK&&s.pitch==43&&s.rate==2&&s.head==50&&!s.dictionaries);++checks;
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.pitch==72&&s.rate==2&&s.head==50&&s.dictionaries);++checks;
    assert(sapi_settings_set_shared(0x10000,TRUE)==S_OK);
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.pitch==43&&s.rate==2&&!s.dictionaries);++checks;
    assert(sapi_settings_write(0xdead,&s)==E_INVALIDARG);s.rate=9;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);++checks;
    /* Legacy settings migration works before any common OutputRate exists. */
    value_count=key_count=0;HKEY key;assert(!RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\ReLoq C\\SAPI5\\Voices\\00040000",0,NULL,0,KEY_SET_VALUE,NULL,&key,NULL));
    sapi_settings_default(&s);s.rate=5;s.pitch=78;wcscpy(s.directory,L"C:\\Existing dictionaries");
    assert(!RegSetValueExW(key,L"Settings",0,REG_BINARY,(const BYTE *)&s,offsetof(struct sapi_settings,original_questions)));
    assert(sapi_settings_read(0x10000,&en)==S_OK&&en.rate==5);++checks;
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.pitch==78&&!s.original_questions&&!s.mellow_mode&&
        !wcscmp(s.directory,L"C:\\Existing dictionaries"));++checks;
    s.original_questions=1;
    assert(!RegSetValueExW(key,L"Settings",0,REG_BINARY,(const BYTE *)&s,offsetof(struct sapi_settings,mellow_mode)));
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.original_questions==1&&!s.mellow_mode&&s.pitch==78);++checks;
    s.mellow_mode=1;assert(sapi_settings_write(0x40000,&s)==S_OK);
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.original_questions==1&&s.mellow_mode==1&&s.pitch==78);++checks;
    assert(!RegSetValueExW(key,L"Settings",0,REG_BINARY,(const BYTE *)&s,offsetof(struct sapi_settings,pitch_hz)));
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.pitch_hz==150&&!s.pitch_hz_enabled&&s.pitch==78);++checks;
    s.pitch_hz=151;s.pitch_hz_enabled=1;
    assert(!RegSetValueExW(key,L"Settings",0,REG_BINARY,(const BYTE *)&s,offsetof(struct sapi_settings,speed_override)));
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.pitch_hz==151&&s.pitch_hz_enabled&&
        !s.speed_override&&s.speed_value==60&&!s.pitch_override&&!s.volume_override&&s.volume_value==100&&
        !s.speed_wpm_enabled&&s.speed_wpm==180);++checks;
    s.speed_override=1;s.speed_value=95;
    assert(!RegSetValueExW(key,L"Settings",0,REG_BINARY,(const BYTE *)&s,offsetof(struct sapi_settings,speed_wpm_enabled)));
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.speed_override&&s.speed_value==95&&
        !s.speed_wpm_enabled&&s.speed_wpm==180);++checks;
    s.speed_wpm_enabled=1;s.speed_wpm=210;
    assert(!RegSetValueExW(key,L"Settings",0,REG_BINARY,(const BYTE *)&s,offsetof(struct sapi_settings,eq_enabled)));
    assert(sapi_settings_read(0x40000,&s)==S_OK&&s.speed_wpm_enabled&&s.speed_wpm==210&&
           !s.eq_enabled&&s.eq_filters[0].frequency==125&&s.eq_filters[0].gain==12);++checks;
    int gains[3];s.eq_enabled=1;s.eq_preset=1;sapi_equalizer_gains(&s,gains);
    assert(gains[0]==16&&gains[1]==12&&gains[2]==11);s.eq_enabled=0;sapi_equalizer_gains(&s,gains);
    assert(gains[0]==12&&gains[1]==12&&gains[2]==12);++checks;
    s.eq_enabled=2;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.eq_enabled=0;++checks;
    s.eq_filters[0].frequency=24001;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.eq_filters[0].frequency=125;++checks;
    s.eq_filters[0].width=9;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.eq_filters[0].width=100;++checks;
    s.speed_wpm_enabled=2;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.speed_wpm_enabled=0;++checks;
    s.speed_wpm=0;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.speed_wpm=180;++checks;
    s.speed_wpm=6001;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.speed_wpm=180;++checks;
    s.speed_value=251;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.speed_value=60;++checks;
    s.speed_override=2;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.speed_override=0;++checks;
    s.pitch_override=2;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.pitch_override=0;++checks;
    s.volume_override=2;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.volume_override=0;++checks;
    s.volume_value=101;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.volume_value=100;++checks;
    s.pitch_hz=24001;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.pitch_hz=150;++checks;
    s.pitch_hz_enabled=2;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.pitch_hz_enabled=0;++checks;
    s.mellow_mode=2;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);s.mellow_mode=1;++checks;
    s.original_questions=2;assert(sapi_settings_write(0x40000,&s)==E_INVALIDARG);++checks;
    open_error=5;assert(sapi_settings_read(0x10000,&en)==S_OK&&en.rate==5);open_error=0;++checks;
    values[0].bytes=8;assert(FAILED(sapi_settings_read(0x40000,&s)));++checks;
    printf("{\"settings_checks\":%u,\"languages\":%zu,\"shared_rate\":true,\"language_settings_preserved\":true}\n",checks,sapi_language_count());return 0;
}
