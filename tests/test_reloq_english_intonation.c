#include "reloq_english_intonation.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
    struct reloq_english_tone_point p[2];
    struct reloq_english_nuclear_context c={RELOQ_EN_TONE_HIGH,0,0,0,0};
    assert(!reloq_english_nuclear_tone(&c,p));
    assert(p[0].percent==10 && p[0].level==50 && p[1].percent==40 && p[1].level==100);
    c.first_word=1;c.first_accent=1;
    assert(!reloq_english_nuclear_tone(&c,p) && p[1].percent==60 && p[1].level==120);
    c.tone=RELOQ_EN_TONE_LOW;
    assert(!reloq_english_nuclear_tone(&c,p) && p[1].percent==15 && p[1].level==25);
    c.last_word=1;
    assert(!reloq_english_nuclear_tone(&c,p) && p[1].percent==60);
    /* Statement, continuing phrase, and question tails are independent
     * of nuclear accent location. A question takes priority over comma. */
    assert(!reloq_english_final_tone(0,0,0,1,p));
    assert(p[0].percent==55 && p[0].level==-20 && p[1].percent==99 && p[1].level==-35);
    assert(!reloq_english_final_tone(0,1,1,0,p) && p[0].percent==70 && p[1].level==0);
    assert(!reloq_english_final_tone(1,1,0,0,p) && p[0].percent==85 && p[0].level==100 && p[1].level==150);
    int64_t tick=-1;
    assert(!reloq_english_tone_tick(100,165,50,&tick) && tick==132);
    assert(!reloq_english_tone_tick(0,INT64_MAX,99,&tick) && tick==INT64_C(9131138316486228048));
    assert(reloq_english_tone_tick(100,99,50,&tick)<0);
    int value=0;
    assert(!reloq_english_tone_value(1189.15,30,3,0,50,&value) && value==1046);
    assert(!reloq_english_tone_value(1189.15,0,3,2,150,&value) && value==1189);
    assert(reloq_english_tone_value(NAN,30,3,0,50,&value)<0);
    c.tone=(enum reloq_english_nuclear_tone)-1;
    p[0].percent=77;assert(reloq_english_nuclear_tone(&c,p)<0 && p[0].percent==77);
    puts("US native nuclear and phrase tone scalars passed");return 0;
}
