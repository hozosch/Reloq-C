#include "reloq_german_liquid_tract.h"
#include "reloq_german_nucleus_acoustics.h"
typedef enum reloq_german_phone Phone;
#define P(n) RELOQ_DE_PHONE_##n
static int vowel(Phone p)
{
    return (p>=P(a) && p<=P(AT)) || p==P(R) || p>=P(Q_E_COLON);
}
static Phone endpoint(Phone p,int left)
{
    if (p==P(Q_aj) || p==P(Q_aw)) return left?(p==P(Q_aj)?P(j):P(w)):P(A);
    if (p==P(Q_oj)) return left?P(j):P(O);
    return p;
}
static void oral(struct reloq_german_obstruent_tract *p,int f1,int f2,int f3)
{p->entering[0]=f1;p->entering[1]=f2;p->entering[2]=f3;}
int reloq_german_j_av(const struct reloq_german_phone_node *n)
{
    if (!n) return 48;
    if (n->same_word_left && n->left_phone==P(A)) return 48;
    switch (n->right_phone) {
    case P(e): case P(i): case P(I): case P(U): case P(u):
    case P(y): case P(Y): case P(Q_oe): case P(Q_OE): return 52;
    case P(AT): return 53;
    default: return 48;
    }
}
int reloq_german_liquid_tract_plan(const struct reloq_german_phone_node *n,
    int initial,int final,struct reloq_german_obstruent_tract *out)
{
    if (!n || !out || (initial!=0 && initial!=1) || (final!=0 && final!=1)) return -1;
    if (n->phone==P(w) && n->same_word_right &&
        (n->same_syllable_right || !vowel(n->right_phone)) &&
        !(n->same_syllable_left && vowel(n->left_phone))) {
        /* Rounded onset glide: the broader second and third resonances
         * persist through its own hold before the following vowel. */
        *out=(struct reloq_german_obstruent_tract){
            {400,940,2260,3600,3900},{-1,-1,-1,-1,-1},{120,250,500,330,260}};
        return 1;
    }
    if (n->phone==P(j) && !n->affricate_role && n->same_syllable_right &&
        vowel(n->right_phone) && !(n->same_syllable_left && vowel(n->left_phone))) {
        struct reloq_german_obstruent_tract p={
            {300,2100,2900,3600,3900},{-1,-1,-1,-1,-1},{120,250,500,330,260}};
        if (n->right_phone==P(u)||n->right_phone==P(U)||n->right_phone==P(o))
            oral(&p,300,1900,2500);
        else if (!initial && n->left_phone==P(AT)) oral(&p,300,1900,2700);
        else if (n->right_phone==P(AT)) oral(&p,300,1940,2700);
        *out=p;return 1;
    }
    if (n->phone==P(w) && n->same_syllable_left && vowel(n->left_phone)) {
        *out=(struct reloq_german_obstruent_tract){
            {400,940,2260,3200,3900},{-1,-1,-1,-1,-1},{120,100,500,330,260}};
        return 1;
    }
    if (n->phone==P(j) && n->same_syllable_left &&
        !vowel(n->left_phone) && !n->affricate_role) {
        /* A glide after a consonant retains its own broad resonances,
         * including before the next syllable's consonant. */
        *out=(struct reloq_german_obstruent_tract){
            {300,2100,2900,3600,3900},{-1,-1,-1,-1,-1},{120,250,500,330,260}};
        return 1;
    }
    if (n->phone==P(j) && n->same_syllable_left && vowel(n->left_phone)) {
        struct reloq_german_obstruent_tract p={
            {420,1900,2450,3400,3900},{-1,-1,-1,-1,-1},{120,100,150,330,260}};
        switch(n->left_phone) {
        case P(e): case P(E): {
            /* Start with the original post-E closing glide. Move its oral
             * resonances one third toward unstressed -stik I, retaining the
             * original bandwidths and source rather than sharpening F2. */
            struct reloq_german_nucleus_acoustics short_i;
            if (reloq_german_nucleus_acoustics_plan(P(I),P(t),P(k),0,0,&short_i)!=1)
                return -1;
            const int original[5]={265,2150,2900,3400,3900};
            for (unsigned i=0;i<5;++i)
                p.entering[i]=(2*original[i]+short_i.entering[i])/3;
            break;
        }
        case P(i): case P(I): case P(y):
            oral(&p,265,2150,2900);break;
        case P(O): case P(o): case P(U): p.entering[0]=350;break;
        case P(u): p.entering[0]=300;break;
        default:break;
        }
        if(n->same_word_right && (n->right_phone==P(n) || n->right_phone==P(m)))p.bandwidth[0]=270;
        *out=p;return 1;
    }
    if (n->phone!=P(r) && n->phone!=P(l)) return 0;
    const int coda=n->same_syllable_left && vowel(n->left_phone);
    /* The onset targets belong to l/r after a syllable or word boundary,
     * even when the preceding phone is a consonant. Onset clusters have
     * their own tract planner and remain outside this branch. */
    const int rhotic_glide=n->phone==P(r) && n->right_phone==P(j) && n->same_syllable_right;
    const int liquid_onset=!n->same_syllable_left && n->same_syllable_right &&
        (vowel(n->right_phone) || rhotic_glide);
    if ((!initial && !liquid_onset && (!n->same_word_left || !vowel(n->left_phone))) ||
        (!coda && !final && (!n->same_word_right || (!vowel(n->right_phone)&&!rhotic_glide)))) return 0;
    Phone l=initial?P(NONE):endpoint(n->left_phone,1),
          r=final?P(NONE):endpoint(n->right_phone,0);
    if (n->phone==P(r) && coda && l==P(AT)) return 0;
    struct reloq_german_obstruent_tract p={
        {400,1320,2100,3600,3900},{-1,-1,-1,-1,-1},{120,100,150,330,260}};
    if (n->phone==P(r) && coda) {
        /* A postvocalic r is the closing vocalic member of the nucleus.
         * Its targets depend on the preceding vowel, not onset-r classes. */
        oral(&p,600,1360,2280);
        if (n->same_word_right && (r==P(m) || r==P(n))) p.bandwidth[0]=270;
        switch (l) {
        case P(A): case P(a): oral(&p,570,1190,2320);break;
        case P(E): oral(&p,650,1300,2600);break;
        case P(e): oral(&p,600,1530,2600);break;
        case P(I): case P(i): oral(&p,600,1530,2400);break;
        case P(O): p.entering[1]=1125;break;
        case P(U): oral(&p,520,1125,2280);break;
        case P(o): oral(&p,550,1040,2060);break;
        case P(u): oral(&p,550,997,2060);break;
        case P(y): oral(&p,500,1400,2060);break;
        /* Retain the requested long-oe colour within the ordinary vocalic
         * r gesture. Timbre no longer bypasses coda/contact planning. */
        case P(Q_oe): oral(&p,600,1320,2300);p.bandwidth[2]=200;break;
        case P(j): case P(w): return 0;
        default:break;
        }
    } else if (n->phone==P(r)) {
        if (l==P(E) || l==P(Q_E_COLON) || l==P(Q_E_TILDE)) p.entering[2]=2200;
        else if (l==P(O) || l==P(U) || l==P(Q_o_TILDE)) oral(&p,400,1200,2000);
        else if (l==P(o) || l==P(u)) oral(&p,400,850,2000);
        else if (l==P(w) || l==P(m) || l==P(p)) p.entering[1]=1000;
        if (r==P(E) || r==P(I) || r==P(Q_E_TILDE)) {p.departing[1]=1450;p.departing[2]=2200;}
        else if (r==P(e) || r==P(i) || r==P(Q_E_COLON)) {p.departing[1]=1670;p.departing[2]=2300;}
        else if (r==P(O) || r==P(U) || r==P(y) || r==P(Q_o_TILDE)) {p.departing[1]=1200;p.departing[2]=2000;}
        else if (r==P(o) || r==P(u)) {p.departing[1]=840;p.departing[2]=2000;}
        if (initial) for (unsigned j=1;j<3;++j)
            if (p.departing[j]>0) p.entering[j]=p.departing[j];
    } else {
        p.entering[3]=3450;
        if (coda) {
            if (n->same_word_right && (r==P(m) || r==P(n) || r==P(G)))
                p.bandwidth[0]=270;
            oral(&p,350,1500,2800);
            if (l==P(e)) p.entering[1]=1600;
            else if (l==P(i)) {oral(&p,300,1550,3100);p.entering[3]=3600;p.entering[4]=4200;}
            else if (l==P(E) || l==P(I) || l==P(y) || l==P(Y) ||
                l==P(Q_oe) || l==P(Q_OE) || l==P(Q_E_TILDE) || l==P(AT) || l==P(Q_E_COLON) || l==P(j)) p.entering[1]=1550;
            else if (l==P(w)) p.entering[1]=1350;
            else if (l==P(R)) p.entering[1]=1400;
            if (l==P(O) || l==P(U) || l==P(o) || l==P(u)) p.entering[2]=2400;
        } else {
            oral(&p,400,1450,l==P(O)?2400:2900);
            if (r==P(A) || r==P(a) || r==P(Q_a_TILDE)) p.entering[1]=1230;
            else if (r==P(I) || r==P(e) || r==P(i)) {p.entering[0]=300;p.entering[1]=1550;}
            else if (r==P(O) || r==P(U) || r==P(Q_o_TILDE) || r==P(Q_oe_TILDE)) p.entering[1]=1400;
            else if (r==P(o) || r==P(u)) p.entering[1]=1300;
            else if (r==P(R)) p.entering[1]=1500;
        }
    }
    *out=p;return 1;
}
#undef P
