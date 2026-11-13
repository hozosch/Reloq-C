/* Exercise the Shell interaction without posting real user notifications. */
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <wchar.h>

typedef BOOL (WINAPI *shell_notify)(DWORD,PNOTIFYICONDATAW);
extern BOOL sapi_dictionary_notify_test(const WCHAR *,const WCHAR *,shell_notify);
static int calls,fail_at;
static BOOL WINAPI fake_notify(DWORD action,PNOTIFYICONDATAW icon)
{
    ++calls;
    if(action==NIM_ADD) {
        if(calls!=1||!icon->hWnd||!(icon->uFlags&NIF_MESSAGE))return FALSE;
    } else if(action==NIM_MODIFY) {
        if(calls!=2||!(icon->uFlags&NIF_INFO)||wcscmp(icon->szInfoTitle,L"ReLoq C")||
           wcscmp(icon->szInfo,L"New entries: 3; new files: 1"))return FALSE;
        if(fail_at==2)return FALSE;
        PostMessageW(icon->hWnd,icon->uCallbackMessage,0,NIN_BALLOONTIMEOUT);
    } else if(action==NIM_DELETE) {
        if(calls!=3)return FALSE;
    } else return FALSE;
    return calls!=fail_at;
}
int main(void)
{
    if(!sapi_dictionary_notify_test(L"ReLoq C",L"New entries: 3; new files: 1",fake_notify)||calls!=3)return 1;
    calls=0;fail_at=1;
    if(sapi_dictionary_notify_test(L"ReLoq C",L"test",fake_notify)||calls!=1)return 2;
    calls=0;fail_at=2;
    if(sapi_dictionary_notify_test(L"ReLoq C",L"New entries: 3; new files: 1",fake_notify)||calls!=3)return 3;
    puts("Shell balloon lifecycle and failure handling passed");
    return 0;
}
