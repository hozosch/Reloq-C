#include "reloq61_german_clock.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *cases[][2]={
        {"8:05", "8 `00 Uhr 5"}, {"08:5 Uhr", "8 `00 Uhr 5"},
        {"08.05 Uhr", "8 `00 Uhr 5"}, {"01:00", "1 Uhr"},
        {"01:05", "`[.1'aj'n] `00 Uhr 5"},
        {"08:05:03", "8 `00 Uhr 5 und 3 Sekunden"},
        {"08:00:00 Uhr", "8 Uhr und 0 Sekunden"},
        {"08:01:01", "8 `00 Uhr 1 und eine Sekunde"},
        {"Um 8:05 und 13:28:25 Uhr.", "Um 8 `00 Uhr 5 und 13 `00 Uhr 28 und 25 Sekunden."},
        {"`[.1t:es] 8:05", "`[.1t:es] 8 `00 Uhr 5"},
        {"`ts1 8:05 `ts0 8:05", "`ts1 8:05 `ts0 8 `00 Uhr 5"},
        {"`l1 8:05 `l4 8:05", "`l1 8:05 `l4 8 `00 Uhr 5"},
        {"1  234 567,89 EUR", "1.234.567,89 EUR"},
        {"1\xa0" "234,56", "1.234,56"},
        {"`ts1 1  234 `ts0 1  234", "`ts1 1  234 `ts0 1.234"},
        {"`l1 1  234 `l4 1  234", "`l1 1  234 `l4 1.234"}
    };
    for(size_t i=0;i<sizeof cases/sizeof *cases;++i) {
        char *text=NULL;size_t *map=NULL;
        assert(reloq61_german_clock_text(cases[i][0],1,0x40000,&text,&map)==1);
        if(strcmp(text,cases[i][1]))fprintf(stderr,"%s => %s\n",cases[i][0],text);
        assert(!strcmp(text,cases[i][1]));
        size_t n=strlen(cases[i][0]),m=strlen(text);
        assert(map[0]==0&&map[n]==m);
        for(size_t j=0;j<n;++j)assert(map[j]<=map[j+1]&&map[j+1]<=m);
        free(text);free(map);
    }
    const char *unchanged[]={"25:00","12:60","12:30:60","12:30:45:10",
        "1.2.3","1.23","08.10.2026","x8:05","8:05x","8:05/3",
        "`[8:05]","`ts1 8:05","1 23","1234 567","1 2345","1 234a"};
    for(size_t i=0;i<sizeof unchanged/sizeof *unchanged;++i) {
        char *text=NULL;size_t *map=NULL;
        assert(reloq61_german_clock_text(unchanged[i],1,0x40000,&text,&map)==0);
        assert(!text&&!map);
    }
    puts("German clocks and grouped numbers: forms, source maps, controls and exclusions passed");
    return 0;
}
