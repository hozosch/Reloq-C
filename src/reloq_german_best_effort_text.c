#include "reloq_german_best_effort_text.h"
#include "reloq_character.h"
#include "reloq_cp1252.h"
#include "reloq_utf8.h"
#include "reloq_rules61.h"
#include "reloq61.h"
#include "reloq61.h"

#include "reloq_german_g2p.h"
#include "reloq_german_morphology.h"
#include "reloq_german_annotation.h"
#include "reloq_german_compound_recovery.h"
#include "reloq_german_generic_prosody.h"
#include "reloq_german_input.h"
#include "reloq_german_lexicon.h"
extern const struct reloq_lexicon_resolver *reloq61_dictionary_resolver(void);
extern void reloq61_dictionary_resolver_release(const struct reloq_lexicon_resolver *);
#include "reloq_german_numbers.h"
#include "reloq_german_word_class.h"
#include "reloq_german_punctuation.h"
#include "reloq_german_root_plan.h"
#include "reloq_inline_tags.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int letter(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           c == 0xc4 || c == 0xd6 || c == 0xdc || c == 0xe4 ||
           c == 0xf6 || c == 0xfc || c == 0xdf || c==0x8c || c==0x9c;
}

static int literal(unsigned char c)
{
    switch (c) {
    case ' ': case '\t': case '\r': case '\n':
    case '.': case ',': case '!': case '?': case ';': case ':':
    case '-': case '(': case ')': case '[': case ']':
    case '\'': case '"': case '/': case '+': case '=': case '%':
    case '@': case '_': case '&': case '#': case '*': case 0x80:
    case 0x84: case 0x85: case 0x91: case 0x92: case 0x93: case 0x94:
    case 0x96: case 0x97:
        return 1;
    default:
        return 0;
    }
}

static int digit(unsigned char c)
{
    return c >= '0' && c <= '9';
}

/* Latin loanwords remain approximate, but an ordinary name with an acute,
 * grave or circumflex should not make a whole utterance silent. Keep German
 * umlauts and ß intact. This one-byte fold preserves source offsets. */
static unsigned char loan_letter(unsigned char c)
{
    /* German umlauts are productive vowel letters in the native rules.
     * They happen to lie inside the Latin-1 acute/grave fold ranges below;
     * preserve them before handling foreign diacritics. */
    if (c == 0xc4 || c == 0xd6 || c == 0xdc ||
        c == 0xe4 || c == 0xf6 || c == 0xfc || c == 0xdf)
        return c;
    if (c >= 0xc0 && c <= 0xc5) return 'A';
    if (c >= 0xe0 && c <= 0xe5) return 'a';
    if (c == 0xc7) return 'C';
    if (c == 0xe7) return 'c';
    if (c >= 0xc8 && c <= 0xcb) return 'E';
    if (c >= 0xe8 && c <= 0xeb) return 'e';
    if (c >= 0xcc && c <= 0xcf) return 'I';
    if (c >= 0xec && c <= 0xef) return 'i';
    if (c == 0xd1) return 'N';
    if (c == 0xf1) return 'n';
    if (c >= 0xd2 && c <= 0xd5) return 'O';
    if (c >= 0xf2 && c <= 0xf5) return 'o';
    if (c >= 0xd9 && c <= 0xdb) return 'U';
    if (c >= 0xf9 && c <= 0xfb) return 'u';
    if (c == 0xdd) return 'Y';
    if (c == 0xfd) return 'y';
    return c;
}

static int reserve(unsigned char **bytes, size_t *capacity, size_t needed)
{
    if (needed <= *capacity) return 0;
    size_t cap = *capacity ? *capacity : 64;
    while (cap < needed) {
        if (cap > SIZE_MAX / 2) {
            cap = needed;
            break;
        }
        cap *= 2;
    }
    unsigned char *next = realloc(*bytes, cap);
    if (!next) return -1;
    *bytes = next;
    *capacity = cap;
    return 0;
}

static unsigned char fold_lower(unsigned char c);
struct annotation_builder {
    unsigned char *output;
    size_t used, capacity, approximate;
    const struct reloq_lexicon *lexicon;
    int (*cancelled)(void *);
    void *cancel_context;
    struct reloq_german_word_features *features;
    size_t feature_count,feature_capacity,word_count;
    int retain_features;
    int numeric_output;
    int prediction_seen;
    int native_first;
    size_t deferred_words;
    size_t input_length;
    /* One entry per emitted SPR group, aligned with its written source. */
    struct reloq_german_prosody_unit *units;
    size_t unit_count,unit_capacity;
    const unsigned char *src_override;size_t src_override_length;
    const unsigned char *ctx_text;size_t ctx_length;int ctx_fixed;
    const char *const *ctx_words;size_t ctx_word_count;size_t ctx_word_at;
    int pending_emphasis;
};

static int push_unit(struct annotation_builder *b,const unsigned char *text,
    size_t length,int weak,int fixed)
{
    if (b->unit_count==b->unit_capacity) {
        size_t cap=b->unit_capacity?b->unit_capacity*2:16;
        if (cap<b->unit_capacity || cap>SIZE_MAX/sizeof *b->units) return -2;
        void *next=realloc(b->units,cap*sizeof *b->units);
        if (!next) return -2;
        b->units=next;b->unit_capacity=cap;
    }
    b->units[b->unit_count].text=text;
    b->units[b->unit_count].length=length;
    b->units[b->unit_count].force_weak=weak;
    b->units[b->unit_count].fixed_phrase=fixed;
    b->units[b->unit_count].force_deaccent=0;
    b->units[b->unit_count].preserve=0;
    b->units[b->unit_count].unresolved=0;
    b->units[b->unit_count].source_emphasis=b->pending_emphasis;
    b->pending_emphasis=0;
    ++b->unit_count;
    return 0;
}

static int remember_word(struct annotation_builder *b,const unsigned char *word,
                         size_t length,const unsigned char *spr,size_t n,int root_plan)
{
    struct reloq_german_word_features f={0};
    (void)root_plan;
    int have_features=0;
    have_features=b->retain_features &&
        reloq_german_word_features_lookup(word,length,spr,n,&f)==1;
    if(b->retain_features && !have_features) {
        have_features=reloq_german_word_features_root_hiatus(spr,n,&f)==1;
        /* Root-internal hiatus is independent of which reader supplied
         * the pronunciation. Protect witnessed compound edges: an open
         * initial constituent still has a fresh vowel-initial head. */
        for(size_t split=3;split+3<=length;++split) {
            if(!(reloq_german_morphology_constituent_class(word,split)&1u) &&
               !reloq_german_morphology_is_root_boundary(word,length,split))continue;
            /* Inflectional syllables are not new lexical heads. A real
             * compound boundary must precede a full vowel, not schwa or
             * the reduced -er nucleus of an adjective ending. */
            unsigned char left[1024];size_t ln=0;
            if(reloq_german_lexicon_word(b->lexicon,word,split,left,sizeof left,&ln)!=1 ||
               ln<5 || ln>=n || memcmp(left,spr,ln-1))continue;
            unsigned syllables=0;
            for(size_t j=0;j<ln;++j)syllables+=left[j]=='.';
            struct reloq_german_spr_cursor hc={0};
            struct reloq_german_spr_event he;
            int head_phone=0;
            while(reloq_german_spr_next(spr,n,&hc,&he)==1)
                if(he.kind==RELOQ_DE_SPR_PHONE && he.syllable==syllables) {head_phone=he.phone;break;}
            if(head_phone==RELOQ_DE_PHONE_AT || head_phone==RELOQ_DE_PHONE_R)continue;
            if(syllables<64) {
                /* A consonant-final constituent has no vowel hiatus to
                 * trigger the root reader, but its vowel-initial head
                 * still needs the same lexical onset metadata. */
                if(!have_features) {
                    if(reloq_german_word_features_from_spr(spr,n,&f)!=1)continue;
                    have_features=1;
                }
                f.glottal_syllables|=UINT64_C(1)<<syllables;
            }
        }
    }
    if (b->retain_features && length && (word[length-1]=='a' || word[length-1]=='A') &&
        reloq_german_word_features_final_a(spr,n,&f)==1) have_features=1;
    if (b->retain_features && reloq_german_word_features_quantity(word,length,spr,n,&f)==1)
        have_features=1;
    if (have_features) {
        if (b->feature_count==b->feature_capacity) {
            size_t cap=b->feature_capacity?b->feature_capacity*2:8;
            if (cap<b->feature_capacity || cap>SIZE_MAX/sizeof f) return -2;
            void *next=realloc(b->features,cap*sizeof f);
            if (!next) return -2;
            b->features=next;b->feature_capacity=cap;
        }
        f.word_index=b->word_count;
        b->features[b->feature_count++]=f;
    }
    if (b->word_count==SIZE_MAX) return -2;
    ++b->word_count;
    return 0;
}

static int append_literal(struct annotation_builder *b, unsigned char c)
{
    if (b->used == SIZE_MAX ||
        reserve(&b->output, &b->capacity, b->used + 2)) return -2;
    b->output[b->used++] = c;
    return 0;
}

/* A valid complete SPR group is the only primitive emitted by this adapter.
 * The observed lexicon and strict G2P always win over the approximate model. */
static int append_word(struct annotation_builder *b,
                       const unsigned char *word, size_t length)
{
    if (b->cancelled && b->cancelled(b->cancel_context)) return 2;
    size_t n = 0, confirmed = 0;
    unsigned char selected[4096];
    int approximate = 0, planned = 0;
    const struct reloq_lexicon_override *explicit_entry=NULL;
    int explicit_word=b->lexicon&&reloq_lexicon_find_override(b->lexicon,word,length,&explicit_entry)==1;
    int voiced_samt=length==4&&(word[0]=='s'||word[0]=='S')&&!memcmp(word+1,"amt",3);
    if(b->native_first&&b->lexicon&&!explicit_word&&!voiced_samt) {
        /* Only an alignment token. The sentence frontend supplies the real
           pronunciation once; on refusal the complete legacy pass is retried.
           This provisional group can never reach the acoustic composer. */
        static const unsigned char alignment[]="`[.1a]";
        size_t add=sizeof alignment-1;
        if(b->used>SIZE_MAX-add-1||reserve(&b->output,&b->capacity,b->used+add+1))return -2;
        memcpy(b->output+b->used,alignment,add);b->used+=add;
        if(b->src_override) {
            if(push_unit(b,b->src_override,b->src_override_length,0,0))return -2;
            b->src_override=NULL;
        } else if(push_unit(b,word,length,0,0))return -2;
        b->units[b->unit_count-1].unresolved=1;
        ++b->word_count;++b->deferred_words;return 0;
    }
    int status=explicit_word?0:reloq_german_morphology_user_compound(
        b->lexicon,word,length,selected,sizeof selected,&n);
    if(!status)status = reloq_german_lexicon_word(
        b->lexicon, word, length, selected, sizeof selected, &n);
    if(status==-1 && (memchr(word,0x8c,length)||memchr(word,0x9c,length)))status=0;
    if (status == 0 && b->lexicon) {
        status=reloq_rules61_word(0x40000,word,length,selected,sizeof selected,&n);
        if(status==1||status==-2)planned=3;
    }
    if (status == 0) {
        status = reloq_german_root_plan_word(b->lexicon,word,length,selected,sizeof selected,&n);
        if (status == 1 || status == -2) {approximate=1;planned=1;}
    }
    if (status == 0) {
        status = reloq_german_unclassified_root_word(b->lexicon,word,length,selected,sizeof selected,&n);
        if (status == 1 || status == -2) {approximate=1;planned=2;}
    }
    if (status == 0) {
        status = reloq_german_g2p_word_best_effort(
            word, length, selected, sizeof selected, &n, &approximate);
        if (status == 1 || status == -2) approximate = 1;
    }
    if ((status != 1 && status != -2) || n == SIZE_MAX || n >= SIZE_MAX - b->used)
        return status == 0 ? 1 : status == -1 ? -1 : -2;
    if (reserve(&b->output, &b->capacity, b->used + n + 1)) return -2;
    if (status==1) {
        if(n>=sizeof selected)return -1;
        memcpy(b->output+b->used,selected,n+1);confirmed=n;
    } else if (planned==3) {
        status=reloq_rules61_word(0x40000,word,length,b->output+b->used,n+1,&confirmed);
    } else if (planned) {
        status = (planned==2?reloq_german_unclassified_root_word:reloq_german_root_plan_word)(b->lexicon,word,length,
            b->output+b->used,n+1,&confirmed);
    } else if (approximate) {
        int again = 0;
        status = reloq_german_g2p_word_best_effort(
            word, length, b->output + b->used, n + 1, &confirmed, &again);
        if (!again) status = -1;
    } else {
        status = reloq_german_lexicon_word(
            b->lexicon, word, length, b->output + b->used,
            n + 1, &confirmed);
    }
    if (status != 1 || confirmed != n ||
        reloq_german_lexicon_validate_spr(b->output + b->used,
                                           confirmed, NULL) != 1)
        return -1;
    reloq_german_morphology_nominal_stress(b->lexicon,word,length,b->output+b->used,n);
    reloq_german_morphology_repair_stem(word,length,b->output+b->used,n);
    /* Explicit user-requested departure from the original's unvoiced
     * standalone samt. Keep the observed dictionary itself unchanged. */
    if(length==4 && (word[0]=='s'||word[0]=='S') &&
       (word[1]=='a'||word[1]=='A') && (word[2]=='m'||word[2]=='M') &&
       (word[3]=='t'||word[3]=='T') && n==9 &&
       !memcmp(b->output+b->used,"`[.1sAmt]",9))b->output[b->used+4]='z';
    if (remember_word(b,word,length,b->output+b->used,n,planned)) return -2;
    if (b->src_override) {
        if (push_unit(b,b->src_override,b->src_override_length,0,0)) return -2;
        b->src_override=NULL;
    } else if (push_unit(b,word,length,0,0)) return -2;
    b->used += n;
    b->approximate += (size_t)approximate;
    return 0;
}

/* A complete compound can fall outside whole-word G2P despite both
 * constituents being independently analyzable. The recovery routine only
 * accepts a unique phonologically plausible split and gives us one SPR word.
 * Retain the letter-name route for everything else. */
static int append_compound(struct annotation_builder *b,
                           const unsigned char *word, size_t length)
{
    if (b->cancelled && b->cancelled(b->cancel_context)) return 2;
    size_t required = 0, confirmed = 0;
    int status = reloq_german_compound_recover(
        b->lexicon, word, length, NULL, 0, &required);
    if (status == 0 || status == -1) return 1;
    if (status != -2 || required == SIZE_MAX ||
        required >= SIZE_MAX - b->used) return -2;
    if (reserve(&b->output, &b->capacity, b->used + required + 1))
        return -2;
    status = reloq_german_compound_recover(
        b->lexicon, word, length, b->output + b->used,
        required + 1, &confirmed);
    if (status != 1 || confirmed != required ||
        reloq_german_lexicon_validate_spr(
            b->output + b->used, required, NULL) != 1)
        return -1;
    if (remember_word(b,word,length,b->output+b->used,required,0)) return -2;
    if (push_unit(b,word,length,0,0)) return -2;
    b->used += required;
    ++b->approximate;
    return 0;
}

/* Character naming is a terminal frontend operation. Passing names such as
 * "Ce", "Ef" or "En" through ordinary word analysis changes their phones.
 * These complete pronunciations were observed through the public German
 * phoneme API for each upper/lowercase character and in letter sequences.
 * Original single_chars -> char_name dispatch explains why this path must
 * stay distinct from ordinary lexical and morphological analysis. */
static int append_letters(struct annotation_builder *b,
                          const unsigned char *word, size_t length)
{
    static const char *const names[] = {
        "`[.1a]", "`[.1be]", "`[.1Te]", "`[.1de]", "`[.1e]",
        "`[.1Ef]", "`[.1ge]", "`[.1ha]", "`[.1i]", "`[.1jOt]",
        "`[.1ka]", "`[.1El]", "`[.1Em]", "`[.1En]", "`[.1o]",
        "`[.1pe]", "`[.1ku]", "`[.1Er]", "`[.1Es]", "`[.1te]",
        "`[.1u]", "`[.1f'aw']", "`[.1ve]", "`[.1Iks]",
        "`[.1Yp.0sI.0lOn]", "`[.1TEt]"
    };
    for (size_t i = 0; i < length; ++i) {
        if (b->cancelled && b->cancelled(b->cancel_context)) return 2;
        unsigned char c = word[i];
        const char *special = c == 0xc4 || c == 0xe4 ? "`[.1'E:']" :
                              c == 0xd6 || c == 0xf6 ? "`[.1'oe']" :
                              c == 0xdc || c == 0xfc ? "`[.1y]" :
                              c == 0xdf ? "`[.1Es.2TEt]" : NULL;
        if (c >= 'a' && c <= 'z') c = (unsigned char)(c - 'a' + 'A');
        if (!special && (c < 'A' || c > 'Z')) return 1;
        if (i && append_literal(b, ' ')) return -2;
        const char *spr = special ? special : names[c - 'A'];
        size_t n = strlen(spr);
        if (n >= SIZE_MAX - b->used ||
            reserve(&b->output, &b->capacity, b->used + n + 1)) return -2;
        memcpy(b->output + b->used, spr, n);
        if (remember_word(b,word+i,1,b->output+b->used,n,0)) return -2;
        if (push_unit(b,word+i,1,0,0)) return -2;
        b->used += n;
    }
    b->approximate += 1;
    return 0;
}


static int append_observed(void *context,const char *body,int weak);
/* Grammar normalization owns abbreviation spelling and its da option. */
enum {NATIVE_TOKEN_ABBREVIATION=2};
static int append_native_token(struct annotation_builder *,const unsigned char *,size_t,int);

static int append_word_token(struct annotation_builder *b,
    const unsigned char *word,size_t length)
{
    int result;
    const struct reloq_lexicon_override *exact = NULL;
    const int overridden=b->lexicon &&
        reloq_lexicon_find_override(b->lexicon,word,length,&exact)==1;
    if (!overridden) {
        b->ctx_text=word;b->ctx_length=length;b->ctx_fixed=RELOQ_DE_SOURCE_NUMBER;
        b->numeric_output=1;
        result=reloq_german_number_word(word,length,append_observed,b);
        b->numeric_output=0;b->ctx_text=NULL;
        if (result!=1) return result;
    }
    const int word_class=reloq_german_word_class(word,length);
    if (!overridden && length>1 && word_class==1) {
        result=append_native_token(b,word,length,NATIVE_TOKEN_ABBREVIATION);
        if(result!=1)return result;
    }
    if (!overridden && (length==1||word_class==1))
        result = append_letters(b, word, length);
    else if (!overridden && word_class==0) {
        /* Word-class capitals use the same lexical/morphological
         * analysis as ordinary case. Preserve the original source
         * span; explicit exact-case caller entries above still win. */
        unsigned char *folded=malloc(length+1);
        if (!folded) return -2;
        for (size_t i=0;i<length;++i) folded[i]=(unsigned char)(word[i]+32);
        folded[length]=0;
        b->src_override=word;b->src_override_length=length;
        result=append_word(b,folded,length);
        b->src_override=NULL;
        free(folded);
    } else
        result = append_word(b, word, length);
    if (result == 1)
        result = append_compound(b, word, length);
    if (result == 1) {
        /* A word outside both lexical and approximate phonetic rules
         * must not silence its entire surrounding NVDA sentence.
         * Spell only this unmodeled token; known G2P and lexicon words
         * keep their original pronunciation. The same letter-name
         * path already serves initialisms and one-letter words. */
        result = append_letters(b, word, length);
    }
    return result;
}

/* Marked compound stress can survive an explicit hyphen (Nord-Ost).
 * A full lexical entry is used only when its syllables and phones already
 * agree with the independently analysed components and it actually marks
 * compound stress. This cannot erase an explicit boundary in über-geben. */
static void compound_lexical_accent(struct annotation_builder *b,
    const unsigned char *word,size_t length,size_t spr_at)
{
    unsigned char key[128];size_t n=0;
    for (size_t i=0;i<length;++i) {
        if (word[i]=='-') continue;
        if (n==sizeof key-1) return;
        key[n++]=fold_lower(word[i]);
    }
    const struct reloq_lexicon_override *entry=NULL;
    if (!b->lexicon || !n) return;
    if (reloq_lexicon_find_override(b->lexicon,key,n,&entry)!=1) {
        if (key[0]>='a' && key[0]<='z') key[0]-=32;
        else if (key[0]==0xe4 || key[0]==0xf6 || key[0]==0xfc) key[0]-=32;
        if (reloq_lexicon_find_override(b->lexicon,key,n,&entry)!=1) return;
    }
    if (entry->spr_length!=b->used-spr_at) return;
    int secondary=0;
    for (size_t i=0;i<entry->spr_length;++i) {
        if (i && entry->spr[i-1]=='.') {
            if (entry->spr[i]=='2') secondary=1;
        } else if (entry->spr[i]!=b->output[spr_at+i]) return;
    }
    if (!secondary) return;
    for (size_t i=1;i<entry->spr_length;++i)
        if (entry->spr[i-1]=='.') b->output[spr_at+i]=entry->spr[i];
}

/* Compose phonetic onset facts along with syllables. A written boundary
 * keeps the right constituent's word-initial onset even when its first
 * syllable is weak. SPR stress alone cannot represent that distinction. */
static int composed_features(struct annotation_builder *b,size_t left,
    size_t right,size_t end,size_t word)
{
    if (!b->retain_features) return 0;
    struct reloq_german_word_features f={0};
    unsigned offset=0;
    for (unsigned side=0;side<2;++side) {
        /* The feature format covers the first 64 syllables. Beyond that,
         * retain the normal stress-based onset policy without leaving the
         * removed right constituent's word index in the feature array. */
        if (offset==64) break;
        size_t at=side?right:left,n=(side?end:right)-at;
        struct reloq_german_spr_cursor cursor={0};
        struct reloq_german_spr_event e;
        uint64_t mask=1,full_a=0,full_quantity=0;unsigned count=0;
        int rc;
        while ((rc=reloq_german_spr_next(b->output+at,n,&cursor,&e))==1) {
            if (e.syllable>=64) {count=64;continue;}
            if (e.kind==RELOQ_DE_SPR_STRESS) {
                if (e.stress) mask|=UINT64_C(1)<<e.syllable;
                count=(unsigned)e.syllable+1;
            }
        }
        if (rc) return -1;
        if (count>64-offset) count=64-offset;
        for (size_t j=0;j<b->feature_count;++j) {
            const struct reloq_german_word_features *old=b->features+j;
            if (old->word_index!=word+side) continue;
            uint64_t known=old->syllable_count>=64?UINT64_MAX:
                (UINT64_C(1)<<old->syllable_count)-1;
            mask=(mask&~known)|(old->glottal_syllables&known);
            full_a=old->full_a_syllables;
            full_quantity=old->full_quantity_syllables;
        }
        f.glottal_syllables|=mask<<offset;
        f.full_a_syllables|=full_a<<offset;
        f.full_quantity_syllables|=full_quantity<<offset;
        offset+=count;
    }
    f.word_index=word;f.syllable_count=offset;
    while (b->feature_count && b->features[b->feature_count-1].word_index>=word)
        --b->feature_count;
    if (b->feature_count==b->feature_capacity) {
        size_t cap=b->feature_capacity?b->feature_capacity*2:8;
        if (cap<b->feature_capacity || cap>SIZE_MAX/sizeof f) return -2;
        void *next=realloc(b->features,cap*sizeof f);
        if (!next) return -2;
        b->features=next;b->feature_capacity=cap;
    }
    b->features[b->feature_count++]=f;
    return 0;
}

/* Split at the written boundary, analyse each constituent, then compose
 * one stress domain. Letter sequences remain separate; their final letter
 * can be the first constituent of a following full word (USB-Kabel).
 * Original stage evidence: hyphenated_compound, split_hyphenated_compound,
 * break_into_ger_morphs and compound_stress_adjustment. No guessed spelling
 * split or word-specific hyphen pronunciation is introduced here. */
static int same_native_phones(const unsigned char *a,size_t an,const unsigned char *c,size_t cn)
{
    struct reloq_german_spr_cursor ac={0},cc={0};
    struct reloq_german_spr_event ae,ce;int ar,cr;
    do {
        do {ar=reloq_german_spr_next(a,an,&ac,&ae);}while(ar==1&&ae.kind!=RELOQ_DE_SPR_PHONE);
        do {cr=reloq_german_spr_next(c,cn,&cc,&ce);}while(cr==1&&ce.kind!=RELOQ_DE_SPR_PHONE);
        if(ar!=cr||ar<0||(ar&&ae.phone!=ce.phone))return 0;
    }while(ar);
    return 1;
}
static int append_explicit_token(struct annotation_builder *b,
    const unsigned char *word,size_t length,const struct reloq_lexicon_override *entry)
{
    size_t n=entry->spr_length;
    if(b->used>SIZE_MAX-n-1||reserve(&b->output,&b->capacity,b->used+n+1))return -2;
    memcpy(b->output+b->used,entry->spr,n);b->output[b->used+n]=0;
    if(remember_word(b,word,length,b->output+b->used,n,0)||
        push_unit(b,word,length,0,RELOQ_DE_SOURCE_SPR))return -2;
    b->units[b->unit_count-1].preserve=1;b->used+=n;return 0;
}
/* Keep the complete native stress domain, and independently recover the
 * written constituent edges that its public SPR cannot encode. A boundary
 * is accepted only after all component phones match that complete reading. */
static int next_component_phone(const unsigned char *s,size_t n,
    struct reloq_german_spr_cursor *cursor,struct reloq_german_spr_event *event)
{
    int rc;do {rc=reloq_german_spr_next(s,n,cursor,event);}
    while(rc==1 && event->kind!=RELOQ_DE_SPR_PHONE);return rc;
}
static int component_matches(const unsigned char *part,size_t pn,
    const unsigned char *whole,size_t wn,struct reloq_german_spr_cursor *position,
    struct reloq_german_spr_event *first,int lexical)
{
    struct reloq_german_spr_cursor c={0},at=*position;
    struct reloq_german_spr_event a,b;int rc,seen=0;
    for(;;) {
        do {rc=reloq_german_spr_next(part,pn,&c,&a);}
        while(rc==1&&a.kind!=RELOQ_DE_SPR_PHONE&&!(lexical&&a.kind==RELOQ_DE_SPR_STRESS));
        if(rc!=1)break;
        int other;
        do {other=reloq_german_spr_next(whole,wn,&at,&b);}
        while(other==1&&b.kind!=RELOQ_DE_SPR_PHONE&&!(lexical&&b.kind==RELOQ_DE_SPR_STRESS));
        if(other!=1||a.kind!=b.kind)return 0;
        if(a.kind==RELOQ_DE_SPR_STRESS) {
            /* A constituent's primary accent may become secondary in a
             * compound. Its other lexical accents remain authoritative. */
            if(a.stress!=b.stress&&!(a.stress==1&&b.stress==2))return 0;
        } else {
            if(a.phone!=b.phone)return 0;
            if(!seen)*first=b;
            seen=1;
        }
    }
    if(rc || !seen)return 0;
    *position=at;return 1;
}
static uint64_t native_compound_onsets(struct annotation_builder *b,
    const unsigned char *word,size_t length,const unsigned char *spr,size_t n)
{
    if(!b->retain_features)return 0;
    /* Most compounds already carry every possible vowel onset. Avoid
     * reanalysing their constituents merely to rediscover those bits. */
    struct reloq_german_word_features known;
    if(reloq_german_word_features_from_spr(spr,n,&known)<1)return 0;
    if(b->feature_count && b->features[b->feature_count-1].word_index==b->word_count-1) {
        const struct reloq_german_word_features *old=b->features+b->feature_count-1;
        uint64_t bits=old->syllable_count>=64?UINT64_MAX:(UINT64_C(1)<<old->syllable_count)-1;
        known.glottal_syllables=(known.glottal_syllables&~bits)|(old->glottal_syllables&bits);
    }
    struct reloq_german_spr_cursor scan={0};struct reloq_german_spr_event e;
    size_t previous=SIZE_MAX;int missing=0;
    while(next_component_phone(spr,n,&scan,&e)==1) {
        if(e.syllable==previous)continue;
        previous=e.syllable;
        if(!e.syllable || e.syllable>=64 || (known.glottal_syllables&(UINT64_C(1)<<e.syllable)))continue;
        if((e.phone>=RELOQ_DE_PHONE_a && e.phone<=RELOQ_DE_PHONE_AT) ||
           (e.phone>=RELOQ_DE_PHONE_Q_E_COLON && e.phone<=RELOQ_DE_PHONE_Q_oe_TILDE)) {missing=1;break;}
    }
    if(!missing)return 0;
    struct reloq_german_spr_cursor position={0};uint64_t mask=0;
    for(size_t at=0;at<length;) {
        size_t end=at;while(end<length && word[end]!='-')++end;
        unsigned char part[4096];size_t pn=0;struct reloq_german_spr_event first={0};
        struct reloq_german_spr_cursor trial=position;
        int found=reloq_german_lexicon_word(b->lexicon,word+at,end-at,part,sizeof part,&pn);
        if(found!=1 || !component_matches(part,pn,spr,n,&trial,&first,0)) {
            trial=position;
            if(reloq_rules61_word(0x40000,word+at,end-at,part,sizeof part,&pn)!=1 ||
               !component_matches(part,pn,spr,n,&trial,&first,0))return 0;
        }
        position=trial;
        const int vowel=(first.phone>=RELOQ_DE_PHONE_a && first.phone<=RELOQ_DE_PHONE_AT) ||
            (first.phone>=RELOQ_DE_PHONE_Q_E_COLON && first.phone<=RELOQ_DE_PHONE_Q_oe_TILDE);
        if(at && vowel && first.syllable<64) {
            struct reloq_german_word_features facts;
            int kind=reloq_german_word_features_from_spr(part,pn,&facts);
            if(kind<1)return 0;
            if(kind!=2 || (facts.glottal_syllables&1u))mask|=UINT64_C(1)<<first.syllable;
        }
        at=end+1;
    }
    struct reloq_german_spr_event extra;
    return next_component_phone(spr,n,&position,&extra)==0?mask:0;
}
static int remember_compound_onsets(struct annotation_builder *b,
    const unsigned char *spr,size_t n,uint64_t mask)
{
    if(!mask)return 0;
    struct reloq_german_word_features facts;
    if(reloq_german_word_features_from_spr(spr,n,&facts)<1)return -1;
    const size_t word=b->word_count-1;
    struct reloq_german_word_features *old=b->feature_count &&
        b->features[b->feature_count-1].word_index==word?b->features+b->feature_count-1:NULL;
    if(old) {
        uint64_t known=old->syllable_count>=64?UINT64_MAX:(UINT64_C(1)<<old->syllable_count)-1;
        facts.glottal_syllables=(facts.glottal_syllables&~known)|(old->glottal_syllables&known);
        facts.full_quantity_syllables=old->full_quantity_syllables;
        facts.full_a_syllables|=old->full_a_syllables;
    }
    if(!(mask&~facts.glottal_syllables))return 0;
    facts.word_index=word;facts.glottal_syllables|=mask;
    if(old){*old=facts;return 0;}
    if(b->feature_count==b->feature_capacity) {
        size_t cap=b->feature_capacity?b->feature_capacity*2:8;
        if(cap<b->feature_capacity || cap>SIZE_MAX/sizeof facts)return -2;
        void *next=realloc(b->features,cap*sizeof facts);if(!next)return -2;
        b->features=next;b->feature_capacity=cap;
    }
    b->features[b->feature_count++]=facts;return 0;
}

static int append_native_compound(struct annotation_builder *b,
    const unsigned char *word,size_t length)
{
    unsigned char spr[4096];size_t n=0;
    if(reloq_rules61_word(0x40000,word,length,spr,sizeof spr,&n)!=1)return 1;
    /* Compare the constituent entries with this complete pronunciation.
     * An isolated native lookup can itself use the optional dictionary and
     * says nothing about whether the whole compound used that entry. */
    struct reloq_german_spr_cursor position={0};
    int lexical=0,matched=1;
    if(b->lexicon)for(size_t at=0;at<length;) {
        size_t end=at;while(end<length&&word[end]!='-')++end;
        const struct reloq_lexicon_override *entry=NULL;
        if(reloq_lexicon_find_override(b->lexicon,word+at,end-at,&entry)==1) {lexical=1;break;}
        at=end+1;
    }
    /* The default native path needs no additional constituent analyses
     * when there is no lexical pronunciation to protect. */
    if(lexical) {
    for(size_t at=0;at<length;) {
        size_t end=at;while(end<length&&word[end]!='-')++end;
        const struct reloq_lexicon_override *entry=NULL;
        unsigned char native[4096];size_t nn=0;
        const unsigned char *part=native;
        if(b->lexicon&&reloq_lexicon_find_override(b->lexicon,word+at,end-at,&entry)==1) {
            part=entry->spr;nn=entry->spr_length;
        } else if(matched&&reloq_rules61_word(0x40000,word+at,end-at,native,sizeof native,&nn)!=1) {
            matched=0;
        }
        struct reloq_german_spr_event first;
        if(matched&&!component_matches(part,nn,spr,n,&position,&first,entry!=NULL))matched=0;
        at=end+1;
    }
    struct reloq_german_spr_event extra;
    if(!matched||next_component_phone(spr,n,&position,&extra)!=0)return 1;
    }
    if(b->used>SIZE_MAX-n-1||reserve(&b->output,&b->capacity,b->used+n+1))return -2;
    memcpy(b->output+b->used,spr,n);
    reloq_german_morphology_nominal_stress(b->lexicon,word,length,b->output+b->used,n);
    reloq_german_morphology_repair_stem(word,length,b->output+b->used,n);
    if(remember_word(b,word,length,b->output+b->used,n,3))return -2;
    uint64_t onsets=native_compound_onsets(b,word,length,b->output+b->used,n);
    if(remember_compound_onsets(b,b->output+b->used,n,onsets) ||
       push_unit(b,word,length,0,0))return -2;
    b->used+=n;return 0;
}
static int append_hyphenated_word(struct annotation_builder *b,
    const unsigned char *word,size_t length)
{
    const struct reloq_lexicon_override *entry=NULL;
    if(b->lexicon&&reloq_lexicon_find_override(b->lexicon,word,length,&entry)==1)
        return append_explicit_token(b,word,length,entry);
    int complete=append_native_compound(b,word,length);
    if(complete!=1)return complete;
    size_t at=0;const size_t first_unit=b->unit_count;
    while (at<length) {
        size_t start=at;
        while (at<length && word[at]!='-') ++at;
        const size_t before=b->used,words=b->word_count;
        /* A constituent must have real phones before concatenation. This
           local resolution avoids retrying an entire sentence for a hyphen. */
        int native_first=b->native_first;b->native_first=0;
        int status=append_word_token(b,word+start,at-start);
        b->native_first=native_first;
        if (status) return status;
        const int spelled=b->unit_count>words &&
            b->units[b->unit_count-1].length==1;
        if (start && !spelled && b->word_count==words+1) {
            size_t left=before;
            while (left && b->output[left-1]!='[') --left;
            if (left<2 || !before || b->output[before-1]!=']') return -1;
            left-=2; /* begin at the SPR control introducer */
            if (composed_features(b,left,before,b->used,words-1)) return -2;
            for (size_t i=before+2;i+1<b->used;++i)
                if (b->output[i]=='.' && b->output[i+1]=='1') b->output[i+1]='2';
            memmove(b->output+before-1,b->output+before+2,b->used-before-2);
            b->used-=3;--b->word_count;--b->unit_count;
            struct reloq_german_prosody_unit *unit=b->units+b->unit_count-1;
            unit->unresolved|=b->units[b->unit_count].unresolved;
            const size_t offset=(size_t)(unit->text-word);
            unit->length=at-offset;
            compound_lexical_accent(b,word+offset,at-offset,left);
        }
        if (at<length) ++at;
    }
    /* Hyphenated letter sequences take a phrase nucleus on their last
     * letter; ordinary spelling-mode sequences keep their existing path. */
    if (b->unit_count>first_unit) b->units[b->unit_count-1].fixed_phrase=1;
    return 0;
}

static int append_number_word(struct annotation_builder *b, const char *word)
{
    if (b->used && append_literal(b, ' ')) return -2;
    b->src_override=(const unsigned char *)word;b->src_override_length=strlen(word);
    return append_word(b, (const unsigned char *)word, strlen(word));
}

/* Numeric and symbol words carry complete observed SPR groups. Keeping
 * their internal syllables together prevents false word-boundary onsets. */
static int append_observed(void *context,const char *body,int weak)
{
    struct annotation_builder *b=context;
    if (b->cancelled && b->cancelled(b->cancel_context)) return 2;
    const char *prefix=b->used?" `[":"`[";
    size_t n=strlen(body),prefix_length=strlen(prefix);
    if (n>SIZE_MAX-prefix_length-2 || b->used>SIZE_MAX-n-prefix_length-2 ||
        reserve(&b->output,&b->capacity,b->used+n+prefix_length+2)) return -2;
    memcpy(b->output+b->used,prefix,prefix_length);b->used+=prefix_length;
    memcpy(b->output+b->used,body,n);
    if (b->numeric_output) {
        /* Requested numeric liaison: vier-und uses a consonantal r.
         * Assign it to the next onset, so the regular r-to-vowel gesture
         * supplies its tract, source and contact duration consistently. */
        unsigned char *written=b->output+b->used;
        for (size_t k=0;k+8<=n;++k)
            if (!memcmp(written+k,"fir.",4) &&
                (written[k+4]=='0' || written[k+4]=='2') &&
                !memcmp(written+k+5,"Unt",3)) {
                written[k+2]='.';written[k+3]=written[k+4];written[k+4]='r';
            }
    }
    b->used+=n;b->output[b->used++]=']';
    size_t groups=1;
    for(size_t i=0;i<n;++i)if(body[i]=='[')++groups;
    if (b->word_count>SIZE_MAX-groups) return -2;
    if (b->numeric_output && b->retain_features) {
        if (b->feature_count>SIZE_MAX-groups ||
            b->feature_count+groups>SIZE_MAX/sizeof *b->features) return -2;
        size_t count=b->feature_count+groups;
        if (count>b->feature_capacity) {
            void *next=realloc(b->features,count*sizeof *b->features);
            if (!next) return -2;
            b->features=next;b->feature_capacity=count;
        }
        /* Requested number policy: restore the reference und onset after
         * five, six and eight, including within larger number groups.
         * Other unit policies and the eighty onset remain independent. */
        size_t group=0;unsigned syllables=0;uint64_t mask=1;
        for (size_t k=0;k<=n;++k) {
            if (k<n && body[k]=='.') {
                if (syllables<64) {
                    const int eighty=k+9<=n && !memcmp(body+k+2,"Ax.0TIX",7);
                    if (eighty) {
                        mask|=UINT64_C(1)<<syllables;
                    }
                    if(k+5<=n && !memcmp(body+k+2,"Unt",3) &&
                        ((k>=4 && !memcmp(body+k-4,"fYnf",4)) ||
                         (k>=4 && !memcmp(body+k-4,"zEks",4)) ||
                         (k>=3 && !memcmp(body+k-3,"Axt",3))))
                        mask|=UINT64_C(1)<<syllables;
                }
                ++syllables;
            }
            if (k==n || body[k]==']') {
                b->features[b->feature_count++]=(struct reloq_german_word_features){
                    .word_index=b->word_count+group++,
                    .glottal_syllables=mask,.syllable_count=syllables};
                syllables=0;mask=1;
            }
        }
        if (group!=groups) return -2;
    }
    b->word_count+=groups;
    for (size_t g=0;g<groups;++g) {
        const unsigned char *text=b->ctx_text;size_t length=b->ctx_length;
        int fixed=b->ctx_fixed;
        if (b->ctx_words && b->ctx_word_at<b->ctx_word_count) {
            const char *w=b->ctx_words[b->ctx_word_at++];
            text=(const unsigned char *)w;length=strlen(w);fixed=0;
        }
        if (!text) {text=(const unsigned char *)"x";length=1;fixed=1;}
        if (push_unit(b,text,length,weak&&g==0,fixed)) return -2;
    }
    return 0;
}

/* A connected spelling can normalize to several native words (place names,
 * choices, and units). Keep the complete written token and its group count
 * together so the sentence pass sees its original delimiter and context. */
static int append_native_token(struct annotation_builder *b,
    const unsigned char *source,size_t length,int dictionary)
{
    if(!length||length>=256)return 1;
    const struct reloq_lexicon_override *entry=NULL;
    if(!dictionary&&b->lexicon&&reloq_lexicon_find_override(b->lexicon,source,length,&entry)==1)
        return append_explicit_token(b,source,length,entry);
    char text[256],phonemes[4096];memcpy(text,source,length);text[length]=0;
    struct reloq61_request q={.language=0x40000,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,92},.text=text,.phonemes=phonemes,
        .phonemes_capacity=sizeof phonemes};
    struct reloq61_result result;
    uint64_t hits=reloq61_dictionary_hits();
    if(reloq61_generate(&q,&result)||result.error||result.unsupported||
        result.phonemes_length>=sizeof phonemes)return 1;
    if(dictionary==1&&reloq61_dictionary_hits()==hits)return 1;
    for(size_t i=0;i<result.phonemes_length;++i)if(!phonemes[i])phonemes[i]=' ';
    phonemes[result.phonemes_length]=0;
    /* Authored constituent corrections and caller pronunciations outrank
     * token normalization. Keep those tokens on the existing word path. */
    for(size_t at=0;!dictionary&&at<length;) {
        size_t end=at;while(end<length&&source[end]!='/')++end;
        size_t part=end-at;
        if(part==4&&((fold_lower(source[at])=='s'&&!memcmp(source+at+1,"amt",3))||
            (fold_lower(source[at])=='p'&&fold_lower(source[at+1])=='l'&&
             fold_lower(source[at+2])=='u'&&fold_lower(source[at+3])=='s')))return 1;
        unsigned char native[4096],authored[4096];size_t n=0;
        int found=reloq_rules61_word(0x40000,source+at,part,native,sizeof native,&n);
        if(b->lexicon&&reloq_lexicon_find_override(b->lexicon,source+at,part,&entry)==1&&
            (found!=1||!same_native_phones(entry->spr,entry->spr_length,native,n)))return 1;
        if(found==1) {
            memcpy(authored,native,n);
            reloq_german_morphology_nominal_stress(b->lexicon,source+at,part,authored,n);
            reloq_german_morphology_repair_stem(source+at,part,authored,n);
            if(memcmp(authored,native,n))return 1;
        }
        at=end+1;
    }
    char *first=strstr(phonemes,"`["),*last=strrchr(phonemes,']');
    if(!first||!last||last<first+2)return 1;
    for(char *p=phonemes;p<first;++p)if(append_literal(b,(unsigned char)*p))return -2;
    *last=0;b->ctx_text=source;b->ctx_length=length;b->ctx_fixed=dictionary==1?RELOQ_DE_SOURCE_DICTIONARY:RELOQ_DE_SOURCE_NATIVE_TOKEN;
    int status=append_observed(b,first+2,0);b->ctx_text=NULL;
    return status;
}

#include "reloq_german_ordinal_context.inc"

static unsigned char fold_lower(unsigned char c)
{
    if ((c>='A' && c<='Z') || c==0xc4 || c==0xd6 || c==0xdc) return (unsigned char)(c+32);
    return c;
}

/* The word that ends before `end` (skipping blanks). Returns its length, or 0
 * when the preceding token is not a word. The lowercase spelling is copied
 * into out (NUL-terminated) and *capital reports its original initial. */
static size_t word_before(const unsigned char *t,size_t end,char *out,
    size_t capacity,int *capital,size_t *start_out)
{
    while (end && (t[end-1]==' ' || t[end-1]=='\t')) --end;
    size_t stop=end;
    while (end && letter(t[end-1])) --end;
    size_t n=stop-end;
    if (!n || n>=capacity) return 0;
    for (size_t i=0;i<n;++i) out[i]=(char)fold_lower(t[end+i]);
    out[n]=0;
    if (capital) *capital=!(fold_lower(t[end])==t[end]);
    if (start_out) *start_out=end;
    return n;
}

static int word_in(const char *const *table,size_t count,const char *word)
{
    size_t lo=0,hi=count;
    while (lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        int c=strcmp(word,table[mid]);
        if (c<0) hi=mid; else if (c>0) lo=mid+1; else return 1;
    }
    return 0;
}

static int in_list(const char *const *list,const char *word)
{
    for (size_t i=0;list[i];++i) if (!strcmp(list[i],word)) return 1;
    return 0;
}

/* -en (1) or -e (0) for an ordinal after the text ending at `at`. */
static int ordinal_dative(const unsigned char *t,size_t at)
{
    static const char *const dative_words[]={"am","im","zum","vom","zur",
        "den","dem","des","einen","einem","eines","einer","meinen","meinem",
        "meines","meiner","seinen","seinem","seines","seiner","ihren","ihrem",
        "ihres","ihrer","unseren","unserem","unseres","unserer","jeden","jedem",
        "jedes","keinen","keinem","keines","keiner","allen","wenigen",
        "manchen","mehreren","ungef\344hr","ca","zirka",NULL};
    static const char *const dative_prepositions[]={"von","mit","aus","bei",
        "nach","seit","auf","in","an","um","unter","\374ber","vor","hinter",
        "neben","zwischen",NULL};
    char w[48],w0[48];int capital=0;size_t start=0;
    size_t n=word_before(t,at,w,sizeof w,&capital,&start);
    if (!n) return 1; /* previous token is a number */
    if (!strcmp(w,"der")) {
        if (word_before(t,start,w0,sizeof w0,NULL,NULL))
            return in_list(dative_prepositions,w0);
        return 0;
    }
    if (in_list(dative_words,w)) return 1;
    if (word_in(ordinal_plain_words,
            sizeof ordinal_plain_words/sizeof ordinal_plain_words[0],w)) return 0;
    return !capital;
}

/* Context of a numeric date: -en after "am", -e after "der", else -er. */
static int date_day_suffix(const unsigned char *t,size_t at)
{
    char w[48];
    if (!word_before(t,at,w,sizeof w,NULL,NULL)) return 0;
    if (!strcmp(w,"am")) return 1;
    if (!strcmp(w,"der")) return 2;
    return 0;
}

/* A numeral followed by a period and a further word is an ordinal. The
 * reference reads it as a cardinal at the start of a sentence, when more
 * than one blank follows, and before the word "Mal". */
static int ordinal_at(const unsigned char *t,size_t n,size_t at,
    size_t *digits_end,uint64_t *value)
{
    if (at && (letter(t[at-1]) || digit(t[at-1]))) return 0;
    size_t end=at;uint64_t v=0;
    while (end<n && digit(t[end])) {
        if (end-at>=9) return 0;
        v=v*10+(uint64_t)(t[end]-'0');++end;
    }
    if (end>=n || t[end]!='.') return 0;
    size_t next=end+1;
    if (next>=n) return 0;
    if (t[next]==',') {
        /* "3., 4." keeps the ordinal and the comma */
    } else {
        if (t[next]!=' ' || next+1>=n) return 0;
        const unsigned char c=t[next+1];
        if (!(letter(c) || digit(c) || c=='(' || c=='-' || c=='"')) return 0;
        if (letter(c)) {
            size_t e=next+1;
            while (e<n && letter(t[e])) ++e;
            if (e-(next+1)==3 && !memcmp(t+next+1,"Mal",3)) return 0;
        }
    }
    size_t back=at;
    while (back && (t[back-1]==' ' || t[back-1]=='\t')) --back;
    if (!back) return 0;
    const unsigned char before=t[back-1];
    if (!letter(before) && !(before=='.' && back>=2 && digit(t[back-2]))) return 0;
    *digits_end=end;*value=v;
    return 1;
}

static int append_ordinal(struct annotation_builder *b,const unsigned char *text,
    size_t at,size_t *next_at)
{
    size_t end=0;uint64_t value=0;
    if (!ordinal_at(text,b->input_length,at,&end,&value)) return 1;
    b->ctx_text=text+at;b->ctx_length=1;b->ctx_fixed=1;
    int result=reloq_german_ordinal_emit(value,ordinal_dative(text,at),append_observed,b);
    b->ctx_text=NULL;
    if (result) return result;
    ++b->approximate;
    *next_at=end+1;
    return 0;
}

static int append_number(struct annotation_builder *b,
    const unsigned char *text,size_t length,size_t *at)
{
    size_t consumed=0,first_unit=b->unit_count;
    b->ctx_text=text+*at;b->ctx_length=1;b->ctx_fixed=1;
    struct reloq_german_number_context number_context={date_day_suffix(text,*at)};
    b->numeric_output=1;
    int result=reloq_german_number_read_ctx(text+*at,length-*at,&consumed,append_observed,b,&number_context);
    b->numeric_output=0;
    b->ctx_text=NULL;
    if (result) return result;
    size_t digits=0;while(digits<consumed&&digit(text[*at+digits]))++digits;
    if(digits==consumed && (digits==1||text[*at]!='0'))
        for(size_t i=first_unit;i<b->unit_count;++i) {
            b->units[i].text=text+*at;b->units[i].length=consumed;
            b->units[i].fixed_phrase=RELOQ_DE_SOURCE_NUMBER;
        }
    *at+=consumed;
    ++b->approximate;
    return 0;
}

/* Single-character names verified against the public German reference
 * output (including CP1252-resolved recordings where required). Only map
 * the exact scalar: NBSP/soft hyphen are deliberately left out so ordinary
 * document spacing and discretionary hyphens are never read as letters. */
struct observed_symbol_name {
    unsigned char code;
    const char *groups[3];
    unsigned char count;
};
static const struct observed_symbol_name observed_symbol_names[] = {
    {0x98,{".1tIl.0d@"},1},            /* ˜ */
    {0xa1,{".1Um.0g@.2ker.0t@s", ".1'aw's.2ru.0f@.2T'aj'.0X@n"},2},
    {0xa4,{".2In.0tEr.0nA.0Tjo.1na.0l@s", ".1v'E:'.0rUGs.2T'aj'.0X@n"},2},
    {0xa6,{".1zEGk.2rEX.0tR", ".1StrIX"},2},
    {0xa8,{".1tre.0mA"},1},
    {0xaa,{".1hox.0g@.2StEl.0t@s", ".1a"},2},
    {0xac,{".1nIXt", ".0zOr.1tir.2T'aj'.0X@n"},2},
    {0xaf,{".1bAl.0k@n"},1},
    {0xb1,{".1plUs", ".1mi.0nUs"},2},
    {0xb6,{".2pA.0rA.1graf"},1},
    {0xb7,{".1mal"},1},
    {0xb8,{".0se.1di.0@"},1},
    {0xb9,{".1hox.0g@.2StEl.0t@", ".1'aj'ns"},2},
    {0xba,{".1hox.0g@.2StEl.0t@s", ".1o"},2},
    {0xbc,{".1'aj'n.2fIr.0t@l"},1},
    {0xbd,{".0'aj'n.1hAlp"},1},
    {0xbe,{".1dr'aj'.2fIr.0t@l"},1},
    {0xbf,{".1Um.0g@.2ker.0t@s", ".1fra.0g@.2T'aj'.0X@n"},2}
};
static int append_observed_symbol(struct annotation_builder *b,
    unsigned char symbol)
{
    for (size_t i=0;i<sizeof observed_symbol_names/
            sizeof observed_symbol_names[0];++i) {
        const struct observed_symbol_name *name=observed_symbol_names+i;
        if (symbol!=name->code) continue;
        for (size_t k=0;k<name->count;++k) {
            int rc=append_observed(b,name->groups[k],0);
            if (rc) return rc;
        }
        return 0;
    }
    return 1;
}

static int append_native_character(struct annotation_builder *,uint32_t,const unsigned char *,int);
static int append_symbol_inner(struct annotation_builder *b, unsigned char symbol);
static int append_symbol(struct annotation_builder *b,const unsigned char *source,int contextual)
{
    b->ctx_text=source;b->ctx_length=1;b->ctx_fixed=1;
    int r=1;
    if(contextual) {
        unsigned char text[5];size_t n=0,at=0;uint32_t scalar=0;
        if(!reloq_cp1252_decode(source,1,text,sizeof text,&n,NULL)&&
           reloq_utf8_next(text,n,&at,&scalar)==1)
            r=append_native_character(b,scalar,source,RELOQ_DE_SOURCE_SYMBOL);
    }
    if(r==1)r=append_symbol_inner(b,*source);
    b->ctx_text=NULL;
    return r;
}
static int append_symbol_inner(struct annotation_builder *b, unsigned char symbol)
{
    int named=append_observed_symbol(b,symbol);
    if (named!=1) return named;
    const char *observed=symbol=='@'?".1klA.0mR.2A.0f@":
        symbol=='#'?".1nU.0mRn.2T'aj'.0X@n":
        symbol=='_'?".1Un.0tR.2StrIX":
        symbol=='-'?".1bIn.0d@.2StrIX":
        symbol=='+'?".1plUs":symbol==0xa7?".2pA.0rA.1graf":
        symbol=='='?".1gl'aj'Xt":symbol==0xb0?".1grat":
        symbol=='^'?".0kOn.1trOl.2T'aj'.0X@n":symbol=='~'?".1tIl.0d@":
        symbol=='|'?".1zEGk.2rEX.0tR] `[.1StrIX":
        symbol==0xa9?".2ko.0py.1rikt":symbol==0xae?".1'aj'n.0g@.2tra.0g@.0n@s] `[.1va.0r@n.2T'aj'.0X@n":
        symbol==0xa3?".1PUnt] `[.1Ster.0lIG":symbol==0xa5?".1jEn":
        symbol==0xa2?".1TEnt":symbol==0xb5?".1my":
        symbol==0xf7?".2di.0vi.1zjons.2T'aj'.0X@n":NULL;
    if (observed) return append_observed(b,observed,0);
    if (symbol == 0xb2 || symbol == 0xb3) {
        int result=append_observed(b,".1hox.0g@.2StEl.0t@",0);
        return result?result:append_observed(b,symbol==0xb2?".1Tv'aj'":".1dr'aj'",0);
    }
    if (symbol=='\\') {
        int result=append_observed(b,".0In.1vEr.0zR",0);
        return result?result:append_observed(b,".1Sr'E:'k.2StrIX",0);
    }
    if (symbol=='<' || symbol=='>') {
        int result=append_observed(b,symbol=='<'?".1kl'aj'.0nR":".1gr'oe'.0sR",0);
        return result?result:append_observed(b,".0Als",1);
    }
    const char *name = symbol == '/' ? "Schr\344gstrich" :
                       symbol == '\\' ? "Backslash" :
                       symbol == '{' || symbol == '}' ? "Klammer" :
                       symbol == '<' ? "kleiner" :
                       symbol == '>' ? "gr\366\337er" :
                       symbol == '$' ? "Dollar" :
                       symbol == 0xa7 ? "Paragraph" :
                       symbol == 0xa9 ? "Copyright" :
                       symbol == 0xb0 ? "Grad" :
                       symbol == 0xd7 ? "Mal" :
                       symbol == '+' ? "Plus" :
                       symbol == '=' ? "gleich" :
                       symbol == '%' ? "Prozent" :
                       symbol == '@' ? "At" :
                       symbol == '_' ? "Unterstrich" :
                       symbol == '&' ? "und" :
                       symbol == '#' ? "Raute" :
                       symbol == '*' ? "Stern" :
                       symbol == 0x80 ? "Euro" : NULL;
    if (!name) {
        if (literal(symbol)) return append_literal(b, symbol);
        /* A CP1252 character may be outside this provisional German
         * alphabet (symbols, Latin loans, controls copied from a document).
         * Never let one such scalar reject the whole NVDA utterance. The
         * explicit placeholder does not invent a pronunciation for it. */
        name = "Zeichen";
    }
    int result = append_number_word(b, name);
    if (!result) ++b->approximate;
    return result;
}

/* Only spelling recognition lives here. The native C normalization owns
 * readings, group counts, punctuation and the current abbreviation option.
 * These keys identify the native abbreviations formerly frozen in RLEX. */
static int abbrev_word_edge(unsigned char c)
{return !letter(c) && !digit(c) && c!='_';}
static int append_abbreviation(struct annotation_builder *b,
    const unsigned char *text,size_t length,size_t *at)
{
    size_t start=*at,end=start;
    if(start&&!abbrev_word_edge(text[start-1]))return 1;
    while(end<length&&letter(text[end]))++end;
    if(end==start)return 1;
    size_t first=end-start;
    if(first==1 && end<length && text[end]=='.') {
        size_t next=end+1,letters=1;
        while(letters<8) {
            while(next<length&&(text[next]==' '||text[next]=='\t'))++next;
            if(next>=length||!letter(text[next])||
               (next+1<length&&letter(text[next+1])))break;
            end=++next;++letters;
            if(next==length||text[next]!='.')break;
            end=++next;
        }
        if(letters<2)return 1;
    } else {
        static const char *const keys[]={
        "Apr","Aug","DM","Dez","Do","Dr","Feb","Febr",
        "Fr","Jul","Jun","Mi","Mio","Mo","Mrd","Mrz",
        "Nov","Nr","Nrn","Okt","Sa","Sep","Sept","Str",
        "Tel","Unauff","bds","bzw","ca","cm","derz","dl",
        "dz","dzt","dztg","etc","evtl","inkl","kg","km",
        "li","mg","min","mm","re","schriftl","sep","t\344gl",
        "unauff","vgbl","vgl","usw","Dr.-Ing","Prof","geb","Di",
        "So","Jan"
        };
        int matched=0;
        for(size_t i=0;i<sizeof keys/sizeof *keys;++i) {
            size_t n=strlen(keys[i]);
            if(n>length-start||memcmp(text+start,keys[i],n)||
               (start+n<length&&!abbrev_word_edge(text[start+n])))continue;
            if(i>=sizeof keys/sizeof *keys-5 &&
               (start+n==length||text[start+n]!='.'))continue;
            if(start+n>end)end=start+n;
            matched=1;
        }
        if(!matched)return 1;
        if(end<length&&text[end]=='.')++end;
    }
    int status=append_native_token(b,text+start,end-start,NATIVE_TOKEN_ABBREVIATION);
    if(!status)*at=end;
    return status;
}

/* The original single_chars -> char_name terminal path serves original
 * CP1252 characters and canonical Latin extensions through one interface. */
static int append_isolated_character(struct annotation_builder *b,uint32_t scalar)
{
    if(scalar<128)return 1;
    struct reloq_character_reading reading;
    int rc=reloq_character_read(scalar,0,&reading);
    if(rc!=1)return rc<0?-1:1;
    for(size_t k=0;k<reading.count;++k) {
        const char *end=strchr(reading.groups[k],']');
        if(!end || (size_t)(end-reading.groups[k])>=256)return -1;
        char body[256];size_t n=(size_t)(end-reading.groups[k]);
        memcpy(body,reading.groups[k],n);body[n]=0;
        rc=append_observed(b,body,0);if(rc)return rc;
    }
    return 0;
}

static int append_native_character(struct annotation_builder *b,uint32_t scalar,
    const unsigned char *source,int kind)
{
    if(!reloq_character_native_available(scalar,0))return 1;
    struct reloq_character_reading reading;
    int found=reloq_character_read_native(scalar,0,&reading);
    if(found!=1 || !reading.original_spr)return found<0?-1:1;
    const unsigned char *s=(const unsigned char *)reading.original_spr;
    size_t n=strlen((const char *)s);
    for(size_t at=0;at<n;) {
        struct reloq_inline_tag tag=reloq_inline_tag_read(s+at,n-at,1);
        size_t count=tag.length?tag.length:1;
        if(b->used>SIZE_MAX-count-1 || reserve(&b->output,&b->capacity,b->used+count+1))return -2;
        memcpy(b->output+b->used,s+at,count);b->used+=count;
        if(tag.kind==RELOQ_TAG_SPR) {
            if(remember_word(b,(const unsigned char *)"",0,s+at,count,0) ||
               push_unit(b,source,1,0,kind))return -2;
        }
        at+=count;
    }
    return 0;
}

/* The native mail class has a dotted host with a two- or three-letter
 * suffix. Other @ sequences retain ordinary word and character analysis. */
static size_t email_end(const unsigned char *s,size_t n,size_t at,size_t *at_sign)
{
    size_t p=at;
    if(p>=n || (!letter(s[p])&&!digit(s[p])) ||
       (p && (s[p-1]=='@'||s[p-1]=='"')))return 0;
    while(p<n && (letter(s[p])||digit(s[p])||strchr("._-+'%!/",s[p])))++p;
    if(p==at || p>=n || s[p]!='@' || s[p-1]=='.')return 0;
    *at_sign=p++;
    size_t label=p,last=0;int dots=0;
    for(;;) {
        label=p;
        if(p>=n || (!letter(s[p])&&!digit(s[p])))return 0;
        while(p<n && (letter(s[p])||digit(s[p])||s[p]=='-'))++p;
        if(s[p-1]=='-')return 0;
        last=p-label;
        if(p+1<n && s[p]=='.' && (letter(s[p+1])||digit(s[p+1]))) {++dots;++p;continue;}
        break;
    }
    /* A bare @ between words is the original character-name path. The
     * mail reader requires an actual dotted host and a complete suffix. */
    if(!dots || last<2 || last>3 || (p<n && (s[p]=='@'||s[p]=='_')) )return 0;
    if(dots)for(size_t i=label;i<p;++i)if(!letter(s[i]))return 0;
    return p;
}

static int append_email(struct annotation_builder *b,const unsigned char *s,
    size_t start,size_t at_sign,size_t end)
{
    int all_letters=1;
    for(size_t i=start;i<at_sign;++i)if(!letter(s[i]))all_letters=0;
    size_t p=start;
    while(p<end) {
        int r=0;size_t first=b->unit_count;
        if(p==at_sign) {
            if(append_literal(b,','))return -2;
            r=append_observed(b,".1Et",0);++p;
        } else if(letter(s[p])) {
            size_t a=p;while(p<end && letter(s[p]))++p;
            size_t n=p-a;
            if(p<=at_sign && (all_letters || n<=3))r=append_letters(b,s+a,n);
            else if(a>at_sign && (n<=2||n>32))r=append_letters(b,s+a,n);
            else {
                unsigned char *key=malloc(n+1);if(!key)return -2;
                for(size_t k=0;k<n;++k)key[k]=fold_lower(s[a+k]);
                key[n]=0;
                b->src_override=s+a;b->src_override_length=n;
                /* These are domain-label pronunciations from the hostname
                 * reader, not whole-word exceptions in ordinary text. */
                if(a>at_sign && n==3 && !memcmp(key,"com",3))r=append_observed(b,".1kom",0);
                else if(a>at_sign && n==3 && !memcmp(key,"net",3))r=append_observed(b,".1net",0);
                else if(a>at_sign && n==3 && !memcmp(key,"edu",3))r=append_observed(b,".1e.0du",0);
                else if(a>at_sign && n==3 && !memcmp(key,"gov",3))r=append_letters(b,s+a,n);
                else r=append_word_token(b,key,n);
                b->src_override=NULL;
                /* Prosody source spans must outlive this temporary fold. */
                for(size_t k=first;k<b->unit_count;++k) {
                    b->units[k].text=s+a;b->units[k].length=n;
                }
                free(key);
            }
        } else if(digit(s[p])) {
            size_t a=p;
            r=append_number(b,s,p<at_sign?p+1:end,&p);
            if(p<=a)return -1;
        } else {
            unsigned char c=s[p++];
            r=c=='.'?append_observed(b,".1pUGkt",0):append_symbol(b,s+p-1,0);
            if(!r && c=='.' && p>at_sign && b->unit_count)
                b->units[b->unit_count-1].force_deaccent=1;
            while(p<at_sign && c=='.' && s[p]=='.')++p;
        }
        if(r)return r;
        if(append_literal(b,' '))return -2;
    }
    return 0;
}

static void map_boundaries(
    struct reloq_german_best_effort_boundary *boundaries,
    size_t boundary_count, size_t *next, size_t encoded_end,
    size_t annotation_end)
{
    while (*next < boundary_count &&
           boundaries[*next].encoded_offset <= encoded_end)
        boundaries[(*next)++].annotation_offset = annotation_end;
}

static int next_annotation_word(const unsigned char *annotation,
                                size_t length,
                                struct reloq_german_annotation_cursor *cursor,
                                size_t *offset,
                                int (*cancelled)(void *), void *context)
{
    struct reloq_german_annotation_event event;
    int state;
    while ((state = reloq_german_annotation_next(
                annotation, length, cursor, 0, &event)) == 1) {
        if (cancelled && cancelled(context)) return 2;
        if (event.kind == RELOQ_DE_ANNOTATION_SPR &&
            event.spr.kind == RELOQ_DE_SPR_WORD_OPEN) {
            *offset = event.offset;
            return 1;
        }
    }
    return state;
}

/* Reaccenting can insert `00 before an SPR group. Indexes were attached
 * to original source tokens before that insertion; carry them across the
 * change by comparing group-open offsets, never by guessing from words.
 * Exact group-boundary indexes remain before the added control, and still
 * select the same following acoustic word. Commit only after both streams
 * and all offsets validate. */
static int remap_accented_boundaries(
    const unsigned char *before, size_t before_length,
    const unsigned char *after, size_t after_length,
    struct reloq_german_best_effort_boundary *boundaries, size_t count,
    int (*cancelled)(void *), void *context)
{
    if (!count) return 0;
    if (count > SIZE_MAX / sizeof(size_t) || after_length < before_length)
        return -2;
    size_t *mapped = malloc(count * sizeof *mapped);
    if (!mapped) return -2;
    struct reloq_german_annotation_cursor old_cursor = {0},
                                            new_cursor = {0};
    size_t old_offset = 0, new_offset = 0, index = 0, shift = 0;
    int result = 0;
    for (;;) {
        int original = next_annotation_word(
            before, before_length, &old_cursor, &old_offset,
            cancelled, context);
        int accented = next_annotation_word(
            after, after_length, &new_cursor, &new_offset,
            cancelled, context);
        if (original == 2 || accented == 2) { result = 2; break; }
        if (original != accented || original < 0) {
            result = -1;
            break;
        }
        if (!original) break;
        if (new_offset < old_offset ||
            new_offset - old_offset < shift ||
            new_offset - old_offset > after_length - before_length) {
            result = -1;
            break;
        }
        while (index < count &&
               boundaries[index].annotation_offset <= old_offset) {
            if (boundaries[index].annotation_offset > SIZE_MAX - shift) {
                result = -1;
                break;
            }
            mapped[index] = boundaries[index].annotation_offset + shift;
            ++index;
        }
        if (result) break;
        shift = new_offset - old_offset;
    }
    if (!result && shift != after_length - before_length) result = -1;
    while (!result && index < count) {
        if (boundaries[index].annotation_offset > before_length ||
            boundaries[index].annotation_offset > SIZE_MAX - shift)
            result = -1;
        else mapped[index] = boundaries[index].annotation_offset + shift;
        ++index;
    }
    if (!result) for (size_t i = 0; i < count; ++i)
        boundaries[i].annotation_offset = mapped[i];
    free(mapped);
    return result;
}

static int remap_contextual_boundaries(
    const unsigned char *before, size_t before_length,
    const unsigned char *after, size_t after_length,
    struct reloq_german_best_effort_boundary *boundaries, size_t count,
    int (*cancelled)(void *), void *context)
{
    if (!count) return 0;
    if (count > SIZE_MAX / sizeof(size_t))
        return -2;
    size_t *mapped = malloc(count * sizeof *mapped);
    if (!mapped) return -2;
    struct reloq_german_annotation_cursor old_cursor = {0},
                                            new_cursor = {0};
    size_t old_offset = 0, new_offset = 0, index = 0;
    int result = 0;
    for (;;) {
        int original = next_annotation_word(
            before, before_length, &old_cursor, &old_offset,
            cancelled, context);
        int accented = next_annotation_word(
            after, after_length, &new_cursor, &new_offset,
            cancelled, context);
        if (original == 2 || accented == 2) { result = 2; break; }
        if (original != accented || original < 0) {
            result = -1;
            break;
        }
        if (!original) break;
        while (index < count &&
               boundaries[index].annotation_offset <= old_offset) {
            mapped[index] = boundaries[index].annotation_offset ? new_offset : 0;
            ++index;
        }
        if (result) break;
    }
    while (!result && index < count) {
        if (boundaries[index].annotation_offset > before_length)
            result = -1;
        else mapped[index] = after_length;
        ++index;
    }
    if (!result) for (size_t i = 0; i < count; ++i)
        boundaries[i].annotation_offset = mapped[i];
    free(mapped);
    return result;
}

static int next_group(const unsigned char *s,size_t n,
    struct reloq_german_annotation_cursor *cursor,size_t *start,size_t *end)
{
    struct reloq_german_annotation_event e;int state;
    while((state=reloq_german_annotation_next(s,n,cursor,UINT32_MAX,&e))==1) {
        if(e.kind!=RELOQ_DE_ANNOTATION_SPR)continue;
        if(e.spr.kind==RELOQ_DE_SPR_WORD_OPEN)*start=e.offset;
        else if(e.spr.kind==RELOQ_DE_SPR_WORD_CLOSE){*end=e.offset+e.length;return 1;}
    }
    return state;
}
static int next_phone(const unsigned char *s,size_t n,
    struct reloq_german_spr_cursor *c,struct reloq_german_spr_event *e)
{
    int state;
    while((state=reloq_german_spr_next(s,n,c,e))==1)
        if(e->kind==RELOQ_DE_SPR_PHONE)return 1;
    return state;
}
static int same_word_shape(const unsigned char *a,size_t an,const unsigned char *b,size_t bn)
{
    struct reloq_german_spr_cursor ac={0},bc={0};
    struct reloq_german_spr_event ae,be;int ar,br;
    do {
        ar=next_phone(a,an,&ac,&ae);br=next_phone(b,bn,&bc,&be);
        if(ar!=br || ar<0)return 0;
        if(ar && (ae.phone!=be.phone || ae.syllable!=be.syllable))return 0;
    } while(ar);
    return 1;
}
/* Context may select a pronunciation that differs from an isolated lexicon
 * entry. Retain our accepted acoustic facts when the phone/syllable shape
 * agrees; otherwise resolve those facts for the selected reading anew. */
static int contextual_features(struct annotation_builder *b,
    const unsigned char *after,size_t after_length,
    const struct reloq61_german_word_facts *facts)
{
    if(!b->retain_features)return 0;
    struct annotation_builder next={.lexicon=b->lexicon,.retain_features=1,
        .cancelled=b->cancelled,.cancel_context=b->cancel_context};
    struct reloq_german_annotation_cursor ac={0},bc={0};size_t known=0;
    for(size_t i=0;i<b->unit_count;++i) {
        size_t a=0,ae=0,c=0,ce=0;
        if(next_group(b->output,b->used,&ac,&a,&ae)!=1 ||
           next_group(after,after_length,&bc,&c,&ce)!=1)goto invalid;
        while(known<b->feature_count && b->features[known].word_index<i)++known;
        if(known<b->feature_count && b->features[known].word_index==i &&
           same_word_shape(b->output+a,ae-a,after+c,ce-c)) {
            if(next.feature_count==next.feature_capacity) {
                size_t cap=next.feature_capacity?2*next.feature_capacity:8;
                if(cap<next.feature_capacity||cap>SIZE_MAX/sizeof *next.features)goto invalid;
                void *p=realloc(next.features,cap*sizeof *next.features);if(!p)goto invalid;
                next.features=p;next.feature_capacity=cap;
            }
            next.features[next.feature_count++]=b->features[known];++next.word_count;
        } else if(remember_word(&next,b->units[i].text,b->units[i].length,after+c,ce-c,0))goto invalid;
        if(facts&&facts[i].syllables) {
            struct reloq_german_word_features f={0};
            int existing=next.feature_count&&next.features[next.feature_count-1].word_index==i;
            if(existing)f=next.features[next.feature_count-1];
            if(reloq_german_word_features_native_onsets(after+c,ce-c,facts[i].glottal_syllables,
                facts[i].syllables,&f)) {
                f.word_index=i;
                if(existing)next.features[next.feature_count-1]=f;
                else {
                    if(next.feature_count==next.feature_capacity) {
                        size_t cap=next.feature_capacity?2*next.feature_capacity:8;
                        if(cap<next.feature_capacity||cap>SIZE_MAX/sizeof f)goto invalid;
                        void *p=realloc(next.features,cap*sizeof f);if(!p)goto invalid;
                        next.features=p;next.feature_capacity=cap;
                    }
                    next.features[next.feature_count++]=f;
                }
            }
        }
    }
    free(b->features);b->features=next.features;b->feature_count=next.feature_count;
    b->feature_capacity=next.feature_capacity;return 0;
invalid:free(next.features);return -1;
}

#include "reloq_german_dictionary_text.inc"

static int plan_tags_checked(int native_first,
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_lexicon *lexicon, int process_tags,
    unsigned char **annotation, size_t *annotation_length,
    size_t *approximate_words, size_t *encoded_error_offset,
    struct reloq_german_best_effort_boundary *boundaries,
    size_t boundary_count,
    struct reloq_german_word_features **features,size_t *feature_count,
    int (*cancelled)(void *), void *cancel_context)
{
    if ((!utf8 && utf8_length) || !annotation || !annotation_length ||
        !approximate_words || (boundary_count && !boundaries) ||
        (!!features != !!feature_count)) return -1;
    if (!utf8) return -1;
    size_t encoded_length = 0;
    if (cancelled && cancelled(cancel_context)) return 2;
    int state = reloq_character_encode_text(
        utf8, utf8_length, NULL, NULL, 0, &encoded_length,
        cancelled, cancel_context);
    if (state == 1) return 2;
    if (state || encoded_length == SIZE_MAX) return state ? -1 : -2;
    unsigned char *encoded = malloc(encoded_length + 1);
    if (!encoded) return -2;
    uint32_t *original_scalars=malloc((encoded_length+1)*sizeof *original_scalars);
    if(!original_scalars){free(encoded);return -2;}
    state = reloq_character_encode_text(
        utf8, utf8_length, encoded, original_scalars, encoded_length + 1,
        &encoded_length, cancelled, cancel_context);
    if (state) {
        free(original_scalars);free(encoded);
        return state == 1 ? 2 : state == -3 ? -2 : -1;
    }
    for(size_t i=0;i<boundary_count;++i)if(boundaries[i].encoded_offset>encoded_length||
       (i&&boundaries[i].encoded_offset<boundaries[i-1].encoded_offset)) {
        free(original_scalars);free(encoded);return -1;
    }
    size_t *main_map=NULL,main_source_length=encoded_length;
    state=normalize_main_text(&encoded,&original_scalars,&encoded_length,
        process_tags,&main_map,cancelled,cancel_context);
    if(state){free(original_scalars);free(encoded);return state;}
    size_t normalized_error=SIZE_MAX;
    size_t *source_error_offset=encoded_error_offset;
    if(main_map&&encoded_error_offset)encoded_error_offset=&normalized_error;
    struct reloq_german_best_effort_boundary *source_boundaries=boundaries;
    if(main_map&&boundary_count) {
        boundaries=malloc(boundary_count*sizeof *boundaries);
        if(!boundaries){free(main_map);free(original_scalars);free(encoded);return -2;}
        for(size_t i=0;i<boundary_count;++i)boundaries[i]=(struct reloq_german_best_effort_boundary){
            main_map[source_boundaries[i].encoded_offset],0};
    }
    unsigned char *original_encoded=malloc(encoded_length+1);
    if (!original_encoded) {
        if(boundaries!=source_boundaries)free(boundaries);
        free(main_map);free(original_scalars);free(encoded);return -2;
    }
    memcpy(original_encoded,encoded,encoded_length+1);
    for (size_t i = 0; i < encoded_length; ++i)
        encoded[i] = loan_letter(encoded[i]);
    for (size_t i = 0; i < boundary_count; ++i)
        if (boundaries[i].encoded_offset > encoded_length ||
            (i && boundaries[i].encoded_offset <
                      boundaries[i - 1].encoded_offset)) {
            if(boundaries!=source_boundaries)free(boundaries);
            free(main_map);free(original_scalars);free(original_encoded);
            free(encoded);
            return -1;
        }
    /* Keep input offsets intact while allowing an internal apostrophe to
     * disappear from the analyzed word. Unit pointers remain valid through
     * the later sentence-accent pass. */
    unsigned char *word_storage=malloc(encoded_length+1);
    if (!word_storage) {
        if(boundaries!=source_boundaries)free(boundaries);
        free(main_map);free(original_scalars);free(original_encoded);free(encoded);return -2;
    }
    memcpy(word_storage,encoded,encoded_length+1);
    struct reloq61_text_options initial_options=reloq61_text_options_current();
    struct annotation_builder b = {0};
    b.native_first=native_first;
    b.prediction_seen=initial_options.phrase_prediction;
    if(process_tags==1)for(size_t i=0;i<encoded_length;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(encoded+i,encoded_length-i,0);
        if(t.kind==RELOQ_TAG_PREDICTION && t.value)b.prediction_seen=1;
        i+=t.length?t.length:1;
    }
    struct reloq_lexicon empty_lexicon={0};
    const struct reloq_lexicon_resolver *resolver=reloq61_dictionary_resolver();
    const struct reloq_lexicon_resolver *previous_resolver=reloq_lexicon_resolver_scope(resolver);
    b.lexicon = lexicon?lexicon:&empty_lexicon;
    b.cancelled = cancelled;
    b.cancel_context = cancel_context;
    b.retain_features=features!=NULL;
    b.input_length=encoded_length;
    /* Main has already normalized the complete text. Its replacement words
     * enter grammar once; a native sub-analysis must not reapply text aliases.
     * Literal Main pronunciations and Root lookup keep their own stages. */
    int previous_text_scope=reloq61_dictionary_text_scope(1);
    size_t at = 0, next_boundary = 0, phrase_word_start = 0, url_end = 0, url_weak_dots_end = 0;
    int result = 0, spelling = 0,retry=0;
    while (at < encoded_length) {
        if (cancelled && cancelled(cancel_context)) {
            result = 2;
            goto done;
        }
        map_boundaries(boundaries, boundary_count, &next_boundary,
                       at, b.used);

        if(process_tags && encoded[at]=='`') {
            struct reloq_inline_tag tag=reloq_inline_tag_read(encoded+at,encoded_length-at,0);
            /* Mode 2 is trusted Unicode character admission. Only the SPR,
               character mode and prominence generated there are annotations;
               ordinary source text has already had controls neutralized. */
            if(process_tags==2 && tag.kind!=RELOQ_TAG_SPR &&
               tag.kind!=RELOQ_TAG_PROMINENCE && tag.kind!=RELOQ_TAG_SPELL)tag=(struct reloq_inline_tag){0};
            if(tag.kind==RELOQ_TAG_DICTIONARY || tag.kind==RELOQ_TAG_PREDICTION) {
                struct reloq61_text_options next=reloq61_text_options_current();
                if(tag.kind==RELOQ_TAG_DICTIONARY)next.dictionary_disabled=!tag.value;
                else next.phrase_prediction=tag.value;
                reloq61_text_options_scope(next);
            }
            if(tag.kind==RELOQ_TAG_SPELL) spelling=tag.value;
            if(tag.kind==RELOQ_TAG_PROMINENCE) {
                /* The original consumes 5..9 without changing emphasis. */
                if(tag.value>4){at+=tag.length;continue;}
                if(process_tags==1)b.pending_emphasis=tag.length==3?1:tag.value+2;
            }
            else if(tag.kind==RELOQ_TAG_SPR &&
                reloq_german_lexicon_validate_spr(encoded+at,tag.length,NULL)!=1) tag.kind=RELOQ_TAG_NONE;
            if(tag.kind==RELOQ_TAG_SPR) {
                if(append_literal(&b,' ')) {result=-2;goto done;}
                for(size_t k=0;k<tag.length;++k)
                    if(append_literal(&b,encoded[at+k])) {result=-2;goto done;}
                if(remember_word(&b,(const unsigned char *)"",0,encoded+at,tag.length,0) ||
                   push_unit(&b,(const unsigned char *)"",0,0,1)) {result=-2;goto done;}
                b.units[b.unit_count-1].preserve=1;
            } else if(tag.kind && tag.kind!=RELOQ_TAG_SPELL) {
                if(append_literal(&b,' ')) {result=-2;goto done;}
                for(size_t k=0;k<tag.length;++k) {
                    if(tag.kind==RELOQ_TAG_PAUSE && k==1 && append_literal(&b,'v')) {result=-2;goto done;}
                    if(append_literal(&b,encoded[at+k])) {result=-2;goto done;}
                }
                if(append_literal(&b,' ')) {result=-2;goto done;}
            }
            if(tag.kind) {at+=tag.length;continue;}
            if(spelling) {
                /* An unrecognized backquote is ordinary input to the
                   active character mode. ts1 discards it; ts2 names it. */
                if(spelling==2) {
                    if(append_literal(&b,' ')){result=-2;goto done;}
                    result=append_native_character(&b,original_scalars[at],original_encoded+at,RELOQ_DE_SOURCE_CHARACTER);
                    if(result)goto done;
                }
                ++at;continue;
            }
            /* Unknown control words follow the reference's character-name
             * path. They never become executable product instructions. */
            size_t end=at+1;
            while(end<encoded_length && (letter(encoded[end])||digit(encoded[end])))++end;
            result=append_word(&b,(const unsigned char *)"Gravis",6);
            for(++at;!result && at<end;++at) {
                if(append_literal(&b,' ')) {result=-2;break;}
                if(letter(encoded[at]))result=append_letters(&b,encoded+at,1);
                else {size_t off=at;result=append_number(&b,encoded,at+1,&off);}
            }
            if(result)goto done;
            continue;
        }
        if(process_tags && spelling) {
            if(spelling==1 && !reloq_character_spelling1(original_scalars[at])) {++at;continue;}
            if(append_literal(&b,' ')) {result=-2;goto done;}
            result=append_native_character(&b,original_scalars[at],original_encoded+at,RELOQ_DE_SOURCE_CHARACTER);
            if(result==1&&original_encoded[at]>=0x80&&
               !reloq_character_native_available(original_scalars[at],0))result=0;
            if(result==1)result=append_isolated_character(&b,original_scalars[at]);
            if(result==1) {
                if(letter(encoded[at]))result=append_letters(&b,encoded+at,1);
                else if(digit(encoded[at])) {size_t off=at;result=append_number(&b,encoded,at+1,&off);}
                else result=append_symbol(&b,original_encoded+at,1);
            }
            if(result)goto done;
            ++at;continue;
        }
        size_t dictionary_length=reloq61_dictionary_match(original_encoded+at,encoded_length-at);
        if(dictionary_length) {
            result=append_native_token(&b,original_encoded+at,dictionary_length,1);
            if(result!=1) {
                if(result)goto done;
                at+=dictionary_length;continue;
            }
        }
        size_t mail_at=0,mail_start=at+(encoded[at]=='<');
        int mail_boundary=!at || (!letter(encoded[at-1])&&!digit(encoded[at-1])&&
            encoded[at-1]!='.'&&encoded[at-1]!='_'&&encoded[at-1]!='-');
        size_t mail_end=mail_boundary?email_end(encoded,encoded_length,mail_start,&mail_at):0;
        if(mail_end) {
            result=append_email(&b,encoded,mail_start,mail_at,mail_end);
            if(result)goto done;
            at=mail_end;
            if(mail_start && encoded[mail_start-1]=='<' && at<encoded_length && encoded[at]=='>')++at;
            map_boundaries(boundaries,boundary_count,&next_boundary,at,b.used);
            continue;
        }
        if((at==0||reloq_german_silent_separator(encoded[at-1]))&&
           ((encoded_length-at>=7&&!memcmp(encoded+at,"http://",7))||
            (encoded_length-at>=8&&!memcmp(encoded+at,"https://",8))||
            (encoded_length-at>=4&&!memcmp(encoded+at,"www.",4)))) {
            url_end=at;
            while(url_end<encoded_length&&!reloq_german_silent_separator(encoded[url_end]))++url_end;
            size_t host=at;
            const int bare=encoded[at]=='w';
            if(!bare)host+=encoded[at+4]=='s'?8:7;
            size_t path=host;
            while(path<url_end&&encoded[path]!='/')++path;
            /* The observed domain-reading path deaccents dots in bare www
             * addresses. A scheme with a path uses ordinary symbol stress. */
            url_weak_dots_end=(bare||path==url_end)?path:0;
        }
        if(at<url_end&&encoded[at]=='.'&&at&&letter(encoded[at-1])&&
           (at<2||!letter(encoded[at-2]))) {
            ++at;continue;
        }
        if(at<url_end&&encoded[at]=='-') {
            result=append_observed(&b,".1bIn.0d@.2StrIX",0);
            ++at;if(result)goto done;
            continue;
        }
        if (reloq_german_silent_separator(encoded[at])) {
            result=append_literal(&b,' ');
            if (result) goto done;
            ++at;
            continue;
        }
        /* A lone diacritic letter is its own character-name token. It must
         * not inherit the loanword rule used for words such as Café. */
        if ((!at || !letter(encoded[at-1])) &&
            (at+1==encoded_length || !letter(encoded[at+1])) &&
            !(at+2<encoded_length&&encoded[at+1]=='-'&&letter(encoded[at+2]))) {
            size_t old_at=at;
            b.ctx_text=encoded+at;b.ctx_length=1;b.ctx_fixed=1;
            result=append_isolated_character(&b,original_scalars[at]);
            b.ctx_text=NULL;
            if (result!=1) {
                if (result) goto done;
                ++at;
                map_boundaries(boundaries,boundary_count,&next_boundary,at,b.used);
                continue;
            }
            at=old_at;
        }
        /* Match verified multi-token abbreviations before normal punctuation
         * can split them into letter names and sentence boundaries. */
        if (letter(encoded[at])) {
            size_t start=at;
            result=append_abbreviation(&b,encoded,encoded_length,&at);
            if (result!=1) {
                if (result) goto done;
                map_boundaries(boundaries,boundary_count,&next_boundary,at,b.used);
                continue;
            }
            at=start;
        }
        if (letter(encoded[at]) && at>=url_end) {
            size_t end=at;
            while(end<encoded_length&&letter(encoded[end]))++end;
            size_t connected=end;int slash=0;
            while(connected+1<encoded_length&&encoded[connected]=='/'&&letter(encoded[connected+1])) {
                slash=1;++connected;
                while(connected<encoded_length&&letter(encoded[connected]))++connected;
            }
            if(slash) {
                result=append_native_token(&b,original_encoded+at,connected-at,0);
                if(result!=1) {
                    if(result)goto done;
                    at=connected;map_boundaries(boundaries,boundary_count,&next_boundary,at,b.used);
                    continue;
                }
            }
            int hyphen=0;
            while (end+1<encoded_length && encoded[end]=='-' && letter(encoded[end+1])) {
                hyphen=1;++end;
                while (end<encoded_length && letter(encoded[end])) ++end;
            }
            if (hyphen) {
                result=append_hyphenated_word(&b,word_storage+at,end-at);
                if (result) goto done;
                at=end;
                map_boundaries(boundaries,boundary_count,&next_boundary,at,b.used);
                continue;
            }
        }
        if (encoded[at]=='-' && at+1<encoded_length && digit(encoded[at+1]) &&
            (!at || !letter(encoded[at-1]))) {
            int sign=at+1<encoded_length && digit(encoded[at+1]) &&
                (!at || (!letter(encoded[at-1]) && !digit(encoded[at-1])));
            size_t left=at,right=at+1;
            while(left&&digit(encoded[left-1]))--left;
            while(right<encoded_length&&digit(encoded[right]))++right;
            const int years=at-left==4&&right-at==5&&encoded[left]!='0'&&encoded[at+1]!='0';
            result=append_observed(&b,years?".0bIs":sign?".1mi.0nUs":".1bIn.0d@.2StrIX",years);
            ++at;
            if (result) goto done;
            map_boundaries(boundaries,boundary_count,&next_boundary,at,b.used);
            continue;
        }
        struct reloq_german_punctuation punctuation;
        if (reloq_german_punctuation_read(encoded,encoded_length,at,
                                         b.word_count>phrase_word_start,&punctuation)) {result=-1;goto done;}
        if (punctuation.action!=RELOQ_DE_PUNCT_UNKNOWN) {
            if (punctuation.action==RELOQ_DE_PUNCT_SPOKEN) {
                b.ctx_text=encoded+at;b.ctx_length=1;b.ctx_fixed=1;
                int kind=encoded[at]=='&'||encoded[at]==0xd7||encoded[at]==0xb7?
                    RELOQ_DE_SOURCE_SYMBOL:RELOQ_DE_SOURCE_CHARACTER;
                result=append_native_character(&b,original_scalars[at],original_encoded+at,kind);
                if(result==1)result=append_observed(&b,punctuation.spoken,punctuation.weak);
                if(!result&&encoded[at]=='.'&&at<url_weak_dots_end&&b.unit_count)
                    b.units[b.unit_count-1].force_deaccent=1;
                b.ctx_text=NULL;
            }
            else if (punctuation.action==RELOQ_DE_PUNCT_BOUNDARY) {
                size_t marks=punctuation.mark=='-'?punctuation.consumed:1;
                result=0;
                for(size_t i=0;i<marks&&!result;++i)
                    result=append_literal(&b,punctuation.mark);
            }
            else result=append_literal(&b,' ');
            if (punctuation.action==RELOQ_DE_PUNCT_BOUNDARY&&
                strchr(".?!:;",punctuation.mark))phrase_word_start=b.word_count;
            at+=punctuation.consumed;
            if (result) goto done;
            map_boundaries(boundaries,boundary_count,&next_boundary,at,b.used);
            continue;
        }
        if (literal(encoded[at])) {
            result = append_symbol(&b,original_encoded+at,1);++at;
            if (result) goto done;
            map_boundaries(boundaries, boundary_count, &next_boundary,
                           at, b.used);
            continue;
        }
        if (digit(encoded[at])) {
            size_t after=0;
            result=append_ordinal(&b,encoded,at,&after);
            if (result!=1) {
                if (result) goto done;
                at=after;
                map_boundaries(boundaries,boundary_count,&next_boundary,at,b.used);
                continue;
            }
            result = append_number(&b, encoded, encoded_length, &at);
            if (result) goto done;
            map_boundaries(boundaries, boundary_count, &next_boundary,
                           at, b.used);
            continue;
        }
        if (encoded[at] == '`') {
            /* A naked ECI control introducer is not ordinary punctuation.
             * The NVDA plain-text driver replaces it before this adapter. */
            result = -1;
            if (encoded_error_offset) *encoded_error_offset = at;
            goto done;
        }
        if (!letter(encoded[at])) {
            result = append_symbol(&b,original_encoded+at,1);++at;
            if (result) goto done;
            map_boundaries(boundaries, boundary_count, &next_boundary,
                           at, b.used);
            continue;
        }
        size_t start = at;
        size_t length=0;
        while (at < encoded_length && (letter(encoded[at]) ||
            (reloq_german_apostrophe(encoded[at]) && at>start &&
             at+1<encoded_length && letter(encoded[at-1]) && letter(encoded[at+1])))) {
            if (cancelled && (at & 255) == 0 &&
                cancelled(cancel_context)) {
                result = 2;
                goto done;
            }
            if (letter(encoded[at])) word_storage[start+length++]=encoded[at];
            ++at;
        }
        const unsigned char *word=word_storage+start;
        result=append_word_token(&b,word,length);
        /* A suspended constituent (Ein- und Ausgang) keeps lexical stress.
         * Its written hyphen distinguishes it from the function word ein. */
        if (!result && at<encoded_length && encoded[at]=='-' &&
            b.unit_count && length>1 &&
            (at+1==encoded_length || !letter(encoded[at+1]))) {
            b.units[b.unit_count-1].text=word_storage+start;
            b.units[b.unit_count-1].length=at-start+1;
        }
        if (result) {
            if (encoded_error_offset) *encoded_error_offset = start;
            goto done;
        }
        map_boundaries(boundaries, boundary_count, &next_boundary,
                       at, b.used);
    }
    if (cancelled && cancelled(cancel_context)) {
        result = 2;
        goto done;
    }
    if (reserve(&b.output, &b.capacity, b.used + 1)) {
        result = -2;
        goto done;
    }
    map_boundaries(boundaries, boundary_count, &next_boundary,
                   encoded_length, b.used);
    b.output[b.used] = 0;
    /* Phrase stress belongs to the native language frontend, not to a
     * screen-reader text substitution. The rule only changes safe,
     * one-to-one aligned grammatical classes and preserves explicit SPR. */
    unsigned char *accented = NULL;
    size_t accented_length = 0;
    reloq61_text_options_scope(initial_options);
    int contextual=0;
    struct reloq61_german_word_facts *native_facts=NULL;
    if((b.retain_features||b.prediction_seen)&&b.unit_count) {
        native_facts=calloc(b.unit_count,sizeof *native_facts);
        if(!native_facts){result=-2;goto done;}
    }
    if(b.lexicon && b.unit_count && b.unit_count==b.word_count) {
        struct reloq_german_context_source source={encoded,original_encoded,word_storage,encoded_length};
        contextual=reloq_rules61_german_context_features(b.units,b.unit_count,b.lexicon,b.output,b.used,
            &accented,&accented_length,native_facts,cancelled,cancel_context,&source);
        if(contextual<0 || contextual==2) {
            free(native_facts);result=contextual==2?2:-2;goto done;
        }
    }
    if(contextual!=1&&b.deferred_words) {
        free(native_facts);retry=1;goto done;
    }
    if(contextual==1)state=0;
    else if (b.unit_count && b.unit_count==b.word_count)
        state = reloq_german_generic_prosody_reaccent_units(
            b.units, b.unit_count, b.output, b.used,
            &accented, &accented_length, cancelled, cancel_context);
    else
        state = reloq_german_generic_prosody_reaccent(
            encoded, encoded_length,
            b.output, b.used,
            &accented, &accented_length, cancelled, cancel_context);
    if (state) {
        free(native_facts);
        result = state == 1 ? 2 : -2;
        goto done;
    }
    if(contextual==1 && contextual_features(&b,accented,accented_length,native_facts)) {
        free(native_facts);free(accented);result=-2;goto done;
    }
    free(native_facts);
    state = (contextual==1?remap_contextual_boundaries:remap_accented_boundaries)(
        b.output, b.used, accented, accented_length,
        boundaries, boundary_count, cancelled, cancel_context);
    if (state) {
        free(accented);
        result = state == 2 ? 2 : state == -2 ? -2 : -1;
        goto done;
    }
    free(b.output);
    b.output = accented;
    b.used = accented_length;
    /* Analysis commands have already acted on grammar. Retire them without
       moving source/index offsets before the acoustic annotation is parsed. */
    for(size_t i=0;i<b.used;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(b.output+i,b.used-i,1);
        if(t.kind==RELOQ_TAG_DICTIONARY || t.kind==RELOQ_TAG_PREDICTION)
            memset(b.output+i,' ',t.length);
        i+=t.length?t.length:1;
    }
    *annotation = b.output;
    *annotation_length = b.used;
    *approximate_words = b.approximate;
    if (features) {*features=b.features;*feature_count=b.feature_count;}
done:
    reloq61_text_options_scope(initial_options);
    reloq61_dictionary_text_scope(previous_text_scope);
    if(main_map&&encoded_error_offset&&result&&normalized_error!=SIZE_MAX) {
        size_t original=0;
        while(original<main_source_length&&main_map[original]<normalized_error)++original;
        *source_error_offset=original;
    }
    if(boundaries!=source_boundaries) {
        if(!result&&!retry)for(size_t i=0;i<boundary_count;++i)
            source_boundaries[i].annotation_offset=boundaries[i].annotation_offset;
        free(boundaries);
        boundaries=source_boundaries;
    }
    free(main_map);
    encoded_error_offset=source_error_offset;
    free(word_storage);
    free(original_scalars);free(original_encoded);
    free(encoded);
    free(b.units);
    if (result||retry) {free(b.output);free(b.features);}
    reloq_lexicon_resolver_scope(previous_resolver);
    reloq61_dictionary_resolver_release(resolver);
    if(retry)return plan_tags_checked(0,utf8,utf8_length,lexicon,process_tags,
        annotation,annotation_length,approximate_words,encoded_error_offset,
        boundaries,boundary_count,features,feature_count,cancelled,cancel_context);
    return result;
}

int reloq_german_best_effort_plan_tags_checked(
    const unsigned char *utf8,size_t utf8_length,const struct reloq_lexicon *lexicon,
    int process_tags,unsigned char **annotation,size_t *annotation_length,
    size_t *approximate_words,size_t *encoded_error_offset,
    struct reloq_german_best_effort_boundary *boundaries,size_t boundary_count,
    struct reloq_german_word_features **features,size_t *feature_count,
    int (*cancelled)(void *),void *cancel_context)
{
    return plan_tags_checked(1,utf8,utf8_length,lexicon,process_tags,
        annotation,annotation_length,approximate_words,encoded_error_offset,
        boundaries,boundary_count,features,feature_count,cancelled,cancel_context);
}

int reloq_german_best_effort_plan_indexed_checked(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_lexicon *lexicon,
    unsigned char **annotation, size_t *annotation_length,
    size_t *approximate_words, size_t *encoded_error_offset,
    struct reloq_german_best_effort_boundary *boundaries,
    size_t boundary_count,
    struct reloq_german_word_features **features,size_t *feature_count,
    int (*cancelled)(void *), void *cancel_context)
{
    return reloq_german_best_effort_plan_tags_checked(utf8,utf8_length,lexicon,0,
        annotation,annotation_length,approximate_words,encoded_error_offset,
        boundaries,boundary_count,features,feature_count,cancelled,cancel_context);
}

int reloq_german_best_effort_annotate_indexed_checked(
    const unsigned char *utf8,size_t length,const struct reloq_lexicon *lexicon,
    unsigned char **annotation,size_t *annotation_length,size_t *approximate,
    size_t *error,struct reloq_german_best_effort_boundary *boundaries,size_t count,
    int (*cancelled)(void *),void *context)
{
    return reloq_german_best_effort_plan_indexed_checked(utf8,length,lexicon,
        annotation,annotation_length,approximate,error,boundaries,count,
        NULL,NULL,cancelled,context);
}

int reloq_german_best_effort_annotate_checked(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_lexicon *lexicon,
    unsigned char **annotation, size_t *annotation_length,
    size_t *approximate_words, size_t *encoded_error_offset,
    int (*cancelled)(void *), void *cancel_context)
{
    return reloq_german_best_effort_annotate_indexed_checked(
        utf8, utf8_length, lexicon, annotation, annotation_length,
        approximate_words, encoded_error_offset, NULL, 0,
        cancelled, cancel_context);
}

int reloq_german_best_effort_annotate(
    const unsigned char *utf8, size_t utf8_length,
    const struct reloq_lexicon *lexicon,
    unsigned char **annotation, size_t *annotation_length,
    size_t *approximate_words, size_t *encoded_error_offset)
{
    return reloq_german_best_effort_annotate_checked(
        utf8, utf8_length, lexicon, annotation, annotation_length,
        approximate_words, encoded_error_offset, NULL, NULL);
}
