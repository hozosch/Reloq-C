#include "reloq_german_coda_timing.h"
#include "reloq_german_transition_duration.h"
#include "reloq_german_generic_transition.h"

typedef enum reloq_german_phone Phone;
static int vowel(Phone p)
{
    return (p>=RELOQ_DE_PHONE_a && p<=RELOQ_DE_PHONE_AT) ||
        p==RELOQ_DE_PHONE_R || p>=RELOQ_DE_PHONE_Q_E_COLON;
}

static int fricative(Phone p)
{
    return p==RELOQ_DE_PHONE_f || p==RELOQ_DE_PHONE_v ||
        p==RELOQ_DE_PHONE_s || p==RELOQ_DE_PHONE_z ||
        p==RELOQ_DE_PHONE_S || p==RELOQ_DE_PHONE_x || p==RELOQ_DE_PHONE_X;
}

/* A syllable boundary does not select one generic coda duration. The
 * following consonant class, word boundary and accent have separate roles
 * in ger_ph_{s,f,S,x,X}_dur. Keep the main gesture separate from the
 * independently established consonant-to-consonant transition.
 * Evidence and holdouts: reference-apple/behavior-0.2.33. */
static int fricative_coda(const struct reloq_german_phone_node *n,
    const struct reloq_german_phone_node *r,
    struct reloq_german_coda_timing *out)
{
    const Phone next=r->phone;
    if (!n->same_syllable_left || n->same_syllable_right ||
        !vowel(n->left_phone) || n->affricate_role || r->affricate_role)
        return 0;
    const int nas=next==RELOQ_DE_PHONE_m || next==RELOQ_DE_PHONE_n || next==RELOQ_DE_PHONE_G;
    const int liquid=next==RELOQ_DE_PHONE_l || next==RELOQ_DE_PHONE_r;
    const int stop=next==RELOQ_DE_PHONE_p || next==RELOQ_DE_PHONE_t ||
        next==RELOQ_DE_PHONE_k || next==RELOQ_DE_PHONE_b ||
        next==RELOQ_DE_PHONE_d || next==RELOQ_DE_PHONE_g;
    if (!nas && !liquid && !stop && !fricative(next)) return 0;
    const int word=!n->same_word_right;
    int32_t body,transition=0;
    switch (n->phone) {
    case RELOQ_DE_PHONE_s:
        if (word) body=fricative(next)?85:nas?(n->stress?102:85):n->stress?96:75;
        else if (next==RELOQ_DE_PHONE_S) body=n->stress==1 && !r->stress?70:35;
        else if (fricative(next)) body=95;
        else if (liquid || next==RELOQ_DE_PHONE_n) body=110;
        else if (nas) body=92;
        else body=r->stress?76:64;
        break;
    case RELOQ_DE_PHONE_f:
        if (n->weak_word_control) body=100;
        else if (next==RELOQ_DE_PHONE_l) body=110;
        else if (next==RELOQ_DE_PHONE_r) body=95;
        else if (!word && fricative(next)) body=72;
        else body=word || r->stress?118:95;
        break;
    case RELOQ_DE_PHONE_S:
        body=next==RELOQ_DE_PHONE_S?70:
            !word && (next==RELOQ_DE_PHONE_p || next==RELOQ_DE_PHONE_t)?80:110;
        break;
    case RELOQ_DE_PHONE_x:
        body=fricative(next)?80:liquid || nas?word?(n->stress?128:90):78:60;
        break;
    case RELOQ_DE_PHONE_X:
        body=next==RELOQ_DE_PHONE_S?85:
            next==RELOQ_DE_PHONE_s || next==RELOQ_DE_PHONE_f ||
            next==RELOQ_DE_PHONE_x || next==RELOQ_DE_PHONE_X?70:
            word || n->stress?90:80;
        break;
    default:return 0;
    }
    if (reloq_german_transition_cc_ticks(n,r,&transition)!=1) return 0;
    *out=(struct reloq_german_coda_timing){body,transition};
    return 1;
}

/* Independently formulated from the duration/transition decisions in
 * ger_ph_{m,n,G,f,s,S,x,X}_dur and get_ger_{nas,fric}_trans_dur.
 * Body and outgoing transition are separate: a 1-tick consonant contact
 * is not a generic 8/30-ms release, and ng->t retains its 30-tick move.
 * See reference-apple/behavior-0.2.29 for observations and branch evidence. */
int reloq_german_coda_timing_plan(
    const struct reloq_german_phone_node *p,size_t count,size_t i,
    int pause_after_next,struct reloq_german_coda_timing *out)
{
    if (!p || !out || i>=count || (pause_after_next!=0 && pause_after_next!=1))
        return -1;
    if (i+1>=count) return 0;
    const struct reloq_german_phone_node *n=p+i,*r=p+i+1;
    /* An initial consonant before the rounded glide keeps a separate
     * consonant body and oral contact; w does not replace both intervals. */
    if (!n->same_word_left && n->same_syllable_right &&
        r->phone==RELOQ_DE_PHONE_w && r->same_syllable_right &&
        vowel(r->right_phone) && !n->affricate_role) {
        int32_t body=0,contact=0;
        switch (n->phone) {
        case RELOQ_DE_PHONE_s: body=92;break;
        case RELOQ_DE_PHONE_z: body=n->stress==1?90:78;break;
        case RELOQ_DE_PHONE_f: body=n->stress?118:95;break;
        case RELOQ_DE_PHONE_v: body=48;break;
        case RELOQ_DE_PHONE_S: body=87;break;
        case RELOQ_DE_PHONE_X: body=n->stress?90:80;break;
        case RELOQ_DE_PHONE_n: body=n->stress==1?70:40;break;
        case RELOQ_DE_PHONE_m: body=85;break;
        case RELOQ_DE_PHONE_l: body=60;break;
        case RELOQ_DE_PHONE_r: body=75;break;
        default:break;
        }
        if (body && reloq_german_transition_cc_ticks(n,r,&contact)==1) {
            *out=(struct reloq_german_coda_timing){body,contact};return 1;
        }
    }
    /* A connected vowel does not give every final s the same body.
     * Phrase prominence on either side selects the observed coda duration. */
    if(n->phone==RELOQ_DE_PHONE_s && !n->same_word_right &&
       n->same_syllable_left && vowel(r->phone) && !n->affricate_role) {
        int body=fricative(n->left_phone)?70:
            n->weak_word_control?75:r->weak_word_control?55:96;
        *out=(struct reloq_german_coda_timing){body,0};return 1;
    }
    if (n->phone==RELOQ_DE_PHONE_s && !n->same_word_right &&
        n->same_syllable_left && !vowel(r->phone) &&
        (n->left_phone==RELOQ_DE_PHONE_p || n->left_phone==RELOQ_DE_PHONE_k ||
         n->left_phone==RELOQ_DE_PHONE_t)) {
        int32_t body=0,contact=0;
        const int cluster=r->same_syllable_right && !vowel(r->right_phone);
        if (reloq_german_generic_fricative_ticks(n,0,cluster,&body)==1 &&
            reloq_german_transition_cc_ticks(n,r,&contact)==1) {
            *out=(struct reloq_german_coda_timing){body,contact};return 1;
        }
    }
    struct reloq_german_phone_node closure;
    if (fricative(n->phone) && r->affricate_role==3 &&
        n->same_word_right && !n->same_syllable_right) {
        closure=*r;closure.affricate_role=0;r=&closure;
    }
    if (r==&closure &&
        n->phone==RELOQ_DE_PHONE_f && n->same_syllable_left && vowel(n->left_phone)) {
        *out=(struct reloq_german_coda_timing){60,1};return 1;
    }
    /* /s.ts/ has its own coda branch: the first fricative is 55 ticks
     * plus one tick of contact, independently of the following stress.
     * Treating the affricate closure as a plain /t/ lengthened this
     * sequence in both character names and ordinary compounds. */
    if(r==&closure && n->phone==RELOQ_DE_PHONE_s &&
       n->same_syllable_left && vowel(n->left_phone)) {
        *out=(struct reloq_german_coda_timing){55,1};return 1;
    }
    if (!n->affricate_role && n->same_word_right &&
        !n->same_syllable_right && vowel(r->phone)) {
        const int nasal_left=n->left_phone==RELOQ_DE_PHONE_m ||
            n->left_phone==RELOQ_DE_PHONE_n || n->left_phone==RELOQ_DE_PHONE_G;
        const int schwa_n=r->phone==RELOQ_DE_PHONE_AT &&
            r->same_syllable_right && r->right_phone==RELOQ_DE_PHONE_n;
        int32_t body=0;
        /* Frication does not become a consonant-cluster fallback merely
         * because the following vowel starts a new syllable. Its CV contact
         * is allocated by the nucleus/glottal planner, never a quiet tail. */
        if (n->phone==RELOQ_DE_PHONE_f)
            body=nasal_left?105:r->stress?118:schwa_n?90:95;
        else if (!r->stress || n->phone==RELOQ_DE_PHONE_x || n->phone==RELOQ_DE_PHONE_X) switch (n->phone) {
        case RELOQ_DE_PHONE_s: body=85;break;
        case RELOQ_DE_PHONE_S: body=nasal_left || !vowel(n->left_phone)?110:125;break;
        case RELOQ_DE_PHONE_x: body=r->phone==RELOQ_DE_PHONE_AT?80:95;break;
        case RELOQ_DE_PHONE_X: body=schwa_n && vowel(n->left_phone)?111:90;break;
        default:break;
        }
        if (body) {*out=(struct reloq_german_coda_timing){body,0};return 1;}
    }
    if (fricative_coda(n,r,out)) return 1;
    const Phone next=r->phone;
    const int nasal=n->phone==RELOQ_DE_PHONE_m || n->phone==RELOQ_DE_PHONE_n ||
        n->phone==RELOQ_DE_PHONE_G;
    /* Nasal + non-stop codas retain their own body and contact. A
     * following consonant does not imply the generic nasal duration.
     * Keep phrase-final lengthening distinct from an internal group.
     * Numeric contexts: behavior-0.2.36/nasal-groups, both A and schwa. */
    if (nasal && n->same_syllable_left && (vowel(n->left_phone) ||
        (i>1 && n->left_phone==RELOQ_DE_PHONE_r && p[i-1].same_syllable_left && vowel(p[i-1].left_phone))) &&
        n->same_word_right && n->same_syllable_right && !r->affricate_role &&
        (next==RELOQ_DE_PHONE_f || next==RELOQ_DE_PHONE_s ||
         next==RELOQ_DE_PHONE_S || next==RELOQ_DE_PHONE_x || next==RELOQ_DE_PHONE_X ||
         next==RELOQ_DE_PHONE_l || next==RELOQ_DE_PHONE_r ||
         next==RELOQ_DE_PHONE_m || next==RELOQ_DE_PHONE_n || next==RELOQ_DE_PHONE_G)) {
        const int liquid=next==RELOQ_DE_PHONE_l || next==RELOQ_DE_PHONE_r;
        const int nas=next==RELOQ_DE_PHONE_m || next==RELOQ_DE_PHONE_n || next==RELOQ_DE_PHONE_G;
        int32_t body,contact;
        if (n->phone==RELOQ_DE_PHONE_G) body=42;
        else if (n->phone==RELOQ_DE_PHONE_m) body=nas?55:liquid?85:80;
        else if (next==RELOQ_DE_PHONE_r) body=n->stress?70:40;
        else if (next==RELOQ_DE_PHONE_l) body=pause_after_next?100:80;
        else if (pause_after_next) body=90;
        else if (next==RELOQ_DE_PHONE_f) body=55;
        else body=n->stress?(next==RELOQ_DE_PHONE_n?60:50):42;
        if (reloq_german_transition_cc_ticks(n,r,&contact)!=1) return 0;
        *out=(struct reloq_german_coda_timing){body,contact};return 1;
    }
    /* An affricate is two acoustic gestures, but its closure still has
     * the original nasal contact. Expanding it must not send the nasal
     * through a generic duration or remove its 1/30-ms transition. */
    if (nasal && n->same_syllable_left && vowel(n->left_phone) &&
        n->same_word_right && n->same_syllable_right && (r->affricate_role&1)) {
        const int labial=r->affricate_role==1;
        const int word_end=i+2<count && !p[i+2].same_word_right;
        const int body=n->phone==RELOQ_DE_PHONE_G?42:
            n->phone==RELOQ_DE_PHONE_m?(labial?(word_end?85:60):80):
            labial?(n->stress?50:42):n->stress==1?45:60;
        *out=(struct reloq_german_coda_timing){body,
            n->phone==RELOQ_DE_PHONE_G && !labial?30:1};return 1;
    }
    if (nasal && n->same_syllable_left && vowel(n->left_phone) &&
        n->same_word_right && !n->same_syllable_right && (r->affricate_role&1)) {
        const int schwa=n->left_phone==RELOQ_DE_PHONE_AT;
        const int labial=r->affricate_role==1,voiced=r->affricate_role==7;
        const int body=n->phone==RELOQ_DE_PHONE_G?42:
            n->phone==RELOQ_DE_PHONE_m?(schwa?52:voiced?85:labial?60:80):
            schwa?72:voiced?85:labial?(n->stress?70:42):60;
        *out=(struct reloq_german_coda_timing){body,
            n->phone==RELOQ_DE_PHONE_G && !labial?30:1};return 1;
    }
    /* Word edges keep the nasal gesture; the following articulation and
     * reduced vowel select its duration before the CC contact is added. */
    if (nasal && !n->same_word_right && n->same_syllable_left &&
        vowel(n->left_phone) && !vowel(next)) {
        const int schwa=n->left_phone==RELOQ_DE_PHONE_AT;
        int32_t body=0,contact=0;
        if (n->phone==RELOQ_DE_PHONE_G) body=81;
        else if (n->phone==RELOQ_DE_PHONE_m) {
            if (next==RELOQ_DE_PHONE_m || next==RELOQ_DE_PHONE_n || next==RELOQ_DE_PHONE_G) body=55;
            else if (n->left_phone==RELOQ_DE_PHONE_R) body=105;
            else if (next==RELOQ_DE_PHONE_k) body=schwa?85:110;
            else if (schwa) body=100;
            else if (next==RELOQ_DE_PHONE_p) body=60;
            else if (next==RELOQ_DE_PHONE_t) body=r->stress==1?85:75;
            else if (next==RELOQ_DE_PHONE_b) body=r->stress==1?85:70;
            else if (next==RELOQ_DE_PHONE_d || next==RELOQ_DE_PHONE_g) body=r->stress==1?85:90;
            else if (fricative(next) || next==RELOQ_DE_PHONE_l || next==RELOQ_DE_PHONE_r) body=90;
        } else {
            if (schwa) body=next==RELOQ_DE_PHONE_t || next==RELOQ_DE_PHONE_d?60:
                next==RELOQ_DE_PHONE_p || next==RELOQ_DE_PHONE_b ||
                next==RELOQ_DE_PHONE_f || next==RELOQ_DE_PHONE_v || next==RELOQ_DE_PHONE_m?105:77;
            else if (next==RELOQ_DE_PHONE_t) body=n->weak_word_control?45:80;
            else if (next==RELOQ_DE_PHONE_d) body=n->stress==1?85:50;
            else if (next==RELOQ_DE_PHONE_f) body=55;
            else if (next==RELOQ_DE_PHONE_l || next==RELOQ_DE_PHONE_r)
                body=n->left_phone==RELOQ_DE_PHONE_R?85:100;
            else if (next==RELOQ_DE_PHONE_n && n->stress) body=60;
            else body=n->stress?70:42;
        }
        if (body && reloq_german_transition_cc_ticks(n,r,&contact)==1) {
            *out=(struct reloq_german_coda_timing){body,contact};return 1;
        }
    }
    const int before_lateral_stop=i+2<count && next==RELOQ_DE_PHONE_d &&
        r->same_word_right && !r->same_syllable_right &&
        p[i+2].phone==RELOQ_DE_PHONE_l;
    if (nasal && n->same_word_right && (!n->same_syllable_right || before_lateral_stop) &&
        n->same_syllable_left && vowel(n->left_phone) &&
        (next==RELOQ_DE_PHONE_b || next==RELOQ_DE_PHONE_d || next==RELOQ_DE_PHONE_g)) {
        const int schwa=n->left_phone==RELOQ_DE_PHONE_AT;
        int32_t body=n->phone==RELOQ_DE_PHONE_G?42:
            n->phone==RELOQ_DE_PHONE_m?schwa?52:next==RELOQ_DE_PHONE_b?70:85:
            schwa?72:next==RELOQ_DE_PHONE_d?
                (i+2<count && p[i+2].phone==RELOQ_DE_PHONE_AT &&
                 r->same_word_right && r->same_syllable_right?70:n->stress==1?85:50):
                n->stress?70:42;
        int32_t contact;
        if (reloq_german_transition_cc_ticks(n,r,&contact)==1) {
            *out=(struct reloq_german_coda_timing){body,contact};return 1;
        }
    }
    if (nasal && n->same_syllable_left && vowel(n->left_phone) &&
        n->same_word_right && !n->same_syllable_right &&
        (fricative(next) || next==RELOQ_DE_PHONE_r)) {
        const int schwa=n->left_phone==RELOQ_DE_PHONE_AT;
        int32_t body=n->phone==RELOQ_DE_PHONE_G?42:
            n->phone==RELOQ_DE_PHONE_m?schwa?52:
                next==RELOQ_DE_PHONE_v||next==RELOQ_DE_PHONE_z||next==RELOQ_DE_PHONE_r?85:80:
            schwa?72:next==RELOQ_DE_PHONE_f?55:
                next==RELOQ_DE_PHONE_r?100:n->stress?70:42;
        int32_t contact;
        if (reloq_german_transition_cc_ticks(n,r,&contact)!=1) return 0;
        *out=(struct reloq_german_coda_timing){body,contact};return 1;
    }
    if (nasal && n->same_syllable_left && vowel(n->left_phone) &&
        n->same_word_right && !n->same_syllable_right &&
        (next==RELOQ_DE_PHONE_m || next==RELOQ_DE_PHONE_n ||
         next==RELOQ_DE_PHONE_G || next==RELOQ_DE_PHONE_l)) {
        const int lateral=next==RELOQ_DE_PHONE_l,schwa=n->left_phone==RELOQ_DE_PHONE_AT;
        int32_t body=n->phone==RELOQ_DE_PHONE_m?(lateral?(schwa?52:85):55):
            n->phone==RELOQ_DE_PHONE_G?42:schwa?72:lateral?80:
            n->stress?(next==RELOQ_DE_PHONE_n?60:70):42;
        int32_t contact;
        if (reloq_german_transition_cc_ticks(n,r,&contact)!=1) return 0;
        *out=(struct reloq_german_coda_timing){body,contact};return 1;
    }
    if ((nasal && (!i || !n->same_syllable_left || !vowel(p[i-1].phone))) ||
        n->affricate_role || r->affricate_role ||
        (next!=RELOQ_DE_PHONE_p && next!=RELOQ_DE_PHONE_t && next!=RELOQ_DE_PHONE_k))
        return 0;
    const int cluster=n->same_syllable_right;
    const int word=!n->same_word_right;
    const int final_pair=cluster && pause_after_next;
    const int following_h=cluster && i+2<count && p[i+2].same_word_left &&
        !p[i+2].same_syllable_left &&
        (p[i+2].phone==RELOQ_DE_PHONE_h || vowel(p[i+2].phone));
    const int schwa=i && p[i-1].phone==RELOQ_DE_PHONE_AT;
    const int stressed=n->stress!=0;
    int32_t body,transition=1;
    switch (n->phone) {
    case RELOQ_DE_PHONE_m:
        if (schwa && next==RELOQ_DE_PHONE_k) body=85;
        else if (cluster)
            body=next==RELOQ_DE_PHONE_p?60:!r->same_word_right?90:80;
        else if (schwa) body=word?100:52;
        else if (next==RELOQ_DE_PHONE_p) body=60;
        else if (word && next==RELOQ_DE_PHONE_k) body=110;
        else if (r->stress==1) body=85;
        else body=word?75:80;
        break;
    case RELOQ_DE_PHONE_n:
        if (final_pair) body=90;
        else if (cluster) body=next==RELOQ_DE_PHONE_t?
            (n->stress==1 || n->weak_word_control?45:following_h?40:60):stressed?50:42;
        else if (schwa) body=!word?72:next==RELOQ_DE_PHONE_t?60:
            next==RELOQ_DE_PHONE_k?77:105;
        else if (next==RELOQ_DE_PHONE_t) body=!word && r->stress==1?45:80;
        else body=stressed?70:42;
        break;
    case RELOQ_DE_PHONE_G:
        body=word?81:cluster && next==RELOQ_DE_PHONE_k && !following_h?(final_pair?85:62):42;
        if (next==RELOQ_DE_PHONE_t) transition=30;
        break;
    case RELOQ_DE_PHONE_f:
        body=final_pair?(n->stress==1?150:110):following_h?(p[i+2].stress?118:95):cluster?60:
            word || r->stress!=0?118:95;
        break;
    case RELOQ_DE_PHONE_s:
        body=n->weak_word_control?55:word?(stressed?96:75):
            (cluster?stressed:r->stress!=0)?76:64;
        break;
    case RELOQ_DE_PHONE_S:
        body=word || next==RELOQ_DE_PHONE_k?110:80;
        break;
    case RELOQ_DE_PHONE_x: body=60;break;
    case RELOQ_DE_PHONE_X:
        body=final_pair?(n->weak_word_control?110:145):
            cluster && !r->same_word_right && next==RELOQ_DE_PHONE_t?75:
            word || stressed?90:80;
        break;
    default:return 0;
    }
    *out=(struct reloq_german_coda_timing){body,transition};
    return 1;
}
