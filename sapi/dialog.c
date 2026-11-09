/* Standard Windows controls provide keyboard and screen-reader semantics.
 * Every valid change commits immediately to this user's language settings. */
#include "settings.h"
#include "dictionary_diff.h"
#include <commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "ui.inc"
enum {LANGUAGE=100,PROFILE,GENDER,RATE,PAUSES,PITCH,INFLECTION,HEAD,ROUGH,BREATH,
      TAGS,ABBREVIATIONS,PREDICTION,DICTIONARIES,BOOST,DIRECTORY,OPEN_FOLDER,RESET,STATUS,SHARED,
      DICT_SOURCE,DICT_DOWNLOAD,ORIGINAL_QUESTIONS,MELLOW_MODE,PITCH_HZ_MODE,PITCH_HZ_VALUE,
      OVERRIDES,OWN_RATE,OWN_RATE_VALUE,OWN_PITCH,OWN_VOLUME,OWN_VOLUME_VALUE,
      OWN_RATE_ECI,OWN_RATE_WPM,OWN_RATE_WPM_VALUE,OWN_PITCH_ECI,OWN_PITCH_HZ,
      OWN_PITCH_VALUE,OWN_PITCH_HZ_VALUE};
#define DICTIONARY_DONE (WM_APP+31)
struct dictionary_task {HWND window;HMODULE module;};
static DWORD WINAPI dictionary_worker(void *parameter)
{
    struct dictionary_task *task=parameter;HWND window=task->window;HMODULE module=task->module;free(task);
    struct sapi_dictionary_update update={0};
    HRESULT result=sapi_dictionary_download_report(&update);LPARAM info=0;
    if(SUCCEEDED(result)&&update.known){size_t count=update.compared?update.added:update.total;
        size_t files=update.compared?update.added_files:update.total_files;
        /* Three 8-MiB volumes cannot contain 2^24 distinct entries. */
        info=(LPARAM)(UINT_PTR)sapi_dictionary_pack_counts(count,files,update.compared);}
    PostMessageW(window,DICTIONARY_DONE,(WPARAM)result,info);
    InterlockedDecrement(&sapi_objects);FreeLibraryAndExitThread(module,0);
    return 0;
}
struct dialog {struct sapi_settings s;int dialect,loading;BOOL shared;const struct sapi_ui *ui;};
#define T(key) (d->ui->text[UI_##key])
static void start_dictionary(HWND window,struct dialog *d)
{
    struct dictionary_task *task=malloc(sizeof *task);if(!task){SetDlgItemTextW(window,STATUS,T(DICT_FAILED));return;}
    task->window=window;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,(LPCWSTR)(const void *)&dictionary_worker,&task->module)) {
        free(task);SetDlgItemTextW(window,STATUS,T(DICT_FAILED));return;
    }
    InterlockedIncrement(&sapi_objects);
    HANDLE thread=CreateThread(NULL,0,dictionary_worker,task,0,NULL);
    if(!thread){InterlockedDecrement(&sapi_objects);FreeLibrary(task->module);free(task);SetDlgItemTextW(window,STATUS,T(DICT_FAILED));return;}
    CloseHandle(thread);EnableWindow(GetDlgItem(window,DICT_SOURCE),FALSE);EnableWindow(GetDlgItem(window,DICT_DOWNLOAD),FALSE);
    SetDlgItemTextW(window,STATUS,T(DICT_DOWNLOADING));
}
static int system_dialect(void)
{
    WORD language=GetUserDefaultUILanguage();
    for(size_t i=0;i<sapi_language_count();++i)if(sapi_language_at(i)->lcid==language)return sapi_language_at(i)->dialect;
    if(PRIMARYLANGID(language)==LANG_ENGLISH)return 0x10000;
    for(size_t i=0;i<sapi_language_count();++i)if(PRIMARYLANGID(sapi_language_at(i)->lcid)==PRIMARYLANGID(language))return sapi_language_at(i)->dialect;
    return 0x10000;
}
static HWND control(HWND parent,const WCHAR *type,const WCHAR *text,DWORD style,int id,int x,int y,int w,int h)
{
    HWND c=CreateWindowExW(!wcscmp(type,L"EDIT")?WS_EX_CLIENTEDGE:0,type,text,
        WS_CHILD|WS_VISIBLE|style,x,y,w,h,parent,(HMENU)(INT_PTR)id,sapi_module,NULL);
    if(c)SendMessageW(c,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);return c;
}
static void label(HWND p,const WCHAR *text,int x,int y,int w)
{control(p,L"STATIC",text,0,-1,x,y+4,w,20);}
static void combo(HWND p,int id,const WCHAR *title,int y)
{label(p,title,12,y,190);control(p,L"COMBOBOX",L"",WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,id,210,y,398,240);}
static void add(HWND p,int id,const WCHAR *text)
{SendDlgItemMessageW(p,id,CB_ADDSTRING,0,(LPARAM)text);}
static void number(HWND p,int id,const WCHAR *title,int y)
{
    label(p,title,12,y,235);HWND e=control(p,L"EDIT",L"",WS_TABSTOP|ES_NUMBER|ES_AUTOHSCROLL,id,260,y,80,24);
    HWND spin=control(p,UPDOWN_CLASSW,L"",UDS_ALIGNRIGHT|UDS_SETBUDDYINT|UDS_ARROWKEYS|UDS_NOTHOUSANDS,id+200,0,0,0,0);
    SendMessageW(spin,UDM_SETBUDDY,(WPARAM)e,0);SendMessageW(spin,UDM_SETRANGE32,0,100);
}
static void check(HWND p,int id,const WCHAR *text,int y)
{control(p,L"BUTTON",text,WS_TABSTOP|BS_AUTOCHECKBOX,id,12,y,596,23);}
static void pitch_hertz(HWND p,const struct dialog *d,int y)
{
    control(p,L"BUTTON",T(PITCH_HZ_MODE),WS_TABSTOP|BS_AUTOCHECKBOX,PITCH_HZ_MODE,350,y,138,24);
    label(p,T(PITCH_HZ),490,y,27);
    HWND e=control(p,L"EDIT",L"",WS_TABSTOP|ES_NUMBER|ES_AUTOHSCROLL,PITCH_HZ_VALUE,520,y,88,24);
    HWND spin=control(p,UPDOWN_CLASSW,L"",UDS_ALIGNRIGHT|UDS_SETBUDDYINT|UDS_ARROWKEYS|UDS_NOTHOUSANDS,
        PITCH_HZ_VALUE+200,0,0,0,0);
    SendMessageW(spin,UDM_SETBUDDY,(WPARAM)e,0);SendMessageW(spin,UDM_SETRANGE32,1,24000);
}
static void enable_pitch_controls(HWND p,const struct dialog *d)
{
    BOOL hz=d->s.pitch_hz_enabled!=0;
    EnableWindow(GetDlgItem(p,PITCH),!hz);EnableWindow(GetDlgItem(p,PITCH+200),!hz);
    EnableWindow(GetDlgItem(p,PITCH_HZ_VALUE),hz);EnableWindow(GetDlgItem(p,PITCH_HZ_VALUE+200),hz);
}
static void populate(HWND window,struct dialog *d)
{
    d->loading=1;
    SendDlgItemMessageW(window,LANGUAGE,CB_SETCURSEL,sapi_language_index(d->dialect),0);
    SendDlgItemMessageW(window,PROFILE,CB_SETCURSEL,d->s.profile,0);
    SendDlgItemMessageW(window,GENDER,CB_SETCURSEL,d->s.gender,0);
    SendDlgItemMessageW(window,RATE,CB_SETCURSEL,d->s.rate,0);
    SendDlgItemMessageW(window,PAUSES,CB_SETCURSEL,d->s.pauses,0);
    SendDlgItemMessageW(window,DICT_SOURCE,CB_SETCURSEL,sapi_dictionary_source(),0);
    const DWORD values[]={d->s.pitch,d->s.inflection,d->s.head,d->s.roughness,d->s.breathiness};
    for(int i=0;i<5;++i)SetDlgItemInt(window,PITCH+i,values[i],FALSE);
    SetDlgItemInt(window,PITCH_HZ_VALUE,d->s.pitch_hz,FALSE);
    CheckDlgButton(window,PITCH_HZ_MODE,d->s.pitch_hz_enabled?BST_CHECKED:BST_UNCHECKED);
    enable_pitch_controls(window,d);
    const DWORD checks[]={d->s.tags,d->s.abbreviations,d->s.prediction,d->s.dictionaries,d->s.boost};
    for(int i=0;i<5;++i)CheckDlgButton(window,TAGS+i,checks[i]?BST_CHECKED:BST_UNCHECKED);
    EnableWindow(GetDlgItem(window,BOOST),!d->s.speed_override);
    CheckDlgButton(window,ORIGINAL_QUESTIONS,d->s.original_questions?BST_CHECKED:BST_UNCHECKED);
    CheckDlgButton(window,MELLOW_MODE,d->s.mellow_mode?BST_CHECKED:BST_UNCHECKED);
    CheckDlgButton(window,SHARED,d->shared?BST_CHECKED:BST_UNCHECKED);
    SetDlgItemTextW(window,RESET,d->shared?T(RESET_ALL):T(RESET));
    SetDlgItemTextW(window,DIRECTORY,d->s.directory);d->loading=0;
}
static void persist(HWND w,struct dialog *d)
{
    HRESULT hr=sapi_settings_write(d->dialect,&d->s);
    SetDlgItemTextW(w,STATUS,FAILED(hr)?T(ERROR):T(ACTIVE));
}
static void commit_pitch_hertz(HWND w,struct dialog *d)
{
    BOOL translated=FALSE;UINT value=GetDlgItemInt(w,PITCH_HZ_VALUE,&translated,FALSE);
    if(translated&&value>=1&&value<=24000){
        if(value!=d->s.pitch_hz){d->s.pitch_hz=value;persist(w,d);}}
    else {d->loading=1;SetDlgItemInt(w,PITCH_HZ_VALUE,d->s.pitch_hz,FALSE);d->loading=0;
        SetDlgItemTextW(w,STATUS,T(ERROR));}
}
static void override_enable(HWND w,const struct dialog *d)
{
    BOOL speed=d->s.speed_override!=0,pitch=d->s.pitch_override!=0;
    EnableWindow(GetDlgItem(w,OWN_RATE_ECI),speed);
    EnableWindow(GetDlgItem(w,OWN_RATE_WPM),speed);
    EnableWindow(GetDlgItem(w,OWN_RATE_VALUE),speed&&!d->s.speed_wpm_enabled);
    EnableWindow(GetDlgItem(w,OWN_RATE_VALUE+200),speed&&!d->s.speed_wpm_enabled);
    EnableWindow(GetDlgItem(w,OWN_RATE_WPM_VALUE),speed&&d->s.speed_wpm_enabled);
    EnableWindow(GetDlgItem(w,OWN_RATE_WPM_VALUE+200),speed&&d->s.speed_wpm_enabled);
    EnableWindow(GetDlgItem(w,OWN_PITCH_ECI),pitch);
    EnableWindow(GetDlgItem(w,OWN_PITCH_HZ),pitch);
    EnableWindow(GetDlgItem(w,OWN_PITCH_VALUE),pitch&&!d->s.pitch_hz_enabled);
    EnableWindow(GetDlgItem(w,OWN_PITCH_VALUE+200),pitch&&!d->s.pitch_hz_enabled);
    EnableWindow(GetDlgItem(w,OWN_PITCH_HZ_VALUE),pitch&&d->s.pitch_hz_enabled);
    EnableWindow(GetDlgItem(w,OWN_PITCH_HZ_VALUE+200),pitch&&d->s.pitch_hz_enabled);
    EnableWindow(GetDlgItem(w,OWN_VOLUME_VALUE),d->s.volume_override);
    EnableWindow(GetDlgItem(w,OWN_VOLUME_VALUE+200),d->s.volume_override);
}
static void override_number(HWND w,struct dialog *d,int id)
{
    DWORD *field=id==OWN_RATE_VALUE?&d->s.speed_value:id==OWN_RATE_WPM_VALUE?&d->s.speed_wpm:
        id==OWN_PITCH_VALUE?&d->s.pitch:id==OWN_PITCH_HZ_VALUE?&d->s.pitch_hz:&d->s.volume_value;
    UINT maximum=id==OWN_RATE_VALUE?250:id==OWN_RATE_WPM_VALUE?6000:id==OWN_PITCH_HZ_VALUE?24000:100;
    UINT minimum=id==OWN_RATE_WPM_VALUE||id==OWN_PITCH_HZ_VALUE?1:0;BOOL translated=FALSE;
    UINT value=GetDlgItemInt(w,id,&translated,FALSE);
    if(translated&&value>=minimum&&value<=maximum){if(value!=*field){*field=value;persist(GetParent(w),d);}}
    else {d->loading=1;SetDlgItemInt(w,id,*field,FALSE);d->loading=0;}
}
static void override_scale(HWND w,const struct dialog *d,int radio,int edit,const WCHAR *text,
    int y,BOOL first,int minimum,int maximum)
{
    control(w,L"BUTTON",text,WS_TABSTOP|BS_AUTORADIOBUTTON|(first?WS_GROUP:0),radio,28,y,270,24);
    label(w,T(OVERRIDE_VALUE),310,y,70);
    HWND e=control(w,L"EDIT",L"",WS_TABSTOP|ES_NUMBER|ES_AUTOHSCROLL,edit,390,y,100,24);
    HWND spin=control(w,UPDOWN_CLASSW,L"",UDS_ALIGNRIGHT|UDS_SETBUDDYINT|UDS_ARROWKEYS|UDS_NOTHOUSANDS,
        edit+200,0,0,0,0);
    SendMessageW(spin,UDM_SETBUDDY,(WPARAM)e,0);SendMessageW(spin,UDM_SETRANGE32,minimum,maximum);
}
static INT_PTR CALLBACK override_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    struct dialog *d=(struct dialog *)GetWindowLongPtrW(w,DWLP_USER);
    if(message==WM_INITDIALOG){
        d=(struct dialog *)lp;SetWindowLongPtrW(w,DWLP_USER,(LONG_PTR)d);
        /* Up-down buddy creation can emit EN_CHANGE immediately. Suppress
         * persistence until every control has received its stored value. */
        d->loading=1;
        SetWindowTextW(w,T(OVERRIDES_TITLE));
        RECT frame={0,0,620,315};AdjustWindowRectEx(&frame,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),FALSE,0);
        SetWindowPos(w,NULL,0,0,frame.right-frame.left,frame.bottom-frame.top,SWP_NOMOVE|SWP_NOZORDER);
        check(w,OWN_RATE,T(OWN_RATE),8);
        override_scale(w,d,OWN_RATE_ECI,OWN_RATE_VALUE,T(OWN_RATE_VALUE),38,TRUE,0,250);
        override_scale(w,d,OWN_RATE_WPM,OWN_RATE_WPM_VALUE,T(OWN_RATE_WPM),68,FALSE,1,6000);
        control(w,L"BUTTON",T(OWN_PITCH),WS_TABSTOP|WS_GROUP|BS_AUTOCHECKBOX,OWN_PITCH,12,102,596,23);
        override_scale(w,d,OWN_PITCH_ECI,OWN_PITCH_VALUE,T(OWN_PITCH_ECI),132,TRUE,0,100);
        override_scale(w,d,OWN_PITCH_HZ,OWN_PITCH_HZ_VALUE,T(OWN_PITCH_HZ),162,FALSE,1,24000);
        control(w,L"BUTTON",T(OWN_VOLUME),WS_TABSTOP|WS_GROUP|BS_AUTOCHECKBOX,OWN_VOLUME,12,196,596,23);
        number(w,OWN_VOLUME_VALUE,T(OWN_VOLUME_VALUE),226);
        control(w,L"BUTTON",T(CLOSE),WS_TABSTOP|BS_DEFPUSHBUTTON,IDOK,498,272,110,28);
        d->loading=1;
        CheckDlgButton(w,OWN_RATE,d->s.speed_override?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(w,OWN_PITCH,d->s.pitch_override?BST_CHECKED:BST_UNCHECKED);
        CheckDlgButton(w,OWN_VOLUME,d->s.volume_override?BST_CHECKED:BST_UNCHECKED);
        CheckRadioButton(w,OWN_RATE_ECI,OWN_RATE_WPM,d->s.speed_wpm_enabled?OWN_RATE_WPM:OWN_RATE_ECI);
        CheckRadioButton(w,OWN_PITCH_ECI,OWN_PITCH_HZ,d->s.pitch_hz_enabled?OWN_PITCH_HZ:OWN_PITCH_ECI);
        SetDlgItemInt(w,OWN_RATE_VALUE,d->s.speed_value,FALSE);
        SetDlgItemInt(w,OWN_RATE_WPM_VALUE,d->s.speed_wpm,FALSE);
        SetDlgItemInt(w,OWN_PITCH_VALUE,d->s.pitch,FALSE);
        SetDlgItemInt(w,OWN_PITCH_HZ_VALUE,d->s.pitch_hz,FALSE);
        SetDlgItemInt(w,OWN_VOLUME_VALUE,d->s.volume_value,FALSE);
        d->loading=0;override_enable(w,d);SetFocus(GetDlgItem(w,OWN_RATE));return FALSE;
    }
    if(!d)return FALSE;
    if(message==WM_CLOSE||(message==WM_COMMAND&&(LOWORD(wp)==IDOK||LOWORD(wp)==IDCANCEL))){
        const int edits[]={OWN_RATE_VALUE,OWN_RATE_WPM_VALUE,OWN_PITCH_VALUE,OWN_PITCH_HZ_VALUE,OWN_VOLUME_VALUE};
        for(size_t i=0;i<sizeof edits/sizeof *edits;++i)override_number(w,d,edits[i]);
        EndDialog(w,IDOK);return TRUE;
    }
    if(message!=WM_COMMAND||d->loading)return FALSE;
    int id=LOWORD(wp),notification=HIWORD(wp);
    if(notification==BN_CLICKED&&(id==OWN_RATE||id==OWN_PITCH||id==OWN_VOLUME)){
        DWORD *field=id==OWN_RATE?&d->s.speed_override:id==OWN_PITCH?&d->s.pitch_override:&d->s.volume_override;
        *field=IsDlgButtonChecked(w,id)==BST_CHECKED;override_enable(w,d);persist(GetParent(w),d);return TRUE;
    }
    if(notification==BN_CLICKED&&(id==OWN_RATE_ECI||id==OWN_RATE_WPM||id==OWN_PITCH_ECI||id==OWN_PITCH_HZ)){
        if(id==OWN_RATE_ECI||id==OWN_RATE_WPM)d->s.speed_wpm_enabled=id==OWN_RATE_WPM;
        else d->s.pitch_hz_enabled=id==OWN_PITCH_HZ;
        override_enable(w,d);persist(GetParent(w),d);return TRUE;
    }
    int edit=id==OWN_RATE_VALUE||id==OWN_RATE_WPM_VALUE||id==OWN_PITCH_VALUE||id==OWN_PITCH_HZ_VALUE||id==OWN_VOLUME_VALUE;
    if(edit&&notification==EN_KILLFOCUS){override_number(w,d,id);return TRUE;}
    if(edit&&notification==EN_CHANGE){
        BOOL translated=FALSE;UINT value=GetDlgItemInt(w,id,&translated,FALSE);
        UINT maximum=id==OWN_RATE_VALUE?250:id==OWN_RATE_WPM_VALUE?6000:id==OWN_PITCH_HZ_VALUE?24000:100;
        UINT minimum=id==OWN_RATE_WPM_VALUE||id==OWN_PITCH_HZ_VALUE?1:0;
        if(translated&&value>=minimum&&value<=maximum)override_number(w,d,id);
        return TRUE;
    }
    return FALSE;
}
static void show_overrides(HWND parent,struct dialog *d)
{
    struct {DLGTEMPLATE dialog;WORD menu,window_class;WCHAR title[32];} templ={
        .dialog={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_CENTER,.cx=335,.cy=150},
        .title=L"ReLoq C"};
    INT_PTR result=DialogBoxIndirectParamW(sapi_module,&templ.dialog,parent,override_proc,(LPARAM)d);
    populate(parent,d);
    if(result==-1)SetDlgItemTextW(parent,STATUS,T(ERROR));
}
static INT_PTR CALLBACK dialog_proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    struct dialog *d=(struct dialog *)GetWindowLongPtrW(w,DWLP_USER);
    if(message==WM_INITDIALOG){d=(struct dialog *)lp;SetWindowLongPtrW(w,DWLP_USER,(LONG_PTR)d);
        SetWindowTextW(w,T(TITLE));
        RECT frame={0,0,620,722};AdjustWindowRectEx(&frame,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),FALSE,0);
        SetWindowPos(w,NULL,0,0,frame.right-frame.left,frame.bottom-frame.top,SWP_NOMOVE|SWP_NOZORDER);
        combo(w,LANGUAGE,T(LANGUAGE),8);combo(w,PROFILE,T(PROFILE),38);
        combo(w,GENDER,T(MODEL),68);combo(w,RATE,T(RATE),98);
        check(w,MELLOW_MODE,T(MELLOW_MODE),128);combo(w,PAUSES,T(PAUSES),151);
        for(size_t i=0;i<sapi_language_count();++i){WCHAR name[128];const struct sapi_language *language=sapi_language_at(i);
            if(!GetLocaleInfoW(language->lcid,LOCALE_SLOCALIZEDDISPLAYNAME,name,128))wcscpy(name,language->name);
            add(w,LANGUAGE,name);}
        for(unsigned i=0;i<8;++i)add(w,PROFILE,sapi_profile_name(i));
        add(w,GENDER,T(MALE));add(w,GENDER,T(FEMALE));
        for(unsigned i=0;i<7;++i){WCHAR text[32];swprintf(text,32,L"%d Hz",sapi_sample_rate(i));add(w,RATE,text);}
        add(w,PAUSES,T(NEVER));add(w,PAUSES,T(FINAL));add(w,PAUSES,T(ALWAYS));
        check(w,SHARED,T(SHARED),183);
        number(w,PITCH,T(PITCH),211);pitch_hertz(w,d,211);number(w,INFLECTION,T(INFLECTION),238);
        control(w,L"BUTTON",T(OVERRIDES),WS_TABSTOP|BS_PUSHBUTTON,OVERRIDES,350,238,258,24);
        number(w,HEAD,T(HEAD),265);number(w,ROUGH,T(ROUGH),292);number(w,BREATH,T(BREATH),319);
        check(w,TAGS,T(TAGS),349);check(w,ABBREVIATIONS,T(ABBREVIATIONS),372);
        check(w,PREDICTION,T(PREDICTION),395);check(w,ORIGINAL_QUESTIONS,T(ORIGINAL_QUESTIONS),418);
        check(w,DICTIONARIES,T(DICTIONARIES),441);check(w,BOOST,T(BOOST),464);
        combo(w,DICT_SOURCE,T(DICT_SOURCE),489);
        add(w,DICT_SOURCE,T(DICT_COMMUNITY));add(w,DICT_SOURCE,T(DICT_ALTERNATIVE));
        control(w,L"BUTTON",T(DICT_DOWNLOAD),WS_TABSTOP|BS_PUSHBUTTON,DICT_DOWNLOAD,210,519,398,24);
        label(w,T(DIRECTORY),12,547,596);
        control(w,L"EDIT",L"",WS_TABSTOP|ES_AUTOHSCROLL,DIRECTORY,12,572,440,24);
        control(w,L"BUTTON",T(OPEN),WS_TABSTOP|BS_PUSHBUTTON,OPEN_FOLDER,462,572,146,24);
        label(w,T(DICT_OVERRIDE),12,599,596);
        control(w,L"STATIC",T(RATE_HINT),0,-1,12,619,596,36);
        control(w,L"STATIC",L"",0,STATUS,12,655,596,24);
        control(w,L"BUTTON",T(RESET),WS_TABSTOP|BS_PUSHBUTTON,RESET,12,685,360,28);
        control(w,L"BUTTON",T(CLOSE),WS_TABSTOP|BS_DEFPUSHBUTTON,IDOK,498,685,110,28);
        populate(w,d);SetDlgItemTextW(w,STATUS,T(ACTIVE));
        SetFocus(GetDlgItem(w,LANGUAGE));return FALSE;
    }
    if(!d)return FALSE;
    if(message==WM_CLOSE){if(d->s.pitch_hz_enabled)commit_pitch_hertz(w,d);EndDialog(w,IDOK);return TRUE;}
    if(message==DICTIONARY_DONE){
        EnableWindow(GetDlgItem(w,DICT_SOURCE),TRUE);EnableWindow(GetDlgItem(w,DICT_DOWNLOAD),TRUE);
        populate(w,d);SetDlgItemTextW(w,STATUS,FAILED((HRESULT)wp)?T(DICT_FAILED):T(ACTIVE));
        if(SUCCEEDED((HRESULT)wp)) {
            WCHAR message[512],entry_text[256],file_text[256],number[32];size_t count,files;int compared;
            if(!sapi_dictionary_unpack_counts((uint32_t)(UINT_PTR)lp,&count,&files,&compared))
                swprintf(message,512,L"%ls\n%ls",T(DICT_COUNT_UNKNOWN),T(DICT_FILES_UNKNOWN));
            else {
                swprintf(number,32,L"%lu",(unsigned long)count);
                swprintf(entry_text,256,compared?T(DICT_UPDATED_COUNT):T(DICT_FIRST_COUNT),number);
                swprintf(number,32,L"%lu",(unsigned long)files);
                swprintf(file_text,256,compared?T(DICT_FILES_NEW):T(DICT_FILES_TOTAL),number);
                swprintf(message,512,L"%ls\n%ls",entry_text,file_text);}
            MessageBoxW(w,message,T(TITLE),MB_OK|MB_ICONINFORMATION);
        }
        return TRUE;
    }
    if(message!=WM_COMMAND)return FALSE;
    int id=LOWORD(wp),notification=HIWORD(wp);
    if(id==IDOK||id==IDCANCEL){if(d->s.pitch_hz_enabled)commit_pitch_hertz(w,d);EndDialog(w,id);return TRUE;}
    if(d->loading)return TRUE;
    if(id==LANGUAGE&&notification==CBN_SELCHANGE){int i=(int)SendDlgItemMessageW(w,id,CB_GETCURSEL,0,0);
        const struct sapi_language *language=sapi_language_at((size_t)i);if(!language)return TRUE;
        d->dialect=language->dialect;HRESULT hr=sapi_settings_read(d->dialect,&d->s);
        if(FAILED(hr))sapi_settings_default(&d->s);populate(w,d);return TRUE;}
    if(id==DICT_SOURCE&&notification==CBN_SELCHANGE){
        DWORD choice=(DWORD)SendDlgItemMessageW(w,id,CB_GETCURSEL,0,0);
        if(choice==1&&!sapi_alternative_dictionary_available())start_dictionary(w,d);
        else {HRESULT result=sapi_dictionary_choose(choice);populate(w,d);SetDlgItemTextW(w,STATUS,FAILED(result)?T(ERROR):T(ACTIVE));}
        return TRUE;
    }
    if(id==DICT_DOWNLOAD&&notification==BN_CLICKED){start_dictionary(w,d);return TRUE;}
    if(id==OVERRIDES&&notification==BN_CLICKED){show_overrides(w,d);return TRUE;}
    if(id==OPEN_FOLDER){WCHAR path[2048];
        if(d->s.directory[0])wcscpy(path,d->s.directory);else if(!sapi_default_dictionary_path(path,2048))return TRUE;
        ShellExecuteW(w,L"open",path,NULL,NULL,SW_SHOWNORMAL);return TRUE;}
    if(id==SHARED&&notification==BN_CLICKED){
        HRESULT hr=sapi_settings_set_shared(d->dialect,IsDlgButtonChecked(w,SHARED)==BST_CHECKED);
        if(SUCCEEDED(hr))hr=sapi_settings_shared(&d->shared);
        if(SUCCEEDED(hr))hr=sapi_settings_read(d->dialect,&d->s);
        populate(w,d);SetDlgItemTextW(w,STATUS,FAILED(hr)?T(ERROR):T(ACTIVE));return TRUE;
    }
    if(id==RESET){DWORD rate=d->s.rate;sapi_settings_default(&d->s);d->s.rate=rate;populate(w,d);persist(w,d);return TRUE;}
    if(notification==BN_CLICKED&&id==PITCH_HZ_MODE){
        d->s.pitch_hz_enabled=IsDlgButtonChecked(w,id)==BST_CHECKED;
        enable_pitch_controls(w,d);persist(w,d);return TRUE;}
    if(notification==EN_KILLFOCUS&&id==PITCH_HZ_VALUE){
        commit_pitch_hertz(w,d);
        return TRUE;}
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
    if(notification==BN_CLICKED&&id==ORIGINAL_QUESTIONS){
        d->s.original_questions=IsDlgButtonChecked(w,id)==BST_CHECKED;persist(w,d);return TRUE;}
    if(notification==BN_CLICKED&&id==MELLOW_MODE){
        d->s.mellow_mode=IsDlgButtonChecked(w,id)==BST_CHECKED;persist(w,d);return TRUE;}
    if(id==DIRECTORY&&notification==EN_CHANGE){WCHAR path[1024];GetDlgItemTextW(w,id,path,1024);
        DWORD attributes=path[0]?GetFileAttributesW(path):FILE_ATTRIBUTE_DIRECTORY;
        if(attributes==INVALID_FILE_ATTRIBUTES||!(attributes&FILE_ATTRIBUTE_DIRECTORY)){
            SetDlgItemTextW(w,STATUS,T(ERROR));return TRUE;}
        wcscpy(d->s.directory,path);persist(w,d);return TRUE;}
    return FALSE;
}
HRESULT sapi_settings_dialog(HWND parent,int dialect)
{
    struct dialog d={.dialect=dialect,.ui=sapi_ui_for_language(GetUserDefaultUILanguage())};
    if(sapi_language_index(dialect)<0)d.dialect=system_dialect();
    if(FAILED(sapi_settings_shared(&d.shared)))d.shared=FALSE;
    if(FAILED(sapi_settings_read(d.dialect,&d.s)))sapi_settings_default(&d.s);
    INITCOMMONCONTROLSEX common={sizeof common,ICC_UPDOWN_CLASS};InitCommonControlsEx(&common);
    /* Empty standard dialog template; all controls are ordinary Win32. */
    struct {DLGTEMPLATE dialog;WORD menu,window_class;WCHAR title[32];} templ={
        .dialog={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_CENTER,.cx=335,.cy=430},
        .title=L"ReLoq C voice settings"};
    /* Controls use pixels, so size the client area independently of dialog
     * units. The initial WM_INITDIALOG layout fits a 620 × 722 client area. */
    INT_PTR result=DialogBoxIndirectParamW(sapi_module,&templ.dialog,parent,dialog_proc,(LPARAM)&d);
    return result==-1?HRESULT_FROM_WIN32(GetLastError()):S_OK;
}
