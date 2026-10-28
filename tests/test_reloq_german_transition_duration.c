#include "reloq_german_transition_duration.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void contracts(void)
{
    struct reloq_german_phone_node c = {
        .phone = RELOQ_DE_PHONE_s, .right_phone = RELOQ_DE_PHONE_i,
        .stress = 1, .same_syllable_right = 1, .same_word_right = 1
    };
    struct reloq_german_phone_node v = {
        .phone = RELOQ_DE_PHONE_i, .right_phone = RELOQ_DE_PHONE_s,
        .stress = 1, .same_syllable_right = 1, .same_word_right = 1
    };
    int32_t ticks = -77;
    assert(reloq_german_transition_cv_ticks(NULL, &ticks) == -1);
    assert(reloq_german_transition_cv_ticks(&c, NULL) == -1);
    assert(reloq_german_transition_vc_ticks(NULL, &c, &ticks) == -1);
    assert(reloq_german_transition_vc_ticks(&v, NULL, &ticks) == -1);
    assert(reloq_german_transition_vc_ticks(&v, &c, NULL) == -1);
    assert(ticks == -77);
    c.same_word_right = 0;
    assert(reloq_german_transition_cv_ticks(&c, &ticks) == 0);
    c.same_word_right = 1;
    c.same_syllable_right = 0;
    assert(reloq_german_transition_cv_ticks(&c, &ticks) == 0);
    c.same_syllable_right = 1;
    c.phone = RELOQ_DE_PHONE_R;
    assert(reloq_german_transition_cv_ticks(&c, &ticks) == 0);
    c.phone = RELOQ_DE_PHONE_p; /* aspiration is a separate planner */
    assert(reloq_german_transition_cv_ticks(&c, &ticks) == 0);
    c.phone = RELOQ_DE_PHONE_s;
    c.stress = 3;
    assert(reloq_german_transition_cv_ticks(&c, &ticks) == 0);
    v.phone = RELOQ_DE_PHONE_Q_aj; /* do not time an inner diphthong */
    assert(reloq_german_transition_vc_ticks(&v, &c, &ticks) == 0);
    v.phone = RELOQ_DE_PHONE_i;
    v.right_phone = RELOQ_DE_PHONE_f;
    assert(reloq_german_transition_vc_ticks(&v, &c, &ticks) == 0);
    v.right_phone = c.phone;
    v.same_word_right = 0;
    assert(reloq_german_transition_vc_ticks(&v, &c, &ticks) == 0);
    assert(ticks == -77);
}

static size_t parse(const char *spr, struct reloq_german_phone_node nodes[16])
{
    struct reloq_german_phone_sequence_info info;
    assert(reloq_german_phone_sequence_build((const unsigned char *)spr,
        strlen(spr), 0, nodes, 16, &info) == 0);
    return info.phone_count;
}

static void parsed_contexts(void)
{
    struct reloq_german_phone_node nodes[16];
    int32_t ticks;
    assert(parse("`[.1li.lu]", nodes) == 4);
    assert(reloq_german_transition_cv_ticks(nodes, &ticks) == 1 && ticks == 10);
    assert(reloq_german_transition_cv_ticks(nodes + 2, &ticks) == 1 && ticks == 45);
    assert(parse("`[.1si.0si.2si]", nodes) == 6);
    assert(reloq_german_transition_cv_ticks(nodes, &ticks) == 1 && ticks == 70);
    assert(reloq_german_transition_cv_ticks(nodes + 2, &ticks) == 1 && ticks == 25);
    assert(reloq_german_transition_cv_ticks(nodes + 4, &ticks) == 1 && ticks == 70);
    assert(parse("`[.1pA.2ar.2'E:'l.2er]", nodes) == 8);
    assert(reloq_german_transition_vc_ticks(nodes + 2, nodes + 3, &ticks) == 1 && ticks == 40);
    assert(reloq_german_transition_vc_ticks(nodes + 4, nodes + 5, &ticks) == 1 && ticks == 30);
    assert(reloq_german_transition_vc_ticks(nodes + 6, nodes + 7, &ticks) == 1 && ticks == 24);
    assert(parse("`[.1sI.0ak.0er]", nodes) == 6);
    assert(reloq_german_transition_vc_ticks(nodes + 2, nodes + 3, &ticks) == 1 && ticks == 40);
    assert(reloq_german_transition_vc_ticks(nodes + 4, nodes + 5, &ticks) == 1 && ticks == 24);
    assert(parse("`[.1@r]", nodes) == 2);
    ticks = -77;
    assert(reloq_german_transition_vc_ticks(nodes, nodes + 1, &ticks) == 0 && ticks == -77);
    /* The body and outgoing contact are distinct in opens/nasals and
     * rhotic clusters. Affricate closures keep their separate planner. */
    assert(parse("`[.1'OE'f.0n@t]",nodes)==5);
    assert(reloq_german_transition_cc_ticks(nodes+1,nodes+2,&ticks)==1 && ticks==45);
    assert(parse("`[.1Af.0rA]",nodes)==4);
    assert(reloq_german_transition_cc_ticks(nodes+1,nodes+2,&ticks)==1 && ticks==1);
    assert(parse("`[.1AfrA]",nodes)==4);
    assert(reloq_german_transition_cc_ticks(nodes+1,nodes+2,&ticks)==1 && ticks==35);
    assert(parse("`[.1AG.0l@]",nodes)==4);
    assert(reloq_german_transition_cc_ticks(nodes+1,nodes+2,&ticks)==1 && ticks==30);
    nodes[2].phone=RELOQ_DE_PHONE_Q_a_TILDE;ticks=-77;
    assert(reloq_german_transition_cc_ticks(nodes+1,nodes+2,&ticks)==0 && ticks==-77);
}

int main(void)
{
    contracts();
    parsed_contexts();
    puts("reloq_german_transition_duration: ok");
    return 0;
}
