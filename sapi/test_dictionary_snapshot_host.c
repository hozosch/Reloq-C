/* Test the real snapshot builder on a POSIX host with a tiny file API shim. */
#define _POSIX_C_SOURCE 200809L
#define RELOQ_SAPI_SNAPSHOT_TEST
#include "dictionary_snapshot.c"
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

static DWORD last_error;
DWORD GetLastError(void){return last_error;}
static int narrow(const WCHAR *wide,char *out,size_t cap)
{
    size_t n=wcstombs(out,wide,cap);if(n==(size_t)-1||n>=cap)return 0;
    for(char *p=out;*p;++p)if(*p=='\\')*p='/';return 1;
}
FILE *test_wfopen(const WCHAR *path,const WCHAR *mode)
{
    char name[4096],flags[16];if(!narrow(path,name,sizeof name)||!narrow(mode,flags,sizeof flags))return NULL;
    return fopen(name,flags);
}
DWORD GetFileAttributesW(const WCHAR *path)
{
    char name[4096];struct stat st;if(!narrow(path,name,sizeof name)||stat(name,&st)){
        last_error=errno==ENOENT?ERROR_FILE_NOT_FOUND:5;return INVALID_FILE_ATTRIBUTES;}
    return S_ISDIR(st.st_mode)?FILE_ATTRIBUTE_DIRECTORY:0;
}
static void wide(WCHAR *out,size_t capacity,const char *path)
{assert(mbstowcs(out,path,capacity)!=(size_t)-1);}
static void write_bytes(const char *path,const char *value)
{FILE *f=fopen(path,"wb");assert(f);size_t n=strlen(value);assert(fwrite(value,1,n,f)==n);assert(!fclose(f));}
static void same_file(const char *path,const char *value)
{FILE *f=fopen(path,"rb");assert(f);char data[1024];size_t n=fread(data,1,sizeof data,f);assert(!ferror(f));
 assert(n==strlen(value)&&!memcmp(data,value,n));assert(!fclose(f));}
int main(void)
{
    char root[]="/tmp/reloq-snapshot-host-XXXXXX";assert(mkdtemp(root));
    char community[512],native[512],speech[512],stage[512],path[1024];
    snprintf(community,sizeof community,"%s/community",root);assert(!mkdir(community,0700));
    snprintf(native,sizeof native,"%s/native.dic",root);snprintf(speech,sizeof speech,"%s/speech.dic",root);
    snprintf(path,sizeof path,"%s/ENUmain.dic",community);write_bytes(path,"shared\tcommunity\n");
    write_bytes(native,"shared\town\n");
    const char *nvda="\xef\xbb\xbf# NVDA\nyoutube\tbanana\t0\t0\n";write_bytes(speech,nvda);
    struct sapi_dictionary_file files[2]={0};files[0].dialect=files[1].dialect=0x10000;
    wide(files[0].path,1024,native);wide(files[1].path,1024,speech);
    files[1].volume=1;files[1].voice=2; /* Existing wrong root volume migrates. */
    WCHAR base[1024],destination[1024];wide(base,1024,community);
    for(unsigned enabled=0;enabled<2;++enabled){
        snprintf(stage,sizeof stage,"%s/stage-%u",root,enabled);assert(!mkdir(stage,0700));
        wide(destination,1024,stage);assert(build(enabled?base:L"",files,2,destination));
        snprintf(path,sizeof path,"%s/ENUmain.dic",stage);same_file(path,"shared\town\n");
        snprintf(path,sizeof path,"%s/nvda-001-00010000-2.nvda",stage);same_file(path,nvda);
        snprintf(path,sizeof path,"%s/nvda.ready",stage);same_file(path,"");
        same_file(speech,nvda);
    }
    snprintf(stage,sizeof stage,"%s/stage-invalid",root);assert(!mkdir(stage,0700));wide(destination,1024,stage);
    write_bytes(speech,"first\tword\nsecond\tvalue\t0\t0\n");
    assert(!build(L"",files,2,destination));
    snprintf(path,sizeof path,"%s/ready",stage);assert(access(path,F_OK)!=0);
    puts("SAPI snapshot: own NVDA and native files, migration and community on/off passed");
    return 0;
}
