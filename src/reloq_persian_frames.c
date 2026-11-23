/* Bounded, thread-local capture of native Persian/English frames. */
#include "reloq_persian_frame_internal.h"
#include "reloq_persian_transition.h"
#include "reloq_persian_tuning.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FA_U_FINAL_R_MARKER (RELOQ_FA_TUNING_MARKER+13.0f)
#define FA_R_I_AFTER_U_MARKER (RELOQ_FA_TUNING_MARKER+15.0f)
#define FA_U_BEFORE_R_MARKER (RELOQ_FA_TUNING_MARKER+16.0f)
#define FA_R_I_UPPER_MARKER (RELOQ_FA_TUNING_MARKER+17.0f)

static _Thread_local int fa_tuning_on=1;
int reloq_fa_tuning_scope(int active)
{
    /* 2 is the previous saved project, retained for a controlled A/B test. */
    if(active!=0 && active!=1 && active!=2)return -1;
    int previous=fa_tuning_on;fa_tuning_on=active;return previous;
}
int reloq_fa_tuning_enabled(void)
{return fa_tuning_on;}

/* The listener's saved Lautlabor project: frequency shifts are applied to
 * the voice-transformed controls, as in the editor, rather than to the raw
 * language targets. The level edits are the same formant-following peaking
 * filters used in its audition, not guessed native formant amplitudes. */
void reloq_fa_tuning_targets(float c[62],unsigned phone,double from,double to,
                             double at,double duration,int rate,double release_ms)
{
    static const int shifts[6][4]={
        {0,0,0,0},{25,-50,25,75},{0,-25,50,25},{0,0,-25,75},
        {0,0,50,425},{0,0,0,0}};
    static const unsigned columns[4]={9,13,15,17};
    int after_r=phone==RELOQ_FA_TUNE_BACK_A_AFTER_R && fa_tuning_on==1;
    if(phone==RELOQ_FA_TUNE_BACK_A_AFTER_R)phone=RELOQ_FA_TUNE_BACK_A;
    if(!fa_tuning_on || !c || phone<1 || (phone>5 && phone!=RELOQ_FA_TUNE_U) ||
       from<0 || to<=from || at<from || at>=to || rate<8000)return;
    double fade_in=after_r?1.0:fmax(0.0,(at-from)/3.0);
    double w=fmin(1.0,fmin(fade_in,fmax(0.0,(to-at)/release_ms)));
    for(unsigned i=0;phone<=5 && i<4;++i)if(shifts[phone][i] && c[columns[i]]>0 && w>0) {
        /* The saved O adds 425 Hz to F4. For a short O immediately before
         * R, the normal 3-ms attack can be as abrupt as the release that
         * the R-context rule already stretches. Fade this one adjustment in
         * across 10 ms, while keeping other vowels and O contexts intact. */
        double weight=w;
        if(phone==RELOQ_FA_TUNE_O && i==3 && release_ms>3.0)
            weight=fmin(weight,fmax(0.0,(at-from)/10.0));
        double v=c[columns[i]]+shifts[phone][i]*weight;
        c[columns[i]]=(float)fmax(40.0,fmin(rate*.475,v));
    }
    /* The newer long-a project was edited on top of the saved project, not
     * on the unmodified core. Apply its deltas as a second editor step. */
    static const int revised_a[4]={100,150,-75,-50};
    if(fa_tuning_on==1 && phone==RELOQ_FA_TUNE_BACK_A)
        for(unsigned i=0;i<4;++i)if(revised_a[i] && c[columns[i]]>0 && w>0) {
            double v=c[columns[i]]+revised_a[i]*w;
            c[columns[i]]=(float)fmax(40.0,fmin(rate*.475,v));
        }
    /* The newer U project is relative to the existing voice as well.
     * Its fifth formant is edited in addition to the first four. */
    static const int revised_u[5]={0,0,25,50,25};
    static const unsigned u_columns[5]={9,13,15,17,19};
    if(fa_tuning_on==1 && phone==RELOQ_FA_TUNE_U)
        for(unsigned i=0;i<5;++i)if(revised_u[i] && c[u_columns[i]]>0 && w>0) {
            double v=c[u_columns[i]]+revised_u[i]*w;
            c[u_columns[i]]=(float)fmax(40.0,fmin(rate*.475,v));
        }
    double centre=at+duration/2.0,gain=1.0;
    if(from>0)gain=fmin(gain,fmax(0.0,(centre-from)/8.0));
    gain=fmin(gain,fmax(0.0,(to-centre)/8.0));
    if(gain>0){c[11]=(float)(RELOQ_FA_TUNING_MARKER+phone);c[12]=(float)gain;}
}

/* The revised back-A targets used to appear only after the vowel edge,
 * producing an abrupt formant step in an already voiced R-to-A transition.
 * Bring those exact targets in smoothly after the R's articulation ends.
 * The other vowel edits, the previous project and the steady vowel are not
 * changed. Work in the same transformed control domain as the saved edit. */
void reloq_fa_tuning_r_to_back_a(float c[62],double from,double to,double at,int rate)
{
    static const int shift[4]={100,125,-25,-25};
    static const unsigned columns[4]={9,13,15,17};
    if(fa_tuning_on!=1 || !c || rate<8000 || from<0 || to<=from || at<from || at>=to)return;
    double w=fmax(0.0,fmin(1.0,(at-from)/(to-from)));
    w=w*w*(3.0-2.0*w);
    for(unsigned i=0;i<4;++i)if(c[columns[i]]>0) {
        double value=c[columns[i]]+shift[i]*w;
        c[columns[i]]=(float)fmax(40.0,fmin(rate*.475,value));
    }
}

static void tuning_prepare(struct reloq_fa_tuning_filter *f,const float c[62],int rate,int revision)
{
    static const int gains[6][5]={
        {0,0,0,0,0},{0,0,0,-9,0},{1,-1,-3,-4,0},
        {0,0,0,0,0},{0,0,0,0,0},{0,0,-4,-2,0}};
    static const unsigned centres[5]={9,13,15,17,19};
    static const int revised_a_gains[5]={1,1,-1,2,0};
    static const int revised_u_gains[5]={1,3,6,12,9};
    if(!f || !c)return;
    int marker=isfinite(c[11]) && c[11]>=RELOQ_FA_TUNING_MARKER+1 &&
               c[11]<=RELOQ_FA_TUNING_MARKER+RELOQ_FA_TUNE_R_VOICE_CODA ? (int)c[11]-RELOQ_FA_TUNING_MARKER:0;
    unsigned phone=(marker>=1 && (marker<=5 || marker==RELOQ_FA_TUNE_U) &&
                    c[11]==(float)(RELOQ_FA_TUNING_MARKER+marker)) ? (unsigned)marker:0;
    /* Preserve the saved U filter while its independent periodic-source
     * smoothing weight occupies the fractional transition marker. */
    if(c[11]>=FA_U_FINAL_R_MARKER && c[11]<FA_U_FINAL_R_MARKER+1.0f)
        phone=RELOQ_FA_TUNE_U;
    if(c[11]>=FA_U_BEFORE_R_MARKER && c[11]<FA_U_BEFORE_R_MARKER+1.0f)
        phone=RELOQ_FA_TUNE_U;
    if(c[11]>=FA_R_I_AFTER_U_MARKER && c[11]<FA_R_I_AFTER_U_MARKER+1.0f)
        phone=RELOQ_FA_TUNE_R_BEFORE_I;
    if(c[11]==FA_R_I_UPPER_MARKER)phone=RELOQ_FA_TUNE_R_BEFORE_I;
    if(marker==RELOQ_FA_TUNE_R_VOICE_BEFORE_I &&
       c[11]==(float)(RELOQ_FA_TUNING_MARKER+marker))phone=RELOQ_FA_TUNE_R_BEFORE_I;
    double weight=c[12];
    if(!fa_tuning_on || rate<8000 || rate>48000 || !isfinite(weight) || weight<=0 || weight>1)phone=0;
    if(revision && (fa_tuning_on!=1 ||
       (phone!=RELOQ_FA_TUNE_BACK_A && phone!=RELOQ_FA_TUNE_U)))phone=0;
    if(!phone) {
        if(f->phone || f->active)memset(f,0,sizeof *f);
        return;
    }
    if(phone!=f->phone || rate!=f->rate) {
        memset(f,0,sizeof *f);
        f->phone=phone;f->rate=rate;
    }
    f->active=0;
    for(unsigned i=0;i<5;++i) {
        struct reloq_fa_tuning_band *b=f->bands+i;
        double frequency=c[centres[i]],width=c[centres[i]+1];
        double gain=(revision?(phone==RELOQ_FA_TUNE_U?revised_u_gains[i]:revised_a_gains[i]):
                     (phone==RELOQ_FA_TUNE_U?0:gains[phone][i]))*weight;
        b->active=0;
        if(!gain || !isfinite(frequency) || !isfinite(width) || frequency<30 || frequency>=rate*.475)continue;
        width=fmax(50.0,width);
        if(frequency!=b->frequency || width!=b->width || gain!=b->gain) {
            double q=fmax(.5,fmin(12.0,frequency/width));
            double omega=6.2831853071795864769*frequency/rate;
            double alpha=sin(omega)/(2*q),a=pow(10.0,gain/40.0),den=1+alpha/a;
            double middle=-2*cos(omega);
            b->b0=(1+alpha*a)/den;b->b1=middle/den;b->b2=(1-alpha*a)/den;
            b->a1=middle/den;b->a2=(1-alpha/a)/den;
            b->frequency=frequency;b->width=width;b->gain=gain;
        }
        b->active=1;f->active=1;
    }
}

void reloq_fa_tuning_prepare(struct reloq_fa_tuning_filter *f,const float c[62],int rate)
{tuning_prepare(f,c,rate,0);}

void reloq_fa_tuning_prepare_revision(struct reloq_fa_tuning_filter *f,const float c[62],int rate)
{tuning_prepare(f,c,rate,1);}

double reloq_fa_tuning_sample(struct reloq_fa_tuning_filter *f,double x)
{
    if(!f || !f->active)return x;
    for(unsigned i=0;i<5;++i) {
        struct reloq_fa_tuning_band *b=f->bands+i;
        if(!b->active)continue;
        double y=b->b0*x+b->b1*b->x1+b->b2*b->x2-b->a1*b->y1-b->a2*b->y2;
        b->x2=b->x1;b->x1=x;b->y2=b->y1;b->y1=y;x=y;
    }
    return x;
}

#if defined(RELOQ_FA_PROBE_EXPORT) && defined(_WIN32)
#define FA_EXPORT __declspec(dllexport)
#elif defined(RELOQ_FA_PROBE_EXPORT) && (defined(__GNUC__) || defined(__clang__))
#define FA_EXPORT __attribute__((visibility("default")))
#else
#define FA_EXPORT
#endif

struct capture_state {
    struct reloq_fa_frame *frames;
    struct reloq_fa_syllable *syllables;
    uint32_t *phone_identities;
    int16_t *pcm;
    size_t count, capacity, maximum, samples;
    size_t pcm_capacity;
    size_t syllable_count, syllable_capacity;
    int syllable_failed;
    size_t phone_identity_count, phone_identity_capacity;
    int phone_identity_failed;
    int active, failed, controls_only, store_pcm;
};
static _Thread_local struct capture_state capture;

FA_EXPORT int reloq_fa_capture_begin(size_t maximum)
{
    if(capture.active || !maximum || maximum>120000 ||
       maximum>SIZE_MAX/sizeof *capture.frames)return -1;
    free(capture.frames);
    free(capture.pcm);
    free(capture.syllables);
    free(capture.phone_identities);
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

/* Called by the capture's own callback thread. The caller must copy the
 * returned rows before returning to the synthesizer, which can reallocate
 * the capture on its next frame. */
const struct reloq_fa_frame *reloq_fa_capture_current(size_t *count, int *failed)
{
    if(!count || !failed || !capture.active)return NULL;
    *count=capture.count;*failed=capture.failed;
    return capture.frames;
}

FA_EXPORT void reloq_fa_capture_release(void)
{
    free(capture.frames);
    free(capture.pcm);
    free(capture.syllables);
    free(capture.phone_identities);
    capture=(struct capture_state){0};
}

void reloq_fa_capture_syllable(void *context,int32_t from,int32_t to)
{
    (void)context;
    if(!capture.active || capture.syllable_failed)return;
    if(from<0 || to<=from || capture.syllable_count>=4096) {
        capture.syllable_failed=1;return;
    }
    if(capture.syllable_count==capture.syllable_capacity) {
        size_t next=capture.syllable_capacity?capture.syllable_capacity*2:32;
        if(next>4096)next=4096;
        void *buffer=realloc(capture.syllables,next*sizeof *capture.syllables);
        if(!buffer){capture.syllable_failed=1;return;}
        capture.syllables=buffer;capture.syllable_capacity=next;
    }
    capture.syllables[capture.syllable_count++]=(struct reloq_fa_syllable){from,to};
}

FA_EXPORT const struct reloq_fa_syllable *reloq_fa_capture_syllables(size_t *count,int *failed)
{
    if(!count || !failed)return NULL;
    *count=capture.syllable_count;*failed=capture.syllable_failed;
    return capture.syllables;
}

void reloq_fa_capture_phone_identity(void *context,uint32_t identity)
{
    (void)context;
    if(!capture.active || capture.phone_identity_failed)return;
    if(capture.phone_identity_count>=4096){capture.phone_identity_failed=1;return;}
    if(capture.phone_identity_count==capture.phone_identity_capacity) {
        size_t next=capture.phone_identity_capacity?capture.phone_identity_capacity*2:32;
        if(next>4096)next=4096;
        void *buffer=realloc(capture.phone_identities,next*sizeof *capture.phone_identities);
        if(!buffer){capture.phone_identity_failed=1;return;}
        capture.phone_identities=buffer;capture.phone_identity_capacity=next;
    }
    capture.phone_identities[capture.phone_identity_count++]=identity;
}

const uint32_t *reloq_fa_capture_phone_identities(size_t *count,int *failed)
{
    if(!count || !failed)return NULL;
    *count=capture.phone_identity_count;*failed=capture.phone_identity_failed;
    return capture.phone_identities;
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
    /* As with capture_current, a callback may inspect an in-progress
     * request on its owning thread. Copy this pointer before synthesis
     * resumes: the next frame may reallocate the underlying buffer. */
    if(!count || !capture.store_pcm)return NULL;
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

/* Select the same voiced donor interval as the request-local Python planner.
 * Real captures are bounded and use small frame ticks; unusual values return
 * an error so older/general callers can retain their own reference algorithm. */
FA_EXPORT int reloq_fa_donor_span(const struct reloq_fa_frame *rows,size_t count,
    size_t leading,size_t syllables,size_t trailing,size_t *first,size_t *last)
{
    if(!first || !last || (!rows && count) || count>120000 || leading>120000 ||
       syllables>120000 || trailing>120000)return -1;
    if(!leading && !trailing){*first=0;*last=count;return 0;}
    if(!syllables)return -1;
    size_t total=leading+syllables+trailing,runs=0,start=0,wanted_first=0,wanted_last=0;
    size_t first_active=0,last_active=0;
    int active=0;
    for(size_t i=0;i<=count;++i) {
        if(i<count && (rows[i].tick<INT32_MIN || rows[i].tick>INT32_MAX))return -1;
        int voiced=i<count && rows[i].controls[2]>10 && rows[i].controls[1]>0;
        if(voiced && !active){start=i;active=1;}
        else if(!voiced && active) {
            if(!runs)first_active=start;
            last_active=i-1;
            if(runs==leading)wanted_first=start;
            if(runs==leading+syllables-1)wanted_last=i;
            ++runs;active=0;
        }
    }
    if(!runs){*first=*last=0;return 0;}
    if(runs==total){*first=wanted_first;*last=wanted_last;return 0;}
    int64_t begin_tick=rows[first_active].tick,end_tick=rows[last_active].tick;
    double width=(double)(end_tick-begin_tick);
    double begin=(double)begin_tick+width*(double)leading/(double)total;
    double end=(double)begin_tick+width*(double)(leading+syllables)/(double)total;
    size_t lo=0,hi=count;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if((double)rows[mid].tick<begin)lo=mid+1;else hi=mid;
    }
    *first=lo;lo=0;hi=count;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if((double)rows[mid].tick<=end)lo=mid+1;else hi=mid;
    }
    *last=lo;
    return 0;
}

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

/* Transfer the Korean donor's pitch without inheriting the Finnish sentence
 * median. Where Persian word markers are available, align word boundaries
 * against the donor's syllable voicing runs. If those cannot be identified,
 * use syllable proportions; if Persian markers are unavailable, keep one
 * monotonic phrase-wide mapping. No caller-owned rows change on an error. */
FA_EXPORT int reloq_fa_transfer_korean_aligned(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_frame *donor,size_t donor_count,const int64_t *word_ticks,
    const uint32_t *syllables,size_t word_count)
{
    if(!rows || !donor || !count || !donor_count || count>120000 || donor_count>120000 ||
       !syllables || !word_count || word_count>4096)return -1;
    size_t syllable_count=0,voiced=0,first=0,last=0;
    for(size_t i=0;i<word_count;++i) {
        if(!syllables[i] || syllables[i]>120000-syllable_count)return -1;
        syllable_count+=syllables[i];
    }
    for(size_t i=0;i<count;++i) {
        if(rows[i].tick<0 || (i && rows[i].tick<=rows[i-1].tick) ||
           !isfinite(rows[i].controls[1]) || !isfinite(rows[i].controls[2]))return -1;
        if(rows[i].controls[2]>10 && rows[i].controls[1]>0) {
            if(!voiced)first=i;
            last=i;++voiced;
        }
    }
    if(voiced<2 || rows[first].tick>=rows[last].tick)return 1;
    struct fa_pitch_point *points=malloc(donor_count*sizeof *points);
    size_t *run_start=malloc(donor_count*sizeof *run_start),*run_end=malloc(donor_count*sizeof *run_end);
    double *target=malloc((word_count+1)*sizeof *target),*source=malloc((word_count+1)*sizeof *source);
    if(!points || !run_start || !run_end || !target || !source) {
        free(points);free(run_start);free(run_end);free(target);free(source);return -2;
    }
    size_t n=0,runs=0;int active=0,status=0;
    for(size_t i=0;i<donor_count;++i) {
        if(donor[i].tick<0 || (i && donor[i].tick<=donor[i-1].tick) ||
           !isfinite(donor[i].controls[1]) || !isfinite(donor[i].controls[2])) {status=-1;goto done;}
        if(donor[i].controls[2]>10 && donor[i].controls[1]>0) {
            if(!active){run_start[runs]=n;active=1;}
            points[n++]=(struct fa_pitch_point){donor[i].tick,donor[i].controls[1]};
        } else if(active) {
            run_end[runs++]=n-1;active=0;
        }
    }
    if(active)run_end[runs++]=n-1;
    if(n<2 || points[0].tick>=points[n-1].tick){status=1;goto done;}
    target[0]=(double)rows[first].tick;source[0]=(double)points[0].tick;
    size_t intervals=1,used=0;
    if(word_ticks && word_count>1) {
        int valid=1;
        for(size_t i=1;i<word_count;++i) {
            used+=syllables[i-1];
            target[i]=(double)word_ticks[i-1];
            source[i]=(runs==syllable_count)
                ? ((double)points[run_end[used-1]].tick+(double)points[run_start[used]].tick)/2.0
                : source[0]+(double)(points[n-1].tick-points[0].tick)*(double)used/(double)syllable_count;
            if(target[i]<=target[i-1] || target[i]>=rows[last].tick ||
               source[i]<=source[i-1] || source[i]>=points[n-1].tick)valid=0;
        }
        if(valid)intervals=word_count;
    }
    target[intervals]=(double)rows[last].tick;source[intervals]=(double)points[n-1].tick;
    /* Validate every result before writing, preserving the target on error. */
    for(int pass=0;pass<2;++pass)for(size_t i=0;i<count;++i)if(rows[i].controls[1]>0) {
        double tick=(double)rows[i].tick;
        size_t lo=0,hi=intervals;
        if(tick<target[0])tick=target[0];
        if(tick>target[intervals])tick=target[intervals];
        while(lo+1<hi){size_t mid=lo+(hi-lo)/2;if(target[mid]<=tick)lo=mid;else hi=mid;}
        double position=source[lo]+(source[lo+1]-source[lo])*(tick-target[lo])/(target[lo+1]-target[lo]);
        double value=fa_pitch_interpolate(points,n,position);
        if(!isfinite(value) || value<=0 || value>FLT_MAX){status=-1;goto done;}
        if(pass)rows[i].controls[1]=(float)value;
    }
done:
    free(points);free(run_start);free(run_end);free(target);free(source);
    return status;
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

/* Warp a donor word through real corresponding syllable boundaries. The
 * target retains its own complete frame clock and acoustic controls. Invalid
 * or unvoiced anchors return without modifying any frame, allowing the caller
 * to use the established whole-word transfer. */
FA_EXPORT int reloq_fa_fit_pitch_anchors(struct reloq_fa_frame *rows,size_t count,
    const float *pilot,size_t length,int64_t start,int64_t end,int64_t a,int64_t b,
    const int64_t *target,const int64_t *source,size_t anchors)
{
    if(!rows || !count || !pilot || !length || !target || !source || !anchors || anchors>255 ||
       start<0 || start>=end || rows[count-1].tick<0 || rows[count-1].tick==INT64_MAX ||
       end>rows[count-1].tick+1 || a<0 || a>=b || (uint64_t)b>=length)return -1;
    int64_t first=start,last=end-1;
    while(first<end && fa_interpolated_voice(rows,count,first)<=0)++first;
    while(last>=start && fa_interpolated_voice(rows,count,last)<=0)--last;
    if(first>=last)return 1;
    int64_t pt=first,ps=a;
    for(size_t i=0;i<anchors;++i) {
        if(target[i]<=pt || target[i]>=last || source[i]<=ps || source[i]>=b)return 1;
        pt=target[i];ps=source[i];
    }
    for(size_t i=0;i<count;++i) {
        int64_t tick=rows[i].tick;
        if(tick<start || tick>=end)continue;
        double t;
        if(tick<=first)t=(double)a;
        else if(tick>=last)t=(double)b;
        else {
            size_t j=0;
            while(j<anchors && tick>target[j])++j;
            int64_t tl=j?target[j-1]:first, sl=j?source[j-1]:a;
            int64_t tr=j<anchors?target[j]:last, sr=j<anchors?source[j]:b;
            t=(double)sl+(double)(sr-sl)*(double)(tick-tl)/(double)(tr-tl);
        }
        size_t lower=(size_t)t;
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
