/* Private, bounded, thread-local capture of native Persian/English frames. */
#include "continuous_frames.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define FA_EXPORT __declspec(dllexport)
#else
#define FA_EXPORT __attribute__((visibility("default")))
#endif

struct capture_state {
    struct reloq_fa_frame *frames;
    size_t count, capacity, maximum, samples;
    int active, failed, controls_only;
};
static _Thread_local struct capture_state capture;

FA_EXPORT int reloq_fa_capture_begin(size_t maximum)
{
    if(capture.active || !maximum || maximum>120000 ||
       maximum>SIZE_MAX/sizeof *capture.frames)return -1;
    free(capture.frames);
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
    capture=(struct capture_state){0};
}

FA_EXPORT int reloq_fa_capture_set_controls_only(int enabled)
{
    if(!capture.active || (enabled!=0 && enabled!=1))return -1;
    capture.controls_only=enabled;
    return 0;
}

int reloq_fa_capture_controls_only(void)
{ return capture.active && capture.controls_only; }

int reloq_fa_capture_advance_samples(size_t count)
{
    if(!capture.active || !capture.controls_only || count>SIZE_MAX-capture.samples)return -1;
    capture.samples+=count;
    return 0;
}

FA_EXPORT size_t reloq_fa_capture_sample_count(void)
{ return capture.active && capture.controls_only ? capture.samples : SIZE_MAX; }

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
