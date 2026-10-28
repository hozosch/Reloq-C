#include "reloq_german_compound_recovery.h"

#include "reloq_german_lexicon.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void expect(const char *input, const char *wanted)
{
    unsigned char spr[1024] = {0};
    size_t length = 0;
    assert(reloq_german_compound_recover(
        NULL, (const unsigned char *)input, strlen(input),
        spr, sizeof spr, &length) == 1);
    assert(length == strlen(wanted));
    assert(strcmp((const char *)spr, wanted) == 0);
    assert(reloq_german_lexicon_validate_spr(spr, length, NULL) == 1);
    /* Two-pass caller contract: no partial group and NUL capacity checked. */
    unsigned char guard[1024];
    memset(guard, 0x53, sizeof guard);
    size_t required = 0;
    assert(reloq_german_compound_recover(
        NULL, (const unsigned char *)input, strlen(input),
        NULL, 0, &required) == -2 && required == length);
    assert(reloq_german_compound_recover(
        NULL, (const unsigned char *)input, strlen(input),
        guard, length, &required) == -2 && required == length);
    for (size_t i = 0; i < sizeof guard; ++i) assert(guard[i] == 0x53);
}

static void untouched(const char *input)
{
    unsigned char spr[96];
    memset(spr, 0x53, sizeof spr);
    size_t length = 999;
    assert(reloq_german_compound_recover(
        NULL, (const unsigned char *)input, strlen(input),
        spr, sizeof spr, &length) == 0);
    assert(length == 999);
    for (size_t i = 0; i < sizeof spr; ++i) assert(spr[i] == 0x53);
}

int main(void)
{
    /* Six independent first roots; one needs a checked -en+s linker. No
     * whole-word output in this family was covered by the prior frontend. */
    expect("Tonqualit\344t", "`[.1tOn.2kva.0li.0tEt]");
    expect("Bildqualit\344t", "`[.1bIlt.2kva.0li.0tEt]");
    expect("Klangqualit\344t", "`[.1klAG.2kva.0li.0tEt]");
    expect("Sprachqualit\344t", "`[.1SprAx.2kva.0li.0tEt]");
    expect("Stimmqualit\344t", "`[.1StIm.2kva.0li.0tEt]");
    expect("Schlafqualit\344t", "`[.1Slaf.2kva.0li.0tEt]");
    expect("Lebensqualit\344t", "`[.1le.0b@ns.2kva.0li.0tEt]");

    /* A covered whole word wins; arbitrary approximate fragments must not
     * move lexical stress to a spurious second component. */
    static const char *const covered[] = {
        "Internet", "Au\337erdem", "Praktikum", "Status",
        "bevorzugen", "Aufgabe", "Unterricht", "Widerstand",
        "Tatsache", "Satzspiegel", "Kontrollfeld", "Lautsprecher"
    };
    for (size_t i = 0; i < sizeof covered / sizeof covered[0]; ++i)
        untouched(covered[i]);
    /* Nested four-syllable heads, poor spelling boundaries and words with
     * no admissible independently checked components are left uncovered. */
    untouched("Tonqualit\344tstest");
    untouched("Plingplongung");
    untouched("Ton");

    size_t unchanged = 77;
    unsigned char spr[4] = {1, 2, 3, 4};
    assert(reloq_german_compound_recover(
        NULL, (const unsigned char *)"Ton qualität", 12,
        spr, sizeof spr, &unchanged) == -1);
    assert(unchanged == 77 && spr[0] == 1);
    puts("German conservative compound recovery OK");
    return 0;
}
