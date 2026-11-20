#include "reloq_persian_transition.h"
#include <math.h>
#include <string.h>

int reloq_persian_match_stop_nasal(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary)
{
    if(!frames || !reference || frames==reference || count>120000 ||
       reference_count>120000 || !boundary || boundary>=end || end>count ||
       reference_boundary>=reference_count)return -1;
    uintptr_t frame_address=(uintptr_t)frames,ref_address=(uintptr_t)reference;
    size_t frame_bytes=count*sizeof *frames,ref_bytes=reference_count*sizeof *reference;
    if(frame_address>UINTPTR_MAX-frame_bytes || ref_address>UINTPTR_MAX-ref_bytes ||
       (frame_address<ref_address+ref_bytes && ref_address<frame_address+frame_bytes))return -1;
    /* Only replace a voiced, aspirated inherited vowel: a clean native nasal
       (and a release burst or H impulse) must pass through unchanged. */
    size_t first=boundary;
    double initial=0;
    while(first<end && initial<4 && frames[first].controls[2]==0 &&
          frames[first].controls[7]==0 && frames[first].controls[8]==0) {
        double step=frames[first].controls[0];
        if(!isfinite(step) || step<=0 || step>20)return -1;
        initial+=step;++first;
    }
    if(first==end || initial>=4 || frames[first].controls[2]<=0 ||
       frames[first].controls[7]<=0 || frames[first].controls[8]!=0)return 0;
    size_t finish=first;
    double width=initial;
    while(finish<end && width<50) {
        const float *c=frames[finish].controls;
        for(unsigned column=1;column<=15;++column)
            if(!isfinite(c[column]))return -1;
        if(c[2]<=0 || c[8]!=0)return 0;
        if(c[7]==0)break;
        double step=c[0];
        if(!isfinite(step) || step<=0 || step>20)return -1;
        width+=step;++finish;
    }
    if(finish==end || width<3 || width>=50)return 0;
    /* Verify the donor is genuinely a nasal, not H, frication or a release.
       Permit the source index to precede voicing by a few silent frames. */
    size_t ref=reference_boundary;
    double elapsed=0,ref_time=0,first_voiced=-1;
    for(size_t i=boundary;i<finish;++i) {
        while(ref<reference_count && ref_time+reference[ref].controls[0]<=elapsed+1e-6) {
            double step=reference[ref].controls[0];
            if(!isfinite(step) || step<=0 || step>20)return -1;
            ref_time+=step;++ref;
        }
        if(ref>=reference_count)return 0;
        const float *c=reference[ref].controls;
        for(unsigned column=1;column<=15;++column)
            if(!isfinite(c[column]) || !isfinite(frames[i].controls[column]))return -1;
        if(c[7]!=0 || c[8]!=0)return 0;
        if(c[2]>0 && (frames[i].controls[1]<=0 || frames[i].controls[3]<0 ||
                      frames[i].controls[3]>100))return 0;
        if(c[2]>0 && first_voiced<0)first_voiced=elapsed;
        if(c[2]<=0 && (first_voiced>=0 || elapsed>=4))return 0;
        double step=frames[i].controls[0];
        if(!isfinite(step) || step<=0 || step>20)return -1;
        elapsed+=step;
    }
    if(first_voiced<0 || first_voiced>=4)return 0;
    ref=reference_boundary;elapsed=ref_time=0;
    static const unsigned frequencies[]={9,13,15};
    for(size_t i=boundary;i<finish;++i) {
        while(ref+1<reference_count && ref_time+reference[ref].controls[0]<=elapsed+1e-6) {
            ref_time+=reference[ref].controls[0];++ref;
        }
        const float *donor=reference[ref].controls;
        float *target=frames[i].controls;
        double ratio=elapsed/width;
        double weight=ratio*ratio*(3-2*ratio);
        target[2]=(float)round(donor[2]*(1-weight)+target[2]*weight);
        target[7]=0;
        for(size_t j=0;j<sizeof frequencies/sizeof frequencies[0];++j) {
            unsigned column=frequencies[j];
            target[column]=(float)(donor[column]*(1-weight)+target[column]*weight);
        }
        elapsed+=target[0];
    }
    return 1;
}

int reloq_persian_bridge_stop_onsets(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed)
{
    if(!input || !output || input==output || !times || !ends || !skip || !changed ||
       !frame_count || frame_count>120000 || segment_count<2 || segment_count>32 ||
       !isfinite(window_ms) || window_ms<4 || window_ms>20 ||
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
    int copied=0;
    for(size_t edge=1;edge<segment_count;++edge) {
        if(skip[edge])continue;
        size_t boundary=ends[edge-1],right=ends[edge];
        size_t left=edge>1?ends[edge-2]:0,quiet=boundary,onset=boundary;
        /* Native source indexes can land immediately before the first
         * voiced frame rather than on it. Accept a few silent milliseconds,
         * but never cross a release burst or an aspirated H onset. */
        while(onset<right && times[onset]-times[boundary]<5 &&
              input[onset].controls[2]==0 && input[onset].controls[7]==0 &&
              input[onset].controls[8]==0)++onset;
        if(onset==right || times[onset]-times[boundary]>=5)continue;
        const float *first=input[onset].controls;
        if(first[2]<20 || first[8]!=0)continue;
        while(quiet>left) {
            const float *c=input[quiet-1].controls;
            if(c[2]!=0 || c[7]!=0 || c[8]!=0)break;
            --quiet;
        }
        double closure=times[onset]-times[quiet];
        if(closure<3 || closure>100)continue;
        size_t end=onset;
        while(end<right && times[end]-times[onset]<window_ms &&
              input[end].controls[2]>0 && input[end].controls[8]==0)++end;
        if(end-onset<3 || times[end]-times[onset]<4)continue;
        if(!copied){memcpy(output,input,frame_count*sizeof *input);copied=1;}
        for(size_t i=onset;i<end;++i) {
            double x=(times[i]-times[onset])/window_ms;
            double smooth=x*x*(3-2*x),drop=24*(1-smooth);
            for(unsigned column=2;column<=7;column+=5) {
                double original=input[i].controls[column];
                if(original>0)output[i].controls[column]=(float)fmax(1.0,round(original-drop));
            }
        }
        changed[edge]=1;
    }
    return 0;
}

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
