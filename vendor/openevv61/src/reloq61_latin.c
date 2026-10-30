#include "reloq61_latin.h"
#include <stddef.h>
#include "reloq61_latin_data.inc"
static int latin_base(uint32_t cp)
{
    size_t lo=0,hi=sizeof latin_bases/sizeof *latin_bases;
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(latin_bases[m].scalar<cp)lo=m+1;else hi=m;}
    return lo<sizeof latin_bases/sizeof *latin_bases&&latin_bases[lo].scalar==cp?latin_bases[lo].base:0;
}
int reloq61_latin_letter_byte(unsigned c)
{
    return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||
        (c>=0xc0&&c<=0xff&&c!=0xd7&&c!=0xf7)||
        c==0x83||c==0x8a||c==0x8c||c==0x8e||c==0x9a||c==0x9c||c==0x9e||c==0x9f;
}
int reloq61_latin_byte(uint32_t cp)
{
    if(cp<128)return reloq61_latin_letter_byte(cp)?(int)cp:0;
    if(cp>=0xc0&&cp<=0xff)return reloq61_latin_letter_byte(cp)?(int)cp:0;
    switch(cp){
    case 0x192:return 0x83;case 0x160:return 0x8a;case 0x152:return 0x8c;
    case 0x17d:return 0x8e;case 0x161:return 0x9a;case 0x153:return 0x9c;
    case 0x17e:return 0x9e;case 0x178:return 0x9f;
    }
    return latin_base(cp);
}
int reloq61_latin_base_byte(unsigned c)
{
    if(c<128)return reloq61_latin_letter_byte(c)?(int)c:0;
    switch(c){
    case 0x83:c=0x192;break;case 0x8a:c=0x160;break;case 0x8c:c=0x152;break;
    case 0x8e:c=0x17d;break;case 0x9a:c=0x161;break;case 0x9c:c=0x153;break;
    case 0x9e:c=0x17e;break;case 0x9f:c=0x178;break;
    }
    return latin_base(c);
}
