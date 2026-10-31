/* Small native installer. A package contains one shared engine DLL; voice
 * tokens are metadata, not fourteen copies of the synthesizer. */
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
static HINSTANCE instance;
static int directory(WCHAR path[2048])
{
    DWORD n=GetModuleFileNameW(instance,path,2048);if(!n||n>=2048)return 0;
    WCHAR *p=wcsrchr(path,L'\\');if(!p)return 0;*p=0;return 1;
}
static int administrator(void)
{
    SID_IDENTIFIER_AUTHORITY authority=SECURITY_NT_AUTHORITY;PSID group=NULL;BOOL member=FALSE;
    if(AllocateAndInitializeSid(&authority,2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&group)){
        CheckTokenMembership(NULL,group,&member);FreeSid(group);}return member;
}
static HRESULT register_dll(const WCHAR *folder,BOOL install)
{
    WCHAR path[2304];swprintf(path,2304,L"%ls\\ReLoqSapi.dll",folder);
    HMODULE dll=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!dll)return HRESULT_FROM_WIN32(GetLastError());
    FARPROC address=GetProcAddress(dll,install?"DllRegisterServer":"DllUnregisterServer");
    HRESULT (WINAPI *call)(void)=NULL;memcpy(&call,&address,sizeof address);
    HRESULT hr=call?call():E_NOINTERFACE;FreeLibrary(dll);return hr;
}
static HRESULT action(BOOL install)
{
    WCHAR source[2048],destination[2048];if(!directory(source))return E_FAIL;
    WCHAR *programs=NULL;HRESULT hr=SHGetKnownFolderPath(&FOLDERID_ProgramFiles,KF_FLAG_DEFAULT,NULL,&programs);
    if(FAILED(hr))return hr;
    int written=swprintf(destination,2048,L"%ls\\ReLoq C\\SAPI5",programs);CoTaskMemFree(programs);
    if(written<0||written>=2048)return E_FAIL;
    if(!administrator()){
        WCHAR self[2048];if(!GetModuleFileNameW(instance,self,2048))return E_FAIL;
        SHELLEXECUTEINFOW execute={.cbSize=sizeof execute,.fMask=SEE_MASK_NOCLOSEPROCESS,
            .lpVerb=L"runas",.lpFile=self,.lpParameters=install?L"/install":L"/uninstall",.nShow=SW_SHOWNORMAL};
        if(!ShellExecuteExW(&execute))return HRESULT_FROM_WIN32(GetLastError());
        if(execute.hProcess)CloseHandle(execute.hProcess);return S_FALSE;
    }
    if(install){
        int rc=SHCreateDirectoryExW(NULL,destination,NULL);
        if(rc!=ERROR_SUCCESS&&rc!=ERROR_ALREADY_EXISTS&&rc!=ERROR_FILE_EXISTS)return HRESULT_FROM_WIN32(rc);
        static const WCHAR *files[]={L"eci.dll",L"ReLoqSapi.dll",L"ReLoqSettings.exe",L"Install-SAPI.exe",L"Test-SAPI.exe",L"README.txt",L"LICENSE"};
        for(size_t i=0;i<sizeof files/sizeof *files;++i){WCHAR from[2304],to[2304];
            swprintf(from,2304,L"%ls\\%ls",source,files[i]);swprintf(to,2304,L"%ls\\%ls",destination,files[i]);
            if(!_wcsicmp(from,to))continue;
            if(!CopyFileW(from,to,FALSE))return HRESULT_FROM_WIN32(GetLastError());
        }
        WCHAR licenses[2304];swprintf(licenses,2304,L"%ls\\licenses",destination);CreateDirectoryW(licenses,NULL);
        static const WCHAR *notices[]={L"OpenEVV-NOTICE.txt",L"Unicode-LICENSE.txt",L"SCOWL-Copyright.txt"};
        for(size_t i=0;i<sizeof notices/sizeof *notices;++i){WCHAR from[2304],to[2304];
            swprintf(from,2304,L"%ls\\licenses\\%ls",source,notices[i]);swprintf(to,2304,L"%ls\\%ls",licenses,notices[i]);
            if(_wcsicmp(from,to)&&!CopyFileW(from,to,FALSE))return HRESULT_FROM_WIN32(GetLastError());}
        WCHAR dic[2304];swprintf(dic,2304,L"%ls\\dictionaries",destination);CreateDirectoryW(dic,NULL);
        static const WCHAR *dictionaries[]={L"DEURoot.dic",L"DEUmain.dic",L"DEUabbr.dic",L"ENURoot.dic",L"ENUmain.dic",L"ENUabbr.dic",L"LICENSE.md",L"manifest.json"};
        for(size_t i=0;i<sizeof dictionaries/sizeof *dictionaries;++i){WCHAR from[2304],to[2304];
            swprintf(from,2304,L"%ls\\dictionaries\\%ls",source,dictionaries[i]);swprintf(to,2304,L"%ls\\%ls",dic,dictionaries[i]);
            /* Keep edited dictionaries on upgrades. An explicit folder can
             * be selected in voice settings without admin permissions. */
            if(!CopyFileW(from,to,TRUE)&&GetLastError()!=ERROR_FILE_EXISTS)return HRESULT_FROM_WIN32(GetLastError());
        }
        return register_dll(destination,TRUE);
    }
    hr=register_dll(destination,FALSE);if(FAILED(hr))return hr;
    static const WCHAR *remove[]={L"eci.dll",L"ReLoqSapi.dll",L"ReLoqSettings.exe",L"Test-SAPI.exe",L"README.txt",L"LICENSE"};
    for(size_t i=0;i<sizeof remove/sizeof *remove;++i){WCHAR path[2304];swprintf(path,2304,L"%ls\\%ls",destination,remove[i]);
        if(!DeleteFileW(path)&&GetLastError()!=ERROR_FILE_NOT_FOUND)return HRESULT_FROM_WIN32(GetLastError());}
    return S_OK; /* Custom settings and dictionaries are deliberately kept. */
}
static void run(HWND window,BOOL install)
{
    HRESULT hr=action(install);if(hr==S_FALSE)return;WCHAR message[512];
    if(FAILED(hr))swprintf(message,512,L"The operation failed (0x%08lX). Close applications using ReLoq and try again.",(unsigned long)hr);
    else swprintf(message,512,L"ReLoq C SAPI5 voices %ls. Restart applications that were already open.",install?L"installed":L"unregistered");
    MessageBoxW(window,message,L"ReLoq C SAPI5",FAILED(hr)?MB_ICONERROR:MB_ICONINFORMATION);
}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_CREATE){
        const WCHAR *text[]={L"&Install / update",L"&Uninstall voices",L"Voice &settings",L"&Close"};
        CreateWindowW(L"STATIC",L"ReLoq C 0.2.201 — SAPI5 x64 preview\nOne voice for each of the 14 languages.",WS_CHILD|WS_VISIBLE,12,12,400,42,window,NULL,instance,NULL);
        for(int i=0;i<4;++i){HWND c=CreateWindowW(L"BUTTON",text[i],WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_PUSHBUTTON,12,66+i*38,400,30,window,(HMENU)(INT_PTR)(100+i),instance,NULL);
            SendMessageW(c,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);}return 0;
    }
    if(message==WM_COMMAND){int id=LOWORD(wp);if(id==100||id==101)run(window,id==100);
        else if(id==102){WCHAR folder[2048],path[2304];if(directory(folder)){swprintf(path,2304,L"%ls\\ReLoqSettings.exe",folder);ShellExecuteW(window,L"open",path,NULL,NULL,SW_SHOWNORMAL);}}
        else if(id==103)DestroyWindow(window);return 0;}
    if(message==WM_CLOSE){DestroyWindow(window);return 0;}if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
int WINAPI wWinMain(HINSTANCE current,HINSTANCE previous,WCHAR *arguments,int show)
{
    (void)previous;instance=current;CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(!wcscmp(arguments,L"/install")||!wcscmp(arguments,L"/uninstall")){
        run(NULL,!wcscmp(arguments,L"/install"));CoUninitialize();return 0;}
    WNDCLASSW cls={.lpfnWndProc=window_proc,.hInstance=instance,.hCursor=LoadCursorW(NULL,IDC_ARROW),
        .hbrBackground=(HBRUSH)(COLOR_BTNFACE+1),.lpszClassName=L"ReLoqSapiInstaller"};RegisterClassW(&cls);
    HWND window=CreateWindowExW(WS_EX_CONTROLPARENT,cls.lpszClassName,L"ReLoq C SAPI5 installer",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
        CW_USEDEFAULT,CW_USEDEFAULT,440,270,NULL,NULL,instance,NULL);if(!window)return 1;
    ShowWindow(window,show);SetFocus(GetDlgItem(window,100));MSG message;
    while(GetMessageW(&message,NULL,0,0)>0){if(!IsDialogMessageW(window,&message)){TranslateMessage(&message);DispatchMessageW(&message);}}
    CoUninitialize();return 0;
}
