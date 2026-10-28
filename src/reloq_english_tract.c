#include "reloq_english_tract.h"
#include <string.h>

int reloq_english_tract_nuclear(unsigned char phone)
{
    /* The fitted multi-syllable prototypes describe nuclei and their
     * liquid/glide continuations. A constriction must retain its own
     * diphone targets: replacing them invents an unrelated nucleus's F1-F3
     * and nasal bandwidths inside stops, fricatives and nasals. */
    return phone && strchr("iIeEAXHxauUocRYWONMlrwy",phone)!=NULL;
}

int reloq_english_tract_targets(int left,int right,int64_t contact,int64_t end,
    struct reloq_english_tract_span out[2],size_t *count)
{
    if(!out||!count||contact<0||end<contact||left< -32768||right< -32768||
       left>32767||right>32767)return -1;
    *count=0;
    /* Original doF, 14c61..14e34: two positive targets in a segment shorter
     * than 16 ticks contract to their truncated arithmetic mean. This
     * threshold applies to the actual segment clock, also at high speed. */
    if(left>0&&right>0&&end-contact<16) {
        left=(left+right)/2;right=-1;
    }
    if(left>0) {
        out[(*count)++]=(struct reloq_english_tract_span){contact,contact,left};
        if(right==-1)
            out[(*count)++]=(struct reloq_english_tract_span){end,end,left};
    }
    if(right>0)
        out[(*count)++]=(struct reloq_english_tract_span){left==-1?contact:end,end,right};
    return 0;
}
