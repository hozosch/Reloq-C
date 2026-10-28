#include "reloq_german_onset_cluster.h"
#include "reloq_german_liquid_tract.h"
#include "reloq_german_nasal_tract.h"
#include "reloq_german_stop_phase.h"
#include "reloq_german_nucleus_duration.h"
#include "reloq_german_generic_noise.h"
#include <limits.h>
#include <string.h>

typedef enum reloq_german_phone Phone;
#define P(x) RELOQ_DE_PHONE_##x
static int vowel(Phone p)
{return (p>=P(a) && p<=P(AT)) || p==P(R) || (p>=P(Q_E_COLON) && p<=P(Q_oe_TILDE));}
static int front(Phone p)
{return p==P(E)||p==P(I)||p==P(e)||p==P(i)||p==P(Q_E_COLON);}
static int closed(Phone p)
{return p==P(e)||p==P(i)||p==P(o)||p==P(u)||p==P(y)||p==P(Q_oe);}
static int stop(Phone p)
{return p==P(p)||p==P(t)||p==P(k)||p==P(b)||p==P(d)||p==P(g);}
static int unvoiced(Phone p) {return p==P(p)||p==P(t)||p==P(k);}
static int nasal(Phone p) {return p==P(m)||p==P(n)||p==P(G);}
static int fricative(Phone p)
{return p==P(f)||p==P(v)||p==P(s)||p==P(z)||p==P(S)||p==P(x)||p==P(X);}
static int head(Phone p)
{return stop(p)||p==P(f)||p==P(v)||p==P(s)||p==P(z)||p==P(S)||p==P(X);}
static int continued_r(const struct reloq_german_phone_node *p,size_t n,size_t i)
{
    if (i+2>=n || p[i+1].phone!=P(r) || vowel(p[i+2].phone)) return 0;
    for (size_t j=i+2;j<n;++j) {
        if (!p[j-1].same_word_right || !p[j-1].same_syllable_right) return 0;
        if (vowel(p[j].phone)) return 1;
        if (!stop(p[j].phone) && !fricative(p[j].phone) &&
            !nasal(p[j].phone) && p[j].phone!=P(l)) return 0;
    }
    return 0;
}
static int valid(const struct reloq_german_phone_node *p,size_t n,size_t i)
{
    const int lateral_boundary=i+2<n && p[i].phone==P(d) &&
        p[i+1].phone==P(l) && p[i].same_word_right;
    if (i+2>=n || !head(p[i].phone) || (p[i].affricate_role && p[i].affricate_role!=2) ||
        (!p[i].same_syllable_right && !lateral_boundary) || !p[i].same_word_right ||
        (p[i+1].phone!=P(r) && p[i+1].phone!=P(l)) ||
        !p[i+1].same_word_right || !p[i+1].same_syllable_right ||
        (!vowel(p[i+2].phone) && !continued_r(p,n,i))) return 0;
    return lateral_boundary || !p[i].same_word_left || vowel(p[i].left_phone) ||
        p[i].affricate_role==2 ||
        (!p[i].same_syllable_left &&
         (nasal(p[i].left_phone) || fricative(p[i].left_phone) ||
          stop(p[i].left_phone) || p[i].left_phone==P(l) || p[i].left_phone==P(r))) ||
        (p[i].same_syllable_left &&
         (p[i].left_phone==P(S) || p[i].left_phone==P(s)) && unvoiced(p[i].phone));
}

/* A stop-liquid onset following another syllable's consonant retains its
 * own closure and release. Left place/manner changes the closure, rather
 * than sending the complete group through the generic two-phone path. */
static void consonant_left_stop(Phone c,Phone left,int rhotic,int stress,
    struct reloq_german_onset_cluster *a)
{
    const Phone place=left==P(b)?P(p):left==P(d)?P(t):left==P(g)?P(k):left;
    switch (c) {
    case P(p):
        a->body_ticks=(left==P(S)?70:nasal(left)?(stress==1?69:50):(stress==1?95:80))+
            (rhotic?0:5);
        if (left==P(n)||left==P(l)||left==P(s)||left==P(z)||left==P(X)||place==P(t))
            a->tract.entering[1]=1500;
        break;
    case P(t):
        a->body_ticks=rhotic && left==P(n)?48:fricative(left)?52:
            place==P(p)?64:place==P(k)?84:69;
        if (place==P(k) && (stress==2 || (!rhotic && stress==1)))
            a->release_ticks=a->release_ticks*3/5;
        if (left==P(X)) {a->tract.entering[1]=1800;a->tract.entering[2]=2700;}
        break;
    case P(k):
        if (rhotic && stress && stop(left)) a->body_ticks=67;
        break;
    case P(b):
        a->body_ticks=place==P(p)?56:place==P(t)?61:
            rhotic?(left==P(r) && stress!=1?78:93):81;
        if (left==P(X)) a->tract.entering[1]=1500;
        else if (left==P(r)) a->tract.entering[1]=900;
        break;
    case P(d):
        a->body_ticks=rhotic?(place==P(t)?32:place==P(p)||place==P(k)?53:72):
            place==P(t)?28:place==P(p)||place==P(k)?73:
            nasal(left)||left==P(l)?45:100;
        if (left==P(l)) a->tract.entering[1]=1250;
        else if (left==P(X)) {a->tract.entering[1]=1800;a->tract.entering[2]=3000;}
        break;
    case P(g):
        if (rhotic && (left==P(f)||left==P(v))) a->body_ticks=50;
        if (left==P(l)) a->tract.entering[1]=1200;
        break;
    default:break;
    }
    if (!unvoiced(c)) a->closure_av=nasal(left)?35:
        fricative(left)||stop(left)||(c==P(d)&&left==P(l))?0:20;
    if (nasal(left) && !(c==P(p) && left==P(n)))
        for (unsigned j=0;j<3;++j) a->tract.entering[j]=-2;
}

/* Additional two-member onsets share the gesture/transition ownership of
 * liquid clusters. These branches describe measured phonetic contexts, not
 * word exceptions. A j source can begin before its oral gesture. */
static int extended_valid(const struct reloq_german_phone_node *p,size_t n,size_t h)
{
    if (h+2>=n || p[h].affricate_role || !p[h].same_word_right ||
        !p[h+1].same_word_right) return 0;
    Phone c=p[h].phone,l=p[h+1].phone;
    const int glide=l==P(j)||l==P(w), vocalic=vowel(p[h+2].phone);
    if ((!p[h].same_syllable_right && !(stop(c)&&glide)) ||
        (!p[h+1].same_syllable_right && (!glide||vocalic))) return 0;
    /* A glide can close the current syllable before another consonant.
     * Its source then has its own sustained voiced plateau. */
    if (stop(c) && l==P(w)) return 6;
    if (!vocalic && l==P(j) && (stop(c)||nasal(c)||c==P(f)||c==P(v)||c==P(z))) return 3;
    /* The k gesture is unchanged when the same onset continues s+t/p.
     * The following consonants keep their own planners. */
    if (c==P(k) && l==P(s) && !p[h].same_syllable_left &&
        (vowel(p[h+2].phone) || unvoiced(p[h+2].phone))) return 8;
    if (!vowel(p[h+2].phone)) return 0;
    /* /w/ is the rounded approximant in e.g. /twA/, distinct from the
     * voiced fricative /v/. Its oral hold and stop release have their own
     * ownership, just as in a stop+j onset. */
    if (c==P(g) && (l==P(n)||l==P(m)) && !p[h].same_syllable_left) return 7;
    if (c==P(k) && l==P(n) && !p[h].same_syllable_left) return 5;
    const int vocalic_left=h>=2 && (c==P(z)||stop(c)) && l==P(j) &&
        p[h-1].phone==P(r) && p[h-1].same_syllable_left &&
        vowel(p[h-2].phone);
    if (c==P(k) && l==P(v) && !p[h].same_syllable_left) return 4;
    if (c==P(k) && l==P(s) && !p[h].same_syllable_left) return 8;
    if (p[h].same_word_left && !vowel(p[h].left_phone) &&
        !vocalic_left && !(c==P(S) && (p[h].left_phone==P(s)||p[h].left_phone==P(S))) && !(c==P(p) && l==P(j) && p[h].left_phone==P(m))) return 0;
    if (l==P(v) && (stop(c) || c==P(f) || c==P(s) || c==P(S) ||
        c==P(x) || c==P(X))) return 4;
    if (c==P(S) && unvoiced(l)) return 1;
    if ((c==P(s)||c==P(S)||c==P(x)) && (l==P(m)||l==P(n))) return 2;
    if (l==P(j) && (stop(c)||c==P(m)||c==P(n)||c==P(f)||c==P(v)||c==P(z))) return 3;
    return 0;
}
static int extended_plan(const struct reloq_german_phone_node *p,size_t n,
    size_t i,struct reloq_german_onset_cluster *out)
{
    size_t h=i;int last=0,kind=extended_valid(p,n,h);
    if (!kind && i) {h=i-1;kind=extended_valid(p,n,h);last=1;}
    if (!kind) return 0;
    Phone c=p[h].phone,l=p[h+1].phone,v=p[h+2].phone;
    int s=p[h].stress,initial=!p[h].same_word_left;
    struct reloq_german_onset_cluster a={0};a.release_ah=-1;
    struct reloq_german_phone_node q=p[i];
    if(kind==3 && stop(c) && !p[h].same_syllable_right && !last) {
        q.same_syllable_left=0;q.same_syllable_right=1;
    }
    if (kind==6 && !last) {
        q.same_syllable_left=0;q.same_syllable_right=1;
    }
    if (kind==8) {
        if (last) {
            if (!vowel(v)) return 0;
            if (reloq_german_obstruent_tract_plan(&q,0,0,&a.tract)!=1) return 0;
            a.body_ticks=s==1?125:s==2?100:85;a.af=62;a.noise[4]=50;
        } else {
            q.right_phone=P(R);
            if (reloq_german_obstruent_tract_plan(&q,initial,0,&a.tract)!=1) return 0;
            a.body_ticks=initial?13:68;a.burst_ticks=8;a.release_ticks=10;
            a.af=60;a.noise[1]=a.noise[2]=60;
            a.tract.departing[0]=300;a.tract.departing[1]=initial?1650:1800;a.tract.departing[2]=2300;
            if (initial) memcpy(a.tract.entering,a.tract.departing,3*sizeof(int32_t));
            if (nasal(q.left_phone)) for(unsigned j=0;j<3;++j) a.tract.entering[j]=-2;
        }
    } else if (kind==7) {
        /* A g+nasal onset owns one voiced closure, its burst and a short
         * nasal contact. The following nasal has its own sustained oral
         * body; it is not a stop-to-vowel release with a nasal substituted.
         * ger_ph_g / ger_ph_g_dur / ger_ph_n, behavior-0.2.42. */
        if (last) {
            struct reloq_german_nasal_tract tract;
            if (reloq_german_nasal_tract_plan(&q,0,0,&tract)!=1) return 0;
            memcpy(a.tract.entering,tract.entering,sizeof tract.entering);
            memcpy(a.tract.departing,tract.departing,sizeof tract.departing);
            memcpy(a.tract.bandwidth,tract.bandwidth,sizeof tract.bandwidth);
            a.body_ticks=l==P(n)?40:v==P(AT)?68:s?64:60;
            a.av=l==P(n)?48:s==1?51:50;a.nasal_peak=350;
        } else {
            q.right_phone=P(R);
            if (reloq_german_obstruent_tract_plan(&q,initial,0,&a.tract)!=1) return 0;
            a.body_ticks=80;a.burst_ticks=10;a.release_ticks=10;
            a.closure_av=nasal(q.left_phone)?35:fricative(q.left_phone)||stop(q.left_phone)?0:20;
            a.af=l==P(n)?60:58;a.noise[1]=l==P(n)?58:65;
            a.tract.departing[0]=300;a.tract.departing[1]=1600;
            a.tract.departing[2]=l==P(n)?2500:2000;
            a.tl_step=q.left_phone==P(r)||q.left_phone==P(R);
            if(nasal(q.left_phone))for(unsigned j=0;j<3;++j)a.tract.entering[j]=-2;
        }
    } else if (kind==5) {
        if (last) {
            struct reloq_german_nasal_tract tract;
            if (reloq_german_nasal_tract_plan(&q,0,0,&tract)!=1) return 0;
            memcpy(a.tract.entering,tract.entering,sizeof tract.entering);
            memcpy(a.tract.departing,tract.departing,sizeof tract.departing);
            memcpy(a.tract.bandwidth,tract.bandwidth,sizeof tract.bandwidth);
            a.body_ticks=40;a.av=48;a.nasal_peak=350;
        } else {
            const int fresh=p[h].left_phone==P(NONE);
            /* The closure belongs to the onset also inside a phrase or
             * word. Only utterance-initial k lacks the preceding hold;
             * word-internal schwa adds four ticks before its release. */
            a.body_ticks=fresh?17:p[h].same_word_left && q.left_phone==P(AT)?72:68;
            a.burst_ticks=12;a.release_ticks=50;
            a.af=65;a.noise[1]=60;a.release_ah=50;
            q.right_phone=P(R);
            if(reloq_german_obstruent_tract_plan(&q,fresh,0,&a.tract)!=1)return 0;
            a.tract.departing[0]=300;a.tract.departing[1]=1800;
            a.tract.departing[2]=2300;
            if(nasal(q.left_phone))for(unsigned j=0;j<3;++j)a.tract.entering[j]=-2;
        }
    } else if (kind==4) {
        /* German w is the voiced fricative /v/. In an onset group its
         * midpoint voicing is independent of the preceding burst/contact,
         * and must not inherit an aspirated stop-to-vowel release.
         * Evidence: behavior-0.2.37, CV clusters and independent holdouts. */
        if (last) {
            if (reloq_german_obstruent_tract_plan(&q,0,0,&a.tract)!=1) return 0;
            a.body_ticks=fricative(c)||c==P(t)?46:62;
            a.midpoint_av=30;a.af=60;a.noise[5]=60;
            if (c==P(s)) {
                a.af=65;
                if (v==P(A)||v==P(a)||v==P(Q_a_TILDE)) a.tract.departing[1]=920;
                else if (v==P(O)||v==P(U)||v==P(o)||v==P(u)||v==P(Q_o_TILDE))
                    a.tract.departing[1]=820;
            }
        } else {
            q.same_syllable_left=0;
            if (stop(c)) q.right_phone=P(A);
            const int fresh=initial && p[h].left_phone==P(NONE);
            if (reloq_german_obstruent_tract_plan(&q,
                c==P(k)?fresh:initial,0,&a.tract)!=1) return 0;
            a.release_ticks=unvoiced(c)?10:1;
            switch (c) {
            case P(p):
                a.body_ticks=initial?15:s!=1 && closed(p[h].left_phone)?85:100;
                a.burst_ticks=10;
                a.af=60;a.noise[5]=54;a.tract.departing[1]=1000;break;
            case P(t):
                a.body_ticks=initial?17:55;a.burst_ticks=12;
                a.af=64;a.noise[2]=45;a.noise[4]=50;
                a.tract.departing[1]=1300;break;
            case P(k):
                a.body_ticks=fresh?13:p[h].left_phone==P(AT)?68:s?83:66;
                a.burst_ticks=8;a.af=65;a.noise[1]=60;
                a.tract.departing[1]=1500;a.tract.departing[2]=2300;
                if(nasal(q.left_phone)) for(unsigned j=0;j<3;++j) a.tract.entering[j]=-2;
                break;
            case P(b):
                a.body_ticks=93;a.burst_ticks=11;a.af=56;a.noise[5]=64;break;
            case P(d):
                a.body_ticks=100;a.burst_ticks=10;a.af=50;a.noise[4]=55;break;
            case P(g):
                a.body_ticks=80;a.burst_ticks=10;a.af=58;a.noise[1]=65;
                a.tract.departing[2]=2000;break;
            case P(f): a.body_ticks=72;a.af=58;a.noise[5]=60;break;
            case P(s): a.body_ticks=82;a.af=62;a.noise[4]=50;break;
            case P(S):
                a.body_ticks=initial?87:110;a.af=56;
                a.noise[1]=56;a.noise[2]=a.noise[3]=62;break;
            case P(x): a.body_ticks=80;a.af=55;a.noise[1]=70;a.noise[3]=55;break;
            case P(X):
                a.body_ticks=s?90:80;a.af=62;
                a.noise[1]=p[h].left_phone==P(e)||p[h].left_phone==P(i)?48:52;
                a.noise[2]=56;break;
            default:return 0;
            }
        }
    } else if (kind==1) {
        if (!last) {
            q.right_phone=P(R);
            if (reloq_german_obstruent_tract_plan(&q,initial,0,&a.tract)!=1) return 0;
            a.body_ticks=l==P(k)?(initial?87:110):80;a.release_ticks=1;
            a.tract.entering[1]=initial&&l==P(p)?1500:1800;
            a.tract.departing[1]=l==P(p)?1500:1800;
            a.af=l==P(t)?58:56;a.noise[1]=l==P(t)?0:56;
            a.noise[2]=a.noise[3]=62;
        } else {
            struct reloq_german_stop_phase phase;
            if (reloq_german_stop_phase_plan(&q,
                p[h+2].same_syllable_right?p[h+2].right_phone:P(NONE),0,&phase)!=1)
                return 0;
            a=phase.gesture;
        }
    } else if (last && (kind==3 || kind==6)) {
        a.body_ticks=kind==6?70:c==P(f)||c==P(v)||c==P(z)?20:60;
        a.av=kind==6?(vowel(v)?50:58):vowel(v)?reloq_german_j_av(&q):54;a.ah=34;
        if (!vowel(v)) a.release_ticks=kind==3?15:
            v==P(k)||v==P(g)||v==P(r)?15:40;
        if (kind==6) a.tract=(struct reloq_german_obstruent_tract){
            {400,940,2260,3600,3900},{-1,-1,-1,-1,-1},{120,250,500,330,260}};
        else if (reloq_german_liquid_tract_plan(&q,0,0,&a.tract)!=1) return 0;
        if (c==P(z)) {
            a.source_lead_tl=1;
        }
        if (!unvoiced(c)) a.source_lead_ticks=c==P(b)?25:
            c==P(d)||c==P(g)?(s?25:15):c==P(m)?15:30;
    } else if ((last && kind==2) || (!last && kind==3 && (c==P(m)||c==P(n)))) {
        Phone nasal=last?l:c;
        a.nasal_peak=350;a.omit_av=!last && !initial;
        if (nasal==P(m)) {
            a.body_ticks=last?(s?64:60):85;a.av=s==1?51:50;
            a.tract=(struct reloq_german_obstruent_tract){
                {300,1000,2500,3600,3900},{-1,1000,-1,-1,-1},{120,150,105,330,260}};
            Phone r=last?v:P(j);
            a.tract.departing[1]=front(r)||r==P(j)?1300:
                r==P(u)||r==P(o)?700:r==P(U)||r==P(O)?750:1000;
            if (r==P(u)) a.tract.entering[2]=2300;
            if (!last) a.tract.entering[1]=initial?1300:750;
        } else {
            a.body_ticks=last?40:s==1?70:40;a.av=48;
            struct reloq_german_nasal_tract tract;
            if (!last) q.right_phone=P(NONE);
            if (reloq_german_nasal_tract_plan(&q,initial&&!last,0,&tract)!=1) return 0;
            memcpy(&a.tract,&tract,sizeof a.tract);
        }
        if (!last) a.release_ticks=nasal==P(m)?15:30;
    } else {
        q.right_phone=kind==2 || kind==6?P(R):c==P(p)?P(E):c==P(d)?P(i):P(I);
        const int after_m=c==P(p) && p[h].left_phone==P(m);
        if (after_m) q.left_phone=P(R);
        if (reloq_german_obstruent_tract_plan(&q,initial,0,&a.tract)!=1) return 0;
        if (kind==2) {
            a.release_ticks=c==P(S)?16:1;
            if (c==P(s)) {a.body_ticks=l==P(m)?92:110;a.af=reloq_german_s_nasal_source(p+h);a.noise[4]=50;}
            else if (c==P(S)) {
                a.body_ticks=initial?87:110;a.af=56;
                a.noise[1]=56;a.noise[2]=a.noise[3]=62;
                a.tract.departing[1]=l==P(m)?1500:1800;
                if (initial) a.tract.entering[1]=a.tract.departing[1];
            } else {
                a.body_ticks=78;a.af=55;a.noise[1]=70;a.noise[3]=55;
                a.tract.departing[1]=a.tract.entering[1]=initial?1350:a.tract.entering[1];
            }
        } else switch (c) {
        case P(p):
            a.body_ticks=initial?15:after_m?52:kind==3 && s!=1 && closed(p[h].left_phone)?85:100;a.burst_ticks=10;
            a.release_ticks=s==1?63:s==2?44:25;a.release_ah=49;
            a.af=60;a.noise[5]=54;
            if (after_m) for (unsigned j=0;j<3;++j) a.tract.entering[j]=-2;
            break;
        case P(t):
            a.body_ticks=initial?17:69;a.burst_ticks=12;
            a.release_ticks=s==1?(vowel(v)?60:90):s==2?63:36;a.release_ah=51;
            a.af=56;a.noise[4]=60;break;
        case P(k):
            a.body_ticks=initial?17:s?87:70;a.burst_ticks=12;
            a.release_ticks=s==1?76:s==2?53:30;a.release_ah=45;
            a.af=65;a.noise[1]=60;break;
        case P(b):
            a.body_ticks=93;a.burst_ticks=11;a.release_ticks=25;
            a.closure_av=20;a.af=56;a.noise[5]=64;break;
        case P(d):
            a.body_ticks=100;a.burst_ticks=10;a.release_ticks=s?25:15;
            a.closure_av=20;a.af=50;a.noise[4]=55;break;
        case P(g):
            a.body_ticks=80;a.burst_ticks=10;a.release_ticks=s?25:15;
            a.closure_av=20;a.af=63;a.noise[2]=60;break;
        case P(f):
            a.body_ticks=s?118:95;a.release_ticks=30;a.af=58;a.noise[5]=60;break;
        case P(v):
            a.body_ticks=initial?48:85;a.release_ticks=30;
            a.af=60;a.noise[5]=60;a.midpoint_av=30;break;
        case P(z):
            /* Voicing begins during the sibilant-to-glide contact. The
             * oral glide and its source are separately timed gestures. */
            a.body_ticks=initial?(s==1?90:78):68;a.release_ticks=30;
            a.tl_step=!initial;
            a.af=66;a.noise[3]=a.noise[4]=50;a.midpoint_av=20;a.tl=35;break;
        default:return 0;
        }
        if (kind==6) {
            if (c==P(t)) {
                a.release_ticks=s==1?90:s==2?63:36;a.af=59;
            } else if (c==P(k)) {
                a.tract.departing[1]=1500;a.tract.departing[2]=2300;
            } else if (c==P(g)) {
                a.tract.departing[2]=2000;
                a.af=58;a.noise[2]=0;a.noise[1]=65;
            }
            if (!initial && !vowel(p[h].left_phone)) {
                consonant_left_stop(c,p[h].left_phone,0,s,&a);
                if (c==P(k) && s && stop(p[h].left_phone)) a.body_ticks=67;
                if (c==P(p) && nasal(p[h].left_phone)) a.body_ticks=52;
                if (c==P(b) && p[h].left_phone!=P(p) && p[h].left_phone!=P(t))
                    a.body_ticks=93;
            }
            if (c==P(k) && !initial && p[h].left_phone==P(AT)) a.body_ticks=72;
        }
    }
    /* A closing glide, or a glide across the syllable boundary, owns the
     * source from this stop's oral release. Its tract starts later. Do not
     * leave aspiration endpoints inside that independently voiced interval.
     * This applies equally to j and w, regardless of the spelling. */
    if (!last && stop(c) && (l==P(j)||l==P(w)) &&
        (!vowel(v)||!p[h].same_syllable_right)) a.release_ah=-1;
    /* Across a syllable boundary the oral stop still has its contact
     * interval, but j voices from the stop's release. It must not inherit
     * the aspiration interval used by a joint stop+j onset. */
    if(kind==3 && !p[h].same_syllable_right && unvoiced(c)) {
        if(!last) {
            if(c==P(k))a.body_ticks=87;
        } else a.source_lead_ticks=c==P(p)?(s==1?63:s==2?44:25):
            c==P(t)?(s==1?60:s==2?63:36):(s==1?76:s==2?53:30);
    }
    /* A connected word-initial stop+j retains the previous articulation.
     * In particular n supplies both nasal voicing and the missing entering
     * formant knots; restarting at an isolated d would truncate its tail. */
    if (kind==3 && !last && initial && p[h].left_phone!=P(NONE) &&
        (c==P(b)||c==P(d)||c==P(g))) {
        const Phone left=p[h].left_phone;
        q.same_syllable_left=0;
        if (reloq_german_obstruent_tract_plan(&q,0,0,&a.tract)!=1) return 0;
        a.closure_av=nasal(left)?35:stop(left)||fricative(left)||
            (c==P(d)&&left==P(l))?0:20;
        if (c==P(b)) {
            a.body_ticks=left==P(p)?56:left==P(t)?61:93;
            if (left==P(X)) a.tract.entering[1]=1500;
            if (left==P(r)) a.tract.entering[1]=900;
        } else if (c==P(d)) {
            a.body_ticks=nasal(left)||left==P(l)?45:left==P(p)||left==P(k)?73:
                left==P(t)?60:100;
            if (left==P(l)) a.tract.entering[1]=1250;
            if (left==P(X)) {a.tract.entering[1]=1800;a.tract.entering[2]=3000;}
            if (vowel(left)) {a.tract.entering[1]=1600;a.tract.entering[2]=2600;}
        } else {
            a.body_ticks=left==P(f)?50:80;
            if (left==P(l)) a.tract.entering[1]=1200;
            if (left==P(r)) a.tract.entering[1]=1500;
        }
        if (nasal(left)) for (unsigned j=0;j<3;++j) a.tract.entering[j]=-2;
    }
    /* Connected word-edge t+v keeps its oral closure. Only the isolated
     * utterance starts at the release; preceding manner controls reduction. */
    if(kind==4 && !last && c==P(t) && initial && p[h].left_phone!=P(NONE)) {
        Phone left=p[h].left_phone;
        a.body_ticks=nasal(left)?80:left==P(t)||left==P(d)?46:
            fricative(left)?58:63;
        a.burst_ticks=12;
        struct reloq_german_phone_node edge=p[h];
        edge.same_syllable_left=0;edge.right_phone=P(A);
        if(reloq_german_obstruent_tract_plan(&edge,0,0,&a.tract)!=1)return 0;
        a.tract.departing[1]=1300;a.tract.departing[2]=2500;
        if(nasal(left))for(unsigned j=0;j<3;++j)a.tract.entering[j]=-2;
    }
    *out=a;return 1;
}

static void following_sibilant(const struct reloq_german_phone_node *p,
    struct reloq_german_onset_cluster *out)
{
    if (p->phone!=P(S) || (p->left_phone!=P(s) && p->left_phone!=P(S))) return;
    out->body_ticks=60;
    out->tract.entering[1]=1800;out->tract.entering[2]=2400;
    out->tract.departing[1]=p->right_phone==P(p)||p->right_phone==P(v)||p->right_phone==P(m)?1500:1800;
    out->tract.departing[2]=2400;
}

int reloq_german_onset_cluster_plan(const struct reloq_german_phone_node *p,
    size_t n,size_t i,struct reloq_german_onset_cluster *out)
{
    if (!p || !out || i>=n) return -1;
    /* Affricate frication owns its existing release clock; its following
     * liquid still enters the shared cluster plan through valid(). */
    if (p[i].affricate_role==2) return 0;
    if (i>=2 && p[i-1].phone==P(r) && continued_r(p,n,i-2) &&
        p[i].same_syllable_right && vowel(p[i].right_phone) && !p[i].affricate_role) {
        struct reloq_german_onset_cluster a={0};a.release_ah=-1;
        struct reloq_german_phone_node q=p[i];q.same_syllable_left=0;
        const Phone c=q.phone;const unsigned s=q.stress;
        if (c==P(t)) {
            struct reloq_german_stop_phase phase;
            if (reloq_german_stop_phase_plan(&q,i+1<n && p[i+1].same_syllable_right?
                    p[i+1].right_phone:P(NONE),0,&phase)!=1) return 0;
            a=phase.gesture;a.release_ticks=0;a.release_ah=-1;
            if (reloq_german_obstruent_tract_plan(&q,0,0,&a.tract)!=1) return 0;
            a.tract.entering[1]=1325;
        } else if (c==P(n)) {
            struct reloq_german_nasal_tract tract;
            if (reloq_german_nasal_tract_plan(&q,0,0,&tract)!=1) return 0;
            memcpy(&a.tract,&tract,sizeof a.tract);
            a.body_ticks=s==1?70:40;a.av=48;a.nasal_peak=350;
        } else if (c==P(l)) {
            if (reloq_german_liquid_tract_plan(&q,0,0,&a.tract)!=1) return 0;
            a.body_ticks=s?53:42;a.av=46;a.ah=34;a.source_lead_ticks=25;
        } else if (c==P(s)||c==P(f)) {
            if (reloq_german_obstruent_tract_plan(&q,0,0,&a.tract)!=1) return 0;
            a.body_ticks=c==P(f)?95:s==1?125:s==2?100:85;
            a.af=c==P(f)?58:62;a.noise[c==P(f)?5:4]=c==P(f)?60:50;
        } else return 0;
        *out=a;return 1;
    }
    if (p[i].same_word_right && p[i].right_phone==P(j) &&
        i+2<n && p[i+1].same_syllable_right && vowel(p[i+2].phone)) {
        struct reloq_german_onset_cluster a={0};a.release_ah=-1;
        if (p[i].phone==P(n) && !p[i].same_syllable_right &&
            p[i].same_syllable_left && vowel(p[i].left_phone)) {
            struct reloq_german_nasal_tract tract;
            if (reloq_german_nasal_tract_plan(p+i,0,0,&tract)!=1) return 0;
            memcpy(&a.tract,&tract,sizeof a.tract);
            a.body_ticks=100;a.release_ticks=30;a.nasal_peak=350;a.omit_av=1;
            *out=a;return 1;
        }
        if (p[i].phone==P(l) && p[i].same_syllable_right) {
            a.tract=(struct reloq_german_obstruent_tract){
                {300,1550,2900,3450,3900},{-1,-1,-1,-1,-1},{120,100,150,330,260}};
            a.body_ticks=76;a.release_ticks=40;a.av=46;a.ah=34;
            *out=a;return 1;
        }
    }
    if (p[i].phone==P(z) && p[i].same_word_right &&
        p[i].same_syllable_right && p[i].right_phone==P(w)) {
        struct reloq_german_onset_cluster a={0};a.release_ah=-1;
        if (reloq_german_obstruent_tract_plan(p+i,!p[i].same_word_left,0,&a.tract)!=1) return 0;
        a.body_ticks=!p[i].same_word_left?(p[i].stress==1?90:78):68;
        a.release_ticks=30;a.af=66;a.noise[3]=a.noise[4]=50;
        a.midpoint_av=20;a.tl=35;a.tl_step=p[i].same_word_left;
        *out=a;return 1;
    }
    if (p[i].phone==P(v) && i>=2 && p[i-1].affricate_role==4 &&
        p[i].same_syllable_left && p[i].same_syllable_right && vowel(p[i].right_phone)) {
        struct reloq_german_onset_cluster a={0};a.release_ah=-1;
        if (reloq_german_obstruent_tract_plan(p+i,0,0,&a.tract)!=1) return 0;
        a.body_ticks=p[i-2].same_word_left?46:64;
        a.midpoint_av=30;a.af=65;a.noise[5]=60;
        Phone v=p[i].right_phone;
        if (v==P(A)||v==P(a)||v==P(Q_a_TILDE)||v==P(Q_aj)||v==P(Q_aw)) a.tract.departing[1]=920;
        else if (v==P(O)||v==P(U)||v==P(o)||v==P(u)||v==P(Q_o_TILDE)||v==P(Q_oj)) a.tract.departing[1]=820;
        *out=a;return 1;
    }
    if (extended_plan(p,n,i,out)) {following_sibilant(p+i,out);return 1;}
    /* r+j is an onset consonant followed by a glide. The original keeps
     * a continuous 40/52 voiced/aspirated plateau here; it neither
     * vocalizes r nor inserts the interrupted simple-r source cycle. */
    if(p[i].phone==P(r) && !p[i].same_syllable_left &&
       p[i].same_syllable_right && p[i].right_phone==P(j) && i+2<n &&
       p[i+1].same_syllable_right && vowel(p[i+2].phone)) {
        struct reloq_german_onset_cluster a={0};a.release_ah=-1;
        if(reloq_german_liquid_tract_plan(p+i,!p[i].same_word_left,0,&a.tract)!=1)return 0;
        a.av=40;a.ah=52;a.body_ticks=44;a.release_ticks=25;
        *out=a;return 1;
    }
    if (p[i].phone==P(j) && !p[i].affricate_role &&
        ((p[i].same_syllable_right && vowel(p[i].right_phone)) ||
         (p[i].same_word_left && p[i].same_word_right && !vowel(p[i].right_phone))) &&
        !(p[i].same_syllable_left && vowel(p[i].left_phone))) {
        struct reloq_german_onset_cluster a={0};a.release_ah=-1;
        if (reloq_german_liquid_tract_plan(p+i,!p[i].same_word_left,0,&a.tract)!=1) return 0;
        a.av=vowel(p[i].right_phone)?reloq_german_j_av(p+i):54;a.ah=34;
        if (!vowel(p[i].right_phone)) a.release_ticks=15;
        a.body_ticks=!p[i].same_word_left?45:
            vowel(p[i].left_phone)||fricative(p[i].left_phone)||
            p[i].left_phone==P(r)||p[i].left_phone==P(w)?20:60;
        *out=a;return 1;
    }
    if (p[i].phone==P(w) && p[i].same_word_left && p[i].same_word_right &&
        !(p[i].same_syllable_left && vowel(p[i].left_phone))) {
        struct reloq_german_onset_cluster a={0};a.release_ah=-1;
        if (reloq_german_liquid_tract_plan(p+i,0,0,&a.tract)!=1) return 0;
        a.body_ticks=stop(p[i].left_phone)||nasal(p[i].left_phone)?70:40;
        a.av=vowel(p[i].right_phone)?50:58;a.ah=34;
        if (!vowel(p[i].right_phone)) a.release_ticks=
            p[i].right_phone==P(k)||p[i].right_phone==P(g)||p[i].right_phone==P(r)?15:40;
        *out=a;return 1;
    }
    size_t h=i;int liquid=0,prefix=0;
    if (i && valid(p,n,i-1)) {h=i-1;liquid=1;}
    else if ((p[i].phone==P(S) || p[i].phone==P(s)) && i+1<n &&
             (!p[i].same_word_left ||
              (p[i].phone==P(s) && !p[i].same_syllable_left) ||
              p[i].left_phone==P(s)||p[i].left_phone==P(S)) &&
             p[i].same_syllable_right && unvoiced(p[i+1].phone) && valid(p,n,i+1))
        {h=i+1;prefix=1;}
    else if (!valid(p,n,i)) return 0;
    const Phone c=p[h].phone,l=p[h+1].phone,v=p[h+2].phone;
    const int continuation=continued_r(p,n,h);
    const int lateral_boundary=c==P(d) && l==P(l) && !p[h].same_syllable_right;
    const int rhotic=l==P(r),s=lateral_boundary?p[h+1].stress:p[h].stress,
        initial=!p[h].same_word_left;
    const int closed_long=closed(v) && !reloq_german_nucleus_shortened(p+h+2);
    const int after_s=p[h].same_syllable_left &&
        (p[h].left_phone==P(S) || p[h].left_phone==P(s));
    struct reloq_german_onset_cluster a={0};
    a.release_ah=-1;
    struct reloq_german_phone_node context=p[i];
    if(lateral_boundary && !liquid) {
        context.same_syllable_left=0;context.same_syllable_right=1;
    }
    if (prefix) {
        if (p[i].phone==P(s)) {
            context.right_phone=P(R);
            if (reloq_german_obstruent_tract_plan(&context,!p[i].same_word_left,0,&a.tract)!=1) return 0;
            a.body_ticks=s?76:64;a.release_ticks=1;
            a.af=62;a.noise[4]=50;
            *out=a;return 1;
        }
        a.body_ticks=c==P(k)?87:80;a.release_ticks=1;
        context.right_phone=P(R);
        if (reloq_german_obstruent_tract_plan(&context,1,0,&a.tract)!=1) return 0;
        a.tract.entering[1]=a.tract.departing[1]=c==P(p)?1500:1800;
        a.af=56;a.noise[1]=56;a.noise[2]=62;a.noise[3]=62;
    } else if (liquid) {
        a.body_ticks=rhotic?20:unvoiced(c)?(!after_s && s==1?30:20):
            c==P(f)||c==P(X)?(s?34:65):c==P(S)?34:c==P(s)?68:s?53:42;
        /* An onset liquid uses its right vowel independently of the preceding
         * constriction. A labial left neighbour additionally rounds r. */
        context.left_phone=P(R);context.same_syllable_left=0;
        if (continuation) {
            /* The r gesture remains consonantal before another member of
             * the onset. Its own oral hold and the following contact are
             * distinct from a release into a vowel (ger_ph_r). */
            a.tract=(struct reloq_german_obstruent_tract){
                {400,1320,2100,3600,3900},{-1,-1,-1,-1,-1},
                {120,100,150,330,260}};
            a.release_ticks=stop(v)?50:25;
        } else if (reloq_german_liquid_tract_plan(&context,0,0,&a.tract)!=1) return 0;
        if (rhotic) {
            a.av=c==P(t)?0:40;a.af=c==P(b)||c==P(d)||c==P(v)?49:45;
            a.noise[1]=50;a.di=c==P(b)||c==P(d)||c==P(f)||c==P(v)||c==P(z)?40:60;
            if (continuation) a.di=unvoiced(c)&&!after_s?60:40;
            /* These are onset-r targets, not the postvocalic R nucleus. */
            if (front(v)) {
                a.tract.entering[1]=v==P(e)||v==P(i)||v==P(Q_E_COLON)?1670:1450;
                a.tract.entering[2]=v==P(e)||v==P(i)||v==P(Q_E_COLON)?2300:2200;
                a.tract.departing[1]=a.tract.departing[2]=-1;
            } else if (v==P(O)||v==P(U)||v==P(o)||v==P(u)||v==P(y)||v==P(Y)) {
                a.tract.entering[2]=2000;a.tract.departing[2]=-1;
                const int labial=c==P(p)||c==P(b)||c==P(f)||c==P(v);
                a.tract.entering[1]=labial?(v==P(o)||v==P(u)?1000:v==P(y)||v==P(Y)?1200:900):1320;
                a.tract.departing[1]=labial && (v==P(o)||v==P(u))?850:-1;
            }
        } else {
            a.av=46;a.ah=34;
            if (stop(c) && !unvoiced(c))
                a.source_lead_ticks=lateral_boundary?1:c==P(b)?10:s?25:15;
        }
    } else {
        context.right_phone=P(R);
        if (after_s) context.left_phone=P(R);
        /* A connected word-initial stop takes its entering locus from the
         * preceding word's final vowel, like a word-internal onset. */
        const int connected=initial && p[h].left_phone!=P(NONE) && stop(c) && c!=P(d) &&
            vowel(p[h].left_phone);
        if (reloq_german_obstruent_tract_plan(&context,initial && !connected,0,&a.tract)!=1) return 0;
        switch (c) {
        case P(p):
            a.body_ticks=initial?(rhotic?10:15):after_s?(rhotic?70:75):rhotic?95:100;
            a.burst_ticks=rhotic?5:10;
            a.release_ticks=after_s?36:rhotic?(s==1?83:s==2?64:initial&&closed_long?45:39):s==1?54:s==2?37:21;
            a.af=60;a.noise[5]=rhotic?68:60;a.release_ah=53;break;
        case P(t):
            a.body_ticks=initial?17:after_s?48:69;a.burst_ticks=12;
            a.release_ticks=after_s && (!rhotic||s!=1)?36:
                rhotic?(s==1?75:s==2?63:initial&&closed_long?36:28):s==1?90:s==2?63:36;
            a.af=rhotic?60:59;a.noise[4]=rhotic?54:60;a.release_ah=rhotic?56:51;
            a.tract.departing[1]=rhotic?1300:1600;break;
        case P(k):
            a.body_ticks=initial?17:rhotic?(s?87:70)-(after_s?4:0):after_s?64:68;
            a.burst_ticks=12;a.release_ticks=after_s&&!rhotic?36:rhotic?(s==1?70:50):s==1?76:s==2?53:30;
            a.af=rhotic?62:64;a.noise[rhotic?1:2]=rhotic?65:60;
            if (!rhotic && (v==P(o)||v==P(u)||v==P(U))) {a.af=60;a.noise[1]=65;a.noise[2]=58;}
            a.release_ah=rhotic?45:50;a.tract.departing[1]=1500;a.tract.departing[2]=2300;break;
        case P(b):
            a.body_ticks=rhotic?(initial||s==1?93:78):81;a.burst_ticks=11;
            a.release_ticks=rhotic?50:10;a.af=rhotic?56:50;a.noise[5]=rhotic?64:63;
            a.closure_av=20;if (rhotic) a.release_ah=54;
            a.tract.departing[1]=rhotic?800:1000;break;
        case P(d):
            a.body_ticks=rhotic?72:100;a.burst_ticks=10;
            a.release_ticks=rhotic?50:s?25:15;a.af=50;a.noise[4]=55;
            a.closure_av=20;if (rhotic) a.release_ah=54;
            a.tract.departing[1]=rhotic?1400:1100;break;
        case P(g):
            a.body_ticks=rhotic?80:58;a.burst_ticks=10;
            a.release_ticks=rhotic?50:s?25:15;a.af=rhotic?62:58;a.noise[1]=rhotic?64:65;
            a.closure_av=20;if (rhotic) a.release_ah=54;
            a.tract.departing[1]=1600;a.tract.departing[2]=2000;break;
        case P(f):
            a.body_ticks=rhotic?95:110;a.release_ticks=rhotic?35:30;
            a.af=58;a.noise[5]=60;a.release_ah=54;break;
        case P(s):
            a.body_ticks=110;a.release_ticks=rhotic?60:1;
            a.af=62;a.noise[4]=50;a.release_ah=54;break;
        case P(S):
            a.body_ticks=initial?87:110;a.release_ticks=rhotic?60:30;
            a.af=56;a.noise[1]=56;a.noise[2]=62;a.noise[3]=62;a.release_ah=54;break;
        case P(X):
            a.body_ticks=s?90:80;a.release_ticks=rhotic?60:30;
            a.af=62;a.noise[1]=52;a.noise[2]=56;a.release_ah=54;break;
        case P(v):
            a.body_ticks=initial?48:85;a.release_ticks=1;
            a.af=60;a.noise[5]=60;a.midpoint_av=30;break;
        case P(z):
            a.body_ticks=initial?(s==1?90:78):68;a.release_ticks=1;
            a.af=66;a.noise[3]=a.noise[4]=50;a.midpoint_av=20;a.tl=35;break;
        default:return 0;
        }
        /* After a preceding word in the same phrase, a word-initial p/t/k
         * of a liquid cluster keeps the full silent closure measured for the
         * reference voice; only an utterance-initial stop starts at once. */
        if (initial && p[h].left_phone!=P(NONE) && unvoiced(c))
            a.body_ticks=a.burst_ticks+(c==P(p)?90:c==P(t)?68:rhotic?75:56);
        Phone left=p[h].left_phone;
        if (left==P(r) && h>=2 && p[h-1].same_syllable_left &&
            vowel(p[h-2].phone)) left=P(R);
        if (!initial && !vowel(left)) {
            if (c==P(S)) a.body_ticks=left==P(s)||left==P(S)?60:85;
            if (c==P(f)) a.body_ticks=rhotic?
                (nasal(left)||fricative(left)?62:95):
                nasal(left)?105:fricative(left)?95:110;
            if (c==P(z)) {
                if (left==P(s)||left==P(z)) a.body_ticks=70;
                if (fricative(left)) {a.midpoint_av=0;a.tl=0;}
                else if (nasal(left)) a.midpoint_av=35;
                else if (stop(left)) {a.midpoint_av=37;a.tl=0;}
            }
            if (c==P(v) && (fricative(left)||left==P(t)||left==P(d))) a.body_ticks=46;
        }
        if (stop(c)) a.tract.departing[0]=c==P(p)&&initial?400:a.tract.entering[0];
        if (after_s && p[h].left_phone==P(s) && c==P(p)) {
            a.body_ticks=(s==1?95:80)+(rhotic?0:5);
            a.tract.entering[1]=1500;
        }
        if (after_s && p[h].left_phone==P(s) && (c==P(t) || c==P(k)))
            a.burst_ticks=8;
        if (stop(c) && !after_s && p[h].left_phone!=P(NONE) &&
            ((!initial && !vowel(p[h].left_phone)) || (initial && nasal(p[h].left_phone))))
            consonant_left_stop(c,p[h].left_phone,rhotic,s,&a);
        /* A connected word-edge t keeps its longer closure except in
         * the n+tr group; internal syllable and word-edge timing differ. */
        if (initial && c==P(t) && nasal(p[h].left_phone) &&
            !(rhotic && p[h].left_phone==P(n))) a.body_ticks=80;
        if (continuation) {
            if (c==P(k)) {
                a.release_ticks=50;
                if (after_s) {a.burst_ticks=8;a.release_ah=-1;}
            } else if (c==P(p) && initial && !s) a.release_ticks=45;
            else if (c==P(t) && initial) a.release_ticks=s==1?90:s==2?63:36;
            else if (c==P(g)) a.release_ah=-1;
            else if (c==P(f)||c==P(s)||c==P(S)||c==P(X)) {
                if (c==P(f)) a.body_ticks=60;
                if (c==P(X)) a.body_ticks=65;
                a.release_ticks=1;a.release_ah=-1;
            }
        }
    }
    if(lateral_boundary && !liquid) a.release_ticks=1;
    *out=a;following_sibilant(p+i,out);return 1;
}

static int source(const struct reloq_german_onset_cluster *p,int64_t t,size_t i,
    int closure,reloq_german_source_emit emit,void *ctx)
{
    if (emit(ctx,RELOQ_TARGET_AF,t,closure?0:p->af,i) ||
        emit(ctx,RELOQ_TARGET_TL,t,closure&&p->closure_av?35:p->tl,i)) return -1;
    for (unsigned j=0;j<6;++j)
        if (emit(ctx,(enum reloq_target_stream)(RELOQ_TARGET_A1F+j),t,closure?0:p->noise[j],i)) return -1;
    if (!p->midpoint_av && !p->omit_av && emit(ctx,RELOQ_TARGET_AV,t,closure?p->closure_av:p->av,i)) return -1;
    return emit(ctx,RELOQ_TARGET_AH,t,closure?0:p->ah,i);
}
int reloq_german_onset_cluster_emit(const struct reloq_german_onset_cluster *p,
    int64_t begin,size_t i,reloq_german_source_emit emit,void *ctx)
{
    if (!p || !emit || begin<0 || p->body_ticks<1 || p->burst_ticks>p->body_ticks ||
        begin>INT64_MAX-p->body_ticks-p->release_ticks) return -1;
    int64_t end=begin+p->body_ticks;
    if (p->tl_step && emit(ctx,RELOQ_TARGET_TL,begin,0,i)) return -1;
    if (begin<p->source_lead_ticks) return -1;
    if (p->source_lead_ticks &&
        (emit(ctx,RELOQ_TARGET_AV,begin-p->source_lead_ticks,p->av,i) ||
         emit(ctx,RELOQ_TARGET_AH,begin-p->source_lead_ticks,p->ah,i))) return -1;
    if (p->source_lead_tl &&
        emit(ctx,RELOQ_TARGET_TL,begin-p->source_lead_ticks,p->tl,i)) return -1;
    if (p->nasal_peak) {
        if (emit(ctx,RELOQ_TARGET_FNZ,begin+p->body_ticks/2,p->nasal_peak,i)) return -1;
    } else if (emit(ctx,RELOQ_TARGET_FNZ,begin,200,i) ||
               emit(ctx,RELOQ_TARGET_FNZ,end,200,i)) return -1;
    if (p->burst_ticks) {
        int64_t at=end-p->burst_ticks;
        if (source(p,begin,i,1,emit,ctx) || source(p,at,i,1,emit,ctx) ||
            source(p,at,i,0,emit,ctx)) return -1;
    } else if (source(p,begin,i,0,emit,ctx)) return -1;
    if (source(p,end,i,0,emit,ctx)) return -1;
    if (p->midpoint_av && emit(ctx,RELOQ_TARGET_AV,begin+p->body_ticks/2,p->midpoint_av,i)) return -1;
    if (p->release_ah>=0) {
        if (emit(ctx,RELOQ_TARGET_AV,end,0,i) || emit(ctx,RELOQ_TARGET_AV,end+p->release_ticks,0,i) ||
            emit(ctx,RELOQ_TARGET_AH,end,p->release_ah,i) || emit(ctx,RELOQ_TARGET_AH,end+p->release_ticks,p->release_ah,i)) return -1;
    }
    return 0;
}
#undef P
