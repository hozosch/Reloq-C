#include "reloq_wordseg.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void check(const char *source,const char *expected,int uk)
{
    unsigned char result[256]={0};
    int changed=reloq_wordseg_english((const unsigned char *)source,
        strlen(source),uk,result,sizeof result);
    if(expected) {
        if(changed!=1||strcmp((const char *)result,expected)) {
            fprintf(stderr,"word segmentation %s: got %d '%s', expected '%s'\n",
                    source,changed,result,expected);
            assert(0);
        }
    } else assert(changed==0);
}

int main(void)
{
    for(int uk=0;uk<2;++uk) {
        check("usrdct","U S R D C T",uk);
        check("cgroup","C group",uk);
        check("TMPFILES","T M P files",uk);
        check("gethostbyaddr","get host by addr",uk);
        check("swapfile","swap file",uk);
        check("fileswap","file swap",uk);
        check("filetable","file table",uk);
        check("swaphost","swap host",uk);
        check("filecandle","file candle",uk);
        check("framebuffer","frame buffer",uk);
        check("keyframe","key frame",uk);
        check("redhead","red head",uk);
        check("Redhead","red head",uk);
        check("REDHEAD","red head",uk);
        check("redheaded","red headed",uk);
        check("redheads","red heads",uk);
        check("newspaper","news paper",uk);
        check("candlelight","candle light",uk);
        check("homophone",uk?NULL:"homo phone",uk);
        check("cupboard",NULL,uk);
        check("Cupboard",NULL,uk);
        check("CUPBOARD",NULL,uk);
        check("cupboards",NULL,uk);
        check("breakfast",NULL,uk);
        check("breakfasts",NULL,uk);
        check("raspberry",NULL,uk);
        check("raspberries",NULL,uk);
        check("strawberry",NULL,uk);
        check("waistcoat",NULL,uk);
        check("butterfly",NULL,uk);
        check("lighthouse",NULL,uk);
        check("switchable",NULL,uk);
        check("Switchable",NULL,uk);
        check("switchably",NULL,uk);
        check("switchability",NULL,uk);
        check("switchless",NULL,uk);
        check("spellless",NULL,uk);
        check("switchlessness",NULL,uk);
        check("happiness",NULL,uk);
        check("paintable",NULL,uk);
        check("deletable",NULL,uk);
        check("rollable",NULL,uk);
        check("viewable",NULL,uk);
        check("searchable",NULL,uk);
        check("jumpable",NULL,uk);
        check("parkable",NULL,uk);
        check("recyclable",NULL,uk);
        check("algorithm",NULL,uk);
        check("HELLO",NULL,uk);
        check("inside",NULL,uk);
        const char *whole=NULL,*joined=NULL;
        assert(reloq_wordseg_reference((const unsigned char *)"Redhead",7,uk,&whole,&joined));
        assert(!strcmp(whole,"`[.2ri.1did]")&&!strcmp(joined,"`[.1rEd.2hEd]"));
        assert(!reloq_wordseg_reference((const unsigned char *)"cupboard",8,uk,&whole,&joined));
        assert(!reloq_wordseg_reference((const unsigned char *)"breakfast",9,uk,&whole,&joined));
        assert(reloq_wordseg_reference((const unsigned char *)"redheaded",9,uk,&whole,&joined));
        assert(!strcmp(whole,uk?"`[.2ri.1di.0dXd]":"`[.2ri.1di.0FXd]"));
        assert(!strcmp(joined,uk?"`[.1rEd.2hE.0dXd]":"`[.1rEd.2hE.0FXd]"));
        unsigned char small[8];
        assert(reloq_wordseg_english((const unsigned char *)"redhead",7,uk,small,sizeof small)==-1);
    }
    assert((reloq_wordseg_category('a')&RELOQ_WORDSEG_LOWER)!=0);
    assert((reloq_wordseg_category('A')&RELOQ_WORDSEG_UPPER)!=0);
    assert((reloq_wordseg_category(0x3bb)&RELOQ_WORDSEG_LOWER)!=0);
    assert((reloq_wordseg_category(0x39b)&RELOQ_WORDSEG_UPPER)!=0);
    assert((reloq_wordseg_category(0x308)&RELOQ_WORDSEG_MARK)!=0);
    assert(!(reloq_wordseg_category(' ')&RELOQ_WORDSEG_WORD));
    unsigned char folded[12];
    assert(reloq_wordseg_casefold(0xdf,folded)==2&&!memcmp(folded,"ss",2));
    assert(reloq_wordseg_casefold(0x39b,folded)==2&&!memcmp(folded,"\xce\xbb",2));
    puts("Native segmentation: English recognition and Unicode case passed");
    return 0;
}
