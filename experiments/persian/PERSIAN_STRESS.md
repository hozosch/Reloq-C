# Persian stress and contour

The dev143 NVDA experiment uses the native US English contour as its pitch
pilot, mapped to the existing Persian frame timing and vowel sound. It maps
the end of a word's voiced portion to the donor's voiced end instead of
dropping the final vowel's pitch movement. A Korean donor is not selected.
Pitch and inflection controls continue to apply. Some unsupported input or
planning failures can still fall back to the previous Finnish path; this is
an experiment to be judged by listeners, not a claimed Persian model.

For word prominence, Iranian Persian generally accents the final syllable
of nouns, adjectives, most adverbs and nominal derivational/inflectional
suffixes. Verbal prefixes such as mi-, na- and be- may carry the accent;
person endings on an unprefixed verb and nominal possessive clitics do not
attract it. Compound verbs and information structure also affect phrase
prominence. The following primary studies explain these distinctions:

* https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/timing-of-prenuclear-pitch-accents-in-persian/C7CA7887F69ACE368ED7126741F695C5
* https://www.isca-archive.org/tal_2018/rahmani18_tal.pdf
* https://www.isca-archive.org/speechprosody_2010/zadeh10_speechprosody.pdf
* https://uu.diva-portal.org/smash/get/diva2:641228/FULLTEXT01.pdf

The native implementation uses Lilak's word-class labels, not its
pronunciations, to apply final accent to an unambiguous lexical word. It
recognizes selected verbal prefixes and person endings only when a verb
analysis can be supported by the word list and a known stem. Explicit ezafe
and previously admitted possessive forms retain the accent of the stem.
For a separately written mi-/nemi- prefix, the native sentence path checks
that the following word is a recognized verb before moving the accent to the
prefix. Numeric groups receive final lexical prominence while the existing
number-phrase weakening is retained. Previously generated character names
are updated only when their phones match a known written name.
Pronunciation tags with multiple separately analyzed segments and words
with conflicting word-class labels retain their previous treatment.

The common conjunction va and the usual complementizer ke are weak before
a following word; the object marker ra is weak after its host, including at
the end of a phrase. A native weak-word control also prevents those unaccented
words from automatically attracting the phrase accent. Citation forms and
punctuation boundaries are retained. The computer term اسلش is accented on
its final syllable in both written-word and slash-character readings.
Several prepositions and ya already have context-sensitive readings. Free
personal and demonstrative pronouns can carry their own lexical accent; the
provider remains responsible for phrase-level pitch and emphasis. Mixed
Persian/English segments and the word-level reference fallback retain these
bounded context rules without introducing extra audible words.

Unvowelled Persian spelling and context can leave grammatical analyses
ambiguous. The current bounded analyzer cannot determine compound verbs,
every clitic, or the intended reading of every homograph. In particular,
this is not a claim that US English intonation is a scientific Persian prosody
model, nor that every Persian word or fallback pronunciation is resolved.
Native-speaker listening remains necessary for the resulting synthesis.
