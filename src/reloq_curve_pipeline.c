#include "reloq_curve_pipeline.h"

#include "reloq_frame_adapter.h"

#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <string.h>

/* A completed schedule is read-only while it is rendered. Validate each
 * curve once, keep constant controls in one frame template and walk only
 * the moving tracks. No utterance or pronunciation result is retained. */
struct moving_control {
    const struct reloq_acoustic_curve *curve;
    size_t left;
    unsigned field;
};
static int constant_control(const struct reloq_acoustic_curve *curve,float *out)
{
    if(!curve->point_count){*out=curve->default_value;return 1;}
    int32_t first=curve->points[0].value;
    if(curve->point_count==1){*out=(float)first;return 1;}
    /* The general reader rejects an interpolated value rounded up to
     * 2^31, even on a flat segment. Keep that boundary on its normal path. */
    if((double)(float)first>=2147483648.0)return 0;
    for(size_t i=1;i<curve->point_count;++i)
        if(curve->points[i].value!=first)return 0;
    *out=(float)first;return 1;
}
static int moving_value(struct moving_control *control,int64_t tick,float *out)
{
    const struct reloq_acoustic_curve *curve=control->curve;
    const struct reloq_acoustic_point *points=curve->points;
    size_t i=control->left;
    if(tick<points[0].tick){*out=(float)points[0].value;return 0;}
    /* Duplicate timestamps retain their first target at the exact tick
     * and their last replacement on the following tick. */
    while(i+1<curve->point_count&&points[i+1].tick<tick)++i;
    if(i+1<curve->point_count&&points[i].tick<tick&&points[i+1].tick==tick)++i;
    float value=(float)points[i].value;
    if(i+1<curve->point_count&&points[i+1].tick>tick) {
        uint64_t span=(uint64_t)points[i+1].tick-(uint64_t)points[i].tick;
        uint64_t elapsed=(uint64_t)tick-(uint64_t)points[i].tick;
        int32_t difference=(int32_t)((int64_t)points[i+1].value-points[i].value);
        float offset=(float)(int64_t)elapsed;
        offset*=(float)difference;offset/=(float)(int64_t)span;value+=offset;
        if(!isfinite(value)||(double)value<(double)INT32_MIN||
            (double)value>=2147483648.0)return -1;
        value=(float)(int32_t)value;
    }
    control->left=i;*out=value;return 0;
}

int reloq_curve_pipeline_render_models(
    const struct reloq_curve_schedule *schedule, long highest_formant,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    size_t *frames_completed,reloq_frame_profile profile,void *profile_context,
    reloq_frame_formants formants)
{
    if (frames_completed) *frames_completed = 0;
    if (!schedule || !session || !sink || !schedule->frame_ticks ||
        schedule->first_tick > schedule->end_tick)
        return RELOQ_SESSION_INVALID;

    struct moving_control moving[RELOQ_PARAMETER_TRACKS];
    float fixed[RELOQ_PARAMETER_TRACKS]={0};size_t moving_count=0;
    for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i) {
        struct reloq_acoustic_cursor checked;
        if (reloq_acoustic_begin(&schedule->tracks[i], &checked))
            return RELOQ_SESSION_INVALID;
        if(!constant_control(&schedule->tracks[i],fixed+i))
            moving[moving_count++]=(struct moving_control){&schedule->tracks[i],0,(unsigned)i};
    }
    struct reloq_acoustic_cursor colour,sibilant;
    if(schedule->s_frication &&
       reloq_acoustic_begin(schedule->s_frication,&sibilant))return RELOQ_SESSION_INVALID;
    if(schedule->bypass_attenuation &&
       reloq_acoustic_begin(schedule->bypass_attenuation,&colour))return RELOQ_SESSION_INVALID;

    size_t completed = 0;
    int64_t tick = schedule->first_tick;
    while (tick < schedule->end_tick) {
        if (reloq_session_cancelled(session)) return RELOQ_SESSION_CANCELLED;

        float controls[RELOQ_PARAMETER_TRACKS];
        memcpy(controls,fixed,sizeof controls);
        for (size_t i = 0; i < moving_count; ++i)
            if (moving_value(moving+i, tick, controls+moving[i].field))
                return RELOQ_SESSION_INVALID;

        if(profile)profile(profile_context,controls);
        struct reloq_core_voiced_frame frame;
        long active_formants=formants?formants(profile_context):highest_formant;
        if (reloq_frame_adapter_map(controls, active_formants, &frame))
            return RELOQ_SESSION_INVALID;
        if(schedule->bypass_attenuation) {
            float db;
            if(reloq_acoustic_value(&colour,tick,&db) || db<0 || db>1200)
                return RELOQ_SESSION_INVALID;
            frame.noise.bypass_attenuation_db=db*.01f;
        }

        if(schedule->s_frication) {
            float active;
            if(reloq_acoustic_value(&sibilant,tick,&active))return RELOQ_SESSION_INVALID;
            frame.noise.s_frication=active>=.5f?(schedule->s_frication_profile==1?2:1):0;
        }

        size_t delivered = 0;
        int result = reloq_session_render_frames(session, &frame, 1, sink,
                                                  context, &delivered);
        if (delivered) {
            if (completed == SIZE_MAX) return RELOQ_SESSION_INVALID;
            completed += delivered;
            if (frames_completed) *frames_completed = completed;
        }
        if (result != RELOQ_SESSION_OK) return result;

        /* Unsigned subtraction supports even a timeline spanning the
         * complete signed tick range. Advance only as far as end_tick, so
         * the signed addition below cannot overflow. */
        uint64_t remaining = (uint64_t)schedule->end_tick - (uint64_t)tick;
        uint32_t advance = remaining < schedule->frame_ticks ?
                           (uint32_t)remaining : schedule->frame_ticks;
        tick += advance;
    }
    return RELOQ_SESSION_OK;
}

int reloq_curve_pipeline_render_profile(
    const struct reloq_curve_schedule *schedule,long highest_formant,
    struct reloq_session *session,reloq_session_pcm_sink sink,void *context,
    size_t *frames_completed,reloq_frame_profile profile,void *profile_context)
{
    return reloq_curve_pipeline_render_models(schedule,highest_formant,session,
        sink,context,frames_completed,profile,profile_context,NULL);
}

int reloq_curve_pipeline_render(const struct reloq_curve_schedule *schedule,
    long highest_formant,struct reloq_session *session,reloq_session_pcm_sink sink,
    void *context,size_t *frames_completed)
{
    return reloq_curve_pipeline_render_profile(schedule,highest_formant,session,
        sink,context,frames_completed,NULL,NULL);
}
