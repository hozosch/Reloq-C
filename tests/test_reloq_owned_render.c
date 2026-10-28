#include "../src/reloq_core_internal.h"
#include "reloq_session.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/* The release comparison also compiles this test against the frozen
 * previous renderer. The ordinary component test uses the transactional
 * public interface as the independent state-management path. */
#ifndef RELOQ_RENDER_REFERENCE
#define RELOQ_RENDER_REFERENCE reloq_core_voiced_frame_render
#endif
int RELOQ_RENDER_REFERENCE(struct reloq_core_voiced_frame_state *,
    const struct reloq_core_voiced_frame *, float *, float *, float *,
    uint8_t *, int16_t *, size_t, size_t *);

enum { CAPACITY = 1400 };
struct samples {
    float output[CAPACITY], af[CAPACITY], scratch[CAPACITY];
    uint8_t mask[CAPACITY];
    int16_t pcm[CAPACITY];
};

static struct reloq_core_voiced_frame frame_for(unsigned n)
{
    struct reloq_core_voiced_frame f = {0};
    static const float lengths[] = {1, 2.5f, 4.1875f, 20.125f, 25};
    f.noise.duration_ms = lengths[n % 5];
    f.pitch_tenths = 1000 + (float)(n % 5) * 60;
    f.open_percent = 45 + (float)(n % 4);
    f.av_level = n % 16 < 9 ? 38 : 0;
    f.flutter = n % 3;
    f.diplophonia = n % 7;
    f.glottal_profile = n % 36;
    f.noise.ah_level = n % 13 < 7 ? 18 : 0;
    f.noise.af_level = n % 11 < 5 ? 24 : 0;
    f.noise.bypass_level = 25;
    f.noise.s_frication = (int)(n % 3);
    f.noise.bypass_attenuation_db = (float)(n % 3);
    f.noise.tract.highest_formant = 5;
    f.noise.tract.nasal_pole = (struct reloq_core_formant){260,75};
    f.noise.tract.nasal_zero = (struct reloq_core_formant){360,90};
    f.noise.tract.tracheal_pole = (struct reloq_core_formant){2100,140};
    f.noise.tract.tracheal_zero = (struct reloq_core_formant){2000,150};
    for (unsigned j = 0; j < 8; ++j) {
        f.noise.tract.oral[j] = (struct reloq_core_formant){
            400 + 420 * (float)j + (float)(n % 4) * 10, 80 + 12 * (float)j};
        f.noise.af_bandwidths[j] = 110 + 15 * (float)j;
        f.noise.af_levels[j] = 17 - (long)j;
    }
    return f;
}

static void initialize(struct reloq_core_voiced_frame_state *s,
                       float rate, unsigned profile)
{
    assert(!reloq_core_voiced_frame_initialize(s,rate,123,0,0,0));
    s->glottal_reference_rate = profile ? 11025 : 0;
    s->frication_reference_rate = profile == 2 ? 16000 :
                                  profile == 1 ? 11025 : 0;
    s->reference_formants = profile == 2 ? 4 : 5;
}

static int render(int owned, struct reloq_core_voiced_frame_state *s,
                  const struct reloq_core_voiced_frame *f,
                  struct samples *b, int pcm, size_t capacity, size_t *written)
{
    if (owned)
        return reloq_core_voiced_frame_render_owned(s,f,b->output,b->af,
            b->scratch,b->mask,pcm?b->pcm:NULL,capacity,written);
    return RELOQ_RENDER_REFERENCE(s,f,b->output,b->af,b->scratch,b->mask,
                                 pcm?b->pcm:NULL,capacity,written);
}

struct capture { size_t calls, samples; int16_t first[48]; };
static int receive(void *context, const int16_t *pcm, size_t count)
{
    struct capture *c = context;
    if (!c->calls) {
        assert(count <= 48);
        memcpy(c->first,pcm,count*sizeof *pcm);
    }
    ++c->calls; c->samples += count;
    return 0;
}

int main(void)
{
    static const float rates[] = {8000,11025,16000,22050,32000,44100,48000};
    unsigned cases = 0, rejected = 0;
    for (unsigned r = 0; r < 7; ++r) for (unsigned profile = 0; profile < 3; ++profile) {
        struct reloq_core_voiced_frame_state old, owned;
        initialize(&old,rates[r],profile); owned = old;
        for (unsigned n = 0; n < 64; ++n) {
            struct reloq_core_voiced_frame f = frame_for(n);
            struct samples a, b;
            memset(&a,0x5a,sizeof a); b = a;
            size_t na = 0, nb = 0;
            int sa = render(0,&old,&f,&a,n%2,CAPACITY,&na);
            int sb = render(1,&owned,&f,&b,n%2,CAPACITY,&nb);
            assert(sa == 0 && sb == 0 && na == nb);
            assert(!memcmp(&a,&b,sizeof a));
            assert(!memcmp(&old,&owned,sizeof old));
            ++cases;
        }
        /* Rejections before preparation, inside filter preparation and
         * during pulse generation must preserve the public state. The
         * owned state is reset, including both FIR histories, afterwards. */
        for (unsigned fault = 0; fault < 8; ++fault) {
            struct reloq_core_voiced_frame f = frame_for(3);
            struct reloq_core_voiced_frame_state before = old, bad = old;
            struct samples a, b;
            memset(&a,0,sizeof a); b = a;
            size_t na = 777, nb = 777, capacity = CAPACITY;
            if (fault == 0) f.glottal_profile = 36;
            if (fault == 1) f.noise.duration_ms = NAN;
            if (fault == 2) capacity = 1;
            if (fault == 3) f.noise.tract.highest_formant = 9;
            if (fault == 4) f.noise.af_bandwidths[0] = NAN;
            if (fault == 5) f.open_percent = 101;
            if (fault == 6) f.pitch_tenths = NAN;
            if (fault == 7) {
                old.voice.period_live = 1;
                old.voice.open_left = -1;
                bad = old; before = old;
            }
            int sa = render(0,&old,&f,&a,1,capacity,&na);
            int sb = render(1,&bad,&f,&b,1,capacity,&nb);
            if(sa != -1 || sb != -1)
                fprintf(stderr,"rejection r=%u profile=%u fault=%u status=%d/%d\n",r,profile,fault,sa,sb);
            assert(sa == -1 && sb == -1);
            assert(na == 777 && nb == 777 && !memcmp(&before,&old,sizeof old));
            initialize(&bad,rates[r],profile);
            struct reloq_core_voiced_frame_state fresh;
            initialize(&fresh,rates[r],profile);
            assert(!memcmp(&bad,&fresh,sizeof bad));
            old = before;
            ++rejected;
        }
    }
    /* Session failures deliver no partial frame. Its documented reset
     * restores the first accepted PCM exactly after a late rejection. */
    for (unsigned r = 0; r < 7; ++r) {
        struct reloq_session_config c = {.sample_rate=rates[r],
            .duration_scale=1,.output_gain=1,.initial_seed=123,.max_frame_samples=CAPACITY};
        struct reloq_session *s = reloq_session_create(&c); assert(s);
        struct reloq_core_voiced_frame f = frame_for(0);
        struct capture a = {0}, b = {0}; size_t completed = 99;
        assert(reloq_session_render_frames(s,&f,1,receive,&a,&completed)==RELOQ_SESSION_OK);
        assert(completed==1 && a.calls==1);
        f.noise.tract.highest_formant=9;
        assert(reloq_session_render_frames(s,&f,1,receive,&a,&completed)==RELOQ_SESSION_RENDER);
        assert(!completed && a.calls==1);
        assert(reloq_session_reset(s)==RELOQ_SESSION_OK);
        f=frame_for(0);
        assert(reloq_session_render_frames(s,&f,1,receive,&b,&completed)==RELOQ_SESSION_OK);
        assert(completed==1 && !memcmp(&a,&b,sizeof a));
        reloq_session_destroy(s);
    }
    printf("Owned render: %u exact full states and sample buffers; %u atomic public rejections; 7 session error/reset recoveries\n",cases,rejected);
    return 0;
}
