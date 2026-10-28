#include "reloq_german_nucleus_schedule.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int cancel(void *p) {return ++*(int *)p == 2;}

static void check(const char *spr, size_t head, const int32_t expected[6])
{
    struct reloq_german_phone_node phones[24];
    struct reloq_german_phone_sequence_info info;
    assert(!reloq_german_phone_sequence_build((const unsigned char *)spr,
        strlen(spr),0,phones,24,&info));
    struct reloq_german_syllable syllables[16];
    size_t count=0;
    assert(!reloq_german_syllable_plan_build(phones,info.phone_count,syllables,16,&count));
    struct reloq_german_nucleus_schedule timing[24];
    assert(!reloq_german_nucleus_schedule_build(phones,info.phone_count,
        syllables,count,NULL,0,timing,NULL,NULL));
    const struct reloq_german_nucleus_schedule *t=timing+head;
    const int32_t actual[]={t->span_ticks,t->incoming_ticks,t->vowel_ticks,
                           t->glide_ticks,t->closing_ticks,t->outgoing_ticks};
    assert(!memcmp(expected,actual,sizeof actual));
    int calls=0;
    assert(reloq_german_nucleus_schedule_build(phones,info.phone_count,
        syllables,count,NULL,0,timing,cancel,&calls)==1);
    syllables[0].phone_count=info.phone_count+1;
    assert(reloq_german_nucleus_schedule_build(phones,info.phone_count,
        syllables,count,NULL,0,timing,NULL,NULL)==-1);
}

int main(void)
{
    /* Values are normalized behavior observations. The bridge derives the
     * entire context and transition allocation from the native annotation. */
    check("`00 `[.0dAs] `[.1mA].",1,(int32_t[]){54,27,3,0,0,24});
    check("`[.1mat].",1,(int32_t[]){168,15,103,0,0,50});
    check("`[.1m'aj'].",1,(int32_t[]){232,15,122,75,20,0});
    check("`[.1kA].",1,(int32_t[]){87,0,87,0,0,0});
    check("`[.1fEn.0stR].",5,(int32_t[]){105,18,87,0,0,0});
    check("`[.1skA].",2,(int32_t[]){97,18,79,0,0,0});
    check("`[.0skA].",2,(int32_t[]){97,50,47,0,0,0});
    check("`[.1mA.2skA].",4,(int32_t[]){97,50,47,0,0,0});
    /* The observed nasal-stop onset owns a 28-tick vowel transition. */
    check("`[.1mA.0ntA].",4,(int32_t[]){97,28,69,0,0,0});
    /* One measured budget is now split between vowel and closing liquid. */
    check("`[.1mAr].",1,(int32_t[]){143,15,88,0,0,40});
    check("`[.1mAr].",2,(int32_t[]){25,0,0,0,25,0});
    check("`[.1mAl].",1,(int32_t[]){134,15,59,0,0,60});
    check("`[.1mAl].",2,(int32_t[]){40,0,0,0,40,0});
    puts("German shared nucleus scheduling checks passed");
    return 0;
}
