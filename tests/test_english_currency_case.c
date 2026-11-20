#include <stdio.h>
#include <string.h>
#include "../src/reloq_english_currency.h"

int main(void)
{
    static const struct {const char *source,*expected;int tags;} cases[]={
        {"$2B", "$2b", 1}, {"Cost $2B today.", "Cost $2b today.", 1},
        {"$1B, $2.5B, $1,234B", "$1b, $2.5b, $1,234b", 1},
        {"\xA3" "2B / \x80" "2B", "\xA3" "2b / \x80" "2b", 1},
        {"$2b $2M 2B", "$2b $2M 2B", 1},
        {"$2Billion $2Bk $2B_foo A$2B", "$2Billion $2Bk $2B_foo A$2B", 1},
        {"$12,34B $2B.5", "$12,34B $2B.5", 1},
        {"`ts1 $2B `ts0 $2B", "`ts1 $2B `ts0 $2b", 1},
        {"`[.1$2B] $2B", "`[.1$2B] $2b", 1},
        {"`ts1 $2B", "`ts1 $2b", 0}
    };
    for(size_t i=0;i<sizeof cases/sizeof *cases;++i) {
        unsigned char text[128];size_t n=strlen(cases[i].source);
        if(n>=sizeof text)return 1;
        memcpy(text,cases[i].source,n+1);
        reloq_english_currency_case(text,n,cases[i].tags);
        if(strcmp((char *)text,cases[i].expected)) {
            fprintf(stderr,"currency case %zu failed\n",i);return 1;
        }
    }
    puts("English currency case and protected-context checks passed.");return 0;
}
