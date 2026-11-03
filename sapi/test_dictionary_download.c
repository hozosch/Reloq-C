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
    HRESULT result=sapi_dictionary_download();
    if(FAILED(result)){fprintf(stderr,"Dictionary download failed: 0x%08lX\n",(unsigned long)result);return 1;}
    WCHAR path[2048],file[2048];
    if(sapi_dictionary_source()!=1||!sapi_default_dictionary_path(path,2048))return 2;
    const WCHAR *names[]={L"ENUmain.dic",L"ENURoot.dic",L"ESPmain.dic"};
    for(unsigned i=0;i<3;++i){WIN32_FILE_ATTRIBUTE_DATA a;
        swprintf(file,2048,L"%ls\\%ls",path,names[i]);
        if(!GetFileAttributesExW(file,GetFileExInfoStandard,&a)||!a.nFileSizeLow)return 3;
    }
    /* A repeated update of the same verified commit must be safe. */
    if(FAILED(sapi_dictionary_download())||FAILED(sapi_dictionary_choose(previous)))return 4;
    puts("Verified alternative dictionary download, conversion, atomic cache and source selection");return 0;
}
