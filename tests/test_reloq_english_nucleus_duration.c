#include "reloq_english_nucleus_duration.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    assert(reloq_english_nucleus_rate_percent(40)==122);
    assert(reloq_english_nucleus_rate_percent(50)==100);
    assert(reloq_english_nucleus_rate_percent(75)==62);
    assert(reloq_english_nucleus_rate_percent(39)==-1);
    struct reloq_english_nucleus result,before;
    memset(&result,0x5a,sizeof result);before=result;
    assert(reloq_english_nucleus_find(NULL,0,&result)==0);
    assert(!memcmp(&result,&before,sizeof result));
    assert(reloq_english_nucleus_base(NULL,1,&result)==-1);
    assert(!memcmp(&result,&before,sizeof result));

    /* A closed single vowel has no second member to inspect. Keep this
     * as a one-element stack array for ASan/UBSan, not padded storage. */
    const struct reloq_english_nucleus_phone single[1]={{
        .phone='x',.syllabic=1,.quantity=1,.closed=1,.following='t'}};
    assert(reloq_english_nucleus_base(single,1,&result)==1);
    assert(result.count==1&&result.base_ticks==70);

    const struct reloq_english_nucleus_phone closed[3]={
        {.phone='x',.syllabic=1,.quantity=1}, {.phone='l'}, {.phone='t'}};
    assert(reloq_english_nucleus_find(closed,3,&result)==1);
    assert(result.count==2&&result.transition_ticks[0]==30);
    const struct reloq_english_nucleus_phone zero[1]={{0}};
    before=result;
    assert(reloq_english_nucleus_find(zero,1,&result)==0);
    assert(!memcmp(&result,&before,sizeof result));

    /* Original shortage contract: individual rounding is observable.
     * The final sum need not equal the fractional requested budget. */
    const struct reloq_english_nucleus_interval in[4]={
        {45,1},{75,1},{10,1},{15,1}};
    int32_t fixed[4]={0},free_ticks=-1;
    const int32_t expected[4]={39,65,9,13};
    assert(!reloq_english_nucleus_distribute(in,4,130,3,fixed,&free_ticks));
    assert(!memcmp(fixed,expected,sizeof fixed)&&free_ticks==3);
    assert(!reloq_english_nucleus_distribute(NULL,0,75.5,3,NULL,&free_ticks));
    assert(free_ticks==75);
    assert(!reloq_english_nucleus_distribute(NULL,0,0,3,NULL,&free_ticks));
    assert(free_ticks==3);

    int32_t saved[4];memcpy(saved,fixed,sizeof fixed);free_ticks=91;
    assert(reloq_english_nucleus_distribute(in,4,NAN,3,fixed,&free_ticks)==-1);
    assert(!memcmp(saved,fixed,sizeof fixed)&&free_ticks==91);
    const struct reloq_english_nucleus_interval overflow[2]={{32767,1},{1,1}};
    assert(reloq_english_nucleus_distribute(overflow,2,300,3,fixed,&free_ticks)==-1);
    assert(!memcmp(saved,fixed,sizeof fixed)&&free_ticks==91);
    /* Complete original stage contrasts: voicing, word boundary and the
     * distinction between a short vowel and a whole closing nucleus. */
    struct reloq_english_nucleus_phone high[]={{.phone='i',.syllabic=1,.quantity=1}};
    double budget=-1;
    assert(reloq_english_nucleus_terminal_budget(high,1,'t',&budget)==1&&budget==160);
    assert(reloq_english_nucleus_terminal_budget(high,1,'d',&budget)==1&&budget==230);
    assert(reloq_english_nucleus_terminal_budget(high,1,'v',&budget)==1&&budget==253);
    assert(reloq_english_nucleus_medial_budget(high,1,0,'m',&budget)==1&&budget==154);
    assert(reloq_english_nucleus_medial_budget(high,1,'b','t',&budget)==1&&budget==170);
    int32_t outgoing=-1,incoming=-1;
    assert(reloq_english_nucleus_hiatus_ticks('i','a',&outgoing,&incoming)==1);
    assert(outgoing==32&&incoming==33);
    assert(reloq_english_nucleus_hiatus_ticks('X','i',&outgoing,&incoming)==1);
    assert(outgoing==0&&incoming==0);
    assert(reloq_english_nucleus_hiatus_ticks('l','u',&outgoing,&incoming)==1);
    assert(outgoing==17&&incoming==18);
    assert(reloq_english_nucleus_exit_ticks('w','t')==40);
    assert(reloq_english_nucleus_exit_ticks('r','n')==15);
    budget=99;
    assert(reloq_english_nucleus_terminal_budget(high,1,'?',&budget)==0&&budget==99);
    puts("English nucleus: interval contracts, rounding, invalid input and single-member bounds passed");
    return 0;
}
