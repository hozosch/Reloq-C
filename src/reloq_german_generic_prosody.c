#include "reloq_german_generic_prosody.h"

#include "reloq_german_annotation.h"
#include "reloq_german_punctuation.h"
#include "reloq_german_phone_sequence.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int reloq_german_generic_prosody_weak_vowel_ticks(
    const struct reloq_german_phone_node *node, int32_t base_ticks,
    int simple_monosyllable, int32_t *ticks)
{
    if (!node || !ticks || base_ticks <= 0) return -1;
    if (!node->weak_word_control || !simple_monosyllable) return 0;
    int percent;
    switch (node->phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_Y:
        percent = 30;
        break;
    case RELOQ_DE_PHONE_a: case RELOQ_DE_PHONE_e:
    case RELOQ_DE_PHONE_i: case RELOQ_DE_PHONE_o:
    case RELOQ_DE_PHONE_u: case RELOQ_DE_PHONE_y:
    case RELOQ_DE_PHONE_AT:
        percent = 37;
        break;
    default:
        return 0;
    }
    int32_t result = (base_ticks * percent + 50) / 100;
    if (node->same_syllable_right && node->same_word_right) {
        const enum reloq_german_phone following = node->right_phone;
        if (following == RELOQ_DE_PHONE_r ||
            following == RELOQ_DE_PHONE_R) {
            /* Mid/low vowel nuclei are largely absorbed by /r/ in weak
             * CVR, while high nuclei retain more of their own interval. */
            const int high = node->phone == RELOQ_DE_PHONE_i ||
                node->phone == RELOQ_DE_PHONE_u ||
                node->phone == RELOQ_DE_PHONE_y;
            result = (result * (high ? 85 : percent == 30 ? 80 : 55)
                      + 50) / 100;
        } else if (following == RELOQ_DE_PHONE_n ||
                   following == RELOQ_DE_PHONE_m)
            result = (result * 90 + 50) / 100;
    }
    *ticks = result < 16 ? 16 : result;
    return 1;
}

#include "reloq_german_phrase_metadata.inc"

struct written_word {
    const unsigned char *text;
    size_t length;
    struct word_role role;
    struct word_role lexical;
    struct word_role medial;
    unsigned flags;
    int accent;
    int observed;
    int force_weak;
    int force_deaccent;
    int da_accent;
    int preserve;
    int break_before; /* 1: audible clause break; 2: accent group only. */
    int question;
    int inner_accent;
    int initial_accent;
};
struct group_stress {
    size_t word_offset, stress_offset;
    unsigned syllables;
};
static int letter(unsigned char c)
{
    return (c>='a' && c<='z') || (c>='A' && c<='Z') ||
        c==0xc4 || c==0xd6 || c==0xdc || c==0xe4 || c==0xf6 || c==0xfc || c==0xdf;
}
static unsigned char lower(unsigned char c)
{
    return (c>='A' && c<='Z') || c==0xc4 || c==0xd6 || c==0xdc?(unsigned char)(c+32):c;
}
static int is_word(const struct written_word *w,const char *s)
{
    if (strlen(s)!=w->length) return 0;
    for (size_t i=0;i<w->length;++i) if (lower(w->text[i])!=(unsigned char)s[i]) return 0;
    return 1;
}
static int ends(const struct written_word *w,const char *tail)
{
    size_t n=strlen(tail);
    if (w->length<n+2) return 0;
    for(size_t i=0;i<n;++i)
        if(lower(w->text[w->length-n+i])!=(unsigned char)tail[i])return 0;
    return 1;
}
/* mark_ger_content_word tests productive endings independently of the
 * finite lexical inventory. First-position unknowns remain undefined;
 * subsequent lowercase forms carry the observed grammatical class. */
static struct word_role ending_role(const struct written_word *w)
{
    if(ends(w,"end"))return (struct word_role){POS_ADJADV,SUB_PRES_PART,1};
    if(ends(w,"ig")||ends(w,"lich")||ends(w,"isch")||ends(w,"bar")||ends(w,"sam"))
        return (struct word_role){POS_ADJADV,SUB_NON,1};
    if((ends(w,"e") || ends(w,"en")) && w->length>5 &&
       lower(w->text[0])=='g' && lower(w->text[1])=='e')
        return (struct word_role){POS_ADJ,SUB_NON,1};
    if(ends(w,"ernen"))return (struct word_role){POS_ADJ,SUB_NON,1};
    if(ends(w,"en")||ends(w,"ern")||ends(w,"eln")||ends(w,"t")||ends(w,"te")||ends(w,"e"))
        return (struct word_role){POS_VERB,SUB_NON,1};
    return observed_roles[0];
}
static void word_role(struct written_word *w,int first,int last)
{
    char key[128];w->role=w->lexical=w->medial=observed_roles[0];w->flags=0;w->observed=0;
    if (w->length<sizeof key) {
        for (size_t i=0;i<w->length;++i) key[i]=(char)lower(w->text[i]);
        key[w->length]=0;
        size_t lo=0,hi=sizeof observed_words/sizeof observed_words[0];
        while (lo<hi) {
            size_t mid=lo+(hi-lo)/2;int order=strcmp(key,observed_words[mid].text);
            if (order<0) hi=mid;
            else if (order>0) lo=mid+1;
            else {
                const struct word_metadata *m=observed_words+mid;
                w->lexical=observed_roles[m->first];
                w->medial=observed_roles[m->middle];
                w->role=observed_roles[first?m->first:last?m->last:m->middle];
                w->flags=m->flags;w->observed=1;break;
            }
        }
    }
    if(!first && !w->observed && w->role.pos==POS_UNDEF) w->role=ending_role(w);
    /* Noun capitalization precedes the content-word disambiguation pass. */
    if (!first && lower(w->text[0])!=w->text[0] &&
        (w->role.pos==POS_VERB || w->role.pos==POS_ADJ ||
         w->role.pos==POS_ADJADV || w->role.pos==POS_UNDEF)) {
        w->role=(struct word_role){POS_NOUN,SUB_NON,1};w->flags=0;
    }
    w->accent=w->role.accent;
}
int reloq_german_observed_verb(const unsigned char *word,size_t length)
{
    if(!word || !length)return 0;
    struct written_word w={.text=word,.length=length};
    word_role(&w,0,0);
    return w.observed && w.medial.pos==POS_VERB;
}
static int adverb(const struct written_word *w)
{return w->role.pos==POS_ADV || w->role.pos==POS_ADJADV;}

/* Attributive inflection is resolved before clause boundaries. Noun
 * phrases may have a determiner, a preposition or coordinated adjectives;
 * a guessed verbal ending must not split an adjective series. */
static int inflected_modifier(const struct written_word *w)
{
    if(w->role.pos==POS_ADJ)return 1;
    if(w->observed || lower(w->text[0])!=w->text[0])return 0;
    return ends(w,"e")||ends(w,"en")||ends(w,"em")||ends(w,"er")||ends(w,"es");
}
static void nominal_groups(struct written_word *w,size_t n)
{
    for(size_t end=1;end<n;++end) {
        if(w[end].role.pos!=POS_NOUN)continue;
        size_t first=end;
        while(first && inflected_modifier(w+first-1)) {
            --first;
            if(first>1 && w[first-1].role.pos==POS_CONJ &&
               w[first-1].role.subtype==SUB_COORD && inflected_modifier(w+first-2))--first;
        }
        for(size_t j=first;j<end;++j)if(w[j].role.pos!=POS_CONJ) {
            w[j].role=(struct word_role){POS_ADJ,SUB_NON,1};
            w[j].accent=1;w[j].flags=0;
        }
    }
}

/* Lexical stress, weak function-word status and phrase accent are separate.
 * The observed class predicates operate on a complete punctuation-delimited
 * phrase. In particular, a trailing weak pronoun cannot steal its nucleus. */
static void phrase_accents(struct written_word *w,size_t n)
{
    for (size_t i=0;i<n;++i) {
        word_role(w+i,!i,i+1==n);
    }
    for(size_t i=1;i<n;++i)
        if(w[i].role.pos==POS_VERB && w[i-1].role.pos==POS_PREP &&
           w[i-1].role.subtype==SUB_ZU)w[i].role.subtype=SUB_ZU_INF;
    /* unfunct_kein_words promotes the negative-determiner class at a
     * phrase edge or immediately after a preposition. The ordinary
     * medial nominal object retains its weak reading. */
    for(size_t i=0;i<n;++i)
        if(w[i].role.pos==POS_DET && w[i].role.subtype==SUB_NEG &&
           (!i || i+1==n || w[i-1].role.pos==POS_PREP))w[i].accent=1;
    /* destress_adverbs distinguishes indefinite pronouns from a following
     * ordinary content word. Their accent does not suppress the preceding
     * adverb. Resolve roles first so the predicate sees the real neighbour.
     * destress_mehr removes the comparative accent after negation before
     * the auxiliary and nuclear-accent passes choose their complement. */
    for(size_t i=1;i<n;++i) {
        if(w[i].role.subtype==SUB_DEACC &&
           !(i+1<n && (w[i+1].role.pos==POS_PRO2 || w[i+1].role.pos==POS_ADJ ||
             (w[i+1].role.pos==POS_DET && w[i+1].role.subtype==SUB_DEF &&
              i+2<n && w[i+2].role.pos==POS_VERB))))w[i].accent=0;
        if(w[i].role.subtype==SUB_MEHR && (is_word(w+i-1,"nicht") ||
           (w[i-1].role.pos==POS_NOUN && i+1<n &&
            (w[i+1].role.pos==POS_VERB || w[i+1].role.pos==POS_AUX))))
            w[i].accent=0;
        /* unfunct_auch: an ensuing predicate (optionally negated) gives
         * the particle its lexical accent. A following nominal object
         * keeps the ordinary weak reading. */
        if(is_word(w+i,"auch") && i+1<n) {
            size_t j=i+1;
            if(is_word(w+j,"nicht") && j+1<n)++j;
            if(w[j].role.pos==POS_VERB || w[j].role.pos==POS_AUX)
                w[i].accent=1;
        }
    }
    nominal_groups(w,n);
    /* A stranded coordinating word closes a fragment with full lexical
     * stress. It does not displace the preceding phrase nucleus. */
    while(n>1&&w[n-1].role.pos==POS_CONJ&&w[n-1].role.subtype==SUB_COORD) {
        w[n-1].accent=1;--n;
    }
    /* A one-word phrase is never reduced: the word keeps its full stress. */
    if (n<2) {if (n==1) w[0].accent=1;return;}
    /* A closing modal belongs to the preceding infinitive's accent domain.
     * The SO leaves the modal weak and applies final-verb/object rules to
     * the lexical predicate, including in coordinated indirect questions. */
    int trailing_modal=0;
    if(n>2 && w[n-1].role.pos==POS_AUX && w[n-1].role.subtype==SUB_MODAL &&
       w[n-2].role.pos==POS_VERB) {
        w[n-1].accent=-1;trailing_modal=1;--n;
    }
    size_t last=n-1;
    if(last && w[last].role.subtype==SUB_NEG && is_word(w+last-1,"aber"))
        w[last].accent=1;
    for (size_t i=0;i<last;++i)
        if (w[i].lexical.pos==POS_DET && w[i].lexical.subtype==SUB_POS &&
            (w[i+1].role.pos==POS_NOUN || w[i+1].role.pos==POS_ADJ ||
             w[i+1].role.pos==POS_ADJADV)) {
            w[i].role=w[i].lexical;w[i].accent=w[i].role.accent;
        }
    if (n>2 && w[0].role.subtype==SUB_DEACC && w[1].role.pos!=POS_PRO &&
        w[1].role.pos!=POS_VERB &&
        !(w[1].role.pos==POS_AUX && w[1].role.subtype==SUB_BE)) w[0].accent=0;
    if (w[last].role.pos==POS_PREP ||
        (w[last].role.pos==POS_DET &&
         (w[last].role.subtype==SUB_POS || w[last].role.subtype==SUB_ALLE ||
          w[last].role.subtype==SUB_INDEF)) ||
        is_word(w+last,"auch") || is_word(w+last,"so")) w[last].accent=1;
    if (w[0].role.pos==POS_WH && is_word(w+last,"das")) w[last].accent=1;
    for (size_t i=1;i<n;++i) {
        if ((is_word(w+i-1,"auch") || is_word(w+i-1,"sogar")) && w[i].role.pos==POS_PRO) {
            w[i].accent=1;
            if (is_word(w+i-1,"sogar")) w[i-1].accent=1;
        }
        if (i==last && w[i-1].role.pos==POS_AUX && w[i-1].role.subtype==SUB_BE &&
            (w[i].role.pos==POS_DET || w[i].role.pos==POS_DEM ||
             (w[i].role.pos==POS_WH && w[i].role.subtype==SUB_WELCH))) w[i].accent=1;
    }
    if (is_word(w+last,"da"))
        for (size_t i=0;i<last;++i)
            if (w[i].role.pos==POS_AUX || w[i].role.pos==POS_VERB) {w[last].accent=1;break;}
    /* handle_final_prep_phr / handle_prep_pro_verb distinguish the
     * verb's lexical paradigm and the preposition's accent class. The
     * complement must close the phrase, optionally before a licensed
     * final verb. A following content word keeps the pronoun weak. */
    size_t object=last,governor=n;
    if(w[last].role.pos==POS_VERB && (w[last].flags&8) && last>1) {
        object=last-1;governor=last;
    }
    if(object && (w[object].role.pos==POS_PRO || w[object].role.pos==POS_DET_PRO) &&
       w[object-1].role.pos==POS_PREP && (w[object-1].flags&48)) {
        if(governor==n)for(size_t j=object-1;j;) {
            --j;
            if(w[j].role.pos==POS_CONJ)break;
            if(w[j].role.pos==POS_VERB || w[j].role.pos==POS_AUX) {
                if(w[j].flags&8)governor=j;
                break;
            }
        }
        if(governor<n) {
            size_t accent_word=(w[object-1].flags&16)?object-1:object;
            w[accent_word].accent=1;
            if(w[governor].role.pos==POS_VERB)w[governor].accent=0;
        }
    }
    /* A final BE auxiliary can be the lexical predicate of an earlier
     * auxiliary/causative when no intervening complement bears an accent. */
    if(w[last].role.pos==POS_AUX && w[last].role.subtype==SUB_BE) {
        for(size_t j=last;j;) {
            --j;
            if(w[j].role.pos==POS_AUX || (w[j].role.pos==POS_VERB && (w[j].flags&1))) {
                w[last].accent=1;
                if(w[j].role.pos==POS_VERB)w[j].accent=0;
                break;
            }
            if(w[j].accent>0 || w[j].role.pos==POS_CONJ)break;
        }
    }
    int complement=w[last].accent>0;
    /* BE and HAV are different lexical classes in the original pass.
     * A copular phrase promotes its remaining complement (or subject);
     * it does not make the copula the default nucleus. HAV only supplies
     * the accent when its weak complement cannot carry it. */
    int copular=0;
    for (size_t i=0;i<last;++i)
        if (w[i].role.pos==POS_AUX && w[i].role.subtype==SUB_BE) copular=1;
    if (copular && adverb(w+last) && w[last].accent<0) {
        w[last].accent=1;complement=1;
    }
    /* Scan from the phrase end, including a final auxiliary. Once an
     * auxiliary supplies the accent, an earlier modal cannot steal it. */
    for (size_t i=n;i;) {
        --i;
        if (w[i].role.pos==POS_CONJ && w[i].role.subtype==SUB_COORD) {
            complement=0;continue;
        }
        int noun_subject=0;
        int reduced_comparative=0;
        if(w[i].role.subtype==SUB_HAV)
            for(size_t j=i+1;j<n;++j)
                reduced_comparative|=w[j].role.subtype==SUB_MEHR && w[j].accent==0;
        if (w[i].role.subtype==SUB_HAV && i+1==last && w[last].role.pos==POS_PRO)
            for (size_t j=0;j<i;++j)
                noun_subject|=w[j].role.pos==POS_NOUN && w[j].accent>0;
        if (w[i].role.pos==POS_AUX && w[i].role.subtype!=SUB_BE && !complement &&
            !noun_subject && !reduced_comparative && !(w[i].role.subtype==SUB_HAV &&
                (w[last].role.pos==POS_DET ||
                 (w[last].role.pos==POS_PRO && w[last-1].role.pos==POS_PREP))))
            w[i].accent=1;
        complement|=w[i].accent>0;
    }
    /* The original tests membership of a verb paradigm before allowing
     * deaccenting. Merely being a verb does not suffice (e.g. kennen). */
    complement=0;
    size_t next_predicate=n;
    for (size_t i=n;i;) {
        --i;
        if (w[i].role.pos==POS_CONJ && w[i].role.subtype==SUB_COORD) {
            complement=0;next_predicate=n;continue;
        }
        /* The original second verb pass also sees a later predicate
         * across an intervening constituent (including weak pronouns or
         * negation). Adjacent verbs form a different domain. Preserve
         * the lexical eligibility test rather than weakening every verb. */
        if (w[i].role.pos==POS_VERB && (w[i].flags&1) &&
            (complement || (next_predicate<n && next_predicate>i+1)))w[i].accent=0;
        if(w[i].role.pos==POS_VERB)next_predicate=i;
        else if(w[i].role.pos==POS_AUX)next_predicate=n;
        /* Nominal/adverbial prominence remains local to its predicate.
         * The separately tracked next predicate supplies the non-adjacent
         * clause condition above; negation alone is not a complement. */
        if (w[i].role.pos==POS_VERB || w[i].role.pos==POS_AUX) complement=0;
        else if(w[i].role.subtype!=SUB_NEG) complement|=w[i].accent>0;
    }
    size_t nucleus=n;
    for (size_t i=n;i;--i) if (w[i-1].accent>0) {nucleus=i-1;break;}
    if (nucleus==n) {
        if (copular && w[last].role.pos==POS_PRO)
            for (size_t i=0;i<last;++i)
                if (w[i].role.pos==POS_PRO && (w[i].role.subtype==SUB_SUBJ ||
                    w[i].role.subtype==SUB_SUBJ_OBJ)) {
                    nucleus=i;break;
                }
        if (nucleus==n) {
            /* If every available accent was suppressed, a lexical content
             * word can still carry the nucleus before trailing weak words. */
            for(size_t i=n;i;--i)if(w[i-1].role.accent>0) {nucleus=i-1;break;}
            if(nucleus==n)nucleus=last;
        }
    }
    /* A stressed particle/root domain retains the final nuclear accent.
     * The reference's compound/prefix-verb checks precede the nominal
     * complement rule; a second noun alone cannot deaccent this domain. */
    if (nucleus==last && w[last].role.pos==POS_VERB && !(w[last].flags&64)) {
        size_t candidate=n,last_aux=n;
        for (size_t i=last;i;--i) {
            if (w[i-1].role.pos==POS_AUX) {last_aux=i-1;break;}
            if (w[i-1].role.pos==POS_CONJ || w[i-1].role.pos==POS_VERB) break;
            /* An intervening modifier remains a syntactic constituent
             * after deaccenting. Skipping it used to expose an earlier
             * noun and move the nuclear accent across that modifier. */
            if (candidate==n && adverb(w+i-1) && w[i-1].accent<=0) break;
            if (candidate==n && w[i-1].accent>0) candidate=i-1;
        }
        int nouns=0,predicates=0,subject=0;
        for (size_t i=0;i<last;++i) {
            if (w[i].accent>0 && (w[i].role.pos==POS_NOUN ||
                (!i && w[i].role.pos==POS_UNDEF &&
                 (ending_role(w+i).pos!=POS_VERB || (n>1 &&
                     (w[1].role.pos==POS_PREP || w[1].role.pos==POS_NOUN))) &&
                 lower(w[i].text[0])!=w[i].text[0]))) ++nouns;
            if (w[i].role.pos==POS_VERB || w[i].role.pos==POS_AUX) ++predicates;
            if(w[i].role.pos==POS_PRO && (w[i].role.subtype==SUB_SUBJ ||
               w[i].role.subtype==SUB_SUBJ_OBJ))subject=1;
        }
        /* Adverb-plus-verb deaccenting has its own lexical class.
         * A verb weak before a later complement is not necessarily weak
         * after a modifier. The modifier may follow a subject or object. */
        const int modifier=candidate<n && adverb(w+candidate);
        int predicative_modifier=0;
        if(modifier) {
            static const char *const members[]={
#include "reloq_german_predicative_modifiers75.inc"
            };
            for(size_t k=0;k<sizeof members/sizeof *members;++k)
                if(is_word(w+candidate,members[k]))predicative_modifier=1;
        }
        /* A nominal object after a determiner remains eligible before
         * a particle verb. Secondary lexical stress alone is not proof
         * that the verb must carry the phrase accent. A Bitte request
         * supplies the same object/infinitive domain without an auxiliary. */
        size_t determiner=candidate;
        while(determiner>0 && w[determiner-1].role.pos==POS_ADJ)--determiner;
        int object=candidate<n && w[candidate].role.pos==POS_NOUN &&
            ((determiner>0 && w[determiner-1].role.pos==POS_DET) ||
             (last_aux<n && w[last_aux].role.subtype==SUB_MODAL));
        int request=is_word(w,"bitte") && candidate<n &&
            w[candidate].role.pos==POS_NOUN;
        if (candidate<n && (!w[last].inner_accent || object || request) &&
            (request || (object && trailing_modal) ||
            (object && w[last].role.subtype==SUB_ZU_INF && w[last].initial_accent) ||
            ((w[last].flags&2) && !(candidate==0 && is_word(w,"bitte")) && !(candidate+1==last &&
                w[candidate].role.pos==POS_UNDEF &&
                w[candidate].medial.pos==POS_ADJADV)) ||
            (last_aux<n && ((w[last_aux].role.subtype==SUB_HAV &&
                            w[candidate].role.pos==POS_NOUN) ||
                           (w[last_aux].role.subtype==SUB_BE &&
                            w[candidate].role.pos==POS_NOUN) ||
                           (w[last_aux].role.subtype==SUB_MODAL &&
                            w[candidate].role.pos==POS_NOUN))) ||
            (!predicates && nouns+subject>1 && w[candidate].role.pos==POS_NOUN) ||
            (modifier && ((w[last].flags&4) || predicative_modifier ||
                          w[last].role.subtype==SUB_BE)))) nucleus=candidate;
        else if (!w[last].inner_accent && is_word(w,"auch") && n==2 && (w[last].flags&1)) nucleus=0;
    }
    if (nucleus==last && (w[last].role.pos==POS_PREP || w[last].role.pos==POS_DAWORD ||
        (w[last].role.pos==POS_DET && w[last].role.subtype==SUB_INDEF))) {
        /* get_ger_nuclear_accent selects the preceding nominal
         * constituent before a final particle. Its predicate need not be
         * inside this comma-delimited phrase. Initial undefined words do
         * not acquire a noun role merely from their capitalization. */
        size_t candidate=n;
        for (size_t i=0;i<last;++i) {
            if (w[i].role.pos==POS_VERB || w[i].role.pos==POS_AUX) candidate=n;
            else if (w[i].accent>0 && w[i].role.pos==POS_NOUN) candidate=i;
        }
        if (candidate<n) nucleus=candidate;
    }
    w[nucleus].accent=2;
    for (size_t i=nucleus+1;i<n;++i) if (w[i].accent>0) w[i].accent=0;

}

static void clause_boundaries(struct written_word *w,size_t n)
{
    for(size_t i=0;i<n;++i)word_role(w+i,!i,i+1==n);
    nominal_groups(w,n);
    int predicate=0;
    for(size_t i=0;i<n;++i) {
        if(w[i].role.pos==POS_CONJ && w[i].role.subtype==SUB_COORD) {
            int right=0;
            for(size_t j=i+1;j<n && w[j].role.pos!=POS_CONJ;++j)
                right|=w[j].role.pos==POS_VERB || w[j].role.pos==POS_AUX;
            /* Subject-bearing continuation, distinct from coordinated
             * verbs sharing one subject and from nominal lists. */
            if(i && i+1<n && predicate && (right || (w[i+1].role.pos==POS_DET &&
                (w[i-1].role.pos==POS_VERB || w[i-1].role.pos==POS_AUX))) &&
               (w[i+1].role.pos==POS_PRO || w[i+1].role.pos==POS_DET ||
                (w[i+1].role.pos==POS_VERB && i+2<n)))
                w[i].break_before=1;
            if(i && i+1<n && w[i].role.accent==0)w[i].break_before=2;
            if(i && i+1<n && predicate && right &&
               (w[i+1].role.pos==POS_WH ||
                (w[i+1].role.pos==POS_CONJ && w[i+1].role.subtype==SUB_SUBORD)))
                w[i].break_before=2;
            predicate=0;
        } else predicate|=w[i].role.pos==POS_VERB || w[i].role.pos==POS_AUX ||
            (!i && w[i].role.pos==POS_UNDEF && ending_role(w+i).pos==POS_VERB);
    }
}

static int copy_annotation(const unsigned char *annotation,size_t length,
    unsigned char **out,size_t *out_length)
{
    if (length==SIZE_MAX) return -1;
    unsigned char *copy=malloc(length+1);if (!copy) return -1;
    memcpy(copy,annotation,length);copy[length]=0;*out=copy;*out_length=length;return 0;
}

/* Shared back end: `words` holds exactly one entry per SPR group. */
static int reaccent_core(struct written_word *words,size_t count,
    const unsigned char *annotation,size_t annotation_length,
    unsigned char **out,size_t *out_length,
    int (*cancelled)(void *),void *cancel_context)
{
    struct group_stress *groups=calloc(count,sizeof *groups);
    enum reloq_german_terminal_mark *marks=malloc(count*sizeof *marks);
    if (!groups || !marks) {free(groups);free(marks);return -1;}
    struct reloq_german_annotation_cursor cursor={0};
    struct reloq_german_annotation_event event;
    size_t seen=0;int parsed,explicit_control=0,result=0;
    while ((parsed=reloq_german_annotation_next(annotation,annotation_length,&cursor,0,&event))==1) {
        if (cancelled && cancelled(cancel_context)) {result=1;goto done;}
        if (event.kind==RELOQ_DE_ANNOTATION_CONTROL && event.length>=2 &&
            annotation[event.offset+1]>='0' && annotation[event.offset+1]<='4') explicit_control=1;
        if (event.kind!=RELOQ_DE_ANNOTATION_SPR) continue;
        if (event.spr.kind==RELOQ_DE_SPR_WORD_OPEN) {
            if (seen==count) break;
            groups[seen].word_offset=event.offset;groups[seen++].stress_offset=SIZE_MAX;
        } else if (seen) {
            if (event.spr.kind==RELOQ_DE_SPR_SYLLABLE) ++groups[seen-1].syllables;
            else if (event.spr.kind==RELOQ_DE_SPR_STRESS) {
                size_t primary=groups[seen-1].stress_offset;
                if(groups[seen-1].syllables==1 && event.spr.stress==1)words[seen-1].initial_accent=1;
                if(event.spr.stress==2 && primary!=SIZE_MAX && annotation[primary]=='1')
                    words[seen-1].inner_accent=1;
                if(event.spr.stress==1 || primary==SIZE_MAX)
                    groups[seen-1].stress_offset=event.offset;
            }
        }
    }
    /* Expanded numbers/abbreviations do not have a one-to-one word mapping.
     * Existing numeric controls also remain caller-owned and idempotent. */
    if (parsed || seen!=count || explicit_control ||
        reloq_german_generic_prosody_marks(annotation,annotation_length,count,marks,count)) {
        result=copy_annotation(annotation,annotation_length,out,out_length);goto done;
    }
    for (size_t first=0;first<count;) {
        if (cancelled && cancelled(cancel_context)) {result=1;goto done;}
        size_t end=first+1;
        while (end<count && marks[end-1]==RELOQ_DE_TERMINAL_IMPLICIT) ++end;
        clause_boundaries(words+first,end-first);
        for(size_t a=first;a<end;) {
            size_t b=a+1;
            while(b<end && !words[b].break_before)++b;
            phrase_accents(words+a,b-a);
            /* assign_stress_in_daword runs in the sentence context. Its
             * focus particles protect the deictic accent; elsewhere a
             * noninitial pronominal adverb stresses its preposition. */
            for(size_t k=a;k<b;++k)if(words[k].role.pos==POS_DAWORD &&
                words[k].length>2 && lower(words[k].text[0])=='d' &&
                lower(words[k].text[1])=='a') {
                if(k==a) {
                    if(is_word(words+k,"damit") && b-a>1 &&
                       (words[k+1].role.pos==POS_VERB || words[k+1].role.pos==POS_AUX))words[k].da_accent=1;
                    continue;
                }
                words[k].da_accent=2;
                static const char *const focus[]={
#include "reloq_german_da_focus75.inc"
                };
                for(size_t j=0;j<sizeof focus/sizeof *focus;++j)
                    if(is_word(words+k-1,focus[j]))words[k].da_accent=1;
                if(k>=a+2 && ((is_word(words+k-2,"erst")&&is_word(words+k-1,"noch")) ||
                    (is_word(words+k-2,"dar\374ber")&&is_word(words+k-1,"hinaus")) ||
                    (is_word(words+k-2,"vor")&&is_word(words+k-1,"allem"))))words[k].da_accent=1;
                if(k>=a+3 && ((is_word(words+k-3,"in")&&is_word(words+k-2,"erster")&&is_word(words+k-1,"linie")) ||
                    (is_word(words+k-3,"in")&&is_word(words+k-2,"der")&&is_word(words+k-1,"hauptsache")) ||
                    (is_word(words+k-3,"vor")&&is_word(words+k-2,"allen")&&is_word(words+k-1,"dingen"))))words[k].da_accent=1;
            }
            /* adjust_accents lowers the word directly before a low
             * nuclear tone. Intervening weak words stop this adjacency;
             * grammatical category and guessed verb endings do not decide
             * whether earlier accents survive. */
            if(reloq_german_terminal_contour_mark(marks[end-1])==RELOQ_DE_TERMINAL_QUESTION)
                for(size_t k=a+1;k<b;++k)
                    if(words[k].accent==2 && words[k-1].accent==1)
                        words[k-1].accent=0;
            a=b;
        }
        for (size_t k=first;k<end;++k) {
            if (words[k].force_weak) words[k].accent=-1;
            else if (words[k].force_deaccent) words[k].accent=0;
            words[k].question=reloq_german_terminal_contour_mark(marks[end-1])==RELOQ_DE_TERMINAL_QUESTION;
        }
        first=end;
    }
    if (count>(SIZE_MAX-annotation_length-1)/5) {result=-1;goto done;}
    unsigned char *copy=malloc(annotation_length+5*count+1);
    if (!copy) {result=-1;goto done;}
    size_t source_at=0,target_at=0;
    for (size_t i=0;i<count;++i) {
        if (cancelled && (i&255)==0 && cancelled(cancel_context)) {free(copy);result=1;goto done;}
        size_t at=groups[i].word_offset;
        memcpy(copy+target_at,annotation+source_at,at-source_at);target_at+=at-source_at;
        if(words[i].break_before==1)copy[target_at++]=',';
        const char *control=words[i].preserve?"":words[i].accent<0?"`00 ":words[i].accent==0?"`0 ":
            words[i].accent==2?"`2 ":words[i].question?"`1 ":"";
        size_t length=strlen(control);memcpy(copy+target_at,control,length);target_at+=length;
        size_t end=i+1<count?groups[i+1].word_offset:annotation_length;
        memcpy(copy+target_at,annotation+at,end-at);
        if(!words[i].preserve && words[i].da_accent && groups[i].syllables>1) {
            unsigned syllable=0;
            for(size_t k=2;k+1<end-at && copy[target_at+k]!=']';++k)
                if(copy[target_at+k]=='.' && copy[target_at+k+1]>='0' &&
                   copy[target_at+k+1]<='2') {
                    ++syllable;
                    copy[target_at+k+1]=syllable==(unsigned)words[i].da_accent?'1':'0';
                }
        }
        if (!words[i].preserve && groups[i].syllables==1 && groups[i].stress_offset!=SIZE_MAX)
            copy[target_at+groups[i].stress_offset-at]=words[i].accent<0?'0':'1';
        target_at+=end-at;source_at=end;
    }
    copy[target_at]=0;*out=copy;*out_length=target_at;
done:
    free(groups);free(marks);return result;
}

int reloq_german_generic_prosody_reaccent(
    const unsigned char *cp1252,size_t cp1252_length,
    const unsigned char *annotation,size_t annotation_length,
    unsigned char **out,size_t *out_length,
    int (*cancelled)(void *),void *cancel_context)
{
    if ((!cp1252 && cp1252_length) || !annotation || !out || !out_length || annotation_length==SIZE_MAX ||
        cp1252_length>=SIZE_MAX/sizeof(struct written_word)-1) return -1;
    if (cancelled && cancelled(cancel_context)) return 1;
    struct written_word *words=calloc(cp1252_length+1,sizeof *words);
    if (!words) return -1;
    size_t count=0;
    for (size_t i=0;i<cp1252_length;) {
        if (cancelled && (i&255)==0 && cancelled(cancel_context)) {free(words);return 1;}
        if (!letter(cp1252[i])) {++i;continue;}
        size_t start=i;while (i<cp1252_length && letter(cp1252[i])) ++i;
        words[count].text=cp1252+start;words[count++].length=i-start;
    }
    if (count<2) {free(words);return copy_annotation(annotation,annotation_length,out,out_length);}
    {
        int r=reaccent_core(words,count,annotation,annotation_length,out,out_length,cancelled,cancel_context);
        free(words);return r;
    }
}

int reloq_german_generic_prosody_reaccent_units(
    const struct reloq_german_prosody_unit *units,size_t count,
    const unsigned char *annotation,size_t annotation_length,
    unsigned char **out,size_t *out_length,
    int (*cancelled)(void *),void *cancel_context)
{
    if ((!units && count) || !annotation || !out || !out_length ||
        annotation_length==SIZE_MAX || count>=SIZE_MAX/sizeof(struct written_word)-1) return -1;
    if (cancelled && cancelled(cancel_context)) return 1;
    if (count<2)
        return copy_annotation(annotation,annotation_length,out,out_length);
    struct written_word *words=calloc(count+1,sizeof *words);
    if (!words) return -1;
    for (size_t i=0;i<count;++i) {
        words[i].text=units[i].text;words[i].length=units[i].length;
        words[i].force_weak=units[i].force_weak;
        words[i].force_deaccent=units[i].force_deaccent;
        words[i].preserve=units[i].preserve;
    }
    int r=reaccent_core(words,count,annotation,annotation_length,out,out_length,cancelled,cancel_context);
    free(words);return r;
}
static enum reloq_german_terminal_mark mark_from_text(
    const unsigned char *text, size_t length,
    enum reloq_german_terminal_mark old,int following_word)
{
    enum reloq_german_terminal_mark result = old;
    for (size_t i = 0; i < length; ++i) {
        switch (text[i]) {
        case '.':
            if (i + 2 < length && text[i + 1] == '.' && text[i + 2] == '.') {
                result = RELOQ_DE_TERMINAL_ELLIPSIS;
                i += 2;
            } else result = RELOQ_DE_TERMINAL_PERIOD;
            break;
        case RELOQ_DE_TEXT_QUESTION_CONTINUE: result=RELOQ_DE_TERMINAL_QUESTION_CONTINUE; break;
        case RELOQ_DE_TEXT_EXCLAMATION_CONTINUE: result=RELOQ_DE_TERMINAL_EXCLAMATION_CONTINUE; break;
        case 0x85: result=RELOQ_DE_TERMINAL_ELLIPSIS; break;
        case '?': result = RELOQ_DE_TERMINAL_QUESTION; break;
        case '!': result = RELOQ_DE_TERMINAL_EXCLAMATION; break;
        case ':': result = RELOQ_DE_TERMINAL_COLON; break;
        case ';': result = RELOQ_DE_TERMINAL_SEMICOLON; break;
        case ',': result = RELOQ_DE_TERMINAL_COMMA; break;
        case '(': case '-': case 0x96: case 0x97: result=RELOQ_DE_TERMINAL_COMMA;break;
        case ')': {
            size_t following=i+1;
            while(following<length&&(text[following]==' '||text[following]=='\t'||text[following]==')'))++following;
            if(following_word||following<length)result=RELOQ_DE_TERMINAL_COMMA;
            break;
        }
        default: break;
        }
    }
    return result;
}

int reloq_german_generic_prosody_marks(
    const unsigned char *annotation, size_t length, size_t word_count,
    enum reloq_german_terminal_mark *marks, size_t capacity)
{
    if (!annotation || !word_count || !marks || capacity < word_count ||
        word_count > SIZE_MAX / sizeof *marks) return -1;
    enum reloq_german_terminal_mark *scratch = calloc(word_count,
                                                        sizeof *scratch);
    if (!scratch) return -1;
    struct reloq_german_annotation_cursor cursor = {0};
    struct reloq_german_annotation_event event;
    size_t opened = 0, closed = 0;
    int result = -1, state;
    while ((state = reloq_german_annotation_next(
                annotation, length, &cursor, 0, &event)) == 1) {
        if (event.kind == RELOQ_DE_ANNOTATION_SPR) {
            if (event.spr.kind == RELOQ_DE_SPR_WORD_OPEN) {
                if (opened != closed || opened >= word_count) goto done;
                ++opened;
            } else if (event.spr.kind == RELOQ_DE_SPR_WORD_CLOSE) {
                if (closed >= opened) goto done;
                ++closed;
            }
        } else if (closed) {
            if (event.kind == RELOQ_DE_ANNOTATION_TEXT)
                scratch[closed - 1] = mark_from_text(
                    annotation + event.offset, event.length,
                    scratch[closed - 1],closed<word_count);
            else if (event.kind == RELOQ_DE_ANNOTATION_CALLBACK_NUL &&
                     scratch[closed - 1] == RELOQ_DE_TERMINAL_IMPLICIT)
                scratch[closed - 1] = RELOQ_DE_TERMINAL_PERIOD;
        }
    }
    if (!state && opened == word_count && closed == word_count) {
        memcpy(marks, scratch, word_count * sizeof *marks);
        result = 0;
    }
done:
    free(scratch);
    return result;
}

int reloq_german_generic_prosody_boundary_ticks(
    enum reloq_german_terminal_mark mark, int32_t *ticks)
{
    if (!ticks) return -1;
    int32_t value;
    switch (mark) {
    case RELOQ_DE_TERMINAL_CONTROL: value=80;break;
    case RELOQ_DE_TERMINAL_IMPLICIT: value = 0; break;
    case RELOQ_DE_TERMINAL_COMMA:
    case RELOQ_DE_TERMINAL_QUESTION_CONTINUE:
    case RELOQ_DE_TERMINAL_EXCLAMATION_CONTINUE:
    case RELOQ_DE_TERMINAL_ELLIPSIS: value = 210; break;
    case RELOQ_DE_TERMINAL_SEMICOLON: value = 310; break;
    case RELOQ_DE_TERMINAL_PERIOD:
    case RELOQ_DE_TERMINAL_QUESTION:
    case RELOQ_DE_TERMINAL_EXCLAMATION:
    case RELOQ_DE_TERMINAL_COLON: value = 460; break;
    default: return -1;
    }
    *ticks = value;
    return 0;
}
