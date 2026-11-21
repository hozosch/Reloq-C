# Persian development and listening comparisons

## Current implementation status

The normal native provider now contains the selected thread-scoped Persian
acoustics, and the shared ECI core exposes text projection, acoustic scope,
frame capture and replay. The development NVDA package uses that same ECI
library for its Persian and Pinglish choices. Other languages keep their
ordinary acoustics while the Persian scope is off.

Native text support includes character names, common words, and selected
compound endings. Each admitted compound stem is checked against the
existing reference in several contexts; unknown forms retain the fallback.
The NVDA adapter also suppresses redundant emoji presentation selectors split
by source indexes, while letting NVDA's symbol level and dictionaries take precedence.

### Current melody and word stress (dev145 experiment)

The dev149 test adds observed syllable anchors to the US pitch transfer when
both native syllable clocks and the analyzed word agree. Ambiguous clocks
fall back to the earlier word-level alignment. It also introduces a
bounded accent rule for directly recognized compound nominalizations, such as
راه‌اندازی and ذخیره‌سازی. See [PERSIAN_STRESS.md](PERSIAN_STRESS.md) for
the limits and supporting linguistic references. Neither change constitutes
a complete independent Persian language frontend.

The Persian and Pinglish test voices use a US-English pitch pilot while
retaining Persian acoustic frames. Stale cancellation and coincident word
boundaries no longer silently discard this pilot; some unsupported raw
controls or unrecoverable native errors can still use the older route. The
Korean donor described below is historical research.
Pitch and inflection controls remain active. The native text core assigns word stress using bounded
word-class and morphological rules, handles common weak function words in
context, and retains the previous analysis where a form is ambiguous. See
[PERSIAN_STRESS.md](PERSIAN_STRESS.md) for the supporting studies and limits.

The dev142 listening branch additionally moves the long back vowel partly
towards German short /o/ as in *oft* and the front vowel partly towards German
long /a/ as in *Tag*. This is an experimental comparison, not a claim that
the resulting vowels or the US-derived melody have been validated by speakers.

The language is **not yet fully implemented in the native core**. Mixed-text
segmentation and sentence planning still run in the NVDA adapter, and words
outside the direct rules may use an in-process eSpeak text fallback. Persian
and Pinglish have not yet been registered as native ECI languages or exposed
as selectable voices in SAPI5 and Android. An Android cross-compilation test
checks the common native components, not end-to-end Persian playback.

### Automatic duration of weak medial U

In automatically generated words, an unstressed medial /u/ before the word's
primary stress no longer inherits the reference's long-vowel annotation.
After stress and compounds are resolved, the shared text core selects the
same /u/ identity and the acoustic rule supplies a bounded intermediate
duration. This also prevents an already unmarked medial /u/ from collapsing
to just a few milliseconds. Persian script and Pinglish share this rule;
explicitly entered long phonetic input stays under the caller's control.
Stressed, initial and final vowels retain their existing handling. These are
listening-test candidates, not a claim that Persian vowel duration is
invariant; see https://pubmed.ncbi.nlm.nih.gov/28112017/.

### Pinglish A quality

The native Pinglish word lookup distinguishes single `a` (the vowel of
`haft`) from `aa` (the back vowel), without word-specific exceptions.
Known Persian spellings are indexed with the same distinction; unresolved
spellings use the shared Persian text path with explicit vowel marks. This
also applies to word endings: write `-haa` for the back-vowel plural. For
example, `salaam` and `iraan` retain the Persian back vowel, while `salam`
and `iran` contain only short `a` sounds. Informal Pinglish conventions vary;
this synthesizer deliberately follows the unambiguous spelling specified here.

### Numbers and character names (dev147 experiment)

The English decimal/version preprocessing applies only to English text in the
Persian-script voice, not to Pinglish. Native Persian number analysis now
exposes structured cardinal, scale, separator and sign events and recognizes
each component of dotted versions, including Persian and Arabic-Indic digits.
The current speech adapter still renders those events through the existing
Finnish-based phoneme bridge; the new event API is not an independent speech
backend. Unicode character resolution also returns an explicit language for
names without a Persian translation, so the adapter can send an English name
to the English engine without handing inline language controls to Finnish.

### Independent orthographic and copula rules (dev148 experiment)

The shared C text frontend has its own narrowly bounded rule for a
consonant–vowel–consonant word with an explicitly written short-vowel mark.
For example, پَر and پُر remain distinct even without an eSpeak lookup.
Unmarked, partially marked, geminated or vowel-carrier spellings are not
guessed by this rule; the established dictionary and reference fallback still
handle them. Persian spelling omits most short vowels in normal texts, so
additional independent lexical and morphological analysis is still needed:
https://pmc.ncbi.nlm.nih.gov/articles/PMC6814645/.

The ordinary copula است / Pinglish ast is unaccented after its host, with
citation and punctuation boundaries retained. This also applies after an
English host. When an unknown Pinglish word needs the temporary reference,
its known neighbors now retain their own contextual stress instead of being
replaced by the reference's whole-sentence stress. The existing US contour
and vowel timing are otherwise kept; the pronunciation and timing remain
subject to listening tests.

### Historical Korean contour experiments (dev137)

The dev137 candidate transfers the Korean core's own pitch to ordinary
Persian and Pinglish speech. An experimental Hangul proxy groups Persian
syllables into approximate Korean accentual phrases, using the existing stress
marks and respecting punctuation. It is not a translation and cannot reproduce
the longer comparison's manually translated Korean sentences exactly. As in the original
comparison, the pitch curve is mapped continuously across a sentence, rather
than being warped separately at every Persian word boundary. Native index
callbacks can report sentence changes too early when many word indexes are
inserted. Sentence planning therefore uses sentence-only marks and, when
needed, a separate controls-only capture with a verified matching frame clock.
At a language
switch the Korean donor includes adjacent proxy context, so a Persian phrase
does not acquire a spurious sentence-final fall before English. English keeps
its own contour with switch-edge correction limited to two semitones.
Separated Persian segments after punctuation or inline controls go through
the same Korean planner instead of silently falling back to Finnish. An NVDA
pitch change inside a Persian utterance retains the Korean sentence contour;
the relative pitch adjustment is measured from native frames. Bracket
boundaries retain their separate treatment. Some other unusual mid-utterance
ECI controls still use the earlier route. Pitch, Hertz pitch, speed and inflection
remain adjustable. Persian at NVDA pitch 65 and inflection 30 now selects
the same Korean donor settings as dev136 did at NVDA pitch 68 and inflection
37 (Korean pitch 68 and inflection 43). Monotonic maps preserve both slider
endpoints; explicit Hertz values remain exact. A prose colon restarts the
Korean sentence planner like a full stop, while numeric time colons and URL
schemes are not treated as sentence breaks. The original long comparison
used Korean inflection 30. This candidate requires native-speaker review.

The donor retains Persian syllable counts while grouping some adjacent words;
it does not insert a special final syllable or fit reference pitch patterns. The Korean
core plans each donor with the supplied punctuation and inflection. Additional
context at a language switch prevents the Korean planner from treating an
unfinished phrase as a sentence ending. This remains a syllable proxy, not a
Korean translation or a claim about correct Persian intonation. The listening
comparison is captured from NVDA's real speech pipeline at explicit settings.

The grouping is an experimental approximation, not a claim that its boundaries
are correct Persian prosody. It prefers short groups and increasingly penalizes
merging marked accents; neither the original listening sample nor measured F0
values enter the algorithm. Korean accentual phrases commonly have two to five
syllables ([Jun 1998](https://linguistics.ucla.edu/people/jun/papers%20in%20pdf/Jun-Phonology1998.pdf));
the new candidate requires native-speaker evaluation and comparison with dev135.
`compare_korean_grouping.py` produces a controlled comparison of the earlier
translated Korean source, the previous word-by-word donor and the grouped
donor, all at Korean inflection 37 and with the same Persian synthesis frames.
It also includes the original comparison and the calibrated current runtime.
Its `--check-runtime` and optional `--old-pcm` verify the respective runtime sections.

For the reviewed long passage, a dense sequence of word marks previously
placed the final question's contour 890 ms before the sentence mark used by
the original experiment, crossing about 497 ms of voiced speech. The dev135
sentence-only marks reproduce all original sentence boundaries; source marks,
glides, inline pitch and multiple sample rates have regression checks. This
does not make the neutral-syllable donor equivalent to translated Korean: in
the reviewed passage, changing the donor at the same Korean inflection causes
larger F0 differences than the original passage-wide pitch normalization.
This is an acoustic observation, not a naturalness or intelligibility score.
`compare_korean_sources.py` creates a controlled English-labelled comparison
from one shared Persian frame capture. Its `--check-runtime` option checks
that the current proxy section matches the live language-engine PCM; an
optional `--original` checks the original approved recording's Korean section.

## Persian oral nasals and nearby vowels

The current listening candidate runs the French /m/ acoustic rules and a
French-bass/US-upper-resonance /n/ against Persian phone records. The /n/ voice
source, F0 and lowest resonance remain French; higher contextual targets come
from the English alveolar rule. US nasal durations and transition
scheduling remain unchanged. Persian vowels next to those nasals keep their own
vowel targets; the nasal remains free to form a normal transition into them.
The vowel rule no longer selects Finnish nasal-specific formant or tilt
allophones for /m/ and /n/. This is distinct from adding a nasal vowel.

`test_us_nasals.py` checks French /m/, French-bass /n/, US phone durations and
the vowel's formants and tilt for both voices and multiple speeds. It also
measures synthesized upper-frequency energy relative to the lower band in
the reported `nun` and `mim` cases. Listening remains necessary to assess
the transitions and perceived voice quality.

## Persian-owned mixed-text analysis

The independent Persian event planner now marks ordinary English words in
Persian text and the English fallback cases in Pinglish as source-backed
English-text events. The original UTF-8 byte offset and length are retained,
and source indices continue to map to event boundaries. Persian/Pinglish
words, numbers and explicit phonetic input still use the same Persian event
inventory. `test_persian_owned_plan.py` builds this analyzer without any
language provider, NVDA or eSpeak; `test_persian_owned_api.py` exercises the
same exported API in the shared library.

The existing production NVDA adapter does not yet consume this event plan for
its complete mixed-language playback. Prosody, streaming and the fallback
for words not covered by the direct analyzer still need to move before the
voice can be described as independent of NVDA and eSpeak.

The opt-in complete C analyzer and projection now apply general orthographic
rules to uncovered Persian words while preserving known word context and
source indices. Explicit vowels and long-vowel letters take precedence;
some unwritten short vowels must be inferred and can remain ambiguous.
For a classified verb following the productive می/نمی prefix, the native
rules first combine an independently known stem with its inflection. This
uses sentence or half-space context and is not applied to the same spelling
in isolation, where a nominal reading may be possible.
When no eSpeak binary is present, the existing NVDA test adapter remains
selectable and uses this native projection for these cases. When a reference
is present its established fallback still applies. Audio planning and mixed
language orchestration still belong to the NVDA adapter, and these fallback
rules need broader pronunciation review.

## Issue 49: earlier listening candidates

The dev123 listening candidate restores the isolated Persian front /æ/ to
the dev121 rule after the dev122 shape was reported as wrong. It keeps the
dev122 US stop, affricate and fricative rules. The short Persian /o/ before
/n/ gets a modestly broader F1 and a reshaped upper response; the back /aː/
in Iran is a different phone and is not the target. A smaller adjustment
also opens /e/ while retaining its first two formant centres. These are
listening candidates, not claims of perceptual equivalence to US /æ/.

The /m/ and /n/ paths use the US duration, acoustic and nasal-transition
rules adapted to Persian records, with a correction for the differing native
F1 scheduling. The separate vowel-nasality experiment remains disabled. L,
the language-specific /x/, uvular and glottal carriers and the shared
sentence-melody planner are not retuned.

`compare_word_stress.py` produces a German-labelled audio comparison of the
current eSpeak-derived stress, a possible final-syllable stress, and no
explicit stress in a few nouns and a short sentence. It is a listening probe,
not an automatic rule: Persian stress depends on grammatical structure,
including prefixed verbs and enclitics. At that time, production text stress remained unchanged until those cases
were addressed. The current bounded rules are described above.

The dev122 candidate transfers the US consonant control flow, duration and
transition calculations for ordinary /p t k b d g f v s z ʃ ʒ/. The native
affricate expansions for /tʃ dʒ/ use those same US stop and fricative rules.
US phone and feature names are mapped to Persian's existing phone records;
the marked uvular and glottal carriers and the Persian /x/ remain separate.
The former automatic Finnish H impulse after /p t k/ is disabled. American
transition features instead schedule aspiration, including after final
/p t k/. The existing Persian L and sentence-melody selection are unchanged.
The earlier selected D-before-B and explicitly marked affricate-gemination
rules remain context-specific exceptions.

The short front vowel now shapes F3, F4 and the F3–F5 bandwidths together,
while retaining its F1/F2. Its comparison covers individual harmonics at
multiple matched fundamental frequencies, not just summed frequency bands.
The other five vowel rules also receive upper-resonance adjustments; /o/ and
/u/ move F2 slightly as well. The accepted back-A F1/F2 damping is retained,
and its nasal correction stays off. This is a listening candidate, not proof
of perceived openness or a claim that every vowel is identical to English.
`test_us_consonants.py` compares actual consonant frame trajectories, including
initial and final /tʃ dʒ/ and /p t k/ across all six neighboring vowels.

The dev115 candidate preserves the US-English pitch contour within an English
segment of a Persian/English utterance. Only its first and/or last 45 ms of
voicing blend towards the existing Persian pilot at an actual language switch;
brackets still use their native boundaries. The calculation is in the common C
frame layer, and the NVDA adapter retains a request-local copy of the original
pitch for repeat planning and streaming. Persian-only and English-only text do
not use this cross-language pitch change.

For a controlled timing comparison, the Persian acoustic scope also shortens
short, unstressed, word-internal vowel nuclei by 10%. The choice is a listening
hypothesis, not a general Persian duration rule or a copy of eSpeak event
times. Word-final vowels, explicitly long or stressed vowels, L, consonant
rules and sample-rate filters stay as before. This needs native-speaker and
NVDA listening feedback.

The dev118 listening candidate keeps the back-A nasal correction disabled.
It broadens the first two *back-A* resonance bandwidths by 25% when they are
explicit positive values, retaining the formant centres and inherited values.
For the short-front /æ/, it leaves those two bandwidths alone and moves
positive F3 targets 140 Hz toward the British "sad" vowel, preserving native
contextual endpoint differences. The corresponding Persian number 100 has an
explicit long-front-A marker, while the written word صد uses short A; both
are covered. For the male voice, B5 moves halfway from 500 to the British
example's 260 Hz; the female B5 is already 500 in both and remains unchanged.
These are listening candidates, not claims that all Persian and British vowel
contexts should have identical acoustic targets.
It blends selected stop release levels and the /m/ and /n/ targets toward
US-English values, while keeping the existing L, contextual transitions and
marked uvular/glottal carriers. These limited changes are listening
hypotheses; they do not establish that the result sounds more natural.
`build_a_nasal_comparison.py` can still make a separate comparison that
re-enables the former back-A correction without changing the default.

`compare_us_persian_melody.py` creates a German-labelled offline WAV with
Persian statements and questions, each followed by the same Persian frames
with a US-derived pitch contour. The proxy preserves the selected Persian
word stress and fits its contour word by word. The script verifies that
replaying the original frames is sample-identical and that the candidate
has the same duration. It is a listening experiment, not an adopted Persian
intonation model or an NVDA implementation.

The notes below record the original experiments and may describe older
private builds or earlier stages of implementation.

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
be accepted by the native phone parser. The generated C table contains 15,811
words, including selected sample words whose pronunciation is derived from
`fa_rules`. It does not start eSpeak or consult its data at runtime. A failed
or absent entry is explicitly reported; the table does not guess. Four
contexts are a useful filter, not a proof of correctness in every context.

An additional generated table covers 34 words whose reference tags differ
between phrase-final and nonfinal position. The C converter applies those
tags when a following Persian word is separated by spaces or tabs, and treats
punctuation and line breaks as boundaries. This is a bounded position rule,
not a complete implementation of contextual stress. Other exclusions include
24 tags the Finnish parser changes and 169 with unsupported phones or stress;
one further candidate cannot be aligned.
`compile_direct_plural.py` also transfers a bounded productive pattern for a
known word followed by a zero-width non-joiner and ها. A bitmap marks bases
whose final and nonfinal tags match the reference in four positions and whose
two outputs the Finnish parser accepts. At runtime C constructs the tag from
the existing base and suffix without starting eSpeak. A generated fingerprint
prevents compiling the bitmap against a different lexicon. This particular
pattern does not cover unknown bases or other forms of morphology.
`compile_direct_suffixes.py` adds six more direct suffix patterns: ها, تر,
م, ت, ش and ی without a non-joiner. Each candidate stem is admitted only
when its generated tag matches eSpeak in four positions and the native
phoneme parser accepts it. The resulting six compact bitmaps contain 71,203
admissions, representing about 70,853 distinct forms not previously covered.
Existing whole-word and context entries keep precedence. Unknown stems and
unadmitted transformations still use the reference fallback; these bounded
rules are not a complete transcription of the 6,104-line `fa_rules` file.
`compile_direct_numbers.py` generates the 0–999 number groups from the
pinned eSpeak dictionary and validates their tags in four contexts. The C
number experiment composes groups up to 999,999,999,999, retaining exactly
one connecting vowel between nonzero groups. It accepts ASCII, Persian and
Arabic-Indic digits; ASCII comma and U+066C group thousands, while ASCII full
stop and U+066B introduce a fractional part. Fraction digits are read
individually; leading-zero integers are also read as individual digits.
Ambiguous or malformed grouping, repeated decimal separators, integers above
12 digits and fractions above 32 digits return a distinct unsupported status.
Leading plus, hyphen-minus and Unicode minus receive Persian sign words;
number ranges, times, dates, ordinals and currency-specific forms are not covered.
The basic 0–999 groups are checked against eSpeak; stress in composed large
numbers and decimal phrases is an approximation.
Unlike eSpeak's double connective in some thousand-plus-unit cases, the
composition uses one connecting vowel.
The user accepted the resulting eleven-example number listening probe on
2026-11-15; wider word and context coverage still needs validation.

The prototype `direct_fa_text.c` handles supported words, these number
forms, whitespace and basic punctuation. It returns source mappings at
**word or number** granularity; it does not supply precise within-word
phoneme positions or direct Latin routing. The remaining native conditional pronunciation rules and morphology in
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
supported marks and whitespace remain intact. For uncovered inputs, the
isolated eSpeak worker analyses punctuation-separated clauses, but the
original punctuation is assigned to the preceding language core. When
supported numbers appear in an otherwise uncovered sentence, checked
invariant Persian placeholder words retain clause context during reference
analysis; those words are replaced with direct numeric tags only if alignment
is unambiguous. This provides no fallback source map. Pinglish uses its
separate reference mode with the same direct numeric substitutions.
This prevents eSpeak from saying a comma name merely because there is no
space after it. This is a bounded experiment: placeholder alignment, clause
splitting and English routing still need wider coverage.
In the offline shared-sentence planner, the inaudible planning proxies also
retain punctuation inside Persian and English parts; this does not yet make
that planner a streaming NVDA integration.

The upstream orthographic replacements equate U+066B and U+066C with commas.
The text path preserves them before numeric analysis so neither is confused
with a sentence pause; the independent normalization probe continues to
reproduce the upstream replacements. For symbol-only inputs the prototype still follows the old
reference behavior. Integration must preserve NVDA's own symbol-level and
spoken-name decisions rather than deciding those here. The worker is started
lazily, so covered Persian or English text does not start another process.
Source maps remain word-granular, fallback has no verified source map, and
neither the NVDA worker nor the accepted streaming acoustic transitions are
connected yet. The focused Windows x64 job compiles the C lookup and tests
sample routing against the eSpeak included in NVDA 2026.3.1. It does not test
an installed Persian NVDA voice.

## Direct text with ECI position marks

`nvda/_reloq_persian_projection.py` is a reusable, not yet registered NVDA
component. It joins the pieces of one speech job before direct analysis, then
projects ECI indexes and boundary-local pitch, speed and volume commands
through the native source map. It never cuts a generated phoneme tag. Since
the map is word/number-granular, an index inside one maps to its beginning;
an in-word prosody change is rejected instead of being placed inaccurately.
User inline controls, unsupported words and unplanned language switches are
deliberately rejected without starting eSpeak in this direct route.

`direct_fa_sequence.py` connects this projection to the experimental
thread-scoped Finnish-based acoustic engine in one native ECI utterance. The
native engine keeps parsing its generated tags even when the user-facing tag
setting is disabled; source-side user tags are rejected by direct preflight.
The tests verify the output against the previously selected reference across
three rates and two voice genders, position ordering, boundary controls,
scope isolation, cancellation and restart. This validates the native
projection and base sound: it does not yet implement the separately accepted
streaming U-to-A or English-to-Ach transitions, nor does it wire Persian into
the NVDA driver. The shared-sentence frame-replay experiment remains offline.

`direct_fa_letters.c` adds 39 standalone Persian-script letter names,
including the Persian alphabet and several additional forms. Their tags are
generated from the pinned eSpeak spelling mode and checked against the
selected Finnish-based native parser, without an eSpeak process at runtime.
For the names of ر and گ, the reference's long initial consonant is rendered
as the corresponding ordinary consonant because the native tag cannot express
eSpeak's length notation directly. Multi-character spelling jobs can name
sequences of supported Persian letters, with indexes at character boundaries;
the Pinglish profile also names sequences of Latin letters. Punctuation and
digits still need their normal NVDA symbol/character handling, and jobs mixing
spelling and ordinary text are not yet supported by this adapter.

The optional reference fallback can now project source indexes for a limited
Persian-only case. The separate helper collects eSpeak word and phoneme events
while discarding its audio. `reference_alignment.py` checks each word start
and the complete phoneme sequence against the independent eSpeak text result,
then builds a monotone UTF-8 source map. The selected Eloquence parser also
has to accept the resulting tags before the scoped experiment emits audio.
This preserves word-level rather than character-level accuracy, and fails
closed when alignment cannot be verified. Directly covered words and numbers
do not start the helper. In a Persian-only fallback with numbers, a checked
stand-in word provides each number's reference position. Its word and phone
events must agree before the stand-in is replaced by the native number tag;
each index inside a number maps to the beginning of that number. Mixed-language
utterances still need the continuous planner.

The eSpeak short-pause phone `_` before a vowel selects Finnish's own
glottal-onset rule without inserting the separate Persian glottal phone.
The selection is scoped to the relevant word on the synthesis thread; it is
cleared after every job. An existing reference glottal phone is not doubled.
The earlier native word boundary after `d` in `بدبخت` changed its prosody
and did not preserve its D perceptually, so it has been removed. A scoped
selected second candidate doubles the native D duration only before B inside one word;
its native release and the whole word's stress remain intact. Consonant
boundaries have no extra punctuation. An explicit pause at a word edge is carried to the next
vowel inside the same phrase; a real punctuation mark remains a native
phrase boundary and does not get an additional inferred stop. At the
beginning of the phrase, the Finnish-based core retains its normal onset. The separate
`direct_fa_pause_words.inc` table covers 98 source words with invariant
internal behavior without an eSpeak worker. Each was checked in four
reference contexts and against the selected native phone parser. Word-edge
pauses stay on the context-aware reference route. Other unsupported pause
phones remain rejected.

If a job has no internal marks, including when it has only a start or end
index, the reference words can be used without asking for events. This also
allows supported direct numbers in a Persian reference phrase. Obtaining
events for internal positions currently requires eSpeak to synthesize
discarded audio in its helper, so the cost grows with text length. Cancellation
stops the helper and the next request starts a new one. This is a bridge
towards testing, not a replacement for the planned native Persian text rules.
The targeted Windows test checks its event alignment against NVDA's eSpeak
DLL; only the Linux probe currently checks the selected acoustic PCM.

`nvda_driver_probe.py` exercises the actual `LanguageEngine` and
`EciTextTransport` contract with private test-only Persian and Pinglish profiles. Its
owner-scoped frontend and acoustic handle support settings, dictionary
selection, word indexes, isolated character names, cancellation and restart.
The native queue is tested locally and in a portable Windows NVDA. The test
profile registers Persian and Pinglish in the explicitly labelled private
add-on. Explicit NVDA language changes and clear Latin runs in Persian text
are spoken with the ordinary language handles. The accepted shared contour
and transitions remain offline, so those switches can still have pauses.

The explicit Pinglish test profile sends general Romanized words to the
`fa-latn` reference and gives all 26 Latin letters their eSpeak spelling names.
Common spellings of selected words such as `salam`, `iran`, `khub`, `nokte`,
`kaghaz` and `farsi` use their accepted Persian-script sounds and stress,
including the selected vowels and uvular consonant. Their
source maps preserve word-level NVDA indexes, including when a reference word
or direct number occurs in the same phrase. A raw phoneme or source position
that cannot be verified is rejected. The profile is selectable in the
experimental NVDA add-on.

`link_unified_probe.py` links the direct Persian text components into the
same private Linux ECI library as the scoped acoustics. It reuses the
previous provider archive and host objects; the final link still performs
link-time optimization over that archive. The resulting library can be
passed as both `--native-library` and `--scope-probe` to the driver test,
so directly covered text uses one native library and no reference process.
The combined probe passed the native queue test, all 98 direct pause-word
comparisons, 54 selected-acoustic comparisons and concurrent Finnish/Persian
isolation checks. This is an experimental link step, not a Windows/NVDA package.

`build_windows_probe.py` prepares the same scope in a separate Windows DLL,
replacing only the changed provider objects in a private copy of the ordinary
provider archive. The dedicated Windows acoustic workflow builds the baseline
archive once and caches it. Direct text changes require a matching private
probe because its embedded rules are used by the test add-on.
It checks Finnish, English and German against the unchanged DLL, tests the
thread-scoped Persian sound, and compares direct Pinglish and Persian output.
The DLL remains a private probe until those Windows checks have passed and the
remaining streaming/NVDA work has been completed.

`build_experimental_nvda.py` prepares a separately labelled test package
using a verified Windows acoustic probe with its own direct Persian text
exports. The add-on loads those rules from the same DLL as the acoustic core.
For words not covered directly, it uses an independently loaded in-process
copy of NVDA's eSpeak library; no helper process is packaged. Its adapter
exposes Persian and Pinglish only when the required exports and NVDA eSpeak
are present. Mixed-language sentences use the accepted shared-contour and
transition planner, with separate handles and bounded streaming; this test
implementation is not yet a native single-pass Persian language module. The
ordinary release builder and language catalog are unchanged.
`test_nvda_package_probe.py` checks the packaged adapter's profiles and native
playback; `test_nvda_real_plugin.py` also exercises it in portable NVDA.

An initial geminated /dʒ/ candidate used a separate D followed by J, but the
user heard insufficiently continuous voicing and did not accept it. The
user selected the first longer single-J candidate: its native closure is
extended, followed by one voiced affricate release. The test driver collapses
eSpeak-derived doubled J's and selects the corresponding native word and J
ordinal on the synthesis thread. Ordinary single J's are unchanged; ambiguous
literal D+ZH contexts retain their old pronunciation. The selected voiced-S setting
changes only the frication target, retaining its voiced source and the
accepted voiceless S. The D and Z choices are listening decisions, not
universal duration or intensity rules.

The comparison contains `تعجب` and `توجه`, each with eSpeak, the earlier
doubled J, and three candidates extending one native J. Across three sample
rates and both voice genders, frame checks show a shorter unvoiced gap than
either rejected doubling. The user selected candidate 1.

The selected Finnish H impulse after P/T/K before a vowel now also applies
when the stop follows /m/ or /n/ inside a word. Its established 60% duration
is unchanged. Other consonant clusters, the special uvular stop, and ordinary
Finnish remain on their previous paths.

Character mode names Persian and Latin letters, Persian/Arabic/ASCII digits,
and common symbols directly, including an explicit space. Other valid Unicode
characters are identified by their code point. Ordinary text and spelling
segments can be combined while preserving NVDA indexes.

```
python3 experiments/persian/compile_direct_replacements.py --check
python3 experiments/persian/compile_direct_lexicon.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compile_direct_plural.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/compile_direct_numbers.py \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic -fPIC -shared -Iinclude \
  experiments/persian/direct_fa_lexicon.c experiments/persian/direct_fa_numbers.c \
  experiments/persian/direct_fa_normalize.c \
  experiments/persian/direct_fa_text.c experiments/persian/direct_fa_letters.c \
  src/reloq_utf8.c -o /tmp/fa-direct.so
python3 experiments/persian/test_direct_fa_numbers.py --native-library /tmp/fa-direct.so \
  --espeak-library /path/to/libespeak-ng.so --data-parent /path/to/data-parent
python3 experiments/persian/test_direct_number_audio.py --native-library /tmp/fa-direct.so \
  --probe /tmp/fa-internal/libreloq_eci.so --wav /tmp/persian-numbers.wav
python3 experiments/persian/test_direct_fa_sequence.py --native-library /tmp/fa-direct.so \
  --scope-probe /tmp/fa-scoped/libreloq_eci_scoped.so \
  --reference /tmp/fa-internal/libreloq_eci.so
python3 experiments/persian/test_direct_fa_lexicon.py --library /tmp/fa-direct.so \
  --probe /tmp/fa-internal/libreloq_eci.so --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
cc -std=c11 -O2 -Wall -Wextra -Werror experiments/persian/espeak_worker.c -ldl -o /tmp/fa-espeak-worker
python3 experiments/persian/test_direct_pause_words.py --native-library /tmp/fa-direct.so \
  --worker /tmp/fa-espeak-worker --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent
python3 experiments/persian/test_nvda_driver_probe.py --native-library /tmp/fa-direct.so \
  --worker /tmp/fa-espeak-worker --espeak-library /path/to/libespeak-ng.so \
  --data-parent /path/to/data-parent --scope-probe /tmp/fa-scoped/libreloq_eci_scoped.so
python3 experiments/persian/link_unified_probe.py --scope-work /tmp/fa-scoped \
  --work /tmp/fa-unified
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

## Short front vowel and continuing bracket phrases

The Persian short-front vowel retains the Finnish provider's contextual
formants and transitions. Immediately before or after the Persian Ach phone, its
first-formant targets are raised by 100 Hz, keeping the other formants and
native inheritance markers. The `alef` reference and non-Ach contexts remain
unchanged from the earlier contextual probe, in both voice types; unscoped
Finnish is unchanged. This is a listening candidate for the targeted context.

For mixed speech, a bracket phrase with more speech following receives the
native core's comma continuation tone and omits only excess end silence.
Explicit sentence and question endings stay intact. Parentheses after a mixed
pass can also bound planning, so a following NVDA role or checkbox state is
handled by its own direct Persian job. Direct verified tags and the finite
internal English proxy alphabet no longer require repeated parser probes;
unverified reference pronunciations retain validation.

The two contextual planning captures can skip discarded sample synthesis and
sample callbacks while retaining their exact native frame and source-index clocks. The pitch
pilot still renders audio to locate its audible end, and final audio is
rendered once from the selected frames. Bounded contour interpolation and
pitch fitting are performed natively, with the Python calculation retained for
older builds and unusually long utterances. Captures fall back to full rendering
when the native control-only capability is absent. Canonically equivalent
orders of Arabic vowel marks and shadda map to the same direct pronunciation.
Reference fallback uses eSpeak's expected mark order without deleting marks.
For a `-دار` half-space compound, independently verified halves that exactly
reproduce its phone and stress sequence receive separate native word units,
restoring the checkbox state's phrase tone. Other compound forms retain their
existing pronunciation and prosodic path.

In both Persian and Pinglish, raw brackets adjacent to phoneme tags are
separated before reaching the Finnish parser, and raw hyphens become word
boundaries. Explicit symbol names and spelling are still passed as Persian
phone tags; native punctuation and index ordering remain intact.

## Uncached mixed-language response and decimal routing

The mixed-sentence planner still captures the established shared contour and
both native language contexts. Its tick extraction, timeline, pilot audible-end
scan and frication flags can use private native routines with the previous
Python implementations retained for older probes. The German Ach reference
capture no longer renders discarded samples. The German handle is opened only
when that selected transition is needed.

When the first native context leaves at least 160 ms of audio ahead of a
250-ms boundary reserve, replay begins while the other context is planned.
Both frame slices use one uninterrupted native synthesis session. Requests
that cannot meet the reserve, and first Persian segments containing the
selected U-to-A glide, keep complete planning before playback. If later
planning fails after playback starts, the first native context is completed
and the original remaining segments are spoken through their own engines;
they are not restarted or dropped. Cancellation releases the replay handle.
The audible-prefix route does not require a previous sentence-result cache.
It does not yet make mixed input equivalent in latency to a single native
language, since the shared contour and language contexts are still planned.

At a language boundary, a bounded inherited voiced carrier is removed before a
verified voiceless fricative, H or stop only if the corresponding unvoiced
native frames follow. Stops retain their closure and aspiration. The selected
NVDA-to-Ach treatment and voiced consonants retain their existing paths.

Neutral number fragments inherit the script context across NVDA text and
index commands. Thus an English decimal split across several pieces stays in
the English core, where a period is spoken as the English decimal point,
while a standalone number or one in Persian remains Persian.
The same applies to numbers adjoining a known English exception in Pinglish.
Explicit language and spelling commands continue to delimit the decision.

`benchmark_uncached.py` measures the first fake WavePlayer feed through the
actual NVDA transport, separately from the first request with cold handles.
It clears cross-request plan, phone, Ach, capitalization and fallback-word
result caches before every utterance, uses different inputs and reports a
native-English comparison. It includes separate indexed reference-fallback
cases both with and without an intervening stop. Its timings do not include a
real device or NVDA event dispatch.

The NVDA test package loads a private copy of NVDA's eSpeak library in the
same process, rather than starting a reference helper process. Its voice and
callback state remain independent from NVDA's own eSpeak. An idle stop keeps
the copy loaded; after an interrupted analysis, the private copy is reloaded
before the next request so no incomplete parser state is reused. The direct
Persian rules still avoid the reference altogether when they cover a text.
Identical text analysis is reused only within an individual alignment attempt;
event evidence remains checked against the ordinary reference phonemes and
positions. Where eSpeak
supports it, discarded event audio is generated at a faster rate and restored
afterward; if its evidence differs, alignment retries at the original rate.
The audible Eloquence voice and its speech rate are unaffected. The sentence
planner checks fallback pronunciations during its complete pilot capture before
streaming, instead of first checking every tag individually; bracket-only
plans retain the earlier preflight before their first streamed audio.

Before NVDA's Persian symbol processing, English decimal runs are preserved
as native text commands, so the Persian symbol dictionary cannot replace the
period with its Persian name. Explicit language boundaries, spelling mode,
NVDA dictionary matches, and ordinary Persian numbers retain their own paths.
The same rule applies to Pinglish words already identified as English
exceptions. The normal native English decimal processing remains in charge.

## Issue 49: character names, hyphens and articulation comparison

The standalone Arabic vowel marks and related signs reported in issue 49 have
Persian character names in the C-backed letter table. They use the pinned
Persian eSpeak character pronunciations, including the multi-syllable name of
U+0670. Combining marks within an ordinary word continue through normal word
analysis. Explicitly spelling a hyphen still speaks its Persian name.

The native word projection treats a hyphen between words as a word boundary.
When an unknown Persian word requires reference fallback, spaced and unspaced
word hyphens also remain boundaries; they do not cause the surrounding words
to be spelled out. The fallback tests use both reported examples with text and
index events split at different positions, and the real NVDA test exercises
both examples and the nine individual marks.

NVDA commonly stops the previous utterance before submitting a new one. A
stale native stop flag used to abort the phoneme validation of an otherwise
valid reference pronunciation, which made the fallback spell its letters.
Before reference validation the Persian engine clears that old native stop,
then checks for a new cancellation. Standalone and mixed-language projections
share this guarded validation path; the sentence-melody algorithm is unchanged.

The front-A listening candidate was revised after the earlier British-F3
adjustment was reported as more muffled. Analysis compares native control
trajectories and rendered vowels from Persian, British and US voices with
matched source pitch; F3, B3 and B5 are considered together. In the measured
male 100/sad example, upper-band energy moves closer to US English, but this
measurement alone cannot establish perceived clarity or Persian naturalness.
The unchanged F1/F2 and the language-specific Q, X and L are separately
checked. The articulation candidate also transfers selected US stop and
frication targets, adjusts consonant durations and stop transitions, and
changes short/long vowel timing before selected voiceless consonants. These
are listening candidates, not claims that every Persian sound should have
English timing. The shared sentence-melody planner is unchanged.

#### Incremental Persian prefix playback

For a covered Persian or Pinglish phrase, independent native handles can
capture the Persian acoustics and US contour concurrently. Each capture may
publish a copied snapshot after two word marks; the NVDA worker fits the first
word and starts replaying its stable prefix while the remaining capture is
completed. Replay retains its native session across the two slices. Weak-word
controls and bracket/comma boundaries are included. Source indexes inside a
phrase, inline pitch changes, unvalidated reference-reader output, and an
initial cross-word vowel glide retain the complete-plan path. The snapshot API
is thread-local; callers copy the rows before returning to native synthesis.
Cancellation stops both capture workers and the replay handle.

`test_prefix_streaming.py` compares PCM and NVDA index positions against the
complete plan over multiple voices, clocks, sample rates, randomized native
words, Persian and Pinglish, and interrupted speech. `benchmark_uncached.py`
records a separate complete/incremental first-feed comparison; this uses a
nonblocking test player and does not measure an actual device or NVDA event
loop. No synthesized audio or sentence result is cached.

# C-only Persian speech pilot (21 November 2026)

`eciPersianSpeakNativeUtf8` is an experimental route from UTF-8 Persian or
Pinglish through the existing language-owned C event plan to the shared ECI
acoustic engine. The caller supplies an idle Finnish-backed ECI handle with
its output callback configured; no NVDA driver or eSpeak runtime is needed.
Unknown Persian words use the C orthographic completion rules, so an
unwritten vowel can still be inferred incorrectly. Persian glottal-onset
and geminated-affricate markings are retained for supported single-language
plans; the API rejects a mixed-English plan containing these markings until
per-run scope is available.

The pilot is deliberately separate from the existing NVDA voice. It does
not yet implement the established US sentence contour, character-spelling
mode, user inline controls or reliable cancellation during preparation.
Some numeric formats, including `3,14`, are currently rejected rather than
silently pronounced incorrectly.
It does not register Persian as an ordinary ECI dialect or connect the new
route to the SAPI5 or Android frontends. The existing accepted Finnish-based
acoustic backend remains in use. These limitations must be resolved before
the pilot replaces the current driver.

`test_native_backend.c` is a standalone C client. It compares the pilot's
output and ordered source indices with the established direct acoustic path,
and exercises Persian, Pinglish, English mixing, uncovered words, multiple
rates and voices. A successful Linux test does not establish Windows/NVDA,
Android, SAPI5, subjective speech quality or real-device responsiveness.
