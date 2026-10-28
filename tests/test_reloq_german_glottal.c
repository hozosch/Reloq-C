#include "reloq_german_glottal.h"
#include "reloq_german_best_effort_text.h"
#include "reloq_german_lexicon.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct captured {int64_t tick[32];int32_t value[32];size_t count;};
static int capture(void *context,enum reloq_target_stream stream,
    int64_t tick,int32_t value,size_t phone)
{
    struct captured *c=context;(void)phone;
    if (stream!=RELOQ_TARGET_DI) return 0;
    assert(c->count<32);c->tick[c->count]=tick;c->value[c->count++]=value;
    return 0;
}
int main(void)
{
    /* Observed connected word onset, and the shorter opening of Andrea. */
    struct reloq_german_glottal_segment s={464,529,586,613,1,0,100,0};
    struct captured c={0};
    assert(!reloq_german_glottal_append(&s,1,capture,&c));
    const int64_t t[]={464,487,508,529,586};
    const int32_t v[]={0,100,100,0,0};
    assert(c.count==5 && !memcmp(c.tick,t,sizeof t) && !memcmp(c.value,v,sizeof v));
    s=(struct reloq_german_glottal_segment){0,0,20,60,1,1,100,0};c.count=0;
    assert(!reloq_german_glottal_append(&s,1,capture,&c));
    assert(c.count==2 && c.tick[1]==16 && c.value[0]==100 && c.value[1]==0);

    /* A bare vowel has a different nasal transition from a vowel with
     * following phones: n + A is 80 ms, n + Anna/An is 30 ms. */
    struct reloq_german_phone_node left={0},right={0};
    left.phone=RELOQ_DE_PHONE_n;right.phone=RELOQ_DE_PHONE_A;right.stress=1;
    assert(reloq_german_glottal_transition(&left,&right)==80);
    right.same_word_right=1;
    assert(reloq_german_glottal_transition(&left,&right)==30);

    /* Connected-onset landmarks from the reference: quantity/context,
     * nasal place and the first element of a diphthong all matter. */
    right.phone=RELOQ_DE_PHONE_AT;
    assert(reloq_german_glottal_transition(&left,&right)==10);
    right.phone=RELOQ_DE_PHONE_u;
    assert(reloq_german_glottal_transition(&left,&right)==50);
    left.phone=RELOQ_DE_PHONE_s;right.phone=RELOQ_DE_PHONE_i;
    assert(reloq_german_glottal_transition(&left,&right)==70);
    right.phone=RELOQ_DE_PHONE_A;
    assert(reloq_german_glottal_transition(&left,&right)==60);
    /* Original natural compound heads: Bundesamt / Bundesausbildung /
     * Bundesordnung retain 30 / 30 / 40 ms oral contacts. */
    left.same_word_right=1;right.stress=0;
    assert(reloq_german_glottal_transition(&left,&right)==30);
    right.phone=RELOQ_DE_PHONE_Q_aw;
    assert(reloq_german_glottal_transition(&left,&right)==30);
    right.phone=RELOQ_DE_PHONE_O;
    assert(reloq_german_glottal_transition(&left,&right)==40);
    left.same_word_right=0;right.phone=RELOQ_DE_PHONE_A;
    assert(reloq_german_glottal_transition(&left,&right)==60);

    /* An inflected known root preserves a non-glottal hiatus. Changed
     * custom pronunciations must not inherit that root's acoustic flags. */
    const unsigned char *theater=(const unsigned char *)"`[.0te.1a.0tR]";
    struct reloq_german_word_features f;
    assert(reloq_german_word_features_lookup((const unsigned char *)"Theater",7,
        theater,strlen((const char *)theater),&f)==1);
    assert(f.syllable_count==2 && f.glottal_syllables==0);
    /* Stress alone cannot restore an onset; the observed Days reading can.
     * Other loan-root hiatus remains continuous. */
    const char *days="`[.1da.2ys]";
    assert(reloq_german_word_features_root_hiatus((const unsigned char *)days,strlen(days),&f)==1);
    assert(f.syllable_count==2 && !(f.glottal_syllables&2));
    assert(reloq_german_word_features_lookup((const unsigned char *)"Days",4,
        (const unsigned char *)days,strlen(days),&f)==1 && (f.glottal_syllables&2));
    const char *download="`[.1dO.0v@n.0lo.2at]";
    assert(reloq_german_word_features_root_hiatus((const unsigned char *)download,
        strlen(download),&f)==1 && f.glottal_syllables==1);
    assert(reloq_german_word_features_root_hiatus(theater,strlen((const char *)theater),&f)==1);
    assert(f.syllable_count==3 && !(f.glottal_syllables&2));
    assert(reloq_german_word_features_lookup((const unsigned char *)"Theater",7,
        theater,strlen((const char *)theater),&f)==1);
    struct reloq_german_phone_node p={0};
    p.phone=RELOQ_DE_PHONE_a;p.stress=1;p.same_word_left=1;p.syllable_in_word=1;
    assert(reloq_german_glottal_onset(&p,NULL)==1);
    assert(reloq_german_glottal_onset(&p,&f)==0);
    assert(!reloq_german_word_features_lookup((const unsigned char *)"Theater",7,
        (const unsigned char *)"`[.1ta]",7,&f));

    /* Number/symbol expansion moves the later lexical word index. */
    struct reloq_lexicon_override row={(const unsigned char *)"Theater",7,
        theater,strlen((const char *)theater)};
    struct reloq_lexicon lex;
    assert(!reloq_lexicon_bind(&lex,NULL,0,&row,1,reloq_german_lexicon_validate_spr,NULL));
    const char *text="21 @ Theater";unsigned char *spr=NULL;
    size_t length=0,approximate=0,count=0;
    struct reloq_german_word_features *features=NULL;
    assert(!reloq_german_best_effort_plan_indexed_checked((const unsigned char *)text,
        strlen(text),&lex,&spr,&length,&approximate,NULL,NULL,0,&features,&count,NULL,NULL));
    assert(count==2 && features[0].word_index==0 && features[0].glottal_syllables==1 &&
        features[1].word_index==2 && features[1].glottal_syllables==0);
    free(features);free(spr);
    text="CD Theater";features=NULL;spr=NULL;
    assert(!reloq_german_best_effort_plan_indexed_checked((const unsigned char *)text,
        strlen(text),&lex,&spr,&length,&approximate,NULL,NULL,0,&features,&count,NULL,NULL));
    assert(count==1 && features[0].word_index==2 && features[0].glottal_syllables==0);
    free(features);free(spr);
    /* User policy: und retains the original onset after five, six and eight;
     * the other requested liaison groups remain without this onset.
     * The eighty constituent retains its onset. Separately spoken leading-
     * zero digits and composite numbers keep their established grouping. */
    const struct {const char *text;size_t words,index;uint64_t mask;} numeric[]={
        {"80",1,0,1},{"81",1,0,5},{"82",1,0,5},{"83",1,0,5},
        {"84",1,0,5},{"85",1,0,7},{"86",1,0,7},{"87",1,0,9},
        {"88",1,0,7},{"89",1,0,5},{"180",2,1,1},{"181",2,1,5},
        {"1082",2,1,5},{"81 000",1,0,5},{"81.000",1,0,5},
        {"87 Millionen",2,0,9},{"81,00 EUR",2,0,5},
        {"21",1,0,1},{"22",1,0,1},{"25",1,0,3},{"26",1,0,3},{"28",1,0,3},{"91",1,0,1},
        {"92",1,0,1},{"08",2,1,1}
    };
    for (size_t i=0;i<sizeof numeric/sizeof *numeric;++i) {
        text=numeric[i].text;features=NULL;spr=NULL;
        assert(!reloq_german_best_effort_plan_indexed_checked((const unsigned char *)text,
            strlen(text),&lex,&spr,&length,&approximate,NULL,NULL,0,&features,&count,NULL,NULL));
        /* Only numeric groups have metadata unless a lexical entry was
         * independently observed; unit words need not contribute one. */
        assert(count>numeric[i].index);
        assert(features[numeric[i].index].glottal_syllables==numeric[i].mask);
        free(features);free(spr);
    }
    return 0;
}
