# Developer tools

## German prose coverage benchmark

`german_prose_coverage.py` measures the actual C text frontend and direct
ECI text-to-PCM path on a fixed, natural German suite:

- 19 original text inputs from the user's German ECI capture files;
- seven longer general prose and punctuation lines from the local test suite;
- four *separately reported* short calibration inputs.

The fixture at `../tests/data/german_prose_coverage.jsonl` contains original
input text and source identifiers, never cached reference audio or phoneme
transcripts. The source `reloq_text_coverage.c` is compiled into a temporary
executable by the script and queries the **real** C annotation frontend in
two modes: no optional lexicon, then `data/german_apple_observed.rlex` loaded.
The latter measures possible annotation coverage. The experimental ECI mode
can load the packaged, merged `.rlex` through `eciLoadLexicon`; words still
require supported C acoustic targets and transitions to produce audio.

From the project root:

```sh
make -C eci libreloq_eci.so
python3 -B tools/test_german_prose_coverage.py
python3 -B tools/german_prose_coverage.py \
  --json-out /tmp/german-coverage.json \
  --markdown-out /tmp/german-coverage.md
```

To check complete PCM bytes against a **fresh, separately supplied** public
Apple ECI 6.1 invocation for native successes, also pass `--apple-eci`,
`--apple-audio-probe`, `--apple-workdir`, and any needed
`--apple-runtime-lib-dir`. The optional `--apple-phoneme-probe` then classifies
the isolated SPR structure of the word forms still unknown to the C frontend.
These isolated responses are not assumed to be sentence-context readings.
Without reference arguments, the report marks audio as `not_compared`, never
as exact. An unsupported text job must emit **no** PCM.

The machine-readable report distinguishes complete-utterance text coverage,
unique and occurrence-weighted word coverage, acoustic rejection, native
PCM production and exact separate reference comparison. Natural prose and
calibration cases are never pooled in the summary. Improvements can change
coverage counts without breaking the test; it checks fixture identity and
diagnostic consistency. A captured baseline and next broad work areas are in
`../reference-apple/german_prose_coverage_baseline.md`.

## Numeric Klatt trace

`reloq_trace_wav` converts a sequence of numeric Klatt frames to signed,
little-endian 16-bit mono WAV using the independent `reloq_core` renderer. It
does not parse text or pronounce words. It never loads an Apple library; the
source-index mapping was checked against the reference-only
`av_ah_frame_probe.c` and `flutter_diplophonia_mixed_frame_probe.c`.

Build and try the included controlled three-frame example:

```sh
cd eloquence-c/tools
make test
./reloq_trace_wav example.trace example.wav
```

The tool writes the WAV to a temporary stream, patches its RIFF sizes after
all frames render, then copies the complete result to the requested path.
Malformed input therefore leaves an existing output file untouched.

## Trace format, version 1

The first tokens and header keys are required in this exact order:

```text
reloq-klatt-trace 1
rate 16000
duration_scale 1
seed 100
source_offset 0
av_trim 0
ah_trim 0
af_trim 0
fhighest 8
output_gain 1
frame
  <64 finite numbers, fields 0 through 63>
frame
  <64 finite numbers, fields 0 through 63>
```

Tokens may cross line boundaries. Empty lines and comments starting with `#`
are allowed; a comment extends to the end of that line. A `frame` token must
be followed by exactly 64 values. At least one frame is required. There is
no fixed input line length. The implementation checks finite float range,
integer-only fields, frame sample counts, allocations, and the RIFF size.
The supported rates are 8000, 11025, 16000, 22050, 32000, and 48000 Hz.
The seed is an unsigned 16-bit initial value for both independent noise
sources. `duration_scale` is a positive float applied to each frame length.
`source_offset`, `av_trim`, `ah_trim`, and `af_trim` are integral source-level
offsets between -400 and 400. `fhighest` selects active oral formants F1..F8 and AF
branches 1..8, and must be an integer from 1 through 8. These are constant
for the entire trace.
`output_gain` is the positive finite post-synthesis gain. It is applied to
the signed 64-bit intermediate before low-word PCM16 conversion; captured
ECI output may use a value different from one.

Fields follow the zero-based float index of the reference Klatt frame:

| Index | Field | Meaning |
| --- | --- | --- |
| 0 | DU | Frame duration in milliseconds, positive |
| 1 | F0 | Pitch in tenths of a hertz, positive when AV > 0 |
| 2 | AV | Voiced level, integer 0..400 |
| 3 | OQ | Open quotient percentage, 0 < OQ <= 100 when AV > 0 |
| 4 | source profile | Integer 0..35 |
| 5, 6 | FL, DI | Flutter >= 0; diplophonia 0 <= DI <= 100 |
| 7, 8 | AH, AF | Aspiration and frication levels, integers 0..400 |
| 9, 10 | F1, B1 | First oral center frequency and bandwidth in Hz |
| 11, 12 | reserved | Observed inert in controlled reference checks |
| 13..26 | F2/B2 .. F8/B8 | Alternating center frequency and bandwidth in Hz |
| 27..34 | NP/BP, NZ/BZ, TP/BT, TZ/BZ | Four nasal/tracheal center and bandwidth pairs in Hz |
| 35..42 | A1F .. A8F | Parallel AF branch levels, integers 0..400 |
| 43 | AB | Direct frication branch level |
| 44..51 | B1F .. B8F | Parallel AF bandwidths in Hz |
| 52..63 | reserved | Observed inert in controlled reference checks |

Frequencies and bandwidths must be nonnegative. Any nonzero reserved
values are counted and reported on stderr, but are not mapped to the typed
renderer. A 62-value vector from `sendArrayParameters` can be written as
indices 0..61 plus two zero values for 62/63. The isolated reference probes
found 62/63 inactive for voiced and mixed frames at the six supported rates.
That observation is limited to those tested frame modes.

PCM follows `reloq_core_voiced_frame_render`, including its signed 16-bit
low-word conversion. A trace containing real language-scheduler frames can
therefore be compared with a separate captured PCM stream, while the trace
itself supplies no language rules or text-to-speech interface.
