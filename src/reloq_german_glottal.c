#include "reloq_german_glottal.h"
#include "reloq_german_transition_duration.h"
#include "reloq_german_stop_phase.h"

static int vowel(enum reloq_german_phone p)
{
    return (p>=RELOQ_DE_PHONE_a && p<=RELOQ_DE_PHONE_AT) ||
        (p>=RELOQ_DE_PHONE_Q_E_COLON && p<=RELOQ_DE_PHONE_Q_oe_TILDE);
}

int reloq_german_glottal_onset(const struct reloq_german_phone_node *p,
    const struct reloq_german_word_features *f)
{
    if (!p || p->same_syllable_left || !vowel(p->phone)) return 0;
    if (f && f->word_index==p->word_index && p->syllable_in_word<64 &&
        p->syllable_in_word<f->syllable_count)
        return (int)((f->glottal_syllables>>p->syllable_in_word)&1u);
    return !p->same_word_left || p->stress!=0;
}

int32_t reloq_german_glottal_transition(
    const struct reloq_german_phone_node *left,
    const struct reloq_german_phone_node *right)
{
    if (!left || !right) return 0;
    struct reloq_german_stop_phase boundary;
    if (reloq_german_stop_vowel_boundary(left,right->stress,
        right->same_syllable_right?right->right_phone:RELOQ_DE_PHONE_NONE,&boundary)==1)
        return boundary.incoming_ticks;
    /* A glottal gesture modulates the source during the existing vowel
     * transition. Its presence does not give every vowel pair 65 ticks. */
    int32_t contact=0;
    /* The reference's acoustic sequence splits a diphthong into its vowel
     * and glide. For the incoming glottal transition, use that first vowel;
     * the portable linguistic sequence retains the complete diphthong. */
    struct reloq_german_phone_node entry=*right;
    if (entry.phone==RELOQ_DE_PHONE_Q_aj || entry.phone==RELOQ_DE_PHONE_Q_aw)
        entry.phone=RELOQ_DE_PHONE_A;
    else if (entry.phone==RELOQ_DE_PHONE_Q_oj) entry.phone=RELOQ_DE_PHONE_O;
    if (reloq_german_transition_vv_ticks(left,&entry,&contact)==1) return contact;
    const int main=right->stress==1;
    const int weak=right->weak_word_control;
    switch (left->phone) {
    case RELOQ_DE_PHONE_r: return main?90:65;
    case RELOQ_DE_PHONE_l: return right->stress?60:40;
    case RELOQ_DE_PHONE_m:
        if(left->same_word_right)return 15;
        return main && !weak && !right->same_word_right?80:40;
    case RELOQ_DE_PHONE_n: case RELOQ_DE_PHONE_G:
        if (!left->same_word_right && main && !weak && !right->same_word_right) return 80;
        if (right->phone==RELOQ_DE_PHONE_AT) return 10;
        if (left->phone==RELOQ_DE_PHONE_n) {
            if (right->phone==RELOQ_DE_PHONE_U) return 40;
            if (right->phone==RELOQ_DE_PHONE_o || right->phone==RELOQ_DE_PHONE_u) return 50;
        }
        return 30;
    case RELOQ_DE_PHONE_f: return left->same_word_right?25:35;
    case RELOQ_DE_PHONE_v: return 35;
    case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_z:
        if(left->same_word_right && !right->stress) {
            /* An unstressed internal head keeps the ordinary short oral
             * contact, even when word formation requests a fresh source
             * onset. Use the diphthong's entering vowel here as above. */
            struct reloq_german_phone_node onset=*left;
            onset.right_phone=entry.phone;
            onset.same_syllable_right=1;
            onset.stress=0;
            if(reloq_german_transition_cv_ticks(&onset,&contact)==1)return contact;
        }
        if(left->same_word_right && right->phone==RELOQ_DE_PHONE_U &&
            right->stress!=1)return 30;
        return right->phone==RELOQ_DE_PHONE_i?70:60;
    case RELOQ_DE_PHONE_S: return 30;
    case RELOQ_DE_PHONE_Z: return 40;
    case RELOQ_DE_PHONE_x: case RELOQ_DE_PHONE_X: return main?50:30;
    case RELOQ_DE_PHONE_p: case RELOQ_DE_PHONE_b:
    case RELOQ_DE_PHONE_t: case RELOQ_DE_PHONE_d:
    case RELOQ_DE_PHONE_k: case RELOQ_DE_PHONE_g: return weak?55:30;
    case RELOQ_DE_PHONE_j: return 50;
    case RELOQ_DE_PHONE_w: return right->phone==RELOQ_DE_PHONE_e?60:40;
    default: return 65;
    }
}

unsigned reloq_german_glottal_strength(const struct reloq_german_phone_node *p)
{
    if (!p || !p->weak_word_control) return 100;
    return vowel(p->left_phone) || p->left_phone==RELOQ_DE_PHONE_R ||
        p->left_phone==RELOQ_DE_PHONE_r || p->left_phone==RELOQ_DE_PHONE_j ||
        p->left_phone==RELOQ_DE_PHONE_w?80:20;
}

static int point(reloq_german_source_emit emit,void *ctx,size_t phone,
                 int64_t tick,unsigned di,unsigned oq)
{
    return emit(ctx,RELOQ_TARGET_DI,tick,(int32_t)di,phone) ||
           emit(ctx,RELOQ_TARGET_OQ,tick,(int32_t)oq,phone);
}

int reloq_german_glottal_append(
    const struct reloq_german_glottal_segment *s,size_t count,
    reloq_german_source_emit emit,void *context)
{
    if ((!s && count) || !emit) return -1;
    for (size_t i=0;i<count;++i) {
        const struct reloq_german_glottal_segment *g=s+i;
        if (g->begin<0 || g->onset<g->begin || g->end<g->onset ||
            g->release<g->end || g->strength>100) return -1;
        if (!g->active) {
            if (point(emit,context,i,g->begin,0,56)) return -1;
        } else if (g->fresh) {
            /* A short first nucleus shares the opening with its outgoing
             * transition: >65 ms permits 21, >40 ms permits 16. */
            int recovery=g->end-g->onset>50 || g->release-g->onset>65?21:
                g->release-g->onset>40?16:0;
            if (recovery) {
                if (point(emit,context,i,g->onset,100,18) ||
                    point(emit,context,i,g->onset+recovery,0,56)) return -1;
            } else if (point(emit,context,i,g->onset,0,56)) return -1;
        } else {
            int64_t duration=g->onset-g->begin;
            if (duration>INT32_MAX) return -1;
            if ((!i || !s[i-1].release_stop) &&
                point(emit,context,i,g->begin,0,56)) return -1;
            if (duration>3 &&
                (point(emit,context,i,g->onset-duration*66/100,g->strength,18) ||
                 point(emit,context,i,g->onset-duration*33/100,g->strength,18))) return -1;
            if (point(emit,context,i,g->onset,0,56)) return -1;
        }
        if ((!g->active || !g->fresh || g->end-g->onset>21) &&
            point(emit,context,i,g->end,0,56)) return -1;
    }
    return 0;
}
