/* Bounded, thread-local capture of native Persian/English frames. */
#include "reloq_persian_frame_internal.h"
#include "reloq_persian_transition.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if defined(RELOQ_FA_PROBE_EXPORT) && defined(_WIN32)
#define FA_EXPORT __declspec(dllexport)
#elif defined(RELOQ_FA_PROBE_EXPORT) && (defined(__GNUC__) || defined(__clang__))
#define FA_EXPORT __attribute__((visibility("default")))
#else
#define FA_EXPORT
#endif

struct capture_state {
    struct reloq_fa_frame *frames;
    int16_t *pcm;
    size_t count, capacity, maximum, samples;
    size_t pcm_capacity;
    int active, failed, controls_only, store_pcm;
};
static _Thread_local struct capture_state capture;

FA_EXPORT int reloq_fa_capture_begin(size_t maximum)
{
    if(capture.active || !maximum || maximum>120000 ||
       maximum>SIZE_MAX/sizeof *capture.frames)return -1;
    free(capture.frames);
    free(capture.pcm);
    capture=(struct capture_state){.maximum=maximum,.active=1};
    return 0;
}

FA_EXPORT const struct reloq_fa_frame *reloq_fa_capture_end(size_t *count, int *failed)
{
    if(!count || !failed || !capture.active)return NULL;
    capture.active=0;
    *count=capture.count;*failed=capture.failed;
    return capture.frames;
}

FA_EXPORT void reloq_fa_capture_release(void)
{
    free(capture.frames);
    free(capture.pcm);
    capture=(struct capture_state){0};
}

FA_EXPORT int reloq_fa_capture_set_controls_only(int enabled)
{
    if(!capture.active || (enabled!=0 && enabled!=1) || (enabled && capture.store_pcm))return -1;
    capture.controls_only=enabled;
    return 0;
}

FA_EXPORT int reloq_fa_capture_set_pcm_storage(int enabled)
{
    if(!capture.active || (enabled!=0 && enabled!=1) || (enabled && capture.controls_only))return -1;
    capture.store_pcm=enabled;
    return 0;
}

int reloq_fa_capture_pcm_storage(void)
{return capture.active && capture.store_pcm;}

int reloq_fa_capture_append_pcm(const int16_t *samples,size_t count)
{
    /* Bound per-request memory independently of the frame count. */
    const size_t limit=(size_t)64*1024*1024;
    if(!capture.active || !capture.store_pcm || (!samples && count) ||
       count>limit-capture.samples) {capture.failed=1;return -1;}
    size_t needed=capture.samples+count;
    if(needed>capture.pcm_capacity) {
        size_t next=capture.pcm_capacity?capture.pcm_capacity:4096;
        while(next<needed)next=next>limit/2?limit:next*2;
        void *data=realloc(capture.pcm,next*sizeof *capture.pcm);
        if(!data){capture.failed=1;return -1;}
        capture.pcm=data;capture.pcm_capacity=next;
    }
    if(count)memcpy(capture.pcm+capture.samples,samples,count*sizeof *samples);
    capture.samples=needed;
    return 0;
}

FA_EXPORT const int16_t *reloq_fa_capture_pcm(size_t *count)
{
    if(!count || capture.active || !capture.store_pcm)return NULL;
    *count=capture.samples;
    return capture.pcm;
}

int reloq_fa_capture_controls_only(void)
{ return capture.active && capture.controls_only; }

int reloq_fa_capture_active(void)
{ return capture.active; }

int reloq_fa_capture_advance_samples(size_t count)
{
    if(!capture.active || !capture.controls_only || count>SIZE_MAX-capture.samples)return -1;
    capture.samples+=count;
    return 0;
}

FA_EXPORT size_t reloq_fa_capture_sample_count(void)
{ return capture.active && (capture.controls_only || capture.store_pcm) ? capture.samples : SIZE_MAX; }

/* Scan the already-rendered pilot in native code. Keep the exact threshold
 * and sample position used by the planner; this does not cache audio. */
FA_EXPORT int64_t reloq_fa_last_audible_sample(const unsigned char *pcm,size_t bytes)
{
    if(!pcm || (bytes & 1) || bytes/2>(size_t)INT64_MAX)return -1;
    for(size_t i=bytes; i>=2; i-=2) {
        unsigned sample=(unsigned)pcm[i-2] | ((unsigned)pcm[i-1]<<8);
        int value=sample>=32768 ? (int)sample-65536 : (int)sample;
        if(value>4 || value< -4)return (int64_t)(i/2);
    }
    return -1;
}

/* These arrays are request-local; extracting them here avoids one Python
 * object per control frame while preserving the original order and clock. */
FA_EXPORT int reloq_fa_frame_ticks(const struct reloq_fa_frame *rows,size_t count,int64_t *ticks)
{
    if(!rows || !count || !ticks)return -1;
    for(size_t i=0;i<count;++i) {
        if(rows[i].tick<0 || (i && rows[i].tick<=rows[i-1].tick))return -1;
        ticks[i]=rows[i].tick;
    }
    return 0;
}

FA_EXPORT int reloq_fa_frame_timeline(const struct reloq_fa_frame *rows,size_t count,double *times)
{
    if((count && !rows) || !times)return -1;
    times[0]=0.0;
    for(size_t i=0;i<count;++i)times[i+1]=times[i]+(double)rows[i].controls[0];
    return 0;
}

FA_EXPORT void reloq_fa_recompute_frication(struct reloq_fa_frame *rows,size_t count)
{
    if(!rows)return;
    for(size_t i=0;i<count;++i) {
        const float *c=rows[i].controls;
        rows[i].s_frication=c[8]>0 && ((double)c[38]>(double)c[37]+10.0 ||
                                       (double)c[39]>(double)c[37]+10.0);
    }
}

/* The same bounded transition calculation is compiled into the common C
 * source tree. This private export lets the NVDA prototype use it too. */
FA_EXPORT int reloq_fa_bridge_continuation(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output,const double *times,const size_t *ends,
    const unsigned char *skip,size_t frame_count,size_t segment_count,
    double window_ms,unsigned char *changed)
{
    return reloq_persian_bridge_continuation(input,output,times,ends,skip,
                                             frame_count,segment_count,window_ms,changed);
}

/* The pilot contour and its voiced anchors are request-local. Perform the
 * same interpolation as the Python planner without a Python callback per
 * millisecond or per frame. All arithmetic between stored floats is double. */
FA_EXPORT int reloq_fa_dense_curve(const struct reloq_fa_frame *rows,size_t count,
    unsigned column,float *out,size_t length)
{
    if(!rows || !count || column>=62 || !out || rows[0].tick<0 ||
       rows[count-1].tick<0 || (uint64_t)rows[count-1].tick+1!=length)return -1;
    size_t cursor=0;
    for(size_t t=0;t<length;++t) {
        while(cursor<count && rows[cursor].tick<=(int64_t)t)++cursor;
        double value;
        if(!cursor)value=rows[0].controls[column];
        else if(cursor>=count)value=rows[count-1].controls[column];
        else {
            size_t a=cursor-1,b=cursor;
            if(rows[b].tick<=rows[a].tick)return -1;
            double x=rows[a].controls[column],y=rows[b].controls[column];
            value=x+(y-x)*(double)((int64_t)t-rows[a].tick)/(double)(rows[b].tick-rows[a].tick);
        }
        out[t]=(float)value;
    }
    return 0;
}

static double fa_interpolated_voice(const struct reloq_fa_frame *rows,size_t count,int64_t tick)
{
    size_t lo=0,hi=count;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(rows[mid].tick<=tick)lo=mid+1;else hi=mid;
    }
    if(!lo)return rows[0].controls[2];
    if(lo>=count)return rows[count-1].controls[2];
    size_t a=lo-1,b=lo;
    double x=rows[a].controls[2],y=rows[b].controls[2];
    return x+(y-x)*(double)(tick-rows[a].tick)/(double)(rows[b].tick-rows[a].tick);
}

struct fa_pitch_point { int64_t tick; double pitch; };

static int fa_pitch_order(const void *left,const void *right)
{
    double a=*(const double *)left,b=*(const double *)right;
    return a<b?-1:a>b?1:0;
}

static double fa_pitch_median(double *values,size_t count)
{
    qsort(values,count,sizeof *values,fa_pitch_order);
    return count&1 ? values[count/2] : (values[count/2-1]+values[count/2])/2.0;
}

static double fa_pitch_interpolate(const struct fa_pitch_point *points,size_t count,double tick)
{
    if(tick<=points[0].tick)return points[0].pitch;
    if(tick>=points[count-1].tick)return points[count-1].pitch;
    size_t lo=0,hi=count-1;
    while(lo+1<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(points[mid].tick<=tick)lo=mid;else hi=mid;
    }
    double part=(tick-points[lo].tick)/(double)(points[hi].tick-points[lo].tick);
    return points[lo].pitch+part*(points[hi].pitch-points[lo].pitch);
}

/* Match the voiced-curve transfer used by the reviewed listening sample.
 * Work only on this request's Persian frames; the Korean donor remains
 * untouched. In mixed text, fade the transfer at actual language edges. */
FA_EXPORT int reloq_fa_transfer_korean_pitch(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_frame *donor,size_t donor_count,unsigned edges,int64_t window)
{
    if(!rows || !donor || !count || !donor_count || count>120000 || donor_count>120000 ||
       edges>3 || window<0)return -1;
    size_t voiced=0,donor_voiced=0,first=0,last=0;
    for(size_t i=0;i<count;++i) {
        if(rows[i].tick<0 || (i && rows[i].tick<=rows[i-1].tick) ||
           !isfinite(rows[i].controls[1]) || !isfinite(rows[i].controls[2]))return -1;
        if(rows[i].controls[2]>10 && rows[i].controls[1]>0) {
            if(!voiced)first=i;
            last=i;++voiced;
        }
    }
    for(size_t i=0;i<donor_count;++i) {
        if(donor[i].tick<0 || (i && donor[i].tick<=donor[i-1].tick) ||
           !isfinite(donor[i].controls[1]) || !isfinite(donor[i].controls[2]))return -1;
        if(donor[i].controls[2]>10 && donor[i].controls[1]>0)++donor_voiced;
    }
    if(voiced<10 || donor_voiced<10 || rows[first].tick>=rows[last].tick)return 1;
    struct fa_pitch_point *points=malloc(donor_voiced*sizeof *points);
    double *original=malloc(voiced*sizeof *original),*mapped=malloc(voiced*sizeof *mapped);
    if(!points || !original || !mapped){free(points);free(original);free(mapped);return -2;}
    size_t at=0;
    for(size_t i=0;i<donor_count;++i)if(donor[i].controls[2]>10 && donor[i].controls[1]>0)
        points[at++]=(struct fa_pitch_point){donor[i].tick,donor[i].controls[1]};
    if(points[0].tick>=points[donor_voiced-1].tick){free(points);free(original);free(mapped);return 1;}
    double span=(double)(rows[last].tick-rows[first].tick);
    double donor_span=(double)(points[donor_voiced-1].tick-points[0].tick);
    at=0;
    for(size_t i=0;i<count;++i)if(rows[i].controls[2]>10 && rows[i].controls[1]>0) {
        double fraction=(double)(rows[i].tick-rows[first].tick)/span;
        original[at]=rows[i].controls[1];
        mapped[at++]=fa_pitch_interpolate(points,donor_voiced,points[0].tick+fraction*donor_span);
    }
    double target_median=fa_pitch_median(original,voiced),donor_median=fa_pitch_median(mapped,voiced);
    free(original);free(mapped);
    if(!isfinite(target_median) || !isfinite(donor_median) || donor_median<=0) {free(points);return -1;}
    double scale=target_median/donor_median;
    if(!isfinite(scale) || scale<=0){free(points);return -1;}
    /* Reject overflow before writing any of the caller's frames. */
    for(size_t i=0;i<count;++i)if(rows[i].controls[1]>0) {
        double fraction=(double)(rows[i].tick-rows[first].tick)/span;
        if(fraction<0)fraction=0;else if(fraction>1)fraction=1;
        double value=scale*fa_pitch_interpolate(points,donor_voiced,points[0].tick+fraction*donor_span);
        if(!isfinite(value) || value>FLT_MAX){free(points);return -1;}
    }
    for(size_t i=0;i<count;++i)if(rows[i].controls[1]>0) {
        double fraction=(double)(rows[i].tick-rows[first].tick)/span;
        if(fraction<0)fraction=0;else if(fraction>1)fraction=1;
        double value=scale*fa_pitch_interpolate(points,donor_voiced,points[0].tick+fraction*donor_span);
        double weight=1;
        if(window && (edges&1u)) {
            double x=(double)(rows[i].tick-rows[first].tick)/(double)window;
            if(x<0)x=0;else if(x>1)x=1;
            weight*=x*x*(3-2*x);
        }
        if(window && (edges&2u)) {
            double x=(double)(rows[last].tick-rows[i].tick)/(double)window;
            if(x<0)x=0;else if(x>1)x=1;
            weight*=x*x*(3-2*x);
        }
        rows[i].controls[1]=(float)(rows[i].controls[1]+weight*(value-rows[i].controls[1]));
    }
    free(points);
    return 0;
}

FA_EXPORT int reloq_fa_fit_pitch(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,int64_t start,int64_t end,int64_t a,int64_t b)
{
    if(!rows || !count || !pilot || !length || start<0 || start>=end ||
       rows[count-1].tick<0 || rows[count-1].tick==INT64_MAX || end>rows[count-1].tick+1 ||
       a<0 || a>=b || (uint64_t)b>=length)return -1;
    int64_t first=start,last=end-1;
    while(first<end && fa_interpolated_voice(rows,count,first)<=0)++first;
    while(last>=start && fa_interpolated_voice(rows,count,last)<=0)--last;
    if(first>=end || last<start || first>=last)return 1;
    for(size_t i=0;i<count;++i) {
        int64_t tick=rows[i].tick;
        if(tick<start || tick>=end)continue;
        double t=tick<first?(double)a:tick>last?(double)b:
                 (double)a+(double)(b-a)*(double)(tick-first)/(double)(last-first);
        size_t lower=(size_t)t;
        if(lower>=length)return -1;
        double value=pilot[lower];
        if(lower+1<length)value+=((double)pilot[lower+1]-value)*(t-(double)lower);
        rows[i].controls[1]=(float)value;
    }
    return 0;
}

/* Preserve each English word's native pitch shape. Blend to the shared
 * sentence contour only at the voiced edges adjoining a Persian segment.
 * original is a request-local snapshot, so a streamed prefix and a later
 * full re-plan do not apply the blend twice. */
FA_EXPORT int reloq_fa_fit_pitch_edges(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,const float *original,int64_t start,int64_t end,
    int64_t a,int64_t b,unsigned edges,int64_t window)
{
    if(!rows || !count || !pilot || !length || !original || start<0 || start>=end ||
       rows[count-1].tick<0 || rows[count-1].tick==INT64_MAX || end>rows[count-1].tick+1 ||
       a<0 || a>=b || (uint64_t)b>=length || !edges || (edges&~3u) ||
       window<=0 || window>1000)return -1;
    int64_t first=start,last=end-1;
    while(first<end && fa_interpolated_voice(rows,count,first)<=0)++first;
    while(last>=start && fa_interpolated_voice(rows,count,last)<=0)--last;
    if(first>=end || last<start || first>=last)return 1;
    for(size_t i=0;i<count;++i) {
        int64_t tick=rows[i].tick;
        if(tick<start || tick>=end)continue;
        double t=tick<first?(double)a:tick>last?(double)b:
                 (double)a+(double)(b-a)*(double)(tick-first)/(double)(last-first);
        size_t lower=(size_t)t;
        double target=pilot[lower];
        if(lower+1<length)target+=((double)pilot[lower+1]-target)*(t-(double)lower);
        double weight=0.0;
        if(edges&1u) {
            double x=(double)(tick-first)/(double)window;
            if(x<0)x=0;else if(x>1)x=1;
            weight=1.0-x*x*(3.0-2.0*x);
        }
        if(edges&2u) {
            double x=(double)(last-tick)/(double)window;
            if(x<0)x=0;else if(x>1)x=1;
            double other=1.0-x*x*(3.0-2.0*x);
            if(other>weight)weight=other;
        }
        rows[i].controls[1]=(float)((double)original[i]+weight*(target-(double)original[i]));
    }
    return 0;
}

int reloq_fa_capture_append(int64_t tick,const float controls[62],
    float duration,float gain,int formants,int head,int gender,
    int s_frication,int limit_frication)
{
    if(!capture.active)return 0;
    if(capture.failed || !controls || tick<0 || !isfinite(duration) || duration<=0 ||
       !isfinite(gain) || gain<0 || gain>16 || formants<1 || formants>8 ||
       head<0 || head>100 || (gender!=0 && gender!=1) ||
       capture.count>=capture.maximum) {
        capture.failed=1;return -1;
    }
    if(capture.count==capture.capacity) {
        size_t next=capture.capacity?capture.capacity*2:512;
        if(next>capture.maximum)next=capture.maximum;
        if(next<capture.capacity || next>SIZE_MAX/sizeof *capture.frames) {
            capture.failed=1;return -1;
        }
        void *data=realloc(capture.frames,next*sizeof *capture.frames);
        if(!data){capture.failed=1;return -1;}
        capture.frames=data;capture.capacity=next;
    }
    struct reloq_fa_frame *row=capture.frames+capture.count++;
    row->tick=tick;
    memcpy(row->controls,controls,sizeof row->controls);
    row->controls[0]=duration;
    row->gain=gain;row->formants=formants;row->head=head;row->gender=gender;
    row->s_frication=s_frication;row->limit_frication=limit_frication;
    return 0;
}
