# Moving the US sentence melody into the Persian core

Status: groundwork only. Nothing in the voice has changed. This file records
what was measured and the plan, so the work can continue in later sessions.

## Goal

Persian keeps the accepted US-derived sentence melody, but the provider
produces it itself, syllable by syllable, the way it does for every other
language. The driver then only controls the voice: no second synthesis pass
and no melody planning in Python.

## What is measured today

First audio through the planner that the NVDA add-on uses (Linux, warm):

| Text | Persian | English |
|---|---|---|
| letter | 1.8 ms | 0.4 ms |
| word | 2.0 to 2.3 ms | 0.3 ms |
| sentence | 3.9 ms | 1.1 ms |
| long sentence | 5.9 ms | 1.7 ms |
| Persian with English words | 6.9 to 12.6 ms | 0.7 to 1.3 ms |
| first word after start | 14.4 ms | 0.9 ms |

The gap comes from rendering each sentence twice (the Persian target and a
silent US donor) and, for mixed text, from frame loops in Python. The pure C
two-pass entry point waits for the whole sentence: about 22 ms for a long
sentence and 220 ms for 100 seconds of unbroken speech.

How far the provider's own melody is from the accepted contour, measured by
`tools/persian_melody/compare_contours.py` over 30 sentences:

- all voiced frames: median 0.82 semitones apart, 90 % within 2.12, largest 7.69
- last 300 ms of each sentence: median 1.05, 90 % within 2.45
- 12.6 % of the frames are more than 2 semitones apart

Plain statements are already close (median about 0.4 to 0.9). Questions,
exclamations and comma phrases differ most (up to 5 to 8 semitones). The same
script is the acceptance measure for the native melody.

## Why a port is possible

The Finnish-based provider that Persian uses and the US-English provider come
from one rule family:

- 410 rules exist in both; 289 have the same number of lines.
- 75 intonation rules are shared. `insert_f0` (300 lines) differs in 8 lines,
  `f0_step` in 2.
- The language-specific tone rules pair up one to one: `fin_high_tone` and
  `eng_high_tone`, `fin_phrase_tone` and `eng_phrase_tone`, and so on.
- The record types have the same numbers (phone 2, word 4, phrase 5,
  syllable 7). The phrase record is identical.
- Global variables line up: every `s` variable has the same number except
  one (`s474` in English is `s483` here), `w` variables from 82 up are 12
  lower here, and `c` and `f` variables are identical.
  `tools/persian_melody/rosetta.py` derives this from the shared rules:
  436 variables and 202 authored symbols, with that single exception.

The English-only rules that produce the melody, with everything they call
that the Persian provider lacks, are 28 rules and about 7,800 lines:

`eng_high_tone`, `eng_low_tone`, the four starred and six nuclear tone rules,
`eng_boundary_tone`, `eng_phrase_tone`, `eng_phr_initial_F0`,
`eng_prenucl_f0`, `eng_postnucl_f0`, `eng_first_postnucl_accent`,
`find_eng_nucleus`, `adjust_eng_accents`, `eng_position_t_ptrs`,
`nucl_acc_t1pos`, `postnucl_high_tone`, the continuation-phrase rules and
`is_minor_phrase_break`. They call four shared rules that already exist here.

## Translator and its check

`tools/persian_melody/translate.py` rewrites an English rule into the Persian
provider's numbering: variables, authored constants, and the field and value
numbers of the records. Numbers are followed from the assignment of a local
to the call that uses it, across jumps, so a field number set in one block
and used in another is still rewritten.

The check translates the English text of every rule both providers share and
compares it with the Persian provider's own text. Of 244 shared rules of
equal length, 199 come out identical: 25,743 lines equal, 224 different. No
record call was left unresolved and no literal had two conflicting uses. The
remaining differences are the expected ones: the line that compares the
current language with the provider's own language, and calls that name the
`eng_` or `fin_` rule.

## The scope is larger than the 28 tone rules

Besides the language-specific tone rules, several shared melody rules differ
between the providers and are part of the US contour:

`adjust_accents` (996 lines here, 1,164 in English), `set_f0_range_ptrs`
(268 and 329), `phr_final_anno` (133 and 383), `post_comma_phrase`,
`assign_phr_final_anno_features`, `find_lang_potential_phrase`,
`first_postnucl_accent`, `postnucl_f0`, `nucleus_vals`,
`init_phrase_inton_vars`, `init_word_inton_vars`, `apply_comma_phr_rules`,
`phrase_level_rules`, `assign_user_stress` and `mark_word_stress`.

These, with what they call, have to be ported as English variants selected
under the same scoped key. The realistic total is 40 to 60 rules.

## First result: the melody generator runs inside the Persian provider

`tools/persian_melody/port.py` translates everything the English
`generate_intonation` reaches (66 rules, about 12,500 lines) and the build
adds it to the prepared Persian rule file. One call chooses between the two
melodies. `eciPersianNativeMelody(1)` selects the US rules for the process;
the default is unchanged and produces identical audio.

Measured with `compare_contours.py` against the accepted contour, 30 sentences:

| | provider's own melody | US rules in the provider |
|---|---|---|
| all frames, median | 0.82 st | 0.53 st |
| all frames, 90 % within | 2.12 st | 1.75 st |
| largest | 7.69 st | 4.35 st |
| last 300 ms, median | 1.05 st | 0.33 st |
| frames more than 2 st apart | 12.6 % | 7.2 % |

Sentence ends, questions and exclamations are now close. What remains is
where accents fall inside the sentence: differences of about 3 semitones on
single syllables in most sentences. That is decided before the melody
generator runs, by rules that are not ported yet:

- `process_annotation` and what it calls differ in 13 rules, among them
  `handle_SPR`, `emphasis_anno`, `word_accent_anno` and `phr_final_anno`.
  They turn the stress digits and the weak-word annotation of the phonetic
  input into word accents. The English versions also insert English phones,
  so they cannot be taken over whole.
- `phrase_level_rules` reaches 11 differing rules. Some assign accents
  (`apply_half_stress`, `reduce_accent_for_selected_words`); others are
  English sound rules (`word_final_flap`, `do_word_initial_creak`) that must
  not run on Persian phones.

A third group runs in the speech module just before the melody:
`init_word_vars` and `init_syll_vars`. They differ strongly between the
providers. Taking both over whole was tried and rejected: no sentence stayed
comparable, because they also feed the duration rules. Only the variables
the melody reads may come from the English versions.

The next step is to port the accent-assigning parts of these groups and
leave durations and phones on the Persian side.

With the US rules selected, the provider streams as it does for other
languages: first audio after about 1 ms, where the fitted two-pass contour
needs 5.6 ms for a short and 22.6 ms for a long sentence in the same C path.

## What still has to be solved

1. **Record fields.** The phone and word records order their fields
   differently (the English word record has four extra fields before
   `stress_level`; the phone record differs after `transition`). Field
   numbers appear as literals and are also passed around in locals, so each
   use has to be translated by field name, not by a blanket substitution.
2. **Field values.** Word categories and phone classes are numbered per
   language. English syntactic tests (`is_minor_phrase_break`, noun and
   preposition phrases) do not apply to Persian; in the accepted contour they
   run on a nonsense donor text, so they must behave as they do for words of
   unknown category.
3. **Authored constants.** The 28 rules use 16 English constants that have no
   counterpart here (scan fences and test strings). They have to be embedded,
   as the Brazilian R port in `experiments/persian/build_native_probe.py`
   already does for its donor.
4. **Inputs.** The accepted contour is fitted from a donor built by
   `src/reloq_persian_us_proxy.c`: its stress pattern, weak words and the
   artificial primary stress for all-unstressed words. The native rules must
   see the same stresses on the Persian records.
5. **Selection.** The Finnish dispatchers (`high_tone`, `phrase_tone`, …)
   choose the `fin_*` rule from a language variable. Under a new scoped key
   they would choose the ported rule instead; the default stays unchanged.

## Second step: the accent rules are ported too

`adjust_accents` and the five rules below it decide how high each accented
word may rise. They live in a different rule file, so the Persian provider
now prepares `delta_rules_c05_fifi.c` as well (23 prepared files). With them
the 30 sentences measure: median 0.53 st, 90 % within 1.69 st, largest
4.35 st, 6.8 % of the frames more than 2 st apart. The end of the sentence
is unchanged (median 0.33 st). The default output is bit-identical.

The remaining distance is in the accents before the last one: they stay
lower and flatter than in the accepted contour.

### Rejected: running the English setup rules

`init_word_vars` and `init_syll_vars` set per-word and per-syllable variables
and differ strongly between the providers. Three ways of letting the US
melody see the English values were tried and rejected:

1. Replacing the Persian rules: durations change, nothing stays comparable.
2. Running the English rules on a private copy of the variables they write,
   carried from word to word: crash in `valid_f0_posn`, reached from
   `eng_phr_initial_F0`.
3. The same, starting each time from the Persian values: empty output.

The setup rules evidently do more than write those variables. The next step
looks at what the ported tone rules read, one value at a time, instead.

## Third step: word stress on the English scale

The flat accents had one cause. The two providers count word stress
differently:

| level | English | Persian provider |
|---|---|---|
| stressed word | 3 | 1 |
| word with the sentence accent | 5 | 2 |
| emphasis | 6, 7 | 3, 4 |

The US rules compare against the English numbers, so every Persian word
looked nearly unstressed to them. The levels live in four variables
(`s236`, `s237`, `s242`, `s243`) and in the word records. While the US rules
run, the variables hold the English numbers and the word-stress field is
read and written through a view (`delta_field_view` in the rule machine,
unset by default). The records keep the Persian numbers, so durations do not
change.

The translator also left field writes (`mark_s`, `mark_v`) untranslated; they
are translated now.

Result over the 30 sentences: median 0.18 st, 90 % within 0.84 st, 2.0 % of
the frames more than 2 st apart (before: 0.53 st, 1.69 st, 6.8 %). The
default output is bit-identical. What remains is in questions and directly
after commas.

## Stages

1. Translate the 28 rules mechanically (rule prefix, variables, symbols),
   then resolve fields, values and constants rule by rule. Compile them into
   the Persian provider behind a scoped key that nothing selects.
2. Measure with `compare_contours.py` against the accepted contour. Target
   before any listening: median under 0.25 semitones and no frame more than
   2 semitones apart in the 30 sentences, then a larger set.
3. Offer it as another listening variant in the NVDA test add-on.
4. Only after listeners accept it: make it the path the voice uses, register
   Persian and Pinglish as ordinary dialects, and remove the melody planning
   from the driver. Mixed Persian and English text and source indexes need
   their own stage; they are not solved by the melody alone.

## Risks

- The rules are machine-recovered code. A wrong field or value number does
  not fail to compile; it produces a slightly wrong melody. Every stage is
  therefore checked by measurement, not by reading.
- The result may be very close rather than identical. Remaining differences
  will be named with the sentences that show them.
