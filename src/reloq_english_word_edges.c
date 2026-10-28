/* English consonant spelling constraints for word classification and syllables.
 * This is an ordinary bounded spelling decision, independent of token storage,
 * scan registers, rule frames and backtracking. Unlisted consonant letters
 * retain the permissive foreign-spelling treatment of the reference behavior. */
#include "reloq_english_word_edges.h"
#include <string.h>

static bool one_of(unsigned char c,const char *letters)
{return c!=0 && strchr(letters,c)!=NULL;}

static bool listed(const unsigned char *s,size_t n,const char *words,
    size_t count,size_t stride)
{
    size_t first=0;
    while(first<count) {
        size_t middle=first+(count-first)/2;
        int order=memcmp(s,words+middle*stride,n);
        if(!order)return true;
        if(order<0)count=middle;else first=middle+1;
    }
    return false;
}

bool reloq_english_onset(const unsigned char *s,size_t n)
{
    if(n<2)return true;
    if(n==2) {
        switch(s[0]) {
        case 'b':case 'd':return !one_of(s[1],"bcdfgjkmnpstvxz");
        case 'c':return one_of(s[1],"hlrz");
        case 'f':case 'h':case 'z':return !one_of(s[1],"bcdfghjkmnpqstvxz");
        case 'g':case 'k':return !one_of(s[1],"bcdfgjkmpstvxz");
        case 'j':case 'l':case 'm':case 'n':case 'q':case 'x':return false;
        case 'p':return !one_of(s[1],"bcdfgjkmnptvxz");
        case 'r':return s[1]=='h';
        case 's':return !one_of(s[1],"bdgjsx");
        case 't':return !one_of(s[1],"bcdfgjkmnptvx");
        case 'w':return one_of(s[1],"hr");
        default:return !one_of(s[1],"jx");
        }
    }
    if(n==3) {
        static const char groups[][4]={"chl","chr","phl","phr","sch","scl",
            "scr","shl","shr","sph","spl","spr","str","thr"};
        return listed(s,n,&groups[0][0],sizeof groups/sizeof *groups,4);
    }
    return n==4 && !memcmp(s,"sch",3) && one_of(s[3],"lmnrw");
}

static bool pair(const unsigned char *s)
{
    switch(s[0]) {
    case 'b':return !one_of(s[1],"cdfgjklmnpqrvwx");
    case 'c':case 'k':return !one_of(s[1],"bcdfgjlmnpqrvwx");
    case 'd':return !one_of(s[1],"bcfgjklmnpqrvwx");
    case 'f':return !one_of(s[1],"bcdghjklmnprv");
    case 'g':return !one_of(s[1],"bcdfjkpqtvx");
    case 'h':return !one_of(s[1],"bcdfghjkpv");
    case 'j':case 'x':return !one_of(s[1],"bcdfghjklmnprvw");
    case 'l':return !one_of(s[1],"jqrwx");
    case 'm':return !one_of(s[1],"cdfghjklrtvwxz");
    case 'n':return !one_of(s[1],"bflprvw");
    case 'p':return !one_of(s[1],"bcdfgjklmnqrvwx");
    case 'r':return !one_of(s[1],"jqw");
    case 's':return !one_of(s[1],"bdgjlnrwx");
    case 't':return !one_of(s[1],"bcdfgjklmnpqrvwx");
    case 'v':case 'z':return !one_of(s[1],"bcdfghjklmnprv");
    default:return true;
    }
}

bool reloq_english_coda(const unsigned char *s,size_t n)
{
    if(n<2)return true;
    if(n==2)return pair(s);
    if(n==3) {
        /* The final s can extend any permitted pair once. Longer codas
         * have their own inventory; this is deliberately not recursive. */
        if(s[2]=='s' && pair(s))return true;
        static const char groups[][4]={"cht","dth","fth","ght","lch","lft",
            "lph","lsh","lst","lth","ltz","mph","mpt","nch","nct","ndt",
            "ngh","nst","ntz","pth","rch","rck","rgh","rld","rph","rsh",
            "rst","rth","rtz","sch","tch"};
        return listed(s,n,&groups[0][0],sizeof groups/sizeof *groups,4);
    }
    if(n==4) {
        static const char groups[][5]={"ghts","lfth","ndth","ngst","ngth",
            "rmth","rnst","wnst"};
        return listed(s,n,&groups[0][0],sizeof groups/sizeof *groups,5);
    }
    return false;
}

bool reloq_english_syllable_onset(const unsigned char *s,size_t n)
{
    if(n<2)return true;
    if(n==2) {
        switch(s[0]) {
        case 'b':case 'c':return one_of(s[1],"hlr");
        case 'd':case 't':return one_of(s[1],"hrw");
        case 'f':return one_of(s[1],"lr");
        case 'g':case 'k':return one_of(s[1],"hlnrw");
        case 'p':return one_of(s[1],"hlrs");
        case 'q':return s[1]=='u';
        case 'r':return s[1]=='h';
        case 's':return one_of(s[1],"chklmnpqtw");
        case 'w':return one_of(s[1],"hr");
        default:return false;
        }
    }
    if(n==3) {
        static const char groups[][4]={"chl","chr","phl","phr","sch","scr",
            "shr","skr","spl","spr","squ","str","thr","thw"};
        return listed(s,n,&groups[0][0],sizeof groups/sizeof *groups,4);
    }
    return n==4 && !memcmp(s,"sch",3) && one_of(s[3],"lr");
}
