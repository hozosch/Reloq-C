/* Native English word segmentation, based on the MIT-licensed OpenEVV
 * adapter. See nvda/wordseg-data/NOTICE and LICENSE for the source data. */
#include "reloq_wordseg.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct wordseg_category_range { uint32_t first, last; unsigned char flags; };
struct wordseg_casefold_entry { uint32_t codepoint; const char *folded; };
struct wordseg_known_chunk { const char *words; unsigned char count; };
struct wordseg_verified_compound {
    const char *word; unsigned char boundary; const char *whole, *joined;
};
#include "reloq_wordseg_data.inc"
#include "reloq_wordseg_compounds.inc"
#define COUNT(a) (sizeof(a)/sizeof *(a))
struct wordseg_table { const char *const *words; size_t count; };
#define TABLE(name) (struct wordseg_table){name,COUNT(name)}

static const struct wordseg_verified_compound *verified_find(const char *word,int uk)
{
    const struct wordseg_verified_compound *table=uk?wordseg_verified_engb:wordseg_verified_enus;
    size_t count=uk?COUNT(wordseg_verified_engb):COUNT(wordseg_verified_enus);
    size_t lo=0,hi=count;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(strcmp(table[mid].word,word)<0)lo=mid+1;else hi=mid;
    }
    return lo<count&&!strcmp(table[lo].word,word)?&table[lo]:NULL;
}

int reloq_wordseg_reference(const unsigned char *original,size_t n,int uk,
    const char **whole,const char **joined)
{
    if(!original||!n||n>32)return 0;
    char word[33];
    for(size_t i=0;i<n;++i) {
        unsigned c=original[i];
        if(c>='A'&&c<='Z')c+=32;
        if(c<'a'||c>'z')return 0;
        word[i]=(char)c;
    }
    word[n]=0;
    const struct wordseg_verified_compound *found=verified_find(word,uk);
    if(!found)return 0;
    if(whole)*whole=found->whole;
    if(joined)*joined=found->joined;
    return 1;
}

static _Thread_local struct reloq_wordseg_options options;
struct reloq_wordseg_options reloq_wordseg_scope(struct reloq_wordseg_options value)
{ struct reloq_wordseg_options old=options;options=value;return old; }
struct reloq_wordseg_options reloq_wordseg_current(void) { return options; }

unsigned reloq_wordseg_category(uint32_t c)
{
    size_t lo=0,hi=COUNT(wordseg_categories);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(wordseg_categories[m].last<c)lo=m+1;else hi=m;}
    return lo<COUNT(wordseg_categories)&&wordseg_categories[lo].first<=c?
        wordseg_categories[lo].flags:0;
}

size_t reloq_wordseg_casefold(uint32_t c,unsigned char output[12])
{
    size_t lo=0,hi=COUNT(wordseg_casefolds);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(wordseg_casefolds[m].codepoint<c)lo=m+1;else hi=m;}
    if(lo<COUNT(wordseg_casefolds)&&wordseg_casefolds[lo].codepoint==c) {
        size_t n=strlen(wordseg_casefolds[lo].folded);
        if(n>12)return 0;
        memcpy(output,wordseg_casefolds[lo].folded,n);return n;
    }
    if(c<0x80){output[0]=(unsigned char)c;return 1;}
    if(c<0x800){output[0]=(unsigned char)(0xc0|(c>>6));output[1]=(unsigned char)(0x80|(c&63));return 2;}
    if(c<0x10000){output[0]=(unsigned char)(0xe0|(c>>12));output[1]=(unsigned char)(0x80|((c>>6)&63));output[2]=(unsigned char)(0x80|(c&63));return 3;}
    if(c<=0x10ffff){output[0]=(unsigned char)(0xf0|(c>>18));output[1]=(unsigned char)(0x80|((c>>12)&63));output[2]=(unsigned char)(0x80|((c>>6)&63));output[3]=(unsigned char)(0x80|(c&63));return 4;}
    return 0;
}

static int member(struct wordseg_table table,const char *text,size_t length)
{
    size_t lo=0,hi=table.count;
    while(lo<hi) {
        size_t m=lo+(hi-lo)/2;const char *value=table.words[m];
        size_t n=strlen(value),common=n<length?n:length;int cmp=memcmp(value,text,common);
        if(!cmp)cmp=(n>length)-(n<length);
        if(cmp<0)lo=m+1;else hi=m;
    }
    if(lo==table.count)return 0;
    const char *value=table.words[lo];
    return strlen(value)==length&&!memcmp(value,text,length);
}

static int vowel(const char *word,size_t pos)
{
    char c=word[pos];
    return c=='a'||c=='e'||c=='i'||c=='o'||c=='u'||
        (c=='y'&&pos&&word[pos-1]!='a'&&word[pos-1]!='e'&&
         word[pos-1]!='i'&&word[pos-1]!='o'&&word[pos-1]!='u');
}

static int coda(const char *s,size_t n,struct wordseg_table table)
{
    if(!n||member(table,s,n))return 1;
    char collapsed[33];size_t out=0;
    if(n==1&&s[0]=='q')collapsed[out++]='k';
    else for(size_t i=0;i<n;++i)if(!i||s[i]!=s[i-1])collapsed[out++]=s[i];
    return (out!=n&&member(table,collapsed,out)) ||
        (n==1&&s[0]=='q'&&member(table,collapsed,out));
}

static int sayable(const char *word,size_t n,int uk)
{
    struct wordseg_table onsets=uk?TABLE(wordseg_engb_onset):TABLE(wordseg_enus_onset);
    struct wordseg_table codas=uk?TABLE(wordseg_engb_coda):TABLE(wordseg_enus_coda);
    size_t first=0;while(first<n&&!vowel(word,first))++first;
    if(first==n||(first&&!member(onsets,word,first)))return 0;
    size_t last=n-1;while(!vowel(word,last))--last;
    if(!coda(word+last+1,n-last-1,codas))return 0;
    for(size_t i=first;i<last;) {
        while(i<last&&vowel(word,i))++i;
        if(i==last)break;
        size_t j=i;while(j<n&&!vowel(word,j))++j;
        size_t k=0;
        for(;k<=j-i;++k)
            if(coda(word+i,k,codas)&&(!(j-i-k)||member(onsets,word+i+k,j-i-k)))break;
        if(k>j-i)return 0;
        i=j;
    }
    return 1;
}

static int stem(const char *word,size_t n)
{
    if(n<2)return 0;
    struct wordseg_table pieces=TABLE(wordseg_pieces);
    if(member(pieces,word,n))return 1;
    char temp[34];memcpy(temp,word,n);temp[n]='e';
    if(member(pieces,temp,n+1))return 1;
    return n>3&&word[n-1]==word[n-2]&&
        word[n-1]!='a'&&word[n-1]!='e'&&word[n-1]!='i'&&word[n-1]!='o'&&word[n-1]!='u'&&
        member(pieces,word,n-1);
}

static int word_piece(const char *word,size_t n)
{
    static const char *const endings[]={"ings","ing","ers","er","ed","es","s"};
    if(member(TABLE(wordseg_pieces),word,n))return 1;
    for(size_t i=0;i<COUNT(endings);++i) {
        const char *end=endings[i];size_t k=strlen(end);
        if(n<k+3||memcmp(word+n-k,end,k))continue;
        if(k==1&&word[n-2]=='s')continue;
        if(k==2&&end[0]=='e'&&end[1]=='s'&&
           !strchr("sxzh",word[n-3]))continue;
        if(stem(word,n-k))return 1;
    }
    return 0;
}

static int suffix_order(const char *value,const char *suffix,size_t n)
{
    size_t length=strlen(value),common=length<n?length:n;
    int order=memcmp(value,suffix,common);
    return order?order:(length>n)-(length<n);
}

static int known_exact(const char *word,size_t n)
{
    if(n<2||word[0]<'a'||word[0]>'z'||word[1]<'a'||word[1]>'z')return 0;
    size_t bucket=(size_t)(word[0]-'a')*26+(size_t)(word[1]-'a');
    size_t begin=wordseg_known_start[bucket],lo=begin,hi=wordseg_known_start[bucket+1];
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(suffix_order(wordseg_known_chunks[mid].words,word+2,n-2)<=0)lo=mid+1;
        else hi=mid;
    }
    if(lo==begin)return 0;
    const struct wordseg_known_chunk *chunk=&wordseg_known_chunks[lo-1];
    const char *value=chunk->words;
    for(unsigned i=0;i<chunk->count;++i) {
        int order=suffix_order(value,word+2,n-2);
        if(!order)return 1;
        if(order>0)return 0;
        value+=strlen(value)+1;
    }
    return 0;
}

static int known_stem(const char *word,size_t n)
{
    return known_exact(word,n)||member(TABLE(wordseg_pieces),word,n);
}

/* Inflections belong to an established word too. Do not replace the native
 * pronunciation simply because the inflected spelling can be divided. */
static int known_word(const char *word,size_t n)
{
    if(known_stem(word,n))return 1;
    static const char *const endings[]={"ings","ing","ers","er","ed","es","s"};
    for(size_t i=0;i<COUNT(endings);++i) {
        size_t k=strlen(endings[i]);if(n<k+3||memcmp(word+n-k,endings[i],k))continue;
        if(k==1&&word[n-2]=='s')continue;
        if(k==2&&endings[i][0]=='e'&&endings[i][1]=='s'&&!strchr("sxzh",word[n-3]))continue;
        size_t root=n-k;
        if(known_stem(word,root))return 1;
        char restored[34];memcpy(restored,word,root);restored[root]='e';
        if(known_stem(restored,root+1))return 1;
        if(root>3&&word[root-1]==word[root-2]&&!strchr("aeiou",word[root-1])&&
           known_stem(word,root-1))return 1;
    }
    if(n>4&&!memcmp(word+n-3,"ies",3)) {
        char singular[34];memcpy(singular,word,n-3);singular[n-3]='y';
        if(known_stem(singular,n-2))return 1;
    }
    return 0;
}

static int leave_word(const char *word,size_t n,int uk)
{
    if(n<2||n>32)return 1;
    int has_vowel=0,roman=1;
    for(size_t i=0;i<n;++i){has_vowel|=vowel(word,i);roman&=strchr("ivxlc",word[i])!=NULL;}
    if(!has_vowel||roman||(n>=2&&word[0]=='m'&&word[1]=='c'))return 1;
    struct wordseg_table protected=uk?TABLE(wordseg_engb_word):TABLE(wordseg_enus_word);
    if(member(protected,word,n)||word_piece(word,n)||known_word(word,n))return 1;
    if(!sayable(word,n,uk))return 0;
    for(size_t i=3;i+3<=n;++i)
        if(word_piece(word,i)&&word_piece(word+i,n-i))return 0;
    return 1;
}

static int append(unsigned char *out,size_t capacity,size_t *used,const char *text,size_t n)
{
    if(*used&&*used+1>=capacity)return -1;
    if(*used)out[(*used)++]=' ';
    if(n>=capacity-*used)return -1;
    memcpy(out+*used,text,n);*used+=n;out[*used]=0;return 0;
}

int reloq_wordseg_english(const unsigned char *original,size_t n,int uk,
    unsigned char *output,size_t capacity)
{
    if(!original||!output||!capacity||n>32)return -1;
    char word[33];
    for(size_t i=0;i<n;++i) {
        unsigned c=original[i];
        if(c>='A'&&c<='Z')c+=32;
        if(c<'a'||c>'z')return 0;
        word[i]=(char)c;
    }
    word[n]=0;
    const struct wordseg_verified_compound *verified=verified_find(word,uk);
    if(verified) {
        size_t used=0;output[0]=0;
        if(append(output,capacity,&used,word,verified->boundary)||
           append(output,capacity,&used,word+verified->boundary,n-verified->boundary))return -1;
        return 1;
    }
    if(leave_word(word,n,uk))return 0;
    const int inf=0x3fffffff;
    int letters[33],other[33];size_t target[33];
    for(size_t i=0;i<=n;++i){letters[i]=other[i]=inf;target[i]=0;}
    other[n]=0;
    for(size_t i=n;i-->0;) {
        int a=letters[i+1],b=other[i+1]+1;
        letters[i]=30+(a<b?a:b);
        for(size_t j=i+2;j<=n;++j) {
            int cost=word_piece(word+i,j-i)?39:
                j-i>=3&&sayable(word+i,j-i,uk)?132:inf;
            int next=letters[j]<other[j]?letters[j]:other[j];
            if(cost==inf||next==inf)continue;
            int score=next+cost;
            if(score<other[i]){other[i]=score;target[i]=j;}
        }
    }
    int by_letter=letters[0]<=other[0];size_t at=0,used=0;output[0]=0;
    while(at<n) {
        size_t end;
        if(by_letter) {
            end=at+1;
            if(word[at]=='a'||word[at]=='i') {
                const char *v=word[at]=='a'?"ay":"igh";
                if(append(output,capacity,&used,v,strlen(v)))return -1;
            } else {
                char c=(char)(word[at]-'a'+'A');
                if(append(output,capacity,&used,&c,1))return -1;
            }
            by_letter=letters[end]<other[end]+1;
        } else {
            end=target[at];if(end<=at)return 0;
            if(member(TABLE(wordseg_abbreviations),word+at,end-at)) {
                for(size_t i=at;i<end;++i){char c=(char)(word[i]-'a'+'A');
                    if(append(output,capacity,&used,&c,1))return -1;}
            } else if(append(output,capacity,&used,word+at,end-at))return -1;
            by_letter=letters[end]<other[end];
        }
        at=end;
    }
    return 1;
}
