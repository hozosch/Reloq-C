#include "speech_dictionary.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BYTES (8u*1024u*1024u)
static FILE *open_wide(const WCHAR *path)
{
#ifdef RELOQ_SAPI_TEST_HOST
    char name[8192];
    if(!WideCharToMultiByte(CP_UTF8,0,path,-1,name,sizeof name,NULL,NULL))return NULL;
    for(char *p=name;*p;++p)if(*p=='\\')*p='/';
    return fopen(name,"rb");
#else
    return _wfopen(path,L"rb");
#endif
}
int sapi_speech_dictionary_load(const WCHAR *directory,int dialect,unsigned profile,
                                struct reloq_nvda_speech **out)
{
    if(!directory||!out||profile>=8)return 0;*out=NULL;
    if(!*directory)return 1;
    WCHAR marker[2048];if(swprintf(marker,2048,L"%ls\\nvda.ready",directory)<0)return 0;
    errno=0;FILE *ready=open_wide(marker);
    if(!ready)return errno==ENOENT||errno==ENOTDIR;
    if(fclose(ready))return 0;
    struct reloq_nvda_speech *dictionary=NULL;
    for(unsigned i=0;i<64;++i)for(unsigned k=0;k<2;++k){
        unsigned voice=k?profile+1:0;WCHAR path[2048];
        if(swprintf(path,2048,L"%ls\\nvda-%03u-%08x-%u.nvda",directory,i,(unsigned)dialect,voice)<0)goto fail;
        errno=0;FILE *f=open_wide(path);
        if(!f){if(errno==ENOENT||errno==ENOTDIR)continue;goto fail;}
        if(fseek(f,0,SEEK_END)){fclose(f);goto fail;}
        long length=ftell(f);if(length<0||(unsigned long)length>MAX_BYTES||fseek(f,0,SEEK_SET)){fclose(f);goto fail;}
        unsigned char *data=malloc(length?(size_t)length:1);if(!data){fclose(f);goto fail;}
        size_t read=fread(data,1,(size_t)length,f);int ok=read==(size_t)length&&!ferror(f);
        if(fclose(f))ok=0;
        if(ok&&!dictionary)dictionary=reloq_nvda_speech_new();
        if(ok)ok=dictionary&&reloq_nvda_speech_add(dictionary,data,read);
        free(data);if(!ok)goto fail;
    }
    *out=dictionary;return 1;
fail:reloq_nvda_speech_free(dictionary);return 0;
}
