# Wideband listening experiment after 1.0

These files build audio comparisons, not an installable engine. Production
acoustics, main and the 1.0 release are unchanged. The listener rejected the
previous body/clarity experiment: its broad low shelf mostly disappeared
under energy matching, and its classic-S option removed the wide companion.
Another listener described 1.0's high-rate S as lispy and distorted; neither
that description nor a preferred remedy has been established objectively.

## Independent hypotheses

| Profile | Periodic excitation | S frication |
| --- | --- | --- |
| A | Release 1.0 | Release 1.0 |
| B | 500-Hz bell and 2.8-kHz shelf | Release 1.0 |
| C | Release 1.0 | Stronger classic-spectrum blend below 5 kHz |
| D | Release 1.0 | C plus smoothed inverse magnitude correction |
| E | B | D |

B uses a Q=0.8 bell rising from +1.5 dB at 16 kHz to +4 dB at 48 kHz.
The upper shelf rises from +0.8 to +2.6 dB. Both act before aspiration
joins periodic excitation, not on the mixed recording. The gain is calibrated
once per output rate using a voiced reference sentence, then kept fixed for
all sentences. This avoids cancelling a bounded midrange change by separately
matching every S sentence.

C retains the existing upper companion and source tilt. The additional
classic blend is zero above 5 kHz and reaches 65% of the remaining blend
below 3.8 kHz. It does not broaden the S poles or impose an 11-kHz cutoff.
D applies two small moving averages to the log magnitude of the bounded
inverse correction, with a nominal 140-Hz radius per pass. The original
finite-kernel power compensation and coefficient ramps follow. This tests
a possible source of graininess, not a proven defect or cure.

## Reproduction

```sh
python3 tools/build_wideband_listening_native.py --output /tmp/reloq-wideband/libreloq_listening.so
python3 tools/build_wideband_listening.py --library /tmp/reloq-wideband/libreloq_listening.so --reference /path/to/release-1.0/libreloq_eci.so --nvda-rate 30 --output /tmp/reloq-wideband-audio
```

Use the exact release-1.0 reference, not the experiment as its own reference.
The native helper copies the rate-transfer source to the selected build
directory, patches that private copy, and links a listening-only wrapper.
Neither is used by any release target. NumPy/SciPy are development-only
dependencies for level checks and guided WAV assembly.

The shared comparison settings are NVDA 30/native 75, pitch 65, inflection
30, volume 100, male model, head 50, without user dictionaries. Raw files
are mono PCM16 at 11.025, 16, 22.05, 32, 44.1 and 48 kHz. Only labelled
sequences are assembled offline at 48 kHz. No time stretching, limiter or
compression is applied. All profiles for a sentence/rate share A's file
gain and one package-wide headroom adjustment.

## Checks and listening

The generator verifies every A recording against the separate 1.0 library,
equal native lengths, unchanged 11.025-kHz outputs, noise independence of
the periodic adjustment, and the shared voice source across S alternatives.
Subtraction of separately quantized native mixtures leaves at most six
PCM units of peak error and less than one unit RMS in these examples.
Native conversion truncates before and after the shared output gain; the
test records this residual instead of asserting exact mixture subtraction.

The S-free body sentences are bit-identical for A/C/D and for B/E.
Their B/A total energy matches within 0.04 dB. At 48 kHz the 300–900-Hz
band relative to 80–250 Hz changes by +2.36 dB in German and +2.73 dB in
US English. These are measured spectral differences, not evidence that
listeners will hear more body. The noise-only S probes retain their upper
band: power above 5.5 kHz changes from 13.13% to 13.94% (German A/D),
and from 10.23% to 10.94% (English A/D).

Each language package has 78 native recordings, four guided comparisons,
an English or German README, a separate blind key and the full validation
manifest. The blind S comparison has six groups of three neutrally numbered
A/C/D recordings. Assess a preferred number, lispiness, grain/distortion
and sharpness separately; no difference is valid feedback. In particular,
the listener who hears the reported problem can evaluate it without first
being told which correction is applied.

`validation/wideband-listening100.json` records archive checksums, counts,
durations and key measurements. No audio is committed to the repository.
These packages do not establish an improvement in keypress-to-audio latency.
