/* Windows CI smoke test: exercise the actual HTTPS source and atomic cache. */
#include "settings.h"
#include <stdio.h>
#include <wchar.h>
HMODULE sapi_module;
int sapi_module_path(WCHAR *path,size_t cap,const WCHAR *name)
{(void)name;if(cap<2)return 0;wcscpy(path,L".");return 1;}
int main(void)
{
    DWORD previous=sapi_dictionary_source();
    struct sapi_dictionary_update first={0},second={0};
    HRESULT result=sapi_dictionary_download_source(1,&first);
    if(FAILED(result)){fprintf(stderr,"Dictionary download failed: 0x%08lX\n",(unsigned long)result);return 1;}
    if(!first.known||!first.total||first.total_files!=3||
       (!first.compared&&(first.added!=first.total||first.added_files!=3)))return 8;
    WCHAR path[2048],file[2048];
    if(sapi_dictionary_source()!=1||!sapi_alternative_dictionary_available()||!sapi_default_dictionary_path(path,2048))return 2;
    const WCHAR *names[]={L"ENUmain.dic",L"ENURoot.dic",L"ESPmain.dic"};
    for(unsigned i=0;i<3;++i){WIN32_FILE_ATTRIBUTE_DATA a;
        swprintf(file,2048,L"%ls\\%ls",path,names[i]);
        if(!GetFileAttributesExW(file,GetFileExInfoStandard,&a)||!a.nFileSizeLow)return 3;
    }
    /* A repeated update of the same verified commit must be safe. */
    if(FAILED(sapi_dictionary_download_source(1,&second))||!second.known||!second.compared)return 4;
    WCHAR after[2048];if(!sapi_default_dictionary_path(after,2048))return 5;
    if(!wcscmp(path,after)&&(second.added!=0||second.added_files!=0))return 6;
    struct sapi_dictionary_update community={0},community_again={0};
    if(FAILED(sapi_dictionary_download_source(0,&community))||!community.known||community.total_files!=6)return 9;
    if(sapi_dictionary_source()!=0||!sapi_default_dictionary_path(path,2048)||!wcsstr(path,L"\\Community\\"))return 10;
    const WCHAR *community_names[]={L"DEUmain.dic",L"DEURoot.dic",L"DEUabbr.dic",L"ENUmain.dic",L"ENURoot.dic",L"ENUabbr.dic"};
    for(unsigned i=0;i<6;++i){WIN32_FILE_ATTRIBUTE_DATA a;
        swprintf(file,2048,L"%ls\\%ls",path,community_names[i]);
        if(!GetFileAttributesExW(file,GetFileExInfoStandard,&a)||!a.nFileSizeLow)return 11;}
    if(FAILED(sapi_dictionary_download_source(0,&community_again))||!community_again.known||!community_again.compared||
       community_again.added||community_again.added_files)return 12;
    if(FAILED(sapi_dictionary_choose(previous)))return 7;
    puts("Verified both dictionary sources, conversion, per-user atomic cache and source selection");return 0;
}
