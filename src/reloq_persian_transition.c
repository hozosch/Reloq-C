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
    size_t end,unsigned char phone,double minimum,struct unvoiced_lead *lead)
{
    double elapsed=0;
    float peak=0;
    for(size_t i=first;i<end;++i) {
        const float *c=frames[i].controls;
        if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
           !isfinite(c[7]) || !isfinite(c[8]))return -1;
        if(c[2]<=0) {
            int expected=(phone=='f' || phone=='s' || phone=='S' || phone=='G') ? c[8]>=20 :
                phone=='h' ? c[7]>0 : c[7]==0 && c[8]==0;
            if(!expected || elapsed<minimum || elapsed>120)return 0;
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
    int found=find_unvoiced_lead(reference,first,end,phone,8,&donor);
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

/* Compare the actual consonant's noise bands, not just a voiced/unvoiced
 * label: the first US frame can still belong to the preceding consonant. */
static int same_onset_source(const float *a,const float *b,unsigned char phone)
{
    if(a[2]>0 || b[2]>0)return 0;
    if(phone=='p' || phone=='t' || phone=='k')
        return a[7]==0 && b[7]==0 && a[8]==0 && b[8]==0;
    if(a[7]!=0 || b[7]!=0 || a[8]<20 || b[8]<20 || fabs(a[8]-b[8])>12)return 0;
    for(unsigned j=35;j<=42;++j)if(fabs(a[j]-b[j])>12)return 0;
    return 1;
}

int reloq_persian_match_us_unvoiced_onset(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary,size_t reference_end,unsigned char phone)
{
    if(!frames || !reference || !boundary || boundary>=end || end>count ||
       reference_boundary>=reference_end || reference_end>reference_count ||
       count>120000 || reference_count>120000 || !phone || !strchr("fstpkS",phone) ||
       count>SIZE_MAX/sizeof *frames || reference_count>SIZE_MAX/sizeof *reference)return -1;
    uintptr_t a=(uintptr_t)frames,b=(uintptr_t)reference;
    size_t an=count*sizeof *frames,bn=reference_count*sizeof *reference;
    if(a>UINTPTR_MAX-an || b>UINTPTR_MAX-bn || (a<b+bn && b<a+an))return -1;
    if(frames[boundary].controls[2]<=0)return 0;
    struct unvoiced_lead target;
    int found=find_unvoiced_lead(frames,boundary,end,phone,.5,&target);
    if(found<0)return -1;
    if(!found || target.duration>100)return 0;
    /* For a stop, confirm that both contexts actually contain its release;
     * a coincidental quiet interval must never be mistaken for a closure. */
    if(phone=='p' || phone=='t' || phone=='k') {
        struct unvoiced_lead donor;
        int matched=find_unvoiced_lead(reference,reference_boundary,reference_end,phone,0,&donor);
        if(matched<0)return -1;
        if(!matched || target.release<20 || donor.release<20 ||
           fabs(target.release-donor.release)>20)return 0;
    }
    size_t stop=boundary,ref=reference_boundary;double elapsed=0,ref_time=0;
    int changed=0;
    while(stop<end && elapsed<=120) {
        while(ref<reference_end) {
            double step=reference[ref].controls[0];
            if(!isfinite(step) || step<=0 || step>20)return -1;
            if(ref_time+step>elapsed+1e-6)break;
            ref_time+=step;++ref;
        }
        if(ref>=reference_end)return 0;
        const float *t=frames[stop].controls,*r=reference[ref].controls;
        if(!isfinite(t[0]) || t[0]<=0 || t[0]>20 || !isfinite(r[0]) || r[0]<=0 || r[0]>20 ||
           !isfinite(t[2]) || !isfinite(t[7]) || !isfinite(t[8]) ||
           !isfinite(r[2]) || !isfinite(r[7]) || !isfinite(r[8]))return -1;
        for(unsigned j=35;j<=42;++j)if(!isfinite(t[j]) || !isfinite(r[j]))return -1;
        if(elapsed>=target.duration && same_onset_source(t,r,phone))break;
        if(t[2]!=r[2] || t[7]!=r[7] || t[8]!=r[8])changed=1;
        for(unsigned j=35;j<=42;++j)if(t[j]!=r[j])changed=1;
        elapsed+=t[0];++stop;
    }
    if(!changed || stop==boundary || stop>=end || ref>=reference_end || elapsed>120)return 0;
    ref=reference_boundary;elapsed=ref_time=0;
    for(size_t i=boundary;i<stop;++i) {
        while(ref+1<reference_end && ref_time+reference[ref].controls[0]<=elapsed+1e-6) {
            ref_time+=reference[ref].controls[0];++ref;
        }
        frames[i].controls[2]=reference[ref].controls[2];
        frames[i].controls[7]=reference[ref].controls[7];
        frames[i].controls[8]=reference[ref].controls[8];
        for(unsigned j=35;j<=42;++j)frames[i].controls[j]=reference[ref].controls[j];
        elapsed+=frames[i].controls[0];
    }
    return 1;
}

/* A target context can inherit an English vowel ahead of the actual voiced
 * stop, even though the English side already ended in silence or a release.
 * Recognize both the low-voice closure and its burst before removing only
 * that inherited vowel; the stop's own voicing and release stay intact. */
static int find_voiced_stop_lead(const struct reloq_fa_frame *frames,
    size_t first,size_t end,struct unvoiced_lead *lead)
{
    double elapsed=0;
    for(size_t i=first;i<end;++i) {
        const float *c=frames[i].controls;
        if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
           !isfinite(c[7]) || !isfinite(c[8]))return -1;
        if(c[2]<=30 && c[7]<=1 && c[8]<=1) {
            if(elapsed<0.5 || elapsed>100)return 0;
            double closure=0;
            for(size_t j=i;j<end && closure<=110;++j) {
                const float *next=frames[j].controls;
                if(!isfinite(next[0]) || next[0]<=0 || next[0]>20 ||
                   !isfinite(next[2]) || !isfinite(next[7]) || !isfinite(next[8]))return -1;
                if(next[8]>=20 && next[2]<=5 && next[7]<=1) {
                    if(closure<2)return 0;
                    lead->end=i;lead->duration=elapsed;return 1;
                }
                if(next[2]>30 || next[7]>1 || next[8]>1)return 0;
                closure+=next[0];
            }
            return 0;
        }
        if(c[2]<30 || c[7]<5 || c[8]>=20)return 0;
        elapsed+=c[0];
        if(elapsed>100)return 0;
    }
    return 0;
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
        if(phone && !strchr("fstpkShGbdg",phone))return -1;
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
        int voiced_before=frames[boundary-1].controls[2]>0;
        /* A silent or noisy English stop cannot legitimately continue as a
         * voiced vowel before a voiceless Persian onset. The target context
         * can nevertheless contain such a stale vowel at the join. */
        if(!voiced_before && !(onsets[edge]&128))continue;
        if(phone=='G' && voiced_before)continue; /* keep the Ach path */
        struct unvoiced_lead target;
        if(strchr("bdg",phone)) {
            if(voiced_before || !(onsets[edge]&128))continue;
            int found=find_voiced_stop_lead(frames,boundary,end,&target);
            if(found<0)return -1;
            if(found){cut_first[edge]=boundary;cut_last[edge]=target.end;widths[edge]=target.duration;}
            continue;
        }
        int found=find_unvoiced_lead(frames,boundary,end,phone,voiced_before?8:0.5,&target);
        if(found<0)return -1;
        if(!found)continue;
        if(voiced_before && (onsets[edge]&128) && reference &&
           reference_edges[edge]<reference_edges[edge+1]) {
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

/* An English context can leave part of its vowel at the beginning of the
 * next Persian segment even after a final stop burst. Only remove a lead
 * ending at the independently measured Persian syllable start, and require
 * the expected low-voice R onset immediately after it. Every R frame and
 * its aspiration and noise are retained. A few inherited voiced frames
 * inside the syllable's onset are lowered to the actual R onset level. */
int reloq_persian_plan_stop_r_onsets(struct reloq_fa_frame *frames,size_t count,
    const size_t *ends,size_t segments,const size_t *starts,
    size_t *counts,double *removed,size_t *new_count)
{
    if(!frames || !ends || !starts || !counts || !removed || !new_count ||
       !count || count>120000 || segments<2 || segments>32 || starts[0])return -1;
    size_t previous=0;
    for(size_t i=0;i<segments;++i) {
        if(ends[i]<=previous || ends[i]>count ||
           (starts[i] && (starts[i]<=previous || starts[i]>=ends[i])))return -1;
        previous=ends[i];
    }
    if(previous!=count)return -1;
    size_t cut_first[32]={0},cut_last[32]={0},low_onset[32]={0};
    double widths[32]={0};
    for(size_t edge=1;edge<segments;++edge) {
        if(!starts[edge])continue;
        size_t boundary=ends[edge-1],onset=starts[edge],end=ends[edge];
        const float *left=frames[boundary-1].controls;
        if(!isfinite(left[2]))return -1;
        if(left[2]>0) {
            /* A voiced English /d,b,g/ can end in a voiced closure rather
             * than a silent or noisy burst. The caller has already verified
             * the final English stop phone; additionally require a sustained
             * closed, low-F1 tract with no aspiration or frication. Never
             * alter those English frames. */
            size_t first=edge>=2?ends[edge-2]:0;
            size_t i=boundary;double closure=0;
            while(i>first && closure<4) {
                const float *c=frames[--i].controls;
                if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
                   !isfinite(c[7]) || !isfinite(c[8]) || !isfinite(c[9]))return -1;
                if(c[2]>50 || c[7]>1 || c[8]>1 || c[9]>300)break;
                closure+=c[0];
            }
            if(closure<4)continue;
        }
        double duration=0;int valid=1;
        for(size_t i=boundary;i<onset;++i) {
            const float *c=frames[i].controls;
            if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
               !isfinite(c[7]) || !isfinite(c[8]))return -1;
            if(c[2]<30 || c[8]>0){valid=0;break;}
            duration+=c[0];
        }
        if(!valid || duration<3 || duration>100)continue;
        double elapsed=0;size_t low_r=SIZE_MAX;int voiced_r=0;
        for(size_t i=onset;i<end && elapsed<=60;++i) {
            const float *c=frames[i].controls;
            if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
               !isfinite(c[7]) || !isfinite(c[8]))return -1;
            if(low_r==SIZE_MAX && elapsed<=8 && c[2]<=15 && c[7]>=12 && c[8]==0)low_r=i;
            if(low_r!=SIZE_MAX && c[2]>=20 && c[7]>=20 && c[8]==0){voiced_r=1;break;}
            elapsed+=c[0];
        }
        if(low_r!=SIZE_MAX && voiced_r) {
            cut_first[edge]=boundary;cut_last[edge]=onset;low_onset[edge]=low_r;widths[edge]=duration;
        }
    }
    for(size_t edge=1;edge<segments;++edge)if(cut_last[edge]) {
        float limit=fmaxf(1.0f,frames[low_onset[edge]].controls[2]);
        for(size_t i=cut_last[edge];i<low_onset[edge];++i)
            if(frames[i].controls[2]>limit)frames[i].controls[2]=limit;
    }
    size_t write=0,read=0;
    for(size_t edge=1;edge<segments;++edge)if(cut_last[edge]) {
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
    *new_count=write;return 0;
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
    float inherited_aspiration=frames[first].controls[7];
    float inherited_voicing=frames[first].controls[2];
    int held_source=1;
    while(finish<end && width<50) {
        const float *c=frames[finish].controls;
        for(unsigned column=1;column<=15;++column)
            if(!isfinite(c[column]))return -1;
        if(c[2]<=0 || c[8]!=0)return 0;
        /* Finnish /m,n/ can carry a nonzero aspiration of their own. After
         * a held English source, its rising new target also marks the end
         * of the inherited vowel. Do not treat a delayed or changing native
         * nasal as that held source. */
        if(c[7]==0 || (initial==0 && held_source && width>=3 &&
                       c[7]>=inherited_aspiration+2.5f))break;
        if(fabs((double)c[2]-inherited_voicing)>0.5 ||
           fabs((double)c[7]-inherited_aspiration)>0.5)held_source=0;
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

/* Find a voiced sibilant's rising part, without crossing into a later phone. */
static int z_onset(const struct reloq_fa_frame *frames,size_t start,size_t end,
    double limit_ms,size_t *peak_index,double *peak_time)
{
    double elapsed=0;float peak=0;size_t selected=start;int seen=0;
    for(size_t i=start;i<end && elapsed<limit_ms;++i) {
        const float *c=frames[i].controls;
        if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
           !isfinite(c[7]) || !isfinite(c[8]))return -1;
        for(unsigned j=35;j<=42;++j)if(!isfinite(c[j]))return -1;
        if(c[8]>=20)seen=1;
        if(seen && c[8]<10)break;
        if(c[8]>peak){peak=c[8];selected=i;*peak_time=elapsed;}
        elapsed+=c[0];
    }
    const float *c=frames[selected].controls;
    if(peak<40 || c[2]<=0 ||
       !(c[38]>c[37]+10 || c[39]>c[37]+10))return 0;
    *peak_index=selected;return 1;
}

int reloq_persian_match_us_z_onset(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary)
{
    if(!frames || !reference || !boundary || boundary>=end || end>count ||
       count>120000 || reference_count>120000 || reference_boundary>=reference_count ||
       count>SIZE_MAX/sizeof *frames || reference_count>SIZE_MAX/sizeof *reference)return -1;
    uintptr_t left=(uintptr_t)frames,right=(uintptr_t)reference;
    size_t left_bytes=count*sizeof *frames,right_bytes=reference_count*sizeof *reference;
    if(left>UINTPTR_MAX-left_bytes || right>UINTPTR_MAX-right_bytes ||
       (left<right+right_bytes && right<left+left_bytes))return -1;
    if(frames[boundary].controls[8]>10 || reference[reference_boundary].controls[8]>10)return 0;
    size_t target_peak=0,donor_peak=0;double target_time=0,donor_time=0;
    int a=z_onset(frames,boundary,end,35,&target_peak,&target_time);
    int b=z_onset(reference,reference_boundary,reference_count,35,&donor_peak,&donor_time);
    if(a<0 || b<0)return -1;
    if(!a || !b || donor_time<=target_time+1 || donor_time>30 ||
       fabs(frames[target_peak].controls[8]-reference[donor_peak].controls[8])>8)return 0;
    for(unsigned j=35;j<=42;++j)
        if(fabs(frames[target_peak].controls[j]-reference[donor_peak].controls[j])>12)return 0;
    /* Verify the complete replacement interval before modifying anything. */
    size_t ref=reference_boundary,stop=boundary;double elapsed=0,ref_time=0;
    while(stop<end && elapsed<=donor_time) {
        while(ref<reference_count && ref_time+reference[ref].controls[0]<=elapsed+1e-6) {
            ref_time+=reference[ref].controls[0];++ref;
        }
        if(ref>=reference_count || (stop>target_peak && frames[stop].controls[8]<20))return 0;
        elapsed+=frames[stop].controls[0];++stop;
    }
    if(elapsed<=donor_time)return 0;
    ref=reference_boundary;elapsed=ref_time=0;
    for(size_t i=boundary;i<stop;++i) {
        while(ref+1<reference_count && ref_time+reference[ref].controls[0]<=elapsed+1e-6) {
            ref_time+=reference[ref].controls[0];++ref;
        }
        frames[i].controls[8]=reference[ref].controls[8];
        for(unsigned j=35;j<=42;++j)frames[i].controls[j]=reference[ref].controls[j];
        elapsed+=frames[i].controls[0];
    }
    return 1;
}

/* The same medial-H source also applies before voiceless fricatives and
 * stops. Their onset ends in a fricative plateau or a verified closure and
 * release. Do not change the plateau, closure, release, vowel or pauses. */
static int h_to_unvoiced_onset(struct reloq_fa_frame *frames,size_t count,
                               size_t boundary,size_t end)
{
    size_t first=boundary;double gap=0;float h=0;
    while(first>0) {
        const float *c=frames[first-1].controls;
        if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
           !isfinite(c[7]) || !isfinite(c[8]))return -1;
        if(c[2]<=5 && c[7]>=20 && c[8]<=5){h=c[7];break;}
        if(c[2]>0 || c[7]>0 || c[8]>0 || (gap+=c[0])>5)return 0;
        --first;
    }
    if(!h || frames[boundary].controls[2]<20 || frames[boundary].controls[8]>5)return 0;
    size_t stop=boundary;double elapsed=0;float last_noise=0;int closure=0;
    while(stop<end && elapsed<150) {
        const float *c=frames[stop].controls;
        if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
           !isfinite(c[7]) || !isfinite(c[8]))return -1;
        if(c[2]<=5 && c[8]>=30)break;
        if(c[2]<=5 && c[7]==0 && c[8]==0 && elapsed>=1) {
            /* A real stop has a short closure followed by its own burst.
             * Require both before touching the preceding H transition. */
            double quiet=0;size_t j=stop;
            for(;j<end && quiet<150;++j) {
                const float *n=frames[j].controls;
                if(!isfinite(n[0]) || n[0]<=0 || n[0]>20 || !isfinite(n[2]) ||
                   !isfinite(n[7]) || !isfinite(n[8]))return -1;
                if(n[8]>=20 && n[2]<=5)break;
                if(n[2]>5 || n[7]!=0 || n[8]!=0)return 0;
                quiet+=n[0];
            }
            if(j>=end || quiet<2 || quiet>=150)return 0;
            closure=1;break;
        }
        if(c[2]<20 || c[8]+2<last_noise)return 0;
        last_noise=c[8];elapsed+=c[0];++stop;
    }
    if(stop==boundary || stop>=end || elapsed>=150 || (!closure && last_noise<20))return 0;
    for(size_t i=first;i<boundary;++i)frames[i].controls[7]=h;
    for(size_t i=boundary;i<stop;++i) {
        frames[i].controls[2]=0;
        frames[i].controls[7]=h;
    }
    (void)count;
    return 1;
}

int reloq_persian_match_h_to_us_z(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end,const struct reloq_fa_frame *reference,
    size_t reference_count,size_t reference_boundary)
{
    if(!frames || boundary<3 || boundary>=end || end>count || count>120000 ||
       count>SIZE_MAX/sizeof *frames)return -1;
    if(!reference) {
        if(reference_count || reference_boundary)return -1;
        return h_to_unvoiced_onset(frames,count,boundary,end);
    }
    if(reference_count>120000 || reference_boundary>=reference_count ||
       count>SIZE_MAX/sizeof *frames || reference_count>SIZE_MAX/sizeof *reference)return -1;
    uintptr_t left=(uintptr_t)frames,right=(uintptr_t)reference;
    size_t a=count*sizeof *frames,b=reference_count*sizeof *reference;
    if(left>UINTPTR_MAX-a || right>UINTPTR_MAX-b ||
       (left<right+b && right<left+a))return -1;
    /* The first source frame must continue the actual final H without an
     * inserted voiced vowel. Confirm its aspiration immediately before the
     * boundary and the same unvoiced reference start. */
    if(frames[boundary-1].controls[2]>5 || reference[reference_boundary].controls[2]>5 ||
       frames[boundary].controls[2]<20 || reference[reference_boundary].controls[7]>5)return 0;
    int h=0;float h_level=0;double prior=0,gap=0;size_t gap_start=boundary;
    for(size_t i=boundary;i>0 && prior<20;) {
        const float *c=frames[--i].controls;
        if(!isfinite(c[0]) || c[0]<=0 || c[0]>20 || !isfinite(c[2]) ||
           !isfinite(c[7]) || !isfinite(c[8]))return -1;
        if(c[2]<=5 && c[7]>=20 && c[8]<=5) {
            h=1;h_level=c[7];break;
        }
        /* Only a very short silent carrier can be part of this H. A real
         * pause, another source or a release burst must remain untouched. */
        if(c[2]>0 || c[7]>0 || c[8]>0 || (gap+=c[0])>5)return 0;
        gap_start=i;
        prior+=c[0];
    }
    if(!h)return 0;
    size_t tp=0,rp=0;double tt=0,rt=0;
    /* At slow rates the same US fricative can need more than 35 ms just
     * to reach its normal noise level. Verify the real onset for its whole
     * natural duration, rather than silently skipping those rates. */
    int ta=z_onset(frames,boundary,end,150,&tp,&tt);
    int ra=z_onset(reference,reference_boundary,reference_count,150,&rp,&rt);
    if(ta<0 || ra<0)return -1;
    if(!ta || !ra || fabs(tt-rt)>3 ||
       fabs(frames[tp].controls[8]-reference[rp].controls[8])>5)return 0;
    size_t stop=boundary,ref=reference_boundary;double elapsed=0,ref_time=0;
    /* Verify every sample against the same US Z before making any changes,
     * and stop only where the two existing voicing trajectories converge. */
    while(stop<end && elapsed<=300) {
        while(ref<reference_count && ref_time+reference[ref].controls[0]<=elapsed+1e-6) {
            ref_time+=reference[ref].controls[0];++ref;
        }
        if(ref>=reference_count)return 0;
        const float *t=frames[stop].controls,*r=reference[ref].controls;
        if(!isfinite(t[0]) || t[0]<=0 || t[0]>20 || !isfinite(r[0]) || r[0]<=0 || r[0]>20 ||
           !isfinite(t[2]) || !isfinite(r[2]) || !isfinite(t[7]) || !isfinite(r[7]) ||
           !isfinite(t[8]) || !isfinite(r[8]) || fabs(t[8]-r[8])>5)return 0;
        for(unsigned j=35;j<=42;++j)
            if(!isfinite(t[j]) || !isfinite(r[j]) || fabs(t[j]-r[j])>5)return 0;
        if(elapsed>tt && t[8]>=20 && fabs(t[2]-r[2])<=2 && fabs(t[7]-r[7])<=2)break;
        if(elapsed>tt && t[8]<20)return 0;
        elapsed+=t[0];++stop;
    }
    if(stop==boundary || stop>=end || elapsed>300)return 0;
    /* The actual medial H-to-Z rule keeps H aspiration while Z frication
     * rises, and starts voicing only after that rise. A final-H carrier
     * previously inserted silence and started Z voicing during the rise.
     * Continue the observed H level across the short carrier and use the
     * native Z onset's existing frication and later voice target. The smooth
     * subtraction after the peak rejoins that target without a voice step. */
    for(size_t i=gap_start;i<boundary;++i)frames[i].controls[7]=h_level;
    float peak_voice=reference[rp].controls[2];
    ref=reference_boundary;elapsed=ref_time=0;
    for(size_t i=boundary;i<stop;++i) {
        while(ref+1<reference_count && ref_time+reference[ref].controls[0]<=elapsed+1e-6) {
            ref_time+=reference[ref].controls[0];++ref;
        }
        if(i<=tp) {
            frames[i].controls[2]=0;
            frames[i].controls[7]=h_level;
        } else {
            double x=fmin(1.0,fmax(0.0,(elapsed-tt)/30.0));
            double smooth=x*x*(3-2*x);
            frames[i].controls[2]=(float)floor(fmax(0.0,
                reference[ref].controls[2]-peak_voice*(1-smooth))+.5);
            frames[i].controls[7]=reference[ref].controls[7];
        }
        elapsed+=frames[i].controls[0];
    }
    return 1;
}

int reloq_persian_smooth_final_r_join(struct reloq_fa_frame *frames,size_t count,
    size_t boundary,size_t end)
{
    static const unsigned columns[]={9,13,15,17};
    if(!frames || !boundary || boundary>=end || end>count || count>120000)return -1;
    const float *before=frames[boundary-1].controls,*first=frames[boundary].controls;
    if((before[2]<=0 && before[7]<=0 && before[8]<=0) ||
       (first[2]<=0 && first[7]<=0 && first[8]<=0))return 0;
    unsigned active=0;float anchor[4];
    for(unsigned j=0;j<4;++j) {
        unsigned col=columns[j];
        if(!isfinite(before[col]) || !isfinite(first[col]) || before[col]<=0 || first[col]<=0)return 0;
        anchor[j]=before[col];
        if(fabs(first[col]-before[col])>=150)active|=1u<<j;
    }
    if(!active)return 0;
    /* Check the complete interval before editing it. Silence stops the
     * correction; its length and all source columns remain exactly the same. */
    size_t stop=boundary;double duration=0;
    while(stop<end && duration<10) {
        const float *c=frames[stop].controls;
        if(!isfinite(c[0]) || c[0]<=0 || c[0]>20)return -1;
        if(c[2]<=0 && c[7]<=0 && c[8]<=0)break;
        for(unsigned j=0;j<4;++j)
            if((active&(1u<<j)) && (!isfinite(c[columns[j]]) || c[columns[j]]<=0))return 0;
        duration+=c[0];++stop;
    }
    if(stop-boundary<3 || duration<4)return 0;
    double elapsed=0;
    for(size_t i=boundary;i<stop;++i) {
        float *c=frames[i].controls;
        double x=fmin(1.0,(elapsed+c[0])/duration),weight=x*x*(3-2*x);
        for(unsigned j=0;j<4;++j)if(active&(1u<<j))
            c[columns[j]]=(float)(anchor[j]+(c[columns[j]]-anchor[j])*weight);
        elapsed+=c[0];
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
