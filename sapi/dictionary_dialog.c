/* Accessible ordered personal dictionary selection for SAPI. */
#include "settings.h"
#include <commdlg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "ui.inc"
#include "dictionary_hint.h"

enum {FILES=600,ADD_FILE,REMOVE_FILE,MOVE_UP,MOVE_DOWN,FILE_LANGUAGE,FILE_VOLUME,FILE_FORMAT,FILE_VOICE};
struct dialog {
    struct sapi_dictionary_file files[SAPI_DICTIONARY_FILES_MAX];
    unsigned char nvda_hints[SAPI_DICTIONARY_FILES_MAX];
    size_t count;int dialect;const struct sapi_settings *settings;const struct sapi_ui *ui;
};
struct choice {struct sapi_dictionary_file file;const struct sapi_ui *ui;};
#define T(d,key) ((d)->ui->text[UI_##key])
static HWND control(HWND parent,const WCHAR *type,const WCHAR *text,DWORD style,int id,int x,int y,int width,int height)
{
    HWND c=CreateWindowExW(0,type,text,WS_CHILD|WS_VISIBLE|style,x,y,width,height,parent,
                           (HMENU)(INT_PTR)id,sapi_module,NULL);
    if(c)SendMessageW(c,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);return c;
}
static void size_window(HWND window,int width,int height)
{
    RECT frame={0,0,width,height};AdjustWindowRectEx(&frame,(DWORD)GetWindowLongPtrW(window,GWL_STYLE),FALSE,0);
    SetWindowPos(window,NULL,0,0,frame.right-frame.left,frame.bottom-frame.top,SWP_NOMOVE|SWP_NOZORDER);
}
static INT_PTR CALLBACK choice_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    struct choice *d=(struct choice *)GetWindowLongPtrW(window,DWLP_USER);
    if(message==WM_INITDIALOG){d=(struct choice *)lp;SetWindowLongPtrW(window,DWLP_USER,(LONG_PTR)d);
        SetWindowTextW(window,T(d,DICT_PICK));size_window(window,510,250);
        control(window,L"STATIC",T(d,LANGUAGE),0,-1,12,14,155,20);
        control(window,L"COMBOBOX",L"",WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,FILE_LANGUAGE,170,10,325,230);
        for(size_t i=0;i<sapi_language_count();++i){const struct sapi_language *language=sapi_language_at(i);WCHAR name[128];
            if(!GetLocaleInfoW(language->lcid,LOCALE_SLOCALIZEDDISPLAYNAME,name,128))wcscpy(name,language->name);
            SendDlgItemMessageW(window,FILE_LANGUAGE,CB_ADDSTRING,0,(LPARAM)name);}
        SendDlgItemMessageW(window,FILE_LANGUAGE,CB_SETCURSEL,sapi_language_index(d->file.dialect),0);
        control(window,L"STATIC",T(d,DICT_FORMAT),0,-1,12,54,155,20);
        control(window,L"COMBOBOX",L"",WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,FILE_FORMAT,170,50,325,180);
        SendDlgItemMessageW(window,FILE_FORMAT,CB_ADDSTRING,0,(LPARAM)T(d,DICT_FORMAT_AUTO));
        SendDlgItemMessageW(window,FILE_FORMAT,CB_ADDSTRING,0,(LPARAM)T(d,DICT_FORMAT_ECI));
        SendDlgItemMessageW(window,FILE_FORMAT,CB_ADDSTRING,0,(LPARAM)T(d,DICT_FORMAT_NVDA));
        SendDlgItemMessageW(window,FILE_FORMAT,CB_SETCURSEL,d->file.format,0);
        control(window,L"STATIC",T(d,DICT_VOLUME),0,-1,12,94,155,20);
        control(window,L"COMBOBOX",L"",WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,FILE_VOLUME,170,90,325,180);
        SendDlgItemMessageW(window,FILE_VOLUME,CB_ADDSTRING,0,(LPARAM)T(d,DICT_VOLUME_MAIN));
        SendDlgItemMessageW(window,FILE_VOLUME,CB_ADDSTRING,0,(LPARAM)T(d,DICT_VOLUME_ROOT));
        SendDlgItemMessageW(window,FILE_VOLUME,CB_ADDSTRING,0,(LPARAM)T(d,DICT_VOLUME_ABBR));
        SendDlgItemMessageW(window,FILE_VOLUME,CB_SETCURSEL,d->file.volume,0);
        control(window,L"STATIC",T(d,DICT_VOICE),0,-1,12,134,155,20);
        control(window,L"COMBOBOX",L"",WS_TABSTOP|WS_VSCROLL|CBS_DROPDOWNLIST,FILE_VOICE,170,130,325,220);
        SendDlgItemMessageW(window,FILE_VOICE,CB_ADDSTRING,0,(LPARAM)T(d,DICT_ALL_VOICES));
        for(unsigned i=0;i<8;++i)SendDlgItemMessageW(window,FILE_VOICE,CB_ADDSTRING,0,(LPARAM)sapi_profile_name(i));
        SendDlgItemMessageW(window,FILE_VOICE,CB_SETCURSEL,d->file.voice,0);
        EnableWindow(GetDlgItem(window,FILE_VOLUME),d->file.format!=SAPI_DICT_NVDA);
        EnableWindow(GetDlgItem(window,FILE_VOICE),d->file.format!=SAPI_DICT_ECI);
        control(window,L"BUTTON",T(d,DICT_OK),WS_TABSTOP|BS_DEFPUSHBUTTON,IDOK,265,200,110,28);
        control(window,L"BUTTON",T(d,DICT_CANCEL),WS_TABSTOP|BS_PUSHBUTTON,IDCANCEL,385,200,110,28);
        SetFocus(GetDlgItem(window,FILE_LANGUAGE));return FALSE;
    }
    if(message==WM_CLOSE){EndDialog(window,IDCANCEL);return TRUE;}
    if(message!=WM_COMMAND||!d)return FALSE;
    if(LOWORD(wp)==FILE_FORMAT&&HIWORD(wp)==CBN_SELCHANGE){
        LRESULT format=SendDlgItemMessageW(window,FILE_FORMAT,CB_GETCURSEL,0,0);
        EnableWindow(GetDlgItem(window,FILE_VOLUME),format!=SAPI_DICT_NVDA);
        EnableWindow(GetDlgItem(window,FILE_VOICE),format!=SAPI_DICT_ECI);
        return TRUE;
    }
    if(LOWORD(wp)==IDCANCEL){EndDialog(window,IDCANCEL);return TRUE;}
    if(LOWORD(wp)==IDOK){
        LRESULT language=SendDlgItemMessageW(window,FILE_LANGUAGE,CB_GETCURSEL,0,0);
        LRESULT volume=SendDlgItemMessageW(window,FILE_VOLUME,CB_GETCURSEL,0,0);
        LRESULT format=SendDlgItemMessageW(window,FILE_FORMAT,CB_GETCURSEL,0,0);
        LRESULT voice=SendDlgItemMessageW(window,FILE_VOICE,CB_GETCURSEL,0,0);
        const struct sapi_language *selected=sapi_language_at((size_t)language);
        if(!selected||volume<0||volume>2||format<0||format>2||voice<0||voice>8)return TRUE;
        d->file.dialect=selected->dialect;d->file.volume=(DWORD)volume;
        d->file.format=(DWORD)format;d->file.voice=(DWORD)voice;EndDialog(window,IDOK);return TRUE;
    }
    return FALSE;
}
static int nvda_hint(const WCHAR *path)
{
    FILE *f=_wfopen(path,L"rb");if(!f)return 0;
    if(fseek(f,0,SEEK_END)){fclose(f);return 0;}
    long size=ftell(f);if(size<0||size>8*1024*1024||fseek(f,0,SEEK_SET)){fclose(f);return 0;}
    unsigned char *data=malloc(size?(size_t)size:1);if(!data){fclose(f);return 0;}
    size_t n=fread(data,1,(size_t)size,f);int valid=n==(size_t)size&&!ferror(f);
    fclose(f);int hint=valid&&sapi_dictionary_looks_nvda(data,n);free(data);return hint;
}
static int select_file(HWND parent,struct dialog *d,struct sapi_dictionary_file *result,unsigned char *hint)
{
    struct choice choice={.file={.dialect=d->dialect,.volume=0},.ui=d->ui};
    WCHAR filter[512]={0},*p=filter;const WCHAR *label=T(d,DICT_PICK),*all=L"All files (*.*)";
    size_t a=wcslen(label),b=wcslen(all);if(a+b+20>=sizeof filter/sizeof *filter)return 0;
    wcscpy(p,label);p+=a+1;wcscpy(p,L"*.dic");p+=6;wcscpy(p,all);p+=b+1;wcscpy(p,L"*.*");
    OPENFILENAMEW open={0};open.lStructSize=sizeof open;open.hwndOwner=parent;open.lpstrFile=choice.file.path;
    open.nMaxFile=sizeof choice.file.path/sizeof *choice.file.path;open.lpstrFilter=filter;open.lpstrTitle=T(d,DICT_PICK);
    open.Flags=OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(!GetOpenFileNameW(&open))return 0;
    *hint=(unsigned char)nvda_hint(choice.file.path);
    if(*hint)choice.file.format=SAPI_DICT_NVDA;
    int dialect=0;unsigned volume=0;
    if(!*hint&&sapi_dictionary_guess(choice.file.path,&dialect,&volume)&&sapi_language_index(dialect)>=0){
        choice.file.dialect=dialect;choice.file.volume=(DWORD)volume;
    }
    struct {DLGTEMPLATE dialog;WORD menu,window_class;WCHAR title[1];} templ={
        .dialog={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_CENTER,.cx=280,.cy=100},.title=L""};
    if(DialogBoxIndirectParamW(sapi_module,&templ.dialog,parent,choice_proc,(LPARAM)&choice)!=IDOK)return 0;
    *result=choice.file;
    *hint=(unsigned char)(choice.file.format==SAPI_DICT_NVDA||
        (choice.file.format==SAPI_DICT_AUTO&&*hint));return 1;
}
static void redraw(HWND window,const struct dialog *d,int selected)
{
    SendDlgItemMessageW(window,FILES,LB_RESETCONTENT,0,0);
    for(size_t i=0;i<d->count;++i){const struct sapi_dictionary_file *file=d->files+i;
        int index=sapi_language_index(file->dialect);const struct sapi_language *language=sapi_language_at((size_t)index);
        const WCHAR *volume=file->volume==0?T(d,DICT_VOLUME_MAIN):file->volume==1?T(d,DICT_VOLUME_ROOT):T(d,DICT_VOLUME_ABBR);
        int nvda=d->nvda_hints[i];
        const WCHAR *scope=nvda?(file->voice?sapi_profile_name(file->voice-1):T(d,DICT_ALL_VOICES)):volume;
        WCHAR text[1600];swprintf(text,1600,L"%ls — %ls — %ls — %ls",nvda?T(d,DICT_FORMAT_NVDA):T(d,DICT_FORMAT_ECI),
            language?language->name:L"",scope,file->path);
        SendDlgItemMessageW(window,FILES,LB_ADDSTRING,0,(LPARAM)text);
    }
    if(d->count&&selected>=0)SendDlgItemMessageW(window,FILES,LB_SETCURSEL,(WPARAM)selected,0);
    BOOL have=d->count&&selected>=0;EnableWindow(GetDlgItem(window,REMOVE_FILE),have);
    EnableWindow(GetDlgItem(window,MOVE_UP),have&&selected>0);
    EnableWindow(GetDlgItem(window,MOVE_DOWN),have&&(size_t)selected+1<d->count);
}
static INT_PTR CALLBACK files_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    struct dialog *d=(struct dialog *)GetWindowLongPtrW(window,DWLP_USER);
    if(message==WM_INITDIALOG){d=(struct dialog *)lp;SetWindowLongPtrW(window,DWLP_USER,(LONG_PTR)d);
        SetWindowTextW(window,T(d,DICT_OWN_TITLE));size_window(window,620,350);
        control(window,L"STATIC",T(d,DICT_OWN_LIST),0,-1,12,10,590,22);
        control(window,L"LISTBOX",L"",WS_TABSTOP|WS_BORDER|WS_VSCROLL|LBS_NOTIFY,FILES,12,36,590,185);
        control(window,L"BUTTON",T(d,DICT_ADD),WS_TABSTOP|BS_PUSHBUTTON,ADD_FILE,12,232,130,28);
        control(window,L"BUTTON",T(d,DICT_REMOVE),WS_TABSTOP|BS_PUSHBUTTON,REMOVE_FILE,152,232,130,28);
        control(window,L"BUTTON",T(d,DICT_UP),WS_TABSTOP|BS_PUSHBUTTON,MOVE_UP,292,232,150,28);
        control(window,L"BUTTON",T(d,DICT_DOWN),WS_TABSTOP|BS_PUSHBUTTON,MOVE_DOWN,452,232,150,28);
        control(window,L"BUTTON",T(d,DICT_OK),WS_TABSTOP|BS_DEFPUSHBUTTON,IDOK,372,305,110,28);
        control(window,L"BUTTON",T(d,DICT_CANCEL),WS_TABSTOP|BS_PUSHBUTTON,IDCANCEL,492,305,110,28);
        redraw(window,d,d->count?0:-1);SetFocus(GetDlgItem(window,d->count?FILES:ADD_FILE));return FALSE;
    }
    if(message==WM_CLOSE){EndDialog(window,IDCANCEL);return TRUE;}
    if(message!=WM_COMMAND||!d)return FALSE;
    int id=LOWORD(wp),notification=HIWORD(wp);
    if(id==FILES&&notification==LBN_SELCHANGE){redraw(window,d,(int)SendDlgItemMessageW(window,FILES,LB_GETCURSEL,0,0));return TRUE;}
    if(id==ADD_FILE&&notification==BN_CLICKED){
        if(d->count==SAPI_DICTIONARY_FILES_MAX){MessageBoxW(window,T(d,DICT_TOO_MANY),T(d,DICT_OWN_TITLE),MB_OK|MB_ICONERROR);return TRUE;}
        struct sapi_dictionary_file item;unsigned char hint=0;if(select_file(window,d,&item,&hint)){
            d->files[d->count]=item;d->nvda_hints[d->count++]=hint;redraw(window,d,(int)d->count-1);}return TRUE;
    }
    if((id==REMOVE_FILE||id==MOVE_UP||id==MOVE_DOWN)&&notification==BN_CLICKED){
        LRESULT selected=SendDlgItemMessageW(window,FILES,LB_GETCURSEL,0,0);if(selected<0||(size_t)selected>=d->count)return TRUE;
        if(id==REMOVE_FILE){memmove(d->files+selected,d->files+selected+1,(d->count-(size_t)selected-1)*sizeof *d->files);--d->count;
            memmove(d->nvda_hints+selected,d->nvda_hints+selected+1,d->count-(size_t)selected);
            if((size_t)selected>=d->count)--selected;}
        else {int other=(int)selected+(id==MOVE_UP?-1:1);if(other<0||(size_t)other>=d->count)return TRUE;
            struct sapi_dictionary_file temp=d->files[selected];d->files[selected]=d->files[other];d->files[other]=temp;
            unsigned char hint=d->nvda_hints[selected];d->nvda_hints[selected]=d->nvda_hints[other];d->nvda_hints[other]=hint;selected=other;}
        redraw(window,d,(int)selected);return TRUE;
    }
    if(id==IDCANCEL){EndDialog(window,IDCANCEL);return TRUE;}
    if(id==IDOK){WCHAR snapshot[2048];
        HRESULT result=sapi_dictionary_prepare(d->settings,d->files,d->count,snapshot,2048);
        if(SUCCEEDED(result))result=sapi_dictionary_files_save(d->files,d->count);
        if(FAILED(result)){MessageBoxW(window,T(d,DICT_IMPORT_ERROR),T(d,DICT_OWN_TITLE),MB_OK|MB_ICONERROR);return TRUE;}
        EndDialog(window,IDOK);return TRUE;
    }
    return FALSE;
}
HRESULT sapi_dictionary_files_dialog(HWND parent,int dialect,const struct sapi_settings *settings)
{
    if(!settings)return E_INVALIDARG;struct dialog *d=calloc(1,sizeof *d);if(!d)return E_OUTOFMEMORY;
    d->dialect=dialect;d->settings=settings;d->ui=sapi_ui_for_language(GetUserDefaultUILanguage());
    HRESULT result=sapi_dictionary_files_load(d->files,SAPI_DICTIONARY_FILES_MAX,&d->count);
    if(SUCCEEDED(result)){
        for(size_t i=0;i<d->count;++i)d->nvda_hints[i]=(unsigned char)(d->files[i].format==SAPI_DICT_NVDA||
            (d->files[i].format==SAPI_DICT_AUTO&&nvda_hint(d->files[i].path)));
        struct {DLGTEMPLATE dialog;WORD menu,window_class;WCHAR title[1];} templ={
            .dialog={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_CENTER,.cx=330,.cy=200},.title=L""};
        INT_PTR choice=DialogBoxIndirectParamW(sapi_module,&templ.dialog,parent,files_proc,(LPARAM)d);
        if(choice==-1)result=HRESULT_FROM_WIN32(GetLastError());
    }
    free(d);return result;
}
