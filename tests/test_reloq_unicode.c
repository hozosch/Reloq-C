#include "reloq_unicode.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static void check(const char *input,int mode,int en,const char *wanted)
{
    size_t length=strlen(input),n=0,*map=NULL;
    unsigned char *out=NULL,*modes=malloc(length?length:1);
    memset(modes,mode,length);
    assert(!reloq_unicode_admit((const unsigned char *)input,length,modes,en,0,&out,&n,&map));
    if(strcmp((const char *)out,wanted))fprintf(stderr,"Unicode %s => %s; wanted %s\n",input,out,wanted);
    assert(n==strlen(wanted)&&!strcmp((const char *)out,wanted));
    assert(map[0]==0&&map[length]==n);
    for(size_t i=1;i<=length;++i)assert(map[i]>=map[i-1]&&map[i]<=n);
    free(modes);free(out);free(map);
}
int main(void)
{
    check("Hallo😀Welt",0,0,"Hallo Welt");
    check("aαb",0,0,"a Alpha b");check("a\xee\x80\x80" "b",0,0,"a b");
    check("aΩb",0,1,"a omega b");
    check("∰",1,0," Volumenintegral ");
    check("⌘",1,0," Sehenswürdigkeitszeichen ");
    check("⚭",1,0," Ehezeichen ");
    check("Ж",1,0," kyrillisches stimmhaftes Scha ");
    check("ა",1,0," georgisches A ");
    check("შ",1,0," georgisches Scha ");
    check("შ",1,1," Georgian sha ");
    check("Შ",1,1," Georgian sha ");
    check("Ⴘ",1,0," georgisches Scha ");
    check("ⴘ",1,1," Georgian sha ");
    check("Ж",1,1," Cyrillic zha ");
    /* A named base must not mask a longer canonical character. This
     * also covers ECI fragments joined before Unicode admission. */
    check("が",1,0," japanisches Ga in Hiragana ");
    check("が",1,0," japanisches Ga in Hiragana ");
    check("がし",1,1," Japanese gah in hiragana  Japanese shee in hiragana ");
    check("ヴ",1,1," Japanese voo in katakana ");
    check("⊖",1,0," eingekreistes Minuszeichen ");
    check("≪",1,0," sehr viel kleiner als ");
    check("\xe2\x81\xa7",1,0," Isolierung von rechts nach links ");
    /* Missing German entries identify the codepoint. Formal English UCD
     * names remain available only in English. */
    check("\xf0\x93\x80\x80",1,0," Unicode U plus 1 3 0 0 0 ");
    check("\xf0\x93\x80\x80",1,1," egyptian hieroglyph a001 ");
    check("α",1,0," Alpha ");check("α",1,1," alpha ");
    check("μ",1,0," My ");check("μ",1,1," mu ");
    check("ÄÖÜß",0,1,"ÄÖÜß");check("ÄÖÜß",0,0,"ÄÖÜß");
    check("café",0,0,"café");check("øł",0,0,"øł");
    check("Sa\xcc\x83o, ac\xcc\xa7" "a\xcc\x83o",0,2,"São, ação");
    check("Helsinki/Turku - ääni",0,2,"Helsinki/Turku - ääni");
    check("ä+@",1,2," `ts2 ä+@ `ts0 ");
    check("Olá😀mundo",0,2,"Olá mundo");
    check("a`ts2 b",0,2,"a ` ts2 b");
    check("üÜ",0,2,"üÜ"); /* Native Finnish rules own the reading. */
    check("u\xcc\x88",0,2,"ü");
    check("x1️⃣y",0,0,"x y");
    check("１２３",0,0,"123");check("١٢٣",0,0,"123");
    check("é",1,0," `ts2 é `ts0 ");
    check("e\xcc\x81",1,0," `ts2 é `ts0 ");
    check("é",1,1," `[.1i] `2 `[.0x.1kyut] ");check("café",0,1,"café");
    check("a",1,1," `[.1e] ");check("b",1,1," `[.1bi] ");
    check("😀",1,0," grinsendes Gesicht ");
    check("😀",1,1," grinning face ");
    check("👩🏽‍💻",1,0," IT-Expertin: mittlere Hautfarbe ");
    check("\xcd\xb8",1,0," Unicode U plus 0 3 7 8 ");
    check("\xee\x80\x80",1,0," Unicode U plus E 0 0 0 ");
    check("\xe3\x90\x80",1,0," CJK vereinheitlichtes Ideogramm 3400 ");
    check("\xf0\xa9\x9b\xa0",1,0," CJK vereinheitlichtes Ideogramm 296E0 ");
    check("\xf0\xaa\x9b\xa0",1,0," Unicode U plus 2 A 6 E 0 ");
    check("가",1,0," Hangul Silbe ga ");
    check("`",1,0," `[.1gra.0vIs] ");
    check("@",1,0," `ts2 @ `ts0 ");
    check("@",1,1," `2 `[.1At] `0 `[.1sYn] ");
    check("+",1,0," `ts2 + `ts0 ");
    check("1️⃣",1,0," Taste: 1 ");
    const unsigned char bad[]={0xc0,0x80},nul[]={0},mode[]={1};
    unsigned char *out=NULL;size_t n=0,*map=NULL;
    assert(reloq_unicode_admit(bad,sizeof bad,NULL,0,0,&out,&n,&map)==-1);
    assert(!out&&!map);
    assert(!reloq_unicode_admit(nul,1,mode,0,0,&out,&n,&map));
    assert(!strcmp((char *)out," Nullzeichen "));free(out);free(map);
    puts("Native Unicode: CLDR languages, original accents, canonical composition, sequences, named/unnamed boundaries and NUL passed");
    return 0;
}
