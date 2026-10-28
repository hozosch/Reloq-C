#include "reloq_english.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct english_word {const char *text,*isolated,*vowel,*consonant;};
#include "reloq_english_lexicon.inc"
#include "reloq_english_vocabulary82.inc"
#include "reloq_english_vocabulary83.inc"

size_t reloq_english_lexicon_size(void)
{return sizeof english_words/sizeof *english_words;}

int reloq_english_primary_word(const unsigned char *word,size_t n,
    const char **isolated,const char **before_vowel,const char **before_consonant)
{
    if(!word || !n || !isolated || !before_vowel || !before_consonant ||
       memchr(word,0,n))return RELOQ_ENGLISH_INVALID;
    size_t lo=0,hi=reloq_english_lexicon_size();
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2,k=strlen(english_words[mid].text);
        int order=memcmp(word,english_words[mid].text,n<k?n:k);
        if(!order)order=n<k?-1:n>k?1:0;
        if(order<0)hi=mid;
        else if(order>0)lo=mid+1;
        else {
            const struct english_word *w=english_words+mid;
            *isolated=w->isolated;*before_vowel=*w->vowel?w->vowel:NULL;
            *before_consonant=*w->consonant?w->consonant:NULL;
            return RELOQ_ENGLISH_OK;
        }
    }
    return RELOQ_ENGLISH_UNCOVERED;
}

int reloq_english_word(const unsigned char *word,size_t n,
    const char **isolated,const char **before_vowel,const char **before_consonant)
{
    int result=reloq_english_primary_word(word,n,isolated,before_vowel,before_consonant);
    if(result!=RELOQ_ENGLISH_UNCOVERED)return result;
    size_t lo=0,hi=sizeof english_supplement82/sizeof *english_supplement82;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2,k=strlen(english_supplement82[mid].text);
        int order=memcmp(word,english_supplement82[mid].text,n<k?n:k);
        if(!order)order=n<k?-1:n>k?1:0;
        if(order<0)hi=mid;
        else if(order>0)lo=mid+1;
        else {
            *isolated=english_supplement82[mid].isolated;
            *before_vowel=*before_consonant=NULL;return RELOQ_ENGLISH_OK;
        }
    }
    lo=0;hi=sizeof english_supplement83/sizeof *english_supplement83;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2,k=strlen(english_supplement83[mid].text);
        int order=memcmp(word,english_supplement83[mid].text,n<k?n:k);
        if(!order)order=n<k?-1:n>k?1:0;
        if(order<0)hi=mid;
        else if(order>0)lo=mid+1;
        else {
            *isolated=english_supplement83[mid].isolated;
            *before_vowel=*before_consonant=NULL;return RELOQ_ENGLISH_OK;
        }
    }
    return RELOQ_ENGLISH_UNCOVERED;
}

