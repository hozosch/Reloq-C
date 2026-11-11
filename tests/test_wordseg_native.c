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
        check("framebuffer","frame buffer",uk);
        check("keyframe","key frame",uk);
        check("algorithm",NULL,uk);
        check("HELLO",NULL,uk);
        check("inside",NULL,uk);
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
