/* Direct Persian character names for the private NVDA experiment. */
#include "direct_fa_letters.h"
#include <stddef.h>

struct fa_letter { uint32_t codepoint; const char *tag; };
static const struct fa_letter fa_letters[] = {
#include "direct_fa_letters.inc"
};

const char *reloq_fa_direct_letter(uint32_t codepoint)
{
    size_t lo=0,hi=sizeof fa_letters/sizeof fa_letters[0];
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(fa_letters[mid].codepoint<codepoint)lo=mid+1;
        else hi=mid;
    }
    return lo<sizeof fa_letters/sizeof fa_letters[0] &&
           fa_letters[lo].codepoint==codepoint ? fa_letters[lo].tag : NULL;
}
