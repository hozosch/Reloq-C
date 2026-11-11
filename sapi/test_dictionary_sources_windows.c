/* Exercise personal files, precedence and per-user snapshots on Windows. */
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

HMODULE sapi_module;
static WCHAR installed[1024];
int sapi_module_path(WCHAR *path,size_t capacity,const WCHAR *name)
{(void)name;if(wcslen(installed)>=capacity)return 0;wcscpy(path,installed);return 1;}
int sapi_language_index(int dialect){return dialect==0x10000?0:dialect==0x40000?1:dialect==0x60000?2:dialect==0x60001?3:-1;}
static int write_file(const WCHAR *path,const char *data)
{FILE *f=_wfopen(path,L"wb");if(!f)return 0;size_t n=strlen(data);int ok=fwrite(data,1,n,f)==n;return fclose(f)==0&&ok;}
static int equals(const WCHAR *path,const char *expected)
{char data[512]={0};FILE *f=_wfopen(path,L"rb");if(!f)return 0;size_t n=fread(data,1,sizeof data,f);int ok=!ferror(f);fclose(f);return ok&&n==strlen(expected)&&!memcmp(data,expected,n);}
int main(void)
{
    WCHAR temp[MAX_PATH],root[MAX_PATH],keyname[128],old_appdata[2048],path[2048],file[2048],first[2048];
    if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"rqd",0,root)||!DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 1;
    DWORD old=GetEnvironmentVariableW(L"LOCALAPPDATA",old_appdata,2048);
    if(!old||old>=2048||!SetEnvironmentVariableW(L"LOCALAPPDATA",root))return 2;
    swprintf(keyname,128,L"Software\\ReLoq-CDictionaryTest-%lu",(unsigned long)GetCurrentProcessId());
    HKEY isolated;if(RegCreateKeyExW(HKEY_CURRENT_USER,keyname,0,NULL,0,KEY_ALL_ACCESS,NULL,&isolated,NULL)!=ERROR_SUCCESS)return 3;
    if(RegOverridePredefKey(HKEY_CURRENT_USER,isolated)!=ERROR_SUCCESS)return 4;
    swprintf(installed,1024,L"%ls\\installed",root);if(!CreateDirectoryW(installed,NULL))return 5;
    const WCHAR *names[]={L"DEUmain.dic",L"DEURoot.dic",L"DEUabbr.dic",L"ENUmain.dic",L"ENURoot.dic",L"ENUabbr.dic"};
    for(unsigned i=0;i<6;++i){swprintf(file,2048,L"%ls\\%ls",installed,names[i]);
        if(!write_file(file,i==3?"alpha\tcommunity\nshared\tcommunity\n":"other\tvalue\n"))return 6;}
    if(!sapi_default_dictionary_path(path,2048)||!wcsstr(path,L"\\Community\\Bundled")||!wcscmp(path,installed))return 7;
    swprintf(file,2048,L"%ls\\ENUmain.dic",path);if(!equals(file,"alpha\tcommunity\nshared\tcommunity\n"))return 8;
    struct sapi_dictionary_file sources[2]={0},loaded[2]={0};size_t count=0;
    sources[0].dialect=sources[1].dialect=0x10000;sources[0].volume=sources[1].volume=0;
    swprintf(sources[0].path,1024,L"%ls\\first.dic",root);swprintf(sources[1].path,1024,L"%ls\\second.dic",root);
    if(!write_file(sources[0].path,"shared\tfirst\ncustom\tone\n")||!write_file(sources[1].path,"shared\tlast\n"))return 9;
    if(FAILED(sapi_dictionary_files_save(sources,2))||FAILED(sapi_dictionary_files_load(loaded,2,&count))||count!=2||
       wcscmp(loaded[0].path,sources[0].path)||wcscmp(loaded[1].path,sources[1].path))return 10;
    struct sapi_settings settings={0};settings.dictionaries=1;wcscpy(settings.directory,installed);
    if(FAILED(sapi_dictionary_resolve(&settings,path,2048)))return 11;wcscpy(first,path);
    swprintf(file,2048,L"%ls\\ENUmain.dic",path);
    if(!equals(file,"alpha\tcommunity\ncustom\tone\nshared\tlast\n")||
       !equals(sources[0].path,"shared\tfirst\ncustom\tone\n"))return 12;
    if(!write_file(sources[1].path,"shared\treplaced-longer\n"))return 13;
    Sleep(550);
    if(FAILED(sapi_dictionary_resolve(&settings,path,2048))||!wcscmp(first,path))return 13;
    swprintf(file,2048,L"%ls\\ENUmain.dic",path);if(!equals(file,"alpha\tcommunity\ncustom\tone\nshared\treplaced-longer\n"))return 14;
    settings.dictionaries=0;if(FAILED(sapi_dictionary_resolve(&settings,path,2048)))return 15;
    swprintf(file,2048,L"%ls\\ENUmain.dic",path);if(!equals(file,"custom\tone\nshared\treplaced-longer\n"))return 16;
    /* Preserve explicit format and voice, and publish NVDA data separately
     * with or without the community native volumes. */
    const char *nvda="\xef\xbb\xbf# NVDA\nyoutube\tbanana\t0\t2\n";
    if(!write_file(sources[0].path,nvda))return 28;
    sources[0].format=SAPI_DICT_NVDA;sources[0].voice=2;
    if(FAILED(sapi_dictionary_files_save(sources,2))||FAILED(sapi_dictionary_files_load(loaded,2,&count))||
       count!=2||loaded[0].format!=SAPI_DICT_NVDA||loaded[0].voice!=2)return 29;
    for(unsigned enabled=0;enabled<2;++enabled){settings.dictionaries=enabled;
        if(FAILED(sapi_dictionary_prepare(&settings,sources,2,path,2048)))return 30;
        swprintf(file,2048,L"%ls\\nvda-000-00010000-2.nvda",path);
        if(!equals(file,nvda)||!equals(sources[0].path,nvda))return 31;
        swprintf(file,2048,L"%ls\\ENUmain.dic",path);
        if(!equals(file,enabled?"alpha\tcommunity\nshared\treplaced-longer\n":"shared\treplaced-longer\n"))return 32;
    }
    sources[0].format=0;sources[0].voice=0;settings.dictionaries=0;
    if(!write_file(sources[0].path,"shared\tfirst\ncustom\tone\n")||FAILED(sapi_dictionary_files_save(sources,2)))return 33;
    sources[0].dialect=0x60000;sources[1].dialect=0x60001;
    if(FAILED(sapi_dictionary_files_save(sources,2))||FAILED(sapi_dictionary_resolve(&settings,path,2048)))return 17;
    swprintf(file,2048,L"%ls\\CHSmain.dic",path);
    if(!equals(file,"custom\tone\nshared\treplaced-longer\n"))return 18;
    if(!DeleteFileW(sources[1].path))return 19;
    settings.dictionaries=1;
    if(SUCCEEDED(sapi_dictionary_prepare(&settings,sources,2,path,2048)))return 20;
    if(FAILED(sapi_dictionary_resolve(&settings,path,2048))||wcscmp(path,installed))return 21;
    settings.dictionaries=0;
    if(FAILED(sapi_dictionary_resolve(&settings,path,2048))||!path[0])return 22;
    struct sapi_dictionary_file saved=sources[0];sources[0]=sources[1];sources[1]=saved;
    if(FAILED(sapi_dictionary_files_save(sources,2))||FAILED(sapi_dictionary_resolve(&settings,path,2048))||path[0])return 23;
    saved=sources[0];sources[0]=sources[1];sources[1]=saved;
    if(FAILED(sapi_dictionary_files_save(sources,2)))return 24;
    if(!write_file(sources[1].path,"shared\trestored\n"))return 25;
    Sleep(550);
    if(FAILED(sapi_dictionary_resolve(&settings,path,2048))||!path[0])return 26;
    swprintf(file,2048,L"%ls\\CHSmain.dic",path);
    if(!equals(file,"custom\tone\nshared\trestored\n"))return 27;
    RegOverridePredefKey(HKEY_CURRENT_USER,NULL);RegCloseKey(isolated);RegDeleteTreeW(HKEY_CURRENT_USER,keyname);
    SetEnvironmentVariableW(L"LOCALAPPDATA",old_appdata);
    puts("SAPI personal dictionaries: per-user files, ordering, merge, edits, missing files and recovery passed");return 0;
}
