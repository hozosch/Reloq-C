Current release: **0.2.186**. Dictionary management is under NVDA Settings -> Eloquence C. Selected own .dic files reload automatically; dictionary refresh uses current master. OK/Apply save, Cancel discards unapplied edits. See `../reference-apple/behavior-0.2.186/README-186.txt` for scope and reproducible checks.

# x64 NVDA synthesizer and exact listening preview

## Selectable experimental synthesizer

The separate `synth/` package registers **Eloquence C (experimentell,
Deutsch)** as an NVDA synthesizer. Build it from a C DLL with the opt-in
generic ECI voice mode:

Building the add-on also requires GNU gettext's `msgfmt`. English UI strings and
the German gettext catalog are included; see `TRANSLATING.md` for other locales.

```sh
make -C eci windows-x64
python3 addon/build_synth.py
python3 -B addon/test_synth.py
```

The package is `dist/reloq-c-synth-experimental-x64-0.2.150.nvda-addon`.
See `../ARBEITSSTAND-0.2.150-DE.md` for current changes and validation.
Install it in a test NVDA profile, restart NVDA, then select the synthesizer
in NVDA's speech settings. It does not require the preview's keyboard
shortcut. The experimental C voice attempts general German prose using the
generic mode (`eciSetParam` local extension `0x1003=1`); exact preview mode
remains unchanged. Natural speech still differs from the reference.
The following version notes are historical.
Version 0.2.26 uses a shared p/t/k/b/d/g phase planner across word and
syllable boundaries, including reduced endings, connected stop releases and
additional nucleus-budget transitions. See the 0.2.26 behavior report for
12,888 component checks and the measured whole-text differences. Lexical
analysis and phrase prominence are unchanged; full equivalence remains open.

Version 0.2.25 extends cluster timing/source gestures to S+stop, fricative+nasal,
consonant+j and X+r/l. Word-boundary r receives measured excitation phases.
Observed grammatical classes now distinguish weak words, deaccented lexical
stress and phrase nuclei, including multi-syllable function words. The packaged
lexicon has 16,105 entries after 184 verified verb-form additions. The special
long-oe+r timbre and total 8 dB source trim are unchanged. Validation uses the
actual C engine and add-on driver with a simulated audio player; it is not a
Windows/NVDA listening test. See `reference-apple/behavior_0_2_25_findings.md`.

Version 0.2.24 adds shared consonant-r/l cluster gestures, including stress,
word-internal position and preceding sh. It corrects acoustic vocalic-r
classification and coda r/l transitions, including nasal bandwidths through
the vowel-r nucleus. The intentional long-oe+r timbre has a further 4 dB
excitation reduction (8 dB total). See
`reference-apple/behavior_0_2_24_findings.md` for measured improvements and
remaining whole-sentence differences. Productive word analysis is unchanged.

Version 0.2.23 models stop closure, burst, frication and following-j gestures
separately for pf, z, tsch and dsch. It corrects supported phrase-final p/t/k
releases and connected diphthong-to-fricative timing. Initial written dsch
uses the observed voiced affricate; the 15,921-entry dictionary is unchanged.
This remains an incomplete voice and word analyzer; see
`reference-apple/behavior_0_2_23_findings.md` for comparisons and regressions.

Version 0.2.16 adds sparse voiced/aspiration contours, connected nasal resonance,
and shared vowel-r/l timing from inspected reference behavior. Productive
inflection retries verified dictionary stems before the fallback letter rules;
exact dictionary words keep priority. There is no blanket gain increase.
In 150 controlled contexts, all nine compared source channels agree in 73 cases
(previously 27). This is not a full-control or audible-equivalence result.
Normal-text duration and spectral averages improve slightly while level error
worsens. See `reference-apple/SOURCE-INFLECTION-0.2.16-DE.md` for complete limits.

Version 0.2.14 activates the independently authored phrase/context duration
rules in supported vowel nuclei. Vowel plateaus, CV/VC transitions and internal
diphthong movement share one budget, including right-owned transitions after
fricative/stop clusters. The 62-sentence normal-text comparison exercises
626 scheduled nuclei and changes all 62 audio outputs. Mean absolute duration
error falls from 152.3 to 141.9 ms, with individual regressions. Unsupported
vowel-r/l nuclei and transition profiles retain their previous geometry.
This release changes timing, not productive word analysis or the dictionary.
See `reference-apple/NUCLEUS-SCHEDULE-0.2.14-DE.md` for verification and limits.

Version 0.2.13 added the independently authored nucleus-acoustics rules for
17 vowel/vocalic-R classes, including stress-sensitive I, entering/departing
resonances and nasal bandwidths. The C scalar model matches all 26,840
applicable inspected contexts after development corrections; 241 unresolved
or out-of-scope contexts are reported separately. Formant gestures now have
their own tracks so source knots cannot erase their movement. The unchanged
15,921-entry dictionary still feeds an incomplete word analyzer. Phonological
source targets are verified upstream; emitted source envelopes, full duration,
nucleus topology and prosody remain incomplete. The normal-text diagnostic
and the listening comparison use ordinary original text, not original SPR
resynthesis. See `reference-apple/NUCLEUS-ACOUSTICS-0.2.13-DE.md`.

The only available language is German, the only sample rate is 11,025 Hz,
and the voice selector has one fixed choice. NVDA rate 0–100 maps to ECI
speed 40–156; Rate Boost reaches up to ECI 250. Pitch, inflection and volume
use the measured native voice scales 0–100. Inflection defaults to 30;
volume zero remains faintly audible. These controls apply only to the
experimental generic voice. Head size, roughness, breathiness, multiple
presets, pause policies and language switching are absent.

When NVDA emits a
`PitchCommand`, for example to raise the pitch while spelling a capital
letter, the driver converts it relative to the configured pitch into a native
inline ECI pitch mark. The C voice applies that mark at the following word or
letter; NVDA's reset command restores the selected pitch. `CharacterModeCommand`
keeps spelled letters separate in the same utterance. This is limited pitch
control, not full inline prosody: rate, volume, language and arbitrary markup
commands within an utterance are not supported.

The queue handles playback-gated indices and cooperative native cancel.
Each NVDA speech sequence, including any interior index marks, stays one
ECI job, so marks do not restart its sentence prosody. Native frame callbacks
are combined into audio buffers; after each utterance the player drains and
reports completion exactly once, as in OpenEVV. A 5-second watchdog releases
an unpaused stalled player. The player stops before native ECI cancellation.
Text is never split at an artificial length limit.
The C voice retains its punctuation pause at the end of a native ECI job, so a
second NVDA speech chunk follows with about the same gap as when both
sentences were synthesized in one job. The driver adds no separate delay.
For 0.2.6, the German text frontend also composes verified `ab-`, `auf-` and
`aus-` verb prefixes. On a disjoint 952-word pronunciation holdout, exact
matches to the public Apple phoneme output rose from 318 to 340 with no
previously exact form lost. That release merged 1,337 new, context-checked
pronunciation entries with the original 1,205, yielding 2,542 entries. The
supplement contains no holdout words. On the local Linux machine, median ECI
handle initialization including the larger lexicon was 0.861 ms slower; the
measured first-PCM time after initialization was essentially unchanged. These
are not Windows or NVDA latency measurements.

The C voice also sets the first F1 target of a `/v/` plus vowel transition to
the separately observed 300 Hz value, leaving `/f/` alone. Before a nasal in
the same word, even across a syllable boundary, the preceding oral vowel now
anticipates the wider B1 bandwidth; the nasal `/n/` FNZ gesture rises during
the nasal segment. Those target-level changes have limited controlled
comparisons, and an audible improvement in normal NVDA speech remains to be
checked. They do not establish a general bass or transition match.

Version 0.2.8 adds a general German separable-particle + `zu` + conservatively
analyzed infinitive rule for 14 particles. Its five most recent particles
changed 29 previously unlexiconed pronunciations in 13,280 inspected forms;
all 29 matched the public Apple phoneme callback, including sentence context.
The independent family folds 13–15 had three exact gains and no observed
phonetic regression. A productive `-los` suffix rule lengthens the final vowel
in the fallback; on 2,960 fresh unlexiconed words, it changed 12 readings,
made six whole word groups exact and caused no observed phoneme regression.
The generic acoustic target streams now route tract,
voiced and noise stages independently with additional point capacity; this
structural change alone is sample-identical on 40 sentence/SPR checks. A new
low-F0 realization applies when the input already contains explicit `0`/`00`
markers. The current natural-text frontend rarely emits those controls, so
this release does not claim a broad consonant or prosody improvement.

Version 0.2.9 repairs whole-utterance silence caused by previously unknown
CP1252 symbols, including section signs, degree signs, superscript digits and
backslashes. Common symbols receive spoken German names; the remaining valid
symbols receive a spoken generic name. An unrecognized word no longer blocks
all the following text: a conservative German compound recovery first combines
two independently renderable constituents into one SPR word, including a
guarded linking-s class; only an unresolved word is spelled. The supplied
`Tonqualität` and `Lebensqualität` repros now produce PCM and completion marks
as whole words. This is a coverage fix, not an Apple-exact pronunciation claim.
The player watchdog also records and clears a stalled output rather than
marking discarded audio as successfully played. The independent ECI sentence
matrix covers 122 complete texts and 8 Unicode admissions; 112 previously
speaking sentences remained bit-identical in the paired comparison.

Version 0.2.12 adds 13,379 original-dictionary candidate pronunciations,
resolved through fresh public Eloquence phoneme responses in isolation and
two contexts. The package contains 15,921 exact-case entries and retains all
2,542 previous entries. Each passes native word/text lookup and ECI synthesis
with audio and its completion index. Eighteen of 58 sentence PCM outputs
change against the old dictionary using identical C rules; no formerly exact
phoneme-group sequence is lost in that comparison. A shared C storage pool
replaces per-word allocations. These results establish dictionary integration,
not complete original word analysis or acoustic identity. The disassembly and
checked word-analysis traces are documented in
`reference-apple/WORD-ANALYSIS-BEHAVIOR-DE.md`.

Version 0.2.11 adds independently authored German CV/VC timing rules, based
on behavior explained through disassembly and separately checked inputs.
The model matches 6,153 applicable original intervals, including 684 fresh
controls. Seventy-four sentence outputs match the audited candidate after
integration. Across their 191,545 frames, only F1/F2/F3 change; length, word
indices and all 59 remaining controls stay equal to the earlier C voice.
Acoustic measurements are mixed, with a recurring regression in sonorant
CV middle-window spectra. This is an experimental listening release of a
broader timing model; full acoustic equivalence remains unfinished. See
`reference-apple/TRANSITION-DURATION-HANDOVER-DE.md`.

Version 0.2.10 adds a German `x/X` (ch) to vowel gesture in the same syllable.
The tract and voiced sources now hand over at their own boundary while the
frication ends on its existing seven-millisecond timeline. In three separate
natural-sentence cohorts, all five affected transitions improved relative to
the public Apple reference in level and six-band spectral shape. All 52
word-initial p/k/t to vowel controls, sample lengths and word indices remained
unchanged. A stop followed by `r` in the same word now carries more source
energy into `r`. The same three cohorts contain 23 such transitions and show
lower median level and spectral-form error in each cohort. Independent tract,
voice and noise origins are now available to the target planner; that
infrastructure alone leaves previous PCM unchanged. These are experimental
transition changes, not a general Eloquence match.

The regular add-on alone includes the merged optional pronunciation lexicon and
a Unicode admission helper; CP1252 text is unchanged except backticks (which
OpenEVV also treats as spaces by default), while other scripts
receive spoken names/placeholders instead of dropping a whole utterance.
The bundled x64 DLL works in an x64 or ARM64EC NVDA process; the Windows loader
rejects pure ARM64 and 32-bit. Actual Windows hearing remains to be checked.

## Manual exact preview

The add-on under `preview/` is a **global plugin** for the separate,
reference-matched exact voice. That narrow mode does not accept general
navigation text. The preview leaves the user's configured synthesizer in
charge and plays only text that exact native ECI preflight accepts. Failure
is announced by NVDA's existing voice. For regular navigation speech, install
the experimental selectable synthesizer above instead.

Build the current pure C Windows x64 DLL and package it:

```sh
make -C eci windows-x64
python3 addon/build_preview.py
python3 -B addon/test_preview.py
```

The package is `dist/reloq-c-preview-x64-0.1.5.nvda-addon`. It includes only
`eci.dll`, the two authored Python ECI modules from `nvda/`, the global plugin,
the manifest, and documentation. It never bundles Apple/OpenEVV code, data,
captured targets, or audio files.

Install the `.nvda-addon` in a test NVDA profile via **Tools → Add-on Store →
Install from external source** or by opening it from Explorer while NVDA is
running. Confirm and restart NVDA. The original synthesizer stays selected.
Use **NVDA+Control+Shift+F11** to hear the pre-filled, naturally written
multisyllabic sequence `Pappe. Packen. Tappen.`, or type another verified
example: `Packte. Packten. Tippte. Kippte.`, `Kette.`, `Pack Tipp.`,
`Pack Pack.`, `Papp Tipp.`, `Pack Tipp. Hack Tipp.`,
`Pass.`, `Tiss.`, `Kuss.`, `Bett.`, `Deck.`, `Guck.`, `Bad.`,
`Mett.`, `Mut.`, `Nock.`, `Not.`, or `Hieb.`
The native ECI path also treats a final bare word such as `Mett` or `Bett`
as period-final; there is no NVDA-side punctuation injection. If a form is
unsupported, the preview reports whether the written word or its acoustic
model is missing, along with the character position when known.
Use **NVDA+Control+Shift+F10** to cancel preview speech.
The text entry dialog and unsupported-text messages use the normal selected
NVDA synthesizer. The ECI output format is mono PCM16 at 11,025 Hz and its text
encoding is CP1252. Plain-language text beyond the covered phonetic forms in
this exact mode, other languages, and other sample rates remain to be
implemented. A word-boundary ECI index works for the measured
connected pairs; indices in unmodeled contexts are rejected before playback.

The manifest requires NVDA 2026.1 and declares compatibility through 2026.3.
It needs a 64-bit NVDA process capable of loading the bundled x64 DLL, such as
x64 NVDA or ARM64EC NVDA on Windows ARM. It is **not** for the 32-bit 2025.3.3
release or pure ARM64 NVDA. The plug-in checks pointer width, then lets the
Windows loader decide whether it can load the DLL; a loader error is announced
with its Windows error detail. The earlier preview was confirmed to start on
the user's ARM64EC NVDA. This updated DLL still needs a hearing test on that
installation. No installation or switch of the user's main synthesizer is
performed by the build.
