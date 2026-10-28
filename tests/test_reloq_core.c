#include "reloq_core.h"
#include <limits.h>
#include <math.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

int main(void)
{
    float clear[2] = {1, -1};
    const int16_t source_i16[2] = {-2, 3};
    const float source_float[2] = {0.5f, -1};
    float history[5] = {0};
    int64_t output_samples[3] = {-7, 7, 0};
    const int64_t pcm_source[5] = {0, 32767, 32768, -32769, INT64_MAX};
    int16_t pcm[5] = {0};
    float previous = 2, latest = 3;
    char text[32] = "a^b|c`ui";
    char escaped[32] = "a\\`p1";
    char output[64];
    unsigned char cancelled = 1;
    struct reloq_core_clock clock;
    struct reloq_core_resonator filter = {
        .input_gain = 0.75f,
        .latest_gain = 0.125f,
        .previous_gain = -0.0625f,
        .ramp_input = {1.0f, 0.5f, 0.25f},
        .ramp_latest = {0.0f, 0.125f, 0.25f},
        .ramp_previous = {0.0f, 0.5f, 0.25f},
        .previous = 0.5f,
        .latest = -0.25f,
        .ramp_remaining = 3,
        .enabled = 1.0f
    };
    const struct reloq_core_resonator initial_filter = filter;
    float resonator_signal[5] = {1, -1, 0.5f, 0.25f, -0.5f};
    float anti_signal[5] = {1, -1, 0.5f, 0.25f, -0.5f};
    const float anti_coefficients[3] = {0.75f, 0.125f, -0.0625f};

    struct reloq_core_resonator prepared = {0};
    CHECK(reloq_core_resonator_set_coefficients(NULL, 0.5f, 0.5f, 1)
          == -1);
    CHECK(reloq_core_resonator_set_coefficients(&prepared, 0.5f, 0.5f, 1)
          == 0);
    CHECK(prepared.previous_gain == -0.25f &&
          prepared.latest_gain == 0.5f && prepared.input_gain == 0.75f);
    CHECK(prepared.ramp_remaining == 3 &&
          prepared.ramp_previous[0] == -0.0625f &&
          prepared.ramp_latest[0] == 0.125f &&
          prepared.ramp_input[0] == 0.9375f &&
          prepared.ramp_previous[2] == -0.1875f &&
          prepared.ramp_latest[2] == 0.375f &&
          prepared.ramp_input[2] == 0.8125f);
    CHECK(reloq_core_resonator_set_coefficients(&prepared, 0.5f, 0.5f, 0)
          == 0 && prepared.ramp_remaining == 0);
    float inverse[3] = {0};
    CHECK(reloq_core_antiresonator_coefficients(&prepared, inverse) == 0);
    CHECK(inverse[0] > 1.3333f && inverse[0] < 1.3334f &&
          inverse[1] < -0.6666f && inverse[1] > -0.6667f &&
          inverse[2] > 0.3333f && inverse[2] < 0.3334f);
    prepared.input_gain = 0;
    CHECK(reloq_core_antiresonator_coefficients(&prepared, inverse) == -1);
    CHECK(reloq_core_resonator_set_coefficients(&prepared, 0.5f, 0.5f, 1)
          == 0);
    CHECK(reloq_core_antiresonator_coefficients(&prepared, inverse) == 0);
    CHECK(prepared.ramp_input[0] > 1.0f &&
          prepared.ramp_latest[0] < 0.0f &&
          prepared.ramp_previous[0] > 0.0f);
    struct reloq_core_clock hz_clock = reloq_core_clock_for_rate(16000.0f);
    CHECK(reloq_core_resonator_set_from_hz(&prepared, &hz_clock,
                                           1000.0f, 100.0f, 0) == 0);
    CHECK(prepared.input_gain > 0.0f &&
          prepared.latest_gain > 0.0f &&
          prepared.previous_gain < 0.0f);
    CHECK(reloq_core_resonator_set_from_hz(&prepared, &hz_clock,
                                           1000.0f, -1.0f, 0) == -1);
    struct reloq_core_resonator glottal = {0};
    struct reloq_core_clock clock8 = reloq_core_clock_for_rate(8000.0f);
    CHECK(reloq_core_glottal_prefilter_configure(&glottal, &clock8,
                                                  5) == 0 &&
          glottal.input_gain == 0x1.75f95ap-1f &&
          glottal.enabled == 20.0f);
    CHECK(reloq_core_glottal_prefilter_configure(&glottal, &hz_clock,
                                                  5) == 0 &&
          glottal.input_gain == 0x1.3aacb8p-1f);
    CHECK(reloq_core_glottal_prefilter_configure(&glottal, &clock8,
                                                  36) == -1 &&
          glottal.enabled == 20.0f);
    CHECK(reloq_core_glottal_prefilter_configure(&glottal, &clock8,
                                                  0) == 0 &&
          glottal.enabled == 0.0f);

    reloq_core_clear(clear, 2);
    CHECK(clear[0] == 0 && clear[1] == 0);
    reloq_core_mix_i16(source_i16, 2, clear, 2);
    reloq_core_mix_float(source_float, 2, clear, 2);
    CHECK(clear[0] == -3 && clear[1] == 4);
    reloq_core_scale_output(output_samples, 3, 0.5f);
    CHECK(output_samples[0] == -3 && output_samples[1] == 3 &&
          output_samples[2] == 0);
    reloq_core_pcm16_from_i64(pcm_source, pcm, 5);
    CHECK(pcm[0] == 0 && pcm[1] == 32767 && pcm[2] == -32768 &&
          pcm[3] == 32767 && pcm[4] == -1);

    reloq_core_parallel0(0.25f, 0.5f, &previous, &latest,
                         history + 2, 3);
    CHECK(history[2] == 2 && history[3] == 1.75f &&
          history[4] == 1.375f);
    CHECK(previous == 1.75f && latest == 1.375f);

    clock = reloq_core_clock_for_rate(16000.0f);
    CHECK(clock.rate_kind == 2 && clock.milliseconds_per_sample == 0.0625f);
    CHECK(clock.samples_per_millisecond == 16.0f);
    CHECK(reloq_core_clock_for_rate(8000.0f).rate_kind == 0);
    CHECK(reloq_core_clock_for_rate(11025.0f).rate_kind == 1);
    CHECK(reloq_core_clock_for_rate(22050.0f).rate_kind == 3);
    struct reloq_core_frame_counts counts =
        reloq_core_frame_counts_for(1.01f, 0, 0.5f, 8.0f);
    CHECK(counts.total == 9 && counts.first == 4 && counts.second == 5);
    CHECK(reloq_core_frame_sample_count(10.0f, 1.0f, 11.025f) == 110);
    CHECK(reloq_core_frame_sample_count(-1.0f, 1.0f, 11.025f) == -1);
    float phase = reloq_core_next_phase(counts.total, 0.125f, 0, 1.01f);
    CHECK(phase > 0.11f && phase < 0.12f);
    uint16_t noise[5] = {0};
    CHECK(reloq_core_noise_fill(noise, 5, 0) == 0x96f5);
    CHECK(noise[0] == 0x3039 && noise[1] == 0x167e &&
          noise[2] == 0x27df && noise[3] == 0x1c2c &&
          noise[4] == 0x96f5);
    struct reloq_core_aspiration aspiration;
    float breath[5] = {2, 2, 2, 2, 2};
    CHECK(reloq_core_aspiration_configure(&aspiration, 0, 20, 1, -1)
          == 0);
    CHECK(aspiration.enabled && aspiration.gain ==
          10.0f * 0x1.90032p-17f);
    CHECK(reloq_core_aspiration_mix(&aspiration, breath, 2, NULL) == 0);
    CHECK(reloq_core_aspiration_mix(&aspiration, breath + 2, 3, NULL) == 0);
    CHECK(aspiration.seed == noise[4]);
    CHECK(breath[0] == 2.0f + (float)(int16_t)noise[0] *
                        aspiration.gain);
    CHECK(breath[4] == 2.0f + (float)((int32_t)noise[4] - 0x10000) *
                        aspiration.gain);
    CHECK(reloq_core_aspiration_configure(&aspiration, 0, 0, 20, 0) == 0);
    CHECK(reloq_core_aspiration_mix(&aspiration, breath, 5, NULL) == 0 &&
          aspiration.seed == 0 && breath[0] != 2.0f);
    struct reloq_core_noise_interval intervals[2] = {{1, 1}, {0, 2}};
    struct reloq_core_noise_gate gate = {1, intervals, 2, NULL};
    float gated[5] = {0};
    CHECK(reloq_core_aspiration_configure(&aspiration, 0, 20, 0, 0) == 0);
    CHECK(reloq_core_aspiration_mix(&aspiration, gated, 5, &gate) == 0);
    CHECK(gated[0] == (float)((int32_t)noise[0] / 2) * aspiration.gain);
    CHECK(gated[1] == (float)(int16_t)noise[1] * aspiration.gain);
    CHECK(gated[2] == (float)((int32_t)noise[2] / 2) * aspiration.gain);
    CHECK(gated[3] == (float)((int32_t)noise[3] / 2) * aspiration.gain);
    CHECK(gated[4] == (float)(-((1 - ((int32_t)noise[4] - 0x10000)) / 2)) *
                      aspiration.gain);
    const uint8_t closed[5] = {1, 0, 1, 1, 1};
    float masked[5] = {0};
    gate.closed_mask = closed;
    CHECK(reloq_core_aspiration_configure(&aspiration, 0, 20, 0, 0) == 0);
    CHECK(reloq_core_aspiration_mix(&aspiration, masked, 5, &gate) == 0);
    CHECK(memcmp(gated, masked, sizeof gated) == 0);
    CHECK(reloq_core_aspiration_configure(&aspiration, 0, LONG_MAX, 1, 0)
          == -1);
    CHECK(reloq_core_aspiration_mix(NULL, breath, 1, NULL) == -1);
    struct reloq_core_frication frication;
    float fricative[5] = {0};
    CHECK(reloq_core_frication_configure(&frication, 0, 20, 20.5f,
                                         1, -1) == 0);
    CHECK(frication.enabled && frication.bypass_gain ==
          -100.0f * 0x1.cdc72p-29f);
    CHECK(reloq_core_frication_fill(&frication, fricative, 2) == 0);
    CHECK(reloq_core_frication_fill(&frication, fricative + 2, 3) == 0);
    CHECK(frication.seed == noise[4] &&
          fricative[0] == (float)noise[0] &&
          fricative[4] == (float)((int32_t)noise[4] - 0x10000));
    CHECK(reloq_core_frication_configure(&frication, 0, 20, 0.0f,
                                         0, 0) == 0);
    CHECK(reloq_core_frication_fill_with_mask(&frication, fricative,
                                               closed, 5) == 0);
    CHECK(frication.seed == noise[4] &&
          fricative[0] == (float)((int32_t)noise[0] / 2) &&
          fricative[1] == (float)noise[1] &&
          fricative[2] == (float)((int32_t)noise[2] / 2) &&
          fricative[4] == -13446.0f);
    CHECK(reloq_core_frication_configure(&frication, 0, 0, 0.0f, 0, 0)
          == 0);
    CHECK(reloq_core_frication_fill(&frication, fricative, 5) == 0 &&
          frication.seed == 0 && fricative[0] == 0.0f);
    CHECK(reloq_core_frication_configure(&frication, 0, 20, NAN, 0, 0)
          == -1);
    CHECK(reloq_core_frication_configure(&frication, 0, LONG_MAX, 1.0f,
                                         0, 0) == -1);
    CHECK(reloq_core_level_gain(-1) == 0.0f &&
          reloq_core_level_gain(0) == 0.0f &&
          reloq_core_level_gain(1) == 0x1.1f3c92p+0f &&
          reloq_core_level_gain(20) == 10.0f &&
          reloq_core_level_gain(40) == 100.0f &&
          reloq_core_level_gain(401) == reloq_core_level_gain(400));
    struct reloq_core_pulse pulse = {0.0f, 2.0f, 0.25f};
    float pulse_samples[4];
    CHECK(reloq_core_pulse_fill(&pulse, pulse_samples, 4) == 0);
    CHECK(pulse_samples[0] == 0.0f && pulse_samples[1] == 1.0f &&
          pulse_samples[2] == 0.0f && pulse_samples[3] == -3.0f &&
          pulse.phase == 8.0f);
    CHECK(reloq_core_pulse_fill(NULL, pulse_samples, 4) == -1);
    struct reloq_core_clock pulse_clock =
        reloq_core_clock_for_rate(11025.0f);
    CHECK(reloq_core_pulse_configure(&pulse, &pulse_clock,
                                     100.0f, 50.0f, 50) == 0);
    CHECK(pulse.phase == 0.0f && pulse.increment > 15.0f &&
          pulse.increment < 16.0f && pulse.curvature > 0.0f);
    CHECK(reloq_core_pulse_configure(&pulse, &pulse_clock,
                                     100.0f, 0.0f, 50) == 0);
    CHECK(pulse.phase == 0.0f && pulse.increment == 0.0f &&
          pulse.curvature == 0.0f);
    CHECK(reloq_core_pulse_fill(&pulse, pulse_samples, 4) == 0);
    CHECK(pulse_samples[0] == 0.0f && pulse_samples[3] == 0.0f);
    CHECK(reloq_core_pulse_configure(&pulse, &pulse_clock,
                                     100.0f, -1.0f, 50) == -1);
    struct reloq_core_voiced_state voice;
    float voiced_samples[4];
    uint8_t voice_mask[4] = {1, 1, 1, 1};
    CHECK(reloq_core_voiced_configure(&voice, 8000.0f,
                                      1000.0f, 50.0f, 40) == 0);
    CHECK(reloq_core_voiced_fill_with_mask(&voice, voiced_samples,
                                           voice_mask, 4) == 0);
    CHECK(voiced_samples[0] == 0.0f && voiced_samples[1] > 0.0f &&
          voice.open_left > 0 && !voice_mask[0] && !voice_mask[3]);
    float first_increment = voice.pulse.increment;
    CHECK(reloq_core_voiced_schedule(&voice, 2000.0f, 40.0f, 50)
          == 0);
    float rest_of_period[76];
    CHECK(reloq_core_voiced_fill(&voice, rest_of_period, 76) == 0 &&
          voice.pulse.increment == first_increment);
    CHECK(reloq_core_voiced_fill(&voice, voiced_samples, 1) == 0 &&
          voice.pulse.increment != first_increment);
    CHECK(reloq_core_voiced_schedule(&voice, NAN, 40.0f, 50) == -1);
    CHECK(reloq_core_voiced_fill(NULL, voiced_samples, 4) == -1);
    CHECK(reloq_core_voiced_configure(&voice, NAN,
                                      1000.0f, 50.0f, 40) == -1);
    CHECK(reloq_core_voiced_configure(&voice, 8000.0f,
                                      1000.0f, NAN, 40) == -1);

    struct reloq_core_voiced_state toggled;
    float transition_samples[80];
    uint8_t transition_mask[80];
    CHECK(reloq_core_voiced_initialize(&toggled, 8000.0f) == 0);
    CHECK(reloq_core_voiced_begin_frame(&toggled, 1000.0f, 50.0f, 0)
          == 0);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &toggled, transition_samples, transition_mask, 4) == 0 &&
          !toggled.period_live && transition_samples[0] == 0.0f &&
          transition_mask[0] == 0);
    CHECK(reloq_core_voiced_begin_frame(&toggled, 1000.0f, 50.0f, 40)
          == 0);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &toggled, transition_samples, transition_mask, 4) == 0 &&
          toggled.period_live && transition_samples[1] > 0.0f);
    CHECK(reloq_core_voiced_begin_frame(&toggled, 1000.0f, 50.0f, 0)
          == 0);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &toggled, transition_samples, transition_mask, 76) == 0 &&
          toggled.period_live && toggled.open_left == 0 &&
          toggled.closed_left == 0 && transition_mask[50] == 0);
    CHECK(reloq_core_voiced_begin_frame(&toggled, 2000.0f, 40.0f, 50)
          == 0 && toggled.pending_change);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &toggled, transition_samples, transition_mask, 1) == 0 &&
          !toggled.pending_change && toggled.period_live);
    CHECK(reloq_core_voiced_begin_frame(&toggled, 2000.0f, 40.0f, 0)
          == 0);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &toggled, transition_samples, transition_mask, 80) == 0 &&
          !toggled.period_live && toggled.next_phase_ms == 0.0f &&
          transition_mask[0] == 0 && transition_mask[79] == 0);

    struct reloq_core_voiced_state modulated;
    struct reloq_core_voiced_modulation_frame modulation = {
        .duration_ms = 1.0f, .duration_scale = 1.0f,
        .pitch_tenths = 1000.0f, .open_percent = 50.0f,
        .av_level = 40, .flutter = 75.0f, .diplophonia = 50.0f
    };
    CHECK(reloq_core_voiced_initialize(&modulated, 8000.0f) == 0);
    CHECK(reloq_core_voiced_begin_modulated_frame(&modulated,
                                                  &modulation) == 0);
    CHECK(reloq_core_voiced_end_modulated_frame(&modulated) == -1);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &modulated, transition_samples, transition_mask, 4) == 0 &&
          modulated.frame_samples_remaining == 4 &&
          modulated.frame_period_configured);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &modulated, transition_samples, transition_mask, 5) == -1 &&
          modulated.frame_samples_remaining == 4);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &modulated, transition_samples, transition_mask, 4) == 0 &&
          reloq_core_voiced_end_modulated_frame(&modulated) == 0 &&
          modulated.elapsed_ms == 1.0f);
    modulation.av_level = 0;
    CHECK(reloq_core_voiced_begin_modulated_frame(&modulated,
                                                  &modulation) == 0);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &modulated, transition_samples, transition_mask, 8) == 0 &&
          transition_mask[0] == 0 && transition_mask[7] == 0 &&
          reloq_core_voiced_end_modulated_frame(&modulated) == 0 &&
          modulated.elapsed_ms == 2.0f);

    /* At DI=100 the alternate pulse is wholly suppressed, while its
     * closed-phase delay and the following ordinary period remain valid. */
    float diplo_samples[320];
    uint8_t diplo_mask[320];
    modulation.duration_ms = 40.0f;
    modulation.flutter = 0.0f;
    modulation.av_level = 40;
    modulation.diplophonia = 100.0f;
    CHECK(reloq_core_voiced_initialize(&modulated, 8000.0f) == 0);
    CHECK(reloq_core_voiced_begin_modulated_frame(&modulated,
                                                  &modulation) == 0);
    CHECK(reloq_core_voiced_fill_frame_with_mask(
              &modulated, diplo_samples, diplo_mask, 320) == 0 &&
          reloq_core_voiced_end_modulated_frame(&modulated) == 0);
    CHECK(diplo_samples[1] != 0.0f &&
          diplo_mask[80] == 1 && diplo_mask[120] == 0 &&
          diplo_samples[121] == 0.0f && diplo_samples[161] != 0.0f &&
          diplo_samples[281] == 0.0f &&
          isfinite(modulated.pulse.phase) &&
          isfinite(modulated.pulse.curvature));
    modulation.diplophonia = nextafterf(100.0f, INFINITY);
    CHECK(reloq_core_voiced_begin_modulated_frame(&modulated,
                                                  &modulation) == -1);

    CHECK(reloq_core_resonator_process(&filter, resonator_signal, 2) == 0);
    CHECK(filter.ramp_remaining == 1);
    CHECK(reloq_core_resonator_process(&filter, resonator_signal + 2, 3) == 0);
    CHECK(resonator_signal[0] == 1 && resonator_signal[1] == -0.5f &&
          resonator_signal[2] == 0.25f && resonator_signal[3] == 0.25f &&
          resonator_signal[4] == -0.359375f);
    CHECK(filter.ramp_remaining == 0 && filter.previous == 0.25f &&
          filter.latest == -0.359375f);

    filter = initial_filter;
    CHECK(reloq_core_antiresonator_process(&filter, anti_coefficients,
                                            anti_signal, 2) == 0);
    CHECK(filter.ramp_remaining == 1);
    CHECK(reloq_core_antiresonator_process(&filter, anti_coefficients,
                                            anti_signal + 2, 3) == 0);
    CHECK(anti_signal[0] == 1 && anti_signal[1] == -0.5f &&
          anti_signal[2] == 0.125f && anti_signal[3] == 0.3125f &&
          anti_signal[4] == -0.375f);
    CHECK(filter.ramp_remaining == 0 && filter.previous == 0.25f &&
          filter.latest == -0.5f);

    filter = initial_filter;
    filter.enabled = 0.0f;
    CHECK(reloq_core_resonator_process(&filter, resonator_signal, 5) == 0);
    CHECK(filter.ramp_remaining == 3 && filter.latest == -0.25f);
    filter.ramp_remaining = 4;
    CHECK(reloq_core_resonator_process(&filter, resonator_signal, 5) == -1);

    struct reloq_core_voiced_bank bank = {0};
    float bank_input[1] = {1.0f};
    bank.highest_formant = 3;
    bank.nasal_pole.enabled = 1.0f;
    bank.nasal_pole.input_gain = 2.0f;
    bank.nasal_zero.enabled = 1.0f;
    bank.nasal_zero_coefficients[0] = 3.0f;
    bank.tracheal_pole.enabled = 1.0f;
    bank.tracheal_pole.input_gain = 4.0f;
    bank.tracheal_zero.enabled = 1.0f;
    bank.tracheal_zero_coefficients[0] = 5.0f;
    const float formant_gains[] = {7.0f, 11.0f, 13.0f, 17.0f};
    for (size_t f = 0; f < 4; ++f) {
        bank.oral[f].enabled = 1.0f;
        bank.oral[f].input_gain = formant_gains[f];
    }
    CHECK(reloq_core_voiced_bank_process(&bank, bank_input, 1) == 0);
    CHECK(bank_input[0] == 120120.0f && bank.oral[3].latest == 0.0f);
    bank.highest_formant = 9;
    CHECK(reloq_core_voiced_bank_process(&bank, bank_input, 1) == -1 &&
          bank_input[0] == 120120.0f);
    CHECK(reloq_core_voiced_bank_process(NULL, bank_input, 1) == -1);
    struct reloq_core_voiced_bank configured_bank = {0};
    struct reloq_core_voiced_formants formants = {
        .nasal_pole = {250.0f, 60.0f},
        .nasal_zero = {350.0f, 90.0f},
        .tracheal_pole = {2400.0f, 120.0f},
        .tracheal_zero = {2000.0f, 160.0f},
        .highest_formant = 2
    };
    for (size_t f = 0; f < RELOQ_CORE_ORAL_FORMANTS; ++f)
        formants.oral[f] = (struct reloq_core_formant){
            500.0f + 400.0f * (float)f, 70.0f + 15.0f * (float)f};
    CHECK(reloq_core_voiced_bank_configure(&configured_bank, &hz_clock,
                                            &formants, 1, 0) == 0);
    CHECK(configured_bank.nasal_pole.enabled == 20.0f &&
          configured_bank.oral[0].enabled == 20.0f &&
          configured_bank.oral[1].enabled == 20.0f &&
          configured_bank.oral[2].enabled == 0.0f &&
          configured_bank.nasal_zero_coefficients[0] > 0.0f);
    formants.oral[1].frequency_hz += 20.0f;
    CHECK(reloq_core_voiced_bank_configure(&configured_bank, &hz_clock,
                                            &formants, 1, 1) == 0 &&
          configured_bank.oral[1].ramp_remaining == 3);
    formants.highest_formant = 9;
    CHECK(reloq_core_voiced_bank_configure(&configured_bank, &hz_clock,
                                            &formants, 1, 1) == -1 &&
          configured_bank.highest_formant == 2);

    formants.highest_formant = 2;
    struct reloq_core_voiced_bank serial_bank = {0};
    struct reloq_core_serial_envelope serial = {0};
    CHECK(reloq_core_serial_frame_begin(
              &serial, &serial_bank, &clock8, &formants,
              35, 0, 16, 8000.0f) == 0 && serial.process &&
          serial.remaining_ms == 20.0f &&
          serial_bank.nasal_pole.enabled == 20.0f);
    CHECK(reloq_core_serial_frame_end(&serial, &serial_bank) == 0 &&
          serial.remaining_ms == 20.0f);
    CHECK(reloq_core_serial_frame_begin(
              &serial, &serial_bank, &clock8, &formants,
              0, 0, 160, 8000.0f) == 0 && serial.process);
    CHECK(reloq_core_serial_frame_end(&serial, &serial_bank) == 0 &&
          serial.remaining_ms == 0.0f);
    serial_bank.oral[0].latest = 1.0f;
    serial_bank.nasal_pole.latest = 2.0f;
    CHECK(reloq_core_serial_frame_begin(
              &serial, &serial_bank, &clock8, &formants,
              0, 0, 16, 8000.0f) == 0 && !serial.process &&
          serial_bank.oral[0].enabled == 0.0f &&
          serial_bank.nasal_pole.enabled == 20.0f);
    CHECK(reloq_core_serial_frame_end(&serial, &serial_bank) == 0 &&
          serial_bank.oral[0].latest == 0.0f &&
          serial_bank.nasal_pole.latest == 2.0f &&
          serial.stages[4].frequency_hz == 0.0f);
    float retained_nasal_gain = serial_bank.nasal_pole.input_gain;
    CHECK(reloq_core_serial_frame_begin(
              &serial, &serial_bank, &clock8, &formants,
              35, 0, 16, 8000.0f) == 0 && serial.process &&
          serial_bank.nasal_pole.input_gain == retained_nasal_gain &&
          serial_bank.oral[0].enabled == 20.0f);
    CHECK(reloq_core_serial_frame_end(&serial, &serial_bank) == 0);

    struct reloq_core_frication_bank af_bank = {0};
    const float af_source[2] = {1.0f, -1.0f};
    float af_destination[2] = {10.0f, 20.0f};
    float af_scratch[2] = {0};
    af_bank.bypass_gain = 2.0f;
    af_bank.active_formants = RELOQ_CORE_FRICATIVE_FORMANTS;
    af_bank.stages[0].enabled = 1.0f;
    af_bank.stages[0].input_gain = 3.0f;
    af_bank.stages[1].enabled = 1.0f;
    af_bank.stages[1].input_gain = 5.0f;
    CHECK(reloq_core_frication_bank_mix(&af_bank, af_source,
                                        af_destination, af_scratch, 2) == 0);
    CHECK(af_destination[0] == 20.0f && af_destination[1] == 10.0f);
    CHECK(reloq_core_frication_bank_mix(&af_bank, af_source,
                                        af_destination, af_destination, 2)
          == -1);
    float af_centers[RELOQ_CORE_FRICATIVE_FORMANTS];
    float af_widths[RELOQ_CORE_FRICATIVE_FORMANTS];
    long af_levels[RELOQ_CORE_FRICATIVE_FORMANTS] = {20, 20};
    for (size_t f = 0; f < RELOQ_CORE_FRICATIVE_FORMANTS; ++f) {
        af_centers[f] = 500.0f + (float)f * 500.0f;
        af_widths[f] = 100.0f;
    }
    CHECK(reloq_core_frication_bank_configure(
              &af_bank, &hz_clock, af_centers, af_widths, af_levels,
              8, 20, 0, 0, -0.5f) == 0);
    CHECK(af_bank.bypass_gain == -0.5f &&
          af_bank.stages[0].input_gain > 0.0f &&
          af_bank.stages[1].input_gain < 0.0f &&
          af_bank.stages[2].input_gain == 0.0f &&
          af_bank.stages[2].enabled == 0.0f);
    af_levels[0] = 0;
    CHECK(reloq_core_frication_bank_configure(
              &af_bank, &hz_clock, af_centers, af_widths, af_levels,
              8, 20, 0, 0, -0.5f) == 0 &&
          af_bank.stages[0].enabled == 20.0f &&
          af_bank.stages[0].input_gain == 0.0f);
    af_widths[0] = -1.0f;
    CHECK(reloq_core_frication_bank_configure(
              &af_bank, &hz_clock, af_centers, af_widths, af_levels,
              8, 20, 0, 0, 1.0f) == -1 && af_bank.bypass_gain == -0.5f);

    struct reloq_core_frication_envelope af_envelope = {0};
    struct reloq_core_frication_bank envelope_bank = {0};
    envelope_bank.active_formants = RELOQ_CORE_FRICATIVE_FORMANTS;
    long envelope_levels[RELOQ_CORE_FRICATIVE_FORMANTS] = {30};
    CHECK(reloq_core_frication_envelope_begin(
              &af_envelope, &envelope_bank, 40, envelope_levels,
              16, 16000.0f) == 0);
    CHECK(af_envelope.global_ms == 20.0f &&
          af_envelope.elapsed_ms == 1.0f &&
          envelope_bank.stages[0].enabled == 20.0f);
    CHECK(reloq_core_frication_envelope_begin(
              &af_envelope, &envelope_bank, 0, envelope_levels,
              16, 16000.0f) == -1);
    CHECK(reloq_core_frication_envelope_end(
              &af_envelope, &envelope_bank) == 0);
    envelope_levels[0] = 0;
    CHECK(reloq_core_frication_envelope_begin(
              &af_envelope, &envelope_bank, 0, envelope_levels,
              16, 16000.0f) == 0);
    CHECK(reloq_core_frication_envelope_end(
              &af_envelope, &envelope_bank) == 0 &&
          af_envelope.global_ms == 19.0f &&
          envelope_bank.stages[0].enabled == 19.0f);
    envelope_bank.stages[0].previous = 2.0f;
    envelope_bank.stages[0].latest = 1.0f;
    CHECK(reloq_core_frication_envelope_begin(
              &af_envelope, &envelope_bank, 0, envelope_levels,
              320, 16000.0f) == 0);
    CHECK(reloq_core_frication_envelope_end(
              &af_envelope, &envelope_bank) == 0 &&
          af_envelope.global_ms == 0.0f &&
          envelope_bank.stages[0].enabled == 0.0f &&
          envelope_bank.stages[0].previous == 0.0f &&
          envelope_bank.stages[0].latest == 0.0f);

    struct reloq_core_noise_frame_state noise_frame_state;
    struct reloq_core_noise_frame noise_frame = {0};
    float noise_frame_output[32], noise_frame_af[32];
    float noise_frame_scratch[32];
    int16_t noise_frame_pcm[32];
    size_t written = 0;
    noise_frame.duration_ms = 2.0f;
    noise_frame.ah_level = 30;
    noise_frame.af_level = 30;
    noise_frame.bypass_level = 20.0f;
    formants.highest_formant = 2;
    noise_frame.tract = formants;
    for (size_t f = 0; f < RELOQ_CORE_FRICATIVE_FORMANTS; ++f) {
        noise_frame.af_bandwidths[f] = 90.0f;
        noise_frame.af_levels[f] = 25;
    }
    CHECK(reloq_core_noise_frame_initialize(&noise_frame_state,
                                             8000.0f, 100,
                                             0, 0, 0) == 0);
    CHECK(noise_frame_state.duration_scale == 1.0f &&
          reloq_core_noise_frame_set_duration_scale(
              &noise_frame_state, 0.0f) == -1 &&
          noise_frame_state.duration_scale == 1.0f);
    CHECK(reloq_core_noise_frame_render(
              &noise_frame_state, &noise_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, noise_frame_pcm,
              8, &written) == -1 && noise_frame_state.frames_rendered == 0);
    CHECK(reloq_core_noise_frame_render(
              &noise_frame_state, &noise_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, noise_frame_pcm,
              32, &written) == 0 && written == 16 &&
          noise_frame_state.frames_rendered == 1 &&
          noise_frame_state.ah_seed != 100 &&
          noise_frame_state.af_seed != 100 &&
          noise_frame_output[0] != 0.0f &&
          noise_frame_state.fricatives.active_formants == 2 &&
          noise_frame_state.fricatives.stages[2].enabled == 0.0f);
    CHECK(reloq_core_noise_frame_set_duration_scale(
              &noise_frame_state, 1.25f) == 0 &&
          noise_frame_state.duration_scale == 1.25f);
    CHECK(reloq_core_noise_frame_render(
              &noise_frame_state, &noise_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, noise_frame_pcm,
              32, &written) == 0 && written == 20);
    CHECK(reloq_core_noise_frame_set_duration_scale(
              &noise_frame_state, 1.0f) == 0);

    struct reloq_core_voiced_frame_state voiced_frame_state;
    struct reloq_core_voiced_frame voiced_frame = {0};
    uint8_t voiced_mask[32];
    voiced_frame.noise = noise_frame;
    voiced_frame.pitch_tenths = 1000.0f;
    voiced_frame.open_percent = 40.0f;
    voiced_frame.av_level = 35;
    voiced_frame.glottal_profile = 5;
    CHECK(reloq_core_voiced_frame_initialize(
              &voiced_frame_state, 8000.0f, 100, 0, 0, 0) == 0);
    CHECK(reloq_core_voiced_frame_render(
              &voiced_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 8, &written) == -1 &&
          voiced_frame_state.noise.frames_rendered == 0);
    CHECK(reloq_core_voiced_frame_render(
              &voiced_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 32, &written) == 0 && written == 16 &&
          voiced_frame_state.noise.frames_rendered == 1 &&
          voiced_frame_state.active_glottal_profile == 5 &&
          voiced_frame_state.noise.ah_seed != 100 &&
          voiced_frame_state.noise.af_seed != 100 &&
          voiced_frame_state.voice.open_left > 0 &&
          voiced_mask[0] == 0 && noise_frame_output[0] != 0.0f);
    voiced_frame.glottal_profile = 6;
    CHECK(reloq_core_voiced_frame_render(
              &voiced_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 32, &written) == 0 &&
          voiced_frame_state.noise.frames_rendered == 2 &&
          voiced_frame_state.active_glottal_profile == 5);
    voiced_frame.glottal_profile = 36;
    CHECK(reloq_core_voiced_frame_render(
              &voiced_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 32, &written) == -1 &&
          voiced_frame_state.noise.frames_rendered == 2);
    struct reloq_core_voiced_frame_state toggled_frame_state;
    voiced_frame.glottal_profile = 0;
    voiced_frame.av_level = 0;
    CHECK(reloq_core_voiced_frame_initialize(&toggled_frame_state,
                                              8000.0f, 100,
                                              0, 0, 0) == 0);
    CHECK(reloq_core_voiced_frame_render(
              &toggled_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 32, &written) == 0 &&
          toggled_frame_state.noise.frames_rendered == 1 &&
          !toggled_frame_state.voice.period_live &&
          voiced_mask[0] == 0 && voiced_mask[15] == 0);
    voiced_frame.av_level = 35;
    CHECK(reloq_core_voiced_frame_render(
              &toggled_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 32, &written) == 0 &&
          toggled_frame_state.voice.period_live &&
          toggled_frame_state.noise.frames_rendered == 2);
    voiced_frame.av_level = 0;
    CHECK(reloq_core_voiced_frame_render(
              &toggled_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 32, &written) == 0 &&
          toggled_frame_state.noise.frames_rendered == 3 &&
          voiced_mask[0] == 0 && voiced_mask[15] == 0);
    voiced_frame.noise.ah_level = 0;
    CHECK(reloq_core_voiced_frame_render(
              &toggled_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 32, &written) == 0 &&
          toggled_frame_state.noise.frames_rendered == 4 &&
          toggled_frame_state.serial.remaining_ms < 20.0f &&
          voiced_mask[0] == 0 && voiced_mask[15] == 0);
    voiced_frame.noise.af_level = 0;
    for (int silent = 0; silent < 16; ++silent)
        CHECK(reloq_core_voiced_frame_render(
                  &toggled_frame_state, &voiced_frame, noise_frame_output,
                  noise_frame_af, noise_frame_scratch, voiced_mask,
                  noise_frame_pcm, 32, &written) == 0);
    CHECK(toggled_frame_state.serial.remaining_ms == 0.0f &&
          toggled_frame_state.noise.tract.oral[0].enabled == 0.0f &&
          toggled_frame_state.noise.tract.nasal_pole.enabled > 0.0f &&
          toggled_frame_state.noise.af_envelope.global_ms == 0.0f &&
          noise_frame_pcm[0] == 0 && noise_frame_pcm[15] == 0);
    voiced_frame.av_level = 35;
    voiced_frame.noise.ah_level = 30;
    voiced_frame.noise.af_level = 30;
    CHECK(reloq_core_voiced_frame_render(
              &toggled_frame_state, &voiced_frame, noise_frame_output,
              noise_frame_af, noise_frame_scratch, voiced_mask,
              noise_frame_pcm, 32, &written) == 0 &&
          toggled_frame_state.serial.remaining_ms == 20.0f &&
          toggled_frame_state.noise.tract.oral[0].enabled == 20.0f);

    CHECK(reloq_core_normalize_byte(0x91) == '\'');
    CHECK(reloq_core_normalize_byte(0x1b) == ' ');
    CHECK(reloq_core_extra_space((const unsigned char *)"a\\b^c|d", 1)
          == 10);

    CHECK(reloq_core_prepare_text(text, strlen(text), 1, output,
                                  sizeof output, 0) == 0);
    CHECK(strcmp(output, "a\\\\^ b\\| c\\` ui") == 0);
    CHECK(reloq_core_prepare_text(escaped, strlen(escaped), 1, output,
                                  sizeof output, 0) == 0);
    CHECK(strcmp(output, "a\\` p1") == 0);
    CHECK(reloq_core_prepare_text(escaped, strlen(escaped), 0, output,
                                  sizeof output, 0) == 0);
    CHECK(strcmp(output, "a\\\\\\\\\\` p1") == 0);
    CHECK(reloq_core_prepare_text(escaped, strlen(escaped), 1, output,
                                  2, 0) == -1);
    CHECK(reloq_core_prepare_text(escaped, strlen(escaped), 1, output,
                                  sizeof output, &cancelled) == 1);
    CHECK(output[0] == 0);
    return 0;
}
