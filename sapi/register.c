#include "settings.h"
#include <stdio.h>
#include <wchar.h>
static LSTATUS string(HKEY base,const WCHAR *path,const WCHAR *name,const WCHAR *value)
{
    HKEY key;LSTATUS rc=RegCreateKeyExW(base,path,0,NULL,0,KEY_SET_VALUE|KEY_WOW64_64KEY,NULL,&key,NULL);
    if(rc==ERROR_SUCCESS){rc=RegSetValueExW(key,name,0,REG_SZ,(const BYTE *)value,(DWORD)((wcslen(value)+1)*2));RegCloseKey(key);}return rc;
}
HRESULT sapi_register(BOOL install,BOOL machine)
{
    HKEY base=machine?HKEY_LOCAL_MACHINE:HKEY_CURRENT_USER;
    WCHAR path[2048],key[512],name[256],lcid[16];LSTATUS rc;
    if(install&&!GetModuleFileNameW(sapi_module,path,2048))return HRESULT_FROM_WIN32(GetLastError());
    const WCHAR *classes[]={RELOQ_ENGINE_CLSID,RELOQ_SETTINGS_CLSID};
    for(unsigned i=0;i<2;++i){swprintf(key,512,L"Software\\Classes\\CLSID\\%ls",classes[i]);
        if(!install){rc=RegDeleteTreeW(base,key);if(rc!=ERROR_SUCCESS&&rc!=ERROR_FILE_NOT_FOUND)return HRESULT_FROM_WIN32(rc);continue;}
        rc=string(base,key,NULL,i?L"ReLoq C voice settings":L"ReLoq C SAPI5 engine");if(rc)return HRESULT_FROM_WIN32(rc);
        wcscat(key,L"\\InprocServer32");rc=string(base,key,NULL,path);if(rc)return HRESULT_FROM_WIN32(rc);
        rc=string(base,key,L"ThreadingModel",L"Both");if(rc)return HRESULT_FROM_WIN32(rc);
    }
    for(size_t i=0;i<sapi_language_count();++i){const struct sapi_language *v=sapi_language_at(i);
        swprintf(key,512,L"Software\\Microsoft\\Speech\\Voices\\Tokens\\ReLoqC_%02u_%08X",(unsigned)i,(unsigned)v->dialect);
        if(!install){rc=RegDeleteTreeW(base,key);if(rc!=ERROR_SUCCESS&&rc!=ERROR_FILE_NOT_FOUND)return HRESULT_FROM_WIN32(rc);continue;}
        swprintf(name,256,L"ReLoq C - %ls",v->name);rc=string(base,key,NULL,name);if(rc)return HRESULT_FROM_WIN32(rc);
        rc=string(base,key,L"CLSID",RELOQ_ENGINE_CLSID);if(rc)return HRESULT_FROM_WIN32(rc);
        HKEY token;rc=RegOpenKeyExW(base,key,0,KEY_SET_VALUE|KEY_WOW64_64KEY,&token);if(rc)return HRESULT_FROM_WIN32(rc);
        DWORD dialect=(DWORD)v->dialect;rc=RegSetValueExW(token,L"ReLoqDialect",0,REG_DWORD,(BYTE *)&dialect,sizeof dialect);RegCloseKey(token);if(rc)return HRESULT_FROM_WIN32(rc);
        size_t root=wcslen(key);wcscat(key,L"\\Attributes");
        swprintf(lcid,16,L"%X",(unsigned)v->lcid);
        if((rc=string(base,key,L"Name",name))||(rc=string(base,key,L"Language",lcid))||
           (rc=string(base,key,L"Vendor",L"ReLoq C"))||(rc=string(base,key,L"Gender",L"Male"))||
           (rc=string(base,key,L"Age",L"Adult")))return HRESULT_FROM_WIN32(rc);
        key[root]=0;wcscat(key,L"\\UI\\EngineProperties");
        rc=string(base,key,NULL,RELOQ_SETTINGS_CLSID);if(rc)return HRESULT_FROM_WIN32(rc);
        rc=string(base,key,L"CLSID",RELOQ_SETTINGS_CLSID);if(rc)return HRESULT_FROM_WIN32(rc);
    }
    return S_OK;
}
