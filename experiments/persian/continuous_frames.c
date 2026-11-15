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
    size_t count, capacity, maximum;
    int active, failed;
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
