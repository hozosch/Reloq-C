#include "reloq_german_obstruent_tract.h"

typedef enum reloq_german_phone Phone;
#define P(n) RELOQ_DE_PHONE_##n

static int vowel(Phone p)
{
    return (p>=P(a) && p<=P(AT)) || p==P(R) || p>=P(Q_E_COLON);
}
static int front(Phone p)
{
    return p==P(e) || p==P(i) || p==P(E) || p==P(I) || p==P(y) ||
        p==P(Y) || p==P(Q_E_COLON) || p==P(Q_oe) || p==P(Q_OE) ||
        p==P(Q_E_TILDE) || p==P(j);
}
static int close_front(Phone p) {return p==P(e) || p==P(i) || p==P(y);}
static int low(Phone p) {return p==P(A) || p==P(a) || p==P(Q_a_TILDE);}
static int high_back(Phone p)
{
    return p==P(U) || p==P(u) || p==P(o) || p==P(Q_o_TILDE);
}
static Phone endpoint(Phone p,int left)
{
    if (p==P(Q_aj) || p==P(Q_aw)) return left?(p==P(Q_aj)?P(j):P(w)):P(A);
    if (p==P(Q_oj)) return left?P(j):p;
    return p;
}
static void pair(int32_t f[5],int f2,int f3) {f[1]=f2;f[2]=f3;}

int reloq_german_obstruent_tract_plan(const struct reloq_german_phone_node *n,
    int initial,int final,struct reloq_german_obstruent_tract *out)
{
    if (!n || !out || (initial!=0 && initial!=1) || (final!=0 && final!=1)) return -1;
    const int lateral_final=final && n->left_phone==P(l) &&
        (n->phone==P(f) || n->phone==P(s));
    const int stop=n->phone==P(b) || n->phone==P(d) || n->phone==P(g) ||
        n->phone==P(p) || n->phone==P(t) || n->phone==P(k);
    const int stop_onset=stop && !final && !n->same_syllable_left && n->same_syllable_right &&
        n->same_word_right && vowel(n->right_phone);
    const int voiced_nasal=n->same_syllable_left && n->same_syllable_right &&
        n->right_phone==P(n) &&
        (vowel(n->left_phone)||n->left_phone==P(j)||n->left_phone==P(w)) &&
        (n->phone==P(b)||n->phone==P(d)||n->phone==P(g));
    const int dental_nasal=n->phone==P(d) && n->right_phone==P(n) &&
        n->same_word_right && !n->same_syllable_right &&
        (vowel(n->left_phone) || n->left_phone==P(r));
    /* Fricatives use neighboring articulation classes across consonant
     * groups and word edges too. Their old vowel-only gate bypassed the
     * detailed planner; the separately scoped stop gate remains below. */
    if (stop && !dental_nasal && !voiced_nasal && ((!initial && !stop_onset &&
         (!n->same_word_left || (!vowel(n->left_phone) && !lateral_final))) ||
        (!final && (!n->same_word_right || !vowel(n->right_phone))))) return 0;
    const Phone c=n->phone,l=initial?P(NONE):endpoint(n->left_phone,1),
        r=final?P(NONE):endpoint(n->right_phone,0);
    if (stop && !dental_nasal && !voiced_nasal && (final || !n->same_syllable_right)) return 0;
    struct reloq_german_obstruent_tract p={
        {300,1500,2600,3600,3900},{300,1500,2600,-1,-1},{120,100,150,330,260}
    };
    switch (c) {
    case P(p):
        pair(p.entering,1000,2500);pair(p.departing,1000,2500);
        if (initial) p.departing[0]=400;
        if (l==P(A) || l==P(a) || l==P(AT)) p.entering[1]=800;
        else if (l==P(e) || l==P(i) || l==P(y) || l==P(j)) p.entering[1]=1500;
        else if (l==P(E) || l==P(I) || l==P(Y) || l==P(Q_OE) || l==P(Q_E_COLON)) p.entering[2]=2200;
        else if (l==P(O)) p.entering[1]=700;
        else if (high_back(l) || l==P(w)) pair(p.entering,600,2100);
        if (r==P(A) || r==P(a)) p.departing[1]=800;
        else if (r==P(E) || r==P(y)) p.departing[1]=1300;
        else if (r==P(I) || r==P(e) || r==P(i) || r==P(Q_E_COLON)) p.departing[1]=1500;
        else if (r==P(O) || r==P(Q_oj)) p.departing[1]=700;
        else if (high_back(r)) p.departing[1]=600;
        break;
    case P(t):
        pair(p.entering,1600,2500);pair(p.departing,1600,2500);
        p.entering[3]=3500;p.entering[4]=4200;
        if (l==P(e) || l==P(i) || l==P(j)) pair(p.entering,1900,2700);
        else if (l==P(I)) pair(p.entering,1800,2700);
        else if (front(l)) p.entering[1]=1800;
        else if (l==P(O)) p.entering[1]=1400;
        else if (high_back(l)) pair(p.entering,1300,2050);
        if (front(r)) pair(p.departing,1700,2700);
        if (r==P(u)) p.departing[0]=280;
        break;
    case P(k):
        pair(p.entering,1500,2300);pair(p.departing,1600,2100);
        if (l==P(A) || l==P(a) || l==P(AT)) pair(p.entering,1750,2200);
        else if (l==P(i) || l==P(j)) pair(p.entering,2200,2400);
        else if (l==P(e)) pair(p.entering,1900,2500);
        else if (l==P(Q_OE)) p.entering[1]=1300;
        else if (front(l)) p.entering[1]=1900;
        else if (l==P(o)) p.entering[1]=960;
        else if (l==P(U) || l==P(u) || l==P(w)) p.entering[1]=1000;
        if (r==P(e) || r==P(i)) pair(p.departing,2200,2850);
        else if (r==P(I)) pair(p.departing,2200,2700);
        else if (front(r)) pair(p.departing,2150,2600);
        else if (r==P(O) || r==P(Q_oj)) pair(p.departing,1400,2000);
        else if (high_back(r)) pair(p.departing,1250,2000);
        break;
    case P(b):
        p.entering[0]=p.departing[0]=200;p.entering[4]=4000;
        pair(p.entering,1000,2500);pair(p.departing,1000,2500);
        if (l==P(e) || l==P(i)) p.entering[1]=1600;
        else if (l==P(y) || l==P(j) || l==P(Q_E_COLON)) p.entering[1]=1500;
        else if (l==P(E) || l==P(I) || l==P(Q_E_TILDE)) p.entering[1]=1250;
        else if (l==P(AT) || low(l)) p.entering[1]=800;
        else if (l==P(R)) p.entering[1]=900;
        else if (l==P(o) || l==P(u) || l==P(Q_o_TILDE)) pair(p.entering,600,2100);
        else if (l==P(O) || l==P(U) || l==P(w)) pair(p.entering,700,2200);
        if (close_front(r) || r==P(I) || r==P(Q_E_COLON)) p.departing[1]=1500;
        else if (r==P(E) || r==P(Q_E_COLON) || r==P(Q_E_TILDE)) p.departing[1]=1300;
        else if (r==P(o) || r==P(u) || r==P(Q_o_TILDE)) p.departing[1]=600;
        else if (r==P(O) || r==P(U) || r==P(Q_oj)) p.departing[1]=700;
        break;
    case P(d):
        p.entering[0]=p.departing[0]=200;p.entering[3]=4000;p.entering[4]=4300;
        pair(p.entering,1600,2600);pair(p.departing,1600,2600);
        if (l==P(e) || l==P(i)) pair(p.entering,1800,2700);
        else if (l==P(U) || l==P(o) || l==P(u)) p.entering[1]=1500;
        if (r==P(i)) pair(p.departing,1800,2700);
        else if (r==P(e) || r==P(I) || r==P(y) || r==P(Q_oe) || r==P(AT)) pair(p.departing,1700,2700);
        else if (r==P(o) || r==P(u)) p.departing[1]=1200;
        else if (r==P(U) || r==P(Q_oj)) p.departing[1]=1400;
        break;
    case P(g):
        pair(p.entering,1500,1890);pair(p.departing,1600,voiced_nasal?2500:2300);
        if (l==P(a)) p.entering[0]=400;
        if (l==P(AT)) pair(p.entering,1800,2050);
        else if (l==P(I)) pair(p.entering,2050,2500);
        else if (l==P(e) || l==P(i)) pair(p.entering,2200,2600);
        else if (l==P(y) || l==P(j)) pair(p.entering,2000,2600);
        else if (front(l)) p.entering[1]=1600;
        else if (l==P(w)) p.entering[1]=1100;
        else if (high_back(l) || l==P(O) || l==P(R) || l==P(Q_oe_TILDE)) p.entering[1]=1200;
        if (r==P(AT)) p.departing[1]=1900;
        else if (r==P(e) || r==P(i)) pair(p.departing,2250,2450);
        else if (r==P(I) || r==P(y) || r==P(Y)) pair(p.departing,2150,2400);
        else if (front(r)) p.departing[1]=2000;
        else if (r==P(O) || r==P(Q_oj)) pair(p.departing,1420,2000);
        else if (r==P(U) || r==P(o)) pair(p.departing,1300,2000);
        else if (r==P(u)) pair(p.departing,900,2000);
        else if (r==P(Q_o_TILDE) || r==P(Q_oe_TILDE)) p.departing[2]=2000;
        break;
    case P(f): case P(v): {
        const int voiced=c==P(v);
        p.entering[0]=p.departing[0]=voiced?300:400;
        pair(p.entering,1000,2300);pair(p.departing,1000,2300);
        if (l==P(l)) p.entering[1]=1200;
        if (voiced) {
            p.bandwidth[1]=95;p.bandwidth[2]=125;
            if (close_front(l) || l==P(j) || l==P(X)) pair(p.entering,1500,2500);
            else if (l==P(I)) pair(p.entering,1350,2500);
        } else if (front(l) || l==P(X)) pair(p.entering,1450,2500);
        if (high_back(l) || (!voiced && l==P(w))) p.entering[1]=750;
        if (close_front(r) || r==P(I) || r==P(j) || r==P(X))
            pair(p.departing,voiced?1500:1400,2500);
        else if (low(r)) p.departing[1]=voiced?800:920;
        else if (r==P(O) || r==P(Q_oj)) p.departing[1]=voiced?800:820;
        else if (high_back(r)) p.departing[1]=voiced?750:820;
        else if (!voiced && r==P(Q_oe)) p.departing[2]=2100;
        break;
    }
    case P(s):
        p.entering[3]=3400;p.entering[4]=4400;
        if (close_front(l) || l==P(Q_oe)) p.entering[1]=1800;
        if (r==P(e) || r==P(i) || r==P(I) || r==P(j)) p.departing[1]=1760;
        if (r==P(U) || r==P(o) || r==P(u)) p.departing[4]=4300;
        break;
    case P(z):
        p.entering[3]=3700;p.entering[4]=4200;
        if (l==P(u) || l==P(w)) p.entering[1]=1210;
        if (low(r)) p.departing[1]=1390;
        else if (r==P(o) || r==P(u) || r==P(Q_o_TILDE)) p.departing[1]=1210;
        break;
    case P(S):
        pair(p.entering,1800,2400);pair(p.departing,1800,2400);
        p.departing[0]=-1;p.departing[3]=3600;p.entering[4]=4000;
        if (l==P(e) || l==P(i)) pair(p.entering,2100,2600);
        else if (l==P(o) || l==P(u)) pair(p.entering,1600,2200);
        if (r==P(e) || r==P(i)) pair(p.departing,2100,2600);
        else if (r==P(u) || r==P(U)) pair(p.departing,1400,2200);
        else if (r==P(o) || r==P(Q_o_TILDE)) pair(p.departing,1600,2200);
        else if (r==P(p) || r==P(b) || r==P(f) || r==P(v) || r==P(m))
            p.departing[1]=1500;
        if (l==P(s) || l==P(S)) {
            p.entering[1]=1800;p.entering[2]=2400;
        }
        break;
    case P(Z):
        pair(p.entering,1700,2300);p.entering[4]=4000;
        p.departing[0]=p.departing[1]=p.departing[2]=-1;
        break;
    case P(X):
        pair(p.entering,2150,2900);pair(p.departing,2150,-1);
        p.entering[3]=3500;p.entering[4]=4500;p.departing[0]=-1;
        switch (l) {
        case P(E): case P(Q_E_TILDE): case P(Q_oe): p.entering[1]=1880;break;
        case P(I): pair(p.entering,2200,3100);break;
        case P(e): pair(p.entering,2430,3100);break;
        case P(i): pair(p.entering,2500,3100);break;
        case P(Y): pair(p.entering,1740,2350);break;
        case P(y): p.entering[1]=2050;break;
        case P(Q_OE): case P(Q_oe_TILDE): p.entering[1]=1720;break;
        case P(j): p.entering[1]=2350;break;
        case P(t): case P(d): case P(s): case P(z): case P(n): case P(l):
        case P(T): p.entering[1]=2430;break;
        default:break;
        }
        switch (r) {
        case P(A): p.departing[1]=1700;break;
        case P(a): p.departing[1]=2150;break;
        case P(E): p.departing[1]=2080;break;
        case P(Q_E_COLON): p.departing[1]=2150;break;
        case P(Q_E_TILDE): case P(y): p.departing[1]=2060;break;
        case P(Q_OE): case P(Q_oe_TILDE): p.departing[1]=1720;break;
        case P(Y): p.departing[1]=1500;break;
        case P(e): p.departing[1]=2430;break;
        case P(i): pair(p.departing,2300,3100);p.departing[3]=3600;break;
        case P(Q_oe): p.departing[1]=1880;break;
        case P(j): p.departing[1]=2500;break;
        default:break;
        }
        break;
    case P(x):
        pair(p.entering,1350,-1);pair(p.departing,1350,-1);p.departing[0]=-1;
        if (l==P(A) || l==P(a)) p.entering[1]=1220;
        else if (l==P(R)) p.entering[1]=1400;
        else if (l==P(o)) p.entering[1]=620;
        else if (l==P(u)) p.entering[1]=750;
        else if (l==P(O) || l==P(U) || l==P(w) || l==P(Q_o_TILDE) || l==P(Q_oe_TILDE)) p.entering[1]=900;
        if (r==P(A)) p.departing[1]=1220;
        else if (r==P(a) || r==P(R)) p.departing[1]=1400;
        else if (r==P(O) || r==P(Q_oj)) p.departing[1]=1040;
        else if (r==P(o)) p.departing[1]=620;
        else if (r==P(u) || r==P(U) || r==P(Q_o_TILDE)) p.departing[1]=900;
        else if (!vowel(r)) p.departing[1]=p.entering[1];
        if (final) p.departing[1]=1100;
        break;
    default:return 0;
    }
    /* Fricatives use the available endpoint at a phrase boundary. Stops
     * retain a distinct closed tract before their release into the vowel. */
    if (!stop && initial) for (unsigned j=0;j<5;++j)
        if (p.departing[j]>0) p.entering[j]=p.departing[j];
    if (final && c!=P(x)) for (unsigned j=1;j<5;++j) p.departing[j]=p.entering[j];
    *out=p;return 1;
}

int reloq_german_obstruent_tract_append(const struct reloq_german_obstruent_tract *p,
    int64_t begin,int64_t end,size_t index,reloq_german_source_emit emit,void *context)
{
    if (!p || !emit || begin<0 || end<begin) return -1;
    for (unsigned j=0;j<5;++j) {
        int a=p->entering[j],b=p->departing[j];
        if (end-begin<16 && a>0 && b>0) {a=(a+b)/2;b=-1;}
        if (b<0) b=a;
        if ((a>0 && emit(context,(enum reloq_target_stream)(RELOQ_TARGET_F1+j),begin,a,index)) ||
            (b>0 && emit(context,(enum reloq_target_stream)(RELOQ_TARGET_F1+j),end,b,index)) ||
            emit(context,(enum reloq_target_stream)(RELOQ_TARGET_B1+j),begin,p->bandwidth[j],index) ||
            emit(context,(enum reloq_target_stream)(RELOQ_TARGET_B1+j),end,p->bandwidth[j],index)) return -1;
    }
    return 0;
}
#undef P
