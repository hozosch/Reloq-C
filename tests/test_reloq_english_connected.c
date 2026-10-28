#include "reloq_english_connected.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check(const char *input,const char *expected,int deleted)
{
    unsigned char b[256];size_t positions[256],n=strlen(input);
    memcpy(b,input,n+1);for(size_t i=0;i<=n;++i)positions[i]=100+i;
    size_t erased=0;if(deleted)erased=(size_t)(strrchr(input,'h')-input);
    reloq_english_connected_rules(b,&n,positions);
    assert(!strcmp((char *)b,expected));assert(n==strlen(expected));
    for(size_t i=0;i<=n;++i)
        assert(positions[i]==100+i+(deleted && i>=erased));
    reloq_english_connected_rules(b,&n,positions);
    assert(!strcmp((char *)b,expected));assert(positions[n]==100+strlen(input));
}
int main(void)
{
    /* Whole SPR contrasts from word_final_flap / delete_h_of_funct_word.
     * A pause blocks the rule, and an accented full h-vowel retains h. */
    check("`[.1rEd] `00 `[.0hIm].","`[.1rEF] `00 `[.0Im].",1);
    check("`[.1rid] `00 `[.0hXm].","`[.1riF] `00 `[.0Xm].",1);
    check("`[.1rEd] `00 `[.1hom].","`[.1rEd] `00 `[.1hom].",0);
    check("`[.1nid] `00 `[.0hAv] `[.1fYl].","`[.1nid] `00 `[.0hAv] `[.1fYl].",0);
    check("`[.1rEd], `00 `[.0hIm].","`[.1rEd], `00 `[.0hIm].",0);
    check("`[.1rEd] `[.1Xp.0Xl].","`[.1rEF] `[.1Xp.0Xl].",0);
    /* Native weak that/could -> weak H contrasts do not require a
     * stressed preceding word. Full stressed her after a weak modal does. */
    check("`[.0DXt] `00 `[.0hIm].","`[.0DXF] `00 `[.0Im].",1);
    check("`00 `[.0kUd] `00 `[.0hi] `[.1wRk].","`00 `[.0kUF] `00 `[.0i] `[.1wRk].",1);
    check("`00 `[.0kUd] `00 `[.1hR].","`00 `[.0kUd] `00 `[.1hR].",0);
    puts("US connected speech and source positions passed");return 0;
}
