#include "nvda_dictionary.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int capture(void *context, const struct reloq_nvda_entry *entry)
{
    unsigned *seen = context;
    if (*seen == 0) {
        assert(!strcmp(entry->pattern, "NVDA"));
        assert(!strcmp(entry->replacement, "En We De A"));
        assert(entry->case_sensitive == 0 && entry->kind == 2);
    } else if (*seen == 1) {
        assert(!strcmp(entry->pattern, "foo([0-9]+)"));
        assert(!strcmp(entry->replacement, "bar\\1"));
        assert(entry->case_sensitive == 1 && entry->kind == 1);
    } else if (*seen == 2) {
        assert(!strcmp(entry->pattern, "#tag"));
        assert(!strcmp(entry->replacement, "#Markierung"));
        assert(entry->case_sensitive == 0 && entry->kind == 6);
    }
    ++*seen;
    return 1;
}

static int parse(const char *text)
{ return reloq_nvda_parse((const unsigned char *)text, strlen(text), NULL, NULL, NULL); }

int main(void)
{
    static const unsigned char valid[] = "\xef\xbb\xbf# Kommentar\r\n"
        "NVDA\tEn We De A\t0\t2\r\nfoo([0-9]+)\tbar\\1\t1\t1\n"
        "\\#tag\t\\#Markierung\t0\t6\n";
    unsigned seen = 0; size_t count = 0;
    assert(reloq_nvda_parse(valid, sizeof valid - 1, capture, &seen, &count) == RELOQ_NVDA_VALID);
    assert(seen == 3 && count == 3);
    assert(parse("# leer\r\n") == RELOQ_NVDA_VALID);
    assert(parse("Wort\t\t1\t0\n") == RELOQ_NVDA_VALID);
    assert(parse("Wort\tErsatz\t0\t3\n") == RELOQ_NVDA_VALID);
    assert(parse("Wort\tErsatz\t0\t4\n") == RELOQ_NVDA_VALID);
    assert(parse("Wort\tErsatz\t0\t5\n") == RELOQ_NVDA_VALID);
    assert(parse("native\tvalue\n") == RELOQ_NVDA_INVALID);
    assert(parse("native\tvalue\nWort\tErsatz\t0\t2\n") == RELOQ_NVDA_INVALID);
    const char mixed[] = "NVDA\tErsatz\t0\t2\nnative\tvalue\n";
    seen = 0;
    assert(reloq_nvda_parse((const unsigned char *)mixed, sizeof mixed - 1,
                           capture, &seen, NULL) == RELOQ_NVDA_INVALID);
    assert(seen == 0);
    assert(parse("Wort\tErsatz\t2\t2\n") == RELOQ_NVDA_INVALID);
    assert(parse("Wort\tErsatz\t0\t7\n") == RELOQ_NVDA_INVALID);
    assert(parse("Wort\tErsatz\t0\t99\n") == RELOQ_NVDA_INVALID);
    assert(parse("Wort\tErsatz\t0\t2\textra\n") == RELOQ_NVDA_INVALID);
    assert(parse("\xff\tErsatz\t0\t2\n") == RELOQ_NVDA_INVALID);
    static const unsigned char embedded_nul[] = { 'x', 0, '\t', 'y', '\t', '0', '\t', '0' };
    assert(reloq_nvda_parse(embedded_nul, sizeof embedded_nul, NULL, NULL, NULL) == RELOQ_NVDA_INVALID);
    puts("NVDA dictionary file parsing passed");
    return 0;
}
