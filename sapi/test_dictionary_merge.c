#include "dictionary_merge.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void check(const char *one,const char *two,int fold,const char *expected)
{
    struct sapi_dictionary_input input[]={{(const unsigned char *)one,strlen(one)},
                                          {(const unsigned char *)two,strlen(two)}};
    unsigned char *out=NULL;size_t size=0;
    assert(sapi_dictionary_merge(input,2,fold,&out,&size));
    assert(size==strlen(expected)&&!memcmp(out,expected,size));free(out);
}
int main(void)
{
    check("a\tcommunity\nB\tbase\n", "a\tfirst\na\tlast\nnew\tentry\n",0,
          "B\tbase\na\tlast\nnew\tentry\n");
    check("Haus\told\n", "haus\tnew\n",1,"haus\tnew\n");
    check("Haus\told\n", "haus\tnew\n",0,"Haus\told\nhaus\tnew\n");
    check(" # comment\nblank\t  \n key \t value \r\n", "",0,"key\tvalue\n");
    unsigned char nul[]={'a','\t','b',0,'x'};struct sapi_dictionary_input input={nul,sizeof nul};
    unsigned char *out=NULL;size_t size=0;assert(!sapi_dictionary_merge(&input,1,0,&out,&size));
    puts("Dictionary merge: source priority, case rules, malformed input and originals passed");
    return 0;
}
