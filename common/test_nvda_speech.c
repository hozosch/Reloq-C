#include "nvda_speech.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void check(struct reloq_nvda_speech *d,const char *text,const char *expected,int match)
{
    char *out=NULL;size_t *map=NULL,length=0;int matched=0;
    assert(reloq_nvda_speech_apply(d,text,strlen(text),&out,&length,&map,&matched));
    assert(length==strlen(expected)&&!strcmp(out,expected)&&matched==match);
    assert(map[0]<=length&&map[strlen(text)]<=length);
    for(size_t i=1;i<=strlen(text);++i)assert(map[i]>=map[i-1]&&map[i]<=length);
    free(out);free(map);
}
int main(void)
{
    const char data[]="\xef\xbb\xbf# comment\n"
        "youtube\tbanana\t0\t0\n"
        "literalprobe\t\\\\1\t1\t0\n"
        "(?P<stem>regprobe)([0-9]+)\t\\g<stem>-\\2\t1\t1\n"
        "wholeprobe\tWeg\t0\t2\n"
        "teilprobe\tTeil\t1\t3\n"
        "anfangprobe\tAnfang\t1\t4\n"
        "endeprobe\tEnde\t1\t5\n"
        "wild?card\tPlatzhalter\t1\t6\n"
        "\\#zeichen\t\\#Ersatz\t1\t0\n";
    struct reloq_nvda_speech *d=reloq_nvda_speech_new();assert(d);
    assert(reloq_nvda_speech_add(d,(const unsigned char *)data,sizeof data-1));
    check(d,"YouTube","banana",1);check(d,"literalprobe","\\\\1",1);
    check(d,"regprobe42","regprobe-42",1);check(d,"WHOLEPROBE","Weg",1);
    check(d,"xwholeprobe","xwholeprobe",0);
    check(d,"teilprobe","teilprobe",0);check(d,"xteilprobe","xTeil",1);
    check(d,"anfangprobe","anfangprobe",0);check(d,"anfangprobex","Anfangx",1);
    check(d,"endeprobe","endeprobe",0);check(d,"xendeprobe","xEnde",1);
    check(d,"wildXcard","Platzhalter",1);check(d,"#zeichen","#Ersatz",1);
    check(d,"Vor YouTube danach","Vor banana danach",1);
    const char invalid[]="good\tbad\t0\t0\n(\tvalue\t1\t1\n";
    assert(!reloq_nvda_speech_add(d,(const unsigned char *)invalid,sizeof invalid-1));
    check(d,"good","good",0);
    const char invalid_ref[]="x\t\\1\t1\t1\n";
    assert(!reloq_nvda_speech_add(d,(const unsigned char *)invalid_ref,sizeof invalid_ref-1));
    reloq_nvda_speech_free(d);
    d=reloq_nvda_speech_new();assert(d);
    const char zero[]="(?=o)\tX\t1\t1\n";
    assert(reloq_nvda_speech_add(d,(const unsigned char *)zero,sizeof zero-1));
    check(d,"foo","fXoXo",1);reloq_nvda_speech_free(d);
    d=reloq_nvda_speech_new();assert(d);
    const char unicode[]="ä?e\tWort\t0\t6\n";
    assert(reloq_nvda_speech_add(d,(const unsigned char *)unicode,sizeof unicode-1));
    check(d,"Äbe","Wort",1);reloq_nvda_speech_free(d);
    d=reloq_nvda_speech_new();assert(d);
    const char escaped[]="\\u00e4\\Z\tVokal\t1\t1\n";
    assert(reloq_nvda_speech_add(d,(const unsigned char *)escaped,sizeof escaped-1));
    check(d,"ä","Vokal",1);check(d,"ä\n","ä\n",0);reloq_nvda_speech_free(d);
    d=reloq_nvda_speech_new();assert(d);
    const char slow[]="(*NO_AUTO_POSSESS)^(a+)+$\tX\t1\t1\n";
    assert(reloq_nvda_speech_add(d,(const unsigned char *)slow,sizeof slow-1));
    char text[4002];memset(text,'a',4000);text[4000]='b';text[4001]=0;
    check(d,text,text,0);reloq_nvda_speech_free(d);
    puts("NVDA speech replacement: entry types, captures, validation and source offsets passed");
    return 0;
}
