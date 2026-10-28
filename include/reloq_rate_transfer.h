#ifndef RELOQ_RATE_TRANSFER_H
#define RELOQ_RATE_TRANSFER_H
#include <stddef.h>
/* A short, continuously updated residual filter transfers the measured
 * reference tract magnitude to a wider clock. Extra upper resonances keep
 * their native response above the reference band. No PCM resampling. */
enum {RELOQ_RATE_TAPS=129,RELOQ_RATE_VOICE_COLOURS=4};
struct reloq_rate_transfer {
    float kernel[RELOQ_RATE_TAPS],target[RELOQ_RATE_TAPS];
    float step[RELOQ_RATE_TAPS];
    float history[2*RELOQ_RATE_TAPS],key[144];
    unsigned cursor,remaining,taps;
    int ready;
    /* Requested broad presence in the wideband voiced tract only. */
    double presence[5],presence_delay[2];
    /* Periodic excitation only: replace the common tract's presence
     * peak by a compensated shelf and upper-harmonic lift, then add a
     * flat-topped +2.5-dB band over 3.5..4 kHz at the extended clocks.
     * voice_colour[3] and voice_band form its two causal sections.
     * Aspiration has its own continuously smoothed level reduction. */
    double voice_colour[RELOQ_RATE_VOICE_COLOURS][5];
    double voice_colour_delay[RELOQ_RATE_VOICE_COLOURS][2];
    double aspiration_voicing,aspiration_step;
    float presence_rate;
    /* Optional second section of the additional periodic presence band.
     * A zero first coefficient bypasses this section. These coefficients
     * and delays belong to the filter, never to a stored speech result. */
    double voice_band[5],voice_band_delay[2];
};
struct reloq_core_voiced_formants;
void reloq_rate_transfer_prepare(struct reloq_rate_transfer *,
    const struct reloq_core_voiced_formants *,float rate,float reference,
    unsigned reference_formants);
void reloq_rate_transfer_process(struct reloq_rate_transfer *,float *,size_t);
void reloq_rate_transfer_process_voice(struct reloq_rate_transfer *,float *,size_t);
void reloq_rate_transfer_process_aspiration(struct reloq_rate_transfer *,float *,size_t,int voiced);
struct reloq_core_frication_bank;
/* Fit the complete coherent AF sum, including the direct noise branch.
 * Both banks consume one shared random source; isolated pole energies do
 * not describe their constructive and destructive interference. */
void reloq_rate_transfer_prepare_noise(struct reloq_rate_transfer *,
    const struct reloq_core_frication_bank *reference_bank,
    const struct reloq_core_frication_bank *wide_bank,float rate,float reference,
    float boundary_width);
struct reloq_noise_profile;
void reloq_rate_transfer_prepare_noise_mix(struct reloq_rate_transfer *,
    const struct reloq_core_frication_bank *low,
    const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *,float rate,float mix);
/* Same source blend, with the accepted 0.2.84 spectral weighting when
 * historic84=1. Both colours share inverse-gain bounds and 3-ms ramps. */
void reloq_rate_transfer_prepare_noise_mix_profile(struct reloq_rate_transfer *,
    const struct reloq_core_frication_bank *low,
    const struct reloq_core_frication_bank *high,
    const struct reloq_core_frication_bank *current,
    const struct reloq_noise_profile *,float rate,float mix,int historic84);
#endif
