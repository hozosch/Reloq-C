#ifndef RELOQ_ENGLISH_CURRENCY_H
#define RELOQ_ENGLISH_CURRENCY_H

#include <stddef.h>
#include "reloq_inline_tags.h"

static int reloq_currency_digit(unsigned char c) {return c>='0' && c<='9';}
static int reloq_currency_word(unsigned char c)
{return (c>='A'&&c<='Z') || (c>='a'&&c<='z') || reloq_currency_digit(c) || c=='_' || c>=0x80;}

/* Normalize the magnitude marker in a complete currency token. B and b are
 * the same abbreviation here; retain the provider's existing number and unit
 * grammar. This does not alter offsets or touch spelling/phonetic controls. */
static void reloq_english_currency_case(unsigned char *text,size_t n,int tags)
{
    int spelling=0;
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(text+i,n-i,0);
        if(t.length && (tags || t.kind==RELOQ_TAG_SPR)) {
            if(t.kind==RELOQ_TAG_SPELL)spelling=t.value;
            i+=t.length;continue;
        }
        unsigned char c=text[i];
        if(spelling || (c!='$' && c!=0xa3 && c!=0x80) ||
           (i && reloq_currency_word(text[i-1]))) {++i;continue;}
        size_t at=i+1,start=at;
        while(at<n && reloq_currency_digit(text[at]))++at;
        if(at==start){++i;continue;}
        /* Group separators are accepted only in complete groups of three. */
        int valid=1;
        while(at<n && text[at]==',' && at+1<n && reloq_currency_digit(text[at+1])) {
            if(at+3>=n || !reloq_currency_digit(text[at+2]) ||
               !reloq_currency_digit(text[at+3]) ||
               (at+4<n && reloq_currency_digit(text[at+4]))) {valid=0;break;}
            at+=4;
        }
        if(valid && at+1<n && text[at]=='.' && reloq_currency_digit(text[at+1])) {
            at+=2;while(at<n && reloq_currency_digit(text[at]))++at;
        }
        if(valid && at<n && text[at]=='B' &&
           (at+1==n || (!reloq_currency_word(text[at+1]) &&
                        !(text[at+1]=='.' && at+2<n && reloq_currency_digit(text[at+2])))))
            text[at]='b';
        ++i;
    }
}
#endif
