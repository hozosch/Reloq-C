/* Persian standalone character naming in the shared, cross-platform core. */
#include "direct_fa_characters.h"
#include "direct_fa_letters.h"
#include "reloq_unicode.h"
#include <stdio.h>
#include <string.h>

struct fa_character_name { uint32_t codepoint; const char *tag; };
struct fa_character_alias { uint32_t codepoint, letter; };
#include "direct_fa_characters.inc"

static uint32_t compatibility_letter(uint32_t codepoint)
{
    size_t lo=0,hi=sizeof fa_letter_aliases/sizeof fa_letter_aliases[0];
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(fa_letter_aliases[mid].codepoint<codepoint)lo=mid+1;
        else hi=mid;
    }
    return lo<sizeof fa_letter_aliases/sizeof fa_letter_aliases[0] &&
           fa_letter_aliases[lo].codepoint==codepoint ? fa_letter_aliases[lo].letter : codepoint;
}

static const char *symbol_name(uint32_t codepoint)
{
    size_t lo=0,hi=sizeof fa_symbol_names/sizeof fa_symbol_names[0];
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(fa_symbol_names[mid].codepoint<codepoint)lo=mid+1;
        else hi=mid;
    }
    return lo<sizeof fa_symbol_names/sizeof fa_symbol_names[0] &&
           fa_symbol_names[lo].codepoint==codepoint ? fa_symbol_names[lo].tag : NULL;
}

static const char *latin_name(uint32_t codepoint)
{
    if(codepoint>='A' && codepoint<='Z')codepoint+='a'-'A';
    return codepoint>='a' && codepoint<='z' ? fa_latin_names[codepoint-'a'] : NULL;
}

static int append(char *buffer,size_t capacity,size_t *used,const char *value)
{
    size_t length=strlen(value);
    if(*used>=capacity || length>=capacity-*used)return -1;
    memcpy(buffer+*used,value,length+1);*used+=length;return 0;
}

int reloq_fa_direct_character(uint32_t codepoint,char *output,size_t capacity)
{
    if(!codepoint || codepoint>0x10ffffu || (codepoint>=0xd800u && codepoint<=0xdfffu))return 0;
    if(!output || !capacity)return -1;
    const char *name=reloq_fa_direct_letter(compatibility_letter(codepoint));
    if(!name)name=symbol_name(codepoint);
    if(!name)name=latin_name(codepoint);
    char result[1024];size_t used=0;
    result[0]=0;
    if(name) {
        if(append(result,sizeof result,&used,name))return -1;
    } else {
        /* Persian CLDR/letter names take priority. If Unicode assigns an
         * English name but no Persian label is available, speak that name
         * in the English core rather than reduce it to an opaque number. */
        char english[512];
        int named=reloq_unicode_name_english(codepoint,english,sizeof english);
        if(named<0)return -1;
        if(named>0) {
            if(append(result,sizeof result,&used,"`l1.0 ") ||
               append(result,sizeof result,&used,english) ||
               append(result,sizeof result,&used," `l9.0"))return -1;
        } else {
            char digits[9];
            if(snprintf(digits,sizeof digits,"%X",(unsigned)codepoint)<1)return -1;
            if(append(result,sizeof result,&used,fa_unicode_name) ||
               append(result,sizeof result,&used," ") ||
               append(result,sizeof result,&used,symbol_name('+')))return -1;
            for(const char *digit=digits;*digit;++digit) {
                const char *part=latin_name((unsigned char)*digit);
                if(!part)part=symbol_name((unsigned char)*digit);
                if(!part || append(result,sizeof result,&used," ") ||
                   append(result,sizeof result,&used,part))return -1;
            }
        }
    }
    if(used>=capacity)return -1;
    memcpy(output,result,used+1);
    return 1;
}
