#include "reloq_german_cvc_lexicon.h"
#include "reloq_german_lexicon.h"
#include "reloq_german_text.h"

#include <string.h>

#define CHECK(test) do { if (!(test)) return __LINE__; } while (0)

static int apple_word(const char *word, const char *spr)
{
    unsigned char out[80] = "untouched";
    size_t length = 999;
    CHECK(reloq_german_lexicon_word(NULL,
                                    (const unsigned char *)word,
                                    strlen(word), out, sizeof out,
                                    &length) == 1);
    CHECK(length == strlen(spr) && strcmp((const char *)out, spr) == 0);
    return 0;
}

static int sentence(const char *text, const char *annotation)
{
    unsigned char out[80] = "untouched";
    size_t length = 999;
    CHECK(reloq_german_text_annotate((const unsigned char *)text,
                                      strlen(text), out, sizeof out,
                                      &length, NULL) == RELOQ_DE_TEXT_OK);
    CHECK(length == strlen(annotation) &&
          strcmp((const char *)out, annotation) == 0);
    return 0;
}

int main(void)
{
    CHECK(reloq_german_cvc_reference_count() == 598);
    const struct reloq_lexicon_override *entry = NULL;
    CHECK(reloq_german_cvc_reference_lookup((const unsigned char *)"ok", 2,
                                              &entry) == 0);
    CHECK(reloq_german_cvc_reference_lookup((const unsigned char *)"Ab", 2,
                                              &entry) == 1 && entry &&
          entry->spr_length == strlen("`[.1Ap]"));
    CHECK(reloq_german_cvc_reference_lookup((const unsigned char *)"", 0,
                                              &entry) == -1);

    /* Vowel length is lexical: the same written class includes long and
     * short reference vowels. These values are public Apple-SPR observations
     * over the complete grid, not hardcoded branches in the G2P function. */
    CHECK(apple_word("Hut", "`[.1hut]") == 0);
    CHECK(apple_word("hat", "`[.1hAt]") == 0);
    CHECK(apple_word("Hip", "`[.1hip]") == 0);
    CHECK(apple_word("Ab", "`[.1Ap]") == 0);
    CHECK(apple_word("Hap", "`[.1hap]") == 0);
    CHECK(apple_word("Hat", "`[.1hAt]") == 0);
    CHECK(apple_word("Ap", "`[.1ap]") == 0);
    /* The same reference-only Cartesian class now includes b/d/g onsets.
     * Lexical vowel length still comes from the public SPR callback. */
    CHECK(apple_word("Bad", "`[.1bat]") == 0);
    CHECK(apple_word("bad", "`[.1bat]") == 0);
    CHECK(apple_word("Gib", "`[.1gIp]") == 0);
    CHECK(apple_word("Geb", "`[.1gep]") == 0);
    CHECK(apple_word("Dad", "`[.1dat]") == 0);
    /* Public m/n-CVC observations preserve lexical length and coda
     * devoicing without introducing spelling-specific G2P branches. */
    CHECK(apple_word("Mut", "`[.1mut]") == 0);
    CHECK(apple_word("mut", "`[.1mut]") == 0);
    CHECK(apple_word("Not", "`[.1not]") == 0);
    CHECK(apple_word("Mib", "`[.1mip]") == 0);
    CHECK(apple_word("Nib", "`[.1nip]") == 0);
    CHECK(sentence("Hut.", "`[.1hut].") == 0);
    CHECK(sentence("hat.", "`[.1hAt].") == 0);
    CHECK(sentence("Hip.", "`[.1hip].") == 0);
    CHECK(sentence("Ab.", "`[.1Ap].") == 0);
    CHECK(sentence("Bad.", "`[.1bat].") == 0);
    CHECK(sentence("Bett.", "`[.1bEt].") == 0);
    CHECK(sentence("Deck.", "`[.1dEk].") == 0);
    CHECK(sentence("Guck.", "`[.1gUk].") == 0);
    CHECK(sentence("Mett.", "`[.1mEt].") == 0);
    CHECK(sentence("Muck.", "`[.1mUk].") == 0);
    CHECK(sentence("Mut.", "`[.1mut].") == 0);
    CHECK(sentence("Nett.", "`[.1nEt].") == 0);
    CHECK(sentence("Nipp.", "`[.1nIp].") == 0);
    CHECK(sentence("Nock.", "`[.1nOk].") == 0);
    CHECK(sentence("Not.", "`[.1not].") == 0);

    /* A caller's validated pronunciation takes priority over generated
     * reference data; CVC data never authorizes an unrelated unknown word. */
    const unsigned char key[] = "hat";
    const unsigned char override[] = "`[.1hat]";
    const struct reloq_lexicon_override row = {
        key, sizeof key - 1, override, sizeof override - 1
    };
    struct reloq_lexicon caller = {0};
    CHECK(reloq_lexicon_bind(&caller, NULL, 0, &row, 1,
                             reloq_german_lexicon_validate_spr, NULL) == 0);
    unsigned char out[80] = "untouched";
    size_t length = 999;
    CHECK(reloq_german_lexicon_word(&caller, key, 3, out,
                                    sizeof out, &length) == 1 &&
          strcmp((const char *)out, "`[.1hat]") == 0);
    strcpy((char *)out, "untouched");
    length = 999;
    CHECK(reloq_german_lexicon_word(NULL, key, 3, out,
                                    strlen("`[.1hAt]"), &length) == -2 &&
          length == strlen("`[.1hAt]") &&
          strcmp((const char *)out, "untouched") == 0);
    strcpy((char *)out, "untouched");
    length = 999;
    CHECK(reloq_german_lexicon_word(NULL, (const unsigned char *)"Bach", 4,
                                    out, sizeof out, &length) == 0 &&
          length == 999 && strcmp((const char *)out, "untouched") == 0);
    return 0;
}
