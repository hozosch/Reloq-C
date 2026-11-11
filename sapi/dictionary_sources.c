/* Ordered personal ECI files, saved in the current user's profile. */
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static const WCHAR registry[]=L"Software\\ReLoq C\\SAPI5";
static int valid(const struct sapi_dictionary_file *file)
{
    if(sapi_language_index(file->dialect)<0||file->volume>2||file->format>SAPI_DICT_NVDA||file->voice>8||!file->path[0]||
       !wmemchr(file->path,0,sizeof file->path/sizeof *file->path))return 0;
    if(wcschr(file->path,L'\t'))return 0;
    return (file->path[0]&&file->path[1]==L':'&&(file->path[2]==L'\\'||file->path[2]==L'/'))||
           (file->path[0]==L'\\'&&file->path[1]==L'\\');
}
HRESULT sapi_dictionary_files_load(struct sapi_dictionary_file *files,size_t capacity,size_t *count)
{
    if(!count||(!files&&capacity))return E_INVALIDARG;*count=0;
    DWORD bytes=0,type=0;LSTATUS rc=RegGetValueW(HKEY_CURRENT_USER,registry,L"DictionaryFiles",
        RRF_RT_REG_MULTI_SZ,&type,NULL,&bytes);
    if(rc==ERROR_FILE_NOT_FOUND||rc==ERROR_PATH_NOT_FOUND)return S_OK;
    if(rc!=ERROR_SUCCESS)return HRESULT_FROM_WIN32(rc);
    if(bytes<2*sizeof(WCHAR)||bytes>(SAPI_DICTIONARY_FILES_MAX*(1024+20)+1)*sizeof(WCHAR)||bytes%sizeof(WCHAR))
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    WCHAR *data=calloc(1,(size_t)bytes+sizeof(WCHAR));if(!data)return E_OUTOFMEMORY;
    rc=RegGetValueW(HKEY_CURRENT_USER,registry,L"DictionaryFiles",RRF_RT_REG_MULTI_SZ,&type,data,&bytes);
    if(rc!=ERROR_SUCCESS){free(data);return HRESULT_FROM_WIN32(rc);}
    size_t end=bytes/sizeof(WCHAR),at=0;HRESULT result=S_OK;
    while(at<end&&data[at]){
        size_t left=end-at,len=0;while(len<left&&data[at+len])++len;
        if(len==left||len<11||data[at+8]!=L'\t'||data[at+10]!=L'\t'||data[at+9]<L'0'||data[at+9]>L'2'||
           *count>=capacity){result=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);break;}
        unsigned dialect=0;for(int i=0;i<8;++i){WCHAR c=data[at+i];unsigned digit;
            if(c>=L'0'&&c<=L'9')digit=(unsigned)(c-L'0');
            else if(c>=L'A'&&c<=L'F')digit=(unsigned)(c-L'A'+10);
            else if(c>=L'a'&&c<=L'f')digit=(unsigned)(c-L'a'+10);
            else {result=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);break;}
            dialect=(dialect<<4)|digit;}
        if(FAILED(result))break;
        struct sapi_dictionary_file *file=files+*count;memset(file,0,sizeof *file);
        file->dialect=(int)dialect;file->volume=(DWORD)(data[at+9]-L'0');
        WCHAR *path=data+at+11,*suffix=wcschr(path,L'\t');
        size_t path_length=suffix?(size_t)(suffix-path):wcslen(path);
        if(path_length>=1024){result=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);break;}
        if(suffix){
            if(suffix[1]<L'0'||suffix[1]>L'2'||suffix[2]!=L'\t'||suffix[3]<L'0'||suffix[3]>L'8'||suffix[4]){
                result=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);break;}
            file->format=(DWORD)(suffix[1]-L'0');file->voice=(DWORD)(suffix[3]-L'0');
        }
        wmemcpy(file->path,path,path_length);file->path[path_length]=0;
        if(!valid(file)){result=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);break;}
        ++*count;at+=len+1;
    }
    free(data);return result;
}
HRESULT sapi_dictionary_files_save(const struct sapi_dictionary_file *files,size_t count)
{
    if((!files&&count)||count>SAPI_DICTIONARY_FILES_MAX)return E_INVALIDARG;
    size_t chars=1;for(size_t i=0;i<count;++i){if(!valid(files+i))return E_INVALIDARG;
        chars+=12+wcslen(files[i].path)+((files[i].format||files[i].voice)?4:0);}
    if(chars<2)chars=2;
    WCHAR *data=calloc(chars,sizeof *data);if(!data)return E_OUTOFMEMORY;
    size_t at=0;for(size_t i=0;i<count;++i){int n;
        if(files[i].format==SAPI_DICT_AUTO&&!files[i].voice)
            n=swprintf(data+at,chars-at,L"%08X\t%lu\t%ls",(unsigned)files[i].dialect,
                (unsigned long)files[i].volume,files[i].path);
        else n=swprintf(data+at,chars-at,L"%08X\t%lu\t%ls\t%lu\t%lu",
            (unsigned)files[i].dialect,(unsigned long)files[i].volume,files[i].path,
            (unsigned long)files[i].format,(unsigned long)files[i].voice);
        if(n<0){free(data);return E_INVALIDARG;}at+=(size_t)n+1;}
    HKEY key;LSTATUS rc=RegCreateKeyExW(HKEY_CURRENT_USER,registry,0,NULL,0,KEY_SET_VALUE,NULL,&key,NULL);
    if(rc==ERROR_SUCCESS){rc=RegSetValueExW(key,L"DictionaryFiles",0,REG_MULTI_SZ,(const BYTE *)data,(DWORD)(chars*sizeof *data));RegCloseKey(key);}
    free(data);return rc==ERROR_SUCCESS?S_OK:HRESULT_FROM_WIN32(rc);
}
