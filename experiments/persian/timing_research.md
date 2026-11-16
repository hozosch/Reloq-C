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
word/rate comparisons were excluded. This is not a measurement of NVDA's
eSpeak 1.51 or a human-speech corpus. At native speed 98 and eSpeak 175 WPM,
the first-word-before-another-word / isolated-word ratios were:

| Vowel and word | Native nucleus target | eSpeak event interval |
| --- | ---: | ---: |
| back A in `سلام` | 0.669 | 0.596 |
| back A in `ایران` | 0.821 | 0.821 |
| final E in `نقطه` | 0.903 | 0.614 |

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

The diagnostic scope differed from the production scope only in its trace
selector. With tracing disabled, 72 combinations of mixed Persian/English
texts, rates and voices produced bit-identical PCM and source indexes.
