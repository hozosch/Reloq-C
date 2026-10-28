/* German consonant spelling constraints for productive word analysis.
 * Rules use ordinary spelling, independent of token storage and execution
 * machinery. See the release notes for their behavioral reference. */
#include "reloq_german_word_edges.h"
#include <string.h>

static bool one_of(unsigned char letter,const char *letters)
{
    return letter!=0 && strchr(letters,letter)!=NULL;
}

static bool spelling_in(const unsigned char *spelling,size_t length,
    const char *words,size_t count,size_t stride)
{
    /* The short, fixed-width inventories are sorted by spelling. */
    size_t first=0;
    while(first<count) {
        size_t middle=first+(count-first)/2;
        int order=memcmp(spelling,words+middle*stride,length);
        if(order==0)return true;
        if(order<0)count=middle;else first=middle+1;
    }
    return false;
}

bool reloq_german_onset(const struct reloq_german_consonants *run)
{
    const unsigned char *s=run->spelling;
    size_t n=run->length;
    if(n==0)return true;
    if(n==1)return s[0]!=0xdf; /* sharp s cannot start this onset */
    if(s[0]=='s' && (s[1]=='h' || s[1]=='z'))return true;
    switch(n) {
    case 2:
        switch(s[0]) {
        case 's':return one_of(s[1],"pkt");
        case 'c':return one_of(s[1],"hlr");
        case 'p':return one_of(s[1],"fhlr");
        case 't':return one_of(s[1],"hr");
        case 'b':case 'f':return one_of(s[1],"lr");
        case 'g':case 'k':return one_of(s[1],"lnr");
        case 'd':return s[1]=='r';
        case 'z':return s[1]=='w';
        case 'q':return s[1]=='u';
        default:return false;
        }
    case 3: {
        static const char triples[][4]={"chl","chr","pfl","pfr","phl","phr",
            "sch","skl","spl","spr","str","thl","thr"};
        return spelling_in(s,n,&triples[0][0],sizeof triples/sizeof *triples,4);
    }
    case 4:return !memcmp(s,"sch",3) && one_of(s[3],"lmnrw");
    default:return false;
    }
}

bool reloq_german_root_coda(const struct reloq_german_consonants *run)
{
    const unsigned char *s=run->spelling;
    size_t n=run->length;
    if(n<2)return true;
    switch(n) {
    case 2:
        switch(s[1]) {
        case 'k':return one_of(s[0],"scrkln");
        case 'p':return one_of(s[0],"srlmp");
        case 's':return one_of(s[0],"srlmn");
        case 't':return one_of(s[0],"sdfkprnlmtz");
        case 'b':return one_of(s[0],"rlmb");
        case 'd':return one_of(s[0],"dlrnmg");
        case 'f':return one_of(s[0],"frlpn");
        case 'g':return one_of(s[0],"grln");
        case 'h':return one_of(s[0],"cprt");
        case 'l':return one_of(s[0],"dhlr");
        case 'n':return one_of(s[0],"gdhlnrw");
        case 'm':return one_of(s[0],"hlmr");
        case 'r':return one_of(s[0],"rh");
        case 'v':return s[0]=='r';
        case 'z':return one_of(s[0],"lnrtz");
        default:return false;
        }
    case 3: {
        static const char triples[][4]={"bst","chs","cht","chz","hnd","hnt",
            "hrd","hrt","lch","ldt","lft","lph","lst","mpf","mph","nch",
            "ndt","nft","ngs","nst","nth","ntz","pst","rch","rck","rdt",
            "rft","rnz","rph","rrh","rst","rth","rtz","rzt","sch","wsk"};
        return spelling_in(s,n,&triples[0][0],sizeof triples/sizeof *triples,4);
    }
    case 4: {
        static const char groups[][5]={"bsch","chst","lbst","lsch","msch","ngst",
            "nsch","rbst","rcht","rnst","rsch","tsch"};
        return spelling_in(s,n,&groups[0][0],sizeof groups/sizeof *groups,5);
    }
    case 5:return !memcmp(s,"rrsch",5);
    default:return false;
    }
}

bool reloq_german_extended_coda(const struct reloq_german_consonants *root,
    const struct reloq_german_consonants *shortened,
    unsigned char last,unsigned char previous,bool has_previous,
    enum reloq_german_ending ending)
{
    if(reloq_german_root_coda(root))return true;
    if(!has_previous)return false;
    switch(ending) {
    case RELOQ_GERMAN_WORD_END:
        if(last=='s') {
            if(one_of(previous,"s\xdfxz"))return false;
        } else if(last!='t' || previous=='t')return false;
        break;
    case RELOQ_GERMAN_DENTAL_END:
        if(last!='t' || previous=='t')return false;
        break;
    case RELOQ_GERMAN_SONORANT_END:
        if(!one_of(last,"lrn"))return false;
        break;
    default:return false;
    }
    return reloq_german_root_coda(shortened);
}
