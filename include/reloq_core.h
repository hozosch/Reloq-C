#ifndef RELOQ_CORE_H
#define RELOQ_CORE_H

#include <stdint.h>
#include <stddef.h>
#include "reloq_rate_transfer.h"
#include "reloq_noise_profile.h"

/* Platform-independent operations shared by the Apple verification bridge
 * and the future standalone synthesizer. No binary layout or ECI types.
 */
void reloq_core_clear(float *samples, long count);
void reloq_core_mix_i16(const int16_t *source, float gain,
                        float *destination, long count);
void reloq_core_mix_float(const float *source, float gain,
                          float *destination, long count);
void reloq_core_scale_output(int64_t *samples, long count, float gain);
/* The default 16-bit ECI path writes the low 16 bits of each intermediate
 * sample. Convert with defined C arithmetic on platforms with int16_t. */
void reloq_core_pcm16_from_i64(const int64_t *source, int16_t *destination,
                                size_t count);
void reloq_core_parallel0(float a, float b, float *previous, float *latest,
                          float *samples, long count);
unsigned char reloq_core_normalize_byte(unsigned char c);
void reloq_core_normalize_text(unsigned char *text);
int reloq_core_extra_space(const unsigned char *text, unsigned char mode);

/* Input is a mutable, NUL-terminated byte string of at least length bytes.
 * The caller owns the output and supplies its full capacity. The optional
 * cancellation byte is sampled between input characters. Return 0 on
 * success, 1 when cancelled, or -1 for invalid input/insufficient space.
 * This is the byte-oriented RomanizerManager preprocessor, not a UTF-8
 * decoder or a language-specific pronunciation rule engine.
 */
int reloq_core_prepare_text(char *input, size_t length, unsigned char mode,
                            char *output, size_t capacity,
                            const volatile unsigned char *cancelled);

struct reloq_core_clock {
    float milliseconds_per_sample;
    float samples_per_millisecond;
    float radians_per_sample;
    float twice_radians_per_sample;
    int rate_kind;
    /* Optional noise calibration clock. Zero preserves numeric ECI PCM. */
    float noise_reference_rate;
    /* Number of measured noise poles when extending that reference.
     * Zero retains the legacy five-pole calibration convention. */
    unsigned noise_reference_formants;
};

/* Klatt's reference sample-rate coefficients. rate_kind is the original
 * 8/11.025/16 kHz selector (0/1/2); 3 denotes another rate, not a
 * complete higher-rate synthesis profile.
 */
struct reloq_core_clock reloq_core_clock_for_rate(float sample_rate);

/* One second-order Klatt resonator. The three ramp arrays describe the
 * initial coefficient transition in chronological order. When the ramp has
 * two steps left, processing starts at array index 1. No platform-specific
 * state fields are exposed. */
struct reloq_core_resonator {
    float input_gain;
    float latest_gain;
    float previous_gain;
    float ramp_input[3];
    float ramp_latest[3];
    float ramp_previous[3];
    float previous;
    float latest;
    long ramp_remaining; /* 0..3 */
    float enabled;
};

/* Set coefficients for an enabled second-order resonator from its radius
 * (the precomputed exp(-bandwidth*pi/rate)) and cos(2*pi*frequency/rate).
 * A changed stage has three intermediate coefficient sets before its target.
 * This only prepares the coefficients: the frame generator must still
 * derive radius/cosine and decide when a stage changes or is disabled.
 */
int reloq_core_resonator_set_coefficients(
    struct reloq_core_resonator *filter, float radius, float cosine,
    int transition);
/* Prepare a resonator directly from a center frequency and bandwidth in Hz.
 * The cosine and exponential are evaluated at the frame clock's rate.
 * Frequency and bandwidth selection for each named Klatt stage remains the
 * responsibility of the frame generator.
 */
int reloq_core_resonator_set_from_hz(
    struct reloq_core_resonator *filter,
    const struct reloq_core_clock *clock, float frequency_hz,
    float bandwidth_hz, int transition);
/* Glottal-source prefilter, selected by the frame's source-profile index.
 * Index 0 bypasses; 1..35 select fixed 8/11.025/16 kHz profiles or the
 * high-rate frequency/bandwidth geometry. Call at the beginning of the
 * pitch period to which a changed index applies. Filter histories persist.
 */
int reloq_core_glottal_prefilter_configure(
    struct reloq_core_resonator *filter,
    const struct reloq_core_clock *clock, long profile_index);

/* Derive the inverse filter coefficients from a prepared resonator. The
 * separate target coefficients are [input, latest, previous]; any pending
 * intermediate coefficients in filter are normalized in place. The caller
 * should use the returned target with antiresonator_process. */
int reloq_core_antiresonator_coefficients(
    struct reloq_core_resonator *filter, float coefficients[3]);

/* Process samples in place. Returns 0 on success, -1 for an invalid state.
 * With enabled==0, both the samples and the filter history are untouched.
 * Floating-point contraction must be disabled for byte-exact reference PCM.
 */
int reloq_core_resonator_process(struct reloq_core_resonator *filter,
                                 float *samples, long count);

/* Companion finite-impulse-response stage. coefficients are input, one-sample
 * history, and two-sample history gains. It shares the typed transition and
 * history state, but its history holds *inputs*, not previous outputs.
 */
int reloq_core_antiresonator_process(struct reloq_core_resonator *filter,
                                     const float coefficients[3],
                                     float *samples, long count);

/* Serial voiced vocal-tract bank after glottal/aspiration excitation. Zero
 * initialize the structure before its first configuration. The
 * oral array is indexed F1..F8. Each stage retains its own coefficient ramp
 * and sample history across frames. The inverse nasal and tracheal targets
 * are prepared with reloq_core_antiresonator_coefficients. The frame
 * generator still needs to select frequencies, bandwidths, and stage flags.
 */
enum { RELOQ_CORE_ORAL_FORMANTS = 8 };
struct reloq_core_voiced_bank {
    struct reloq_core_resonator nasal_pole;
    struct reloq_core_resonator nasal_zero;
    struct reloq_core_resonator tracheal_pole;
    struct reloq_core_resonator tracheal_zero;
    struct reloq_core_resonator oral[RELOQ_CORE_ORAL_FORMANTS];
    float nasal_zero_coefficients[3];
    float tracheal_zero_coefficients[3];
    long highest_formant; /* 0..8; F1 is always evaluated */
};
struct reloq_core_formant {
    float frequency_hz;
    float bandwidth_hz;
};
struct reloq_core_voiced_formants {
    struct reloq_core_formant oral[RELOQ_CORE_ORAL_FORMANTS];
    struct reloq_core_formant nasal_pole;
    struct reloq_core_formant nasal_zero;
    struct reloq_core_formant tracheal_pole;
    struct reloq_core_formant tracheal_zero;
    long highest_formant; /* 1..8 for active voicing */
};
/* Prepare the serial bank for a frame in Hz. A changed frame uses the
 * reference's three-step coefficient transition, including normalized
 * inverse-filter steps. The caller selects active voicing; inactive banks
 * retain their histories but bypass their stages. */
int reloq_core_voiced_bank_configure(
    struct reloq_core_voiced_bank *bank,
    const struct reloq_core_clock *clock,
    const struct reloq_core_voiced_formants *frame,
    int active, int transition);
/* In-place path: NP -> NZ -> TP -> TZ -> Fhighest ... F2 -> F1.
 * A disabled stage is bypassed by its underlying filter function. */
int reloq_core_voiced_bank_process(struct reloq_core_voiced_bank *bank,
                                    float *samples, size_t count);

/* Frame envelope for the serial tract's 20 ms ringout. On AV=AH=0 the
 * previous bank keeps processing until the marker expires. Oral stages then
 * clear while NP/NZ/TP/TZ preserve their gates and histories. The saved
 * frequency/bandwidth cache also models the named stages' behavior on
 * reactivation. Initialize both structures to zero, and call begin,
 * process, end once per complete frame in that order. */
enum { RELOQ_CORE_SERIAL_STAGES = 12 };
struct reloq_core_serial_stage_cache {
    float frequency_hz;
    float bandwidth_hz;
};
struct reloq_core_serial_envelope {
    float remaining_ms;
    float elapsed_ms;
    struct reloq_core_serial_stage_cache
        stages[RELOQ_CORE_SERIAL_STAGES];
    struct reloq_core_voiced_formants frame;
    int release;
    int process;
    int frame_open;
};
int reloq_core_serial_frame_begin(
    struct reloq_core_serial_envelope *envelope,
    struct reloq_core_voiced_bank *bank,
    const struct reloq_core_clock *clock,
    const struct reloq_core_voiced_formants *frame,
    long av_level, long ah_level,
    size_t sample_count, float sample_rate);
int reloq_core_serial_frame_process(
    const struct reloq_core_serial_envelope *envelope,
    struct reloq_core_voiced_bank *bank,
    float *samples, size_t count);
int reloq_core_serial_frame_end(
    struct reloq_core_serial_envelope *envelope,
    struct reloq_core_voiced_bank *bank);

/* Eight parallel fricative noise resonators, in source order. Zero initialize
 * the structure before its first configuration. The caller
 * supplies the raw AF noise and an independent scratch buffer of count
 * floats. Destination may already contain the voiced bank output. Source,
 * scratch and destination must not overlap, since each branch consumes the
 * same unmodified AF samples. A zero AiF gain leaves a previously enabled
 * resonator on with input_gain==0, so its existing history can decay.
 */
enum { RELOQ_CORE_FRICATIVE_FORMANTS = 8 };
struct reloq_core_frication_bank {
    struct reloq_core_resonator stages[RELOQ_CORE_FRICATIVE_FORMANTS];
    float bypass_gain;
    long active_formants; /* 1..8; upper AF stages are outside the frame */
    /* Extended-rate AB retains the physical bandwidth of the reference
     * source. Two low-pass sections; delay history survives frame changes. */
    float bypass_rate, bypass_reference, bypass_scale;
    float bypass_coefficients[2][5], bypass_delay[2][2];
    float colour_rate, colour_db, colour_coefficients[3], colour_delay;
    int shaped_source;
};
/* Prepare AF parallel stages from named formant frequencies, AF bandwidths
 * and integer AiF levels. Source level offsets and the direct AB gain come
 * from the same frame and constants used by frication_configure. No ramp is
 * applied to these parallel stages on frame changes; histories persist.
 */
int reloq_core_frication_bank_configure(
    struct reloq_core_frication_bank *bank,
    const struct reloq_core_clock *clock,
    const float centers_hz[RELOQ_CORE_FRICATIVE_FORMANTS],
    const float bandwidths_hz[RELOQ_CORE_FRICATIVE_FORMANTS],
    const long levels[RELOQ_CORE_FRICATIVE_FORMANTS],
    long active_formants, long global_level, long source_offset, long source_trim,
    float bypass_gain);
int reloq_core_frication_bank_mix(struct reloq_core_frication_bank *bank,
                                  const float *source, float *destination,
                                  float *scratch, size_t count);
/* Optional authored high shelf on AB only; positive dB attenuate highs. */
void reloq_core_frication_bank_colour(struct reloq_core_frication_bank *bank,
    float sample_rate,float attenuation_db);

/* The AF release markers use milliseconds from the actual frame sample
 * count. Call begin after bank configuration, process that frame, then end.
 * The global marker and per-stage markers persist between frames. During
 * AF=0 release, branches use zero input and only their filter histories
 * continue; no cached noise buffer is read. */
struct reloq_core_frication_envelope {
    float global_ms;
    float elapsed_ms;
    uint8_t zero_level[RELOQ_CORE_FRICATIVE_FORMANTS];
    int release_global;
    int frame_open;
};
int reloq_core_frication_envelope_begin(
    struct reloq_core_frication_envelope *envelope,
    struct reloq_core_frication_bank *bank,
    long global_level,
    const long levels[RELOQ_CORE_FRICATIVE_FORMANTS],
    size_t sample_count, float sample_rate);
int reloq_core_frication_envelope_end(
    struct reloq_core_frication_envelope *envelope,
    struct reloq_core_frication_bank *bank);

struct reloq_core_frame_counts {
    long total;
    long first;
    long second;
};

/* Frame duration and split in samples. phase_ms carries the fractional
 * residue from the preceding frame; split is the source frame's fraction.
 * The original uses distinct 0.9999 and 0.4999 rounding biases here.
 */
struct reloq_core_frame_counts reloq_core_frame_counts_for(
    float duration_ms, float phase_ms, float split,
    float samples_per_millisecond);
/* Complete input frame length uses truncation rather than the period's
 * rounding bias. duration_scale is the engine's constant frame multiplier. */
long reloq_core_frame_sample_count(float duration_ms, float duration_scale,
                                    float samples_per_millisecond);
float reloq_core_next_phase(long samples, float milliseconds_per_sample,
                             float phase_ms, float duration_ms);

/* Fill the 16-bit pseudo-random source for one frame. The seed is the
 * frame's integer noise parameter; result is the last generated value.
 * A zero-length frame returns the first seeded value without storing it.
 */
uint16_t reloq_core_noise_fill(uint16_t *samples, size_t count, int32_t seed);

/* The aspiration excitation before its vocal-tract filters. A nonzero AH
 * level enables the noise source. The two level offsets come from the
 * synthesizer's constant parameters; the source seed advances only while
 * aspiration is active. Configure once for each frame, then mix its samples
 * in chunks into the existing glottal signal. This is not the final AH
 * signal: the downstream resonators and inverse filters are separate.
 */
struct reloq_core_aspiration {
    uint16_t seed;
    float gain;
    int enabled;
};
struct reloq_core_noise_interval {
    size_t skip;
    size_t attenuate;
};
/* The voiced period scheduler supplies these spans for one source block.
 * Noise is halved in the initial span and each interval's attenuate span;
 * skip spans retain full amplitude. Alternatively, closed_mask from
 * voiced_fill_with_mask supplies one byte per sample and takes precedence
 * over the intervals. NULL means unvoiced, with no damping.
 */
struct reloq_core_noise_gate {
    size_t initial_attenuate;
    const struct reloq_core_noise_interval *intervals;
    size_t interval_count;
    const uint8_t *closed_mask;
};
int reloq_core_aspiration_configure(struct reloq_core_aspiration *aspiration,
                                    int32_t seed, long level,
                                    long first_offset, long second_offset);
int reloq_core_aspiration_mix(struct reloq_core_aspiration *aspiration,
                              float *destination, size_t count,
                              const struct reloq_core_noise_gate *gate);

/* The separate AF noise source and its direct (AB) branch. The caller routes
 * the raw float samples through the fricative resonators; bypass_gain is the
 * signed gain of the branch that does not pass through those resonators.
 * This stage does not mix with the voiced signal or produce final PCM.
 */
struct reloq_core_frication {
    uint16_t seed;
    float bypass_gain;
    int enabled;
};
int reloq_core_frication_configure(struct reloq_core_frication *frication,
                                   int32_t seed, long level,
                                   float bypass_level, long first_offset,
                                   long second_offset);
int reloq_core_frication_fill(struct reloq_core_frication *frication,
                              float *samples, size_t count);
/* With active voicing, AF noise is halved during closed pitch phases before
 * any parallel formants or AB bypass. Pass the same closed mask used for AH;
 * NULL disables gating. A zero AF level leaves the seed unchanged.
 */
int reloq_core_frication_fill_with_mask(
    struct reloq_core_frication *frication, float *samples,
    const uint8_t *closed_mask, size_t count);

/* Integer level-to-linear-amplitude map used by Klatt source and noise
 * gains. Zero or negative levels are silent; positive levels saturate at
 * 400. The split tables preserve the reference's single-precision values.
 */
float reloq_core_level_gain(long level);

/* One glottal open-phase segment. The caller prepares phase, increment and
 * curvature for the current pitch period and handles period boundaries.
 * Samples are the float excitation before subsequent filtering and mixing.
 */
struct reloq_core_pulse {
    float phase;
    float increment;
    float curvature;
};
/* Configure a fresh, ordinary voiced period from the frame's pitch in
 * tenths of a hertz, AV level and open quotient percentage. This is the
 * baseline path with no flutter, diplophonia or level offsets. The clock
 * comes from reloq_core_clock_for_rate. */
int reloq_core_pulse_configure(struct reloq_core_pulse *pulse,
                                const struct reloq_core_clock *clock,
                                float pitch_tenths, float open_percent,
                                long voice_level);
int reloq_core_pulse_fill(struct reloq_core_pulse *pulse,
                          float *samples, size_t count);

/* Continuous baseline voicing across frame boundaries. Schedule changed
 * pitch, open quotient or level at the next glottal period boundary. Later
 * generator stages will handle flutter and diplophonia. */
struct reloq_core_voiced_state {
    struct reloq_core_clock clock;
    float sample_rate;
    struct reloq_core_pulse pulse;
    float period_ms;
    float open_fraction;
    float next_phase_ms;
    long open_left;
    long closed_left;
    float pending_pitch_tenths;
    float pending_open_percent;
    long pending_voice_level;
    int pending_change;
    int period_live;
    int frame_av_positive;
    struct reloq_core_pulse base_pulse;
    float elapsed_ms;
    float frame_duration_ms;
    float frame_duration_scale;
    float frame_pitch_tenths;
    float frame_flutter;
    float frame_diplophonia;
    long pre_left;
    long diplophonia_cut;
    long flutter_delta;
    size_t frame_samples_remaining;
    int diplophonia_active;
    int next_attenuated;
    int frame_period_configured;
    int modulated_frame_open;
};
struct reloq_core_voiced_modulation_frame {
    float duration_ms;
    float duration_scale;
    float pitch_tenths;
    float open_percent;
    long av_level;
    float flutter;
    float diplophonia;
    int period_active; /* retain raw AV period timing at zero gain */
};
/* FL perturbs F0 once at the first new period of each frame; DI alternates
 * normal and delayed/attenuated periods. Begin, fill exactly the frame's
 * sample count (possibly in chunks), then end to advance elapsed time.
 * The ordinary source calls below remain valid for unmodulated callers.
 * AH/AF and downstream filters are separate. */
int reloq_core_voiced_begin_modulated_frame(
    struct reloq_core_voiced_state *voice,
    const struct reloq_core_voiced_modulation_frame *frame);
int reloq_core_voiced_end_modulated_frame(
    struct reloq_core_voiced_state *voice);
/* Initialize before a frame sequence that may begin with AV=0. begin_frame
 * accepts zero or positive AV and schedules positive changes at the next
 * period boundary; an AV=0 frame drains only the old period. The frame fill
 * disables the closed-phase noise mask over the entire AV=0 frame. It keeps
 * the fractional phase when the old period ends exactly at the frame edge. */
int reloq_core_voiced_initialize(struct reloq_core_voiced_state *voice,
                                  float sample_rate);
int reloq_core_voiced_begin_frame(struct reloq_core_voiced_state *voice,
                                   float pitch_tenths, float open_percent,
                                   long voice_level);
int reloq_core_voiced_fill_frame_with_mask(
    struct reloq_core_voiced_state *voice, float *samples,
    uint8_t *closed_mask, size_t count);
int reloq_core_voiced_configure(struct reloq_core_voiced_state *voice,
                                 float sample_rate, float pitch_tenths,
                                 float open_percent, long voice_level);
int reloq_core_voiced_fill(struct reloq_core_voiced_state *voice,
                            float *samples, size_t count);
/* Accept frame changes without interrupting the current pitch period. The
 * latest scheduled pitch, open quotient and AV are applied when a new
 * period begins; subsequent calls before that boundary replace the pending
 * values. This baseline path requires positive AV and F0.
 */
int reloq_core_voiced_schedule(struct reloq_core_voiced_state *voice,
                               float pitch_tenths, float open_percent,
                               long voice_level);
/* As above, also mark closed-phase samples for the aspiration source's
 * voiced-noise damping. The mask has exactly count bytes: 1 closed, 0 open.
 */
int reloq_core_voiced_fill_with_mask(struct reloq_core_voiced_state *voice,
                                     float *samples, uint8_t *closed_mask,
                                     size_t count);

/* Controlled, independent C synthesis of an AH+AF frame without AV. This
 * covers the serial vocal tract and parallel frication bank, including
 * source seeds and AF release between frames. It is a useful subset for
 * developing the full frame scheduler, not a text-to-speech interface.
 * Call initialize once and reuse the state for successive frames.
 */
struct reloq_core_noise_frame {
    float duration_ms;
    long ah_level;
    long af_level;
    float bypass_level;
    struct reloq_core_voiced_formants tract;
    float af_bandwidths[RELOQ_CORE_FRICATIVE_FORMANTS];
    long af_levels[RELOQ_CORE_FRICATIVE_FORMANTS];
    float bypass_attenuation_db; /* requested wideband colour; default zero */
    int s_frication; /* 0: other phone, 1: current s/z/t, 2: German 0.2.84 s/z/t */
};
struct reloq_core_noise_frame_state {
    struct reloq_core_clock clock;
    float sample_rate;
    float duration_scale; /* constant UI-frame duration multiplier */
    long source_offset;
    long ah_trim;
    long af_trim;
    uint16_t ah_seed;
    uint16_t af_seed;
    struct reloq_core_voiced_bank tract;
    struct reloq_core_frication_bank fricatives;
    struct reloq_core_frication_envelope af_envelope;
    size_t frames_rendered;
};
int reloq_core_noise_frame_initialize(
    struct reloq_core_noise_frame_state *state,
    float sample_rate, int32_t initial_seed,
    long source_offset, long ah_trim, long af_trim);
int reloq_core_noise_frame_set_duration_scale(
    struct reloq_core_noise_frame_state *state, float duration_scale);
/* The three float buffers must each hold capacity samples and must not
 * overlap. PCM may be NULL; when provided it holds capacity samples.
 * written receives the number of float and PCM samples on success. */
int reloq_core_noise_frame_render(
    struct reloq_core_noise_frame_state *state,
    const struct reloq_core_noise_frame *frame,
    float *output, float *raw_af, float *scratch,
    int16_t *pcm, size_t capacity, size_t *written);

/* Controlled AV+AH+AF frames, with continuous pitch periods and the serial
 * and parallel filter histories carried across frames. This baseline mode
 * accepts source profiles 0..35 and AV=0, including AV=AH=0 release. The
 * current source profile persists until a newly voiced period starts. It is
 * deliberately a numeric frame interface; phonetic and language scheduling
 * have not been ported. All four scratch/output arrays hold capacity samples
 * and must be distinct. Initialize once before the first frame. */
struct reloq_core_voiced_frame {
    struct reloq_core_noise_frame noise;
    float pitch_tenths;
    float open_percent;
    long av_level;
    long glottal_profile;
    float flutter;
    float diplophonia;
};
struct reloq_core_voiced_frame_state {
    struct reloq_core_noise_frame_state noise;
    struct reloq_core_voiced_state voice;
    struct reloq_core_resonator glottal;
    long active_glottal_profile;
    struct reloq_core_serial_envelope serial;
    long av_trim; /* extra voiced-source level trim; zero by default */
    float glottal_reference_rate; /* 0 keeps the numeric reference profiles */
    unsigned reference_formants;
    float frication_reference_rate; /* independent AF clock; 0 is native */
    struct reloq_rate_transfer rate_transfer;
    struct reloq_rate_transfer noise_transfer;
    struct reloq_noise_profile noise_profile;
};
int reloq_core_voiced_frame_initialize(
    struct reloq_core_voiced_frame_state *state,
    float sample_rate, int32_t initial_seed,
    long source_offset, long ah_trim, long af_trim);
int reloq_core_voiced_frame_set_av_trim(
    struct reloq_core_voiced_frame_state *state, long av_trim);
int reloq_core_voiced_frame_render(
    struct reloq_core_voiced_frame_state *state,
    const struct reloq_core_voiced_frame *frame,
    float *output, float *raw_af, float *scratch, uint8_t *closed_mask,
    int16_t *pcm, size_t capacity, size_t *written);

#endif
