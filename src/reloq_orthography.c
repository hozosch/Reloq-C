#include "reloq_orthography.h"
#include <string.h>

static unsigned char base(unsigned char c)
{
    switch(c) {
    case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe5:return 'a';
    case 0xe6:return 0xe4;
    case 0xe7:return 'c';
    case 0xf0:return 'd';
    /* Acute e retains the language's separate lexical vowel category. */
    case 0xe8: case 0xea: case 0xeb:return 'e';
    case 0xec: case 0xed: case 0xee: case 0xef:return 'i';
    case 0xf1:return 'n';
    case 0xf2: case 0xf3: case 0xf4: case 0xf5:return 'o';
    case 0xf8:return 'o';
    case 0x9c:return 0xf6;
    case 0xf9: case 0xfa: case 0xfb:return 'u';
    case 0xfd:return 'y';
    case 0x9a:return 's';
    default:return c;
    }
}

static int apostrophe_ending(const struct reloq_orthographic_letter *s,size_t n)
{
    if(n!=4 && n!=5)return 0;
    static const unsigned char beginning[]="sche";
    for(size_t i=0;i<4;++i)if(s[i].spelling!=beginning[i])return 0;
    return n==4 || (s[4].spelling && strchr("nmrs",s[4].spelling)!=NULL);
}

size_t reloq_german_orthography(const struct reloq_orthographic_letter *word,
    size_t length,struct reloq_orthographic_change *changes)
{
    size_t count=0;unsigned char previous=0;
    for(size_t i=0;i<length;++i) {
        unsigned char c=word[i].spelling,normalized=base(c);
        struct reloq_orthographic_change change={RELOQ_ORTHOGRAPHY_KEEP,c};
        if(c!=normalized) {
            change.action=RELOQ_ORTHOGRAPHY_REPLACE;change.spelling=normalized;
        } else if(c=='\'' && !apostrophe_ending(word+i+1,length-i-1))
            change.action=RELOQ_ORTHOGRAPHY_DELETE;
        else if(i+1<length && c=='u' && previous=='q' && word[i+1].vowel)
            change.action=RELOQ_ORTHOGRAPHY_CONSONANT;
        changes[i]=change;count+=change.action!=RELOQ_ORTHOGRAPHY_KEEP;
        if(change.action!=RELOQ_ORTHOGRAPHY_DELETE)previous=change.spelling;
    }
    return count;
}
