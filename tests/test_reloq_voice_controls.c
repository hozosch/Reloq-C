#include "reloq_voice_controls.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static void close_relative(double predicted, double observed, double limit)
{
    assert(observed > 0);
    assert(fabs(predicted / observed - 1.0) < limit);
}

int main(void)
{
    const int mapping[] = {40, 69, 98, 127, 156};
    for (int i = 0; i < 5; ++i)
        assert(reloq_voice_speed_from_nvda(25*i, 0) == mapping[i]);
    assert(reloq_voice_speed_from_nvda(0, 1) == 64);
    assert(reloq_voice_speed_from_nvda(100, 1) == 250);
    assert(reloq_voice_speed_from_nvda(-1, 0) == -1);
    assert(reloq_voice_speed_from_nvda(50, 2) == -1);

    /* The helper is fitted on a different sentence. These complete-sentence
     * held-out Apple sample counts were never input to the model. */
    const int speeds[] = {40, 69, 98, 127, 156};
    const int actual[] = {32703, 18821, 10230, 5951, 3355};
    for (int i = 0; i < 5; ++i)
        close_relative(reloq_voice_speed_time_scale(speeds[i]) * 26840,
                       actual[i], .04);
    assert(reloq_voice_speed_time_scale(50) == 1.0);
    assert(reloq_voice_speed_time_scale(0) < 0);
    assert(reloq_voice_speed_time_scale(251) < 0);

    /* Public physical-Hz readback. Sentence mean F0 also contains phrase
     * declination and is therefore not a direct test of this control. */
    const int pitch[] = {0, 25, 50, 75, 100};
    const int hz[] = {40, 49, 78, 164, 422};
    for (int i = 0; i < 5; ++i)
        assert((int)(118.915*reloq_voice_pitch_f0_scale(pitch[i])+.01)==hz[i]);
    assert(reloq_voice_pitch_f0_scale(65) == 1.0);
    assert(reloq_voice_pitch_f0_scale(-1) < 0);
    assert(reloq_voice_pitch_f0_scale(101) < 0);

    assert(reloq_voice_head_formant_hundredths(0) == 125);
    assert(reloq_voice_head_formant_hundredths(25) == 112);
    assert(reloq_voice_head_formant_hundredths(50) == 100);
    assert(reloq_voice_head_formant_hundredths(75) == 87);
    assert(reloq_voice_head_formant_hundredths(100) == 75);
    assert(reloq_voice_head_formant_hundredths(101) == -1);
    assert(reloq_voice_volume_gain(100) == 1.0);
    assert(reloq_voice_volume_gain(-1) < 0);
    assert(reloq_voice_volume_gain(101) < 0);
    /* Held-out complete utterance in the same Apple voice: amplitude
     * percentages are independent of its words and elapsed duration. */
    const int volumes[] = {0, 25, 50, 75, 100};
    const double heldout_rms[] = {67.239, 169.324, 425.733, 953.438, 3243.887};
    for (int i = 0; i < 5; ++i)
        close_relative(reloq_voice_volume_gain(volumes[i]),
                       heldout_rms[i] / heldout_rms[4], .015);
    puts("voice controls: Apple/OpenEVV mappings and held-out ratios pass");
    return 0;
}
