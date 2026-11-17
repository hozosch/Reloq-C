# Persian word-frequency research inventory

`fa_50k.txt` is the Persian 2018 OpenSubtitles frequency list from
[FrequencyWords](https://github.com/hermitdave/FrequencyWords/blob/master/content/2018/fa/fa_50k.txt),
by Hermit Dave and contributors. The upstream project identifies its content
as CC BY-SA 4.0; see its [license description](https://github.com/hermitdave/FrequencyWords/blob/master/README.md)
and the [license](https://creativecommons.org/licenses/by-sa/4.0/).
The file is unchanged: Git blob `22455fa993ab2a54304d33d9c76102b55d2d1619`,
SHA-256 `96ea178f77022cdb2d358feb75a4fc474754daa224a4beba296ad777a514bdf7`.

The generator uses this frequency inventory to select candidate words, then
normalizes and validates their pronunciations against the pinned eSpeak NG
Persian data and the native phoneme parser. The resulting table contains
pronunciations and applies GPL version 3, a compatible license for adaptations
of CC BY-SA 4.0 material. Frequency counts are not included in the table.
The corpus is subtitle text, so its coverage is not representative of every
kind of Persian text.

`compile_direct_frequency_compounds.py` separately checks five productive
endings for these stems in four reference contexts. Its admission bits refer
to the word table's fingerprint and are required by the C compound code.
They are GPL-3.0-only for the same reason as the word table.

To regenerate or check the word table, pass `--native-library` a build of the
Persian C components compiled with `RELOQ_FA_EXCLUDE_FREQUENCY_WORDS` defined.
That baseline prevents the generator from treating its own previous output as
an existing rule. `--normalizer-library` must export
`reloq_fa_direct_normalize` and `reloq_fa_direct_free`; a test build exposing
the unhidden C components can serve both roles. `--probe` uses the acoustic
engine, and both generators require the pinned eSpeak NG 1.53.0 data solely
for offline reference checks. The generated tables use no eSpeak at runtime.

The frequency table is an incremental port, not a complete grapheme-to-phoneme
engine. Text not admitted by native rules must continue through the existing
fallback until its pronunciation and timing have been covered and verified.
