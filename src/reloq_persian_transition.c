#include "reloq_persian_transition.h"
#include <math.h>
#include <string.h>

struct unvoiced_lead {
    size_t end;
    double duration;
    double closure;
    float initial_noise,onset_noise,release;
};

/* Identify the actual source transition, never relying on a spelling alone. */
static int find_unvoiced_lead(const struct reloq_fa_frame *frames,size_t first,
    size_t end,unsigned char phone,struct unvoiced_lead *lead)
{
    double elapsed=0;
    float peak=0;
    for(size_t i=first;i<end;++i) {
        const float *c=frames[i].controls;
        if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
           !isfinite(c[7]) || !isfinite(c[8]))return -1;
        if(c[2]<=0) {
            int expected=(phone=='f' || phone=='s' || phone=='S') ? c[8]>=20 :
                phone=='h' ? c[7]>0 : c[7]==0 && c[8]==0;
            if(!expected || elapsed<8 || elapsed>120)return 0;
            lead->end=i;lead->duration=elapsed;lead->initial_noise=peak;
            lead->onset_noise=c[8];lead->release=-1;lead->closure=0;
            if(phone=='t' || phone=='p' || phone=='k') {
                double closure=0;
                for(size_t j=i;j<end && closure<=110;++j) {
                    const float *next=frames[j].controls;
                    if(!isfinite(next[0]) || next[0]<=0 || next[0]>20 ||
                       !isfinite(next[2]) || !isfinite(next[7]) || !isfinite(next[8]))return -1;
                    if(next[8]>=20 && next[2]<=0 && next[7]==0){lead->release=next[8];break;}
                    if(next[2]>0 || next[7]>0)break;
                    closure+=next[0];
                }
                lead->closure=closure;
            }
            return 1;
        }
        if(c[8]>peak)peak=c[8];
        elapsed+=c[0];
        if(elapsed>120)return 0;
    }
    return 0;
}

static int same_unvoiced_context(const struct reloq_fa_frame *reference,
    size_t first,size_t end,unsigned char phone,const struct unvoiced_lead *target)
{
    struct unvoiced_lead donor;
    int found=find_unvoiced_lead(reference,first,end,phone,&donor);
    /* A neutral proxy is not evidence that the requested consonant has no
     * lead-in. Distinguish that from a real but incompatible onset. */
    if(found<0)return -1;
    if(!found) {
        const float *first_frame=reference[first].controls;
        int direct=(phone=='f' || phone=='s' || phone=='S') ? first_frame[8]>=20 :
            phone=='h' ? first_frame[7]>0 : first_frame[7]==0 && first_frame[8]==0;
        /* A genuine unvoiced onset at the marked edge is a conflicting
         * reference, not the neutral proxy's missing consonant. */
        return first_frame[2]<=0 && direct ? 0 : 2;
    }
    double tolerance=fmax(6.0,target->duration*0.4);
    if(fabs(target->duration-donor.duration)>tolerance)return 0;
    if(phone=='t' || phone=='p' || phone=='k')
        return target->release>=20 && donor.release>=20 &&
               fabs(target->release-donor.release)<=20;
    if(phone=='f' || phone=='s' || phone=='S')
        return target->initial_noise>=10 && donor.initial_noise>=10 &&
               fabs(target->onset_noise-donor.onset_noise)<=20;
    return 1;
}

/* A neutral-vowel proxy may omit the consonant entirely. In that case keep
 * a short, continuous native-shaped lead only when the voice and aspiration
 * begin at the preceding values and a genuine stop burst or a smoothly
 * arriving fricative is present. H retains its separate established path. */
static int native_shaped_lead(const struct reloq_fa_frame *frames,size_t boundary,
    unsigned char phone,const struct unvoiced_lead *lead)
{
    if(boundary<2 || lead->duration>45 || phone=='h')return 0;
    const float *left=frames[boundary-1].controls,*first=frames[boundary].controls;
    if(frames[boundary-2].controls[2]<=0 || left[2]<20 || first[2]<20 || left[8]!=0 ||
       first[8]>5 || fabs((double)left[2]-first[2])>8 ||
       fabs((double)left[7]-first[7])>8)return 0;
    if(phone=='t' || phone=='p' || phone=='k')
        return lead->closure>=3 && lead->closure<=100 && lead->release>=20;
    return lead->initial_noise>=10 && lead->onset_noise>=20 &&
           fabs((double)lead->initial_noise-lead->onset_noise)<=15;
}

int reloq_persian_plan_unvoiced_onsets(struct reloq_fa_frame *frames,size_t count,
    const size_t *ends,size_t segments,const unsigned char *onsets,
    const struct reloq_fa_frame *reference,size_t reference_count,
    const size_t *reference_edges,size_t *counts,double *removed,size_t *new_count)
{
    if(!frames || !ends || !onsets || !counts || !removed || !new_count ||
       !count || count>120000 || segments<2 || segments>32 ||
       reference_count>120000 || (!!reference != !!reference_edges))return -1;
    size_t previous=0;
    for(size_t i=0;i<segments;++i) {
        if(ends[i]<=previous || ends[i]>count)return -1;
        unsigned char phone=onsets[i]&127;
        if(phone && !strchr("fstpkSh",phone))return -1;
        previous=ends[i];
    }
    if(previous!=count)return -1;
    if(reference) {
        for(size_t i=0;i<=segments;++i)
            if(reference_edges[i]>reference_count ||
               (i && reference_edges[i]<reference_edges[i-1]))return -1;
    }
    size_t cut_first[32]={0},cut_last[32]={0};
    double widths[32]={0};
    for(size_t edge=1;edge<segments;++edge) {
        unsigned char phone=onsets[edge]&127;
        if(!phone)continue;
        size_t boundary=ends[edge-1],end=ends[edge];
        if(frames[boundary-1].controls[2]<=0)continue;
        struct unvoiced_lead target;
        int found=find_unvoiced_lead(frames,boundary,end,phone,&target);
        if(found<0)return -1;
        if(!found)continue;
        if((onsets[edge]&128) && reference && reference_edges[edge]<reference_edges[edge+1]) {
            int same=same_unvoiced_context(reference,reference_edges[edge],
                reference_edges[edge+1],phone,&target);
            if(same<0)return -1;
            if(same==1 || (same==2 && native_shaped_lead(frames,boundary,phone,&target)))continue;
        }
        cut_first[edge]=boundary;cut_last[edge]=target.end;widths[edge]=target.duration;
    }
    size_t write=0,read=0;
    for(size_t edge=1;edge<segments;++edge) {
        if(!cut_last[edge])continue;
        size_t n=cut_first[edge]-read;
        if(n && write!=read)memmove(frames+write,frames+read,n*sizeof *frames);
        write+=n;read=cut_last[edge];
    }
    size_t tail=count-read;
    if(tail && write!=read)memmove(frames+write,frames+read,tail*sizeof *frames);
    write+=tail;previous=0;
    for(size_t i=0;i<segments;++i) {
        counts[i]=ends[i]-previous-(cut_last[i]-cut_first[i]);
        removed[i]=widths[i];previous=ends[i];
    }
    *new_count=write;
    return 0;
}

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

static double ease(double x){return x*x*(3-2*x);}

static size_t lower_time(const double *times,size_t count,double value,int right)
{
    size_t a=0,b=count;
    while(a<b) {
        size_t mid=a+(b-a)/2;
        if(times[mid]<value || (right && times[mid]==value))a=mid+1;
        else b=mid;
    }
    return a;
}

int reloq_persian_bridge_remaining(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed)
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
    int copied=0;
    static const unsigned columns[]={9,13,15};
    for(size_t edge=1;edge<segment_count;++edge) {
        if(skip[edge])continue;
        size_t boundary=ends[edge-1],left_limit=edge>1?ends[edge-2]:0,right_limit=ends[edge];
        const float *left=input[boundary-1].controls,*right=input[boundary].controls;
        if(left[2]<=0 || right[2]<=0)continue;
        size_t short_end=boundary;
        while(short_end<right_limit && input[short_end].controls[2]>0 &&
              times[short_end]-times[boundary]<5)++short_end;
        if(short_end>boundary && short_end<right_limit && input[short_end].controls[2]==0 &&
           times[short_end]-times[boundary]<=5 && left[2]<=20 && left[7]>0 &&
           input[short_end].controls[7]>0) {
            if(!copied){memcpy(output,input,frame_count*sizeof *input);copied=1;}
            const float *destination=input[short_end].controls;
            size_t width=short_end-boundary;
            for(size_t i=boundary;i<short_end;++i) {
                double f=(double)(i-boundary+1)/(double)(width+1);
                output[i].controls[2]=(float)nearbyint(left[2]*(1-f));
                output[i].controls[7]=(float)nearbyint(left[7]+((double)destination[7]-left[7])*f);
                for(size_t c=0;c<3;++c) {
                    unsigned column=columns[c];
                    output[i].controls[column]=(float)(left[column]+((double)destination[column]-left[column])*f);
                }
            }
            changed[edge]=1;continue;
        }
        double amp=(double)right[2]-left[2],asp=(double)right[7]-left[7],maximum=0;
        for(size_t c=0;c<3;++c) {
            double difference=fabs((double)right[columns[c]]-left[columns[c]]);
            if(difference>maximum)maximum=difference;
        }
        if(fabs(amp)<12 && fabs(asp)<12 && maximum<100)continue;
        size_t first=lower_time(times,frame_count+1,times[boundary]-window_ms,0);
        size_t last=lower_time(times,frame_count+1,times[boundary]+window_ms,1);
        if(first<left_limit)first=left_limit;
        if(last>right_limit)last=right_limit;
        for(size_t i=first;i<boundary;++i)if(input[i].controls[2]<=0)first=i+1;
        for(size_t i=boundary;i<last;++i)if(input[i].controls[2]<=0){last=i;break;}
        if(boundary-first<3 || last-boundary<3 || times[last-1]<=times[first])continue;
        double span=times[last-1]-times[first];
        if(!copied){memcpy(output,input,frame_count*sizeof *input);copied=1;}
        for(size_t c=0;c<3;++c) {
            unsigned column=columns[c];
            double a=input[first].controls[column],b=input[last-1].controls[column];
            for(size_t i=first;i<last;++i) {
                double weight=ease((times[i]-times[first])/span);
                output[i].controls[column]=(float)(a+(b-a)*weight);
            }
        }
        if(amp>=12 && times[boundary-1]>times[first]) {
            double duration=times[boundary-1]-times[first];
            for(size_t i=first;i<boundary;++i) {
                double weight=ease((times[i]-times[first])/duration);
                output[i].controls[2]=(float)fmin(80,nearbyint(input[i].controls[2]+amp*weight));
            }
        } else if(amp<=-12 && times[last-1]>times[boundary]) {
            double duration=times[last-1]-times[boundary];
            for(size_t i=boundary;i<last;++i) {
                double weight=1-ease((times[i]-times[boundary])/duration);
                output[i].controls[2]=(float)fmin(80,nearbyint(input[i].controls[2]-amp*weight));
            }
        }
        if(left[7]>0 && right[7]==0 && times[boundary-1]>times[first]) {
            double duration=times[boundary-1]-times[first];
            for(size_t i=first;i<boundary;++i) {
                double weight=ease((times[i]-times[first])/duration);
                output[i].controls[7]=(float)nearbyint(input[i].controls[7]*(1-weight));
            }
        } else if(left[7]==0 && right[7]>0 && times[last-1]>times[boundary]) {
            double duration=times[last-1]-times[boundary];
            for(size_t i=boundary;i<last;++i) {
                double weight=ease((times[i]-times[boundary])/duration);
                output[i].controls[7]=(float)nearbyint(input[i].controls[7]*weight);
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
