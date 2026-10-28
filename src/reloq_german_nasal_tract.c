#include "reloq_german_nasal_tract.h"

typedef enum reloq_german_phone Phone;
static int high_front(Phone p)
{
    return p==RELOQ_DE_PHONE_i || p==RELOQ_DE_PHONE_I || p==RELOQ_DE_PHONE_e;
}
static int back(Phone p)
{
    return p==RELOQ_DE_PHONE_o || p==RELOQ_DE_PHONE_O ||
        p==RELOQ_DE_PHONE_u || p==RELOQ_DE_PHONE_U || p==RELOQ_DE_PHONE_Q_o_TILDE;
}
static int front(Phone p)
{
    return high_front(p) || p==RELOQ_DE_PHONE_E || p==RELOQ_DE_PHONE_y ||
        p==RELOQ_DE_PHONE_Y || p==RELOQ_DE_PHONE_Q_E_COLON ||
        p==RELOQ_DE_PHONE_Q_oe || p==RELOQ_DE_PHONE_Q_OE ||
        p==RELOQ_DE_PHONE_Q_E_TILDE;
}
static Phone left_endpoint(Phone p)
{
    if (p==RELOQ_DE_PHONE_Q_aj || p==RELOQ_DE_PHONE_Q_oj) return RELOQ_DE_PHONE_j;
    if (p==RELOQ_DE_PHONE_Q_aw) return RELOQ_DE_PHONE_w;
    if (p==RELOQ_DE_PHONE_P) return RELOQ_DE_PHONE_f;
    if (p==RELOQ_DE_PHONE_T) return RELOQ_DE_PHONE_s;
    if (p==RELOQ_DE_PHONE_C) return RELOQ_DE_PHONE_S;
    if (p==RELOQ_DE_PHONE_J) return RELOQ_DE_PHONE_Z;
    return p;
}
static Phone right_endpoint(Phone p)
{
    if (p==RELOQ_DE_PHONE_Q_aj || p==RELOQ_DE_PHONE_Q_aw) return RELOQ_DE_PHONE_A;
    return p;
}

int reloq_german_nasal_tract_plan(const struct reloq_german_phone_node *n,
    int initial,int final,struct reloq_german_nasal_tract *out)
{
    if (!n || !out || (initial!=0 && initial!=1) || (final!=0 && final!=1)) return -1;
    const int velar=n->phone==RELOQ_DE_PHONE_G;
    const int labial=n->phone==RELOQ_DE_PHONE_m;
    if (!velar && !labial && n->phone!=RELOQ_DE_PHONE_n) return 0;
    Phone l=initial?RELOQ_DE_PHONE_NONE:left_endpoint(n->left_phone);
    Phone r=final?RELOQ_DE_PHONE_NONE:right_endpoint(n->right_phone);
    if (labial) {
        /* m uses the same place gesture in onsets and codas, including
         * consonant contacts and word edges. Phrase-final m closes F1 and
         * sustains the entering F2; it is not an onset-shaped generic nasal.
         * ger_ph_m / doF, checked across 0.2.35 nasal edge observations. */
        struct reloq_german_nasal_tract m={
            {300,1000,2500,3600,3900},{-1,1000,-1,-1,-1},{120,150,105,330,260}
        };
        if (l==RELOQ_DE_PHONE_a || l==RELOQ_DE_PHONE_A) m.entering[1]=750;
        else if (l==RELOQ_DE_PHONE_e || l==RELOQ_DE_PHONE_i ||
                 l==RELOQ_DE_PHONE_X) m.entering[1]=1300;
        else if (n->left_phone==RELOQ_DE_PHONE_Q_aj || n->left_phone==RELOQ_DE_PHONE_Q_oj) m.entering[1]=1100;
        else if (back(l) || l==RELOQ_DE_PHONE_R) m.entering[1]=700;
        else if (l==RELOQ_DE_PHONE_r) m.entering[1]=900;
        if (front(r) || r==RELOQ_DE_PHONE_X) m.departing[1]=1300;
        else if (r==RELOQ_DE_PHONE_r) m.departing[1]=850;
        else if (back(r) || r==RELOQ_DE_PHONE_Q_oe_TILDE ||
                 r==RELOQ_DE_PHONE_R || r==RELOQ_DE_PHONE_Q_oj) m.departing[1]=700;
        if (l==RELOQ_DE_PHONE_u || l==RELOQ_DE_PHONE_U || l==RELOQ_DE_PHONE_o ||
            back(r) || r==RELOQ_DE_PHONE_Q_oe_TILDE || r==RELOQ_DE_PHONE_R ||
            r==RELOQ_DE_PHONE_Q_oj || n->left_phone==RELOQ_DE_PHONE_Q_aj ||
            n->left_phone==RELOQ_DE_PHONE_Q_oj) m.entering[2]=2300;
        if (initial) m.entering[1]=m.departing[1];
        if (final) {m.entering[0]=200;m.departing[1]=m.entering[1];}
        *out=m;return 1;
    }
    struct reloq_german_nasal_tract p={
        {velar?300:200,velar?2100:1600,velar?2600:2500,velar?3400:3350,3800},
        {-1,-1,-1,-1,-1},
        {velar?108:120,velar?150:200,velar?120:80,330,260}
    };
    if (velar) {
        if (l==RELOQ_DE_PHONE_R) {p.entering[1]=1600;p.entering[2]=2100;}
        else if (back(l) || l==RELOQ_DE_PHONE_w) {p.entering[1]=800;p.entering[2]=2100;}
        else if (l==RELOQ_DE_PHONE_E) {p.entering[1]=1800;p.entering[2]=2100;}
        else if (l==RELOQ_DE_PHONE_A) {p.entering[1]=1750;p.entering[2]=2200;}
        if (back(r) || r==RELOQ_DE_PHONE_a || r==RELOQ_DE_PHONE_A ||
            r==RELOQ_DE_PHONE_R || r==RELOQ_DE_PHONE_Q_oj || r==RELOQ_DE_PHONE_Q_a_TILDE)
            p.departing[1]=1600;
        else if (high_front(r) || r==RELOQ_DE_PHONE_Q_E_COLON) {
            p.departing[1]=2100;p.departing[2]=2500;
        } else if (front(r) || r==RELOQ_DE_PHONE_AT) {
            p.departing[1]=1900;p.departing[2]=2300;
        }
    } else {
        p.departing[0]=200;p.departing[1]=1600;p.departing[2]=2500;
        if (!n->same_word_left && front(l)) {p.entering[1]=1700;p.entering[2]=2600;}
        else if (high_front(l) || l==RELOQ_DE_PHONE_S) {p.entering[2]=2800;p.entering[3]=3000;}
        else if (l==RELOQ_DE_PHONE_j) p.entering[1]=1800;
        else if (l==RELOQ_DE_PHONE_u || l==RELOQ_DE_PHONE_o) p.entering[1]=1450;
        else if (l==RELOQ_DE_PHONE_U || l==RELOQ_DE_PHONE_w) p.entering[1]=1400;
        else if (l==RELOQ_DE_PHONE_O) {p.entering[1]=1500;p.entering[2]=2700;}
        else if (l==RELOQ_DE_PHONE_m) {p.entering[1]=1700;p.entering[2]=2700;}
        else if ((front(l) && l!=RELOQ_DE_PHONE_y) || l==RELOQ_DE_PHONE_AT || l==RELOQ_DE_PHONE_X) {
            p.entering[1]=1700;p.entering[2]=2600;
        }
        if (high_front(r)) {
            p.departing[1]=1850;p.departing[2]=2800;p.entering[3]=3400;
        } else if (r==RELOQ_DE_PHONE_u || r==RELOQ_DE_PHONE_o || r==RELOQ_DE_PHONE_Q_oj)
            p.departing[1]=1400;
        else if (back(r) || r==RELOQ_DE_PHONE_R) p.departing[1]=1500;
        else if (front(r)) {p.departing[1]=1700;p.departing[2]=2600;}
        else if (r==RELOQ_DE_PHONE_m) {p.departing[1]=1700;p.departing[2]=2700;}
        else if (r==RELOQ_DE_PHONE_p || r==RELOQ_DE_PHONE_b || r==RELOQ_DE_PHONE_P ||
                 r==RELOQ_DE_PHONE_f || r==RELOQ_DE_PHONE_v || r==RELOQ_DE_PHONE_w)
            p.departing[1]=1200;
    }
    /* Phrase edges reuse the available target of the opposite edge.
     * n combines its two F3 targets into one sustained resonance. */
    if (initial) for (unsigned j=0;j<5;++j)
        if (p.departing[j]>0) p.entering[j]=p.departing[j];
    if (final) for (unsigned j=1;j<5;++j) p.departing[j]=p.entering[j];
    if (!velar) {p.entering[2]=(p.entering[2]+p.departing[2])/2;p.departing[2]=-1;}
    *out=p;return 1;
}

int reloq_german_nasal_tract_append(const struct reloq_german_nasal_tract *p,
    int64_t begin,int64_t end,size_t index,reloq_german_source_emit emit,void *context)
{
    if (!p || !emit || begin<0 || end<begin) return -1;
    for (unsigned j=0;j<5;++j) {
        int a=p->entering[j],b=p->departing[j];
        /* doF's short-core branch merges two positive targets below 16 ms.
         * Absent departures sustain the entering value through the core. */
        if (end-begin<16 && a>0 && b>0) {a=(a+b)/2;b=-1;}
        if (b<0) b=a;
        if (emit(context,(enum reloq_target_stream)(RELOQ_TARGET_F1+j),begin,a,index) ||
            emit(context,(enum reloq_target_stream)(RELOQ_TARGET_F1+j),end,b,index) ||
            emit(context,(enum reloq_target_stream)(RELOQ_TARGET_B1+j),begin,p->bandwidth[j],index) ||
            emit(context,(enum reloq_target_stream)(RELOQ_TARGET_B1+j),end,p->bandwidth[j],index)) return -1;
    }
    return 0;
}
