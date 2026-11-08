#ifndef RELOQ_VOICE_CONTROLS_H
#define RELOQ_VOICE_CONTROLS_H

/* Independent C calibration against Apple's public ECI voice API. These
 * helpers compute target parameters; they do not modify any voice or audio.
 * -1 / negative return means an invalid caller setting. In the original
 * ECI scale, 50 is the voice's native default speed and 65 its pitch. */
int reloq_voice_speed_from_nvda(int percent, int boost);
double reloq_voice_speed_time_scale(int native_speed);
/* Internal pitch representation: the ordinary ECI scale remains 0..100.
 * Hertz values use a disjoint range while they travel through word controls;
 * they never enter the public native 0..100 parameter or the rule provider. */
enum { RELOQ_PITCH_HZ_OFFSET = 1000, RELOQ_PITCH_HZ_MIN = 1,
       RELOQ_PITCH_HZ_MAX = 24000 };
static inline int reloq_pitch_is_hz(int value)
{return value>=RELOQ_PITCH_HZ_OFFSET+RELOQ_PITCH_HZ_MIN &&
        value<=RELOQ_PITCH_HZ_OFFSET+RELOQ_PITCH_HZ_MAX;}
static inline int reloq_pitch_valid(int value)
{return (value>=0 && value<=100) || reloq_pitch_is_hz(value);}
double reloq_voice_pitch_f0_scale(int native_pitch);
int reloq_voice_head_formant_hundredths(int head_size);
double reloq_voice_volume_gain(int native_volume);
/* Source dB at existing /f, v/ endpoints (German W is /v/). Keep the
 * filter colour, voiced source and timing; raise only active frication. */
static inline int reloq_voice_labiodental_gain(float sample_rate)
{return sample_rate>=32000?3:sample_rate>=22050?2:0;}

#endif
