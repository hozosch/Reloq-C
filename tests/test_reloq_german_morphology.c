#include "reloq_german_morphology.h"
#include "reloq_german_lexicon.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define ENTRY(k,s) {(const unsigned char *)(k),sizeof(k)-1,(const unsigned char *)(s),sizeof(s)-1}
static void expect(const struct reloq_lexicon *lexicon,const char *word,const char *spr)
{
    unsigned char buffer[128],before[128];size_t n=999;
    memset(buffer,0xa5,sizeof buffer);memcpy(before,buffer,sizeof buffer);
    assert(reloq_german_morphology_word(lexicon,(const unsigned char *)word,
        strlen(word),buffer,1,&n)==-2);
    assert(n==strlen(spr) && !memcmp(buffer,before,sizeof buffer));
    assert(reloq_german_morphology_word(lexicon,(const unsigned char *)word,
        strlen(word),buffer,sizeof buffer,&n)==1);
    assert(n==strlen(spr) && !strcmp((const char *)buffer,spr));
}
int main(void)
{
    const struct reloq_lexicon_override entries[]={
        ENTRY("buch","`[.1bux]"), ENTRY("computer","`[.0kOm.1pju.0tR]"),
        ENTRY("geben","`[.1ge.0b@n]"), ENTRY("grob","`[.1grop]"),
        ENTRY("haus","`[.1h'aw's]"), ENTRY("holz","`[.1hOlT]"),
        ENTRY("klein","`[.1kl'aj'n]"),
        ENTRY("licht","`[.1lIXt]"), ENTRY("sicher","`[.1zI.0XR]")
    };
    struct reloq_lexicon lexicon;
    const size_t count=sizeof entries/sizeof entries[0];
    assert(!reloq_lexicon_bind(&lexicon,NULL,0,entries,count,reloq_german_lexicon_validate_spr,NULL));
    expect(&lexicon,"Holzhaus","`[.1hOlT.2h'aw's]");
    expect(&lexicon,"Holzhausbuch","`[.1hOlT.2h'aw's.2bux]");
    expect(&lexicon,"HOLZBUCH","`[.1hOlT.2bux]");
    expect(&lexicon,"Holzlicht","`[.1hOlT.0lIXt]");
    expect(&lexicon,"aufgeben","`[.1'aw'f.2ge.0b@n]");
    expect(&lexicon,"aufzugeben","`[.1'aw'f.0Tu.2ge.0b@n]");
    expect(&lexicon,"kleinem","`[.1kl'aj'.0n@m]");
    expect(&lexicon,"kleiner","`[.1kl'aj'.0nR]");
    expect(&lexicon,"kleinsten","`[.1kl'aj'n.0st@n]");
    expect(&lexicon,"kleinst","`[.1kl'aj'nst]");
    expect(&lexicon,"sicherste","`[.1zI.0XR.0st@]");
    expect(&lexicon,"kleineren","`[.1kl'aj'.0n@.0r@n]");
    expect(&lexicon,"gebenden","`[.1ge.0b@n.0d@n]");
    expect(&lexicon,"vergebendes","`[.0fEr.1ge.0b@n.0d@s]");
    expect(&lexicon,"grobe","`[.1gro.0b@]");
    expect(&lexicon,"groben","`[.1gro.0b@n]");
    expect(&lexicon,"Hauses","`[.1h'aw'.0z@s]");
    expect(&lexicon,"Computern","`[.0kOm.1pju.0tRn]");
    expect(&lexicon,"Computers","`[.0kOm.1pju.0tRs]");
    expect(&lexicon,"Holzhauses","`[.1hOlT.2h'aw'.0z@s]");
    expect(&lexicon,"gesichert","`[.0g@.1zI.0XRt]");
    expect(&lexicon,"gesicherten","`[.0g@.1zI.0XR.0t@n]");
    expect(&lexicon,"Versicherung","`[.0fEr.1zI.0X@.0rUG]");
    expect(&lexicon,"ungesichert","`[.1Un.0g@.2zI.0XRt]");
    {
        const struct reloq_lexicon_override stems[]={
            ENTRY("abteilen","`[.0Ap.1t'aj'.0l@n]"),
            ENTRY("freudig","`[.1fr'oj'.0dIX]"),
            ENTRY("leisten","`[.1l'aj'.0st@n]"),
            ENTRY("ruhig","`[.1ru.0IX]")
        };
        struct reloq_lexicon family;
        assert(!reloq_lexicon_bind(&family,NULL,0,stems,4,reloq_german_lexicon_validate_spr,NULL));
        expect(&family,"ruhigen","`[.1ru.0I.0g@n]");
        expect(&family,"ruhiges","`[.1ru.0I.0g@s]");
        expect(&family,"freudiger","`[.1fr'oj'.0dI.0gR]");
        expect(&family,"Leistungen","`[.1l'aj'.0stUG.0@n]");
        expect(&family,"Abteilung","`[.0Ap.1t'aj'.0lUG]");
    }
    {
        const struct reloq_lexicon_override stems[]={ENTRY("dunkel","`[.1dUG.0k@l]")};
        struct reloq_lexicon family;
        assert(!reloq_lexicon_bind(&family,NULL,0,stems,1,reloq_german_lexicon_validate_spr,NULL));
        expect(&family,"dunklen","`[.1dUG.0kl@n]");
        expect(&family,"dunkler","`[.1dUG.0klR]");
    }
    unsigned char buffer[128]={0x55};size_t n=77;
    {
        const struct reloq_lexicon_override stems[]={
            ENTRY("bruch","`[.1brUx]"), ENTRY("frisch","`[.1frIS]")
        };
        struct reloq_lexicon family;
        assert(!reloq_lexicon_bind(&family,NULL,0,stems,2,reloq_german_lexicon_validate_spr,NULL));
        expect(&family,"Abbruch","`[.1Ap.2brUx]");
        expect(&family,"frischeren","`[.1frI.0S@.0r@n]");
        /* The new particle class is not permission for infinitival -zu-. */
        assert(reloq_german_morphology_word(&family,
            (const unsigned char *)"abzubruch",9,buffer,sizeof buffer,&n)==0);
        struct reloq_lexicon_override altered[]={
            ENTRY("bruch","`[.1brux]"), ENTRY("frisch","`[.1friS]")
        };
        assert(!reloq_lexicon_bind(&family,NULL,0,altered,2,reloq_german_lexicon_validate_spr,NULL));
        assert(reloq_german_morphology_word(&family,
            (const unsigned char *)"abbruch",7,buffer,sizeof buffer,&n)==0);
        assert(reloq_german_morphology_word(&family,
            (const unsigned char *)"frischeren",10,buffer,sizeof buffer,&n)==0);
    }
    {
        unsigned char s[128]="`[.0nA.1tyr.0lIX]",copy[128];
        const unsigned char word[]="nat\xfc" "rlichen";
        size_t used=0;
        memcpy(copy,s,sizeof s);
        assert(reloq_german_adjective_base(word,sizeof word-1)==9);
        assert(reloq_german_adjective_form(word,sizeof word-1,s,strlen((char *)s),
            s,4,&used)==-2 && !memcmp(s,copy,sizeof s));
        assert(used==strlen("`[.0nA.1tyr.0lI.0X@n]"));
        assert(reloq_german_adjective_form(word,sizeof word-1,s,strlen((char *)s),
            s,sizeof s,&used)==1);
        assert(!strcmp((char *)s,"`[.0nA.1tyr.0lI.0X@n]"));
        const struct reloq_lexicon_override stems[]={
            ENTRY("nat\xfc" "rlich","`[.0nA.1tyr.0lIX]"),
            ENTRY("systematisch","`[.0zY.0ste.1ma.0tIS]")
        };
        struct reloq_lexicon family;
        assert(!reloq_lexicon_bind(&family,NULL,0,stems,2,reloq_german_lexicon_validate_spr,NULL));
        expect(&family,"unnat\xfc" "rlichen","`[.1Un.0nA.2tyr.0lI.0X@n]");
        expect(&family,"systematische","`[.0zY.0ste.1ma.0tI.0S@]");
        expect(&family,"Nat\xfc" "rlichkeit","`[.0nA.1tyr.0lIX.0k'aj't]");
    }
    assert(reloq_german_morphology_word(&lexicon,(const unsigned char *)"Holzzzhaus",10,buffer,sizeof buffer,&n)==0);
    assert(n==77 && buffer[0]==0x55);
    /* The role belongs to an exact observed pronunciation, never an
     * arbitrary caller override sharing the same spelling. */
    struct reloq_lexicon_override changed[sizeof entries/sizeof entries[0]];
    memcpy(changed,entries,sizeof changed);
    changed[5].spr=(const unsigned char *)"`[.1hOlz]";
    changed[3].spr=(const unsigned char *)"`[.1grOp]";
    changed[count-1].spr=(const unsigned char *)"`[.1zi.0XR]";
    assert(!reloq_lexicon_bind(&lexicon,NULL,0,changed,count,reloq_german_lexicon_validate_spr,NULL));
    assert(reloq_german_morphology_word(&lexicon,(const unsigned char *)"Holzhaus",8,buffer,sizeof buffer,&n)==0);
    assert(n==77 && buffer[0]==0x55);
    assert(reloq_german_morphology_word(&lexicon,(const unsigned char *)"grobe",5,buffer,sizeof buffer,&n)==0);
    assert(reloq_german_morphology_word(&lexicon,(const unsigned char *)"gesichert",9,buffer,sizeof buffer,&n)==0);
    assert(reloq_german_morphology_word(NULL,(const unsigned char *)"Holzhaus",8,buffer,sizeof buffer,&n)==0);
    {
        static const struct {const char *word,*before,*after;} repairs[]={
            {"konzentrieren","`[.1kOn.0T@n.2tri.0r@n]","`[.0kOn.0TEn.1tri.0r@n]"},
            {"KONZENTRIERTEST","`[.1kOn.0T@n.2trir.0t@st]","`[.0kOn.0TEn.1trir.0t@st]"},
            {"unkonzentriert","`[.1Un.2kOn.0T@n.2trirt]","`[.1Un.0kOn.0TEn.2trirt]"},
            {"aufkonzentrierenden","`[.1'aw'f.2kOn.0T@n.2tri.0r@n.0d@n]","`[.1'aw'f.0kOn.0TEn.2tri.0r@n.0d@n]"},
            {"Konzentration","`[.2kOn.0TEn.0trA.1Tjon]","`[.2kOn.0TEn.0trA.1Tjon]"},
            {"zentrieren","`[.0TEn.1tri.0r@n]","`[.0TEn.1tri.0r@n]"},
            {"konzentrieren","`[.0kOn.0TEn.1tri.0r@n]","`[.0kOn.0TEn.1tri.0r@n]"},
            {"anderes","`[.1kOn.0T@n.2tri.0r@n]","`[.1kOn.0T@n.2tri.0r@n]"}
        };
        for(size_t i=0;i<sizeof repairs/sizeof *repairs;++i) {
            unsigned char repaired[128];strcpy((char *)repaired,repairs[i].before);
            reloq_german_morphology_repair_stem((const unsigned char *)repairs[i].word,
                strlen(repairs[i].word),repaired,strlen((char *)repaired));
            assert(!strcmp((char *)repaired,repairs[i].after));
        }
    }
    puts("dictionary-backed morphology: component stress, prefixes, capacity and override isolation passed");
    return 0;
}
