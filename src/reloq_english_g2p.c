#include <stddef.h>
#include <string.h>
#include "reloq_english_g2p83.inc"

static int vowel_letter(unsigned char c){return c&&strchr("aeiouy",c)!=NULL;}
static unsigned char lower(unsigned char c){return c>='A'&&c<='Z'?c+32:c;}
static int letter_at(const unsigned char *w,size_t n,ptrdiff_t i)
{return i<0||(size_t)i>=n?0:lower(w[i])-'a'+1;}
static int cap(size_t n,int maximum){return n>(size_t)maximum?maximum:(int)n;}

static unsigned decision(const struct g2p_node *tree,const int f[21],unsigned phone)
{
    int node=0;
    while(tree[node].left>=0) {
        const struct g2p_node *p=tree+node;
        if(p->feature<432)node=f[p->feature/27]==p->feature%27?p->right:p->left;
        else if(p->feature<437)node=f[16+p->feature-432]<=p->threshold?p->left:p->right;
        else node=phone==(unsigned)(p->feature-437)?p->right:p->left;
    }
    return tree[node].label;
}

/* Bounded integer context tree compiled from original ECI observations.
 * Exact dictionary and productive morphology precede this path. Character
 * mode keeps its explicit names; ordinary unknown words receive phones. */
int reloq_english_g2p(const unsigned char *word,size_t n,char *out,size_t capacity)
{
    if(!word||!n||!out||capacity<8)return 0;
    for(size_t i=0;i<n;++i)if(lower(word[i])<'a'||lower(word[i])>'z')return 0;
    size_t total_vowels=0,left_vowels=0,used=2;
    for(size_t i=0;i<n;++i)total_vowels+=vowel_letter(lower(word[i]));
    out[0]='`';out[1]='[';
    for(size_t i=0;i<n;++i) {
        int f[21],k=0;
        for(int d=-4;d<=4;++d)f[k++]=letter_at(word,n,(ptrdiff_t)i+d);
        for(int j=0;j<3;++j)f[k++]=letter_at(word,n,j);
        for(int j=0;j<4;++j)f[k++]=letter_at(word,n,(ptrdiff_t)n-j-1);
        f[k++]=cap(i,31);f[k++]=cap(n-i-1,31);f[k++]=cap(n,40);
        f[k++]=cap(left_vowels,15);
        size_t here=(size_t)vowel_letter(lower(word[i]));
        f[k++]=cap(total_vowels-left_vowels-here,15);left_vowels+=here;
        unsigned phone=decision(english_g2p_nodes,f,0);
        unsigned marks=decision(english_stress_nodes,f,phone);
        const char *phones=english_g2p_labels[phone];
        for(size_t j=0;phones[j];++j) {
            unsigned mark=(marks>>(2*j))&3;
            if(used+4>=capacity)return 0;
            if(mark){out[used++]='.';out[used++]=(char)('0'+mark-1);}
            out[used++]=phones[j];
        }
    }
    if(used==2)return 0;
    if(out[2]!='.') {
        if(used+3>=capacity)return 0;
        memmove(out+4,out+2,used-2);out[2]='.';out[3]='1';used+=2;
    }
    for(size_t i=2;i<used;++i)if(out[i]=='h') {
        size_t next=i+1;
        while(next+1<used&&out[next]=='.')next+=2;
        if(next==used||!strchr("iIeEAXHxauUocRYWOmnrlwy",out[next])) {
            if(used+2>=capacity)return 0;
            memmove(out+i+2,out+i+1,used-i-1);out[i+1]='x';++used;++i;
        }
    }
    /* Separate local decisions cannot leave a word without primary stress.
     * Preserve the first predicted primary; subsequent stresses are secondary. */
    size_t primary=0,secondary=0;
    for(size_t i=2;i+1<used;++i)if(out[i]=='.') {
        if(out[i+1]=='1'){if(!primary)primary=i+1;else out[i+1]='2';}
        else if(out[i+1]=='2'&&!secondary)secondary=i+1;
    }
    if(!primary)out[secondary?secondary:3]='1';
    out[used++]=']';out[used]=0;return 1;
}
