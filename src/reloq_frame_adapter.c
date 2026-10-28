#include "reloq_frame_adapter.h"

#include <math.h>
#include <string.h>

/* The reference numeric frame has two unused slots between F1 and F2.
 * F1/B1 are 9/10, F2/B2 start at 13/14, and F8/B8 end at 25/26. */
static const unsigned char oral_center[RELOQ_CORE_ORAL_FORMANTS] = {
    9, 13, 15, 17, 19, 21, 23, 25
};

static int level(float value, long *result)
{
    if (value < 0.0f || value > 400.0f || truncf(value) != value)
        return -1;
    *result = (long)value;
    return 0;
}

int reloq_frame_adapter_map(
    const float controls[RELOQ_PARAMETER_TRACKS], long fhighest,
    struct reloq_core_voiced_frame *out)
{
    if (!controls || !out || fhighest < 1 || fhighest > 8)
        return -1;
    for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i)
        if (!isfinite(controls[i])) return -1;

    struct reloq_core_voiced_frame next = {0};
    struct reloq_core_noise_frame *noise = &next.noise;
    struct reloq_core_voiced_formants *tract = &noise->tract;
    noise->duration_ms = controls[0];
    next.pitch_tenths = controls[1];
    next.open_percent = controls[3];
    next.flutter = controls[5];
    next.diplophonia = controls[6];
    noise->bypass_level = controls[43];
    tract->highest_formant = fhighest;

    /* Integer-only source levels cannot silently truncate at this boundary.
     * The bypass level, unlike AiF, may be fractional in the reference. */
    if (noise->duration_ms <= 0.0f || next.flutter < 0.0f ||
        next.diplophonia < 0.0f || next.diplophonia > 100.0f ||
        controls[4] < 0.0f || controls[4] > 100.0f ||
        truncf(controls[4]) != controls[4] ||
        noise->bypass_level < -400.0f || noise->bypass_level > 400.0f ||
        level(controls[2], &next.av_level) ||
        controls[7] < -400.0f || controls[7] > 400.0f ||
        truncf(controls[7]) != controls[7] ||
        level(controls[8], &noise->af_level))
        return -1;
    /* Female vowel corrections can produce negative AH targets. Preserve
     * them through interpolation and source-offset addition. Zero alone
     * switches the reference aspiration generator off. */
    noise->ah_level=(long)controls[7];
    /* The language scheduler may exceed 35 (small female models add a
     * source tilt offset); the numeric reference caps the filter index. */
    next.glottal_profile = controls[4]>35.0f?35:(long)controls[4];
    if (next.av_level > 0 &&
        (next.pitch_tenths <= 0.0f || next.open_percent < 0.0f ||
         next.open_percent > 100.0f))
        return -1;

    tract->nasal_pole = (struct reloq_core_formant){controls[27], controls[28]};
    tract->nasal_zero = (struct reloq_core_formant){controls[29], controls[30]};
    tract->tracheal_pole = (struct reloq_core_formant){controls[31], controls[32]};
    tract->tracheal_zero = (struct reloq_core_formant){controls[33], controls[34]};
    for (size_t i = 27; i <= 34; ++i)
        if (controls[i] < 0.0f) return -1;
    for (size_t i = 0; i < RELOQ_CORE_ORAL_FORMANTS; ++i) {
        size_t center = oral_center[i];
        tract->oral[i] = (struct reloq_core_formant){
            controls[center], controls[center + 1]};
        noise->af_bandwidths[i] = controls[44 + i];
        if (controls[center] < 0.0f || controls[center + 1] < 0.0f ||
            noise->af_bandwidths[i] < 0.0f ||
            level(controls[35 + i], &noise->af_levels[i]))
            return -1;
    }
    *out = next;
    return 0;
}
