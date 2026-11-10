/* Independent per-user updater. It never runs inside a SAPI host process. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <wincrypt.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "update_data.h"
#include "ui.inc"
#ifndef RELOQ_APP_VERSION
#define RELOQ_APP_VERSION "1.3.0"
#endif
#define CHECK_LIMIT (2u*1024u*1024u)
static const WCHAR update_key[]=L"Software\\ReLoq C\\SAPI5";
static const struct sapi_ui *ui;
#define T(name) (ui->text[UI_##name])
static BOOL automatic(void)
{
    DWORD setting=1,bytes=sizeof setting;
    if(RegGetValueW(HKEY_CURRENT_USER,update_key,L"AutomaticUpdates",RRF_RT_REG_DWORD,NULL,&setting,&bytes)!=ERROR_SUCCESS)return TRUE;
    return setting!=0;
}
static BOOL set_automatic(BOOL enabled)
{
    HKEY key;DWORD value=enabled?1:0;LSTATUS r=RegCreateKeyExW(HKEY_CURRENT_USER,update_key,0,NULL,0,KEY_SET_VALUE,NULL,&key,NULL);
    if(r!=ERROR_SUCCESS)return FALSE;r=RegSetValueExW(key,L"AutomaticUpdates",0,REG_DWORD,(BYTE *)&value,sizeof value);
    RegCloseKey(key);return r==ERROR_SUCCESS;
}
static BOOL previously_offered(const WCHAR *version)
{
    WCHAR stored[48]={0};DWORD bytes=sizeof stored;
    return RegGetValueW(HKEY_CURRENT_USER,update_key,L"LastOfferedUpdate",RRF_RT_REG_SZ,NULL,stored,&bytes)==ERROR_SUCCESS
        &&!wcscmp(stored,version);
}
static void remember_offer(const WCHAR *version)
{
    HKEY key;if(RegCreateKeyExW(HKEY_CURRENT_USER,update_key,0,NULL,0,KEY_SET_VALUE,NULL,&key,NULL)!=ERROR_SUCCESS)return;
    RegSetValueExW(key,L"LastOfferedUpdate",0,REG_SZ,(const BYTE *)version,(DWORD)((wcslen(version)+1)*sizeof(WCHAR)));
    RegCloseKey(key);
}
static HINTERNET request(HINTERNET session,const WCHAR *address,HINTERNET *connection)
{
    URL_COMPONENTSW c={0};WCHAR host[256],path[2048],extra[1024];c.dwStructSize=sizeof c;
    c.lpszHostName=host;c.dwHostNameLength=256;c.lpszUrlPath=path;c.dwUrlPathLength=2048;
    c.lpszExtraInfo=extra;c.dwExtraInfoLength=1024;
    if(!WinHttpCrackUrl(address,0,0,&c)||c.nScheme!=INTERNET_SCHEME_HTTPS||c.dwUserNameLength||c.dwPasswordLength)return NULL;
    if(c.dwUrlPathLength+c.dwExtraInfoLength+1>=2048)return NULL;
    path[c.dwUrlPathLength]=0;extra[c.dwExtraInfoLength]=0;wcscat(path,extra);host[c.dwHostNameLength]=0;
    *connection=WinHttpConnect(session,host,c.nPort,0);if(!*connection)return NULL;
    HINTERNET r=WinHttpOpenRequest(*connection,L"GET",path,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE);
    if(!r)return NULL;
    DWORD policy=WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP;
    if(!WinHttpSetOption(r,WINHTTP_OPTION_REDIRECT_POLICY,&policy,sizeof policy)||
       !WinHttpSendRequest(r,WINHTTP_NO_ADDITIONAL_HEADERS,0,WINHTTP_NO_REQUEST_DATA,0,0,0)||!WinHttpReceiveResponse(r,NULL)){
        WinHttpCloseHandle(r);return NULL;}
    DWORD status=0,size=sizeof status;
    if(!WinHttpQueryHeaders(r,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&size,WINHTTP_NO_HEADER_INDEX)||status!=200){
        WinHttpCloseHandle(r);return NULL;}
    return r;
}
static HINTERNET session(void)
{
    HINTERNET s=WinHttpOpen(L"ReLoq-C-SAPI-updater",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
    if(s)WinHttpSetTimeouts(s,10000,10000,30000,30000);return s;
}
static int check(struct reloq_update *update)
{
    int result=-1;DWORD used=0;unsigned char *buffer=malloc(CHECK_LIMIT+1);HINTERNET s=session(),c=NULL,r=NULL;
    if(!buffer||!s)goto done;
    r=request(s,L"https://api.github.com/repos/hozosch/Reloq-C/releases/latest",&c);if(!r)goto done;
    for(;;){DWORD got=0;if(!WinHttpReadData(r,buffer+used,CHECK_LIMIT+1-used,&got))goto done;
        used+=got;if(used>CHECK_LIMIT)goto done;if(!got)break;}
    result=reloq_update_parse((char *)buffer,used,RELOQ_APP_VERSION,update);
 done:if(r)WinHttpCloseHandle(r);if(c)WinHttpCloseHandle(c);if(s)WinHttpCloseHandle(s);free(buffer);return result;
}
static int verified_download(const struct reloq_update *update,WCHAR path[MAX_PATH])
{
    WCHAR dir[MAX_PATH],tmp[MAX_PATH],address[1024];DWORD n=GetTempPathW(MAX_PATH,dir);
    if(!n||n>=MAX_PATH||!GetTempFileNameW(dir,L"rlq",0,tmp))return 0;
    if(swprintf(path,MAX_PATH,L"%ls.exe",tmp)<0||!MoveFileW(tmp,path)){DeleteFileW(tmp);return 0;}
    if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,update->url,-1,address,1024)){DeleteFileW(path);return 0;}
    HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,TRUNCATE_EXISTING,FILE_ATTRIBUTE_TEMPORARY,NULL);
    HCRYPTPROV provider=0;HCRYPTHASH hash=0;HINTERNET s=NULL,c=NULL,r=NULL;int ok=0;
    if(file==INVALID_HANDLE_VALUE)goto done;
    if(!CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT)||
       !CryptCreateHash(provider,CALG_SHA_256,0,0,&hash))goto done;
    s=session();if(!s)goto done;r=request(s,address,&c);if(!r)goto done;
    DWORD total=0;unsigned char buffer[65536];
    for(;;){DWORD got=0,written=0;if(!WinHttpReadData(r,buffer,sizeof buffer,&got))goto done;if(!got)break;
        if(got>update->size-total||!CryptHashData(hash,buffer,got,0)||
           !WriteFile(file,buffer,got,&written,NULL)||written!=got)goto done;total+=got;}
    if(total!=update->size||!FlushFileBuffers(file))goto done;
    BYTE digest[32];DWORD len=sizeof digest;if(!CryptGetHashParam(hash,HP_HASHVAL,digest,&len,0)||len!=sizeof digest)goto done;
    char hex[65];static const char digits[]="0123456789abcdef";
    for(unsigned i=0;i<32;++i){hex[i*2]=digits[digest[i]>>4];hex[i*2+1]=digits[digest[i]&15];}hex[64]=0;
    ok=!strcmp(hex,update->sha256);
 done:if(r)WinHttpCloseHandle(r);if(c)WinHttpCloseHandle(c);if(s)WinHttpCloseHandle(s);
    if(hash)CryptDestroyHash(hash);if(provider)CryptReleaseContext(provider,0);
    if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);if(!ok)DeleteFileW(path);return ok;
}
/* Return 2 after launching an installer so the background process releases its executable. */
static int check_once(BOOL interactive,char *offered,size_t offered_size)
{
    struct reloq_update update={0};int found=check(&update);
    if(found<0){if(interactive)MessageBoxW(NULL,T(UPDATE_FAILED),T(UPDATE_TITLE),MB_OK|MB_ICONERROR|MB_SETFOREGROUND);return 0;}
    if(!found){if(interactive)MessageBoxW(NULL,T(UPDATE_NONE),T(UPDATE_TITLE),MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND);return 0;}
    if(!interactive&&offered&&strcmp(offered,update.version)==0)return 0;
    if(offered&&strlen(update.version)<offered_size)strcpy(offered,update.version);
    WCHAR version[48],question[512];
    if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,update.version,-1,version,48))return 0;
    if(!interactive&&previously_offered(version))return 0;
    if(!interactive)remember_offer(version);
    swprintf(question,512,T(UPDATE_AVAILABLE),version);
    if(MessageBoxW(NULL,question,T(UPDATE_TITLE),MB_YESNO|MB_ICONQUESTION|MB_SETFOREGROUND)!=IDYES)return 0;
    MessageBoxW(NULL,T(UPDATE_DOWNLOADING),T(UPDATE_TITLE),MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND);
    WCHAR path[MAX_PATH];
    if(!verified_download(&update,path)){MessageBoxW(NULL,T(UPDATE_FAILED),T(UPDATE_TITLE),MB_OK|MB_ICONERROR|MB_SETFOREGROUND);return 0;}
    if((INT_PTR)ShellExecuteW(NULL,L"open",path,NULL,NULL,SW_SHOWNORMAL)<=32){
        DeleteFileW(path);MessageBoxW(NULL,T(UPDATE_LAUNCH_FAILED),T(UPDATE_TITLE),MB_OK|MB_ICONERROR|MB_SETFOREGROUND);return 0;}
    return 2;
}
enum {AUTO_CHECK=300,CHECK_NOW};
static void launch_self(const WCHAR *arguments)
{
    WCHAR path[MAX_PATH];DWORD n=GetModuleFileNameW(NULL,path,MAX_PATH);
    if(n&&n<MAX_PATH)ShellExecuteW(NULL,L"open",path,arguments,NULL,SW_SHOWNORMAL);
}
static HWND button(HWND parent,const WCHAR *text,DWORD style,int id,int x,int y,int width)
{
    HWND h=CreateWindowW(L"BUTTON",text,WS_CHILD|WS_VISIBLE|WS_TABSTOP|style,x,y,width,26,parent,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL);
    SendMessageW(h,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);return h;
}
static INT_PTR CALLBACK dialog(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    (void)lp;
    if(message==WM_INITDIALOG){SetWindowTextW(w,T(UPDATE_TITLE));
        RECT frame={0,0,440,132};AdjustWindowRectEx(&frame,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),FALSE,0);
        SetWindowPos(w,NULL,0,0,frame.right-frame.left,frame.bottom-frame.top,SWP_NOMOVE|SWP_NOZORDER);
        button(w,T(AUTO_UPDATE),BS_AUTOCHECKBOX,AUTO_CHECK,12,12,416);
        CheckDlgButton(w,AUTO_CHECK,automatic()?BST_CHECKED:BST_UNCHECKED);
        button(w,T(CHECK_UPDATE),BS_PUSHBUTTON,CHECK_NOW,12,52,250);
        button(w,T(CLOSE),BS_DEFPUSHBUTTON,IDOK,318,90,110);return TRUE;}
    if(message==WM_CLOSE||(message==WM_COMMAND&&LOWORD(wp)==IDOK)){EndDialog(w,0);return TRUE;}
    if(message==WM_COMMAND&&LOWORD(wp)==AUTO_CHECK&&HIWORD(wp)==BN_CLICKED){
        BOOL enabled=IsDlgButtonChecked(w,AUTO_CHECK)==BST_CHECKED;
        if(!set_automatic(enabled))MessageBoxW(w,T(ERROR),T(UPDATE_TITLE),MB_OK|MB_ICONERROR);
        else if(enabled)launch_self(L"--once");return TRUE;}
    if(message==WM_COMMAND&&LOWORD(wp)==CHECK_NOW&&HIWORD(wp)==BN_CLICKED){launch_self(L"--check");return TRUE;}
    return FALSE;
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,LPWSTR command,int show)
{
    (void)previous;(void)show;ui=sapi_ui_for_language(GetUserDefaultUILanguage());
#ifdef RELOQ_UPDATER_TEST
    int argc=0;LPWSTR *argv=CommandLineToArgvW(GetCommandLineW(),&argc);
    if(argv&&argc==5&&!wcscmp(argv[1],L"--test-download")) {
        struct reloq_update test={0};WCHAR *end=NULL;unsigned long size=wcstoul(argv[3],&end,10);
        int valid=end&&!*end&&size>0&&size<=RELOQ_UPDATE_LIMIT&&
            WideCharToMultiByte(CP_UTF8,0,argv[2],-1,test.url,sizeof test.url,NULL,NULL)>0&&
            WideCharToMultiByte(CP_UTF8,0,argv[4],-1,test.sha256,sizeof test.sha256,NULL,NULL)>0;
        test.size=(uint32_t)size;WCHAR path[MAX_PATH];int ok=valid&&verified_download(&test,path);
        if(ok)DeleteFileW(path);LocalFree(argv);return ok?0:9;
    }
    if(argv)LocalFree(argv);
#endif
    if(!wcscmp(command,L"--background")){
        HANDLE mutex=CreateMutexW(NULL,TRUE,L"Local\\ReLoqC-SAPI-Update");
        if(!mutex||GetLastError()==ERROR_ALREADY_EXISTS){if(mutex)CloseHandle(mutex);return 0;}
        char offered[48]={0};for(;;){if(automatic()&&check_once(FALSE,offered,sizeof offered)==2)break;Sleep(24u*60u*60u*1000u);}
        ReleaseMutex(mutex);CloseHandle(mutex);return 0;
    }
    if(!wcscmp(command,L"--check")){check_once(TRUE,NULL,0);return 0;}
    if(!wcscmp(command,L"--once")){if(automatic())check_once(FALSE,NULL,0);return 0;}
    struct {DLGTEMPLATE dialog;WORD menu,window_class;WCHAR title[32];} templ={
        .dialog={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_CENTER,.cx=240,.cy=90},.title=L"ReLoq C updates"};
    DialogBoxIndirectParamW(instance,&templ.dialog,NULL,dialog,0);return 0;
}
