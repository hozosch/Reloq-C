/* Download the chosen community dictionary outside the speech and UI
 * threads. A complete commit-addressed snapshot is published atomically. */
#include "settings.h"
#include "dictionary_diff.h"
#include <winhttp.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define LIMIT (8u*1024u*1024u)
static const WCHAR registry[]=L"Software\\ReLoq C\\SAPI5";
struct dictionary_repository {
    const WCHAR *directory,*commit_key,*repository;
    const WCHAR *const *local,*const *remote;
    const char *const *remote_utf8;
    unsigned count;int convert_utf8;
};
static const WCHAR *const community_names[]={L"DEUmain.dic",L"DEURoot.dic",L"DEUabbr.dic",L"ENUmain.dic",L"ENURoot.dic",L"ENUabbr.dic"};
static const char *const community_utf8[]={"DEUmain.dic","DEURoot.dic","DEUabbr.dic","ENUmain.dic","ENURoot.dic","ENUabbr.dic"};
static const WCHAR *const alternative_names[]={L"ENUmain.dic",L"ENURoot.dic",L"ESPmain.dic"};
static const WCHAR *const alternative_remote[]={L"enumain.dic",L"enuroot.dic",L"espmain.dic"};
static const char *const alternative_utf8[]={"enumain.dic","enuroot.dic","espmain.dic"};
static const struct dictionary_repository repositories[]={
    {L"Community",L"CommunityCommit",L"eigencrow/IBMTTSDictionaries",community_names,community_names,community_utf8,6,0},
    {L"Alternative",L"AlternativeCommit",L"Mohamed00/AltIBMTTSDictionaries",alternative_names,alternative_remote,alternative_utf8,3,1}};
static int mkdirs(WCHAR *path);
static HRESULT put_value(const WCHAR *name,DWORD type,const void *data,DWORD len)
{
    HKEY k;LSTATUS e=RegCreateKeyExW(HKEY_CURRENT_USER,registry,0,NULL,0,KEY_SET_VALUE,NULL,&k,NULL);
    if(!e){e=RegSetValueExW(k,name,0,type,data,len);RegCloseKey(k);}return e?HRESULT_FROM_WIN32(e):S_OK;
}
static int is_sha(const char *sha)
{for(int i=0;i<40;++i)if(!((sha[i]>='0'&&sha[i]<='9')||(sha[i]>='a'&&sha[i]<='f')))return 0;return sha[40]==0;}
static int is_sha_w(const WCHAR *sha)
{for(int i=0;i<40;++i)if(!((sha[i]>=L'0'&&sha[i]<=L'9')||(sha[i]>=L'a'&&sha[i]<=L'f')))return 0;return sha[40]==0;}
DWORD sapi_dictionary_source(void)
{
    DWORD n=0,size=sizeof n;
    if(RegGetValueW(HKEY_CURRENT_USER,registry,L"DictionarySource",RRF_RT_REG_DWORD,NULL,&n,&size)!=ERROR_SUCCESS||n>1)return 0;
    return n;
}
HRESULT sapi_dictionary_choose(DWORD n)
{return n>1?E_INVALIDARG:put_value(L"DictionarySource",REG_DWORD,&n,sizeof n);}
static int base_path(DWORD source,WCHAR *path,size_t cap)
{
    DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",path,(DWORD)cap);
    const WCHAR suffix[]=L"\\ReLoq C\\SAPI5\\Dictionaries\\";
    if(source>1||!n||n>=cap||n+wcslen(suffix)+wcslen(repositories[source].directory)>=cap)return 0;
    wcscat(path,suffix);wcscat(path,repositories[source].directory);return 1;
}
static int complete(const WCHAR *path,const struct dictionary_repository *repo)
{
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if(!GetFileAttributesExW(path,GetFileExInfoStandard,&attr)||!(attr.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))return 0;
    for(unsigned i=0;i<repo->count;++i){WCHAR file[2048];
        if(swprintf(file,2048,L"%ls\\%ls",path,repo->local[i])<0||
           !GetFileAttributesExW(file,GetFileExInfoStandard,&attr)||(attr.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))return 0;
    }
    return 1;
}
static int snapshot_path(DWORD source,WCHAR *path,size_t capacity)
{
    if(source>1)return 0;const struct dictionary_repository *repo=repositories+source;
    WCHAR sha[41]={0};DWORD bytes=sizeof sha;
    if(RegGetValueW(HKEY_CURRENT_USER,registry,repo->commit_key,RRF_RT_REG_SZ,NULL,sha,&bytes)!=ERROR_SUCCESS||!is_sha_w(sha)||!base_path(source,path,capacity))return 0;
    if(wcslen(path)+1+40>=capacity)return 0;wcscat(path,L"\\");wcscat(path,sha);
    return complete(path,repo);
}
int sapi_alternative_dictionary_available(void)
{WCHAR path[2048];return snapshot_path(1,path,2048);}

/* Copy the installer snapshot once into the user's writable profile. Never
 * rewrite the installed files; existing user and downloaded data stay put. */
static int bundled_path(WCHAR *path,size_t capacity)
{
    WCHAR base[2048],installed[2048],stage[2048],from[2048],to[2048];
    if(!base_path(0,base,2048)||!mkdirs(base)||!sapi_module_path(installed,2048,L"dictionaries"))return 0;
    if(swprintf(path,capacity,L"%ls\\Bundled",base)<0)return 0;
    if(complete(path,repositories))return 1;
    if(!complete(installed,repositories)||swprintf(stage,2048,L"%ls\\bundled-%lu-%lu",base,
        (unsigned long)GetCurrentProcessId(),(unsigned long)GetCurrentThreadId())<0||!CreateDirectoryW(stage,NULL))return 0;
    int ok=1;
    for(unsigned i=0;i<repositories[0].count;++i){
        if(swprintf(from,2048,L"%ls\\%ls",installed,community_names[i])<0||
           swprintf(to,2048,L"%ls\\%ls",stage,community_names[i])<0||!CopyFileW(from,to,FALSE)){ok=0;break;}}
    if(ok&&!MoveFileExW(stage,path,MOVEFILE_WRITE_THROUGH))ok=complete(path,repositories);
    if(!ok||complete(path,repositories)){
        for(unsigned i=0;i<repositories[0].count;++i){swprintf(to,2048,L"%ls\\%ls",stage,community_names[i]);DeleteFileW(to);}
        RemoveDirectoryW(stage);
    }
    return ok&&complete(path,repositories);
}
int sapi_default_dictionary_path(WCHAR *path,size_t capacity)
{
    DWORD source=sapi_dictionary_source();
    if(snapshot_path(source,path,capacity))return 1;
    return source?0:bundled_path(path,capacity)||sapi_module_path(path,capacity,L"dictionaries");
}
static int mkdirs(WCHAR *path)
{
    for(WCHAR *p=path+3;*p;++p)if(*p==L'\\') {*p=0;BOOL ok=CreateDirectoryW(path,NULL);DWORD e=GetLastError();*p=L'\\';if(!ok&&e!=ERROR_ALREADY_EXISTS)return 0;}
    return CreateDirectoryW(path,NULL)||GetLastError()==ERROR_ALREADY_EXISTS;
}
static int download(HINTERNET session,const WCHAR *host,const WCHAR *url,unsigned char **out,DWORD *length,DWORD limit)
{
    HINTERNET connect=WinHttpConnect(session,host,INTERNET_DEFAULT_HTTPS_PORT,0);if(!connect)return 0;
    HINTERNET req=WinHttpOpenRequest(connect,L"GET",url,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE);
    int ok=0;unsigned char *data=NULL;DWORD used=0,status=0,sz=sizeof status;
    if(!req)goto done;
    DWORD no_redirect=WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    if(!WinHttpSetOption(req,WINHTTP_OPTION_REDIRECT_POLICY,&no_redirect,sizeof no_redirect)||
       !WinHttpSendRequest(req,WINHTTP_NO_ADDITIONAL_HEADERS,0,WINHTTP_NO_REQUEST_DATA,0,0,0)||!WinHttpReceiveResponse(req,NULL)||
       !WinHttpQueryHeaders(req,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&sz,WINHTTP_NO_HEADER_INDEX)||status!=200)goto done;
    data=malloc((size_t)limit+1);if(!data)goto done;
    for(;;){DWORD count=0,got=0;if(!WinHttpQueryDataAvailable(req,&count))goto done;if(!count)break;
        if(count>limit-used||!WinHttpReadData(req,data+used,count,&got)||!got)goto done;used+=got;}
    data[used]=0;*out=data;*length=used;data=NULL;ok=1;
done:free(data);if(req)WinHttpCloseHandle(req);WinHttpCloseHandle(connect);return ok;
}
/* The GitHub tree contains one small flat object per top-level path. */
static int tree_hash(const char *tree,const char *name,char result[41])
{
    const char *p=tree;while((p=strstr(p,"\"path\""))) {
        const char *colon=strchr(p,':'),*end;if(!colon)return 0;
        while(*++colon==' '||*colon=='\t'||*colon=='\r'||*colon=='\n'){}
        if(*colon++!='\"')return 0;end=strchr(colon,'\"');if(!end)return 0;
        if((size_t)(end-colon)==strlen(name)&&!memcmp(colon,name,strlen(name))) {
            const char *close=strchr(end,'}'),*sha=strstr(end,"\"sha\"");
            if(!close||!sha||sha>close||(sha=strchr(sha,':'))==NULL)return 0;
            while(*++sha==' '||*sha=='\t'||*sha=='\r'||*sha=='\n'){}
            if(*sha++!='\"'||strlen(sha)<41||sha[40]!='\"')return 0;
            memcpy(result,sha,40);result[40]=0;return is_sha(result);
        }p=end+1;
    }return 0;
}
static int blob_ok(const unsigned char *data,DWORD size,const char *expected)
{
    HCRYPTPROV provider=0;HCRYPTHASH hash=0;BYTE digest[20];DWORD len=sizeof digest;int ok=0;
    char header[40];int n=snprintf(header,sizeof header,"blob %lu",(unsigned long)size);
    if(n<0||!CryptAcquireContextW(&provider,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT)||
       !CryptCreateHash(provider,CALG_SHA1,0,0,&hash)||!CryptHashData(hash,(const BYTE *)header,(DWORD)n+1,0)||
       !CryptHashData(hash,data,size,0)||!CryptGetHashParam(hash,HP_HASHVAL,digest,&len,0)||len!=20)goto done;
    char hex[41];for(int i=0;i<20;++i)snprintf(hex+i*2,3,"%02x",digest[i]);ok=!strcmp(hex,expected);
done:if(hash)CryptDestroyHash(hash);if(provider)CryptReleaseContext(provider,0);return ok;
}
static int save_converted(const WCHAR *path,const unsigned char *data,DWORD size)
{
    for(DWORD i=0;i<size;++i)if(!data[i])return 0;
    FILE *out=_wfopen(path,L"wb");if(!out)return 0;int ok=1;
    DWORD start=0;if(size>=3&&data[0]==0xef&&data[1]==0xbb&&data[2]==0xbf)start=3;
    while(start<size&&ok) {
        DWORD end=start;while(end<size&&data[end]!='\n'&&data[end]!='\r')++end;
        int count=(int)(end-start);if(count) {
            int wide=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,(const char *)data+start,count,NULL,0);
            if(!wide){if(fwrite(data+start,1,count,out)!=(size_t)count)ok=0;}
            else {WCHAR *w=malloc((size_t)wide*sizeof *w);char *n=malloc((size_t)wide*4);BOOL replaced=FALSE;
                if(!w||!n||!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,(const char *)data+start,count,w,wide))ok=0;
                else {int bytes=WideCharToMultiByte(1252,WC_NO_BEST_FIT_CHARS,w,wide,n,wide*4,NULL,&replaced);
                    if(bytes&&!replaced&&fwrite(n,1,(size_t)bytes,out)!=(size_t)bytes)ok=0;}
                free(w);free(n);}
        }
        if(fputc('\n',out)==EOF)ok=0;while(end<size&&(data[end]=='\n'||data[end]=='\r'))++end;start=end;
    }
    if(fclose(out))ok=0;return ok;
}
static int save_raw(const WCHAR *path,const unsigned char *data,DWORD size)
{
    for(DWORD i=0;i<size;++i)if(!data[i])return 0;
    FILE *out=_wfopen(path,L"wb");if(!out)return 0;
    int ok=fwrite(data,1,size,out)==size;if(fclose(out))ok=0;return ok;
}
static void remove_stage(const WCHAR *path,const struct dictionary_repository *repo)
{WCHAR file[2048];for(unsigned i=0;i<repo->count;++i){swprintf(file,2048,L"%ls\\%ls",path,repo->local[i]);DeleteFileW(file);}RemoveDirectoryW(path);}
static int same_file(const WCHAR *a,const WCHAR *b)
{
    FILE *first=_wfopen(a,L"rb"),*second=_wfopen(b,L"rb");int ok=0;
    if(first&&second) {
        unsigned char x[4096],y[4096];ok=1;
        for(;;) {size_t n=fread(x,1,sizeof x,first),m=fread(y,1,sizeof y,second);
            if(n!=m||memcmp(x,y,n)){ok=0;break;}if(n<sizeof x){if(ferror(first)||ferror(second))ok=0;break;}}
    }
    if(first)fclose(first);if(second)fclose(second);return ok;
}
static int read_volume(const WCHAR *path,unsigned char **out,size_t *size)
{
    FILE *f=_wfopen(path,L"rb");if(!f)return 0;
    unsigned char *data=malloc(LIMIT+1);if(!data){fclose(f);return 0;}
    size_t n=fread(data,1,LIMIT+1,f);int ok=!ferror(f)&&n<=LIMIT;
    fclose(f);if(!ok){free(data);return 0;}*out=data;*size=n;return 1;
}
static void count_update(const WCHAR *previous,const WCHAR *stage,int had_previous,const struct dictionary_repository *repo,
                         struct sapi_dictionary_update *result)
{
    if(!result)return;
    result->known=1;result->compared=had_previous;
    for(unsigned i=0;i<repo->count;++i){WCHAR before_path[2048],after_path[2048];
        unsigned char *before=NULL,*after=NULL;size_t n=0,m=0,added=0,total=0;
        swprintf(after_path,2048,L"%ls\\%ls",stage,repo->local[i]);
        if(had_previous)swprintf(before_path,2048,L"%ls\\%ls",previous,repo->local[i]);
        int ok=read_volume(after_path,&after,&m)&&(!had_previous||read_volume(before_path,&before,&n))&&
            sapi_dictionary_diff(before,n,after,m,!wcscmp(repo->local[i],L"DEURoot.dic"),&added,&total);
        free(before);free(after);
        if(!ok){result->known=0;return;}
        result->added+=added;result->total+=total;
        ++result->total_files;if(!had_previous)++result->added_files;
    }
}
HRESULT sapi_dictionary_download_source(DWORD source,struct sapi_dictionary_update *report)
{
    if(source>1)return E_INVALIDARG;const struct dictionary_repository *repo=repositories+source;
    if(report)memset(report,0,sizeof *report);
    WCHAR previous[2048]={0},old_commit[41]={0};DWORD old_size=sizeof old_commit;
    int had_previous=snapshot_path(source,previous,2048);
    if(!had_previous&&!source)had_previous=bundled_path(previous,2048);
    LSTATUS prior_status=RegGetValueW(HKEY_CURRENT_USER,registry,repo->commit_key,RRF_RT_REG_SZ,
        NULL,old_commit,&old_size);
    int prior_recorded=prior_status!=ERROR_FILE_NOT_FOUND&&prior_status!=ERROR_PATH_NOT_FOUND;
    HINTERNET session=WinHttpOpen(L"ReLoq-C-SAPI-dictionaries",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);
    if(!session)return E_FAIL;WinHttpSetTimeouts(session,8000,8000,12000,12000);
    unsigned char *data=NULL,*tree=NULL;DWORD size=0,tree_size=0;char sha[41]={0},expected[6][41];WCHAR base[2048],stage[2048],target[2048],url[512],wsha[41];HRESULT result=E_FAIL;
    swprintf(url,512,L"/repos/%ls/branches/master",repo->repository);
    if(!download(session,L"api.github.com",url,&data,&size,1024*1024))goto done;
    const char *p=strstr((char *)data,"\"sha\"");if(!p||(p=strchr(p,':'))==NULL)goto done;
    while(*++p==' '||*p=='\t'||*p=='\r'||*p=='\n'){}if(*p++!='\"'||strlen(p)<41||p[40]!='\"')goto done;
    memcpy(sha,p,40);if(!is_sha(sha))goto done;free(data);data=NULL;
    for(int i=0;i<40;++i)wsha[i]=(WCHAR)sha[i];wsha[40]=0;
    swprintf(url,512,L"/repos/%ls/git/trees/%ls",repo->repository,wsha);
    if(!download(session,L"api.github.com",url,&tree,&tree_size,2*1024*1024))goto done;
    p=strstr((char *)tree,"\"truncated\"");if(!p||(p=strchr(p,':'))==NULL)goto done;
    while(*++p==' '||*p=='\t'||*p=='\r'||*p=='\n'){}if(strncmp(p,"false",5))goto done;
    for(unsigned i=0;i<repo->count;++i)if(!tree_hash((char *)tree,repo->remote_utf8[i],expected[i]))goto done;
    if(!base_path(source,base,2048)||!mkdirs(base))goto done;
    swprintf(target,2048,L"%ls\\%ls",base,wsha);
    swprintf(stage,2048,L"%ls\\download-%lu-%lu",base,(unsigned long)GetCurrentProcessId(),(unsigned long)GetCurrentThreadId());
    remove_stage(stage,repo);if(!CreateDirectoryW(stage,NULL))goto done;
    for(unsigned i=0;i<repo->count;++i) {
        WCHAR file[2048];swprintf(url,512,L"/%ls/%ls/%ls",repo->repository,wsha,repo->remote[i]);
        if(!download(session,L"raw.githubusercontent.com",url,&data,&size,LIMIT)||!blob_ok(data,size,expected[i]))goto fail_stage;
        swprintf(file,2048,L"%ls\\%ls",stage,repo->local[i]);
        if(!(repo->convert_utf8?save_converted(file,data,size):save_raw(file,data,size)))goto fail_stage;free(data);data=NULL;
    }
    count_update(previous,stage,had_previous,repo,report);
    if(report&&prior_recorded&&!had_previous)report->known=0;
    if(!MoveFileExW(stage,target,MOVEFILE_WRITE_THROUGH)) {
        WIN32_FILE_ATTRIBUTE_DATA attr;
        if(!GetFileAttributesExW(target,GetFileExInfoStandard,&attr)||!(attr.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))goto fail_stage;
        for(unsigned i=0;i<repo->count;++i){WCHAR a[2048],b[2048];
            swprintf(a,2048,L"%ls\\%ls",stage,repo->local[i]);swprintf(b,2048,L"%ls\\%ls",target,repo->local[i]);
            if(!same_file(a,b))goto fail_stage;
        }
        remove_stage(stage,repo);
    }
    result=put_value(repo->commit_key,REG_SZ,wsha,sizeof wsha);
    if(SUCCEEDED(result))result=sapi_dictionary_choose(source);
    goto done;
fail_stage:remove_stage(stage,repo);
done:free(data);free(tree);WinHttpCloseHandle(session);return result;
}
HRESULT sapi_dictionary_download_report(struct sapi_dictionary_update *report)
{return sapi_dictionary_download_source(sapi_dictionary_source(),report);}
HRESULT sapi_dictionary_download(void){return sapi_dictionary_download_report(NULL);}
