/* Exact public-reference SPR groups for orthographic abbreviations, not
 * speculative full-engine equivalence.  Their word-boundary controls vary
 * with sentence context and are intentionally checked separately. */
#include "reloq_german_best_effort_text.h"
#include "reloq_german_annotation.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void check(const char *input, const char *const *groups, size_t group_count)
{
    unsigned char *annotation=NULL;
    size_t length=0, approximate=100;
    int status=reloq_german_best_effort_annotate(
        (const unsigned char *)input,strlen(input),NULL,
        &annotation,&length,&approximate,NULL);
    assert(status==0);
    size_t seen=0;
    struct reloq_german_annotation_cursor cursor={0};
    struct reloq_german_annotation_event event;
    while ((status=reloq_german_annotation_next(
        annotation,length,&cursor,0,&event))==1) {
        if (event.kind==RELOQ_DE_ANNOTATION_SPR &&
            event.spr.kind==RELOQ_DE_SPR_WORD_OPEN) {
            assert(seen<group_count);
            const char *expected=groups[seen];
            size_t at=event.offset;
            size_t n=strlen(expected);
            assert(at+n<=length);
            assert(!memcmp(annotation+at,expected,n));
            ++seen;
        }
    }
    assert(!status && seen==group_count);
    free(annotation);
}
int main(void)
{
    static const char *const zb[]={"`[.0TUm]", "`[.1b'aj'.2Spil]"};
    static const char *const zz[]={"`[.0Tur]", "`[.1T'aj't]"};
    static const char *const dh[]={"`[.0dAs]", "`[.1h'aj'st]"};
    static const char *const usw[]={"`[.0Unt]", "`[.0zo]", "`[.1v'aj'.0tR]"};
    static const char *const mow[]={"`[.1mer]", "`[.1o.0dR]", "`[.1ve.0nI.0gR]"};
    static const char *const dr[]={"`[.1dOk.0tor.2In.0Ze.2nj'oe'r]"};
    static const char *const email[]={"`[.1i.2mel]"};
    static const char *const km[]={"`[.2ki.0lo.1me.0tR]", "`[.0pro]", "`[.1StUn.0d@]"};
    check("z.B",zb,2);
    check("z. B.",zb,2);
    check("z.B.",zb,2);
    check("z.Z",zz,2);
    check("d.h",dh,2);
    check("usw",usw,3);
    check("u.s.w",usw,3);
    check("m.o.w",mow,3);
    check("Dr.-Ing",dr,1);
    check("E-Mail",email,1);
    check("km/h",km,3);
    puts("Measured reference abbreviations: OK");
    return 0;
}
