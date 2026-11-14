# Persian listening probe: L and vowel-duration notes

Status: research and test plan only. The combined L rule and revised vowel
duration are not implemented yet. The existing binary remains a private
Finnish-based acoustic experiment, not a selectable Persian language.

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

## Evidence

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

3. Sadeghi et al., “The phonetic realization of word stress in Persian”,
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
