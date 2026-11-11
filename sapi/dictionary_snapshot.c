/* Merge user files with the selected source into per-user immutable snapshots.
 * Source files are never changed. A snapshot is only published after every
 * volume was read successfully and the source metadata is still unchanged. */
#include "settings.h"
#include "dictionary_merge.h"
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define LIMIT (8u*1024u*1024u)
struct volume {int dialect;const WCHAR *prefix;};
static const struct volume volumes[]={
    {0x10000,L"ENU"},{0x10001,L"ENG"},{0x20000,L"ESP"},{0x20001,L"ESM"},
    {0x30000,L"FRA"},{0x30001,L"FRC"},{0x40000,L"DEU"},{0x50000,L"ITA"},
    {0x60000,L"CHS"},{0x70000,L"PTB"},{0x80000,L"JPN"},{0x90000,L"FIN"},
    {0xa0000,L"KOR"},{0x120000,L"ELL"}};
static const WCHAR *const suffixes[]={L"main.dic",L"Root.dic",L"abbr.dic"};
/* Short-lived metadata cache keeps ordinary SAPI announcements responsive;
 * changes are rechecked shortly and the previous complete snapshot remains
 * usable while a personal file is temporarily inaccessible. */
static SRWLOCK cache_lock=SRWLOCK_INIT;
static struct {
    int valid;DWORD enabled,source;ULONGLONG checked;
    WCHAR directory[1024],path[2048];size_t count;
    struct sapi_dictionary_file files[SAPI_DICTIONARY_FILES_MAX];
} cache;
static int mkdirs(WCHAR *path)
{
    for(WCHAR *p=path+3;*p;++p)if(*p==L'\\'){
        *p=0;BOOL ok=CreateDirectoryW(path,NULL);DWORD e=GetLastError();*p=L'\\';
        if(!ok&&e!=ERROR_ALREADY_EXISTS)return 0;}
    return CreateDirectoryW(path,NULL)||GetLastError()==ERROR_ALREADY_EXISTS;
}
static int snapshot_root(WCHAR *path,size_t capacity)
{
    DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",path,(DWORD)capacity);
    const WCHAR suffix[]=L"\\ReLoq C\\SAPI5\\Dictionaries\\Snapshots";
    if(!n||n>=capacity||n+wcslen(suffix)>=capacity)return 0;
    wcscat(path,suffix);return mkdirs(path);
}
static int volume_path(WCHAR *out,size_t capacity,const WCHAR *directory,size_t language,unsigned volume)
{return swprintf(out,capacity,L"%ls\\%ls%ls",directory,volumes[language].prefix,suffixes[volume])>=0;}
static int attributes(HCRYPTHASH hash,const WCHAR *path,int required)
{
    WIN32_FILE_ATTRIBUTE_DATA a={0};BOOL exists=GetFileAttributesExW(path,GetFileExInfoStandard,&a);
    if(!exists){DWORD error=GetLastError();
        if(required||!(error==ERROR_FILE_NOT_FOUND||error==ERROR_PATH_NOT_FOUND))return 0;}
    if(exists&&(a.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))return 0;
    return CryptHashData(hash,(const BYTE *)path,(DWORD)((wcslen(path)+1)*sizeof(WCHAR)),0)&&
           CryptHashData(hash,(const BYTE *)&exists,sizeof exists,0)&&
           (!exists||(CryptHashData(hash,(const BYTE *)&a.ftLastWriteTime,sizeof a.ftLastWriteTime,0)&&
                      CryptHashData(hash,(const BYTE *)&a.nFileSizeHigh,sizeof a.nFileSizeHigh,0)&&
                      CryptHashData(hash,(const BYTE *)&a.nFileSizeLow,sizeof a.nFileSizeLow,0)));
}
static int signature(const WCHAR *base,const struct sapi_dictionary_file *files,size_t count,WCHAR hex[65])
{
    HCRYPTPROV provider=0;HCRYPTHASH hash=0;BYTE value[32];DWORD length=sizeof value;int ok=0;
    if(!CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_AES,CRYPT_VERIFYCONTEXT)||
       !CryptCreateHash(provider,CALG_SHA_256,0,0,&hash))goto done;
    if(!CryptHashData(hash,(const BYTE *)base,(DWORD)((wcslen(base)+1)*sizeof(WCHAR)),0))goto done;
    if(base[0])for(size_t i=0;i<sizeof volumes/sizeof *volumes;++i)for(unsigned j=0;j<3;++j){
        WCHAR path[2048];if(!volume_path(path,2048,base,i,j)||!attributes(hash,path,0))goto done;}
    for(size_t i=0;i<count;++i){
        if(!CryptHashData(hash,(const BYTE *)&files[i].dialect,sizeof files[i].dialect,0)||
           !CryptHashData(hash,(const BYTE *)&files[i].volume,sizeof files[i].volume,0)||
           !attributes(hash,files[i].path,1))goto done;
    }
    if(!CryptGetHashParam(hash,HP_HASHVAL,value,&length,0)||length!=32)goto done;
    for(size_t i=0;i<32;++i)swprintf(hex+2*i,3,L"%02x",value[i]);hex[64]=0;ok=1;
done:if(hash)CryptDestroyHash(hash);if(provider)CryptReleaseContext(provider,0);return ok;
}
static int read_file(const WCHAR *path,int required,unsigned char **data,size_t *length)
{
    *data=NULL;*length=0;FILE *f=_wfopen(path,L"rb");
    if(!f){DWORD attr=GetFileAttributesW(path);return !required&&attr==INVALID_FILE_ATTRIBUTES&&
        (GetLastError()==ERROR_FILE_NOT_FOUND||GetLastError()==ERROR_PATH_NOT_FOUND);}
    unsigned char *bytes=malloc(LIMIT+1);if(!bytes){fclose(f);return 0;}
    size_t n=fread(bytes,1,LIMIT+1,f);int ok=!ferror(f)&&n<=LIMIT;fclose(f);
    if(!ok){free(bytes);return 0;}*data=bytes;*length=n;return 1;
}
static int ready(const WCHAR *path)
{
    WCHAR marker[2048];WIN32_FILE_ATTRIBUTE_DATA a;
    return swprintf(marker,2048,L"%ls\\ready",path)>=0&&
        GetFileAttributesExW(marker,GetFileExInfoStandard,&a)&&!(a.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY);
}
static void clear_stage(const WCHAR *path)
{
    WCHAR file[2048];for(size_t i=0;i<sizeof volumes/sizeof *volumes;++i)for(unsigned j=0;j<3;++j)
        if(volume_path(file,2048,path,i,j))DeleteFileW(file);
    if(swprintf(file,2048,L"%ls\\ready",path)>=0)DeleteFileW(file);RemoveDirectoryW(path);
}
static int build(const WCHAR *base,const struct sapi_dictionary_file *files,size_t count,const WCHAR *stage)
{
    for(size_t lang=0;lang<sizeof volumes/sizeof *volumes;++lang)for(unsigned volume=0;volume<3;++volume){
        struct sapi_dictionary_input input[SAPI_DICTIONARY_FILES_MAX+1]={0};size_t used=0;WCHAR path[2048];
        if(base[0]){
            unsigned char *data=NULL;size_t length=0;
            if(!volume_path(path,2048,base,lang,volume)||!read_file(path,0,&data,&length))return 0;
            if(data)input[used++]=(struct sapi_dictionary_input){data,length};
        }
        int ok=1;
        for(size_t i=0;i<count;++i)if(files[i].dialect==volumes[lang].dialect&&files[i].volume==volume){
            unsigned char *data=NULL;size_t length=0;
            if(!read_file(files[i].path,1,&data,&length)){ok=0;break;}
            input[used++]=(struct sapi_dictionary_input){data,length};}
        unsigned char *merged=NULL;size_t size=0;
        if(ok&&used)ok=sapi_dictionary_merge(input,used,volumes[lang].dialect==0x40000&&volume==1,&merged,&size);
        for(size_t i=0;i<used;++i)free((void *)input[i].data);
        if(ok&&used){if(!volume_path(path,2048,stage,lang,volume))ok=0;
            else {FILE *f=_wfopen(path,L"wb");if(!f)ok=0;else{ok=fwrite(merged,1,size,f)==size;if(fclose(f))ok=0;}}}
        free(merged);if(!ok)return 0;
    }
    WCHAR marker[2048];if(swprintf(marker,2048,L"%ls\\ready",stage)<0)return 0;
    FILE *f=_wfopen(marker,L"wb");if(!f)return 0;return fclose(f)==0;
}
HRESULT sapi_dictionary_prepare(const struct sapi_settings *settings,const struct sapi_dictionary_file *files,
                                size_t count,WCHAR *path,size_t capacity)
{
    if(!settings||!path||!capacity||(!files&&count)||count>SAPI_DICTIONARY_FILES_MAX)return E_INVALIDARG;path[0]=0;
    WCHAR base[2048]=L"";
    if(settings->dictionaries){
        if(settings->directory[0])wcscpy(base,settings->directory);
        else if(!sapi_default_dictionary_path(base,2048))return E_FAIL;
        WIN32_FILE_ATTRIBUTE_DATA a;
        if(!GetFileAttributesExW(base,GetFileExInfoStandard,&a)||!(a.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))return E_FAIL;
    }
    HRESULT result=S_OK;
    if(!count){if(wcslen(base)>=capacity)result=E_INVALIDARG;else wcscpy(path,base);return result;}
    WCHAR root[2048],before[65],after[65],target[2048],stage[2048];
    if(!snapshot_root(root,2048)||!signature(base,files,count,before)||
       swprintf(target,2048,L"%ls\\%ls",root,before)<0||wcslen(target)>=capacity)return E_FAIL;
    if(ready(target)){wcscpy(path,target);return S_OK;}
    if(swprintf(stage,2048,L"%ls\\stage-%lu-%lu",root,(unsigned long)GetCurrentProcessId(),
        (unsigned long)GetCurrentThreadId())<0)return E_FAIL;
    clear_stage(stage);
    if(!CreateDirectoryW(stage,NULL))return E_FAIL;
    if(!build(base,files,count,stage)||!signature(base,files,count,after)||wcscmp(before,after))result=E_FAIL;
    else if(!MoveFileExW(stage,target,MOVEFILE_WRITE_THROUGH)&&!ready(target))result=E_FAIL;
    else {wcscpy(path,target);result=S_OK;}
    clear_stage(stage);return result;
}
HRESULT sapi_dictionary_resolve(const struct sapi_settings *settings,WCHAR *path,size_t capacity)
{
    if(!settings||!path||!capacity)return E_INVALIDARG;
    struct sapi_dictionary_file *files=calloc(SAPI_DICTIONARY_FILES_MAX,sizeof *files);if(!files)return E_OUTOFMEMORY;
    size_t count=0;HRESULT result=sapi_dictionary_files_load(files,SAPI_DICTIONARY_FILES_MAX,&count);
    DWORD source=sapi_dictionary_source();ULONGLONG now=GetTickCount64();int match=0;
    if(SUCCEEDED(result)&&count){
        AcquireSRWLockShared(&cache_lock);
        match=cache.valid&&cache.enabled==settings->dictionaries&&cache.source==source&&
              cache.count==count&&!wcscmp(cache.directory,settings->directory)&&
              !memcmp(cache.files,files,count*sizeof *files);
        if(match&&now-cache.checked<500&&wcslen(cache.path)<capacity){
            wcscpy(path,cache.path);ReleaseSRWLockShared(&cache_lock);free(files);return S_OK;
        }
        ReleaseSRWLockShared(&cache_lock);
    }
    if(SUCCEEDED(result))result=sapi_dictionary_prepare(settings,files,count,path,capacity);
    if(FAILED(result)&&match){
        AcquireSRWLockShared(&cache_lock);
        if(cache.valid&&cache.enabled==settings->dictionaries&&cache.source==source&&cache.count==count&&
           !wcscmp(cache.directory,settings->directory)&&!memcmp(cache.files,files,count*sizeof *files)&&
           ready(cache.path)&&wcslen(cache.path)<capacity){wcscpy(path,cache.path);result=S_OK;}
        ReleaseSRWLockShared(&cache_lock);
    } else if(SUCCEEDED(result)&&count){
        AcquireSRWLockExclusive(&cache_lock);
        cache.valid=1;cache.enabled=settings->dictionaries;cache.source=source;cache.checked=now;
        wcscpy(cache.directory,settings->directory);wcscpy(cache.path,path);cache.count=count;
        memcpy(cache.files,files,count*sizeof *files);
        ReleaseSRWLockExclusive(&cache_lock);
    }
    free(files);return result;
}
