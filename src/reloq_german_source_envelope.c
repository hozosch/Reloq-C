#include "reloq_german_source_envelope.h"
#include "reloq_german_pause.h"
#include "reloq_german_nucleus_acoustics.h"
#include "reloq_german_onset_cluster.h"
#include <limits.h>

typedef enum reloq_german_phone Phone;
/* User-selected long-oe+r level; the tract and other r contexts are retained. */
enum { LONG_OE_R_ATTENUATION_DB = 8 };
static int vowel(Phone p)
{
    return (p>=RELOQ_DE_PHONE_a && p<=RELOQ_DE_PHONE_AT) ||
        p==RELOQ_DE_PHONE_R || p>=RELOQ_DE_PHONE_Q_E_COLON;
}
static int nasal(Phone p)
{
    return p==RELOQ_DE_PHONE_m || p==RELOQ_DE_PHONE_n || p==RELOQ_DE_PHONE_G;
}
static int stop(Phone p)
{
    return p==RELOQ_DE_PHONE_p || p==RELOQ_DE_PHONE_t || p==RELOQ_DE_PHONE_k ||
        p==RELOQ_DE_PHONE_b || p==RELOQ_DE_PHONE_d || p==RELOQ_DE_PHONE_g;
}
static int coda_liquid(const struct reloq_german_phone_node *n)
{
    return (n->phone==RELOQ_DE_PHONE_r || n->phone==RELOQ_DE_PHONE_l) &&
        n->same_syllable_left && vowel(n->left_phone);
}
static int coda_liquid_at(const struct reloq_german_phone_node *p,size_t i)
{
    return coda_liquid(p+i) || (i && p[i].phone==RELOQ_DE_PHONE_l &&
        p[i].same_syllable_left && p[i-1].phone==RELOQ_DE_PHONE_r &&
        coda_liquid(p+i-1));
}
static int fricative(Phone p)
{
    return p==RELOQ_DE_PHONE_f || p==RELOQ_DE_PHONE_v ||
        p==RELOQ_DE_PHONE_s || p==RELOQ_DE_PHONE_z ||
        p==RELOQ_DE_PHONE_S || p==RELOQ_DE_PHONE_Z ||
        p==RELOQ_DE_PHONE_x || p==RELOQ_DE_PHONE_X;
}
static int liquid(Phone p)
{
    return p==RELOQ_DE_PHONE_l || p==RELOQ_DE_PHONE_r ||
        p==RELOQ_DE_PHONE_R || p==RELOQ_DE_PHONE_j || p==RELOQ_DE_PHONE_w;
}
static int front_vowel(Phone p)
{
    return p==RELOQ_DE_PHONE_E || p==RELOQ_DE_PHONE_I ||
        p==RELOQ_DE_PHONE_e || p==RELOQ_DE_PHONE_i ||
        p==RELOQ_DE_PHONE_y || p==RELOQ_DE_PHONE_Y ||
        p==RELOQ_DE_PHONE_Q_oe || p==RELOQ_DE_PHONE_Q_OE ||
        p==RELOQ_DE_PHONE_Q_E_COLON || p==RELOQ_DE_PHONE_Q_E_TILDE;
}

/* Postvocalic n/ng before another consonant. These are the independently
 * described ger_ph_n/ger_ph_G excitation decisions, not a gain adjustment.
 * Zero means no local AV target: neighboring sources retain interpolation.
 * A negative result leaves an unaudited context with the existing planner. */
static int nasal_cluster_av(const struct reloq_german_phone_node *n,
    unsigned next_stress,unsigned next_affricate,Phone after)
{
    if (n->phone==RELOQ_DE_PHONE_n && n->same_word_right &&
        n->right_phone==RELOQ_DE_PHONE_g &&
        (after==RELOQ_DE_PHONE_n || after==RELOQ_DE_PHONE_m))
        return n->stress?48:46;
    if (n->phone==RELOQ_DE_PHONE_m) {
        const Phone r=n->right_phone;
        if (r==RELOQ_DE_PHONE_l) return 0;
        if (r==RELOQ_DE_PHONE_r || nasal(r)) return next_stress==1?51:50;
        if (r==RELOQ_DE_PHONE_b || r==RELOQ_DE_PHONE_d || r==RELOQ_DE_PHONE_g ||
            r==RELOQ_DE_PHONE_v || r==RELOQ_DE_PHONE_z)
            return n->stress?(next_stress==1?51:50):45;
        return stop(r) || fricative(r)?45:-1;
    }
    if (n->phone!=RELOQ_DE_PHONE_n && n->phone!=RELOQ_DE_PHONE_G) return -1;
    const int velar=n->phone==RELOQ_DE_PHONE_G;
    /* The voiced affricate owns a closure and frication interval, but
     * retains its lexical conditioning of the preceding nasal. Treating
     * its expanded /d/ as a plain stop removes the nasal's AV anchors and
     * lets the closure's falling source reach backwards into its release. */
    const Phone right=next_affricate==7?RELOQ_DE_PHONE_J:n->right_phone;
    int av;
    switch (right) {
    case RELOQ_DE_PHONE_l: case RELOQ_DE_PHONE_j: case RELOQ_DE_PHONE_w:
        return 0;
    case RELOQ_DE_PHONE_b: case RELOQ_DE_PHONE_d: case RELOQ_DE_PHONE_g:
        return velar?42:0;
    case RELOQ_DE_PHONE_p: case RELOQ_DE_PHONE_t: case RELOQ_DE_PHONE_k:
    case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_S:
    case RELOQ_DE_PHONE_Z: case RELOQ_DE_PHONE_X: case RELOQ_DE_PHONE_x:
    case RELOQ_DE_PHONE_P: case RELOQ_DE_PHONE_T: case RELOQ_DE_PHONE_C:
        av=velar?42:45;break;
    case RELOQ_DE_PHONE_J: av=velar?42:48;break;
    case RELOQ_DE_PHONE_v: case RELOQ_DE_PHONE_z: case RELOQ_DE_PHONE_m:
    case RELOQ_DE_PHONE_n: case RELOQ_DE_PHONE_G: case RELOQ_DE_PHONE_r:
        av=velar?45:48;break;
    default:return -1;
    }
    return av-(!velar && !n->stress?2:0);
}

enum reloq_german_source_kind reloq_german_source_classify(
    const struct reloq_german_phone_node *p, size_t count, size_t i,
    int nucleus, int phrase_final)
{
    if (!p || i>=count) return RELOQ_SOURCE_NONE;
    const struct reloq_german_phone_node *n=p+i;
    if (i && n->same_syllable_left && liquid(n->phone) &&
        (vowel(p[i-1].phone) || (n->phone==RELOQ_DE_PHONE_l &&
         p[i-1].phone==RELOQ_DE_PHONE_r && coda_liquid_at(p,i-1))))
        return RELOQ_SOURCE_CARRY;
    if (nucleus) return RELOQ_SOURCE_NUCLEUS;
    /* Word-internal Z retains its voiced source before consonants as well
     * as vowels. The word-final phonetic class has no AV plateau. */
    if (n->phone==RELOQ_DE_PHONE_Z && n->same_word_right)
        return RELOQ_SOURCE_VOICED_NOISE;
    const int next_vowel=(n->same_syllable_right ||
        nasal(n->phone)) && (vowel(n->right_phone) ||
        (n->phone==RELOQ_DE_PHONE_r && n->right_phone==RELOQ_DE_PHONE_j));
    const int previous_vowel=i && n->same_word_left && vowel(p[i-1].phone);
    if (nasal(n->phone)) {
        if (phrase_final && (previous_vowel || (i && n->same_word_left &&
            (coda_liquid_at(p,i-1) || (n->same_syllable_left &&
             (stop(p[i-1].phone) || fricative(p[i-1].phone))))))) return RELOQ_SOURCE_NASAL_FINAL;
        /* A word boundary alone does not restart nasal voicing between
         * vowels. The phrase composer resets this bridge after a pause. */
        if (next_vowel && i && (vowel(p[i-1].phone) || coda_liquid_at(p,i-1)))
            return RELOQ_SOURCE_NASAL_BRIDGE;
        if (next_vowel && i && !n->same_syllable_left &&
            (fricative(p[i-1].phone) || nasal(p[i-1].phone) || stop(p[i-1].phone)))
            return RELOQ_SOURCE_NASAL_ONSET;
        if (next_vowel && !n->same_word_left) return RELOQ_SOURCE_NASAL_ONSET;
        if ((previous_vowel || (i && n->same_syllable_left &&
            coda_liquid_at(p,i-1))) && nasal_cluster_av(n,i+1<count?p[i+1].stress:0,
                i+1<count?p[i+1].affricate_role:0,
                i+1<count&&p[i+1].same_syllable_right?p[i+1].right_phone:RELOQ_DE_PHONE_NONE)>=0)
            return RELOQ_SOURCE_NASAL_CLUSTER;
    }
    if (next_vowel) {
        if (n->phone==RELOQ_DE_PHONE_l || n->phone==RELOQ_DE_PHONE_w ||
            n->phone==RELOQ_DE_PHONE_j) return RELOQ_SOURCE_LIQUID_ONSET;
        if (n->phone==RELOQ_DE_PHONE_z && n->same_word_left &&
            !n->same_syllable_left && fricative(n->left_phone))
            return RELOQ_SOURCE_UNVOICED;
        if (n->phone==RELOQ_DE_PHONE_v || n->phone==RELOQ_DE_PHONE_z)
            return RELOQ_SOURCE_PARTIAL_VOICE;
        if (n->phone==RELOQ_DE_PHONE_Z) return RELOQ_SOURCE_VOICED_NOISE;
        if (n->phone==RELOQ_DE_PHONE_r) {
            const int32_t ticks=reloq_german_source_consonant_ticks(p,count,i);
            if (ticks>=65) return RELOQ_SOURCE_R_INITIAL;
            if (ticks) return RELOQ_SOURCE_R_MEDIAL;
        }
    }
    return RELOQ_SOURCE_NONE;
}

int32_t reloq_german_source_consonant_ticks(
    const struct reloq_german_phone_node *p, size_t count, size_t i)
{
    if (!p || i>=count) return 0;
    const struct reloq_german_phone_node *n=p+i;
    /* A nasal closing an obstruent group is not a fresh CV onset and
     * cannot inherit the old shared 68-tick consonant fallback. The
     * language's final m/n/ng duration branches keep separate bodies;
     * only n distinguishes an unaccented syllable here. The pause still
     * owns its existing release. See behavior-0.2.76/terminal-nasal. */
    if (nasal(n->phone) && n->right_phone==RELOQ_DE_PHONE_NONE &&
        n->same_syllable_left && (stop(n->left_phone) || fricative(n->left_phone)))
        return n->phone==RELOQ_DE_PHONE_m?120:
            n->phone==RELOQ_DE_PHONE_G?140:n->stress?40:110;
    if (n->phone==RELOQ_DE_PHONE_w && n->same_syllable_right &&
        vowel(n->right_phone) &&
        !(n->same_syllable_left && vowel(n->left_phone))) {
        /* A connected word-onset w has a short oral hold. Within an
         * onset, stops/nasals and fricatives keep distinct glide timing. */
        if (!n->same_word_left && n->left_phone!=RELOQ_DE_PHONE_NONE) return 10;
        return n->same_syllable_left &&
            (stop(n->left_phone) || nasal(n->left_phone))?70:40;
    }
    /* Also applies after an expanded affricate, where the preceding
     * frication owns a separate gesture but w keeps its cluster duration. */
    if (n->phone==RELOQ_DE_PHONE_v && n->same_syllable_left &&
        n->same_syllable_right && vowel(n->right_phone) &&
        (fricative(n->left_phone) || stop(n->left_phone)))
        return fricative(n->left_phone) || n->left_phone==RELOQ_DE_PHONE_t?46:62;
    if (nasal(n->phone) && !n->same_word_right && n->same_syllable_left &&
        vowel(n->left_phone) && vowel(n->right_phone))
        return n->phone==RELOQ_DE_PHONE_G?81:
            n->phone==RELOQ_DE_PHONE_n?(n->weak_word_control?40:50):
            n->left_phone==RELOQ_DE_PHONE_R?105:
            n->left_phone==RELOQ_DE_PHONE_AT?72:n->stress?70:50;
    if (!n->same_syllable_right || (!vowel(n->right_phone) &&
        !(n->phone==RELOQ_DE_PHONE_r && n->right_phone==RELOQ_DE_PHONE_j)) ||
        n->same_syllable_left) return 0;
    /* A stop at the end of a consonant group shortens the following f
     * independently of its accent. Vocalic r remains a vowel context;
     * syllable-final l still participates in the consonant group. */
    if (i>=2 && n->phone==RELOQ_DE_PHONE_f &&
        (p[i-1].phone==RELOQ_DE_PHONE_p || p[i-1].phone==RELOQ_DE_PHONE_t ||
         p[i-1].phone==RELOQ_DE_PHONE_k) && p[i-1].same_syllable_left &&
        !vowel(p[i-2].phone) &&
        !(p[i-2].phone==RELOQ_DE_PHONE_r && coda_liquid(p+i-2))) return 95;
    const int internal=n->same_word_left;
    if(i && n->phone==RELOQ_DE_PHONE_z && !internal) {
        Phone left=p[i-1].phone;
        if(stop(left)) {
            if(i>=2 && p[i-1].same_syllable_left) {
                if(nasal(p[i-2].phone))return 85;
                if(fricative(p[i-2].phone))return 52;
            }
            return 72;
        }
        if(fricative(left))return left==RELOQ_DE_PHONE_s || left==RELOQ_DE_PHONE_z?70:78;
        if(nasal(left) || coda_liquid_at(p,i-1))return 78;
    }
    if (internal && i && n->phone==RELOQ_DE_PHONE_f && nasal(p[i-1].phone))
        return !p[i-1].stress && n->stress==1?92:105;
    if (internal && i && n->phone==RELOQ_DE_PHONE_S &&
        (stop(p[i-1].phone) || nasal(p[i-1].phone))) return 110;
    if (internal && i && (n->phone==RELOQ_DE_PHONE_v || n->phone==RELOQ_DE_PHONE_z) &&
        (stop(p[i-1].phone) || fricative(p[i-1].phone) || nasal(p[i-1].phone) ||
         coda_liquid_at(p,i-1))) {
        const Phone left=p[i-1].phone;
        if (n->phone==RELOQ_DE_PHONE_z)
            return left==RELOQ_DE_PHONE_s || left==RELOQ_DE_PHONE_z?70:68;
        return fricative(left) || left==RELOQ_DE_PHONE_t || left==RELOQ_DE_PHONE_d?46:62;
    }
    if (i && nasal(n->phone) && (fricative(p[i-1].phone) || nasal(p[i-1].phone) ||
        stop(p[i-1].phone) || p[i-1].phone==RELOQ_DE_PHONE_l)) {
        const Phone left=p[i-1].phone;
        if (n->phone==RELOQ_DE_PHONE_G) return 42;
        if (n->phone==RELOQ_DE_PHONE_n)
            return left==RELOQ_DE_PHONE_l?(n->stress==1?70:40):nasal(left)?52:70;
        if (!n->stress) return 60;
        return !nasal(left)?64:internal || left==RELOQ_DE_PHONE_m?70:85;
    }
    /* Syllable-onset liquids keep their own duration and source after a
     * consonant. The previous syllable's stress also selects r's gesture. */
    if (n->phone==RELOQ_DE_PHONE_r) {
        if (!i) return 75;
        const Phone left=p[i-1].phone;
        const int diph=left==RELOQ_DE_PHONE_Q_aj || left==RELOQ_DE_PHONE_Q_aw ||
            left==RELOQ_DE_PHONE_Q_oj;
        if (vowel(left) && !diph && p[i-1].stress==1 && !n->stress) return 55;
        return (!p[i-1].stress && n->stress==1) || front_vowel(n->right_phone)?65:44;
    }
    if (i && n->phone==RELOQ_DE_PHONE_l && !vowel(p[i-1].phone)) {
        switch (p[i-1].phone) {
        case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_z: return 68;
        case RELOQ_DE_PHONE_S: return 34;
        case RELOQ_DE_PHONE_p: case RELOQ_DE_PHONE_t: case RELOQ_DE_PHONE_k:
        case RELOQ_DE_PHONE_b: case RELOQ_DE_PHONE_d: case RELOQ_DE_PHONE_g:
            return n->stress?34:65;
        case RELOQ_DE_PHONE_m: case RELOQ_DE_PHONE_n: case RELOQ_DE_PHONE_G:
        case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_v: case RELOQ_DE_PHONE_X:
        case RELOQ_DE_PHONE_x: case RELOQ_DE_PHONE_l: return n->stress?53:42;
        case RELOQ_DE_PHONE_r: return internal?76:60;
        default:break;
        }
    }
    if (internal && (!i || (!vowel(p[i-1].phone) && !coda_liquid_at(p,i-1)))) return 0;
    switch (n->phone) {
    case RELOQ_DE_PHONE_v: return internal?62:n->stress==1?75:48;
    case RELOQ_DE_PHONE_z: {
        const Phone r=n->right_phone;
        const int back=r==RELOQ_DE_PHONE_O || r==RELOQ_DE_PHONE_U ||
            r==RELOQ_DE_PHONE_o || r==RELOQ_DE_PHONE_u || r==RELOQ_DE_PHONE_R ||
            r==RELOQ_DE_PHONE_Q_oj || r==RELOQ_DE_PHONE_Q_o_TILDE ||
            r==RELOQ_DE_PHONE_Q_oe_TILDE;
        return internal?68:n->stress==1?90:back?62:78;
    }
    case RELOQ_DE_PHONE_Z: return 99;
    case RELOQ_DE_PHONE_m:
        return n->stress?85:internal && n->right_phone==RELOQ_DE_PHONE_AT &&
            i+1<count && p[i+1].same_syllable_right &&
            (p[i+1].right_phone==RELOQ_DE_PHONE_n ||
             p[i+1].right_phone==RELOQ_DE_PHONE_l)?68:60;
    case RELOQ_DE_PHONE_n: return n->stress==1?70:
        internal && p[i-1].stress==1 && !n->stress?30:40;
    case RELOQ_DE_PHONE_G: return internal && p[i-1].stress==1 && !n->stress?60:42;
    case RELOQ_DE_PHONE_l:
        if (i && (p[i-1].phone==RELOQ_DE_PHONE_Q_aj || p[i-1].phone==RELOQ_DE_PHONE_Q_oj))
            return 48;
        /* The reduced closed -l@n onset has its own short interval.
         * Diphthong-owned l above keeps its separate 48-tick branch. */
        if (internal && !n->stress && n->right_phone==RELOQ_DE_PHONE_AT &&
            i+2<count && p[i+1].same_syllable_right &&
            p[i+1].right_phone==RELOQ_DE_PHONE_n) return 32;
        return !internal?60:p[i-1].phone==RELOQ_DE_PHONE_AT?68:
        n->stress || p[i-1].phone==RELOQ_DE_PHONE_Q_aw?76:
        50;
    case RELOQ_DE_PHONE_w: return 40;
    case RELOQ_DE_PHONE_j: return internal?20:45;
    default: return 0;
    }
}

int reloq_german_source_release_ticks(Phone ph,enum reloq_german_source_kind kind)
{
    if (kind==RELOQ_SOURCE_NASAL_FINAL) return ph==RELOQ_DE_PHONE_m?15:30;
    if (kind==RELOQ_SOURCE_CARRY) return ph==RELOQ_DE_PHONE_w?40:30;
    if (kind==RELOQ_SOURCE_NUCLEUS) return ph==RELOQ_DE_PHONE_Q_aw?40:
        ph==RELOQ_DE_PHONE_Q_aj || ph==RELOQ_DE_PHONE_Q_oj?30:60;
    return 0;
}

/* Nasal anti-resonance is a sparse trajectory. The original contributes
 * one midpoint per nasal, not a nasal plateau with independent edge resets.
 * Neighboring oral cores and pauses supply their own targets; interpolation
 * consequently spans VC, CC, word boundaries and consecutive nasals. */
int reloq_german_resonance_append(enum reloq_german_phone phone,
    int64_t begin,int64_t end,int32_t oral_fnz,size_t index,
    reloq_german_source_emit emit,void *context)
{
    if (!emit || begin<0 || end<begin || oral_fnz<0 ||
        phone<=RELOQ_DE_PHONE_NONE || phone>RELOQ_DE_PHONE_Q_oe_TILDE) return -1;
    if (nasal(phone))
        return emit(context,RELOQ_TARGET_FNZ,begin+(end-begin)/2,
            phone==RELOQ_DE_PHONE_G?400:350,index);
    return emit(context,RELOQ_TARGET_FNZ,begin,oral_fnz,index) ||
           emit(context,RELOQ_TARGET_FNZ,end,oral_fnz,index)?-1:0;
}

struct output { reloq_german_source_emit emit; void *context; size_t index; };
int reloq_german_source_owns_tilt(enum reloq_german_source_kind kind)
{
    return (kind>=RELOQ_SOURCE_NUCLEUS && kind<=RELOQ_SOURCE_VOICED_NOISE) ||
        kind==RELOQ_SOURCE_NASAL_CLUSTER || kind==RELOQ_SOURCE_R_INITIAL ||
        kind==RELOQ_SOURCE_R_MEDIAL;
}

/* Spectral tilt has the source's boundaries, including a nucleus carried
 * through vocalic r. Tract knots inside those intervals cannot introduce
 * a new source ramp. Equal-tick targets retain the outgoing then incoming
 * owner, just like AV/AH. Model controls are applied by the caller. */
int reloq_german_source_tilt_append(
    const struct reloq_german_phone_node *p,
    const struct reloq_german_source_segment *s,size_t count,
    reloq_german_source_emit emit,void *context)
{
    if((!p&&count)||(!s&&count)||!emit)return -1;
    for(size_t i=0;i<count;++i) {
        if(!reloq_german_source_owns_tilt(s[i].kind) || s[i].kind==RELOQ_SOURCE_CARRY || p[i].phone==RELOQ_DE_PHONE_z)continue;
        size_t last=i;
        if(s[i].kind==RELOQ_SOURCE_NUCLEUS)
            while(last+1<count && s[last+1].kind==RELOQ_SOURCE_CARRY &&
                  p[last+1].same_syllable_left)++last;
        int64_t begin=s[i].begin+(s[i].kind==RELOQ_SOURCE_NUCLEUS);
        const int tilt=0;
        if(emit(context,RELOQ_TARGET_TL,begin,tilt,i) ||
           emit(context,RELOQ_TARGET_TL,s[last].end,tilt,i))return -1;
    }
    return 0;
}

static int point(struct output *o, enum reloq_target_stream s, int64_t t, int v)
{
    return o->emit(o->context,s,t,v,o->index);
}
static int pair(struct output *o,int64_t t,int av,int ah)
{
    return point(o,RELOQ_TARGET_AV,t,av) || point(o,RELOQ_TARGET_AH,t,ah);
}
static int plateau(struct output *o,int64_t a,int64_t b,int av,int ah)
{
    return pair(o,a,av,ah) || pair(o,b,av,ah);
}

int reloq_german_source_envelope_append(
    const struct reloq_german_phone_node *p,
    const struct reloq_german_source_segment *s,size_t count,
    reloq_german_source_emit emit,void *context)
{
    if ((!p && count) || (!s && count) || !emit) return -1;
    struct output out={emit,context,0};
    for (size_t i=0;i<count;++i) {
        out.index=i;
        const int64_t a=s[i].begin,b=s[i].end;
        if (a<0 || b<=a || b>INT64_MAX-60 || (i && a<s[i-1].end)) return -1;
        if (s[i].kind==RELOQ_SOURCE_NONE) continue;
        const Phone ph=p[i].phone;
        switch (s[i].kind) {
        case RELOQ_SOURCE_NUCLEUS: {
            size_t last=i;
            while (last+1<count && s[last+1].kind==RELOQ_SOURCE_CARRY &&
                   p[last+1].same_syllable_left) ++last;
            if (pair(&out,a+1,s[i].av,34)) return -1;
            if (last>=i+1 && ph==RELOQ_DE_PHONE_Q_oe &&
                p[i+1].phone==RELOQ_DE_PHONE_r) {
                /* Preserve the selected r colour and 8 dB target level.
                 * Vocalic r shares the vowel's source: introduce its
                 * attenuation over the existing vowel-to-r contact, not
                 * as a second, eight-tick gesture after the r has arrived.
                 * The original source continues through this contact. */
                const size_t r=i+1;
                int64_t begin=s[r].begin;
                int64_t r_end=s[r].end-(last>r?s[r].outgoing_ticks:0);
                int64_t total=s[last].end-a-1;
                int64_t contact=begin-s[i].outgoing_ticks;
                if (contact<a+1) contact=a+1;
                if (contact>begin) contact=begin;
                int64_t elapsed=contact-a-1;
                int av=(int)((s[i].av*(total-elapsed)+s[i].av_end*elapsed+total/2)/total);
                /* Unmeasured zero-contact inputs still need a continuous
                 * one-tick entry rather than two different equal-tick
                 * targets. Ordinary vocalic-r contacts end at begin. */
                int64_t settled=begin>contact?begin:begin+1;
                if (settled>r_end) settled=r_end;
                elapsed=settled-a-1;
                int settled_av=(int)((s[i].av*(total-elapsed)+s[i].av_end*elapsed+total/2)/total);
                int end_av=s[i].av_end;
                if (last>r) {
                    elapsed=r_end-a-1;
                    end_av=(int)((s[i].av*(total-elapsed)+s[i].av_end*elapsed)/total);
                }
                if (pair(&out,contact,av,34)) return -1;
                if (last==r && s[r].phrase_final) {
                    /* With no following consonant, the short vocalic-r
                     * core remains part of the settling gesture. Ease
                     * the selected attenuation across contact and core,
                     * reaching its unchanged level at the release. */
                    static const int attenuation[4]={1,4,7,8};
                    for (unsigned step=1;step<=4;++step) {
                        int64_t t=contact+(r_end-contact)*step/4;
                        elapsed=t-a-1;
                        int level=(int)((s[i].av*(total-elapsed)+s[i].av_end*elapsed+total/2)/total);
                        if (pair(&out,t,level-attenuation[step-1],
                                 34-attenuation[step-1])) return -1;
                    }
                } else if (pair(&out,settled,settled_av-LONG_OE_R_ATTENUATION_DB,
                         34-LONG_OE_R_ATTENUATION_DB) ||
                    pair(&out,r_end,end_av-LONG_OE_R_ATTENUATION_DB,
                         34-LONG_OE_R_ATTENUATION_DB)) return -1;
                if (last>r) {
                    /* Restore the continuing nucleus through r-to-l,
                     * keeping the user's attenuation confined to r. */
                    elapsed=s[r+1].begin-a-1;
                    av=(int)((s[i].av*(total-elapsed)+s[i].av_end*elapsed)/total);
                    if (pair(&out,s[r+1].begin,av,34) ||
                        pair(&out,s[last].end,s[i].av_end,34)) return -1;
                }
            } else if (pair(&out,s[last].end,s[i].av_end,34)) return -1;
            break;
        }
        case RELOQ_SOURCE_CARRY: break;
        case RELOQ_SOURCE_UNVOICED:
            if (plateau(&out,a,b,0,0)) return -1;
            if (p[i].affricate_role==2 && p[i].same_syllable_right &&
                i+1<count && (p[i].right_phone==RELOQ_DE_PHONE_l ||
                              p[i].right_phone==RELOQ_DE_PHONE_r)) {
                struct reloq_german_onset_cluster next;
                if (reloq_german_onset_cluster_plan(p,count,i+1,&next)==1 &&
                    plateau(&out,b,s[i+1].begin,0,54)) return -1;
            }
            break;
        case RELOQ_SOURCE_NASAL_ONSET: {
            int av=ph==RELOQ_DE_PHONE_m?(p[i].stress==1?51:50):
                   ph==RELOQ_DE_PHONE_n?48:49;
            if (plateau(&out,a,b,av,0)) return -1;
            break;
        }
        case RELOQ_SOURCE_NASAL_BRIDGE:
            /* There is no nasal AV plateau between the two vowels. */
            if (point(&out,RELOQ_TARGET_AH,a,0) ||
                point(&out,RELOQ_TARGET_AH,b,0)) return -1;
            break;
        case RELOQ_SOURCE_NASAL_CLUSTER: {
            const int av=nasal_cluster_av(p+i,i+1<count?p[i+1].stress:0,
                i+1<count?p[i+1].affricate_role:0,
                i+1<count&&p[i+1].same_syllable_right?p[i+1].right_phone:RELOQ_DE_PHONE_NONE);
            if (av<0) return -1;
            if (av>0 && (point(&out,RELOQ_TARGET_AV,a,av) ||
                         point(&out,RELOQ_TARGET_AV,b,av))) return -1;
            if (point(&out,RELOQ_TARGET_AH,a,0) ||
                point(&out,RELOQ_TARGET_AH,b,0)) return -1;
            break;
        }
        case RELOQ_SOURCE_NASAL_FINAL: {
            if(p[i].same_syllable_left && (stop(p[i].left_phone) || fricative(p[i].left_phone))) {
                int av=ph==RELOQ_DE_PHONE_m?50:ph==RELOQ_DE_PHONE_G?43:p[i].stress?48:46;
                if(plateau(&out,a,b,av,0))return -1;
                break;
            }
            const int weak=p[i].stress==0;
            if (ph==RELOQ_DE_PHONE_m) {
                int av=weak?43:45;
                if (p[i].left_phone==RELOQ_DE_PHONE_a || p[i].left_phone==RELOQ_DE_PHONE_A ||
                    p[i].left_phone==RELOQ_DE_PHONE_R) av-=3;
                if (plateau(&out,a,b,av,0)) return -1;
            } else if (point(&out,RELOQ_TARGET_AV,a+(b-a)*4/5,
                            ph==RELOQ_DE_PHONE_G || !weak?42:40) ||
                       point(&out,RELOQ_TARGET_AH,a,0) ||
                       point(&out,RELOQ_TARGET_AH,b,0)) return -1;
            break;
        }
        case RELOQ_SOURCE_LIQUID_ONSET: {
            int av=ph==RELOQ_DE_PHONE_l?46:ph==RELOQ_DE_PHONE_w?50:48;
            if(ph==RELOQ_DE_PHONE_j) {
                const Phone r=p[i].right_phone;
                if(r==RELOQ_DE_PHONE_AT)av=53;
                else if(r==RELOQ_DE_PHONE_i || r==RELOQ_DE_PHONE_I ||
                    r==RELOQ_DE_PHONE_u || r==RELOQ_DE_PHONE_U ||
                    r==RELOQ_DE_PHONE_y || r==RELOQ_DE_PHONE_Y ||
                    r==RELOQ_DE_PHONE_Q_oe || r==RELOQ_DE_PHONE_Q_OE)av=52;
            }
            const int lead=i && !s[i-1].phrase_final &&
                ((ph==RELOQ_DE_PHONE_l &&
                  (s[i-1].kind==RELOQ_SOURCE_UNVOICED || nasal(p[i-1].phone))) ||
                 (ph==RELOQ_DE_PHONE_w && p[i].same_word_left &&
                  !stop(p[i-1].phone) && !vowel(p[i-1].phone)));
            const int64_t begin=lead?s[i-1].end:a;
            if (plateau(&out,begin,b,av,34)) return -1;
            break;
        }
        case RELOQ_SOURCE_PARTIAL_VOICE: {
            const int fresh=i && s[i-1].phrase_final && !s[i-1].continuing_comma;
            int av=ph==RELOQ_DE_PHONE_v?30:!fresh && nasal(p[i].left_phone)?35:
                !fresh && stop(p[i].left_phone)?37:20;
            /* A single midpoint anchors voicing. Adding a zero onset or
             * another AV endpoint would change both neighboring ramps. */
            if (point(&out,RELOQ_TARGET_AV,a+(b-a)/2,av) ||
                point(&out,RELOQ_TARGET_AH,a,0) ||
                point(&out,RELOQ_TARGET_AH,b,0)) return -1;
            break;
        }
        case RELOQ_SOURCE_VOICED_NOISE:
            if (plateau(&out,a,b,40,0)) return -1;
            break;
        case RELOQ_SOURCE_R_INITIAL: {
            const int64_t first=(b-a)/5,second=first+((b-a)*3+5)/10,
                third=second+first+1;
            if (b-a<56 || plateau(&out,a,a+first,0,27) ||
                plateau(&out,a+first+1,a+second,0,48) ||
                plateau(&out,a+second+1,a+third,0,27) ||
                plateau(&out,a+third+1,b,40,52)) return -1;
            break;
        }
        case RELOQ_SOURCE_R_MEDIAL: {
            const int64_t first=(b-a)*35/100;
            const int attenuation=p[i].same_word_left &&
                p[i].left_phone==RELOQ_DE_PHONE_Q_oe?LONG_OE_R_ATTENUATION_DB:0;
            if (p[i].right_phone==RELOQ_DE_PHONE_U &&
                       vowel(p[i].left_phone) &&
                       p[i].left_phone!=RELOQ_DE_PHONE_Q_aj &&
                       p[i].left_phone!=RELOQ_DE_PHONE_Q_aw &&
                       p[i].left_phone!=RELOQ_DE_PHONE_Q_oj) {
                /* The short back nucleus selects continuous r voicing;
                 * its onset does not traverse the two constrictions used
                 * before other nuclei and after consonants/glides. */
                if (plateau(&out,a,b,40-attenuation,52-attenuation)) return -1;
            } else if (b-a<21 || plateau(&out,a,a+first,0,27-attenuation) ||
                       plateau(&out,a+first+1,b,27-attenuation,52-attenuation)) return -1;
            break;
        }
        default: return -1;
        }
        if (!s[i].phrase_final) continue;
        struct reloq_german_pause release;
        if (reloq_german_pause_plan(p+i,s[i].kind==RELOQ_SOURCE_CARRY,
                s[i].continuing_comma?RELOQ_DE_TERMINAL_COMMA:RELOQ_DE_TERMINAL_PERIOD,
                s[i].continuing_comma,&release)) return -1;
        if (ph==RELOQ_DE_PHONE_r && p[i].same_word_left &&
            p[i].left_phone==RELOQ_DE_PHONE_Q_oe && release.ah>0)
            release.ah=release.ah>LONG_OE_R_ATTENUATION_DB?
                release.ah-LONG_OE_R_ATTENUATION_DB:0;
        if (reloq_german_pause_release(&release,b,i,emit,context) ||
            pair(&out,b+release.release_ticks,0,0)) return -1;
    }
    return 0;
}
