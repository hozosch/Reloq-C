# Persian prosody and timing: research notes

## What the research supports

- The last syllable is a useful default for Persian nouns, adjectives and
  many adverbs, not a rule for every orthographic word. Nominal inflection
  such as plural -hā can attract stress, while possessive enclitics generally
  do not. In verbs, prefixes such as mi- and the negative prefix can attract
  stress; personal endings do not automatically become stressed. Complex
  predicates have additional phrase-level behavior. See [Sadeghi,
  The timing of pre-nuclear pitch accents in Persian](https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/timing-of-prenuclear-pitch-accents-in-persian/C7CA7887F69ACE368ED7126741F695C5)
  and [Sadat-Tehrani, The Intonation Patterns of Interrogatives in Persian](https://journals.dartmouth.edu/journals/xmlpage/1/article/389?htmlOnce=yes).
- Lexical prominence and sentence intonation are related but distinct.
  In an experiment, lexical stress affected duration even in words without
  a pitch accent. Intensity and spectral tilt showed different behavior:
  [Sadeghi, Word-level prominence in Persian](https://pubmed.ncbi.nlm.nih.gov/28112017/).
  Other analyses emphasize pitch; the phonological classification remains
  debated. Do not assume a fixed, language-wide amplitude boost.
- One account based on 2,112 read utterances by eight speakers identifies
  accentual phrases and intonational phrases. Its pitch rises are aligned
  with stressed syllables, with different peak alignment for nuclear and
  pre-nuclear accents: [Sadat-Tehrani, The structure of Persian intonation](https://www.isca-archive.org/speechprosody_2008/sadattehrani08_speechprosody.html).
  Questions need context-sensitive treatment: polar questions in the
  interrogatives study typically end high, whereas wh-questions can fall.
- Measured vowel durations vary with context, rate and prosodic position.
  A small pilot study of two Tehrani speakers found that average durations
  did not simply follow the traditional short/long vowel pairings:
  [Aronow et al., A pilot acoustic study of Modern Persian vowels](https://journals.linguisticsociety.org/proceedings/index.php/PLSA/article/view/4059).
  These observations are not a timing table for a fast screen reader.
- Consonant length and the duration of adjacent transitions affect how
  speakers perceive segments, and interact with speaking rate. A production
  and perception study also distinguishes singleton and geminate consonants:
  [Hansen & Myers, The consonant length contrast in Persian](https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/consonant-length-contrast-in-persian-production-and-perception/4B0976552F4619BDA4030813FB8202E3).

## Findings in this project

The dev124 "no word stress" listening experiment rewrote every generated
syllable's marker to `.0`, which marks every syllable as unstressed. It did
not leave the choice of stress to the synthesizer. Explicit phoneme tags from
the user remained unchanged. At 16 kHz, Reed, speed 98, a coarse voiced-vowel
frame classifier counts 53 ms for the vowel portion of `[.1pA]` and 41 ms
for `[.0pA]`; these are within-engine proxy measurements, not manually
segmented human phoneme boundaries.

Removing those stress markers therefore changes timing as well as intonation.
The effect is separate from the US-consonant changes: for the same `.1tA`
input and settings, the number of frames before that vowel proxy increased
from 27 ms in dev121 to 35 ms in dev123, while its proxy duration stayed at
53 ms. In `.1mA`, the corresponding pre-vowel interval grew from 32 to
45 ms while the proxy changed from 45 to 47 ms. These are simple, controlled
examples, not a claim about every word or a universal vowel/consonant ratio.

The selected native rules additionally shortened short unstressed internal
vowels by 10%, and shortened selected long vowels before ordinary consonants
by 13% as a US timing experiment. Neither reduction is a measured Persian
duration law.

## Candidate and next steps

The dev125 listening candidate restores the frontend's existing stress
marks so that duration can be compared independently of the all-unstressed
experiment. This baseline is not presented as a correct Persian stress
model: for example, its stored pronunciation of کتاب is first-stressed,
whereas a final-stress noun is the research-backed comparison. The candidate
also disables the two timing reductions above and extends vowel-nucleus
targets by 20%, retaining native stress, phrase and transition mechanisms.
The 20% is a deliberately audible experiment, not a scientific constant.

For future implementation, separate (1) pronunciation and syllabification,
(2) morphology-aware lexical stress, (3) phrase accents and their focus or
question context, and (4) segment and transition durations. Test nouns,
plural and possessive endings, prefixed and negative verbs, complex
predicates, punctuation, connected mixed-language text, fast and slow rates,
and explicit ECI phonetic controls independently. Confirm representative
pronunciations with native speakers before treating heuristics as defaults.

## Follow-up listening candidate: dev126

Feedback on dev125 found the overall vowel length improved but occasionally
stiff, with the vowel of yek (one) too long, and the front vowel after /x/
too open. The new candidate retains the opening before /x/, removes the
extra opening after /x/, and keeps a bounded duration extension with a
smaller boost for the /j e k/ word-final sequence. The /k/ articulation and
its release retain their existing US-derived rules. The bound and local factor
are listening hypotheses, not measurements of a Persian duration law.

The short /o/ nucleus and its source target are aligned to the selected
pronunciation of دو across consonant contexts. The transitions into and out
of the vowel continue to be generated by the surrounding sounds; the other
vowels retain their own vowel identities.

The separate dev126 contour recording compares the existing pronunciation at
two inflection settings, with the median voiced F0 of the quieter-contour
sample matched to the first. Each pair has identical segment-control frames,
timing, and syllable stress markers; only the F0 curve changes. This makes an
audible, controlled comparison but does not establish that either contour is
a correct Persian model. The experimental contour is not enabled in the
test add-on. Native speaker feedback, lexical morphology, focus and question
type are still needed before designing a more realistic default.

## Follow-up listening candidate: dev127

The listener requested the timbre of the British voice's vowel in `sad` for
the Persian front vowel in all contexts. The candidate uses the donor's
F1/F2/F3 centre targets and inherited upper formants and bandwidths, while
retaining Persian duration, lexical stress and consonant transitions. This
is a requested timbral comparison, not a claim that British /æ/ is an exact
phonetic model for every Persian speaker. Compare the rendered spectra at
matched F0 and test different voices, head sizes and neighbouring phones.

The earlier audio comparisons explicitly used native speed 98, independent
of the listener's NVDA setting. New spoken comparisons default to 60 and
retain an adjustable command-line speed. They separate a previous/new front
vowel comparison from the previously proposed paired F0-contour comparison.
Both comparisons can be generated with German or English spoken labels; the
Persian samples and comparison conditions stay the same.

## Follow-up listening candidate: dev128

At the listener's request, the British-derived front vowel receives a local
two-percent increase of its positive formant targets, similar to reducing the
donor's head size by about four points around the default setting. Native
voice-dependent transforms and frequency ceilings still apply. No global
head-size, duration, or source controls are changed; the direction is a
listening hypothesis rather than a measurement of a correct Persian vowel.

The same eSpeak build produces first-syllable stress for `کتاب` and `خانه`,
and middle-syllable stress for `مدرسه`, which is also how the current
generated tags for those individual words mark them. This is an observation
about the two implementations, not evidence that these stress positions are
always linguistically correct. A Persian intonation model must still account
for the alignment of phrase-level rises with prominence, morphology, focus,
and question type, rather than assuming that a different contour corrects
the lexical stress automatically.

The original Korean listening comparison captures actual Korean-core pitch
curves, stretches each one to the duration of the corresponding Persian
utterance, and matches its median to the Persian baseline. It changes only F0
in a replay of the same Persian frames. The transplant is not aligned by word
or syllable, and it was neither a Persian prosody model nor enabled in the
voice at that stage. Each example includes statement/question punctuation as appropriate.

For the longer comparison the Persian passage is captured continuously with
sentence indexes. The corresponding Korean sentences are synthesized
individually because that provider can emit intermediate ECI indexes only
at the end of its utterance. Each donor F0 curve is mapped to the voiced span
of its Persian sentence, then normalized over the whole passage. Captured
non-F0 controls and all frame timings are checked for exact identity. This
avoids drifting across sentence boundaries, but still does not claim
syllable-level alignment or preserve cross-sentence Korean prosodic context.

Following the native-speaker preference for that sample, dev131 uses a
request-local Korean donor for ordinary Persian and Pinglish speech. Because
the sample's hand-translated Korean sentences cannot be supplied for
arbitrary text, the donor is a Hangul syllable-count proxy with phrase
punctuation. Its contour is an approximation to the reviewed sample, not
the same seven stored curves. The native C transfer changes F0 only and
normalizes median pitch; the NVDA adapter still owns proxy generation,
language segmentation and capture/replay. English word interiors use their
own native pitch; only actual switch edges are fit to Persian. Inflection
remains controlled by the user's setting. Inline controls that cannot be
safely planned retain the earlier route. The candidate needs new listening
feedback for arbitrary text, questions and mixed-language switches.

The separate eSpeak-F0 comparison estimates periodic pitch from the eSpeak
waveform, interpolates through unvoiced gaps, aligns the two engines at word
boundaries, and matches median F0. It leaves the Persian frames' sounds,
stress markers, and durations intact. The estimate and within-word timing
are approximate; do not treat it as a direct eSpeak prosody implementation.
The word-stress comparison instead changes the markers for a small set of
noun, plural and prefixed-verb examples. This can also change native syllable
durations. These are listening alternatives for native-speaker review, not
rules enabled in the shipping voice.
