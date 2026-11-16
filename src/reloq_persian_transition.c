#include "reloq_persian_transition.h"
#include <math.h>
#include <string.h>

int reloq_persian_bridge_continuation(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output, const double *times, const size_t *ends,
    const unsigned char *skip, size_t frame_count, size_t segment_count,
    double window_ms, unsigned char *changed)
{
    if(!input || !output || input==output || !times || !ends || !skip || !changed ||
       !frame_count || frame_count>120000 || segment_count<2 || segment_count>32 ||
       !isfinite(window_ms) || window_ms<=0 || window_ms>50 ||
       frame_count>SIZE_MAX/sizeof *input)return -1;
    size_t previous=0;
    for(size_t i=0;i<segment_count;++i) {
        if(ends[i]<=previous || ends[i]>frame_count)return -1;
        previous=ends[i];
    }
    if(previous!=frame_count)return -1;
    for(size_t i=0;i<=frame_count;++i)
        if(!isfinite(times[i]) || (i && times[i]<times[i-1]))return -1;
    memset(changed,0,segment_count);
    static const unsigned columns[]={9,13,15};
    int copied=0;
    for(size_t edge=1;edge<segment_count;++edge) {
        if(skip[edge])continue;
        size_t boundary=ends[edge-1],right_limit=ends[edge];
        if(boundary<2 || boundary>=right_limit)continue;
        const float *left=input[boundary-1].controls,*first=input[boundary].controls;
        if(left[2]<=0 || input[boundary-2].controls[2]<=0)continue;
        if(first[2]<=0 && first[7]==0 && first[8]==0)continue;
        double offsets[3],maximum=0;
        for(unsigned c=0;c<3;++c) {
            offsets[c]=(double)left[columns[c]]-(double)first[columns[c]];
            if(fabs(offsets[c])>maximum)maximum=fabs(offsets[c]);
        }
        if(maximum<30)continue;
        size_t last=boundary;
        while(last<right_limit && times[last]-times[boundary]<=window_ms) {
            const float *c=input[last].controls;
            if(c[2]<=0 && c[7]==0 && c[8]==0)break;
            ++last;
        }
        if(last-boundary<3 || times[last-1]-times[boundary]<5)continue;
        double duration=times[last-1]-times[boundary];
        if(!copied){memcpy(output,input,frame_count*sizeof *input);copied=1;}
        for(size_t i=boundary;i<last;++i) {
            double weight=1-(times[i]-times[boundary])/duration;
            for(unsigned c=0;c<3;++c) {
                double value=(double)input[i].controls[columns[c]]+offsets[c]*weight;
                output[i].controls[columns[c]]=(float)(value<0?0:value);
            }
        }
        changed[edge]=1;
    }
    return 0;
}
