/* Hints for the SAPI dictionary picker. Both are suggestions: a user may
 * assign a native dictionary to a different language or volume. */
#ifndef RELOQ_SAPI_DICTIONARY_HINT_H
#define RELOQ_SAPI_DICTIONARY_HINT_H
#include <stddef.h>
#include <wchar.h>

static inline wchar_t sapi_dict_ascii_lower(wchar_t c)
{return c>=L'A'&&c<=L'Z'?c+(L'a'-L'A'):c;}

static inline int sapi_dict_equal(const wchar_t *a,const wchar_t *b)
{
    while(*a&&*b){if(sapi_dict_ascii_lower(*a++)!=sapi_dict_ascii_lower(*b++))return 0;}
    return !*a&&!*b;
}

static inline int sapi_dictionary_guess(const wchar_t *path,int *dialect,unsigned *volume)
{
    struct prefix {wchar_t code[4];int dialect;};
    static const struct prefix prefixes[]={
        {L"ENU",0x10000},{L"ENG",0x10001},{L"ESP",0x20000},{L"ESM",0x20001},
        {L"FRA",0x30000},{L"FRC",0x30001},{L"DEU",0x40000},{L"ITA",0x50000},
        {L"CHS",0x60000},{L"PTB",0x70000},{L"JPN",0x80000},{L"FIN",0x90000},
        {L"KOR",0xa0000},{L"ELL",0x120000}};
    static const wchar_t *const suffixes[]={L"main.dic",L"root.dic",L"abbr.dic"};
    if(!path||!dialect||!volume)return 0;
    const wchar_t *name=path;
    for(const wchar_t *p=path;*p;++p)if(*p==L'\\'||*p==L'/')name=p+1;
    if(wcslen(name)<4)return 0;
    for(size_t i=0;i<sizeof prefixes/sizeof *prefixes;++i){
        if(sapi_dict_ascii_lower(name[0])!=sapi_dict_ascii_lower(prefixes[i].code[0])||
           sapi_dict_ascii_lower(name[1])!=sapi_dict_ascii_lower(prefixes[i].code[1])||
           sapi_dict_ascii_lower(name[2])!=sapi_dict_ascii_lower(prefixes[i].code[2]))continue;
        for(unsigned v=0;v<3;++v)if(sapi_dict_equal(name+3,suffixes[v])){
            *dialect=prefixes[i].dialect;*volume=v;return 1;
        }
    }
    return 0;
}

/* NVDA files are UTF-8 text with pattern, replacement, case and kind in
 * four tab-separated columns. Native ECI replacements may contain tabs,
 * so the user can explicitly select native Eloquence format. */
static inline int sapi_dictionary_looks_nvda(const unsigned char *data,size_t length)
{
    if(!data)return 0;
    size_t pos=length>=3&&data[0]==0xef&&data[1]==0xbb&&data[2]==0xbf?3:0;
    while(pos<length){
        size_t end=pos;while(end<length&&data[end]!='\r'&&data[end]!='\n')++end;
        size_t begin=pos;while(begin<end&&(data[begin]==' '||data[begin]=='\t'))++begin;
        if(begin<end&&data[begin]!='#'){
            size_t tab[3],count=0;
            for(size_t i=pos;i<end;++i)if(data[i]=='\t'){if(count<3)tab[count]=i;++count;}
            if(count==3&&tab[1]+2==tab[2]&&(data[tab[1]+1]=='0'||data[tab[1]+1]=='1')&&tab[2]+1<end){
                size_t j=tab[2]+1;while(j<end&&data[j]>='0'&&data[j]<='9')++j;
                if(j==end)return 1;
            }
        }
        pos=end;while(pos<length&&(data[pos]=='\r'||data[pos]=='\n'))++pos;
    }
    return 0;
}
#endif
