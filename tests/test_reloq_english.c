#include "reloq_english.h"
#include "reloq_english_voice.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void text_case(const char *text,const char *expected,size_t spelling)
{
    unsigned char *spr=NULL;size_t n=0,words=0;
    assert(!reloq_english_text_annotation((const unsigned char *)text,strlen(text),&spr,&n,&words));
    assert(words==spelling && n==strlen(expected) && !strcmp((char *)spr,expected));
    free(spr);
}

int main(void)
{
    text_case("quietness","`[.1kwY.0Xt.0nXs]",0);
    text_case("clearness","`[.1klir.0nXs]",0);
    text_case("quickly","`[.1kwI.0kli]",0);
    text_case("safely","`[.1se.0fli]",0);
    text_case("hoping","`[.1ho.0pIG]",0);
    text_case("hopping","`[.1ha.0pIG]",0);
    text_case("hated","`[.1he.0FXd]",0);
    text_case("books","`[.1bUks]",0);
    text_case("dogs","`[.1dcgz]",0);
    text_case("churches","`[.1CR.0CXz]",0);
    text_case("00123","`[.1zir.0o] `[.1zir.0o] `[.1wHn] `[.1tu] `[.1Tri] ",1);
    text_case("2026","`0 `[.1twEn.0ti] `0 `[.1twEn.0ti] `2 `[.1sIks]",0);
    text_case("123456789012","`[.1wHn] `[.1hHn.0drXd] `[.1twEn.0ti] `2 `[.1Tri] `[.1bIl.0yXn] `[.1fcr] `[.1hHn.0drXd] `[.1fIf.0ti] `2 `[.1sIks] `[.1mIl.0yXn] `[.1sE.0vXn] `[.1hHn.0drXF] `[.1e.0Fi] `2 `[.1nYn] `[.1TW.0zXnd] `[.1twElv] ",0);
    const char *solo=NULL,*vowel=NULL,*consonant=NULL;
    assert(reloq_english_word((const unsigned char *)"the",3,
        &solo,&vowel,&consonant)==RELOQ_ENGLISH_OK);
    assert(!strcmp(solo,"`[.1DH]"));
    assert(!strcmp(vowel,"`00 `[.0Di]"));
    assert(!strcmp(consonant,"`00 `[.0DX]"));
    assert(reloq_english_word((const unsigned char *)"qzxqzx",6,
        &solo,&vowel,&consonant)==RELOQ_ENGLISH_UNCOVERED);
    struct reloq_english_phone *phones=NULL;size_t count=0,error=0;
    const char *button="`[.1bH.0?N]";
    assert(!reloq_english_parse((const unsigned char *)button,strlen(button),
        &phones,&count,&error));
    assert(count==4 && phones[3].phone=='n' && phones[3].syllabic &&
        phones[3].stress==0 && phones[0].stress==1);
    free(phones);phones=NULL;
    const char *weak="`00 `[.0Di] `[.1wRld]";
    assert(!reloq_english_parse((const unsigned char *)weak,strlen(weak),
        &phones,&count,&error));
    assert(count==6 && phones[0].weak && phones[1].weak && !phones[2].weak &&
        phones[2].word==1 && phones[2].phone=='w');
    assert(phones[0].accent==-1 && phones[2].accent==1);
    free(phones);phones=NULL;
    const char *bad[]={"`[.1]","`[.1a.0]","`[.1a", "`[.1@]","`[.1a.0.1i]"};
    for(size_t i=0;i<sizeof bad/sizeof *bad;++i) {
        count=99;
        assert(reloq_english_parse((const unsigned char *)bad[i],strlen(bad[i]),
            &phones,&count,&error)==RELOQ_ENGLISH_INVALID);
        assert(!phones && count==99);
    }
    const unsigned char nul[]={'`','[','.','1','a',']',0};
    assert(reloq_english_parse(nul,sizeof nul,&phones,&count,&error)==RELOQ_ENGLISH_INVALID);
    return 0;
}
