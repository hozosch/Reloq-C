# Persian: first listening comparison

## Target architecture

For the future Persian voice, retain Finnish as the general source of vowel
qualities and sentence melody. Use German as the starting point for aspirated
/p t k/ and the Ach-laut. The long back /A/ is the separately requested
experiment: start from the US `father` vowel and close it somewhat towards the
eSpeak Persian vowel. The uvular consonant requires its own implementation.
These are design choices for a Persian module; they do not change Finnish,
German, or US voices in the normal build. The current hearing experiments use
separate existing native language paths and have not yet assembled them into
a selectable Persian voice.

The comparison tests acoustic building blocks, not an implemented Persian
voice or general Persian grapheme-to-phoneme conversion. It uses the existing
German and Finnish native engines for manually entered phoneme sequences.
The audio labels explicitly distinguish those approximations from the
Persian and Arabic eSpeak NG recordings used as references.

`نقطه` (point) is commonly transcribed /noqte/; in Iranian Persian the
uvular consonant has context-dependent stop and fricative realizations,
including devoicing before voiceless consonants. The Arabic /q/ is a
voiceless uvular stop; the existing ordinary German /k/ is velar and is
included as a contrast, not described as a correct uvular /q/. German /r/
and /g/ are also comparisons rather than a native Persian phoneme.

Persian has six vowels. Finnish front `ä`, `e`, `i`, `o` and `u` offer
useful starting points. Finnish `a` is a comparison for Persian's back
vowel, not necessarily the correct rounded Persian realization. The
prosody section contrasts a statement and question in the Persian eSpeak
voice with the existing Finnish and German contours. Neither is claimed
as a verified Persian intonation model.

Research references:

* [Uvular lenition in Persian](https://cla-acl.ca/pdfs/actes-2021/Ariyaee_and_Kochetov-Acoustics_of_Persian_uvular_lenition_in_consonant_clusters.pdf)
* [Persian intonation, eight speakers](https://www.isca-archive.org/speechprosody_2008/sadattehrani08_speechprosody.html)
* [Persian interrogative intonation](https://journals.dartmouth.edu/journals/xmlpage/1/article/389?htmlOnce=yes)
* [eSpeak NG Persian voice source](https://github.com/espeak-ng/espeak-ng/tree/master/dictsource)

To reproduce, install eSpeak NG (the initial recordings used version 1.51)
and Python NumPy and SciPy, then run:

```
python3 experiments/persian/espeak_reference.py --output /tmp/persian-reference
python3 experiments/persian/listening.py --library eci/libreloq_eci.so \
  --reference-dir /tmp/persian-reference --wav /tmp/persian-comparison.wav
```

The eSpeak samples are comparison recordings, not copied phoneme tables or
embedded software in the ReLoq binary. Record the eSpeak version and source
when producing them; eSpeak NG is distributed under GPL-3.0-or-later.

## Finnish-based long-a and uvular-stop experiment

`build_native_probe.py` produces a separate, disposable Linux library from
temporary copies of Finnish and US provider rules. It is not a Persian module and
does not modify the normal engine. In this library only, `RELOQ_FA_PROBE_A=1`
adjusts Finnish /a/; `RELOQ_FA_PROBE_Q=1` and `=2` modify the regular Finnish
/k/ when the input spelling is `q`. The settings are listening probes, not
production controls, and must not be enabled in a normal Finnish voice.

eSpeak NG commit `688cb60368d6e0998f3632af8d1c3c2ad4191fc2`,
`phsource/ph_farsi`, selects `phsource/vowel/0_3` for its Persian /A/;
`dictsource/fa_list` transcribes Iran as `i:RAn`. The spectral file's two
middle frames have peak frequencies F1=656, F2=1000, F3=2620 Hz. The Finnish
/a/ rule starts at 625, 1050, 2600 Hz, so the experiment shifts positive
target frequencies by +31, -50, +20 Hz while retaining contextual differences
and native inherited-value sentinels. eSpeak's additive spectral frames and
Eloquence's Klatt parameters are different models, so matching these nominal
frequencies is only an approximation, not a demonstrated acoustic match.
The cited eSpeak reference is GPL-3.0-or-later; this root experiment follows
the enclosing project's GPL license.

eSpeak's common `q` source describes a voiceless uvular stop and uses the
short `ustop/q` release sample. The `ar` and `fa` phoneme tables both inherit
`base1`, and neither language's included source redefines `q`; the same
underlying `q` is therefore shared. Persian transcribes the test words with
`q1`, while Arabic uses `q`. The recording of each word retains its own voice
and phonetic context. The
two original exploratory variants shift F2/F3 for the existing /k/ and reduce
its higher burst amplitudes. They do not copy the eSpeak sample and have not
been validated as uvular or as the voiced Persian stop/fricative. The input
spellings `qalam` and `noqte` are used only to select those release experiments;
their Finnish stress, vowels, syllabification and phoneme name remain Finnish.

After building the ordinary Linux engine, reproduce the experiment with:

```
python3 experiments/persian/build_native_probe.py --work-dir /tmp/fa-probe
python3 experiments/persian/inspect_espeak_spectrum.py /path/to/espeak-ng/phsource/vowel/0_3
python3 experiments/persian/espeak_reference.py --output /tmp/fa-reference
python3 experiments/persian/test_native_probe.py --baseline eci/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/compare_native_probe.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-long-a-q.wav
```

The reference recording uses eSpeak NG 1.51. `test_native_probe.py` checks the
probe and baseline PCM at several rates, genders and contexts and checks that
disabled settings, normal `k`, an unaffected Finnish vowel, and other
languages remain unchanged. It does not verify perceptual or phonetic quality.

## US father vowel and fricated release

The US word `father` uses the phone `a` in this engine. Its native `ga_ph_a`
rule begins with the F1/F2/F3 values 750/1200/2440 Hz for `father` and the
manually entered `Iran` comparison. In the private test library only,
`RELOQ_FA_PROBE_US_A=1` moves these by -47/-100/+90 Hz, and `=2` by
-94/-200/+180 Hz, retaining context and inheritance markers. These are
nominal interpolations towards the eSpeak 0_3 nucleus, not a claim that the
different synthesizers sound identical. The Finland and English comparison
uses explicit phoneme sequences; the English R remains English.

`RELOQ_FA_PROBE_Q=3` and `=4` build on the earlier candidate 1 and extend its
release by approximately 10 and 25 ms. They increase frication energy using
bands informed by the German native Ach-laut rule. These are experiments in
the Finnish Klatt timeline rather than a concatenated /k/ and /x/ waveform.
The result has not been validated as a uvular consonant, and general
Persian phonotactics or its voiced /ɢ~ʁ/ allophones are not implemented.

The reference generator can also render Persian eSpeak's `پا تا کار` and
`تا` for inspecting stop onsets. Persian /p t k/ are often aspirated in
initial and intervocalic positions, but the amount depends on context; see
[Bijankhan and Nourbakhsh, 2009](https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/abs/voice-onset-time-in-persian-initial-and-intervocalic-stop-production/1BB880891FBD64D7FDF76E9E7A2678F0).
Measurements of Finnish and German are comparisons, not a claim that all
Persian stops can be copied wholesale from one language.
For the future Persian frontend, use German-style aspirated stops as the
starting point in initial and intervocalic contexts; don't present the
unaspirated Finnish realisation as the Persian target. This is an acoustic
design decision, not yet a transfer of the German stop implementation.

For an extra check, `measure_t_onset.py` estimates release and periodic vowel
onsets from the synthesised /ta/ recordings. With the existing default voice
at 16 kHz, it estimates a gap of about 19 ms for Finnish, 94 ms for German,
and 57 ms for eSpeak 1.51 Persian. These are approximate audio-only estimates
for different voices and are not the same measurements as human VOT. A
[study of seven Persian-speaking men](https://www.latl.leeds.ac.uk/wp-content/uploads/sites/49/2019/05/Heselwood-and-Mahmoodzade_2007.pdf)
found a mean initial /t/ VOT of 85.28 ms (SD 20.40 ms) in its controlled
word context. That supports an aspirated starting point, not a universal
85-ms constant for every consonant, speaker, rate, or position.

```
python3 experiments/persian/build_native_probe.py --work-dir /tmp/fa-probe --jobs 8
python3 experiments/persian/espeak_reference.py --output /tmp/fa-reference
python3 experiments/persian/measure_t_onset.py --library eci/libreloq_eci.so \
  --reference-dir /tmp/fa-reference
python3 experiments/persian/test_refined_probe.py --baseline eci/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/compare_refined_probe.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-refined.wav
```
