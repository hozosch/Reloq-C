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
