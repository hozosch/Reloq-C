#include "reloq_german_word_class.h"
#include <string.h>

static int member(const char *set,const unsigned char *s,size_t n)
{
    while (*set) {
        const char *end=strchr(set,' ');
        if (!end) end=set+strlen(set);
        if ((size_t)(end-set)==n && !memcmp(set,s,n)) return 1;
        set=*end?end+1:end;
    }
    return 0;
}
static int vowel(unsigned char c)
{
    return c=='a' || c=='e' || c=='i' || c=='o' || c=='u' ||
        c=='y' || c==0xe4 || c==0xf6 || c==0xfc;
}

/* Independently described edge classes, checked against the original
 * legality decisions. The permissive sh-/sz- branch is intentional: the
 * original does not test the remainder of those initial clusters. */
static int initial(const unsigned char *s,size_t n)
{
    if (!n) return 1;
    if (n==1) return s[0]!=0xdf;
    if (s[0]=='s' && (s[1]=='h' || s[1]=='z')) return 1;
    return member("bl br ch chl chr cl cr dr fl fr gl gn gr kl kn kr "
        "pf pfl pfr ph phl phr pl pr sch schl schm schn schr schw "
        "sk skl sp spl spr st str th thl thr tr zw",s,n);
}
int reloq_german_root_coda_legal(const unsigned char *s,size_t n)
{
    if (!n) return 1;
    if (n==1) return 1;
    return member("bb bst ch chs chst cht chz ck dd dl dn dt ff ft gd gg gn "
        "hl hm hn hnd hnt hr hrd hrt kk kt lb lbst lch ld ldt lf lft lg lk "
        "ll lm ln lp lph ls lst lt lz mb md mm mp mpf mph ms mt nch nd ndt "
        "nf nft ng ngs ngst nk nn ns nst nt nth ntz nz pf ph pp pst pt "
        "rb rbst rch rcht rck rd rdt rf rft rg rh rk rl rm rn rnst rnz rp "
        "rph rr rrh rs rst rt rth rtz rv rz rzt sch sk sp ss st th tt tz "
        "wn wsk zt zz",s,n);
}
static int final(const unsigned char *s,size_t n)
{
    if (reloq_german_root_coda_legal(s,n)) return 1;
    if (n<2) return 0;
    /* The word-ending pass retries the root after one inflectional s/t.
     * It rejects doubled t and s after a sibilant. */
    const unsigned char last=s[n-1],before=s[n-2];
    if (last=='t' && before!='t') return reloq_german_root_coda_legal(s,n-1);
    if (last=='s' && before!='s' && before!='z' && before!='x' && before!=0xdf)
        return reloq_german_root_coda_legal(s,n-1);
    return 0;
}

int reloq_german_word_class(const unsigned char *word,size_t length)
{
    if (!word || !length) return -1;
    size_t vowels=0;int has_y=0;
    for (size_t i=0;i<length;++i) {
        unsigned char c=word[i];
        if (!((c>='A' && c<='Z') || c==0xc4 || c==0xd6 || c==0xdc)) return -1;
        vowels+=(size_t)vowel((unsigned char)(c+32));
        has_y|=c=='Y';
    }
    if (length==1) return 1;
    /* The long-token branch follows the homogeneous-letter checks. */
    if (!has_y && (!vowels || vowels==length)) return 1;
    if (length>6) return 0;
    unsigned char lower[7];
    for (size_t i=0;i<length;++i) lower[i]=(unsigned char)(word[i]+32);
    lower[length]=0;
    /* Publicly verified lexical decisions, separate from edge grammar.
     * API is handled before the original acronym rule; SWAPO bypasses it. */
    if (member("swapo",lower,length)) return 0;
    if (member("aeg adac afa ag ap api ard at eg eh eth ewg ews iara iata "
        "ios ici ig imi iq itt oeec oex oil orf ov un url us usa biz ca cia "
        "diy fa fao gi mad plo reu sas siti sos smuv sna spu sto tau tei tu "
        "va weu \366iag",lower,length)) return 1;
    for (size_t i=1;i<length;++i) if (lower[i]=='y' && lower[i-1]=='y') return 1;
    size_t begin=0,end=length;
    while (begin<length && !vowel(lower[begin])) ++begin;
    while (end && !vowel(lower[end-1])) --end;
    return !initial(lower,begin) || !final(lower+end,length-end);
}
