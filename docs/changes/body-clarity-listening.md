# Slower body, clarity and S listening trials

These are audio comparisons based on the unmodified 1.0 runtime. No public
add-on, APK, SAPI package or release is changed. The listening-only link
wrapper in `tests/reloq_listening_voice_colour.c` is not part of any normal
release build target.

The previous WAV generator used native speed 98, which corresponds to NVDA
50 without rate boost. These comparisons use NVDA 30, native speed 75.
Speech is synthesized at that setting, rather than time-stretched. The
German unvoiced-S test changes from 1.601 to 2.611 seconds, and the English
one from 1.658 to 2.683 seconds.

Profiles:

| Label | Periodic voice | S noise |
| --- | --- | --- |
| A | 1.0 | 1.0 |
| B | Body trial | 1.0 |
| C | Clarity trial | 1.0 |
| D | Body and clarity | 1.0 |
| E | 1.0 colour | Classic 11-kHz target |
| F | Body and clarity | Classic 11-kHz target |

The body trial adds a broad 900-Hz low shelf to periodic excitation only.
Its nominal gain progresses from 1 dB at 16 kHz to 3 dB at 48 kHz. The
clarity trial adds a 4.9-kHz upper shelf, from 1 to 3.5 dB. The 11.025-kHz
path bypasses both. These are separate listening hypotheses, not established
perceptual improvements.

The S trial fits the original 11.025-kHz noise bank directly with the
existing bounded, power-normalized causal filter. It excludes the wide
companion's S noise and the extra source tilt, while retaining other noise
classes. It does not broaden the S resonance or reuse the rejected B/C
S-only experiments. No runtime audio resampling is introduced.

The generator captures the native noise-only branch and a half-level voice
pilot, then solves for the voicing gain needed to match full-signal energy.
The result is rendered again by the native synthesizer. This avoids scaling
the noise while matching overall level. A/B/C/D noise-only output compares
bit-exactly; E/F share their separate S target. A compares bit-exactly with
the official 1.0 host, and all six 11.025-kHz outputs are identical.

Across German and US English, 180 profile/sentence/rate checks preserve
native durations and stay within 0.0031 dB of the reference energy. Each
package contains 97 checked PCM16 mono WAVs: native-rate individual clips
and guided comparisons assembled offline at 48 kHz. The original native
rate bandwidth is preserved in that assembly. The generated archives are
closed, CRC-checked and atomically published as complete files.

Build and generate on the development host:

```sh
python3 tools/build_voice_colour_listening_native.py --output /tmp/reloq-listening/libreloq_listening.so
python3 tools/build_voice_colour_listening.py --library /tmp/reloq-listening/libreloq_listening.so --reference /path/to/1.0/libreloq_eci.so --nvda-rate 30 --output /tmp/reloq-listening/audio
```

The tools require NumPy and SciPy for measurement and offline assembly only.
They are not dependencies of the speech engine or its installable packages.
