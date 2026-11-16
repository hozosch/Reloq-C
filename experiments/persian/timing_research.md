# Persian listening probe: L and vowel-duration notes

Historical probe notes: the user accepted the stable Salam-based A, the intermediate final
E/A durations, the Finnish viisi S noise, the mixed German/French L and the
60% Finnish H transition in the private listening build. The selected examples
use eSpeak 1.51's stress on Donyā and Iran; alternative final-stress examples
remain available for research. At the time these notes were first written,
general Persian text analysis and a selectable Persian language were not yet
implemented; both are now available in the private test add-on.

## Decisions already made for the next probe

- Keep the accepted Finnish base and question contour, the three-resonance
  back-A approximation (`RELOQ_FA_PROBE_A=1`), Finnish ä for the Persian
  front vowel, and the native Finnish H transition at 60% (`H=5`).
- Apply the adapted German L rule inside a word and the adapted French L
  rule to an L at the end of a word. This boundary must still be recognized
  when another word or punctuation follows. A syllable-final L before a
  consonant in the same word is not automatically word-final. Do not use
  the earlier sharp formant-offset experiment as a substitute.
- The exact duration of each vowel is to be measured and compared in context;
  no universally fixed ratio has been established by these sources.

  eSpeak's [Persian phoneme source](https://github.com/espeak-ng/espeak-ng/blob/688cb60368d6e0998f3632af8d1c3c2ad4191fc2/phsource/ph_farsi)
  gives the long back A a nominal length of 200 units and the front A 160.
  The optional `RELOQ_FA_PROBE_VOWEL=1` uses their ratio, 125%, as a
  multiplier of the Finnish short back-A nucleus, after normalizing its
  default starting value to that of Finnish front A. The same mode matches
  ordinary E to the front-A starting value (eSpeak assigns both 160). Native
  context, stress, speed and phrase-position adjustments are retained; the
  original Finnish long-A marker is left alone for comparisons. These
  nominal values are not absolute milliseconds and do not establish a
  universal 125% ratio in running speech or in a different engine.

- In the earlier probe, `RELOQ_FA_PROBE_S=1` copied S resonance targets from
  Finnish `viisi`. The subsequent listening feedback asked instead for its
  noise. The revised candidate copies the S noise amplitudes from `viisi`
  and leaves the native formant transitions and timing intact. The user
  accepted it for Persian S in all positions. It remains a private setting,
  not a completed Persian language rule.

- The accepted back A in `salām` is the reference for a new context-stability
  candidate. Its measured F1/F2/F3 centres can be supplied using the private
  `RELOQ_FA_PROBE_A_TARGET` setting. The rule shifts positive endpoints by a
  common amount to preserve the existing endpoint differences and sentinel
  markers. A matching numerical centre is not proof of matching sound; it
  must be checked by ear in multiple contexts. The accepted Salam example
  itself must remain unchanged. For the default listening voice, the traced
  centres are 656, 1000 and 2620 Hz. In the shorter Iran example before the
  candidate, F2 moves from 1150 to 1250 Hz; this demonstrates a real native
  context difference, not a claim that the 200-Hz centre shift explains every
  perceptual difference.

- The centre-only adjustment was still perceived as changing the A quality.
  The accepted listening setting holds the first four formant targets and
  bandwidths of the accepted Salam nucleus, while the adjacent phonemes keep
  their own native trajectories. That is stronger than averaging each pair
  of endpoints. The user accepted this stabilized sound.

- At word-final E and back A, the accepted midpoint candidate moves both the
  native short and long Finnish comparison towards the middle. In the tested
  contexts, E uses 1.566 times the short or 0.734 times the long nucleus; A
  uses 1.28 or 0.82. Across the tested speeds these are within a few percent
  of the midpoint, not an exact general duration law. A word fence confines
  this to the end of the word: e.g. A before /n/ in Iran and E in sefr should
  remain unchanged. This uses explicit test phones, not a completed Persian
  text frontend.

- The short vs Finnish-long phoneme tests are distinct from the eSpeak base
  ratio. `fa_noqte` has a final eSpeak `e:` and `fa_ta` has `A:` in the 1.51
  reference. Compare each explicitly long Finnish phone with its shorter
  version; do not blindly mark all word-final vowels long. The user prefers
  the previous long-phone A in Iran, which is before /n/, so preserve that
  separate example too. Measured nucleus targets are engine scheduling
  values, not identical to eSpeak event positions or acoustic boundaries.

- For the examples in this probe, follow eSpeak 1.51's stress: `s a l ˈA m`,
  `ˈd o n j A`, and `ˈiː r A n`. These are explicitly annotated example
  phones, not an implementation of eSpeak's complete stress or dictionary
rules; an actual Persian frontend must provide those for arbitrary input.

## Evidence

The exact acoustic target still needs context-matched recordings rather than
treating synthesizer scheduling units as measured vowel boundaries. A 2017
two-speaker colloquial-speech pilot found that average durations did not match
traditional long/short pairings; this small sample is not a universal table.
Sadeghi's controlled 12-speaker study found differences for its selected
vowel groups. An earlier multi-speaker automatic-alignment study reported
variation by vowel and syllable structure, while the small 2017 pilot found
no notable coda-structure effect. These differences argue against one fixed
ratio for all vowels and contexts. Another controlled study found that stress
affects syllable duration with and without accent; it does not establish a
precise vowel-only multiplier.

- Aronow et al. (2017), colloquial-speech pilot:
  https://doi.org/10.3765/plsa.v2i0.4059
- Gharavian et al. (2000), automatic-alignment study:
  https://assta.org/proceedings/sst/SST-00/cache/SST-00-Chapter7-p8.pdf
- Sadeghi (2017), word-level prominence:
  https://doi.org/10.1177/0023830916684862

1. Sadeghi, “The timing of pre-nuclear pitch accents in Persian” (2017),
   https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/timing-of-prenuclear-pitch-accents-in-persian/C7CA7887F69ACE368ED7126741F695C5
   reports significantly longer phonologically long than short vowels in
   controlled, accented material from 12 speakers. The study also describes
   the generally word-final stress of Persian nouns, with grammatical
   exceptions. The effect is evidence for a tendency under those conditions;
   it does not establish a fixed universal multiplier for all contexts.

2. Jones, “A Corpus Phonetic Study of Contemporary Persian Vowels in Casual
   Speech” (2019),
   https://repository.upenn.edu/bitstreams/4b6835ad-a66f-48d7-97fd-1258bc30718f/download
   measured a conversational corpus using automatic forced alignment. Its
   table reports a 60-ms median for both front /æ/ and the low back vowel,
   and means of 73.67 ms and 72.43 ms respectively. These are pooled values,
   not matched stressed-vs-unstressed pairs or targets for a different
   synthesizer. The two distributions overlap strongly; spontaneous speech
   is not a reason to ignore the controlled-speech result above.

3. Sadeghi, “Acoustic correlates of lexical stress in Persian”,
   ICPhS 2011,
   https://www.internationalphoneticassociation.org/icphs-proceedings/ICPhS2011/OnlineProceedings/RegularSession/Sadeghi/Sadeghi.pdf
   found stress-related effects on **syllable** duration. Their controlled
   examples do not directly specify a vowel-only scaling factor. The same
   study also describes interactions between stress and position.

4. Majidi and Ternes, “Persian (Farsi)”, Illustrations of the IPA,
   https://www.cambridge.org/core/services/aop-cambridge-core/content/view/5E477B9CC844D65E88493A7BE5E836AF/S0025100300004461a.pdf/persian_farsi.pdf
   describes educated Tehran speech and notes that vowels are longer under
   stress, with additional contextual variation. This illustration uses one
   speaker and should not be treated as a population-wide duration table.

## Concrete follow-up

- Trace the Finnish kernel's existing nucleus-duration calculation before
  changing it; it already calls separate stress and phrase-position
  adjustments. Compare like-for-like speed, voice and context against normal
  eSpeak and a human-speech reference where available. Record the boundaries
  used in measurement rather than inferring one vowel's length from the
  duration of the complete word.
- Compare both vowels in `salām`, the back vowel in `Irān`, and the
  final back vowel in `donyā` alone and before a following word; include
  stressed and unstressed examples. Test more than one speaking rate.
- The current `PERSIAN_PHRASE` test tag explicitly marks the first syllable
  of `donyā` as stressed. Compare a last-syllable-stressed probe as well,
  without changing the previously accepted question contour silently.
- Check L inside a word, word-final before a following word, word-final
  before punctuation, and syllable-final within a consonant cluster. Compare
  native rule traces with German and French and check normal Finnish and
  other languages for isolation.
- Produce a short German-labelled listening comparison once the runtime is
  available. Explicit phoneme tags are test constructions and do not imply
  that Persian text analysis or an installable voice is finished.

## eSpeak 1.51 timing observation

In the isolated eSpeak NG 1.51 diagnostic at 175 WPM, the event-to-event
interval from the back-A phoneme marker to the next phoneme marker in
`دنیا` was 134 ms when the word was spoken alone, versus 101 ms in
`سلام دنیا.`. In the same two conditions, the corresponding A interval in
`سلام` was 151 ms alone versus 90 ms before the following word. These
intervals include the engine's definition of event positions and
coarticulation; they are not manually segmented acoustic vowel boundaries
and cannot be applied directly to another synthesizer. They do demonstrate
substantial context dependence. Reproduce the observations with
`python3 experiments/persian/trace_espeak_events.py` with libespeak-ng and
its voice data installed.

## Internal timing observations

The private Finnish diagnostic uses units of 1/10,000 ms for its nucleus
target. With Donyā as a single explicit phone example, shifting stress from
the first to the final syllable moved the back-A target from about 83.8 ms
to 119.7 ms at the default setting. This does not by itself establish the
physical duration of the vowel or prove the final result perceptually correct.
The old Iran example explicitly marks both /i/ and /a/ as long Finnish
phonemes and yielded an A nucleus target of approximately 183 ms; it also
places stress on the first syllable. A second Iran example retains the vowel
colour, removes the explicit Finnish long-A marker and places stress on the
last syllable. These are listening probes, not a universal Persian rule.

In the earlier probe, before back-A's starting value was normalized to the
front-A base, with final stress in each isolated test at the default native
speed, the internal back-A nucleus targets were:

| Example | Finnish short A | 125% candidate | Finnish long A |
| --- | ---: | ---: | ---: |
| Donyā | 119.7 ms | 149.6 ms | 210.0 ms |
| Iran | 158.6 ms | 198.2 ms | 207.9 ms |

These are scheduling targets from the Finnish core, not measured acoustic
vowel boundaries. They cannot be directly equated with eSpeak's event times.

## Automated context screening, 2026-11-16

`compare_vowel_corpus.py` compares each directly supported Persian word in
the reference corpus alone and before `سلام`, at three native speeds and
three eSpeak rates. It retains a comparison only when the first word's
native tags, vowel sequence and stress match across conditions and both
synthesizers. It reports source-unit durations for inspection, but scores
only a *within-engine* ratio between the two positions. Contexts with
unmatched pronunciations, stress or vowel counts are explicitly excluded.
Candidates are ranked when the ratio difference has the same direction and
exceeds 0.15 at all three speeds. This is a screening heuristic, not an
acoustically measured vowel duration or an instruction to change a rule.
The native and eSpeak rate values are representative; the two scales are not
assumed to be equivalent.

On the local diagnostic build with eSpeak NG **1.53.0**, 46 direct word
candidates produced 261 matched vowel observations across three rates; nine
word/rate comparisons were excluded. The matching event intervals were then
replaced by those from the **1.52.0** eSpeak DLL in the portable NVDA
2026.3.1 Windows test. All 261 observations retained a compatible vowel and
stress; none of these matched intervals changed between the two reference
builds. These are not human-speech measurements. At native speed 98 and eSpeak 175 WPM,
the first-word-before-another-word / isolated-word ratios were:

| Vowel and word | Native nucleus target | eSpeak event interval |
| --- | ---: | ---: |
| back A in `سلام` | 0.669 | 0.596 |
| back A in `ایران` | 0.821 | 0.821 |
| final E in `نقطه` | 0.903 | 0.614 |

The full matched observations and the screened candidate list are recorded
in `vowel_timing_reference_1_52.json` for further analysis. This is offline
research data; the NVDA driver does not load it or use it as a speech cache.

These figures identify contexts for more precise study, not equivalent
physical vowel boundaries. In particular, the already accepted intermediate
word-final E and A and the Iran anchor must not be replaced by blindly
applying the eSpeak event ratio. A short, representative listening check
remains useful for proposed changes, rather than requiring hundreds of
isolated examples. For an acoustic measurement, record audio and use a
validated boundary/aligner on both engines under matched conditions first.

Reproduce with a completed `build_scoped_acoustics_probe.py` work directory
from the same source checkout. The tracing switch is forwarded only by a
separately linked diagnostic library; it is absent from the add-on:

```sh
python3 -B experiments/persian/link_unified_probe.py \
  --scope-work SCOPE_WORK --work TIMING_WORK --continuous-capture --timing-trace
python3 -B experiments/persian/compare_vowel_corpus.py \
  --scope-probe TIMING_WORK/libreloq_eci_persian_probe.so \
  --native-library FA_DIRECT_LIBRARY --espeak-library ESPEAK_LIBRARY \
  --data-parent ESPEAK_DATA_PARENT --output vowel-comparison.json
```

To compare against the eSpeak library packaged by a particular Windows NVDA,
`compare_vowel_corpus.py --espeak-only` exports that library's events without
the diagnostic acoustic library. The Windows package test writes the result
as `persian-timing/espeak-vowels.json`. Combine it with the local diagnostic
report, rejecting incompatible corpus hashes or vowel identities:

```sh
python3 -B experiments/persian/compare_saved_vowels.py \
  --native vowel-comparison.json --espeak espeak-vowels.json \
  --output nvda-vowel-comparison.json
```

The native report has already been screened against its local reference, so
this cross-platform comparison is the conservative intersection of both
screens. Different synthesis cores and platforms still define the measured
boundaries differently.

The diagnostic scope differed from the production scope only in its trace
selector. With tracing disabled, 72 combinations of mixed Persian/English
texts, rates and voices produced bit-identical PCM and source indexes.

## Initial /æ/ in alāmat, 2026-11-16

For `علامت` (alāmat, “mark”), eSpeak NG 1.53 emits the phoneme string
`a: l 'A m a t`. The first vowel's eSpeak event-to-next-event interval is
92 ms at 175 WPM, compared with 100 ms for the last /æ/ in the isolated word
(92 ms versus 94 ms before `دار`). These are event intervals, not manually
aligned acoustic boundaries. The `a:` marker has a meaning inside eSpeak;
copying it as a Finnish long-vowel tag is not an equivalent duration rule.

At native speed 98, the selected Persian scope's old isolated-word nucleus
targets were 69.04 ms for the initial /æ/ and 31.83 ms for the last /æ/.
Using the ordinary short front-A phone for just the initial vowel changes the
first target to 31.14 ms while leaving the later two targets unchanged in
this explicit-phone experiment. The same direction held at native speeds 40
and 200 and before a following word. Targets and eSpeak events have different
boundaries and rate scales; the within-word comparison motivates a targeted
listening candidate rather than a claim of exact physical duration.

Sadeghi's controlled study describes /æ/ as a phonologically short vowel and
finds shorter /æ/ and /e/ than /aː/ and /uː/ in its accented material (12
speakers). Stress and context still affect duration; a small conversational
pilot by Aronow et al. found different aggregate patterns. Neither study
specifies an exact duration for this word. The candidate therefore changes
only the first vowel in the alāmat family, keeps the existing stress and
other vowel targets, and leaves unrelated /æ/ and the accepted Iran A alone.

- https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/timing-of-prenuclear-pitch-accents-in-persian/C7CA7887F69ACE368ED7126741F695C5
- https://doi.org/10.3765/plsa.v2i0.4059

## NVDA symbol-data initialization, 2026-11-16

In an instrumented portable NVDA 2026.3.1 run, the first speech-pipeline
request for `نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA` took
81.3 ms to the first fake player feed. Its in-word half-space triggered the
initial load of NVDA's Persian symbol processor; the symbol call itself took
47.5 ms. Repeating the text took 24.6 ms in that run. Persian speech results
were not retained between requests.

The Persian test add-on now asks NVDA to load its own symbol data at Persian
voice/profile selection. In a subsequent run on a separate Windows CI
runner, that selection took 27.5 ms for its first Persian preparation, while
the first request of the same mixed sentence reached the fake player in
24.6 ms; its symbol call took 0.3 ms. A changed sentence then took 12.8 ms.
The repeated sentence's PCM hash matched the earlier run. These are separate
shared runners rather than a controlled benchmark, and fake player timings
exclude the sound device. They establish the location of the first-load cost,
not a guaranteed improvement on every computer or parity with the native
English path. NVDA continues to own and invalidate its symbol data; no text,
phoneme or sentence result is cached by this preparation.

## Initial /æ/ in alāve, 2026-11-16

The pinned eSpeak 1.53 reference gives `علاوه` as `a: l 'A v e:`.
At 175 WPM, the first front-vowel event interval is 92 ms and the stressed
back-vowel interval is 124 ms. Those are event intervals, not measured vowel
boundaries. The former Finnish long-vowel mapping inverted the relationship:
at native speed 98, the diagnostic nucleus targets were 69.04 and 42.19 ms.
Using the ordinary front vowel for this word reduces its target to 31.14 ms
without changing the back vowel, its stress or its accepted formant targets.
The two within-engine ratios are about 0.74; event intervals and native
nucleus targets are not directly interchangeable. Before another word, the
reference's ratio differs, so this does not establish a universal duration
rule. The correction is limited to `علاوه`, `بعلاوه` and their verified forms.

## Early control-only contour, 2026-11-16

For a long initial segment, the pitch fitted to that segment needs the
control frames and its internal boundary mark, but not the pilot's final
audible-sample position. On multicore systems an independently owned Persian
handle can capture those identical controls without synthesizing discarded
audio. Once at least 300 ms of safe first-segment audio is available, replay
starts; the full audible-EOF pilot is then computed for the rest of the
sentence. Short or unsupported cases keep the existing complete route.

Five alternating local runs using the NVDA transport and fake player gave
medians of 12.76 ms before and 8.71 ms after for the previously reported
Persian-first mixed sentence, with loaded handles and no speech-result cache.
The medians for the first request were 56.74 and 52.17 ms respectively.
These are not hardware or end-to-end NVDA measurements. Exact PCM and source
index comparisons, cancellation and fallback checks remain the acceptance
criteria; the 300-ms reserve is a lower bound, not a guarantee against
underruns on every device.

The English handle's rule parser also performs one-time work on its first
phoneme request. Selecting a Persian voice now initializes that parser on the
owning native worker using a neutral discarded probe; no probe result or user
text is retained. In five alternating local direct-engine comparisons, the
first reported mixed sentence after voice construction reached its first PCM
callback at medians of 56.63 ms before and 31.13 ms after. The initialization
cost is moved into voice selection; it is not eliminated, and a caller that
speaks immediately may still wait for it. Later uncached requests and the
audio device have separate costs. Cancellation, dictionaries, restored voice
settings and speech output must still be tested independently.

## English-first prefixes and a unified DLL, 2026-11-16

Some English openings contain enough speech for the same early control-only
contour path even when the first segment is far shorter than 80 characters.
The planner estimates eligibility using the English phoneme count and active
speed or WPM; an inline speed command, a rapid setting or an insufficient
prefix keeps the complete-pilot route. The actual prefix is checked again
before audio is sent. English-first candidates require at least 200 ms of
planned audio, in addition to the existing 60 ms boundary reserve (250 ms for
an English-to-Persian /x/ transition); the previously enabled long-prefix
route retains its 300 ms requirement. Neither reserve guarantees against
dropouts on every machine.

For an alternating local comparison of the warmed, uncached engine at 16 kHz
and native speed 98, `NVDA نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از`
reached its first PCM callback at approximately 43 ms before and 11 ms
after. In the real transport with a fake player, four different English-first
sentences had median first-feed times of about 30 ms and 10 ms respectively.
These are local test-system measurements, not an end-to-end NVDA or device
latency guarantee. Full PCM and NVDA index positions were identical in the
comparison. The full contour still runs after playback starts; an injected
80-ms delay and cancellation are tested explicitly.

The acoustic probe already embeds the direct Persian text sources. The test
add-on now uses those exports from its acoustic DLL instead of loading a
second copy in `fa-direct.dll`. The artifact selector requires the acoustic
and embedded text sources to match the checked-out revision, and a dedicated
test checks the confirmed `alāve`, `alāmat` and other readings in that same
DLL. This removes a duplicate library load; it does not yet move the Python
language-switch and shared-contour planner into the native rules engine.

## Shared C projection and high-rate S listening candidate, 2026-11-16

The common ECI library now exposes request-local Persian text and source-event
projection through `eciPersianProjectUtf8` and `eciPersianRelease`. Its direct
rules and the selected English-to-Persian resonance bridge are portable C
sources used in the NVDA test path. The existing fallback still handles text
outside the direct rules. The remaining sentence planner and language routing
are not yet native, and Persian is not yet registered as an ECI, SAPI or
Android language. This is groundwork for those hosts, not a completed port.

For four repetitions of a local projection microbenchmark, medians for a
40-word job with source marks were approximately 0.51–0.53 ms in the previous
Python mapping and 0.22–0.23 ms with C mapping. This is not an NVDA or device
latency measurement and does not establish equal responsiveness to other
languages. The text and source marks are request-local; no utterance results
are retained. The C and former Python transition outputs are also compared
frame-for-frame and as completed speech.

At 32, 44.1 and 48 kHz, the high-rate S-like frication extension is reduced
and gently rolled off above the main band, but a high-frequency extension is
retained. The previous Castilian/Greek narrower mode and rates at or below
22.05 kHz are unchanged. On sustained /s/ frames from English `see`, German
`Kasse` and Persian `/sa/`, with two genders, a local PCM comparison found
changes between +0.03 and +0.08 dB below 5 kHz, about -2.6 to -2.7 dB in
8–11 kHz, and about -3.2 to -3.5 dB above 11 kHz. These bands are measurement
intervals, not a guarantee of perceived loudness or clarity. The existing
source classifier can also include S-like stop releases; the selected /f/
and vowel checks remain unchanged. Listening at the user's actual output
device is required before adopting the candidate.
