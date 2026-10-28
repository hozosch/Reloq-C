#include "reloq_voice_controls.h"

#include <math.h>
#include <stddef.h>

struct anchor { int setting; double value; };

/* Measured 11,025-Hz complete-utterance durations relative to native speed
 * 50, with anchors at the normal NVDA range, boost range and curvature
 * changes. Short utterances can differ by several frames at high speeds. */
static const struct anchor time_scale[] = {
    {40, 24431.0 / 20042.0}, {50, 1.0}, {64, 15213.0 / 20042.0},
    {69, 14025.0 / 20042.0}, {87, 9845.0 / 20042.0},
    {98, 7645.0 / 20042.0}, {105, 6809.0 / 20042.0},
    {110, 6061.0 / 20042.0}, {127, 4455.0 / 20042.0},
    {143, 3289.0 / 20042.0}, {156, 2563.0 / 20042.0},
    {175, 1837.0 / 20042.0}, {203, 1320.0 / 20042.0},
    {225, 1056.0 / 20042.0}, {250, 979.0 / 20042.0}
};

/* Callback PCM RMS at eleven thousand samples per second, all measured from
 * a public Apple ECI voice. No Klatt control frame changes with volume.
 * Native 100 is the maximum and is normalized to the existing C level. */
static const struct anchor volume_gain[] = {
    {0, 57.363 / 2768.533}, {5, 72.277 / 2768.533},
    {10, 81.129 / 2768.533}, {15, 102.194 / 2768.533},
    {20, 128.713 / 2768.533}, {25, 144.450 / 2768.533},
    {30, 181.907 / 2768.533}, {35, 204.134 / 2768.533},
    {40, 257.047 / 2768.533}, {45, 288.437 / 2768.533},
    {50, 363.184 / 2768.533}, {55, 407.530 / 2768.533},
    {60, 513.112 / 2768.533}, {65, 646.022 / 2768.533},
    {70, 646.022 / 2768.533}, {75, 813.358 / 2768.533},
    {80, 1024.011 / 2768.533}, {85, 1460.046 / 2768.533},
    {90, 1895.036 / 2768.533}, {92, 2066.394 / 2768.533},
    {95, 2333.272 / 2768.533}, {100, 1.0}
};

static double interpolate(const struct anchor *anchors, size_t count,
                          int setting)
{
    if (setting < anchors[0].setting || setting > anchors[count-1].setting)
        return -1.0;
    for (size_t i = 1; i < count; ++i) {
        if (setting <= anchors[i].setting) {
            double t = (double)(setting - anchors[i-1].setting) /
                       (anchors[i].setting - anchors[i-1].setting);
            /* Natural-time and F0 changes are multiplicative. Interpolate
             * their logarithms between independently observed anchors. */
            return exp(log(anchors[i-1].value) * (1.0-t) +
                       log(anchors[i].value) * t);
        }
    }
    return anchors[count-1].value;
}

int reloq_voice_speed_from_nvda(int percent, int boost)
{
    if (percent < 0 || percent > 100 || (boost != 0 && boost != 1)) return -1;
    int native = 40 + (116 * percent + 50) / 100;
    if (boost) {
        native = (8 * native + 2) / 5;
        if (native > 250) native = 250;
    }
    return native;
}

double reloq_voice_speed_time_scale(int native_speed)
{
    return interpolate(time_scale, sizeof time_scale / sizeof time_scale[0],
                       native_speed);
}

double reloq_voice_pitch_f0_scale(int native_pitch)
{
    if(native_pitch<0 || native_pitch>100)return -1;
    if(native_pitch==65)return 1;
    /* Continuous physical pitch, before the public integer-Hz display.
     * Verified against all 101 reference settings; sentence mean F0 is
     * not a control curve because phrase declination has its own scale. */
    const double hz=35.11023877+4.889761232*exp2(.06306456*native_pitch);
    return hz/118.915;
}

int reloq_voice_head_formant_hundredths(int head_size)
{
    if (head_size < 0 || head_size > 100) return -1;
    /* Integer hundredth scale inferred from independent F1/F2 targets:
     * 0=1.25, 25=1.12, 50=1.00, 75=.87, 100=.75. */
    return (250 - head_size) / 2;
}

double reloq_voice_volume_gain(int native_volume)
{
    return interpolate(volume_gain, sizeof volume_gain / sizeof volume_gain[0],
                       native_volume);
}
