#include "reloq_german_affricate.h"
#include <stdint.h>

typedef enum reloq_german_phone Phone;
#define P(x) RELOQ_DE_PHONE_##x
static int vowel(Phone p)
{ return (p>=P(a) && p<=P(AT)) || p==P(R) || p>=P(Q_E_COLON); }
static int affricate(Phone p)
{ return p==P(P) || p==P(T) || p==P(C) || p==P(J); }

int reloq_german_affricate_expand(struct reloq_german_phone_node *p,
    size_t *count, size_t capacity)
{
    if (!count || (!p && *count) || *count>capacity) return -1;
    size_t needed=*count;
    for (size_t i=0;i<*count;++i)
        if (affricate(p[i].phone)) {
            if (needed==capacity) return -1;
            ++needed;
        }
    size_t end=needed;
    for (size_t i=*count;i-- >0;) {
        struct reloq_german_phone_node n=p[i];
        Phone q=n.phone;
        if (affricate(q)) {
            /* The lexical /dZ/ loses voicing in a syllable coda. */
            if (q==P(J) && (!n.same_syllable_right ||
                (!vowel(n.right_phone) && n.same_syllable_left))) q=P(C);
            unsigned role=q==P(P)?1:q==P(T)?3:q==P(C)?5:7;
            n.phone=q==P(P)?P(f):q==P(T)?P(s):q==P(C)?P(S):P(Z);
            n.affricate_role=(uint8_t)(role+1);p[--end]=n;
            n.phone=q==P(P)?P(p):q==P(J)?P(d):P(t);
            n.affricate_role=(uint8_t)role;p[--end]=n;
        } else p[--end]=n;
    }
    for (size_t i=0;i<needed;++i) {
        p[i].left_phone=i?p[i-1].phone:P(NONE);
        p[i].right_phone=i+1<needed?p[i+1].phone:P(NONE);
        p[i].same_word_left=i && p[i-1].word_index==p[i].word_index;
        p[i].same_syllable_left=i && p[i-1].syllable_index==p[i].syllable_index;
        p[i].same_word_right=i+1<needed && p[i+1].word_index==p[i].word_index;
        p[i].same_syllable_right=i+1<needed && p[i+1].syllable_index==p[i].syllable_index;
        if (i && p[i].phone==P(j) && p[i].same_syllable_left &&
            p[i].same_syllable_right && vowel(p[i].right_phone) &&
            p[i-1].affricate_role && !(p[i-1].affricate_role&1))
            p[i].affricate_role=9;
    }
    *count=needed;return 0;
}

int reloq_german_affricate_context_ticks(const struct reloq_german_phone_node *p,
    size_t count,size_t i,int final,int fresh,int following_final,int32_t *ticks)
{
    if (!p || !ticks || i>=count) return -1;
    const struct reloq_german_phone_node *n=p+i;
    const unsigned role=n->affricate_role;
    if (!role) return 0;
    if (role==9) {*ticks=50;return 1;}
    if (role>8) return -1;
    const int onset=role&1;
    const Phone left=onset?n->left_phone:i?p[i-1].left_phone:P(NONE);
    const int nasal_left=left==P(m)||left==P(n)||left==P(G);
    const int vocalic_left=!onset && left==P(r) && i>=3 &&
        p[i-2].same_syllable_left && vowel(p[i-3].phone);
    int32_t span;
    if (onset) {
        if (i+1==count) return -1;
        const Phone right=i+2<count && p[i+1].same_word_right?p[i+2].phone:P(NONE);
        const int word_initial=!n->same_word_left;
        const int final_stop=right==P(p)||right==P(t)||right==P(k);
        if (role==1) span=fresh?15:
            i+2<count && p[i+1].same_syllable_right &&
                (right==P(t)||right==P(r)||right==P(l))?44:60;
        else if (role==3 && fresh) span=10;
        else if (role==3 && final_stop) span=following_final?55:40;
        else if (role==3 && left==P(r) && i>=2 &&
                 p[i-1].same_syllable_left && !vowel(p[i-2].phone)) span=50;
        else if (nasal_left && role==3) span=left==P(n)?41:right==P(NONE)?70:50;
        else if (role==3 && !n->same_syllable_left &&
                 (left==P(f)||left==P(s)||left==P(z)||left==P(S)||left==P(x)||left==P(X)||
                  left==P(p)||left==P(t)||left==P(k)||left==P(b)||left==P(d)||left==P(g)))
            span=left==P(s)||left==P(z)?65:50;
        else if (role==5 && fresh) span=10;
        else if (role==5 && word_initial) span=
            left==P(p)||left==P(t)||left==P(k)?39:
            left==P(s)||left==P(S)?60:
            nasal_left||left==P(l)||left==P(f)||left==P(x)||left==P(X)?73:56;
        else if (nasal_left && role==5) span=37;
        else if (role==7) span=n->stress?60:50;
        else if (role==5) span=right==P(NONE)?62:48;
        else span=right==P(O) || right==P(U) || right==P(o) ||
                   right==P(u) || right==P(R)?53:64;
    } else {
        if (!i) return -1;
        const int word_initial=!p[i-1].same_word_left;
        const int stop_contact=n->same_word_right &&
            (n->right_phone==P(p)||n->right_phone==P(t)||n->right_phone==P(k));
        if (role==2 && n->same_syllable_right &&
            (n->right_phone==P(l)||n->right_phone==P(r))) {
            span=n->right_phone==P(r)?95:
                left!=P(NONE)&&!vowel(left)&&!vocalic_left?95:110;
            if (n->stress) span=span*3/5;
        }
        else if (stop_contact) span=role==2?(n->stress?90:110):role==4?(following_final?125:80):70;
        else if (role==2) span=final?(n->stress?90:150):
            nasal_left && !n->same_syllable_right?(n->stress?57:76):
            /* A stressed /pf/ after a consonant contracts its frication.
             * Vocalic r retains the preceding vowel's timing domain. */
            n->stress && n->same_syllable_right && left!=P(NONE) &&
                !vowel(left) && !vocalic_left?57:n->stress?71:95;
        else if (role==8) span=n->stress?50:45;
        else if (final) span=125;
        else if (role==6) span=word_initial?(n->right_phone==P(i)?95:85):70;
        /* Original /s.tsV/ contracts only the secondary-stressed
         * fricative half. Primary and unstressed syllables keep their
         * existing 90/60-tick bodies; the release stays separate. */
        else if(role==4 && n->stress==2 && left==P(s) &&
                p[i-1].same_word_left && !p[i-1].same_syllable_left &&
                n->same_syllable_right && vowel(n->right_phone))span=65;
        else span=fresh?75:n->right_phone==P(v)?(left==P(s)?50:60):
                  n->right_phone==P(l)||n->right_phone==P(r)||n->right_phone==P(n)||n->right_phone==P(m)?80:
                  n->right_phone==P(j)?80:
                  n->stress==1?90:n->stress==2?80:60;
        /* The stop's release connects into the fricative, independently
         * of its plateau. It is not aspiration into the vowel. */
        span+=role==8?1:10;
    }
    *ticks=span;return 1;
}

int reloq_german_affricate_ticks(const struct reloq_german_phone_node *p,
    size_t count,size_t i,int final,int32_t *ticks)
{
    if (!p || i>=count) return -1;
    size_t start=i-(p[i].affricate_role && !(p[i].affricate_role&1) && i?1:0);
    size_t following=start+2;
    return reloq_german_affricate_context_ticks(p,count,i,final,
        start==0 || p[start].left_phone==P(NONE),
        following<count && p[following].right_phone==P(NONE),ticks);
}

int reloq_german_affricate_tract(const struct reloq_german_phone_node *n,
    int initial,int final,Phone following,struct reloq_german_obstruent_tract *out)
{
    if (!n || !out) return -1;
    unsigned role=n->affricate_role;
    if (!role) return 0;
    if (role==9) {
        const Phone r=n->right_phone;
        const int back=r==P(U) || r==P(o) || r==P(u);
        *out=(struct reloq_german_obstruent_tract){
            {300,back?1900:r==P(AT)?1940:2100,back?2500:r==P(AT)?2700:2900,3600,3900},
            {-1,-1,-1,-1,-1},{120,250,500,330,260}};
        return 1;
    }
    struct reloq_german_phone_node context=*n;
    /* The closing half conditions the tract by the preceding vowel; the
     * fricative starts with its own place, not a vowel guessed from spelling. */
    if (role&1) {
        context.right_phone=P(A);context.same_word_right=context.same_syllable_right=1;
        context.same_syllable_left=0;
    } else {
        context.left_phone=P(R);context.same_word_left=1;initial=0;
        /* Preserve the actual following consonant through the frication. */
    }
    int result=reloq_german_obstruent_tract_plan(&context,initial,
        role&1?0:final,out);
    if (result!=1) return result;
    if (!initial && (role&1) && (n->left_phone==P(m)||n->left_phone==P(n)||n->left_phone==P(G)) &&
        !(role==1 && n->left_phone==P(n)))
        for (unsigned j=0;j<3;++j) out->entering[j]=-2;
    if (!initial && role==1 && n->left_phone==P(n)) {out->entering[1]=1500;out->entering[2]=2500;}
    switch (role) {
    case 1:
        if (!n->same_word_left) out->departing[0]=400;
        out->departing[1]=900;
        if (n->same_syllable_left && (n->left_phone==P(A) || n->left_phone==P(a))) {
            out->entering[0]=500;out->entering[1]=900;
        }
        break;
    case 3:
        out->departing[1]=1600;
        if (n->left_phone==P(r)) {
            out->entering[0]=300;out->entering[1]=1325;out->entering[2]=2500;
        }
        if (following==P(U)||following==P(o)||following==P(u)) out->departing[4]=4300;
        if (!initial && !n->same_syllable_left &&
            (n->left_phone==P(f)||n->left_phone==P(s)||n->left_phone==P(S)||
             n->left_phone==P(x)||n->left_phone==P(X)||n->left_phone==P(z)||
             n->left_phone==P(p)||n->left_phone==P(t)||n->left_phone==P(k)||
             n->left_phone==P(b)||n->left_phone==P(d)||n->left_phone==P(g))) {
            const int palatal=n->left_phone==P(X);
            *out=(struct reloq_german_obstruent_tract){
                {300,palatal?1800:1600,palatal?2700:2500,3500,4200},
                {-1,1600,2500,-1,following==P(U)||following==P(o)||following==P(u)?4300:-1},
                {120,100,150,330,260}};
        }
        break;
    case 5: out->departing[1]=2300;out->departing[2]=2800;break;
    case 7: out->departing[1]=1800;break;
    case 6:
        out->entering[2]=2600;out->entering[3]=out->departing[3]=3800;
        if (final) out->departing[2]=2600;
        break;
    case 8:
        out->entering[1]=1800;out->entering[2]=2400;out->entering[3]=3800;
        break;
    default:break;
    }
    return 1;
}
#undef P
