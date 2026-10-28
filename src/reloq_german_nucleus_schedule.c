#include "reloq_german_glottal.h"
#include "reloq_german_nucleus_schedule.h"

#include "reloq_german_generic_diphthong.h"
#include "reloq_german_transition_duration.h"
#include "reloq_german_onset_cluster.h"
#include "reloq_german_stop_phase.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef enum reloq_german_phone Phone;

static int vowel(Phone p)
{
    return (p >= RELOQ_DE_PHONE_a && p <= RELOQ_DE_PHONE_AT) ||
        p == RELOQ_DE_PHONE_R || p >= RELOQ_DE_PHONE_Q_E_COLON;
}

static int short_vowel(Phone p)
{
    return p == RELOQ_DE_PHONE_A || p == RELOQ_DE_PHONE_E ||
        p == RELOQ_DE_PHONE_I || p == RELOQ_DE_PHONE_O ||
        p == RELOQ_DE_PHONE_U || p == RELOQ_DE_PHONE_Y ||
        p == RELOQ_DE_PHONE_AT || p == RELOQ_DE_PHONE_R ||
        p == RELOQ_DE_PHONE_Q_OE;
}

static int voiceless(Phone p)
{
    return p == RELOQ_DE_PHONE_p || p == RELOQ_DE_PHONE_t ||
        p == RELOQ_DE_PHONE_k || p == RELOQ_DE_PHONE_f ||
        p == RELOQ_DE_PHONE_s || p == RELOQ_DE_PHONE_S ||
        p == RELOQ_DE_PHONE_x || p == RELOQ_DE_PHONE_X ||
        p == RELOQ_DE_PHONE_h || p == RELOQ_DE_PHONE_P ||
        p == RELOQ_DE_PHONE_T || p == RELOQ_DE_PHONE_C;
}

static int diphthong(Phone p)
{
    return p >= RELOQ_DE_PHONE_Q_aj && p <= RELOQ_DE_PHONE_Q_oj;
}

static int pause_after(const struct reloq_german_phone_node *p,
    const enum reloq_german_terminal_mark *marks, size_t count)
{
    return !p->same_word_right && marks && p->word_index < count &&
        marks[p->word_index] != RELOQ_DE_TERMINAL_CONTROL &&
        marks[p->word_index] != RELOQ_DE_TERMINAL_IMPLICIT;
}

static int prominence(unsigned control, int final)
{
    switch (control) {
    case RELOQ_DE_WORD_CONTROL_00: return -1;
    case RELOQ_DE_WORD_CONTROL_0: return 0;
    case RELOQ_DE_WORD_CONTROL_1: return 1;
    case RELOQ_DE_WORD_CONTROL_2: return 2;
    case RELOQ_DE_WORD_CONTROL_3: case RELOQ_DE_WORD_CONTROL_4: return 2;
    default: return final ? 2 : 1;
    }
}

static int stop(Phone p)
{
    return p == RELOQ_DE_PHONE_p || p == RELOQ_DE_PHONE_t ||
        p == RELOQ_DE_PHONE_k;
}

static int fricative(Phone p)
{
    return p == RELOQ_DE_PHONE_f || p == RELOQ_DE_PHONE_v ||
        p == RELOQ_DE_PHONE_s || p == RELOQ_DE_PHONE_z ||
        p == RELOQ_DE_PHONE_S || p == RELOQ_DE_PHONE_Z ||
        p == RELOQ_DE_PHONE_x || p == RELOQ_DE_PHONE_X;
}

static int cluster_stop(const struct reloq_german_phone_node *p, size_t head)
{
    return head >= 2 && p[head].same_syllable_left &&
        p[head-1].same_syllable_left && stop(p[head-1].phone) &&
        !vowel(p[head-2].phone) && p[head-2].phone != RELOQ_DE_PHONE_h;
}

/* An isolated stop keeps its release in the consonant. In an onset
 * cluster that transition belongs to the following nucleus instead. */
static int left_owned(const struct reloq_german_phone_node *p, size_t head)
{
    if (!head || !p[head].same_syllable_left) return 0;
    return stop(p[head-1].phone) && !cluster_stop(p,head);
}

/* One entrance decision for simple and multipart nuclei. The stop release
 * in an onset cluster belongs to the nucleus, including vowel+r/l/j/w.
 * Returning to generic timing here discarded the entire shared budget. */
static int incoming_interval(const struct reloq_german_phone_node *p,
    size_t head, int glottal, int32_t *incoming)
{
    const struct reloq_german_phone_node *node=p+head;
    *incoming=0;
    /* A glottal onset has a separate contact before the oral budget. */
    if (glottal) return 1;
    /* A coda /n,G/ can close the preceding syllable, but its release into the next
     * vowel still belongs to that vowel's budget (10 ms into schwa).
     * A syllable boundary must not delete this acoustic transition. */
    const int nasal_link=head && node->same_word_left &&
        (p[head-1].phone==RELOQ_DE_PHONE_G || p[head-1].phone==RELOQ_DE_PHONE_n);
    const int closing_j_link=head && node->same_word_left && !node->same_syllable_left &&
        !node->stress && p[head-1].phone==RELOQ_DE_PHONE_j &&
        p[head-1].same_syllable_left && vowel(p[head-1].left_phone);
    if (closing_j_link) *incoming=50;
    else if (head && node->same_word_left && vowel(p[head-1].phone) &&
        !node->same_syllable_left && !node->stress) {
        if (reloq_german_transition_vv_ticks(p+head-1,node,incoming)!=1) return 0;
        if(!p[head-1].stress && diphthong(node->phone))*incoming-=*incoming/2;
    } else if (head && (node->same_syllable_left || nasal_link) && node->same_word_left) {
        const Phone left = p[head-1].phone;
        if (nasal_link) {
            struct reloq_german_phone_node from=p[head-1];
            from.same_syllable_right=1;
            if (reloq_german_transition_cv_ticks(&from,incoming)!=1) return 0;
        } else if (cluster_stop(p,head)) {
            struct reloq_german_stop_phase phase;
            struct reloq_german_phone_node from=p[head-1];
            if (head>=3 && from.phone==RELOQ_DE_PHONE_t &&
                p[head-2].phone==RELOQ_DE_PHONE_r && p[head-2].same_syllable_left &&
                !vowel(p[head-3].phone)) from.same_syllable_left=0;
            if (reloq_german_stop_phase_plan(&from,
                node->same_syllable_right?node->right_phone:RELOQ_DE_PHONE_NONE,0,&phase)==1)
                *incoming=phase.incoming_ticks;
            else {
                if (!fricative(p[head-2].phone)) return 0;
                *incoming=left==RELOQ_DE_PHONE_k && node->stress!=1?50:18;
            }
        } else if (!left_owned(p,head) && left != RELOQ_DE_PHONE_h) {
            struct reloq_german_stop_phase phase;
            if (reloq_german_stop_phase_plan(p+head-1,
                node->same_syllable_right?node->right_phone:RELOQ_DE_PHONE_NONE,head==1,&phase)==1)
                *incoming=phase.incoming_ticks;
            else if (reloq_german_transition_cv_ticks(p+head-1, incoming) != 1) return 0;
        }
    }
    return 1;
}

static int intervals(const struct reloq_german_phone_node *p, size_t count,
    size_t head, int connected, int glottal, int short_quantity,
    struct reloq_german_nucleus_schedule *out)
{
    const struct reloq_german_phone_node *node = p + head;
    int32_t incoming = 0, outgoing = 0;
    if (!incoming_interval(p,head,glottal,&incoming)) return 0;
    struct reloq_german_generic_diphthong diph = {0};
    const int is_diph = diphthong(node->phone);
    struct reloq_german_phone_node diph_context=*node;
    struct reloq_german_onset_cluster cluster;
    const int into_cluster=connected && head+1<count &&
        reloq_german_onset_cluster_plan(p,count,head+1,&cluster)==1;
    const int fricative_link=connected &&
        head+1<count && fricative(p[head+1].phone);
    struct reloq_german_stop_phase next_stop;
    const int stop_link=connected && head+1<count &&
        reloq_german_stop_phase_plan(p+head+1,
            head+2<count && p[head+2].same_syllable_right?p[head+2].right_phone:RELOQ_DE_PHONE_NONE,0,&next_stop)==1;
    /* Expanded affricates begin with a real closure. Its contact with a
     * preceding diphthong survives resyllabification; the frication owns
     * the stop release, but must not disable the nucleus schedule. */
    const int affricate_link=connected && head+1<count &&
        p[head+1].affricate_role>=1 && p[head+1].affricate_role<=7 &&
        (p[head+1].affricate_role&1);
    const int nasal_right_link=connected && head+1<count &&
        (p[head+1].phone==RELOQ_DE_PHONE_n || p[head+1].phone==RELOQ_DE_PHONE_m ||
         p[head+1].phone==RELOQ_DE_PHONE_G);
    const int lateral_right_link=connected && head+1<count &&
        p[head+1].phone==RELOQ_DE_PHONE_l && !node->same_syllable_right;
    const int rhotic_right_link=connected && head+1<count && p[head+1].phone==RELOQ_DE_PHONE_r &&
        !node->same_syllable_right;
    const int glide_right_link=connected && head+1<count &&
        (p[head+1].phone==RELOQ_DE_PHONE_j || p[head+1].phone==RELOQ_DE_PHONE_w);
    if (fricative_link || stop_link || into_cluster || affricate_link || nasal_right_link || lateral_right_link || rhotic_right_link || glide_right_link)
        diph_context.same_syllable_right=1;
    if (is_diph && reloq_german_generic_diphthong_plan(&diph_context, connected, &diph) != 1)
        return 0;
    /* An unglottalised, unstressed vowel-to-diphthong contact crosses two nuclei.
     * Its left and right halves are distributed in their own budgets.
     * Charging all of it to the right lost the outgoing left gesture. */
    if(head+1<count && !node->stress && node->same_word_right && !node->same_syllable_right &&
       !p[head+1].stress && diphthong(p[head+1].phone)) {
        int32_t contact=0;
        if(reloq_german_transition_vv_ticks(node,p+head+1,&contact)!=1)return 0;
        outgoing=contact/2;
    }
    if (head + 1 < count && connected && !vowel(p[head+1].phone) &&
        p[head+1].phone != RELOQ_DE_PHONE_h) {
        switch (p[head+1].phone) {
        case RELOQ_DE_PHONE_b: case RELOQ_DE_PHONE_d: case RELOQ_DE_PHONE_g:
            if (p[head+1].affricate_role==7) break;
            if (node->same_syllable_right && head+2<count &&
                p[head+1].same_syllable_right && p[head+2].phone==RELOQ_DE_PHONE_n &&
                head+3==count) break;
            if (!stop_link && !into_cluster && (!node->same_word_right || node->same_syllable_right ||
                head+2>=count || !p[head+1].same_syllable_right ||
                (!vowel(p[head+2].phone) && !into_cluster))) return 0;
            break;
        case RELOQ_DE_PHONE_P: case RELOQ_DE_PHONE_T: case RELOQ_DE_PHONE_C:
        case RELOQ_DE_PHONE_J: return 0; /* No matching outgoing gesture yet. */
        default: break;
        }
        if (is_diph) {
            /* A continuing word edge retains the same glide-to-consonant
             * contact. A boundary pause was excluded by connected above. */
            /* A cluster owns the release of its first stop, not the
             * preceding glide-to-closure contact. Retain the nucleus
             * budget across that boundary, including weak diphthongs. */
            if (!node->same_syllable_right && !fricative_link && !stop_link && !into_cluster && !affricate_link &&
                !nasal_right_link && !lateral_right_link && !rhotic_right_link && !glide_right_link) return 0;
            const int back_glide=node->phone==RELOQ_DE_PHONE_Q_aw;
            outgoing = glide_right_link?(back_glide?40:50):
                rhotic_right_link?(back_glide?15:35):
                lateral_right_link?(back_glide?40:10):diph.before_coda_ticks;
            if (back_glide && nasal_right_link && !node->same_syllable_right &&
                p[head+1].phone==RELOQ_DE_PHONE_n) outgoing=40;
            if ((stop_link || into_cluster || affricate_link) && node->phone==RELOQ_DE_PHONE_Q_aw &&
                (p[head+1].phone==RELOQ_DE_PHONE_t || p[head+1].phone==RELOQ_DE_PHONE_d)) outgoing=40;
            if (!outgoing) return 0;
        } else {
            struct reloq_german_phone_node a = *node, b = p[head+1];
            /* The quantity has already been chosen by the frontend.
             * Do not apply the SPR long-vowel shortening bug a second time.
             * Transitions also span ordinary syllable/word boundaries. */
            a.stress = 1;
            /* A voiced closure before r/l has the same entering transition
             * as a simple closure, although its release is cluster-owned. */
            if (into_cluster) {
                b.right_phone=RELOQ_DE_PHONE_R;
                b.same_syllable_left=0;b.same_syllable_right=1;
            }
            if (b.phone==RELOQ_DE_PHONE_k && b.right_phone==RELOQ_DE_PHONE_h &&
                head+3<count && p[head+2].same_syllable_right && vowel(p[head+3].phone))
                b.right_phone=p[head+3].phone;
            a.same_word_right = a.same_syllable_right = 1;
            if (reloq_german_transition_vc_ticks(&a, &b, &outgoing) != 1) return 0;
            if (node->phone==RELOQ_DE_PHONE_R && !node->same_word_right) {
                if (b.phone==RELOQ_DE_PHONE_v) outgoing=30;
                else if (b.phone==RELOQ_DE_PHONE_j) outgoing=60;
            }
            /* Quantity also selects the contact into a syllable-initial r.
             * Keeping the long contact after shortening the nucleus steals
             * 16/20 ticks from its body. Public SPR does not encode this bit. */
            if (short_quantity && rhotic_right_link && b.same_syllable_right &&
                vowel(b.right_phone) &&
                (node->phone==RELOQ_DE_PHONE_e || node->phone==RELOQ_DE_PHONE_i ||
                 node->phone==RELOQ_DE_PHONE_o || node->phone==RELOQ_DE_PHONE_u ||
                 node->phone==RELOQ_DE_PHONE_y))
                outgoing=node->phone==RELOQ_DE_PHONE_e?24:20;
        }
    }
    struct reloq_nucleus_interval fixed[4];
    int32_t allocated[4], free_ticks;
    size_t n = 0, in = SIZE_MAX, glide = SIZE_MAX, closing = SIZE_MAX, end = SIZE_MAX;
    if (incoming) { in=n; fixed[n++]=(struct reloq_nucleus_interval){incoming,1}; }
    if (is_diph) {
        glide=n; fixed[n++]=(struct reloq_nucleus_interval){diph.glide_ticks,1};
        closing=n; fixed[n++]=(struct reloq_nucleus_interval){out->budget.base.closing_ticks,1};
    }
    if (outgoing) { end=n; fixed[n++]=(struct reloq_nucleus_interval){outgoing,1}; }
    if (reloq_german_nucleus_distribute(fixed,n,out->budget.adjusted_ticks,
                                      3,allocated,&free_ticks)) return -1;
    out->vowel_ticks = free_ticks;
    out->incoming_ticks = in == SIZE_MAX ? 0 : allocated[in];
    out->outgoing_ticks = end == SIZE_MAX ? 0 : allocated[end];
    out->glide_ticks = glide == SIZE_MAX ? 0 : allocated[glide];
    out->closing_ticks = closing == SIZE_MAX ? 0 : allocated[closing];
    out->span_ticks = free_ticks;
    for (size_t i=0;i<n;++i) out->span_ticks += allocated[i];
    return 1;
}

/* A two-phone nucleus has one budget, including its internal transition.
 * Its closing liquid must not add another generic consonant duration. */
static int multi_intervals(const struct reloq_german_phone_node *p,size_t count,
    size_t head,int connected,int glottal,struct reloq_german_nucleus_schedule plan[2])
{
    const Phone tail=p[head+1].phone;
    if (diphthong(p[head].phone) && tail==RELOQ_DE_PHONE_l) {
        int32_t incoming=0,outgoing=0;
        if (!incoming_interval(p,head,glottal,&incoming)) return 0;
        if (connected && head+2<count && p[head+2].phone!=RELOQ_DE_PHONE_h &&
            !(vowel(p[head+2].phone) && !p[head+2].same_syllable_left)) {
            const Phone next=p[head+2].phone;
            if (vowel(next) || next==RELOQ_DE_PHONE_h) return 0;
            outgoing=next==RELOQ_DE_PHONE_l?30:next==RELOQ_DE_PHONE_w?10:
                next==RELOQ_DE_PHONE_j?40:25;
        }
        /* A diphthong plus l has three members in the shared budget:
         * vowel, j/w and lateral. The glide and both contacts consume that
         * one budget; l must not add an independent generic duration. */
        const int glide=p[head].phone==RELOQ_DE_PHONE_Q_oj?45:75;
        struct reloq_nucleus_interval fixed[6]={{incoming,incoming?1:0},
            {glide,1},{20,1},{p[head].phone==RELOQ_DE_PHONE_Q_aw?40:50,1},
            {40,1},{outgoing,outgoing?1:0}};
        struct reloq_nucleus_interval used[6];size_t map[6],n=0;
        int32_t allocated[6],value[6]={0},free_ticks;
        for (size_t j=0;j<6;++j) if (fixed[j].ticks) {map[n]=j;used[n++]=fixed[j];}
        if (reloq_german_nucleus_distribute(used,n,plan[0].budget.adjusted_ticks,
            3,allocated,&free_ticks)) return -1;
        for (size_t j=0;j<n;++j) value[map[j]]=allocated[j];
        plan[0].incoming_ticks=value[0];plan[0].vowel_ticks=free_ticks;
        plan[0].glide_ticks=value[1];plan[0].closing_ticks=value[2];
        plan[0].outgoing_ticks=value[3];
        plan[0].span_ticks=value[0]+free_ticks+value[1]+value[2]+value[3];
        plan[1].closing_ticks=value[4];plan[1].outgoing_ticks=value[5];
        plan[1].span_ticks=value[4]+value[5];return 1;
    }
    if (diphthong(p[head].phone) ||
        (tail!=RELOQ_DE_PHONE_r && tail!=RELOQ_DE_PHONE_l &&
         tail!=RELOQ_DE_PHONE_j && tail!=RELOQ_DE_PHONE_w)) return 0;
    int32_t incoming=0,internal=0,outgoing=0;
    if (!incoming_interval(p,head,glottal,&incoming)) return 0;
    struct reloq_german_phone_node v=p[head];v.stress=1;
    if (tail==RELOQ_DE_PHONE_j || tail==RELOQ_DE_PHONE_w)
        internal=tail==RELOQ_DE_PHONE_w?75:p[head].phone==RELOQ_DE_PHONE_O?45:
            p[head].phone==RELOQ_DE_PHONE_o || p[head].phone==RELOQ_DE_PHONE_u ||
            p[head].phone==RELOQ_DE_PHONE_U || p[head].phone==RELOQ_DE_PHONE_R?50:75;
    else if (reloq_german_transition_vc_ticks(&v,p+head+1,&internal)!=1) return 0;
    if (connected && head+2<count && p[head+2].phone!=RELOQ_DE_PHONE_h &&
        !(vowel(p[head+2].phone) && !p[head+2].same_syllable_left)) {
        const Phone next=p[head+2].phone;
        if (vowel(next) || next==RELOQ_DE_PHONE_h) return 0;
        if (tail==RELOQ_DE_PHONE_j || tail==RELOQ_DE_PHONE_w)
            outgoing=next==RELOQ_DE_PHONE_l?50:
                next==RELOQ_DE_PHONE_j || next==RELOQ_DE_PHONE_w?50:
                next==RELOQ_DE_PHONE_r?(p[head+2].same_syllable_left?50:35):
                tail==RELOQ_DE_PHONE_w && (next==RELOQ_DE_PHONE_s || next==RELOQ_DE_PHONE_t)?40:15;
        else if (tail==RELOQ_DE_PHONE_l)
            outgoing=next==RELOQ_DE_PHONE_l?30:next==RELOQ_DE_PHONE_w?10:
                     next==RELOQ_DE_PHONE_j?40:25;
        else {
            const Phone first=p[head].phone;
            const int front=first==RELOQ_DE_PHONE_E || first==RELOQ_DE_PHONE_e ||
                first==RELOQ_DE_PHONE_I || first==RELOQ_DE_PHONE_i ||
                first==RELOQ_DE_PHONE_Y || first==RELOQ_DE_PHONE_y ||
                first==RELOQ_DE_PHONE_Q_E_COLON || first==RELOQ_DE_PHONE_Q_OE ||
                first==RELOQ_DE_PHONE_Q_oe;
            outgoing=next==RELOQ_DE_PHONE_t || next==RELOQ_DE_PHONE_d?
                (front?50:20):next==RELOQ_DE_PHONE_l?20:30;
        }
    }
    struct reloq_nucleus_interval fixed[4]={
        {incoming, incoming?1:0},{internal,1},
        {plan[0].budget.base.closing_ticks,1},{outgoing,outgoing?1:0}};
    int32_t allocated[4],free_ticks;
    /* Empty intervals are omitted because the budget API requires a
     * positive minimum for every interval that actually exists. */
    struct reloq_nucleus_interval used[4];size_t map[4],n=0;
    for (size_t j=0;j<4;++j) if (fixed[j].ticks) {map[n]=j;used[n++]=fixed[j];}
    int32_t values[4]={0};
    if (reloq_german_nucleus_distribute(used,n,plan[0].budget.adjusted_ticks,
        3,allocated,&free_ticks)) return -1;
    for (size_t j=0;j<n;++j) values[map[j]]=allocated[j];
    plan[0].incoming_ticks=values[0];plan[0].vowel_ticks=free_ticks;
    plan[0].outgoing_ticks=values[1];
    plan[0].span_ticks=values[0]+free_ticks+values[1];
    plan[1].closing_ticks=values[2];plan[1].outgoing_ticks=values[3];
    plan[1].span_ticks=values[2]+values[3];
    return 1;
}

/* Vowel, vocalic r and lateral share one nucleus budget. These contacts
 * remain distinct even when the syllable is too short for their full sizes. */
static int triple_intervals(const struct reloq_german_phone_node *p,size_t count,
    size_t head,int connected,int glottal,struct reloq_german_nucleus_schedule plan[3])
{
    if (diphthong(p[head].phone) || p[head+1].phone!=RELOQ_DE_PHONE_r ||
        p[head+2].phone!=RELOQ_DE_PHONE_l) return 0;
    int32_t incoming=0,internal=0,outgoing=0;
    if (!incoming_interval(p,head,glottal,&incoming)) return 0;
    struct reloq_german_phone_node v=p[head];v.stress=1;
    if (reloq_german_transition_vc_ticks(&v,p+head+1,&internal)!=1) return 0;
    if (connected && head+3<count && p[head+3].phone!=RELOQ_DE_PHONE_h &&
        !(vowel(p[head+3].phone) && !p[head+3].same_syllable_left)) {
        const Phone next=p[head+3].phone;
        if (vowel(next)) return 0;
        outgoing=next==RELOQ_DE_PHONE_l?30:next==RELOQ_DE_PHONE_w?10:
            next==RELOQ_DE_PHONE_j?40:25;
    }
    struct reloq_nucleus_interval fixed[6]={{incoming,incoming?1:0},
        {internal,1},{20,1},{20,1},{40,1},{outgoing,outgoing?1:0}};
    struct reloq_nucleus_interval used[6];size_t map[6],n=0;
    int32_t allocated[6],value[6]={0},free_ticks;
    for (size_t j=0;j<6;++j) if (fixed[j].ticks) {map[n]=j;used[n++]=fixed[j];}
    if (reloq_german_nucleus_distribute(used,n,plan[0].budget.adjusted_ticks,
        3,allocated,&free_ticks)) return -1;
    for (size_t j=0;j<n;++j) value[map[j]]=allocated[j];
    plan[0].incoming_ticks=value[0];plan[0].vowel_ticks=free_ticks;
    plan[0].outgoing_ticks=value[1];
    plan[0].span_ticks=value[0]+free_ticks+value[1];
    plan[1].closing_ticks=value[2];plan[1].outgoing_ticks=value[3];
    plan[1].span_ticks=value[2]+value[3];
    plan[2].closing_ticks=value[4];plan[2].outgoing_ticks=value[5];
    plan[2].span_ticks=value[4]+value[5];return 1;
}

int reloq_german_nucleus_schedule_build_with_features(
    const struct reloq_german_phone_node *phones, size_t count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    const struct reloq_german_word_features *features, size_t feature_count,
    struct reloq_german_nucleus_schedule *out,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    if ((!phones && count) || (!out && count) || (!syllables && syllable_count) ||
        (!features && feature_count) ||
        count > SIZE_MAX / sizeof *out ||
        count > SIZE_MAX / (3*sizeof(struct reloq_german_duration_phone)) ||
        count > (SIZE_MAX/sizeof(size_t)-1)/2 ||
        syllable_count > SIZE_MAX/(2*sizeof(unsigned)) ||
        syllable_count > UINT_MAX) return -1;
    if (!count) return syllable_count ? -1 : 0;
    if (cancelled && cancelled(cancel_context)) return 1;
    struct reloq_german_duration_phone *normalized = calloc(3*count, sizeof *normalized);
    size_t *mapping = malloc((2*count+1)*sizeof *mapping);
    unsigned *following = calloc(2*syllable_count, sizeof *following);
    if (!normalized || !mapping || !following) {
        free(normalized);free(mapping);free(following);return -1;
    }
    size_t *ends = mapping + count + 1;
    size_t used = 0;
    int result = -1;
    for (size_t i=0;i<count;++i) {
        if (cancelled && !(i&255) && cancelled(cancel_context)) {result=1;goto done;}
        Phone p=phones[i].phone;
        if (p<=RELOQ_DE_PHONE_NONE || p>RELOQ_DE_PHONE_Q_oe_TILDE || phones[i].stress>2)
            goto done;
        mapping[i]=used;
        /* In the observed duration topology h is a vowel onset property.
         * Its independently emitted aspiration interval must not also count
         * as a shortening consonant in the neighboring nucleus budget. */
        if (p==RELOQ_DE_PHONE_h) {ends[i]=used;continue;}
        normalized[used++]=(struct reloq_german_duration_phone){p,phones[i].stress,
            (uint8_t)(short_vowel(p)||reloq_german_nucleus_shortened(phones+i)),
            (uint8_t)(voiceless(p) ||
                (p==RELOQ_DE_PHONE_Z && !phones[i].same_word_right)),0};
        if (diphthong(p)) {
            normalized[used-1].phone=p==RELOQ_DE_PHONE_Q_oj ? RELOQ_DE_PHONE_O : RELOQ_DE_PHONE_A;
            normalized[used-1].short_quantity=1;
            normalized[used++]=(struct reloq_german_duration_phone){
                p==RELOQ_DE_PHONE_Q_aw ? RELOQ_DE_PHONE_w : RELOQ_DE_PHONE_j,
                phones[i].stress,0,0,0};
        }
        ends[i]=used;
        if (pause_after(phones+i, marks, mark_count)) ++used;
    }
    mapping[count]=used;
    size_t cursor=0;
    for (size_t s=0;s<syllable_count;++s) {
        if (syllables[s].first_phone!=cursor || !syllables[s].phone_count ||
            syllables[s].phone_count>count-cursor) goto done;
        cursor+=syllables[s].phone_count;
    }
    if (cursor!=count) goto done;
    for (size_t s=syllable_count;s-- > 0;) {
        size_t last=syllables[s].first_phone+syllables[s].phone_count-1;
        /* A continuing phrase boundary does not end the look-ahead for
         * unstressed syllables. That scan controls final-nucleus shortening
         * independently of the acoustic pause and the local final region. */
        const int ended=pause_after(phones+last,marks,mark_count) &&
            !reloq_german_terminal_continues(marks[phones[last].word_index]);
        if (s+1<syllable_count && !ended) {
            following[s*2+1]=following[(s+1)*2+1]+1;
            if (!syllables[s+1].stress) following[s*2]=following[(s+1)*2]+1;
        }
    }
    memset(out,0,count*sizeof *out);
    size_t word_first=0,word_last=0,word_syllable_first=0,word_syllable_end=0,feature=0;
    for (size_t s=0;s<syllable_count;++s) {
        if (cancelled && cancelled(cancel_context)) {result=1;goto done;}
        const struct reloq_german_syllable *syll=syllables+s;
        if (s==word_syllable_end) {
            word_first=syll->first_phone;word_last=word_first+syll->phone_count;
            word_syllable_first=s;word_syllable_end=s+1;
            while (word_syllable_end<syllable_count &&
                syllables[word_syllable_end].word_index==syll->word_index) {
                word_last+=syllables[word_syllable_end].phone_count;++word_syllable_end;
            }
        }
        size_t first,length;
        int found=reloq_german_nucleus_find(phones+syll->first_phone,
                                           syll->phone_count,&first,&length);
        if (found<0) goto done;
        if (!found || length>3) continue;
        const size_t head=syll->first_phone+first;
        if (length==1 && head+1<syll->first_phone+syll->phone_count && vowel(phones[head+1].phone)) continue;
        const int final=word_last==count || pause_after(phones+word_last-1,marks,mark_count);
        struct reloq_german_nucleus_context ctx={
            normalized,used,mapping[word_first],ends[word_last-1],
            mapping[syll->first_phone],ends[syll->first_phone+syll->phone_count-1],
            mapping[head],ends[head+length-1],word_syllable_end-word_syllable_first,
            s-word_syllable_first,syll->stress,prominence(phones[head].prosody_control,final),
            final,final && s+1==word_syllable_end,following[s*2],following[s*2+1],
            (unsigned)left_owned(phones,head)
        };
        struct reloq_german_nucleus_schedule plan={0};
        while (feature<feature_count && features[feature].word_index<syll->word_index) ++feature;
        if (phones[head].syllable_in_word<64 && feature<feature_count &&
            features[feature].word_index==syll->word_index &&
            ((features[feature].full_quantity_syllables>>phones[head].syllable_in_word)&1u))
            normalized[mapping[head]].short_quantity=0;
        if (reloq_german_nucleus_budget(&ctx,&plan.budget)!=1) continue;
        if (length==1 && phones[head].phone==RELOQ_DE_PHONE_A && !syll->stress &&
            head+1==count && phones[head].syllable_in_word<64 &&
            feature<feature_count && features[feature].word_index==syll->word_index &&
            ((features[feature].full_a_syllables>>phones[head].syllable_in_word)&1u)) {
            /* Requested voice preference, not an inferred original rule.
             * Both quantities pass through the same contextual duration
             * planner. Use one quarter of the long-short difference before
             * rounding and interval allocation. */
            const size_t index=mapping[head];
            struct reloq_german_duration_phone saved=normalized[index];
            struct reloq_german_nucleus_budget longer;
            normalized[index].phone=RELOQ_DE_PHONE_a;
            normalized[index].short_quantity=0;
            const int supported=reloq_german_nucleus_budget(&ctx,&longer);
            normalized[index]=saved;
            if (supported!=1) goto done;
            plan.budget.phrase_ticks=(3.0*plan.budget.phrase_ticks+longer.phrase_ticks)/4.0;
            plan.budget.adjusted_ticks=(3.0*plan.budget.adjusted_ticks+longer.adjusted_ticks)/4.0;
        }
        const int connected=head+length<count && !pause_after(phones+head+length-1,marks,mark_count) &&
            (phones[head+length-1].same_word_right || !marks ||
             marks[phones[head+length-1].word_index]!=RELOQ_DE_TERMINAL_CONTROL);
        const int glottal=reloq_german_glottal_onset(phones+head,
            feature<feature_count?features+feature:NULL);
        if (length==3) {
            struct reloq_german_nucleus_schedule triple[3]={plan,{0},{0}};
            found=triple_intervals(phones,count,head,connected,glottal,triple);
            if (found<0) goto done;
            if (found) for (size_t j=0;j<3;++j) out[head+j]=triple[j];
            continue;
        }
        if (length==2) {
            struct reloq_german_nucleus_schedule pair[2]={plan,{0}};
            found=multi_intervals(phones,count,head,connected,glottal,pair);
            if (found<0) goto done;
            if (found) {out[head]=pair[0];out[head+1]=pair[1];}
            continue;
        }
        found=intervals(phones,count,head,connected,glottal,
            normalized[mapping[head]].short_quantity,&plan);
        if (found<0) goto done;
        if (found) out[head]=plan;
    }
    result=0;
done:
    free(normalized);free(mapping);free(following);
    return result;
}

int reloq_german_nucleus_schedule_build(
    const struct reloq_german_phone_node *phones, size_t count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    struct reloq_german_nucleus_schedule *out,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    return reloq_german_nucleus_schedule_build_with_features(phones,count,
        syllables,syllable_count,marks,mark_count,NULL,0,out,cancelled,cancel_context);
}
