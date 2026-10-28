#include "reloq_english.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int reloq_english_parse(const unsigned char *text,size_t n,
    struct reloq_english_phone **phones,size_t *count,size_t *error)
{
    if(!text || !phones || !count || n>SIZE_MAX/sizeof **phones)
        return RELOQ_ENGLISH_INVALID;
    struct reloq_english_phone *out=calloc(n?n:1,sizeof *out);
    if(!out)return RELOQ_ENGLISH_MEMORY;
    size_t used=0,word=0,syllable=0,i=0;unsigned stress=0,weak=0;
    int in_word=0,have_syllable=0,syllable_has_phone=0,accent=1;
    for(;i<n;++i) {
        unsigned char c=text[i];
        if(!c)goto invalid;
        if(c=='`' && !in_word) {
            if(i+1<n && text[i+1]=='[') {
                ++i;in_word=1;have_syllable=0;syllable_has_phone=0;++word;continue;
            }
            if(i+1<n && text[i+1]>='0' && text[i+1]<='2') {
                weak=text[++i]=='0' && i+1<n && text[i+1]=='0';
                accent=weak?-1:text[i]-'0';
                if(weak)++i;
                continue;
            }
            goto invalid;
        }
        if(!in_word) {
            if(c==' '||c=='\t'||c=='\r'||c=='\n'||strchr(".,!?;:",c))continue;
            goto invalid;
        }
        if(c==']') {
            if(!have_syllable || !syllable_has_phone)goto invalid;
            in_word=0;weak=0;accent=1;continue;
        }
        if(c=='.') {
            if(have_syllable && !syllable_has_phone)goto invalid;
            if(i+1>=n || text[i+1]<'0' || text[i+1]>'2')goto invalid;
            stress=text[++i]-'0';++syllable;have_syllable=1;syllable_has_phone=0;continue;
        }
        if(!have_syllable || !c || !strchr("bpdtF?kgDTvfzsZSJChmnGrlywiIeEAXHxauUocRYWONM",c))
            goto invalid;
        syllable_has_phone=1;
        out[used++]=(struct reloq_english_phone){
            c=='N'?'n':c=='M'?'m':c,(unsigned char)stress,
            c=='N'||c=='M',(unsigned char)weak,(signed char)accent,word-1,syllable-1,i};
    }
    if(in_word)goto invalid;
    *phones=out;*count=used;return RELOQ_ENGLISH_OK;
invalid:
    if(error)*error=i;
    free(out);return RELOQ_ENGLISH_INVALID;
}
