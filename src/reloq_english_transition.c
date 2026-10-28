#include "reloq_english_transition.h"
#include <string.h>

static int in(unsigned char c,const char *s){return c&&strchr(s,c)!=NULL;}

int reloq_english_nasal_transition_ticks(unsigned char left,unsigned char right,int boundary)
{
    if(!in(left,"mnG") || (right&&!in(right,"iIeEAXHxauUocRYWObpdtF?kgDTvfzsZSJCmnGrlywNM")))return -1;
    if(!right)return left=='n'?80:left=='m'?50:30;
    if(right=='N')right='n';
    if(right=='M')right='m';
    /* The original first tests the following place, then the preceding
     * nasal place (0x95ad6..0x95b47). Velar-to-alveolar contacts retain
     * 30 ticks, including n, stops and sibilants. */
    if(left=='G'&&in(right,"dtFsznJC"))return 30;
    /* Obstruent closure (0x9584f) and fricative constriction (0x95883).
     * The alveolar fricatives require an alveolar nasal. */
    if(in(right,"bpdtF?kgJC"))return 4;
    if(in(right,"DTvfszZS")) {
        if(in(right,"sz")&&left!='n')return 30;
        return 4;
    }
    if(in(right,"mnG"))return 1;
    /* Lexical-phone refinements follow the broad manner tests. */
    if(left=='m')return right=='w'?1:30;
    if(left=='n') {
        if(boundary)return in(right,"cO")?25:right=='y'?15:30;
        if(in(right,"acYWO"))return 50;
        if(right=='R')return 40;
        if(right=='y')return 15;
    }
    return 30;
}
