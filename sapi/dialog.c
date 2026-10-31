/* Standard Windows controls provide keyboard and screen-reader semantics.
 * Every valid change commits immediately to this user's language settings. */
#include "settings.h"
#include <commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#include <wchar.h>
enum {LANGUAGE=100,PROFILE,GENDER,RATE,PAUSES,PITCH,INFLECTION,HEAD,ROUGH,BREATH,
      TAGS,ABBREVIATIONS,PREDICTION,DICTIONARIES,BOOST,DIRECTORY,OPEN_FOLDER,RESET,STATUS};
struct dialog {struct sapi_settings s;int dialect,loading;};
static HWND control(HWND parent,const WCHAR *type,const WCHAR *text,DWORD style,int id,int x,int y,int w,int h)
{
    HWND c=CreateWindowExW(!wcscmp(type,L"EDIT")?WS_EX_CLIENTEDGE:0,type,text,
        WS_CHILD|WS_VISIBLE|style,x,y,w,h,parent,(HMENU)(INT_PTR)id,sapi_module,NULL);
    if(c)SendMessageW(c,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);return c;
}
static void label(HWND p,const WCHAR *text,int x,int y,int w)
{control(p,L"STATIC",text,0,-1,x,y+4,w,20);}
static void combo(HWND p,int id,const WCHAR *title,int y)
{label(p,title,12,y,140);control(p,L"COMBOBOX",L"",WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,id,158,y,330,240);}
static void add(HWND p,int id,const WCHAR *text)
{SendDlgItemMessageW(p,id,CB_ADDSTRING,0,(LPARAM)text);}
static void number(HWND p,int id,const WCHAR *title,int y)
{
    label(p,title,12,y,235);HWND e=control(p,L"EDIT",L"",WS_TABSTOP|ES_NUMBER|ES_AUTOHSCROLL,id,260,y,80,24);
    HWND spin=control(p,UPDOWN_CLASSW,L"",UDS_ALIGNRIGHT|UDS_SETBUDDYINT|UDS_ARROWKEYS|UDS_NOTHOUSANDS,id+200,0,0,0,0);
    SendMessageW(spin,UDM_SETBUDDY,(WPARAM)e,0);SendMessageW(spin,UDM_SETRANGE32,0,100);
}
static void check(HWND p,int id,const WCHAR *text,int y)
{control(p,L"BUTTON",text,WS_TABSTOP|BS_AUTOCHECKBOX,id,12,y,472,23);}
static void populate(HWND window,struct dialog *d)
{
    d->loading=1;
    SendDlgItemMessageW(window,LANGUAGE,CB_SETCURSEL,sapi_language_index(d->dialect),0);
    SendDlgItemMessageW(window,PROFILE,CB_SETCURSEL,d->s.profile,0);
    SendDlgItemMessageW(window,GENDER,CB_SETCURSEL,d->s.gender,0);
    SendDlgItemMessageW(window,RATE,CB_SETCURSEL,d->s.rate,0);
    SendDlgItemMessageW(window,PAUSES,CB_SETCURSEL,d->s.pauses,0);
    const DWORD values[]={d->s.pitch,d->s.inflection,d->s.head,d->s.roughness,d->s.breathiness};
    for(int i=0;i<5;++i)SetDlgItemInt(window,PITCH+i,values[i],FALSE);
    const DWORD checks[]={d->s.tags,d->s.abbreviations,d->s.prediction,d->s.dictionaries,d->s.boost};
    for(int i=0;i<5;++i)CheckDlgButton(window,TAGS+i,checks[i]?BST_CHECKED:BST_UNCHECKED);
    SetDlgItemTextW(window,DIRECTORY,d->s.directory);d->loading=0;
}
static void persist(HWND w,struct dialog *d)
{
    HRESULT hr=sapi_settings_write(d->dialect,&d->s);
    SetDlgItemTextW(w,STATUS,FAILED(hr)?L"Could not store this change.":L"Active for the next utterance. No Save button is needed.");
}
static INT_PTR CALLBACK dialog_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    struct dialog *d=(struct dialog *)GetWindowLongPtrW(w,DWLP_USER);
    if(message==WM_INITDIALOG){d=(struct dialog *)lp;SetWindowLongPtrW(w,DWLP_USER,(LONG_PTR)d);
        RECT frame={0,0,500,640};AdjustWindowRectEx(&frame,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),FALSE,0);
        SetWindowPos(w,NULL,0,0,frame.right-frame.left,frame.bottom-frame.top,SWP_NOMOVE|SWP_NOZORDER);
        combo(w,LANGUAGE,L"&Language:",12);combo(w,PROFILE,L"Voice &profile:",44);
        combo(w,GENDER,L"Voice &model:",76);combo(w,RATE,L"Sample &rate:",108);combo(w,PAUSES,L"Shorten &pauses:",140);
        for(size_t i=0;i<sapi_language_count();++i)add(w,LANGUAGE,sapi_language_at(i)->name);
        for(unsigned i=0;i<8;++i)add(w,PROFILE,sapi_profile_name(i));
        add(w,GENDER,L"Male");add(w,GENDER,L"Female");
        for(unsigned i=0;i<7;++i){WCHAR text[32];swprintf(text,32,L"%d Hz",sapi_sample_rate(i));add(w,RATE,text);}
        add(w,PAUSES,L"Never");add(w,PAUSES,L"At the end of an utterance");add(w,PAUSES,L"Always");
        number(w,PITCH,L"Base &pitch (0–100):",180);number(w,INFLECTION,L"&Inflection (0–100):",212);
        number(w,HEAD,L"&Head size (0–100):",244);number(w,ROUGH,L"Ro&ughness (0–100):",276);
        number(w,BREATH,L"&Breathiness (0–100):",308);
        check(w,TAGS,L"Process &ECI tags",344);check(w,ABBREVIATIONS,L"Use &abbreviation dictionary",368);
        check(w,PREDICTION,L"Use phrase pre&diction",392);check(w,DICTIONARIES,L"Use community / custom &dictionaries",416);
        check(w,BOOST,L"Enable speed b&oost",440);
        label(w,L"Dictionary folder (empty = bundled):",12,470,460);
        control(w,L"EDIT",L"",WS_TABSTOP|ES_AUTOHSCROLL,DIRECTORY,12,496,372,24);
        control(w,L"BUTTON",L"&Open folder",WS_TABSTOP|BS_PUSHBUTTON,OPEN_FOLDER,390,496,98,24);
        label(w,L"SAPI controls speed and volume. Sample rate applies on format negotiation.",12,528,475);
        control(w,L"STATIC",L"",0,STATUS,12,555,475,36);
        control(w,L"BUTTON",L"&Reset language",WS_TABSTOP|BS_PUSHBUTTON,RESET,12,600,140,28);
        control(w,L"BUTTON",L"&Close",WS_TABSTOP|BS_DEFPUSHBUTTON,IDOK,378,600,110,28);
        populate(w,d);SetDlgItemTextW(w,STATUS,L"Changes apply immediately to the next utterance.");
        SetFocus(GetDlgItem(w,LANGUAGE));return FALSE;
    }
    if(!d)return FALSE;
    if(message==WM_CLOSE){EndDialog(w,IDOK);return TRUE;}
    if(message!=WM_COMMAND)return FALSE;
    int id=LOWORD(wp),notification=HIWORD(wp);
    if(id==IDOK||id==IDCANCEL){EndDialog(w,id);return TRUE;}
    if(d->loading)return TRUE;
    if(id==LANGUAGE&&notification==CBN_SELCHANGE){int i=(int)SendDlgItemMessageW(w,id,CB_GETCURSEL,0,0);
        const struct sapi_language *language=sapi_language_at((size_t)i);if(!language)return TRUE;
        d->dialect=language->dialect;HRESULT hr=sapi_settings_read(d->dialect,&d->s);
        if(FAILED(hr))sapi_settings_default(&d->s);populate(w,d);return TRUE;}
    if(id==OPEN_FOLDER){WCHAR path[2048];
        if(d->s.directory[0])wcscpy(path,d->s.directory);else if(!sapi_module_path(path,2048,L"dictionaries"))return TRUE;
        ShellExecuteW(w,L"open",path,NULL,NULL,SW_SHOWNORMAL);return TRUE;}
    if(id==RESET){sapi_settings_default(&d->s);populate(w,d);persist(w,d);return TRUE;}
    if(notification==CBN_SELCHANGE&&id>=PROFILE&&id<=PAUSES){
        DWORD value=(DWORD)SendDlgItemMessageW(w,id,CB_GETCURSEL,0,0);
        if(id==PROFILE){sapi_settings_profile(&d->s,value);populate(w,d);}
        else if(id==GENDER)d->s.gender=value;else if(id==RATE)d->s.rate=value;else d->s.pauses=value;
        persist(w,d);return TRUE;
    }
    if(notification==EN_CHANGE&&id>=PITCH&&id<=BREATH){BOOL translated=FALSE;UINT value=GetDlgItemInt(w,id,&translated,FALSE);
        if(!translated||value>100)return TRUE;
        DWORD *fields[]={&d->s.pitch,&d->s.inflection,&d->s.head,&d->s.roughness,&d->s.breathiness};
        *fields[id-PITCH]=value;persist(w,d);return TRUE;}
    if(notification==BN_CLICKED&&id>=TAGS&&id<=BOOST){DWORD *fields[]={&d->s.tags,&d->s.abbreviations,&d->s.prediction,&d->s.dictionaries,&d->s.boost};
        *fields[id-TAGS]=IsDlgButtonChecked(w,id)==BST_CHECKED;persist(w,d);return TRUE;}
    if(id==DIRECTORY&&notification==EN_CHANGE){GetDlgItemTextW(w,id,d->s.directory,1024);persist(w,d);return TRUE;}
    return FALSE;
}
HRESULT sapi_settings_dialog(HWND parent,int dialect)
{
    struct dialog d={.dialect=dialect};if(sapi_language_index(dialect)<0)d.dialect=0x10000;
    if(FAILED(sapi_settings_read(d.dialect,&d.s)))sapi_settings_default(&d.s);
    INITCOMMONCONTROLSEX common={sizeof common,ICC_UPDOWN_CLASS};InitCommonControlsEx(&common);
    /* Empty standard dialog template; all controls are ordinary Win32. */
    struct {DLGTEMPLATE dialog;WORD menu,window_class;WCHAR title[32];} templ={
        .dialog={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_CENTER,.cx=335,.cy=430},
        .title=L"ReLoq C voice settings"};
    /* Controls use pixels, so size the client area independently of dialog
     * units. The initial WM_INITDIALOG layout fits a 500 × 640 client area. */
    INT_PTR result=DialogBoxIndirectParamW(sapi_module,&templ.dialog,parent,dialog_proc,(LPARAM)&d);
    return result==-1?HRESULT_FROM_WIN32(GetLastError()):S_OK;
}
