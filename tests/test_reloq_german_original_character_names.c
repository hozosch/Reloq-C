/* Original public German SPR observations for single encoded characters.
 * These verify that the plain-text frontend keeps character naming separate
 * from the foreign-loanword fold and ordinary German vowel processing. */
#include "reloq_german_best_effort_text.h"
#include "reloq_german_annotation.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void check(const char *text,const char *const expected[],size_t count)
{
    unsigned char *annot=NULL;
    size_t n=0,approx=0,seen=0;
    int rc=reloq_german_best_effort_annotate(
        (const unsigned char *)text,strlen(text),NULL,&annot,&n,&approx,NULL);
    assert(rc==0 && annot);
    struct reloq_german_annotation_cursor cursor={0};
    struct reloq_german_annotation_event event;
    while ((rc=reloq_german_annotation_next(annot,n,&cursor,0,&event))==1) {
        if (event.kind!=RELOQ_DE_ANNOTATION_SPR ||
            event.spr.kind!=RELOQ_DE_SPR_WORD_OPEN) continue;
        assert(seen<count);
        size_t l=strlen(expected[seen]);
        assert(l<=n-event.offset);
        assert(memcmp(annot+event.offset,expected[seen],l)==0);
        ++seen;
    }
    assert(rc==0 && seen==count);
    free(annot);
}

#define SINGLE(in,spr) do {const char *a[]={"`[" spr "]"};check(in,a,1);}while(0)
#define DOUBLE(in,a,b) do {const char *v[]={"`[" a "]", "`[" b "]"};check(in,v,2);}while(0)
int main(void)
{
    DOUBLE("À",".1a",".1gra.0vIs");
    DOUBLE("é",".1e",".0A.1kut");
    DOUBLE("ê",".1e",".1TIr.0kUm.2flEks");
    DOUBLE("ë",".1e",".1tre.0mA");
    DOUBLE("ï",".1i",".1tre.0mA");
    DOUBLE("ñ",".1En",".1tIl.0d@");
    DOUBLE("Ç",".1Te",".0se.1di.0@");
    DOUBLE("œ",".1o",".1e");
    DOUBLE("ø",".1o",".1Sr'E:'k.2StrIX");
    SINGLE("ä",".1'E:'");
    SINGLE("Ö",".1'oe'");
    SINGLE("½",".0'aj'n.1hAlp");
    DOUBLE("±",".1plUs",".1mi.0nUs");
    SINGLE("¨",".1tre.0mA");
    SINGLE("¸",".0se.1di.0@");
    DOUBLE("¤",".2In.0tEr.0nA.0Tjo.1na.0l@s",".1v'E:'.0rUGs.2T'aj'.0X@n");
    DOUBLE("¦",".1zEGk.2rEX.0tR",".1StrIX");
    DOUBLE("¬",".1nIXt",".0zOr.1tir.2T'aj'.0X@n");
    SINGLE("¶",".2pA.0rA.1graf");
    DOUBLE("¹",".1hox.0g@.2StEl.0t@",".1'aj'ns");
    DOUBLE("¿",".1Um.0g@.2ker.0t@s",".1fra.0g@.2T'aj'.0X@n");
    puts("Measured original German character names: OK");
    return 0;
}
