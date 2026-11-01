#include "reloq61_english_clock.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port/evv_arena.h"

int main(void)
{
    const char *cases[][2]={
        {"13:28:25","13:28 and 25 seconds"},
        {"1:02:03","1:02 and 3 seconds"},
        {"00:00:00.","00:00 and 0 seconds."},
        {"13:28:01.5","13:28 and 1.5 seconds"},
        {"00:20:48.526","00:20 and 48.526 seconds"},
        {"12:00:01 pm","12:00 and 1 second pm"},
        {"13:28:01.000.","13:28 and 1.000 second."},
        {"At 13:28:25, then 1:02:03.","At 13:28 and 25 seconds, then 1:02 and 3 seconds."},
        {"`[.1t:es] 13:28:25","`[.1t:es] 13:28 and 25 seconds"},
        {"`ts1 13:28:25 `ts0 13:28:01","`ts1 13:28:25 `ts0 13:28 and 1 second"},
        {"`l4 13:28:25 `l1 13:28:01","`l4 13:28:25 `l1 13:28 and 1 second"}
    };
    for(unsigned i=0;i<sizeof cases/sizeof *cases;++i) {
        char *text=NULL;size_t *map=NULL;
        assert(reloq61_english_clock_text(cases[i][0],1,0x10000,&text,&map)==1);
        if(strcmp(text,cases[i][1])){fprintf(stderr,"%s => %s\n",cases[i][0],text);abort();}
        size_t n=strlen(cases[i][0]),m=strlen(text);
        assert(map[0]==0&&map[n]==m);
        for(size_t k=0;k<n;++k)assert(map[k]<=map[k+1]&&map[k+1]<=m);
        free(text);free(map);
    }
    const char *unchanged[]={"13:28","13:28:60","13:60:00","123:28:25",
        "24:01:01","name13:28:25","13:28:25name","13:28:25:50",
        "`[13:28:25]","`ts1 13:28:25"};
    for(unsigned i=0;i<sizeof unchanged/sizeof *unchanged;++i) {
        char *text=NULL;size_t *map=NULL;
        assert(reloq61_english_clock_text(unchanged[i],1,0x10000,&text,&map)==0);
        assert(!text&&!map);
    }
    puts("English clocks: native lexical projection, seconds/fractions/singular, source maps, annotations and rejected identifiers passed");
    return 0;
}
