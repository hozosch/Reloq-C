#include <windows.h>
#include <wchar.h>
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,WCHAR *arguments,int show)
{
    (void)previous;(void)arguments;(void)show;WCHAR path[2048];
    DWORD n=GetModuleFileNameW(instance,path,2048);if(!n||n>=2048)return 1;
    WCHAR *p=wcsrchr(path,L'\\');if(!p||p-path>2000)return 1;wcscpy(p+1,L"ReLoqSapi.dll");
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!module){MessageBoxW(NULL,L"ReLoqSapi.dll could not be loaded.",L"ReLoq C",MB_ICONERROR);return 1;}
    FARPROC address=GetProcAddress(module,"ReLoqSettings");HRESULT (WINAPI *dialog)(HWND,int)=NULL;
    memcpy(&dialog,&address,sizeof address);HRESULT hr=dialog?dialog(NULL,-1):E_FAIL;
    FreeLibrary(module);return FAILED(hr)?1:0;
}
