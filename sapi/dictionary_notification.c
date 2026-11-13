/* A Shell balloon is transient on Windows 11. This worker owns a private,
 * hidden window, so completion is independent of the settings dialog. */
#include "dictionary_notification.h"
#include <shellapi.h>
#include <wchar.h>

#define DICTIONARY_NOTICE_CALLBACK (WM_APP+84)
typedef BOOL (WINAPI *shell_notify)(DWORD,PNOTIFYICONDATAW);

static BOOL notify_with(const WCHAR *title,const WCHAR *text,shell_notify notify)
{
    if(!title||!text||!text[0]||!notify)return FALSE;
    HWND window=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"",WS_POPUP,0,0,0,0,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window)return FALSE;
    NOTIFYICONDATAW icon={0};
    icon.cbSize=sizeof icon;icon.hWnd=window;icon.uID=1;
    icon.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;
    icon.uCallbackMessage=DICTIONARY_NOTICE_CALLBACK;
    icon.hIcon=LoadIconW(NULL,IDI_INFORMATION);
    wcsncpy(icon.szTip,title,(sizeof icon.szTip/sizeof *icon.szTip)-1);
    BOOL added=notify(NIM_ADD,&icon),shown=FALSE;
    if(added) {
        icon.uFlags=NIF_INFO;
        icon.dwInfoFlags=NIIF_INFO|NIIF_RESPECT_QUIET_TIME;
        wcsncpy(icon.szInfoTitle,title,(sizeof icon.szInfoTitle/sizeof *icon.szInfoTitle)-1);
        wcsncpy(icon.szInfo,text,(sizeof icon.szInfo/sizeof *icon.szInfo)-1);
        shown=notify(NIM_MODIFY,&icon);
        if(shown&&SetTimer(window,1,30000,NULL)) {
            MSG message;
            while(GetMessageW(&message,window,0,0)>0) {
                if(message.message==WM_TIMER ||
                   (message.message==DICTIONARY_NOTICE_CALLBACK &&
                    (message.lParam==NIN_BALLOONHIDE||message.lParam==NIN_BALLOONTIMEOUT||
                     message.lParam==NIN_BALLOONUSERCLICK)))break;
                TranslateMessage(&message);DispatchMessageW(&message);
            }
            KillTimer(window,1);
        }
        notify(NIM_DELETE,&icon);
    }
    DestroyWindow(window);
    return shown;
}

BOOL sapi_dictionary_notify(const WCHAR *title,const WCHAR *text)
{return notify_with(title,text,Shell_NotifyIconW);}

#ifdef RELOQ_DICTIONARY_NOTICE_TEST
BOOL sapi_dictionary_notify_test(const WCHAR *title,const WCHAR *text,shell_notify mock)
{return notify_with(title,text,mock);}
#endif
