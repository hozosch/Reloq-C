#include "reloq_german_best_effort_text.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *annotate(const char *text)
{
    unsigned char *out=NULL;size_t n=0,approximate=0;
    assert(!reloq_german_best_effort_annotate((const unsigned char *)text,
        strlen(text),NULL,&out,&n,&approximate,NULL));
    assert(n==strlen((const char *)out));return out;
}
/* Phrase accent controls ("`2 ") are prosody, checked elsewhere against
 * the reference; the wording tests compare the spoken word sequence. */
static void strip_controls(unsigned char *s)
{
    unsigned char *w=s;
    for (;*s;++s) {
        if (s[0]==0x60 && s[1]>='0' && s[1]<='9') {
            unsigned char *end=s+1;
            while (*end>='0' && *end<='9') ++end;
            if (*end==' ') {s=end;continue;}
        }
        *w++=*s;
    }
    *w=0;
}
static void check(const char *text,const char *body)
{
    unsigned char *out=annotate(text);
    strip_controls(out);
    if (!strstr((const char *)out,body))
        fprintf(stderr,"%s: expected %s, got %s\n",text,body,out);
    assert(strstr((const char *)out,body));free(out);
}
static void same(const char *left,const char *right)
{
    unsigned char *a=annotate(left),*b=annotate(right);
    assert(!strcmp((const char *)a,(const char *)b));free(a);free(b);
}
int main(void)
{
    /* A spelled productive compound and its digits share the phoneme,
     * syllable and numeric onset policy, including the requested vier-r. */
    const char *units[]={"ein","zwei","drei","vier","fünf","sechs","sieben","acht","neun"};
    const char *tens[]={"zwanzig","dreißig","vierzig","fünfzig","sechzig","siebzig","achtzig","neunzig"};
    for (unsigned t=0;t<8;++t) for (unsigned u=0;u<9;++u) {
        char word[64],number[8];
        snprintf(word,sizeof word,"%sund%s",units[u],tens[t]);
        snprintf(number,sizeof number,"%u",(t+2)*10+u+1);
        same(word,number);
    }
    same("FÜNFUNDZWANZIG","25");
    same("Vierundachtzig","84");
    check("vierund","`[.1fi.2rUnt]");
    check("24","`[.1fi.0rUnt.2TvAn.0TIX]");
    check("21","`[.1'aj'n.0Unt.2TvAn.0TIX]");
    check("1000","`[.1'aj'n.2t'aw'.0z@nt]");
    check("1234","`[.1Tv'OE'lf.2hUn.0dRt]");
    check("1.234","`[.1'aj'n.2t'aw'.0z@nt]");
    check("0007","`[.1nUl] `[.1nUl] `[.1nUl] `[.1zi.0b@n]");
    /* Requested policy: decimal digits remain separate, monetary cents
     * form one amount. Currency spelling must not change synthesis input. */
    check("3,14","`[.1dr'aj'] `[.1kO.0mA] `[.1'aj'ns] `[.1fir]");
    const char *money[]={"3,14 EUR","3,14 Euro","3,14 €","3,14€","3,14eur"};
    unsigned char *first=annotate(money[0]);
    {unsigned char *c=annotate(money[0]);strip_controls(c);
     assert(strstr((const char *)c,"`[.1dr'aj'] `[.1'oj'.0ro] `[.1fIr.2Ten]"));free(c);}
    for (size_t i=1;i<sizeof money/sizeof *money;++i) {
        unsigned char *next=annotate(money[i]);
        assert(!strcmp((const char *)first,(const char *)next));free(next);
    }
    free(first);
    check("0,50 EUR","`[.1fYnf.0TIX] `[.1TEnt]");
    check("08.10.2026","`[.1Ax.0tR] `[.0Ok.1to.0bR]");
    check("8:05 Uhr","`[.1Axt] `[.0ur] `[.1fYnf]");
    const char *clocks[]={"8:05","08:05","8:5","08:5","08:05 Uhr",
        "8:05Uhr","8:05 uhr","08.05 Uhr"};
    for(size_t i=0;i<sizeof clocks/sizeof clocks[0];++i) same("8:05 Uhr",clocks[i]);
    same("8:05:03 Uhr","08:5:3");
    same("8:05:03 Uhr","08:05:03");
    check("08:05:03","`[.1Axt] `[.0ur] `[.1fYnf] `[.0Unt]");
    check("08:05:03","`[.1dr'aj'] `[.0ze.1kUn.0d@n]");
    check("01:00","`[.1'aj'n] `[.1ur]");
    check("08:00:00","`[.1nUl] `[.0ze.1kUn.0d@n]");
    check("08:01:01","`[.0ze.1kUn.0d@]");
    check("08:00:03","`[.1Axt] `[.1ur] `[.0Unt] `[.1dr'aj']");
    check("08:01:01","`[.1Axt] `[.0ur] `[.1'aj'ns]");
    {const char *texts[]={"08:05:03","08:01:01","08:00:03","08:00:00",
         "Die Uhr zeigt 8:05:03 Uhr.","Die Uhr zeigt 8:00:03 Uhr."};
     for(size_t i=0;i<sizeof texts/sizeof texts[0];++i) {
         unsigned char *s=annotate(texts[i]);
         assert(!strstr((const char *)s,"mi.1nu.0t"));free(s);
     }}
    {unsigned char *a=annotate("8:05:03"),*b=annotate("8:00:03");
     assert(strstr((const char *)a,"`00 `[.0ur]"));
     assert(strstr((const char *)b,"`[.1ur]"));free(a);free(b);}
    same("Die Zahl 1.234.567,89 EUR.","Die Zahl 1 234 567,89 EUR.");
    same("1.234","1 234");same("1.234","1\xc2\xa0" "234");
    same("1.234.567","1  234 567");
    same("1.234,50 EUR","1 234,50 €");
    same("1.234,56","1 234,56");
    /* Invalid fields and versions cannot emit a partial clock. */
    const char *invalid[]={"25:00","12:60","12:30:60","12:30:45:10","1.2.3"};
    for(size_t i=0;i<sizeof invalid/sizeof invalid[0];++i) {
        unsigned char *s=annotate(invalid[i]);
        assert(!strstr((const char *)s,"[.1ur]"));free(s);
    }
    {unsigned char *s=annotate("1 23");assert(!strstr((const char *)s,"t'aw'.0z@nt"));free(s);}
    check("1/2","`[.0'aj'n.1hAlp]");
    check("-5","`[.1mi.0nUs] `[.1fYnf]");
    check("@","`[.1klA.0mR.2A.0f@]");
    check("#","`[.1nU.0mRn.2T'aj'.0X@n]");
    return 0;
}
