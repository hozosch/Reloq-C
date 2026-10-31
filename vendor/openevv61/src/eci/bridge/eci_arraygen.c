#include "reloq61_english_bank.h"
/* Turning a language's arrays into a stream of synthesiser frames.
 *
 * A Delta array is a sparse thing: a stream of values, each at some offset,
 * with nothing said about the gaps. The synthesiser wants the opposite -- a
 * full frame of sixty-two parameters every step, for ever. What sits between
 * the two is a value set: one cursor per stream, each remembering the pair of
 * points it is currently between, so asking for a value at a moment is a
 * matter of walking the cursor forward and interpolating between them.
 *
 * The interpolation is a straight line in whole numbers: how far through the
 * gap the moment is, times how much the value changes across it, divided by
 * how wide the gap is. A gap of no width, or no change, answers the value it
 * started at.
 *
 * The frame handed on is filled from a set of defaults, then overwritten by
 * whichever streams the caller's map names, and its first word is the step --
 * shortened on the last frame so the run ends where it was asked to.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int32_t arrayStreamFind(void*,const char*);
#include "delta.h"
#include "reloq61.h"
#include "klatt_state.h"
#include "klatt_lang.h"
#include "reloq61_parameter_names.h"
#include "reloq61_rule_trace.h"

/* How many parameters a frame carries. */
#define FRAME_PARMS 0x3e

/* What each frame parameter was interpolated from, when EVV_ARRAY_TAP names
 * a file.
 *
 * A formant track is not a value some rule wrote down and it is not a curve
 * fitted to anything. It is a list of breakpoints in an array -- a value at
 * an offset, and a straight line in whole numbers to the next -- and this
 * writes that list out as the cursors cross onto each one. Which is the
 * values and the timing together, the two things a generator needs and the
 * second of which nothing else here reports.
 *
 * A line a breakpoint: which parameter, where it starts, how wide, and the
 * two values. `map' lines say which stream each parameter reads, once a run.
 */
static const char *array_tap_name[FRAME_PARMS];

#if EVV_RULE_TRACE_ENABLED
static FILE *array_tap;
static int   array_tap_tried;
static FILE *arrayTap(void)
{
    const char *name;

    if (array_tap_tried)
        return array_tap;
    array_tap_tried = 1;
    name = getenv("EVV_ARRAY_TAP");
    if (name != 0 && *name != 0)
        array_tap = fopen(name, "w");
    return array_tap;
}
#else
#define arrayTap() ((FILE *)0)
#endif

/* One cursor: the two points it is between, and the gap worked out from
   them. */
typedef struct Cursor {
    int32_t at_l;      /* +0x00, where the left point is */
    int32_t val_l;     /* +0x04 */
    int32_t at_r;      /* +0x08, where the right point is; -1 means unstarted */
    int32_t val_r;     /* +0x0c */
    int32_t span;      /* +0x10, how wide the gap is */
    int32_t rise;      /* +0x14, how much the value changes across it */
} Cursor;

typedef struct ValueSet {
    Cursor  *cursors;  /* +0x00, one per stream */
    int32_t  from;     /* +0x04 */
    int32_t  to;       /* +0x08 */
} ValueSet;

/* What the array generator hangs off the machine. */
#define GEN(d)       EVV_AT(DeltaLang *, ((delta_state *)(d))->dlang)
#define GEN_SET(d)      (*(ValueSet **)&GEN(d)->set_a)
#define GEN_AT(d)       (GEN(d)->at)
#define GEN_KLATT(d)    (GEN(d)->klatt)
#define GEN_END(d)      (GEN(d)->spoken)
#define GEN_STARTED(d)  (GEN(d)->marked)
#define GEN_LIMIT(d)    (GEN(d)->queued)

extern int32_t streamArrayCount(delta_state *d);
extern int32_t arrayStreamLastOffset(delta_state *d, int32_t stream);
extern int32_t arrayStreamFirstVal(delta_state *d, int32_t stream,
                                   int32_t *at, int32_t *val);
extern int32_t arrayStreamNextVal(delta_state *d, int32_t stream,
                                  int32_t *at, int32_t *val);

static void valueSetReset(delta_state *d, ValueSet *vs, int32_t from,
                          int32_t to);

/* Every cursor unstarted, and the window recorded. */
static void valueSetReset(delta_state *d, ValueSet *vs, int32_t from,
                          int32_t to)
{
    int32_t i;

    vs->from = from;
    vs->to = to;

    for (i = 0; i < streamArrayCount(d); i++)
        vs->cursors[i].at_r = -1;
}

/* A private rule-only request has its own zero-based output clock. Keep the
   allocated interpolation cursors, but discard the preceding utterance's
   window and positions before its next independent control plan. */
void reloq61_array_reset(delta_state *d)
{
    if(GEN_SET(d))valueSetReset(d,GEN_SET(d),0,0);
    GEN_AT(d)=GEN_END(d)=GEN_STARTED(d)=GEN_LIMIT(d)=0;
}

static ValueSet *valueSetNew(delta_state *d, int32_t from, int32_t to)
{
    ValueSet *vs = (ValueSet *)malloc(sizeof(ValueSet));

    if (!vs)
        return 0;

    vs->cursors = (Cursor *)calloc((size_t)streamArrayCount(d),
                                   sizeof(Cursor));
    if (!vs->cursors) {
        free(vs);
        return 0;
    }

    valueSetReset(d, vs, from, to);
    return vs;
}

static int32_t valueSetDelete(ValueSet *vs)
{
    if (vs) {
        if (vs->cursors)
            free(vs->cursors);
        free(vs);
    }
    return 0;
}
/* A lexical bank replaces targets only from its current boundary onward.
 * Complete the preceding window before calling this operation. */
void reloq61_array_bank_begin(delta_state*d,int32_t at)
{
 extern void reloq61_array_bank_reset(void*,int32_t);
 if(GEN_SET(d))valueSetReset(d,GEN_SET(d),at,at);
 reloq61_array_bank_reset(d,at);
 GEN_AT(d)=GEN_END(d)=GEN_LIMIT(d)=at;
}


static int32_t valueSetInRange(const ValueSet *vs, int32_t at)
{
    return at >= vs->from && at < vs->to;
}

/* Walk one stream's cursor forward until the moment falls inside the gap it
   is holding, then read off the line between the two points. A stream that
   has run out holds its last value to the end of the window. */
static int32_t valueSetValue(delta_state *d, ValueSet *vs, int32_t stream,
                             int32_t at)
{
    Cursor *c = &vs->cursors[stream];
    int32_t moved = 0;

    while (at > c->at_r) {
        moved = 1;
        int32_t next = 0;
        int32_t val = 0;

        if (c->at_r == -1) {
            /* Nothing read yet: take the first point, or give up on the
               stream and hold nought to the end. */
            if (arrayStreamFirstVal(d, stream, &val, &next)) {
                c->at_l = 0;
                c->val_l = next;
                c->at_r = val;
                c->val_r = next;
            } else {
                c->at_l = 0;
                c->val_l = 0;
                c->at_r = vs->to;
                c->val_r = 0;
            }
        } else {
            /* Step on: what was the right point becomes the left one. */
            c->at_l = c->at_r;
            c->val_l = c->val_r;
            if (arrayStreamNextVal(d, stream, &val, &next)) {
                c->at_r = val;
                c->val_r = next;
            } else {
                c->at_r = vs->to;
            }
        }
    }

    c->span = c->at_r - c->at_l;
    c->rise = c->val_r - c->val_l;

    if (moved && arrayTap() != 0)
        fprintf(arrayTap(), "at %d\t%s\tspan %d\t%d..%d\n",
                (int)c->at_l,
                (stream >= 0 && stream < FRAME_PARMS
                 && array_tap_name[stream] != 0)
                    ? array_tap_name[stream] : "?",
                (int)c->span, (int)c->val_l, (int)c->val_r);

    if (c->rise == 0 || c->span == 0)
        return c->val_l;

    /* Truncate the complete interpolated value, including its left
       endpoint. Truncating just a negative slope first rounds falling
       controls one unit upward. Keep the original single-precision
       multiply/divide order used by the ReLoq numeric scheduler. */
    float offset=(float)(at-c->at_l);
    offset*=(float)c->rise;offset/=(float)c->span;
    return (int32_t)((float)c->val_l+offset);
}

/* How far the streams the map names actually go. Nothing is generated past
   the first one that runs out. */
static int32_t definitionBoundary(delta_state *d, const int32_t *map,
                                  int32_t to)
{
    int32_t best = to;
    int32_t i;

    for (i = 0; i < FRAME_PARMS; i++) {
        int32_t stream = map[i + 1];
        int32_t last;

        if (stream == -1)
            continue;

        last = arrayStreamLastOffset(d, stream);
        if (last < best)
            best = last;
    }

    return best;
}

/* The language is done with its arrays. */
void dlangCleanup(delta_state *d)
{
    if (GEN_SET(d)) {
        valueSetDelete(GEN_SET(d));
        GEN_SET(d) = 0;
    }
}

/* Walk the window a step at a time, building a frame at each step and
   handing it to the synthesiser. Answers false if the synthesiser stopped,
   the caller interrupted, or the window ran out. */
int32_t sendArrayParameters(delta_state *d, int32_t from, int32_t to,
                            int32_t bounded, int32_t continuing,
                            int32_t exact, int32_t unused,
                            int32_t step, const int32_t *map,
                            const int32_t *defaults)
{
    int32_t frame[FRAME_PARMS];
    int32_t stopped = 0;
    int32_t at;
    int32_t i;

    (void)unused;

    if (from == 0)
        GEN_STARTED(d) = 0;
    GEN_LIMIT(d) = to;

    for (i = 0; i < FRAME_PARMS; i++)
        frame[i] = defaults[i];
    frame[0] = step;

    if (arrayTap() != 0) {
        fprintf(arrayTap(), "run from %d to %d step %d\n",
                (int)from, (int)to, (int)step);
        for (i = 0; i < FRAME_PARMS; i++) {
            if (map[i + 1] == -1)
                continue;
            if (map[i + 1] >= 0 && map[i + 1] < FRAME_PARMS && i < 62)
                array_tap_name[map[i + 1]] = reloq61_parameter_names[i];
            fprintf(arrayTap(), "map %s\tstream %d\tdefault %d\n",
                    i < 62 ? reloq61_parameter_names[i] : "?",
                    (int)map[i + 1], (int)defaults[i]);
        }
    }

    if (bounded) {
        if (!continuing)
            from = GEN_AT(d);
        if (!exact) {
            to = definitionBoundary(d, map, to);
            if (to < from)
                to = from;
            /* End on a whole step. */
            to -= (to - from) % step;
        }
        GEN_AT(d) = to;
    }
    GEN_END(d) = to;

    if (GEN_SET(d) == 0) {
        GEN_SET(d) = valueSetNew(d, from, to);
        if (GEN_SET(d) == 0)
            return 0;
    } else if (continuing || !bounded) {
        valueSetReset(d, GEN_SET(d), from, to);
    } else {
        GEN_SET(d)->to = to;
    }

    if(GEN(d)->reloq_frame) {
        extern void reloq61_clock_begin(void *,int32_t);
        reloq61_clock_begin(GEN(d)->reloq_context,from);
    }
    /* ReLoq plans sparse intervals, then evaluates their exact integer
       controls while producing PCM. Walking every millisecond here first
       duplicated the interpolation and delayed the first audio callback. */
    if (GEN(d)->reloq_frame && step==1) {
        extern int reloq61_wants_spans(void *);
        extern int reloq61_array_span(void *,int32_t,int32_t,
            const struct reloq61_curve [62],int32_t);
        if (reloq61_wants_spans(GEN(d)->reloq_context)) {
            for(at=from;at<to;) {
                if(checkInterrupt(d))return 0;
                int32_t end=to;
                struct reloq61_curve curves[FRAME_PARMS];
                for(i=0;i<FRAME_PARMS;++i) {
                    const int32_t stream=map[i+1];
                    curves[i]=(struct reloq61_curve){at,defaults[i],to,defaults[i]};
                    if(stream==-1)continue;
                    (void)valueSetValue(d,GEN_SET(d),stream,at);
                    const Cursor *c=GEN_SET(d)->cursors+stream;
                    curves[i]=(struct reloq61_curve){c->at_l,c->val_l,c->at_r,c->val_r};
                    /* The old cursor includes its right endpoint; it
                       advances on the following millisecond. */
                    if(c->at_r<end-1)end=c->at_r+1;
                }
                if(end<=at || !reloq61_array_span(GEN(d)->reloq_context,
                    at,end,curves,GEN(d)->reloq_volume))return 0;
                at=end;
            }
            return 1;
        }
    }

    for (at = from; !stopped; at += step) {
        if (checkInterrupt(d))
            break;
        if (!valueSetInRange(GEN_SET(d), at))
            break;

        if (arrayTap() != 0)
            fprintf(arrayTap(), "frame %d\n", (int)at);

        for (i = 0; i < FRAME_PARMS; i++) {
            if (map[i + 1] == -1)
                continue;
            frame[i] = valueSetValue(d, GEN_SET(d), map[i + 1], at);
        }

        /* The last frame is short, so the run stops where it was asked to. */
        if (at + step > to)
            frame[0] = at + step - to;

        if (!klattSynthFrame(d, frame))
            stopped = 1;
    }

    return stopped == 0;
}

/* Transfer the host's F0 targets before the donor's other acoustic streams
 * replace the current bank window. No waveform or utterance result is kept. */
static void bank_pitch_point(struct reloq61_english_bank *p,int32_t at,int32_t value)
{
 if(value<=0||p->pitch_count>=4096)return;
 if(p->pitch_count){struct reloq61_bank_pitch *last=p->pitch+p->pitch_count-1;
  if(at<last->at)return;if(at==last->at){last->value=value;return;}}
 p->pitch[p->pitch_count++]=(struct reloq61_bank_pitch){at,value};
}
int reloq61_english_bank_pitch(void *state,struct reloq61_english_bank *p,int32_t from,int32_t to)
{
 delta_state *d=state;int32_t stream=arrayStreamFind(d,"F0"),at,value;
 if(stream<0||to<=from)return -1;
 p->pitch=malloc(4096*sizeof*p->pitch);if(!p->pitch)return -1;p->pitch_count=0;
 ValueSet *vs=GEN_SET(d);
 if(vs){Cursor *c=vs->cursors+stream;if(c->at_r!=-1){bank_pitch_point(p,c->at_l-from,c->val_l);bank_pitch_point(p,c->at_r-from,c->val_r);}}
 while(arrayStreamNextVal(d,stream,&at,&value))bank_pitch_point(p,at-from,value);
 if(!p->pitch_count)return -1;
 size_t first=0;while(first+1<p->pitch_count&&p->pitch[first+1].at<=0)++first;
 if(first){memmove(p->pitch,p->pitch+first,(p->pitch_count-first)*sizeof*p->pitch);p->pitch_count-=first;}
 if(p->pitch[0].at<0){
  int32_t v=p->pitch[0].value;
  if(p->pitch_count>1){float f=(float)(-p->pitch[0].at)/(float)(p->pitch[1].at-p->pitch[0].at);v=(int32_t)((float)v+f*(float)(p->pitch[1].value-v));}
  p->pitch[0]=(struct reloq61_bank_pitch){0,v};
 }
 return 0;
}
