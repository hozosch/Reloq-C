#include "reloq_character.h"
#include "reloq_utf8.h"
#include "reloq_unicode.h"
#include <stdlib.h>
#include <string.h>
#define COUNT(a) (sizeof(a)/sizeof *(a))
#include "reloq_character_data.inc"
#include "reloq_character_native.inc"
#include "reloq_character_multilingual.inc"
static const struct original_character *original(uint32_t cp)
{
    size_t a=0,b=COUNT(original_characters);
    while(a<b){size_t m=a+(b-a)/2;if(original_characters[m].cp<cp)a=m+1;else b=m;}
    return a<COUNT(original_characters)&&original_characters[a].cp==cp?original_characters+a:NULL;
}
static const struct latin_character *latin(uint32_t cp)
{
    size_t a=0,b=COUNT(latin_characters);
    while(a<b){size_t m=a+(b-a)/2;if(latin_characters[m].cp<cp)a=m+1;else b=m;}
    return a<COUNT(latin_characters)&&latin_characters[a].cp==cp?latin_characters+a:NULL;
}
int reloq_character_latin_base(uint32_t cp,unsigned char *base)
{
    if(!base)return -1;
    const struct latin_character *l=latin(cp);
    if(l){*base=l->base;return 1;}
    return 0;
}
/* Groups point into immutable SPRs. The length is bounded by the closing
 * bracket, and callers append them as phonetic units rather than words. */
static int groups(struct reloq_character_reading *r,const char *spr)
{
    const char *s=spr;
    while((s=strstr(s,"`["))!=NULL) {
        if(r->count==COUNT(r->groups))return -1;
        s+=2;r->groups[r->count++]=s;
    }
    return 0;
}
int reloq_character_read(uint32_t cp,int english,struct reloq_character_reading *out)
{
    if(!out || (unsigned)english>1)return -1;
    struct reloq_character_reading r={0};
    const struct original_character *o=original(cp);
    if(o){r.original_spr=o->spr[english];if(groups(&r,r.original_spr))return -1;*out=r;return 1;}
    const struct latin_character *l=latin(cp);
    if(!l)return 0;
    o=original(l->prefix);if(!o || groups(&r,o->spr[english]))return 0;
    for(unsigned k=0;k<3 && l->marks[k];++k) {
        size_t a=0,b=COUNT(character_marks);
        while(a<b){size_t m=a+(b-a)/2;if(character_marks[m].cp<l->marks[k])a=m+1;else b=m;}
        if(a==COUNT(character_marks)||character_marks[a].cp!=l->marks[k])return 0;
        if(groups(&r,character_marks[a].spr[english]))return -1;
    }
    *out=r;return 1;
}
static const struct original_character *native(uint32_t cp,int english)
{
    size_t a=0,b=COUNT(native_characters);
    while(a<b){size_t m=a+(b-a)/2;if(native_characters[m].cp<cp)a=m+1;else b=m;}
    return a<COUNT(native_characters)&&native_characters[a].cp==cp&&
        *native_characters[a].spr[english]?native_characters+a:NULL;
}
int reloq_character_native_available(uint32_t cp,int english)
{return (unsigned)english<=1&&native(cp,english)!=NULL;}
int reloq_character_read_native(uint32_t cp,int english,struct reloq_character_reading *out)
{
    if(!out||(unsigned)english>1)return -1;
    const struct original_character *entry=native(cp,english);
    if(!entry)return reloq_character_read(cp,english,out);
    struct reloq_character_reading r={.original_spr=entry->spr[english]};
    /* Preserve the requested short U after admitting the native character. */
    if(!english&&cp=='+')r.original_spr="`[.1plUs]";
    if(!english&&cp==0xb1)r.original_spr="`[.1plUs] `2 `[.1mi.0nUs]";
    if(groups(&r,r.original_spr))return -1;
    *out=r;return 1;
}
int reloq_character_spelling1(uint32_t cp)
{
    size_t a=0,b=COUNT(native_spelling1);
    while(a<b){size_t m=a+(b-a)/2;if(native_spelling1[m]<cp)a=m+1;else b=m;}
    if(a<COUNT(native_spelling1)&&native_spelling1[a]==cp)return 1;
    unsigned char base;
    return cp>255 && reloq_character_latin_base(cp,&base)==1;
}
static int language_index(int language)
{
    size_t lo=0,hi=COUNT(multi_languages);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(multi_languages[m]<language)lo=m+1;else hi=m;}
    return lo<COUNT(multi_languages)&&multi_languages[lo]==language?(int)lo:-1;
}
int reloq_character_locale(int language)
{
    int index=language_index(language);return index<0?-1:multi_locale[index];
}
static int language_available(uint32_t cp,int index)
{
    size_t lo=0,hi=COUNT(multi_character);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(multi_character[m].key<cp)lo=m+1;else hi=m;}
    return lo<COUNT(multi_character)&&multi_character[lo].key==cp&&
        (multi_character[lo].available&(1u<<(unsigned)index))!=0;
}
int reloq_character_native_available_language(uint32_t cp,int language)
{
    if(language==0x40000||language==0x10000)
        return reloq_character_native_available(cp,language==0x10000);
    int index=language_index(language);return index>=0&&language_available(cp,index);
}
static const char *accent_label(uint32_t cp,int index)
{
    switch(cp){
    case 0xa8:cp=0x308;break;case 0xaf:case 0x2c9:cp=0x304;break;
    case 0xb4:cp=0x301;break;case 0xb8:cp=0x327;break;
    case 0x2c6:cp=0x302;break;case 0x2c7:cp=0x30c;break;
    case 0x2d8:cp=0x306;break;case 0x2d9:cp=0x307;break;
    case 0x2da:cp=0x30a;break;case 0x2db:cp=0x328;break;
    case 0x2dc:cp=0x303;break;case 0x2dd:cp=0x30b;break;
    default:break;
    }
    size_t lo=0,hi=COUNT(multi_mark);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(multi_mark[m].key<cp)lo=m+1;else hi=m;}
    if(lo==COUNT(multi_mark)||multi_mark[lo].key!=cp)return NULL;
    uint32_t offset=multi_mark[lo].value[index];
    return offset?(const char *)multi_mark_strings+offset:NULL;
}
static int extension(uint32_t cp,int index,struct reloq_character_extension *out,unsigned depth)
{
    if(depth==4||language_available(cp,index))return 0;
    struct reloq_character_extension r={0};const char *label=accent_label(cp,index);
    if(label){r.accents[r.count++]=label;*out=r;return 1;}
    size_t lo=0,hi=COUNT(multi_latin);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(multi_latin[m].cp<cp)lo=m+1;else hi=m;}
    if(lo==COUNT(multi_latin)||multi_latin[lo].cp!=cp)return 0;
    const struct multi_latin_entry *entry=multi_latin+lo;
    if(language_available(entry->base,index))r.base=entry->base;
    else if(!extension(entry->base,index,&r,depth+1))return 0;
    for(unsigned k=0;k<3&&entry->marks[k];++k){
        label=accent_label(entry->marks[k],index);
        if(!label||r.count==COUNT(r.accents))return 0;
        r.accents[r.count++]=label;
    }
    *out=r;return 1;
}
int reloq_character_extension_read(uint32_t cp,int language,struct reloq_character_extension *out)
{
    if(!out)return -1;
    int index=language_index(language);if(index<0)return -1;
    return extension(cp,index,out,0);
}
int reloq_character_encode_text(const unsigned char *s,size_t n,unsigned char *out,
    uint32_t *scalars,size_t cap,size_t *written,reloq_german_cancel_check cancel,void *ctx)
{
    if(!s||!written||(!out&&cap)||n==SIZE_MAX||n>SIZE_MAX/sizeof(uint32_t)-1)return -1;
    unsigned char *tmp=out?malloc(n+1):NULL;
    uint32_t *ids=scalars?malloc((n+1)*sizeof *ids):NULL;
    if((out&&!tmp)||(scalars&&!ids)){free(tmp);free(ids);return -3;}
    size_t at=0,used=0;int status=-1;
    while(at<n) {
        if(cancel && !(used&255) && cancel(ctx)){status=1;goto done;}
        size_t start=at;uint32_t cp;
        if(reloq_utf8_next(s,n,&at,&cp)!=1)goto done;
        while(at<n) {
            size_t next=at;uint32_t mark;
            if(reloq_utf8_next(s,n,&next,&mark)!=1)goto done;
            uint32_t merged=reloq_unicode_compose(cp,mark);
            if(!merged)break;
            cp=merged;at=next;
        }
        unsigned char byte[5];size_t bytes=0;
        if(reloq_german_input_encode(s+start,at-start,byte,sizeof byte,&bytes,NULL) || bytes!=1) {
            unsigned char base=0;
            if(reloq_character_latin_base(cp,&base)!=1)goto done;
            byte[0]=base;
        }
        if(tmp)tmp[used]=byte[0];
        if(ids)ids[used]=cp;
        ++used;
    }
    if(cancel && cancel(ctx)){status=1;goto done;}
    if((out||scalars) && cap<=used){*written=used;status=-2;goto done;}
    if(tmp){tmp[used]=0;memmove(out,tmp,used+1);}
    if(ids)memmove(scalars,ids,used*sizeof *ids);
    *written=used;status=0;
done:free(tmp);free(ids);return status;
}
