#include "reloq_english_tract.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    struct reloq_english_tract_span p[2];size_t n;
    assert(!reloq_english_tract_targets(1450,1851,100,115,p,&n));
    assert(n==2&&p[0].value==1650&&p[1].value==1650);
    assert(!reloq_english_tract_targets(1450,1851,100,116,p,&n));
    assert(n==2&&p[0].value==1450&&p[1].value==1851);
    assert(!reloq_english_tract_targets(-2,-2,100,200,p,&n)&&n==0);
    assert(!reloq_english_tract_targets(-1,1500,100,200,p,&n));
    assert(n==1&&p[0].begin==100&&p[0].end==200&&p[0].value==1500);
    assert(!reloq_english_tract_targets(-2,1500,100,200,p,&n));
    assert(n==1&&p[0].begin==200&&p[0].end==200);
    assert(reloq_english_tract_targets(500,1000,200,100,p,&n)==-1);
    assert(!reloq_english_tract_nuclear('t'));
    assert(!reloq_english_tract_nuclear('n'));
    assert(reloq_english_tract_nuclear('r'));
    assert(reloq_english_tract_nuclear('X'));
    puts("English tract: absent ownership, target retention and exact 16-tick contraction boundary passed");
    return 0;
}
