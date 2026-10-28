#ifndef RELOQ_VOICE_CONTROLS_H
#define RELOQ_VOICE_CONTROLS_H

/* Independent C calibration against Apple's public ECI voice API. These
 * helpers compute target parameters; they do not modify any voice or audio.
 * -1 / negative return means an invalid caller setting. In the original
 * ECI scale, 50 is the voice's native default speed and 65 its pitch. */
int reloq_voice_speed_from_nvda(int percent, int boost);
double reloq_voice_speed_time_scale(int native_speed);
double reloq_voice_pitch_f0_scale(int native_pitch);
int reloq_voice_head_formant_hundredths(int head_size);
double reloq_voice_volume_gain(int native_volume);
/* Source dB at existing /f, v/ endpoints (German W is /v/). Keep the
 * filter colour, voiced source and timing; raise only active frication. */
static inline int reloq_voice_labiodental_gain(float sample_rate)
{return sample_rate>=32000?3:sample_rate>=22050?2:0;}

#endif
