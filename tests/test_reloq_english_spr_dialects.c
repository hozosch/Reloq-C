#include "reloq_english.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    struct reloq_english_phone *phones=NULL;
    size_t count=99,error=0;
    const char *british="`[.1sw@p.0dL]";
    assert(reloq_english_parse((const unsigned char *)british,strlen(british),
        &phones,&count,&error)==RELOQ_ENGLISH_INVALID);
    assert(!phones&&count==99);
    assert(reloq_english_parse_british((const unsigned char *)british,strlen(british),
        &phones,&count,&error)==RELOQ_ENGLISH_OK);
    assert(count==6&&phones[2].phone=='@'&&!phones[2].syllabic);
    assert(phones[5].phone=='l'&&phones[5].syllabic&&phones[5].stress==0);
    free(phones);phones=NULL;
    const char *invalid[]={"`[.1]","`[.1sw@p.0]","`[.1sw#p]","`[.1sw@p"};
    for(size_t i=0;i<sizeof invalid/sizeof *invalid;++i) {
        count=99;
        assert(reloq_english_parse_british((const unsigned char *)invalid[i],strlen(invalid[i]),
            &phones,&count,&error)==RELOQ_ENGLISH_INVALID);
        assert(!phones&&count==99);
    }
    return 0;
}
