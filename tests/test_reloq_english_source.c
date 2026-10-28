#include "reloq_english_source.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    int64_t first=-7,end=-9;
    assert(!reloq_english_source_window(130,30,229,&first,&end));
    assert(first==101&&end==329);
    assert(reloq_english_source_window(INT64_MAX,0,1,&first,&end)==-1);
    assert(first==101&&end==329);
    assert(reloq_english_source_window(10,11,1,&first,&end)==-1);
    assert(reloq_english_source_window(0,0,0,&first,&end)==-1);
    assert(!reloq_english_source_plan(NULL,0,NULL));
    struct reloq_english_source_phone p[5]={
        {.phone='i',.stress=1,.word_start=1,.syllable_start=1,.end=100,.release=165},
        {.phone='r',.start=100,.contact=165,.end=195,.release=220},
        {.phone='s',.start=195,.contact=220,.end=310,.release=311},
        {.phone='t',.start=310,.contact=311,.end=371,.release=401},
        {.phone='a',.start=371,.contact=401,.end=500,.release=530}};
    struct reloq_english_source_span s[5];
    assert(!reloq_english_source_plan(p,5,s));
    assert(s[0].first==1&&s[0].end==220&&s[0].last==1&&s[1].owner==0);
    assert(s[4].early&&s[4].unaspirated_stop&&s[4].first==372);
    p[3].word_start=1;
    assert(!reloq_english_source_plan(p,5,s));
    assert(!s[4].early&&s[4].first==402);
    p[4].word_start=1;
    assert(!reloq_english_source_plan(p,5,s));
    assert(s[4].early&&s[4].first==372);
    p[4].aspirated=1;
    assert(!reloq_english_source_plan(p,5,s));
    assert(!s[4].early&&s[4].first==402);
    p[1].syllable_start=1;
    assert(!reloq_english_source_plan(p,5,s));
    assert(s[1].owner==1&&s[1].voiced_sonorant&&s[1].first==165);
    assert(reloq_english_source_needs_plateau('r',s+1,0,0,1,1));
    assert(!reloq_english_source_needs_plateau('r',s+1,1,0,1,1));
    assert(!reloq_english_source_needs_plateau('n',s+1,0,0,0,1));
    struct reloq_english_source_span saved[5];memcpy(saved,s,sizeof s);
    p[4].end=p[4].contact-1;
    assert(reloq_english_source_plan(p,5,s)==-1&&!memcmp(s,saved,sizeof s));
    assert(reloq_english_source_boundary_contact('t',1)==20);
    assert(reloq_english_source_boundary_contact('k',1)==30);
    assert(reloq_english_source_boundary_contact('p',0)==-1);
    puts("English source: joint nucleus ownership, S+stop/word/h boundaries and invalid-input atomicity passed");
    return 0;
}
