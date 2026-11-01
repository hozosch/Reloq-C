# Native S levels and upper periodic harmonics

The listener rejected both previous body experiments. In the latest wideband
comparison no difference was audible, and the requested body refers mainly
to upper voiced components, closer to the perceived effect of richness in
DECtalk than to added bass or lower mids. Additional listener feedback also
rejected the S level increase. The previous listening profiles remain
experiments, not accepted release changes.

## Production source change

Remove the additional +4 dB at active S/Z AF endpoints in German, American
English and the nine languages which later adopted that adjustment. The
native target value now reaches the existing AF scheduler directly. The
older independent English component planner loses its matching +4-dB branch
as well, even though it is not the active ECI renderer. Strict compilation
of that component passes. Nine shared bindings no longer need per-endpoint
phoneme/record lookups.
Boundary timing, zero source levels and language-specific stream layouts
remain intact. Japanese, Castilian Spanish and Greek had no such source
increase and retain their existing levels. Chinese's internal English path
inherits the American-English rollback as well.

This removes the specific S/Z source increase; it does not remove the shared
wideband noise profile or its bandwidth calibration. The independently
accepted F/V/W adjustment and other consonant controls remain in place.
The German medial /ts/ correction is a separate existing rule and is retained.

`tools/check_native_s_rollback.py` compares the zero listening profile with
the separate official 1.0 host at all seven rates, including 8 kHz. All 77
changed-language S cases have less energy with identical lengths. The 21
exception-language S cases and 315 vowel, plosive, labiodental and German
affricate controls are PCM-exact. One clipped sample in the Korean 8-kHz
baseline is absent after rollback; no comparison gains a clipped sample.
This does not establish that perceived high-rate S distortion is resolved.

## Listening source experiment

The first source-filter probe was stopped before delivery: normal male
voicing already has an open glottal prefilter, so reducing its index changes
almost nothing. Measured upper/lower band changes stayed below 0.001 dB.
This is a useful rejected hypothesis, not a reason to export another
inaudible comparison.

An isolated pulse-compression probe was also rejected before delivery.
Its stronger setting clipped native German output, while the milder settings
had comparatively small upper-band changes. No compressed-pulse source is
included in the delivered prototype or production code.

DECtalk's own [richness documentation](https://dectalk.github.io/dectalk/idh_sdk_3_richness_ri.htm)
and [Nopen documentation](https://dectalk.github.io/dectalk/idh_sdk_3_nopen_fixed_nf.htm)
describe a source-quality control related to the open portion of a glottal
period. Changing the entire OQ schedule in this core would also change its
AH/AF masks. The delivered listening prototype instead shapes upper periodic
harmonics with one gentle 1.8-kHz high shelf, before aspiration joins voicing.
It does not change pulse shape, open phase, fractional period handling,
noise masks, formant frequencies or bandwidths. This is an independent
spectral-source trial, not an implementation of DECtalk's ri mechanism.

The experiment lives in `tests/reloq_listening_source.c`, linked only by
`tools/build_source_listening_native.py`. The production numeric core is used
unchanged.
No release target includes these changes. Source profile A is the S rollback
alone; B and C progressively shape upper harmonics at extended rates.
Their nominal high-shelf gains rise from 1.6/2.4 dB at 16 kHz to 8/12 dB at
48 kHz, respectively. 11.025 kHz has no extra source colour.
The generator calibrates periodic energy once
per rate/profile at a source sentence, then keeps that gain for all sentences.
No lower-mid bell, postmix EQ, limiter, compressor or time stretching is used.

In the delivered source sentences at 48 kHz, the absolute 2.5–5-kHz periodic
band increases by 6.25/8.73 dB in German (B/C) and 6.97/10.15 dB in US English,
after periodic level calibration. Total source-sentence energy matches A
within 0.05 dB. Noise-only synthesis is bit-exact between A/B/C in all 36
language/rate/sentence combinations. All native utterance lengths match,
and no native test capture clips. These are measurable signal changes,
not an established subjective preference or cure for lispiness.

The small C filter test verifies 18 exact block/reset comparisons and clock
bypasses. At 48 kHz its nominal B/C gains measure 7.67/11.43 dB at 4 kHz,
with less than 0.003 dB change at 100 Hz, before level calibration.
Each delivered language package has 66 native recordings and five guided
comparisons, plus instructions, a separate blind key and its validation
manifest. Checksums, durations and measurements are recorded in
`../validation/source-listening100.json`.

```sh
python3 tools/build_source_listening_native.py --output /tmp/reloq-source/libreloq_source_listening.so
python3 tools/check_native_s_rollback.py --library /tmp/reloq-source/libreloq_source_listening.so --baseline /path/to/release-1.0/libreloq_eci.so --output /tmp/native-s-rollback.json
python3 tools/build_source_listening.py --library /tmp/reloq-source/libreloq_source_listening.so --reference /path/to/release-1.0/libreloq_eci.so --output /tmp/reloq-source-audio
```

All files use native speed 75 (NVDA 30), pitch 65, inflection 30, male model,
head 50 and no user dictionaries. Guided comparisons use offline 48-kHz
assembly; individual recordings retain their native rates. The source trial
requires separate listening acceptance before it becomes production code.
Neither the S rollback nor these audio files are published as a new release.
