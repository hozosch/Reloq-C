#include "reloq_core_internal.h"
#include <limits.h>
#include <math.h>
#include <string.h>
#include "reloq_glottal_profiles.inc"

void reloq_core_clear(float *samples, long count)
{
    for (long i = 0; i < count; ++i)
        samples[i] = 0.0f;
}

void reloq_core_mix_i16(const int16_t *source, float gain,
                        float *destination, long count)
{
    for (long i = 0; i < count; ++i)
        destination[i] += (float)source[i] * gain;
}

void reloq_core_mix_float(const float *source, float gain,
                          float *destination, long count)
{
    for (long i = 0; i < count; ++i)
        destination[i] += source[i] * gain;
}

void reloq_core_scale_output(int64_t *samples, long count, float gain)
{
    if (gain == 1.0f)
        return;
    for (long i = 0; i < count; ++i) {
        float scaled = (float)samples[i] * gain;
        samples[i] = (int64_t)scaled;
    }
}

void reloq_core_pcm16_from_i64(const int64_t *source, int16_t *destination,
                                size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        uint16_t low = (uint16_t)source[i];
        int32_t signed_value = low < 0x8000u ?
                               (int32_t)low : (int32_t)low - 0x10000;
        destination[i] = (int16_t)signed_value;
    }
}

void reloq_core_parallel0(float a, float b, float *previous, float *latest,
                          float *samples, long count)
{
    float current = *latest;
    samples[-2] = *previous;
    samples[-1] = current;
    if (count < 0)
        count = 0;
    for (long i = 0; i < count; ++i) {
        float lag_two = a * samples[i - 2];
        float lag_one = b * current;
        current = lag_two + lag_one;
        samples[i] = current;
    }
    *previous = count >= 2 ? samples[count - 2] : samples[-1];
    *latest = samples[count - 1];
}

unsigned char reloq_core_normalize_byte(unsigned char c)
{
    if (c == 10 || (c >= 0x20 && c <= 0x7e) ||
        (c >= 0xa0 && c <= 0xfe))
        return c;
    if (c == 0x91 || c == 0x92)
        return '\'';
    switch (c) {
    case 0x80: case 0x85: case 0x8a: case 0x8c:
    case 0x93: case 0x94: case 0x95: case 0x96:
    case 0x97: case 0x98: case 0x99: case 0x9a:
    case 0x9c:
        return c;
    default:
        return ' ';
    }
}

void reloq_core_normalize_text(unsigned char *text)
{
    for (; *text; ++text)
        *text = reloq_core_normalize_byte(*text);
}

int reloq_core_extra_space(const unsigned char *text, unsigned char mode)
{
    int count = 0;
    for (const unsigned char *p = text; *p; ++p) {
        if (*p == '\\')
            count += 5;
        else if (*p == '^')
            count += 3;
        else if (*p == '|')
            count += 2;
        else if (*p == '`' && (!mode || p[1] == 'g' || p[1] == 'i' ||
                               strncmp((const char *)p, "`ui", 3) == 0))
            count += 2;
    }
    return count;
}

static int append_bytes(char *output, size_t capacity, size_t *used,
                        const char *source, size_t length)
{
    if (*used >= capacity || length > capacity - *used - 1)
        return -1;
    memcpy(output + *used, source, length);
    *used += length;
    output[*used] = 0;
    return 0;
}

static int append_segment(char *output, size_t capacity, size_t *used,
                          const char *first, const char *last)
{
    size_t length = (size_t)(last - first);
    /* strncat in the matched Apple implementation stops at embedded NUL. */
    const char *zero = memchr(first, 0, length);
    if (zero)
        length = (size_t)(zero - first);
    return append_bytes(output, capacity, used, first, length);
}

static int append_special(char *output, size_t capacity, size_t *used,
                          char character)
{
    switch (character) {
    case '\\': return append_bytes(output, capacity, used, "\\\\\\\\", 4);
    case '^': return append_bytes(output, capacity, used, "\\\\^ ", 4);
    case '|': return append_bytes(output, capacity, used, "\\| ", 3);
    case '`': return append_bytes(output, capacity, used, "\\` ", 3);
    default: return -1;
    }
}

int reloq_core_prepare_text(char *input, size_t length, unsigned char mode,
                            char *output, size_t capacity,
                            const volatile unsigned char *cancelled)
{
    size_t used = 0, cursor = 0, segment = 0;
    if (!input || !output || !capacity || strlen(input) < length)
        return -1;
    output[0] = 0;
    while (cursor < length) {
        if (cancelled && *cancelled) {
            output[0] = 0;
            return 1;
        }
        char c = input[cursor];
        if (c == '|' || c == '^') {
            if (append_segment(output, capacity, &used,
                               input + segment, input + cursor) ||
                append_special(output, capacity, &used, c))
                return -1;
            segment = ++cursor;
            continue;
        }
        if (c == '\n') {
            if (!cursor || input[cursor - 1] != ' ') {
                input[cursor++] = ' ';
            } else {
                if (append_segment(output, capacity, &used,
                                   input + segment, input + cursor))
                    return -1;
                segment = ++cursor;
            }
            continue;
        }
        if (mode && c == '`') {
            if (append_segment(output, capacity, &used,
                               input + segment, input + cursor))
                return -1;
            segment = cursor;
            if (input[cursor + 1] == 'g' || input[cursor + 1] == 'i' ||
                strncmp(input + cursor, "`ui", 3) == 0) {
                if (append_bytes(output, capacity, &used, "\\` ", 3))
                    return -1;
                segment = cursor + 1;
            }
            ++cursor;
            continue;
        }
        if (c == '\\') {
            if (append_segment(output, capacity, &used,
                               input + segment, input + cursor))
                return -1;
            segment = cursor;
            if (mode && (input[cursor + 1] == '`' ||
                         input[cursor + 1] == '\\'))
                ++cursor;
            if (append_special(output, capacity, &used, input[cursor]))
                return -1;
            segment = ++cursor;
            continue;
        }
        if (!mode && c == '`') {
            if (append_segment(output, capacity, &used,
                               input + segment, input + cursor) ||
                append_special(output, capacity, &used, c))
                return -1;
            segment = ++cursor;
            continue;
        }
        ++cursor;
    }
    return append_segment(output, capacity, &used,
                          input + segment, input + cursor);
}

struct reloq_core_clock reloq_core_clock_for_rate(float sample_rate)
{
    struct reloq_core_clock clock;
    clock.milliseconds_per_sample = 1000.0f / sample_rate;
    clock.samples_per_millisecond = sample_rate * 0.001f;
    clock.radians_per_sample =
        clock.milliseconds_per_sample * 3.1415927410125732f;
    clock.radians_per_sample *= 0.001f;
    clock.twice_radians_per_sample =
        clock.radians_per_sample + clock.radians_per_sample;
    clock.rate_kind = sample_rate == 11025.0f ? 1 :
                      sample_rate == 16000.0f ? 2 :
                      sample_rate == 8000.0f ? 0 : 3;
    clock.noise_reference_rate = 0;
    clock.noise_reference_formants = 0;
    return clock;
}

int reloq_core_resonator_set_coefficients(
    struct reloq_core_resonator *filter, float radius, float cosine,
    int transition)
{
    if (!filter)
        return -1;

    float previous_gain = -radius * radius;
    float latest_gain = radius * cosine;
    latest_gain += latest_gain;
    float input_gain = 1.0f - latest_gain;
    input_gain -= previous_gain;

    if (transition) {
        float step_latest = latest_gain - filter->latest_gain;
        step_latest *= 0.25f;
        float step_previous = previous_gain - filter->previous_gain;
        step_previous *= 0.25f;
        for (int step = 0; step < 3; ++step) {
            float fraction = (float)(step + 1);
            float interpolated_latest = fraction * step_latest;
            interpolated_latest += filter->latest_gain;
            float interpolated_previous = fraction * step_previous;
            interpolated_previous += filter->previous_gain;
            float interpolated_input = 1.0f - interpolated_latest;
            interpolated_input -= interpolated_previous;
            filter->ramp_input[step] = interpolated_input;
            filter->ramp_latest[step] = interpolated_latest;
            filter->ramp_previous[step] = interpolated_previous;
        }
        filter->ramp_remaining = 3;
    } else {
        filter->ramp_remaining = 0;
    }

    filter->input_gain = input_gain;
    filter->latest_gain = latest_gain;
    filter->previous_gain = previous_gain;
    return 0;
}

int reloq_core_resonator_set_from_hz(
    struct reloq_core_resonator *filter,
    const struct reloq_core_clock *clock, float frequency_hz,
    float bandwidth_hz, int transition)
{
    if (!filter || !clock || !isfinite(frequency_hz) ||
        !isfinite(bandwidth_hz) || bandwidth_hz < 0.0f ||
        !isfinite(clock->radians_per_sample) ||
        !isfinite(clock->twice_radians_per_sample) ||
        clock->radians_per_sample <= 0.0f)
        return -1;
    float angle = frequency_hz * clock->twice_radians_per_sample;
    float decay = bandwidth_hz * -clock->radians_per_sample;
    if (!isfinite(angle) || !isfinite(decay))
        return -1;
    float cosine = cosf(angle);
    float radius = expf(decay);
    return reloq_core_resonator_set_coefficients(filter, radius, cosine,
                                                   transition);
}

int reloq_core_glottal_prefilter_configure(
    struct reloq_core_resonator *filter,
    const struct reloq_core_clock *clock, long profile_index)
{
    if (!filter || !clock || profile_index < 0 || profile_index > 35 ||
        clock->rate_kind < 0 || clock->rate_kind > 3)
        return -1;
    if (profile_index == 0) {
        filter->enabled = 0.0f;
        return 0;
    }
    if (clock->rate_kind == 3) {
        float frequency = reloq_glottal_high_frequency[profile_index];
        if (reloq_core_resonator_set_from_hz(
                filter, clock, frequency, frequency * 0.375f, 0))
            return -1;
    } else {
        const float (*profiles)[3] = clock->rate_kind == 0 ?
            reloq_glottal_8000 : reloq_glottal_11025_16000;
        filter->input_gain = profiles[profile_index][0];
        filter->latest_gain = profiles[profile_index][1];
        filter->previous_gain = profiles[profile_index][2];
        filter->ramp_remaining = 0;
    }
    filter->enabled = 20.0f;
    return 0;
}

/* Preserve a sampled source filter's physical poles in extended speech
 * profiles. The legacy numeric API still uses its original rate classes. */
static int source_profile(struct reloq_core_resonator *filter,
    const struct reloq_core_clock *clock,long index,float reference)
{
    if(reference<=0 || reference==clock->samples_per_millisecond*1000)
        return reloq_core_glottal_prefilter_configure(filter,clock,index);
    struct reloq_core_clock original=reloq_core_clock_for_rate(reference);
    if(reloq_core_glottal_prefilter_configure(filter,&original,index))return -1;
    if(!index)return 0;
    float source_gain=filter->input_gain/(1-filter->latest_gain-filter->previous_gain);
    float radius=sqrtf(-filter->previous_gain);
    float cosine=filter->latest_gain/(2*radius);
    if(cosine>1)cosine=1;
    if(cosine< -1)cosine=-1;
    float ratio=original.samples_per_millisecond/clock->samples_per_millisecond;
    int status=reloq_core_resonator_set_coefficients(filter,powf(radius,ratio),
        cosf(acosf(cosine)*ratio),0);
    filter->input_gain*=source_gain;
    return status;
}

int reloq_core_antiresonator_coefficients(
    struct reloq_core_resonator *filter, float coefficients[3])
{
    if (!filter || !coefficients || filter->input_gain == 0.0f ||
        filter->ramp_remaining < 0 || filter->ramp_remaining > 3)
        return -1;
    if (filter->ramp_remaining) {
        for (int step = 0; step < 3; ++step)
            if (filter->ramp_input[step] == 0.0f)
                return -1;
    }

    coefficients[0] = 1.0f / filter->input_gain;
    coefficients[1] = -filter->latest_gain / filter->input_gain;
    coefficients[2] = -filter->previous_gain / filter->input_gain;
    if (filter->ramp_remaining) {
        for (int step = 0; step < 3; ++step) {
            float input = filter->ramp_input[step];
            filter->ramp_input[step] = 1.0f / input;
            filter->ramp_latest[step] = -filter->ramp_latest[step] / input;
            filter->ramp_previous[step] =
                -filter->ramp_previous[step] / input;
        }
    }
    return 0;
}

int reloq_core_resonator_process(struct reloq_core_resonator *filter,
                                 float *samples, long count)
{
    if (!filter || !samples || filter->ramp_remaining < 0 ||
        filter->ramp_remaining > 3)
        return -1;
    if (filter->enabled == 0.0f)
        return 0;

    long length = count > 0 ? count : 0;
    long ramp = filter->ramp_remaining;
    long ramp_count = ramp < length ? ramp : length;
    float previous = filter->previous;
    float latest = filter->latest;
    float old_latest = latest;

    for (long i = 0; i < ramp_count; ++i) {
        long index = 3 - ramp + i;
        float lag_one = filter->ramp_latest[index] * latest;
        float input = filter->ramp_input[index] * samples[i];
        float sum = input + lag_one;
        float lag_two = filter->ramp_previous[index] * previous;
        float result = lag_two + sum;
        samples[i] = result;
        previous = latest;
        latest = result;
    }
    filter->ramp_remaining -= ramp_count;

    for (long i = ramp_count; i < length; ++i) {
        float lag_one = filter->latest_gain * latest;
        float input = filter->input_gain * samples[i];
        float sum = input + lag_one;
        float lag_two = filter->previous_gain * previous;
        float result = lag_two + sum;
        samples[i] = result;
        previous = latest;
        latest = result;
    }
    filter->previous = length ? previous : old_latest;
    filter->latest = length ? latest : old_latest;
    return 0;
}

int reloq_core_antiresonator_process(struct reloq_core_resonator *filter,
                                     const float coefficients[3],
                                     float *samples, long count)
{
    if (!filter || !coefficients || !samples || filter->ramp_remaining < 0 ||
        filter->ramp_remaining > 3)
        return -1;
    if (filter->enabled == 0.0f)
        return 0;

    long length = count > 0 ? count : 0;
    long ramp = filter->ramp_remaining;
    long ramp_count = ramp < length ? ramp : length;
    float previous = filter->previous;
    float latest = filter->latest;
    float old_latest = latest;

    for (long i = 0; i < ramp_count; ++i) {
        long index = 3 - ramp + i;
        float current = samples[i];
        float lag_one = filter->ramp_latest[index] * latest;
        float input = filter->ramp_input[index] * current;
        float sum = input + lag_one;
        float lag_two = previous * filter->ramp_previous[index];
        samples[i] = lag_two + sum;
        previous = latest;
        latest = current;
    }
    filter->ramp_remaining -= ramp_count;

    for (long i = ramp_count; i < length; ++i) {
        float current = samples[i];
        float lag_one = coefficients[1] * latest;
        float input = coefficients[0] * current;
        float sum = input + lag_one;
        float lag_two = previous * coefficients[2];
        samples[i] = lag_two + sum;
        previous = latest;
        latest = current;
    }
    filter->previous = length > 1 ? previous : old_latest;
    filter->latest = latest;
    return 0;
}

static int reloq_core_prepare_formant(
    struct reloq_core_resonator *filter,
    const struct reloq_core_clock *clock,
    const struct reloq_core_formant *specification,
    int transition, int inverse, float coefficients[3], int enabled)
{
    if (reloq_core_resonator_set_from_hz(
            filter, clock, specification->frequency_hz,
            specification->bandwidth_hz, transition))
        return -1;
    if (inverse && reloq_core_antiresonator_coefficients(
                       filter, coefficients))
        return -1;
    filter->enabled = enabled ? 20.0f : 0.0f;
    return 0;
}

int reloq_core_voiced_bank_configure(
    struct reloq_core_voiced_bank *bank,
    const struct reloq_core_clock *clock,
    const struct reloq_core_voiced_formants *frame,
    int active, int transition)
{
    if (!bank || !clock || !frame || frame->highest_formant < 0 ||
        frame->highest_formant > RELOQ_CORE_ORAL_FORMANTS)
        return -1;
    struct reloq_core_voiced_bank next = *bank;
    next.highest_formant = frame->highest_formant;
    if (reloq_core_prepare_formant(&next.nasal_pole, clock,
                                   &frame->nasal_pole, transition, 0,
                                   NULL, active) ||
        reloq_core_prepare_formant(&next.nasal_zero, clock,
                                   &frame->nasal_zero, transition, 1,
                                   next.nasal_zero_coefficients, active) ||
        reloq_core_prepare_formant(&next.tracheal_pole, clock,
                                   &frame->tracheal_pole, transition, 0,
                                   NULL, active) ||
        reloq_core_prepare_formant(&next.tracheal_zero, clock,
                                   &frame->tracheal_zero, transition, 1,
                                   next.tracheal_zero_coefficients, active))
        return -1;
    for (long f = 0; f < RELOQ_CORE_ORAL_FORMANTS; ++f) {
        if (f > 0 && f >= frame->highest_formant) {
            next.oral[f].enabled = 0.0f;
            continue;
        }
        if (reloq_core_prepare_formant(&next.oral[f], clock,
                                       &frame->oral[f], transition, 0,
                                       NULL, active))
            return -1;
    }
    *bank = next;
    return 0;
}

int reloq_core_voiced_bank_process(struct reloq_core_voiced_bank *bank,
                                    float *samples, size_t count)
{
    if (!bank || !samples || count > LONG_MAX ||
        bank->highest_formant < 0 ||
        bank->highest_formant > RELOQ_CORE_ORAL_FORMANTS)
        return -1;
    long length = (long)count;
    if (reloq_core_resonator_process(&bank->nasal_pole, samples, length) ||
        reloq_core_antiresonator_process(&bank->nasal_zero,
                                         bank->nasal_zero_coefficients,
                                         samples, length) ||
        reloq_core_resonator_process(&bank->tracheal_pole, samples, length) ||
        reloq_core_antiresonator_process(&bank->tracheal_zero,
                                         bank->tracheal_zero_coefficients,
                                         samples, length))
        return -1;
    for (long f = bank->highest_formant; f >= 2; --f)
        if (reloq_core_resonator_process(&bank->oral[f - 1],
                                         samples, length))
            return -1;
    return reloq_core_resonator_process(&bank->oral[0], samples, length);
}

static struct reloq_core_resonator *reloq_core_serial_stage(
    struct reloq_core_voiced_bank *bank, int index)
{
    switch (index) {
    case 0: return &bank->nasal_pole;
    case 1: return &bank->nasal_zero;
    case 2: return &bank->tracheal_pole;
    case 3: return &bank->tracheal_zero;
    default: return &bank->oral[index - 4];
    }
}

static const struct reloq_core_formant *reloq_core_serial_specification(
    const struct reloq_core_voiced_formants *frame, int index)
{
    switch (index) {
    case 0: return &frame->nasal_pole;
    case 1: return &frame->nasal_zero;
    case 2: return &frame->tracheal_pole;
    case 3: return &frame->tracheal_zero;
    default: return &frame->oral[index - 4];
    }
}

int reloq_core_serial_frame_begin(
    struct reloq_core_serial_envelope *envelope,
    struct reloq_core_voiced_bank *bank,
    const struct reloq_core_clock *clock,
    const struct reloq_core_voiced_formants *frame,
    long av_level, long ah_level,
    size_t sample_count, float sample_rate)
{
    if (!envelope || !bank || !clock || !frame || envelope->frame_open ||
        av_level < 0 ||
        frame->highest_formant < 1 ||
        frame->highest_formant > RELOQ_CORE_ORAL_FORMANTS ||
        !isfinite(sample_rate) || sample_rate <= 0.0f ||
        sample_count > SIZE_MAX / 1000u)
        return -1;
    float elapsed_ms = (float)(sample_count * 1000u);
    elapsed_ms /= sample_rate;
    if (!isfinite(elapsed_ms))
        return -1;
    struct reloq_core_serial_envelope next = *envelope;
    struct reloq_core_voiced_bank result = *bank;
    if (av_level || ah_level)
        next.remaining_ms = 20.0f;
    float entry = next.remaining_ms;
    if (!isfinite(entry) || entry < 0.0f)
        return -1;
    result.highest_formant = frame->highest_formant;
    int nasal_pair_equal =
        frame->nasal_pole.frequency_hz == frame->nasal_zero.frequency_hz &&
        frame->nasal_pole.bandwidth_hz == frame->nasal_zero.bandwidth_hz;
    int tracheal_pair_equal =
        frame->tracheal_pole.frequency_hz ==
            frame->tracheal_zero.frequency_hz &&
        frame->tracheal_pole.bandwidth_hz ==
            frame->tracheal_zero.bandwidth_hz;
    for (int i = 0; i < RELOQ_CORE_SERIAL_STAGES; ++i) {
        if (i >= 4 && i - 4 >= frame->highest_formant)
            continue;
        struct reloq_core_resonator *stage =
            reloq_core_serial_stage(&result, i);
        const struct reloq_core_formant *spec =
            reloq_core_serial_specification(frame, i);
        const struct reloq_core_serial_stage_cache *cache =
            &next.stages[i];
        int enable = i < 4 ? entry > 0x1.4f8b58p-17f : entry != 0.0f;
        if (enable && i < 4 &&
            (i < 2 ? nasal_pair_equal : tracheal_pair_equal)) {
            /* The matching pole and inverse-zero pair is a bypass. Keep
             * its old coefficients until an unequal pair reactivates it. */
            stage->enabled = 0.0f;
            continue;
        }
        if (!enable) {
            if (i >= 4) {
                stage->enabled = 0.0f;
            } else if (stage->enabled != 0.0f) {
                /* Even with the global marker at zero, the named stages'
                 * stale gates enter the generic coefficient bookkeeping.
                 * A changed saved F/B rearms raw target rows without
                 * replacing the old target or its retained history. */
                if (spec->frequency_hz != cache->frequency_hz ||
                    spec->bandwidth_hz != cache->bandwidth_hz) {
                    for (int row = 0; row < 3; ++row) {
                        stage->ramp_input[row] = stage->input_gain;
                        stage->ramp_latest[row] = stage->latest_gain;
                        stage->ramp_previous[row] = stage->previous_gain;
                    }
                    stage->ramp_remaining = 3;
                } else {
                    stage->ramp_remaining = 0;
                }
            }
            continue;
        }
        if (spec->frequency_hz != cache->frequency_hz ||
            spec->bandwidth_hz != cache->bandwidth_hz) {
            float *inverse = i == 1 ? result.nasal_zero_coefficients :
                             i == 3 ? result.tracheal_zero_coefficients :
                                      NULL;
            if (reloq_core_prepare_formant(
                    stage, clock, spec, cache->frequency_hz != 0.0f,
                    inverse != NULL, inverse, 1))
                return -1;
        }
        stage->enabled = entry;
    }
    next.elapsed_ms = elapsed_ms;
    next.frame = *frame;
    next.release = av_level == 0 && ah_level == 0;
    next.process = entry > 0x1.a36e2ep-14f;
    next.frame_open = 1;
    *envelope = next;
    *bank = result;
    return 0;
}

int reloq_core_serial_frame_process(
    const struct reloq_core_serial_envelope *envelope,
    struct reloq_core_voiced_bank *bank,
    float *samples, size_t count)
{
    if (!envelope || !bank || !samples || !envelope->frame_open)
        return -1;
    return envelope->process ?
        reloq_core_voiced_bank_process(bank, samples, count) : 0;
}

int reloq_core_serial_frame_end(
    struct reloq_core_serial_envelope *envelope,
    struct reloq_core_voiced_bank *bank)
{
    if (!envelope || !bank || !envelope->frame_open)
        return -1;
    struct reloq_core_serial_envelope next = *envelope;
    struct reloq_core_voiced_bank result = *bank;
    if (next.release) {
        next.remaining_ms -= next.elapsed_ms;
        if (next.remaining_ms < 0x1.4f8b58p-17f)
            next.remaining_ms = 0.0f;
    }
    for (int i = 0; i < RELOQ_CORE_SERIAL_STAGES; ++i) {
        if (i >= 4 && i - 4 >= next.frame.highest_formant)
            continue;
        struct reloq_core_resonator *stage =
            reloq_core_serial_stage(&result, i);
        struct reloq_core_serial_stage_cache *cache =
            &next.stages[i];
        if (stage->enabled == 0.0f) {
            stage->previous = 0.0f;
            stage->latest = 0.0f;
            cache->frequency_hz = 0.0f;
            cache->bandwidth_hz = 0.0f;
        } else {
            const struct reloq_core_formant *spec =
                reloq_core_serial_specification(&next.frame, i);
            cache->frequency_hz = spec->frequency_hz;
            cache->bandwidth_hz = spec->bandwidth_hz;
        }
    }
    next.frame_open = 0;
    *envelope = next;
    *bank = result;
    return 0;
}

static int reloq_core_add_long(long a, long b, long *sum)
{
    if ((b > 0 && a > LONG_MAX - b) ||
        (b < 0 && a < LONG_MIN - b))
        return -1;
    *sum = a + b;
    return 0;
}

/* Squared impulse-response norm of A/(1-Bz^-1-Cz^-2). This follows from
 * the two autocorrelation recurrences, including both feedback terms.
 * Equal DC gain is not equal noise gain when the sampling clock changes.
 * Double precision avoids cancellation for narrow, low-frequency poles. */
static double noise_power(double frequency,double bandwidth,double rate)
{
    const double pi=3.14159265358979323846;
    double r=exp(-pi*bandwidth/rate),c=-r*r;
    double b=2*r*cos(2*pi*frequency/rate),a=1-b-c;
    return a*a*(1-c)/((1+c)*((1-c)*(1-c)-b*b));
}

/* Fourth-order maximally-flat low-pass for the unshaped AB branch.
 * The reference's sample clock bounded this noise at its Nyquist. Merely
 * copying its gain to a wider clock changes both spectral density and
 * centroid. Preserve that physical band, normalizing the actual impulse
 * energy once per clock change. The new upper resonators remain separate. */
static float bypass_sample(float c[2][5],float d[2][2],float x)
{
    for(unsigned i=0;i<2;++i) {
        float y=c[i][0]*x+d[i][0];
        d[i][0]=c[i][1]*x-c[i][3]*y+d[i][1];
        d[i][1]=c[i][2]*x-c[i][4]*y;x=y;
    }
    return x;
}
static void bypass_clock(struct reloq_core_frication_bank *bank,float rate,float reference)
{
    if(bank->bypass_rate==rate && bank->bypass_reference==reference)return;
    bank->bypass_rate=rate;bank->bypass_reference=reference;
    bank->bypass_scale=1;
    memset(bank->bypass_delay,0,sizeof bank->bypass_delay);
    if(reference<=0 || rate<=reference)return;
    const double pi=3.14159265358979323846;
    const double q[2]={0.541196100146197,1.306562964876377};
    double k=tan(pi*reference*.5/rate);
    for(unsigned i=0;i<2;++i) {
        double norm=1/(1+k/q[i]+k*k);float *c=bank->bypass_coefficients[i];
        c[0]=(float)(k*k*norm);c[1]=2*c[0];c[2]=c[0];
        c[3]=(float)(2*(k*k-1)*norm);c[4]=(float)((1-k/q[i]+k*k)*norm);
    }
    float history[2][2]={{0}};double energy=0;
    for(unsigned i=0;i<2048;++i) {
        double y=bypass_sample(bank->bypass_coefficients,history,i==0?1:0);
        energy+=y*y;
    }
    if(energy>0 && isfinite(energy))bank->bypass_scale=(float)(1/sqrt(energy));
}

int reloq_core_frication_bank_configure(
    struct reloq_core_frication_bank *bank,
    const struct reloq_core_clock *clock,
    const float centers_hz[RELOQ_CORE_FRICATIVE_FORMANTS],
    const float bandwidths_hz[RELOQ_CORE_FRICATIVE_FORMANTS],
    const long levels[RELOQ_CORE_FRICATIVE_FORMANTS],
    long active_formants, long global_level, long source_offset, long source_trim,
    float bypass_gain)
{
    if (!bank || !clock || !centers_hz || !bandwidths_hz || !levels ||
        !isfinite(bypass_gain) || active_formants < 1 ||
        active_formants > RELOQ_CORE_FRICATIVE_FORMANTS)
        return -1;
    struct reloq_core_frication_bank next = *bank;
    /* AF has its own reference clock. At that clock the original numeric
     * bank is used directly; wider clocks preserve its spectral density.
     * The voiced tract's reference rate must not recolour this source. */
    next.bypass_gain = bypass_gain;
    float bandwidth=clock->noise_reference_rate;
    bypass_clock(&next,clock->samples_per_millisecond*1000,bandwidth);
    next.active_formants = active_formants;
    for (int stage = 0; stage < active_formants; ++stage) {
        struct reloq_core_resonator *filter = &next.stages[stage];
        /* A released AiF stage is left untouched until it is reactivated.
         * In particular its last coefficients survive a silent AF gap. */
        if (levels[stage] == 0 && filter->enabled == 0.0f)
            continue;
        if (reloq_core_resonator_set_from_hz(
                filter, clock, centers_hz[stage], bandwidths_hz[stage],
                0))
            return -1;
        float scale = 0.0f;
        if (levels[stage] != 0) {
            long level;
            if (reloq_core_add_long(levels[stage], source_offset, &level) ||
                reloq_core_add_long(level, source_trim, &level) ||
                reloq_core_add_long(level, global_level, &level))
                return -1;
            float polarity = (stage & 1) ? -0x1.cdc72p-29f :
                                           0x1.cdc72p-29f;
            scale = reloq_core_level_gain(level) * polarity;
            if(clock->noise_reference_rate>0 && bandwidths_hz[stage]>0) {
                double rate=clock->samples_per_millisecond*1000.0;
                /* New upper channels inherit the measured last band's
                 * noise energy. Their relative levels are supplied by the
                 * voice profile; do not fold them around the old Nyquist. */
                int anchor=stage<5?stage:4;
                double before=noise_power(centers_hz[anchor],
                    bandwidths_hz[anchor],clock->noise_reference_rate);
                double after=noise_power(centers_hz[stage],
                    bandwidths_hz[stage],rate);
                if(isfinite(before) && isfinite(after) && before>0 && after>0)
                    scale*=(float)sqrt(before/after);
            }
        }
        filter->input_gain *= scale;
        if (global_level != 0 && levels[stage] != 0)
            filter->enabled = 20.0f;
    }
    *bank = next;
    return 0;
}

void reloq_core_frication_bank_colour(struct reloq_core_frication_bank *bank,
    float sample_rate,float attenuation_db)
{
    if(!bank || !isfinite(sample_rate) || sample_rate<16000 ||
       !isfinite(attenuation_db) || attenuation_db<0 || attenuation_db>12)return;
    if(bank->colour_rate==sample_rate && bank->colour_db==attenuation_db)return;
    const double k=tan(3.14159265358979323846*4000/sample_rate);
    const double g=pow(10,-attenuation_db/20);
    bank->colour_rate=sample_rate;bank->colour_db=attenuation_db;
    bank->colour_coefficients[0]=(float)((k+g)/(k+1));
    bank->colour_coefficients[1]=(float)((k-g)/(k+1));
    bank->colour_coefficients[2]=(float)((k-1)/(k+1));
}

int reloq_core_frication_bank_mix(struct reloq_core_frication_bank *bank,
                                  const float *source, float *destination,
                                  float *scratch, size_t count)
{
    if (!bank || !source || !destination || !scratch ||
        source == destination || source == scratch ||
        destination == scratch || count > LONG_MAX ||
        count > SIZE_MAX / sizeof *scratch)
        return -1;
    long length = (long)count;
    if(bank->shaped_source) {
        reloq_core_mix_float(source,bank->bypass_gain,destination,length);
    } else if(bank->colour_rate>0) {
        const int limited=bank->bypass_reference>0 && bank->bypass_rate>bank->bypass_reference;
        float gain=bank->bypass_gain*(limited?bank->bypass_scale:1);
        const float *c=bank->colour_coefficients;
        for(size_t i=0;i<count;++i) {
            float x=limited?bypass_sample(bank->bypass_coefficients,bank->bypass_delay,source[i]):source[i];
            float y=c[0]*x+bank->colour_delay;
            bank->colour_delay=c[1]*x-c[2]*y;
            destination[i]+=gain*y;
        }
    } else if(bank->bypass_reference>0 && bank->bypass_rate>bank->bypass_reference) {
        float gain=bank->bypass_gain*bank->bypass_scale;
        for(size_t i=0;i<count;++i)
            destination[i]+=gain*bypass_sample(bank->bypass_coefficients,
                bank->bypass_delay,source[i]);
    } else reloq_core_mix_float(source, bank->bypass_gain, destination, length);
    for (int stage = 0; stage < bank->active_formants; ++stage) {
        if (bank->stages[stage].enabled == 0.0f)
            continue;
        memcpy(scratch, source, count * sizeof *scratch);
        if (reloq_core_resonator_process(&bank->stages[stage],
                                         scratch, length))
            return -1;
        for (long i = 0; i < length; ++i)
            destination[i] += scratch[i];
    }
    return 0;
}

int reloq_core_frication_envelope_begin(
    struct reloq_core_frication_envelope *envelope,
    struct reloq_core_frication_bank *bank,
    long global_level,
    const long levels[RELOQ_CORE_FRICATIVE_FORMANTS],
    size_t sample_count, float sample_rate)
{
    if (!envelope || !bank || !levels || envelope->frame_open ||
        !isfinite(sample_rate) || sample_rate <= 0.0f ||
        sample_count > SIZE_MAX / 1000u)
        return -1;
    float elapsed_ms = (float)(sample_count * 1000u);
    elapsed_ms /= sample_rate;
    if (!isfinite(elapsed_ms))
        return -1;
    float global_ms = global_level != 0 ? 20.0f : envelope->global_ms;
    for (int i = 0; i < bank->active_formants; ++i) {
        envelope->zero_level[i] = levels[i] == 0;
        if (global_ms > 0x1.a36e2ep-14f) {
            if (levels[i] != 0)
                bank->stages[i].enabled = global_ms;
        } else {
            bank->stages[i].enabled = 0.0f;
        }
    }
    envelope->global_ms = global_ms;
    envelope->elapsed_ms = elapsed_ms;
    envelope->release_global = global_level == 0;
    envelope->frame_open = 1;
    return 0;
}

int reloq_core_frication_envelope_end(
    struct reloq_core_frication_envelope *envelope,
    struct reloq_core_frication_bank *bank)
{
    if (!envelope || !bank || !envelope->frame_open)
        return -1;
    for (int i = 0; i < bank->active_formants; ++i)
        if (envelope->zero_level[i] && bank->stages[i].enabled != 0.0f) {
            float remaining = bank->stages[i].enabled -
                              envelope->elapsed_ms;
            bank->stages[i].enabled = remaining < 0.0f ? 0.0f : remaining;
        }
    if (envelope->release_global &&
        envelope->global_ms > 0x1.a36e2ep-14f) {
        float remaining = envelope->global_ms - envelope->elapsed_ms;
        envelope->global_ms = remaining < 0x1.a36e2ep-14f ? 0.0f :
                              remaining;
    }
    for (int i = 0; i < bank->active_formants; ++i)
        if (bank->stages[i].enabled == 0.0f) {
            bank->stages[i].previous = 0.0f;
            bank->stages[i].latest = 0.0f;
        }
    envelope->frame_open = 0;
    return 0;
}

struct reloq_core_frame_counts reloq_core_frame_counts_for(
    float duration_ms, float phase_ms, float split,
    float samples_per_millisecond)
{
    struct reloq_core_frame_counts counts;
    float total = duration_ms - phase_ms;
    total *= samples_per_millisecond;
    counts.total = (long)((double)total + 0.9999);

    float first = duration_ms * split;
    first -= phase_ms;
    first *= samples_per_millisecond;
    counts.first = (long)((double)first + 0.4999);
    counts.second = counts.total - counts.first;
    return counts;
}

long reloq_core_frame_sample_count(float duration_ms, float duration_scale,
                                    float samples_per_millisecond)
{
    if (duration_ms < 0.0f || duration_scale <= 0.0f ||
        samples_per_millisecond <= 0.0f)
        return -1;
    float scaled = duration_ms * duration_scale;
    scaled *= samples_per_millisecond;
    if (!isfinite(scaled) || (double)scaled >= (double)LONG_MAX)
        return -1;
    return (long)scaled;
}

float reloq_core_next_phase(long samples, float milliseconds_per_sample,
                             float phase_ms, float duration_ms)
{
    /* Keep the Windows source clock's intermediate precision. */
    double next = (double)samples * (double)milliseconds_per_sample;
    next += (double)phase_ms;
    next -= (double)duration_ms;
    return (float)next;
}

uint16_t reloq_core_noise_fill(uint16_t *samples, size_t count, int32_t seed)
{
    uint16_t value = (uint16_t)((uint32_t)seed * 20077u + 12345u);
    for (size_t i = 0; i < count; ++i) {
        if (i)
            value = (uint16_t)((uint32_t)value * 20077u + 12345u);
        samples[i] = value;
    }
    return value;
}

int reloq_core_aspiration_configure(struct reloq_core_aspiration *aspiration,
                                    int32_t seed, long level,
                                    long first_offset, long second_offset)
{
    if (!aspiration)
        return -1;
    aspiration->seed = (uint16_t)seed;
    aspiration->enabled = level != 0;
    aspiration->gain = 0.0f;
    if (!aspiration->enabled)
        return 0;
    if ((first_offset > 0 && level > LONG_MAX - first_offset) ||
        (first_offset < 0 && level < LONG_MIN - first_offset))
        return -1;
    level += first_offset;
    if ((second_offset > 0 && level > LONG_MAX - second_offset) ||
        (second_offset < 0 && level < LONG_MIN - second_offset))
        return -1;
    level += second_offset;
    aspiration->gain = reloq_core_level_gain(level) * 0x1.90032p-17f;
    return 0;
}

int reloq_core_aspiration_mix(struct reloq_core_aspiration *aspiration,
                              float *destination, size_t count,
                              const struct reloq_core_noise_gate *gate)
{
    if (!aspiration || (count && !destination) ||
        (gate && !gate->closed_mask &&
         gate->interval_count && !gate->intervals))
        return -1;
    if (!aspiration->enabled)
        return 0;
    size_t total_count = count;
    size_t position = 0;
    size_t start = 0;
    size_t end = gate ? gate->initial_attenuate : 0;
    if (end > count)
        end = count;
    size_t interval = 0;
    while (count) {
        uint16_t noise[200];
        size_t n = count < 200 ? count : 200;
        aspiration->seed = reloq_core_noise_fill(
            noise, n, (int32_t)aspiration->seed);
        for (size_t i = 0; i < n; ++i) {
            while (gate && !gate->closed_mask && position >= end &&
                   interval < gate->interval_count) {
                size_t skip = gate->intervals[interval].skip;
                size_t length = gate->intervals[interval].attenuate;
                size_t remaining = total_count - end;
                start = end + (skip < remaining ? skip : remaining);
                remaining = total_count - start;
                end = start + (length < remaining ? length : remaining);
                ++interval;
            }
            int32_t signed_noise = noise[i] < 0x8000u ?
                (int32_t)noise[i] : (int32_t)noise[i] - 0x10000;
            if (gate && (gate->closed_mask ? gate->closed_mask[position] :
                         position >= start && position < end))
                signed_noise = signed_noise >= 0 ?
                    signed_noise / 2 : -((1 - signed_noise) / 2);
            destination[i] += (float)signed_noise * aspiration->gain;
            ++position;
        }
        destination += n;
        count -= n;
    }
    return 0;
}

int reloq_core_frication_configure(struct reloq_core_frication *frication,
                                   int32_t seed, long level,
                                   float bypass_level, long first_offset,
                                   long second_offset)
{
    if (!frication || !isfinite(bypass_level) ||
        (double)bypass_level >= (double)LONG_MAX ||
        (double)bypass_level <= (double)LONG_MIN)
        return -1;
    frication->seed = (uint16_t)seed;
    frication->enabled = level != 0;
    frication->bypass_gain = 0.0f;
    if (bypass_level == 0.0f)
        return 0;
    long bypass = (long)bypass_level;
    if ((first_offset > 0 && level > LONG_MAX - first_offset) ||
        (first_offset < 0 && level < LONG_MIN - first_offset))
        return -1;
    level += first_offset;
    if ((second_offset > 0 && level > LONG_MAX - second_offset) ||
        (second_offset < 0 && level < LONG_MIN - second_offset))
        return -1;
    level += second_offset;
    if ((bypass > 0 && level > LONG_MAX - bypass) ||
        (bypass < 0 && level < LONG_MIN - bypass))
        return -1;
    level += bypass;
    frication->bypass_gain =
        reloq_core_level_gain(level) * -0x1.cdc72p-29f;
    return 0;
}

int reloq_core_frication_fill(struct reloq_core_frication *frication,
                              float *samples, size_t count)
{
    return reloq_core_frication_fill_with_mask(frication, samples, NULL,
                                                count);
}

int reloq_core_frication_fill_with_mask(
    struct reloq_core_frication *frication, float *samples,
    const uint8_t *closed_mask, size_t count)
{
    if (!frication || (count && !samples))
        return -1;
    if (!frication->enabled) {
        for (size_t i = 0; i < count; ++i)
            samples[i] = 0.0f;
        return 0;
    }
    size_t position = 0;
    while (count) {
        uint16_t noise[200];
        size_t n = count < 200 ? count : 200;
        frication->seed = reloq_core_noise_fill(
            noise, n, (int32_t)frication->seed);
        for (size_t i = 0; i < n; ++i) {
            int32_t signed_noise = noise[i] < 0x8000u ?
                (int32_t)noise[i] : (int32_t)noise[i] - 0x10000;
            if (closed_mask && closed_mask[position + i])
                signed_noise = signed_noise >= 0 ?
                    signed_noise / 2 : -((1 - signed_noise) / 2);
            samples[i] = (float)signed_noise;
        }
        samples += n;
        count -= n;
        position += n;
    }
    return 0;
}

float reloq_core_level_gain(long level)
{
    static const float decades[21] = {
        0x1p+0f, 0x1.4p+3f, 0x1.9p+6f, 0x1.f4p+9f,
        0x1.388p+13f, 0x1.86ap+16f, 0x1.e848p+19f, 0x1.312dp+23f,
        0x1.7d784p+26f, 0x1.dcd65p+29f, 0x1.2a05f2p+33f,
        0x1.74876ep+36f, 0x1.d1a94ap+39f, 0x1.2309cep+43f,
        0x1.6bcc42p+46f, 0x1.c6bf52p+49f, 0x1.1c3794p+53f,
        0x1.634578p+56f, 0x1.bc16d6p+59f, 0x1.158e46p+63f,
        0x1.5af1d8p+66f
    };
    static const float twentieths[20] = {
        0x1p+0f, 0x1.1f3c92p+0f, 0x1.4248e8p+0f, 0x1.699c18p+0f,
        0x1.95bb8cp+0f, 0x1.c73d4ap+0f, 0x1.fec97ep+0f,
        0x1.1e8e68p+1f, 0x1.41857ap+1f, 0x1.68c0c6p+1f,
        0x1.94c586p+1f, 0x1.c62942p+1f, 0x1.fd93c4p+1f,
        0x1.1de0a4p+2f, 0x1.40c282p+2f, 0x1.67e6p+2f,
        0x1.93d01p+2f, 0x1.c515d8p+2f, 0x1.fc5ebcp+2f,
        0x1.1d3346p+3f
    };
    if (level <= 0)
        return 0.0f;
    if (level > 400)
        level = 400;
    return decades[level / 20] * twentieths[level % 20];
}

int reloq_core_pulse_configure(struct reloq_core_pulse *pulse,
                                const struct reloq_core_clock *clock,
                                float pitch_tenths, float open_percent,
                                long voice_level)
{
    if (!pulse || !clock || !isfinite(pitch_tenths) ||
        !isfinite(open_percent) ||
        !isfinite(clock->samples_per_millisecond) ||
        pitch_tenths <= 0.0f || open_percent < 0.0f ||
        voice_level <= 0 || clock->samples_per_millisecond <= 0.0f)
        return -1;
    /* A release target can close the source before AV reaches zero.
     * Keep the period clock and closed mask, without dividing by a zero
     * open duration or generating a new excitation pulse. */
    if (open_percent == 0.0f) {
        *pulse = (struct reloq_core_pulse){0};
        return 0;
    }
    float period_ms = 10000.0f / pitch_tenths;
    float period_samples = clock->samples_per_millisecond * period_ms;
    float open_fraction = open_percent * 0.01f;
    float open_samples = open_fraction * period_samples;
    float base = 1.5f / open_samples;
    float gain = reloq_core_level_gain(voice_level);
    float increment = gain * 0x1.278002p+4f;
    increment *= base;
    if (!isfinite(increment) || !isfinite(base) || increment <= 0.0f)
        return -1;
    float curvature = base / increment;
    if (!isfinite(curvature))
        return -1;
    pulse->phase = 0.0f;
    pulse->increment = increment;
    pulse->curvature = curvature;
    return 0;
}

int reloq_core_pulse_fill(struct reloq_core_pulse *pulse,
                          float *samples, size_t count)
{
    if (!pulse || !samples)
        return -1;
    float phase = pulse->phase;
    for (size_t i = 0; i < count; ++i) {
        float square = pulse->curvature * phase;
        square *= phase;
        samples[i] = phase - square;
        phase += pulse->increment;
    }
    pulse->phase = phase;
    return 0;
}

int reloq_core_voiced_configure(struct reloq_core_voiced_state *voice,
                                 float sample_rate, float pitch_tenths,
                                 float open_percent, long voice_level)
{
    if (!voice || !isfinite(sample_rate) || sample_rate <= 0.0f ||
        !isfinite(open_percent) || open_percent > 100.0f)
        return -1;
    struct reloq_core_clock clock =
        reloq_core_clock_for_rate(sample_rate);
    struct reloq_core_pulse pulse;
    if (reloq_core_pulse_configure(&pulse, &clock, pitch_tenths,
                                   open_percent, voice_level))
        return -1;
    float period_ms = 10000.0f / pitch_tenths;
    float period_samples = period_ms * clock.samples_per_millisecond;
    if (!isfinite(period_samples) || period_samples < 1.0f ||
        (double)period_samples >= (double)LONG_MAX)
        return -1;
    voice->clock = clock;
    voice->sample_rate = sample_rate;
    voice->pulse = pulse;
    voice->period_ms = period_ms;
    voice->open_fraction = open_percent * 0.01f;
    voice->next_phase_ms = 0.0f;
    voice->open_left = 0;
    voice->closed_left = 0;
    voice->pending_pitch_tenths = pitch_tenths;
    voice->pending_open_percent = open_percent;
    voice->pending_voice_level = voice_level;
    voice->pending_change = 0;
    voice->period_live = 1;
    voice->frame_av_positive = 1;
    voice->base_pulse = pulse;
    voice->elapsed_ms = 0.0f;
    voice->frame_duration_ms = 0.0f;
    voice->frame_duration_scale = 0.0f;
    voice->frame_pitch_tenths = 0.0f;
    voice->frame_flutter = 0.0f;
    voice->frame_diplophonia = 0.0f;
    voice->pre_left = 0;
    voice->diplophonia_cut = 0;
    voice->flutter_delta = 0;
    voice->frame_samples_remaining = 0;
    voice->diplophonia_active = 0;
    voice->next_attenuated = 0;
    voice->frame_period_configured = 0;
    voice->modulated_frame_open = 0;
    return 0;
}

int reloq_core_voiced_initialize(struct reloq_core_voiced_state *voice,
                                  float sample_rate)
{
    if (!voice || !isfinite(sample_rate) || sample_rate <= 0.0f)
        return -1;
    struct reloq_core_voiced_state next = {0};
    next.clock = reloq_core_clock_for_rate(sample_rate);
    next.sample_rate = sample_rate;
    *voice = next;
    return 0;
}

int reloq_core_voiced_begin_frame(struct reloq_core_voiced_state *voice,
                                   float pitch_tenths, float open_percent,
                                   long voice_level)
{
    if (!voice || voice_level < 0 ||
        voice->clock.samples_per_millisecond <= 0.0f)
        return -1;
    if (voice_level > 0) {
        if (voice->period_live) {
            if (reloq_core_voiced_schedule(voice, pitch_tenths,
                                           open_percent, voice_level))
                return -1;
        } else {
            if (reloq_core_voiced_configure(voice, voice->sample_rate,
                                            pitch_tenths, open_percent,
                                            voice_level))
                return -1;
        }
    }
    voice->frame_av_positive = voice_level > 0;
    return 0;
}

int reloq_core_voiced_begin_modulated_frame(
    struct reloq_core_voiced_state *voice,
    const struct reloq_core_voiced_modulation_frame *frame)
{
    if (!voice || !frame || voice->modulated_frame_open ||
        !isfinite(frame->duration_ms) || frame->duration_ms <= 0.0f ||
        !isfinite(frame->duration_scale) || frame->duration_scale <= 0.0f ||
        !isfinite(frame->pitch_tenths) ||
        !isfinite(frame->open_percent) ||
        !isfinite(frame->flutter) || frame->flutter < 0.0f ||
        !isfinite(frame->diplophonia) ||
        frame->diplophonia < 0.0f || frame->diplophonia > 100.0f ||
        frame->av_level < 0)
        return -1;
    long count = reloq_core_frame_sample_count(
        frame->duration_ms, frame->duration_scale,
        voice->clock.samples_per_millisecond);
    if (count < 0)
        return -1;
    struct reloq_core_voiced_state next = *voice;
    float elapsed = next.elapsed_ms;
    int active = next.diplophonia_active;
    int alternating = next.next_attenuated;
    int source_active = frame->av_level > 0 || frame->period_active;
    int fresh = source_active && !next.period_live;
    if (source_active && frame->av_level == 0) {
        if (next.period_live) {
            if (reloq_core_voiced_schedule(
                    &next, frame->pitch_tenths, frame->open_percent, 1))
                return -1;
        } else if (reloq_core_voiced_configure(
                       &next, next.sample_rate, frame->pitch_tenths,
                       frame->open_percent, 1)) {
            return -1;
        }
        next.pending_voice_level = 0;
        next.frame_av_positive = 1;
    } else if (reloq_core_voiced_begin_frame(
                   &next, frame->pitch_tenths, frame->open_percent,
                   frame->av_level)) {
        return -1;
    }
    if (fresh) {
        next.elapsed_ms = elapsed;
        next.diplophonia_active = active;
        next.next_attenuated = alternating;
    }
    next.frame_duration_ms = frame->duration_ms;
    next.frame_duration_scale = frame->duration_scale;
    next.frame_pitch_tenths = frame->pitch_tenths;
    next.frame_flutter = frame->flutter;
    next.frame_diplophonia = frame->diplophonia;
    next.frame_period_configured = 0;
    next.frame_samples_remaining = (size_t)count;
    next.modulated_frame_open = 1;
    *voice = next;
    return 0;
}

int reloq_core_voiced_end_modulated_frame(
    struct reloq_core_voiced_state *voice)
{
    if (!voice || !voice->modulated_frame_open ||
        voice->frame_samples_remaining)
        return -1;
    float delta = voice->frame_duration_ms * voice->frame_duration_scale;
    float elapsed = voice->elapsed_ms + delta;
    if (!isfinite(delta) || !isfinite(elapsed))
        return -1;
    voice->elapsed_ms = elapsed;
    voice->modulated_frame_open = 0;
    return 0;
}

int reloq_core_voiced_fill_frame_with_mask(
    struct reloq_core_voiced_state *voice, float *samples,
    uint8_t *closed_mask, size_t count)
{
    if (!voice || !samples || count > LONG_MAX ||
        (voice->modulated_frame_open &&
         count > voice->frame_samples_remaining))
        return -1;
    if (voice->frame_av_positive) {
        if (reloq_core_voiced_fill_with_mask(
                voice, samples, closed_mask, count))
            return -1;
        if (voice->modulated_frame_open)
            voice->frame_samples_remaining -= count;
        return 0;
    }
    if (closed_mask)
        memset(closed_mask, 0, count);
    size_t carried = 0;
    if (voice->period_live) {
        if (voice->pre_left < 0 || voice->open_left < 0 ||
            voice->closed_left < 0)
            return -1;
        carried = (size_t)voice->pre_left < count ?
                  (size_t)voice->pre_left : count;
        carried += (size_t)voice->open_left < count - carried ?
                   (size_t)voice->open_left : count - carried;
        carried += (size_t)voice->closed_left < count - carried ?
                   (size_t)voice->closed_left : count - carried;
        if (carried && reloq_core_voiced_fill(voice, samples, carried))
            return -1;
    }
    if (carried < count) {
        reloq_core_clear(samples + carried, (long)(count - carried));
        voice->period_live = 0;
        voice->next_phase_ms = 0.0f;
    }
    if (voice->modulated_frame_open)
        voice->frame_samples_remaining -= count;
    return 0;
}

int reloq_core_voiced_schedule(struct reloq_core_voiced_state *voice,
                               float pitch_tenths, float open_percent,
                               long voice_level)
{
    if (!voice || !isfinite(pitch_tenths) || pitch_tenths <= 0.0f ||
        !isfinite(open_percent) || open_percent < 0.0f ||
        open_percent > 100.0f || voice_level <= 0)
        return -1;
    float period_ms = 10000.0f / pitch_tenths;
    float samples = period_ms * voice->clock.samples_per_millisecond;
    if (!isfinite(samples) || samples < 1.0f ||
        (double)samples >= (double)LONG_MAX)
        return -1;
    voice->pending_pitch_tenths = pitch_tenths;
    voice->pending_open_percent = open_percent;
    voice->pending_voice_level = voice_level;
    voice->pending_change = 1;
    return 0;
}

static int reloq_core_voiced_prepare_modulated_period(
    struct reloq_core_voiced_state *voice)
{
    if (!voice->frame_period_configured) {
        float pitch = voice->frame_pitch_tenths;
        voice->flutter_delta = 0;
        if (voice->frame_flutter != 0.0f) {
            float scale = voice->frame_flutter * pitch;
            scale *= 0x1.a36e2ep-13f;
            float wobble = sinf(voice->elapsed_ms * 0x1.46d8a6p-4f);
            wobble += sinf(voice->elapsed_ms * 0x1.6d733ep-5f);
            wobble += sinf(voice->elapsed_ms * 0x1.e3d5dep-6f);
            float change = wobble * scale;
            if (!isfinite(change) ||
                (double)change >= (double)LONG_MAX ||
                (double)change <= (double)LONG_MIN)
                return -1;
            voice->flutter_delta = (long)change;
            pitch += (float)voice->flutter_delta;
        }
        if (voice->pending_voice_level > 0) {
            if (reloq_core_pulse_configure(
                    &voice->base_pulse, &voice->clock, pitch,
                    voice->pending_open_percent,
                    voice->pending_voice_level))
                return -1;
        } else {
            /* A raw AV can retain period timing and the noise mask while
             * its adjusted gain mutes newly scheduled pulses. */
            voice->base_pulse = (struct reloq_core_pulse){0};
        }
        voice->period_ms = 10000.0f / pitch;
        voice->open_fraction =
            voice->pending_open_percent * 0x1.47ae14p-7f;
        voice->pending_change = 0;
        float di = voice->frame_diplophonia;
        if (voice->diplophonia_active && di == 0.0f) {
            voice->diplophonia_active = 0;
            voice->next_attenuated = 0;
        } else if (!voice->diplophonia_active && di > 0.0f) {
            voice->diplophonia_active = 1;
            voice->next_attenuated = 0;
        }
        float period_samples =
            voice->clock.samples_per_millisecond * voice->period_ms;
        float closed_fraction = 1.0f - voice->open_fraction;
        float cut = di * period_samples;
        cut *= closed_fraction;
        cut *= 0x1.47ae14p-7f;
        if (!isfinite(cut) || cut < 0.0f ||
            (double)cut >= (double)LONG_MAX)
            return -1;
        voice->diplophonia_cut =
            voice->diplophonia_active ? (long)cut : 0;
        voice->frame_period_configured = 1;
    }
    return 0;
}

int reloq_core_voiced_fill_with_mask(struct reloq_core_voiced_state *voice,
                                     float *samples, uint8_t *closed_mask,
                                     size_t count)
{
    if (!voice || !samples)
        return -1;
    size_t offset = 0;
    while (offset < count) {
        if (voice->pre_left == 0 && voice->open_left == 0 &&
            voice->closed_left == 0) {
            if (voice->modulated_frame_open) {
                if (reloq_core_voiced_prepare_modulated_period(voice))
                    return -1;
            } else if (voice->pending_change) {
                if (reloq_core_pulse_configure(
                        &voice->pulse, &voice->clock,
                        voice->pending_pitch_tenths,
                        voice->pending_open_percent,
                        voice->pending_voice_level))
                    return -1;
                voice->period_ms =
                    10000.0f / voice->pending_pitch_tenths;
                voice->open_fraction =
                    voice->pending_open_percent * 0.01f;
                voice->pending_change = 0;
            }
            struct reloq_core_frame_counts lengths =
                reloq_core_frame_counts_for(
                    voice->period_ms, voice->next_phase_ms,
                    voice->open_fraction,
                    voice->clock.samples_per_millisecond);
            if (lengths.total <= 0 || lengths.first < 0 ||
                lengths.second < 0 ||
                (voice->modulated_frame_open &&
                 lengths.second < voice->diplophonia_cut))
                return -1;
            voice->pre_left = 0;
            if (voice->modulated_frame_open) {
                voice->pulse = voice->base_pulse;
                if (voice->diplophonia_active) {
                    if (voice->next_attenuated) {
                        float gain = 1.0f -
                            voice->frame_diplophonia *
                            0x1.47ae14p-7f;
                        if (gain < 0.0f)
                            return -1;
                        if (gain == 0.0f) {
                            /* DI=100 suppresses this alternate pulse. Keep
                             * the delayed period and its closed-phase mask. */
                            voice->pulse.increment = 0.0f;
                            voice->pulse.curvature = 0.0f;
                        } else {
                            voice->pulse.increment *= gain;
                            voice->pulse.curvature /= gain;
                        }
                        voice->pre_left = voice->diplophonia_cut;
                    }
                    voice->next_attenuated = !voice->next_attenuated;
                }
            }
            voice->pulse.phase =
                voice->next_phase_ms *
                voice->clock.samples_per_millisecond;
            voice->pulse.phase *= voice->pulse.increment;
            voice->next_phase_ms = reloq_core_next_phase(
                lengths.total, voice->clock.milliseconds_per_sample,
                voice->next_phase_ms, voice->period_ms);
            voice->open_left = lengths.first;
            voice->closed_left = lengths.second - voice->pre_left;
        }
        size_t available = count - offset;
        if (voice->pre_left) {
            size_t n = available < (size_t)voice->pre_left ?
                       available : (size_t)voice->pre_left;
            reloq_core_clear(samples + offset, (long)n);
            if (closed_mask)
                memset(closed_mask + offset, 1, n);
            voice->pre_left -= (long)n;
            offset += n;
        } else if (voice->open_left) {
            size_t n = available < (size_t)voice->open_left ?
                       available : (size_t)voice->open_left;
            if (reloq_core_pulse_fill(&voice->pulse, samples + offset, n))
                return -1;
            if (closed_mask)
                memset(closed_mask + offset, 0, n);
            voice->open_left -= (long)n;
            offset += n;
        } else {
            size_t n = available < (size_t)voice->closed_left ?
                       available : (size_t)voice->closed_left;
            reloq_core_clear(samples + offset, (long)n);
            if (closed_mask)
                memset(closed_mask + offset, 1, n);
            voice->closed_left -= (long)n;
            offset += n;
        }
    }
    return 0;
}

int reloq_core_voiced_fill(struct reloq_core_voiced_state *voice,
                            float *samples, size_t count)
{
    return reloq_core_voiced_fill_with_mask(voice, samples, NULL, count);
}

int reloq_core_noise_frame_initialize(
    struct reloq_core_noise_frame_state *state,
    float sample_rate, int32_t initial_seed,
    long source_offset, long ah_trim, long af_trim)
{
    if (!state || !isfinite(sample_rate) || sample_rate <= 0.0f)
        return -1;
    memset(state, 0, sizeof *state);
    state->clock = reloq_core_clock_for_rate(sample_rate);
    state->sample_rate = sample_rate;
    state->duration_scale = 1.0f;
    state->source_offset = source_offset;
    state->ah_trim = ah_trim;
    state->af_trim = af_trim;
    state->ah_seed = (uint16_t)initial_seed;
    state->af_seed = (uint16_t)initial_seed;
    return 0;
}

int reloq_core_noise_frame_set_duration_scale(
    struct reloq_core_noise_frame_state *state, float duration_scale)
{
    if (!state || !isfinite(duration_scale) || duration_scale <= 0.0f)
        return -1;
    state->duration_scale = duration_scale;
    return 0;
}

int reloq_core_noise_frame_render(
    struct reloq_core_noise_frame_state *state,
    const struct reloq_core_noise_frame *frame,
    float *output, float *raw_af, float *scratch,
    int16_t *pcm, size_t capacity, size_t *written)
{
    if (!state || !frame || !output || !raw_af || !scratch || !written ||
        output == raw_af || output == scratch || raw_af == scratch ||
        state->frames_rendered == SIZE_MAX ||
        !isfinite(frame->duration_ms) || frame->duration_ms <= 0.0f ||
        !isfinite(frame->bypass_attenuation_db) ||
        frame->bypass_attenuation_db<0 || frame->bypass_attenuation_db>12)
        return -1;
    long count = reloq_core_frame_sample_count(
        frame->duration_ms, state->duration_scale,
        state->clock.samples_per_millisecond);
    if (count <= 0 || (size_t)count > capacity)
        return -1;

    struct reloq_core_noise_frame_state next = *state;
    if(next.sample_rate>=16000 &&
       (frame->bypass_attenuation_db>0 || next.fricatives.colour_rate>0))
        reloq_core_frication_bank_colour(&next.fricatives,next.sample_rate,
            frame->bypass_attenuation_db);
    struct reloq_core_aspiration ah;
    struct reloq_core_frication af;
    if (reloq_core_aspiration_configure(
            &ah, next.ah_seed, frame->ah_level,
            next.source_offset, next.ah_trim) ||
        reloq_core_frication_configure(
            &af, next.af_seed, frame->af_level, frame->bypass_level,
            next.source_offset, next.af_trim) ||
        reloq_core_voiced_bank_configure(
            &next.tract, &next.clock, &frame->tract, 1,
            next.frames_rendered != 0))
        return -1;

    float centers[RELOQ_CORE_FRICATIVE_FORMANTS];
    for (int i = 0; i < RELOQ_CORE_FRICATIVE_FORMANTS; ++i)
        centers[i] = frame->tract.oral[i].frequency_hz;
    float bypass = frame->af_level != 0 ? af.bypass_gain : 0.0f;
    if ((frame->af_level != 0 || next.af_envelope.global_ms != 0.0f) &&
        reloq_core_frication_bank_configure(
            &next.fricatives, &next.clock, centers,
            frame->af_bandwidths, frame->af_levels,
            frame->tract.highest_formant, frame->af_level,
            next.source_offset, next.af_trim, bypass))
        return -1;
    /* Numeric frames are prepared once and rendered in callback blocks.
     * Release times and filter histories advance after each block, while
     * the AH and AF generators carry their independent seeds forward. */
    for (size_t at = 0; at < (size_t)count; ) {
        size_t length = (size_t)count - at;
        if (length > 200) length = 200;
        float *part = output + at;
        float *part_af = raw_af + at;
        if (at == 0) {
            if (reloq_core_frication_envelope_begin(
                    &next.af_envelope, &next.fricatives, frame->af_level,
                    frame->af_levels, length, next.sample_rate))
                return -1;
        } else {
            float elapsed = (float)(length * 1000u);
            elapsed /= next.sample_rate;
            if (!isfinite(elapsed)) return -1;
            next.af_envelope.elapsed_ms = elapsed;
            next.af_envelope.frame_open = 1;
        }
        reloq_core_clear(part, (long)length);
        if (reloq_core_aspiration_mix(&ah, part, length, NULL) ||
            reloq_core_voiced_bank_process(&next.tract, part, length))
            return -1;
        if (next.af_envelope.global_ms > 0.0f) {
            if (reloq_core_frication_fill(&af, part_af, length) ||
                reloq_core_frication_bank_mix(
                    &next.fricatives, part_af, part, scratch + at,
                    length) ||
                reloq_core_frication_envelope_end(
                    &next.af_envelope, &next.fricatives))
                return -1;
        } else {
            reloq_core_clear(part_af, (long)length);
        }
        if (pcm)
            for (size_t i = 0; i < length; ++i) {
                if (!isfinite(part[i]) || part[i] >= 0x1p63f ||
                    part[i] < -0x1p63f)
                    return -1;
                int64_t value = (int64_t)part[i];
                reloq_core_pcm16_from_i64(&value, pcm + at + i, 1);
            }
        at += length;
    }
    if (next.af_envelope.frame_open) {
        for (int i = 0; i < next.fricatives.active_formants; ++i)
            if (next.fricatives.stages[i].enabled == 0.0f) {
                next.fricatives.stages[i].previous = 0.0f;
                next.fricatives.stages[i].latest = 0.0f;
            }
        next.af_envelope.frame_open = 0;
    }
    next.ah_seed = ah.seed;
    next.af_seed = af.seed;
    next.frames_rendered++;
    *state = next;
    *written = (size_t)count;
    return 0;
}

int reloq_core_voiced_frame_initialize(
    struct reloq_core_voiced_frame_state *state,
    float sample_rate, int32_t initial_seed,
    long source_offset, long ah_trim, long af_trim)
{
    if (!state)
        return -1;
    struct reloq_core_voiced_frame_state next = {0};
    if (reloq_core_noise_frame_initialize(
            &next.noise, sample_rate, initial_seed,
            source_offset, ah_trim, af_trim) ||
        reloq_core_voiced_initialize(&next.voice, sample_rate))
        return -1;
    *state = next;
    return 0;
}

int reloq_core_voiced_frame_set_av_trim(
    struct reloq_core_voiced_frame_state *state, long av_trim)
{
    if (!state)
        return -1;
    state->av_trim = av_trim;
    return 0;
}

int reloq_core_voiced_frame_render_owned(
    struct reloq_core_voiced_frame_state *state,
    const struct reloq_core_voiced_frame *frame,
    float *output, float *raw_af, float *scratch, uint8_t *closed_mask,
    int16_t *pcm, size_t capacity, size_t *written)
{
    if (!state || !frame || !output || !raw_af || !scratch ||
        !closed_mask || !written || output == raw_af ||
        output == scratch || raw_af == scratch ||
        frame->glottal_profile < 0 || frame->glottal_profile > 35 ||
        !isfinite(frame->noise.duration_ms) ||
        frame->noise.duration_ms <= 0.0f ||
        !isfinite(frame->noise.bypass_attenuation_db) ||
        frame->noise.bypass_attenuation_db<0 || frame->noise.bypass_attenuation_db>12 ||
        state->noise.frames_rendered == SIZE_MAX)
        return -1;
    long voice_level = 0;
    if (frame->av_level > 0 &&
        (reloq_core_add_long(frame->av_level, state->av_trim,
                             &voice_level) ||
         reloq_core_add_long(voice_level, state->noise.source_offset,
                             &voice_level)))
        return -1;
    /* The raw AV still drives the serial release gate. The adjusted gain
     * can be zero or negative; the reference then mutes new voiced pulses. */
    if (voice_level < 0)
        voice_level = 0;
    if (frame->av_level < 0)
        return -1;
    long count = reloq_core_frame_sample_count(
        frame->noise.duration_ms, state->noise.duration_scale,
        state->noise.clock.samples_per_millisecond);
    if (count <= 0 || (size_t)count > capacity)
        return -1;

    struct reloq_core_noise_frame_state *noise = &state->noise;
    const int shaped=state->frication_reference_rate==16000 && noise->sample_rate>=16000;
    noise->clock.noise_reference_rate=state->frication_reference_rate;
    noise->clock.noise_reference_formants=state->reference_formants;
    const struct reloq_core_noise_frame *params = &frame->noise;
    if(state->frication_reference_rate>0 && !shaped)
        reloq_core_frication_bank_colour(&noise->fricatives,noise->sample_rate,
            params->bypass_attenuation_db);
    struct reloq_core_aspiration ah;
    struct reloq_core_frication af;
    const struct reloq_core_voiced_modulation_frame modulation = {
        .duration_ms = params->duration_ms,
        .duration_scale = noise->duration_scale,
        .pitch_tenths = frame->pitch_tenths,
        .open_percent = frame->open_percent,
        .av_level = voice_level,
        .flutter = frame->flutter,
        .diplophonia = frame->diplophonia,
        .period_active = frame->av_level > 0
    };
    if (reloq_core_voiced_begin_modulated_frame(&state->voice, &modulation) ||
        reloq_core_aspiration_configure(
            &ah, noise->ah_seed, params->ah_level,
            noise->source_offset, noise->ah_trim) ||
        reloq_core_frication_configure(
            &af, noise->af_seed, params->af_level, params->bypass_level,
            noise->source_offset, noise->af_trim))
        return -1;

    /* A sampled white source has variance density proportional to 1/Fs.
     * Aspiration enters the same physical vocal-tract passband at every
     * extended rate. Its source density must therefore track the clock. */
    if(state->glottal_reference_rate>0) {
        ah.gain*=sqrtf(noise->sample_rate/state->glottal_reference_rate);
        reloq_rate_transfer_prepare(&state->rate_transfer,&params->tract,
            noise->sample_rate,state->glottal_reference_rate,state->reference_formants);
    }

    float centers[RELOQ_CORE_FRICATIVE_FORMANTS];
    float widths[RELOQ_CORE_FRICATIVE_FORMANTS];
    long levels[RELOQ_CORE_FRICATIVE_FORMANTS];
    long noise_count=params->tract.highest_formant;
    for (int i = 0; i < RELOQ_CORE_FRICATIVE_FORMANTS; ++i)
        centers[i] = params->tract.oral[i].frequency_hz;
    memcpy(widths,params->af_bandwidths,sizeof widths);
    memcpy(levels,params->af_levels,sizeof levels);
    if(shaped)reloq_noise_profile_prepare(&state->noise_profile,noise->sample_rate,
        frame->av_level,params->af_level,params->bypass_level,centers,widths,levels,&noise_count);
    noise->fricatives.shaped_source=shaped;
    float bypass = params->af_level != 0 ? af.bypass_gain : 0.0f;
    if ((params->af_level != 0 || noise->af_envelope.global_ms != 0.0f) &&
        reloq_core_frication_bank_configure(
            &noise->fricatives, &noise->clock, centers,
            widths, levels,
            noise_count, params->af_level,
            noise->source_offset, noise->af_trim, bypass))
        return -1;

    if(shaped && params->af_level) {
        float mix=params->s_frication?(noise->sample_rate==16000?.25f:.5f):0;
        struct reloq_core_frication_bank low={0},high={0};
        if(mix) {
            float original_f[8];
            for(unsigned j=0;j<8;++j)original_f[j]=params->tract.oral[j].frequency_hz;
            struct reloq_core_clock lc=reloq_core_clock_for_rate(11025);
            struct reloq_core_clock hc=reloq_core_clock_for_rate(22050);
            if(reloq_core_frication_bank_configure(&low,&lc,original_f,
                params->af_bandwidths,params->af_levels,5,params->af_level,
                noise->source_offset,noise->af_trim,bypass) ||
               reloq_core_frication_bank_configure(&high,&hc,original_f,
                params->af_bandwidths,params->af_levels,5,params->af_level,
                noise->source_offset,noise->af_trim,bypass))return -1;
        }
        reloq_rate_transfer_prepare_noise_mix_profile(&state->noise_transfer,&low,&high,
            &noise->fricatives,&state->noise_profile,noise->sample_rate,mix,
            params->s_frication==2);
    }
    if(!shaped && state->frication_reference_rate>0 && noise->sample_rate>state->frication_reference_rate) {
        struct reloq_core_frication_bank original={0};
        struct reloq_core_clock old_clock=reloq_core_clock_for_rate(state->frication_reference_rate);
        unsigned count=(unsigned)params->tract.highest_formant;
        if(reloq_core_frication_bank_configure(&original,&old_clock,centers,
            params->af_bandwidths,params->af_levels,count,params->af_level,
            noise->source_offset,noise->af_trim,bypass))return -1;
        reloq_rate_transfer_prepare_noise(&state->noise_transfer,&original,&noise->fricatives,
            noise->sample_rate,state->frication_reference_rate,params->af_bandwidths[count-1]);
    }

    /* KlattSynth prepares the numeric frame once, then runs its source,
     * resonators, noise and release timers in callback-sized blocks. Keep
     * the voiced modulation clock open across all blocks. */
    for (size_t at = 0; at < (size_t)count; ) {
        size_t length = (size_t)count - at;
        if (length > 200) length = 200;
        float *part = output + at;
        float *part_af = raw_af + at;
        float *part_scratch = scratch + at;
        uint8_t *part_mask = closed_mask + at;
        int fast_silence = frame->av_level == 0 &&
            params->ah_level == 0 && params->af_level == 0 &&
            state->serial.remaining_ms == 0.0f &&
            noise->af_envelope.global_ms == 0.0f &&
            state->voice.pre_left == 0 && state->voice.open_left == 0 &&
            state->voice.closed_left == 0;
        size_t carried = 0;
        if (state->voice.period_live) {
            if (state->voice.pre_left < 0 || state->voice.open_left < 0 ||
                state->voice.closed_left < 0)
                return -1;
            carried = (size_t)state->voice.pre_left < length ?
                      (size_t)state->voice.pre_left : length;
            carried += (size_t)state->voice.open_left < length - carried ?
                       (size_t)state->voice.open_left : length - carried;
            carried += (size_t)state->voice.closed_left < length - carried ?
                       (size_t)state->voice.closed_left : length - carried;
        }
        if (at == 0) {
            if (reloq_core_serial_frame_begin(
                    &state->serial, &noise->tract, &noise->clock,
                    &params->tract, frame->av_level, params->ah_level,
                    length, noise->sample_rate) ||
                reloq_core_frication_envelope_begin(
                    &noise->af_envelope, &noise->fricatives,
                    params->af_level, levels,
                    length, noise->sample_rate))
                return -1;
        } else {
            /* Coefficients and stage gates were prepared at frame entry.
             * The callback loop only updates its duration and release
             * markers; preparing the filters again would erase history
             * when a release expires between two callback blocks. */
            float elapsed = (float)(length * 1000u);
            elapsed /= noise->sample_rate;
            if (!isfinite(elapsed)) return -1;
            state->serial.elapsed_ms = elapsed;
            noise->af_envelope.elapsed_ms = elapsed;
            state->serial.process =
                state->serial.remaining_ms > 0x1.a36e2ep-14f;
            state->serial.frame_open = 1;
            noise->af_envelope.frame_open = 1;
        }
        if (reloq_core_voiced_fill_frame_with_mask(
                &state->voice, part, part_mask, length))
            return -1;
        if (fast_silence && at != 0) {
            reloq_core_clear(part, (long)length);
            reloq_core_clear(part_af, (long)length);
            if (pcm) memset(pcm + at, 0, length * sizeof *pcm);
            state->serial.frame_open = 0;
            at += length;
            continue;
        }

        /* Carry the existing period with its active source filter. Empty
         * remainders are frame boundaries, not filter operations: the
         * legacy zero-length helper collapses its two history samples. */
        if (!fast_silence && carried && reloq_core_resonator_process(
                &state->glottal, part, (long)carried))
            return -1;
        if (frame->av_level > 0 && carried < length &&
            frame->glottal_profile != state->active_glottal_profile) {
            if (source_profile(&state->glottal,&noise->clock,
                    frame->glottal_profile,state->glottal_reference_rate))
                return -1;
            state->active_glottal_profile = frame->glottal_profile;
        }
        if (!fast_silence && state->glottal.enabled != 0.0f) {
            if (carried >= 2)
                part[carried - 2] = state->glottal.previous;
            if (carried >= 1)
                part[carried - 1] = state->glottal.latest;
        }
        if (!fast_silence && carried < length && reloq_core_resonator_process(
                &state->glottal, part + carried,
                (long)(length - carried)))
            return -1;

        /* Colour only the periodic excitation, before aspiration joins
         * it. The common tract and the complete AF branch stay intact. */
        reloq_rate_transfer_process_voice(&state->rate_transfer,part,length);
        struct reloq_core_noise_gate gate = {.closed_mask = part_mask};
        if(state->rate_transfer.presence_rate>0) {
            float breath[200]={0};
            if(reloq_core_aspiration_mix(&ah,breath,length,&gate))return -1;
            reloq_rate_transfer_process_aspiration(&state->rate_transfer,breath,
                length,frame->av_level>0);
            for(size_t j=0;j<length;++j)part[j]+=breath[j];
        } else if (reloq_core_aspiration_mix(&ah, part, length, &gate))return -1;
        if (!state->serial.process && params->af_level == 0 &&
            noise->af_envelope.global_ms == 0.0f)
            reloq_core_clear(part, (long)length);
        if (reloq_core_serial_frame_process(
                &state->serial, &noise->tract, part, length))
            return -1;
        reloq_rate_transfer_process(&state->rate_transfer,part,length);
        float extended_af[200]={0};
        int af_active=noise->af_envelope.global_ms > 0.0f;
        if (af_active) {
            if (reloq_core_frication_fill_with_mask(
                    &af, part_af, part_mask, length))return -1;
            if(shaped)reloq_noise_profile_process(&state->noise_profile,part_af,length);
            if (reloq_core_frication_bank_mix(
                    &noise->fricatives, part_af,
                    state->frication_reference_rate>0?extended_af:part, part_scratch,
                    length) ||
                reloq_core_frication_envelope_end(
                    &noise->af_envelope, &noise->fricatives))
                return -1;
        } else {
            /* The reference bypasses the AF bank after the global marker
             * expires, even if an individual stage retains an enabled gate
             * and filter history from the preceding callback. */
            reloq_core_clear(part_af, (long)length);
        }
        if(state->frication_reference_rate>0) {
            if(!shaped || af_active)
                reloq_rate_transfer_process(&state->noise_transfer,extended_af,length);
            else memset(state->noise_transfer.history,0,sizeof state->noise_transfer.history);
            for(size_t j=0;j<length;++j)part[j]+=extended_af[j];
        }
        if (reloq_core_serial_frame_end(&state->serial, &noise->tract))
            return -1;
        if (pcm)
            for (size_t i = 0; i < length; ++i) {
                if (!isfinite(part[i]) || part[i] >= 0x1p63f ||
                    part[i] < -0x1p63f)
                    return -1;
                int64_t value = (int64_t)part[i];
                reloq_core_pcm16_from_i64(&value, pcm + at + i, 1);
            }
        at += length;
    }
    if (noise->af_envelope.frame_open) {
        /* The inactive AF path skips the callback bank entirely. Its
         * ordinary end-of-frame bookkeeping still clears histories of
         * stages whose gate is already zero, without decaying live stages
         * for callback blocks that never entered that bank. */
        for (int i = 0; i < noise->fricatives.active_formants; ++i)
            if (noise->fricatives.stages[i].enabled == 0.0f) {
                noise->fricatives.stages[i].previous = 0.0f;
                noise->fricatives.stages[i].latest = 0.0f;
            }
        noise->af_envelope.frame_open = 0;
    }
    if (reloq_core_voiced_end_modulated_frame(&state->voice))
        return -1;
    noise->ah_seed = ah.seed;
    noise->af_seed = af.seed;
    noise->frames_rendered++;
    *written = (size_t)count;
    return 0;
}

/* Standalone callers retain failure atomicity. The continuous session
 * uses the owned-state path above: its documented reset after an error
 * makes copying all filter histories on every frame unnecessary. */
int reloq_core_voiced_frame_render(
    struct reloq_core_voiced_frame_state *state,
    const struct reloq_core_voiced_frame *frame,
    float *output, float *raw_af, float *scratch, uint8_t *closed_mask,
    int16_t *pcm, size_t capacity, size_t *written)
{
    if (!state) return -1;
    struct reloq_core_voiced_frame_state next = *state;
    int status = reloq_core_voiced_frame_render_owned(
        &next, frame, output, raw_af, scratch, closed_mask,
        pcm, capacity, written);
    if (!status) *state = next;
    return status;
}
