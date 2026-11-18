# Persian stress and contour

## dev150: sentence clocks, numbers and plural endings

The provider's syllable clock can restart after sentence punctuation. The
trace now adds the actual elapsed synthesis clock at a reset, giving both
the Persian carrier and the US donor utterance-relative boundaries. If the
clock or a boundary cannot be verified, the optional trace is rejected and
the existing word-level transfer is used instead. This allows syllable-level
alignment in later sentences, including a number after a colon.

The US pilot now preserves consonants in each verified phonetic word, even
when the US phoneme display legitimately changes an intervocalic d or t to a
flap. In particular, ۹۶ retains its long final /ʃ/ onset in the pilot. The
unchanged Persian audio still supplies the sound; only its pitch transfer is
guided by this silent donor. Unusable pilot words fall back individually to
safe syllables, and multisyllabic unaccented pilot tags are made acceptable
to the US parser without changing the Persian phonemes or lexical stress.

Sentence colons before spaced slashes are treated as prose boundaries even
in mixed-language text. Contiguous numeric times, URL schemes and drive
paths retain their existing interpretation, as do standalone symbol names.
When NVDA announces a non-spaced prose colon but drops the punctuation, a
verified guard retains its configured spoken name and adds the missing
sentence boundary. Clock and address colons, custom symbol rules that would
change under the guard, character navigation and speech dictionaries are
left to NVDA.
NVDA normally removes the Persian question mark at silent punctuation
levels. In ordinary Persian speech, a guarded question boundary now survives
that preprocessing so the native core can request the question contour. A
character-navigation request, an explicitly spoken or custom symbol, and a
matching speech dictionary retain their normal NVDA behavior.

For directly recognized half-space plural forms ending in ‌ها, ‌های or
‌هایی, the native lexicon places the primary accent on -hâ when the expected
pronunciation matches and no verb classification contradicts it. The ezafe
or indefinite -i remains unstressed; unsupported readings remain unchanged.
Persian grammar and stress references for these suffixes:
https://assets.cambridge.org/97805210/91244/excerpt/9780521091244_excerpt.pdf
and https://zaspil.leibniz-zas.de/article/download/70/69.

## dev149: observed syllable timing and compound nominals

The US pitch pilot now records the native syllable boundaries of both the
Persian acoustic carrier and the US donor. Within a word, corresponding
observed syllable boundaries anchor the transfer rather than stretching the
whole voiced word uniformly. Other acoustic controls and the original
Persian time axis remain untouched. Sentence-local clock resets can be
resolved when independently captured word marks and all syllable counts
agree. For a different count, an unvoiced anchor, or another ambiguous
boundary, the previous word-level transfer remains in use. The result is a listening
candidate, not proof of natural Iranian Persian intonation.

For an existing, directly recognized compound nominal ending in ‌اندازی,
‌سازی, ‌یابی or ‌نویسی, native lexical stress now falls on the final
derivational syllable. A separately marked ezafe remains unstressed, and a
known verbal classification is not overwritten. Thus راه‌اندازی no longer
receives three primary accents. The rule does not assign this analysis to
unknown fallback words or to separately written words. The independent
motivation for final stress in Persian nouns, including compounds, is
documented at https://sites.la.utexas.edu/persian_online_resources/nouns/.
ZWNJ can mark a compound boundary, but is not an infallible grammatical
classifier: https://aclanthology.org/2020.coling-main.406/.

The dev145 NVDA experiment uses the native US English contour as its pitch
pilot, mapped to the existing Persian frame timing and vowel sound. It maps
the end of a word's voiced portion to the donor's voiced end instead of
dropping the final vowel's pitch movement. A Korean donor is not selected.
Pitch and inflection controls continue to apply. A stale stop on the English
handle is cleared safely before planning a new request, and unavailable word
boundaries use the US phrase contour instead of discarding the donor. An
emergency voiced US proxy handles an otherwise unusable donor. Certain raw
controls and unrecoverable native failures can still use the old path; this
is an experiment to be judged by listeners, not a claimed Persian model.

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

An explicit native neutral-context list covers va, ya, ke, az, be, ba, dar,
ta, and joz before a following word. The object marker ra is weak after its
host, including at the end of a phrase. These categories, the possibility of
focus, and contrasting stressed pronouns are discussed in Hosseini (2012):
https://uu.diva-portal.org/smash/get/diva2:641228/FULLTEXT01.pdf
The policy is separate from the reference-derived phone table. It also works
for the corresponding Pinglish aliases and across an English host boundary.
A native weak-word control prevents those deliberately unaccented words from
automatically attracting the phrase accent. An all-zero reference tag alone
does not force arbitrary content words to be weak. Citation forms and
punctuation boundaries are retained. The computer term اسلش is accented on
its final syllable in both written-word and slash-character readings.
Adjectives ending in -i retain final stress when Lilak classifies the word
only as an adjective (or adjective and adverb). Words also classified as
nouns are not reinterpreted, since -i can be an unstressed indefinite clitic.
The ambiguous spelling تو can be an independent pronoun or a preposition and
is not unconditionally weakened. Free personal and demonstrative pronouns
can carry their own lexical accent; the
provider remains responsible for phrase-level pitch and emphasis. Mixed
Persian/English segments and the word-level reference fallback retain these
bounded context rules without introducing extra audible words.

In dev148, written است / Pinglish ast is treated as an unstressed copula
after a preceding host in an ordinary phrase, including when the host is an
English segment. Its isolated citation form and a form after punctuation
retain their accent. This is a neutral-context approximation: information
structure or exceptional syntax can require prominence, and a general
syntactic parser is not yet present. Independent work on Persian accent
illustrates the unaccented copula after the lexical predicate:
https://pmc.ncbi.nlm.nih.gov/articles/PMC4671725/

The current duration rules remain unchanged. Persian stress can affect
syllable duration, but a controlled study does not specify a safe universal
multiplier for this synthesizer: https://pubmed.ncbi.nlm.nih.gov/28112017/.

Unvowelled Persian spelling and context can leave grammatical analyses
ambiguous. The current bounded analyzer cannot determine compound verbs,
every clitic, or the intended reading of every homograph. In particular,
this is not a claim that US English intonation is a scientific Persian prosody
model, nor that every Persian word or fallback pronunciation is resolved.
Native-speaker listening remains necessary for the resulting synthesis.
