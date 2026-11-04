#include "dictionary_diff.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check(const char *before,const char *after,int fold,size_t added,size_t total)
{
    size_t a=0,t=0;
    assert(sapi_dictionary_diff((const unsigned char *)before,strlen(before),
        (const unsigned char *)after,strlen(after),fold,&a,&t));
    assert(a==added&&t==total);
}
int main(void)
{
    check("Keep\told\nDelete\told\nDuplicate\tfirst\nDuplicate\tsecond\n",
          "Keep\tchanged\nDuplicate\tsecond\nAdded\tnew\nAdded\tnewer\n",0,1,3);
    check("READ\told\n","read\tchanged\nNEW\troot\n",1,1,2);
    check("","word\tone\nword\ttwo\nOTHER\tthree\n",0,2,2);
    check("word\tone\n","word\tdifferent\n",0,0,1);
    check("old\tone\n","",0,0,0);
    size_t added,total;const unsigned char invalid[]={ 'x','\t','y',0,'\n' };
    assert(!sapi_dictionary_diff(NULL,0,invalid,sizeof invalid,0,&added,&total));
    puts("Dictionary update counts: additions, duplicate keys, removals, value edits and root case passed");
    return 0;
}
