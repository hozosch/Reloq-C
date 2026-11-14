# Persian: first listening comparison

## Target architecture

For the future Persian voice, retain Finnish as the general source of vowel
qualities and sentence melody, with the accepted Spanish-inspired question
rise. The current listening choices use a 60%-duration native Finnish H
after eligible /p t k/, the German Ach-laut and an adaptation of German L.
The selected long back /A/ follows the earlier three-peak eSpeak-informed
approximation in the Finnish path; short /æ/ uses Finnish ä. The selected
uvular-release experiment still requires its own phoneme implementation.
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

## Selected sound and Latin-script experiments

In the private test library, `RELOQ_FA_PROBE_A=2` selects the requested
40% US `father` and 60% US `off` target. The two US rules start at nominal
F1/F2/F3 750/1200/2440 Hz and 700/1050/2410 Hz respectively; interpolation
therefore gives 720/1110/2422 Hz, F4=3600 Hz and B1=168 Hz. These are Klatt
parameter targets, not an interpolation of recorded audio. They are applied
to the existing Finnish /a/ while preserving its native coarticulation,
inherited-value markers, other vowels, and sentence melody. For the Iran
test, the selected vowel's steady-state first three formant targets are
720/1110/2422 Hz.

`RELOQ_FA_PROBE_Q=5` uses the selected Q release, Q4, with the release
frication amplitude lowered by 3 dB. In initial `qalam`, the measured release
RMS is approximately 3 dB lower, the timing is unchanged and the later vowel
samples are unchanged. The normal Finnish scheduler suppresses the Q release
in `noqte`; this private build inserts it ahead of a following /p t k/ while
keeping the native stop timing. Source-spelling `q` is only a private
experimental switch and is not a general Persian phoneme implementation.

`RELOQ_FA_PROBE_X=1` substitutes the German /x/ rule's frication and formant
targets into a temporary Finnish `G` phoneme slot. This is a first acoustic
transfer, not a sample copy or a verified complete native /x/ phone; enabling
it repurposes Finnish /ŋ/ and therefore must not change an ordinary Finnish
voice. `RELOQ_FA_PROBE_STOPS=1` adds a first German-informed aspiration
experiment for /p t k/ before a vowel, initially or between vowels, while
leaving the experimental `q` distinct. It uses the German rule's AH levels
49/47/45 respectively and adds 40 ms before voicing. That timing is a
listening hypothesis informed by the eSpeak comparison, not a fixed Persian
voice-onset-time rule or a complete transfer of the German consonants.
Neither the private settings nor the later reference-stage Pinglish work
constitute a selectable Persian voice.

`RELOQ_FA_PROBE_RISE=1` selects a low nuclear accent and high phrase and
boundary tones for questions, retaining Finnish timing and the Finnish
contour for statements. This was the first Spanish-inspired approximation;
it did not reproduce the actual higher Spanish phrase target. Finnish
currently classifies all example question marks alike. Persian yes/no
questions often end high, while WH questions can fall; their distinction
requires a Persian frontend and grammatical analysis. See
[Sadat-Tehrani, Persian interrogatives](https://journals.dartmouth.edu/journals/xmlpage/1/article/389?htmlOnce=yes)
and [Estebas-Vilaplana and Prieto, Castilian Spanish intonation](https://sp-tobi.upf.edu/sites/default/files/2024-07/estebas-prieto2010.pdf).

The intended Latin-script routing has two explicit modes:

* In ordinary Persian, an embedded Latin word uses ReLoq's US-English
  frontend, as requested. It does not use eSpeak's separate English voice.
* In explicit `fa-Latn`/Pinglish mode, Latin spellings use Persian rules.
  For example, eSpeak's reference reads `khane` as `xane` and `ghaza` with
  its `q1` phoneme. Some inputs are dictionary or English fallback cases;
  a Latin word alone cannot reliably identify the intended language.

`reference_espeak_1_51.jsonl` contains 114 reference entries generated with
eSpeak NG 1.51 (`fa`, `fa-latn`, and `en-us`). The checked-out eSpeak rules
used for research are from commit `688cb60368d6e0998f3632af8d1c3c2ad4191fc2`;
the source checkout and reference engine version differ. eSpeak NG is
GPL-3.0-or-later. The experimental `pinglish_candidate.c`, in the GPL root
project and not in the MIT provider, follows its basic Latin letter and
digraph rules and reports an *unstressed candidate* only. It deliberately
defers unsupported contexts. The greeting `salam` is an intentional Pinglish
exception: it follows the Persian-script eSpeak reference `salAm`, not the
different `fa-latn` 1.51 output; the English mode still routes the original
Latin word to US English. A complete frontend still needs dictionary
priority and exceptions, stress and duration, Persian-script rules, mixed
text segmentation and conversion from these reference mnemonics to real
ReLoq phones. The route and candidate APIs are not connected to the ECI,
NVDA, SAPI5 or Android frontends.

Focused checks and listening sample, using the separately built probe:

```
python3 experiments/persian/test_selected_probe.py --baseline eci/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/test_question_rise.py --library /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/test_stop_aspiration.py --library /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/test_pinglish_candidate.py
python3 experiments/persian/compare_selected_probe.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-selected.wav
python3 experiments/persian/collect_espeak_corpus.py --output /tmp/fa-corpus.jsonl
```

The corpus collector prints the installed eSpeak version; save it alongside a
new corpus so that results from a different version are not confused with the
1.51 references. The sound checks cover several rates and genders, disabled
settings and language isolation; they are not a perceptual certification.

## Later listening candidates

In response to the listening feedback, `RELOQ_FA_PROBE_A=3` interpolates the
US formant parameters at 20% `father` and 80% `off`, yielding nominal F1/F2/F3
710/1080/2416 Hz and B1 184 Hz. `=4` uses the `off` endpoint for comparison,
700/1050/2410 Hz and B1 200 Hz. Both retain the Finnish timing and
coarticulation offsets, and are parameter interpolation rather than a mix of
recorded sounds. The earlier `=2` remains available as a control.

For `RELOQ_FA_PROBE_RISE=2`, the final Finnish phrase tone is moved to 70%
of the target segment and raised by 70 native pitch units; the final boundary
is adjusted separately. In the test phrase this yields the same 170/190
native targets as the Spanish reference, and the terminal F0 can be measured
against that voice. This is a transfer of two points in one test context, not
a claim that Finnish and Spanish have identical pitch timing or that every
Persian question should rise.

`RELOQ_FA_PROBE_H=1` shortens the **native Finnish /h/** following /p t k/
and before a vowel to 80% of its native duration. The hearing samples use
explicit Finnish `pha`, `tha` and `kha` sequences for the full-length
reference. In the private build, `RELOQ_FA_PROBE_H=2` automatically adds that
shortened native H transition after a Finnish initial or intervocalic /p t k/
followed by a vowel, excluding the experimental source-spelled Q. It leaves
consonant clusters such as `noqte` alone. This is a narrow listening probe,
not yet a Persian allophone system or a user-facing Persian frontend. It does
not modify ordinary Finnish speech, Q or /x/ when the probe is disabled.
The earlier German-informed `RELOQ_FA_PROBE_STOPS=1` remains as a comparison;
do not combine it with the H probe. Future Persian rules must select the
intended allophones and map them to actual phones before this becomes a
selectable voice.

```
python3 experiments/persian/test_finetuned_probe.py --previous /tmp/old-fa/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/compare_finetuned_probe.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-finetuned.wav
```

## German L rule in the Finnish listening probe

The chosen long back vowel uses `RELOQ_FA_PROBE_A=1`, the earlier three-peak
eSpeak-informed approximation. The user's preferred Finnish H is now
`RELOQ_FA_PROBE_H=5`, which scales a real native H after eligible /p t k/ to
60% of its full duration. An 80-ms H consequently becomes 48 ms.

The former `RELOQ_FA_PROBE_L=1` kept Finnish's L rule and added frequency
offsets. It was judged too sharp and is retained solely as a historical
comparison. `RELOQ_FA_PROBE_L=2` instead generates a temporary adaptation of
the actual German `ger_ph_l` rule. It maps its acoustic state slots, phone
symbols, word-boundary fence and vowel-feature tests to the Finnish engine;
in particular Finnish ä follows the German short-front-vowel branch, while
Finnish a follows the analogous German low-a branch. The native Finnish
timing, voice defaults, vowels and prosody remain in force. This is a probe,
not a claim that switching languages yields identical waveforms or that all
Persian phonotactic contexts have been validated.

For matching German and Finnish phone sequences, the focused test compares
the actual rule's formant targets, bandwidths, voicing and source level using
optional internal traces, in addition to checking PCM isolation, rates,
gender, accepted question tone, short ä and the 60% H. The annotated audio
compares the German core, Finnish core and translated German rule in Salam,
Qalam and other positions. Qalam still uses the deliberately narrow Q=6 test
switch for a manually supplied Finnish /k/.

```
python3 experiments/persian/test_german_l_transfer.py --baseline eci/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/compare_german_l_transfer.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-german-l.wav
```

## 50% Finnish H, German-informed L and eSpeak vowel targets

`RELOQ_FA_PROBE_H=4` retains automatic native Finnish H after eligible /p t k/
and sets that H to 50% of its original duration. A native 80-ms H becomes
40 ms, compared with 56 ms at the earlier 70% setting. The actual H duration
still depends on the native context and speed. Standalone H, stop clusters
and selected Q are not aspirated by this test switch.

`RELOQ_FA_PROBE_L=1` adjusts the Finnish L formant trajectories towards the
German core's typical L targets (German nominal F2/F3/F4 1450/2900/3450 Hz;
Finnish nominal 1050/2500/3000 Hz). It preserves native context adjustments
and negative inheritance markers; this is an acoustic hypothesis, not a
wholesale transfer of the German rule or a validated Persian L model.

`RELOQ_FA_PROBE_A=7` applies the first three frequency offsets from `A=1`
and additionally shifts the fourth Finnish /a/ target towards the eSpeak 0_3
central frame. The nominal target is 656/1000/2620/3040 Hz. eSpeak's Swedish
`A:` used in `jag` instead uses `aa_4`, whose central peaks are close, but not
identical (664/1000/2720 Hz in its first marked nucleus). Neither this
similarity nor matching four peak frequencies guarantees identical speech.

The eSpeak `fa+klatt` and `fa+klatt2` samples can help inspect frame-to-Klatt
parameter conversion. Their source waveforms, synthesis and tuning still
differ from ReLoq's. They are diagnostic material and are not a target for
matching overall voice quality; normal eSpeak Persian is the listening
reference. `RELOQ_FA_PROBE_Q=6` is a deliberately restricted, private probe:
it applies the selected Q=5 release to every manually entered Finnish /k/.
The test example `QALAM` uses this to place the Q before two Finnish `ä`
phones. Never enable it as a general language setting.

To reproduce, build a private library as described above, then run
`espeak_reference.py --include-klatt`, `test_light_l_probe.py` and
`compare_light_l_probe.py`. The comparison script accepts `--swedish-reference`
for the normal eSpeak `sv_jag.wav` sample. It requires the ordinary Persian
reference clips via `--reference-dir`.

## Finnish/off mixture, shorter H and Salam

`RELOQ_FA_PROBE_A=5` interpolates the Finnish /a/ and US `off` Klatt target
parameters equally. Its nominal F1/F2/F3 are 663/1050/2505 Hz, with F4 3550
and B1 160. `=6` uses 25% Finnish and 75% `off`, yielding approximately
681/1050/2458 Hz, F4 3575 and B1 180. The native contextual offsets and
sentinels remain in place. These are parameter mixtures, not mixes of recorded
sound. All prior candidates remain available for comparison.

`RELOQ_FA_PROBE_H=3` uses the same automatic native Finnish H transition as
`=2`, shortened from 80% to 70% of its full native duration. In a context
where the full H is 80 ms this makes the transition 56 ms, 8 ms shorter than
the previous 64-ms candidate. The accepted `RELOQ_FA_PROBE_RISE=2` question
contour is unchanged.

The Persian-script reference `سلام` is `sal'Am` in eSpeak's Persian mnemonic
phonemes: the first `a` is the short front vowel and the later `A` is the
back vowel. In the temporary Finnish examples these map respectively to
Finnish `A` (ä) and the separately tuned Finnish `a`, giving
`` `[.0sA.1lam] ``. `fa_reference_a_to_finnish` records those two mappings for
the experimental frontend. Its Pinglish candidate includes a narrow `salam`
exception for the same contrast, because ordinary Pinglish spelling does not
always identify long vowels. These are not yet connected to a selectable
Persian module.

```
python3 experiments/persian/test_balanced_probe.py --baseline eci/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/test_pinglish_candidate.py
python3 experiments/persian/compare_balanced_probe.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-balanced.wav
```
