#include "reloq_german_lexicon.h"

#include "reloq_german_cvc_lexicon.h"
#include "reloq_german_g2p.h"
#include "reloq_german_morphology.h"
#include "reloq_german_root_plan.h"
#include "reloq_german_spr.h"

#include <string.h>

static int german_word(const unsigned char *word, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        unsigned char c = word[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              c == 0xc4 || c == 0xd6 || c == 0xdc || c == 0xe4 ||
              c == 0xf6 || c == 0xfc || c == 0xdf))
            return 0;
    }
    return 1;
}

int reloq_german_lexicon_validate_spr(const unsigned char *spr,
                                       size_t length, void *context)
{
    (void)context;
    if (!spr || length < 3 || spr[0] != '`' || spr[1] != '[')
        return 0;
    struct reloq_german_spr_cursor cursor = {0};
    struct reloq_german_spr_event event;
    int state;
    while ((state = reloq_german_spr_next(spr, length,
                                          &cursor, &event)) == 1) {}
    return state == 0;
}

static int copy_pronunciation(const struct reloq_lexicon_override *entry,
                              unsigned char *output, size_t capacity,
                              size_t *byte_length)
{
    /* Caller-owned entries may come from a different language; all paths
     * pass the same German structural validator before copying. */
    if (!reloq_german_lexicon_validate_spr(entry->spr,
                                            entry->spr_length, NULL))
        return -1;
    if (!output || capacity <= entry->spr_length) {
        *byte_length = entry->spr_length;
        return -2;
    }
    memmove(output, entry->spr, entry->spr_length);
    output[entry->spr_length] = 0;
    *byte_length = entry->spr_length;
    return 1;
}

int reloq_german_lexicon_word(const struct reloq_lexicon *view,
                              const unsigned char *word, size_t word_length,
                              unsigned char *output, size_t capacity,
                              size_t *byte_length)
{
    if (!word || !word_length || !byte_length || (!output && capacity) ||
        !german_word(word, word_length))
        return -1;
    if (view) {
        const struct reloq_lexicon_override *entry = NULL;
        int state = reloq_lexicon_find_override(view, word, word_length,
                                                 &entry);
        if (state < 0) return -1;
        if (state == 1)
            return copy_pronunciation(entry, output, capacity, byte_length);
    }
    const struct reloq_lexicon_override *reference = NULL;
    /* Root readings independently obtained from deu.so. Their productive
     * inflections and compounds still run through the native morphology. */
    static const struct {const char *word,*spr;} roots82[]={
        {"kontext","`[.0kOn.1tEkst]"},
        {"pers\366nlich","`[.0pEr.1z'oe'n.0lIX]"},
#include "reloq_german_roots85.inc"
    };
    for(size_t k=0;k<sizeof roots82/sizeof *roots82;++k) {
        if(strlen(roots82[k].word)!=word_length)continue;
        size_t j=0;
        for(;j<word_length;++j) {
            unsigned char c=word[j];if(c>='A'&&c<='Z')c+=32;
            if(c==0xc4||c==0xd6||c==0xdc)c+=32;
            if(c!=(unsigned char)roots82[k].word[j])break;
        }
        if(j==word_length) {
            struct reloq_lexicon_override e={word,word_length,
                (const unsigned char *)roots82[k].spr,strlen(roots82[k].spr)};
            return copy_pronunciation(&e,output,capacity,byte_length);
        }
    }
    int state = reloq_german_cvc_reference_lookup(word, word_length,
                                                    &reference);
    if (state < 0) return -1;
    if (state == 1)
        return copy_pronunciation(reference, output, capacity, byte_length);
    state = reloq_german_morphology_word(view, word, word_length,
        output, capacity, byte_length);
    if (state) return state;
    /* A measured final-root class can be reanalysed after weak inflection;
     * the isolated root's public reading is not a contextual stem. */
    for (size_t ending=1;ending<=2 && word_length>ending+3;++ending) {
        size_t root=word_length-ending;
        if (word[root]!='e' || (ending==2 && !strchr("nmrs",word[root+1]))) continue;
        unsigned nuclei=0;
        for(size_t i=0;i<root;++i)
            if(strchr("aeiouy",word[i]) && (!i || !strchr("aeiouy",word[i-1])))++nuclei;
        if(nuclei<2 || !(reloq_german_morphology_constituent_class(word,root)&6u))continue;
        state=reloq_german_root_plan_word(view,word,word_length,output,capacity,byte_length);
        if(state)return state;
    }
    return reloq_german_g2p_word(word, word_length, output,
                                  capacity, byte_length);
}
