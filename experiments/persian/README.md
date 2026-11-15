# Persian: first listening comparison

## Target architecture

For the future Persian voice, retain Finnish as the general source of vowel
qualities and sentence melody, with the accepted Spanish-inspired question
rise. The current listening choices use a 60%-duration native Finnish H
after eligible /p t k/, the German Ach-laut, German word-internal L and
French word-final L.
The selected long back /A/ follows the earlier three-peak eSpeak-informed
approximation in the Finnish path; short /æ/ uses Finnish ä. The selected
uvular-release experiment has a distinct phone in the private probe; it still
needs a proper per-language implementation for a production Persian voice.
These are design choices for a Persian module; they do not change Finnish,
German, or US voices in the normal build. The current hearing experiments use
separate existing native language paths and have not yet assembled them into
a selectable Persian voice.

## Minimum path to a first NVDA test

1. Register a separate Persian language ID and expose it through the NVDA
   language registry and native ECI path. Scope the selected Finnish-based
   acoustic behavior to Persian engine instances, not process-wide probe
   environment variables, so ordinary Finnish is unchanged. Give the uvular
   experiment its own phone rather than converting every /k/; the /x/ probe
   must not remove a genuine Finnish sound.
2. Add Persian-script text analysis based on eSpeak NG `dictsource/fa_rules`
   and `dictsource/fa_list`, and map their phone, syllable, stress and
   word-boundary output onto the chosen sounds. The source files are rules
   and lexical data consumed by eSpeak's dictionary compiler and translator,
   not a plain list of unconditional letter replacements: the adapter or
   port must preserve precedence, contextual rules, normalization and
   dictionary stress flags. `fa_list` also contains letter and number names.
   Use compatible eSpeak source/data versions and preserve their GPL-3.0-or-
   later attribution. A small,
   explicitly bounded lexicon can support an early sample test, while
   practical running text needs broader dictionary/rule and fallback
   coverage. Normal embedded Latin should go to the US-English path;
   Pinglish is a separate, later mode. The present Latin candidate is not a
   Persian-script analyzer.
3. Build the Windows x64 ECI DLL and package the NVDA add-on, then check
   language selection, Persian sample text, other languages, stop/cancel and
   normal voice/rate settings. A full release requires broader testing; the
   first restricted test does not require complete number or symbol coverage.

The local listening probes are Linux libraries, not an installable NVDA
language. Earlier comparisons use manually annotated phones; the separate
text-frontend experiment below now applies eSpeak analysis to supported text.

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

## French L rule in the Finnish listening probe

`RELOQ_FA_PROBE_L=3` adapts the French France `fren_ph_l` rule and its
`fren_lat_Fv` formant-transition rule to Finnish phoneme and feature records.
The mapping distinguishes French acoustic fields, Finnish fields, and the
French word-boundary fence. French short-front /E/ is matched to Finnish ä;
the independently selected Finnish back A stays on `RELOQ_FA_PROBE_A=1`.
The previous German L remains available as `L=2` for a fair listening contrast.

This is a private listening probe with Finnish timing and voice settings.
French's special voice-setting branch for L before /r/ is not transferred;
Finnish has no equivalent to French /ɥ/ or nasal vowels in this experiment.
The French /x/ context uses Finnish `G`, the temporary Persian /x/ slot:
use `RELOQ_FA_PROBE_X=1` for such tests and do not interpret that slot as an
ordinary Finnish /ŋ/ when the French L experiment is active.
The user-confirmed native Finnish H remains `H=5`, or 60% of full H duration.

```
python3 experiments/persian/test_french_l_transfer.py --baseline eci/libreloq_eci.so \
  --previous /tmp/old-fa/libreloq_eci.so --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/compare_french_l_transfer.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-french-l.wav
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


## Mixed L and contextual vowel-duration probe

In the private listening build, `RELOQ_FA_PROBE_L=4` selects the adapted
German rule for L inside a word and the adapted French rule for L at the end
of a word. The Finnish phone scanner uses its word fence, so a following word
does not turn a final L into an internal L, and a consonant within the same
word does not make a preceding L final. The scanner state is restored before
the acoustic rule runs. `L=2` and `L=3` remain available for comparison.
H=5, A=1 and RISE=2 are unchanged.

The Finnish kernel already adjusts nucleus duration for stress and phrase
position. In one test at its default speed the nucleus for the back vowel in
isolated Donyā was approximately 84 ms with the first syllable stressed and
120 ms with the final syllable stressed. These are internal timing targets,
not manually segmented acoustic measurements. Persian nouns generally take
final stress, although exceptions depend on morphology. The new
`PERSIAN_PHRASE_FINAL_STRESS` is an explicit phoneme test; the older
`PERSIAN_PHRASE` remains for comparisons. These tags are not a complete
Persian text analysis.

Published evidence reports longer vowels in controlled conditions but
strongly overlapping durations in spontaneous speech; it does not support a
single universal A-vowel multiplier. For a separate listening candidate,
`RELOQ_FA_PROBE_VOWEL=1` scales the Finnish short back-A nucleus to 125% of
its contextual duration, after normalizing the usual Finnish back-A starting
value to the Finnish front-A starting value. The factor comes from the nominal 200/160
back-A/front-A length settings in eSpeak's Persian phoneme source; these are
not millisecond measurements and the two synthesizers do not apply duration
identically. The front vowel and an explicitly long Finnish A stay unchanged.
Thus 125% is an eSpeak-informed comparison, not a proven universal rule.
The contextual evidence, observations, limitations and next checks are
documented in [timing_research.md](timing_research.md).

The newest hearing candidate changes the *noise amplitudes* of S towards
Finnish `viisi`, leaving the original formant transitions alone. A separate
optional A-centre candidate can be derived from the accepted Salam example;
it preserves the Salam example and the native endpoint differences. Neither
the accepted Iran duration nor eSpeak's explicit long final phones imply that
all word-final A/E phones should receive the same duration. See the timing
notes for the boundaries and the eSpeak comparison.

The user accepted the `salam-fixed` A nucleus, the intermediate word-final
E/A duration (`RELOQ_FA_PROBE_FINAL=1`), and the viisi S noise in all Persian
positions. `set_persian_probe()` selects these together in the private
listening build. These switches remain process-global listening controls, not
a selectable Persian language or changes to the ordinary Finnish build.

## Isolated eSpeak text-to-phoneme probe

`espeak_ng_probe.py` calls a matching eSpeak NG library and compiled data set
to obtain the actual `fa_rules`/`fa_list` results, including stress and clause
terminators. It is deliberately a separate-process research interface:
eSpeak's translator and decoder are process-global and must not be initialized
inside NVDA's own eSpeak process. The output is eSpeak's phoneme notation,
until it is passed through the experimental mapper below. eSpeak may read ahead into the next
clause, so the returned text cursor cannot be used as an exact clause or word
span. An embedded `(en)` segment must be routed from the original
text to ReLoq US English; it is not permission to use eSpeak's English voice.

For a matched eSpeak NG 1.53.0 build and data from source commit
`688cb60368d6e0998f3632af8d1c3c2ad4191fc2`, the direct API matched the
executable on all 114 stored test inputs. Compared with the separate 1.51
reference outputs, 113 of 114 phoneme strings matched; `خواهر` gained an
explicit length mark in 1.53.0. Record and ship matching code and data, rather
than assuming the two eSpeak versions are interchangeable. No eSpeak binary
or data has been added to the regular add-on or runtime.

```
python3 experiments/persian/test_espeak_ng_probe.py \
  --library /path/to/libespeak-ng.so \
  --data-parent /path/to/directory-containing-espeak-ng-data \
  --binary /path/to/espeak-ng
```

```
python3 experiments/persian/test_combined_l.py --baseline eci/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/test_vowel_timing.py --baseline eci/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/compare_combined_l_timing.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-l-timing.wav
```

## Experimental Persian text mapping and distinct Q

`text_frontend.py` maps eSpeak's analysed Persian phones, vowel lengths and
stress to the accepted Finnish-based acoustic probe. It maps eSpeak's short
`a` to Finnish `A` (ä), its back `A` to the tuned Finnish `a`, and its `x` to
the private German-derived Ach-laut. The mapper supplies the earlier accepted
long Finnish A only for the observed Iran phoneme sequence; it does not assume
that all Persian back vowels have that duration. Syllable assignment is a
first approximation, not an established general Persian syllabifier.

Q=7 reserves the Finnish phone `R` as an internal marker for a Persian
uvular; it is converted to a marked native stop in the isolated probe, while
ordinary Finnish `k` remains `k` and Persian trilled `r` uses Finnish `r`.
The existing Q5 release and cluster release are retained, and ordinary K
still receives the selected 60% H impulse. Q=7 and its marker are private
experiment controls, not a new public phonetic alphabet or a safe replacement
for a production per-instance language module.

`RELOQ_FA_PROBE_GLOTTAL=1` separately reserves Finnish `w` as an experimental
glottal-stop marker. It becomes a distinct marked stop with the ordinary K
burst removed and no automatically inserted H. The initial duration target is
20 ms, informed by eSpeak events for three example words; this is a preliminary
acoustic approximation which still needs listening and linguistic validation. Until a
third independent phone is available, genuine eSpeak `w` in Persian analysis
is explicitly rejected rather than confused with the glottal marker.

The mapper analyses the complete eSpeak clause first. For a normal Persian
input, it checks eSpeak's English-fallback phones against the original Latin
spans and hands those original spans to ReLoq's US English. Explicit Pinglish
input uses `fa-latn` instead. Complicated unmatched fallback spans are
rejected, not silently replaced. Clause splitting and language changes still
need broader real-world and NVDA tests.

The Finnish phone-display helper omits coda H in words such as `ده`, `۱۲` and
`مهر`. When spoken alone, PCM shows a real H noise segment. A terminal H was
nevertheless lost when another word followed. The experimental mapper now
places an H before a specially marked, short silent carrier in this position,
which retains the native H impulse across the word boundary; H followed by a
consonant retains its existing native path. The tests check PCM rather than
the phone display, and the sound still needs user review. The mapper and
eSpeak library are not wired into NVDA, eSpeak translator state
cannot be shared unsafely with NVDA's own eSpeak engine, and no Windows build
or character/index mapping has been implemented here. These are not failures
that the resulting voice can yet recover from in general running text.

With the separately built Q=7 native probe and matched eSpeak 1.53.0 library
and data used above:

```
python3 experiments/persian/test_distinct_q.py --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/test_glottal_h.py \
  --previous /tmp/fa-distinct-q/libreloq_eci.so --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/test_text_frontend.py \
  --library /path/to/libespeak-ng.so \
  --data-parent /path/to/directory-containing-espeak-ng-data \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/listen_text_frontend.py \
  --probe /tmp/fa-probe/libreloq_eci.so \
  --espeak-library /path/to/libespeak-ng.so --espeak-binary /path/to/espeak-ng \
  --data-parent /path/to/directory-containing-espeak-ng-data \
  --wav /tmp/persian-text.wav
```

The 94 stored Persian/Pinglish reference inputs currently include 93 mapped
cases and one explicitly rejected genuine `w`. This is a small regression corpus, not a
measure of accuracy or coverage on arbitrary Persian text. The test checks
that native phonetic tags are preserved; it does not prove that all resulting
pronunciations are linguistically correct or perceptually accepted.

For another focused iteration on the same unchanged source and ordinary
engine, the experimental build can reuse the previous probe archive. Prepare
the new directory, then list the generated C files allowed to differ:

```
python3 experiments/persian/build_native_probe.py --dry-run --work-dir /tmp/fa-next
python3 experiments/persian/incremental_native_probe.py \
  --previous /tmp/fa-probe --work /tmp/fa-next \
  --changed delta_rules_c23_fifi.c --jobs 4
```

The helper refuses unexpected changed generated files and compiles only the
listed files. The final link still performs whole-program optimization and can
take time; this shortcut is for focused listening probes, not release checks.

## Follow-up listening candidates

The H insertion previously examined the last phone even when it belonged to
the preceding word. Thus the K in `قلم کاغذ` lost its automatic aspiration,
although isolated `کاغذ` retained it. The revised private probe uses the
native word fence: P/T/K at a new word can receive the accepted 60% Finnish H
impulse, while an internal consonant cluster is not changed indiscriminately.

`RELOQ_FA_PROBE_GLOTTAL=2` is an alternative to the previous silent K-like
closure. It makes the carrier brief, uses neighbouring-vowel formants instead
of K targets, and requests Finnish's native glottal onset on the following
vowel. `RELOQ_FA_PROBE_END_H=1` shapes the final-H carrier using its preceding
vowel, avoiding an unrelated K-like tract. The user accepted glottal candidate
2 and the shaped H in the listening comparison. These remain private acoustic
probes, not a validated general model of Persian speech.

`RELOQ_FA_PROBE_A_NASAL=1` or `=2` leaves the accepted A formants and
bandwidths fixed and tries two mild nasal pole/zero separations. The change is
restricted to the selected back A, including its long form. The user selected
the first, milder level; this is an acoustic choice, not a claim that Persian
has a phonemic nasal vowel.

The earlier mixed-language sample separately synthesized each language chunk
and measured roughly 390 ms of silence after each. `mixed_language_probe.py`
is an offline experiment that retains 25 ms of quiet at an unpunctuated switch,
70 ms following a detected final stop, and full punctuation pauses. It keeps
both voices' own prosody. It does not implement streaming, sample-accurate NVDA
indexes, cancellation, pitch-contour joining or a production language switch.
Do not enable the offline buffer as an NVDA solution without those checks.

The user accepted glottal candidate 2, A-nasal candidate 1, and the short
language-switch pauses from that comparison. `set_confirmed_persian_probe()`
selects the acoustic choices together in the isolated test process. The
Persian and English engines still work on separate chunks and keep their own
pitch contours; a truly shared sentence melody needs different architecture.

`streaming_boundary.py` is a separate low-latency experiment for the accepted
short pause. It forwards audible data immediately, holds the quiet suffix,
preserves index order, punctuation pauses and stop closures, and drops its
unemitted tail and indices on cancellation. Its tests compare real PCM with the
earlier offline result at three sample rates. It uses NumPy in this research
environment and is not yet wired to the NVDA worker or a production Persian
language module; source-position mapping and end-to-end playback still need
verification.

## K and final-H follow-up

In the back-A context of `کاغذ`, diagnostic traces show that the Finnish K
has an A4 noise-band value of 40 where the German K has 0; the separately
marked uvular phone is not being selected for this K. This difference is a
candidate explanation for the perceived similarity, not proof that it alone
determines the sound. `RELOQ_FA_PROBE_K_SHAPE=1` selects the German value for
this one noise band, and `=2` selects the midpoint 20. Both retain the chosen
Finnish H aspiration and leave the uvular phone, other K contexts and ordinary
languages alone. The acoustic choices need a listening decision.

`RELOQ_FA_PROBE_END_H_DB=2` or `=3` reduces the H source parameter by two or
three dB for a marked final H following Finnish ä, as in `ده` and `دوازده`.
The resulting change in the mixed output depends on the other sound sources;
the setting does not turn down the entire word. Other H contexts remain intact.
The same accepted glottal and first, mild A-nasal settings are selected for
each comparison. The subsequent user feedback asked for a quieter H and said
that the K still sounded wrong, so the previous K/H candidate was not accepted.
None of these flags affects the normal, non-probe build.

## Compare K before front and back A

The user reports that K before Finnish ä already sounds good. For the same
word-initial environment its noise bands are 0/65/50/0/0/0, versus
0/70/0/40/0/0 before the chosen back A. The previous trial changed only the
fourth band; it did not transfer the third. `RELOQ_FA_PROBE_K_SHAPE=3` copies
the front-vowel K's noise profile to K before back A. `=4` additionally moves
the release-side resonances halfway toward the front-vowel K, and `=5` tries
their full values. This changes only the private K-before-back-A condition,
not K before ä or the separately marked uvular stop; the fixed back-vowel
target and 60%-duration H aspiration remain enabled. The resulting sound and
especially the transition to the dark vowel require listening.

`RELOQ_FA_PROBE_END_H_DB` now accepts 1–18 dB of H-source attenuation so the
new comparison can contrast 6, 9 and 12 dB against the earlier 3 dB setting.
These are source parameters, not measured whole-word level changes. The user
accepted K hearing variant 1 (`RELOQ_FA_PROBE_K_SHAPE=3`, the front-K noise
profile with the original back-vowel transition) and a 9 dB H-source setting.
`set_confirmed_persian_probe()` now selects both with the earlier accepted
vowels, glottal stop, aspiration and prosody in the private listening build.

```
python3 experiments/persian/test_followup.py --previous /tmp/fa-old/libreloq_eci.so \
  --probe /tmp/fa-new/libreloq_eci.so
python3 experiments/persian/test_k_h_tuning.py --probe /tmp/fa-new/libreloq_eci.so
python3 experiments/persian/test_front_k_and_quiet_h.py --probe /tmp/fa-new/libreloq_eci.so
python3 experiments/persian/test_mixed_language_probe.py
python3 experiments/persian/test_streaming_boundary.py
python3 experiments/persian/test_mixed_language_native.py --probe /tmp/fa-new/libreloq_eci.so
python3 experiments/persian/compare_followup.py --previous /tmp/fa-old/libreloq_eci.so \
  --probe /tmp/fa-new/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --espeak-binary /path/to/espeak-ng --data-parent /path/to/data-parent \
  --wav /tmp/persian-followup.wav
python3 experiments/persian/compare_k_h_tuning.py --probe /tmp/fa-new/libreloq_eci.so \
  --espeak-library /path/to/libespeak-ng.so --espeak-binary /path/to/espeak-ng \
  --data-parent /path/to/data-parent --wav /tmp/persian-k-h.wav
python3 experiments/persian/compare_front_k_and_quieter_h.py --probe /tmp/fa-new/libreloq_eci.so \
  --espeak-library /path/to/libespeak-ng.so --espeak-binary /path/to/espeak-ng \
  --data-parent /path/to/data-parent --wav /tmp/persian-front-k-h.wav
```

## Shared Persian and English sentence contour

An independently rendered language chunk ends its own pitch phrase. In one
measured statement, the first Persian chunk fell from about 112 to 80 Hz and
the English chunk restarted around 115 Hz; the English fell to about 73 Hz
before the next Persian chunk restarted around 112 Hz. Merely shortening
silence leaves these pitch resets intact.

`build_contour_probe.py` builds a separate Linux-only ECI library with a
private F0 trace and override on native control frames. It does not alter the
ordinary source or library. `shared_sentence_contour.py` uses a single
Finnish-based sentence to plan F0. For each English part, a **silent proxy**
uses the actual English engine's syllable count and stress with approximate
Finnish vowels. The proxy PCM is never played. Its native F0 is time-mapped to
the voiced intervals of real Persian and US-English synthesis, so the two
neighbouring voiced intervals use the same boundary pitch. Only F0 changes;
the native engine renders the actual consonants, vowels, durations, voicing
and sound sources. The accepted K, H, vowel, glottal and aspiration settings
are active in this comparison. Punctuation and the chosen question rise are
planned in the Finnish-based sentence.

The user accepted this shared sentence melody but requested removal of the
remaining pauses. `stream_shared()` demonstrates output of already planned
frames; it delivers audio as it is rendered and tests ordered synthetic part
markers and cancellation of held PCM. Synthesis of the current native chunk
is not yet interrupted by that experimental cancel. The implementation plans and prerenders the sentence,
uses process-wide environment variables and temporary files, and cannot be
shipped in NVDA. Its markers identify language segments, not source text
positions; arbitrary pitch changes, source-index mapping, concurrent engines,
real cancellation and user settings all need a request-scoped implementation.
The English surrogate and the time mapping are approximations, so even though
the F0 targets meet exactly, naturalness needs listening and is not guaranteed.

There is a native precedent for the next step: Mandarin already projects
English word/syllable records into its sentence, generates English acoustic
frames in a request-local bank, and uses host F0 targets for that bank (see
`vendor/openevv61/lang/zhcn61/foreign_bank_zhcn61.c` and
`vendor/openevv61/src/reloq61_bridge.c`). That mechanism depends on the
Mandarin graph and is not automatically a Finnish/Persian implementation.
An actual Persian language module would need an adapted projection and donor
bank, with preserved source indexes and cancellation, before NVDA integration.

## Remove added pauses at language boundaries

The earlier comparison inserted 25 ms of source-silent PCM at ordinary
switches and 70 ms after a final P/T/K. The no-extra-pause experiment removes
those fixed budgets without shortening the native segment by an arbitrary
amount. For each nonfinal segment it temporarily renders the engine's own
terminal-pause-suppressed mode. It verifies that this is a byte-identical
prefix of the full segment, then retains at least that native phonetic clock
and the entire audible release and filter decay. Explicit punctuation is
protected. No added padding or waveform crossfade is introduced; native silent
stop closures within the segment and its source contour remain intact.

`Comparison.shared` now contains the connected variant, while
`Comparison.short_pause` keeps the prior shared-melody comparison. The default
`stream_shared()` emits the same connected result; `seamless=False` reproduces
the earlier boundary policy. The native pause-mode setting is restored after
each probe. This is still an offline, multi-pass prototype with process-wide
hooks and synthetic segment indexes; its latency, source positions and
interruptible native synthesis are not yet the production NVDA solution.
Measured boundary continuity, preserved native releases, question contour,
stop examples, punctuation handling and real streamed PCM are regression
checks, not a substitute for the user's listening assessment.

```
python3 experiments/persian/build_contour_probe.py --archive /tmp/fa-kh/libreloq61.a \
  --work /tmp/fa-contour --jobs 4
python3 experiments/persian/test_shared_sentence_contour.py \
  --probe /tmp/fa-contour/libreloq_eci.so --baseline /tmp/fa-kh/libreloq_eci.so \
  --espeak-library /path/to/libespeak-ng.so --data-parent /path/to/data-parent
python3 experiments/persian/compare_shared_sentence_contour.py \
  --probe /tmp/fa-contour/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-shared-contour.wav
python3 experiments/persian/compare_connected_switch.py \
  --probe /tmp/fa-contour/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-connected-switch.wav
```

## E next to J

The user selected a full adjustment of the E next to J, as in `یک` / `yek`,
towards the already accepted E in `صفر` / `sefr`. In the Finnish provider's
native controls, the raw first three vowel resonances at the beginning of the
E in `yek` were 400, 2000 and 2800 Hz; the E in `sefr` used 450, 1900 and
2550 Hz at its beginning and 450, 1850 and 2500 Hz at its end. The private
probe sets the first three targets on both sides of an E adjacent to J to
450, 1875 and 2525 Hz, the midpoint of the `sefr` targets. Voice scaling is
applied afterwards by the ordinary core. The correction is gated by the
Persian test helper and does not change E in other environments or the
ordinary Finnish or English builds. It keeps the existing vowel timing.

`set_confirmed_persian_probe()` selects the full adjustment. The optional
halfway mode remains only for experiments; no listening decision is needed
to choose between the two. The rendered sound and its transitions still need
the user's listening assessment.

## Language continuation with sentence context

The user found that removing silence did not remove the perceived pauses:
the isolated sections still tapered as sentence endings. In one example,
the isolated first Persian section took about 270 ms and its voiced source
fell to zero, while the same section in full context lasted about 173 ms and
the voiced source stayed active. The English section showed the same problem.

`contextual_continuation.py` is an offline listening prototype that plans the
whole clause with each native core, using silent syllable proxies for parts in
the other language. It selects controls only from the actual language parts,
applies the accepted shared F0 plan, and sends all selected controls to one
continuous Klatt session. Thus the source and filter state can continue across
the switch; no proxy audio is played. The F0 seam, final question rise, native
frame replay, and preservation of the agreed K, H and vowel choices are
checked at multiple sample rates and with both genders. Replay rejects voice,
equalizer and rate settings that the private session does not reproduce.

This is still a test, not an NVDA-ready engine: the proxies and word-boundary
selection are approximations, and sentence planning currently pre-renders with
process-wide hooks. Real-time integration, accurate source positions,
interruptible synthesis, concurrency and all user voice settings require a
request-scoped implementation. A continuous source and matching pitch targets
alone do not establish that every transition sounds natural.

```
python3 experiments/persian/build_native_probe.py --dry-run --work-dir /tmp/fa-e-src
python3 experiments/persian/incremental_native_probe.py --previous /tmp/fa-earlier \
  --work /tmp/fa-e-src --changed delta_rules_c17_fifi.c --archive-only --jobs 4
python3 experiments/persian/build_contour_probe.py --archive /tmp/fa-e-src/libreloq61.a \
  --work /tmp/fa-context --jobs 4
python3 experiments/persian/test_e_near_j.py --probe /tmp/fa-context/libreloq_eci.so \
  --previous /tmp/fa-earlier/libreloq_eci.so
python3 experiments/persian/test_contextual_continuation.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_contextual_and_e.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-context.wav
```

## Phoneme-aware language transitions

The user accepted the E adjustment but reported that the contextual language
switch still jumped and did not glide naturally. The earlier silent proxies
had the right syllable counts but usually used M at every onset and omitted
word-final consonants. They therefore often supplied the wrong neighbouring
sound to a core at the language boundary. In `man NVDA`, for example, the
old join changed the first resonance from 300 to 750 Hz in one millisecond.

The optional `phone_context=True` maps recognizable native phoneme tags to
approximate tags for the other core, including onset and coda consonants.
These tags are validated against the receiving core; unsupported or rejected
sequences fall back to the earlier syllable proxy. The foreign words themselves
are still spoken by their own core and the previously accepted Finnish-based
F0 pilot is unchanged. In the same example, the first-resonance jump with this
context fell to about 100 Hz. This is a boundary approximation, not a complete
cross-language phoneme mapping; other language and word combinations can still
need work.

The second listening variant additionally uses `bridge_ms=25`: at a language
boundary it mixes the two native local trajectories over up to 25 ms on
either side using a smooth weighting. Only formant and nasal-filter controls
are mixed. Pitch, voiced and noise source levels, frication-band settings,
frame lengths and closures are not directly changed by the bridge. It skips
the whole window if either plan has a silent or fricated frame there, including
the stop at `yek laptop`. A filter transition can still alter the final sound;
and the phoneme-aware planning itself can change native timing and adjacent
controls. The audible result therefore needs the user's comparison, especially
around H, K, S, nasal sounds and question boundaries.

The user accepted the second variant. The existing low-level defaults are
intentionally retained as a comparison reference; the private
`accepted_contextual_compare()` helper selects phoneme-aware planning and the
25 ms sonorant bridge together. This is not an NVDA implementation. These
experiments reuse the existing private library, so no full rebuild is needed.

```
python3 experiments/persian/test_contextual_transitions.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_contextual_transitions.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-transitions.wav
```

## Transition from English to Persian Ach

The user reported that the accepted second variant still stumbled between
`NVDA` and the initial Ach sound of `خوب`. Its sonorant bridge did not run at
that boundary because a fricative starts immediately afterwards. In this
example the pitch and source levels already joined, but F1 jumped from about
350 to 450 Hz and F3 from about 2430 to 2700 Hz in one millisecond.

The optional `ach_onset_ms=25` trial anticipates those filter values in the
last 25 ms of the preceding English voiced tail, with a smooth weighting. It
only applies when a Persian part starts with the private `G` / Ach marker,
the preceding English tail is voiced and has no frication, and an actual
fricative follows. It neither adds duration nor changes pitch, voiced and
noise levels, or any frame at or after the Ach onset. The excitation and
filter history reaching that onset can change its resulting PCM, so the
transition and the preserved sound still require listening. Other accepted
variant-2 boundaries and the E and K/laptop checks remain unchanged.
If a preceding consonant falls within the 25 ms window at high speed, the
trial starts after its last unvoiced or fricated frame instead.

```
python3 experiments/persian/test_ach_continuation.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_ach_continuation.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-ach-transition.wav
```

## Direct unvoiced Ach after English

The user did not hear an improvement from anticipating the Ach formants and
suggested that the friction should follow the English A directly, with no
extra voiced portion in front of it. At the `NVDA` to `خوب` boundary in the
private probe, the first 31 ms of the Persian portion carry a voiced source;
the native fricative then continues with voicing off and the noise source
active. The earlier 25 ms preparation is not used in this new comparison.

The optional `direct_ach=True` path keeps the chosen variant-2 sentence and
the entire preceding English vowel. At a Persian initial `G` following a
voiced English tail, it removes only the continuous voiced lead-in up to the
first native frame with the voiced source off and a strong frication source
on. It requires a matching native noise band within 120 ms and does not cut
through unexpected silence; if that evidence is absent, the segment is left
alone. The remaining native noise controls and later pitch, vowel and
consonant frames are retained. The sentence becomes shorter by the removed
lead-in, and the same continuous synthesizer renders across the new join.
Native synthesis histories can still affect the sound, so the onset and
perceived loudness require listening.

Focused checks cover questions, two Ach boundaries in one sentence,
unrelated transitions, the selected E and K/laptop example, three sample
rates, both genders and several speech speeds. The user-facing result is
still an offline listening experiment, not an NVDA implementation.

```
python3 experiments/persian/test_direct_ach.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_direct_ach.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-direct-ach.wav
```

## Preserve the English vowel release into Ach

The user heard an improvement from direct noise but said the final A of
`NVDA` no longer sounded as if it finished properly. In the default Reed
example, the last 100 ms of the English vowel's F1–F3 controls match native
English `NVDA hello` exactly. At the language boundary, however, the direct
Ach switches F3 from about 2430 to 1406 Hz while the engine may still carry
the previous glottal period and filter history. Matching the controls before
the boundary therefore did not prove that the rendered vowel could finish
naturally.

The optional `ach_release=True` trial keeps the direct unvoiced noise onset
and gradually blends the shared tract filter from its preceding English
value to the native Ach values. The blend follows one pitch period, bounded
to 4–15 ms, and is limited to the continuous unvoiced noise at the start of the
Ach. It preserves the English frames, the noise and voiced source levels,
frication-band controls, F0, frame lengths and all later native controls. The
shared tract filter also colours the first milliseconds of the Ach noise, so
the vowel release and Ach timbre both require listening. No duration is added.

```
python3 experiments/persian/test_ach_vowel_release.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_ach_vowel_release.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-a-release.wav
```

## German-planned vowel-to-Ach comparison

The user heard little change in the first sentence and a partial improvement
in the others, and suggested checking how the German core plans the
transition. In native `rauchen` at the default male test speed, German starts
the X noise while the last 15 ms of the vowel are still voiced; the frication
then continues without voicing. A German phonetic probe with an /aɪ/ glide
and a forced following /x/ gives a similar onset. This tag is an artificial
reference, not an ordinary German word and not the same vowel as English
/eɪ/. Its glide is useful here for observing the German timing and acoustic
trajectory; the normal word corroborates the basic overlap.

`german_ach_transition.py` captures the German reference at the same speed,
rate, pitch and voice settings as the private mixed-language test. The first
candidate uses its measured source and band ramps to introduce Ach noise on
the final existing frames of English A. It preserves the English voiced
source and its duration; no separate voiced Ach segment is reintroduced.
The second candidate also maps the German F1/F2 movement onto the existing
English and Persian targets, and follows the German F3 trend briefly during
the unvoiced Ach before fading back to the Persian target. That second choice
can change the Ach's timbre and must be judged by listening.

The adaptation is limited to a native English final `e` phoneme, the letter
A sound, immediately before a confirmed Persian initial Ach. Other switches
and the accepted vowels retain their controls. The measured overlap scales
with the German reference at the requested speaking speed. These are still
offline prototypes using matching voice settings and a private library.

```
python3 experiments/persian/test_german_ach_transition.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_german_ach_transition.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-german-ach.wav
```

## Continue the German-guided Ach into the next vowel

The user found the German resonance candidate the best so far but still
incomplete. In `NVDA خوب`, its F3 falls from about 2430 Hz at the boundary to
about 1928 Hz during X and then rises again towards 2200 Hz at the next voiced
frame. In the German diphthong-to-X reference, F3 instead moves gradually
through the unvoiced interval. German F2 also begins moving toward the
following vowel within the fricative, while the mixed-language candidate holds
it almost still until the vowel begins. These observations identify possible
contributors to the audible difference; they do not establish its cause.

Two new listening modes start with the user's preferred German resonance
candidate. Mode 1 maps the German F3 progress onto the actual interval from
the English vowel target to the first voiced Persian target. Mode 2 adds a
German-guided F2 progression from the beginning of X to the first settled
following vowel. Both use the actual English/Persian endpoints rather than
copying German vowel values. Neither changes source levels, pitch, frame
durations, the initial noise ramp or subsequent vowel targets. Mode 2 does
alter the F2 transition during the beginning of the following voiced vowel,
so both its colour and the Ach require listening. An unsupported boundary or
an intervening unvoiced phone leaves the F2 experiment inactive.

```
python3 experiments/persian/test_german_ach_flow.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_german_ach_flow.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-german-flow.wav
```

## Incoming A-to-Ach and Persian word onsets

The user clarified that the previous two experiments mostly changed the end
of Ach and the following vowel, whereas the problem was the *incoming*
English A-to-Ach transition. `compare_incoming_ach.py` returns to the earlier
German resonance candidate. Trial 1 shortens its pre-boundary noise/resonance
overlap to 40% of its German-derived duration. Trial 2 instead stretches the
existing voiced English A glide by 25%, with no added segment. Both keep the
unvoiced Ach and the following-vowel controls, except that the first 15 ms of
Ach can reflect the changed incoming tract trajectory. The changed frame
durations in trial 2 also change oscillator timing, so identical controls do
not imply bit-identical subsequent samples. Both trials are opt-in, and need
the user's listening judgment. At very fast speed a glide may be too short to
qualify; the lengthening then correctly does nothing.

Assadi's [2003 experimental study](https://www.internationalphoneticassociation.org/icphs-proceedings/ICPhS2003/papers/p15_3117.pdf)
found that Persian vowel-initial words can be glottalized without a complete
closure; a full glottal stop is not mandatory on every word boundary. Its
isolated-word measurements involved just two speakers and should not be
treated as a universal frequency. The study also describes separately the
segmental glottal sound within some words. In the current frontend eSpeak
does not emit a separate `?` for `ایران`, whereas it does for `ساعت`.

In the Finnish-based probe `من ایران` has a pronounced word-onset drop in
the glottal open-quotient control despite continuing voiced-source level.
`persian_word_onsets.py` compares the original with an experimental 50%
reduction of that control excursion. It keeps frame durations, source levels,
pitch and all other controls, and skips sentence starts and words containing
or immediately following the private `w` marker used for an explicit
glottal stop or H carrier. This adjustment is a perceptual test, not a claim
to reproduce the irregular phonation described by the study. The detector
skips boundaries where it cannot confidently identify the native pattern;
it has not been integrated into the streaming frontend or NVDA.

```
python3 experiments/persian/test_incoming_ach.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_incoming_ach.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-incoming-ach.wav
python3 experiments/persian/test_word_onsets.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_word_onsets.py \
  --probe /tmp/fa-context/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-word-onsets.wav
```

## Earlier onset and voice-release experiments

The first no-extra-onset experiment skipped the Finnish `adjust_phones`
pass's call to `glottalize_vow` on all words, including a vowel-initial
first word. That scope has since been narrowed at the user's request; see
the next section. The separately mapped `w`/`?` phone remains independent:
`ساعت` still contains its accepted lexical stop, and the final-H carrier
remains a separate marker.

Removing the native marker changes timing as well as its open-quotient and
diplophonia targets. For example, `من ایران ...` is about 50 ms shorter at
the 16-kHz male reference setting. It also exposes the previously selected
native stop-aspiration plan in contexts like `یک آدم`. Neither effect should
be silently assumed inaudible. These are native plan changes rather than
PCM cuts. The private switch is process-wide and affects the Finnish-based
provider when enabled, so it is not a request-scoped production feature and
must not be treated as an NVDA implementation.

For `NVDA خوب`, the previously preferred German-resonance baseline holds
English AV at about 53 until the X boundary, then sets it to zero; the
oscillator drains its last period into the X. Two new opt-in comparisons
fade that AV by up to 12 dB over the last 24 ms of the existing English A,
with the previously selected rising X noise. No voiced X frame is added;
period changes are scheduled at native pitch boundaries, and the existing
last period and tract still decay naturally. The second comparison also
stretches the already existing falling-F1/rising-F2 A glide by 15%. It adds
no new phone or silent pause, but it does extend the utterance slightly.
The user selected the second comparison as the incoming A-to-Ach baseline.

```
python3 experiments/persian/test_no_auto_glottal.py \
  --previous /tmp/fa-context/libreloq_eci.so --probe /tmp/fa-no-auto/libreloq_eci.so \
  --espeak-library /path/to/libespeak-ng.so --data-parent /path/to/data-parent
python3 experiments/persian/compare_no_auto_glottal.py \
  --probe /tmp/fa-no-auto/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-no-auto-glottal.wav
python3 experiments/persian/test_voice_release.py \
  --probe /tmp/fa-no-auto/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_voice_release.py \
  --probe /tmp/fa-no-auto/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-a-voice-release.wav
```

## Retain the first word onset and select A-to-Ach variant 2

The user clarified that the first word should keep its former glottal onset;
only the extra Finnish onsets between words should be removed. In the private
provider the `RELOQ_FA_PROBE_NO_AUTO_GLOTTAL` switch now examines the native
word record at the position being adjusted. It retains `glottalize_vow` for
word 1 and skips it for later words. If the word record cannot be read, it
retains the unmodified native rule. The native numbering restarts for a new
sentence; commas do not begin a new sentence. The existing explicit `w`/`?`
glottal and final-H carrier remain independent of this automatic rule.

Context planning inserts silent Finnish-shaped proxies for English. A Persian
word after an English word is therefore a later word in the same native plan,
and the extra onset is suppressed there too. The tests inspect the actual
native controls at that mixed-language boundary, verify isolated `ایران` is
byte-identical to the old first-word pronunciation, and verify the separate
lexical stops, repeated words, sentence boundaries, punctuation and multiple
speaking rates. This still uses the process-wide probe switch: it is not a
thread-safe, request-scoped Persian language module or NVDA integration.

`accepted_persian_ach_compare` selects the user's chosen transition: the
German-guided resonance and noise, the 24-ms AV fall, and the 15% extension
of the existing A glide. It adds no voiced Ach phone or silent pause. The
older `accepted_german_ach_compare` remains available as the preselection
baseline for reproducing the earlier comparisons.

```
python3 experiments/persian/test_no_auto_glottal.py \
  --previous /tmp/fa-context/libreloq_eci.so --probe /tmp/fa-internal/libreloq_eci.so \
  --espeak-library /path/to/libespeak-ng.so --data-parent /path/to/data-parent
python3 experiments/persian/test_voice_release.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compare_first_word_onsets.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --wav /tmp/persian-first-word.wav
```

## Compare U→A across a word boundary

The user hears an unnatural transition in `او آمد` and asks whether a
glottal stop between vowels is natural Persian. A glottalized onset is one
documented possibility, but not an obligatory realization in all contexts.
[Assadi 2003](https://www.internationalphoneticassociation.org/icphs-proceedings/ICPhS2003/papers/p15_3117.pdf)
examines Persian word-initial glottalization in two speakers; full closure
and weaker glottalized realizations occur. [Yazarlou 2014](https://www.sciencedirect.com/science/article/pii/S1877042814037604)
reports direct formant transitions and several glottal realizations in a
four-speaker study of hiatus within words and at morpheme boundaries, not a
direct measurement of the word boundary in `او آمد`. Do not extrapolate
its percentages to that phrase. Both direct and glottalized versions merit
listening; the current between-word policy remains unchanged pending a
decision. The Finnish comparison is glottalization, not a proven complete
acoustic closure.

At the reference settings, eSpeak maps `توالت` to a U→A sequence inside a
word. The private Finnish-based voice plans the first two formant movements
in that word and in `او آمد` over essentially the same 30-ms interval.
Those matching controls do not by themselves prove why the word boundary
sounds abrupt. In `او آمد` F0 falls by roughly 30 Hz in about 20 ms around
that interval. This suggests another listening comparison; it is not a
measured claim that F0 alone causes the reported sound.

`persian_vowel_glide.py` captures one complete Persian clause and introduces
no pause or new phone. The first candidate begins the existing native U→A
tract movement up to 25 ms earlier, eases it to the same native A target,
and preserves the time, source controls and F0. The second additionally
eases the *local* F0 fall, preserving its peak and its later native endpoint.
Both stay inside the adjacent vowels' verified voiced, non-glottal region,
leave at least half of the first word untouched, and stop at the first A
plateau rather than a later vowel. A comma, a consonant, an explicit `w`
marker, another vowel combination or an already glottalized onset leaves
the comparison inactive. At fast speeds the adjustment is correspondingly
bounded and can be skipped if the native frame pattern is not suitable.

This is a frame-replay listening prototype for a Persian-only clause, not
yet an automatic rule in a production language module or NVDA. The narrated
comparison uses three sentences, a Finnish-based word-internal example,
and an eSpeak reference. It includes the previous Finnish glottalization as
a comparison only, without changing the selected onset policy.

The user selected candidate 2: both the existing tract transition and the
local F0 movement are eased. The transition calculation now lives in the
standard-library-only `nvda/_reloq_persian_glide.py`, independent of NumPy,
environment variables and file-based traces. Its caller supplies native
control frames and verified word boundaries. This is a reusable calculation,
not yet a streaming or NVDA integration. The refactored isolated-clause
comparison was checked byte-for-byte against the previously heard result in
60 text/voice/rate combinations.

`accepted_persian_sentence_compare` combines the selected U→A glide and the
previously selected English A→Ach transition in one private full-context
experiment. It retains the first-word onset and does not smooth over comma,
other vowels, noise or an explicit glottal marker. Its extra word indexes are
checked against the original rendering, and the two transitions are tested
in both orders. Like the other full-context experiments, it still uses
private file-based capture, a limited replay configuration and eSpeak in a
separate process; it must not be installed as an NVDA voice.

The separate `build_scoped_acoustics_probe.py` experiment replaces the
process-wide acoustic-probe switches with a thread-local selection of the
approved values. It uses the existing private probe generator and only
rebuilds the changed provider objects. The selected Finnish-based sound is
compared with the earlier probe; scope disabled is compared with ordinary
Finnish, including parallel requests and donor-language isolation. The
caller must restore the previous scope; this is still a private prototype,
not an ECI per-handle setting or an NVDA language.

`espeak_worker.c` and `espeak_worker.py` provide a separate-process text
analysis experiment. They keep eSpeak's global decoder and voice state out
of the host process and support cancelling a blocked request and starting a
fresh worker. Their output is checked against the existing phoneme probe,
including Persian, Pinglish, English, punctuation and mixed text. A focused
Windows x64 job builds the worker and compares it with the actual eSpeak DLL
and language data included in NVDA 2026.3.1, including cancellation and
restart. It is not yet used by a driver. Text coverage,
source-to-speech indexes, and integration with the continuous acoustic
planner are still required for an installable NVDA test.
In the current 94-item reference corpus, 93 inputs map successfully; the
remaining `fa-latn` example contains an actual /w/, which must gain its own
representation instead of reusing the private glottal-stop marker. The
ordinary Persian-script mode routes the Latin `Windows` example to English.

## Initial direct C text components

The desired permanent direction is native Persian text analysis in the shared
C engine. `reference-espeak/persian` preserves the pinned source files and
their original GPL-3.0-or-later notices. `compile_direct_replacements.py`
transfers all 174 `.replace` entries in `fa_rules` into a static native table;
`direct_fa_normalize.c` applies them without eSpeak, including Arabic/Persian
letter variants, character deletions and two-character ligatures.

`compile_direct_lexicon.py` builds a first exact-word C table using the pinned
eSpeak 1.53 reference and the selected Finnish-based probe *offline*. Each
candidate must produce the same supported tag in four reference contexts and
be accepted by the native phone parser. The generated C table contains 15,755
words, including selected sample words whose pronunciation is derived from
`fa_rules`. It does not start eSpeak or consult its data at runtime. A failed
or absent entry is explicitly reported; the table does not guess. Four
contexts are a useful filter, not a proof of correctness in every context.

An additional generated table covers 34 words whose reference tags differ
between phrase-final and nonfinal position. The C converter applies those
tags when a following Persian word is separated by spaces or tabs, and treats
punctuation and line breaks as boundaries. This is a bounded position rule,
not a complete implementation of contextual stress. Other exclusions include
24 tags the Finnish parser changes and 163 with unsupported phones or stress.
The prototype `direct_fa_text.c` handles only supported words, whitespace and
basic punctuation. It returns source mappings at **word** granularity; it
does not supply precise within-word phoneme positions, numbers or Latin
routing. The remaining native conditional pronunciation rules and morphology in
`fa_rules` are still unimplemented, as are the streaming acoustic transitions
and integration with NVDA. This is not a selectable voice or an add-on.
For `او آمد`, the direct C output also feeds the existing private frame-replay
glide through `compare_tagged_vowel_glide`; its selected candidate is compared
byte-for-byte with the previously accepted eSpeak-analysed candidate. This
checks continuity of the chosen sound, not a real-time implementation.

`hybrid_fa_frontend.py` tests a direct-first route for a future early NVDA
trial. It passes unambiguous English ASCII text unchanged to the English path
and converts covered Persian text directly. Persian comma, semicolon and
question marks become their ordinary native punctuation equivalents; other
supported marks and whitespace remain intact. For uncovered inputs without
numbers, the isolated eSpeak worker analyses punctuation-separated clauses,
but the original punctuation is assigned to the preceding language core.
This prevents eSpeak from saying a comma name merely because there is no
space after it. Pinglish uses the separate reference mode. This is a bounded
experiment: clause splitting and English routing still need wider coverage,
and numeric fallback has not adopted the native punctuation strategy.

The upstream orthographic replacements equate U+066B and U+066C with commas.
Both the direct text converter and the hybrid experiment reject those two
numeric separators until number parsing is implemented; they must not become
sentence pauses. For symbol-only inputs the prototype still follows the old
reference behavior. Integration must preserve NVDA's own symbol-level and
spoken-name decisions rather than deciding those here. The worker is started
lazily, so covered Persian or English text does not start another process.
Source maps remain word-granular, fallback has no verified source map, and
neither the NVDA worker nor the accepted streaming acoustic transitions are
connected yet. The focused Windows x64 job compiles the C lookup and tests
sample routing against the eSpeak included in NVDA 2026.3.1. It does not test
an installed Persian NVDA voice.

```
python3 experiments/persian/compile_direct_replacements.py --check
python3 experiments/persian/compile_direct_lexicon.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -fPIC -shared -Iinclude \
  experiments/persian/direct_fa_lexicon.c experiments/persian/direct_fa_normalize.c \
  experiments/persian/direct_fa_text.c src/reloq_utf8.c -o /tmp/fa-direct.so
python3 experiments/persian/test_direct_fa_lexicon.py --library /tmp/fa-direct.so \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
cc -std=c11 -O2 -Wall -Wextra -Werror experiments/persian/espeak_worker.c -ldl -o /tmp/fa-espeak-worker
python3 experiments/persian/test_hybrid_fa_frontend.py --native-library /tmp/fa-direct.so \
  --worker /tmp/fa-espeak-worker --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
```

```
python3 experiments/persian/test_vowel_glide.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/test_glide_planner.py
python3 experiments/persian/test_selected_sentence.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/build_scoped_acoustics_probe.py --work /tmp/fa-scoped
python3 experiments/persian/test_scoped_acoustics_probe.py \
  --scope-probe /tmp/fa-scoped/libreloq_eci_scoped.so \
  --reference /tmp/fa-internal/libreloq_eci.so --baseline eci/libreloq_eci.so \
  --espeak-library /path/to/libespeak-ng.so --data-parent /path/to/data-parent
cc -std=c11 -O2 -Wall -Wextra -Werror experiments/persian/espeak_worker.c -ldl -o /tmp/fa-espeak-worker
python3 experiments/persian/test_espeak_worker.py --worker /tmp/fa-espeak-worker \
  --espeak-library /path/to/libespeak-ng.so --data-parent /path/to/data-parent
python3 experiments/persian/compare_vowel_glide.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --espeak-binary /path/to/espeak-ng --data-parent /path/to/data-parent \
  --wav /tmp/persian-u-a-glide.wav
```
