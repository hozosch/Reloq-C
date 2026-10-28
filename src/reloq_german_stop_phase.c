#include "reloq_german_stop_phase.h"
#include "reloq_german_terminal_stop_tract.h"
#include <string.h>

typedef enum reloq_german_phone Phone;
#define P(n) RELOQ_DE_PHONE_##n
static int vowel(Phone p)
{
    return (p>=P(a) && p<=P(AT)) || p==P(R) ||
        p==P(Q_E_COLON) || p==P(Q_oe) || p==P(Q_OE) || p>=P(Q_a_TILDE);
}
static int closed(Phone p)
{
    return p==P(e) || p==P(i) || p==P(o) || p==P(u) || p==P(y) || p==P(Q_oe) ||
        p==P(Q_o_TILDE) || p==P(Q_oe_TILDE);
}
static int front(Phone p)
{
    return p==P(e) || p==P(E) || p==P(i) || p==P(I) || p==P(y) || p==P(Y) ||
        p==P(Q_E_COLON) || p==P(Q_oe) || p==P(Q_OE) || p==P(Q_E_TILDE);
}
static int nasal(Phone p) {return p==P(m) || p==P(n) || p==P(G);}
static int fricative(Phone p)
{
    return p==P(f) || p==P(s) || p==P(S) || p==P(X) || p==P(x) || p==P(Z);
}
static int stop(Phone p) {return p==P(p) || p==P(t) || p==P(k) || p==P(b) || p==P(d) || p==P(g);}
static int dorsal_or_labial(Phone p) {return p==P(p) || p==P(b) || p==P(k) || p==P(g);}
static Phone left_endpoint(Phone p)
{
    if (p==P(Q_aj) || p==P(Q_oj)) return P(j);
    if (p==P(Q_aw)) return P(w);
    /* Before another syllable's onset these obstruents are devoiced. */
    if (p==P(b)) return P(p);
    if (p==P(d)) return P(t);
    if (p==P(g)) return P(k);
    if (p==P(v)) return P(f);
    if (p==P(z)) return P(s);
    return p;
}
int reloq_german_stop_phase_plan(const struct reloq_german_phone_node *n,
    Phone coda,int fresh,struct reloq_german_stop_phase *out)
{
    if (!n || !out || fresh<0 || fresh>1 || n->stress>2) return -1;
    if (!stop(n->phone) || n->affricate_role || !n->same_syllable_right ||
        !n->same_word_right) return 0;
    const int grouped=n->same_syllable_left;
    if (grouped && n->left_phone!=P(s) && n->left_phone!=P(S) &&
        n->left_phone!=P(f) && n->left_phone!=P(X) &&
        n->left_phone!=P(n) && n->left_phone!=P(l) &&
        n->left_phone!=P(p) && n->left_phone!=P(t) && n->left_phone!=P(k)) return 0;
    /* A stop before another onset stop keeps its independent closure and
     * inter-stop contact. The next stop retains the vowel entrance. */
    if (!grouped && (n->phone==P(p)||n->phone==P(t)||n->phone==P(k)) && stop(n->right_phone)) {
        if (!fresh) {
            struct reloq_german_phone_node group=*n;
            group.same_word_left=group.same_syllable_left=1;
            int status=reloq_german_stop_coda_phase(&group,n->stress,0,P(NONE),out);
            if (status==1) {
                struct reloq_german_obstruent_tract entering;
                struct reloq_german_phone_node onset=*n;
                onset.right_phone=P(A);
                if (reloq_german_obstruent_tract_plan(&onset,0,0,&entering)==1)
                    memcpy(out->gesture.tract.entering,entering.entering,sizeof entering.entering);
            }
            return status;
        }
        const Phone c=n->phone,r=n->right_phone;
        const int dental=r==P(t)||r==P(d),velar=r==P(k)||r==P(g);
        const int quiet=(c==P(p)&&(r==P(p)||r==P(b))) ||
            (c==P(t)&&dental) || (c==P(k)&&velar);
        struct reloq_german_stop_phase p={0};
        struct reloq_german_onset_cluster *a=&p.gesture;
        a->body_ticks=c==P(p)?15:quiet?5:c==P(t)?15:13;
        a->burst_ticks=quiet?0:c==P(p)?10:c==P(t)?12:8;
        a->release_ticks=c==P(t)&&r==P(k)?30:c==P(k)&&r==P(t)?20:1;
        a->release_ah=c==P(t)&&r==P(k)?51:c==P(k)&&r==P(t)?50:-1;
        a->tract=(struct reloq_german_obstruent_tract){
            {c==P(p)?350:300,c==P(p)?(dental?1150:1000):c==P(t)?1600:dental?1650:1500,
             c==P(k)?2300:2500,c==P(t)?3500:3600,c==P(t)?4200:3900},
            {-1,-1,-1,-1,-1},{120,100,150,330,260}};
        if (!quiet) {
            a->af=c==P(p)?60:c==P(t)?70:dental?54:65;
            if(c==P(p))a->noise[5]=dental?60:54;
            else if(c==P(t)) {
                if(velar){a->noise[1]=50;a->noise[4]=40;}
                else a->noise[2]=50;
            } else a->noise[1]=60;
        }
        p.incoming_ticks=a->release_ticks;*out=p;return 1;
    }
    Phone v=n->right_phone;
    if (v==P(Q_aj) || v==P(Q_aw)) v=P(A);
    if (v==P(Q_oj)) v=P(O);
    if (!vowel(v)) return 0;
    Phone l=fresh?P(NONE):left_endpoint(n->left_phone),c=n->phone;
    const int s=n->stress,primary=s==1,internal=n->same_word_left;
    const int long_v=closed(v),front_v=front(v),reduced=v==P(AT) || v==P(R);
    int closure=0,burst=0,release=0;
    struct reloq_german_stop_phase p={0};
    struct reloq_german_onset_cluster *a=&p.gesture;
    a->release_ah=-1;
    switch (c) {
    case P(p):
        burst=10;
        closure=fresh?5:nasal(l)?(primary?64:45):
            primary || (internal && vowel(l) && !closed(l) && l!=P(a) && l!=P(Q_E_COLON))?90:75;
        release=primary?(long_v?63:50):s?(long_v?44:35):(long_v?25:19);
        if (primary && (v==P(U) || v==P(u))) release=long_v?44:35;
        if (internal && !primary) release=s?35:19;
        a->af=60;a->noise[5]=internal?54:
            v==P(O) || v==P(o) || v==P(U) || v==P(u) || v==P(R) ||
            v==P(Q_o_TILDE) || v==P(Q_oe_TILDE)?66:64;
        a->release_ah=49;
        break;
    case P(t):
        burst=front_v?18:12;
        if (fresh) closure=5;
        else if (l==P(s) || l==P(S)) closure=internal?50:55;
        else if (internal && l==P(p)) closure=52;
        else if (internal && l==P(k)) closure=72;
        else if (v==P(AT) && coda==P(n)) closure=38;
        else if (v==P(R) || (v==P(AT) && coda==P(NONE))) closure=34;
        else if (l==P(n)) closure=internal?32:39;
        else closure=internal?57:68;
        if (!fresh && v==P(AT) && coda==P(n) && (l==P(s) || l==P(S))) closure=72;
        if (!fresh && v==P(AT) && coda==P(n) && (l==P(f) || l==P(X))) closure=76;
        if (!fresh && v==P(AT) && coda==P(n) && l==P(l)) closure=45;
        release=primary?(long_v?90:72):s?(long_v?63:50):(long_v?36:28);
        if (primary && (v==P(U) || v==P(u))) release=long_v?63:50;
        if (internal && !primary) release=s?50:28;
        if (s && l==P(k) && !grouped) release=release*3/5;
        a->af=v==P(AT)?56:59;a->noise[4]=56;a->release_ah=47;
        break;
    case P(k):
        burst=12;
        closure=fresh?5:internal && l==P(AT)?60:l==P(n) || l==P(G)?55:
            !s || l==P(m)?70:dorsal_or_labial(l) || fricative(l)?55:75;
        release=primary?(long_v?76:61):50;
        a->af=v==P(A) || v==P(a) || v==P(Q_a_TILDE)?60:
            v==P(E) || v==P(Q_OE) || v==P(Q_oe_TILDE)?54:62;
        if (v==P(I) || v==P(i) || v==P(e) || v==P(y) || v==P(Y) || v==P(Q_oe) || v==P(AT)) {
            a->af=64;a->noise[2]=60;
        } else a->noise[1]=70;
        a->release_ah=s?45:48;
        break;
    case P(b):
        burst=11;
        if (l==P(p)) closure=45;
        else if (l==P(t)) closure=50;
        else if (l==P(m)) closure=35;
        else if (l==P(n)) closure=54;
        else if (v==P(R)) closure=28;
        else if (!s) closure=60;
        else if (v==P(A) || v==P(a) || v==P(Q_a_TILDE)) closure=70;
        else if (v==P(i) || v==P(I) || v==P(u) || v==P(U) || v==P(y) || v==P(Y)) closure=95;
        else if (!internal && (l==P(k) || l==P(l) || l==P(G))) closure=48;
        else if (!internal && fricative(l)) closure=70;
        else closure=v==P(AT)?(coda==P(n)?30:coda==P(l)?48:58):82;
        release=front_v || v==P(AT)?25:15;
        a->af=56;a->noise[5]=64;
        break;
    case P(d):
        burst=v==P(R)?7:primary && (v==P(e) || v==P(i) || v==P(y) || v==P(Q_oe))?15:10;
        if (!internal && !s) closure=l==P(t) || l==P(n)?30:40;
        else if (reduced) closure=20;
        else if (nasal(l) || l==P(l)) closure=35;
        else closure=l==P(t) && s?50:s?90:45;
        if (dorsal_or_labial(l)) closure=(closure*70+50)/100;
        if (internal && l==P(t)) closure=(closure*35+50)/100;
        if (v==P(AT) && (coda==P(n) || coda==P(l))) {
            burst=7;
            if (coda==P(n)) closure=l==P(n)?20:
                l==P(l) || l==P(r) || l==P(R) || front(l)?40:
                vowel(l) || l==P(j) || l==P(w)?30:70;
            else closure=!internal && !s && (l==P(t) || l==P(n))?30:40;
            if (dorsal_or_labial(l)) closure=(closure*70+50)/100;
            if (internal && l==P(t)) closure=(closure*35+50)/100;
        }
        release=reduced?35:front_v?(s?25:15):45;
        a->af=v==P(A) || v==P(a) || v==P(R) || v==P(Q_a_TILDE)?53:
            v==P(AT)?48:v==P(e) || v==P(i)?52:50;
        a->noise[4]=55;
        break;
    case P(g):
        burst=primary?20:10;
        if (nasal(l)) closure=26;
        else if (l==P(f)) closure=40;
        else if (l==P(l)) closure=long_v?70:42;
        else if (vowel(l)) {
            if (!s) closure=50;
            else if (primary && internal && (closed(l) || long_v)) closure=48;
            else closure=!closed(l) && !long_v?42:70;
        } else if (l==P(j) || l==P(w) || l==P(r)) closure=s?70:50;
        else closure=70;
        if (v==P(AT) && coda==P(n)) closure=63;
        else if (v==P(AT) && coda==P(l)) closure=
            !s && (vowel(l) || l==P(r))?50:l==P(l) || (vowel(l) && !closed(l))?42:28;
        release=v==P(A) || v==P(a) || v==P(AT) || v==P(Q_a_TILDE)?35:s?25:15;
        a->af=56;a->noise[1]=70;
        if (v==P(I) || v==P(i) || v==P(e) || v==P(y) || v==P(Y) || v==P(AT)) {
            a->af=63;a->noise[1]=0;a->noise[2]=60;
        } else if (v==P(E)) {a->af=63;a->noise[1]=65;}
        break;
    default:return 0;
    }
    if (grouped && s && c==P(k) && l==P(t)) closure=55;
    /* Inside a consonant onset the vowel starts voicing immediately
     * after the burst; it owns the tract transition. No aspirated source
     * plateau may replace that transition. */
    if (grouped && l!=P(p) && l!=P(t) && l!=P(k)) {
        if (c==P(p) && l==P(n)) closure=42;
        if (c==P(t) && (l==P(s)||l==P(S))) {closure=50;burst=8;}
        if (c==P(k) && fricative(l)) {
            closure=s?75:70;
            if (l==P(s)||l==P(S)) burst=8;
        }
        if (c==P(g) && l==P(l) && primary) closure=70;
        if (c==P(d) && l==P(f) && v==P(e) && !s) closure=30;
        if ((c==P(p)||c==P(t)||c==P(k)) && fricative(l))
            release=c==P(k)&&!primary?50:18;
        a->release_ah=-1;
    }
    if (c==P(b) || c==P(d) || c==P(g))
        a->closure_av=stop(l) || fricative(l) || (c==P(d) && l==P(l))?0:nasal(l)?35:20;
    else if (!grouped) a->release_ticks=release;
    a->body_ticks=closure+burst;a->burst_ticks=burst;p.incoming_ticks=release;
    struct reloq_german_phone_node tract=*n;
    tract.left_phone=l;
    /* The release duration uses the diphthong's initial vowel class, but
     * its tract retains the lexical diphthong identity (notably d + eu). */
    tract.right_phone=n->right_phone==P(Q_oj)?P(Q_oj):v;
    tract.same_syllable_left=0;
    /* A lexical coda r uses a reduced-vowel tract while retaining its
     * consonantal duration class above. */
    if (l==P(r)) tract.left_phone=P(R);
    if (reloq_german_obstruent_tract_plan(&tract,fresh,0,&a->tract)!=1) return 0;
    if (c==P(p)) {
        if (!internal) a->tract.departing[0]=400;
        if (l==P(t) || l==P(s) || l==P(X) || l==P(n) || l==P(l)) a->tract.entering[1]=1500;
    }
    if (c==P(t) && l==P(X)) {a->tract.entering[1]=1800;a->tract.entering[2]=2700;}
    if (grouped && c==P(k) && l==P(l)) a->tract.entering[0]=500;
    if (c==P(b) && l==P(X)) a->tract.entering[1]=1500;
    if (c==P(d)) {
        if (!internal && vowel(l)) {a->tract.entering[1]=1600;a->tract.entering[2]=2600;}
        if (l==P(X)) {a->tract.entering[1]=1800;a->tract.entering[2]=3000;}
        if (l==P(l)) a->tract.entering[1]=1250;
    }
    if (c==P(g)) {
        if (l==P(l)) a->tract.entering[1]=1200;
        if (l==P(r)) a->tract.entering[1]=1500;
    }
    /* A nasal supplies the incoming resonance. An absent knot preserves
     * that continuous transition instead of inserting a generic closure. */
    if (nasal(l) && !(c==P(p) && l==P(n)))
        for (unsigned j=0;j<3;++j) a->tract.entering[j]=-2;
    *out=p;return 1;
}

int reloq_german_stop_sonorant_phase(const struct reloq_german_phone_node *n,
    unsigned next_stress,struct reloq_german_stop_phase *out)
{
    if (!n || !out || next_stress>2) return -1;
    Phone c=left_endpoint(n->phone),r=n->right_phone,l=n->left_phone;
    if ((c!=P(p) && c!=P(t) && c!=P(k)) || !n->same_word_right ||
        !n->same_syllable_left || n->same_syllable_right ||
        !vowel(l) || n->affricate_role || (!nasal(r) && r!=P(l))) return 0;
    struct reloq_german_stop_phase p={0};
    struct reloq_german_onset_cluster *a=&p.gesture;
    struct reloq_german_phone_node tract=*n;
    tract.phone=c;tract.same_word_right=0;
    if (l==P(AT)) tract.left_phone=P(A);
    if (reloq_german_terminal_stop_tract_plan(&tract,&a->tract)!=1) return 0;
    a->release_ticks=r==P(l)?20:1;a->release_ah=-1;
    if (c==P(p)) {
        a->body_ticks=100;a->burst_ticks=10;a->af=60;
        a->noise[r==P(m)?1:5]=r==P(m)?44:54;
        a->tract.departing[1]=r==P(n)?1300:1000;
        if (l==P(AT)) {a->tract.entering[0]=300;a->tract.entering[1]=800;}
        if (r==P(l)) a->release_ah=53;
    } else if (c==P(t)) {
        a->body_ticks=r==P(l)?69:r==P(n)?75:102;a->burst_ticks=12;
        a->af=r==P(l)?56:r==P(n)?48:54;
        a->noise[4]=r==P(m)?51:r==P(n)?47:60;
        if (r==P(n)) a->noise[2]=47;
        a->tract.departing[1]=1600;a->tract.departing[2]=2500;
        if (r==P(l)) a->release_ah=51;
    } else {
        a->body_ticks=l==P(AT)?72:r==P(n)||r==P(l)||next_stress?87:70;
        a->burst_ticks=12;
        a->af=r==P(m)?50:r==P(l)?57:45;
        a->noise[1]=r==P(m)||r==P(l)?70:60;
        a->tract.departing[1]=r==P(n)?1800:1500;a->tract.departing[2]=2300;
        if (l==P(AT)) a->tract.entering[0]=300;
        if (r==P(l)) a->release_ah=50;
    }
    p.incoming_ticks=a->release_ticks;*out=p;return 1;
}

int reloq_german_stop_fricative_phase(const struct reloq_german_phone_node *n,
    unsigned next_stress,struct reloq_german_stop_phase *out)
{
    if (!n || !out || next_stress>2) return -1;
    const Phone c=left_endpoint(n->phone),r=n->right_phone,l=n->left_phone;
    if ((c!=P(p) && c!=P(t) && c!=P(k)) || !n->same_word_right ||
        !n->same_syllable_left || n->same_syllable_right ||
        !vowel(l) || n->affricate_role ||
        (r!=P(f) && r!=P(v) && r!=P(s) && r!=P(z) &&
         r!=P(S) && r!=P(x) && r!=P(X))) return 0;
    struct reloq_german_stop_phase p={0};
    struct reloq_german_onset_cluster *a=&p.gesture;
    struct reloq_german_phone_node tract=*n;
    tract.phone=c;tract.same_word_right=0;
    if (l==P(AT)) tract.left_phone=P(A);
    if (reloq_german_terminal_stop_tract_plan(&tract,&a->tract)!=1) return 0;
    a->release_ah=-1;
    if (c==P(p)) {
        a->body_ticks=100;a->burst_ticks=10;a->af=60;
        a->release_ticks=r==P(f)?15:1;
        a->noise[5]=r==P(f)?67:54;
        a->tract.departing[1]=r==P(f)?900:
            r==P(s)||r==P(z)||r==P(X)?1300:1000;
        if (l==P(AT)) {a->tract.entering[0]=300;a->tract.entering[1]=800;}
    } else if (c==P(t)) {
        a->body_ticks=r==P(f)||r==P(v)||r==P(S)?55:r==P(s)?71:69;
        a->burst_ticks=12;a->release_ticks=r==P(f)||r==P(v)?40:18;
        a->af=r==P(s)||r==P(S)||r==P(x)||r==P(X)?66:64;
        a->noise[4]=r==P(v)||r==P(S)?50:53;
        if (r==P(v)) a->noise[2]=45;
        if (r==P(X)) {a->noise[4]=0;a->noise[3]=47;}
        a->tract.departing[1]=r==P(f)||r==P(v)?1300:r==P(S)?2300:r==P(X)?1700:1600;
        a->tract.departing[2]=r==P(S)?2800:r==P(X)?2700:2500;
    } else {
        a->body_ticks=l==P(AT)||r==P(s)?68:next_stress||r==P(z)?83:66;
        a->burst_ticks=8;a->release_ticks=1;
        a->af=r==P(s)||r==P(S)?60:52;
        a->noise[1]=r==P(s)||r==P(S)?60:70;
        a->tract.departing[1]=r==P(s)||r==P(z)?1800:r==P(X)?2150:1500;
        a->tract.departing[2]=r==P(X)?2600:2300;
        if (l==P(AT) || l==P(O) || l==P(U)) a->tract.entering[0]=300;
    }
    p.incoming_ticks=a->release_ticks;*out=p;return 1;
}

int reloq_german_stop_pair_phase(const struct reloq_german_phone_node *n,
    unsigned next_stress,struct reloq_german_stop_phase *out)
{
    if (!n || !out || next_stress>2) return -1;
    const Phone c=n->phone,r=n->right_phone,l=n->left_phone;
    if ((c!=P(p) && c!=P(t) && c!=P(k)) || !stop(r) ||
        !n->same_word_left || !n->same_syllable_left || n->same_syllable_right ||
        n->affricate_role) return 0;
    /* The terminal constriction supplies the incoming tract; the following
     * consonant, rather than a vowel quality, selects its departing tract. */
    struct reloq_german_phone_node tract=*n;tract.same_word_right=0;
    if (l==P(Q_aj)) tract.left_phone=P(i);
    if (l==P(AT)) tract.left_phone=P(A);
    struct reloq_german_stop_phase p={0};
    struct reloq_german_onset_cluster *a=&p.gesture;
    if (reloq_german_terminal_stop_tract_plan(&tract,&a->tract)!=1 || l==P(l) || l==P(R)) return 0;
    if (l==P(AT)) {
        a->tract.entering[0]=300;
        if (c==P(p)) a->tract.entering[1]=800;
    }
    const int internal=n->same_word_right;
    a->release_ticks=1;a->release_ah=-1;
    if (c==P(p)) {
        a->body_ticks=r==P(p)?40:internal && !n->stress && l==P(Q_aj)?85:100;
        if (r!=P(p) && r!=P(b)) {
            a->burst_ticks=10;a->af=60;
            a->noise[5]=(r==P(k) || r==P(g)) && (internal || !front(l))?54:60;
        }
        if (r==P(t) || r==P(d)) a->tract.departing[1]=1300;
    } else if (c==P(t)) {
        a->body_ticks=r==P(t)?30:r==P(d)?
            (!internal && !closed(l) && l!=P(Q_aj)?50:next_stress?70:40):internal?42:58;
        if (r!=P(t) && r!=P(d)) {
            a->burst_ticks=internal?12:8;a->af=70;
            if (r==P(k) || r==P(g)) {a->noise[1]=50;a->noise[4]=40;}
            else a->noise[2]=50;
            if (r==P(k)) {a->release_ticks=30;a->release_ah=51;}
            else if (!internal && (r==P(p) || r==P(b))) a->release_ah=51;
        }
        a->tract.departing[1]=1600;a->tract.departing[2]=2500;
    } else {
        int closure=l==P(AT)?60:r==P(k) && next_stress?65:internal?52:40;
        if (r!=P(k) && r!=P(g)) {
            a->burst_ticks=internal?8:12;
            if (r==P(p) || r==P(b)) {a->af=50;a->noise[1]=70;}
            else {a->af=54;a->noise[1]=60;}
            if (r==P(t)) {a->release_ticks=20;a->release_ah=50;}
        }
        a->body_ticks=closure+a->burst_ticks;
        a->tract.departing[1]=r==P(t) || r==P(d)?1800:1500;
        a->tract.departing[2]=2300;
    }
    p.incoming_ticks=a->release_ticks;
    *out=p;return 1;
}
/* A word-final stop owns its complete closure and place release. These
 * decisions are independent of the following word's onset planner. */
struct stop_left_coeff {Phone phone;int16_t place[3][9];};
struct stop_right_coeff {Phone phone;int16_t place[3][12];};
#include "reloq_german_stop_edges.inc"
int reloq_german_stop_word_edge(const struct reloq_german_phone_node *n,
    unsigned next_stress,int final,struct reloq_german_stop_phase *out)
{
    if (!n || !out || next_stress>2 || (final!=0 && final!=1)) return -1;
    if (n->same_word_right || !n->same_word_left || !n->same_syllable_left || n->affricate_role) return 0;
    const Phone c=n->phone,original_left=n->left_phone,
        l=left_endpoint(original_left),r=final?P(NONE):n->right_phone;
    const int rv=vowel(r)||r==P(Q_aj)||r==P(Q_aw)||r==P(Q_oj);
    const int place=c==P(p)?0:c==P(t)?1:c==P(k)?2:-1;
    if (place<0) return 0;
    const struct stop_left_coeff *left=NULL;
    const struct stop_right_coeff *right=NULL;
    for (size_t i=0;i<sizeof left_coefficients/sizeof *left_coefficients;++i)
        if (left_coefficients[i].phone==original_left) {left=left_coefficients+i;break;}
    if (!left || !left->place[place][8]) return 0;
    if (!final) {
        for (size_t i=0;i<sizeof right_coefficients/sizeof *right_coefficients;++i)
            if (right_coefficients[i].phone==r) {right=right_coefficients+i;break;}
        if (!right) return 0;
    }
    struct reloq_german_stop_phase p={0};struct reloq_german_onset_cluster *a=&p.gesture;
    a->tract=(struct reloq_german_obstruent_tract){
        {300,1500,2500,place==1?3500:3600,place==1?4200:3900},
        {300,1500,2500,-1,-1},{120,100,150,330,260}};
    for (unsigned j=0;j<3;++j) {
        a->tract.entering[j]=left->place[place][j+(final?3:0)];
        a->tract.departing[j]=final?left->place[place][j+6]:right->place[place][j+9];
    }
    a->release_ah=-1;
    a->burst_ticks=place==0?10:place==1?8:(l==P(s)||l==P(S)?8:12);
    if (final) {
        a->af=place==0?60:place==1?(l==P(r)||l==P(R))?53:48:62;
        a->noise[place==0?5:place==1?4:1]=place==0?64:60;
        p.incoming_ticks=60;
    } else {
        const int16_t *v=right->place[place];
        a->release_ticks=v[0];a->release_ah=v[1]>0?v[1]:-1;a->af=v[2];
        for (unsigned j=0;j<6;++j) a->noise[j]=v[j+3];
        p.incoming_ticks=v[0];
    }
    int body=0;
    if (c==P(p)) {
        body=final?125:r==P(p)?40:l==P(m)||l==P(n)?52:
            n->stress==1 || vowel(l)?100:85;
        if (!final && (r==P(l)||r==P(r)) && l==P(S)) body=75;
        if (!final && (r==P(p)||r==P(b))) {a->burst_ticks=0;a->af=0;}
        if ((front(l)||l==P(l)) && a->noise[5] && (final || r!=P(f))) a->noise[5]=60;
        if (!final && r==P(r)) {
            a->release_ticks=fricative(l)?18:n->stress==1?(next_stress?63:50):
                n->stress==2?(next_stress?44:35):(next_stress?25:19);
        }
    } else if (c==P(t)) {
        const int nas=nasal(l),fr=fricative(l),sibil=l==P(s)||l==P(S);
        const int natural=nas?94:l==P(l)?65:l==P(r)?65:fr?48:65;
        if (final) body=l==P(p)?60:l==P(k)?80:nas?94:l==P(l)||l==P(t)||l==P(j)||l==P(w)?65:l==P(r)?93:
            sibil?88:fr?65:108;
        else if (rv) body=r==P(AT)||r==P(R)?42:!n->stress || l==P(n)||sibil?58:
            l==P(p)?60:l==P(k)?80:fr?65:natural;
        else if (r==P(t)) body=l==P(p)?52:l==P(k)?72:30;
        else if (r==P(d)) body=vowel(l)&&!closed(l)&&l!=P(Q_aj)?50:next_stress?70:40;
        else if (r==P(p)||r==P(b)||r==P(k)||r==P(g)) body=l==P(p)?60:l==P(k)?80:58;
        else if (r==P(s)) body=l==P(s)?68:l==P(n)?48:
            nas||fr||stop(l)||l==P(l)?53:67;
        else if (r==P(f)||r==P(v)||r==P(S)) body=l==P(p)?34:l==P(k)?44:l==P(t)?37:
            nas?40:l==P(l)?65:fr?(r==P(v)?54:48):51;
        else if (r==P(z)||r==P(Z)||r==P(x)||r==P(X)) body=l==P(p)?60:l==P(k)?80:nas?40:natural;
        else if (nasal(r)) body=l==P(p)?60:l==P(k)?80:
            vowel(l)?98:natural;
        else if (r==P(l)||r==P(r)||r==P(j)||r==P(w)) body=l==P(p)?60:l==P(k)?80:
            l==P(n)&&r==P(r)?44:natural;
        else return 0;
        if (!final && (r==P(t)||r==P(d))) {a->burst_ticks=0;a->af=0;}
        if (!final && r==P(m)) a->release_ticks=n->stress?50:15;
    } else {
        int closure;
        if (final) closure=l==P(AT)?60:110;
        else if (l==P(AT)) closure=60;
        else if (r==P(k)) closure=next_stress?65:40;
        else if (r==P(g)||r==P(p)||r==P(b)||r==P(t)||r==P(d)) closure=40;
        else if (r==P(s)) closure=60;
        else if (r==P(z)) closure=75;
        else if (rv) closure=l==P(n)||l==P(G)||stop(l)?55:l==P(m)?70:75;
        else if ((r==P(f)||r==P(v)||r==P(S)||r==P(x)||r==P(X)||r==P(m)||r==P(G)||r==P(r)) && stop(l)) closure=55;
        else closure=75;
        if (!final && (r==P(k)||r==P(g))) {a->burst_ticks=0;a->af=0;}
        body=closure+a->burst_ticks;
        if ((final || rv) && (front(l)||l==P(AT))) {
            a->noise[1]=0;a->noise[2]=60;
        }
        if (!final && rv && (r==P(A)||r==P(a)||r==P(Q_a_TILDE)))
            a->af=l==P(AT)?57:l==P(A)||l==P(a)||l==P(Q_a_TILDE)?65:front(l)?60:63;
    }
    if (!a->burst_ticks) for (unsigned j=0;j<6;++j) a->noise[j]=0;
    /* An unaccented final t keeps the f contact but contributes no
     * aspiration plateau. Accent and contact length are independent. */
    if (c==P(t) && r==P(f) && !n->stress) a->release_ah=-1;
    /* The following vowel owns a word-edge CV interval. CC contacts are
     * owned here; phrase releases remain in the shared pause composer. */
    if (!final && rv) a->release_ticks=0;
    a->body_ticks=body;*out=p;return 1;
}

/* A stop within a consonant group keeps three independent intervals:
 * constriction, burst and contact. The preceding gesture can omit an oral
 * target (notably after a nasal); that omission must survive a quiet closure.
 * Scalar place targets are shared with word-edge stops, while duration and
 * release depend on syllable grouping and the next phone's stress. */
int reloq_german_stop_vowel_boundary(const struct reloq_german_phone_node *n,
    unsigned next_stress,Phone vowel_coda,struct reloq_german_stop_phase *out)
{
    if (!n || !out || next_stress>2) return -1;
    /* The acoustic sequence ends a diphthong in j/w. Keep that closing
     * articulation when its coda releases into a new vowel syllable. */
    struct reloq_german_phone_node closing=*n;
    /* Secondary closed-vowel nuclei can absorb the contact in their oral
     * budget. That separate budget case is outside this diphthong-coda
     * extension; retain its existing path until that ownership is ported. */
    if((closing.left_phone==P(Q_aj) || closing.left_phone==P(Q_aw) ||
        closing.left_phone==P(Q_oj)) && next_stress==2 && closed(n->right_phone))return 0;
    if(closing.left_phone==P(Q_aj) || closing.left_phone==P(Q_oj))closing.left_phone=P(j);
    else if(closing.left_phone==P(Q_aw))closing.left_phone=P(w);
    n=&closing;
    const Phone c=n->phone,l=n->left_phone,
        v=n->right_phone==P(Q_aj)||n->right_phone==P(Q_aw)?P(A):
          n->right_phone==P(Q_oj)?P(O):n->right_phone;
    if (n->affricate_role || !n->same_word_left || !n->same_word_right ||
        !n->same_syllable_left || n->same_syllable_right ||
        (c!=P(p)&&c!=P(t)&&c!=P(k)) || !vowel(v) ||
        (!vowel(l)&&!nasal(l)&&l!=P(l)&&l!=P(r)&&l!=P(j)&&l!=P(w)&&!fricative(l)&&!stop(l))) return 0;
    struct reloq_german_phone_node q=*n;
    q.same_syllable_left=0;q.same_syllable_right=1;
    q.stress=c==P(k)?next_stress:n->stress;
    struct reloq_german_stop_phase p;
    if (reloq_german_stop_phase_plan(&q,vowel_coda,0,&p)!=1) return 0;
    struct reloq_german_onset_cluster *a=&p.gesture;
    const int fr=fricative(l),sibil=l==P(s)||l==P(S);
    a->release_ticks=0;a->release_ah=-1;
    const int long_next=closed(v) && next_stress;
    if (c==P(p)) {
        a->body_ticks=l==P(n)||l==P(m)?52:vowel(l)||n->stress==1?100:85;
        a->burst_ticks=10;a->af=60;a->noise[5]=54;
        p.incoming_ticks=n->stress==1?(long_next?63:50):n->stress==2?(long_next?44:35):(long_next?25:19);
        if (n->stress==1&&(v==P(U)||v==P(u)))p.incoming_ticks=p.incoming_ticks*7/10;
    } else if (c==P(t)) {
        a->burst_ticks=sibil?8:front(v)?18:12;
        int closure=l==P(n)?32:sibil?50:l==P(p)?52:l==P(k)?72:57;
        if(v==P(AT)&&vowel_coda==P(n))closure=sibil?72:l==P(f)||l==P(X)?76:l==P(l)?45:38;
        else if((v==P(R)||(v==P(AT)&&vowel_coda==P(NONE)))&&!sibil)closure=34;
        a->body_ticks=closure+a->burst_ticks;
        a->af=56;a->noise[4]=56;
        p.incoming_ticks=n->stress==1?(long_next?90:72):n->stress==2?(long_next?63:50):(long_next?36:28);
        if(n->stress==1&&(v==P(U)||v==P(u)))p.incoming_ticks=p.incoming_ticks*7/10;
        if(nasal(l)&&next_stress==2)p.incoming_ticks=40;
    } else {
        a->burst_ticks=sibil?8:12;
        a->body_ticks=(l==P(n)||l==P(G)?55:l==P(m)?70:l==P(AT)?60:next_stress?75:58)+a->burst_ticks;
        const int low=l==P(A)||l==P(a),forward=front(v);
        a->af=low?(forward?62:65):front(l)?60:l==P(AT)?57:63;
        a->noise[1]=front(l)||l==P(AT)?0:low&&forward?62:70;
        a->noise[2]=front(l)||l==P(AT)?60:0;
        p.incoming_ticks=n->stress==1?(long_next?76:61):n->stress==2?(long_next?53:42):(long_next?30:23);
        if (v==P(U)||v==P(u))p.incoming_ticks=61;
        else if (v==P(o))p.incoming_ticks=next_stress==1?76:61;
    }
    if(fr)p.incoming_ticks=18;
    /* The coda keeps its measured closing place, while the new nucleus
     * provides only the departing place. Nasal contacts preserve absent
     * entering knots instead of inserting a default consonant target. */
    struct reloq_german_phone_node edge=*n;
    edge.same_word_right=0;
    struct reloq_german_stop_phase crossing;
    if(reloq_german_stop_word_edge(&edge,next_stress,0,&crossing)==1) {
        memcpy(a->tract.entering,crossing.gesture.tract.entering,sizeof a->tract.entering);
        memcpy(a->tract.departing,crossing.gesture.tract.departing,sizeof a->tract.departing);
    }
    if(nasal(l) && !(c==P(p)&&l==P(n)))
        for(unsigned j=0;j<3;++j)a->tract.entering[j]=-2;
    if(c==P(k)&&l==P(l))a->tract.entering[0]=500;
    if(c==P(t)&&l==P(r)) {a->tract.entering[1]=1400;a->tract.entering[2]=2350;}
    /* The closing tract retains the preceding a gesture even when the
     * next vowel owns a new syllable; onset defaults would erase it. */
    if (l==P(A)) {
        if (c==P(p)) {
            a->tract.entering[0]=500;a->tract.entering[1]=900;
            a->tract.departing[0]=300;
        } else if (c==P(k)) {
            a->tract.entering[0]=500;a->tract.departing[0]=300;
        } else if (c==P(t) && v==P(U)) {
            a->tract.departing[1]=1600;a->tract.departing[2]=2500;
        }
    }
    *out=p;return 1;
}

int reloq_german_stop_coda_phase(const struct reloq_german_phone_node *n,
    unsigned next_stress,int next_final,Phone after_next,struct reloq_german_stop_phase *out)
{
    if (!n || !out || next_stress>2 || (next_final!=0 && next_final!=1)) return -1;
    if (!n->same_word_left || !n->same_word_right || !n->same_syllable_left ||
        n->affricate_role) return 0;
    /* A retained voiced closure before final n has its own source phase.
     * The phonetic adjustment has already decided whether b/d/g survives;
     * mapping its place to p/t/k here must not devoice it again. */
    if(next_final && n->same_syllable_right && n->right_phone==P(n) &&
       (vowel(n->left_phone)||n->left_phone==P(j)||n->left_phone==P(w)||
        (n->left_phone>=P(Q_aj)&&n->left_phone<=P(Q_oj))) &&
       (n->phone==P(b)||n->phone==P(d)||n->phone==P(g))) {
        struct reloq_german_stop_phase voiced={0};
        struct reloq_german_onset_cluster *a=&voiced.gesture;
        if(reloq_german_obstruent_tract_plan(n,0,0,&a->tract)!=1)return 0;
        const int bilabial=n->phone==P(b),velar=n->phone==P(g);
        a->body_ticks=bilabial?51:velar?80:50;
        a->burst_ticks=bilabial?11:10;
        a->closure_av=20;a->tl_step=1;
        a->af=bilabial?56:velar?60:42;
        a->noise[bilabial?5:velar?1:4]=bilabial?64:velar?58:60;
        a->release_ticks=velar?10:1;a->release_ah=-1;
        voiced.incoming_ticks=a->release_ticks;
        *out=voiced;return 1;
    }
    /* The text analyser can retain a voiced d across a syllable boundary
     * before n. Its closure is still voiced; routing its place through the
     * final-t table discards both the source and the dental release. */
    if(n->phone==P(d) && n->right_phone==P(n) && !n->same_syllable_right &&
       (vowel(n->left_phone) || n->left_phone==P(r))) {
        struct reloq_german_stop_phase voiced={0};
        struct reloq_german_onset_cluster *a=&voiced.gesture;
        if(reloq_german_obstruent_tract_plan(n,0,0,&a->tract)!=1)return 0;
        a->body_ticks=50;a->burst_ticks=10;a->closure_av=20;a->tl_step=1;
        a->af=42;a->noise[4]=60;a->release_ticks=1;a->release_ah=-1;
        voiced.incoming_ticks=1;
        *out=voiced;return 1;
    }
    const Phone c=left_endpoint(n->phone),l=left_endpoint(n->left_phone),
        r=next_final?left_endpoint(n->right_phone):n->right_phone;
    if ((c!=P(p) && c!=P(t) && c!=P(k)) ||
        (!stop(r) && !fricative(r) && r!=P(v) && r!=P(z) && !nasal(r) &&
         r!=P(l) && r!=P(r))) return 0;
    /* A same-syllable liquid has its own onset-group planner. */
    if (n->same_syllable_right && (r==P(l)||r==P(r))) return 0;
    struct reloq_german_phone_node edge=*n;
    edge.phone=c;edge.right_phone=r;edge.same_word_right=0;
    struct reloq_german_stop_phase p;
    if (reloq_german_stop_word_edge(&edge,next_stress,0,&p)!=1) return 0;
    struct reloq_german_onset_cluster *a=&p.gesture;
    const int sibil=l==P(s)||l==P(S),fr=fricative(l),nas=nasal(l);
    const int same=n->same_syllable_right;
    a->release_ah=-1;
    if (c==P(p)) {
        a->body_ticks=r==P(p)?40:l==P(m)||l==P(n)?52:
            n->stress==1 || vowel(l)?100:85;
        if ((r==P(l)||r==P(r)) && l==P(S)) a->body_ticks=75;
        a->burst_ticks=r==P(p)||r==P(b)?0:10;
        a->release_ticks=r==P(f)?(same?10:15):
            r==P(l)?20:r==P(r)?(fr?18:n->stress==1?(next_stress?63:50):
                n->stress==2?(next_stress?44:35):(next_stress?25:19)):
            same && fricative(r)?10:1;
        if (same && r==P(f)) a->body_ticks=60;
        /* ger_ph_p_dur keeps short-vowel and m/n closures, but contracts
         * a primary-stress closure before a continuing fricative group.
         * The lookahead remains within the word, not just the syllable. */
        const int short_left=l==P(A)||l==P(E)||l==P(I)||l==P(O)||
            l==P(U)||l==P(Y)||l==P(AT)||l==P(R)||l==P(Q_OE);
        if (same && (r==P(s)||r==P(S))) {
            if (!short_left && l!=P(m) && l!=P(n))
                a->body_ticks=n->stress!=1?85:
                    after_next!=P(NONE)&&!vowel(after_next)?50:100;
        }
        if (same && r==P(f) && after_next!=P(NONE) && !vowel(after_next))
            a->body_ticks=44;
        a->af=a->burst_ticks?60:0;
        a->noise[5]=r==P(f)?67:r==P(t)||r==P(d)?60:54;
        if (r==P(m)) a->noise[5]=0;
        if (same && r==P(s)) {a->noise[5]=0;a->noise[1]=42;}
        if (r==P(l)) a->release_ah=53;
        if (l==P(l)) a->tract.entering[1]=1500;
    } else if (c==P(t)) {
        a->burst_ticks=r==P(t)||r==P(d)?0:sibil?8:
            same && (r==P(s)||r==P(S))?5:12;
        int closure=nas?32:l==P(l)?57:fr?40:57;
        if (stop(r)) closure=r==P(d)?(next_stress?70:40):
            r==P(t)?(l==P(p)?52:l==P(k)?72:30):
            l==P(p)?31:l==P(k)?43:30;
        else if (r==P(f)||r==P(v)||r==P(S))
            closure=l==P(p)?26:l==P(k)?36:l==P(l)?57:nas?32:fr?(r==P(v)?46:40):same?57:43;
        else if (r==P(s)) closure=same?(sibil?65:l==P(n)?36:
                l==P(p)||l==P(k)?45:nas||fr||l==P(l)?65:59):
            l==P(s)?60:l==P(n)?40:nas||fr||l==P(l)||stop(l)?45:59;
        else if (r==P(m)||r==P(G)) closure=vowel(l)?90:
            l==P(p)?52:l==P(k)?72:fr?40:57;
        else if (r==P(n)) closure=vowel(l)?63:
            l==P(p)?36:l==P(k)?50:fr?28:40;
        else if (r==P(l)||r==P(r)) closure=
            l==P(p)?52:l==P(k)?72:r==P(r)&&l==P(n)?36:fr?40:57;
        else if (l==P(p)) closure=52;
        else if (l==P(k)) closure=72;
        else if (r==P(r) && l==P(n)) closure=36;
        a->body_ticks=closure+a->burst_ticks;
        a->release_ticks=r==P(k)?30:r==P(f)||r==P(v)?(same?10:40):
            fricative(r)||r==P(z)?(same?10:18):r==P(l)||r==P(r)?20:1;
        if (r==P(k)) a->release_ah=51;

        if (r==P(l)) a->release_ah=51;
        if (r==P(n)) {a->af=48;a->noise[2]=47;a->noise[4]=47;}
    } else {
        const int release_stop=stop(r),reduced=l==P(AT);
        int closure;
        a->burst_ticks=r==P(k)||r==P(g)?0:
            release_stop||fricative(r)||r==P(v)||r==P(z)||sibil?8:12;
        if (reduced) closure=60;
        else if (release_stop) closure=r==P(k)&&next_stress?65:same?(nas?55:65):52;
        else if (r==P(s)) closure=60;
        else if (r==P(z)) closure=75;
        else if (r==P(n)) closure=same?56:75;
        else if (r==P(l)) closure=75;
        else closure=next_stress?(stop(l)?55:75):58;
        a->body_ticks=closure+a->burst_ticks;
        a->release_ticks=r==P(t)?20:r==P(l)||r==P(r)?20:
            same && (fricative(r)||r==P(v)||r==P(z))?10:1;
        if (r==P(t)) a->release_ah=50;
        if (r==P(l)) a->release_ah=50;
        if (same && (r==P(p)||r==P(b)||r==P(f)||r==P(v)||r==P(x)||r==P(X)||r==P(m)||r==P(n))) {a->af=65;a->noise[1]=60;}
        if (same && (r==P(s)||r==P(S))) a->noise[2]=60;
    }
    if (!a->burst_ticks) {a->af=0;for (unsigned j=0;j<6;++j) a->noise[j]=0;}
    p.incoming_ticks=a->release_ticks;*out=p;return 1;
}

#undef P
