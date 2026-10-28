#include "reloq_inline_tags.h"
#include "reloq_voice_controls.h"
#include "reloq_german_generic_voice.h"

#include "reloq_curve_pipeline.h"
#include "reloq_curve_scheduler.h"
#include "reloq_german_generic_melody.h"
#include "reloq_german_generic_nasal_coda.h"
#include "reloq_german_coda_timing.h"
#include "reloq_german_transition_duration.h"
#include "reloq_german_generic_diphthong.h"
#include "reloq_german_generic_coarticulation.h"
#include "reloq_german_generic_noise.h"
#include "reloq_german_nucleus_acoustics.h"
#include "reloq_german_nasal_tract.h"
#include "reloq_german_obstruent_tract.h"
#include "reloq_german_liquid_tract.h"
#include "reloq_german_nucleus_schedule.h"
#include "reloq_german_source_envelope.h"
#include "reloq_german_pause.h"
#include "reloq_german_glottal.h"
#include "reloq_german_voice_model.h"
#include "reloq_german_onset_cluster.h"
#include "reloq_german_stop_phase.h"
#include "reloq_german_generic_prosody.h"
#include "reloq_german_generic_index.h"
#include "reloq_german_annotation.h"
#include "reloq_german_generic_timing.h"
#include "reloq_german_generic_stop_connection.h"
#include "reloq_german_input.h"
#include "reloq_german_generic_tract.h"
#include "reloq_german_generic_transition.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_german_phone_sequence.h"
#include "reloq_german_text.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_voice_controls.h"
#include "reloq_target_stage_router.h"
#include "reloq_german_generic_point_budget.h"
#include "reloq_german_affricate.h"
#include "reloq_german_terminal_stop_tract.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef RELOQ_GENERIC_PHONE_TRACE
#include <stdio.h>
#endif

/* These compact classes provide provisional durations and unsupported
 * nucleus types. The separate nucleus-acoustics module owns verified
 * simple-vowel/resonance properties; consonant trajectories, timing,
 * source envelopes and prosody still contain independent approximations.
 * Units are the native 1-ms target ticks and the existing Klatt controls. */
struct vowel_shape {
    enum reloq_german_phone phone;
    int32_t f1, f2, f3;
    int32_t duration, av;
};

static const struct vowel_shape vowel_shapes[] = {
    {RELOQ_DE_PHONE_A, 700, 1160, 2500, 100, 51},
    {RELOQ_DE_PHONE_E, 550, 1800, 2600, 104, 52},
    {RELOQ_DE_PHONE_I, 380, 2000, 2800, 100, 53},
    {RELOQ_DE_PHONE_O, 570, 900, 2500, 102, 53},
    {RELOQ_DE_PHONE_U, 420, 930, 2400, 98, 54},
    {RELOQ_DE_PHONE_a, 800, 1220, 2500, 180, 50},
    {RELOQ_DE_PHONE_e, 370, 2150, 2700, 170, 51},
    {RELOQ_DE_PHONE_i, 265, 2150, 2900, 165, 53},
    {RELOQ_DE_PHONE_o, 390, 735, 2400, 172, 54},
    {RELOQ_DE_PHONE_u, 250, 750, 2400, 165, 55},
    {RELOQ_DE_PHONE_y, 300, 1850, 2700, 165, 52},
    {RELOQ_DE_PHONE_Y, 410, 1700, 2600, 96, 52},
    {RELOQ_DE_PHONE_AT, 490, 1500, 2500, 75, 49},
    {RELOQ_DE_PHONE_R, 600, 1360, 2280, 100, 48},
    {RELOQ_DE_PHONE_Q_E_COLON, 500, 2005, 2600, 144, 51},
    {RELOQ_DE_PHONE_Q_oe, 380, 1450, 2500, 170, 52},
    {RELOQ_DE_PHONE_Q_OE, 530, 1250, 2500, 98, 51},
    {RELOQ_DE_PHONE_Q_aj, 820, 1220, 2500, 180, 50},
    {RELOQ_DE_PHONE_Q_aw, 780, 1160, 2500, 180, 50},
    {RELOQ_DE_PHONE_Q_oj, 550, 900, 2500, 175, 53},
    {RELOQ_DE_PHONE_Q_a_TILDE, 730, 1150, 2450, 155, 49},
    {RELOQ_DE_PHONE_Q_E_TILDE, 500, 1680, 2550, 155, 50},
    {RELOQ_DE_PHONE_Q_o_TILDE, 460, 850, 2400, 155, 51},
    {RELOQ_DE_PHONE_Q_oe_TILDE, 440, 1400, 2500, 155, 50}
};

struct profile { int32_t value[RELOQ_TARGET_STREAM_COUNT]; };

static void get_formants(const struct profile *p, int32_t f[3])
{
    f[0] = p->value[RELOQ_TARGET_F1];
    f[1] = p->value[RELOQ_TARGET_F2];
    f[2] = p->value[RELOQ_TARGET_F3];
}

static void set_formants(struct profile *p, const int32_t f[3])
{
    p->value[RELOQ_TARGET_F1] = f[0];
    p->value[RELOQ_TARGET_F2] = f[1];
    p->value[RELOQ_TARGET_F3] = f[2];
}

struct source_point {
    int64_t tick; int32_t value;
    enum reloq_target_stream stream; size_t phone, serial;
};
struct source_points {struct source_point *points; size_t count,capacity;};

struct inline_timing {
    int64_t *starts, *mapped;
    double *scales;
    size_t count;
};
static int inline_time_value(int64_t tick,const int64_t *starts,const int64_t *mapped,
    const double *scale,size_t count,int64_t *out);

struct output_settings {
    int rate_factor; /* Percent of the default target time axis. */
    int pitch_factor; /* Two hundredths of the default F0. */
    double native_time_scale;
    double native_pitch_scale;
    int volume_attenuation; /* Rounded dB applied to every source. */
    int mute;
    int nucleus_tract; /* Nucleus owns formants, bandwidths and nasal resonance. */
    int nucleus_source; /* Independent AV/AH points own this nucleus. */
    int cluster_source;
    int noise_owned; /* An explicit source handoff owns the noise tracks. */
    struct source_points *sources;
    size_t source_phone;
    int pause_tract; /* Pause contributes no F1/F2/F3 targets. */
    int tract_owned; /* Independent sparse F1-F5 and B1-B5 events. */
    struct reloq_german_voice_model model;
    const struct reloq_german_phone_node *phones;
    size_t phone_count;
    size_t model_owner_plus_one;
    const struct reloq_german_source_segment *model_segments;
    const struct inline_timing *timing;
    struct source_points *colour,*sibilants;
    const struct reloq_inline_word *models;
    size_t model_count;
};

static int output_settings_init(
    struct output_settings *out,
    const struct reloq_german_generic_settings *requested)
{
    const struct reloq_german_generic_settings defaults = {50, 50, 100, 0, 0, 0,0,0,0,0,0};
    const struct reloq_german_generic_settings *s = requested ?
        requested : &defaults;
    if (s->rate < 0 || s->rate > 100 ||
        s->pitch < 0 || s->pitch > 100 ||
        s->volume < 0 || s->volume > 100 ||
        (s->native_speed &&
         (s->native_speed < 40 || s->native_speed > 250)) ||
        s->native_pitch_plus_one < 0 || s->native_pitch_plus_one > 101 ||
        s->native_inflection_plus_one < 0 ||
        s->native_inflection_plus_one > 101 || s->pause_mode<0 || s->pause_mode>2 ||
        s->head_size_plus_one<0 || s->head_size_plus_one>101 ||
        s->roughness<0 || s->roughness>100 || s->breathiness<0 || s->breathiness>100 ||
        s->gender<0 || s->gender>1)
        return -1;
    *out = (struct output_settings){
        150 - s->rate, 150 + s->pitch,
        s->native_speed ?
            reloq_voice_speed_time_scale(s->native_speed) : 0.0,
        s->native_pitch_plus_one ?
            reloq_voice_pitch_f0_scale(s->native_pitch_plus_one - 1) : 0.0,
        s->volume ? (int)lround(20.0 * log10(100.0 / s->volume)) : 0,
        s->volume == 0, 0, 0, 0, 0, NULL, 0, 0, 0,
        {s->head_size_plus_one?s->head_size_plus_one-1:50,s->roughness,s->breathiness,s->gender,11025},
        NULL,0,0,NULL,NULL,NULL,NULL,NULL,0
    };
    return 0;
}

static struct reloq_german_voice_model inline_model(
    const struct reloq_inline_word *word,float rate)
{
    return (struct reloq_german_voice_model){word->head_size,word->roughness,
        word->breathiness,word->gender,rate};
}

/* Voice anatomy acts on sparse targets before interpolation. Keeping the
 * owning phone here preserves nasal and consonant resonance decisions. */
static const struct vowel_shape *vowel(enum reloq_german_phone phone);
static int colour_point(struct source_points *out,int64_t tick,int value,size_t owner)
{
    if(!out)return 0;
    if(out->count==out->capacity) {
        size_t n=out->capacity?out->capacity*2:64;
        if(n<=out->capacity || n>SIZE_MAX/sizeof *out->points)return -1;
        void *p=realloc(out->points,n*sizeof *out->points);if(!p)return -1;
        out->points=p;out->capacity=n;
    }
    out->points[out->count]=(struct source_point){tick,value,RELOQ_TARGET_AB,owner,out->count};
    ++out->count;return 0;
}
static int model_append(struct reloq_target_stage_router *router,int64_t tick,
    const struct reloq_target_event *events,size_t count,
    const struct output_settings *settings,size_t owner)
{
    if(count>RELOQ_TARGET_STREAM_COUNT)return -1;
    struct reloq_target_event mapped[RELOQ_TARGET_STREAM_COUNT+1];
    size_t used=0;
    for(size_t j=0;j<count;++j) {
        if(settings->sibilants && events[j].stream==RELOQ_TARGET_AF && events[j].value>0) {
            if((events[j].offset>0 && tick>INT64_MAX-events[j].offset) ||
               (events[j].offset<0 && tick<INT64_MIN-events[j].offset))return -1;
            enum reloq_german_phone p=owner<settings->phone_count?settings->phones[owner].phone:RELOQ_DE_PHONE_NONE;
            if(colour_point(settings->sibilants,tick+events[j].offset,
                p==RELOQ_DE_PHONE_s||p==RELOQ_DE_PHONE_z||p==RELOQ_DE_PHONE_t,owner))return -1;
        }
        if(settings->colour && events[j].stream==RELOQ_TARGET_AB && events[j].value>0) {
            enum reloq_german_phone p=owner<settings->phone_count?
                settings->phones[owner].phone:RELOQ_DE_PHONE_NONE;
            int db=p==RELOQ_DE_PHONE_f || p==RELOQ_DE_PHONE_p || p==RELOQ_DE_PHONE_b?300:0;
            if((events[j].offset>0 && tick>INT64_MAX-events[j].offset) ||
               (events[j].offset<0 && tick<INT64_MIN-events[j].offset) ||
               colour_point(settings->colour,tick+events[j].offset,db,owner))return -1;
        }
        size_t selected=owner;
        if(settings->model_owner_plus_one &&
           (events[j].stream!=RELOQ_TARGET_FNP ||
            (owner<settings->phone_count && vowel(settings->phones[owner].phone))))
            selected=settings->model_owner_plus_one-1;
        /* Anticipatory frication already belongs to the incoming phone;
         * the held vowel tract at this knot must not recolour its source. */
        if(events[j].stream>=RELOQ_TARGET_AF && events[j].stream<=RELOQ_TARGET_AB &&
           selected<settings->phone_count && owner<settings->phone_count &&
           vowel(settings->phones[selected].phone) &&
           (settings->phones[owner].phone==RELOQ_DE_PHONE_z ||
            settings->phones[owner].phone==RELOQ_DE_PHONE_s ||
            settings->phones[owner].phone==RELOQ_DE_PHONE_f ||
            settings->phones[owner].phone==RELOQ_DE_PHONE_v ||
            settings->phones[owner].phone==RELOQ_DE_PHONE_S ||
            settings->phones[owner].phone==RELOQ_DE_PHONE_Z ||
            settings->phones[owner].phone==RELOQ_DE_PHONE_x ||
            settings->phones[owner].phone==RELOQ_DE_PHONE_X))selected=owner;
        struct reloq_german_phone_node node;
        const struct reloq_german_phone_node *phone=NULL;
        if(selected<settings->phone_count) {
            node=settings->phones[selected];
            if(settings->model_segments && settings->model_segments[selected].phrase_final)
                node.right_phone=RELOQ_DE_PHONE_NONE;
            phone=&node;
        }
        struct reloq_german_voice_model model=settings->model;
        if(phone && settings->models && phone->word_index<settings->model_count)
            model=inline_model(settings->models+phone->word_index,model.sample_rate);
        if (phone && (phone->phone==RELOQ_DE_PHONE_t || phone->phone==RELOQ_DE_PHONE_d) &&
            (model.sample_rate>=16000?
             model.head_size<=(model.gender?40:30):
             (reloq_german_voice_model_formants(&model)==4 ||
              (model.sample_rate==8000 && reloq_german_voice_model_formants(&model)==3) ||
              (model.sample_rate==8000 && phone->phone==RELOQ_DE_PHONE_d)))) {
            const int small=model.gender && model.head_size<30;
            const int reduced=model.sample_rate==8000 &&
                reloq_german_voice_model_formants(&model)==3;
            const enum reloq_target_stream pole=small||reduced?RELOQ_TARGET_A3F:RELOQ_TARGET_A4F;
            /* Dental bursts move with the active filter bank. The source
             * event still decides when that burst is on or off. */
            if (events[j].stream==pole) continue;
            if (events[j].stream==RELOQ_TARGET_A5F) {
                mapped[used++]=(struct reloq_target_event){pole,events[j].offset,
                    events[j].value?(phone->phone==RELOQ_DE_PHONE_d?55:small?54:52):0};
                mapped[used]=events[j];mapped[used++].value=0;continue;
            }
        }
        mapped[used]=events[j];
        mapped[used].value=reloq_german_voice_model_target(&model,phone,
            phone && selected+1<settings->phone_count?settings->phones+selected+1:NULL,
            mapped[used].stream,mapped[used].value);
        /* Balance the German S source against the stronger Z release in
         * every position. Apply 4 dB after anatomy/source adjustments, so
         * the level change is uniform across profiles. Preserve zero
         * excitation and the complete ts affricate used as the reference. */
        if (mapped[used].stream==RELOQ_TARGET_AF && mapped[used].value>0 &&
            phone && !phone->affricate_role &&
            (phone->phone==RELOQ_DE_PHONE_s || phone->phone==RELOQ_DE_PHONE_z))
            mapped[used].value+=4;
        /* A word-medial /ts/ carries one continuous stop/release source.
         * Ease both halves by one dB after model/breath adjustments. Keep
         * initial and final /ts/, its spectral targets and all clocks. */
        if (mapped[used].stream==RELOQ_TARGET_AF && mapped[used].value>0 &&
            phone && (phone->affricate_role==3 ||
                      (phone->affricate_role==4 && selected))) {
            size_t start=selected-(phone->affricate_role==4);
            if (start+1<settings->phone_count &&
                settings->phones[start].same_word_left &&
                settings->phones[start+1].same_word_right)
                --mapped[used].value;
        }
        /* Raise the other consonants by one source dB, after anatomy and
         * breath controls. S and the two parts of /ts/ keep their own
         * balance. A vocalized rhotic still belongs to the vowel source.
         * Zero closures remain zero, and no envelope knots are added. */
        if (phone && mapped[used].value>0 &&
            (mapped[used].stream==RELOQ_TARGET_AV ||
             mapped[used].stream==RELOQ_TARGET_AH ||
             mapped[used].stream==RELOQ_TARGET_AF) &&
            phone->phone!=RELOQ_DE_PHONE_NONE && !vowel(phone->phone) &&
            phone->phone!=RELOQ_DE_PHONE_s && phone->phone!=RELOQ_DE_PHONE_z &&
            phone->affricate_role!=3 && phone->affricate_role!=4 &&
            !(phone->phone==RELOQ_DE_PHONE_r && phone->same_syllable_left &&
              vowel(phone->left_phone)))++mapped[used].value;
        if(phone && (phone->phone==RELOQ_DE_PHONE_f ||
                     phone->phone==RELOQ_DE_PHONE_v) &&
           mapped[used].stream==RELOQ_TARGET_AF && mapped[used].value>0)
            mapped[used].value+=reloq_voice_labiodental_gain(settings->model.sample_rate);
        ++used;
    }
    return reloq_target_stage_router_append(router,tick,mapped,used);
}

static int scale_tick(int64_t tick, const struct output_settings *settings,
                      int64_t *result)
{
    if(settings->timing) {
        const struct inline_timing *m=settings->timing;
        return inline_time_value(tick,m->starts,m->mapped,m->scales,m->count,result);
    }
    if (settings->native_time_scale > 0.0) {
        double scaled = (double)tick * settings->native_time_scale;
        if (tick < 0 || !isfinite(scaled) ||
            scaled >= (double)INT64_MAX - 1.0) return -1;
        *result = (int64_t)llround(scaled);
        return 0;
    }
    if (tick < 0 || tick > (INT64_MAX - 50) / settings->rate_factor)
        return -1;
    *result = (tick * settings->rate_factor + 50) / 100;
    return 0;
}

/* The opt-in model's source gains need phrase-level calibration. A build-time
 * trim lets development probes compare one shared source scaling across
 * unrelated utterances, rather than fitting individual words. Four units
 * were recalibrated after the tract and consonant controls gained their
 * own verified source routes. Two units have lower active-RMS error on
 * the original four held-out development shapes than zero, four or more
 * units of trim (see GENERIC-VOICE-DE.md). */
#ifndef RELOQ_GENERIC_SOURCE_TRIM
#define RELOQ_GENERIC_SOURCE_TRIM 2
#endif

static void silence(struct profile *p)
{
    static const int32_t initial[RELOQ_TARGET_STREAM_COUNT] = {
        [RELOQ_TARGET_F0] = 1100,
        [RELOQ_TARGET_F1] = 500,
        [RELOQ_TARGET_F2] = 1500,
        [RELOQ_TARGET_F3] = 2500,
        [RELOQ_TARGET_F4] = 3600,
        [RELOQ_TARGET_F5] = 3900,
        [RELOQ_TARGET_B1] = 120,
        [RELOQ_TARGET_B2] = 100,
        [RELOQ_TARGET_B3] = 150,
        [RELOQ_TARGET_B4] = 330,
        [RELOQ_TARGET_B5] = 260,
        [RELOQ_TARGET_FNP] = 200,
        [RELOQ_TARGET_FNZ] = 200,
        [RELOQ_TARGET_FTP] = 250,
        [RELOQ_TARGET_FTZ] = 250,
        [RELOQ_TARGET_OQ] = 56
    };
    memcpy(p->value, initial, sizeof initial);
}

static const struct vowel_shape *vowel(enum reloq_german_phone phone)
{
    for (size_t i = 0; i < sizeof vowel_shapes / sizeof vowel_shapes[0]; ++i)
        if (phone == vowel_shapes[i].phone) return vowel_shapes + i;
    return NULL;
}

static int is_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k || phone == RELOQ_DE_PHONE_b ||
           phone == RELOQ_DE_PHONE_d || phone == RELOQ_DE_PHONE_g ||
           phone == RELOQ_DE_PHONE_P || phone == RELOQ_DE_PHONE_T ||
           phone == RELOQ_DE_PHONE_C || phone == RELOQ_DE_PHONE_J;
}

static int syllabic_sonorant(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_R || phone == RELOQ_DE_PHONE_r ||
           phone == RELOQ_DE_PHONE_l || phone == RELOQ_DE_PHONE_m ||
           phone == RELOQ_DE_PHONE_n || phone == RELOQ_DE_PHONE_G;
}

static int voiced_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_b || phone == RELOQ_DE_PHONE_d ||
           phone == RELOQ_DE_PHONE_g || phone == RELOQ_DE_PHONE_J;
}

static int32_t pitch(const struct reloq_german_phone_node *node)
{
    int32_t result = node->stress == 1 ? 1230 :
                     node->stress == 2 ? 1120 : 1030;
    if (node->syllable_in_word > 0 && result > 900) result -= 45;
    return result;
}

static int32_t vowel_duration(const struct vowel_shape *shape,
                              const struct reloq_german_phone_node *node,
                              int before_plain_cv)
{
    int32_t duration = shape->duration;
    int32_t contextual = 0;
    if (shape->phone == RELOQ_DE_PHONE_Q_E_COLON)
        return node->same_syllable_right &&
               (node->right_phone == RELOQ_DE_PHONE_n ||
                node->right_phone == RELOQ_DE_PHONE_m ||
                node->right_phone == RELOQ_DE_PHONE_s ||
                node->right_phone == RELOQ_DE_PHONE_f) &&
               node->stress == 1 ? 158 : 144;
    if (before_plain_cv &&
        reloq_german_generic_pre_stop_vowel_ticks(node, &contextual) == 1)
        return contextual;
    if (node->stress == 0) duration = duration * 65 / 100;
    else if (node->stress == 2) duration = duration * 85 / 100;
    if (node->same_syllable_right && is_stop(node->right_phone))
        duration = duration * 80 / 100;
    return duration < 48 ? 48 : duration;
}

static int32_t duration(const struct reloq_german_phone_node *node,
                        const struct vowel_shape *shape, int before_plain_cv,
                        int phrase_final, int following_onset_cluster)
{
    if (shape) return vowel_duration(shape, node, before_plain_cv);
    int32_t contextual = 0;
    if (reloq_german_generic_fricative_ticks(
            node, phrase_final, following_onset_cluster,
            &contextual) == 1) return contextual;
    switch (node->phone) {
    case RELOQ_DE_PHONE_p: case RELOQ_DE_PHONE_t:
    case RELOQ_DE_PHONE_k: return 76;
    case RELOQ_DE_PHONE_b: case RELOQ_DE_PHONE_d:
    case RELOQ_DE_PHONE_g: return 58;
    case RELOQ_DE_PHONE_P: case RELOQ_DE_PHONE_T:
    case RELOQ_DE_PHONE_C: case RELOQ_DE_PHONE_J: return 112;
    case RELOQ_DE_PHONE_s: case RELOQ_DE_PHONE_S:
    case RELOQ_DE_PHONE_x: case RELOQ_DE_PHONE_X: return 108;
    case RELOQ_DE_PHONE_f: case RELOQ_DE_PHONE_h:
    case RELOQ_DE_PHONE_z: case RELOQ_DE_PHONE_Z:
    case RELOQ_DE_PHONE_v: case RELOQ_DE_PHONE_w: return 88;
    case RELOQ_DE_PHONE_m: case RELOQ_DE_PHONE_n:
    case RELOQ_DE_PHONE_G: return 68;
    default: return 60;
    }
}

static int nucleus_plan(const struct reloq_german_phone_node *node,
                         int weak_final,
                         struct reloq_german_nucleus_acoustics *out)
{
    /* Postvocalic R can be part of a multi-phone nucleus in the reference.
     * Its topology needs the separate nucleus analysis; do not pretend that
     * a single-phone gesture covers that case. */
    if (node->phone == RELOQ_DE_PHONE_R && node->same_syllable_left &&
        vowel(node->left_phone)) return 0;
    /* The open e nucleus does not take a following onset-r's coda
     * colour (he.ri, fe.rj). The boundary still owns its ordinary
     * duration and interpolation. */
    const int onset_r_after_e=node->phone==RELOQ_DE_PHONE_e &&
        node->right_phone==RELOQ_DE_PHONE_r && !node->same_syllable_right;
    int status = reloq_german_nucleus_acoustics_plan(
        node->phone,
        node->same_word_left ? node->left_phone : RELOQ_DE_PHONE_NONE,
        node->same_word_right && !onset_r_after_e && !(node->right_phone==RELOQ_DE_PHONE_j &&
            !node->same_syllable_right && (node->phone==RELOQ_DE_PHONE_A ||
            node->phone==RELOQ_DE_PHONE_o)) ? node->right_phone : RELOQ_DE_PHONE_NONE,
        node->stress, weak_final, out);
    /* An onset r across a syllable boundary does not close the short-i
     * nucleus. Its anticipatory lowering is a 350-Hz plateau with a
     * separate F2 departure, rather than the 280-Hz vocalic-r centre. */
    if(status==1 && node->phone==RELOQ_DE_PHONE_I &&
       node->same_word_right && node->right_phone==RELOQ_DE_PHONE_r &&
       !node->same_syllable_right) {
        out->entering[0]=350;
        out->departing[1]=1850;out->end_mask|=2u;
        ++out->av;++out->av_end;
    }
    if (status==1 && onset_r_after_e && node->same_word_left &&
        node->left_phone==RELOQ_DE_PHONE_i) {
        /* Explicit user preference for i.e.r: retain the colour of our
         * former he.r context, not the original's F1=500 after i.
         * The ordinary he.r context above now follows the original. */
        out->entering[0]=400;out->entering[1]=2050;
        out->departing[1]=2050;out->end_mask|=2u;
    }
    /* Reduced er anticipates the next oral gesture across a lexical edge.
     * Nasal bandwidth and source changes still belong to the word itself;
     * sharing those as well would incorrectly nasalize the preceding er.
     * A diphthong supplies its closing glide as the left context. */
    if (status == 1 && node->phone == RELOQ_DE_PHONE_R &&
        !node->same_word_right && node->right_phone != RELOQ_DE_PHONE_NONE) {
        enum reloq_german_phone left=node->same_word_left?
            node->left_phone:RELOQ_DE_PHONE_NONE;
        if (left==RELOQ_DE_PHONE_Q_aj || left==RELOQ_DE_PHONE_Q_oj)
            left=RELOQ_DE_PHONE_j;
        else if (left==RELOQ_DE_PHONE_Q_aw) left=RELOQ_DE_PHONE_w;
        const enum reloq_german_phone right=node->right_phone==RELOQ_DE_PHONE_v?
            RELOQ_DE_PHONE_w:node->right_phone==RELOQ_DE_PHONE_n?
            RELOQ_DE_PHONE_NONE:node->right_phone;
        struct reloq_german_nucleus_acoustics contact;
        if (reloq_german_nucleus_acoustics_plan(node->phone,left,
                right,node->stress,weak_final,&contact)!=1) return -1;
        memcpy(out->entering,contact.entering,sizeof out->entering);
        memcpy(out->departing,contact.departing,sizeof out->departing);
        out->end_mask=contact.end_mask;
        if (node->right_phone==RELOQ_DE_PHONE_h || node->right_phone==RELOQ_DE_PHONE_n)
            out->end_mask &= (uint8_t)~2u;
    }
    /* A dental coda does not raise the following short-u nucleus as
     * a dental onset does. The stop still owns its oral departure. */
    if (status==1 && node->phone==RELOQ_DE_PHONE_U &&
        node->left_phone==RELOQ_DE_PHONE_t && node->same_word_left &&
        !node->same_syllable_left) out->entering[1]=930;
    return status;
}

/* Aspiration uses the same contextual nucleus amplitude as voicing.
 * The original h pass subtracts 9 at a fresh phrase onset and 6 otherwise;
 * an internal h reads the preceding nucleus, including vocalic r/l carry. */
static int32_t h_aspiration(const struct reloq_german_phone_node *phones,
    const struct reloq_german_source_segment *sources,size_t i,int fresh,int final)
{
    const struct reloq_german_phone_node *v=phones+i+1;
    struct reloq_german_nucleus_acoustics n;
    int av=v->phone==RELOQ_DE_PHONE_Q_oj?53:50;
    if (nucleus_plan(v,!v->stress && final,&n)==1) av=n.av;
    else if (!v->stress && final) av-=3;
    /* k+h delegates the whole breath interval to h; its level is derived
     * from the following nucleus, including in a medial onset. */
    if (i && phones[i-1].phone==RELOQ_DE_PHONE_k && phones[i-1].same_syllable_right)
        return av-((i==1 || sources[i-2].phrase_final)?9:6);
    if (!fresh && phones[i].same_word_left)
        for (size_t k=i;k && phones[k].same_word_left;--k)
            if (sources[k-1].kind==RELOQ_SOURCE_NUCLEUS) {
                av=sources[k-1].av;break;
            }
    return av-(fresh?9:6);
}

static void nucleus_profile(struct profile *p,
                             const struct reloq_german_nucleus_acoustics *n)
{
    for (unsigned i = 0; i < 5; ++i) {
        p->value[RELOQ_TARGET_F1 + i] = n->entering[i];
        p->value[RELOQ_TARGET_B1 + i] = n->bandwidth[i];
    }
    /* Unsupported connected nuclei retain the provisional envelope.
     * The independent source pass uses both AV endpoints where its
     * beginning and end boundaries are established. */
    p->value[RELOQ_TARGET_FNP] = n->fnp;
    p->value[RELOQ_TARGET_FNZ] = n->fnz;
    p->value[RELOQ_TARGET_AV] = n->av;
    p->value[RELOQ_TARGET_AH] = 34;
}

static void vowel_profile(struct profile *p, const struct vowel_shape *shape,
                          const struct reloq_german_phone_node *node)
{
    struct reloq_german_generic_tract tract;
    silence(p);
    p->value[RELOQ_TARGET_F0] = pitch(node);
    (void)reloq_german_generic_vowel_tract(
        node, shape->f1, shape->f2, shape->f3, shape->av, &tract);
    p->value[RELOQ_TARGET_F1] = tract.f1;
    p->value[RELOQ_TARGET_F2] = tract.f2;
    p->value[RELOQ_TARGET_F3] = tract.f3;
    p->value[RELOQ_TARGET_B1] = tract.b1;
    p->value[RELOQ_TARGET_B2] = tract.b2;
    p->value[RELOQ_TARGET_FNP] = tract.fnp;
    p->value[RELOQ_TARGET_FNZ] = tract.fnz;
    p->value[RELOQ_TARGET_AV] = tract.av;
    p->value[RELOQ_TARGET_AH] = tract.ah;
    (void)reloq_german_generic_prenasal_bandwidth(node, p->value);
    struct reloq_german_nucleus_acoustics nucleus;
    if (nucleus_plan(node, 0, &nucleus) == 1)
        nucleus_profile(p, &nucleus);
}

static void nasal_profile(struct profile *p,const struct reloq_german_nasal_tract *n)
{
    for (unsigned j=0;j<5;++j) {
        p->value[RELOQ_TARGET_F1+j]=n->entering[j];
        p->value[RELOQ_TARGET_B1+j]=n->bandwidth[j];
    }
}

static int nasal_tract_plan(const struct reloq_german_phone_node *phones,
    size_t count,size_t index,const struct reloq_german_phone_node *node,
    int initial,int final,struct reloq_german_nasal_tract *out)
{
    struct reloq_german_phone_node acoustic=*node;
    if ((node->phone==RELOQ_DE_PHONE_m || node->phone==RELOQ_DE_PHONE_G) &&
        index && node->same_word_left &&
        phones[index-1].phone==RELOQ_DE_PHONE_r)
        acoustic.left_phone=RELOQ_DE_PHONE_r;
    /* h is a separate interval here, but an onset property of the next
     * vowel in the reference. It must not hide that vowel's nasal target. */
    if (acoustic.right_phone==RELOQ_DE_PHONE_h && index+2<count &&
        phones[index+1].same_syllable_right && vowel(phones[index+2].phone))
        acoustic.right_phone=phones[index+2].phone;
    return reloq_german_nasal_tract_plan(&acoustic,initial,final,out);
}

static void obstruent_profile(struct profile *p,const struct reloq_german_obstruent_tract *t)
{
    for (unsigned j=0;j<5;++j) {
        if (t->entering[j]>0) p->value[RELOQ_TARGET_F1+j]=t->entering[j];
        p->value[RELOQ_TARGET_B1+j]=t->bandwidth[j];
    }
}

static int consonant_tract_plan(const struct reloq_german_phone_node *node,
    int initial,int final,enum reloq_german_phone after_affricate,
    struct reloq_german_obstruent_tract *out)
{
    int result=reloq_german_affricate_tract(node,initial,final,after_affricate,out);
    if (result) return result;
    if (final) {
        result=reloq_german_terminal_stop_tract_plan(node,out);
        if (result) return result;
    }
    result=reloq_german_obstruent_tract_plan(node,initial,final,out);
    return result?result:reloq_german_liquid_tract_plan(node,initial,final,out);
}

static void consonant_profile(struct profile *p,
                              const struct reloq_german_phone_node *node,
                              int32_t f0)
{
    enum reloq_german_phone phone = node->phone;
    silence(p);
    p->value[RELOQ_TARGET_F0] = f0;
    if (reloq_german_generic_fricative_noise(node, p->value) == 1) {
        struct reloq_german_obstruent_tract tract;
        if (consonant_tract_plan(node,
                node->left_phone==RELOQ_DE_PHONE_NONE,
                node->right_phone==RELOQ_DE_PHONE_NONE,RELOQ_DE_PHONE_NONE,&tract)==1)
            obstruent_profile(p,&tract);
        return;
    }
    switch (phone) {
    case RELOQ_DE_PHONE_m: case RELOQ_DE_PHONE_n:
    case RELOQ_DE_PHONE_G:
    case RELOQ_DE_PHONE_l: case RELOQ_DE_PHONE_r:
    case RELOQ_DE_PHONE_R: case RELOQ_DE_PHONE_j: {
        struct reloq_german_generic_tract tract;
        (void)reloq_german_generic_sonorant_tract(node, &tract);
        p->value[RELOQ_TARGET_F1] = tract.f1;
        p->value[RELOQ_TARGET_F2] = tract.f2;
        p->value[RELOQ_TARGET_F3] = tract.f3;
        p->value[RELOQ_TARGET_B1] = tract.b1;
        p->value[RELOQ_TARGET_B2] = tract.b2;
        p->value[RELOQ_TARGET_FNP] = tract.fnp;
        p->value[RELOQ_TARGET_FNZ] = tract.fnz;
        p->value[RELOQ_TARGET_AV] = tract.av;
        p->value[RELOQ_TARGET_AH] = tract.ah;
        break;
    }
    default:
        break;
    }
    struct reloq_german_nucleus_acoustics nucleus;
    if (nucleus_plan(node, 0, &nucleus) == 1)
        nucleus_profile(p, &nucleus);
    struct reloq_german_nasal_tract nasal;
    if (reloq_german_nasal_tract_plan(node,node->left_phone==RELOQ_DE_PHONE_NONE,
            node->right_phone==RELOQ_DE_PHONE_NONE,&nasal)==1)
        nasal_profile(p,&nasal);
    struct reloq_german_obstruent_tract liquid;
    if (reloq_german_liquid_tract_plan(node,node->left_phone==RELOQ_DE_PHONE_NONE,
            node->right_phone==RELOQ_DE_PHONE_NONE,&liquid)==1)
        obstruent_profile(p,&liquid);
}

static void stop_burst(struct profile *p,
                       const struct reloq_german_phone_node *node,
                       int32_t f0, int syllable_onset,
                       enum reloq_german_generic_noise_phase phase)
{
    silence(p);
    p->value[RELOQ_TARGET_F0] = f0;
    (void)reloq_german_generic_stop_noise(
        node, syllable_onset, phase, p->value);
    if (phase == RELOQ_DE_GENERIC_ASPIRATION && p->value[RELOQ_TARGET_AH])
        p->value[RELOQ_TARGET_AH] += RELOQ_GENERIC_SOURCE_TRIM;
}

static int emit(struct reloq_target_stage_router *router, int64_t tick,
                const struct profile *profile,
                const struct output_settings *settings);

/* A stop burst is a short plateau, not a linear ramp from its closure.
 * Duplicate targets at each side of the plateau are intentional: the curve
 * scheduler treats equal-tick points as an instantaneous source change.
 * Only the p/t/k noise sources use this gesture; the tract keeps its normal
 * coarticulatory trajectory and fricatives retain their own profiles. */
static int emit_stop_plateau(
    struct reloq_target_stage_router *router, int64_t burst_tick,
    int64_t aspiration_tick, const struct profile *burst,
    const struct profile *aspiration, const struct output_settings *output)
{
    struct profile peak = *burst;
    struct profile before = peak;
    before.value[RELOQ_TARGET_AV] = 0;
    before.value[RELOQ_TARGET_AH] = 0;
    before.value[RELOQ_TARGET_AF] = 0;
    for (size_t i = RELOQ_TARGET_A1F; i <= RELOQ_TARGET_AB; ++i)
        before.value[i] = 0;
    struct output_settings handoff = *output;
    handoff.noise_owned = 1;
    return emit(router, burst_tick, &before, output) ||
           emit(router, burst_tick, &peak, output) ||
           emit(router, aspiration_tick, &peak, output) ||
           emit(router, aspiration_tick, aspiration, &handoff);
}

static int source_point_add(struct source_points *out,
    enum reloq_target_stream stream,int64_t tick,int32_t value,size_t phone)
{
    if (!out || out->count>=out->capacity) return -1;
    out->points[out->count]=(struct source_point){tick,value,stream,phone,out->count};
    ++out->count;return 0;
}

static int source_point_order(const void *a,const void *b)
{
    const struct source_point *x=a,*y=b;
    if (x->stream!=y->stream) return x->stream<y->stream?-1:1;
    if (x->tick!=y->tick) return x->tick<y->tick?-1:1;
    if (x->phone!=y->phone) return x->phone<y->phone?-1:1;
    return x->serial<y->serial?-1:x->serial>y->serial;
}

/* Only phrase-restart knots were added after the first sort. Merge that
 * suffix into the existing order, using unused point capacity as scratch.
 * A nearly full caller buffer can still use the ordinary complete sort. */
static void sort_added_source_points(struct source_points *out,size_t sorted)
{
    size_t added=out->count-sorted;
    if(!added)return;
    if(added>out->capacity-out->count) {
        qsort(out->points,out->count,sizeof *out->points,source_point_order);
        return;
    }
    if(added>1)qsort(out->points+sorted,added,sizeof *out->points,source_point_order);
    struct source_point *scratch=out->points+out->capacity-added;
    memcpy(scratch,out->points+sorted,added*sizeof *scratch);
    size_t left=sorted,right=added,at=out->count;
    while(right) {
        --at;
        if(left && source_point_order(out->points+left-1,scratch+right-1)>0)
            out->points[at]=out->points[--left];
        else out->points[at]=scratch[--right];
    }
}

static int emit_source_point(void *context,enum reloq_target_stream stream,
    int64_t tick,int32_t value,size_t phone)
{
    struct output_settings *settings=context;
    int64_t at;
    if (scale_tick(tick,settings,&at)) return -1;
    if ((stream==RELOQ_TARGET_AV || stream==RELOQ_TARGET_AH) && value>0) {
        value=settings->mute?0:value-settings->volume_attenuation;
        if (value<0) value=0;
    }
    return source_point_add(settings->sources,stream,at,value,phone);
}

struct envelope_output {
    struct output_settings *settings;
    const struct reloq_german_source_segment *segments;
    size_t count;
};
static int emit_envelope_point(void *context,enum reloq_target_stream stream,
    int64_t tick,int32_t value,size_t phone)
{
    struct envelope_output *e=context;
    if (phone<e->count && e->settings->native_time_scale>0.0 &&
        e->segments[phone].kind==RELOQ_SOURCE_NUCLEUS &&
        tick==e->segments[phone].begin+1 &&
        (stream==RELOQ_TARGET_AV || stream==RELOQ_TARGET_AH || stream==RELOQ_TARGET_TL)) {
        /* The source's first point is one output millisecond after its
         * boundary. Native speed scales the phone intervals first; it
         * does not compress this event offset into a same-tick step.
         * Keep that ordering in the existing target-time adapter. */
        int64_t begin,end;
        if (scale_tick(e->segments[phone].begin,e->settings,&begin) ||
            scale_tick(e->segments[phone].end,e->settings,&end)) return -1;
        struct output_settings direct=*e->settings;
        direct.native_time_scale=0.0;direct.rate_factor=100;direct.timing=NULL;
        return emit_source_point(&direct,stream,begin<end?begin+1:end,value,phone);
    }
    return emit_source_point(e->settings,stream,tick,value,phone);
}

struct cluster_output {
    struct reloq_target_stage_router *router;
    struct output_settings *settings;
};
static int emit_cluster_point(void *context,enum reloq_target_stream stream,
    int64_t tick,int32_t value,size_t phone)
{
    struct cluster_output *c=context;
    if (stream==RELOQ_TARGET_AV || stream==RELOQ_TARGET_AH || stream==RELOQ_TARGET_FNZ ||
        stream==RELOQ_TARGET_TL)
        return emit_source_point(c->settings,stream,tick,value,phone);
    int64_t at;
    if (scale_tick(tick,c->settings,&at)) return -1;
    if (stream==RELOQ_TARGET_AF && value>0) {
        value=c->settings->mute?0:value-c->settings->volume_attenuation;
        if (value<0) value=0;
    }
    const struct reloq_target_event e={stream,0,value};
    return model_append(c->router,at,&e,1,c->settings,phone);
}

static int emit_selected(struct reloq_target_stage_router *router,
                         int64_t tick, const struct profile *profile,
                         const struct output_settings *settings,
                         int suppress_noise)
{
    struct reloq_target_event events[RELOQ_TARGET_STREAM_COUNT];
    size_t event_count = 0;
    int64_t output_tick;
    if (scale_tick(tick, settings, &output_tick)) return -1;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        /* F0 is an utterance-level contour. Repeating a phone's static
         * pitch on every source/formant target would erase its movement. */
        if (i == RELOQ_TARGET_F0 || i == RELOQ_TARGET_FNZ ||
            (settings->tract_owned &&
             ((i>=RELOQ_TARGET_F1 && i<=RELOQ_TARGET_F5) ||
              (i>=RELOQ_TARGET_B1 && i<=RELOQ_TARGET_B5))) ||
            (settings->pause_tract && i>=RELOQ_TARGET_F1 && i<=RELOQ_TARGET_F3) ||
            (settings->nucleus_tract && ((i >= RELOQ_TARGET_F1 &&
             i <= RELOQ_TARGET_F5) || (i>=RELOQ_TARGET_B1 && i<=RELOQ_TARGET_B5) ||
             i==RELOQ_TARGET_FNP || i==RELOQ_TARGET_FNZ)) ||
            ((suppress_noise || settings->noise_owned) && i >= RELOQ_TARGET_AF &&
             i <= RELOQ_TARGET_AB) ||
            (settings->nucleus_source && (i == RELOQ_TARGET_AV || i == RELOQ_TARGET_AH)))
            continue;
        if (i == RELOQ_TARGET_OQ || i == RELOQ_TARGET_DI ||
            (settings->cluster_source && (i==RELOQ_TARGET_TL || i==RELOQ_TARGET_FNZ))) continue;
        if(i==RELOQ_TARGET_TL && settings->nucleus_source && settings->model_segments &&
           settings->source_phone<settings->phone_count &&
           settings->phones[settings->source_phone].phone!=RELOQ_DE_PHONE_z &&
           reloq_german_source_owns_tilt(settings->model_segments[settings->source_phone].kind))continue;
        int32_t value = profile->value[i];
        if ((i == RELOQ_TARGET_AV || i == RELOQ_TARGET_AH ||
             i == RELOQ_TARGET_AF) && value > 0) {
            value = settings->mute ? 0 :
                    value - (i == RELOQ_TARGET_AF ? 0 : RELOQ_GENERIC_SOURCE_TRIM) -
                    settings->volume_attenuation;
            if (value < 0) value = 0;
        }
        if (i==RELOQ_TARGET_AV || i==RELOQ_TARGET_AH || i==RELOQ_TARGET_FNZ || i==RELOQ_TARGET_TL) {
            const size_t owner=i==RELOQ_TARGET_TL && settings->model_owner_plus_one?
                settings->model_owner_plus_one-1:settings->source_phone;
            if (source_point_add(settings->sources,(enum reloq_target_stream)i,
                output_tick,value,owner)) return -1;
            continue;
        }
        events[event_count++] = (struct reloq_target_event){
            (enum reloq_target_stream)i, 0, value};
    }
    return model_append(router,output_tick,events,event_count,settings,settings->source_phone);
}

static int emit_nucleus_tract(
    struct reloq_target_stage_router *router, int64_t at,
    const int32_t formants[5], const struct output_settings *settings)
{
    int64_t scaled;
    struct reloq_target_event events[5];
    if (scale_tick(at, settings, &scaled)) return -1;
    for (unsigned i = 0; i < 5; ++i)
        events[i] = (struct reloq_target_event){
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + i), 0, formants[i]};
    return model_append(router,scaled,events,5,settings,settings->source_phone);
}

static int emit_nucleus_resonance(struct reloq_target_stage_router *router,
    int64_t at,const struct reloq_german_nucleus_acoustics *n,
    struct output_settings *settings)
{
    int64_t scaled;
    struct reloq_target_event events[6];
    if (scale_tick(at,settings,&scaled)) return -1;
    for (unsigned j=0;j<5;++j)
        events[j]=(struct reloq_target_event){
            (enum reloq_target_stream)(RELOQ_TARGET_B1+j),0,n->bandwidth[j]};
    events[5]=(struct reloq_target_event){RELOQ_TARGET_FNP,0,n->fnp};
    return model_append(router,scaled,events,6,settings,settings->source_phone) ||
        emit_source_point(settings,RELOQ_TARGET_FNZ,at,n->fnz,settings->source_phone);
}

struct nasal_output {
    struct reloq_target_stage_router *router;
    const struct output_settings *settings;
};
static int emit_nasal_tract(void *context,enum reloq_target_stream stream,
    int64_t tick,int32_t value,size_t index)
{
    (void)index;
    struct nasal_output *out=context;int64_t at;
    if (scale_tick(tick,out->settings,&at)) return -1;
    const struct reloq_target_event event={stream,0,value};
    return model_append(out->router,at,&event,1,out->settings,index);
}

static int emit(struct reloq_target_stage_router *router, int64_t tick,
                const struct profile *profile,
                const struct output_settings *settings)
{
    return emit_selected(router, tick, profile, settings, 0);
}

/* A common phone-to-pause path is used both inside a request and at its
 * end. The original pause has no primary formant targets. A fabricated
 * neutral F1/F2/F3 profile would bend the preceding audible release. */
static int emit_pause(struct reloq_target_stage_router *router, int64_t tick,
    const struct reloq_german_phone_node *node,
    struct reloq_german_source_segment *source, size_t index,
    enum reloq_german_terminal_mark mark, int continuing,
    const struct profile *previous, const struct output_settings *output,
    int32_t *window,int quiet_override,int explicit_pause,int pause_mode)
{
    struct reloq_german_pause pause;
    if (reloq_german_pause_plan(node,source->kind==RELOQ_SOURCE_CARRY,
                              mark,continuing,&pause)) return -1;
    if(quiet_override>=0)pause.quiet_ticks=quiet_override;
    /* Apply the policy before any targets or PCM exist. The preceding
     * phone's release, clause-final duration and melody remain intact.
     * Reference p1/p2/p3 all have the same three-tick quiet body. */
    if(!explicit_pause && (pause_mode==2 || (pause_mode==1 && !continuing)))
        pause.quiet_ticks=3;
    *window=pause.release_ticks+pause.quiet_ticks;
#ifdef RELOQ_GENERIC_PHONE_TRACE
    fprintf(stderr,"RELOQ_PAUSE %zu %d %lld %d %d %d %d %d\n",index,
        (int)node->phone,(long long)tick,pause.release_ticks,pause.quiet_ticks,
        pause.av,pause.ah,pause.voiced_ticks);
#endif
    if (tick<0 || tick>INT64_MAX-*window) return -1;
    source->continuing_comma=continuing &&
        reloq_german_terminal_continues(mark);
    struct output_settings tail=*output;
    tail.source_phone=index;
    tail.nucleus_source=source->kind!=RELOQ_SOURCE_NONE;
    tail.nucleus_tract=0;
    tail.noise_owned=0;
    if (emit(router,tick,previous,&tail)) return -1;
    if (!tail.nucleus_source &&
        reloq_german_pause_release(&pause,tick,index,emit_source_point,&tail)) return -1;
    if(pause.voiced_ticks && (node->phone==RELOQ_DE_PHONE_m || node->phone==RELOQ_DE_PHONE_n)) {
        /* A nasal's voiced tail retains the base source tilt up to its
         * release switch. In particular the m-body's extra five-dB
         * correction ends here; ramping all the way to the quiet target
         * would brighten the still-voiced tail. These are final model
         * values, so they must not pass through anatomy mapping twice. */
        struct reloq_german_phone_node release=*node;
        release.phone=RELOQ_DE_PHONE_n;release.right_phone=RELOQ_DE_PHONE_NONE;
        int tl=reloq_german_voice_model_target(&output->model,&release,NULL,RELOQ_TARGET_TL,0);
        if(emit_source_point(&tail,RELOQ_TARGET_TL,tick,tl,SIZE_MAX) ||
           emit_source_point(&tail,RELOQ_TARGET_TL,tick+pause.voiced_ticks,tl,SIZE_MAX) ||
           emit_source_point(&tail,RELOQ_TARGET_TL,tick+pause.voiced_ticks,0,SIZE_MAX))return -1;
    }
    struct profile quiet;
    silence(&quiet);
    tail.source_phone=index+1;
    tail.nucleus_source=0;
    tail.pause_tract=1;
    /* A new phrase's initial targets supersede the previous silence. Keep
     * the old hold on its own side of that discontinuity; otherwise equal-
     * tick quiet anchors hide the first nasal resonance/source target. */
    int64_t quiet_end;
    if (scale_tick(tick+*window,&tail,&quiet_end)) return -1;
    if (continuing && !reloq_german_terminal_continues(mark) && quiet_end>0) --quiet_end;
    struct output_settings direct=tail;
    direct.native_time_scale=0.0;direct.rate_factor=100;direct.timing=NULL;
    if (continuing && !reloq_german_terminal_continues(mark)) {
        struct nasal_output hold={router,&direct};
        for (unsigned j=0;j<3;++j)
            if (emit_nasal_tract(&hold,(enum reloq_target_stream)(RELOQ_TARGET_F1+j),
                quiet_end,previous->value[RELOQ_TARGET_F1+j],index)) return -1;
    }
    tail.tract_owned=0;tail.cluster_source=0;
    direct.tract_owned=0;direct.cluster_source=0;
    /* Neutral phonation persists through a pause. A later fresh glottal
     * onset must not interpolate backwards into the preceding release. */
    if (emit_source_point(&tail,RELOQ_TARGET_DI,tick+pause.release_ticks,0,index+1) ||
        emit_source_point(&direct,RELOQ_TARGET_DI,quiet_end,0,index+1) ||
        emit_source_point(&tail,RELOQ_TARGET_OQ,tick+pause.release_ticks,56,index+1) ||
        emit_source_point(&direct,RELOQ_TARGET_OQ,quiet_end,56,index+1)) return -1;
    return emit_source_point(&tail,RELOQ_TARGET_FNZ,tick+pause.release_ticks,200,index+1) ||
           emit_source_point(&direct,RELOQ_TARGET_FNZ,quiet_end,200,index+1) ||
           emit(router,tick+pause.release_ticks,&quiet,&tail) ||
           emit(router,quiet_end,&quiet,&direct);
}

static int emit_vowel(struct reloq_target_stage_router *router, int64_t tick,
                      const struct profile *profile,
                      const struct output_settings *settings,
                      int fricative_vowel)
{
    return emit_selected(router, tick, profile, settings, fricative_vowel);
}

static int emit_noise_end(struct reloq_target_stage_router *router,
                          int64_t tick, const struct output_settings *settings)
{
    struct reloq_target_event events[RELOQ_TARGET_AB - RELOQ_TARGET_AF + 1];
    size_t n = 0;
    int64_t at;
    if (scale_tick(tick, settings, &at)) return -1;
    for (size_t i = RELOQ_TARGET_AF; i <= RELOQ_TARGET_AB; ++i)
        events[n++] = (struct reloq_target_event){(enum reloq_target_stream)i,
                                                  0, 0};
    return model_append(router,at,events,n,settings,settings->source_phone);
}

static int emit_source_range(struct reloq_target_stage_router *router,
    int64_t tick, const struct profile *profile,
    const struct output_settings *settings, size_t first, size_t last)
{
    struct reloq_target_event events[RELOQ_TARGET_STREAM_COUNT];
    size_t n=0;int64_t at;
    if (scale_tick(tick,settings,&at)) return -1;
    for (size_t i=first;i<=last;++i) {
        if (i==RELOQ_TARGET_OQ || i==RELOQ_TARGET_DI) continue;
        int32_t value=profile->value[i];
        if (i<=RELOQ_TARGET_AF && value>0) {
            value=settings->mute?0:value-settings->volume_attenuation-
                ((i==RELOQ_TARGET_AV || i==RELOQ_TARGET_AH)?RELOQ_GENERIC_SOURCE_TRIM:0);
            if (value<0) value=0;
        }
        if (i==RELOQ_TARGET_AV || i==RELOQ_TARGET_AH || i==RELOQ_TARGET_FNZ) {
            if (source_point_add(settings->sources,(enum reloq_target_stream)i,
                at,value,settings->source_phone)) return -1;
        } else events[n++]=(struct reloq_target_event){(enum reloq_target_stream)i,0,value};
    }
    return model_append(router,at,events,n,settings,settings->source_phone);
}

static int noisy_fricative(enum reloq_german_phone p)
{
    return p==RELOQ_DE_PHONE_f || p==RELOQ_DE_PHONE_v ||
        p==RELOQ_DE_PHONE_s || p==RELOQ_DE_PHONE_z ||
        p==RELOQ_DE_PHONE_S || p==RELOQ_DE_PHONE_Z ||
        p==RELOQ_DE_PHONE_x || p==RELOQ_DE_PHONE_X;
}

static int palatal_fricative(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_x || p == RELOQ_DE_PHONE_X;
}

/* Select one stop owner for rendering and for the preceding vowel's
 * lookahead. A separate VC approximation must not introduce a conflicting
 * target before the selected closure begins. */
static int stop_before_h(const struct reloq_german_phone_node *phones,
    size_t count,size_t i,const enum reloq_german_terminal_mark *marks)
{
    return i+2<count && (phones[i].phone==RELOQ_DE_PHONE_p ||
        phones[i].phone==RELOQ_DE_PHONE_t ||
        phones[i].phone==RELOQ_DE_PHONE_k) &&
        !phones[i].affricate_role && phones[i+1].phone==RELOQ_DE_PHONE_h &&
        phones[i+1].same_syllable_right && vowel(phones[i+2].phone) &&
        (phones[i].same_word_right || !marks ||
         marks[phones[i].word_index]==RELOQ_DE_TERMINAL_IMPLICIT);
}

/* Repeated velar closure before a same-syllable sibilant. The first
 * constriction moves from its vowel-conditioned place to a shared closure;
 * only the second constriction owns the burst and release. */
static int repeated_velar_sibilant(const struct reloq_german_phone_node *p,
    size_t count,size_t i,struct reloq_german_stop_phase *out)
{
    if(p[i].phone!=RELOQ_DE_PHONE_k)return 0;
    size_t first=i;
    if(i && p[i-1].phone==RELOQ_DE_PHONE_k && p[i].same_syllable_left)
        first=i-1;
    if(first+2>=count || p[first+1].phone!=RELOQ_DE_PHONE_k ||
       !p[first].same_word_left || !p[first].same_syllable_left ||
       !p[first].same_syllable_right || !p[first+1].same_word_right ||
       !p[first+1].same_syllable_right || p[first].affricate_role ||
       p[first+1].affricate_role ||
       (p[first+2].phone!=RELOQ_DE_PHONE_s && p[first+2].phone!=RELOQ_DE_PHONE_z))return 0;
    struct reloq_german_phone_node closing=p[first];
    closing.right_phone=p[first+2].phone;closing.same_syllable_right=0;
    struct reloq_german_stop_phase phase;
    if(reloq_german_stop_fricative_phase(&closing,p[first+2].stress,&phase)!=1)return 0;
    if(closing.left_phone==RELOQ_DE_PHONE_o)phase.gesture.tract.entering[0]=300;
    static const int32_t place[5]={300,1500,2300,3600,3900};
    if(i==first) {
        phase.gesture.body_ticks=closing.left_phone==RELOQ_DE_PHONE_AT?60:65;
        phase.gesture.burst_ticks=0;phase.gesture.release_ticks=1;
        phase.gesture.af=0;memset(phase.gesture.noise,0,sizeof phase.gesture.noise);
        memcpy(phase.gesture.tract.departing,place,3*sizeof *place);
        phase.gesture.tract.departing[3]=phase.gesture.tract.departing[4]=-1;
        phase.incoming_ticks=1;
    } else {
        memcpy(phase.gesture.tract.entering,place,sizeof place);
        phase.gesture.tract.departing[0]=300;
        phase.gesture.tract.departing[1]=1800;
        phase.gesture.tract.departing[2]=2300;
        phase.gesture.tract.departing[3]=phase.gesture.tract.departing[4]=-1;
        phase.gesture.body_ticks=68;phase.gesture.burst_ticks=8;
        phase.gesture.release_ticks=phase.incoming_ticks=10;
        phase.gesture.af=60;memset(phase.gesture.noise,0,sizeof phase.gesture.noise);
        phase.gesture.noise[1]=phase.gesture.noise[2]=60;
    }
    phase.gesture.release_ah=-1;*out=phase;return 1;
}

static int select_stop_phase(
    const struct reloq_german_phone_node *phones,size_t phone_count,size_t i,
    const struct reloq_german_phone_node *node,int fresh,
    const enum reloq_german_terminal_mark *marks,
    struct reloq_german_stop_phase *out)
{
    if (stop_before_h(phones,phone_count,i,marks)) {
        /* h is the following nucleus's aspiration, not an oral target.
         * Reuse its stop-place plan, but leave the entire breath interval
         * to h. The lexical edge retains its shorter, eight-tick burst. */
        struct reloq_german_phone_node oral=*node;
        oral.right_phone=phones[i+2].phone;
        const int internal=node->same_word_right;
        const int sibilant=node->left_phone==RELOQ_DE_PHONE_s ||
            node->left_phone==RELOQ_DE_PHONE_S;
        const enum reloq_german_phone coda=phones[i+2].same_syllable_right?
            phones[i+2].right_phone:RELOQ_DE_PHONE_NONE;
        int status;
        if (internal) {
            oral.same_syllable_left=0;oral.same_syllable_right=1;
            status=reloq_german_stop_phase_plan(&oral,coda,fresh,out);
            if (status==1 && sibilant && node->phone==RELOQ_DE_PHONE_t) {
                out->gesture.body_ticks+=8-out->gesture.burst_ticks;
                out->gesture.burst_ticks=8;
            }
        } else {
            status=reloq_german_stop_word_edge(&oral,phones[i+2].stress,0,out);
            if (status==1 && !node->stress && node->phone==RELOQ_DE_PHONE_t)
                out->gesture.body_ticks=node->weak_word_control?43:58;
            else if (status==1 && oral.right_phone==RELOQ_DE_PHONE_AT && node->phone==RELOQ_DE_PHONE_t) {
                struct reloq_german_stop_phase reduced;
                oral.same_word_right=oral.same_syllable_right=1;
                oral.same_syllable_left=0;
                if (reloq_german_stop_phase_plan(&oral,coda,0,&reduced)==1)
                    out->gesture.body_ticks=reduced.gesture.body_ticks-
                        reduced.gesture.burst_ticks+8;
            }
        }
        if (status) {
            if (status==1) {
                if (node->same_syllable_left && vowel(node->left_phone)) {
                    struct reloq_german_phone_node closing=*node;
                    struct reloq_german_obstruent_tract tract;
                    closing.same_word_right=0;
                    if(reloq_german_terminal_stop_tract_plan(&closing,&tract)==1)
                        memcpy(out->gesture.tract.entering,tract.entering,sizeof tract.entering);
                }
                if (node->phone==RELOQ_DE_PHONE_k && !node->same_syllable_right) {
                    /* The original coda branch keeps the closure's left
                     * place and anticipates the vowel through h. The next
                     * syllable's accent selects closure length, while h
                     * alone owns aspiration. These clocks are independent. */
                    const enum reloq_german_phone l=node->left_phone,v=oral.right_phone;
                    const int nasal=l==RELOQ_DE_PHONE_n || l==RELOQ_DE_PHONE_G;
                    const int low=l==RELOQ_DE_PHONE_A || l==RELOQ_DE_PHONE_a;
                    const int front=l==RELOQ_DE_PHONE_e || l==RELOQ_DE_PHONE_E ||
                        l==RELOQ_DE_PHONE_i || l==RELOQ_DE_PHONE_I ||
                        l==RELOQ_DE_PHONE_y || l==RELOQ_DE_PHONE_Y ||
                        l==RELOQ_DE_PHONE_Q_oe || l==RELOQ_DE_PHONE_Q_OE ||
                        l==RELOQ_DE_PHONE_Q_E_COLON || l==RELOQ_DE_PHONE_AT;
                    if (l==RELOQ_DE_PHONE_O) out->gesture.tract.entering[0]=300;
                    else if (l==RELOQ_DE_PHONE_l) out->gesture.tract.entering[0]=500;
                    const int forward=v==RELOQ_DE_PHONE_e || v==RELOQ_DE_PHONE_i ||
                        v==RELOQ_DE_PHONE_E || v==RELOQ_DE_PHONE_I;
                    out->gesture.burst_ticks=sibilant?8:12;
                    out->gesture.body_ticks=(nasal?55:l==RELOQ_DE_PHONE_AT?60:
                        internal && !phones[i+2].stress?58:75)+out->gesture.burst_ticks;
                    memset(out->gesture.noise,0,sizeof out->gesture.noise);
                    out->gesture.af=low?(forward?(internal?62:63):65):
                        front?(l==RELOQ_DE_PHONE_AT||l==RELOQ_DE_PHONE_Y?57:60):63;
                    if (!internal && forward) {
                        out->gesture.af=63;out->gesture.noise[1]=60;
                    } else if (front) out->gesture.noise[2]=60;
                    else out->gesture.noise[1]=low && forward?62:70;
                }
                out->gesture.release_ticks=out->incoming_ticks=0;
                out->gesture.release_ah=-1;
                if (node->phone==RELOQ_DE_PHONE_t) {
                    out->gesture.af=oral.right_phone==RELOQ_DE_PHONE_AT?56:59;
                    memset(out->gesture.noise,0,sizeof out->gesture.noise);
                    out->gesture.noise[4]=56;
                }
            }
            return status;
        }
    }
    int duplicate=repeated_velar_sibilant(phones,phone_count,i,out);
    if(duplicate)return duplicate;
    int status=
        reloq_german_stop_word_edge(phones+i,i+1<phone_count?phones[i+1].stress:0,
        !phones[i].same_word_right && (i+1==phone_count ||
         (marks && marks[phones[i].word_index]!=RELOQ_DE_TERMINAL_IMPLICIT)),out);
    if (!status)
        status=reloq_german_stop_phase_plan(phones+i,
        i+1<phone_count && phones[i+1].same_syllable_right?phones[i+1].right_phone:RELOQ_DE_PHONE_NONE,
        fresh,out);
    if (status) return status;
    if (i+1<phone_count)
        status=reloq_german_stop_vowel_boundary(phones+i,phones[i+1].stress,
            phones[i+1].same_syllable_right?phones[i+1].right_phone:RELOQ_DE_PHONE_NONE,out);
    if (status) return status;
    /* /tv/ has a stop closure and a labiodental release even when
     * the syllabifier assigns the complete group to the next nucleus.
     * A preceding consonant must not send this group to the generic CV
     * planner. Vocalic r belongs to the postvocalic duration class. */
    struct reloq_german_phone_node coda=phones[i];
    int tv_cross=coda.phone==RELOQ_DE_PHONE_t && coda.right_phone==RELOQ_DE_PHONE_v &&
        coda.same_word_left && coda.same_word_right && !coda.same_syllable_left &&
        !vowel(coda.left_phone) && coda.left_phone!=RELOQ_DE_PHONE_NONE;
    if(tv_cross)coda.same_syllable_left=1;
    if (!status && i+1<phone_count)
        status=reloq_german_stop_coda_phase(&coda,phones[i+1].stress,
        !phones[i+1].same_word_right && (i+2==phone_count ||
         (marks && marks[phones[i+1].word_index]!=RELOQ_DE_TERMINAL_IMPLICIT)),
        i+2<phone_count && phones[i+1].same_word_right?phones[i+2].phone:RELOQ_DE_PHONE_NONE,out);
    if(status==1 && tv_cross &&
       (coda.left_phone==RELOQ_DE_PHONE_s || coda.left_phone==RELOQ_DE_PHONE_S)) {
        out->gesture.body_ticks+=12-out->gesture.burst_ticks;
        out->gesture.burst_ticks=12;
    }
    if(status==1 && tv_cross && coda.left_phone==RELOQ_DE_PHONE_r && i>=2 &&
       phones[i-1].same_syllable_left && vowel(phones[i-2].phone))
        out->gesture.body_ticks=55;
    if (status) return status;
    if (!status && i+2<phone_count &&
        phones[i+1].same_syllable_right && vowel(phones[i+2].phone))
        status=reloq_german_stop_sonorant_phase(node,phones[i+1].stress,out);
    if (status) return status;
    if (!status && i+1<phone_count)
        status=reloq_german_stop_fricative_phase(node,phones[i+1].stress,out);
    if (status) return status;
    if (!status && i+2<phone_count &&
        (node->same_word_right || !marks || marks[node->word_index]==RELOQ_DE_TERMINAL_IMPLICIT)) {
        struct reloq_german_stop_phase following;
        if (reloq_german_stop_phase_plan(phones+i+1,
        phones[i+2].same_syllable_right?phones[i+2].right_phone:RELOQ_DE_PHONE_NONE,
        0,&following)==1)
        status=reloq_german_stop_pair_phase(phones+i,phones[i+1].stress,out);
        if (status) return status;
    }
    return status;
}

static int generic_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_generic_settings *settings,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    int64_t *word_ticks, size_t word_count,
    const struct reloq_inline_word *controls,
    const struct reloq_german_word_features *features,size_t feature_count,
    struct reloq_target_builder *builder, int64_t *end_tick,
    reloq_german_cancel_check cancelled, void *cancel_context,float sample_rate,
    const struct inline_timing *timing_map,struct source_points *colour,struct source_points *sibilants)
{
    struct output_settings output;
    if ((!phones && phone_count) || (!syllables && syllable_count) ||
        !builder || !end_tick || !phone_count || !syllable_count ||
        (feature_count && !features))
        return -1;
    if (marks && (!mark_count ||
                  phones[phone_count - 1].word_index != mark_count - 1))
        return -1;
    if (word_ticks && (!word_count ||
                       phones[phone_count - 1].word_index != word_count - 1))
        return -1;
    if (output_settings_init(&output, settings)) return -1;
    output.model.sample_rate=sample_rate;
    output.models=controls;output.model_count=controls?word_count:0;
    output.timing=timing_map;
    output.colour=colour;output.sibilants=sibilants;
    if(timing_map)output.native_time_scale=1.0;
    if (cancelled && cancelled(cancel_context)) return 1;
    size_t cursor = 0;
    for (size_t s = 0; s < syllable_count; ++s) {
        const struct reloq_german_syllable *syllable = syllables + s;
        if (syllable->first_phone != cursor || !syllable->phone_count ||
            syllable->phone_count > phone_count - cursor ||
            !syllable->nucleus_contiguous)
            return -1;
        /* German weak -er and reduced endings may use syllabic R/n/l.
         * The structural syllable planner reports zero vowel nuclei for
         * these; an approximating voice can still give the sonorant its
         * own sustained voiced interval. Do not admit all-obstruent groups. */
        if (!syllable->nucleus_count) {
            int found = 0;
            for (size_t p = cursor;
                 p < cursor + syllable->phone_count; ++p)
                found |= syllabic_sonorant(phones[p].phone);
            if (!found) return -1;
        }
        cursor += syllable->phone_count;
    }
    if (cursor != phone_count) return -1;
    if (syllable_count > SIZE_MAX / sizeof(struct reloq_german_generic_melody_syllable) ||
        phone_count > SIZE_MAX / sizeof(struct reloq_german_nucleus_schedule))
        return -1;
    struct reloq_german_generic_melody_syllable *melody =
        calloc(syllable_count, sizeof *melody);
    if (!melody) return -1;
    struct reloq_german_nucleus_schedule *timings=calloc(phone_count,sizeof *timings);
    if (!timings) {free(melody);return -1;}
    int scheduled=reloq_german_nucleus_schedule_build_with_features(phones,phone_count,
        syllables,syllable_count,marks,mark_count,features,feature_count,
        timings,cancelled,cancel_context);
    if (scheduled) {free(timings);free(melody);return scheduled;}
    struct reloq_target_stage_router stages = {0};
    if (reloq_target_stage_router_begin_borrowed(&stages, builder) &&
        reloq_target_stage_router_begin(&stages, builder)) {
        free(melody);
        free(timings);
        return -1;
    }
    output.phones=phones;output.phone_count=phone_count;
    struct reloq_german_glottal_segment *glottal=calloc(phone_count,sizeof *glottal);
    struct reloq_german_source_segment *source_segments=calloc(phone_count,sizeof *source_segments);
    const size_t source_capacity=builder->stream[RELOQ_TARGET_AV].capacity+
                                 builder->stream[RELOQ_TARGET_AH].capacity+
                                 builder->stream[RELOQ_TARGET_FNZ].capacity+
                                 builder->stream[RELOQ_TARGET_TL].capacity+
                                 builder->stream[RELOQ_TARGET_OQ].capacity+
                                 builder->stream[RELOQ_TARGET_DI].capacity;
    struct source_points source_points={NULL,0,source_capacity};
    if (source_capacity<=SIZE_MAX/sizeof *source_points.points)
        source_points.points=malloc(source_capacity*sizeof *source_points.points);
    if (!glottal || !source_segments || !source_points.points) {
        free(glottal);free(source_segments);free(source_points.points);
        reloq_target_stage_router_dispose(&stages);free(timings);free(melody);return -1;
    }
    output.sources=&source_points;
    output.model_segments=source_segments;
    for (size_t s = 0; s < syllable_count; ++s)
        melody[s].start_tick = -1;
    int result = -1;
    struct profile previous, current, burst;
    silence(&previous);
    int64_t tick = controls && controls[0].pause>0?controls[0].pause+1:0;
    int previous_source_owned=0,previous_tract_owned=0,previous_cluster=0;
    int previous_noise_contact=0;
    size_t feature_index=0,phrase_start_phone=0;
    int64_t previous_pitch_begin=0,previous_pitch_end=0,previous_body_end=0;
    for (size_t i = 0; i < phone_count; ++i) {
        if (cancelled && cancelled(cancel_context)) do { result = 1; goto cleanup; } while (0);
        output.source_phone=i;
        if(controls)output.model=inline_model(controls+phones[i].word_index,sample_rate);
        output.nucleus_source=0;
        output.tract_owned=0;
        output.noise_owned=0;
        output.cluster_source=0;
        const int incoming_tract_owned=previous_tract_owned;
        struct reloq_german_phone_node acoustic_node=phones[i];
        /* Articulation at a written phrase end cannot anticipate a phone
         * from the following phrase. Keep lexical indices/adjacency intact. */
        if (!acoustic_node.same_word_right && marks &&
            marks[acoustic_node.word_index]!=RELOQ_DE_TERMINAL_IMPLICIT)
            acoustic_node.right_phone=RELOQ_DE_PHONE_NONE;
        /* The lexical r remains in the annotation. Only its acoustic coda
         * role supplies the following consonant's vowel context. */
        const int after_vocalic_r=i && acoustic_node.same_word_left &&
            phones[i-1].phone==RELOQ_DE_PHONE_r && timings[i-1].closing_ticks;
        const int after_coda_l=i && acoustic_node.same_word_left &&
            phones[i-1].phone==RELOQ_DE_PHONE_l && timings[i-1].closing_ticks;
        const int after_closing_j=i>=2 && acoustic_node.same_syllable_left &&
            phones[i-1].phone==RELOQ_DE_PHONE_j && timings[i-1].closing_ticks &&
            noisy_fricative(acoustic_node.phone);
        if (after_vocalic_r) acoustic_node.left_phone=RELOQ_DE_PHONE_R;
        if (after_closing_j) acoustic_node.left_phone=
            phones[i-2].phone==RELOQ_DE_PHONE_o || phones[i-2].phone==RELOQ_DE_PHONE_O?
                RELOQ_DE_PHONE_Q_oj:RELOQ_DE_PHONE_Q_aj;
        const struct reloq_german_phone_node *node = &acoustic_node;
        struct reloq_german_onset_cluster onset;
        int cluster_active=reloq_german_onset_cluster_plan(phones,phone_count,i,&onset);
        if (cluster_active<0) goto cleanup;
        struct reloq_german_nucleus_schedule *timing=timings+i;
        if (node->phone <= RELOQ_DE_PHONE_NONE ||
            node->phone > RELOQ_DE_PHONE_Q_oe_TILDE) goto cleanup;
        int32_t boundary_pause = 0;
        if (i && !node->same_word_left) {
            if (marks &&
                reloq_german_generic_prosody_boundary_ticks(
                    marks[phones[i - 1].word_index], &boundary_pause))
                goto cleanup;
            /* The ordinary connection retains the preceding gesture.
             * Written clause punctuation releases it into silence. */
            if (boundary_pause) {
                if (emit_pause(&stages,tick,phones+i-1,source_segments+i-1,i-1,
                        marks[phones[i-1].word_index],1,&previous,&output,&boundary_pause,
                        controls?controls[node->word_index].pause:-1,
                        controls?controls[node->word_index].explicit_pause:0,
                        settings?settings->pause_mode:0))
                    goto cleanup;
                struct profile quiet;
                silence(&quiet);
                tick += boundary_pause;
                previous = quiet;
                phrase_start_phone=i;
                if (!reloq_german_terminal_continues(marks[phones[i-1].word_index])) {
                    acoustic_node.left_phone=RELOQ_DE_PHONE_NONE;
                    previous_source_owned=previous_tract_owned=previous_cluster=previous_noise_contact=0;
                }
                if (reloq_german_terminal_continues(marks[phones[i-1].word_index])) ++tick;
                else if (node->phone==RELOQ_DE_PHONE_n || node->phone==RELOQ_DE_PHONE_m ||
                         node->phone==RELOQ_DE_PHONE_G) {
                    /* A complete phrase restarts sparse-curve interpolation.
                     * The first nasal midpoint extends back to that phrase's
                     * start, rather than rising from the preceding silence. */
                    if (emit_source_point(&output,RELOQ_TARGET_FNZ,tick,
                        node->phone==RELOQ_DE_PHONE_G?400:350,i)) goto cleanup;
                }
            }
        }
        if (boundary_pause && cluster_active) {
            struct reloq_german_phone_node fresh[4];
            const size_t remaining=phone_count-i<4?phone_count-i:4;
            memcpy(fresh,phones+i,remaining*sizeof *fresh);
            fresh[0].left_phone=RELOQ_DE_PHONE_NONE;
            cluster_active=reloq_german_onset_cluster_plan(fresh,remaining,0,&onset);
            if (cluster_active<0) goto cleanup;
        }
        while (feature_index<feature_count && features[feature_index].word_index<node->word_index)
            ++feature_index;
        struct reloq_german_glottal_segment *glot=glottal+i;
        glot->active=(unsigned)reloq_german_glottal_onset(node,
            feature_index<feature_count?features+feature_index:NULL);
        glot->fresh=!i || boundary_pause!=0;
        glot->strength=reloq_german_glottal_strength(node);
        int connected_stop_vowel=0;
        if (i && !glot->active && !glot->fresh && timing->span_ticks &&
            !timing->incoming_ticks && !timing->glide_ticks && !timing->closing_ticks) {
            struct reloq_german_stop_phase boundary;
            int32_t contact=0;
            /* Text analysis can suppress a glottal onset on a stressed
             * vowel too. It still owns the same measured vowel contact;
             * stress alone must not bypass the existing glide scheduler. */
            if (!glot->active && node->same_word_left && !node->same_syllable_left &&
                vowel(node->left_phone)) {
                if (reloq_german_transition_vv_ticks(phones+i-1,node,&contact)<0)
                    goto cleanup;
            } else if (reloq_german_stop_vowel_boundary(phones+i-1,node->stress,
                node->same_syllable_right?node->right_phone:RELOQ_DE_PHONE_NONE,&boundary)==1)
                contact=boundary.incoming_ticks;
            else if (!glot->active && node->same_word_left && !node->same_syllable_left &&
                     noisy_fricative(node->left_phone)) {
                struct reloq_german_phone_node from=phones[i-1];
                from.same_syllable_right=1;
                if (reloq_german_transition_cv_ticks(&from,&contact)<0) goto cleanup;
            }
            if (contact) {
                /* Accented reduced-r carries an independent nucleus budget;
                 * its preceding stop contact must not shorten that body. */
                if (node->phone==RELOQ_DE_PHONE_R && node->stress &&
                    (node->left_phone==RELOQ_DE_PHONE_p ||
                     node->left_phone==RELOQ_DE_PHONE_t ||
                     node->left_phone==RELOQ_DE_PHONE_k)) {
                    if(timing->budget.adjusted_ticks>INT32_MAX-contact)goto cleanup;
                    timing->budget.adjusted_ticks+=contact;
                }
                struct reloq_nucleus_interval fixed[2]={
                    {contact,1},{timing->outgoing_ticks,1}};
                int32_t allocated[2],free_ticks;
                size_t count=timing->outgoing_ticks?2:1;
                if (reloq_german_nucleus_distribute(fixed,count,
                    timing->budget.adjusted_ticks,3,allocated,&free_ticks)) goto cleanup;
                timing->incoming_ticks=allocated[0];
                timing->outgoing_ticks=count==2?allocated[1]:0;
                timing->vowel_ticks=free_ticks;
                timing->span_ticks=free_ticks+allocated[0]+timing->outgoing_ticks;
                connected_stop_vowel=1;
            }
        }
        glot->begin=tick;
        if (glot->active && !glot->fresh && !connected_stop_vowel) {
            int32_t transition=reloq_german_glottal_transition(phones+i-1,node);
            if (tick>INT64_MAX-transition) goto cleanup;
            tick+=transition;
        }
        /* A requested glottal onset follows the same separate contact as
         * a lexical onset. Do not consume its vowel's oral budget. */
        glot->onset=tick;
        if (word_ticks && (!i || !node->same_word_left)) {
            if (node->word_index >= word_count ||
                scale_tick(tick, &output,
                           &word_ticks[node->word_index])) goto cleanup;
        }
        const struct vowel_shape *shape = vowel(node->phone);
        struct reloq_german_stop_phase stop_phase;
        int stop_phase_active=cluster_active?0:select_stop_phase(
            phones,phone_count,i,node,!i || boundary_pause!=0,marks,&stop_phase);
        if (stop_phase_active<0) goto cleanup;
        if (stop_phase_active && i+1<phone_count && node->phone==RELOQ_DE_PHONE_t &&
            !node->same_syllable_right && node->same_word_right &&
            node->right_phone!=RELOQ_DE_PHONE_AT &&
            !reloq_german_glottal_onset(phones+i+1,
                feature_index<feature_count?features+feature_index:NULL)) {
            struct reloq_german_stop_phase boundary;
            if (reloq_german_stop_vowel_boundary(phones+i,phones[i+1].stress,
                phones[i+1].same_syllable_right?phones[i+1].right_phone:RELOQ_DE_PHONE_NONE,&boundary)==1)
                stop_phase.gesture.af=59;
        }
        const int before_plain_cv = shape && i + 2 < phone_count &&
            node->same_word_right && !node->same_syllable_right &&
            phones[i + 1].same_syllable_right &&
            vowel(phones[i + 2].phone);
        const int phrase_final = !node->same_word_right &&
            (i + 1 == phone_count ||
             (marks && marks[node->word_index] !=
                       RELOQ_DE_TERMINAL_IMPLICIT));
        const int coda_after_vowel = !shape && i &&
            node->same_syllable_left && (vowel(phones[i - 1].phone) || after_vocalic_r || after_coda_l || after_closing_j);
        struct reloq_german_generic_nasal_coda nasal_coda = {0};
        const int nasal_context = reloq_german_generic_nasal_coda_plan(
            node, coda_after_vowel, phrase_final,
            i + 1 < phone_count && node->same_word_right &&
                vowel(phones[i + 1].phone),
            previous.value[RELOQ_TARGET_AV], RELOQ_GENERIC_SOURCE_TRIM,
            &nasal_coda);
        if (nasal_context < 0) goto cleanup;
        const int following_onset_cluster = !node->same_word_right &&
            i + 2 < phone_count &&
            !vowel(phones[i + 1].phone) &&
            phones[i + 1].same_word_right &&
            phones[i + 2].same_syllable_left &&
            !vowel(phones[i + 2].phone);
        int32_t span = duration(node, shape, before_plain_cv, phrase_final,
                                following_onset_cluster);
        if (!shape && node->phone == RELOQ_DE_PHONE_h && i + 1 < phone_count &&
            phones[i + 1].same_word_left && vowel(phones[i + 1].phone))
            span = node->stress == 1 ? 80 : 75;
        if (nasal_context == 1) span = nasal_coda.duration_ticks;
        if (shape && i + 2 < phone_count &&
            phones[i + 2].same_word_left &&
            phones[i + 2].same_syllable_left &&
            !phones[i + 2].same_word_right) {
            struct reloq_german_generic_cluster_stop following = {0};
            int32_t vowel_ticks = 0;
            const int pair = reloq_german_generic_cluster_stop(
                phones + i + 1, 1, &following);
            const int context = reloq_german_generic_cluster_vowel_ticks(
                node, &vowel_ticks);
            if (pair < 0 || context < 0) goto cleanup;
            if (pair && context) span = vowel_ticks;
        }
        struct reloq_german_generic_diphthong diph = {0};
        size_t word_end = i;
        if (shape && shape->phone >= RELOQ_DE_PHONE_Q_aj &&
            shape->phone <= RELOQ_DE_PHONE_Q_oj)
            while (word_end + 1 < phone_count &&
                   phones[word_end + 1].word_index == node->word_index)
                ++word_end;
        const int connected_word = word_end + 1 < phone_count &&
            (!marks || marks[node->word_index] ==
                       RELOQ_DE_TERMINAL_IMPLICIT);
        int is_diphthong = reloq_german_generic_diphthong_plan(
            node, connected_word, &diph);
        if (is_diphthong < 0) goto cleanup;
        if (is_diphthong) span = diph.duration_ticks;
        struct reloq_german_generic_stop_timing stop_timing = {0};
        int calibrated_stop =
            stop_phase_active?0:reloq_german_generic_stop_onset_timing(node, &stop_timing);
        if (calibrated_stop < 0) goto cleanup;
        struct reloq_german_generic_internal_stop_pair internal_pair = {0};
        const int internal_onset = !stop_phase_active && i >= 2 ?
            reloq_german_generic_internal_stop_pair(
                phones, phone_count, i - 1, &internal_pair) : 0;
        if (internal_onset < 0) goto cleanup;
        if (internal_onset) {
            stop_timing = internal_pair.onset;
            calibrated_stop = 1;
        }
        if (calibrated_stop) span = stop_timing.total_ticks;
        const int word_initial = !i || !node->same_word_left;
        int32_t connected_burst_tick = 0;
        const int connected_onset = boundary_pause || stop_phase_active ? 0 :
            reloq_german_generic_connected_onset(node,
                                                  &connected_burst_tick);
        if (connected_onset < 0) goto cleanup;
        int32_t connected_shift = 0;
        if (connected_onset) {
            connected_shift = connected_burst_tick -
                (calibrated_stop ? stop_timing.burst_at_ticks : 5);
        }
        if (connected_onset && calibrated_stop) {
            const int32_t shift = connected_shift;
            stop_timing.burst_at_ticks += shift;
            stop_timing.aspiration_at_ticks += shift;
            stop_timing.total_ticks += shift;
            /* The /k/ coda's release already contributes aspiration to a
             * following /t/. Preserve its clear burst but shorten the
             * second post-burst breath to avoid a doubled, airy transition. */
            if (node->left_phone == RELOQ_DE_PHONE_k &&
                node->phone == RELOQ_DE_PHONE_t) {
                const int32_t after_burst = stop_timing.total_ticks -
                    stop_timing.aspiration_at_ticks;
                stop_timing.total_ticks =
                    stop_timing.aspiration_at_ticks + after_burst * 3 / 5;
            }
            span = stop_timing.total_ticks;
        } else if (connected_onset) span += connected_shift;
        struct reloq_german_generic_connected_coda connected = {0};
        int32_t pause_after = 0;
        if (marks && !node->same_word_right &&
            reloq_german_generic_prosody_boundary_ticks(
                marks[node->word_index], &pause_after)) goto cleanup;
        int connected_coda = pause_after ? 0 :
            reloq_german_generic_connected_coda(node, &connected);
        if (connected_coda < 0) goto cleanup;
        const int internal_coda =
            reloq_german_generic_internal_stop_pair(
                phones, phone_count, i, &internal_pair);
        if (internal_coda < 0) goto cleanup;
        if (internal_coda) {
            connected = (struct reloq_german_generic_connected_coda){
                internal_pair.coda_ticks,
                internal_pair.coda_burst_at_ticks,
                internal_pair.merge_coda_with_onset
            };
            connected_coda = 1;
        }
        if (connected_coda) span = connected.span_ticks;
        struct reloq_german_generic_cluster_stop cluster = {0};
        int cluster_first = 0, cluster_last = 0;
        int32_t inherited_release = 0;
        /* The calibration only applies to a complete stressed VCC ending
         * the word.  Onsets, /st/ groups and longer codas retain their own
         * planner instead of inheriting the two-stop source geometry. */
        if (i + 1 < phone_count &&
            (i + 2 == phone_count ||
             phones[i + 2].word_index != node->word_index))
            cluster_first = reloq_german_generic_cluster_stop(
                node, 1, &cluster);
        if (!cluster_first && i >= 2 &&
                 phones[i - 2].word_index == node->word_index &&
                 vowel(phones[i - 2].phone) &&
                 phones[i - 1].same_syllable_left) {
            struct reloq_german_generic_cluster_stop first_fit = {0};
            const int matching_first = reloq_german_generic_cluster_stop(
                phones + i - 1, 1, &first_fit);
            int32_t matching_vowel_ticks = 0;
            const int matching_vowel =
                reloq_german_generic_cluster_vowel_ticks(
                    phones + i - 2, &matching_vowel_ticks);
            if (matching_first < 0 || matching_vowel < 0) goto cleanup;
            if (matching_first && matching_vowel) {
                struct reloq_german_phone_node contextual = *node;
                if (pause_after)
                    contextual.right_phone = RELOQ_DE_PHONE_NONE;
                cluster_last = reloq_german_generic_cluster_stop(
                    &contextual, 0, &cluster);
                if (cluster_last && stop_phase_active && !previous_cluster)
                    inherited_release=first_fit.carry_aspiration_ticks?
                        first_fit.carry_aspiration_ticks:1;
            }
        }
        if (cluster_first < 0 || cluster_last < 0) goto cleanup;
        if (cluster_first || cluster_last) span = cluster.span_ticks;
        const int syllable_onset = word_initial ||
            (node->same_syllable_right && vowel(node->right_phone));
        const int affricate = node->phone == RELOQ_DE_PHONE_P ||
            node->phone == RELOQ_DE_PHONE_T ||
            node->phone == RELOQ_DE_PHONE_C ||
            node->phone == RELOQ_DE_PHONE_J;
        struct reloq_german_generic_voiced_onset_timing voiced_timing = {0};
        const int calibrated_voiced = !affricate && !stop_phase_active ?
            reloq_german_generic_voiced_onset_timing(after_vocalic_r?phones+i:node, &voiced_timing) : 0;
        if (calibrated_voiced < 0) goto cleanup;
        if (calibrated_voiced) span=voiced_timing.total_ticks;
        else if (!stop_phase_active && word_initial && voiced_stop(node->phone) && !affricate) {
            span = node->phone == RELOQ_DE_PHONE_b ?
                (node->right_phone == RELOQ_DE_PHONE_I ||
                 node->right_phone == RELOQ_DE_PHONE_i ||
                 node->right_phone == RELOQ_DE_PHONE_U ||
                 node->right_phone == RELOQ_DE_PHONE_u ? 106 : 93) :
                node->phone == RELOQ_DE_PHONE_d ? 100 : 90;
        }
        if (!word_initial && affricate) span = 170;
        if (shape && !timing->span_ticks) {
            int32_t weak_ticks = 0;
            const size_t next_syllable = node->syllable_index + 1;
            const int one_syllable = node->syllable_in_word == 0 &&
                (next_syllable >= syllable_count ||
                 phones[syllables[next_syllable].first_phone].word_index !=
                     node->word_index);
            const int one_onset = i && node->same_word_left &&
                !phones[i - 1].same_word_left &&
                (node->left_phone == RELOQ_DE_PHONE_d ||
                 node->left_phone == RELOQ_DE_PHONE_b ||
                 node->left_phone == RELOQ_DE_PHONE_g ||
                 node->left_phone == RELOQ_DE_PHONE_z ||
                 node->left_phone == RELOQ_DE_PHONE_v ||
                 node->left_phone == RELOQ_DE_PHONE_m ||
                 node->left_phone == RELOQ_DE_PHONE_n);
            const int simple_coda = !node->same_word_right ||
                (i + 1 < phone_count && node->same_syllable_right &&
                 !phones[i + 1].same_word_right);
            const int weak = reloq_german_generic_prosody_weak_vowel_ticks(
                node, shape->duration,
                one_syllable && one_onset && simple_coda, &weak_ticks);
            if (weak < 0) goto cleanup;
            if (weak) span = weak_ticks;
        }
        if (timing->span_ticks) {
            span=timing->span_ticks;
            if (is_diphthong) {
                diph.glide_ticks=timing->glide_ticks;
                diph.endpoint_hold_ticks=timing->closing_ticks;
            }
        }
        /* A completed phrase cannot resyllabify its final nasal into the
         * next phrase's vowel. Use the bounded acoustic context for that
         * duration decision, as for its tract and source envelope. */
        int32_t source_duration=phrase_final?
            reloq_german_source_consonant_ticks(node,1,0):
            reloq_german_source_consonant_ticks(phones,phone_count,i);
        if (boundary_pause && node->same_syllable_right && vowel(node->right_phone)) {
            /* A silent phrase boundary ends the previous consonant group.
             * In particular n after n must recover the full fresh onset. */
            struct reloq_german_phone_node fresh=*node;
            fresh.left_phone=RELOQ_DE_PHONE_NONE;
            source_duration=reloq_german_source_consonant_ticks(&fresh,1,0);
        }
        if (!boundary_pause && node->phone==RELOQ_DE_PHONE_S &&
            (node->left_phone==RELOQ_DE_PHONE_s || node->left_phone==RELOQ_DE_PHONE_S))
            source_duration=60;
        if (source_duration && !node->affricate_role) span=source_duration;
        const int affricate_part=node->affricate_role;
        const size_t affricate_head=i-(affricate_part && !(affricate_part&1) && i?1:0);
        const size_t after_affricate=affricate_head+2;
        const int affricate_following_final=after_affricate<phone_count &&
            !phones[after_affricate].same_word_right &&
            (after_affricate+1==phone_count || (marks && marks[phones[after_affricate].word_index]!=RELOQ_DE_TERMINAL_IMPLICIT));
        if (reloq_german_affricate_context_ticks(phones,phone_count,i,phrase_final,
                affricate_head==phrase_start_phone,affricate_following_final,&span)<0)
            goto cleanup;
        const int terminal_stop=phrase_final && coda_after_vowel &&
            (node->phone==RELOQ_DE_PHONE_p || node->phone==RELOQ_DE_PHONE_t ||
             node->phone==RELOQ_DE_PHONE_k);
        if (terminal_stop) span=node->phone==RELOQ_DE_PHONE_p?125:
            node->phone==RELOQ_DE_PHONE_t?108:
            node->left_phone==RELOQ_DE_PHONE_AT?72:122;
        if ((after_vocalic_r || after_coda_l) && phrase_final) {
            if (node->phone==RELOQ_DE_PHONE_t) span=after_coda_l?65:93;
            else if (node->phone==RELOQ_DE_PHONE_m) span=after_coda_l?120:105;
            else if (node->phone==RELOQ_DE_PHONE_n) span=after_coda_l && !node->stress?110:95;
            else if (node->phone==RELOQ_DE_PHONE_f) span=150;
            else if (node->phone==RELOQ_DE_PHONE_s) span=after_coda_l?125:155;
        }
        if (cluster_active) span=onset.body_ticks+onset.release_ticks;
        if (stop_phase_active) span=stop_phase.gesture.body_ticks+stop_phase.gesture.release_ticks;
        struct reloq_german_coda_timing coda_timing={0};
        const int next_final=i+1<phone_count && !phones[i+1].same_word_right &&
            (i+2==phone_count || (marks && marks[phones[i+1].word_index]!=RELOQ_DE_TERMINAL_IMPLICIT));
        int coda_timed=phrase_final || cluster_active || stop_phase_active?0:
            reloq_german_coda_timing_plan(phones,phone_count,i,next_final,&coda_timing);
        /* A glottal weak/secondary short-u onset keeps the preceding s
         * gesture separate. A secondary syllable must not select the
         * generic 64-tick internal coda merely because it was not moved
         * into the following unstressed syllable by phone analysis. */
        if(!coda_timed && !affricate_part && node->phone==RELOQ_DE_PHONE_s &&
            node->same_word_right && !node->same_syllable_right && i+1<phone_count &&
            phones[i+1].phone==RELOQ_DE_PHONE_U && phones[i+1].stress!=1 &&
            reloq_german_glottal_onset(phones+i+1,
                feature_index<feature_count?features+feature_index:NULL)) {
            coda_timing=(struct reloq_german_coda_timing){phones[i+1].stress==2?100:85,0};
            coda_timed=1;
        }
        if (!coda_timed && !phrase_final && after_coda_l &&
            node->phone==RELOQ_DE_PHONE_n && i+1<phone_count &&
            node->same_word_right && !node->same_syllable_right &&
            phones[i+1].phone==RELOQ_DE_PHONE_d) {
            coda_timing=(struct reloq_german_coda_timing){node->stress?85:50,1};
            coda_timed=1;
        }
        /* Nasals closing a syllable before a glottal onset retain their
         * coda body. Resyllabified CV timing would truncate that release. */
        if(!phrase_final && coda_after_vowel && !node->same_syllable_right &&
            node->same_word_right && i+1<phone_count &&
            (node->phone==RELOQ_DE_PHONE_n || node->phone==RELOQ_DE_PHONE_m || node->phone==RELOQ_DE_PHONE_G) &&
            reloq_german_glottal_onset(phones+i+1,
                feature_index<feature_count?features+feature_index:NULL)) {
            int body=node->phone==RELOQ_DE_PHONE_G?42:
                node->phone==RELOQ_DE_PHONE_n?(node->left_phone==RELOQ_DE_PHONE_AT?72:50):
                node->left_phone==RELOQ_DE_PHONE_AT?52:
                node->left_phone==RELOQ_DE_PHONE_R?105:85;
            coda_timing=(struct reloq_german_coda_timing){body,0};coda_timed=1;
        }
        if (coda_timed<0) goto cleanup;
        if (!coda_timed && !phrase_final && !cluster_active && !stop_phase_active &&
            i+1<phone_count) {
            int32_t contact=0;
            int covered=reloq_german_transition_cc_ticks(node,phones+i+1,&contact);
            if (covered<0) goto cleanup;
            if (covered) {
                coda_timing=(struct reloq_german_coda_timing){span,contact};
                coda_timed=1;
            }
        }
        if (!coda_timed && affricate_part && !(affricate_part&1) &&
            node->same_word_right && i+1<phone_count &&
            !vowel(node->right_phone) && node->right_phone!=RELOQ_DE_PHONE_h) {
            struct reloq_german_phone_node contact=*node;
            contact.affricate_role=0;
            int32_t outgoing=0;
            if (reloq_german_transition_cc_ticks(&contact,phones+i+1,&outgoing)==1) {
                coda_timing=(struct reloq_german_coda_timing){span,outgoing};coda_timed=1;
            }
        }
        if (coda_timed) span=coda_timing.body_ticks+coda_timing.outgoing_ticks;
        /* The legacy first member leaves k-t/t-k aspiration in the next
         * member's time budget. A phase-owned closure begins after that
         * contact; changing planners must not remove the carried breath. */
        if (inherited_release) {
            if (tick>INT64_MAX-inherited_release) goto cleanup;
            tick+=inherited_release;
            glot->onset=tick;
        }
        if (tick > INT64_MAX - span - 200) goto cleanup;
        if (node->syllable_index >= syllable_count) goto cleanup;
        struct reloq_german_generic_melody_syllable *beat =
            melody + node->syllable_index;
        if (beat->start_tick < 0) {
            beat->start_tick = tick;
            beat->onset_begin_tick = tick;
            beat->word_index = node->word_index;
            beat->first_phone = node->phone;
            beat->stress = node->stress;
            beat->prosody_control = node->prosody_control;
            if(node->prosody_tail)beat->prosody_control|=RELOQ_DE_MELODY_TAIL;
            if(shape && i && !glot->fresh) {
                beat->onset_begin_tick=previous_pitch_begin;
                beat->onset_end_tick=previous_pitch_end;
                beat->preceding_stress=phones[i-1].stress;
                beat->inherited_vowel=vowel(phones[i-1].phone)!=NULL;
            }
        }
        if (shape && beat->closure_tick == 0)
            beat->nucleus_tick = tick;
        if (shape) {
            beat->closure_tick = tick + span;
            beat->vowel_core_end_tick=tick+(timing->vowel_ticks?
                timing->incoming_ticks+timing->vowel_ticks:span);
        }
        else if (!syllables[node->syllable_index].nucleus_count &&
                 syllabic_sonorant(node->phone) && !beat->closure_tick) {
            beat->nucleus_tick = tick;
            beat->closure_tick = tick + span;
        }
        if (shape) vowel_profile(&current, shape, node);
        else consonant_profile(&current, node, pitch(node));
        /* The louder run-on s/x applies only without a prosodic break. */
        if (!shape && phrase_final) {
            if (node->phone==RELOQ_DE_PHONE_s && current.value[RELOQ_TARGET_AF]==65)
                current.value[RELOQ_TARGET_AF]=62;
            else if (node->phone==RELOQ_DE_PHONE_x && !node->same_syllable_right &&
                     current.value[RELOQ_TARGET_AF]==55)
                current.value[RELOQ_TARGET_AF]=52;
            else if (node->phone==RELOQ_DE_PHONE_X && !node->same_syllable_right &&
                     current.value[RELOQ_TARGET_AF]==62)
                current.value[RELOQ_TARGET_AF]=57;
            else if (node->phone==RELOQ_DE_PHONE_Z)
                current.value[RELOQ_TARGET_AF]=54;
        }
        struct reloq_german_nucleus_acoustics nucleus;
        const size_t next_syllable = node->syllable_index + 1;
        const int last_in_phrase = next_syllable == syllable_count ||
            (next_syllable < syllable_count && marks &&
             phones[syllables[next_syllable].first_phone].word_index !=
                 node->word_index &&
             marks[node->word_index] != RELOQ_DE_TERMINAL_IMPLICIT);
        struct reloq_german_phone_node nucleus_context=*node;
        /* Use the requested brighter a only at the end of the entire
         * utterance, never before another spoken word. The schedule
         * gives it a duration between contextual short and long a.
         * Only normal-text metadata selects this voice preference. */
        if (node->phone==RELOQ_DE_PHONE_A && !node->stress &&
            i+1==phone_count && feature_index<feature_count &&
            features[feature_index].word_index==node->word_index &&
            node->syllable_in_word<64 &&
            ((features[feature_index].full_a_syllables>>node->syllable_in_word)&1u))
            nucleus_context.phone=RELOQ_DE_PHONE_a;
        if (shape && i>=2 && stop_before_h(phones,phone_count,i-2,marks)) {
            nucleus_context.left_phone=phones[i-2].phone;
            nucleus_context.same_word_left=1;
            if (is_diphthong) vowel_profile(&current,shape,&nucleus_context);
        }
        /* h has no oral target: the adjacent vowel remains the nucleus's
         * right articulation context while aspiration spans the interval. */
        if (node->same_word_right && node->right_phone==RELOQ_DE_PHONE_h &&
            i+2<phone_count && phones[i+1].same_syllable_right &&
            vowel(phones[i+2].phone))
            nucleus_context.right_phone=phones[i+2].phone;
        /* A connected word edge retains the stop's place into the next
         * vowel. Only a real pause selects an independent vowel onset. */
        if (!boundary_pause && !node->same_word_left &&
            (node->left_phone==RELOQ_DE_PHONE_p ||
             node->left_phone==RELOQ_DE_PHONE_t ||
             node->left_phone==RELOQ_DE_PHONE_k)) {
            nucleus_context.same_word_left=1;
            if (is_diphthong) {
                /* A diphthong's first vowel retains the same stop place
                 * through a connected word edge, including a glottal gesture. */
                nucleus_context.same_syllable_left=1;
                vowel_profile(&current,shape,&nucleus_context);
            }
        }
        const int nucleus_active = nucleus_plan(
            &nucleus_context, !node->stress && last_in_phrase, &nucleus);
        if (nucleus_active < 0) goto cleanup;
        if (nucleus_active && nucleus_context.phone!=node->phone) {
            struct reloq_german_nucleus_acoustics short_a;
            struct reloq_german_phone_node shorter=nucleus_context;
            shorter.phone=node->phone;
            if (nucleus_plan(&shorter,!node->stress && last_in_phrase,&short_a)!=1)
                goto cleanup;
            /* Explicit voice preference: interpolate the contextual oral
             * targets, retaining the established intermediate duration. */
            for (unsigned j=0;j<5;++j) {
                const int32_t a=(short_a.end_mask&(1u<<j))?
                    short_a.departing[j]:short_a.entering[j];
                const int32_t b=(nucleus.end_mask&(1u<<j))?
                    nucleus.departing[j]:nucleus.entering[j];
                nucleus.entering[j]=(3*nucleus.entering[j]+short_a.entering[j])/4;
                nucleus.departing[j]=(a+3*b)/4;
            }
            nucleus.end_mask|=short_a.end_mask;
        }
        const int closing_rl=nucleus_active && i+2<phone_count &&
            phones[i+1].phone==RELOQ_DE_PHONE_r && timings[i+1].closing_ticks &&
            phones[i+2].phone==RELOQ_DE_PHONE_l && timings[i+2].closing_ticks;
        /* The i/I source spans the whole r-l nucleus. The isolated r
         * reduction does not apply; both vowels use the observed 57 dB
         * strong onset (54 dB in a weak final nucleus). */
        if (closing_rl && (node->phone==RELOQ_DE_PHONE_i || node->phone==RELOQ_DE_PHONE_I)) {
            nucleus.av+=5; nucleus.av_end+=5;
        }
        size_t nasal_index=i+2;
        if (closing_rl)
            nasal_index=i+3;
        if (nucleus_active && nasal_index<phone_count && timing->outgoing_ticks &&
            (phones[i+1].phone==RELOQ_DE_PHONE_r || phones[i+1].phone==RELOQ_DE_PHONE_j ||
             phones[i+1].phone==RELOQ_DE_PHONE_l) && timings[i+1].closing_ticks &&
            phones[i+1].same_word_right &&
            (phones[nasal_index].phone==RELOQ_DE_PHONE_m || phones[nasal_index].phone==RELOQ_DE_PHONE_n ||
             (phones[i+1].phone==RELOQ_DE_PHONE_l && phones[nasal_index].phone==RELOQ_DE_PHONE_G))) {
            struct reloq_german_nucleus_acoustics nasal;
            if (reloq_german_nucleus_acoustics_plan(node->phone,node->left_phone,
                phones[nasal_index].phone,node->stress,!node->stress && last_in_phrase,&nasal)==1)
            {
                for (unsigned j=0;j<5;++j) nucleus.bandwidth[j]=nasal.bandwidth[j];
                /* A closing r anticipates the same-syllable n endpoint.
                 * m and a nasal in the next syllable retain the r endpoint. */
                if(phones[i+1].phone==RELOQ_DE_PHONE_r &&
                   phones[nasal_index].phone==RELOQ_DE_PHONE_n && phones[nasal_index].same_syllable_left)
                    nucleus.av_end=closing_rl?48-(!node->stress && last_in_phrase?3:0):nasal.av_end;
                if (phones[i+1].phone==RELOQ_DE_PHONE_l &&
                    phones[nasal_index].phone==RELOQ_DE_PHONE_n && phones[nasal_index].same_syllable_left &&
                    node->phone==RELOQ_DE_PHONE_AT) {
                    nucleus.av-=3; nucleus.av_end-=3;
                }
            }
        }
        if (nucleus_active) nucleus_profile(&current, &nucleus);
        /* The release of a stop is still audible during the following /r/.
         * Keep the two excitation sources active while its own tract takes
         * over; the ordinary /r/ profile drops both sources too abruptly. */
        if (!shape && i && node->same_word_left &&
            node->phone == RELOQ_DE_PHONE_r &&
            is_stop(node->left_phone)) {
            current.value[RELOQ_TARGET_AH] = 56;
            current.value[RELOQ_TARGET_AF] = 54;
        }
        if (noisy_fricative(node->phone) && current.value[RELOQ_TARGET_AV]>0)
            current.value[RELOQ_TARGET_AV]+=RELOQ_GENERIC_SOURCE_TRIM;
        int32_t coda_formants[3] = {0};
        if (coda_after_vowel) {
            get_formants(&previous, coda_formants);
            set_formants(&current, coda_formants);
        }
        if (nasal_context == 1) {
            if (nasal_coda.f1_override)
                current.value[RELOQ_TARGET_F1] = nasal_coda.f1_override;
            current.value[RELOQ_TARGET_AV] = nasal_coda.start_av;
        }
        struct reloq_german_generic_cv_gesture cv = {0};
        int cv_applies = 0;
        if (!shape && i + 1 < phone_count && node->same_syllable_right) {
            const struct vowel_shape *following = vowel(phones[i + 1].phone);
            if (following) {
                struct profile next_vowel;
                int32_t f[3];
                vowel_profile(&next_vowel, following, phones + i + 1);
                get_formants(&next_vowel, f);
                cv_applies = reloq_german_generic_cv_gesture(node, f, &cv);
                if (cv_applies < 0) goto cleanup;
                if (cv_applies && cv.replace_consonant_tract)
                    set_formants(&current, cv.first_formant);
            }
        }
        if (!i && noisy_fricative(node->phone)) {
            previous=current;
            if (node->phone==RELOQ_DE_PHONE_v || node->phone==RELOQ_DE_PHONE_z)
                previous.value[RELOQ_TARGET_AV]=0;
        }
        struct reloq_german_nasal_tract nasal_tract;
        const int nasal_tract_active=cluster_active?0:nasal_tract_plan(phones,phone_count,i,node,
            !i || boundary_pause!=0,phrase_final,&nasal_tract);
        if (nasal_tract_active<0) goto cleanup;
        if (nasal_tract_active) nasal_profile(&current,&nasal_tract);
        struct reloq_german_obstruent_tract obstruent_tract;
        const int voiceless_tract_stop=node->phone==RELOQ_DE_PHONE_p ||
            node->phone==RELOQ_DE_PHONE_t || node->phone==RELOQ_DE_PHONE_k;
        int obstruent_tract_active=voiceless_tract_stop && !calibrated_stop &&
            !terminal_stop && !affricate_part?0:
            consonant_tract_plan(node,!i || boundary_pause!=0,phrase_final,
                i+2<phone_count && phones[i+1].same_word_right?phones[i+2].phone:RELOQ_DE_PHONE_NONE,
                &obstruent_tract);
        /* Nasals retain their own tract owner after vocalic r/l as well.
         * Its final m targets already include this context. A second
         * obstruent pass would append an earlier target after the core end. */
        if (cluster_active) {obstruent_tract=onset.tract;obstruent_tract_active=1;}
        if (stop_phase_active) {obstruent_tract=stop_phase.gesture.tract;obstruent_tract_active=1;}
        /* A vocalic r preserves the rounded nucleus's anticipatory
         * dental closure. This also applies inside ts and before another
         * syllable; terminal-stop selection alone misses those contacts. */
        if (after_vocalic_r && obstruent_tract_active &&
            node->phone==RELOQ_DE_PHONE_t && i>1) {
            const enum reloq_german_phone v=phones[i-2].phone;
            if (v==RELOQ_DE_PHONE_O) {obstruent_tract.entering[1]=1400;obstruent_tract.entering[2]=2350;}
            else if (v==RELOQ_DE_PHONE_o || v==RELOQ_DE_PHONE_u || v==RELOQ_DE_PHONE_U) {
                obstruent_tract.entering[1]=1300;obstruent_tract.entering[2]=2050;
            }
        }
        if (obstruent_tract_active && node->phone==RELOQ_DE_PHONE_r &&
            timing->closing_ticks && i+2<phone_count &&
            phones[i+1].phone==RELOQ_DE_PHONE_l && timings[i+1].closing_ticks &&
            phones[i+1].same_word_right &&
            (phones[i+2].phone==RELOQ_DE_PHONE_n || phones[i+2].phone==RELOQ_DE_PHONE_m))
            obstruent_tract.bandwidth[0]=270;
        if (obstruent_tract_active<0) goto cleanup;
        if (obstruent_tract_active) obstruent_profile(&current,&obstruent_tract);
        output.tract_owned=nasal_tract_active || obstruent_tract_active;
        struct reloq_german_source_segment *source=source_segments+i;
        source->begin=tick;source->end=tick+span;source->phrase_final=phrase_final;
        if (coda_timed) source->end=tick+coda_timing.body_ticks;
        if (stop_phase_active) source->end=tick+stop_phase.gesture.body_ticks;
        else if (cluster_active) source->end=tick+onset.body_ticks;
        source->incoming_ticks=timing->incoming_ticks;
        source->outgoing_ticks=timing->outgoing_ticks;
        const int outgoing_diphthong=shape && !node->stress && node->same_word_right &&
            !node->same_syllable_right && i+1<phone_count && !phones[i+1].stress &&
            phones[i+1].phone>=RELOQ_DE_PHONE_Q_aj && phones[i+1].phone<=RELOQ_DE_PHONE_Q_oj &&
            timing->outgoing_ticks;
        if(outgoing_diphthong)source->end-=timing->outgoing_ticks;
        const int vowel_contact=shape && i && node->same_word_left &&
            !node->same_syllable_left &&
            (vowel(node->left_phone) || (node->left_phone==RELOQ_DE_PHONE_j && timings[i-1].closing_ticks)) &&
            timing->incoming_ticks && !glot->active;
        if (vowel_contact) source->begin+=timing->incoming_ticks;
        source->kind=reloq_german_source_classify(phones,phone_count,i,
            nucleus_active || is_diphthong,phrase_final);
        if(!boundary_pause && node->phone==RELOQ_DE_PHONE_z &&
            !node->same_syllable_left && noisy_fricative(node->left_phone))
            source->kind=RELOQ_SOURCE_UNVOICED;
        if (boundary_pause && source->kind==RELOQ_SOURCE_NASAL_BRIDGE)
            source->kind=RELOQ_SOURCE_NASAL_ONSET;
        if (boundary_pause && node->phone==RELOQ_DE_PHONE_r && source_duration==75)
            source->kind=RELOQ_SOURCE_R_INITIAL;
        if (affricate_part==8) ++source->begin;
        if (source->kind==RELOQ_SOURCE_CARRY &&
            (!i || (source_segments[i-1].kind!=RELOQ_SOURCE_NUCLEUS &&
                    source_segments[i-1].kind!=RELOQ_SOURCE_CARRY)))
            source->kind=RELOQ_SOURCE_NONE;
        if (nucleus_active) {
            source->av=nucleus.av;source->av_end=nucleus.av_end;
            /* A new syllable's nasal/lateral is not the closing phone of
             * this nucleus. Short-u also keeps its ordinary source before
             * an onset s/z; its coda boost belongs within one syllable.
             * The incoming tract transition still applies in both cases. */
            if (!node->same_syllable_right &&
                (node->right_phone==RELOQ_DE_PHONE_m || node->right_phone==RELOQ_DE_PHONE_n ||
                 node->right_phone==RELOQ_DE_PHONE_G || node->right_phone==RELOQ_DE_PHONE_l ||
                 (node->phone==RELOQ_DE_PHONE_U &&
                  (node->right_phone==RELOQ_DE_PHONE_s || node->right_phone==RELOQ_DE_PHONE_z)))) {
                struct reloq_german_nucleus_acoustics plain;
                if (reloq_german_nucleus_acoustics_plan(nucleus_context.phone,
                    node->same_word_left?node->left_phone:RELOQ_DE_PHONE_NONE,
                    RELOQ_DE_PHONE_NONE,node->stress,!node->stress && last_in_phrase,&plain)==1) {
                    source->av=plain.av;source->av_end=plain.av_end;
                }
            }
        }
        else if (is_diphthong) {
            source->av=node->phone==RELOQ_DE_PHONE_Q_oj?53:50;
            source->av_end=node->phone==RELOQ_DE_PHONE_Q_aj?45:
                           node->phone==RELOQ_DE_PHONE_Q_aw?48:51;
            if (node->phone==RELOQ_DE_PHONE_Q_oj && node->same_syllable_right &&
                (node->right_phone==RELOQ_DE_PHONE_n || node->right_phone==RELOQ_DE_PHONE_G))
                source->av_end=48;
            if (!node->stress && last_in_phrase) {source->av-=3;source->av_end-=3;}
        }
        /* The fricative's steady source ends before its CC contact. Do not
         * extend zero voicing or full frication to the next phone's onset. */
        const int fricative_contact=(coda_timed ||
            (!phrase_final && i+1<phone_count && vowel(phones[i+1].phone) &&
             !phones[i+1].same_syllable_left)) &&
            (node->phone==RELOQ_DE_PHONE_s || node->phone==RELOQ_DE_PHONE_f ||
             node->phone==RELOQ_DE_PHONE_S || node->phone==RELOQ_DE_PHONE_x ||
             node->phone==RELOQ_DE_PHONE_X);
        if (fricative_contact) source->kind=RELOQ_SOURCE_UNVOICED;
        if (cluster_active || stop_phase_active) source->kind=RELOQ_SOURCE_NONE;
        const int rhotic_source=(source->kind==RELOQ_SOURCE_R_INITIAL ||
            source->kind==RELOQ_SOURCE_R_MEDIAL);
        const int quiet_nasal=(source->kind==RELOQ_SOURCE_NASAL_ONSET ||
            source->kind==RELOQ_SOURCE_NASAL_BRIDGE) && i && !vowel(node->left_phone);
        const int quiet_coda=coda_timed && source->kind==RELOQ_SOURCE_NASAL_CLUSTER;
        const int closing_sonorant=timing->closing_ticks && source->kind==RELOQ_SOURCE_CARRY;
        const int quiet_liquid=rhotic_source || quiet_nasal || quiet_coda ||
            closing_sonorant ||
            ((node->phone==RELOQ_DE_PHONE_l || node->phone==RELOQ_DE_PHONE_w) &&
             source->kind==RELOQ_SOURCE_LIQUID_ONSET);
        const int obstruent_nasal=source->kind==RELOQ_SOURCE_NASAL_FINAL &&
            node->same_syllable_left &&
            (is_stop(node->left_phone) || noisy_fricative(node->left_phone));
        if (quiet_liquid || obstruent_nasal) {
            for (size_t j=RELOQ_TARGET_AF;j<=RELOQ_TARGET_AB;++j) current.value[j]=0;
            if (emit_noise_end(&stages,tick,&output) ||
                emit_noise_end(&stages,quiet_coda?source->end:
                    tick+span-(closing_sonorant?timing->outgoing_ticks:0),&output)) goto cleanup;
        }
        const int phase_owned=cluster_active || stop_phase_active;
        output.nucleus_source=phase_owned || source->kind!=RELOQ_SOURCE_NONE;
        output.noise_owned=phase_owned || quiet_liquid || obstruent_nasal;
        output.cluster_source=phase_owned;
        if (!i && calibrated_voiced) {
            /* A leading voiced closure starts at the first frame. A
             * preceding same-tick silent point would mute that frame. */
            previous=current;
            (void)reloq_german_generic_voiced_stop_closure(node->phone,previous.value);
            previous.value[RELOQ_TARGET_AV]+=RELOQ_GENERIC_SOURCE_TRIM;
        }
        if (node->phone==RELOQ_DE_PHONE_Q_aj || node->phone==RELOQ_DE_PHONE_Q_oj) {
            current.value[RELOQ_TARGET_B1]=
                node->right_phone==RELOQ_DE_PHONE_n ||
                node->right_phone==RELOQ_DE_PHONE_m ||
                node->right_phone==RELOQ_DE_PHONE_G?270:120;
            current.value[RELOQ_TARGET_B3]=150;
        }
        if (!i && !calibrated_voiced) {
            /* The utterance starts directly on the first phone's tract
             * targets; there is no neutral frame before them. */
            for (size_t s = RELOQ_TARGET_F1; s <= RELOQ_TARGET_FTZ; ++s)
                previous.value[s] = current.value[s];
            /* An initial onset h is already aspirated in the first frame. */
            if (node->phone == RELOQ_DE_PHONE_h && phone_count > 1 &&
                phones[1].same_word_left && vowel(phones[1].phone))
                previous.value[RELOQ_TARGET_AH] = h_aspiration(phones,source_segments,0,1,last_in_phrase) +
                                                  RELOQ_GENERIC_SOURCE_TRIM;
        }
        struct output_settings boundary_output=output;
        /* This knot holds the preceding phone at the new boundary. Its
         * anatomy and source defaults still belong to that preceding phone. */
        boundary_output.model_owner_plus_one=i && !boundary_pause?i:0;
        boundary_output.nucleus_source=phase_owned || quiet_liquid ||
            (i?previous_source_owned:output.nucleus_source);
        boundary_output.noise_owned=phase_owned || quiet_liquid || obstruent_nasal || previous_cluster || previous_noise_contact;
        boundary_output.cluster_source=phase_owned || previous_cluster;
        boundary_output.pause_tract=boundary_pause!=0;
        boundary_output.tract_owned=output.tract_owned || (previous_tract_owned && !boundary_pause);
        /* Nucleus landmarks own their tract, including the interval of a
         * glottal gesture. Repeating the old profile at the new onset would
         * hold the old mouth shape through that interval and then jump. */
        const int nasal_oral_handoff=i && !boundary_pause && node->same_word_left &&
            node->left_phone>=RELOQ_DE_PHONE_Q_a_TILDE &&
            node->left_phone<=RELOQ_DE_PHONE_Q_oe_TILDE && noisy_fricative(node->phone);
        /* Nasal resonance leaves the preceding vowel at its last nucleus
         * landmark. Repeating its pole at the fricative boundary holds the
         * old resonance until a zero-width jump into the oral consonant. */
        boundary_output.nucleus_tract=((nucleus_active || is_diphthong) && i &&
            vowel(node->left_phone) && !boundary_pause) || nasal_oral_handoff;
        if (boundary_pause && !reloq_german_terminal_continues(marks[phones[i-1].word_index])) {
            /* A completed phrase has its own curve origin. The old quiet
             * profile belongs before this frame, not on the new onset. */
            boundary_output.nucleus_source=boundary_output.noise_owned=1;
            boundary_output.cluster_source=boundary_output.tract_owned=1;
        }
        if (emit(&stages,tick,&previous,&boundary_output)) goto cleanup;
        if (previous_noise_contact && !phase_owned && !quiet_liquid && !obstruent_nasal &&
            emit_source_range(&stages,tick+(shape?timing->incoming_ticks:0),&current,&output,
                              RELOQ_TARGET_AF,RELOQ_TARGET_AB)) goto cleanup;
        previous_noise_contact=fricative_contact || quiet_coda || closing_sonorant;
        previous_source_owned=output.nucleus_source;
        previous_tract_owned=output.tract_owned;
        previous_cluster=phase_owned;
        if (phase_owned) {
            /* A consonant-to-j source starts at the previous oral end.
             * Its tract still begins after the context's contact interval. */
            if (cluster_active && (node->phone==RELOQ_DE_PHONE_j || node->phone==RELOQ_DE_PHONE_w) && i &&
                !boundary_pause && node->same_word_left && !vowel(node->left_phone) &&
                !((vowel(node->right_phone)!=NULL) && node->same_syllable_left &&
                  (node->left_phone==RELOQ_DE_PHONE_p || node->left_phone==RELOQ_DE_PHONE_t ||
                   node->left_phone==RELOQ_DE_PHONE_k)) &&
                source_segments[i-1].end<=tick) {
                onset.source_lead_ticks=(int32_t)(tick-source_segments[i-1].end);
                onset.source_lead_tl=1;
            }
            const struct reloq_german_onset_cluster *gesture=stop_phase_active?&stop_phase.gesture:&onset;
            struct cluster_output context={&stages,&output};
            if (inherited_release && emit_source_range(&stages,tick,&previous,
                &output,RELOQ_TARGET_AV,RELOQ_TARGET_AH)) goto cleanup;
            if (reloq_german_onset_cluster_emit(gesture,tick,i,emit_cluster_point,&context)) goto cleanup;
            current.value[RELOQ_TARGET_AV]=gesture->av;
            current.value[RELOQ_TARGET_AH]=gesture->release_ah>=0?gesture->release_ah:gesture->ah;
            current.value[RELOQ_TARGET_AF]=gesture->af;
            current.value[RELOQ_TARGET_TL]=gesture->tl;
            for (unsigned j=0;j<6;++j) current.value[RELOQ_TARGET_A1F+j]=gesture->noise[j];
            previous=current;
        } else if (is_stop(node->phone)) {
            struct profile closure;
            silence(&closure);
            closure.value[RELOQ_TARGET_F0] = pitch(node);
            if (cv_applies && cv.replace_consonant_tract)
                set_formants(&closure, cv.first_formant);
            else if (coda_after_vowel)
                set_formants(&closure, coda_formants);
            stop_burst(&burst, node, pitch(node), syllable_onset,
                       RELOQ_DE_GENERIC_BURST);
            if (cv_applies && cv.replace_consonant_tract)
                set_formants(&burst, cv.first_formant);
            else if (coda_after_vowel)
                set_formants(&burst, coda_formants);
            if ((affricate_part&1) || terminal_stop) {
                const int32_t burst_ticks=affricate_part?
                    (affricate_part==1 || affricate_part==7?10:5):
                    node->phone==RELOQ_DE_PHONE_p?10:node->phone==RELOQ_DE_PHONE_t?8:12;
                if (affricate_part) {
                    burst.value[RELOQ_TARGET_AF]=affricate_part==1?60:
                        affricate_part==7?51:66;
                    for (unsigned j=RELOQ_TARGET_A1F;j<=RELOQ_TARGET_AB;++j)
                        burst.value[j]=0;
                    burst.value[affricate_part==1?RELOQ_TARGET_AB:RELOQ_TARGET_A5F]=
                        affricate_part==1?67:affricate_part==7?55:affricate_part==5?50:53;
                }
                if (emit(&stages,tick,&closure,&output) ||
                    emit_stop_plateau(&stages,tick+span-burst_ticks,tick+span,
                                      &burst,&closure,&output)) goto cleanup;
                if (affricate_part && affricate_part!=7 &&
                    emit_noise_end(&stages,quiet_coda?source->end:tick+span,&output)) goto cleanup;
                previous=affricate_part && affricate_part!=7?closure:burst;
            } else if (word_initial && voiced_stop(node->phone) && !affricate) {
                (void)reloq_german_generic_voiced_stop_closure(
                    node->phone, closure.value);
                /* Keep the measured AV=20 inside the low-level closure;
                 * the generic voice's normal source trim belongs to the
                 * full vowel and burst, not this already weak murmur. */
                closure.value[RELOQ_TARGET_AV] += RELOQ_GENERIC_SOURCE_TRIM;
                const int32_t burst_at = calibrated_voiced ?
                    voiced_timing.burst_at_ticks : span - 15;
                /* Maintain the low voiced source throughout the closure.
                 * A single early closure target followed by an unvoiced
                 * burst target linearly fades AV/TL across the whole stop. */
                if (emit(&stages, tick, &closure, &output) ||
                    emit(&stages, tick + burst_at, &closure, &output) ||
                    emit(&stages, tick + burst_at, &burst, &output) ||
                    emit(&stages, tick + span, &burst, &output))
                    goto cleanup;
                previous = burst;
            } else if (word_initial && affricate) {
                struct profile frication;
                stop_burst(&frication, node, pitch(node), 1,
                           RELOQ_DE_GENERIC_FRICATION);
                if (cv_applies && cv.replace_consonant_tract)
                    set_formants(&frication, cv.release_formant);
                if (emit(&stages, tick + 5, &burst, &output) ||
                    emit(&stages, tick + 20, &frication, &output) ||
                    emit(&stages, tick + span - 8, &frication, &output))
                    goto cleanup;
                previous = frication;
            } else if (word_initial) {
                /* A voiceless initial stop has a brief noise burst followed
                 * by aspiration. Residual frication fades during that
                 * interval rather than stopping at aspiration onset. */
                struct profile aspiration;
                stop_burst(&aspiration, node, pitch(node), 1,
                           RELOQ_DE_GENERIC_ASPIRATION);
                if (cv_applies && cv.replace_consonant_tract)
                    set_formants(&aspiration, cv.release_formant);
                const int burst_end = calibrated_stop ?
                    stop_timing.aspiration_at_ticks :
                    connected_shift +
                    (node->phone == RELOQ_DE_PHONE_p ? 15 :
                    node->phone == RELOQ_DE_PHONE_t &&
                    (node->right_phone == RELOQ_DE_PHONE_e ||
                     node->right_phone == RELOQ_DE_PHONE_i ||
                     node->right_phone == RELOQ_DE_PHONE_I) ? 23 : 17);
                struct profile end_aspiration = aspiration;
                if (cv_applies && cv.anticipate_during_consonant) {
                    struct profile next_vowel;
                    vowel_profile(&next_vowel, vowel(node->right_phone),
                                  phones + i + 1);
                    int32_t f[3];
                    get_formants(&next_vowel, f);
                    set_formants(&end_aspiration, f);
                }
                if ((connected_onset &&
                     emit(&stages, tick + 1, &closure, &output)) ||
                    emit_stop_plateau(&stages, tick +
                                      (connected_onset ?
                                       connected_burst_tick : 5),
                                      tick + burst_end, &burst,
                                      &aspiration, &output) ||
                    emit_selected(&stages, tick + span - 8, &end_aspiration,
                         &output, 1) ||
                    emit_noise_end(&stages, tick + span, &output)) goto cleanup;
                previous = end_aspiration;
            } else if (affricate) {
                struct profile frication;
                stop_burst(&frication, node, pitch(node), syllable_onset,
                           RELOQ_DE_GENERIC_FRICATION);
                if (cv_applies && cv.replace_consonant_tract)
                    set_formants(&frication, cv.release_formant);
                if (emit(&stages, tick + 5, &closure, &output) ||
                    emit(&stages, tick + 55, &burst, &output) ||
                    emit(&stages, tick + 70, &frication, &output) ||
                    emit(&stages, tick + span - 8, &frication, &output))
                    goto cleanup;
                previous = frication;
            } else if (syllable_onset) {
                /* A stop after a syllable boundary remains an onset even
                 * within one word. The preceding closure is retained, then
                 * its place burst hands off to aspiration or weak voicing. */
                if (voiced_stop(node->phone)) {
                    (void)reloq_german_generic_voiced_stop_closure(
                        node->phone, closure.value);
                    closure.value[RELOQ_TARGET_AV] +=
                        RELOQ_GENERIC_SOURCE_TRIM;
                    const int32_t burst_at=calibrated_voiced?
                        voiced_timing.burst_at_ticks:span-15;
                    if (emit(&stages, tick + (calibrated_voiced?0:5), &closure, &output) ||
                        emit(&stages, tick + burst_at, &closure, &output) ||
                        emit(&stages, tick + burst_at, &burst, &output) ||
                        emit(&stages, tick + span, &burst, &output))
                        goto cleanup;
                    previous = burst;
                } else {
                    struct profile aspiration;
                    stop_burst(&aspiration, node, pitch(node), 1,
                               RELOQ_DE_GENERIC_ASPIRATION);
                    if (cv_applies && cv.replace_consonant_tract)
                        set_formants(&aspiration, cv.release_formant);
                    struct profile end_aspiration = aspiration;
                    if (cv_applies && cv.anticipate_during_consonant) {
                        struct profile next_vowel;
                        vowel_profile(&next_vowel, vowel(node->right_phone),
                                      phones + i + 1);
                        int32_t f[3];
                        get_formants(&next_vowel, f);
                        set_formants(&end_aspiration, f);
                    }
                    int64_t burst_tick = tick + (calibrated_stop ?
                        stop_timing.burst_at_ticks : span - 27);
                    int64_t aspiration_tick = tick + (calibrated_stop ?
                        stop_timing.aspiration_at_ticks : span - 12);
                    if (emit(&stages, tick + 5, &closure, &output) ||
                        emit_stop_plateau(&stages, burst_tick,
                                          aspiration_tick, &burst,
                                          &aspiration, &output) ||
                        emit_selected(&stages, tick + span - 8,
                             &end_aspiration, &output, 1) ||
                        emit_noise_end(&stages, tick + span, &output)) goto cleanup;
                    previous = end_aspiration;
                }
            } else if (cluster_first || cluster_last) {
                if (cluster_last)
                    set_formants(&closure, cluster.release_formants);
                struct profile release = burst;
                release.value[RELOQ_TARGET_AV] = 0;
                release.value[RELOQ_TARGET_AH] = 0;
                release.value[RELOQ_TARGET_AF] = cluster.af_level;
                for (size_t j = RELOQ_TARGET_A1F;
                     j <= RELOQ_TARGET_AB; ++j)
                    release.value[j] = 0;
                if (cluster.a2f_level)
                    release.value[RELOQ_TARGET_A2F] = cluster.a2f_level;
                if (cluster.a5f_level)
                    release.value[RELOQ_TARGET_A5F] = cluster.a5f_level;
                if (cluster.ab_level)
                    release.value[RELOQ_TARGET_AB] = cluster.ab_level;
                if (cluster_first && cluster.carry_aspiration_ticks == 0) {
                    int32_t start[3], at_burst[3];
                    get_formants(&closure, start);
                    for (int j = 0; j < 3; ++j)
                        at_burst[j] = start[j] +
                            (cluster.release_formants[j] - start[j]) *
                            (span - cluster.burst_ticks) / span;
                    set_formants(&release, at_burst);
                } else if (cluster_last)
                    set_formants(&release, cluster.release_formants);
                struct profile breath = closure;
                /* Keep the preceding vowel's coda tract through a carried
                 * breath. An immediate 500→300 Hz F1 handoff on /k→t/ made
                 * the 20-ms AH interval almost 10 dB weaker in our Klatt
                 * path despite equal AH targets; the following stop then
                 * moves to its own place tract at the closure landmark. */
                if (cluster_last || !cluster.carry_aspiration_ticks)
                    set_formants(&breath, cluster.release_formants);
                const int32_t ah = cluster_first ?
                    cluster.carry_aspiration_level :
                    cluster.following_aspiration_level;
                if (ah) breath.value[RELOQ_TARGET_AH] =
                    ah + RELOQ_GENERIC_SOURCE_TRIM;
                /* The carried k->t / t->k release owns AH up to the next
                 * closure. Its AF endpoint belongs to the burst end;
                 * inserting a same-tick zero would remove the observed
                 * interpolation toward that closure (do_release). */
                if (cluster_first && cluster.carry_aspiration_ticks)
                    for (size_t j=RELOQ_TARGET_AF;j<=RELOQ_TARGET_AB;++j)
                        breath.value[j]=release.value[j];
                const int32_t closure_at = cluster_last &&
                    phones[i - 1].phone == RELOQ_DE_PHONE_k &&
                    node->phone == RELOQ_DE_PHONE_t ? 20 :
                    cluster_last && phones[i - 1].phone == RELOQ_DE_PHONE_t &&
                    node->phone == RELOQ_DE_PHONE_k ? 30 : 0;
                if (closure_at && emit_source_range(&stages,tick+closure_at,
                        &previous,&output,RELOQ_TARGET_AV,RELOQ_TARGET_AH))
                    goto cleanup;
                if ((cluster_last && closure_at > 1 &&
                     previous.value[RELOQ_TARGET_AH] == 0) ||
                    emit(&stages, tick + closure_at, &closure, &output))
                    goto cleanup;
                if (cluster.merge_with_next) {
                    if (emit(&stages, tick + span, &breath, &output))
                        goto cleanup;
                    previous = breath;
                } else {
                    if (emit_stop_plateau(&stages,
                            tick + span - cluster.burst_ticks,
                            tick + span, &release, &breath,
                            &output)) goto cleanup;
                    previous = breath;
                }
            } else if (connected_coda) {
                if (emit(&stages, tick + 1, &closure, &output))
                    goto cleanup;
                if (connected.merge_with_next) {
                    /* Consecutive equal stops share the following onset's
                     * release; the coda supplies closure without a burst. */
                    previous = closure;
                } else {
                    struct profile before = burst;
                    before.value[RELOQ_TARGET_AV] = 0;
                    before.value[RELOQ_TARGET_AH] = 0;
                    before.value[RELOQ_TARGET_AF] = 0;
                    for (size_t j = RELOQ_TARGET_A1F;
                         j <= RELOQ_TARGET_AB; ++j)
                        before.value[j] = 0;
                    if (emit(&stages, tick + connected.burst_at_ticks,
                             &before, &output) ||
                        emit(&stages, tick + connected.burst_at_ticks,
                             &burst, &output) ||
                        emit(&stages, tick + span, &burst, &output))
                        goto cleanup;
                    previous = burst;
                }
            } else {
                if (emit(&stages, tick + (coda_after_vowel ? 1 : 5),
                         &closure, &output) ||
                    emit(&stages, tick + span - 23, &burst, &output)) goto cleanup;
                previous = burst;
            }
        } else {
            int32_t incoming_formants[5];
            for (unsigned j = 0; j < 5; ++j)
                incoming_formants[j] = previous.value[RELOQ_TARGET_F1 + j];
            output.nucleus_tract = nucleus_active;
            /* All vowels own their frication handoff separately from tract
             * and voicing knots, including incoming AND outgoing changes. */
            const int fricative_vowel = shape != NULL;
            const int incoming_noise = shape && i && node->same_word_left &&
                (node->same_syllable_left || connected_stop_vowel) &&
                (noisy_fricative(node->left_phone) || voiced_stop(node->left_phone) ||
                 (is_stop(node->left_phone) && timing->incoming_ticks>0) ||
                 (reloq_german_onset_cluster_plan(phones,phone_count,i-1,&onset)==1 && onset.af) ||
                 node->left_phone==RELOQ_DE_PHONE_P || node->left_phone==RELOQ_DE_PHONE_T ||
                 node->left_phone==RELOQ_DE_PHONE_C || node->left_phone==RELOQ_DE_PHONE_J);
            struct reloq_german_generic_vc_gesture coda;
            int anticipates = 0;
            int following_tract_owned=0;
            int32_t coda_before = outgoing_diphthong?timing->outgoing_ticks:0;
            if (shape && i + 1 < phone_count &&
                (timing->span_ticks ? timing->outgoing_ticks>0 : node->same_syllable_right) &&
                !vowel(phones[i + 1].phone)) {
                struct profile next_consonant;
                int32_t vf[3], cf[3];
                consonant_profile(&next_consonant, phones + i + 1,
                                  pitch(phones + i + 1));
                /* A labial nasal before the next vowel or an unpunctuated
                 * word edge keeps its 300-Hz closure in the preceding vowel
                 * handoff. Otherwise its 200-Hz terminal place would make a
                 * spurious one-tick formant jump at the nasal onset. */
                const struct reloq_german_phone_node *next = phones + i + 1;
                const int next_phrase_final = !next->same_word_right &&
                    (i + 2 == phone_count ||
                     (marks && marks[next->word_index] !=
                               RELOQ_DE_TERMINAL_IMPLICIT));
                struct reloq_german_generic_nasal_coda next_nasal = {0};
                const int next_context =
                    reloq_german_generic_nasal_coda_plan(
                        next, 1, next_phrase_final,
                        i + 2 < phone_count && next->same_word_right &&
                            vowel(phones[i + 2].phone),
                        current.value[RELOQ_TARGET_AV],
                        RELOQ_GENERIC_SOURCE_TRIM, &next_nasal);
                if (next_context < 0) goto cleanup;
                if (next_context == 1 && next_nasal.f1_override)
                    next_consonant.value[RELOQ_TARGET_F1] =
                        next_nasal.f1_override;
                struct reloq_german_nasal_tract next_tract;
                if (nasal_tract_plan(phones,phone_count,i+1,next,0,next_phrase_final,&next_tract)==1) {
                    nasal_profile(&next_consonant,&next_tract);
                    following_tract_owned=1;
                }
                struct reloq_german_obstruent_tract next_obstruent;
                int next_obstruent_active=consonant_tract_plan(
                    next,0,next_phrase_final,
                    i+3<phone_count && phones[i+2].same_word_right?phones[i+3].phone:RELOQ_DE_PHONE_NONE,
                    &next_obstruent);
                if (next_obstruent_active<0) goto cleanup;
                /* Use the same owner for the incoming VC transition and
                 * the consonant itself, including silent stop phases. */
                {
                    struct reloq_german_onset_cluster next_cluster;
                    const int c=reloq_german_onset_cluster_plan(phones,phone_count,i+1,&next_cluster);
                    if (c<0) goto cleanup;
                    if (c==1) {next_obstruent=next_cluster.tract;next_obstruent_active=1;}
                    else {
                        struct reloq_german_stop_phase next_stop;
                        const int t=select_stop_phase(phones,phone_count,i+1,next,0,marks,&next_stop);
                        if (t<0) goto cleanup;
                        if (t==1) {next_obstruent=next_stop.gesture.tract;next_obstruent_active=1;}
                    }
                }
                if (next_obstruent_active) {
                    obstruent_profile(&next_consonant,&next_obstruent);
                    following_tract_owned=1;
                }
                get_formants(&current, vf);
                get_formants(&next_consonant, cf);
                struct reloq_german_phone_node contextual=*node;
                if (timing->span_ticks)
                    contextual.same_word_right=contextual.same_syllable_right=1;
                anticipates = reloq_german_generic_vc_gesture(
                    &contextual, phones + i + 1, vf, cf, &coda);
                if (anticipates < 0) goto cleanup;
                if (next_obstruent_active && timing->outgoing_ticks) {
                    anticipates=1;
                    coda.anticipation_ticks=timing->outgoing_ticks;
                    for (unsigned j=0;j<3;++j)
                        coda.coda_formant[j]=next_obstruent.entering[j]>0?
                            next_obstruent.entering[j]:vf[j];
                }
                if (anticipates) {
                    coda_before = coda.anticipation_ticks;
                    if (is_diphthong && diph.before_coda_ticks)
                        coda_before = diph.before_coda_ticks;
                    if (timing->span_ticks) coda_before=timing->outgoing_ticks;
                    else if (coda_before > span - 9) coda_before = span - 9;
                }
            }
            const int h_onset = !shape && node->phone == RELOQ_DE_PHONE_h &&
                i + 1 < phone_count && phones[i + 1].same_word_left &&
                vowel(phones[i + 1].phone);
            if (affricate_part==2) current.value[RELOQ_TARGET_AF]=60;
            if (affricate_part==4) {
                /* Use the accepted initial /ts/ release level in medial,
                 * final and connected positions too. Keep their own clocks
                 * and formant frequencies; reduced models still route this
                 * single branch onto their active upper parallel pole. */
                current.value[RELOQ_TARGET_AF]=65;
                current.value[RELOQ_TARGET_A5F]=55;
            }
            const int32_t initial_ticks=affricate_part==9?30:
                affricate_part && !(affricate_part&1)?
                (affricate_part==8?1:10):noisy_fricative(node->phone)?0:
                timing->span_ticks || coda_after_vowel ? 1 : 7;
            int32_t vowel_last_tick = initial_ticks;
            int incoming_glide = 0;
            struct reloq_german_generic_nasal_transition nasal_transition;
            int nasal = reloq_german_generic_nasal_transition(
                node, span, &nasal_transition);
            if (nasal < 0) goto cleanup;
            if (is_diphthong) {
                /* The vowel onset and end of its plateau are separate
                 * landmarks. A preceding stop must not erase the first. */
                vowel_last_tick=timing->span_ticks?timing->incoming_ticks:0;
                if (emit_vowel(&stages,tick+vowel_last_tick,&current,
                    &output,fricative_vowel)) goto cleanup;
            } else if (h_onset) {
                /* An onset h has no tract target of its own: the tract and
                 * parallel noise amplitudes glide linearly from the preceding
                 * phone to the vowel, while voicing stops and aspiration
                 * holds one measured level. */
                const int32_t level = h_aspiration(phones,source_segments,i,!i || boundary_pause!=0,last_in_phrase) +
                                      RELOQ_GENERIC_SOURCE_TRIM;
                struct profile target;
                struct reloq_german_phone_node next=phones[i+1];
                const int after_t=i && stop_before_h(phones,phone_count,i-1,marks);
                if (after_t) {
                    next.left_phone=phones[i-1].phone;
                    next.same_word_left=1;
                }
                vowel_profile(&target, vowel(next.phone), &next);
                target.value[RELOQ_TARGET_AV] = 0;
                target.value[RELOQ_TARGET_AF] = 0;
                target.value[RELOQ_TARGET_AH] = level;
                target.value[RELOQ_TARGET_F0] = current.value[RELOQ_TARGET_F0];
                struct profile start = i && !boundary_pause ? previous : target;
                start.value[RELOQ_TARGET_AV] = 0;
                start.value[RELOQ_TARGET_AF] = 0;
                if (after_t && node->same_word_left)
                    start.value[RELOQ_TARGET_AF]=previous.value[RELOQ_TARGET_AF];
                start.value[RELOQ_TARGET_AH] = level;
                start.value[RELOQ_TARGET_F0] = current.value[RELOQ_TARGET_F0];
                if ((i && !boundary_pause && emit_vowel(&stages, tick, &previous, &output, fricative_vowel)) ||
                    emit_vowel(&stages, tick, &start, &output, fricative_vowel) ||
                    emit_vowel(&stages, tick + span, &target, &output, fricative_vowel))
                    goto cleanup;
                current = target;
                vowel_last_tick = span;
            } else if (nasal == 1) {
                struct profile oral_start = current;
                oral_start.value[RELOQ_TARGET_FNZ] =
                    nasal_transition.initial_fnz;
                current.value[RELOQ_TARGET_FNZ] =
                    nasal_transition.final_fnz;
                if (emit_vowel(&stages, tick + (coda_after_vowel ? 1 : 7),
                               &oral_start, &output, fricative_vowel) ||
                    emit_vowel(&stages,
                               tick + nasal_transition.rise_after_ticks,
                               &current, &output, fricative_vowel))
                    goto cleanup;
                if (nasal_context == 1) {
                    if (nasal_coda.taper_percent) {
                        struct profile taper = current;
                        taper.value[RELOQ_TARGET_AV] = nasal_coda.taper_av;
                        if (emit_vowel(&stages,
                                       tick + span *
                                           nasal_coda.taper_percent / 100,
                                       &taper, &output, fricative_vowel))
                            goto cleanup;
                    }
                    current.value[RELOQ_TARGET_AV] = nasal_coda.end_av;
                }
            } else if (node->same_syllable_right &&
                (node->phone == RELOQ_DE_PHONE_v ||
                 node->phone == RELOQ_DE_PHONE_z)) {
                struct profile voiceless_start = current;
                voiceless_start.value[RELOQ_TARGET_AV] = 0;
                if (emit_vowel(&stages, tick,
                               &voiceless_start, &output, fricative_vowel) ||
                    emit_vowel(&stages, tick + span / 2, &current,
                               &output, fricative_vowel))
                    goto cleanup;
            } else if (vowel_contact) {
                incoming_glide=1;
                vowel_last_tick=timing->incoming_ticks;
                if (emit_vowel(&stages,tick+vowel_last_tick,&current,
                               &output,fricative_vowel)) goto cleanup;
            } else if ((shape || timing->span_ticks) && i &&
                       (node->same_syllable_left || connected_stop_vowel || (timing->incoming_ticks &&
                        node->same_word_left && (node->left_phone==RELOQ_DE_PHONE_G ||
                        node->left_phone==RELOQ_DE_PHONE_n))) &&
                       !vowel(phones[i - 1].phone)) {
                int32_t f[3];
                struct reloq_german_generic_cv_gesture onset;
                get_formants(&current, f);
                int active = reloq_german_generic_cv_gesture(
                    phones + i - 1, f, &onset);
                if (active < 0) goto cleanup;
                if (timing->span_ticks && timing->incoming_ticks) active=1;
                if (active) {
                    incoming_glide = 1;
                    struct profile beginning = current;
                    int32_t old_formants[3];
                    get_formants(&previous, old_formants);
                    set_formants(&beginning, old_formants);
                    int32_t rise = timing->span_ticks ? timing->incoming_ticks :
                        onset.vowel_glide_ticks;
                    if (!timing->span_ticks && rise > span - 9) rise = span - 9;
                    if (anticipates && rise >= span - coda_before)
                        rise = span - coda_before - 1;
                    if (rise < 1) rise = 1;
                    struct output_settings beginning_output=output;
                    if (incoming_tract_owned) beginning_output.tract_owned=1;
                    if (emit_vowel(&stages, tick + 1, &beginning,
                                   &beginning_output, fricative_vowel) ||
                        (rise > 1 && emit_vowel(&stages, tick + rise,
                                                &current, &output,
                                                fricative_vowel))) goto cleanup;
                    vowel_last_tick = rise;
                } else if (emit_vowel(&stages,
                                      tick + initial_ticks,
                                      &current, &output, fricative_vowel))
                    goto cleanup;
            } else if (emit_vowel(&stages, tick + initial_ticks,
                                  &current, &output, fricative_vowel))
                goto cleanup;
            if (is_diphthong) {
                struct profile glide = current;
                set_formants(&glide, diph.endpoint);
                if (node->phone==RELOQ_DE_PHONE_Q_aw) {
                    /* Closing w has its own upper resonance and damping.
                     * Folding two gestures must not retain the a plateau's
                     * narrow third pole throughout the glide. */
                    glide.value[RELOQ_TARGET_F4]=3200;
                    glide.value[RELOQ_TARGET_B3]=500;
                }
                if (node->phone==RELOQ_DE_PHONE_Q_oj) {
                    glide.value[RELOQ_TARGET_F4]=3400;
                    if (node->same_word_right) {
                        if (node->right_phone==RELOQ_DE_PHONE_n && !node->same_syllable_right)
                            glide.value[RELOQ_TARGET_F3]=2500;
                        if (node->right_phone==RELOQ_DE_PHONE_f || node->right_phone==RELOQ_DE_PHONE_S)
                            glide.value[RELOQ_TARGET_F2]=1800;
                        else if (node->right_phone==RELOQ_DE_PHONE_k || node->right_phone==RELOQ_DE_PHONE_X)
                            glide.value[RELOQ_TARGET_F2]=1950;
                    }
                }
                if (node->phone==RELOQ_DE_PHONE_Q_aj) {
                    glide.value[RELOQ_TARGET_F4]=3400;
                    if(node->same_word_right && node->right_phone==RELOQ_DE_PHONE_X)
                        glide.value[RELOQ_TARGET_F2]=1950;
                    const int nasal_right=node->right_phone==RELOQ_DE_PHONE_n ||
                        node->right_phone==RELOQ_DE_PHONE_m || node->right_phone==RELOQ_DE_PHONE_G;
                    glide.value[RELOQ_TARGET_B1]=nasal_right?270:120;
                    if (node->right_phone==RELOQ_DE_PHONE_m) {
                        glide.value[RELOQ_TARGET_F2]=1780;glide.value[RELOQ_TARGET_F3]=2400;
                    } else if (node->right_phone==RELOQ_DE_PHONE_n)
                        glide.value[RELOQ_TARGET_F3]=2500;
                }
                int32_t plateau = span - coda_before -
                                  diph.endpoint_hold_ticks;
                int32_t glide_at = plateau - diph.glide_ticks;
                if (glide_at <= vowel_last_tick)
                    glide_at = vowel_last_tick + 1;
                if (plateau <= glide_at) plateau = glide_at + 1;
                if (plateau >= span - coda_before)
                    plateau = span - coda_before - 1;
                if (glide_at >= plateau) glide_at = plateau - 1;
                if (glide_at <= vowel_last_tick || plateau <= glide_at ||
                    emit_vowel(&stages, tick + glide_at, &current,
                               &output, fricative_vowel) ||
                    emit_vowel(&stages, tick + plateau, &glide,
                               &output, fricative_vowel))
                    goto cleanup;
                current = glide;
            }
            struct reloq_german_generic_vowel_center center = {0};
            int center_active = shape && !nucleus_active ?
                reloq_german_generic_vowel_center(
                    node, current.value[RELOQ_TARGET_F2], &center) : 0;
            if (center_active < 0) goto cleanup;
            if (center_active && anticipates) {
                int32_t return_at = span - center.before_coda_ticks;
                const int32_t plateau_at = span - coda_before;
                if (return_at > plateau_at) return_at = plateau_at;
                /* Keep target times ordered even for reduced or fast
                 * vowels; there must be room after the CV rise. */
                if (return_at <= vowel_last_tick)
                    return_at = vowel_last_tick + 1;
                if (return_at > plateau_at) return_at = plateau_at;
                if (return_at > vowel_last_tick &&
                    return_at < plateau_at) {
                    struct profile middle = current;
                    middle.value[RELOQ_TARGET_F2] = center.neutral_f2;
                    if (emit_vowel(&stages, tick + return_at, &middle,
                                   &output, fricative_vowel))
                        goto cleanup;
                }
                current.value[RELOQ_TARGET_F2] = center.neutral_f2;
            }
            if (anticipates) {
                struct profile coda_end = current;
                set_formants(&coda_end, coda.coda_formant);
                if (is_diphthong)
                    coda_end.value[RELOQ_TARGET_AV] = diph.final_av;
                struct output_settings coda_output=output;
                coda_output.tract_owned=following_tract_owned;
                if (emit_vowel(&stages, tick + span - coda_before, &current,
                               &output, fricative_vowel) ||
                    emit_vowel(&stages, tick + span, &coda_end,
                               &coda_output, fricative_vowel))
                    goto cleanup;
                previous = coda_end;
            } else {
                if (is_diphthong)
                    current.value[RELOQ_TARGET_AV] = diph.final_av;
                const int before_vowel = !shape && node->same_word_right &&
                    node->same_syllable_right &&
                    palatal_fricative(node->phone) &&
                    i + 1 < phone_count && vowel(phones[i + 1].phone);
                if (before_vowel) {
                    struct profile following;
                    vowel_profile(&following, vowel(phones[i + 1].phone),
                                  phones + i + 1);
                    struct profile end = current;
                    /* Fronting and voicing step at the x/X -> vowel boundary.
                     * Keep the established frication timeline unchanged. */
                    end.value[RELOQ_TARGET_F1] =
                        current.value[RELOQ_TARGET_F1] +
                        (following.value[RELOQ_TARGET_F1] -
                         current.value[RELOQ_TARGET_F1]) * 40 / 100;
                    end.value[RELOQ_TARGET_F2] =
                        current.value[RELOQ_TARGET_F2] +
                        (following.value[RELOQ_TARGET_F2] -
                         current.value[RELOQ_TARGET_F2]) * 55 / 100;
                    /* The voiced source starts in the vowel. The fricative
                     * does not borrow an early vowel AV/AH step. */
                    if (emit_vowel(&stages, tick + span - 30, &current,
                                   &output, fricative_vowel) ||
                        emit_vowel(&stages, tick + span, &current,
                                   &output, fricative_vowel) ||
                        emit_vowel(&stages, tick + span, &end,
                                   &output, fricative_vowel))
                        goto cleanup;
                    previous = end;
                } else if (!h_onset) {
                    int32_t hold = outgoing_diphthong?span-timing->outgoing_ticks:
                        coda_timed?coda_timing.body_ticks:nasal_context == 1 &&
                        nasal_coda.end_av != nasal_coda.start_av ? span :
                        noisy_fricative(node->phone) ||
                        (is_diphthong && vowel(node->right_phone)) ||
                        span<=16 ? span : span-8;
                    /* A long incoming vowel contact can leave less than
                     * eight ticks of plateau. Its hold starts only after
                     * that contact; it cannot precede the target already
                     * emitted at the end of the glide. */
                    if (hold<vowel_last_tick) hold=vowel_last_tick;
                    if (emit_vowel(&stages,tick+hold,&current,&output,
                                   fricative_vowel)) goto cleanup;
                }
                if (center_active) {
                    current.value[RELOQ_TARGET_F2] = center.neutral_f2;
                    if (emit_vowel(&stages, tick + span, &current,
                                   &output, fricative_vowel))
                        goto cleanup;
                }
                if (!before_vowel) previous = current;
            }
            if (nucleus_active) {
                int32_t outgoing[5], handoff[5];
                int32_t left_at = timing->span_ticks?timing->incoming_ticks:vowel_last_tick;
                int32_t right_at = span - coda_before;
                if (left_at >= right_at) left_at = right_at - 1;
                if (left_at < 0) left_at = 0;
                if (right_at < left_at) right_at = left_at;
                for (unsigned j = 0; j < 5; ++j) {
                    outgoing[j] = nucleus.end_mask & (1u << j) ?
                        nucleus.departing[j] : nucleus.entering[j];
                    handoff[j] = outgoing[j];
                }
                if (incoming_glide && left_at > 1 && !incoming_tract_owned && !vowel_contact &&
                    emit_nucleus_tract(&stages, tick + 1,
                                       incoming_formants, &output))
                    goto cleanup;
                if (emit_nucleus_tract(&stages, tick + left_at,
                                       nucleus.entering, &output) ||
                    emit_nucleus_tract(&stages, tick + right_at,
                                       outgoing, &output))
                    goto cleanup;
                /* Bandwidth and nasal-pole targets start at the nucleus
                 * landmark. A preceding owned stop must not suppress them
                 * until a generic late-vowel anchor. */
                const int32_t resonance_at=timing->span_ticks?timing->incoming_ticks:left_at;
                if (emit_nucleus_resonance(&stages,tick+resonance_at,&nucleus,&output) ||
                    emit_nucleus_resonance(&stages,tick+right_at,&nucleus,&output)) goto cleanup;
                if (anticipates) {
                    for (unsigned j = 0; j < 3; ++j)
                        handoff[j] = coda.coda_formant[j];
                    if (!following_tract_owned && emit_nucleus_tract(&stages, tick + span,
                                           handoff, &output)) goto cleanup;
                }
                for (unsigned j = 0; j < 5; ++j)
                    previous.value[RELOQ_TARGET_F1 + j] = handoff[j];
            }
            output.nucleus_tract = 0;
            if (shape) {
                int32_t noise_end=(incoming_noise || vowel_contact)?vowel_last_tick:0;
                if (emit_noise_end(&stages,tick+noise_end,&output)) goto cleanup;
                struct profile next_noise;
                silence(&next_noise);
                const int outgoing_noise=anticipates && i+1<phone_count &&
                    noisy_fricative(phones[i+1].phone);
                if (outgoing_noise) {
                    struct reloq_german_phone_node following=phones[i+1];
                    if (!following.same_word_right && marks &&
                        marks[following.word_index]!=RELOQ_DE_TERMINAL_IMPLICIT)
                        following.right_phone=RELOQ_DE_PHONE_NONE;
                    consonant_profile(&next_noise,&following,pitch(&following));
                    struct reloq_german_onset_cluster noise_cluster;
                    if(reloq_german_onset_cluster_plan(phones,phone_count,i+1,&noise_cluster)==1) {
                        next_noise.value[RELOQ_TARGET_AF]=noise_cluster.af;
                        for(unsigned j=0;j<6;++j)
                            next_noise.value[RELOQ_TARGET_A1F+j]=noise_cluster.noise[j];
                    }
                }
                struct output_settings incoming_noise_output=output;
                incoming_noise_output.source_phone=i+1;
                int32_t steady_end=anticipates?span-coda_before:span;
                if (steady_end<noise_end) steady_end=noise_end;
                if (emit_noise_end(&stages,tick+steady_end,&output) ||
                    (outgoing_noise && emit_source_range(&stages,tick+span,
                        &next_noise,&incoming_noise_output,RELOQ_TARGET_AF,RELOQ_TARGET_AB))) goto cleanup;
                for (size_t j=RELOQ_TARGET_AF;j<=RELOQ_TARGET_AB;++j)
                    previous.value[j]=next_noise.value[j];
            }
        }
        if (nasal_tract_active) {
            struct nasal_output tract_output={&stages,&output};
            if (reloq_german_nasal_tract_append(&nasal_tract,tick,
                    tick+(coda_timed?coda_timing.body_ticks:span),i,
                    emit_nasal_tract,&tract_output)) goto cleanup;
            for (unsigned j=0;j<5;++j) {
                int32_t a=nasal_tract.entering[j],b=nasal_tract.departing[j];
                if (span<16 && b>0) a=(a+b)/2,b=-1;
                previous.value[RELOQ_TARGET_F1+j]=b>0?b:a;
                previous.value[RELOQ_TARGET_B1+j]=nasal_tract.bandwidth[j];
            }
#ifdef RELOQ_GENERIC_PHONE_TRACE
            fprintf(stderr,"RELOQ_NASAL_TRACT %zu %lld %lld\n",i,(long long)tick,(long long)(tick+span));
#endif
        }
        if (obstruent_tract_active) {
            struct nasal_output tract_output={&stages,&output};
            const int32_t tract_span=coda_timed?coda_timing.body_ticks:stop_phase_active?stop_phase.gesture.body_ticks:cluster_active?onset.body_ticks:voiceless_tract_stop && !terminal_stop &&
                !affricate_part?stop_timing.aspiration_at_ticks:
                (node->phone==RELOQ_DE_PHONE_r || node->phone==RELOQ_DE_PHONE_l ||
                 node->phone==RELOQ_DE_PHONE_j || node->phone==RELOQ_DE_PHONE_w) &&
                timing->closing_ticks?span-timing->outgoing_ticks:span;
            const int32_t tract_delay=affricate_part==9?30:
                affricate_part && !(affricate_part&1)?
                (affricate_part==8?1:10):0;
            if (reloq_german_obstruent_tract_append(&obstruent_tract,tick+tract_delay,tick+tract_span,i,
                    emit_nasal_tract,&tract_output)) goto cleanup;
            for (unsigned j=0;j<5;++j) {
                int32_t a=obstruent_tract.entering[j],b=obstruent_tract.departing[j];
                if (tract_span<16 && a>0 && b>0) a=(a+b)/2,b=-1;
                if (b>0 || a>0) previous.value[RELOQ_TARGET_F1+j]=b>0?b:a;
                previous.value[RELOQ_TARGET_B1+j]=obstruent_tract.bandwidth[j];
            }
#ifdef RELOQ_GENERIC_PHONE_TRACE
            fprintf(stderr,"RELOQ_OBSTRUENT_TRACT %zu %lld %lld\n",i,(long long)(tick+tract_delay),(long long)(tick+tract_span));
#endif
        }
        if (!phase_owned && !nucleus_active && node->phone!=RELOQ_DE_PHONE_h) {
            const int32_t begin_delay=is_diphthong?timing->incoming_ticks:
                affricate_part==9?30:affricate_part && !(affricate_part&1)?
                (affricate_part==8?1:10):0;
            const int32_t body_end=coda_timed?coda_timing.body_ticks:
                is_diphthong || timing->closing_ticks?span-timing->outgoing_ticks:span;
            if (reloq_german_resonance_append(node->phone,tick+begin_delay,
                tick+body_end,current.value[RELOQ_TARGET_FNZ],i,
                emit_source_point,&output)) goto cleanup;
        }
        output.tract_owned=0;
        glot->release_stop=(unsigned)(stop_phase_active && stop_phase.gesture.burst_ticks);
        glot->end=tick+(stop_phase_active?stop_phase.gesture.body_ticks-stop_phase.gesture.burst_ticks:
            cluster_active?onset.body_ticks-onset.burst_ticks:timing->vowel_ticks?timing->incoming_ticks+timing->vowel_ticks:span);
#ifdef RELOQ_GENERIC_PHONE_TRACE
        fprintf(stderr,"RELOQ_PHONE_TICK %zu %u %lld %lld\n",i,(unsigned)node->phone,(long long)tick,(long long)span);
        if (coda_timed) fprintf(stderr,"RELOQ_CODA_TIMING %zu %d %d\n",i,
            coda_timing.body_ticks,coda_timing.outgoing_ticks);
#endif
        const int64_t melody_body_end=tick+(stop_phase_active?stop_phase.gesture.body_ticks:
            cluster_active?onset.body_ticks:coda_timed?coda_timing.body_ticks:
            timing->vowel_ticks?timing->incoming_ticks+timing->vowel_ticks:span);
        /* A vowel with no incoming voiced contact starts its pitch range
         * at its own onset. The preceding stop's aspiration is outside
         * that range, including an unstressed vowel before the next word. */
        previous_pitch_begin=shape && timing->incoming_ticks?previous_body_end:tick;
        previous_pitch_end=shape?tick+span-timing->outgoing_ticks:melody_body_end;
        if(is_diphthong && timing->closing_ticks)
            previous_pitch_begin=previous_pitch_end-timing->closing_ticks;
        previous_body_end=melody_body_end;
        if (i==syllables[node->syllable_index].first_phone)
            beat->first_body_end_tick=melody_body_end;
        /* Pitch onset refers to the last oral onset member. A postaccent
         * return skips nasal and breath-only heads, then spans the first
         * remaining phone. These are distinct ranges in the original
         * intonation scheduler (1136 and 1168 in the development observer). */
        if (!beat->closure_tick && node->phone!=RELOQ_DE_PHONE_h)
            beat->onset_begin_tick=tick;
        if (!beat->return_end_tick && node->phone!=RELOQ_DE_PHONE_h &&
            node->phone!=RELOQ_DE_PHONE_m && node->phone!=RELOQ_DE_PHONE_n &&
            node->phone!=RELOQ_DE_PHONE_G) {
            beat->return_begin_tick=tick;
            beat->return_end_tick=shape?tick+span:melody_body_end;
        }
        if (!shape && beat->closure_tick && !syllabic_sonorant(node->phone) &&
            ((node->phone!=RELOQ_DE_PHONE_j && node->phone!=RELOQ_DE_PHONE_w) ||
             !vowel(node->left_phone)) && !beat->coda_body_end_tick) {
            beat->coda_begin_tick=tick;
            beat->coda_body_end_tick=melody_body_end;
        }
        tick += span;
#ifdef RELOQ_GENERIC_PHONE_TRACE
        fprintf(stderr,"RELOQ_RULE_PATH %zu %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
            i,(unsigned)node->phone,(unsigned)node->left_phone,(unsigned)node->right_phone,
            node->stress,node->same_word_left,node->same_word_right,
            node->same_syllable_left,node->same_syllable_right,
            shape!=NULL,timing->span_ticks!=0,cluster_active!=0,nucleus_active!=0,
            (unsigned)source_segments[i].kind);
        if (timing->span_ticks)
            fprintf(stderr,"RELOQ_NUCLEUS_TIMING %zu %d %d %d %d %d %d %.17g\n",
                i,timing->span_ticks,timing->incoming_ticks,timing->vowel_ticks,
                timing->glide_ticks,timing->closing_ticks,timing->outgoing_ticks,
                timing->budget.adjusted_ticks);
        if (stop_phase_active)
            fprintf(stderr,"RELOQ_STOP_PHASE %zu %d %d %d\n",i,
                stop_phase.gesture.body_ticks,stop_phase.gesture.burst_ticks,
                stop_phase.incoming_ticks);
        /* Test-only phoneme landmarks. This branch is absent from product
         * builds; the standalone observer compiles this source separately.
         * The same timeline drives the rendered target curves, so acoustic
         * comparisons can label intervals without waveform-only guessing. */
        fprintf(stderr, "RELOQ_PHONE_SPAN %zu %zu %zu %zu %zu %lld %lld\n",
                i, node->source_offset, node->source_length,
                node->word_index, node->syllable_index,
                (long long)(tick - span), (long long)tick);
#endif
        beat->end_tick = tick;
        if (!beat->closure_tick)
            beat->onset_end_tick=tick-(stop_phase_active?
                span-stop_phase.gesture.body_ticks:cluster_active?
                span-onset.body_ticks:0);
        else if (!shape && !obstruent_nasal && (node->phone==RELOQ_DE_PHONE_m ||
                 node->phone==RELOQ_DE_PHONE_n || node->phone==RELOQ_DE_PHONE_G ||
                 node->phone==RELOQ_DE_PHONE_l || node->phone==RELOQ_DE_PHONE_r ||
                 ((node->phone==RELOQ_DE_PHONE_j || node->phone==RELOQ_DE_PHONE_w) &&
                  vowel(node->left_phone))))
            beat->closure_tick=melody_body_end;
    }
    output.source_phone=phone_count;
    output.nucleus_source=previous_source_owned;
    const enum reloq_german_terminal_mark final_mark=marks?
        marks[mark_count-1]:RELOQ_DE_TERMINAL_IMPLICIT;
    int32_t terminal_send_ticks;
    if (emit_pause(&stages,tick,phones+phone_count-1,source_segments+phone_count-1,
            phone_count-1,final_mark,0,&previous,&output,&terminal_send_ticks,
            controls?controls[mark_count].pause:-1,
            controls?controls[mark_count].explicit_pause:0,
            settings?settings->pause_mode:0)) goto cleanup;
    double pitch_scale = output.native_pitch_scale > 0.0 ?
        output.native_pitch_scale : output.pitch_factor / 200.0;
    double time_scale = output.native_time_scale > 0.0 ?
        output.native_time_scale : output.rate_factor / 100.0;
    int inflection = settings && settings->native_inflection_plus_one ?
        settings->native_inflection_plus_one - 1 : 30;
    /* Profile gestures have independent source, frication and formant
     * timelines. Finish their stream-wise merge before the phrase-level
     * melody adds F0; no phone-local static pitch overwrites its contour. */
    for (size_t i=0;i<phone_count;++i)
        glottal[i].release=i+1<phone_count?source_segments[i+1].begin:source_segments[i].end+60;
#ifdef RELOQ_GENERIC_PHONE_TRACE
    for (size_t i=0;i<phone_count;++i) {
        const struct reloq_german_glottal_segment *g=glottal+i;
        fprintf(stderr,"RELOQ_GLOTTAL %zu %u %u %lld %lld %lld %lld %u\n",i,
            g->active,g->fresh,(long long)g->begin,(long long)g->onset,
            (long long)g->end,(long long)g->release,g->strength);
    }
#endif
    const size_t glottal_points=source_points.count;
    /* The original chooses its 21/16-ms glottal recovery after the native
     * phone schedule has been timed. Scaling the completed source gesture
     * again made a fast initial vowel recover in only 8 ms. Map boundaries
     * first, then generate the physical recovery on the output clock. */
    struct reloq_german_glottal_segment *timed_glottal=
        malloc(phone_count*sizeof *timed_glottal);
    if(!timed_glottal)goto cleanup;
    memcpy(timed_glottal,glottal,phone_count*sizeof *timed_glottal);
    int timing_failed=0;
    for(size_t i=0;i<phone_count && !timing_failed;++i) {
        struct reloq_german_glottal_segment *g=timed_glottal+i;
        timing_failed=scale_tick(g->begin,&output,&g->begin) ||
            scale_tick(g->onset,&output,&g->onset) ||
            scale_tick(g->end,&output,&g->end) ||
            scale_tick(g->release,&output,&g->release);
    }
    struct output_settings glottal_clock=output;
    glottal_clock.native_time_scale=0;glottal_clock.rate_factor=100;
    glottal_clock.timing=NULL;
    int glottal_failed=timing_failed || reloq_german_glottal_append(
        timed_glottal,phone_count,emit_source_point,&glottal_clock);
    free(timed_glottal);
    if(glottal_failed)goto cleanup;
    for (size_t q=glottal_points;q<source_points.count;++q) {
        struct source_point *p=source_points.points+q;
        if (p->stream!=RELOQ_TARGET_DI || p->phone>=phone_count) continue;
        struct reloq_german_onset_cluster c;
        if (reloq_german_onset_cluster_plan(phones,phone_count,p->phone,&c)==1 && c.di)
            p->value=c.di;
        else if (p->phone && !glottal[p->phone].active && timings[p->phone].incoming_ticks &&
            reloq_german_onset_cluster_plan(phones,phone_count,p->phone-1,&c)==1 && c.di) {
            int64_t begin;
            if (scale_tick(glottal[p->phone].begin,&output,&begin)) goto cleanup;
            if (p->tick==begin && scale_tick(glottal[p->phone].begin+
                timings[p->phone].incoming_ticks,&output,&p->tick)) goto cleanup;
        }
    }
    struct envelope_output envelope={&output,source_segments,phone_count};
    if (reloq_german_source_envelope_append(phones,source_segments,phone_count,
        emit_envelope_point,&envelope)) goto cleanup;
    if (reloq_german_source_tilt_append(phones,source_segments,phone_count,
        emit_envelope_point,&envelope)) goto cleanup;
    /* A fresh phrase extends each first source knot back to its local
     * zero, just as a standalone utterance does. Retain the preceding
     * silence through the last output frame before that origin. */
    qsort(source_points.points,source_points.count,sizeof *source_points.points,source_point_order);
    const size_t source_count=source_points.count;
    size_t restarted[RELOQ_TARGET_STREAM_COUNT]={0};
    for (size_t q=0;q<source_count;++q) {
        const struct source_point point=source_points.points[q];
        const size_t i=point.phone;
        if (!i || i>=phone_count || i<=restarted[point.stream] || !glottal[i].fresh || !marks ||
            reloq_german_terminal_continues(marks[phones[i-1].word_index])) continue;
        int64_t origin;
        if (scale_tick(glottal[i].begin,&output,&origin)) goto cleanup;
        if (point.tick<origin) continue;
        restarted[point.stream]=i;
        if (point.tick>origin &&
            source_point_add(&source_points,point.stream,origin,point.value,i)) goto cleanup;
    }
    /* Gestures keep their phone order at identical times: the ending
     * nucleus precedes the next consonant's source step. */
    sort_added_source_points(&source_points,source_count);
    for (size_t q=0;q<source_points.count;++q) {
        const struct source_point *p=source_points.points+q;
        const struct reloq_target_event event={p->stream,0,p->value};
        if (model_append(&stages,p->tick,&event,1,&output,p->phone)) goto cleanup;
    }
    if (reloq_target_stage_router_merge(&stages, builder)) goto cleanup;
    if (reloq_german_generic_melody_append_controls(
            melody, syllable_count, marks, mark_count, inflection,
            pitch_scale, timing_map?1.0:time_scale, controls, builder)) goto cleanup;
    if(timing_map) {
        struct reloq_target_buffer *f0=builder->stream+RELOQ_TARGET_F0;
        for(size_t i=0;i<f0->count;++i)
            if(scale_tick(f0->points[i].tick,&output,&f0->points[i].tick))goto cleanup;
    }
    if (scale_tick(tick + terminal_send_ticks, &output, end_tick))
        goto cleanup;
    result = 0;
cleanup:
    free(glottal);free(source_segments);free(source_points.points);
    reloq_target_stage_router_dispose(&stages);
    free(melody);
    free(timings);
    return result;
}

int reloq_german_generic_plan_with_settings(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_generic_settings *settings,
    struct reloq_target_builder *builder, int64_t *end_tick,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    return generic_plan(phones, phone_count, syllables, syllable_count,
                        settings, NULL, 0, NULL, 0, NULL, NULL, 0, builder, end_tick, cancelled,
                        cancel_context,11025,NULL,NULL,NULL);
}

int reloq_german_generic_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_target_builder *builder, int64_t *end_tick,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    return reloq_german_generic_plan_with_settings(
        phones, phone_count, syllables, syllable_count, NULL,
        builder, end_tick, cancelled, cancel_context);
}

static int text_error(int status)
{
    if (status == RELOQ_DE_TEXT_CANCELLED) return RELOQ_DE_UTTERANCE_CANCELLED;
    if (status == RELOQ_DE_TEXT_UNCOVERED) return RELOQ_DE_UTTERANCE_UNCOVERED;
    if (status == RELOQ_DE_TEXT_NO_MEMORY) return RELOQ_DE_UTTERANCE_MEMORY;
    return RELOQ_DE_UTTERANCE_INVALID;
}

/* Map inline ECI pitch changes through exactly the same acoustic word-open
 * convention as index marks. The G2P frontend has already moved any mark
 * inside a source token past that token. Last mark at one position wins. */
static int pitch_words(const unsigned char *annotation, size_t length,
                       const struct reloq_german_generic_pitch_mark *marks,
                       size_t count, size_t word_count, int baseline,
                       int *word_pitch)
{
    if (!annotation || (count && !marks) ||
        (word_count && !word_pitch) || baseline < 0 || baseline > 100)
        return -1;
    for (size_t i = 0; i < count; ++i)
        if (marks[i].native_pitch < 0 || marks[i].native_pitch > 100 ||
            marks[i].annotation_offset > length ||
            (i && marks[i].annotation_offset < marks[i - 1].annotation_offset))
            return -1;
    struct reloq_german_annotation_cursor cursor = {0};
    struct reloq_german_annotation_event event;
    size_t next = 0, word = 0;
    int current = baseline, state;
    while ((state = reloq_german_annotation_next(
                annotation, length, &cursor, 0, &event)) == 1) {
        if (event.kind != RELOQ_DE_ANNOTATION_SPR ||
            event.spr.kind != RELOQ_DE_SPR_WORD_OPEN) continue;
        if (word >= word_count) return -1;
        while (next < count && marks[next].annotation_offset <= event.offset)
            current = marks[next++].native_pitch;
        word_pitch[word++] = current;
    }
    return state == 0 && word == word_count ? 0 : -1;
}

static int scaled_pitch_value(int32_t value, double factor, int32_t *out)
{
    double scaled = (double)value * factor;
    if (!isfinite(scaled) || scaled < 0 || scaled > INT32_MAX) return -1;
    *out = (int32_t)lround(scaled);
    return 0;
}

/* Change only the F0 stream after the utterance-wide melody is planned.
 * A pair of targets at a word start keeps all preceding contour samples at
 * the old setting while the next frame begins the new setting. No acoustic
 * timeline, source/formant target, index or pause is moved. */
static int apply_pitch_words(struct reloq_target_builder *builder,
                             const int64_t *word_ticks, size_t word_count,
                             const int *word_pitch, int baseline)
{
    if (!builder || !word_ticks || !word_pitch || baseline < 0 || baseline > 100)
        return -1;
    struct reloq_target_buffer *f0 = &builder->stream[RELOQ_TARGET_F0];
    if (!f0->count) return -1;
    double base = reloq_voice_pitch_f0_scale(baseline);
    if (!isfinite(base) || base <= 0) return -1;
    struct reloq_acoustic_curve original = {0, f0->points, f0->count};
    struct reloq_acoustic_cursor cursor;
    if (reloq_acoustic_begin(&original, &cursor)) return -1;
    if (word_count > (SIZE_MAX - f0->count) / 2) return -1;
    size_t limit = f0->count + 2 * word_count;
    if (limit > f0->capacity || limit > SIZE_MAX / sizeof *f0->points)
        return -1;
    struct reloq_acoustic_point *updated = malloc(limit * sizeof *updated);
    if (!updated) return -1;
    size_t source = 0, written = 0, word = 0;
    int active = baseline, result = -1;
    while (word < word_count) {
        int64_t tick = word_ticks[word];
        if (tick < 0 || (word && tick < word_ticks[word - 1])) goto done;
        int change = word_pitch[word];
        while (word + 1 < word_count && word_ticks[word + 1] == tick)
            change = word_pitch[++word];
        if (change < 0 || change > 100) goto done;
        if (change != active) {
            double before = reloq_voice_pitch_f0_scale(active) / base;
            double after = reloq_voice_pitch_f0_scale(change) / base;
            if (!isfinite(before) || !isfinite(after) ||
                before <= 0 || after <= 0) goto done;
            while (source < f0->count && f0->points[source].tick < tick) {
                struct reloq_acoustic_point point = f0->points[source++];
                if (scaled_pitch_value(point.value, before, &point.value))
                    goto done;
                updated[written++] = point;
            }
            if (tick == 0 && !source) {
                /* No preceding audio: change the initial contour directly. */
                active = change;
                ++word;
                continue;
            }
            int32_t old_value = 0, new_value = 0;
            if (source < f0->count && f0->points[source].tick == tick) {
                int32_t last = 0;
                do {
                    struct reloq_acoustic_point point = f0->points[source++];
                    last = point.value;
                    if (scaled_pitch_value(point.value, before, &point.value))
                        goto done;
                    updated[written++] = point;
                } while (source < f0->count && f0->points[source].tick == tick);
                if (scaled_pitch_value(last, after, &new_value)) goto done;
                updated[written++] =
                    (struct reloq_acoustic_point){tick, new_value};
            } else {
                float baseline_value = 0;
                if (reloq_acoustic_value(&cursor, tick, &baseline_value) ||
                    scaled_pitch_value((int32_t)baseline_value, before,
                                       &old_value) ||
                    scaled_pitch_value((int32_t)baseline_value, after,
                                       &new_value)) goto done;
                updated[written++] =
                    (struct reloq_acoustic_point){tick, old_value};
                updated[written++] =
                    (struct reloq_acoustic_point){tick, new_value};
            }
            active = change;
        }
        ++word;
    }
    double factor = reloq_voice_pitch_f0_scale(active) / base;
    if (!isfinite(factor) || factor <= 0) goto done;
    while (source < f0->count) {
        struct reloq_acoustic_point point = f0->points[source++];
        if (scaled_pitch_value(point.value, factor, &point.value)) goto done;
        updated[written++] = point;
    }
    if (written > f0->capacity) goto done;
    memcpy(f0->points, updated, written * sizeof *updated);
    f0->count = written;
    result = 0;
done:
    free(updated);
    return result;
}

static int session_cancelled(void *context)
{
    return reloq_session_cancelled((const struct reloq_session *)context);
}

#include "reloq_german_inline_controls.inc"

static int render(
    const unsigned char *utf8, size_t length,
    const struct reloq_lexicon *lexicon,
    const struct reloq_german_generic_settings *settings,
    int annotation_input,
    const struct reloq_german_word_features *features,size_t feature_count,
    const struct reloq_german_generic_index *indices, size_t index_count,
    reloq_german_generic_index_sink index_sink,
    const struct reloq_german_generic_pitch_mark *pitches, size_t pitch_count,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info,int inline_volume)
{
    struct reloq_inline_word *controls=NULL;size_t control_words=0;
    struct inline_timing speed_map={0};
    struct source_points colour={0},sibilants={0};
    struct reloq_acoustic_point *sibilant_points=NULL;
    struct reloq_acoustic_curve sibilant_curve={0};
    struct reloq_acoustic_point *colour_points=NULL;
    struct reloq_acoustic_curve colour_curve={0};
    unsigned char *annotation = NULL;
    struct reloq_german_phone_node *phones = NULL;
    struct reloq_german_syllable *syllables = NULL;
    enum reloq_german_terminal_mark *marks = NULL;
    struct reloq_acoustic_point *points = NULL;
    int64_t *word_ticks = NULL;
    int *word_pitch = NULL;
    size_t *index_positions = NULL;
    struct reloq_german_utterance_info progress = {0};
    size_t annotation_bytes = 0, bad = 0;
    struct reloq_german_word_features *resolved_features=NULL;
    int status = RELOQ_DE_UTTERANCE_INVALID;
    if (!info || !session || !sink || (length && !utf8) ||
        (index_count && (!indices || !index_sink)) ||
        ((pitch_count || controls) && !pitches)) return status;
    *info = progress;
    struct output_settings output;
    if (output_settings_init(&output, settings)) return status;
    output.model.sample_rate=reloq_session_sample_rate(session);
    if (!length) {
        for (size_t i = 0; i < pitch_count; ++i)
            if (pitches[i].annotation_offset || pitches[i].native_pitch < 0 ||
                pitches[i].native_pitch > 100) return status;
        for (size_t i = 0; i < index_count; ++i) {
            if (indices[i].annotation_offset) return status;
            if (index_sink(context, i)) return RELOQ_DE_UTTERANCE_SINK;
        }
        return RELOQ_DE_UTTERANCE_OK;
    }
    if (session_cancelled(session)) return RELOQ_DE_UTTERANCE_CANCELLED;
    size_t first = 0;
    while (first < length && (utf8[first] == ' ' || utf8[first] == '\t' ||
                              utf8[first] == '\r' || utf8[first] == '\n'))
        ++first;
    if (annotation_input || (first < length && utf8[first] == '`')) {
        /* Public annotation offsets include leading text. Discarding it
         * moves index/pitch marks and can put an end index past the buffer. */
        if (annotation_input) first = 0;
        annotation_bytes = length - first;
        if (annotation_bytes == SIZE_MAX) return RELOQ_DE_UTTERANCE_MEMORY;
        annotation = malloc(annotation_bytes + 1);
        if (!annotation) return RELOQ_DE_UTTERANCE_MEMORY;
        memcpy(annotation, utf8 + first, annotation_bytes);
        annotation[annotation_bytes] = 0;
    } else {
        int result = reloq_german_text_annotate_checked(
            utf8, length, NULL, 0, &annotation_bytes, &bad, lexicon,
            session_cancelled, session);
        if (result) {
            info->error_offset = bad;
            return text_error(result);
        }
        if (annotation_bytes == SIZE_MAX) return RELOQ_DE_UTTERANCE_MEMORY;
        annotation = malloc(annotation_bytes + 1);
        if (!annotation) return RELOQ_DE_UTTERANCE_MEMORY;
        size_t written = annotation_bytes;
        result = reloq_german_text_annotate_checked(
            utf8, length, annotation, annotation_bytes + 1, &written,
            &bad, lexicon, session_cancelled, session);
        if (result || written != annotation_bytes) {
            status = result ? text_error(result) : RELOQ_DE_UTTERANCE_INVALID;
            progress.error_offset = bad;
            goto done;
        }
        size_t encoded_bytes = 0;
        result = reloq_german_input_encode_checked(
            utf8, length, NULL, 0, &encoded_bytes, NULL,
            session_cancelled, session);
        if (result || encoded_bytes == SIZE_MAX) {
            status = result == 1 ? RELOQ_DE_UTTERANCE_CANCELLED :
                     RELOQ_DE_UTTERANCE_MEMORY;
            goto done;
        }
        unsigned char *encoded = malloc(encoded_bytes + 1);
        if (!encoded) { status = RELOQ_DE_UTTERANCE_MEMORY; goto done; }
        result = reloq_german_input_encode_checked(
            utf8, length, encoded, encoded_bytes + 1, &encoded_bytes, NULL,
            session_cancelled, session);
        if (result) {
            free(encoded);
            status = result == 1 ? RELOQ_DE_UTTERANCE_CANCELLED :
                     RELOQ_DE_UTTERANCE_MODEL;
            goto done;
        }
        unsigned char *accented = NULL;
        size_t accented_bytes = 0;
        result = reloq_german_generic_prosody_reaccent(
            encoded, encoded_bytes, annotation, annotation_bytes,
            &accented, &accented_bytes, session_cancelled, session);
        free(encoded);
        if (result) {
            status = result == 1 ? RELOQ_DE_UTTERANCE_CANCELLED :
                     RELOQ_DE_UTTERANCE_MEMORY;
            goto done;
        }
        free(annotation);
        annotation = accented;
        annotation_bytes = accented_bytes;
    }
    int resolved=reloq_german_word_features_resolve(annotation,annotation_bytes,
        features,feature_count,&resolved_features,&feature_count,session_cancelled,session);
    if(resolved) {
        status=resolved==1?RELOQ_DE_UTTERANCE_CANCELLED:
            resolved==-2?RELOQ_DE_UTTERANCE_MEMORY:RELOQ_DE_UTTERANCE_INVALID;
        goto done;
    }
    features=resolved_features;
    if(inline_volume>=0 && inline_words(annotation,annotation_bytes,settings,
        inline_volume,&controls,&control_words)) {status=RELOQ_DE_UTTERANCE_MEMORY;goto done;}
    progress.annotation_bytes = annotation_bytes;
    if (annotation_bytes > SIZE_MAX / (2*sizeof *phones)) {
        status = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    phones = malloc((annotation_bytes ? 2*annotation_bytes : 1) * sizeof *phones);
    if (!phones) { status = RELOQ_DE_UTTERANCE_MEMORY; goto done; }
    struct reloq_german_phone_sequence_info sequence = {0};
    int parsed = reloq_german_phone_sequence_build_checked(
        annotation, annotation_bytes, 0, phones, annotation_bytes,
        &sequence, session_cancelled, session);
    if (parsed) {
        status = parsed == 1 ? RELOQ_DE_UTTERANCE_CANCELLED :
                 parsed == -2 ? RELOQ_DE_UTTERANCE_MEMORY :
                 RELOQ_DE_UTTERANCE_INVALID;
        goto done;
    }
    /* The original h rule consumes h before a voiceless stop without an
     * acoustic segment. Keeping a generic h here invents a breath/pause and
     * prevents the stop from seeing its real left neighbor. Preserve source
     * offsets and syllable identities; expansion below rebuilds adjacency. */
    int coda_domain=1;
    for(size_t i=sequence.phone_count;i-- > 0;) {
        /* Devoicing belongs to the complete coda, not only its last
         * written phone. In make_ger_phon_adjustments the remaining
         * consonant category excludes n/r/l/j/w; m/ng do not license
         * voicing. Keep onset clusters outside this backward domain. */
        if(!phones[i].same_syllable_right)coda_domain=1;
        if(vowel(phones[i].phone)){coda_domain=0;continue;}
        const enum reloq_german_phone following=i+1<sequence.phone_count?
            phones[i+1].phone:RELOQ_DE_PHONE_NONE;
        const int coda_cluster=phones[i].same_syllable_left &&
            phones[i].same_syllable_right && coda_domain &&
            following!=RELOQ_DE_PHONE_n && following!=RELOQ_DE_PHONE_r &&
            following!=RELOQ_DE_PHONE_l && following!=RELOQ_DE_PHONE_j &&
            following!=RELOQ_DE_PHONE_w;
        /* SPR's consonant identity still passes through the reference
         * syllable-final allophone rule. It is not limited to text input.
         * Z deliberately stays voiced; the original class excludes it. */
        if(!phones[i].same_syllable_right || coda_cluster) {
            switch(phones[i].phone) {
            case RELOQ_DE_PHONE_b:phones[i].phone=RELOQ_DE_PHONE_p;break;
            case RELOQ_DE_PHONE_d:phones[i].phone=RELOQ_DE_PHONE_t;break;
            case RELOQ_DE_PHONE_g:phones[i].phone=RELOQ_DE_PHONE_k;break;
            case RELOQ_DE_PHONE_v:phones[i].phone=RELOQ_DE_PHONE_f;break;
            case RELOQ_DE_PHONE_z:phones[i].phone=RELOQ_DE_PHONE_s;break;
            default:break;
            }
        }
    }
    size_t retained=0;
    for (size_t i=0;i<sequence.phone_count;++i) {
        const enum reloq_german_phone right=i+1<sequence.phone_count?phones[i+1].phone:RELOQ_DE_PHONE_NONE;
        if (phones[i].phone==RELOQ_DE_PHONE_h &&
            (right==RELOQ_DE_PHONE_NONE ||
             right==RELOQ_DE_PHONE_p || right==RELOQ_DE_PHONE_t || right==RELOQ_DE_PHONE_k ||
             right==RELOQ_DE_PHONE_P || right==RELOQ_DE_PHONE_C || right==RELOQ_DE_PHONE_T)) continue;
        phones[retained++]=phones[i];
    }
    sequence.phone_count=retained;
    if (reloq_german_affricate_expand(phones,&sequence.phone_count,2*annotation_bytes)) {
        status=RELOQ_DE_UTTERANCE_MODEL;goto done;
    }
    progress.phone_count = sequence.phone_count;
    if (!sequence.phone_count) {
        status = RELOQ_DE_UTTERANCE_OK;
        for (size_t i = 0; i < pitch_count; ++i)
            if (pitches[i].annotation_offset > annotation_bytes ||
                pitches[i].native_pitch < 0 || pitches[i].native_pitch > 100 ||
                (i && pitches[i].annotation_offset <
                      pitches[i - 1].annotation_offset)) {
                status = RELOQ_DE_UTTERANCE_INVALID;
                break;
            }
        for (size_t i = 0; i < index_count; ++i) {
            if (indices[i].annotation_offset > annotation_bytes ||
                (i && indices[i].annotation_offset <
                      indices[i - 1].annotation_offset)) {
                status = RELOQ_DE_UTTERANCE_INVALID;
                break;
            }
        }
        if (status == RELOQ_DE_UTTERANCE_OK)
            for (size_t i = 0; i < index_count; ++i)
                if (index_sink(context, i)) {
                    status = RELOQ_DE_UTTERANCE_SINK;
                    break;
                }
        goto done;
    }
    if (sequence.word_count > SIZE_MAX / sizeof *marks) {
        status = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    marks = malloc(sequence.word_count * sizeof *marks);
    if (!marks) { status = RELOQ_DE_UTTERANCE_MEMORY; goto done; }
    if (index_count || pitch_count || controls) {
        if (sequence.word_count > SIZE_MAX / sizeof *word_ticks ||
            index_count > SIZE_MAX / sizeof *index_positions ||
            ((pitch_count || controls) &&
             sequence.word_count > SIZE_MAX / sizeof *word_pitch)) {
            status = RELOQ_DE_UTTERANCE_MEMORY;
            goto done;
        }
        word_ticks = malloc(sequence.word_count * sizeof *word_ticks);
        if (index_count)
            index_positions = malloc(index_count * sizeof *index_positions);
        if (pitch_count || controls)
            word_pitch = malloc(sequence.word_count * sizeof *word_pitch);
        if (!word_ticks || (index_count && !index_positions) ||
            ((pitch_count || controls) && !word_pitch)) {
            status = RELOQ_DE_UTTERANCE_MEMORY;
            goto done;
        }
    }
    if ((pitch_count || controls) && pitch_words(
            annotation, annotation_bytes, pitches, pitch_count,
            sequence.word_count,
            settings && settings->native_pitch_plus_one ?
                settings->native_pitch_plus_one - 1 : 65,
            word_pitch)) {
        status = RELOQ_DE_UTTERANCE_INVALID;
        goto done;
    }
    if (reloq_german_generic_prosody_marks(
            annotation, annotation_bytes, sequence.word_count,
            marks, sequence.word_count)) {
        status = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    if(controls) {
        if(control_words!=sequence.word_count) {status=RELOQ_DE_UTTERANCE_INVALID;goto done;}
        inline_pitch(controls,control_words,pitches,pitch_count,word_pitch);
        for(size_t w=1;w<control_words;++w) if(controls[w].pause>0 &&
            marks[w-1]==RELOQ_DE_TERMINAL_IMPLICIT) marks[w-1]=RELOQ_DE_TERMINAL_CONTROL;
    }
    if (sequence.syllable_count > SIZE_MAX / sizeof *syllables) {
        status = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    syllables = malloc((sequence.syllable_count ? sequence.syllable_count : 1) *
                       sizeof *syllables);
    if (!syllables) { status = RELOQ_DE_UTTERANCE_MEMORY; goto done; }
    size_t syllable_count = 0;
    int planned = reloq_german_syllable_plan_build_checked(
        phones, sequence.phone_count, syllables, sequence.syllable_count,
        &syllable_count, session_cancelled, session);
    if (planned) {
        status = planned == 1 ? RELOQ_DE_UTTERANCE_CANCELLED :
                 planned == -3 ? RELOQ_DE_UTTERANCE_MEMORY :
                 RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    struct reloq_german_generic_point_budget budget;
    if (reloq_german_generic_point_budget(
            sequence.phone_count, sequence.word_count,
            pitch_count != 0 || controls != NULL, &budget)) {
        status = RELOQ_DE_UTTERANCE_MEMORY;
        goto done;
    }
    points = malloc(budget.total_points * sizeof *points);
    if (!points) { status = RELOQ_DE_UTTERANCE_MEMORY; goto done; }
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    size_t point_offset = 0;
    for (size_t stream = 0; stream < RELOQ_TARGET_STREAM_COUNT; ++stream) {
        buffers[stream] = (struct reloq_target_buffer){
            points + point_offset, budget.capacity[stream], 0};
        point_offset += budget.capacity[stream];
    }
    struct reloq_target_builder builder;
    if (reloq_target_builder_begin(&builder, buffers)) {
        status = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    int64_t end_tick = 0;
    struct reloq_german_generic_settings speed_settings={50,50,100,0,0,0,0,0,0,0,0};
    if(settings)speed_settings=*settings;
    const struct reloq_german_generic_settings *plan_settings=settings;
    int tagged_speed=0,uniform_speed=controls && control_words?controls[0].speed:-1;
    for(size_t i=0;i<control_words;++i) {
        tagged_speed|=controls[i].speed>=0;
        if(controls[i].speed!=uniform_speed)uniform_speed=-1;
    }
    if(tagged_speed && uniform_speed>=0) {
        /* Identical controls enter exactly the slider's planning path. */
        speed_settings.native_speed=uniform_speed;plan_settings=&speed_settings;
    } else if(tagged_speed) {
        /* Establish unrounded word boundaries, then generate every source
         * and tract knot with its final clock. Never stretch an already
         * quantized fast schedule to make a slower tagged word. */
        speed_settings.native_speed=50;plan_settings=&speed_settings;
        planned=generic_plan(phones,sequence.phone_count,syllables,syllable_count,
            plan_settings,marks,sequence.word_count,word_ticks,sequence.word_count,
            controls,features,feature_count,&builder,&end_tick,
            session_cancelled,session,output.model.sample_rate,NULL,NULL,NULL);
        if(planned || inline_timing_begin(&speed_map,word_ticks,control_words,controls,
            output.native_time_scale>0?output.native_time_scale:output.rate_factor/100.0)) {
            status=planned==1?RELOQ_DE_UTTERANCE_CANCELLED:RELOQ_DE_UTTERANCE_MODEL;goto done;
        }
        for(size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i)buffers[i].count=0;
        if(reloq_target_builder_begin(&builder,buffers)) {status=RELOQ_DE_UTTERANCE_MODEL;goto done;}
    }
    if((pitch_count || controls) && sequence.word_count) {
        int uniform_pitch=word_pitch[0];
        for(size_t i=1;i<sequence.word_count;++i)
            if(word_pitch[i]!=uniform_pitch){uniform_pitch=-1;break;}
        if(uniform_pitch>=0) {
            /* Constant inline pitch uses the same physical baseline and
             * absolute declination as the NVDA slider, before rounding. */
            speed_settings.native_pitch_plus_one=uniform_pitch+1;
            plan_settings=&speed_settings;
        }
    }
    planned = generic_plan(
        phones, sequence.phone_count, syllables, syllable_count, plan_settings,
        marks, sequence.word_count,
        word_ticks, (index_count || pitch_count || controls) ? sequence.word_count : 0,
        controls,
        features,feature_count,
        &builder, &end_tick, session_cancelled, session,output.model.sample_rate,
        speed_map.count?&speed_map:NULL,output.model.sample_rate>=16000?&colour:NULL,
        output.model.sample_rate>=16000?&sibilants:NULL);
    if (planned) {
        status = planned == 1 ? RELOQ_DE_UTTERANCE_CANCELLED :
                 RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    if(sibilants.count) {
        qsort(sibilants.points,sibilants.count,sizeof *sibilants.points,source_point_order);
        if(sibilants.count>SIZE_MAX/2/sizeof *sibilant_points){status=RELOQ_DE_UTTERANCE_MEMORY;goto done;}
        sibilant_points=malloc(2*sibilants.count*sizeof *sibilant_points);
        if(!sibilant_points){status=RELOQ_DE_UTTERANCE_MEMORY;goto done;}
        size_t used=0;int last=0;
        for(size_t i=0;i<sibilants.count;++i) {
            int64_t t=sibilants.points[i].tick;
            if(i+1<sibilants.count&&sibilants.points[i+1].tick==t)continue;
            if(t>0)sibilant_points[used++]=(struct reloq_acoustic_point){t-1,(float)last};
            last=sibilants.points[i].value;
            sibilant_points[used++]=(struct reloq_acoustic_point){t,(float)last};
        }
        sibilant_curve=(struct reloq_acoustic_curve){0,sibilant_points,used};
    }
    if(colour.count) {
        qsort(colour.points,colour.count,sizeof *colour.points,source_point_order);
        if(colour.count>SIZE_MAX/sizeof *colour_points) {status=RELOQ_DE_UTTERANCE_MEMORY;goto done;}
        colour_points=malloc(colour.count*sizeof *colour_points);
        if(!colour_points) {status=RELOQ_DE_UTTERANCE_MEMORY;goto done;}
        for(size_t i=0;i<colour.count;++i)
            colour_points[i]=(struct reloq_acoustic_point){colour.points[i].tick,colour.points[i].value};
        colour_curve=(struct reloq_acoustic_curve){0,colour_points,colour.count};
    }
    if ((pitch_count || controls) && apply_pitch_words(
            &builder, word_ticks, sequence.word_count, word_pitch,
            plan_settings && plan_settings->native_pitch_plus_one ?
                plan_settings->native_pitch_plus_one - 1 : 65)) {
        status = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    if(controls && controls[0].pause>0 && inline_leading_quiet(&builder,word_ticks[0])) {
        status=RELOQ_DE_UTTERANCE_MODEL;goto done;
    }
    progress.planned_ticks = end_tick;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        progress.target_count += builder.stream[s].count;
    float defaults[RELOQ_PARAMETER_TRACKS];
    uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS];
    unsigned char mapped[RELOQ_PARAMETER_TRACKS];
    if (reloq_german_voice_defaults(defaults, fields, &config)) {
        status = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    float neutral_model_defaults[5];
    for(unsigned k=0;k<5;++k)neutral_model_defaults[k]=defaults[inline_model_defaults[k]];
    int model_settings_changed=0;
    for(size_t w=0;w<control_words;++w)model_settings_changed|=
        controls[w].gender!=output.model.gender || controls[w].head_size!=output.model.head_size ||
        controls[w].roughness!=output.model.roughness || controls[w].breathiness!=output.model.breathiness;
    if(controls && control_words)output.model=inline_model(controls,output.model.sample_rate);
    /* The child female profile broadens the parallel noise poles as well
     * as changing their excitation levels. These constant fields have no
     * named target stream and must be set before exporting the schedule.
     * The fifth pole is unused at the original sample clocks. */
    const int three_poles=reloq_german_voice_model_formants(&output.model)==3;
    const int child_source=output.model.gender && output.model.head_size<30;
    if (child_source || three_poles) {
        defaults[46]=1000;
        defaults[47]=1000;
        if(output.model.sample_rate<=11025)defaults[48]=0;
    }
    if(three_poles && !child_source) {defaults[11]=100;defaults[34]=15;}
    if (reloq_target_builder_export(&builder, fields, defaults, curves,
                                    mapped)) {
        status = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    config.sample_rate=output.model.sample_rate;
    struct reloq_curve_scheduler scheduler;
    struct reloq_curve_schedule schedule;
    if (reloq_curve_scheduler_begin(&scheduler, 0, 1, mapped) ||
        reloq_curve_scheduler_plan(&scheduler, curves, end_tick, 1,
                                   &schedule)) {
        status = RELOQ_DE_UTTERANCE_MODEL;
        goto done;
    }
    if(colour.count)schedule.bypass_attenuation=&colour_curve;
    if(sibilants.count)schedule.s_frication=&sibilant_curve;
    schedule.s_frication_profile=1;
    struct inline_audio controlled={sink,index_sink,context,controls,word_ticks,
        sequence.word_count,0,0,11};
    if(controls) {
        controlled.per_tick=(int)reloq_core_frame_sample_count((float)schedule.frame_ticks,
            config.duration_scale,config.sample_rate*0.001f);
        if(controlled.per_tick<1) {status=RELOQ_DE_UTTERANCE_MODEL;goto done;}
        sink=inline_pcm;index_sink=inline_index;context=&controlled;
    }
    struct reloq_german_generic_index_stream index_stream;
    if (index_count) {
        long per_tick = reloq_core_frame_sample_count(
            (float)schedule.frame_ticks, config.duration_scale,
            config.sample_rate * 0.001f);
        if (per_tick < 1 || reloq_german_generic_index_positions(
                annotation, annotation_bytes, indices, index_count,
                word_ticks, sequence.word_count, (size_t)per_tick,
                index_positions)) {
            status = RELOQ_DE_UTTERANCE_MODEL;
            goto done;
        }
        if (reloq_german_generic_index_stream_begin(
                &index_stream, index_positions, index_count,
                sink, index_sink, context)) {
            status = RELOQ_DE_UTTERANCE_SINK;
            goto done;
        }
    }
    int changing_model=model_settings_changed;
    for(size_t w=1;w<control_words;++w)changing_model|=
        controls[w].gender!=controls[0].gender || controls[w].head_size!=controls[0].head_size ||
        controls[w].roughness!=controls[0].roughness || controls[w].breathiness!=controls[0].breathiness;
    struct inline_models model_cursor={.words=controls,.starts=word_ticks,
        .count=control_words,.tick=schedule.first_tick,.step=schedule.frame_ticks,
        .model=output.model,.session=session};
    for(unsigned k=0;k<5;++k) {
        model_cursor.fixed[k]=schedule.tracks[inline_model_defaults[k]].point_count==0;
        model_cursor.neutral[k]=neutral_model_defaults[k];
    }
    int rendered = changing_model ? reloq_curve_pipeline_render_models(
        &schedule,reloq_german_voice_model_formants(&output.model),session,
        index_count?reloq_german_generic_index_stream_pcm:sink,
        index_count?(void *)&index_stream:context,&progress.frames_completed,
        inline_model_frame,&model_cursor,inline_model_formants) : output.model.sample_rate==11025 ? reloq_curve_pipeline_render(
        &schedule, reloq_german_voice_model_formants(&output.model), session,
        index_count ? reloq_german_generic_index_stream_pcm : sink,
        index_count ? (void *)&index_stream : context,
        &progress.frames_completed) : reloq_curve_pipeline_render_profile(
        &schedule,reloq_german_voice_model_formants(&output.model),session,
        index_count?reloq_german_generic_index_stream_pcm:sink,
        index_count?(void *)&index_stream:context,&progress.frames_completed,
        reloq_german_voice_model_frame,&output.model);
    if (rendered == RELOQ_SESSION_OK && index_count &&
        reloq_german_generic_index_stream_end(&index_stream))
        rendered = RELOQ_SESSION_SINK;
    status = rendered == RELOQ_SESSION_OK ? RELOQ_DE_UTTERANCE_OK :
             rendered == RELOQ_SESSION_CANCELLED ? RELOQ_DE_UTTERANCE_CANCELLED :
             rendered == RELOQ_SESSION_MEMORY ? RELOQ_DE_UTTERANCE_MEMORY :
             rendered == RELOQ_SESSION_SINK ? RELOQ_DE_UTTERANCE_SINK :
             RELOQ_DE_UTTERANCE_RENDER;
done:
    free(colour.points);free(colour_points);
    free(sibilants.points);free(sibilant_points);
    free(resolved_features);
    free(controls);
    free(word_pitch);
    free(index_positions);
    free(word_ticks);
    free(speed_map.starts);free(speed_map.scales);
    free(points);
    free(syllables);
    free(marks);
    free(phones);
    free(annotation);
    *info = progress;
    return status;
}

int reloq_german_generic_render(
    const unsigned char *utf8, size_t length,
    const struct reloq_lexicon *lexicon,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info)
{
    return render(utf8, length, lexicon, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, 0,
                  session, sink, context, info,-1);
}

int reloq_german_generic_render_with_settings(
    const unsigned char *utf8, size_t length,
    const struct reloq_lexicon *lexicon,
    const struct reloq_german_generic_settings *settings,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info)
{
    return render(utf8, length, lexicon, settings, 0, NULL, 0, NULL, 0, NULL, NULL, 0,
                  session, sink, context, info,-1);
}

int reloq_german_generic_render_annotation(
    const unsigned char *annotation, size_t length,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info)
{
    return render(annotation, length, NULL, NULL, 1, NULL, 0, NULL, 0, NULL, NULL, 0,
                  session, sink, context, info,-1);
}

int reloq_german_generic_render_annotation_with_settings(
    const unsigned char *annotation, size_t length,
    const struct reloq_german_generic_settings *settings,
    struct reloq_session *session, reloq_session_pcm_sink sink, void *context,
    struct reloq_german_utterance_info *info)
{
    return render(annotation, length, NULL, settings, 1, NULL, 0, NULL, 0, NULL, NULL, 0,
                  session, sink, context, info,-1);
}

int reloq_german_generic_render_annotation_indexed_with_settings(
    const unsigned char *annotation, size_t length,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices, size_t index_count,
    struct reloq_session *session, reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink, void *context,
    struct reloq_german_utterance_info *info)
{
    return render(annotation, length, NULL, settings, 1, NULL, 0,
                  indices, index_count, index_sink, NULL, 0,
                  session, pcm_sink, context, info,-1);
}

int reloq_german_generic_render_annotation_indexed_with_settings_and_pitch(
    const unsigned char *annotation, size_t length,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices, size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches, size_t pitch_count,
    struct reloq_session *session, reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink, void *context,
    struct reloq_german_utterance_info *info)
{
    return render(annotation, length, NULL, settings, 1, NULL, 0,
                  indices, index_count, index_sink, pitches, pitch_count,
                  session, pcm_sink, context, info,-1);
}

int reloq_german_generic_render_text_plan(
    const unsigned char *annotation,size_t length,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_word_features *features,size_t feature_count,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink,void *context,
    struct reloq_german_utterance_info *info)
{
    return render(annotation,length,NULL,settings,1,features,feature_count,
                  indices,index_count,index_sink,pitches,pitch_count,
                  session,pcm_sink,context,info,-1);
}

int reloq_german_generic_render_tagged_text_plan(
    const unsigned char *annotation,size_t length,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_word_features *features,size_t feature_count,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink pcm_sink,
    reloq_german_generic_index_sink index_sink,void *context,
    struct reloq_german_utterance_info *info,int native_volume)
{
    return render(annotation,length,NULL,settings,1,features,feature_count,
                  indices,index_count,index_sink,pitches,pitch_count,
                  session,pcm_sink,context,info,native_volume);
}
