# Persian stress and contour

The Persian voice uses the existing Finnish-based native contour. A Korean
donor exported by the native library is not selected by the Persian driver.
The user's pitch and inflection controls continue to apply.

For word prominence, Iranian Persian generally accents the final syllable
of nouns, adjectives, most adverbs and nominal derivational/inflectional
suffixes. Verbal prefixes such as mi-, na- and be- may carry the accent;
person endings on an unprefixed verb and nominal possessive clitics do not
attract it. Compound verbs and information structure also affect phrase
prominence. The following primary studies explain these distinctions:

* https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/timing-of-prenuclear-pitch-accents-in-persian/C7CA7887F69ACE368ED7126741F695C5
* https://www.isca-archive.org/tal_2018/rahmani18_tal.pdf
* https://www.isca-archive.org/speechprosody_2010/zadeh10_speechprosody.pdf

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

Unvowelled Persian spelling and context can leave grammatical analyses
ambiguous. The current bounded analyzer cannot determine compound verbs,
every clitic, or the intended reading of every homograph. In particular,
this is not a claim that Finnish intonation is a scientific Persian prosody
model, nor that every Persian word or fallback pronunciation is resolved.
Native-speaker listening remains necessary for the resulting synthesis.
