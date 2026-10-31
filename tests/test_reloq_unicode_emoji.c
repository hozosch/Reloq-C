/* Exhaustive admission of the shipped RGI inventory through each language.
 * The literal CLDR pools are test expectations, not a second runtime copy. */
#include "reloq_unicode.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/reloq_unicode_data.inc"
#include "../src/reloq_unicode_multilingual.inc"
#include "../src/reloq_unicode_emoji.inc"
#define COUNT(a) (sizeof(a)/sizeof *(a))
#define STR(i) ((const char *)unicode_strings+(i))

static const char *expected(const char *key,int locale)
{
    if(locale<0) {
        size_t lo=0,hi=COUNT(unicode_cldr);
        while(lo<hi){size_t m=lo+(hi-lo)/2;if(strcmp(STR(unicode_cldr[m].key),key)<0)lo=m+1;else hi=m;}
        assert(lo<COUNT(unicode_cldr)&&!strcmp(STR(unicode_cldr[lo].key),key));
        uint32_t offset=locale==-1?unicode_cldr[lo].de:unicode_cldr[lo].en;
        assert(offset);return STR(offset);
    }
    size_t lo=0,hi=COUNT(multi_names);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(strcmp((const char *)multi_name_strings+multi_names[m].key,key)<0)lo=m+1;else hi=m;}
    assert(lo<COUNT(multi_names)&&!strcmp((const char *)multi_name_strings+multi_names[lo].key,key));
    uint32_t offset=multi_names[lo].label[locale];assert(offset);
    return (const char *)multi_name_strings+offset;
}

int main(void)
{
    static const struct {int language,locale;} languages[]={
        {0x40000,-1},{0x10000,-2},{0x10001,-2},{0x20000,0},
        {0x20001,0},{0x30000,1},{0x30001,1},{0x50000,2},
        {0x60000,3},{0x70000,4},{0x80000,5},{0x90000,6},
        {0xa0000,7},{0x120000,8}
    };
    size_t checks=0;
    for(size_t k=0;k<COUNT(unicode_emoji_keys);++k) {
        const char *key=STR(unicode_emoji_keys[k]);size_t length=strlen(key);
        char input[164];assert(length<sizeof input-4);memcpy(input,key,length);
        /* One explicitly selected emoji also covers the text-default base
         * symbols. Multi-codepoint rows accept the normalized VS16 form. */
        unsigned first=(unsigned char)key[0];size_t scalar=first<128?1:first<224?2:first<240?3:4;
        if(length==scalar){memcpy(input+length,"\xef\xb8\x8f",3);length+=3;}
        input[length]=0;
        for(size_t l=0;l<COUNT(languages);++l)for(int mode=0;mode<2;++mode) {
            unsigned char modes[164],*output=NULL;size_t written=0,*map=NULL;
            memset(modes,mode,length);
            assert(!reloq_unicode_admit_language((const unsigned char *)input,length,modes,
                languages[l].language,0,&output,&written,&map));
            const char *label=expected(key,languages[l].locale);size_t wanted=strlen(label);
            if(written!=wanted+2||output[0]!=' '||memcmp(output+1,label,wanted)||output[wanted+1]!=' ')
                fprintf(stderr,"Emoji %s language %x mode %d: %s; expected %s\n",key,languages[l].language,mode,output,label);
            assert(written==wanted+2&&output[0]==' '&&!memcmp(output+1,label,wanted)&&output[wanted+1]==' '&&output[written]==0);
            assert(map[0]==0&&map[length]==written);
            for(size_t i=1;i<=length;++i)assert(map[i]>=map[i-1]&&map[i]<=written);
            free(output);free(map);++checks;
        }
    }
    printf("CLDR emoji: %zu localized sentence/character readings and source maps passed\n",checks);
    return 0;
}
